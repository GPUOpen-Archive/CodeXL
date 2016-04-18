//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suGlobalVariables.cpp
///
//==================================================================================

//------------------------------ suGlobalVariables.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>

// Local:
#include <src/suDebugLogInitializer.h>
#include <src/suSpiesUtilitiesModuleInitializer.h>
#include <AMDTServerUtilities/Include/suTechnologyMonitorsManager.h>
#include <AMDTServerUtilities/Include/suAllocatedObjectsMonitor.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suMemoryAllocationMonitor.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

// Mac OS X:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    #include <AMDTServerUtilities/Include/suMacOSXInterception.h>
    #include <AMDTServerUtilities/Include/suSpyBreakpointImplementation.h>
#endif

// ------------- Global variables -------------

// ---------- !!!! NOTICE - THIS SHOULD BE THE FIRST INITIALIZED STATIC VARIABLE !!!! ----------
// (This ensures that the we have an active debug log while static objects are initialized)
suDebugLogInitializer debugLogInitializer;

// The API thread id. We store it to make sure we are not using it to read API calls
// after the debugged process starts terminating
osThreadId su_stat_SpyAPIThreadId = OS_NO_THREAD_ID;

// Stores the current execution mode:
static apExecutionMode su_stat_executionMode = AP_DEBUGGING_MODE;

// Contains true when the spies are running in a "stand alone" mode: a mode in which the spies are running
// without an attached debugger.
static bool su_stat_isRunningInStandaloneMode = true;

// Will get true when we are during the debugged process termination:
static bool su_stat_isDuringDebuggedProcessTermination = false;

// The path of the log files directory:
static osFilePath* su_stat_SessionlogFilesDirectoryPath = NULL;

// The path of the log files directory:
static osFilePath* su_stat_ProjectlogFilesDirectoryPath = NULL;

// Gets true when textures data logging is enabled:
bool su_stat_isImagesDataLoggingEnabled = true;

// Logged textures file type:
static apFileType su_stat_loggedTexturesFileType = AP_JPEG_FILE;

// Logging limits:
static unsigned int su_stat_maxLoggedOpenGLCallsPerContext = AP_DEFAULT_OPENGL_CONTEXT_CALLS_LOG_MAX_SIZE;
static unsigned int su_stat_maxLoggedOpenCLCallsPerContext = AP_DEFAULT_OPENCL_CONTEXT_CALLS_LOG_MAX_SIZE;
static unsigned int su_stat_maxLoggedOpenCLCommandPerQueue = AP_DEFAULT_OPENCL_QUEUE_COMMANDS_LOG_MAX_SIZE;

// Global server environment settings:
static suGlobalServerEnvironmentSettings su_stat_globalServerEnvironmentSettings;

// The install directory of the debugger that debugs us:
static osFilePath* su_stat_debuggerInstallDirectory = NULL;

// Marks if the calls history logger should flush the log file after each monitored function call:
static bool su_stat_flushLogFileAfterEachFunctionCall = false;

// OpenGL frame terminators (a mask of apFrameTerminators values):
unsigned int su_stat_frameTerminatorsMask = AP_DEFAULT_FRAME_TERMINATORS;

// Slow time motion delay time units:
int su_stat_slowMotionDelayTimeUnits = 0;

// Log files recording:
// Contains true iff the HTML log files are active (are recording):
bool su_stat_areHTMLLogFilesActive = false;
// Contains true iff HTML log files were active during the run of the debugged process:
bool su_stat_wereHTMLLogFilesActive = false;

// Reference to the singleton instances:
suMemoryAllocationMonitor& su_stat_memoryAllocationMonitor = suMemoryAllocationMonitor::instance();
apMonitoredFunctionsManager& su_stat_theMonitoredFunMgr = apMonitoredFunctionsManager::instance();
suBreakpointsManager& su_stat_theBreakpointsManager = suBreakpointsManager::instance();
suAllocatedObjectsMonitor& su_stat_theAllocatedObjectsMonitor = suAllocatedObjectsMonitor::instance();
suInteroperabilityHelper& su_stat_interoperabilityHelper = suInteroperabilityHelper::instance();

