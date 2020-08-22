/** @file MsgCommon.h
 */
#ifndef MSG_COMMON_H_
#define MSG_COMMON_H_
//=============================================================================
// INCLUDES 
//=============================================================================
#include <stdint.h>

//=============================================================================
// DEFINITIONS
//============================================================================= 
#define MSG_ACK_TRUE        (1)
#define MSG_ACK_FALSE       (0)

#define MSG_INVALID_BLOCK_ID (0)

#define MSG_COMMON          (0x8629d1c3)
#define MSG_COMMON_ACK      (0x8629d1c300000001)
#define MSG_COMMON_NACK     (0x8629d1c300000002)
#define MSG_COMMON_SHUTDOWN (0x8629d1c300000003)

/**
 * This structure is the most basic message type consisting of only the message 
 * type and no other data. 
 */
typedef struct {
    uint64_t msgType;
} MsgCommon_T;

//=============================================================================
// FUNCTION DEFINITIONS
//=============================================================================
/**
 * Receives a string from a ZMQ socket. The caller must free the buffer returned 
 * by this routine. The default string size supported by this method is 1 KiB 
 * allocated on the stack. 
 * 
 * @note This routine should be used for receiving identity information from a 
 *       ZMQ socket operating in ROUTER mode and not for transferring large
 *       amounts of data.
 *  
 * @note Method is based on zguide / ZeroMQ documentation for interacting with 
 *       ROUTER sockets.
 *  
 * @param pSocket Socket to receive data from
 * @return char* Copy of the string received from socket
 */
char* ZMQ_RecvString (void* pSocket);

/**
 * This method functions identically to ZMQ_RecvString, however the buffer does 
 * not need to be freed since it is supplied by the caller. 
 *  
 * @param pSocket Socket to receive data from
 * @param pString String buffer to populate
 * @param maxLength Maximum length of pString
 * @return size_t Number of bytes received
 */
uint64_t ZMQ_RecvStringNoCopy (void* pSocket, char* pString, uint64_t maxLength);

//============================================================================= 
#endif //MSG_COMMON_H_
