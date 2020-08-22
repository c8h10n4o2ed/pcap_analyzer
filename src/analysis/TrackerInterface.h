/**@file TrackerInterface.h 
 */
#ifndef TRACKER_INTERFACE_H_
#define TRACKER_INTERFACE_H_
//=============================================================================
// INCLUDES
//=============================================================================
#include <stdint.h>
#include <string>
#include <deque>

//=============================================================================
// DEFINITIONS
//=============================================================================

class TrackerInterface {
public:
    virtual size_t get_opened (void) = 0;
    virtual size_t get_closed (void) = 0;
};

//=============================================================================
#endif //TRACKER_INTERFACE_H_
