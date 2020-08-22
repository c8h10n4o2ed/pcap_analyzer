/**@file ICMPTracker.cpp
 */
//=============================================================================
// INCLUDES
//=============================================================================
#include "ICMPTracker.h"
#include "PacketConnectionTracker.h"

//=============================================================================
// DEFINITIONS
//=============================================================================
static std::shared_ptr<ICMPTracker> gs_ICMPTracker = nullptr;
extern std::shared_ptr<PacketMsgProxy> g_packetMsgProxy;

//=============================================================================
// IMPLEMENTATION
//=============================================================================
ICMPTracker::ICMPTracker (uint64_t timeout_us)
  : m_addrList(),
    m_timeout_us(timeout_us),
    m_opened(0),
    m_closed(0),
    m_typenameMap()
{
    m_typenameMap[0] = "ECHO_REPLY";
    m_typenameMap[1] = "RESERVED1";
    m_typenameMap[2] = "RESERVED2";
    m_typenameMap[3] = "DEST_UNREACHABLE";
    m_typenameMap[4] = "SOURCE_QUENCH";
    m_typenameMap[5] = "REDIRECT";
    m_typenameMap[6] = "ALT_HOST";
    m_typenameMap[7] = "RESERVED7";
    m_typenameMap[8] = "ECHO_REQUEST";
    m_typenameMap[9] = "ROUTER_ADVERTISEMENT";
    m_typenameMap[10] = "ROUTER_SOLICITATION";
    m_typenameMap[11] = "TIME_EXCEEDED";
    m_typenameMap[12] = "PARAM_PROBLEM";
    m_typenameMap[13] = "TIMESTAMP_REQUEST";
    m_typenameMap[14] = "TIMESTAMP_REPLY";
    m_typenameMap[15] = "INFO_REQUEST";
    m_typenameMap[16] = "INFO_REPLY";
    m_typenameMap[17] = "ADDRESS_MASK_REQUEST";
    m_typenameMap[18] = "ADDRESS_MASK_REPLY";
    m_typenameMap[19] = "RESERVED";
    m_typenameMap[20] = "RESERVED";
    m_typenameMap[21] = "RESERVED";
    m_typenameMap[22] = "RESERVED";
    m_typenameMap[23] = "RESERVED";
    m_typenameMap[24] = "RESERVED";
    m_typenameMap[25] = "RESERVED";
    m_typenameMap[26] = "RESERVED";
    m_typenameMap[27] = "RESERVED";
    m_typenameMap[28] = "RESERVED";
    m_typenameMap[29] = "RESERVED";
    m_typenameMap[30] = "TRACEROUTE";
    m_typenameMap[31] = "DGM_CONVERT_ERROR";
    m_typenameMap[32] = "MOBILE_HOST_REDIR";
    m_typenameMap[42] = "EXTENDED_ECHO_REQ";
    m_typenameMap[43] = "EXTENDED_ECHO_REPLY";
}

ICMPTracker::~ICMPTracker (void)
{
    m_addrList.clear();
}

std::string ICMPTracker::get_type_name (long msgtype) {
    std::string sRetVal = "";
    if (m_typenameMap.count(msgtype) > 0) {
        sRetVal = m_typenameMap[msgtype];
    }
    /*
    for (size_t i=0; i<sizeof(msgtype)*8; i++) {
        auto bit = (msgtype >> (sizeof(msgtype)*8 - i)) & 0x1;
        auto bit_next = (msgtype >> (sizeof(msgtype)*8 - i + 1)) & 0x1;
        if (bit && (m_typenameMap.count(i) > 0)) {
            if (bit_next) {
                sRetVal += m_typenameMap[i] + ",";
            } else {
                sRetVal += m_typenameMap[i];
            }
        }
    }
    */
    return sRetVal;
}

