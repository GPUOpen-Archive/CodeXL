//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Generic Logging mechanism for PerfStudio
//==============================================================================

#ifndef GPS_LOGGER_H
#define GPS_LOGGER_H

#ifndef LOG_MODULE
    /// Definition
    #define LOG_MODULE ""
#endif

#include <assert.h>

// Uncomment this next line to get __FILE__, __LINE__ and __FUNCTION__ information in log file
// # define PS_LOG_DEBUG

/// Log messages can be errors, warnings or messages
enum LogType
{
    NA_LOG_TYPE = 0,
    logRAW,            // Only log RAW message without any additional data
    logASSERT,
    logERROR,
    logWARNING,
    logMESSAGE,
    logDEBUG,
    logTRACE
};

/// Log trace type enumerations
enum LogTraceType
{
    NA_LOGTRACE_TYPE = 0,
    traceENTER,
    traceEXIT,
    traceMESSAGE
};

// These macros are all of the form:
//
// #define Foo  if (0) ; else _Foo
//
// where _Foo is a hidden function that implements the logging functionality
// this trick is used to allow the use of varargs to the Log functions while still supporting
// overloading for __FILE__, __LINE__ and __FUNCTION__
// _SetupLog() always returns 0

#define LogConsole if(_SetupLog(true, LOG_MODULE, __FILE__, __LINE__, __FUNCTION__)) ; else _Log ///< Logging helper macro
#define LogTrace if(_SetupLog(false, LOG_MODULE, __FILE__, __LINE__, __FUNCTION__)) ; else _LogTrace ///< Logging helper macro
#define Log if(_SetupLog(false, LOG_MODULE, __FILE__, __LINE__, __FUNCTION__)) ; else _Log ///< Logging helper macro
#define LogHeader if(_SetupLog(false, LOG_MODULE, __FILE__, __LINE__, __FUNCTION__)) ; else _LogHeader ///< Logging helper macro
#define LogFooter if(_SetupLog(false, LOG_MODULE, __FILE__, __LINE__, __FUNCTION__)) ; else _LogFooter ///< Logging helper macro

// PerfStudio Assert capability that also logs asserts to the regular logging mechanism
// For now, it continues to use the regular assert mechanism - this can easily be turned off at
// a later date if it proves problematic (for example with fullscreen applications)
#define PsAssertAlways(_Expression) if (!(_Expression)) { Log(logASSERT, #_Expression"\n"); assert(_Expression); } ///< Assert macro
#define PsDoBreak(_Expression) { Log(logASSERT, _Expression"\n"); assert(0); } ///< Break macro
#define PsDoVerify(_Expression) if (!(_Expression)) { Log(logASSERT, #_Expression"\n"); assert(0); } ///< Verify macro

#ifdef _DEBUG
    #define PsAssert PsAssertAlways ///< Asserts boil away to nothing in release builds
    #define PsBreak PsDoBreak ///< Break boil away to nothing in release builds
    #define PsVerify PsDoVerify ///< Verify
#else
    #define PsAssert(_Expression)  ///< Asserts boil away to nothing in release builds
    #define PsBreak(_Expression)   ///< Break boil away to nothing in release builds
    #define PsVerify(_Expression) (_Expression) ///< Verify
#endif

// Function prototypes for functions defined in Logger.cpp

bool _SetupLog(const bool console, const char* module, const char* file, int line, const char* function);

void _Log(enum LogType type, const char* fmt, ...);
void _LogTrace(enum LogTraceType traceType, const char* fmt, ...);

void _LogHeader(void);
void _LogFooter(void);
const char* GetLogFilename(void);
void LogFileInitialize(void);

#endif // GPS_LOGGER_H
