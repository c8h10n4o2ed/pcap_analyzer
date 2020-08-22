/**@file PacketConnectionTracker.h
 */
#ifndef PACKET_CONNECTION_HANDLER_H_
#define PACKET_CONNECTION_HANDLER_H_
//=============================================================================
// INCLUDES
//=============================================================================
#define TINS_IS_CXX11 1
#include <string>
#include <string.h>
#include <vector>
#include <tins/tins.h>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <memory>
#include <utility>
#include <algorithm>

#include "Logging.h"
#include "BTree.h"
#include "PacketMsgProxy.h"
#include "MD5ByteContainer.h"

//=============================================================================
// DEFINITIONS
//=============================================================================
using namespace Tins;

class ConnectionMetadata {
public:
    ConnectionMetadata (void) :
        hash(""),
        src(0),
        dst(0),
        protocol(0),
        l4_protocol(0),
        l4_src(0),
        l4_dst(0),
        timestamp_s(0),
        timestamp_us(0),
        end_timestamp_s(0),
        end_timestamp_us(0),
        msgtype(0),
        seqnum(0)
    {
    }

    void update_hash (void) {
        std::string tmp;
        uint32_t addr = src ^ dst;
        uint16_t l4 = l4_src ^ l4_dst;
        uint16_t p = protocol;
        tmp += std::string((char*)&addr, 4);
        //tmp += std::string((char*)&dst, 4);
        tmp += std::string((char*)&p, 2);
        //tmp += std::string((char*)&l4_protocol, 2);
        tmp += std::string((char*)&l4, 2);
        //tmp += std::string((char*)&l4_dst, 2);
        tmp += std::string((char*)&msgtype, 4);
        tmp += std::string((char*)&seqnum, 4);
        tmp += std::string((char*)&timestamp_s, 8);
        tmp += std::string((char*)&timestamp_us, 8);

        MD5ByteContainer md5Container((uint8_t*)tmp.c_str(), tmp.size());
        hash = md5Container.toHexString();
    }

    const std::string src_str (void) const {
        char tmpBuf[512];
        snprintf(tmpBuf, sizeof(tmpBuf),
                 "%u.%u.%u.%u", 
                 (src >> 0) & 0xFF,
                 (src >> 8) & 0xFF,
                 (src >> 16) & 0xFF,
                 (src >> 24) & 0xFF);
        return std::string(tmpBuf);
    }

    const std::string dst_str (void) const {
        char tmpBuf[512];
        snprintf(tmpBuf, sizeof(tmpBuf),
                 "%u.%u.%u.%u", 
                 (dst >> 0) & 0xFF,
                 (dst >> 8) & 0xFF,
                 (dst >> 16) & 0xFF,
                 (dst >> 24) & 0xFF);
        return std::string(tmpBuf);
    }

public:
    std::string hash;
    uint32_t src;
    uint32_t dst;
    uint16_t protocol;
    uint16_t l4_protocol;
    uint16_t l4_src;
    uint16_t l4_dst;
    long int timestamp_s;
    long int timestamp_us;
    long int end_timestamp_s;
    long int end_timestamp_us;
    long msgtype;
    long seqnum;
};

/**
 * This object tracks packets in such a way that connection 
 * metadata is established. When a new connection is made the 
 * resulting metadata is pushed via ZMQ to the database update 
 * script. 
 */
class PacketConnectionTracker {
public:
    PacketConnectionTracker (
        uint64_t timeout_us,
        std::string sDisable
    );

    /**
     * This routine handles packets. 
     *  
     * @param packet Reference to packet just received.
     */
    virtual void on_packet (const Packet& p);

    /**
     * This routine is called whenever a connection is made. 
     *  
     * @param cid Connection ID concatenation of source and 
     *            destination IPv4 addresses. The connection hash is
     *            passed as a secondary parameter.
     * @param hash Connection hash string. 
     */
    virtual void on_connection (uint64_t cid, std::string hash);


    /**
     * Retrieves the packet count. 
     *  
     * @return size_t Packet count.
     */
    virtual size_t packet_count (void);

    virtual void prune_connections (const Packet& last_packet);

protected:
    //BTree<uint64_t, ConnectionMetadata> m_btree;
    size_t m_packetCount;
    uint64_t m_timeout_us;

    bool m_enable_tcp;
    bool m_enable_udp;
    bool m_enable_icmp;
};


//=============================================================================
#endif //PACKET_CONNECTION_HANDLER_H_
