/**@file UDPTracker.cpp
 */
//=============================================================================
// INCLUDES
//=============================================================================
#include "UDPTracker.h"
#include "PacketConnectionTracker.h"

//=============================================================================
// DEFINITIONS
//=============================================================================
static std::shared_ptr<UDPTracker> gs_UDPTracker = nullptr;
extern std::shared_ptr<PacketMsgProxy> g_packetMsgProxy;

//=============================================================================
// IMPLEMENTATION
//=============================================================================
UDPTracker::UDPTracker (uint64_t timeout_us)
  : m_addrList(),
    m_timeout_us(timeout_us),
    m_opened(0),
    m_closed(0)
{
}

UDPTracker::~UDPTracker (void)
{
    m_addrList.clear();
}

void UDPTracker::on_packet (const Packet& packet) {
    const PDU* pduPtr = packet.pdu();
    long int seconds = packet.timestamp().seconds();
    long int microseconds = packet.timestamp().microseconds();

    const IP* ipHeader = pduPtr->find_pdu<IP>();
    const UDP* UDPHeader = pduPtr->find_pdu<UDP>();
    const RawPDU* raw = pduPtr->find_pdu<RawPDU>();

    if (!UDPHeader || !ipHeader) {
        return;
    }

    UDPAddressTuple hdrTemp;
    hdrTemp.src = ipHeader->src_addr();
    hdrTemp.dst = ipHeader->dst_addr();
    hdrTemp.dport = UDPHeader->dport();
    hdrTemp.sport = UDPHeader->sport();
    hdrTemp.timestamp_s = seconds;
    hdrTemp.timestamp_us = microseconds;
    hdrTemp.last_active_s = seconds;
    hdrTemp.last_active_us = microseconds;
    hdrTemp.state = UDP_ACTIVE;

    auto ctmp = find_udp(m_addrList.begin(),
                         m_addrList.end(),
                         hdrTemp,
                         seconds,
                         microseconds,
                         m_timeout_us);

    if (ctmp != m_addrList.end()) {
        if ((*ctmp).state != UDP_CLOSED) {
            auto cm = ConnectionMetadata();
            cm.src = (*ctmp).src;
            cm.dst = (*ctmp).dst;
            cm.l4_dst = (*ctmp).dport;
            cm.l4_src = (*ctmp).sport;
            cm.protocol = 17;
            cm.l4_protocol = 17;
            cm.timestamp_s = (*ctmp).timestamp_s;
            cm.timestamp_us = (*ctmp).timestamp_us;
            cm.update_hash();

            if ((((*ctmp).last_active_s * (10^6) + 
                  (*ctmp).last_active_us) + m_timeout_us) >
                (seconds * (10^6) + microseconds)) {
                (*ctmp).last_active_s = seconds;
                (*ctmp).last_active_us = microseconds;
            } else {
                (*ctmp).state = UDP_CLOSED;
                cm.end_timestamp_s = seconds;
                cm.end_timestamp_us = microseconds;
                cm.update_hash();

                PrintLogMessage(
                    LEVEL_DEBUG,
                    SUBSYSTEM_UDP,
                    "UDP CLOSE %-15s: %-15s:%5u -> %-15s:%5u",
                    cm.hash.c_str(),
                    cm.src_str().c_str(), (*ctmp).sport,
                    cm.dst_str().c_str(), (*ctmp).dport
                );

                m_closed++;

                g_packetMsgProxy->on_end_connection(&cm);
            }
        }
    } else {
        auto cm = ConnectionMetadata();
        cm.src = ipHeader->src_addr();
        cm.dst = ipHeader->dst_addr();
        cm.l4_dst = UDPHeader->dport();
        cm.l4_src = UDPHeader->sport();
        cm.protocol = 17;
        cm.l4_protocol = 17;
        cm.timestamp_s = seconds;
        cm.timestamp_us = microseconds;
        cm.seqnum = 0;
        cm.msgtype = 0;
        cm.update_hash();

        m_addrList.push_back(hdrTemp);

        PrintLogMessage(
            LEVEL_DEBUG,
            SUBSYSTEM_UDP,
            "UDP OPEN  %-15s: %-15s:%5u -> %-15s:%5u",
            cm.hash.c_str(),
            cm.src_str().c_str(), cm.l4_src,
            cm.dst_str().c_str(), cm.l4_dst
        );

        m_opened++;

        g_packetMsgProxy->on_connection(&cm);
    }
}

void UDPTracker::prune_connections (
    uint64_t last_s,
    uint64_t last_us
) {
    bool done = false;
    auto iter = m_addrList.begin();
    while (!done) {
    for_start:
        for (;iter != m_addrList.end(); iter++) {
            if (((*iter).state == UDP_CLOSED) ||
                ((((*iter).last_active_s) * 10^6 + (*iter).last_active_us + m_timeout_us) < 
                 (last_s * (10^6) + last_us))) {

                auto cm = ConnectionMetadata();
                cm.src = (*iter).src;
                cm.dst = (*iter).dst;
                cm.l4_dst = (*iter).dport;
                cm.l4_src = (*iter).sport;
                cm.protocol = 17;
                cm.l4_protocol = 17;
                cm.timestamp_s = (*iter).timestamp_s;
                cm.timestamp_us = (*iter).timestamp_us;
                cm.update_hash();

                PrintLogMessage(
                    LEVEL_DEBUG,
                    SUBSYSTEM_UDP,
                    "UDP CLOSE %-15s: %-15s:%5u -> %-15s:%5u",
                    cm.hash.c_str(),
                    cm.src_str().c_str(), (*iter).sport,
                    cm.dst_str().c_str(), (*iter).dport
                );

                g_packetMsgProxy->on_end_connection(&cm);

                m_closed++;
                iter = m_addrList.erase(iter);
                goto for_start;
            }
        }
        done = true;
    }
}

void UDPTracker::on_state_update (const Packet& p) {

}

size_t UDPTracker::get_opened (void) {
    return m_opened;
}

size_t UDPTracker::get_closed (void) {
    return m_closed;
}

std::shared_ptr<UDPTracker> UDPTracker::GetStaticInstance (uint64_t timeout_us) {
    if (gs_UDPTracker == nullptr) {
        gs_UDPTracker = std::make_shared<UDPTracker>(timeout_us);
    }
    return gs_UDPTracker;
}

//=============================================================================
