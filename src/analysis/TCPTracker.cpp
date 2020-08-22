/**@file TCPTracker.cpp
 */
//=============================================================================
// INCLUDES
//=============================================================================
#include "TCPTracker.h"
#include "PacketConnectionTracker.h"

//=============================================================================
// DEFINITIONS
//=============================================================================
static std::shared_ptr<TCPTracker> gs_tcpTracker = nullptr;
extern std::shared_ptr<PacketMsgProxy> g_packetMsgProxy;

//=============================================================================
// IMPLEMENTATION
//=============================================================================
TCPTracker::TCPTracker (uint64_t timeout_us)
  : m_addrList(),
    m_timeout_us(timeout_us),
    m_opened(0),
    m_closed(0)
{
}

TCPTracker::~TCPTracker (void)
{
    m_addrList.clear();
}

void TCPTracker::on_packet (const Packet& packet) {
    const PDU* pduPtr = packet.pdu();
    long int seconds = packet.timestamp().seconds();
    long int microseconds = packet.timestamp().microseconds();

    const IP* ipHeader = pduPtr->find_pdu<IP>();
    const TCP* tcpHeader = pduPtr->find_pdu<TCP>();
    const RawPDU* raw = pduPtr->find_pdu<RawPDU>();

    if (!tcpHeader || !ipHeader) {
        return;
    }

    TCPAddressTuple hdrTemp;
    hdrTemp.src = ipHeader->src_addr();
    hdrTemp.dst = ipHeader->dst_addr();
    hdrTemp.dport = tcpHeader->dport();
    hdrTemp.sport = tcpHeader->sport();
    hdrTemp.timestamp_s = seconds;
    hdrTemp.timestamp_us = microseconds;
    hdrTemp.state = TCP_LISTEN;

    auto ctmp = find_tcp(m_addrList.begin(), m_addrList.end(), hdrTemp);
    if (ctmp != m_addrList.end()) {
        if ((*ctmp).state != TCP_CLOSED &&
            tcpHeader->get_flag(TCP::FIN)) {
            auto cm = ConnectionMetadata();
            cm.src = (*ctmp).src;
            cm.dst = (*ctmp).dst;
            cm.l4_dst = (*ctmp).dport;
            cm.l4_src = (*ctmp).sport;
            cm.protocol = 6;
            cm.l4_protocol = 6;
            cm.timestamp_s = (*ctmp).timestamp_s;
            cm.timestamp_us = (*ctmp).timestamp_us;
            cm.end_timestamp_s = seconds;
            cm.end_timestamp_us = microseconds;
            cm.update_hash();
            (*ctmp).state = TCP_CLOSED;

            PrintLogMessage(
                LEVEL_DEBUG,
                SUBSYSTEM_TCP,
                "TCP CLOSE %-15s: %-15s:%5u -> %-15s:%5u",
                cm.hash.c_str(),
                cm.src_str().c_str(), (*ctmp).sport,
                cm.dst_str().c_str(), (*ctmp).dport
            );

            m_closed++;

            g_packetMsgProxy->on_end_connection(&cm);
        }
    } else if (tcpHeader->get_flag(TCP::SYN) &&
               tcpHeader->get_flag(TCP::ACK)) {
        auto cm = ConnectionMetadata();
        cm.src = ipHeader->dst_addr();  //transposed s/d addrs
        cm.dst = ipHeader->src_addr();
        cm.l4_dst = tcpHeader->sport(); //transposed s/d ports
        cm.l4_src = tcpHeader->dport();
        cm.protocol = 6;
        cm.l4_protocol = 6;
        cm.timestamp_s = seconds;
        cm.timestamp_us = microseconds;
        cm.update_hash();

        m_addrList.push_back(hdrTemp);

        PrintLogMessage(
            LEVEL_DEBUG,
            SUBSYSTEM_TCP,
            "TCP OPEN  %-15s: %-15s:%5u -> %-15s:%5u",
            cm.hash.c_str(),
            cm.src_str().c_str(), cm.l4_src,
            cm.dst_str().c_str(), cm.l4_dst
        );

        m_opened++;

        g_packetMsgProxy->on_connection(&cm);
    }
}

void TCPTracker::prune_connections (void) {
    bool done = false;
    auto iter = m_addrList.begin();
    while (!done) {
    for_start:
        for (;iter != m_addrList.end(); iter++) {
            if ((*iter).state == TCP_CLOSED) {
                iter = m_addrList.erase(iter);
                goto for_start;
            }
        }
        done = true;
    }
}

void TCPTracker::on_state_update (const Packet& p) {

}

size_t TCPTracker::get_opened (void) {
    return m_opened;
}

size_t TCPTracker::get_closed (void) {
    return m_closed;
}

std::shared_ptr<TCPTracker> TCPTracker::GetStaticInstance (uint64_t timeout_us) {
    if (gs_tcpTracker == nullptr) {
        gs_tcpTracker = std::make_shared<TCPTracker>(timeout_us);
    }
    return gs_tcpTracker;
}

//=============================================================================
