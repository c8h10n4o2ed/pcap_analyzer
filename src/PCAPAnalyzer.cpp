/**@file PCAPAnalyzer.cpp 
 */
//=============================================================================
// INCLUDES
//=============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <thread>
#include <unistd.h>
#include <string>
#include <vector>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <pcrecpp.h>
#define TINS_IS_CXX11 1
#include <tins/tins.h>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <stdexcept>
#include "MsgContext.h"

//Darwin only
#ifdef _DARWIN
#include <libgen.h>
#endif

#include "argparse.hpp"
#include "Logging.h"
#include "PCAPSorter.h"

//Analysis
#include "PacketAnalyzer.h"
#include "PacketConnectionTracker.h"
#include "PacketHash.h"
#include "PacketMsgProxy.h"

#include "TCPTracker.h"
#include "UDPTracker.h"
#include "ICMPTracker.h"

//=============================================================================
// DEFINITIONS
//=============================================================================
using namespace Tins;

struct Args {
    argparse::ArgValue<bool> verbose;
    argparse::ArgValue<bool> flush_table;
    argparse::ArgValue<std::string> directory;
    argparse::ArgValue<std::string> output;
    argparse::ArgValue<std::string> zmq;
    argparse::ArgValue<std::string> config;
    argparse::ArgValue<uint64_t> timeout;
    argparse::ArgValue<std::string> disable;
};

size_t g_packetCounter = 0;
size_t g_totalPacketCounter = 0;
FILE* g_fpOutput = NULL;
long int g_startTime = 0xFFFFFFFFFFFFFFF;
long int g_startTimeUs = 0xFFFFFFFFFFFFFFF;
long int g_stopTime = 0;
long int g_stopTimeUs = 0;
std::shared_ptr<PacketMsgProxy> g_packetMsgProxy = nullptr;
std::shared_ptr<PacketConnectionTracker> g_connTracker = nullptr;
static Packet gs_last_packet;

bool pcap_on_packet (const Packet& packet);
bool pcap_process_file (std::string sFile, std::string sOutput);
std::vector<std::string> pcap_get_dir_listing (std::string sDir, std::string sPattern);
std::string timestamp_to_string (long int seconds, long int us_partial);

//=============================================================================
// IMPLEMENTATION
//=============================================================================
void SignalHandler (int signal) {
    PrintSimpleLogMessage(LEVEL_DEBUG, "Received signal %d, shutting down", signal);
}

void InitializeSigterm (void) {
    struct sigaction action;
    action.sa_handler = SignalHandler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
}

std::string timestamp_to_string (long int seconds, long int us_partial) {
    std::string retVal;
    char tmpBuf[512];

    std::time_t ts = (std::time_t)seconds;
    auto lt = std::localtime(&ts);

    snprintf(tmpBuf, sizeof(tmpBuf), "%02u:%02u:%02u.%06lu %02u/%02u/%04u",
             lt->tm_hour, lt->tm_min, lt->tm_sec, us_partial,
             1 + lt->tm_mon, lt->tm_mday, 1900 + lt->tm_year);

    retVal += std::string(tmpBuf);
    return retVal;
}

bool pcap_on_packet (const Packet& packet) {
    bool retValue = true;    
    
    const PDU* pduPtr = packet.pdu();
    long int seconds = packet.timestamp().seconds();
    long int microseconds = packet.timestamp().microseconds();
    
    if (seconds < g_startTime &&
        microseconds < g_startTimeUs) {
        g_startTime = seconds;
        g_startTimeUs = microseconds;
    }

    if (seconds >= g_stopTime ||
        ((seconds > g_stopTime) && microseconds >= g_stopTimeUs)) {
        g_stopTime = seconds;
        g_stopTimeUs = microseconds;
    }

    g_connTracker->on_packet(packet);
    gs_last_packet = packet;
    g_packetCounter++;
    //Continue looping by returning true
Exit:
    return retValue;
}

std::vector<std::string> pcap_get_dir_listing (std::string sDir, std::string sPattern) {
    struct stat fs;
    struct dirent* pDirEntry = NULL;
    DIR* pDir;
    pcrecpp::RE fileSearchRegex(sPattern);
    std::vector<std::string> retValue;
    
    //Stat the directory
    if (stat(sDir.c_str(), &fs) == -1) {
        printf("Failed to stat dir: %s", sDir.c_str());
        goto Exit;
    }

    //Verify that this is a directory
    if (!(fs.st_mode & S_IFDIR)) {
        printf("Path provided is not a directory, skipping!");
        goto Exit;
    }

    //Get the file listing
    pDir = opendir(sDir.c_str());
    if (!pDir) {
        printf("Unable to access directory!");
        goto Exit;
    }

    do {
        std::string sName;

        pDirEntry = readdir(pDir);
        if (!pDirEntry) {
            break;
        }

        if (!(pDirEntry->d_type & DT_REG)) {
            //Skipping irregular files
            continue;
        }

        sName = pDirEntry->d_name;
        if (fileSearchRegex.FullMatch(sName)) {
            retValue.push_back(sDir + "/" + sName);
        }
    } while (pDirEntry);

    closedir(pDir);
    
Exit:
    return retValue;
}

