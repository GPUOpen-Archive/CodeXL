//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDebugLog.cpp
///
//=====================================================================

//------------------------------ osDebugLog.cpp ------------------------------

// Standard C:
#include <string.h>

#ifdef _GR_IPHONE_DEVICE_BUILD
    // Apple Sytem Log:
    #include <asl.h>
#endif

// Infra:
// This must come before any Boost includes, for the macro __STDC_LIMIT_MAX definition:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osUser.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// The debug log file extension:
#define OS_DEBUG_LOG_FILE_EXTENSION L"log"

// The maximal old log file size, measured in bytes (currently 100K).
// (When an old log file is bigger than this size, opening the log
//  file again will erase its old content)
#define OS_MAX_OLD_LOG_FILE_SIZE 102400

// Static members initializations:
osDebugLog* osDebugLog::_pMySingleInstance = NULL;

// The severity strings:
static const wchar_t* stat_severityStrings[] = { L"ERROR", L"INFO", L"DEBUG", L"EXTENSIVE", L"UNKNOWN" };


// ---------------------------------------------------------------------------
// Name:        osDebugLogSeverityToString
// Description: Translates a given log severity to a string.
// Arguments:   severity - The input severity.
// Return Val:  const char* - The output string.
// Author:      AMD Developer Tools Team
// Date:        29/8/2005
// ---------------------------------------------------------------------------
const wchar_t* osDebugLogSeverityToString(osDebugLogSeverity severity)
{
    const wchar_t* retVal = stat_severityStrings[4];

    switch (severity)
    {
        case OS_DEBUG_LOG_ERROR:
            retVal = stat_severityStrings[0];
            break;

        case OS_DEBUG_LOG_INFO:
            retVal = stat_severityStrings[1];
            break;

        case OS_DEBUG_LOG_DEBUG:
            retVal = stat_severityStrings[2];
            break;

        case OS_DEBUG_LOG_EXTENSIVE:
            retVal = stat_severityStrings[3];
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osStringToDebugLogSeverity
// Description: Translates a given string to the log severity.
// Arguments:   const char* - The input string.
// Return Val:  severity - The output severity.
// Author:      AMD Developer Tools Team
// Date:        5/9/2005
// ---------------------------------------------------------------------------
osDebugLogSeverity osStringToDebugLogSeverity(const wchar_t* severityString)
{
    osDebugLogSeverity retVal = OS_DEBUG_LOG_INFO;

    if (wcscmp(severityString, stat_severityStrings[0]) == 0)
    {
        retVal = OS_DEBUG_LOG_ERROR;
    }
    else if (wcscmp(severityString, stat_severityStrings[1]) == 0)
    {
        retVal = OS_DEBUG_LOG_INFO;
    }
    else if (wcscmp(severityString, stat_severityStrings[2]) == 0)
    {
        retVal = OS_DEBUG_LOG_DEBUG;
    }
    else if (wcscmp(severityString, stat_severityStrings[3]) == 0)
    {
        retVal = OS_DEBUG_LOG_EXTENSIVE;
    }

    return retVal;
}

#ifdef _GR_IPHONE_DEVICE_BUILD
// ---------------------------------------------------------------------------
// Name:        osAddPrintoutToiPhoneConsole
// Description: Adds a string to the iPhone console. This function is a wrapper
//              for asl_log.
//              See http://developer.apple.com/Mac/library/documentation/Darwin/Reference/ManPages/man3/asl_log.3.html
//              for more details.
// Author:      AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
bool osAddPrintoutToiPhoneConsole(const gtString& printoutString, osDebugLogSeverity printoutSeverity)
{
    bool retVal = true;

    // Should be one of ASL_LEVEL_EMERG, ASL_LEVEL_ALERT, ASL_LEVEL_CRIT, ASL_LEVEL_ERR,
    // ASL_LEVEL_WARNING, ASL_LEVEL_NOTICE, ASL_LEVEL_INFO, ASL_LEVEL_DEBUG:
    int aslLevel = ASL_LEVEL_NOTICE;

    switch (printoutSeverity)
    {
        case OS_DEBUG_LOG_ERROR:
            aslLevel = ASL_LEVEL_ERR;
            break;

        case OS_DEBUG_LOG_INFO:
            aslLevel = ASL_LEVEL_INFO;
            break;

        case OS_DEBUG_LOG_DEBUG:
            aslLevel = ASL_LEVEL_DEBUG;
            break;

        case OS_DEBUG_LOG_EXTENSIVE:
            aslLevel = ASL_LEVEL_DEBUG;
            break;

        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    // Make sure the printout is filtered in by ASL:
    // First, get the mask (the asl_set_filter function returns the previous mask, so we set it to any value and restore it later:
    int mask = asl_set_filter(NULL, 0);

    bool goOn = true;
    int usedASLLevel = aslLevel;

    while (goOn)
    {
        // See if the current level is supported by the ASL - if the appropriate bit in the mask is on:
        if (((1 << usedASLLevel) & mask) != 0)
        {
            goOn = false;
        }
        else
        {
            // Raise the level, looping around from ASL_LEVEL_EMERG to ASL_LEVEL_DEBUG if the user does not want to
            // see high-level (note that the higher priorities are lower numbers) reports for some reason:
            usedASLLevel--;

            if (usedASLLevel < 0)
            {
                usedASLLevel += 8;
            }

            // If we somehow slipped out of range, break so we don't get stuck:
            if ((usedASLLevel < 0) || (usedASLLevel >= 8))
            {
                usedASLLevel = aslLevel;
            }

            // If we returned to the original level, i.e. the mask has no level defined, just give up)
            if (usedASLLevel == aslLevel)
            {
                break;
            }
        }
    }

    // Restore the mask as it were:
    asl_set_filter(NULL, mask);

    asl_log(NULL, NULL, usedASLLevel, printoutString.asCharArray());

    return retVal;
}
#endif // _GR_IPHONE_DEVICE_BUILD

// ---------------------------------------------------------------------------
// Name:        osDebugLog::instance
// Description: Returns a reference to the single instance of this class.
//              The first call to this function creates this signle instance.
// Author:      AMD Developer Tools Team
// Date:        29/8/2005
// ---------------------------------------------------------------------------
osDebugLog& osDebugLog::instance()
{
    // If my single instance was not created yet - create it:
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new osDebugLog;
    }

    // Return my single instance:
    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        osDebugLog::osDebugLog
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        29/8/2005
// Implementation notes:
//   We cannot use gtAssert here, since sometimes gtAssert calls us.
// ---------------------------------------------------------------------------
osDebugLog::osDebugLog()
    : _loggedSeverity(OS_DEBUG_LOG_INFO), _isInitialized(false)
{
    // Register myself as the assertion failure handler:
    registerSelfAsAssertionHandler();

    // Reset the current session start time:
    EndSession();
}


// ---------------------------------------------------------------------------
// Name:        osDebugLog::~osDebugLog
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        29/8/2005
// ---------------------------------------------------------------------------
osDebugLog::~osDebugLog()
{
    // Un-register this class instance from being a global assertion failure handler:
    gtUnRegisterAssertionFailureHandler(this);

    // If I was initialized:
    if (_isInitialized)
    {
        terminate();
    }

    // Mark that this class single instance was deleted:
    _pMySingleInstance = NULL;
}


// ---------------------------------------------------------------------------
// Name:        osDebugLog::initialize
// Description: Initialize the log file.
// Arguments:   logFileName - The log file name. The log file will reside under
//                            the current user temp directory.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/8/2005
// ---------------------------------------------------------------------------
bool osDebugLog::initialize(const gtString& logFileName, const wchar_t* pszProductDescription /* = nullptr */, const wchar_t* pszOSDescription /* = nullptr */, const osFilePath& logFilePath/* = osFilePath()*/)
{
    bool retVal = false;

    if (NULL != pszProductDescription)
    {
        setProductDescription(pszProductDescription);
    }

    if (NULL != pszOSDescription)
    {
        setOSDescriptionString(pszOSDescription);
    }

    // Calculate the log file path:
    osFilePath logFilePathCalculated = logFilePath;
    calculateLogFilePath(logFileName, logFilePathCalculated);

    // Initialize the log file:
    bool rc1 = initialize(logFilePathCalculated);
    GT_IF_WITH_ASSERT(rc1)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osDebugLog::initialize
// Description: Initialize the log file.
// Arguments:   logFilePath - A full path, including file name and extension,
//                            of the log file to be used.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/5/2008
// ---------------------------------------------------------------------------
bool osDebugLog::initialize(const osFilePath& logFilePath)
{
#ifdef _GR_IPHONE_DEVICE_BUILD
    // On the iPhone, we use ASL instead of writing to a file, so this function simply needs to return true
    _isInitialized = true;
#else // ndef _GR_IPHONE_DEVICE_BUILD

    // If we are already initialized:
    if (_isInitialized)
    {
        // Get the existing debug log file name:
        const osFilePath& existingFilePath = _debugLogFile.path();

        // If we were asked to initialize a different log file name:
        if (existingFilePath != logFilePath)
        {
            // Output a re-initialization message:
            gtString logMessage = OS_STR_DebugLogIsReInitialized;
            logMessage += logFilePath.asString();
            OS_OUTPUT_DEBUG_LOG(logMessage.asCharArray(), OS_DEBUG_LOG_INFO);

            // Terminate the opened log file:
            terminate();
        }
    }

    if (!_isInitialized)
    {
        // Log the used log file path:
        bool rc1 = _debugLogFile.setPath(logFilePath);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Calculate the log file open mode:
            osFile::osOpenMode fileOpenMode = calculateLogFileOpenMode();

            // Open the log file:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            bool rc2 = _debugLogFile.open(osChannel::OS_UNICODE_TEXT_CHANNEL, fileOpenMode);
#else
            bool rc2 = _debugLogFile.open(osChannel::OS_ASCII_TEXT_CHANNEL, fileOpenMode);
#endif
            GT_IF_WITH_ASSERT(rc2)
            {
                // Mark that the file was initialized:
                _isInitialized = true;

                // If the file is recreated:
                if (fileOpenMode == osFile::OS_OPEN_TO_WRITE)
                {
                    // Output the log file header:
                    outputLogFileHeader();
                }

                // Output the session header:
                outputSessionHeader();
            }
        }
    }

#endif // _GR_IPHONE_DEVICE_BUILD

    return _isInitialized;
}


// ---------------------------------------------------------------------------
// Name:        osDebugLog::terminate
// Description: Closes the debug log file.
// Author:      AMD Developer Tools Team
// Date:        30/8/2005
// ---------------------------------------------------------------------------
void osDebugLog::terminate()
{
    if (_isInitialized)
    {
        // All "log file terminated" printout:
        addPrintout(OS_PREPEND_L(__FUNCTION__) , OS_PREPEND_L(__FILE__), __LINE__ , OS_STR_DebugLogIsTerminated, OS_DEBUG_LOG_INFO);
    }

    // If the debug log file is opened - close it:
    if (_debugLogFile.isOpened())
    {
        _debugLogFile.close();
    }

    // Mark that I am not initialized:
    _isInitialized = false;
}


// ---------------------------------------------------------------------------
// Name:        osDebugLog::addPrintout
// Description:
//   Adds a potential debug print to the debug log.
//   The debug print will be actually added into the debug log if its severity
//   is "higher" than the current debug log logged severity.
//   Examples:
//   1. If the current debug log logged severity is OS_DEBUG_LOG_ERROR and the
//      added message severity is OS_DEBUG_LOG_INFO, the message will NOT be added
//      into the log file.
//   1. If the current debug log logged severity is OS_DEBUG_LOG_EXTENSIVE, all
//      message will be added into the log file.
//
// Arguments:  functionName - The name of the function that added the printout.
//                            (Use the __FUNCTION__ macro).
//             fileName - The name of the file that contains the code that added
//                        the printout (Use the __FILE__ macro).
//             lineNumber - The line number in that file in which the addPrintout
//                          call resides.
//             message - The logged message.
//             severity - The message severity.
//
// Author:      AMD Developer Tools Team
// Date:        29/8/2005
// ---------------------------------------------------------------------------
void osDebugLog::addPrintout(const wchar_t* functionName, const wchar_t* fileName, int lineNumber,
                             const wchar_t* message, osDebugLogSeverity severity)
{
    if (isAboveLoggedSeverityThreshold(severity))
    {
        // Create the printout object:
        osDebugLogPrintout printout;
        printout._printoutSeverity = severity;

        // Get the current thread id:
        osThreadId currentThreadId = osGetCurrentThreadId();

        // Translate it to a string:
        gtString currentThreadIdAsString;
        osThreadIdAsString(currentThreadId, currentThreadIdAsString);

        // Get the log severity (as a string):
        const wchar_t* severityAsString = osDebugLogSeverityToString(severity);

        // Get the current time (as a string):
        osTime currentTime;
        currentTime.setFromCurrentTime();
        gtString currentTimeAsString;
        currentTime.timeAsString(currentTimeAsString, osTime::DATE_TIME_DISPLAY, osTime::LOCAL);

        // Add the current time stamp (milliseconds):
        gtString timeStampAsString;
        osStopWatch::appendCurrentTimeAsString(timeStampAsString);

        gtString timeStampMilli;
        timeStampAsString.getSubString(timeStampAsString.length() - 3, timeStampAsString.length() - 1, timeStampMilli);
        currentTimeAsString.appendFormattedString(L".%ls", timeStampMilli.asCharArray());

        // Build the printout string:
        printout._printoutString.makeEmpty();
        printout._printoutString += currentTimeAsString;
        printout._printoutString += OS_STR_DebugLogDelimiter;
        printout._printoutString += timeStampAsString;
        printout._printoutString += OS_STR_DebugLogDelimiter;
        printout._printoutString += severityAsString;
        printout._printoutString += OS_STR_DebugLogDelimiter;
        printout._printoutString += m_currentSessionStartTime;
        printout._printoutString += OS_STR_DebugLogDelimiter;
        printout._printoutString += currentThreadIdAsString;
        printout._printoutString += OS_STR_DebugLogDelimiter;

        if (functionName)
        {
            printout._printoutString += functionName;
        }

        printout._printoutString += OS_STR_DebugLogDelimiter;
        printout._printoutString += fileName;
        printout._printoutString += OS_STR_DebugLogDelimiter;
        printout._printoutString.appendFormattedString(L"%d", lineNumber);
        printout._printoutString += OS_STR_DebugLogDelimiter;
        printout._printoutString += message;
        printout._printoutString += '\n';

#ifdef _GR_IPHONE_DEVICE_BUILD
        // Add a prefix to strings we send to the iPhone console:
        printout._printoutString.prepend(OS_STR_iPhoneConsolePrintoutPrefix);
#endif

        // Output the printout in a thread synchronized way:
        addSynchronizedPrintout(printout);
    }
}

// ---------------------------------------------------------------------------
// Name:        osDebugLog::addPrintout
// Description: char version of the function that calls after conversion
//              the unicode version
// Author:      AMD Developer Tools Team
// Date:        19/6/2011
// ---------------------------------------------------------------------------

void osDebugLog::addPrintout(const char* functionName, const char* fileName, int lineNumber,
                             const wchar_t* message, osDebugLogSeverity severity)
{
    if (isAboveLoggedSeverityThreshold(severity))
    {
        gtString functionNameStr;
        gtString fileNameStr;
        functionNameStr.fromASCIIString(functionName, (int)strlen(functionName));
        fileNameStr.fromASCIIString(fileName, (int)strlen(fileName));

        addPrintout(functionNameStr.asCharArray(), fileNameStr.asCharArray(), lineNumber, message, severity);
    }
}

// ---------------------------------------------------------------------------
// Name:        osDebugLog::onAssertionFailure
// Description: Is called when an assertion failure occur.
// Arguments:   functionName - The name of the function that triggered the assertion
//              fileName, lineNumber - The name of the file and line number that contains
//                                     the code that triggered the assertion.
//              message - An optional assertion message.
// Author:      AMD Developer Tools Team
// Date:        30/8/2005
// ---------------------------------------------------------------------------
void osDebugLog::onAssertionFailure(const wchar_t* functionName, const wchar_t* fileName,
                                    int lineNumber, const wchar_t* message)
{
    // Add a printout to the log file:
    addPrintout(functionName, fileName, lineNumber, message, OS_DEBUG_LOG_ERROR);
}


// ---------------------------------------------------------------------------
// Name:        osDebugLog::registerSelfAsAssertionHandler
// Description:
//  Registers this class instance as the assertion handler.
//  I.E: This class onAssertionFailure will be called whenever an assertion
//       failure occurres.
// Author:      AMD Developer Tools Team
// Date:        21/2/2007
// ---------------------------------------------------------------------------
void osDebugLog::registerSelfAsAssertionHandler()
{
    // Yaki - 18/10/2006:
    // On Win32, console applications, in debug mode, assert() calls abort() by
    // default. We change the error mode to _OUT_TO_MSGBOX to prevent assert() from
    // calling about() and making the program exit.
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    _set_error_mode(_OUT_TO_MSGBOX);
#endif
#endif

    // Register this class instance as the global assertion handler:
    gtRegisterAssertionFailureHandler(this);
}


// ---------------------------------------------------------------------------
// Name:        osDebugLog::calculateLogFilePath
// Description: Inputs a log file name and output a log file path under the
//              current user temp directory.
// Arguments: logFileName - The log file name.
//            logFilePath - Will get the log file path.
// Author:      AMD Developer Tools Team
// Date:        31/8/2005
// ---------------------------------------------------------------------------
void osDebugLog::calculateLogFilePath(const gtString& logFileName, osFilePath& logFilePath)
{
    // Get the current user name:
    gtString currentUserName;

    // Yaki - 28/9/2008:
    // The call to osGetCurrentUserName was replaced by osGetCurrentProcessEnvVariableValue("USERNAME").
    // This is done, since it is not recommended to call the Win32 GetUserName function, inside a DLLMain
    // function (See DllMain Callback Function MSDN documentation). To be more specific, calling GetUserName
    // under Vista crashes Qt based applications (Google earth, Autodesk MudBox, etc).
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    bool rc1 = osGetCurrentProcessEnvVariableValue(L"USERNAME", currentUserName);
#else
    bool rc1 = osGetCurrentUserName(currentUserName);
#endif

    if (!rc1)
    {
        currentUserName = L"unknown";
    }

    // Add the current user name to the log file name:
    gtString logFileNameWithUser = logFileName;
    logFileNameWithUser += L"-";
    logFileNameWithUser += currentUserName;

    // Generate the log file path (under the user temp directory):
    osFilePath debugLogFilePath = logFilePath.isDirectory() ? logFilePath : osFilePath::OS_TEMP_DIRECTORY;
    debugLogFilePath.setFileName(logFileNameWithUser);
    debugLogFilePath.setFileExtension(OS_DEBUG_LOG_FILE_EXTENSION);

    // Output it:
    logFilePath = debugLogFilePath;
}


// ---------------------------------------------------------------------------
// Name:        osDebugLog::calculateLogFileOpenMode
// Description: Calculates a log file open mode.
// Return Val:  osFile::osOpenMode - Will get the log file open mode.
// Author:      AMD Developer Tools Team
// Date:        31/8/2005
// Implementation notes:
//   If an old log file of the input path exists and its size is bigger than
//   the maximal old log file size, we will erase the old log file content
//   while opening it. Otherwise, we will append to an existing log file.
// ---------------------------------------------------------------------------
osFile::osOpenMode osDebugLog::calculateLogFileOpenMode() const
{
    osFile::osOpenMode retVal = osFile::OS_OPEN_TO_WRITE;

    // This is not relevant on the iPhone, where we don't print to a file:
#ifndef _GR_IPHONE_DEVICE_BUILD
    // If the file already exist on disk:
    bool fileExistsOnDisk = _debugLogFile.path().isRegularFile();

    if (fileExistsOnDisk)
    {
        // Get the file size:
        unsigned long fileSize = 0;
        GT_IF_WITH_ASSERT(_debugLogFile.getSize(fileSize))
        {
            // If the file is smaller than the maximal old file size, we will
            // open the file for append:
            if (fileSize < OS_MAX_OLD_LOG_FILE_SIZE)
            {
                retVal = osFile::OS_OPEN_TO_APPEND;
            }
        }
    }

#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osDebugLog::outputLogFileHeader
// Description: Outputs the log file header
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/8/2005
// ---------------------------------------------------------------------------
bool osDebugLog::outputLogFileHeader()
{
    bool retVal = false;

    // If the log file is initialized:
    if (_isInitialized)
    {
        OS_OUTPUT_DEBUG_LOG(OS_STR_DebugLogHeader, OS_DEBUG_LOG_INFO);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osDebugLog::updateOSString
// Description: Retrieves the Operating System string and stores it in _osShortDescriptionString.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/7/2007
// ---------------------------------------------------------------------------
bool osDebugLog::updateOSString()
{
    bool retVal = false;

    bool rc1 = osGetOSShortDescriptionString(_osShortDescriptionString);
    GT_IF_WITH_ASSERT(rc1)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osDebugLog::outputSessionHeader
// Description: Output the current session header into the log file.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/8/2005
// ---------------------------------------------------------------------------
bool osDebugLog::outputSessionHeader()
{
    bool retVal = false;

    // If the log file is initialized:
    if (_isInitialized)
    {
        // Get the current application name:
        gtString currentAppName = OS_STR_UnknownApplication;
        osGetCurrentApplicationName(currentAppName);

        // Get the OS Version string:
        updateOSString();

        // Add comments to the system information string:
        gtString sysInfoStr = _osDescriptionString;
        gtStringTokenizer systemInfoTok(_osDescriptionString, L"\n");
        gtString sysInfoToken;

        while (systemInfoTok.getNextToken(sysInfoToken))
        {
            OS_OUTPUT_DEBUG_LOG(sysInfoToken.asCharArray(), OS_DEBUG_LOG_INFO);
        };

        // Build a session start message:
        // *INDENT-OFF*
        gtString sessionStartMessage;
        sessionStartMessage.append(L"Application=").append(currentAppName.asCharArray());
        sessionStartMessage.append(L", Product=").append(_productDescriptionString.asCharArray());
        sessionStartMessage.append(L", OS=").append(_osShortDescriptionString.asCharArray());
        sessionStartMessage.append(L", Version=");

        osProductVersion appVersion;
        osGetApplicationVersion(appVersion);
        sessionStartMessage.append(appVersion.toString()).append(' ');
        // *INDENT-ON*

#if AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
        sessionStartMessage += OS_STR_ReleaseVersion;

#elif AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
        sessionStartMessage += OS_STR_DebugVersion;

#endif

        OS_OUTPUT_DEBUG_LOG(sessionStartMessage.asCharArray(), OS_DEBUG_LOG_INFO);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osDebugLog::addSynchronizedPrintout
//
// Description:
//  Adds a debug printout into the debug log. This is done in a threads synchronized
//  manner.
//
// Arguments: printout - The debug printout to be added to the debug log file.
// Author:      AMD Developer Tools Team
// Date:        31/8/2009
//
// Implementation notes:
//  Sometimes, a thread within the debugged process is frozen by the debugger while
//  adding a debug printout. In this stage, any thread trying to add a debug printout
//  will also freeze while trying to lock the write critical section.
//  This problem sometimes occur when we try to execute a function within the debugged
//  process address space using the debugger (see osProcessDebugger::makeThreadExecuteFunction)
//  when debug log level is DEBUG.
//  To fix the problem, the thread tries to lock the write critical section for 100 ms.
//  If it does not manage to do so, it only adds the printout to a pending printouts queue.
//  The printout will be printed at the next time addSynchronizedPrintout will be called.
// ---------------------------------------------------------------------------
void osDebugLog::addSynchronizedPrintout(const osDebugLogPrintout& printout)
{
    // Try to entering the write critical section for 100 ms:
    bool criticalSectionEntered = false;

    for (int i = 0; i < 20; i++)
    {
        criticalSectionEntered = _writeCriticalSection.tryEntering();

        if (criticalSectionEntered)
        {
            break;
        }
        else
        {
            osSleep(5);
        }
    }

    // If we didn't manage entering the critical section:
    if (!criticalSectionEntered)
    {
        // Add the printout to the pending debug printouts queue:
        bool enteredQueueCS = _pendingDebugPrintoutsCriticalSection.tryEntering();

        if (enteredQueueCS)
        {
            _pendingDebugPrintouts.push(printout);
            _pendingDebugPrintoutsCriticalSection.leave();
        }
        else
        {
            osWPerror(OS_STR_FAILED_TO_ADD_DEBUG_MSG_TO_QUEUE);
        }
    }
    else
    {
        // We entered the write critical section.

        // If there are pending printouts, add them to the debug log file:
        bool enteredQueueCS = _pendingDebugPrintoutsCriticalSection.tryEntering();

        if (!enteredQueueCS)
        {
            osWPerror(OS_STR_FAILED_TO_RETRIEVE_DEBUG_MSG_FROM_QUEUE);
        }
        else
        {
            while (!_pendingDebugPrintouts.empty())
            {
                osDebugLogPrintout pendingMsg = _pendingDebugPrintouts.front();
                _pendingDebugPrintouts.pop();
                pendingMsg._printoutString.prepend(OS_STR_DELAYED_DEBUG_PRINTOUT_PREFIX);

#ifdef _GR_IPHONE_DEVICE_BUILD
                osAddPrintoutToiPhoneConsole(pendingMsg._printoutString, pendingMsg._printoutSeverity);
#else // ndef _GR_IPHONE_DEVICE_BUILD
                _debugLogFile.writeString(pendingMsg._printoutString);
#endif // _GR_IPHONE_DEVICE_BUILD
            }

            _pendingDebugPrintoutsCriticalSection.leave();
        }

#ifdef _GR_IPHONE_DEVICE_BUILD
        // Add the current message to the iPhone console:
        osAddPrintoutToiPhoneConsole(printout._printoutString, printout._printoutSeverity);
#else // ndef _GR_IPHONE_DEVICE_BUILD
        // Add the current message into the debug log file:
        _debugLogFile.writeString(printout._printoutString);

        // Flush the log file:
        // (This slows down performance, but verifies that we get last printouts even when we crash):
        _debugLogFile.flush();
#endif // _GR_IPHONE_DEVICE_BUILD

        // Leave the write critical section:
        _writeCriticalSection.leave();
    }
}


// ---------------------------------------------------------------------------
// Name:        osDebugLog::setLoggedSeverity
// Description: Set debug log file severity.
// Arguments:   osDebugLogSeverity loggedSeverity
// Return Val:  void
// Author:      AMD Developer Tools Team
// Date:        20/10/2009
// ---------------------------------------------------------------------------
void osDebugLog::setLoggedSeverity(osDebugLogSeverity loggedSeverity)
{
    if ((OS_DEBUG_LOG_DEBUG <= loggedSeverity) && (OS_DEBUG_LOG_DEBUG > _loggedSeverity))
    {
        _loggedSeverityChangedToHigh = true;
    }

    // Set the debugged log severity:
    _loggedSeverity = loggedSeverity;

    // Output the message:
    gtString logSeverity = osDebugLogSeverityToString(_loggedSeverity);
    gtString message;
    message.appendFormattedString(OS_STR_DebugLogSeverity, logSeverity.asCharArray());
    OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_INFO);
}

// ---------------------------------------------------------------------------
// Name:        osDebugLog::setLoggedSeverity
// Description: Set debug log file severity.
// Arguments:   osDebugLogSeverity loggedSeverity
// Return Val:  void
// Author:      AMD Developer Tools Team
// Date:        20/10/2009
// ---------------------------------------------------------------------------
void osDebugLog::setLoggedSeverityChangedToHigh(bool loggedSeverityChangedToHigh)
{
    // Set the debugged log severity:
    _loggedSeverityChangedToHigh = loggedSeverityChangedToHigh;
}

// ---------------------------------------------------------------------------
// Name:        osDebugLog::loggedSeverityAsString
// Description: Return a string that represents the log level
// Arguments:   osDebugLogSeverity severity
// Return Val:  const char*
// Author:      AMD Developer Tools Team
// Date:        23/8/2010
// ---------------------------------------------------------------------------
const wchar_t* osDebugLog::loggedSeverityAsString(osDebugLogSeverity severity)
{
    const wchar_t* pRetVal = OS_STR_OptionsAdvancedDebugLogLevelUnknown;

    switch (severity)
    {
        case OS_DEBUG_LOG_ERROR:
            pRetVal = OS_STR_OptionsAdvancedDebugLogLevelError;
            break;

        case OS_DEBUG_LOG_INFO:
            pRetVal = OS_STR_OptionsAdvancedDebugLogLevelInfo;
            break;

        case OS_DEBUG_LOG_DEBUG:
            pRetVal = OS_STR_OptionsAdvancedDebugLogLevelDebug;
            break;

        case OS_DEBUG_LOG_EXTENSIVE:
            pRetVal = OS_STR_OptionsAdvancedDebugLogLevelExtensive;
            break;

        default:
            GT_ASSERT_EX(false, L"Unknown log level");
            break;
    }

    return pRetVal;
}

void osDebugLog::StartSession()
{
    // Add the current time stamp (milliseconds):
    m_currentSessionStartTime.makeEmpty();
    osStopWatch::appendCurrentTimeAsString(m_currentSessionStartTime);
}

void osDebugLog::EndSession()
{
    m_currentSessionStartTime = L"0";
}

inline bool osDebugLog::isAboveLoggedSeverityThreshold(osDebugLogSeverity severity) const
{
    // If the log file is initialized:the message severity is "higher" than the current debug log logged severity:
    return _isInitialized && severity <= _loggedSeverity;
}


//////////////////////////////////////////////////////////////////////////
/// osDebugLogTrace class functions
//////////////////////////////////////////////////////////////////////////
osDebugLogTrace::osDebugLogTrace(const wchar_t* funcName) : m_pRetVal(nullptr)
{
    if (osDebugLog::instance().isAboveLoggedSeverityThreshold(OS_DEBUG_LOG_DEBUG))
    {
        m_funcName = funcName;
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Entering %ls()", m_funcName.asCharArray());
    }
}

osDebugLogTrace::osDebugLogTrace(const wchar_t* funcName, bool& retVal) : m_pRetVal(&retVal)
{
    if (osDebugLog::instance().isAboveLoggedSeverityThreshold(OS_DEBUG_LOG_DEBUG))
    {
        m_funcName = funcName;
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Entering %ls()", m_funcName.asCharArray());
    }
}

osDebugLogTrace::osDebugLogTrace(const char* funcName) : m_pRetVal(nullptr)
{
    if (osDebugLog::instance().isAboveLoggedSeverityThreshold(OS_DEBUG_LOG_DEBUG))
    {
        m_funcName.fromASCIIString(funcName, (int)strlen(funcName));
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Entering %ls()", m_funcName.asCharArray());
    }
}

osDebugLogTrace::osDebugLogTrace(const char* funcName, bool& retVal) : m_pRetVal(&retVal)
{
    if (osDebugLog::instance().isAboveLoggedSeverityThreshold(OS_DEBUG_LOG_DEBUG))
    {
        m_funcName.fromASCIIString(funcName, (int)strlen(funcName));
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Entering %ls()", m_funcName.asCharArray());
    }
}

osDebugLogTrace::~osDebugLogTrace()
{
    if (nullptr == m_pRetVal)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Exiting %ls()", m_funcName.asCharArray());
    }
    else
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Exiting %ls(), returned %d", m_funcName.asCharArray(), int(*m_pRetVal));
    }
}

