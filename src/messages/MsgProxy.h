/** @file MsgProxy.h
 */
#ifndef MSG_PROXY_H_
#define MSG_PROXY_H_
//=============================================================================
// INCLUDES 
//=============================================================================
#include <string>
#include <zmq.h>
#include "MsgCommon.h"

//=============================================================================
// DEFINITIONS
//=============================================================================
/** 
 * Valid types of message proxies. 
 */
typedef enum {
    MSG_PROXY_INPROC,
    MSG_PROXY_TCP,
    MSG_PROXY_IPC
} MsgProxyType_T;

/**
 * This object is responsible for sending and receiving synchronous/asynchronous
 * messages.
 */
class MsgProxy {
public:
    MsgProxy (
        std::string dstSocketName,
        MsgProxyType_T msgProxyType,
        int zmqSocketType = ZMQ_REQ
    );
    virtual ~MsgProxy (void);

    /**
     * Connects to the remote server socket.
     * @return bool true on success, false on failure
     */
    virtual bool connect (void);

    /**
     * Disconnects from the remote server socket.
     * @return bool true on success, false on failure
     */
    virtual bool disconnect (void);

    /**
     * Sends a message to the remote server socket. 
     *  
     * This method copies pData prior to sending data to the remote socket. 
     *  
     * @param pData 
     * @param dataSize 
     * @return bool true on success, false on failure 
     */
    virtual bool sendMessage (void* pData, uint32_t dataSize);

    /**
     * Identical to the sendMessage(pData,dataSize) method except that it specifies 
     * the socket to send a message on. 
     *  
     * @param pSocket 
     * @param pData 
     * @param dataSize 
     * @return bool 
     */
    virtual bool sendMessage (void* pSocket, void* pData, uint32_t dataSize);

    /**
     * Receives a message from a remote server. 
     *  
     * This method copies into pData up to maxDataSize. 
     *  
     * @param pData The output buffer
     * @param maxDataSize Maximum size of output buffer
     * @return bool true on success, false on failure
     */
    virtual bool receiveMessage (void* pData, uint32_t maxDataSize);

    /**
     * Identical to the receiveMessage(pData,maxDataSize) method except that it 
     * specifies the socket to receive a message from. 
     *  
     * @param pSocket 
     * @param pData 
     * @param maxDataSize 
     * @return bool 
     */
    virtual bool receiveMessage (void* pSocket, void* pData, uint32_t maxDataSize);
    
    /**
     * Receives a message without first knowing how large the message is. 
     *  
     * The *ppData buffer must eventually be freed by the caller. 
     *  
     * @param ppData Pointer that is populated with buffer
     * @return bool true on success, false on failure
     */
    virtual bool receiveMessageAlloc (void** ppData);

    /**
     * Identical to the receiveMessageAlloc(ppData) method except that it specifies 
     * the socket that data is being received on. 
     *  
     * @param pSocket 
     * @param ppData 
     * @return bool 
     */
    virtual bool receiveMessageAlloc (void* pSocket, void** ppData);

protected:
    std::string m_socketName;
    MsgProxyType_T m_proxyType;

    void* m_pContext;
    void* m_pSocket;
    int m_zmqSocketType;
};

//============================================================================= 
#endif