// Mac OSX only:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // See gsMacOSXInterception.cpp
    suFunctionInterceptionInformation* su_stat_functionInterceptionInfo = NULL;

    // iPhone device only:
    #ifdef _GR_IPHONE_DEVICE_BUILD
        suARMv6InterceptionIsland* su_stat_armv6InterceptionIslands = NULL;
        suSpyBreakpointImplementation& su_stat_spyBreakpointsImplemenation = suSpyBreakpointImplementation::instance();
    #endif
#endif


// ---------- !!!! NOTICE - THIS SHOULD BE THE LAST INITIALIZED STATIC VARIABLE !!!! ----------
// (This ensures that the module will be initialized only after all static variables were initialized)
suSpiesUtilitiesModuleInitializer& su_stat_theSpiesUtilitiesInitializer = suSpiesUtilitiesModuleInitializer::instance();



// ------------- Global variables access functions -------------


// ---------------------------------------------------------------------------
// Name:        suSetSpiesAPIThreadId
// Description: Logs the Spies API thread id.
// Arguments: spiesAPIThreadId - The spies API thread id.
// Author:      Yaki Tebeka
// Date:        26/11/2009
// ---------------------------------------------------------------------------
void suSetSpiesAPIThreadId(osThreadId spiesAPIThreadId)
{
    // Output the API thread id to the log file:
    gtString logStr = SU_STR_DebugLog_APIThreadId;
    logStr.appendFormattedString(L"%u", spiesAPIThreadId);
    OS_OUTPUT_DEBUG_LOG(logStr.asCharArray(), OS_DEBUG_LOG_DEBUG);

    su_stat_SpyAPIThreadId = spiesAPIThreadId;
}


// ---------------------------------------------------------------------------
// Name:        suSpiesAPIThreadId
// Description: Retrieves the spies API thread id.
// Author:      Yaki Tebeka
// Date:        26/11/2009
// ---------------------------------------------------------------------------
osThreadId suSpiesAPIThreadId()
{
    return su_stat_SpyAPIThreadId;
}

// ---------------------------------------------------------------------------
// Name:        suIsSpiesAPIThreadId
// Description: Return true iff the input thread id is the spies API thread id.
// Author:      Uri Shomroni
// Date:        2/11/2015
// ---------------------------------------------------------------------------
bool suIsSpiesAPIThreadId(osThreadId threadId)
{
    return (su_stat_SpyAPIThreadId == threadId);
}

// ---------------------------------------------------------------------------
// Name:        suSetDebuggedProcessExecutionMode
// Description: Sets the current execution mode
// Author:      Yaki Tebeka
// Date:        4/11/2009
// ---------------------------------------------------------------------------
void suSetDebuggedProcessExecutionMode(apExecutionMode executionMode)
{
    su_stat_executionMode = executionMode;

    suTechnologyMonitorsManager::instance().notifyMonitorsOnDebuggedProcessExecutionModeChanged(executionMode);
}


// ---------------------------------------------------------------------------
// Name:        suDebuggedProcessExecutionMode
// Description: Returns the current execution mode.
// Author:      Yaki Tebeka
// Date:        4/11/2009
// ---------------------------------------------------------------------------
apExecutionMode suDebuggedProcessExecutionMode()
{
    return su_stat_executionMode;
}


// ---------------------------------------------------------------------------
// Name:        suSetStandaloneModeStatus
// Description: Sets the spies run status.
// Arguments:
//  isRunningInStandaloneMode - true - mark that we are running "standalone" mode - without an attached debugger.
//                              false - mark that we are running in an "API" mode - with an attached debugger.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
void suSetStandaloneModeStatus(bool isRunningInStandaloneMode)
{
    su_stat_isRunningInStandaloneMode = isRunningInStandaloneMode;
}


// ---------------------------------------------------------------------------
// Name:        suIsRunningInStandaloneMode
// Description: Returns true when the spies are running in a "standalone" mode - without an attached debugger.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
bool suIsRunningInStandaloneMode()
{
    return su_stat_isRunningInStandaloneMode;
}


// ---------------------------------------------------------------------------
// Name:        suOnDebuggedProcessTermination
// Description: Marks that the debugged process is during termination.
// Author:      Yaki Tebeka
// Date:        26/11/2009
// ---------------------------------------------------------------------------
void suOnDebuggedProcessTermination()
{
    su_stat_isDuringDebuggedProcessTermination = true;
}


