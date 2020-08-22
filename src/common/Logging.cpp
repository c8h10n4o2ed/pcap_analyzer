/** @file Logging.cpp
 */
//=============================================================================
// INCLUDES 
//============================================================================= 
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "Logging.h"

#ifndef MSVC
# define ENABLE_COLOR_CONSOLE_OUTPUT
# include <sys/timeb.h>
#endif
#define LOG_TEMP_BUFFER_SIZE 4096

pthread_mutex_t g_logMutex;
static char* gs_logTemp = NULL;
static char* gs_depthSpaces = NULL;
static uint64_t gs_depthSpacesLength = 0;

uint32_t g_isLoggingInitialized = 0;
uint64_t g_droppedLogMessages = 0;

static SubsystemLogLevel_T g_sSubsystemLevels[] = SUBSYSTEM_LOG_LEVELS;

static LogLevelDescriptor_T g_sLevels[] = {
    { LEVEL_NONE,    "NONE",    "\e[37m", "\e[0m"}, //Grey
    { LEVEL_ERROR,   "ERROR",   "\e[31m", "\e[0m"}, //Red
    { LEVEL_WARNING, "WARNING", "\e[33m", "\e[0m"}, //Yellow
    { LEVEL_INFO,    "INFO",    "\e[37m", "\e[0m"}, //Grey
    { LEVEL_DEBUG,   "DEBUG",   "\e[37m", "\e[0m"}, //Grey
    { LEVEL_VERBOSE, "VERBOSE", "\e[37m", "\e[0m"}, //Grey
    { LEVEL_NONE, 0 }
};

//============================================================================= 
// DEFINITIONS
//============================================================================= 
void _PrintLogMessage (LogLevel_T level, uint64_t srcId, const char* file, const unsigned int line, const char* msg, va_list ap);

//============================================================================= 
// IMPLEMENTATION
//============================================================================= 
uint32_t InitializeLogging (void)
{
    pthread_mutex_init(&g_logMutex, NULL);
    gs_logTemp = (char*)malloc(LOG_TEMP_BUFFER_SIZE);
    g_isLoggingInitialized = 1;
    return 0;
}

const char* GetBasename (const char* pStr) {
    size_t len = strlen(pStr);

    //Starting at the end of the string
    //Look for first non-period, non-A through Z character 
    //Return character immediately following this char 
    if ( len > 0 ) {
        for (int j = len - 1; j > 0; j--) {
            if ( pStr[j] != '_' &&
                 pStr[j] != '.' &&
                 !(pStr[j] >= 'a' && pStr[j] <= 'z') && 
                 !(pStr[j] >= 'A' && pStr[j] <= 'Z') &&
                 !(pStr[j] >= '0' && pStr[j] <= '9') ) {
                return (pStr + j + 1);
            }
        }
    }

    return pStr;
}

void _PrintLogDump (LogLevel_T level, uint64_t srcId, const char* file, const unsigned int line, const char* name, void* buffer, unsigned int length) {
    uint8_t* p = (uint8_t*)buffer;
    char tempBuffer[256];
    char tempBuffer2[8];

    bool allowMsg = true;
    SubsystemLogLevel_T* pSubPtr = (SubsystemLogLevel_T*)&g_sSubsystemLevels;
    size_t subSize = sizeof(g_sSubsystemLevels) / sizeof(SubsystemLogLevel_T);

    //Look through the subsystems and filter based on level
    for (size_t i=0; i<subSize; i++) {
        if (pSubPtr->subsystemId == srcId) {
            // Only allow the message if the subsystem is set high enough
            // in order to capture those messages.
            if (level > pSubPtr->level) {
                allowMsg = false;                
            }
            break;
        }
        pSubPtr++;
    }

    if (!allowMsg) {
        return;
    }

    tempBuffer[0] = '\0';

    #define HEX_LINE_LENGTH 32
    FileLine_PrintLogMessage(LEVEL_WARNING, srcId, file, line, name);

    if ( !buffer ) {
        FileLine_PrintLogMessage(LEVEL_DEBUG, srcId, file, line, "<null buffer>");
    } else {
        //Write first line of data into buffer then print buffer
        for (unsigned int i = 0; i < length; i+=HEX_LINE_LENGTH) {
            //Print one line at a time
            for (unsigned int j = 0; j < HEX_LINE_LENGTH; j++) {
                if ( (i+j) >= length ) {
                    break;
                }
                sprintf(tempBuffer2, "%02x ", p[i+j]);
                strcat(tempBuffer, tempBuffer2);
            }
            FileLine_PrintLogMessage(level, srcId, file, line, tempBuffer);
            tempBuffer[0] = '\0';
        }
    }
}

void PrintPythonSimpleLogMsg (LogLevel_T level, std::string msg) {
    PrintSimpleLogMessage(level, msg.c_str());
}

void PrintPythonSimpleLogMsg2 (uint64_t level, const char* msg) {
    PrintSimpleLogMessage((LogLevel_T)level, msg);
}

void PrintPythonLogMsg (LogLevel_T level, uint64_t srcId, std::string msg) {
    PrintLogMessage(level, srcId, msg.c_str());
}

