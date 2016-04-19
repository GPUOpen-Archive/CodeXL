//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSpiesUtilitiesDLLInitializationFunctions.cpp
///
//==================================================================================

//------------------------------ suSpiesUtilitiesDLLInitializationFunctions.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osMessageBox.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/apiClassesInitFunc.h>
#include <AMDTAPIClasses/Include/apApiFunctionsInitializationData.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>

// Local:
#include <src/suAPIFunctionsStubs.h>
#include <src/suAPIFunctionsImplementations.h>
#include <src/suSpiesUtilitiesDLLInitializationFunctions.h>
#include <src/suSpyToAPIConnector.h>
#include <AMDTServerUtilities/Include/suTechnologyMonitorsManager.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // Mac only:
    #include <fcntl.h>
    #ifdef _GR_IPHONE_DEVICE_BUILD
        // iPhone on-device only:
        #include <AMDTOSWrappers/Include/osSocket.h>
        #include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>
    #endif
#endif


// ---------------------------------------------------------------------------
// Name:        suInitializeSpiesUtilitiesModule
// Description: Initializes the Spies utilities dll.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/11/2009
// ---------------------------------------------------------------------------
bool suInitializeSpiesUtilitiesModule()
{
    static bool stat_retVal = false;

    // Verify that we are only initialized once:
    static bool stat_wasInitialized = false;

    if (!stat_wasInitialized)
    {
        stat_wasInitialized = true;

        // Set the thread naming prefix:
        osThread::setThreadNamingPrefix("CodeXL Servers");

        // Initialize global variables:
        suInitializeGlobalVariables();

        // Initialize the APIClasses library:
        bool rcAPIClasses = apiClassesInitFunc();
        GT_IF_WITH_ASSERT(rcAPIClasses)
        {
            // Initialize the Spies API functions infrastructure:
            suInitializeSpyAPIFunctionsInfra();

            // Register API Stub functions:
            suRegisterAPIStubFunctions();

            // Triggering AP_API_CONNECTION_ESTABLISHED event with AP_SPIES_UTILITIES_API_CONNECTION:
            // =====================================================================================
            // Instead of us, the client side API will trigger the AP_API_CONNECTION_ESTABLISHED event with AP_SPIES_UTILITIES_API_CONNECTION.
            // This resolves a bootstrapping issue in which the client needs to get this event in order of creating the spies event forwarding pipe.
            // - The client side event is triggered by gaSpyConnectionWaiterThread::triggerAPIConnectionEstablishedEvent
            // - We are skipping the suRegisterAPIConnectionAsActive(AP_SPIES_UTILITIES_API_CONNECTION) call

            // Initialize the API connection or standalone mode:
            bool rcInitAPI = suInitializeAPIOrStandaloneMode();

            if (!rcInitAPI)
            {
                // Display a communication failed message:
                suDisplayCommunicationFailedMessage();
            }
            else
            {
                // Redirect stdout if needed:
                bool rcStdOut = suRedirectSpyStdOutIfNeeded();
                GT_IF_WITH_ASSERT(rcStdOut)
                {
                    stat_retVal = true;
                }
            }
        }

        if (stat_retVal)
        {
            // Display an initialization success message:
            OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_SpyUtilitiesInitializedSuccessfully, OS_DEBUG_LOG_DEBUG);
        }
        else
        {
            suOnSpiesUtilitiesInitializationFailure();
        }
    }

    return stat_retVal;
}


// ---------------------------------------------------------------------------
// Name:        suTerminateSpiesUtilitiesModule
// Description: Terminates the Spies Utilities DLL.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/11/2009
// ---------------------------------------------------------------------------
bool suTerminateSpiesUtilitiesModule()
{
    bool retVal = true;

    // Terminate the Spy API thread:
    suSpyToAPIConnector& apiConnector = suSpyToAPIConnector::instance();
    bool rc1 = apiConnector.terminate();

    if (!rc1)
    {
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_FailedToTerminateAPI, OS_DEBUG_LOG_ERROR);
    }

    // Terminate the debug log file:
    suTerminateDebugLogFile();

    retVal = rc1;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suReportDebuggedProcessTermination
