/**@file PacketMsgProxy.h 
 */
#ifndef PACKET_MSG_PROXY_H_
#define PACKET_MSG_PROXY_H_
//=============================================================================
// INCLUDES
//=============================================================================
#include "MsgProxy.h"
#include "PacketConnectionTracker.h"
#include <string>
#include <stdint.h>

//=============================================================================
// DEFINITIONS
//=============================================================================
class ConnectionMetadata;
class PacketMsgProxy : public MsgProxy {
public:
    PacketMsgProxy (std::string sConnectStr);
    virtual ~PacketMsgProxy (void);

    /**
     * Notifies the ZMQ host of a new connection. 
     */
    virtual bool on_connection (
        const ConnectionMetadata* meta
    );

    /**
     * Updates the end timestamp of a connection as well as the 
     * state. 
     * 
     * @param meta 
     * @return bool 
     */
    virtual bool on_end_connection (
        const ConnectionMetadata* meta
    );

    virtual void sync (void);

    /**
     * Notifies the ZMQ host that a connection has ended. 
     *  
     * @param seconds 
     * @param microseconds 
     * @param hash 
     * @return bool 
     */
    #if 0
    virtual bool on_connection_end (
        long int seconds,
        long int microseconds,
        std::string hash
    );
    #endif
};

//=============================================================================
#endif //PACKET_MSG_PROXY_H_
