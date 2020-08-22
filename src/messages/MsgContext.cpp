/** @file MsgContext.cpp
 */ 
//============================================================================= 
// INCLUDES
//============================================================================= 
#include "Logging.h"
#include "MsgContext.h"
#include <zmq.h>

//============================================================================= 
// DEFINITIONS
//============================================================================= 
static void* gs_pZmqContext = NULL;

//============================================================================= 
// IMPLEMENTATION
//============================================================================= 
void ZMQ_ContextSet (void* pContext) {
    if ( gs_pZmqContext ) {
        PrintLogMessage(
            LEVEL_WARNING,
            SUBSYSTEM_ZMQ,
            "Reinitialized ZMQ context"
        );
    }
    gs_pZmqContext = pContext;
}

void* ZMQ_ContextGet (void) {
    return gs_pZmqContext;
}

bool InitializeZMQ (void) {
    void* context = zmq_ctx_new();
    ZMQ_ContextSet(context);
    PrintLogMessage(
        LEVEL_DEBUG,
        SUBSYSTEM_ZMQ,
        "Initialized ZMQ context"
    );
    return true;
}

//============================================================================= 

