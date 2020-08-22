/** @file Logging.h
 */
#ifndef LOGGING_H_
#define LOGGING_H_
//=============================================================================
// INCLUDES 
//============================================================================= 
#include <stdint.h>
#include <string>
#include "LoggingConfig.h"

//============================================================================= 
// TYPE DEFINITIONS
//============================================================================= 
/** 
 * List of all possible debug levels.
 */ 
typedef enum {
    LEVEL_NONE,
    LEVEL_ERROR,
    LEVEL_WARNING,    
    LEVEL_INFO,
    LEVEL_DEBUG,
    LEVEL_VERBOSE
} LogLevel_T;

#define LEVEL_MAX LEVEL_VERBOSE

/**
 * Used for a list of all subsystem max logging levels.
 */
typedef struct {
    uint32_t subsystemId;
    LogLevel_T level;
} SubsystemLogLevel_T;

/**
 * Used to assign a name to each log level.
 */
typedef struct {
    LogLevel_T level;
    const char* name;
    const char* color_escape_sequence;
    const char* color_reset_sequence;
} LogLevelDescriptor_T;

//============================================================================= 
// FUNCTION DEFINITIONS
//============================================================================= 

/**
 * Initializes logging based on the built-in configuration.
 * 
 * @return uint32_t
 */
uint32_t InitializeLogging (void);

/**
 * Determines if a log level is enabled for a specific source.
 *  
 * @param logLevel The log level 
 * @param srcid The source ID
 * @return bool 
 */
bool IsLogEnabled (LogLevel_T logLevel, uint64_t srcId);

/**
 * Returns the name of the specified log level. 
 *  
 * @param logLevel 
 * @return char* 
 */
const char* GetLogLevelName (LogLevel_T logLevel);

/**
 * Returns spacers used for prefixing entries. 
 *  
 * @note If the normal string used for spacers is not large enough, it is 
 *       reallocated and filled with enough spaces to match nDepth + nDepth/2.
 *  
 * @param nDepth 
 * @return const char* 
 */
const char* GetDepthSpaces (unsigned int nDepth);

/**
 * Returns the log level index of a specified log level. 
 *  
 * @param logLevel 
 * @return int 
 */
int GetLogLevelIndex (LogLevel_T logLevel);

/**
 * Prints log message with a specific source component ID. 
 *  
 * @param level 
 * @param srcId 
 * @param msg 
 */
void FileLine_PrintLogMessage (LogLevel_T level, uint64_t srcId, const char* file, const unsigned int line, const char* msg, ...);

/**
 * Prints log message using a source ID of zero. 
 *  
 * @param level 
 * @param msg 
 */
void FileLine_PrintSimpleLogMessage (LogLevel_T level, const char* file, const unsigned int line, const char* msg, ...);

/**
 * Prints a buffer to the log 
 *  
 * @param level 
 * @param srcId 
 * @param file 
 * @param line 
 * @param buffer 
 * @param length 
 */
void _PrintLogDump (LogLevel_T level, uint64_t srcId, const char* file, const unsigned int line, const char* name, void* buffer, unsigned int length);

/**
 * Helper function to allow for Python via SWIG to call logging 
 * functions. 
 * 
 * @param level 
 * @param msg 
 */
void PrintPythonSimpleLogMsg (LogLevel_T level, std::string msg);

void PrintPythonSimpleLogMsg2 (uint64_t level, const char* msg);

/**
 * Helper function to allow for Python via SWIG to call logging 
 * functions. 
 * 
 * @param level 
 * @param srcId 
 * @param msg 
 */
void PrintPythonLogMsg (LogLevel_T level, uint64_t srcId, std::string msg);

#ifdef MSVC
#define PrintLogMessage(level, srcid, msg, ...) FileLine_PrintLogMessage(level, srcid, __FILE__, __LINE__, msg, __VA_ARGS__)
#define PrintSimpleLogMessage(level, msg, ...) FileLine_PrintSimpleLogMessage(level, __FILE__, __LINE__, msg, __VA_ARGS__)
#else
#define PrintLogMessage(level, srcid, msg...) FileLine_PrintLogMessage(level, srcid, __FILE__, __LINE__, msg)
#define PrintSimpleLogMessage(level, msg...) FileLine_PrintSimpleLogMessage(level, __FILE__, __LINE__, msg)
#endif
#define PrintLogDump(level, srcid, name, buffer, length) _PrintLogDump(level, srcid, __FILE__, __LINE__, name, buffer, length)

#define _msg_separator "----------------------------------------------------"
#define PrintSimpleSeparator(level) FileLine_PrintSimpleLogMessage(level, __FILE__, __LINE__, _msg_separator)
#define PrintLogSeparator(level, srcid) FileLine_PrintLogMessage(level, srcid, __FILE__, __LINE__, _msg_separator)

//=============================================================================
#endif //LOGGING_H_