// ---------------------------------------------------------------------------
// Name:        suIsDuringDebuggedProcessTermination
// Description: Returns true iff the debugged process is terminating.
// Author:      Yaki Tebeka
// Date:        26/11/2009
// ---------------------------------------------------------------------------
bool suIsDuringDebuggedProcessTermination()
{
    return su_stat_isDuringDebuggedProcessTermination;
}


// ---------------------------------------------------------------------------
// Name:        suSetCurrentSessionLogFilesDirectory
// Description: Set the spies log files directory path.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
void suSetCurrentSessionLogFilesDirectory(const osFilePath& directoryPath)
{
    if (su_stat_SessionlogFilesDirectoryPath == NULL)
    {
        su_stat_SessionlogFilesDirectoryPath = new osFilePath(directoryPath);
    }
    else
    {
        *su_stat_SessionlogFilesDirectoryPath = directoryPath;
    }
}


// ---------------------------------------------------------------------------
// Name:        suCurrentSessionLogFilesDirectory
// Description: Returns the spies log files directory path.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
const osFilePath& suCurrentSessionLogFilesDirectory()
{
    if (su_stat_SessionlogFilesDirectoryPath == NULL)
    {
        osFilePath defaultLogFilesPath(osFilePath::OS_TEMP_DIRECTORY);
        suSetCurrentSessionLogFilesDirectory(defaultLogFilesPath);
    }

    return *su_stat_SessionlogFilesDirectoryPath;
}

// ---------------------------------------------------------------------------
// Name:        suSetCurrentProjectLogFilesDirectory
// Description: Set the project log files directory path.
// Author:      Bhattacharyya Koushik
// Date:        28/08/2012
// ---------------------------------------------------------------------------
void suSetCurrentProjectLogFilesDirectory(const osFilePath& directoryPath)
{
    if (su_stat_ProjectlogFilesDirectoryPath == NULL)
    {
        su_stat_ProjectlogFilesDirectoryPath = new osFilePath(directoryPath);
    }
    else
    {
        *su_stat_ProjectlogFilesDirectoryPath = directoryPath;
    }
}


// ---------------------------------------------------------------------------
// Name:        suCurrentProjectLogFilesDirectory
// Description: Returns the project log files directory path.
// Author:      Bhattacharyya Koushik
// Date:        28/08/2012
// ---------------------------------------------------------------------------
const osFilePath& suCurrentProjectLogFilesDirectory()
{
    if (su_stat_ProjectlogFilesDirectoryPath == NULL)
    {
        osFilePath defaultLogFilesPath(osFilePath::OS_TEMP_DIRECTORY);
        suSetCurrentProjectLogFilesDirectory(defaultLogFilesPath);
    }

    return *su_stat_ProjectlogFilesDirectoryPath;
}

// ---------------------------------------------------------------------------
// Name:        suSetDebuggerInstallDir
// Description: Log the install directory of the debugger that debugs us.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
void suSetDebuggerInstallDir(const osFilePath& installDirectory)
{
    if (su_stat_debuggerInstallDirectory == NULL)
    {
        su_stat_debuggerInstallDirectory = new osFilePath(installDirectory);
    }
    else
    {
        *su_stat_debuggerInstallDirectory = installDirectory;
    }
}


// ---------------------------------------------------------------------------
// Name:        suEnableImagesDataLogging
// Description: Sets the textures data logging status.
// Arguments: isEnabled - true - texture data will be logged.
//                        false - textures data will not be logged.
// Return Val: void
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
void suEnableImagesDataLogging(bool isEnabled)
{
    su_stat_isImagesDataLoggingEnabled = isEnabled;
}


// ---------------------------------------------------------------------------
// Name:        suIsTexturesDataLoggingEnabled
// Description: Returns the textures data logging status.
// Return Val: bool  - true - texture data will be logged.
//                     false - textures data will not be logged.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
bool suIsTexturesDataLoggingEnabled()
{
    return su_stat_isImagesDataLoggingEnabled;
}


// ---------------------------------------------------------------------------
// Name:        suSetLoggedTexturesFileType
// Description: Sets the file type in which logged textures will be stored.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
void suSetLoggedTexturesFileType(apFileType fileType)
{
    su_stat_loggedTexturesFileType = fileType;
}