void _PrintLogMessage (LogLevel_T level, uint64_t srcId, const char* file, const unsigned int line, const char* msg, va_list ap)
{
#ifndef MSVC
    char timeStrBuf[128];
#endif
    char timeBuf[128];
    time_t temp;
    bool allowMsg = true;
    SubsystemLogLevel_T* pSubPtr = (SubsystemLogLevel_T*)&g_sSubsystemLevels;
    size_t subSize = sizeof(g_sSubsystemLevels) / sizeof(SubsystemLogLevel_T);

    //Look through the subsystems and filter based on level
    for (size_t i=0; i<subSize; i++) {
        if (pSubPtr->subsystemId == srcId) {
            // Only allow the message if the subsystem is set high enough
            // in order to capture those messages.
            if (level > pSubPtr->level) {
                allowMsg = false;                
            }
            break;
        }
        pSubPtr++;
    }

    if (!allowMsg) {
        return;
    }

    if ( !g_isLoggingInitialized ) {
        g_droppedLogMessages += 1;
        return;
    }

    if ( g_droppedLogMessages > 0 ) {
        PrintLogMessage(LEVEL_INFO, 0, "Dropped %d log messages", g_droppedLogMessages);
        g_droppedLogMessages = 0;
    }

    pthread_mutex_lock(&g_logMutex);

    vsprintf(gs_logTemp, msg, ap);
#ifndef MSVC
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    temp = ((uint64_t)ts.tv_sec * 1000000 + (uint64_t)ts.tv_nsec / 1000) / 1000000;    
    snprintf(
        timeStrBuf,
        sizeof(timeStrBuf),
        "%%G%%m%%d-%%H%%M%%S.%-06d",
        ts.tv_nsec / 1000
    );
    strftime(timeBuf, sizeof(timeBuf), timeStrBuf, localtime(&temp));
#else
    time(&temp);
    strftime(timeBuf, sizeof(timeBuf), "%G%m%d-%H%M%S", localtime(&temp));
#endif
    

#ifdef ENABLE_COLOR_CONSOLE_OUTPUT
    int logLevelIndex = GetLogLevelIndex(level);
    printf("%s[%s:src=%08llx:tid=%08x:%-28s:%-5d:%-7s]: %s%s\n",
           g_sLevels[logLevelIndex].color_escape_sequence,
           timeBuf,
           srcId,
#ifdef MSVC
           (uint64_t)pthread_self().x,
#else
		   (uint64_t)pthread_self(),
#endif
           GetBasename(file),
           line,
           GetLogLevelName(level),           
           gs_logTemp,
           g_sLevels[logLevelIndex].color_reset_sequence
    );
#else
    printf("[%s:src=%08llx:tid=%08x:%-28s:%-5d:%-7s]: %s\n",
           timeBuf,
           srcId,
#ifdef MSVC
		   (uint64_t)pthread_self().x,
#else
		   (uint64_t)pthread_self(),
#endif
           GetBasename(file),
           line,
           GetLogLevelName(level),           
           gs_logTemp
    );
#endif

    fflush(stdout);

    pthread_mutex_unlock(&g_logMutex);
}

void FileLine_PrintLogMessage (LogLevel_T level, uint64_t srcId, const char* file, const unsigned int line, const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    _PrintLogMessage(level, srcId, file, line, msg, ap);
    va_end(ap);
}

void FileLine_PrintSimpleLogMessage (LogLevel_T level, const char* file, const unsigned int line, const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    _PrintLogMessage(level, 0, file, line, msg, ap);
    va_end(ap);
}

const char* GetLogLevelName (LogLevel_T logLevel)
{
    LogLevelDescriptor_T* pLevel = g_sLevels;
    while ( pLevel->name ) {
        if ( pLevel->level == logLevel ) {
            return pLevel->name;
        }
        pLevel++;
    }
    return "UNKNOWN";
}

int GetLogLevelIndex (LogLevel_T logLevel) 
{
    LogLevelDescriptor_T* pLevel = g_sLevels;
    int index = 0;

    while ( pLevel->name ) {
        if ( pLevel->level == logLevel ) {
            return index;
        }
        pLevel++;
        index++;
    }
    return 0;
}

const char* GetDepthSpaces (unsigned int nDepth) {
    uint64_t allocSize = 0;
    uint64_t index = 0;

    //static char* gs_depthSpaces = NULL;
    //static uint64_t gs_depthSpacesLength = 0;

    if ( !gs_depthSpaces ||
         gs_depthSpacesLength < (nDepth * 2) ) {

        //Reallocate
        allocSize = (nDepth + 500) * 2;
        gs_depthSpaces = (char*)malloc(allocSize+1);
        gs_depthSpacesLength = allocSize;

        memset(gs_depthSpaces, 0x20, gs_depthSpacesLength);
        gs_depthSpaces[allocSize] = 0;
    }

    //Return pointer within spaces buffer
    index = gs_depthSpacesLength - nDepth * 2;

    //This buffer location has nDepth*2 space characters followed by a null terminator.
    return (gs_depthSpaces + index);
}

bool IsLogEnabled (LogLevel_T logLevel, uint64_t srcId) {
    bool allowMsg = true;
    SubsystemLogLevel_T* pSubPtr = (SubsystemLogLevel_T*)&g_sSubsystemLevels;
    size_t subSize = sizeof(g_sSubsystemLevels) / sizeof(SubsystemLogLevel_T);

    //Look through the subsystems and filter based on level
    for (size_t i=0; i<subSize; i++) {
        if (pSubPtr->subsystemId == srcId) {
            // Only allow the message if the subsystem is set high enough
            // in order to capture those messages.
            if (logLevel > pSubPtr->level) {
                allowMsg = false;        
            }
            break;
        }
        pSubPtr++;
    }
    return allowMsg;
}

//=============================================================================

