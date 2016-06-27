//==============================================================================
// Copyright (c) 2007-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Generic Logging mechanism for PerfStudio
//==============================================================================

#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osSystemError.h>

#ifdef _LINUX
    #include <unistd.h>
    #include "WinDefs.h"
    #include "OSWrappers.h"
#endif

#include "Logger.h"
#include "misc.h"
#include "SharedGlobal.h"
#include "NamedMutex.h"

static const char* s_mutexName = "PerfStudioLogfileMutex"; ///< Logger mutex

/// Mutex functionality. Uses a platform-independent NamedMutex object, which must be dynamically allocated
/// since the logging functions could be called when the shared library is loaded (reason yet unknown) and
/// when this happens, the loader may not have initialized static data.
class LogMutex : public TSingleton< LogMutex >
{
    friend class TSingleton< LogMutex >;

public:

    /// Initiate a lock
    /// \return true if success false if fail
    bool Lock()
    {
        if (m_mutex->OpenOrCreate(s_mutexName) == false)
        {
            LogConsole(logERROR, "Could not create Mutex (%d).\n", osGetLastSystemError());
            return (false);
        }

        if (m_mutex->Lock() == false)
        {
            LogConsole(logERROR, "Could not Lock Mutex (%d).\n", osGetLastSystemError());
            return (false);
        }

        return true;
    }

    /// Initiate an unlock
    void Unlock()
    {
        m_mutex->Unlock();
    }

protected:

    /// Constructor
    LogMutex()
    {
        m_mutex = new NamedMutex();
    }

    /// Destructor
    ~LogMutex()
    {
        delete m_mutex;
    }

private:

    NamedMutex*  m_mutex; ///< mutex object
};

/// Wait for Mutex
bool LogMutexLock(void)
{
    return LogMutex::Instance()->Lock();
}

/// Release global shared memory region so it can be accessed by other processes.
void LogMutexUnlock(void)
{
    LogMutex::Instance()->Unlock();
}

#ifdef WIN32
    #include <windows.h>

    // In debug builds, add __FILE__, __LINE__ and __FUNCTION__ information to
    // log messages
    // for all builds - add log module name
    // Note: windows specific __declspec() modifier used here to ensure that all statics are thread-safe
    __declspec(thread) static const char* s_LogModule;
    __declspec(thread) static bool s_LogConsole;
    __declspec(thread) static const char* s_LogFile;  /*lint -esym(551,s_LogFile) suppress error about variable not accessed */
    __declspec(thread) static int s_LogLine;          /*lint -esym(551,s_LogLine) suppress error about variable not accessed */
    __declspec(thread) static const char* s_LogFunction;
    __declspec(thread) static bool s_ConsoleAttached = false;
    __declspec(thread) static int logIndent = 0;

#elif defined (_LINUX)
    // Get the name of the application that this shared library is linked to. This name is printed to the log
    extern char* program_invocation_name;

    static __thread const char* s_LogModule;
    static __thread bool s_LogConsole;
    static __thread const char* s_LogFile;  /*lint -esym(551,s_LogFile) suppress error about variable not accessed */
    static __thread int s_LogLine;          /*lint -esym(551,s_LogLine) suppress error about variable not accessed */
    static __thread const char* s_LogFunction;
    static __thread bool s_ConsoleAttached = false;
    static __thread int logIndent = 0;

#endif

/// Returns reference to file that is currently being used.
/// \return The log's filename
const char* GetLogFilename()
{
    if (SG_GET_BOOL(OptionNoLogfile))
    {
        return (NULL);
    }
    else
    {
        return (SG_GET_PATH(LogfilePath));
    }
}

/// Initialize use of Log File. This function is only called once per invocation of the PerfStudio server and forces the overwriting of any previous logfile
void LogFileInitialize(void)
{
    FILE* f;
    const char* logFilename = GetLogFilename();

    // It is valid for there to be no Log File. However since this
    // is the only place where the log file is initialized, we flag
    // it as something to note in the console.

    if (logFilename != NULL)
    {
        // Open for writing - this will overwrite any previous logfile
        fopen_s(&f, logFilename, "w+");    // read binary

        if (f != NULL)
        {
            fprintf(f, "Logging Started: %s\n\n", GetTimeStr().asCharArray());
            fclose(f);
        }
        else
        {
            LogConsole(logERROR, "Unable to open logfile %s for writing \n", logFilename);
        }
    }
}

/// Setup up the log for first time use
/// \param console Log to console flag
/// \param module The name of the calling module
/// \param file The name of the calling file
/// \param line The line number
/// \param function THe function name
/// \return
bool _SetupLog(const bool console, const char* module, const char* file, int line, const char* function)
{
    s_LogConsole = console;
    s_LogModule = module;
    s_LogFile = file;
    s_LogLine = line;
    s_LogFunction = function;

    if (s_ConsoleAttached == false)
    {
        // ensure the log system is attached to a console
        // AttachConsole requires Win 2K or later.
#if defined (WIN32)
        AttachConsole(ATTACH_PARENT_PROCESS);
#elif defined (_LINUX)
#pragma message("TODO: IMPLEMENT ME!")
#endif
        s_ConsoleAttached = true;
    }

    return (0);
}