// ---------------------------------------------------------------------------
// Name:        suLoggedTexturesFileType
// Description: Returns the file type in which logged textures are be stored.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
apFileType suLoggedTexturesFileType()
{
    return su_stat_loggedTexturesFileType;
}


// ---------------------------------------------------------------------------
// Name:        suSetLoggingLimits
// Description: Set the maximal amount of items logged in:
//              * OpenGL context function calls log
//              * OpenCL context function calls log
//              * OpenCL command queue commands log
// Author:      Uri Shomroni
// Date:        21/2/2010
// ---------------------------------------------------------------------------
void suSetLoggingLimits(unsigned int maxOpenGLCallsPerContext, unsigned int maxOpenCLCallsPerContext, unsigned int maxOpenCLCommandPerQueue)
{
    su_stat_maxLoggedOpenGLCallsPerContext = maxOpenGLCallsPerContext;
    su_stat_maxLoggedOpenCLCallsPerContext = maxOpenCLCallsPerContext;
    su_stat_maxLoggedOpenCLCommandPerQueue = maxOpenCLCommandPerQueue;
}

// ---------------------------------------------------------------------------
// Name:        suMaxLoggedOpenGLCallsPerContext
// Description: Returns the maximal amount of calls to be logged by each OpenGL
//              context's calls logger.
// Author:      Uri Shomroni
// Date:        21/2/2010
// ---------------------------------------------------------------------------
unsigned int suMaxLoggedOpenGLCallsPerContext()
{
    return su_stat_maxLoggedOpenGLCallsPerContext;
}

// ---------------------------------------------------------------------------
// Name:        suMaxLoggedOpenCLCalls
// Description: Returns the maximal amount of calls to be logged by the OpenCL
//              monitor's calls logger.
// Author:      Uri Shomroni
// Date:        21/2/2010
// ---------------------------------------------------------------------------
unsigned int suMaxLoggedOpenCLCalls()
{
    return su_stat_maxLoggedOpenCLCallsPerContext;
}

// ---------------------------------------------------------------------------
// Name:        suMaxLoggedOpenCLCommandPerQueue
// Description: Returns the maximal amount of OpenCL commands to be logged for
//              each queue by the queues monitors.
// Author:      Uri Shomroni
// Date:        21/2/2010
// ---------------------------------------------------------------------------
unsigned int suMaxLoggedOpenCLCommandPerQueue()
{
    return su_stat_maxLoggedOpenCLCommandPerQueue;
}

// ---------------------------------------------------------------------------
// Name:        suGetGlobalServerEnvironmentSettings
// Description: Gets the environment settings
// Author:      Uri Shomroni
// Date:        16/10/2013
// ---------------------------------------------------------------------------
const suGlobalServerEnvironmentSettings& suGetGlobalServerEnvironmentSettings()
{
    return su_stat_globalServerEnvironmentSettings;
}

// ---------------------------------------------------------------------------
// Name:        suGetMutableGlobalServerEnvironmentSettings
// Description: Gets a mutable version of the environment settings, to allow
//              modification of their values.
// Author:      Uri Shomroni
// Date:        16/10/2013
// ---------------------------------------------------------------------------
suGlobalServerEnvironmentSettings& suGetMutableGlobalServerEnvironmentSettings()
{
    return su_stat_globalServerEnvironmentSettings;
}

// ---------------------------------------------------------------------------
// Name:        suDebuggerInstallDir
// Description: Returns the install directory of the debugger that debugs us.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
const osFilePath& suDebuggerInstallDir()
{
    if (su_stat_debuggerInstallDirectory == NULL)
    {
        osFilePath emptyPath;
        suSetDebuggerInstallDir(emptyPath);
    }

    return *su_stat_debuggerInstallDirectory;
}


// ---------------------------------------------------------------------------
// Name:        flushLogFileAfterEachFunctionCall
// Description: Marks if the calls history logger should flush the log file after each monitored function call
// Arguments: shouldFlush - true - will cause the log file to be flushed after each monitored function call.
//                          false - the log file will be flushed according to system decisions.
// Author:      Yaki Tebeka
// Date:        4/11/2009
// ---------------------------------------------------------------------------
void suFlushLogFileAfterEachFunctionCall(bool shouldFlush)
{
    su_stat_flushLogFileAfterEachFunctionCall = shouldFlush;
}


