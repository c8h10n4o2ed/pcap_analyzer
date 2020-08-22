/** @file MsgContext.h
 */ 
#ifndef MSG_CONTEXT_H_
#define MSG_CONTEXT_H_
//============================================================================= 
// DEFINITIONS
//============================================================================= 
/**
 * Initializes the ZMQ messaging library
 */
bool InitializeZMQ (void);

/**
 * Sets the current ZMQ context
 * @param pContext ZMQ context
 */
void ZMQ_ContextSet (void* pContext);

/**
 * Retrieves the current ZMQ context
 * @return void* ZMQ context
 */
void* ZMQ_ContextGet (void);

//============================================================================= 
#endif //MSG_CONTEXT_H_