void ICMPTracker::on_packet (const Packet& packet) {
    const PDU* pduPtr = packet.pdu();
    long int seconds = packet.timestamp().seconds();
    long int microseconds = packet.timestamp().microseconds();

    const IP* ipHeader = pduPtr->find_pdu<IP>();
    const ICMP* ICMPHeader = pduPtr->find_pdu<ICMP>();
    const RawPDU* raw = pduPtr->find_pdu<RawPDU>();

    if (!ICMPHeader || !ipHeader) {
        return;
    }

    ICMPAddressTuple hdrTemp;
    hdrTemp.src = ipHeader->src_addr();
    hdrTemp.dst = ipHeader->dst_addr();
    hdrTemp.dport = 0;
    hdrTemp.sport = 0;
    hdrTemp.timestamp_s = seconds;
    hdrTemp.timestamp_us = microseconds;
    hdrTemp.last_active_s = seconds;
    hdrTemp.last_active_us = microseconds;
    hdrTemp.state = ICMP_ACTIVE;
    hdrTemp.msgtype = ICMPHeader->type();
    hdrTemp.seqnum = ICMPHeader->sequence();

    auto ctmp = find_icmp(m_addrList.begin(),
                          m_addrList.end(),
                          hdrTemp,
                          seconds,
                          microseconds,
                          m_timeout_us);

    if (ctmp != m_addrList.end()) {
        if ((*ctmp).state != ICMP_CLOSED) {
            auto cm = ConnectionMetadata();
            cm.src = (*ctmp).src;
            cm.dst = (*ctmp).dst;
            cm.l4_dst = (*ctmp).dport;
            cm.l4_src = (*ctmp).sport;
            cm.msgtype = (*ctmp).msgtype;
            cm.seqnum = (*ctmp).seqnum;
            cm.protocol = 1;
            cm.l4_protocol = 0;
            cm.timestamp_s = (*ctmp).timestamp_s;
            cm.timestamp_us = (*ctmp).timestamp_us;
            cm.update_hash();

            if ((((*ctmp).last_active_s * (10^6) + 
                  (*ctmp).last_active_us) + m_timeout_us) >
                (seconds * (10^6) + microseconds)) {
                (*ctmp).last_active_s = seconds;
                (*ctmp).last_active_us = microseconds;
            } else {
                (*ctmp).state = ICMP_CLOSED;
                cm.end_timestamp_s = seconds;
                cm.end_timestamp_us = microseconds;
                cm.update_hash();

                #if 1
                PrintLogMessage(
                    LEVEL_DEBUG,
                    SUBSYSTEM_ICMP,
                    "ICMP CLOSE %-15s: %-15s -> %-15s:%02x/%s (seqnum = %u)",
                    cm.hash.c_str(),
                    cm.src_str().c_str(),
                    cm.dst_str().c_str(),
                    (*ctmp).msgtype,
                    get_type_name((*ctmp).msgtype).c_str(),
                    (*ctmp).seqnum
                );
                #endif

                m_closed++;
                g_packetMsgProxy->on_end_connection(&cm);
            }
        }
    } else {
        auto cm = ConnectionMetadata();
        cm.src = ipHeader->src_addr();
        cm.dst = ipHeader->dst_addr();
        cm.l4_dst = 0;
        cm.l4_src = 0;
        cm.protocol = 1;
        cm.l4_protocol = 0;
        cm.msgtype = ICMPHeader->type();
        cm.seqnum = ICMPHeader->sequence();
        cm.timestamp_s = seconds;
        cm.timestamp_us = microseconds;
        cm.update_hash();

        m_addrList.push_back(hdrTemp);

        PrintLogMessage(
            LEVEL_DEBUG,
            SUBSYSTEM_ICMP,
            "ICMP %-15s: %-15s -> %-15s:%02x/%s (seqnum = %u)",
            cm.hash.c_str(),
            cm.src_str().c_str(),
            cm.dst_str().c_str(), ICMPHeader->type(), get_type_name(ICMPHeader->type()).c_str(), ICMPHeader->sequence()
        );

        m_opened++;

        g_packetMsgProxy->on_connection(&cm);
    }
}

void ICMPTracker::prune_connections (
    uint64_t last_s,
    uint64_t last_us
) {
    bool done = false;
    auto iter = m_addrList.begin();
    while (!done) {
    for_start:
        for (;iter != m_addrList.end(); iter++) {
            if (((*iter).state == ICMP_CLOSED) ||
                ((((*iter).last_active_s) * 10^6 + (*iter).last_active_us + m_timeout_us) < 
                 (last_s * (10^6) + last_us))) {

                auto cm = ConnectionMetadata();
                cm.src = (*iter).src;
                cm.dst = (*iter).dst;
                cm.l4_dst = (*iter).dport;
                cm.l4_src = (*iter).sport;
                cm.protocol = 1;
                cm.l4_protocol = 0;
                cm.msgtype = (*iter).msgtype;
                cm.seqnum = (*iter).seqnum;
                cm.timestamp_s = (*iter).timestamp_s;
                cm.timestamp_us = (*iter).timestamp_us;
                cm.update_hash();

                #if 1
                PrintLogMessage(
                    LEVEL_DEBUG,
                    SUBSYSTEM_ICMP,
                    "ICMP CLOSE %-15s: %-15s-> %-15s:%02x/%s (seqnum = %u)",
                    cm.hash.c_str(),
                    cm.src_str().c_str(),
                    cm.dst_str().c_str(), (*iter).msgtype, get_type_name((*iter).msgtype).c_str(), (*iter).seqnum
                );
                #endif

                g_packetMsgProxy->on_end_connection(&cm);

                m_closed++;
                iter = m_addrList.erase(iter);
                goto for_start;
            }
        }
        done = true;
    }
}

void ICMPTracker::on_state_update (const Packet& p) {

}

size_t ICMPTracker::get_opened (void) {
    return m_opened;
}

size_t ICMPTracker::get_closed (void) {
    return m_closed;
}

std::shared_ptr<ICMPTracker> ICMPTracker::GetStaticInstance (uint64_t timeout_us) {
    if (gs_ICMPTracker == nullptr) {
        gs_ICMPTracker = std::make_shared<ICMPTracker>(timeout_us);
    }
    return gs_ICMPTracker;
}

//=============================================================================