// Description:
//   Reports, to the OpenGL server and to the debugged process debugger, that the
//   debugged process is going to be terminated.
//
// Arguments:
//  reportToDebugger - true - report both to the OpenGL server and to the debugged process debugger.
//                     false - report only to the OpenGL server.
//
// Author:      Yaki Tebeka
// Date:        26/10/2008
// Implementations notes:
//   - The report to the debugger is done by throwing a breakpoint with an
//     AP_BEFORE_DEBUGGED_PROCESS_TERMINATION_HIT break reason.
//   - When this function is called, the Spy API thread is already terminated. However, we would
//     like to continue and answer API requests. To achieve that, we listen to the API socket using
//     the thread that called this function.
// ---------------------------------------------------------------------------
void suReportDebuggedProcessTermination()
{
    // If the debugged process termination was not already reported:
    if (!suIsDuringDebuggedProcessTermination())
    {
        // Mark that the debugged process is during termination process:
        suOnDebuggedProcessTermination();

        // Output a debug string:
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_reportingDebuggedProcessTermination, OS_DEBUG_LOG_DEBUG);

        // Alert the technology monitors about the coming debugged process termination:
        suTechnologyMonitorsManager::instance().notifyMonitorsOnProcessTerminationAlert();

        // If we are running with a debugger:
        bool isRunningInStandaloneMode = suIsRunningInStandaloneMode();

        if (!isRunningInStandaloneMode)
        {
            // If we need to report the debugged process debugger about the termination:
            bool reportToDebugger = !suIsTerminationInitiatedByAPI();

            if (reportToDebugger)
            {
                // Notify the debugger we are about to enter the termination loop:
                suBeforeEnteringTerminationAPILoop();
            }

#ifdef _GR_IPHONE_DEVICE_BUILD
            // On the iPhone, we need to send a debugged process terminated event, even if we pressed the "stop" button:
            apDebuggedProcessTerminatedEvent terminationEvent(0);
            bool rcEve = suForwardEventToClient(terminationEvent);
            GT_ASSERT(rcEve);
#endif
        }

        // Unregister us from the "Active APIs" list:
        suRegisterAPIConnectionAsInactive(AP_SPIES_UTILITIES_API_CONNECTION);
    }
}

