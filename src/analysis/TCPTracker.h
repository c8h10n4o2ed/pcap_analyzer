/**@file TCPTracker.h 
 */
#ifndef TCP_TRACKER_H_
#define TCP_TRACKER_H_
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
 * Describes all possible TCP states. 
 */
typedef enum {
    TCP_CLOSED      = 0,
    TCP_LISTEN      = 1,
    TCP_SYN_RECV    = 2,
    TCP_SYN_SEND    = 3,
    TCP_ESTABLISHED = 4,
    TCP_FIN_WAIT_1  = 5,
    TCP_FIN_WAIT_2  = 6,
    TCP_CLOSE_WAIT  = 7,
    TCP_CLOSING     = 8,
    TCP_LAST_ACK    = 9,
    TCP_TIME_WAIT   = 10
} TCP_State_T;

class TCPAddressTuple {
public:
    uint32_t src;
    uint32_t dst;
    uint16_t dport;
    uint16_t sport;
    uint64_t timestamp_s;
    uint64_t timestamp_us;

    TCP_State_T state;
};

template<class InputIt, class T>
constexpr InputIt find_tcp(InputIt first, InputIt last, const T& value)
{
    for (; first != last; ++first) {
        if ((((*first).src == value.dst &&
            (*first).dst == value.src) ||
            ((*first).src == value.src ||
            (*first).dst == value.dst)) &&
            ((*first).dport == value.dport ||
             (*first).sport == value.sport ||
             (*first).dport == value.sport ||
             (*first).sport == value.dport)) {
            return first;
        }
    }
    return last;
}

class TCPAddressCompare {
public:
    template <class Type1, class Type2>
    inline bool operator() (const Type1& a, const Type2 &b) {
        bool lt = std::less<uint32_t>()(a.src, b.src) &
                  std::less<uint32_t>()(a.dst, b.dst) &
                  std::less<uint16_t>()(a.dport, b.dport) &
                  std::less<uint16_t>()(a.sport, b.sport);
        return lt;
    }
};

/**
 * This object tracks TCP connections.
 */
class TCPTracker :
    public TrackerInterface 
{
public:
    TCPTracker (uint64_t timeout_us);
    virtual ~TCPTracker (void);

    /**
     * This routine handles packets.
     *  
     * @param packet Reference to packet just received.
     */
    virtual void on_packet (const Packet& packet);

    /**
     * Prunes all closed connections. 
     */
    virtual void prune_connections (void);

    /**
     * Updates state of connections given a particular packet.
     *  
     * @param p Packet object.
     */
    virtual void on_state_update (const Packet& p);

    virtual size_t get_opened (void);
    virtual size_t get_closed (void);
public:
    static std::shared_ptr<TCPTracker> GetStaticInstance (uint64_t timeout_us);

protected:
    std::deque<TCPAddressTuple> m_addrList;
    uint64_t m_timeout_us;
    size_t m_opened;
    size_t m_closed;
};

//=============================================================================
#endif //TCP_TRACKER_H_