/// Write log messages into the logfile.
/// \param pMessage Message to log
static void _logWrite(const char* pMessage)
{
    FILE* f;
    const char* logFilename = GetLogFilename();

    if (logFilename != NULL)
    {
        if (LogMutexLock())   // wait for exclusive access to logfile
        {
            fopen_s(&f, logFilename, "a+");    // append

            if (f != NULL)
            {
                fprintf(f, "%s", pMessage);
                fclose(f);
            }
            else
            {
#if defined (_WIN32)
                __declspec(thread) static bool recursiveWrite = false;  // Avoid getting into an infinite loop if file can't be opened.
#elif defined (_LINUX)
                static __thread bool recursiveWrite = false;  // Avoid getting into an infinite loop if file can't be opened.
#endif

                if (!recursiveWrite)
                {
                    recursiveWrite = true;
                    LogConsole(logERROR, "Unable to open logfile %s for append. Message Dropped = \n\t%s\n", logFilename, pMessage);
                    recursiveWrite = false;
                }
            }

            LogMutexUnlock();
        }
    }
}

#define PS_LOG_MAX_LENGTH 1024 ///< Max log message length
#define PS_LOG_INDENT_SIZE 4 ///< Log indent size

/// Supports the trace feature
/// \param traceType The trace type
/// \param fmt The string and format data
void _LogTrace(enum LogTraceType traceType, const char* fmt, ...)
{
    // check to see if logging of trace messages is enabled,
    // if not, don't process the call.
    if (((logTRACE - logERROR) > SG_GET_INT(OptionLogLevel)) && (s_LogConsole == false))
    {
        return;
    }

    int nSize;
    int nLen = 0;
    bool truncated = false;
    char traceString[PS_LOG_MAX_LENGTH] = "";

    switch (traceType)
    {
        case traceENTER:

            if (truncated == false)
            {
                nSize = _snprintf_s(&traceString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "Enter: %s() ", s_LogFunction);

                if ((truncated = (nSize == -1)) == false)
                {
                    nLen += nSize;
                }
            }

            break;

        case traceEXIT:
            logIndent -= PS_LOG_INDENT_SIZE;

            if (logIndent < 0)
            {
                logIndent = 0;
            }

            truncated = (nLen == PS_LOG_MAX_LENGTH);

            if (truncated == false)
            {
                nSize = _snprintf_s(&traceString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "Exit : %s() ", s_LogFunction);

                if ((truncated = (nSize == -1)) == false)
                {
                    nLen += nSize;
                }
            }

            break;

        case traceMESSAGE:
            // do nothing
            break;

        default:
            break;
    }

    // Add the actual Log Message
    if (truncated == false)
    {
        va_list arg_ptr;
        va_start(arg_ptr, fmt);

        nSize = vsnprintf_s(&traceString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, fmt, arg_ptr);

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }

        va_end(arg_ptr);
        /*lint -esym(438,arg_ptr) suppress lint warning for variable not used after assignment */
    }

    // For trace messages - force a "\n" at the end
    if (truncated == false)
    {
        nSize = _snprintf_s(&traceString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "\n");

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

    /*lint -esym(438,nLen) suppress lint warning for variable not used after assignment */
    /*lint -esym(438,truncated) suppress lint warning for variable not used after assignment */

    _Log(logTRACE, traceString);

    // log the ENTER message at the previous depth
    // and indent the upcoming messages
    if (traceType == traceENTER)
    {
        logIndent += PS_LOG_INDENT_SIZE;
    }
}