// ---------------------------------------------------------------------------
// Name:        suShouldFlushLogFileAfterEachFunctionCall
// Description:
//   Returns true iff the calls history logger should flush the
//   log file after each monitored function call.
// Author:      Yaki Tebeka
// Date:        4/11/2009
// ---------------------------------------------------------------------------
bool suShouldFlushLogFileAfterEachFunctionCall()
{
    return su_stat_flushLogFileAfterEachFunctionCall;
}


// ---------------------------------------------------------------------------
// Name:        suSetFrameTerminatorsMask
// Description: Sets the OpenGL frame terminators
// Arguments: frameTerminatorsMask - A mask of apFrameTerminators values.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
void suSetFrameTerminatorsMask(unsigned int frameTerminatorsMask)
{
    su_stat_frameTerminatorsMask = frameTerminatorsMask;
}


// ---------------------------------------------------------------------------
// Name:        suFrameTerminatorsMask
// Description: Returns the OpenGL frame terminators as a mask of apFrameTerminators values.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
unsigned int suFrameTerminatorsMask()
{
    return su_stat_frameTerminatorsMask;
}



// ---------------------------------------------------------------------------
// Name:        suSetSlowMotionDelayTimeUnits
// Description: Set slow motion delay value
// Arguments: unsigned int slowMotionDelay
// Return Val: void
// Author:      Sigal Algranaty
// Date:        29/12/2009
// ---------------------------------------------------------------------------
void suSetSlowMotionDelayTimeUnits(unsigned int slowMotionDelayTimeUnits)
{
    su_stat_slowMotionDelayTimeUnits = slowMotionDelayTimeUnits;
}


// ---------------------------------------------------------------------------
// Name:        suSlowMotionDelayTimeUnits
// Description: Return the slow motion delay time units
// Return Val: int
// Author:      Sigal Algranaty
// Date:        29/12/2009
// ---------------------------------------------------------------------------
int suSlowMotionDelayTimeUnits()
{
    return su_stat_slowMotionDelayTimeUnits;
}



// ---------------------------------------------------------------------------
// Name:        suStartHTMLLogFileRecording
// Description: Starts HTML log file recording mode
// Return Val: void
// Author:      Sigal Algranaty
// Date:        29/12/2009
// ---------------------------------------------------------------------------
void suStartHTMLLogFileRecording()
{
    su_stat_areHTMLLogFilesActive = true;
    su_stat_wereHTMLLogFilesActive = true;

}

// ---------------------------------------------------------------------------
// Name:        suStopHTMLLogFileRecording
// Description: Stops HTML log file recording mode
// Return Val: void
// Author:      Sigal Algranaty
// Date:        29/12/2009
// ---------------------------------------------------------------------------
void suStopHTMLLogFileRecording()
{
    su_stat_areHTMLLogFilesActive = false;
}

// ---------------------------------------------------------------------------
// Name:        suAreHTMLLogFilesActive
// Description: Are HTML files recording active?
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        29/12/2009
// ---------------------------------------------------------------------------
bool suAreHTMLLogFilesActive()
{
    return su_stat_areHTMLLogFilesActive;
}

// ---------------------------------------------------------------------------
// Name:        suWereHTMLLogFilesActive
// Description: Were HTML log files recording active?
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        29/12/2009
// ---------------------------------------------------------------------------
bool suWereHTMLLogFilesActive()
{
    return su_stat_wereHTMLLogFilesActive;
}


// ---------------------------------------------------------------------------
// Name:        suTerminateGlobalVariables
// Description: Terminates all global variables that aren't automatically destructed.
// Author:      Uri Shomroni
// Date:        21/12/2009
// ---------------------------------------------------------------------------
void suTerminateGlobalVariables()
{
    delete su_stat_SessionlogFilesDirectoryPath;
    su_stat_SessionlogFilesDirectoryPath = NULL;

    delete su_stat_ProjectlogFilesDirectoryPath;
    su_stat_ProjectlogFilesDirectoryPath = NULL;

    delete su_stat_debuggerInstallDirectory;
    su_stat_debuggerInstallDirectory = NULL;
}

