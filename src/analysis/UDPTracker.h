/**@file UDPTracker.h 
 */
#ifndef UDP_TRACKER_H_
#define UDP_TRACKER_H_
//=============================================================================
// INCLUDES
//=============================================================================
#define TINS_IS_CXX11 1
#include <string>
#include <string.h>
#include <vector>
#include <tins/tins.h>
#include <stdint.h>
#include <vector>
#include <deque>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <memory>
#include <utility>
#include <algorithm>

#include "TrackerInterface.h"
#include "Logging.h"
#include "BTree.h"
#include "PacketMsgProxy.h"
#include "MD5ByteContainer.h"

//=============================================================================
// DEFINITIONS
//=============================================================================
using namespace Tins;

/**
 * Describes all possible UDP states. 
 */
typedef enum {
    UDP_CLOSED      = 0,
    UDP_ACTIVE      = 1,
    UDP_TIMEOUT     = 2
} UDP_State_T;

class UDPAddressTuple {
public:
    UDPAddressTuple (void)
     : src(0),
       dst(0),
       dport(0),
       sport(0),
       timestamp_s(0),
       timestamp_us(0),
       last_active_s(0),
       last_active_us(0),
       state(UDP_ACTIVE)
    {
    }

public:
    uint32_t src;
    uint32_t dst;
    uint16_t dport;
    uint16_t sport;
    uint64_t timestamp_s;
    uint64_t timestamp_us;
    uint64_t last_active_s;
    uint64_t last_active_us;
    UDP_State_T state;
};

template<class InputIt, class T>
constexpr InputIt find_udp (
    InputIt first,
    InputIt last,
    const T& value,
    uint64_t last_s,
    uint64_t last_us,
    uint64_t timeout_us
) {
    for (; first != last; ++first) {
        if ((((*first).src == value.dst &&
            (*first).dst == value.src) ||
            ((*first).src == value.src ||
            (*first).dst == value.dst)) &&
            ((*first).dport == value.dport ||
             (*first).sport == value.sport ||
             (*first).dport == value.sport ||
             (*first).sport == value.dport)) {

            #if 0
            if ((((*first).last_active_s * (10^6) + 
                  (*first).last_active_us) + timeout_us) >
                (last_s * (10^6) + last_us)) {
                return first;
            }
            #endif
            return first;
        }
    }
    return last;
}

/**
 * This object tracks UDP connections.
 */
class UDPTracker :
    public TrackerInterface
{
public:
    UDPTracker (uint64_t timeout_us);
    virtual ~UDPTracker (void);

    /**
     * This routine handles packets.
     *  
     * @param packet Reference to packet just received.
     */
    virtual void on_packet (const Packet& packet);

    /**
     * Prunes all closed connections. 
     */
    virtual void prune_connections (
        uint64_t last_s,
        uint64_t last_us
    );

    /**
     * Updates state of connections given a particular packet.
     *  
     * @param p Packet object.
     */
    virtual void on_state_update (const Packet& p);

    virtual size_t get_opened (void);

    virtual size_t get_closed (void);

public:
    static std::shared_ptr<UDPTracker> GetStaticInstance (uint64_t timeout_us);

protected:
    std::deque<UDPAddressTuple> m_addrList;
    uint64_t m_timeout_us;
    size_t m_opened;
    size_t m_closed;
};

//=============================================================================
#endif //UDP_TRACKER_H_