/// Generate a composite log message - then finally send to handler to write and/or display
/// \param type The trace type
/// \param fmt The string and format data
void _Log(enum LogType type, const char* fmt, ...)
{
    // check to see if logging of the level is enabled, or if s_LogConsole is specified
    // if not, don't process it.
    int logLevel = SG_GET_INT(OptionLogLevel);

    if (((type - logERROR) > logLevel && s_LogConsole == false))
    {
        return;
    }

    // Define buffer to store maximum current log message.
    // Different destinations will receive a subset of this string.
    // On Win32 the entire string is passed to OutputDebugString()

    char fullString[PS_LOG_MAX_LENGTH] = "";
    int nLen = 0;
    int nSize;
    bool truncated = false;

    char* pLogString; // String to print to logfile
    char* pConsole;   // String to print to console doesn't include debug information
    char* pRaw;       // Raw string passed in without any additional decoration

    if (truncated == false)
    {
        // Prepend "PerfStudio: " for windows OutputDebugString() messages
        nSize = _snprintf_s(&fullString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "PerfStudio: ");

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

    pLogString = &fullString[nLen]; // logfile message doesn't include above WIN32 section

#if (defined PS_LOG_DEBUG) && (defined _DEBUG)

    if (truncated == false)
    {
        // In debug builds, include the __FILE__, __LINE__ and __FUNCTION__ information
        nSize = _snprintf_s(&fullString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "%s(%d) : %s(): ", s_LogFile, s_LogLine, s_LogFunction);

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

#endif

    // Prepend accurate timestamp
    if (truncated == false)
    {
        gtASCIIString time = GetMicroTimeStr();
        time = time.substr(12);
        nSize = _snprintf_s(&fullString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "%-14s: ", time.asCharArray());

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

    pConsole = &fullString[nLen];  // String to print to console starts here

    if (truncated == false)
    {
        // Add the message type string
        switch (type)
        {
            case logERROR:
                nSize = _snprintf_s(&fullString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "Error:   ");
                break;

            case logWARNING:
                nSize = _snprintf_s(&fullString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "Warning: ");
                break;

            case logMESSAGE:
                nSize = _snprintf_s(&fullString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "Message: ");
                break;

            case logTRACE:
                nSize = _snprintf_s(&fullString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "Trace:   ");
                break;

            case logASSERT:
                nSize = _snprintf_s(&fullString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "Assert:  ");
                break;

            case logDEBUG:
                nSize = _snprintf_s(&fullString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "Debug:   ");
                break;

            case logRAW:
                // Skip
                nSize = 0;
                break;

            default:
                nSize = _snprintf_s(&fullString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "Unknown: ");
                break;
        }

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

    // Add the module identifier
    if (s_LogModule  && (truncated == false))
    {
        nSize = _snprintf_s(&fullString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, "PID: %10u TID: %10u %-14s: ", osGetCurrentProcessId(), osGetCurrentThreadId(), s_LogModule);

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }
    }

    if (truncated == false)
    {
        if (logLevel >= logTRACE - logERROR)
        {
            // Add the indent
            for (int i = 0; (i < logIndent) && (nLen < PS_LOG_MAX_LENGTH - 1); i++, nLen++)
            {
                fullString[nLen] = ' ';
            }

            fullString[nLen] = 0;  // Ensure string in buffer remains null terminated
            truncated = (nLen == PS_LOG_MAX_LENGTH - 1);
        }
    }

    pRaw = &fullString[nLen];  // Raw undecorated string starts here

    // Add the actual Log Message
    if (truncated == false)
    {
        va_list arg_ptr;
        va_start(arg_ptr, fmt);

        nSize = vsnprintf_s(&fullString[nLen], PS_LOG_MAX_LENGTH - nLen, _TRUNCATE, fmt, arg_ptr);

        if ((truncated = (nSize == -1)) == false)
        {
            nLen += nSize;
        }

        va_end(arg_ptr);
    }

    // Check if message has been truncated and clean up accordingly

    if (truncated == true)
    {
        // Truncation occurred - change end of line to reflect this
        char truncationString[] = " ... \n";
        sprintf_s(&fullString[PS_LOG_MAX_LENGTH - sizeof(truncationString)], sizeof(truncationString), "%s", truncationString);
    }

    // Message Constructed. Print to Log, Console, and OutputDebugString as necessary
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
            OutputDebugString(fullString);
        }
        else
        {
            // not a console message - filter based on log level
            if ((type - logERROR) <= logLevel)
            {
                if (type == logTRACE)
                {
                    // Trace messages also go to the console
                    printf("%s", pConsole);
                }

                _logWrite(pLogString);
                OutputDebugString(fullString);
            }
        }
    }
}

/// Generate a header for the logs
void _LogHeader(void)
{
    char   szLibPath [ PS_MAX_PATH ];
#if defined (WIN32)
    GetModuleFileNameA(NULL, szLibPath, PS_MAX_PATH);
#elif defined (_LINUX)

    if (program_invocation_name[0] == '/')
    {
        // file contains full path already, so don't append path
        sprintf_s(szLibPath, PS_MAX_PATH, "%s", program_invocation_name);
    }
    else
    {
        char currentDir[PS_MAX_PATH];
        getcwd(currentDir, PS_MAX_PATH);
        sprintf_s(szLibPath, PS_MAX_PATH, "%s/%s", currentDir, program_invocation_name);
    }

#endif
    _Log(logRAW,  "---------------BEGIN------------------\n");
    _Log(logRAW,  "App : %s\n", szLibPath);
    _Log(logRAW,  "PID: %i\n", osGetCurrentProcessId());
    _Log(logRAW,  "Time: %s\n", GetTimeStr().asCharArray());
}

/// Generate a footer for the logs
void _LogFooter(void)
{
    char   szLibPath [ PS_MAX_PATH ];
#if defined (WIN32)
    GetModuleFileNameA(NULL, szLibPath, PS_MAX_PATH);
#elif defined (_LINUX)

    if (program_invocation_name[0] == '/')
    {
        // file contains full path already, so don't append path
        sprintf_s(szLibPath, PS_MAX_PATH, "%s", program_invocation_name);
    }
    else
    {
        char currentDir[PS_MAX_PATH];
        getcwd(currentDir, PS_MAX_PATH);
        sprintf_s(szLibPath, PS_MAX_PATH, "%s/%s", currentDir, program_invocation_name);
    }

#endif
    _Log(logRAW,  "App : %s\n", szLibPath);
    _Log(logRAW,  "PID: %i\n", osGetCurrentProcessId());
    _Log(logRAW,  "Time: %s\n", GetTimeStr().asCharArray());
    _Log(logRAW, "--------------THE END------------------\n");
}
