//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Generic Logging mechanism for CodeXL GPU Profiler
//==============================================================================

#ifndef _LOGGER_H_
#define _LOGGER_H_

#ifndef LOG_MODULE
    #define LOG_MODULE ""
#endif

#include <assert.h>

namespace GPULogger
{

/// \defgroup Logger Logger
/// This module handles output logging to a file.
///
/// \ingroup Common
// @{

// Uncomment this next line to get __FILE__, __LINE__ and __FUNCTION__ information in log file
// # define SP_LOG_DEBUG

// Log messages can be errors, warnings or messages
enum LogType
{
    NA_LOG_TYPE = 0,
    logRAW,            //< Only log RAW message without any additional data
    logASSERT,
    logERROR,
    logWARNING,
    logMESSAGE,
    logTRACE,
    traceENTER,
    traceEXIT,
    traceMESSAGE
};

// APP Profiler Assert capability that also logs asserts to the regular logging mechanism
// For now, it continues to use the regular assert mechanism - this can easily be turned off at
// a later date if it proves problematic (for example with fullscreen applications)
#define SpAssertAlways(_Expression) if (!(_Expression)) { Log(GPULogger::logASSERT, #_Expression"\n"); assert(_Expression); }
#define SpDoBreak(_Expression) { Log(GPULogger::logASSERT, _Expression"\n"); assert(0); }
#define SpDoVerify(_Expression) if (!(_Expression)) { Log(GPULogger::logASSERT, #_Expression"\n"); assert(0); }

#ifdef _DEBUG
    #define SpAssert SpAssertAlways
    #define SpBreak SpDoBreak
    #define SpVerify SpDoVerify
#else
    #define SpAssert(_Expression)  // Asserts boil away to nothing in release builds
    #define SpBreak(_Expression)   // Break boil away to nothing in release builds
    #define SpVerify(_Expression) (_Expression)
#endif

#define SpAssertRet(_Expression) SpAssert(_Expression); if(!(_Expression)) return

// Function prototypes for functions defined in Logger.cpp

/// Generate a composite log message - then finally send to handler to write and/or display
/// \param type Log type
/// \param fmt format string
void Log(enum LogType type, const char* fmt, ...);

// *INDENT-OFF*
#ifdef WIN32
/// Generate a composite log message - then finally send to handler to write and/or display
/// \param type Log type
/// \param fmt format string
void LogW(enum LogType type, const wchar_t* fmt, ...);
#endif
// *INDENT-ON*

/// Log header
void LogHeader(void);

/// Log footer
void LogFooter(void);

/// Return log file name
/// \return log file name string
const char* GetLogFilename(void);

/// Initialize log, create a new file, open file handle
/// \param strFileName File name, if not specified, use default path
void LogFileInitialize(const char* strFileName = 0);

// @}
}

#endif // _LOGGER_H_
