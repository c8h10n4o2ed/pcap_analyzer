/**@file PacketMsgProxy.cpp
 */
//=============================================================================
// INCLUDES
//=============================================================================
#include "PacketMsgProxy.h"
#include "Messages.pb.h"
#include "Logging.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "PacketConnectionTracker.h"

//=============================================================================
// IMPLEMENTATION
//=============================================================================
PacketMsgProxy::PacketMsgProxy (std::string sConnectStr) :
    MsgProxy(sConnectStr, MSG_PROXY_TCP, ZMQ_REQ)
{
}

PacketMsgProxy::~PacketMsgProxy (void)
{
}

bool PacketMsgProxy::on_end_connection (
    const ConnectionMetadata* meta
) {
    bool retValue = false;
    pcap_analyzer::GenericMessage gmsg;

    pcap_analyzer::ConnectionCloseNotify notifyBuf;
    notifyBuf.set_hash(meta->hash);
    notifyBuf.set_timestamp_s(meta->timestamp_s);
    notifyBuf.set_timestamp_us(meta->timestamp_us);
    std::string s = notifyBuf.SerializeAsString();
    size_t msgSize = s.size();

    gmsg.set_data(s);
    gmsg.set_msgtype(pcap_analyzer::GenericMessage_MsgType_CONNECTION_CLOSE_NOTIFY);
    std::string s2 = gmsg.SerializeAsString();
    msgSize = s2.size();

    if (!sendMessage((void*)s2.c_str(), s2.size())) {
        PrintSimpleLogMessage(LEVEL_ERROR, "Unable to send packet");
    } else {
        void* msgData = NULL;

        //Now receive a reply
        if (!receiveMessageAlloc(&msgData)) {
            PrintSimpleLogMessage(LEVEL_ERROR, "Unable to receive message");
        }

        if (msgData) {
            free(msgData);
        }
        
        retValue = true;
    }
Exit:
    return retValue;
}

bool PacketMsgProxy::on_connection (
    const ConnectionMetadata* meta
) {
    bool retValue = false;
    pcap_analyzer::GenericMessage gmsg;

    pcap_analyzer::ConnectionNotify notifyBuf;
    notifyBuf.set_hash(meta->hash);
    notifyBuf.set_timestamp_s(meta->timestamp_s);
    notifyBuf.set_timestamp_us(meta->timestamp_us);
    notifyBuf.set_src(meta->src_str());
    notifyBuf.set_dst(meta->dst_str());
    notifyBuf.set_protocol(meta->protocol);
    notifyBuf.set_l4_protocol(meta->l4_protocol);
    notifyBuf.set_l4_src(meta->l4_src);
    notifyBuf.set_l4_dst(meta->l4_dst);
    notifyBuf.set_msgtype(meta->msgtype);
    notifyBuf.set_seqnum(meta->seqnum);
    std::string s = notifyBuf.SerializeAsString();
    size_t msgSize = s.size();

    gmsg.set_data(s);
    gmsg.set_msgtype(pcap_analyzer::GenericMessage_MsgType_CONNECTION_NOTIFY);
    std::string s2 = gmsg.SerializeAsString();
    msgSize = s2.size();

    if (!sendMessage((void*)s2.c_str(), s2.size())) {
        PrintSimpleLogMessage(LEVEL_ERROR, "Unable to send packet");
    } else {
        void* msgData = NULL;

        //Now receive a reply
        if (!receiveMessageAlloc(&msgData)) {
            PrintSimpleLogMessage(LEVEL_ERROR, "Unable to receive message");
        }

        if (msgData) {
            free(msgData);
        }
        
        retValue = true;
    }
Exit:
    return retValue;
}

void PacketMsgProxy::sync (void) {
    bool retValue = false;
    pcap_analyzer::GenericMessage gmsg;

    gmsg.set_data("");
    gmsg.set_msgtype(pcap_analyzer::GenericMessage_MsgType_SYNC);
    std::string s2 = gmsg.SerializeAsString();
    size_t msgSize = s2.size();

    if (!sendMessage((void*)s2.c_str(), msgSize)) {
        PrintSimpleLogMessage(LEVEL_ERROR, "Unable to send packet");
    } else {
        void* msgData = NULL;

        //Now receive a reply
        if (!receiveMessageAlloc(&msgData)) {
            PrintSimpleLogMessage(LEVEL_ERROR, "Unable to receive message");
        }

        if (msgData) {
            free(msgData);
        }
    }
}

//=============================================================================
