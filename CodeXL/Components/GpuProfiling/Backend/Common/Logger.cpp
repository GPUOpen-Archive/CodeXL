//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Generic Logging mechanism for CodeXL GPU Profiler
//==============================================================================

#include <cstdio> // for snprintf
#ifdef _WIN32
    #include <windows.h>
#else
    #include <string.h> // strcpy
    #include <stdarg.h>
#endif
#include "AMDTMutex.h"
#include "Logger.h"
#include "OSUtils.h"
#include "StringUtils.h"
#include "Defs.h"

#define SP_LOG_MAX_LENGTH 1024
#define SP_LOG_INDENT_SIZE 4

static bool OptionNoLogfile = false;
static char LogfilePath[SP_MAX_PATH];

// *INDENT-OFF*
#ifdef WIN32
static bool s_ConsoleAttached = false;
#endif
// *INDENT-ON*

static AMDTMutex sMutex;


namespace GPULogger
{


// Returns reference to file that is currently being used.
//
const char* GetLogFilename()
{
    if (OptionNoLogfile)
    {
        return (NULL);
    }
    else
    {
        return LogfilePath;
    }
}

// Initialize use of Log File
// This function is only called once per invocation of the server and forces the overwriting of
// any previous logfile

void LogFileInitialize(const char* strFileName)
{
#ifndef DISABLE_LOG

    FILE* f = NULL;
    const char* logFilename;

    if (strFileName == NULL)
    {
        SP_strcpy(LogfilePath, SP_MAX_PATH, "c:\\splog.txt");
        logFilename = GetLogFilename();
    }
    else
    {
        logFilename = strFileName;
        SP_strcpy(LogfilePath, SP_MAX_PATH, strFileName);
    }

    // It is valid for there to be no Log File. However since this
    // is the only place where the log file is initialized, we flag
    // it as something to note in the console.

    if (logFilename != NULL)
    {
        // Open for writing - this will overwrite any previous logfile
#ifdef _WIN32
        fopen_s(&f, logFilename, "w+");    // read binary
#else
        f = fopen(logFilename, "w+");
#endif

        if (f != NULL)
        {
            fprintf(f, "Logging Started: %s\n\n", StringUtils::GetTimeString().c_str());
            fclose(f);
        }
        else
        {
            Log(logERROR, "Unable to open logfile %s for writing \n", logFilename);
        }
    }

#ifdef WIN32

    if (s_ConsoleAttached == false)
    {
        // ensure the log system is attached to a console
        // AttachConsole requires Win 2K or later.
        AttachConsole(ATTACH_PARENT_PROCESS);
        s_ConsoleAttached = true;
    }

#endif

#else
    SP_UNREFERENCED_PARAMETER(strFileName);
#endif   //DISABLE_LOG
}

#ifndef DISABLE_LOG

// In debug builds, add __FILE__, __LINE__ and __FUNCTION__ information to
// log messages
// for all builds - add log module name
static const char* s_LogModule;

static int logIndent = 0;

// *INDENT-OFF*
#ifdef _DEBUG
static int OptionLogLevel = traceMESSAGE;
#else
static int OptionLogLevel = logERROR;
#endif
// *INDENT-ON*

//
// Write log messages into the logfile.
//
static void _logWrite(const char* pMessage)
{
    FILE* f = 0;
    const char* logFilename = GetLogFilename();

    if (logFilename != NULL)
    {
#ifdef _WIN32
        fopen_s(&f, logFilename, "a+");    // append
#else
        f = fopen(logFilename, "a+");
#endif

        if (f != NULL)
        {
            fprintf(f, "%s", pMessage);
            fclose(f);
        }
    }
}
#ifdef WIN32
//
// Write log messages into the logfile.
//
static void _logWriteW(const wchar_t* pMessage)
{
    FILE* f = 0;
    const char* logFilename = GetLogFilename();

    if (logFilename != NULL)
    {
        fopen_s(&f, logFilename, "a+");    // append

        if (f != NULL)
        {
            fwprintf(f, L"%s", pMessage);
            fclose(f);
        }
    }
}

#endif //WIN32

#endif //DISABLE_LOG

void Log(enum LogType type, const char* fmt, ...)
{
#ifdef DISABLE_LOG
    SP_UNREFERENCED_PARAMETER(type);
    SP_UNREFERENCED_PARAMETER(fmt);
    return;
#else

    AMDTScopeLock s(&sMutex);

    bool s_LogConsole = false;

    const char* s_LogFunction = __FUNCTION__;

    va_list arg_ptr;

    va_start(arg_ptr, fmt);

    // check to see if logging of the level is enabled, or if s_LogConsole is specified
    // if not, don't process it.
    if (type > OptionLogLevel)
    {
        va_end(arg_ptr);
        return;
    }

    // Define buffer to store maximum current log message.
    // Different destinations will receive a subset of this string.
    // On Win32 the entire string is passed to OutputDebugString()

    char fullString[SP_LOG_MAX_LENGTH] = "\0";
    int nLen = 0;
    int nSize;
    bool truncated = false;

    char* pLogString; // String to print to logfile
    char* pConsole;   // String to print to console doesn't include debug information
    char* pRaw;       // Raw string passed in without any additional decoration

    switch (type)
    {
        case traceENTER:

            if (truncated == false)
            {
                nSize = SP_snprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, "Enter: %s() ", s_LogFunction);

                if ((truncated = (nSize == -1)) == false)
                {
                    nLen += nSize;
                }
            }

            break;

        case traceEXIT:
            logIndent -= SP_LOG_INDENT_SIZE;

            if (logIndent < 0)
            {
                logIndent = 0;
            }

            truncated = (nLen == SP_LOG_MAX_LENGTH);

            if (truncated == false)
            {
                nSize = SP_snprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, "Exit : %s() ", s_LogFunction);

                if ((truncated = (nSize == -1)) == false)
                {
                    nLen += nSize;
                }
            }