bool pcap_process_file (std::string sFile, std::string sOutput) {
    bool retValue = false;
    FileSniffer sniffer(sFile.c_str());

    #if 0
    g_fpOutput = fopen(sOutput.c_str(), "a");
    if (!g_fpOutput) {
        PrintSimpleLogMessage(LEVEL_ERROR, "Unable to open output file!");
        return retValue;
    }
    #endif

    std::string cur_sFile = std::string(basename((char*)sFile.c_str()));
    //fprintf(g_fpOutput, "# %s\n", basename(sFile.c_str()));

    g_packetCounter = 0;
    sniffer.sniff_loop(pcap_on_packet);
    g_totalPacketCounter += g_packetCounter;

    //Flush any connections not already sent to the database
    g_packetMsgProxy->sync();

    g_connTracker->prune_connections(gs_last_packet);

    PrintSimpleLogMessage(LEVEL_DEBUG, "%10llu packets in %s", g_packetCounter, sFile.c_str());

    //fclose(g_fpOutput);

    retValue = true;
Exit:
    return retValue;
}


int main (int argc, char* argv[]) {
    InitializeLogging();
    InitializeSigterm();

    PrintSimpleLogMessage(LEVEL_INFO, "Starting pcap_analyzer");
    InitializeZMQ();

    Args args;
    auto parser = argparse::ArgumentParser(argv[0], "pcap_analyzer");

    parser.add_argument(args.verbose, "--verbose", "-v")
        .help("Increase verbosity")
        .default_value("false")
        .action(argparse::Action::STORE_TRUE);
        //.choices({"0", "1", "2"});

    parser.add_argument(args.directory, "--directory", "-d")
        .help("Input directory (PCAPs)")
        .default_value("pcaps");
    
    parser.add_argument(args.output, "--output", "-o")
        .help("Output file")
        .default_value("output.txt");

    parser.add_argument(args.zmq, "--zmq", "-z")
        .help("ZMQ router host string (tcp://ip:port)")
        .default_value("tcp://127.0.0.1:5555");

    parser.add_argument(args.config, "--config", "-c")
        .help("Specify config file")
        .default_value("");

    parser.add_argument(args.flush_table, "--flush", "-f")
        .help("Flushes connection table prior to use")
        .default_value("false")
        .action(argparse::Action::STORE_TRUE);

    parser.add_argument(args.timeout, "--timeout", "-t")
        .help("Set the connection timeout for UDP/TCP/etc.. in milliseconds")
        .default_value("1000");

    parser.add_argument(args.disable, "--disable")
        .help("Disable particular analysis (e.g. --disable tcp,udp,icmp")
        .default_value("");

    parser.parse_args(argc, argv);

    bool bVerbose = args.verbose;
    bool bFlushTable = args.flush_table;
    std::string sDir = args.directory;
    std::string sOutput = args.output;
    std::string sZmq = args.zmq;
    std::string sConfig = args.config;
    uint64_t timeout = args.timeout;
    std::string sDisable = args.disable;

    g_packetMsgProxy = std::make_shared<PacketMsgProxy>(sZmq);
    g_connTracker = std::make_shared<PacketConnectionTracker>(timeout * 1000, sDisable);
   
    PrintSimpleLogMessage(LEVEL_INFO, "Input directory: %s", sDir.c_str());
    PrintSimpleLogMessage(LEVEL_INFO, "ZMQ connection string: %s", sZmq.c_str());
    PrintSimpleLogMessage(LEVEL_DEBUG, "Connection timeout: %lu milliseconds", timeout);

    auto pcapList = pcap_get_dir_listing(sDir, ".*\\.pcap.*");
    PrintSimpleLogMessage(LEVEL_DEBUG, "Processing %u PCAP files", pcapList.size());

    //Sort including proper handling of the number at the end
    //of some of the pcap files.
    std::sort(pcapList.begin(), pcapList.end(), CAPNumericalCompare());

    for (auto pcapFile : pcapList) {
        try {
            pcap_process_file(pcapFile, sOutput);   
        } catch (std::exception& e) {
            PrintSimpleLogMessage(LEVEL_ERROR, "Exception on %s", pcapFile.c_str());
        }        
    }

    PrintSimpleLogMessage(LEVEL_DEBUG, "Total packets: %llu", g_connTracker->packet_count());
    PrintSimpleLogMessage(LEVEL_DEBUG, "TCP connections : %-8llu opened, %-8llu closed (timeout %lu milliseconds)",
                          TCPTracker::GetStaticInstance(timeout)->get_opened(),
                          TCPTracker::GetStaticInstance(timeout)->get_closed(),
                          timeout);
    PrintSimpleLogMessage(LEVEL_DEBUG, "UDP connections : %-8llu opened, %-8llu closed (timeout %lu milliseconds)",
                          UDPTracker::GetStaticInstance(timeout)->get_opened(),
                          UDPTracker::GetStaticInstance(timeout)->get_closed(),
                          timeout);
    PrintSimpleLogMessage(LEVEL_DEBUG, "ICMP connections: %-8llu opened, %-8llu closed (timeout %lu milliseconds)",
                          ICMPTracker::GetStaticInstance(timeout)->get_opened(),
                          ICMPTracker::GetStaticInstance(timeout)->get_closed(),
                          timeout);
    
    auto startTime = timestamp_to_string(g_startTime, g_startTimeUs);
    auto stopTime = timestamp_to_string(g_stopTime, g_stopTimeUs);
    PrintSimpleLogMessage(LEVEL_DEBUG, (std::string("Start Time : ") + startTime).c_str());
    PrintSimpleLogMessage(LEVEL_DEBUG, (std::string("Stop Time  : ") + stopTime).c_str());

    return 0;
}

