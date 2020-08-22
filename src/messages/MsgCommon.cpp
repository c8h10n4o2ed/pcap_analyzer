/** @file MsgCommon.cpp
 */
//=============================================================================
// INCLUDES 
//============================================================================= 
#include <zmq.h>
#include <string.h>
#include "MsgCommon.h"
#include "Logging.h"

//============================================================================= 
// IMPLEMENTATION
//============================================================================= 
char* ZMQ_RecvString (void* pSocket) {
    char tempBuffer[1024];

    memset(tempBuffer, 0, sizeof(tempBuffer));
    zmq_recv(pSocket, tempBuffer, sizeof(tempBuffer), 0);
    tempBuffer[sizeof(tempBuffer) - 1] = '\0';

    return strdup(tempBuffer);
}

uint64_t ZMQ_RecvStringNoCopy (void* pSocket, char* pString, uint64_t maxLength) {
    uint64_t bytesRecv;
    memset(pString, 0, maxLength);
    bytesRecv = zmq_recv(pSocket, pString, maxLength, 0);
    pString[maxLength - 1] = '\0';
    return bytesRecv;
}

//=============================================================================
