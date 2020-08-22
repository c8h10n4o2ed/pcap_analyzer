/**@file LoggingConfig.h 
 * 
 * This file defines some baseline logging levels for each component of the compiler. 
 * Different verbosity settings will affect/override the settings in this file. 
 *  
 * The purpose of this file is to incrementally disable or limit printing of log messages 
 * to the console.
 */
#ifndef LOGGING_CONFIG_H_
#define LOGGING_CONFIG_H_
//=============================================================================
// DEFINITIONS
//=============================================================================
#define SUBSYSTEM_COMMON                0x00000000
#define SUBSYSTEM_CLIENT                0x00000001
#define SUBSYSTEM_TEST                  0x00000002
#define SUBSYSTEM_ZMQ                   0x00000003
#define SUBSYSTEM_CONN_TRACK            0x00000004
#define SUBSYSTEM_TCP                   0x00000005
#define SUBSYSTEM_UDP                   0x00000006
#define SUBSYSTEM_ICMP                  0x00000007

#define SUBSYSTEM_LOG_LEVELS \
    {\
        {SUBSYSTEM_COMMON,                  LEVEL_MAX}, \
        {SUBSYSTEM_CLIENT,                  LEVEL_MAX}, \
        {SUBSYSTEM_TEST,                    LEVEL_MAX}, \
        {SUBSYSTEM_ZMQ,                     LEVEL_INFO}, \
        {SUBSYSTEM_CONN_TRACK,              LEVEL_MAX}, \
        {SUBSYSTEM_ICMP,                    LEVEL_MAX}, \
        {SUBSYSTEM_UDP,                     LEVEL_MAX}, \
        {SUBSYSTEM_TCP,                     LEVEL_MAX}  \
    }

//=============================================================================
#endif //LOGGING_CONFIG_H_