            break;

        default:
            break;
    }

#ifdef WIN32

    if (truncated == false)
    {
        // Prepend "CodeXL GPU Profiler: " for windows OutputDebugString() messages
        nSize = SP_snprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, "CodeXL GPU Profiler: ");

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

#endif

    pLogString = &fullString[nLen]; // logfile message doesn't include above WIN32 section

#if (defined SP_LOG_DEBUG) && (defined _DEBUG)
    char* s_LogFile = __FILE__;
    int s_LogLine = __LINE__;

    if (truncated == false)
    {
        // In debug builds, include the __FILE__, __LINE__ and __FUNCTION__ information
        nSize = SP_snprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, "%s(%d) : %s(): ", s_LogFile, s_LogLine, s_LogFunction);

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

#endif

    //// Prepend accurate timestamp
    //if ( truncated == false )
    //{
    //   std::string time = StringUtils::GetTimeString();
    //   nSize = _snprintf_s( &fullString[nLen], SP_LOG_MAX_LENGTH - nLen, _TRUNCATE, "%-14s: ", time.c_str() );
    //   if ( ( truncated = ( nSize == -1 ) ) == false )
    //   {
    //      nLen += nSize;
    //   }
    //}

    pConsole = &fullString[nLen];  // String to print to console starts here

    if (truncated == false)
    {
        // Add the message type string
        switch (type)
        {
            case logERROR:
                nSize = SP_snprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, "Error:   ");
                break;

            case logWARNING:
                nSize = SP_snprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, "Warning: ");
                break;

            case logMESSAGE:
                nSize = SP_snprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, "Message: ");
                break;

            case logTRACE:
                nSize = SP_snprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, "Trace:   ");
                break;

            case logASSERT:
                nSize = SP_snprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, "Assert:  ");
                break;

            case logRAW:
                // Skip
                nSize = 0;
                break;

            case traceENTER:
            case traceEXIT:
            case traceMESSAGE:
                nSize = SP_snprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, "Trace:  ");
                break;

            default:
                nSize = SP_snprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, "Unknown: ");
                break;
        }

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

    // Add the module identifier
    if (s_LogModule && (truncated == false))
    {
        nSize = SP_snprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, "%-14s: ", s_LogModule);

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

    if (truncated == false)
    {
        if (OptionLogLevel >= logTRACE - logERROR)
        {
            // Add the indent
            for (int i = 0; (i < logIndent) && (nLen < SP_LOG_MAX_LENGTH); i++, nLen++)
            {
                fullString[nLen] = ' ';
            }

            truncated = (nLen == SP_LOG_MAX_LENGTH);
        }
    }

    pRaw = &fullString[nLen];  // Raw undecorated string starts here

    // Add the actual Log Message
    if (truncated == false)
    {
        nSize = SP_vsnprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, fmt, arg_ptr);

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

    // Check if message has been truncated and clean up accordingly

    if (truncated == true)
    {
        // Truncation occurred - change end of line to reflect this
        char truncationString[] = " ... \n";
        SP_sprintf(&fullString[SP_LOG_MAX_LENGTH - sizeof(truncationString)], sizeof(truncationString), "%s", truncationString);
    }

    if (type > logTRACE)
    {
        // For trace messages - force a "\n" at the end
        if (truncated == false)
        {
            nSize = SP_snprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, "\n");

            if ((truncated = (nSize == -1)) == false)
            {
                nLen += nSize;
            }
        }
    }

    // Message Constructed. Print to Log, Console, and OutputDebugString as necessary
    //
    if (type == logRAW)
    {
        if (s_LogConsole)
        {
            // Send string to console
            printf("%s", pRaw);
        }

        _logWrite(pRaw);
    }
    else
    {
        if (s_LogConsole)
        {
            // Console messages are always printed in console and in log file
            // regardless of logLEVEL
            printf("%s", pConsole);
            _logWrite(pLogString);
#ifdef WIN32
            SP_TODO("revisit use of OutputDebugStringA for Unicode support")
            OutputDebugStringA(fullString);
#endif
        }
        else
        {
            // not a console message - filter based on log level
            if ((type - logERROR) <= OptionLogLevel)
            {
                if (type == logTRACE)
                {
                    // Trace messages also go to the console
                    printf("%s", pConsole);
                }

                _logWrite(pLogString);
#ifdef WIN32
                SP_TODO("revisit use of OutputDebugStringA for Unicode support")
                OutputDebugStringA(fullString);
#endif
            }
        }
    }

    va_end(arg_ptr);