// ---------------------------------------------------------------------------
// Name:        suInitializeDebugLogFile
// Description: Initializes the Spies debug log file.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
bool suInitializeDebugLogFile()
{
    bool retVal = false;

    // Initialize the debug log:
    osDebugLog& theDebugLog = osDebugLog::instance();
    bool rc1 = theDebugLog.initialize(SU_STR_debugLogFileName, SU_STR_debugLogServersUtilitiesDescription);
    GT_ASSERT(rc1);

    // Get the debug log severity level (passed to the spy as an environment variable):
    gtString debugLosSeverityAsString;
    bool rc2 = osGetCurrentProcessEnvVariableValue(OS_STR_envVar_debugLogSeverity, debugLosSeverityAsString);

    if (!rc2)
    {
        // Prompt an error:
        OS_OUTPUT_DEBUG_LOG(SU_STR_FailedToGetDebugLogSeverity, OS_DEBUG_LOG_ERROR);
    }
    else
    {
        // Translate the debug log severity string to a osDebugLogSeverity:
        osDebugLogSeverity debugLogSeverity = osStringToDebugLogSeverity(debugLosSeverityAsString.asCharArray());

        // Initialize the debug log severity to it:
        theDebugLog.setLoggedSeverity(debugLogSeverity);
    }

    // rcDebuggedAppNameEnvVar failure should not fail this function:
    retVal = rc1;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suTerminateDebugLogFile
// Description: Terminates and closes the Spy's debug log file.
// Return Val: void
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void suTerminateDebugLogFile()
{
    osDebugLog& theDebugLog = osDebugLog::instance();
    theDebugLog.terminate();
}


// ---------------------------------------------------------------------------
// Name:        suInitializeAPIOrStandaloneMode
// Description:
//  Initializes API connection with the debugger, or standalone mode where applicable
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        3/1/2010
// ---------------------------------------------------------------------------
bool suInitializeAPIOrStandaloneMode()
{
    bool retVal = false;
    bool isRunningInAPIMode = false;

    // If the process in which we are loaded (or it's ancestor) was launched using CodeXL:
    bool wasLaunchedUsingCodeXL = suWasLaunchedUsingCodeXL();

    if (wasLaunchedUsingCodeXL)
    {
        // If this module is attached to the currently debugged application:
        bool isAttachedToDebuggedApp = suIsAttachedToDebuggedApplication();

        if (isAttachedToDebuggedApp)
        {
            // Initialize the connection between this Spy DLL and its API:
            bool rcInitAPI = suInitializeSpyToAPIConnection();
            GT_IF_WITH_ASSERT(rcInitAPI)
            {
                // Mark that the servers are running in "API" mode:
                isRunningInAPIMode = true;
                retVal = true;
            }
        }
        else
        {
            // We are running under a debugger, but this module is not attached to the currently debugged application.
            // This means that either:
            // a. A gremedy developer is debugging in a "standalone mode".
            // b. The debugged application launched another process. We would not like to attach this process to the client API.
            //    (Example: Maya on windows launches its splash screen as a new process that also uses OpenGL)
            isRunningInAPIMode = false;
            retVal = true;
        }
    }

    // If we are not running in "API" mode:
    if (!isRunningInAPIMode)
    {
        // Initialize the API in a "stand-alone" mode:
        suInitializeAPIInStandaloneMode();

        // Output an appropriate debug log message:
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_ServersAreInitializedInStandaloneMode, OS_DEBUG_LOG_INFO);
    }

    // Log the run mode:
    suSetStandaloneModeStatus(!isRunningInAPIMode);

    // If we are in a debug build:
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    {
        // Return true to enable debugging the spies in a standalone configuration:
        retVal = true;
    }
#endif

    // Get the global environment settings:
    suGlobalServerEnvironmentSettings& mutableEnvironmentSettings = suGetMutableGlobalServerEnvironmentSettings();
    gtString gsDontForceOpenGLDebugContextsString;

    if (osGetCurrentProcessEnvVariableValue(OS_STR_envVar_gsDontForceOpenGLDebugContexts, gsDontForceOpenGLDebugContextsString))
    {
        unsigned int gsDontForceOpenGLDebugContextsFlag = 0;

        if (gsDontForceOpenGLDebugContextsString.toUnsignedIntNumber(gsDontForceOpenGLDebugContextsFlag))
        {
            mutableEnvironmentSettings.m_gsDontForceOpenGLDebugContexts = (0 != gsDontForceOpenGLDebugContextsFlag);
        }
    }

    gtString csDontAddDebuggingBuildFlagsString;

    if (osGetCurrentProcessEnvVariableValue(OS_STR_envVar_csDontAddDebuggingBuildFlags, csDontAddDebuggingBuildFlagsString))
    {
        unsigned int csDontAddDebuggingBuildFlagsFlag = 0;

        if (csDontAddDebuggingBuildFlagsString.toUnsignedIntNumber(csDontAddDebuggingBuildFlagsFlag))
        {
            mutableEnvironmentSettings.m_csDontAddDebuggingBuildFlags = (0 != csDontAddDebuggingBuildFlagsFlag);
        }
    }

    gtString hdForceResetOfKernelDebugging;

    if (osGetCurrentProcessEnvVariableValue(OS_STR_envVar_hdForceResetOfKernelDebugging, hdForceResetOfKernelDebugging))
    {
        unsigned int hdForceResetOfKernelDebuggingFlag = 0;

        if (hdForceResetOfKernelDebugging.toUnsignedIntNumber(hdForceResetOfKernelDebuggingFlag))
        {
            mutableEnvironmentSettings.m_hdForceResetOfKernelDebugging = (0 != hdForceResetOfKernelDebuggingFlag);
        }
    }

    gtString suDontFixCRInSourceStrings;

    if (osGetCurrentProcessEnvVariableValue(OS_STR_envVar_suDontFixCRInSourceStrings, suDontFixCRInSourceStrings))
    {
        unsigned int suDontFixCRInSourceStringsFlag = 0;

        if (suDontFixCRInSourceStrings.toUnsignedIntNumber(suDontFixCRInSourceStringsFlag))
        {
            mutableEnvironmentSettings.m_suDontFixCRInSourceStrings = (0 != suDontFixCRInSourceStringsFlag);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suInitializeSpyToAPIConnection
// Description: Initialize the connection between this OpenCL Spy dll and its API
//              (That resides in the g-Debugger process).
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool suInitializeSpyToAPIConnection()
{
    bool retVal = false;

    // Get the event suppression flag before opening the events channel:
    gtString suppressSpyEventsAsString;
    bool rcSupp = osGetCurrentProcessEnvVariableValue(OS_STR_envVar_suppressSpyEvents, suppressSpyEventsAsString);

    if (rcSupp)
    {
        // If the environment variable was not set, leave it at its default value:
        if (suppressSpyEventsAsString == OS_STR_envVar_valueTrue)
        {
            suSupressSpyEvents(true);
        }
        else if (suppressSpyEventsAsString == OS_STR_envVar_valueFalse)
        {
            suSupressSpyEvents(false);
        }
    }

    // TCP/IP connection related variables:
    bool isUsingTCPIP = false;
    osPortAddress apiConnectionAddress;

    // Get an instance of the OpenCL API connector:
    suSpyToAPIConnector& apiConnector = suSpyToAPIConnector::instance();

    // Get the API connection socket shared memory object name:
    gtString apiConnectionSharedMemoryObjName;
    bool rcPipe = osGetCurrentProcessEnvVariableValue(OS_STR_envVar_APIPipeName, apiConnectionSharedMemoryObjName);
    bool rcEnvVariables = rcPipe;

    // Get the TCP / IP values from the appropriate environment variables:
    gtString ipHostname;
    bool rcHost = osGetCurrentProcessEnvVariableValue(OS_STR_envVar_serverConnectionIPHostname, ipHostname);
    gtString ipPortAsString;
    bool rcPort = osGetCurrentProcessEnvVariableValue(OS_STR_envVar_serverConnectionIPPort, ipPortAsString);

    // We expect only one connection method to be defined (note that even if this assertion fails, we still connect via the pipe):
    GT_ASSERT(rcPipe != (rcHost || rcPort));

    // We also expect both TCP / IP settings to be set, or neither:
    GT_ASSERT(rcHost == rcPort);

    // If we don't have a shared memory object, see if we have a TCP / IP connection:
    if (!rcPipe)
    {
        if (!(rcHost || rcPort))
        {
            // Output an appropriate debug log printout:
            OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_failedToReadAPISharedObjName, OS_DEBUG_LOG_ERROR);
        }
        else // rcHost || rcPort
        {
            unsigned int ipPort = 0;
            bool rcPortNum = ipPortAsString.toUnsignedIntNumber(ipPort);

            // Check we have all the info and the values are legal:
            if (rcHost && rcPort && rcPortNum && (ipPort <= USHRT_MAX))
            {
                apiConnectionAddress.setAsRemotePortAddress(ipHostname, (unsigned short)ipPort);
                rcEnvVariables = true;
                isUsingTCPIP = true;
            }
            else
            {
                // Output an appropriate debug log printout:
                OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_failedToReadTCPIPEnvVariables, OS_DEBUG_LOG_ERROR);
            }
        }
    }

    // If we managed to get all required environment variables value:
    GT_IF_WITH_ASSERT(rcEnvVariables)
    {
        // Initialize the API connection with the CodeXL application:
        if (isUsingTCPIP)
        {
            // Via TCP / IP:
            retVal = apiConnector.initialize(apiConnectionAddress);
        }
        else
        {
            // Via shared memory object:
            retVal = apiConnector.initialize(apiConnectionSharedMemoryObjName);
        }
    }

    if (retVal)
    {
        // Output a message that says that we are in API mode:
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_ServersAreInitializedInAPIMode, OS_DEBUG_LOG_INFO);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suInitializeAPIInStandaloneMode
// Description: "Initialize" the Spies API in spy "standalone" mode.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool suInitializeAPIInStandaloneMode()
{
    bool retVal = false;

    // Set the debugged application path to be the working directory:
    osFilePath workDirPath;
    osFilePath debuggedApplicationPath;
    bool rc = osGetCurrentApplicationPath(debuggedApplicationPath);
    GT_IF_WITH_ASSERT(rc)
    {
        osDirectory debuggedApplicationDir;
        rc = debuggedApplicationPath.getFileDirectory(debuggedApplicationDir);

        if (rc)
        {
            workDirPath = debuggedApplicationDir.directoryPath();
        }
    }

    // If we failed to get the debugged application working directory:
    if (!rc)
    {
        // Set the working directory to be the temp directory:
        workDirPath.setPath(osFilePath::OS_TEMP_DIRECTORY);
    }

    // Values needed for "initializing" the API:
    osFilePath logFilesDirPath(osFilePath::OS_TEMP_DIRECTORY);
    gtString logFilesDirFromEnvVar;

    if (osGetCurrentProcessEnvVariableValue(OS_STR_envVar_logFilesPath, logFilesDirFromEnvVar))
    {
        logFilesDirPath.setFullPathFromString(logFilesDirFromEnvVar);
    }

    osFilePath debuggerInstallDirPath(osFilePath::OS_TEMP_DIRECTORY);
    gtString debuggerInstallDirFromEnvVar;

    if (osGetCurrentProcessEnvVariableValue(OS_STR_envVar_debuggerInstallDir, debuggerInstallDirFromEnvVar))
    {
        debuggerInstallDirPath.setFullPathFromString(debuggerInstallDirFromEnvVar);
    }

    apApiFunctionsInitializationData initData;
    initData.setFrameTerminators(AP_DEFAULT_FRAME_TERMINATORS);
    initData.setDebuggerInstallDir(debuggerInstallDirPath);
    initData.setDebuggedProcessWorkDir(workDirPath);
    initData.setLogFilesDirectoryPath(logFilesDirPath);
    initData.setLoggedTexturesFileType(AP_JPEG_FILE);
    initData.setLoggingLimits(AP_DEFAULT_OPENGL_CONTEXT_CALLS_LOG_MAX_SIZE, AP_DEFAULT_OPENCL_CONTEXT_CALLS_LOG_MAX_SIZE, AP_DEFAULT_OPENGL_CONTEXT_CALLS_LOG_MAX_SIZE);

    // "Initialize" the API:
    bool rc1 = gaIntializeAPIImpl(initData);
    GT_ASSERT(rc1);

    // Disable textures data logging:
    suEnableImagesDataLogging(false);

    // If this is a debug build:
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    {
        // In debug set the standalone execution mode to debug:
        suSetDebuggedProcessExecutionMode(AP_DEBUGGING_MODE);
    }
#else
    {
        // Set the execution mode to profile mode:
        // suSetDebuggedProcessExecutionMode(AP_PROFILING_MODE);

        // Uri - 16/7/13 - Profiling mode is no longer supported by the CodeXL Debugging servers:
        suSetDebuggedProcessExecutionMode(AP_DEBUGGING_MODE);
    }
#endif

    retVal = rc1;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suWasLaunchedUsingCodeXL
// Description: Returns true iff this process (or one of it's ancestors) was
//              launched using CodeXL
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        13/1/2010
// ---------------------------------------------------------------------------
bool suWasLaunchedUsingCodeXL()
{
    bool retVal = false;

    // If we are on the iPhone device:
#ifdef _GR_IPHONE_DEVICE_BUILD
    {
        // Get a state variable name that should be available on iPhone.
        gtString serverConnectionIPHostname;
        bool rcEnvVar = osGetCurrentProcessEnvVariableValue(OS_STR_envVar_serverConnectionIPHostname, serverConnectionIPHostname);

        if (rcEnvVar)
        {
            // We managed to get an environment variable that was written using CodeXL:
            retVal = true;
        }
    }
#else
    {
        // Get the debugged application name (written as a state variable by the CodeXL):
        gtString debuggedApplicationName;
        bool rcDebuggedAppNameEnvVar = osGetCurrentProcessEnvVariableValue(OS_STR_envVar_debuggedAppName, debuggedApplicationName);

        if (rcDebuggedAppNameEnvVar)
        {
            // We managed to get an environment variable that was written using CodeXL:
            retVal = true;
        }
    }
#endif



    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suIsAttachedToDebuggedApplication
// Description: Returns true iff this module is attached to the currently debugged application's process.
// Author:      Yaki Tebeka
// Date:        3/1/2010
// ---------------------------------------------------------------------------
bool suIsAttachedToDebuggedApplication()
{
    bool retVal = false;

    // Get the debugged application name (written as a state variable by the debugger):
    gtString debuggedApplicationName;
    bool rcDebuggedAppNameEnvVar = osGetCurrentProcessEnvVariableValue(OS_STR_envVar_debuggedAppName, debuggedApplicationName);

    // Uri, 2/12/09 - On the iPhone, we currently do not check the debugged application name.
    // We could instruct the user to add a "SU_DEBUGGED_APPLICATION_NAME" env. var in XCode,
    // but it is pretty much unnecessary
#ifdef _GR_IPHONE_DEVICE_BUILD
    {
        rcDebuggedAppNameEnvVar = osGetCurrentApplicationName(debuggedApplicationName);
    }
#endif

    // If we didn't manage to get the debugged application name:
    if (!rcDebuggedAppNameEnvVar)
    {
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_failedToReadDebuggedApplicationName, OS_DEBUG_LOG_INFO);
    }
    else
    {
        // Get the name of the application to which this module is attached:
        gtString currentApplicationName;
        bool rcCurrAppName = osGetCurrentApplicationName(currentApplicationName);

        // Uri, 16/9/09: On the Mac, Autodesk Maya creates a small process to query the user if he wants to activate
        // the trial version. This mini-process does not have a bundle, and so our version of osGetCurrentApplicationPath() fails.
        // So, instead of failing here (which would cause the spy to load on this small process), we pass an dummy string and a "true"
        // return value so that CodeXL won't get stuck. This just causes the spy to not load for a process whose name we don't know.
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
        {
            if (!rcCurrAppName)
            {
                GT_ASSERT(rcCurrAppName);
                currentApplicationName = SU_STR_unknownApplicationName;
                rcCurrAppName = true;
            }
        }
#endif

        GT_IF_WITH_ASSERT(rcCurrAppName)
        {
            // If this module is attached to the currently debugged application:
            if (currentApplicationName.compareNoCase(debuggedApplicationName) == 0)
            {
                retVal = true;
            }
            else
            {
                // Output an appropriate debug log printout:
                gtString logMsg;
                logMsg.appendFormattedString(SU_STR_DebugLog_notRunningUnderDebuggedApp, debuggedApplicationName.asCharArray(), currentApplicationName.asCharArray());
                OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_INFO);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suRedirectSpyStdOutIfNeeded
// Description: Checks for the GS_STDOUT_REDIRECT environment variable and
//              applies it if it is present.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        7/6/2009
// ---------------------------------------------------------------------------
bool suRedirectSpyStdOutIfNeeded()
{
    bool retVal = false;

    // On Mac OSX:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    {
        // Get the output file name:
        gtString stdoutFilePath;
        bool rcEnvVar = osGetCurrentProcessEnvVariableValue(OS_STR_envVar_stdoutRedirectionFile, stdoutFilePath);

        if (rcEnvVar)
        {
            // Open it:
            int stdoutFileFD = ::open(stdoutFilePath.asCharArray(), O_WRONLY);
            GT_IF_WITH_ASSERT(stdoutFileFD > -1)
            {
                // Redirect this app's stdout to this file descriptor
                int resultCode = ::dup2(stdoutFileFD, /*stdout*/ 1);
                GT_IF_WITH_ASSERT(resultCode > -1)
                {
                    gtString redirectingMessage = SU_STR_DebugLog_redirectedStdoutMessage;
                    redirectingMessage.append(stdoutFilePath);
                    OS_OUTPUT_DEBUG_LOG(redirectingMessage.asCharArray(), OS_DEBUG_LOG_DEBUG);

                    retVal = true;
                }
            }
        }
        else
        {
            // This only happens if the environment variables doesn't exist, which means
            // we don't need to redirect it anyway:
            retVal = true;
        }
    }
#else
    {
        // This operation is never needed on Linux / Windows:
        retVal = true;
    }
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suInitializeGlobalVariables
// Description: Initializes the global variable to default values.
// Author:      Sigal Algranaty
// Date:        3/12/2009
// ---------------------------------------------------------------------------
void suInitializeGlobalVariables()
{
    // Initialize the log files directory to the user temp directory:
    // (When the API connection is established, we will get the log files directory from the debugger)
    osFilePath logFilesDirectoryPath;
    logFilesDirectoryPath.setPath(osFilePath::OS_TEMP_DIRECTORY);
    suSetCurrentSessionLogFilesDirectory(logFilesDirectoryPath);
}


// ---------------------------------------------------------------------------
// Name:        suDisplayCommunicationFailedMessage
// Description:
//   Pops up a communication failed message box to the user and outputs a
//   similar debug string to be displayed in the debugger.
//
// Author:      Yaki Tebeka
// Date:        6/10/2004
// ---------------------------------------------------------------------------
void suDisplayCommunicationFailedMessage()
{
    // Output an error message to be displayed by the debugger (if exists):
    osOutputDebugString(SU_STR_communicationFailedMessage);

    // If the process in which we are loaded (or it's ancestor) was launched using CodeXL:
    bool wasLaunchedUsingCodeXL = suWasLaunchedUsingCodeXL();

    if (wasLaunchedUsingCodeXL)
    {
        // Output the same error message in a message box:
        osMessageBox messageBox(SU_STR_CodeXLError, SU_STR_communicationFailedMessage, osMessageBox::OS_STOP_SIGN_ICON);
        messageBox.display();
    }
    else
    {
        // We weren't launched using CodeXL. This probably means that the user is running, outside of CodeXL,
        // an application that uses one of our servers. Build an appropriate message and display it in a message box:
        gtString message = SU_STR_ServerRunsWithNoDebuggerMessage1;

        gtString applicationName = L"UNKNOWN";
        osGetCurrentApplicationName(applicationName);

        message += applicationName;
        message += SU_STR_ServerRunsWithNoDebuggerMessage2;
        message += applicationName;
        message += SU_STR_ServerRunsWithNoDebuggerMessage3;
        message += applicationName;
        message += SU_STR_ServerRunsWithNoDebuggerMessage4;

        osFilePath applicationPath;
        bool rc = osGetCurrentApplicationPath(applicationPath);

        if (rc)
        {
            osDirectory appDir;
            rc = applicationPath.getFileDirectory(appDir);

            if (rc)
            {
                message += L" (";
                message += appDir.directoryPath().asString();
                message += L")";
            }
        }

        osMessageBox messageBox(SU_STR_CodeXLError, message, osMessageBox::OS_STOP_SIGN_ICON);
        messageBox.display();
    }
}


// ---------------------------------------------------------------------------
// Name:        suOnSpiesUtilitiesInitializationFailure
// Description:
//  Displays an initialization error to the user and exits the debugged process.
// Return Val: void
// Author:      Yaki Tebeka
// Date:        31/12/2009
// ---------------------------------------------------------------------------
void suOnSpiesUtilitiesInitializationFailure()
{
    // Write a failure message into the log file:
    OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_SpyUtilitiesInitializationFailed, OS_DEBUG_LOG_ERROR);
    osOutputDebugString(SU_STR_DebugLog_SpyUtilitiesInitializationFailed);

    // Output a similar error message in a message box:
    gtString applicationName = L"UNKNOWN";
    osGetCurrentApplicationName(applicationName);
    gtString errMsg;
    errMsg.appendFormattedString(GS_STR_SpiesUtilitiesInitializationFailureMessage, applicationName.asCharArray());
    osMessageBox messageBox(SU_STR_CodeXLError, errMsg.asCharArray(), osMessageBox::OS_STOP_SIGN_ICON);
    messageBox.display();

    // Exit the debugged application:
    osExitCurrentProcess(5);
}

