/**@file PacketConnectionTracker.cpp 
 */
//=============================================================================
// INCLUDES
//=============================================================================
#include "PacketConnectionTracker.h"
#include "Logging.h"
#include "MD5ByteContainer.h"
#include "TCPTracker.h"
#include "UDPTracker.h"
#include "ICMPTracker.h"

//=============================================================================
// IMPLEMENTATION
//=============================================================================
PacketConnectionTracker::PacketConnectionTracker (uint64_t timeout_us, std::string sDisable)
 : m_packetCount(0),
   m_timeout_us(timeout_us),
   m_enable_tcp(true),
   m_enable_udp(true),
   m_enable_icmp(true)
{
    if (sDisable.find(std::string("tcp")) != std::string::npos) {
        m_enable_tcp = false;
    }
    if (sDisable.find(std::string("udp")) != std::string::npos) {
        m_enable_udp = false;
    }
    if (sDisable.find(std::string("icmp")) != std::string::npos) {
        m_enable_icmp = false;
    }
}

void PacketConnectionTracker::prune_connections (const Packet& last_packet) {
    long int seconds = last_packet.timestamp().seconds();
    long int microseconds = last_packet.timestamp().microseconds();

    TCPTracker::GetStaticInstance(m_timeout_us)->prune_connections();
    UDPTracker::GetStaticInstance(m_timeout_us)->prune_connections(seconds, microseconds);    
    ICMPTracker::GetStaticInstance(m_timeout_us)->prune_connections(seconds, microseconds);    
}

void PacketConnectionTracker::on_packet (const Packet& packet) {
    const PDU* pduPtr = packet.pdu();
    long int seconds = packet.timestamp().seconds();
    long int microseconds = packet.timestamp().microseconds();

    const IP* ipHeader = pduPtr->find_pdu<IP>();
    const TCP* tcpHeader = pduPtr->find_pdu<TCP>();
    const UDP* udpHeader = pduPtr->find_pdu<UDP>();
    const ICMP* icmpHeader = pduPtr->find_pdu<ICMP>();
    const RawPDU* raw = pduPtr->find_pdu<RawPDU>();

    if (tcpHeader && m_enable_tcp) {
        TCPTracker::GetStaticInstance(m_timeout_us)->on_packet(packet);
    }
    if (udpHeader && m_enable_udp) {
        UDPTracker::GetStaticInstance(m_timeout_us)->on_packet(packet);
    } 
    if (icmpHeader && m_enable_icmp) {
        ICMPTracker::GetStaticInstance(m_timeout_us)->on_packet(packet);
    }

    m_packetCount++;
}

void PacketConnectionTracker::on_connection (uint64_t cid, std::string hash) {
    PrintLogMessage(
        LEVEL_VERBOSE,
        SUBSYSTEM_CONN_TRACK,
        "on_connection(0x%llx, %s)",
        cid, hash.c_str()
    );
    //g_packetMsgProxy->on_connection(...);
}

size_t PacketConnectionTracker::packet_count (void) {
    return m_packetCount;
}

//=============================================================================
