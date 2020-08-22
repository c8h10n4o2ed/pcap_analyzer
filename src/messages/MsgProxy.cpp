/** @file MsgProxy.cpp
 */
//=============================================================================
// INCLUDES 
//============================================================================ 
#include "Logging.h"
#include "MsgProxy.h"
#include "MsgContext.h"
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

//=============================================================================
// IMPLEMENTATION
//============================================================================ 

MsgProxy::MsgProxy (std::string dstSocketName, MsgProxyType_T msgProxyType, int zmqSocketType) :
    m_pContext(NULL),
    m_pSocket(NULL),
    m_zmqSocketType(zmqSocketType)
{
    m_proxyType = msgProxyType;
    m_socketName = dstSocketName;

    connect();
}

MsgProxy::~MsgProxy (void) {
    disconnect();
}

bool MsgProxy::disconnect (void) {
    if ( m_pSocket ) {
        zmq_close(m_pSocket);
        m_pSocket = NULL;
        return true;
    } else {
        PrintLogMessage(
            LEVEL_WARNING,
            SUBSYSTEM_ZMQ,
            "Disconnected NULL socket"
        );
        return false;
    }
}

bool MsgProxy::connect (void) {
    char socketIdentity[64];

    //Check for context
    if ( !m_pContext ) {
        m_pContext = ZMQ_ContextGet();
        if ( !m_pContext ) {
            goto ErrorExit;
        }
    }

    if ( !m_pSocket ) {        
        //m_pSocket = zmq_socket(m_pContext, ZMQ_REQ);
        m_pSocket = zmq_socket(m_pContext, m_zmqSocketType);

        //Set a printable socket ID
        //This is essential for correct functionality of ZMQ router sockets
        snprintf(
            socketIdentity, 
            sizeof(socketIdentity),
            "MP_%08x_%08x",
            random(), random()
        );
        zmq_setsockopt(m_pSocket, ZMQ_IDENTITY, socketIdentity, strlen(socketIdentity));

        //PrintSimpleLogMessage(LEVEL_DEBUG, "Using identity %s", socketIdentity);

        if ( !zmq_connect(m_pSocket, m_socketName.c_str()) ) {
            PrintLogMessage(
                LEVEL_DEBUG,
                SUBSYSTEM_ZMQ,
                "Connected to socket (this=0x%016x, socket='%s')",
                this, m_socketName.c_str()
            );
        } else {
            goto ErrorExit;
        }
    } else {
        PrintLogMessage(
            LEVEL_WARNING,
            SUBSYSTEM_ZMQ,
            "Already initialized (this=0x%016x, m_pContext=0x%016x, m_pSocket=0x%016x)",
            this, m_pContext, m_pSocket
        );
    }
    
    return true;

ErrorExit:
    PrintLogMessage(
        LEVEL_ERROR,
        SUBSYSTEM_ZMQ,
        "Unable to connect to socket (this=0x%016x, context=0x%016x, socket='%s')",
        this, m_pContext, m_socketName.c_str()
    );

    if ( m_pSocket ) {
        zmq_close(m_pSocket);
    }
    return false;
}


bool MsgProxy::sendMessage (void* pData, uint32_t dataSize) {
    return sendMessage(m_pSocket, pData, dataSize);
}

bool MsgProxy::sendMessage (void* pSocket, void* pData, uint32_t dataSize) {
    bool retValue = false;
    zmq_msg_t request;

    if ( !pSocket ) {
        goto Exit;
    }

    zmq_msg_init(&request);
    if ( !zmq_msg_init_size(&request, dataSize) ) {
        memcpy(zmq_msg_data(&request), pData, dataSize);
        zmq_msg_send(&request, pSocket, 0);
        zmq_msg_close(&request);

        #if 0
        PrintSimpleLogMessage(
           LEVEL_DEBUG,
           "Sent message 0x%llx of length %d",
           pData, dataSize
        );
        #endif
    } else {
        PrintLogMessage(
            LEVEL_ERROR,
            SUBSYSTEM_ZMQ,
            "Unable to allocate message memory!"
        );
        goto Exit;
    }
    retValue = true;
Exit:
    return retValue;
}

bool MsgProxy::receiveMessageAlloc (void** ppData) {
    return receiveMessageAlloc(m_pSocket, ppData);
}

bool MsgProxy::receiveMessageAlloc (void* pSocket, void** ppData) {
    size_t msgSize;
    zmq_msg_t msg;

    if ( !ppData ) {
        PrintLogMessage(
           LEVEL_ERROR,
           SUBSYSTEM_ZMQ,
           "Invalid parameter ppData!"
        );
    }

    zmq_msg_init(&msg);
    zmq_msg_recv(&msg, pSocket, 0);

    msgSize = zmq_msg_size(&msg);

    #if 0
    if ( msgSize == 0 ) {
        PrintSimpleLogMessage(LEVEL_WARNING, "Received NULL message");
    }
    #endif

    //Allocate sufficient space for message
    *ppData = malloc(msgSize);    
    if ( !*ppData ) {
        PrintLogMessage(
           LEVEL_WARNING,
           SUBSYSTEM_ZMQ,
           "Unable to allocate message (size=%d bytes)",
           msgSize
        );
    } else {    
        memcpy(*ppData, zmq_msg_data(&msg), msgSize);
    }
    zmq_msg_close(&msg);

    return true;
}

bool MsgProxy::receiveMessage (void* pData, uint32_t maxDataSize) {
    return receiveMessage(m_pSocket, pData, maxDataSize);
}

bool MsgProxy::receiveMessage (void* pSocket, void* pData, uint32_t maxDataSize) {
    size_t msgSize;
    size_t copySize;

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmq_msg_recv(&msg, pSocket, 0);

    msgSize = zmq_msg_size(&msg);
    copySize = msgSize;

    if ( msgSize == 0 ) {
        PrintLogMessage(
            LEVEL_WARNING,
            SUBSYSTEM_ZMQ,
            "Received NULL message"
        );
    }

    if ( msgSize > maxDataSize ) {
        PrintLogMessage(
           LEVEL_WARNING,
           SUBSYSTEM_ZMQ,
           "Truncating message (size=%d, maxSize=%d)",
           msgSize, maxDataSize
        );
        copySize = maxDataSize;
    }

    memset(pData, 0, maxDataSize);
    memcpy(pData, zmq_msg_data(&msg), copySize);

    zmq_msg_close(&msg);

    #if 0
    PrintSimpleLogMessage(
       LEVEL_DEBUG,
       "Received message 0x%llx of length %d",
       pData, copySize
    );
    #endif

    return true;
}

//=============================================================================