#endif
}

#ifdef WIN32
#define _CRT_STDIO_ISO_WIDE_SPECIFIERS
void LogW(enum LogType type, const wchar_t* fmt, ...)
{
#ifdef DISABLE_LOG
    SP_UNREFERENCED_PARAMETER(type);
    SP_UNREFERENCED_PARAMETER(fmt);
    return;
#else

    AMDTScopeLock s(&sMutex);

    bool s_LogConsole = false;

    va_list arg_ptr;

    va_start(arg_ptr, fmt);

    // check to see if logging of the level is enabled, or if s_LogConsole is specified
    // if not, don't process it.
    if (type > OptionLogLevel)
    {
        va_end(arg_ptr);
        return;
    }

    // Define buffer to store maximum current log message.
    // Different destinations will receive a subset of this string.
    // On Win32 the entire string is passed to OutputDebugString()

    wchar_t fullString[SP_LOG_MAX_LENGTH] = L"\0";
    int nLen = 0;
    int nSize;
    bool truncated = false;

    wchar_t* pLogString; // String to print to logfile
    wchar_t* pConsole;   // String to print to console doesn't include debug information
    wchar_t* pRaw;       // Raw string passed in without any additional decoration

    if (truncated == false)
    {
        // Prepend "CodeXL GPU Profiler: " for windows OutputDebugString() messages
        nSize = _snwprintf_s(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, _TRUNCATE, L"CodeXL GPU Profiler: ");

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

    pLogString = &fullString[nLen]; // logfile message doesn't include above WIN32 section

    pConsole = &fullString[nLen];  // String to print to console starts here

    if (truncated == false)
    {
        // Add the message type string
        switch (type)
        {
            case logERROR:
                nSize = SP_snwprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, L"Error:   ");
                break;

            case logWARNING:
                nSize = SP_snwprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, L"Warning: ");
                break;

            case logMESSAGE:
                nSize = SP_snwprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, L"Message: ");
                break;

            case logTRACE:
                nSize = SP_snwprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, L"Trace:   ");
                break;

            case logASSERT:
                nSize = SP_snwprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, L"Assert:  ");
                break;

            case logRAW:
                // Skip
                nSize = 0;
                break;

            case traceENTER:
            case traceEXIT:
            case traceMESSAGE:
                nSize = SP_snwprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, L"Trace:  ");
                break;

            default:
                nSize = SP_snwprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, L"Unknown: ");
                break;
        }

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

    // Add the module identifier
    if (s_LogModule && (truncated == false))
    {
        nSize = SP_snwprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, L"%-14s: ", s_LogModule);

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

    if (truncated == false)
    {
        if (OptionLogLevel >= logTRACE - logERROR)
        {
            // Add the indent
            for (int i = 0; (i < logIndent) && (nLen < SP_LOG_MAX_LENGTH); i++, nLen++)
            {
                fullString[nLen] = L' ';
            }

            truncated = (nLen == SP_LOG_MAX_LENGTH);
        }
    }

    pRaw = &fullString[nLen];  // Raw undecorated string starts here

    // Add the actual Log Message
    if (truncated == false)
    {
        nSize = SP_vsnwprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, fmt, arg_ptr);

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

    // Check if message has been truncated and clean up accordingly

    if (truncated == true)
    {
        // Truncation occurred - change end of line to reflect this
        wchar_t truncationString[] = L" ... \n";
        swprintf_s(&fullString[SP_LOG_MAX_LENGTH - sizeof(truncationString)], sizeof(truncationString), L"%ws", truncationString);
    }

    if (type > logTRACE)
    {
        // For trace messages - force a "\n" at the end
        if (truncated == false)
        {
            nSize = SP_snwprintf(&fullString[nLen], SP_LOG_MAX_LENGTH - nLen, L"\n");

            if ((truncated = (nSize == -1)) == false)
            {
                nLen += nSize;
            }
        }
    }

    // Message Constructed. Print to Log, Console, and OutputDebugString as necessary
    //
    if (type == logRAW)
    {
        if (s_LogConsole)
        {
            // Send string to console
            wprintf(L"%ws", pRaw);
        }

        _logWriteW(pRaw);
    }
    else
    {
        if (s_LogConsole)
        {
            // Console messages are always printed in console and in log file
            // regardless of logLEVEL
            wprintf(L"%ws", pConsole);
            _logWriteW(pLogString);
            OutputDebugStringW(fullString);
        }
        else
        {
            // not a console message - filter based on log level
            if ((type - logERROR) <= OptionLogLevel)
            {
                if (type == logTRACE)
                {
                    // Trace messages also go to the console
                    wprintf(L"%ws", pConsole);
                }

                _logWriteW(pLogString);
                OutputDebugStringW(fullString);
            }
        }
    }

    va_end(arg_ptr);
#endif
}

#endif



void LogHeader(void)
{
#ifdef _WIN32
    char   szLibPath[SP_MAX_PATH];
    GetModuleFileNameA(NULL, szLibPath, SP_MAX_PATH);
    Log(logRAW, "---------------BEGIN-------------------\n");
    Log(logRAW, "App : %s\n", szLibPath);
    Log(logRAW, "Time: %s\n", StringUtils::GetTimeString().c_str());
#endif
}

void LogFooter(void)
{
    Log(logRAW, "Time: %s\n", StringUtils::GetTimeString().c_str());
    Log(logRAW, "--------------THE END------------------\n");
}

}
