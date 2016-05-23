//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdLinuxProcessDebugger.cpp
///
//==================================================================================

//------------------------------ pdLinuxProcessDebugger.cpp ------------------------------

// POSIX:
#include <signal.h>

// ABI (for function name demangling)
#include <cxxabi.h>

// STL
#include <thread>
#include <chrono>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osBundle.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunResumedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunStartedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreationFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <src/pdStringConstants.h>
#include <src/pdGDBDataStructs.h>
#include <src/pdLinuxProcessDebugger.h>
#include <src/pdLauncherProcessWatcherThread.h>
#include <src/pdDebuggedProcessWatcherThread.h>

// Represents an unknown index:
#define PD_UNKNOWN_INDEX -1

// Contains the name of the AMD OpenGL driver path. This is appended to the end of the
// library path environment variable to allow the fglrx_dri to get the correct dispatch
// table. See BUG404491.
static const gtString s_libglDriversPathEnvVariableName = L"LIBGL_DRIVERS_PATH";

// Contains he loader "pre-load libraries" environment variable name:
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT

    // The "library path" env variable name:
    static const gtString s_ldLibraryPathEnvVariableName = L"LD_LIBRARY_PATH";

    // The "framework path" env variable name (unused in Linux):
    static const gtString s_dyldFrameworkPathEnvVariableName;

    // The "preload" env variable name:
    static const gtString s_ldPreloadEnvVariableName = L"LD_PRELOAD";

    #define PD_LINUX_GDB_PATH L"gdb"

#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT

    // The "library path" env variable name:
    static const gtString s_ldLibraryPathEnvVariableName = L"DYLD_LIBRARY_PATH";

    // The "framework path" env variable name:
    static const gtString s_dyldFrameworkPathEnvVariableName = L"DYLD_FRAMEWORK_PATH";

    // The "preload" env variable name:
    static const gtString s_ldPreloadEnvVariableName = L"DYLD_INSERT_LIBRARIES";

    // The "force flat namespace" env variable name:
    static const gtString s_forceFlatNamespaceEnvVariableName = L"DYLD_FORCE_FLAT_NAMESPACE";

    // We need the name of the GS_STDOUT_REDIRECT and GS_OPENGL_ES_FRAMEWORK_PATH environment variables in Mac:
    #include <GROpenGLServers/gsPublicStringConstants.h>

    // For debugging iPhone apps:
    #include <src/pdIPhoneDebuggingFunctions.h>
    #include <AMDTOSWrappers/Include/osApplication.h>
    #include <AMDTOSWrappers/Include/osMessageBox.h>

    #define PD_MAC_GDB_PATH L"/Developer/usr/libexec/gdb/gdb-i386-apple-darwin"

    #define PD_IPHONE_SIMULATOR_GDB_PATH L"/Developer/Platforms/iPhoneSimulator.platform/Developer/usr/libexec/gdb/gdb-i386-apple-darwin"

#else
    #error Error: Unknown linux variant
#endif

static const gtASCIIString emptyCmdParam;


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::pdLinuxProcessDebugger
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        20/12/2006
// ---------------------------------------------------------------------------
pdLinuxProcessDebugger::pdLinuxProcessDebugger()
    : pdProcessDebugger(), _pDebuggedProcessThreadsData(NULL), _isUnderHostBreakpoint(false), _pLauncherProcessWatcherThread(NULL),
      _debuggedExecutableArchitecture(OS_UNKNOWN_ARCHITECTURE), _pWatcherThread(NULL), m_triggeringThreadId(OS_NO_THREAD_ID),
      m_hostBreakReason(AP_FOREIGN_BREAK_HIT), m_lastStepKind(AP_FOREIGN_BREAK_HIT),
      _currentGDBState(gdb_state::gdb_not_initialized_state)
{
    initialize();
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::~pdLinuxProcessDebugger
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        20/12/2006
// ---------------------------------------------------------------------------
pdLinuxProcessDebugger::~pdLinuxProcessDebugger()
{
    OS_OUTPUT_DEBUG_LOG(L"Starting ~pdLinuxProcessDebugger", OS_DEBUG_LOG_DEBUG);

    // If a debugged process exists - terminate it:
    if (debuggedProcessExists())
    {
        terminateDebuggedProcess();
    }

    // Terminate the process debugger:
    bool rc1 = _gdbDriver.terminate();
    GT_ASSERT(rc1);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting pdGDBThreadDataList", OS_DEBUG_LOG_DEBUG);
    // Allocated data cleanup:
    delete _pDebuggedProcessThreadsData;
    _pDebuggedProcessThreadsData = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting pdGDBThreadDataList", OS_DEBUG_LOG_DEBUG);

    clearCallStacksMap();

    OS_OUTPUT_DEBUG_LOG(L"Before deleting pdLauncherProcessWatcherThread", OS_DEBUG_LOG_DEBUG);
    delete _pLauncherProcessWatcherThread;
    _pLauncherProcessWatcherThread = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting pdLauncherProcessWatcherThread", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting pdDebuggedProcessWatcherThread", OS_DEBUG_LOG_DEBUG);
    delete _pWatcherThread;
    _pWatcherThread = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting pdDebuggedProcessWatcherThread", OS_DEBUG_LOG_DEBUG);


    OS_OUTPUT_DEBUG_LOG(L"Ending ~pdLinuxProcessDebugger", OS_DEBUG_LOG_DEBUG);
}

////////////////////////////////////////////////////////////////////////////
/// \brief Do host debugger (gdb, VS, etc..) initialization prerequestics
///
/// \param processCreationData a data needed for the process debugger creation and initailization
/// \return true - success, false - failed
/// \author Vadim Entov
/// \date 21/01/2016
bool pdLinuxProcessDebugger::initializeDebugger(const apDebugProjectSettings& processCreationData)
{
    bool retVal = false;

    bool reportedFailure = false;

    if (gdb_state::gdb_not_initialized_state == _currentGDBState)
    {
        // Initialize gdb:
        bool rcGDBInit = launchGDB(processCreationData);
        GT_IF_WITH_ASSERT(rcGDBInit)
        {
            // Store the debugged process creation data:
            _debuggedProcessCreationData = processCreationData;

            // Verify that the executable exists on disk:
            const osFilePath& executablePath = processCreationData.executablePath();

            if (executablePath.isExecutable())
            {
                bool rcArch = setDebuggedProcessArchitecture();
                GT_ASSERT(rcArch);

                gtString executablePathAsString = executablePath.asString();

                // Handle Mac OS X *.app application bundles:
#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
                {
                    if (!executablePath.isRegularFile())
                    {
                        executablePathAsString = osGetExecutableFromMacApplicationBundle(executablePathAsString);
                    }
                }
#endif

                // Add quotation marks around the path to make sure that names containing spaces are treated correctly:
                executablePathAsString.append('\"').prepend('\"');

#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
                {
                    // Linux does not support OpenGL ES:
                    if (processCreationData.projectExecutionTarget() == AP_IPHONE_SIMULATOR_EXECUTION_TARGET)
                    {
                        bool rcModule = loadProcessDebuggerESLauncher();

                        // Do not assert - machines with no iPhoneSimulatorRemoteClient are supposed to fail this:
                        if (rcModule)
                        {
                            // On the iPhone, since we attach to the debugged process and not run it, we need to redirect
                            // the stdout pipe manually (the --tty option doesn't work). Create an environment variable
                            // that will contain the spies directory:
                            osEnvironmentVariable stdoutRedirectEnvVariable;
                            stdoutRedirectEnvVariable._name = OS_STR_envVar_stdoutRedirectionFile;
                            _gdbDriver.getOutputReaderThreadFileName(stdoutRedirectEnvVariable._value);
                            _debuggedProcessCreationData.addEnvironmentVariable(stdoutRedirectEnvVariable);

                            // Also send the OpenGL ES framework path
                            osEnvironmentVariable openglesFrameworkPathEnvVariable;
                            openglesFrameworkPathEnvVariable._name = GS_STR_envVar_openglesFrameworkPath;
                            openglesFrameworkPathEnvVariable._value = osGetOpenGLESFrameworkPath();
                            _debuggedProcessCreationData.addEnvironmentVariable(openglesFrameworkPathEnvVariable);

                            // Make GDB wait for the debugged application:
                            gtString debuggedAppName;
                            executablePath.getFileName(debuggedAppName);
                            debuggedAppName.append('\"').prepend('\"');
                            std::string utf8debuggedAppName;
                            debuggedAppName.asUtf8(utf8debuggedAppName);
                            bool rcWait = _gdbDriver.executeGDBCommand(PD_GDB_WAIT_FOR_PROCESS_CMD, utf8debuggedAppName.c_str());
                            GT_IF_WITH_ASSERT(rcWait)
                            {
                                retVal = launchApplicationWithiPhoneSimulator(executablePath, _debuggedProcessCreationData);
                            }
                        }
                        else
                        {
                            // Display a message to the user:
                            osMessageBox msgBox("", PD_STR_iPhoneSimulatorIsNotInstalled, osMessageBox::OS_STOP_SIGN_ICON);
                            msgBox.display();

                            // Terminate the gdb driver:
                            _gdbDriver.terminate();
                        }

                        return retVal;
                    }
                }
#endif

                // Set GDB's current debugged executable to be the input executable path:
                std::string utf8debuggedAppName;
                executablePathAsString.asUtf8(utf8debuggedAppName);

                bool rc3 = _gdbDriver.executeGDBCommand(PD_GDB_SET_DEBUGGED_PROGRAM_CMD, utf8debuggedAppName.c_str());
                GT_IF_WITH_ASSERT(rc3)
                {
                    // Set the debugged process command line arguments:
                    bool rc4 = setDebuggedProcessCommandLineArguments();
                    GT_IF_WITH_ASSERT(rc4)
                    {
                        // start the thread of the output reader:
                        if (NULL != _gdbDriver.getOutputReaderThread())
                        {
                            _gdbDriver.getOutputReaderThread()->execute();
                        }

                        // Set the debugged process working directory:
                        bool rc5 = setDebuggedProcessWorkingDirectory();
                        GT_IF_WITH_ASSERT(rc5)
                        {
                            // Set the debugged process environment variables:
                            bool rc6 = setDebuggedProcessEnvVariables();
                            GT_IF_WITH_ASSERT(rc6)
                            {
                                _currentGDBState = gdb_state::gdb_initialized_state;
                                retVal = true;
                            }
                        }
                        else
                        {
                            // Trigger "process terminated" event:
                            apDebuggedProcessTerminatedEvent processTerminatedEvent(-1);
                            apEventsHandler::instance().registerPendingDebugEvent(processTerminatedEvent);
                            reportedFailure = true;
                        }
                    }
                }
            }
        }

        // If we failed and did not report it:
        if (!retVal && !reportedFailure)
        {
            // Send a process creation failure event:
            gtString commandLine = processCreationData.executablePath().asString();
            const gtString& commandArgs = processCreationData.commandLineArguments();

            if (!commandArgs.isEmpty())
            {
                commandLine.append(' ').append(commandArgs);
            }

            apDebuggedProcessCreationFailureEvent failureEve(apDebuggedProcessCreationFailureEvent::COULD_NOT_CREATE_PROCESS,
                                                             commandLine, processCreationData.workDirectory().asString(), L"Failed to initialize process");
            apEventsHandler::instance().registerPendingDebugEvent(failureEve);
        }
    }
    else
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::launchDebuggedProcess
// Description: Launches a process for debugging.
// Arguments:   processCreationData - Data needed for the process creation
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        20/12/2006
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::launchDebuggedProcess()
{
    bool retVal = false;

    bool reportedFailure = false;

    if (_currentGDBState == gdb_state::gdb_initialized_state)
    {
        // The process creation time:
        osTime processCreationTime;
        processCreationTime.setFromCurrentTime();

        // Trigger "process created" event:
        apDebuggedProcessCreatedEvent processCreatedEvent(_debuggedProcessCreationData, processCreationTime, NULL);
        apEventsHandler::instance().registerPendingDebugEvent(processCreatedEvent);

        // Run the executable until it reaches the main function:
        bool rc7 = _gdbDriver.executeGDBCommand(PD_GDB_RUN_DEBUGGED_PROCESS_CMD, "");
        GT_IF_WITH_ASSERT(rc7)
        {
            retVal = true;
        }
        else
        {
            // Trigger "process terminated" event:
            apDebuggedProcessTerminatedEvent processTerminatedEvent(-1);
            apEventsHandler::instance().registerPendingDebugEvent(processTerminatedEvent);
            reportedFailure = true;
        }
    }

    // If we failed and did not report it:
    if (!retVal && !reportedFailure)
    {
        // Send a process creation failure event:
        gtString commandLine = _debuggedProcessCreationData.executablePath().asString();
        const gtString& commandArgs = _debuggedProcessCreationData.commandLineArguments();

        if (!commandArgs.isEmpty())
        {
            commandLine.append(' ').append(commandArgs);
        }

        apDebuggedProcessCreationFailureEvent failureEve(apDebuggedProcessCreationFailureEvent::COULD_NOT_CREATE_PROCESS,
                                                         commandLine, _debuggedProcessCreationData.workDirectory().asString(), L"Failed to launch process");
        apEventsHandler::instance().registerPendingDebugEvent(failureEve);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::debuggedProcessExists
// Description: Returns true iff there is a launched debugged process.
// Author:      Yaki Tebeka
// Date:        20/12/2006
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::debuggedProcessExists() const
{
    return _debuggedProcessExists;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::debuggedProcessCreationData
// Description: Returns the debugged process creation data.
// Author:      Yaki Tebeka
// Date:        16/11/2003
// ---------------------------------------------------------------------------
const apDebugProjectSettings* pdLinuxProcessDebugger::debuggedProcessCreationData() const
{
    return &_debuggedProcessCreationData;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::terminateDebuggedProcess
// Description: Terminates the debugged process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/11/2003
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::terminateDebuggedProcess()
{
    bool retVal = false;

    if (::kill(_debuggedProcessPid, 0) == 0)
    {
        // Ask GDB to kill the debugged process:
        bool bSuspendedBefore = false;
        GT_IF_WITH_ASSERT(trySuspendProcess(bSuspendedBefore))
        {
            bool rc1 = _gdbDriver.executeGDBCommand(PD_GDB_ABORT_DEBUGGED_PROCESS_CMD, emptyCmdParam);
            _gdbDriver.StartBackgoundGDBListen();
            GT_IF_WITH_ASSERT(rc1)
            {
                retVal = true;
            }
        }
    }
    else
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::isDebugging64BitApplication
// Description: Query whether the debugged application is a 64-bit application.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        21/9/2009
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::isDebugging64BitApplication(bool& is64Bit) const
{
    bool retVal = true;

    is64Bit = _isDebugging64BitApplication;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::amountOfDebuggedProcessThreads
// Description: Returns the amount of debugged process threads.
// Author:      Yaki Tebeka
// Date:        8/5/2005
// ---------------------------------------------------------------------------
int pdLinuxProcessDebugger::amountOfDebuggedProcessThreads() const
{
    int retVal = 0;

    // Validity check:
    if (_pDebuggedProcessThreadsData != NULL)
    {
        retVal = _pDebuggedProcessThreadsData->_threadsDataList.length();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::getThreadId
// Description: Inputs a thread index and returns its id.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        8/5/2005
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::getThreadId(int threadIndex, osThreadId& threadId) const
{
    bool retVal = false;
    threadId = OS_NO_THREAD_ID;

    // Validity check:
    if (_pDebuggedProcessThreadsData != NULL)
    {
        // Iterate the debugged process threads:
        int currIndex = 0;
        gtList<pdGDBThreadData>::const_iterator iter = _pDebuggedProcessThreadsData->_threadsDataList.begin();
        gtList<pdGDBThreadData>::const_iterator endIter = _pDebuggedProcessThreadsData->_threadsDataList.end();

        while (iter != endIter)
        {
            // If the current thread is the thread we are looking for:
            if (currIndex == threadIndex)
            {
                threadId = (*iter)._OSThreadId;
                retVal = true;
                break;
            }

            currIndex++;
            iter++;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::setSpiesAPIThreadId
// Description:
//   Sets the Spy API thread id.
//   (The Spy API thread is created by the Spy dll and serves API function calls)
// Arguments:   spiesAPIThreadId - The ID of the spies API thread.
// Author:      Yaki Tebeka
// Date:        8/6/2004
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::setSpiesAPIThreadId(osThreadId spiesAPIThreadId)
{
    GT_IF_WITH_ASSERT(spiesAPIThreadId != OS_NO_THREAD_ID)
    {
        // Translate the thread id to a string:
        gtString threadIdAsString;
        osThreadIdAsString(spiesAPIThreadId, threadIdAsString);

        // Output debug log message:
        gtString dbgLogMsg = PD_STR_gotOGLServerAPIThreadId;
        dbgLogMsg += L": ";
        dbgLogMsg += threadIdAsString;
        OS_OUTPUT_DEBUG_LOG(dbgLogMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

        // Log the API thread ID:
        _spiesAPIThreadId = spiesAPIThreadId;

        // If the API thread was already registered as running by gdb:
        int APIThreadIndex = spiesAPIThreadIndex();

        if (APIThreadIndex != -1)
        {
            // Mark that the API thread is running:
            _isAPIThreadRunning = true;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::suspendDebuggedProcessThread
// Description: Suspends a debugged process thread.
// Arguments:   threadId - The input thread id.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/6/2004
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::suspendDebuggedProcessThread(osThreadId threadId)
{
    bool retVal = false;

#if (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    gtString threadIndexAsString;
    int threadIndex = getGDBThreadId(threadId);
    threadIndexAsString.appendFormattedString(L"%d", threadIndex);
    retVal = _gdbDriver.executeGDBCommand(PD_SUSPEND_THREAD_CMD, threadIndexAsString);
#elif (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    // On Linux POSIX, there is no command to suspend / resume the run of
    // a single thread.
    (void)(threadId); // unused
    GT_ASSERT(false);
#else
#error Unknown Linux Variant!
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::resumeDebuggedProcessThread
// Description: Resumes a debugged process thread.
// Arguments:   threadId - The input thread id.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/6/2004
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::resumeDebuggedProcessThread(osThreadId threadId)
{
    bool retVal = false;

#if (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    gtString threadIndexAsString;
    int threadIndex = getGDBThreadId(threadId);
    threadIndexAsString.appendFormattedString(L"%d", threadIndex);
    retVal = _gdbDriver.executeGDBCommand(PD_RESUME_THREAD_CMD, threadIndexAsString);
#elif (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    // On Linux POSIX, there is no command to suspend / resume the run of
    // a single thread.
    (void)(threadId); // unused
    GT_ASSERT(false);
#else
#error Unknown Linux Variant!
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::spiesAPIThreadIndex
// Description: Returns the spies API thread index (see setSpiesAPIThreadId).
// Author:      Yaki Tebeka
// Date:        11/9/2005
// ---------------------------------------------------------------------------
int pdLinuxProcessDebugger::spiesAPIThreadIndex() const
{
    int retVal = -1;
    int currentIndex = 0;

    // Validity check:
    if ((_pDebuggedProcessThreadsData != NULL) && (_spiesAPIThreadId != OS_NO_THREAD_ID))
    {
        // Iterate the debugged process threads:
        gtList<pdGDBThreadData>::const_iterator iter = _pDebuggedProcessThreadsData->_threadsDataList.begin();
        gtList<pdGDBThreadData>::const_iterator endIter = _pDebuggedProcessThreadsData->_threadsDataList.end();

        while (iter != endIter)
        {
            // If the current thread is the Spy API thread:
            if ((*iter)._OSThreadId == _spiesAPIThreadId)
            {
                retVal = currentIndex;
                break;
            }

            currentIndex++;
            iter++;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::mainThreadId
// Description: Returns the debugged application main thread id, or OS_NO_THREAD_ID
//              if it does not exist.
// Author:      Yaki Tebeka
// Date:        11/9/2005
// ---------------------------------------------------------------------------
osThreadId pdLinuxProcessDebugger::mainThreadId() const
{
    osThreadId retVal = OS_NO_THREAD_ID;

    // GDB thread #1 should be the main application thread:
    gtList<pdGDBThreadData>::const_iterator iter = _pDebuggedProcessThreadsData->_threadsDataList.begin();
    gtList<pdGDBThreadData>::const_iterator endIter = _pDebuggedProcessThreadsData->_threadsDataList.end();

    while (iter != endIter)
    {
        // If the current thread is the main thread id (GDB thread #1):
        if ((*iter)._gdbThreadId == 1)
        {
            retVal = (*iter)._OSThreadId;
            break;
        }

        iter++;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::spiesAPIThreadId
// Description:
//   Returns the spies API thread Id, or 0 is the spy api thread id is not
//   known (see comment at setSpiesAPIThreadId).
// Author:      Yaki Tebeka
// Date:        8/6/2004
// ---------------------------------------------------------------------------
osThreadId pdLinuxProcessDebugger::spiesAPIThreadId() const
{
    return _spiesAPIThreadId;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::debuggedProcessId
// Description: Returns the debugged process ID.
// Author:      Uri Shomroni
// Date:        16/9/2010
// ---------------------------------------------------------------------------
osProcessId pdLinuxProcessDebugger::debuggedProcessId() const
{
    // TO_DO: Uri, 16/9/10 - We would need to get this when creating the debugged
    // app or maintain it from the process creation event.
    return 0;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::isSpiesAPIThreadRunning
// Description: Returns true iff the spies API thread is running.
// Author:      Yaki Tebeka
// Date:        5/7/2004
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::isSpiesAPIThreadRunning() const
{
    // In Linux and Mac, threads created in module constructors start running
    // only when the main function starts running. So the thread being created
    // (_spiesAPIThreadId != OS_NO_THREAD_ID) is not enough. We need to see if
    // it STARTED RUNNING. If it has an index in gdb, it is enough. However,
    // when running (not suspended), we do not have thread indices, so we need
    // to hold a member for when this is true:
    bool retVal = _isAPIThreadRunning;

    if (_pDebuggedProcessThreadsData != NULL)
    {
        retVal = (spiesAPIThreadIndex() > -1);
    }

    // However, if the was destroyed, we need to mark it as not running:
    retVal = retVal && (_spiesAPIThreadId != OS_NO_THREAD_ID);

    // If we got a value we did not expect:
    if (retVal != _isAPIThreadRunning)
    {
        // Cast to non-const to note the change:
        ((pdLinuxProcessDebugger*)this)->_isAPIThreadRunning = retVal;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::suspendDebuggedProcess
// Description:
//   Suspends all the debugged process threads, except the Spy
//   API handling thread.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        8/6/2004
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::suspendDebuggedProcess()
{
    // Suspend the debugged process run:
    bool retVal = false;

    // This function is not used on LINUX:
    GT_ASSERT(false);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::resumeDebuggedProcess
// Description: Resumes the run of a suspended debugged process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        20/5/2004
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::resumeDebuggedProcess()
{
    bool retVal = false;

    // Mark the debugged process run as resumed (not suspended):
    _isDuringInternalContinue = false;
    bool wasProcessSuspended = _isDebuggedProcssSuspended;
    _isDebuggedProcssSuspended = false;

#if (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    {
        // Mark that we are resuming threads:
        _isDuringGDBSynchronusCommandExecution = true;

        bool canResumeThreads = false;
        bool canResumeProcess = true;

        // Pressing "stop" while suspended gets us here after a fatal (SIGKILL) signal:
        if (!_isDuringFatalSignalSuspension)
        {
            // In Mac, we resume all threads except the API thread (which we didn't suspend), as we do in Windows
            // (note that since this is possible, we don't need to lock the condition inside
            // the OpenGL function calls, so we don't)

            // Tell the gdb driver that we are about to send an internal interrupt (SIGINT):
            _gdbDriver.waitForInternalDebuggedProcessInterrupt();
            _isDuringInternalContinue = true;

            // Suspend the debugged process run by sending it an interrupt (SIGINT):
            int rc1 = ::kill(_debuggedProcessPid, SIGINT);
            GT_IF_WITH_ASSERT(rc1 == 0)
            {
                // Even if we can't perform synchronous commands, we can resume the app:
                canResumeProcess = true;

                // Wait until the gdb listener thread notifies us that the debugged process was suspended:
                bool rc2 = waitForDebuggedProcessSuspensionCondition();
                GT_IF_WITH_ASSERT(rc2)
                {
                    canResumeThreads = true;
                }
            }
        }
        else // _isDuringFatalSignalSuspension
        {
            // GDB is already suspended, we can resume the process (but not the threads):
            canResumeProcess = true;
            canResumeThreads = false;
        }

        GT_IF_WITH_ASSERT(canResumeProcess)
        {
            retVal = true;

            if (canResumeThreads)
            {
                int numOfThreads = amountOfDebuggedProcessThreads();
                osThreadId spyAPIThreadId = spiesAPIThreadId();

                for (int i = 0; i < numOfThreads; i++)
                {
                    osThreadId threadId = OS_NO_THREAD_ID;
                    bool rcID = getThreadId(i, threadId);
                    GT_IF_WITH_ASSERT(rcID)
                    {
                        // Ignore the Spies API thread:
                        if ((threadId != spyAPIThreadId) && !(isDriverThread(threadId)))
                        {
                            retVal = resumeDebuggedProcessThread(threadId) && retVal;
                        }
                    }
                }

                GT_ASSERT(retVal);
            }

            gtString emptyString;

            // Mark we are done with resuming threads:
            _isDuringGDBSynchronusCommandExecution = false;
            _isDuringInternalContinue = false;

            _gdbDriver.executeGDBCommand(PD_CONTINUE_CMD, emptyString);
        }
        else
        {
            // Clear the flags anyway:
            _isDuringGDBSynchronusCommandExecution = false;
            _isDuringInternalContinue = false;
        }
    }
#elif (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    {
        // In Linux, the debugged process is currently resumed by the API (thought the Spy API thread).
        // So - we assume that the debugged process run will be resumed right after the call to this function.
        retVal = true;
    }
#else
#error Unknown Linux variant!
#endif

    // If we are during fatal exception suspension:
    if (_isDuringFatalSignalSuspension)
    {
        // Ask GDB to kill the debugged process:
        bool rc1 = _gdbDriver.executeGDBCommand(PD_GDB_ABORT_DEBUGGED_PROCESS_CMD, emptyCmdParam);
        GT_IF_WITH_ASSERT(rc1)
        {
            retVal = true;
        }
    }
    else // !_isDuringFatalSignalSuspension
    {
        if (_isUnderHostBreakpoint)
        {
            gaUnLockDriverThreads();

            retVal = tryResumeProcess();
        }

        // Notify the GDB driver:
        GT_IF_WITH_ASSERT(retVal)
        {
            _gdbDriver.onDebuggedProcessRunResumed();

            if (wasProcessSuspended)
            {
                // Notify observers that the debugged process run is resumed:
                apDebuggedProcessRunResumedEvent processResumedEvent;
                apEventsHandler::instance().registerPendingDebugEvent(processResumedEvent);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::isDebuggedProcssSuspended
// Description:
//    Returns true iff the below two conditions are met:
//    a. The debugged process exists.
//    b. Its run is suspended.
//
//    A  debugged process can be suspended by hitting a breakpoint / etc.
//
// Author:      Yaki Tebeka
// Date:        20/5/2004
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::isDebuggedProcssSuspended()
{
    bool retVal = _isDebuggedProcssSuspended;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::onEvent
// Description: Called when a debug event is reported.
// Author:      Uri Shomroni
// Date:        20/4/2009
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::handleDebugEvent
// Description: Is called by the main application thread to handle events
//              For more details - see "Notification mechanism overview" at the
//              top of pdProcessDebugger.cpp.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        22/6/2004
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::onEvent(const apEvent& eve, bool& vetoEvent)
{
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {
        // Before the event was handled by the debugged application:
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            onDebuggedProcessCreationEvent();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
        {
            onProcessRunStartedEvent(eve);
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            // If we didn't decide to ignore this event already:
            if (!vetoEvent)
            {
                onProcessRunSuspendedEvent(eve);

                // Fix case 47: We sometimes get a process suspended event, but the process run
                // was already resumed:
                if (!isDebuggedProcssSuspended())
                {
                    vetoEvent = true;
                }
            }
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        {
            // If the process run was resumed during our internal continue process:
            if ((_isDuringInternalContinue) && (!(_isDuringFunctionExecution || _isDuringGDBSynchronusCommandExecution)))
            {
                // Veto the event (do not pass it to event observers)
                vetoEvent = true;
            }
            else
            {
                onProcessRunResumedEvent();
            }
        }
        break;

        case apEvent::AP_BREAKPOINT_HIT:
        {
            onBreakpointHitEvent(eve);
        }
        break;

        // After the event was handled by the application:
        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            // If we already knew the debugged process is dead, perform the initialization
            // but don't report it to other parts of the app (or the debugged process events
            // view shows two "process terminated" lines, etc.):
            vetoEvent = !_debuggedProcessExists;
            onDebuggedProcessTerminationEvent();
        }
        break;

        default:
        {
            // An event that we currently don't handle.
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::onEventRegistration
// Description: Called before a debug event is registered by the apEventsHandler
// Author:      Uri Shomroni
// Date:        21/4/2009
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::registerPendingDebugEvent
// Description:
//   Is called by any thread to registered an event to be handled by the main application thread.
// Arguments:   eve - A class holding information about the debugged process event.
// Author:      Yaki Tebeka
// Date:        23/1/2007
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::onEventRegistration(apEvent& eve, bool& vetoEvent)
{
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // If we got the breakpoint for starting or ending kernel debugging, do not pass it on
    // to the application, since it is an internal-only breakpoint used for synchronization:
    bool isInternalBreakpoint = false;

    if (eventType == apEvent::AP_BREAKPOINT_HIT)
    {
        const apBreakpointHitEvent& bpEve = (const apBreakpointHitEvent&)eve;
        apBreakReason breakReason = bpEve.breakReason();
        m_triggeringThreadId = eve.triggeringThreadId();

        // Assume a breakpoint is not a host breakpoint:
        _isUnderHostBreakpoint = (AP_HOST_BREAKPOINT_HIT == breakReason || AP_BREAK_COMMAND_HIT == breakReason);

        if ((breakReason == AP_BEFORE_KERNEL_DEBUGGING_HIT) || (breakReason == AP_AFTER_KERNEL_DEBUGGING_HIT))
        {
            // Continue the debugged process:
            bool rc = _gdbDriver.executeGDBCommand(PD_CONTINUE_CMD, " --all");
            GT_ASSERT(rc);

            // Ignore this event from this point:
            isInternalBreakpoint = true;
            vetoEvent = true;
        }
        else if (AP_STEP_OVER_BREAKPOINT_HIT == breakReason)
        {
            // If this was a step event, check if it's a host step:
            if (AP_FOREIGN_BREAK_HIT != m_lastStepKind)
            {
                // Change its type accordingly and clear the member:
                apBreakpointHitEvent& mutableBPEve = (apBreakpointHitEvent&)bpEve;
                mutableBPEve.setBreakReason(m_lastStepKind);
                m_lastStepKind = AP_FOREIGN_BREAK_HIT;

                // Host steps are host breakpoints:
                _isUnderHostBreakpoint = true;
            }
        }

        // Get the (possibly updated) value of the host break reason:
        m_hostBreakReason = (_isUnderHostBreakpoint ? bpEve.breakReason() : AP_FOREIGN_BREAK_HIT);
    }
    else if (eventType == apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED)
    {
        m_hostBreakReason = AP_FOREIGN_BREAK_HIT;
    }

    // If we are during function execution:
    // ===================================
    bool isExecutingSynchronusCommand = (_isDuringFunctionExecution || _isDuringGDBSynchronusCommandExecution);

    if (isExecutingSynchronusCommand)
    {
        // Only a few types of events are expected when executing synch. commands
        switch (eventType)
        {
            case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
            case apEvent::AP_GDB_OUTPUT_STRING:
            {
                vetoEvent = true;
            }
            break;

            case apEvent::AP_EXCEPTION:
            {
                // If this is a SIGINT exception (generated by us to suspend the debugged process run):
                osExceptionReason exceptionReason = ((const apExceptionEvent&)eve).exceptionReason();

                if (exceptionReason == OS_SIGINT_SIGNAL || exceptionReason == OS_STANDALONE_THREAD_STOPPED)
                {
                    // Veto it:
                    vetoEvent = true;
                }
                else
                {
                    // We got an unexpected exception:
                    gtString errorMessage = PD_STR_exceptionDuringFunctionExecution;
                    errorMessage.appendFormattedString(L"%d", exceptionReason);
                    GT_ASSERT_EX(false, errorMessage.asCharArray());
                }
            }
            break;

            case apEvent::AP_GDB_LISTENER_THREAD_WAS_SUSPENDED_EVENT:
            {
                // The gdb listener thread was suspended after reading all gdb messages.
                // Release the main application thread:
                //bool rc1 = releaseDebuggedProcessSuspensionCondition();
                //GT_ASSERT(rc1);
                vetoEvent = true;
            }
            break;

            case apEvent::AP_SPY_PROGRESS_EVENT:
            {
                // This event is expected during function execution, do nothing!
            }
            break;

            case apEvent::AP_BREAKPOINTS_UPDATED_EVENT:
            {
            }
            break;

            case apEvent::AP_GDB_ERROR:
            {
                // We got an unexpected error:
                gtString errorMessage = PD_STR_gdbErrorDuringFunctionExecution;
                GT_ASSERT_EX(false, errorMessage.asCharArray());
            }
            break;

            default:
            {
                // We should not get other event types during function execution:
                gtString errorMessage = PD_STR_unexpectedEventDuringFunctionExecution;
                errorMessage.appendFormattedString(L"%d", eventType);
                GT_ASSERT_EX(false, errorMessage.asCharArray());
            }
            break;
        }
    }
    else
    {
        if ((eventType == apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED) ||
            ((eventType == apEvent::AP_BREAKPOINT_HIT) && (!isInternalBreakpoint)))
        {
            onProcessRunSuspendedEventRegistration(eve);
        }
        else if (eventType == apEvent::AP_THREAD_CREATED)
        {
            onThreadCreatedEventRegistration((apThreadCreatedEvent&)eve, vetoEvent);
        }
    }

    // This needs to be done here and not in ::onEvent, since we need to hit the "resume" (if needed)
    // as soon as possible, else it stops main thread operations (e.g. performance counter value reading),
    // which in turn prevent the event from getting to the event notification, causing a hang.
    if (eventType == apEvent::AP_EXCEPTION)
    {
        // If we haven't decided to ignore this event already:
        if (!vetoEvent)
        {
            onExceptionEventRegistration((const apExceptionEvent&)eve);
        }
    }

    // These two events need to be handled at registartion time, since we need them here so we
    // can ignore the breakpoints that immediately follow them:
    if (eventType == apEvent::AP_BEFORE_KERNEL_DEBUGGING_EVENT)
    {
        // Kernel debugging is about to start, mark this:
        _gdbDriver.kernelDebuggingAboutToStart();
        _isDuringKernelDebugging = true;
    }
    else if (eventType == apEvent::AP_AFTER_KERNEL_DEBUGGING_EVENT)
    {
        // Kernel debugging just ended, mark this:
        _isDuringKernelDebugging = false;
        _gdbDriver.kernelDebuggingJustFinished();
    }
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::fillThreadCreatedEvent
// Description:
//  Does nothing, since on Linux, apThreadCreatedEvent objects are already
//  filled when they are reported.
// Author:      Yaki Tebeka
// Date:        9/5/2005
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::fillThreadCreatedEvent(apThreadCreatedEvent& eve)
{
    (void)(eve); // unused
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::canGetCallStacks
// Description: Query whether this process debugger can get debugged process
//              calls stacks by itself (without the API
// Author:      Uri Shomroni
// Date:        25/1/2010
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::canGetCallStacks()
{
    // The Linux process debugger controls gdb, so it CAN make threads calls stacks:
    return true;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::canMakeThreadExecuteFunction
// Description: Returns whether this process debugger implementation supports
//              the "make thread execute function" mechanism.
// Author:      Uri Shomroni
// Date:        2/11/2009
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::canMakeThreadExecuteFunction(const osThreadId& threadId)
{
    (void)(threadId); // unused
    // Check if target thread or someone other thread not into the openGl driver thread

    return _canMakeExecuteFunction;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::makeThreadExecuteFunction
// Description:
//  Inputs a suspended thread id and a function address. Makes the thread execute
//  the input function and returns the thread to its original execution position
//  and suspension.
//
//  NOTICE: The input function should trigger a breakpoint exception when it is done!
//
// Arguments:   threadId - The input thread id.
//              funcAddress - The input function address.
// Return Val:  bool - Success / failure.
// Implementation notes:
//   a. Stores the thread execution context.
//   b. Sets the thread execution context to the input function address.
//   c. Resumed the thread run.
//   d. When the executed function is done, it triggers a breakpoint exception.
//   e. We catch the breakpoint exception and restore the thread execution context.
//
// Author:      Yaki Tebeka
// Date:        11/5/2005
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::makeThreadExecuteFunction(const osThreadId& threadId,
                                                       osProcedureAddress64 funcAddress)
{
    bool retVal = false;
    bool wasFunctionExecuted = false;

    // Output debug log:
    OS_OUTPUT_DEBUG_LOG(PD_STR_makeThreadExecuteFunctionStarted, OS_DEBUG_LOG_DEBUG);

    // Sanity check - make sure that the debugged process is suspended:
    GT_IF_WITH_ASSERT(_isDebuggedProcssSuspended && _isDuringInternalContinue)
    {
        // Sanity check - make sure that we have the debugged process OS id:
        GT_IF_WITH_ASSERT(_debuggedProcessPid != 0)
        {
            // Tell the gdb driver that we are about to send an internal interrupt (SIGINT):
            _gdbDriver.waitForInternalDebuggedProcessInterrupt();

            // Mark the we are during function execution:
            _isDuringFunctionExecution = true;

            // Suspend the debugged process run by sending it an interrupt (SIGINT):
            //int rc1 = ::kill(_debuggedProcessPid, SIGINT);
            bool suspendBefore = false;
            GT_IF_WITH_ASSERT(trySuspendProcess(suspendBefore))
            {
                {
#if (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
                    // In Mac, we need to release the thread to allow it to execute our function.
                    bool rcResum = isDriverThread(threadId);

                    if (!rcResum)
                    {
                        // We only need to resume non-driver threads, since the driver threads are
                        // not suspended to begin with:
                        rcResum = resumeDebuggedProcessThread(threadId);
                    }

                    GT_IF_WITH_ASSERT(rcResum)
#endif
                    {
                        // Switch GDB's active thread to be the input thread id:
                        bool rc3 = switchGDBActiveThread(threadId);
                        GT_IF_WITH_ASSERT(rc3)
                        {
                            // Build a function address string:
                            // Example: "0x2aaaa()"
                            gtASCIIString functionAddressStr;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

                            if (_debuggedExecutableArchitecture == OS_I386_ARCHITECTURE)
                            {
                                functionAddressStr.appendFormattedString("%p()", funcAddress);
                            }
                            else if (_debuggedExecutableArchitecture == OS_X86_64_ARCHITECTURE)
                            {
                                functionAddressStr.appendFormattedString("%#018llx()", funcAddress);
                            }
                            else
                            {
                                // Unsupported or unknown architecture, we should not get here!
                                GT_ASSERT(false);
                            }

#else
                            // Just support the same architecture as the one we are running:
                            functionAddressStr.appendFormattedString("%p()", funcAddress);
#endif

                            // Execute the function using the current debugged process thread:
                            bool rc5 = _gdbDriver.executeGDBCommand(PD_EXECUTE_FUNCTION_CMD, functionAddressStr);
                            GT_IF_WITH_ASSERT(rc5)
                            {
                                wasFunctionExecuted = true;
                            }

#if (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

                            // Yaki 23/8/2009
                            // As of XCode tools 3.2 (comes with Snow Leopard), gdb version 6.3.50-20050815 (Apple version gdb-1344), the below PD_CONTINUE_CMD
                            // gets stuck, with gdb claiming that it cannot resume the process when the current thread is suspended.
                            // The below setting of the active thread to a thread that is not suspended (the API thread) seems to work-around the problem

                            // Get the API thread's gdb id:
                            gtString APIThreadGDBId;
                            bool rcGetAPIThreadGDBId = getAPIThreadGDBThreadID(APIThreadGDBId);
                            GT_IF_WITH_ASSERT(rcGetAPIThreadGDBId)
                            {
                                // Make the API thread gdb's current thread:
                                bool rcMakeThreadCurrent = _gdbDriver.executeGDBCommand(PD_SET_ACTIVE_THREAD_CMD, APIThreadGDBId);
                                GT_ASSERT(rcMakeThreadCurrent);
                            }

                            // In Mac, we need to suspend the thread after it executed our function.
                            bool rcSusp = isDriverThread(threadId);

                            if (!rcSusp)
                            {
                                // We only suspend non-driver threads
                                rcSusp = suspendDebuggedProcessThread(threadId);
                            }

                            GT_ASSERT(rcSusp);
#endif

                            // Mark that the function execution ended:
                            _isDuringFunctionExecution = false;

                            // Mark that we only continue running the debugged process
                            // for internal purposes (to the outside world, the debugged process will still
                            // be seen as suspended)
                            _isDuringInternalContinue = true;

                            // Resume the debugged process execution:
                            if (!suspendBefore)
                            {
                                if (!_isDuringFatalSignalSuspension)
                                {
                                    if (_isUnderHostBreakpoint)
                                    {
                                        GT_IF_WITH_ASSERT(ReleaseSpyThread())
                                        {
                                            // If we managed to execute the function:
                                            if (wasFunctionExecuted)
                                            {
                                                retVal = true;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        GT_IF_WITH_ASSERT(tryResumeProcess())
                                        {
                                            // If we managed to execute the function:
                                            if (wasFunctionExecuted)
                                            {
                                                retVal = true;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                retVal = true;
                            }
                        }
                    }
                }
            }
        }
    }

    if (!retVal)
    {
        // Output failure debug log:
        OS_OUTPUT_DEBUG_LOG(PD_STR_remoteFuncFailed, OS_DEBUG_LOG_ERROR);
    }

    // Mark the the function execution ended:
    _isDuringFunctionExecution = false;

    // Output debug log:
    OS_OUTPUT_DEBUG_LOG(PD_STR_makeThreadExecuteFunctionEnded, OS_DEBUG_LOG_DEBUG);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::afterAPICallIssued
// Description: Is called after an API call (and all its parameters) were sent
//              To the spy
// Date:        30/4/2009
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::afterAPICallIssued()
{

}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::getDebuggedThreadCallStack
// Description:
//   Get the call stack of a debugged process thread.
//   The debugged thread must be suspended before the call to this function.
//
// Arguments:   threadId - The debugged process thread id.
//              callStack - The output call stack.
//              hideSpyDLLsFunctions - if true, stack frames that contain spy DLLs
//                                     functions (and all stack frames that appear beneath
//                                     them) will be removed from the output call stack.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        14/3/2007
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::getDebuggedThreadCallStack(osThreadId threadId, osCallStack& callStack,
                                                        bool hideSpyDLLsFunctions)
{
    bool retVal = false;

    // Make sure the output stack is empty:
    callStack.clearStack();

    // We operate only when the debugged process is suspended:
    if (isDebuggedProcssSuspended())
    {
        // See if we already got this thread's calls stack during this suspension:
        gtMap<osThreadId, pdGDBCallStack*>::const_iterator findIter = _threadCallStacks.find(threadId);
        gtMap<osThreadId, pdGDBCallStack*>::const_iterator endIter = _threadCallStacks.end();

        if (findIter != endIter)
        {
            // Get the calls stack:
            const pdGDBCallStack* pCallsStack = (*findIter).second;
            GT_IF_WITH_ASSERT(pCallsStack != NULL)
            {
                // Make sure the logged call stack belongs to the input thread:
                GT_IF_WITH_ASSERT(pCallsStack->_callStack.threadId() == threadId)
                {
                    // If we need to hide spy dll functions:
                    if (hideSpyDLLsFunctions)
                    {
                        outputHiddenSpyCallStack(pCallsStack->_callStack, callStack);
                    }
                    else
                    {
                        // Simply output the logged call stack:
                        callStack = pCallsStack->_callStack;
                    }

                    retVal = true;
                }
            }
        }
        else // findIter == endIter
        {
            // If we didn't get the calls stack yet, we need to update it:
            if (_isDuringInternalContinue)
            {
                pdGDBCallStack* pCallsStack = NULL;
                retVal = updateThreadCallStackDuringInternalContinue(threadId, pCallsStack);
                GT_IF_WITH_ASSERT(retVal && (pCallsStack != NULL))
                {
                    // Update this stack in the map and return it:
                    _threadCallStacks[threadId] = pCallsStack;

                    // If we need to hide spy dll functions:
                    if (hideSpyDLLsFunctions)
                    {
                        outputHiddenSpyCallStack(pCallsStack->_callStack, callStack);
                    }
                    else
                    {
                        // Simply output the logged call stack:
                        callStack = pCallsStack->_callStack;
                    }
                }
            }
            else // !_isDuringInternalContinue
            {
                // This should only happen in crashes:
                GT_ASSERT(_isDuringFatalSignalSuspension);

                pdGDBCallStack* pCallsStack = NULL;
                retVal = updateThreadCallStack(threadId, pCallsStack);
                GT_IF_WITH_ASSERT(retVal && (pCallsStack != NULL))
                {
                    // Update this stack in the map and return it:
                    _threadCallStacks[threadId] = pCallsStack;

                    // If we need to hide spy dll functions:
                    if (hideSpyDLLsFunctions)
                    {
                        outputHiddenSpyCallStack(pCallsStack->_callStack, callStack);
                    }
                    else
                    {
                        // Simply output the logged call stack:
                        callStack = pCallsStack->_callStack;
                    }
                }
            }
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////
/// \brief Get host breakpoint triggering thread id
///
/// \param index a indx of requested thread
/// \return true if success or false
///
/// \author Vadim Entov
/// \date 21/02/2016
bool pdLinuxProcessDebugger::getBreakpointTriggeringThreadIndex(int& index) const
{
    bool retVal = getThreadIndexFromId(m_triggeringThreadId, index);

    return retVal && (-1 < index);
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::fillCallsStackDebugInfo
// Description: fills callStack with debug information based on the give instruction pointers
// Author:      Uri Shomroni
// Date:        26/10/2008
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::fillCallsStackDebugInfo(osCallStack& callStack, bool hideSpyDLLsFunctions)
{
    // Construct a string that will call the addr2line utility and ask for any and all
    // debug information, based on the instruction pointers:
    int n = callStack.amountOfStackFrames();

    osCallStack interimStack;
    bool stackOk = true;
    int numberOfSkippedFrames = 0;
    bool bSuspended = false;


    // If the stack is empty, no need to bother with any of this
    if (n > 0)
    {
        // To execute the gdb queries we are about to make, we need to stop the internal continue:
        // Tell the gdb driver that we are about to send an internal interrupt (SIGINT):
        if (!_isDuringFatalSignalSuspension && !_isDebuggedProcssSuspended)
        {
            _gdbDriver.waitForInternalDebuggedProcessInterrupt();
        }

        _isDuringGDBSynchronusCommandExecution = true;

        bool canGetStackInfo = true;

        // Check if we need to break the internal resume:
        if (!_isDuringFatalSignalSuspension)
        {
            if (_isDebuggedProcssSuspended)
            {
                // Suspend the debugged process run by sending it an interrupt (SIGINT):
                GT_IF_WITH_ASSERT(trySuspendProcess(bSuspended))
                {
                    canGetStackInfo = true;
                }
            }
            else
            {
                canGetStackInfo = !_isDuringInternalContinue;
            }
        }
        else
        {
            // We are during a debugged process crash, we can simply get the debug info without a
            // need to suspend the internal run. However, if we ARE in an internal continue, we
            // cannot stop it at this point:
            canGetStackInfo = !_isDuringInternalContinue;
        }

        if (canGetStackInfo)
        {
            // Iterate the frames, and fill in each one's debug info:
            for (int j = 0; j < n; j++)
            {
                // Get whatever information we already have about the frame:
                const osCallStackFrame* pCurrentOrigFrame = callStack.stackFrame(j);
                osCallStackFrame currentFrame;
                GT_IF_WITH_ASSERT(pCurrentOrigFrame != NULL)
                {
                    currentFrame = *pCurrentOrigFrame;
                }
                else
                {
                    stackOk = false;
                    break;
                }

                // Get the source code data and the library data for the instruction address with gdb's
                //info line and info sharedlibrary commands.
                pdGDBSourceCodeData* pSourceCodeData = NULL;
                pdGDBLibraryData* pLibraryData = NULL;

                osInstructionPointer instructionPointerAddress = pCurrentOrigFrame->instructionCounterAddress();

                if ((0 == j) && (!isAtAPIOrKernelBreakpoint(OS_NO_THREAD_ID)))
                {
                    instructionPointerAddress = (osInstructionPointer)((gtUInt64)instructionPointerAddress + 1);
                }

                if (instructionPointerAddress != (osInstructionPointer)NULL)
                {
                    // We give the "info line" command the addresses as "info line *0xfeedface", so it
                    // will know to recognize them as addresses inside function instead of function adrresses
                    gtASCIIString instructionPointerAddressAsInfoLineParameter = '*';
                    gtASCIIString instructionPointerAddressAsInfoSharedlibraryParameter;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

                    if (_debuggedExecutableArchitecture == OS_I386_ARCHITECTURE)
                    {
                        instructionPointerAddressAsInfoLineParameter.appendFormattedString("%p", (gtUInt32)instructionPointerAddress);
                        instructionPointerAddressAsInfoSharedlibraryParameter.appendFormattedString("%p", (gtUInt32)instructionPointerAddress);
                    }
                    else if (_debuggedExecutableArchitecture == OS_X86_64_ARCHITECTURE)
                    {
                        instructionPointerAddressAsInfoLineParameter.appendFormattedString("%#018llx", instructionPointerAddress);
                        instructionPointerAddressAsInfoSharedlibraryParameter.appendFormattedString("%#018llx", instructionPointerAddress);
                    }
                    else
                    {
                        // Unsupported or unknown architecture, we should not get here!
                        GT_ASSERT(false);
                    }

#else
                    // Just support the same architecture as the one we are running:
                    instructionPointerAddressAsInfoLineParameter.appendFormattedString("%p", instructionPointerAddress);
                    instructionPointerAddressAsInfoSharedlibraryParameter.appendFormattedString("%p", instructionPointerAddress);
#endif

                    bool rcCommand = _gdbDriver.executeGDBCommand(PD_GET_DEBUG_INFO_AT_ADDRESS, instructionPointerAddressAsInfoLineParameter, (const pdGDBData**)(&pSourceCodeData));
                    GT_ASSERT(rcCommand);

                    _gdbDriver.setInstructionAddressToFind(instructionPointerAddress);
                    rcCommand = _gdbDriver.executeGDBCommand(PD_GET_LIBRARY_AT_ADDRESS, instructionPointerAddressAsInfoSharedlibraryParameter, (const pdGDBData**)(&pLibraryData));
                    _gdbDriver.setInstructionAddressToFind(NULL);
                    GT_ASSERT(rcCommand);

                    // We assert here since the source code command returns a result even if it fails
                    GT_IF_WITH_ASSERT(pSourceCodeData != NULL)
                    {
                        currentFrame.setSourceCodeFilePath(pSourceCodeData->_sourceCodeFilePath);
                        currentFrame.setSourceCodeFileLineNumber(pSourceCodeData->_lineNumber);

                        // Note that if we do not find some parts of the information, we simply leave it as it is,
                        // Since the osCallsStackReader on the spy side get some of the information as well:
                        // Also check if this is a spy function:
                        gtString sourceCodePathAsString = pSourceCodeData->_sourceCodeFilePath.asString();

                        static const gtString spyFileName1 = L"gsOpenGLWrappers.cpp";
                        static const gtString spyFileName2 = L"gsOpenGLMonitor.cpp";
                        static const gtString spyFileName3 = L"gsOpenGLExtensionsWrappers.cpp";
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
                        static const gtString spyFileName4 = L"gsGLXWrappers.cpp";
#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
                        static const gtString spyFileName4 = L"gsCGLWrappers.cpp";
#else
#error unknown Linux variant!
#endif
                        static const gtString spyFileName5 = L"csOpenCLWrappers.cpp";
                        static const gtString spyFileName6 = L"csOpenCLExtensionsWrappers.cpp";
                        static const gtString spyFileName7 = L"csOpenGLIntegrationWrappers.cpp";
                        static const gtString spyFileName8 = L"csOpenCLMonitor";

                        // Check if it contains spy files:
                        if ((sourceCodePathAsString.find(spyFileName1) != -1) ||
                            (sourceCodePathAsString.find(spyFileName2) != -1) ||
                            (sourceCodePathAsString.find(spyFileName3) != -1) ||
                            (sourceCodePathAsString.find(spyFileName4) != -1) ||
                            (sourceCodePathAsString.find(spyFileName5) != -1) ||
                            (sourceCodePathAsString.find(spyFileName6) != -1) ||
                            (sourceCodePathAsString.find(spyFileName7) != -1) ||
                            (sourceCodePathAsString.find(spyFileName8) != -1))
                        {
                            currentFrame.markAsSpyFunction();
                        }
                    }

                    if (pLibraryData != NULL)
                    {
                        currentFrame.setModuleFilePath(pLibraryData->_libraryFilePath);

                        static const gtString openGLSpyModuleName = OS_GREMEDY_OPENGL_SERVER_MODULE_NAME;
                        static const gtString openGLESSpyESModuleName = OS_OPENGL_ES_COMMON_DLL_NAME;
                        static const gtString openCLSpyModuleName = OS_GREMEDY_OPENCL_SERVER_MODULE_NAME;
                        const gtString& libraryPathAsString = pLibraryData->_libraryFilePath.asString();

                        if ((libraryPathAsString.find(openGLSpyModuleName) >= 0) ||
                            (libraryPathAsString.find(openGLESSpyESModuleName) >= 0) ||
                            (libraryPathAsString.find(openCLSpyModuleName) >= 0))
                        {
                            static const gtString linuxSystemPathPrefix = L"/usr/lib";

                            if (!libraryPathAsString.startsWith(linuxSystemPathPrefix))
                            {
                                currentFrame.markAsSpyFunction();
                            }
                        }
                    }
                    else
                    {
                        // We do not assert here as it causes the program to hang, and not finding this info is okay:
                        gtString errMsg = L"Could not find module information for address ";
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

                        if (_debuggedExecutableArchitecture == OS_I386_ARCHITECTURE)
                        {
                            errMsg.appendFormattedString(L"%p", (gtUInt32)instructionPointerAddress);
                        }
                        else if (_debuggedExecutableArchitecture == OS_X86_64_ARCHITECTURE)
                        {
                            errMsg.appendFormattedString(GT_64_BIT_POINTER_FORMAT_LOWERCASE, instructionPointerAddress);
                        }
                        else
                        {
                            // Unsupported or unknown architecture, we should not get here!
                            GT_ASSERT(false);

                            // Add the address as 64-bit, to be sure:
                            errMsg.appendFormattedString(GT_64_BIT_POINTER_FORMAT_LOWERCASE, instructionPointerAddress);
                        }

#else
                        errMsg.appendFormattedString(L"%p", instructionPointerAddress);
#endif
                        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                    }
                }

                // Note that frames might get marked as spy functions when we get their module names
                // (in osCallsStackReader), so this check isn't the same as checking if the source
                // code file name has one of the spy names:
                if (hideSpyDLLsFunctions && currentFrame.isSpyFunction())
                {
                    interimStack.clearStack();
                    numberOfSkippedFrames = j + 1;
                }
                else
                {
                    interimStack.addStackFrame(currentFrame);
                }
            }
        }

        // If we've internally suspended the debugged process run:
        if (canGetStackInfo && !_isDuringFatalSignalSuspension)
        {
            // Resume (internally) the debugged process run:
            _isDuringGDBSynchronusCommandExecution = false;

            if (!bSuspended)
            {
                if (_isUnderHostBreakpoint)
                {
                    GT_ASSERT(ReleaseSpyThread());
                }
                else
                {
                    if (!bSuspended)
                    {
                        GT_ASSERT(tryResumeProcess());
                    }
                }
            }
        }
        else
        {
            // clear the flag anyway:
            _isDuringGDBSynchronusCommandExecution = false;
        }
    }

    // Make sure no data was somehow lost:
    GT_IF_WITH_ASSERT((interimStack.amountOfStackFrames() + numberOfSkippedFrames) == n)
    {
    }
    else
    {
        stackOk = false;
    }

    if (stackOk)
    {
        callStack = interimStack;
    }
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::initialize
// Description: Initializes this class members.
// Author:      Yaki Tebeka
// Date:        4/1/2007
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::initialize()
{
    _debuggedProcessPid = 0;
    _spiesAPIThreadId = OS_NO_THREAD_ID;
    _isAPIThreadRunning = false;
    _debuggedProcessExists = false;
    _currentGDBState = gdb_state::gdb_not_initialized_state;
    _isDebuggedProcssSuspended = false;
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
    _isDebugging64BitApplication = false;
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    _isDebugging64BitApplication = true;
#else
#error Unkown address space size!
#endif
    _isDuringInternalContinue = false;
    _isDuringGDBSynchronusCommandExecution = false;
    _isDuringFunctionExecution = false;
    _isDuringKernelDebugging = false;
    _isDuringFatalSignalSuspension = false;

    delete _pDebuggedProcessThreadsData;
    _pDebuggedProcessThreadsData = NULL;

    clearCallStacksMap();

    _functionNameAtAddress.clear();
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::launchGDB
// Description: Launch the gdb executable, appropriate to the platform on which we run and the
//              process that is about to be debugged.
// Arguments: processCreationData - Contains details about the process to be debugged.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/8/2010
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::launchGDB(const apDebugProjectSettings& processCreationData)
{
    bool retVal = false;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    {
        (void)(processCreationData); // unused
        // On Linux - use the default gdb executable:
        retVal = _gdbDriver.initialize(PD_LINUX_GDB_PATH);
    }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    {
        gtString gdbExecutable = PD_MAC_GDB_PATH;
        apProjectExecutionTarget projExecutionTarget = processCreationData.projectExecutionTarget();

        // The iPhone simulator uses a different version of gdb:
        switch (projExecutionTarget)
        {
            case AP_LOCAL_EXECUTION_TARGET:
            {
                gdbExecutable = PD_MAC_GDB_PATH;
            }
            break;

            case AP_IPHONE_SIMULATOR_EXECUTION_TARGET:
            {
                gdbExecutable = PD_IPHONE_SIMULATOR_GDB_PATH;
            }
            break;

            case AP_IPHONE_DEVICE_EXECUTION_TARGET:
            {
                // The iPhone device should run using pdIPhoneDeviceProcessDebugger...
                GT_ASSERT(false);
            }
            break;

            default:
            {
                // Unexpected project type!
                GT_ASSERT(false);
            }
            break;
        }

        // Initialize GDB using the path we got:
        retVal = _gdbDriver.initialize(gdbExecutable);
    }
#else
    {
#error Unknown build target!
    }
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::setDebuggedProcessEnvVariables
// Description: Sets additional debugged process environment variables that
//              appear in the process creation data.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/1/2007
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::setLoaderEvnVariables
// Description:
//  Sets debugged process environment variables that will cause
//  the Linux loader to load our OpenGL server instead of the system's
//  OpenGL implementation.
// Arguments: userLdlibrarypath - The user's "library path" env variable content.
//            userLdPreload - The user's "preload" env variable content.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/2/2007
//
// Implementation notes:
//
//   We add our spies directory as the first directory in the "library path"
//   environment variable. This causes our OpenGL implementation to be loaded
//   before the system's OpenGL implementation.
//
//   We also enable the use of the "preload" environment variable.
//   Notice that when using this option:
//   a. Our OpenGL implementation is initialized twice
//      (probably once when it is pre-loaded and second time when the debugged
//       process attaches to it).
//   b. It caused a crash when debugging the Maya application (we don't know why).
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::setDebuggedProcessEnvVariables()
{
    bool retVal = true;

    // Will get true if the user set the "library path" or "preload" environment variables:
    bool ldLibraryPathSet = false;
    bool ldPreloadSet = false;

    // Iterate the environment variables contained in _debuggedProcessCreationData:
    const gtList<osEnvironmentVariable>& envVariablesList = _debuggedProcessCreationData.environmentVariables();
    gtList<osEnvironmentVariable>::const_iterator iter = envVariablesList.begin();
    gtList<osEnvironmentVariable>::const_iterator endIter = envVariablesList.end();

    while (iter != endIter)
    {
        // Get the current environment variable name:
        const gtString& envVariableName = (*iter)._name;

        if (envVariableName == s_ldLibraryPathEnvVariableName)
        {
            // If this is the "library path" environment variable:
            ldLibraryPathSet = true;
        }
        else if (envVariableName == s_ldPreloadEnvVariableName)
        {
            // If this is the "preload" environment variable - log its content for now:
            ldPreloadSet = true;
        }

        gtString currentEnvVarString;
        bool rc1 = processEnvVariableToString(*iter, currentEnvVarString);
        GT_ASSERT(rc1);

        // Add / Set the current environment variable to GDB's debugged program:
        bool rc2 = _gdbDriver.executeGDBCommand(PD_SET_ENV_VARIABLE_CMD, currentEnvVarString.asASCIICharArray());
        GT_ASSERT(rc2);

        retVal = retVal && rc1 && rc2;

        iter++;
    }

    // If the user didn't set the [DY]LD_LIBRARY_PATH variable, we still need to:
    if (!ldLibraryPathSet)
    {
        gtString ldLibraryPathString;
        osEnvironmentVariable dummyEnvValue;
        dummyEnvValue._name = s_ldLibraryPathEnvVariableName;
        bool rc3 = processEnvVariableToString(dummyEnvValue, ldLibraryPathString);
        GT_ASSERT(rc3);
        bool rc4 = _gdbDriver.executeGDBCommand(PD_SET_ENV_VARIABLE_CMD, ldLibraryPathString.asASCIICharArray());
        GT_ASSERT(rc4);
        retVal = retVal && rc3 && rc4;
    }

    // If the user didn't set the LD_PRELOAD/DYLD_INSERT_LIBRARIES variable, we may still need to:
    if (!ldPreloadSet)
    {
        gtString ldPreloadString;
        osEnvironmentVariable dummyEnvValue;
        dummyEnvValue._name = s_ldPreloadEnvVariableName;
        bool rc5 = processEnvVariableToString(dummyEnvValue, ldPreloadString);
        GT_ASSERT(rc5);
        bool rc6 = _gdbDriver.executeGDBCommand(PD_SET_ENV_VARIABLE_CMD, ldPreloadString.asASCIICharArray());
        GT_ASSERT(rc6);
        retVal = retVal && rc5 && rc6;
    }

    // If debugging HSA, set the HSA_TOOLS_LIB and HSA_EMULATE_AQL environment variables:
    if (_debuggedProcessCreationData.shouldDebugHSAKernels())
    {
        osEnvironmentVariable hsaToolsLibEnvVar;
        hsaToolsLibEnvVar._name = L"HSA_TOOLS_LIB";
        hsaToolsLibEnvVar._value = L"libhsa-runtime-tools64.so.1 libAMDTHsaServer.so";
        gtString hsaToolsLibString;
        bool rc7 = processEnvVariableToString(hsaToolsLibEnvVar, hsaToolsLibString);
        GT_ASSERT(rc7);
        bool rc8 = _gdbDriver.executeGDBCommand(PD_SET_ENV_VARIABLE_CMD, hsaToolsLibString.asASCIICharArray());
        GT_ASSERT(rc8);
        retVal = retVal && rc7 && rc8;

        osEnvironmentVariable hsaEmulateAqlEnvVar;
        hsaEmulateAqlEnvVar._name = L"HSA_EMULATE_AQL";
        hsaEmulateAqlEnvVar._value = L"1";
        gtString hsaEmulateAqlString;
        bool rc9 = processEnvVariableToString(hsaEmulateAqlEnvVar, hsaEmulateAqlString);
        GT_ASSERT(rc9);
        bool rc10 = _gdbDriver.executeGDBCommand(PD_SET_ENV_VARIABLE_CMD, hsaEmulateAqlString.asASCIICharArray());
        GT_ASSERT(rc10);
        retVal = retVal && rc9 && rc10;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::processEnvVariableToString
// Description: Outputs into envVarAsString a string of the form VARNAME=VALUE
//              based on envVar. If envVar is one of the specially set environment
//              variables (LD/DYLD_LIBRARY_PATH and LD_PRELOAD/DYLD_INSERT_LIBRARIES)
//              also adds the value needed by CodeXL for the interception.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        12/5/2009
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::processEnvVariableToString(const osEnvironmentVariable& envVar, gtString& envVarAsString)
{
    bool retVal = false;

    gtString varName = envVar._name;
    gtString varValue = envVar._value;
    apInterceptionMethod interceptionMethod = _debuggedProcessCreationData.interceptionMethod();

    if (varName == s_ldLibraryPathEnvVariableName)
    {
        // If this is the [DY]LD_LIBRARY_PATH variable, we need to add the CodeXL binaries dir to it:
        retVal = processLibraryPathEnvVariableValue(varValue);
    }
    else if (varName == s_dyldFrameworkPathEnvVariableName)
    {
        // If this is the DYLD_FRAMEWORK_PATH variable on the Mac, add the current process value (and the iPhone SDK if needed):
        retVal = processFrameworkPathEnvVariableValue(varValue);
    }
    else if ((varName == s_ldPreloadEnvVariableName) && (interceptionMethod == AP_PRELOAD_INTERCEPTION))
    {
        // If we are using LD_PRELOAD/DYLD_INSERT_LIBRARIES interception, we need to
        // Add the spy name to the list:
        retVal = processPreloadEnvVariableValue(varValue);
    }
    else
    {
        // Leave the value as it is:
        retVal = true;
    }

    // Create the output string:
    envVarAsString = varName;
    envVarAsString.append('=');
    envVarAsString.append(varValue);

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::processPreloadEnvVariableValue
// Description:
//  Sets debugged process "preload" environment variables.
//  This will cause the Linux loaded to load our OpenGL server instead
//  of the system's OpenGL implementation.
// Arguments: ldPreloadValue - The user's "preload" env variable content.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/2/2007
// Implementation notes:
//   We set the "preload" environment variable to point our OpenGL server.
//   When "preload" is set, the dynamic linker will use the specified library
//   before any other when it searches for shared libraries.
//   For more details see http://developers.sun.com/solaris/articles/lib_interposers.html
//   or google for "preload".
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::processPreloadEnvVariableValue(gtString& ldPreloadValue)
{
    bool retVal = true;

#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
    static const char preloadListDelimiter = ' ';
#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    static const char preloadListDelimiter = ':';
#else
#error Unknown Linux variant
#endif

    // Remove quotation marks from the user's "preload" environment variable content:
    ldPreloadValue.replace(L"\"", L"", true);
    ldPreloadValue.replace(L"\'", L"", true);

    // Get this process "preload" environment variable content (which was inherited from the shell):
    gtString thisProcessLdPreload;
    osGetCurrentProcessEnvVariableValue(s_ldPreloadEnvVariableName, thisProcessLdPreload);

    // Remove quotation marks:
    thisProcessLdPreload.replace(L"\"", L"", true);
    thisProcessLdPreload.replace(L"\'", L"", true);

    // If we are doing a "Preload" interception:
    apInterceptionMethod interceptionMethod = _debuggedProcessCreationData.interceptionMethod();

    // Make sure the windows interception method is not being used by mistake:
    GT_ASSERT((AP_LIBRARY_PATH_INTERCEPTION == interceptionMethod) || (AP_PRELOAD_INTERCEPTION == interceptionMethod));

    if (interceptionMethod == AP_PRELOAD_INTERCEPTION)
    {
        bool wasLdPreloadEmpty = ldPreloadValue.isEmpty();

        // Get the spies directory:
        const osFilePath& spiesDir = _debuggedProcessCreationData.spiesDirectory();

        // Calculate the OpenGL spy module full path:
        gtString openGLSpyFullPath = spiesDir.asString();
        openGLSpyFullPath.append(osFilePath::osPathSeparator).append(OS_GREMEDY_OPENGL_SERVER_MODULE_NAME);

        // Add it to the env. variable:
        ldPreloadValue.prepend(preloadListDelimiter).prepend(openGLSpyFullPath);

        /*      if (apDoesProjectTypeSupportOpenGLES(projType))
                {
                    // Calculate the OpenGL ES spy module full path:
                    gtString openGLESSpyFullPath = spiesDir.asString();
                    openGLESSpyFullPath.append(osFilePath::osPathSeparator).append(OS_OPENGL_ES_LOADER_DLL_NAME);

                        // Add it to the env. variable:
                    ldPreloadValue.prepend(preloadListDelimiter).prepend(openGLESSpyFullPath);
                }*/

        // Calculate the OpenCL spy module full path:
        gtString openCLSpyModuleFullPath = spiesDir.asString();
        openCLSpyModuleFullPath.append(osFilePath::osPathSeparator).append(OS_GREMEDY_OPENCL_SERVER_MODULE_NAME);

        // Add it to the env. variable:
        ldPreloadValue.prepend(preloadListDelimiter).prepend(openCLSpyModuleFullPath);

        // If the variable value was empty, we should now have a redundant trailing list delimiter:
        if (wasLdPreloadEmpty)
        {
            ldPreloadValue.removeTrailing(preloadListDelimiter);
        }
    }

    // Add this process's "preload" content:
    if (!thisProcessLdPreload.isEmpty())
    {
        if (!ldPreloadValue.isEmpty())
        {
            ldPreloadValue += preloadListDelimiter;
        }

        ldPreloadValue += thisProcessLdPreload;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::processLibraryPathEnvVariableValue
// Description:
//  Sets debugged process "library path" environment variables to contain:
//  a. The spies directory.
//  b. The debugger installation directory.
//  c. The user's "library path" content.
//  d. This process "library path" content.
//
// Arguments: userLdlibrarypath - The user's "library path" env variable content.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/2/2007
// Implementation notes:
//  We add the debugger bin directory to the "library path" environment variable.
//  This will enable our OpenGL server to load the shared libraries on which it is dependent.
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::processLibraryPathEnvVariableValue(gtString& ldLibraryPathValue)
{
    bool retVal = true;

    // Remove quotation marks from the user's "library path" content:
    ldLibraryPathValue.replace(L"\"", L"", true);
    ldLibraryPathValue.replace(L"\'", L"", true);

    // Get this process "library path" (which was inherited from the shell):
    gtString thisProcessLdlibrarypath;
    osGetCurrentProcessEnvVariableValue(s_ldLibraryPathEnvVariableName, thisProcessLdlibrarypath);

    // Remove quotation marks:
    thisProcessLdlibrarypath.replace(L"\"", L"", true);
    thisProcessLdlibrarypath.replace(L"\'", L"", true);

    // Get the debugger's bin directory path:
    const osFilePath& debuggerBinDir = _debuggedProcessCreationData.debuggerInstallDir();

    // Add the debugger installation directory:
    if (!ldLibraryPathValue.isEmpty())
    {
        ldLibraryPathValue.prepend(osFilePath::osEnvironmentVariablePathsSeparator);
    }

    ldLibraryPathValue.prepend(debuggerBinDir.asString());

    // If we are doing a "Library path" interception:
    apInterceptionMethod interceptionMethod = _debuggedProcessCreationData.interceptionMethod();

    // Make sure the windows interception method is not being used by mistake:
    GT_ASSERT((AP_LIBRARY_PATH_INTERCEPTION == interceptionMethod) || (AP_PRELOAD_INTERCEPTION == interceptionMethod));

    if (interceptionMethod == AP_LIBRARY_PATH_INTERCEPTION)
    {
        // Add the spies directory to be first:
        const osFilePath& spiesDir = _debuggedProcessCreationData.spiesDirectory();
        ldLibraryPathValue.prepend(osFilePath::osEnvironmentVariablePathsSeparator).prepend(spiesDir.asString());
    }

    // Add this process "library path" content:
    if (!thisProcessLdlibrarypath.isEmpty())
    {
        ldLibraryPathValue += osFilePath::osEnvironmentVariablePathsSeparator;
        ldLibraryPathValue += thisProcessLdlibrarypath;
    }

    // BUG404491: Starting with Catalyst 13.6Beta and newer, the fglrx_dri.so OpenGL driver needs to be able to load symbols from the libGL.so.1.2 OpenGL runtime.
    // This runtime is placed in one of the LIBGL_DRIVERS_PATH folders. To minimize impact on the user app, we add this at the _end_ of the variable value.
    // We do not assume or assert the variable is present to allow this to work on non-AMD systems.
    gtString thisProcessLibglDriversPathValue;
    bool rclibGL = osGetCurrentProcessEnvVariableValue(s_libglDriversPathEnvVariableName, thisProcessLibglDriversPathValue);

    if (rclibGL && (!thisProcessLibglDriversPathValue.isEmpty()))
    {
        ldLibraryPathValue.append(osFilePath::osEnvironmentVariablePathsSeparator).append(thisProcessLibglDriversPathValue);

        // The LIBGL_DRIVERS_PATH sometimes points to the [...]/fglrx/dri folder, where the library we need is in [...]/fglrx.
        // Add a /.. as well:
        static const gtString upOneLevel = L"/..";
        static const gtString upOneLevelAndColon = L"/..:";
        static const gtString colon = osFilePath::osEnvironmentVariablePathsSeparator;

        thisProcessLibglDriversPathValue.append(upOneLevel).replace(colon, upOneLevelAndColon);

        ldLibraryPathValue.append(osFilePath::osEnvironmentVariablePathsSeparator).append(thisProcessLibglDriversPathValue);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::processFrameworkPathEnvVariableValue
// Description: Processes the value of the DYLD_FRAMEWORK_PATH environment
//              variable, adding this process' value and the iPhone SDK value
//              if this is an ES project.
//              On Linux, does nothing
// Arguments: gtString& dyldFrameworkPathValue
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        13/5/2009
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::processFrameworkPathEnvVariableValue(gtString& dyldFrameworkPathValue)
{
    bool retVal = true;
    (void)(dyldFrameworkPathValue); // unused
#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    // Remove quotation marks from the user's "framework path" content:
    dyldFrameworkPathValue.replace(L"\"", L"", true);
    dyldFrameworkPathValue.replace(L"\'", L"", true);

    // Get this process "framework path" (which was inherited from the shell):
    gtString thisProcessDyldFrameworkPath;
    osGetCurrentProcessEnvVariableValue(s_dyldFrameworkPathEnvVariableName, thisProcessDyldFrameworkPath);

    // Remove quotation marks:
    thisProcessDyldFrameworkPath.replace(L"\"", L"", true);
    thisProcessDyldFrameworkPath.replace(L"\'", L"", true);

    if (!thisProcessDyldFrameworkPath.isEmpty())
    {
        if (!dyldFrameworkPathValue.isEmpty())
        {
            dyldFrameworkPathValue.append(osFilePath::osEnvironmentVariablePathsSeparator);
        }

        dyldFrameworkPathValue.append(thisProcessDyldFrameworkPath);
    }

    /*  // TO_DO iPhone: Uri, 08/06/09 - this causes the iPhone Simulator to crash,
        // probably since it loads the wrong (iPhone) version of a framework such as
        // CoreServices or Foundation. We should either add this in the DYLD_FALLBACK_FRAMEWORK_PATH
        // or hope that the iPhone simulator does it correctly by itself...

        // If this is an iPhone project, add the iPhone SDK as the frameworks path
        if (_debuggedProcessCreationData.projectType() == AP_OPENGL_ES_PROJECT)
        {
            if (!dyldFrameworkPathValue.isEmpty())
            {
                dyldFrameworkPathValue.append(osFilePath::osEnvironmentVariablePathsSeparator);
            }

            // The iPhone SDK frameworks path is the parent directory of the OpenGL ES framework we use:
            osDirectory iphoneSDKFrameworkPath;
            iphoneSDKFrameworkPath.setDirectoryFullPathFromString(osGetOpenGLESFrameworkPath());
            iphoneSDKFrameworkPath.upOneLevel();
            dyldFrameworkPathValue.append(iphoneSDKFrameworkPath.directoryPath().asString());
        }
        */

#elif AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
    // Do nothing...
#else
#error Unknown Linux Variant!
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::setDebuggedProcessCommandLineArguments
// Description:
//  Sets the debugged process command line arguments, as specified by
//  the process creation data.
// Author:      Yaki Tebeka
// Date:        29/1/2007
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::setDebuggedProcessCommandLineArguments()
{
    bool retVal = false;

    // Get the debugged process command line arguments:
    const gtString& constCommandLineArguments = _debuggedProcessCreationData.commandLineArguments();
    gtString commandLineArguments = constCommandLineArguments.asCharArray();

    gtString outputFileName;
    bool appendMode;

    if (!osCheckForOutputRedirection(commandLineArguments, outputFileName, appendMode) && (NULL != _gdbDriver.getOutputReaderThread()))
    {
        commandLineArguments.append(L" > ");
        commandLineArguments.append(_gdbDriver.getOutputReaderThread()->getPipeFilePath());
        commandLineArguments.append(L" < ");
        gtString inputPipeName = L"";
        _gdbDriver.getProcessConsolePipeName(inputPipeName);
        commandLineArguments.append(inputPipeName);

    }

    // Set GDBs debugged process command line arguments:
    std::string utf8CmdLineArgs;
    commandLineArguments.asUtf8(utf8CmdLineArgs);
    bool rc1 = _gdbDriver.executeGDBCommand(PD_SET_COMMAND_LINE_ARGS_CMD, utf8CmdLineArgs.c_str());
    GT_IF_WITH_ASSERT(rc1)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::setDebuggedProcessWorkingDirectory
// Description:
//  Sets the debugged process working directory, as specified by
//  the process creation data.
// Author:      Yaki Tebeka
// Date:        29/1/2007
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::setDebuggedProcessWorkingDirectory()
{
    bool retVal = false;

    // Get the debugged process working directory:
    const gtString& workDirectoryAsStr = _debuggedProcessCreationData.workDirectory().asString();

    // Set GDBs debugged process command line arguments:
    std::string utf8WorkDirectory;
    workDirectoryAsStr.asUtf8(utf8WorkDirectory);
    bool rc1 = _gdbDriver.executeGDBCommand(PD_SET_WORK_DIR_CMD, utf8WorkDirectory.c_str());
    GT_IF_WITH_ASSERT(rc1)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::setDebuggedProcessArchitecture
// Description: Finds out what is the debugged process' architecture and sets
//              it into the member. In mac, also notifies gdb of this.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        1/9/2009
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::setDebuggedProcessArchitecture()
{
    bool retVal = false;

    const osFilePath& executablePath = _debuggedProcessCreationData.executablePath();

    // Get the available architectures:
    gtVector<osModuleArchitecture> availableArchitectures;
    osGetModuleArchitectures(executablePath, availableArchitectures);
    _debuggedExecutableArchitecture = OS_UNKNOWN_ARCHITECTURE;

    // gdb prefers 64-bit over 32-bit if both are present, so let's note this:
    int numberOfArchitectures = (int)availableArchitectures.size();

    for (int i = 0; i < numberOfArchitectures; i++)
    {
        osModuleArchitecture currArch = availableArchitectures[i];

        switch (currArch)
        {
            case OS_UNKNOWN_ARCHITECTURE:
            {
                // An unknown architecture is worse than any other kind, do no change
            }
            break;

            case OS_UNSUPPORTED_ARCHITECTURE:
            {
                // An unsupported architecture is worse than any other known kind
                if (_debuggedExecutableArchitecture == OS_UNKNOWN_ARCHITECTURE)
                {
                    _debuggedExecutableArchitecture = currArch;
                }
            }
            break;

            case OS_I386_ARCHITECTURE:
            {
                // 32-bit is better than unsupported kinds
                if ((_debuggedExecutableArchitecture == OS_UNKNOWN_ARCHITECTURE) ||
                    (_debuggedExecutableArchitecture == OS_UNSUPPORTED_ARCHITECTURE))
                {
                    _debuggedExecutableArchitecture = currArch;
                }
            }
            break;

            case OS_X86_64_ARCHITECTURE:
            {
                // 64-bit is better than 32-bit and unsupported kinds
                if ((_debuggedExecutableArchitecture == OS_UNKNOWN_ARCHITECTURE) ||
                    (_debuggedExecutableArchitecture == OS_UNSUPPORTED_ARCHITECTURE) ||
                    (_debuggedExecutableArchitecture == OS_I386_ARCHITECTURE))
                {
                    _debuggedExecutableArchitecture = currArch;
                }
            }
            break;

            default:
            {
                // Unexpected value!
                GT_ASSERT(false);
            }
            break;
        }
    }

    // Notify the GDB driver of this:
    _gdbDriver.setModuleArchitecture(_debuggedExecutableArchitecture);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // Use the chosen architecture to run the debugged app:
    gtString usedArchitecture;
    gtString usedABI;

    // The following values are the same as given by gdb when starting up with the -arch
    // command line argument ("gdb -arch x86_64", etc):
    switch (_debuggedExecutableArchitecture)
    {
        case OS_I386_ARCHITECTURE:
        {
            usedArchitecture = "i386";
            usedABI = "Darwin";
        }
        break;

        case OS_X86_64_ARCHITECTURE:
        {
            usedArchitecture = "i386:x86-64";
            usedABI = "Darwin64";
        }
        break;

        default:
        {
            // Unknown / Unsupported architecture, we shouldn't get here!
            GT_ASSERT(false);
        }
        break;
    }

    // Set the values we got into gdb:
    if (!(usedArchitecture.isEmpty() || usedABI.isEmpty()))
    {
        // Set the architecture first as the ABI is checked against it:
        bool rcArch = _gdbDriver.executeGDBCommand(PD_GDB_SET_ARCHITECTURE_CMD, usedArchitecture);
        GT_IF_WITH_ASSERT(rcArch)
        {
            bool rcABI = _gdbDriver.executeGDBCommand(PD_GDB_SET_ABI_CMD, usedABI);
            GT_IF_WITH_ASSERT(rcABI)
            {
                // Note the success:
                retVal = true;
            }
        }
    }

    // Update the value of the 64-bit member according to the executable path:
    bool rc64Bit = osIs64BitModule(executablePath, _isDebugging64BitApplication);
    GT_ASSERT(rc64Bit);
#else // ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // In Linux, there is no need to notify gdb of this:
    retVal = true;
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::afterGDBProcessSuspensionActions
// Description: Is called after GDB notifies us that the debugged process
//              was suspended.
// Author:      Yaki Tebeka
// Date:        4/1/2007
// Implementation notes:
//   When the debugged process is suspended by GDB, we need to leave all
//   debugged process threads suspended and resume the run of the spy API
//   thread. But, on Linux POSIX you cannot suspend / resume a single thread run
//   (unlike Solaris, that have thr_suspend() and thr_resume()).
//
//   Therefore, we do the following:
//   a. Update the debugged process threads data.
//   b. Change the instruction counter of all debugged process threads,
//      except the spy API thread, to point the osSleepForever function.
//   c. Resume the debugged process run.
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::afterGDBProcessSuspensionActions()
{
    bool retVal = false;

    // Output debug log message:
    OS_OUTPUT_DEBUG_LOG(PD_STR_afterGDBProcessSuspensionActionsStarts, OS_DEBUG_LOG_DEBUG);

    // On Generic Linux versions:
#if (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    {
        // If we don't have the debugged process id:
        if (_debuggedProcessPid == 0)
        {
            // Get the debugged process pid:
            bool rc1 = logDebuggedProcessPid();
            GT_ASSERT(rc1);
        }
    }
#endif

    // Update the debugged process threads data:
    bool rc2 = updateDebuggedProcessThreadsData();
    GT_ASSERT(rc2);

    // Update the current thread's call stack:
    bool rc3 = updateCurrentThreadCallStack();
    GT_ASSERT(rc3);

#if (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    // Suspend the debugged process threads (except the API thread):
    bool rc6 = suspendDebuggedProcessThreads();
    GT_IF_WITH_ASSERT(rc6)
#elif (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    // In Linux, until we will find a better solution, the debugged process
    // threads are currently suspended by the Spy (see gsOpenGLMonitor).
#else
#error Unknown Linux variant!
#endif

    // Output debug log message:
    OS_OUTPUT_DEBUG_LOG(PD_STR_resumingDebuggedProcessRunInternally, OS_DEBUG_LOG_DEBUG);

    // Mark that we only continue running the debugged process
    // for internal purposes (to the outside world, the debugged process will still
    // be seen as suspended)
    _isDuringInternalContinue = true;


    // Resume the debugged process execution:
    if (!_isDuringFatalSignalSuspension)
    {
        if (_isUnderHostBreakpoint)
        {
            retVal = ReleaseSpyThread();
        }
        else
        {
            retVal = tryResumeProcess();
        }
    }

    // Output debug log message:
    OS_OUTPUT_DEBUG_LOG(PD_STR_afterGDBProcessSuspensionActionsEnds, OS_DEBUG_LOG_DEBUG);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::logCreatedThread
// Description: Logs the data of a created debugged process thread.
// Arguments: OSThreadId - The thread's OS id
// Author:      Yaki Tebeka
// Date:        4/1/2007
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::logCreatedThread(osThreadId OSThreadId)
{
    // If the debugged process threads data structure does not exist - create it:
    if (_pDebuggedProcessThreadsData == NULL)
    {
        _pDebuggedProcessThreadsData = new pdGDBThreadDataList;

    }

    // Check if the thread is already registered:
    bool isThreadRegistered = false;
    gtList<pdGDBThreadData>::const_iterator iter = _pDebuggedProcessThreadsData->_threadsDataList.begin();
    gtList<pdGDBThreadData>::const_iterator endIter = _pDebuggedProcessThreadsData->_threadsDataList.end();

    while (iter != endIter)
    {
        if ((*iter)._OSThreadId == OSThreadId)
        {
            isThreadRegistered = true;
            break;
        }

        iter++;
    }

    // If the thread was not registered - register it:
    if (!isThreadRegistered)
    {
        pdGDBThreadData createdThreadData;
        createdThreadData._OSThreadId = OSThreadId;
        _pDebuggedProcessThreadsData->_threadsDataList.push_back(createdThreadData);
    }
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::updateDebuggedProcessThreadsData
// Description: Updates the debugged process threads data.
// Author:      Yaki Tebeka
// Date:        4/1/2007
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::updateDebuggedProcessThreadsData()
{
    bool retVal = false;

    // Get the debugged process threads data:
    const pdGDBData* pGDBOutputData = NULL;

    // On Mac, starting with Xcode 3.2.3, the "info threads" command does not give us all the data we need, so we use the machine interface "-thread-list-ids" instead.
    // On Linux, the machine interface function is not implementer on all platforms, so we cannot use it as we might not get the data.
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
    bool rc1 = _gdbDriver.executeGDBCommand(PD_GET_THREADS_INFO_CMD, emptyCmdParam, &pGDBOutputData);
#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    bool rc1 = _gdbDriver.executeGDBCommand(PD_GET_THREADS_INFO_VIA_MI_CMD, emptyCmdParam, &pGDBOutputData);
#else
#error Unknown Linux Variant!
#endif
    GT_IF_WITH_ASSERT(rc1 && (pGDBOutputData != NULL))
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(pGDBOutputData->type() == pdGDBData::PD_GDB_THREAD_DATA_LIST)
        {
            // Clear old threads data:
            delete _pDebuggedProcessThreadsData;
            _pDebuggedProcessThreadsData = NULL;

            // Store the threads data;
            _pDebuggedProcessThreadsData = (pdGDBThreadDataList*)pGDBOutputData;

            bool rc = fillThrdsRealizeCollection();
            GT_ASSERT(rc);

            retVal = true;
        }
    }

    return retVal;
}

///////////////////////////////////////////////////////////////////////
/// \brief Fill collection of gdb thred indexes for future resuming.
///     Spy thread and all opencl driver threads will resumed on host
///     breakpoint or host break.
///
/// \return true - success, false - fail
/// \author
/// \date 30/03/2016
bool pdLinuxProcessDebugger::fillThrdsRealizeCollection()
{
    _canMakeExecuteFunction = true;

    bool retVal = true;

    m_GDBIdsOfThreadsToRelease.clear();

    if (_spiesAPIThreadId != OS_NO_THREAD_ID)
    {
        int spyGDBId = getGDBThreadId(_spiesAPIThreadId);
        m_GDBIdsOfThreadsToRelease.insert(spyGDBId);
    }

    /// Dont remove. Need for future using. Vadim
    /*
    if (nullptr != _pDebuggedProcessThreadsData)
    {
        for (auto thrd : _pDebuggedProcessThreadsData->_threadsDataList)
        {
            if (thrd._OSThreadId != _spiesAPIThreadId && thrd._OSThreadId != m_triggeringThreadId)
            {
                pdGDBCallStack* pCallStackData = NULL;

                GT_IF_WITH_ASSERT(updateThreadCallStack(thrd._OSThreadId, pCallStackData))
                {
                    GT_IF_WITH_ASSERT(nullptr != pCallStackData)
                    {
                        auto framesCount = pCallStackData->_callStack.amountOfStackFrames();

                        if (framesCount > 2)
                        {
                            fillCallsStackDebugInfo(pCallStackData->_callStack, false);

                            auto frame = pCallStackData->_callStack.stackFrame(framesCount - 3);

                            GT_IF_WITH_ASSERT(frame)
                            {
                                gtString moduleName = L"";

                                if (frame->moduleFilePath().getFileName(moduleName))
                                {
                                    if (moduleName.find(L"libamdocl") == 0 ||
                                        moduleName.find(L"fglrx") == 0 ||
                                        moduleName.find(L"libatiadlxx") == 0 ||
                                        moduleName.find(L"libatiadlxy") == 0)
                                    {
                                        m_GDBIdsOfThreadsToRelease.insert(thrd._gdbThreadId);
                                    }
                                }
                                else
                                {
                                    if (_canMakeExecuteFunction)
                                    {
                                        for (int frameIndex = 0; frameIndex < framesCount; frameIndex++)
                                        {
                                            auto currentFrame = pCallStackData->_callStack.stackFrame(frameIndex);

                                            GT_IF_WITH_ASSERT(currentFrame)
                                            {
                                                gtString moduleName = L"";

                                                if (currentFrame->moduleFilePath().getFileName(moduleName))
                                                {
                                                    if (moduleName.find(L"fglrx") == 0 ||
                                                        moduleName.find(L"libatiadlxx") == 0 ||
                                                        moduleName.find(L"libatiadlxy") == 0)
                                                    {
                                                        _canMakeExecuteFunction = false;
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    */
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::updateCurrentThreadCallStack
// Description: Updates the current thread's call stack data.
// Author:      Yaki Tebeka
// Date:        14/3/2007
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::updateCurrentThreadCallStack()
{
    bool retVal = false;

    // Get the active thread id:
    osThreadId activeThreadId = OS_NO_THREAD_ID;
    bool rc2 = getActiveThreadId(activeThreadId);
    GT_IF_WITH_ASSERT(rc2)
    {
        // Verify that the active thread id is valid:
        GT_IF_WITH_ASSERT((activeThreadId != OS_NO_THREAD_ID) || _isDuringFatalSignalSuspension)
        {
            // Get the thread's calls stack:
            pdGDBCallStack* pCallStackData = NULL;
            retVal = updateThreadCallStack(activeThreadId, pCallStackData);

            GT_IF_WITH_ASSERT(retVal)
            {
                // Store the call stack:
                _threadCallStacks[activeThreadId] = pCallStackData;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::updateThreadCallStack
// Description: Get a thread's call stack. This function assumes GDB is suspended
//              (and not in an internal continue)
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        31/1/2010
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::updateThreadCallStack(osThreadId threadId, pdGDBCallStack*& pCallStackData)
{
    bool retVal = false;

    if (threadId == _spiesAPIThreadId)
    {
        // The spy thread in running statce. Ignore update
        retVal = true;
        return retVal;
    }


    // Make sure the pointer we got is clean:
    GT_ASSERT(pCallStackData == NULL);

    // Get the currently active thread:
    osThreadId formerlyActiveThreadId = OS_NO_THREAD_ID;
    bool rcAct = getActiveThreadId(formerlyActiveThreadId);
    GT_IF_WITH_ASSERT(rcAct)
    {
        // If we want a thread other than the active thread, we need to change it:
        if (threadId != formerlyActiveThreadId)
        {
            // Switch to this thread:
            bool rcSetAct = switchGDBActiveThread(threadId);
            GT_ASSERT(rcSetAct);
        }
    }
    else
    {
        bool rcSetAct = _gdbDriver.executeGDBCommand(PD_SET_ACTIVE_THREAD_CMD, gtASCIIString(std::to_string(threadId).c_str()), nullptr);
        GT_ASSERT(rcSetAct);
    }

    bool bSuspend = true;

    // Get the current thread's call stack:
    const pdGDBData* pGDBOutputData = NULL;
    bool rcKill = _gdbDriver.executeGDBCommand(PD_GET_CUR_THREAD_CALL_STACK_CMD, emptyCmdParam, &pGDBOutputData);
    GT_IF_WITH_ASSERT(rcKill && (pGDBOutputData != NULL))
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(pGDBOutputData->type() == pdGDBData::PD_GDB_CALL_STACK_DATA)
        {
            // Get the call stack data:
            pCallStackData = (pdGDBCallStack*)pGDBOutputData;
            GT_IF_WITH_ASSERT(pCallStackData != NULL)
            {
                // Store the thread whose call stack we updated:
                pCallStackData->_callStack.setThreadId(threadId);

                retVal = true;
            }
        }
    }

    // If we changed the active thread, restore it:
    if ((rcAct) && (threadId != formerlyActiveThreadId))
    {
        bool rcResetAct = switchGDBActiveThread(formerlyActiveThreadId);
        GT_ASSERT(rcResetAct);
    }

    if (!bSuspend)
    {
        GT_ASSERT(ReleaseSpyThread());
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::updateThreadCallStackDuringInternalContinue
// Description: Suspends the gdb internal continue then gets the specified thread's
//              call stack
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        1/2/2010
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::updateThreadCallStackDuringInternalContinue(osThreadId threadId, pdGDBCallStack*& pCallStackData)
{
    bool retVal = false;

    // Sanity check - make sure that the debugged process is suspended:
    GT_IF_WITH_ASSERT(_isDebuggedProcssSuspended && _isDuringInternalContinue)
    {
        // Sanity check - make sure that we have the debugged process OS id:
        GT_IF_WITH_ASSERT(_debuggedProcessPid != 0)
        {
            // Tell the gdb driver that we are about to send an internal interrupt (SIGINT):
            _gdbDriver.waitForInternalDebuggedProcessInterrupt();

            // Mark the we are during function execution:
            _isDuringFunctionExecution = true;
            _isDuringInternalContinue = false;
            bool gotStack = false;
            pdGDBCallStack* pCallStack = NULL;

            // Suspend the debugged process run by sending it an interrupt (SIGINT):
            //int rcKill = ::kill(_debuggedProcessPid, SIGINT);
            //GT_IF_WITH_ASSERT(rcKill == 0)
            bool beforeSuspend = false;
            GT_IF_WITH_ASSERT(trySuspendProcess(beforeSuspend))
            {
                // Wait until the gdb listener thread notifies us that the debugged process was suspended:
                //                bool rcWait = waitForDebuggedProcessSuspensionCondition();
                //              GT_IF_WITH_ASSERT(rcWait)
                {
                    bool rcStack = updateThreadCallStack(threadId, pCallStack);
                    GT_IF_WITH_ASSERT(rcStack && (pCallStack != NULL))
                    {
                        gotStack = true;
                    }
                }
            }

            // Mark that the function execution ended:
            _isDuringFunctionExecution = false;

            // Mark that we only continue running the debugged process
            // for internal purposes (to the outside world, the debugged process will still
            // be seen as suspended)
            _isDuringInternalContinue = true;

            // Resume the debugged process execution:
            if (!beforeSuspend)
            {
                bool rc6 = false;

                if (!_isDuringFatalSignalSuspension)
                {
                    if (_isUnderHostBreakpoint)
                    {
                        rc6 = ReleaseSpyThread();
                    }
                    else
                    {
                        rc6 = tryResumeProcess();
                    }
                }

                GT_IF_WITH_ASSERT(rc6)
                {
                    retVal = gotStack;
                    pCallStackData = pCallStack;
                }
            }
            else
            {
                retVal = gotStack;
                pCallStackData = pCallStack;
            }

            // If we failed but got a calls stack, clean up the memory:
            if ((!retVal) && (pCallStack != NULL))
            {
                delete pCallStack;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::clearCallStacksMap
// Description: Clears the map of calls stacks and deletes all the structs holding them
// Author:      Uri Shomroni
// Date:        1/2/2010
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::clearCallStacksMap()
{
    // Go over all key-value pairs:
    gtMap<osThreadId, pdGDBCallStack*>::iterator iter = _threadCallStacks.begin();
    gtMap<osThreadId, pdGDBCallStack*>::iterator endIter = _threadCallStacks.end();

    while (iter != endIter)
    {
        // Delete the current item's calls stack struct:
        delete(*iter).second;

        iter++;
    }

    _threadCallStacks.clear();
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::suspendDebuggedProcessThreads
// Description: Suspends the debugged process threads.
// Author:      Yaki Tebeka
// Date:        4/1/2007
// Implementation notes:
//  Unfortunately, the below code does not seem to run on Linux.
//  Maybe, when GDB will support thread scheduler locking on Linux it will
//  be useful (see set "scheduler-locking" command documentation).
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::suspendDebuggedProcessThreads()
{
    bool retVal = true;

#if (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    // In Mac, we suspend all threads except the API thread, as we do in Windows
    // (note that since this is possible, we don't need to lock the condition inside
    // the OpenGL function calls, so we don't)
    int numOfThreads = amountOfDebuggedProcessThreads();
    osThreadId spyAPIThreadId = spiesAPIThreadId();

    for (int i = 0; i < numOfThreads; i++)
    {
        osThreadId threadId = OS_NO_THREAD_ID;
        bool rcID = getThreadId(i, threadId);
        GT_IF_WITH_ASSERT(rcID)
        {
            // Ignore the API threads:
            if ((threadId != spyAPIThreadId) && !(isDriverThread(threadId)))
            {
                retVal = suspendDebuggedProcessThread(threadId) && retVal;
            }
        }
    }

#elif (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    // Suspending threads is not supported on Linux
    GT_ASSERT(false);
#else
#error Unknown Linux variant!
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::getFunctionNameAtAddress
// Description:
//   Inputs an address in debugged process address space and returns the name
//   of the function that resides in this address.
//
// Arguments: address - The input address.
//            functionName - Will get the name of the function that resides in the
//                           input address. Notice that the name will include a "()" suffix.
//                           Example: "foo()"
//
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/1/2008
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::getFunctionNameAtAddress(osProcedureAddress address, gtString& functionName)
{
    bool retVal = false;

    // If we already know which function resides at the given address:
    gtMap<osProcedureAddress, gtString>::iterator iter = _functionNameAtAddress.find(address);

    if (iter != _functionNameAtAddress.end())
    {
        functionName = iter->second;
        retVal = true;
    }
    else
    {
        // Ask GDB for the name of the symbol that resides in our input address:
        gtASCIIString addressAsString;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        if (_debuggedExecutableArchitecture == OS_I386_ARCHITECTURE)
        {
            addressAsString.appendFormattedString("%p", address);
        }
        else if (_debuggedExecutableArchitecture == OS_X86_64_ARCHITECTURE)
        {
            addressAsString.appendFormattedString("%#018llx", address);
        }
        else
        {
            // Unsupported or unknown architecture, we should not get here!
            GT_ASSERT(false);
        }

#else
        // Just support the same architecture as the one we are running:
        addressAsString.appendFormattedString("%p", address);
#endif
        const pdGDBData* pGDBOutputData = NULL;
        bool rc1 = _gdbDriver.executeGDBCommand(PD_GET_SYMBOL_AT_ADDRESS, addressAsString, &pGDBOutputData);
        GT_IF_WITH_ASSERT(rc1 && (pGDBOutputData != NULL))
        {
            // Downcast the GDB result:
            const pdGDBStringData* pGDBResultString = (const pdGDBStringData*)pGDBOutputData;

            // Verify that the output symbol is a function:
            static const gtASCIIString stat_functionSymbolSuffix("()");
            int functionSuffixPosition = pGDBResultString->_gdbOutputString.find(stat_functionSymbolSuffix);
            GT_IF_WITH_ASSERT(functionSuffixPosition != -1)
            {
                // Output the function name:
                functionName.fromASCIIString(pGDBResultString->_gdbOutputString.asCharArray());

                // Log the function name for future calls to this function:
                _functionNameAtAddress[address] = functionName;

                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::onDebuggedProcessCreationEvent
// Description: Is called when the debugged process is created.
// Arguments:   event - An event representing the process creation.
// Author:      Yaki Tebeka
// Date:        4/1/2007
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::onDebuggedProcessCreationEvent()
{
    // Nothing to be done for now.
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::onDebuggedProcessTerminationEvent
// Description: Is called by the main thread when the debugged process is
//              terminated.
//              Performs clean ups:
// Author:      Yaki Tebeka
// Date:        4/1/2007
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::onDebuggedProcessTerminationEvent()
{
    // Terminate the gdb connection:
    _gdbDriver.terminate();

    // Re-initialize this class:
    initialize();

    m_mapBpFileNameToIndex.clear();
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::onProcessRunStartedEvent
// Description: Is called by the main thread when the debugged process run
//              is started.
// Author:      Yaki Tebeka
// Date:        4/1/2007
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::onProcessRunStartedEvent(const apEvent& eve)
{
    (void)(eve); // unused
    _debuggedProcessExists = true;
    _isDebuggedProcssSuspended = false;

    // On Mac OS X:
#if (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    {
        // Get the debugged process pid:
        _debuggedProcessPid = ((const apDebuggedProcessRunStartedEvent&)eve).processId();

        // On the iPhone simulator, we need to watch if the simulator app exited without exiting cleanly in gdb:
        if (_debuggedProcessCreationData.projectExecutionTarget() == AP_IPHONE_SIMULATOR_EXECUTION_TARGET)
        {
            // If the watcher thread was not terminated, terminate it:
            if (_pWatcherThread != NULL)
            {
                if (_pWatcherThread->isAlive())
                {
                    _pWatcherThread->terminate();
                }

                delete _pWatcherThread;
                _pWatcherThread = NULL;
            }

            // Release thread memory:
            delete _pWatcherThread;

            // Create the new thread:
            _pWatcherThread = new pdDebuggedProcessWatcherThread(_debuggedProcessPid, _gdbDriver);


            // Run a watcher thread to get the event when the process exits:
            _pWatcherThread->execute();
        }
    }
#else
    _debuggedProcessPid = ((const apDebuggedProcessRunStartedEvent&)eve).processId();
#endif
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::onProcessRunSuspendedEvent
// Description: Is called by the main thread when the debugged process run
//              is suspended.
// Author:      Yaki Tebeka
// Date:        4/1/2007
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::onProcessRunSuspendedEvent(const apEvent& eve)
{
    (void)(eve); // unused
    // All handling of suspension and breakpoint event is done at
    // registration time (onProcessRunSuspendedEventRegistration())
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::onProcessRunResumedEvent
// Description: Is called by the main thread when the debugged process run
//              is resumed.
// Author:      Yaki Tebeka
// Date:        4/1/2007
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::onProcessRunResumedEvent()
{
    OS_OUTPUT_DEBUG_LOG(L"Process was resumed", OS_DEBUG_LOG_DEBUG);

    // If we are not during an "internal continue run" of the debugged
    // process:
    if (!_isDuringInternalContinue)
    {
        _isDebuggedProcssSuspended = false;

        // Clear the calls stacks as they are no longer valid:
        clearCallStacksMap();
    }
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::onBreakpointHitEvent
// Description: Is called by the main thread when the debugged process hits
//              a breakpoint.
// Author:      Yaki Tebeka
// Date:        4/1/2007
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::onBreakpointHitEvent(const apEvent& eve)
{
    // Sometimes breakpoint events fire before the process suspension events and sometimes they
    // fire after them, so treat them the same:
    onProcessRunSuspendedEvent(eve);
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::onExceptionEventRegistration
// Description:
//   Is called when an exception event (signal) is registered.
// Arguments: event - A class representing the exception event.
// Author:      Yaki Tebeka
// Date:        1/7/2007
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::onExceptionEventRegistration(const apExceptionEvent& eve)
{
    // If the debugged process is about to die:
    bool isFatalSignal = eve.isFatalLinuxSignal();

    if (isFatalSignal)
    {
        // Mark that the debugged process is suspended due to a debugged process fatal signal:
        _isDebuggedProcssSuspended = true;
        _isDuringFatalSignalSuspension = true;

        // Update the debugged process threads data:
        bool rc1 = updateDebuggedProcessThreadsData();
        GT_ASSERT(rc1);

        // Update the current thread's call stack (this will held identifying the crash reason):
        bool rc2 = updateCurrentThreadCallStack();
        GT_ASSERT(rc2);

        // Set the debugged process active thread as the event triggering thread:
        osThreadId activeThreadId = OS_NO_THREAD_ID;
        getActiveThreadId(activeThreadId);
        ((apExceptionEvent&)eve).setTriggeringThreadId(activeThreadId);
    }
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::onProcessRunSuspendedEventRegistration
// Description: is called when a process run suspension event is about to be registered
// Author:      Uri Shomroni
// Date:        21/4/2009
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::onProcessRunSuspendedEventRegistration(apEvent& eve)
{
    // Output debug log message:
    OS_OUTPUT_DEBUG_LOG(PD_STR_processRunWasSuspended, OS_DEBUG_LOG_DEBUG);

    // If this is the first time I am notified about this suspension:
    if (!_isDebuggedProcssSuspended)
    {
        // Do not perform the internal continue if we are handling a fatal signal:
        if (!_isDuringFatalSignalSuspension)
        {
            // Perform "after debugged process suspension" actions:
            afterGDBProcessSuspensionActions();
        }
    }

    // Mark that the debugged process is suspended:
    _isDebuggedProcssSuspended = true;

    // Get the active thread id:
    osThreadId activeThreadId = OS_NO_THREAD_ID;
    getActiveThreadId(activeThreadId);

    // Set the debugged process active thread as the event triggering thread:
    if (eve.triggeringThreadId() != OS_NO_THREAD_ID)
    {
        auto osId = getOSThreadIdByGDBIndex((int)eve.triggeringThreadId());
        eve.setTriggeringThreadId(osId);
    }
    else
    {
        eve.setTriggeringThreadId(activeThreadId);
    }
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::onThreadCreatedEventRegistration
// Description: Called when a thread created event is registered
// Author:      Uri Shomroni
// Date:        3/2/2010
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::onThreadCreatedEventRegistration(apThreadCreatedEvent& eve, bool& vetoEvent)
{
    // Log the created thread:
    const osThreadId threadOSId = eve.threadOSId();
    logCreatedThread(threadOSId);

    // If this thread is the API thread, it means the API thread is now running:
    if ((_spiesAPIThreadId != OS_NO_THREAD_ID) && (eve.threadOSId() == _spiesAPIThreadId))
    {
        _isAPIThreadRunning = true;
        (void)(vetoEvent); // unused
#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
        {
            // On Mac, we have no other thread created events, so it seems weird that only one thread
            // created event appears:
            vetoEvent = true;
        }
#endif // AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    }
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::logDebuggedProcessPid
// Description:
//  Retrieves the debugged process pid, and stores it into _debuggedProcessPid.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/8/2007
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::logDebuggedProcessPid()
{
    bool retVal = false;

    // Get the debugged process pid:
    const pdGDBData* pGDBOutputData = NULL;
    bool rc1 = _gdbDriver.executeGDBCommand(PD_GET_EXECUTABLE_PID_CMD, emptyCmdParam, &pGDBOutputData);
    GT_IF_WITH_ASSERT(rc1 && (pGDBOutputData != NULL))
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(pGDBOutputData->type() == pdGDBData::PD_GDB_PROCESS_ID)
        {
            // Log the debugged process id:
            _debuggedProcessPid = ((pdGDBProcessId*)pGDBOutputData)->_processPid;

            // Output a debug string:
            gtString dbgMsg = PD_STR_debuggedProcessPID;
            dbgMsg.appendFormattedString(L"%d", _debuggedProcessPid);
            OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::getAPIThreadGDBThreadID
// Description: Outputs the GDB id of the Spy API thread.
// Arguments: apiThreadGDBId - Will get the GDB thread id of the Spy API thread.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        4/1/2007
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::getAPIThreadGDBThreadID(gtString& apiThreadGDBId)
{
    bool retVal = false;

    apiThreadGDBId.makeEmpty();

    // Validity check:
    if (_pDebuggedProcessThreadsData != NULL)
    {
        // Iterate the debugged process threads:
        gtList<pdGDBThreadData>::const_iterator iter = _pDebuggedProcessThreadsData->_threadsDataList.begin();
        gtList<pdGDBThreadData>::const_iterator endIter = _pDebuggedProcessThreadsData->_threadsDataList.end();

        while (iter != endIter)
        {
            // If the current thread is the API thread:
            if ((*iter)._OSThreadId == _spiesAPIThreadId)
            {
                // Output the GDB thread id of the spies API thread:
                int spiesAPIThreadId = (*iter)._gdbThreadId;
                apiThreadGDBId.appendFormattedString(L"%d", spiesAPIThreadId);

                retVal = true;
                break;
            }

            iter++;
        }
    }


    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::getActiveThreadId
// Description: Outputs GDB's active thread id.
// Arguments: activeThreadId - Will get GDB's active thread id.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        4/1/2007
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::getActiveThreadId(osThreadId& activeThreadId)
{
    bool retVal = false;

    activeThreadId = OS_NO_THREAD_ID;

    // Validity check:
    if (_pDebuggedProcessThreadsData != NULL)
    {
        // Iterate the debugged process threads:
        gtList<pdGDBThreadData>::const_iterator iter = _pDebuggedProcessThreadsData->_threadsDataList.begin();
        gtList<pdGDBThreadData>::const_iterator endIter = _pDebuggedProcessThreadsData->_threadsDataList.end();

        while (iter != endIter)
        {
            // If the current thread is GDB's active thread:
            if ((*iter)._isGDBsActiveThread)
            {
                // Output it's OS id:
                activeThreadId = (*iter)._OSThreadId;

                retVal = true;
                break;
            }

            iter++;
        }
    }

    return retVal;
}

///////////////////////////////////////////////////////////////////////////////////
/// \brief Check if host debugging available
///
/// \return true - available, false - not implemented yet
///
/// \author Vadim Entov
/// \date  07/09/2015
bool pdLinuxProcessDebugger::canGetHostVariables() const
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////////
/// \brief Suspend debugged process in case process in running state
///
/// \param[out]  suspendedBefore a process already suspended
/// \param[in] onlyRelevantThrds: if false juset suspend all threads and don't check if driver and spy threads runs
/// \return true - successfull call, false - failed call
///
/// \author Vadim Entov
/// \date   24/09/2015
bool pdLinuxProcessDebugger::trySuspendProcess(bool& suspendedBefore, bool bRelevantThrds)
{
    bool result = false;

    if (!_gdbDriver.IsAllThreadsStopped(bRelevantThrds ? &m_GDBIdsOfThreadsToRelease : nullptr))
        //if (!isDebuggedProcssSuspended())
    {
        suspendedBefore = false;

        if (!_gdbDriver.executeGDBCommand(PD_GDB_SUSPEND_DEBUGGED_PROCESS_CMD, "--all"))
        {
            result = false;
        }
        else
        {
            auto start = std::chrono::system_clock::now();

            do
            {
                auto current = std::chrono::system_clock::now();

                if (std::chrono::duration_cast<std::chrono::milliseconds>(current - start).count() > SUSPEND_PROCESS_WAITING_TIMEOUT)
                {
                    break;
                }

                //std::this_thread::sleep_for(std::chrono::milliseconds(10));
                osSleep(10);

            }
            while (!_gdbDriver.IsAllThreadsStopped());

            bool bKillWasSent = false;

            if (!_gdbDriver.IsAllThreadsStopped())
            {
                ::kill(_debuggedProcessPid, SIGINT);
                bKillWasSent = true;
            }

            while (!_gdbDriver.IsAllThreadsStopped());

            if (bKillWasSent)
            {
                GT_ASSERT(_gdbDriver.FlushPrompt());
            }

            result = true;
        }
    }
    else
    {
        /// In case already suspended thread not need process resume on leave function
        suspendedBefore = true;
        result = true;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////
/// \brief Resume suspended process
///
/// \return true - successfull call, false - failed call
///
/// \author Vadim Entov
/// \date   24/09/2015
bool pdLinuxProcessDebugger::tryResumeProcess()
{
    bool result = true;

    if (_gdbDriver.IsAllThreadsStopped(&m_GDBIdsOfThreadsToRelease))
    {
        result = _gdbDriver.executeGDBCommand(PD_CONTINUE_CMD, " --all");

        if (result)
        {
            //waitForDebuggedProcessSuspensionCondition();
            while (!_gdbDriver.IsAllThreadsRunning());

            _gdbDriver.onDebuggedProcessRunResumed();
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////
/// \brief Convert thread index to gdb thread id
///
/// \param[in]  threadId a id of requested thread
/// \param[out] reference to result value
///
/// \return true - successfull call, false - failed call
///
/// \author Vadim Entov
/// \date  09/012/2015
bool pdLinuxProcessDebugger::getThreadGDBId(osThreadId threadId, int& gdbId) const
{
    bool retVal = false;
    gdbId = -1;

    // Validity check:
    if (_pDebuggedProcessThreadsData != NULL)
    {
        // Iterate the debugged process threads:
        for (auto const& iter : _pDebuggedProcessThreadsData->_threadsDataList)
        {
            // If the current thread is the thread we are looking for:
            if (threadId == iter._OSThreadId)
            {
                gdbId = iter._gdbThreadId;
                retVal = true;
                break;
            }
        }
    }

    return retVal;
}
bool pdLinuxProcessDebugger::getThreadIndexFromId(osThreadId threadId, int& threadIndex) const
{
    bool retVal = false;
    threadIndex = 0;

    // Validity check:
    if (_pDebuggedProcessThreadsData != NULL)
    {
        // Iterate the debugged process threads:
        for (auto const& iter : _pDebuggedProcessThreadsData->_threadsDataList)
        {
            // If the current thread is the thread we are looking for:
            if (threadId == iter._OSThreadId)
            {
                retVal = true;
                break;
            }

            ++threadIndex;
        }
    }

    return retVal;
}


///////////////////////////////////////////////////////////////////////////////////
/// \brief Get locals variables list for specified frame and thhread
///
/// \param[in]  threadId a id of requested thread
/// \param[in]  callStackFrameIndex a index of call stack requested frame
/// \param[out] o_variables a refernce to vector of local variables name
///
/// \return true - successfull call, false - failed call
///
/// \author Vadim Entov
/// \date  07/09/2015
bool pdLinuxProcessDebugger::getHostLocals(osThreadId threadId, int callStackFrameIndex, gtVector<gtString>& o_variables)
{
    gtASCIIString parametersString = "";
    bool suspendBefore = true;
    bool returnResult = false;
    _isDuringGDBSynchronusCommandExecution = true;

    bool rcSuspend = true;

    if (!_isUnderHostBreakpoint && !_isDuringFatalSignalSuspension)
    {
        rcSuspend = trySuspendProcess(suspendBefore);
    }

    GT_IF_WITH_ASSERT(rcSuspend)
    {
        osCallStack realCallStack;
        osCallStack visibleCallStack; // Call stack without spy functions

        GT_IF_WITH_ASSERT(getDebuggedThreadCallStack(threadId, realCallStack, false) && getDebuggedThreadCallStack(threadId, visibleCallStack, true))
        {
            int diff = realCallStack.amountOfStackFrames() - visibleCallStack.amountOfStackFrames();

            GT_ASSERT(diff >= 0);

            /// get locals variables snapshot
            parametersString = "";
            parametersString += " --thread ";

            int threadIndex = 1;

            if (getThreadGDBId(threadId, threadIndex))
            {
                parametersString += std::to_string(threadIndex).c_str();
                parametersString += " --frame ";
                parametersString += std::to_string(callStackFrameIndex + diff).c_str();
                parametersString += " 1";

                pdGDBFrameLocalsData* localsData = nullptr;

                if (_gdbDriver.executeGDBCommand(PD_GET_LOCALS_INFO_CMD, parametersString, (const pdGDBData**)(&localsData)))
                {
                    if (nullptr != localsData)
                    {
                        for (auto& it : localsData->_localsVariables)
                        {
                            o_variables.push_back(it.first);
                        }

                        returnResult = true;
                    }
                }
            }
        }
    }

    if (!suspendBefore)
    {
        returnResult &= tryResumeProcess();
    }

    _isDuringGDBSynchronusCommandExecution = false;
    return returnResult;
}

bool pdLinuxProcessDebugger::getHostVariableValue(osThreadId threadId, int callStackFrameIndex, const gtString& variableName, gtString& o_varValue, gtString& o_varValueHex, gtString& o_varType)
{
    GT_UNREFERENCED_PARAMETER(threadId);
    GT_UNREFERENCED_PARAMETER(callStackFrameIndex);
    GT_UNREFERENCED_PARAMETER(o_varValueHex);
    GT_UNREFERENCED_PARAMETER(o_varType);

    gtASCIIString parametersString = "";
    bool returnResult = false;
    bool suspendBefore = true;

    bool rcSuspend = true;

    if (!_isUnderHostBreakpoint  && !_isDuringFatalSignalSuspension)
    {
        rcSuspend = trySuspendProcess(suspendBefore);
    }

    GT_IF_WITH_ASSERT(rcSuspend)
    {
        osCallStack realCallStack;
        osCallStack visibleCallStack; // Call stack without spy functions

        GT_IF_WITH_ASSERT(getDebuggedThreadCallStack(threadId, realCallStack, false) && getDebuggedThreadCallStack(threadId, visibleCallStack, true))
        {
            int diff = realCallStack.amountOfStackFrames() - visibleCallStack.amountOfStackFrames();

            GT_ASSERT(diff >= 0);

            parametersString += std::to_string(callStackFrameIndex + diff).c_str();

            GT_IF_WITH_ASSERT(_gdbDriver.executeGDBCommand(PD_SET_ACTIVE_FRAME_CMD, parametersString, nullptr))
            {
                pdGDBFrameLocalVariableValue* localsData = nullptr;
                parametersString = variableName.asASCIICharArray();
                GT_IF_WITH_ASSERT(_gdbDriver.executeGDBCommand(PD_GET_LOCAL_VARIABLE_CMD, parametersString, (const pdGDBData**)(&localsData)))
                {
                    if (nullptr != localsData)
                    {
                        o_varValue = localsData->_variableValue;

                        parametersString = variableName.asASCIICharArray();

                        pdGDBFVariableType* typeData = nullptr;
                        GT_IF_WITH_ASSERT(_gdbDriver.executeGDBCommand(PD_GET_VARIABLE_TYPE_CMD, parametersString, (const pdGDBData**)(&typeData)))
                        {
                            if (nullptr != typeData)
                            {
                                o_varType = typeData->_variableType;
                                returnResult = true;
                            }
                        }
                    }
                }
            }

            if (!suspendBefore)
            {
                bool res = tryResumeProcess();

                if (returnResult)
                {
                    returnResult = res;
                }
            }
        }
    }

    return returnResult;
}

///////////////////////////////////////////////////////////////////////////////////
/// \brief Resume spy thread only
///
/// \return true - successfull call, false - failed call
///
/// \author Vadim Entov
/// \date  19/01/2016
bool pdLinuxProcessDebugger::ReleaseSpyThread()
{
    bool result = false;

    if (m_GDBIdsOfThreadsToRelease.size())
    {
        for (auto id : m_GDBIdsOfThreadsToRelease)
        {
            GT_IF_WITH_ASSERT(-1 != id)
            {
                if (!_gdbDriver.IsThreadRunning(id))
                {
                    GT_IF_WITH_ASSERT(switchGDBActiveThread(getOSThreadIdByGDBIndex(id)))
                    {
                        GT_IF_WITH_ASSERT(_gdbDriver.executeGDBCommand(PD_CONTINUE_THREAD_CMD, ""))
                        {
                            while (!_gdbDriver.IsThreadRunning(id));

                            result = true;
                        }
                    }
                }
                else
                {
                    result = true;
                }
            }
        }

        switchGDBActiveThread(m_triggeringThreadId, false);
    }
    else
    {
        result = true;
    }

    return result;
}



// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::getGDBThreadId
// Description: Inputs an OS thread id and returns GDB's id for this thread.
// Arguments: threadId - The input OS thread id.
// Return Val: int - The output GDB thread id, or -1 if GDB does not recognize
//                   the input OS thread.
// Author:      Yaki Tebeka
// Date:        17/1/2008
// ---------------------------------------------------------------------------
int pdLinuxProcessDebugger::getGDBThreadId(const osThreadId& threadId)
{
    int retVal = -1;

    if (NULL != _pDebuggedProcessThreadsData)
    {
        // Iterate the debugged process threads:
        gtList<pdGDBThreadData>::const_iterator iter = _pDebuggedProcessThreadsData->_threadsDataList.begin();
        gtList<pdGDBThreadData>::const_iterator endIter = _pDebuggedProcessThreadsData->_threadsDataList.end();

        while (iter != endIter)
        {
            // If the current thread is GDB's active thread:
            if ((*iter)._OSThreadId == threadId)
            {
                // Output it's OS id:
                retVal = (*iter)._gdbThreadId;
                break;
            }

            iter++;
        }
    }

    return retVal;
}

///////////////////////////////////////////////////////////////////////////////////
/// \brief Get OS thread ID by GDB index
///
/// \param gdbThreadId a gdb thread index
/// \return osThreadId in case success or OS_NO_THREADID
///
/// \author Vadim Entov
/// \date   21/02/2016
osThreadId pdLinuxProcessDebugger::getOSThreadIdByGDBIndex(int  gdbThreadId)
{
    osThreadId retVal = OS_NO_THREAD_ID;

    if (NULL != _pDebuggedProcessThreadsData)
    {
        // Iterate the debugged process threads:
        gtList<pdGDBThreadData>::const_iterator iter = _pDebuggedProcessThreadsData->_threadsDataList.begin();
        gtList<pdGDBThreadData>::const_iterator endIter = _pDebuggedProcessThreadsData->_threadsDataList.end();

        while (iter != endIter)
        {
            // If the current thread is GDB's active thread:
            if ((*iter)._gdbThreadId == gdbThreadId)
            {
                // Output it's OS id:
                retVal = (*iter)._OSThreadId;
                break;
            }

            iter++;
        }
    }

    return retVal;
}


///////////////////////////////////////////////////////////////////////////////////
/// \brief Check if host debugging feature available
///
/// \return true - successfull call, false - failed call
///
/// \author Vadim Entov
/// \date   16/09/2015
bool pdLinuxProcessDebugger::canPerformHostDebugging() const
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////////
/// \brief Get type of breakpoint for specific thread.
///     Check under which kind of breakpoint requested thread was stopped.
///
/// \param[in]  threadId a id of requested thread
///
/// \return true - successfull call, false - failed call
///
/// \author Vadim Entov
/// \date   16/09/2015
bool pdLinuxProcessDebugger::isAtAPIOrKernelBreakpoint(osThreadId threadId) const
{
    GT_UNREFERENCED_PARAMETER(threadId);
    bool result = false;

    if (_isUnderHostBreakpoint)
    {
        result = false;
    }
    else
    {
        result = true;
    }

    return result;
}

apBreakReason pdLinuxProcessDebugger::hostBreakReason() const
{
    return m_hostBreakReason;
}

///////////////////////////////////////////////////////////////////////////////////
/// \brief Set breakpoint on specified cpp code line
///
/// \param[in]  fileName a breakpoint target file name
/// \param[in]  lineNumber a breakpoint target line number
///
/// \return true - successfull call, false - failed call
///
/// \author Vadim Entov
/// \date   16/09/2015
bool pdLinuxProcessDebugger::setHostSourceBreakpoint(const osFilePath& fileName, int lineNumber)
{
    gtASCIIString parametersString = "";
    bool returnResult = false;

    auto keyValue = std::make_pair(fileName, lineNumber);
    pdGDBBreakpointIndex* bpIndex = nullptr;

    auto value = m_mapBpFileNameToIndex.find(keyValue);

    if (m_mapBpFileNameToIndex.end() == value)
    {
        m_mapBpFileNameToIndex[keyValue] = -1;
    }

    if (-1 == m_mapBpFileNameToIndex[keyValue])
    {
        GT_IF_WITH_ASSERT(gdb_state::gdb_initialized_state == _currentGDBState)
        {
            gtString _fileName;

            fileName.getFileNameAndExtension(_fileName);

            parametersString = _fileName.asASCIICharArray();
            parametersString += ":";
            parametersString += std::to_string(lineNumber).c_str();

            if (_debuggedProcessExists)
            {
                _gdbDriver.waitForInternalDebuggedProcessInterrupt();
                _isDuringGDBSynchronusCommandExecution = true;

                bool suspendBefore = true;

                GT_IF_WITH_ASSERT(trySuspendProcess(suspendBefore))
                {
                    returnResult = _gdbDriver.executeGDBCommand(PD_GDB_SET_BREAKPOINT_CMD, parametersString, (const pdGDBData**)(&bpIndex));

                    GT_IF_WITH_ASSERT(returnResult)
                    {
                        _isDuringGDBSynchronusCommandExecution = false;

                        if (!suspendBefore)
                        {
                            returnResult = tryResumeProcess();
                        }
                        else
                        {
                            returnResult = true;
                        }

                        _isDuringGDBSynchronusCommandExecution = false;
                    }
                }
            }
            else
            {
                returnResult = _gdbDriver.executeGDBCommand(PD_GDB_SET_BREAKPOINT_CMD, parametersString, (const pdGDBData**)(&bpIndex));
            }
        }

        if (returnResult)
        {
            GT_IF_WITH_ASSERT(bpIndex)
            {
                m_mapBpFileNameToIndex[keyValue] = bpIndex->m_gdbBreakpointIndex;
            }
        }
    }

    return returnResult;
}

///////////////////////////////////////////////////////////////////////////////////
/// \brief Delete host breakpoint
///
/// \param[in]  fileName a breakpoint full file name
/// \param[in]  lineNumber a breakpoint line number
/// \return true - successfull call, false - failed call
///
/// \author Vadim Entov
/// \date   25/01/2016
bool pdLinuxProcessDebugger::deleteHostSourceBreakpoint(const osFilePath& fileName, int lineNumber)
{
    gtASCIIString parametersString = "";
    bool returnResult = false;

    auto value = m_mapBpFileNameToIndex.find(std::make_pair(fileName, lineNumber));

    if (m_mapBpFileNameToIndex.end() != value)
    {
        if (-1 != value->second)
        {
            if (gdb_state::gdb_initialized_state == _currentGDBState)
            {
                parametersString += std::to_string(value->second).c_str();

                if (_debuggedProcessExists)
                {
                    _gdbDriver.waitForInternalDebuggedProcessInterrupt();
                    _isDuringGDBSynchronusCommandExecution = true;
                    bool suspendBefore = true;

                    GT_IF_WITH_ASSERT(trySuspendProcess(suspendBefore))
                    {
                        returnResult = _gdbDriver.executeGDBCommand(PD_GDB_DELETE_BREAKPOINT_CMD, parametersString, nullptr);

                        _isDuringGDBSynchronusCommandExecution = false;

                        if (!suspendBefore)
                        {
                            returnResult = tryResumeProcess();
                        }

                        _isDuringGDBSynchronusCommandExecution = false;
                    }
                }
                else
                {
                    returnResult = true;
                }
            }
            else
            {
                returnResult = true;
            }
        }
        else
        {
            returnResult = true;
        }

        if (returnResult)
        {
            m_mapBpFileNameToIndex.erase(value);
        }
    }
    else
    {
        returnResult = true;
    }

    return returnResult;
}


///////////////////////////////////////////////////////////////////////////////////
/// \brief Set breakpoint on specific cpp code function
///
/// \param[in]  funcName a breakpoint target function name
///
/// \return true - successfull call, false - failed call
///
/// \author Vadim Entov
/// \date   16/09/2015
bool pdLinuxProcessDebugger::setHostFunctionBreakpoint(const gtString& funcName)
{
    gtASCIIString parametersString = "";
    bool suspendBefore = true;
    bool returnResult = false;

    if (trySuspendProcess(suspendBefore))
    {
        /// get locals variables snapshot
        parametersString = funcName.asASCIICharArray();

        returnResult = _gdbDriver.executeGDBCommand(PD_GDB_SET_BREAKPOINT_CMD, parametersString, nullptr);

        returnResult = tryResumeProcess();
    }

    return returnResult;
}

///////////////////////////////////////////////////////////////////////////////////
/// \brief Do host "step over/step into" debugging action
///
/// \param[in]  threadId a id of requested thread
/// \param[in]  stepType a type of step action
/// \return true - successfull call, false - failed call
///
/// \author Vadim Entov
/// \date   16/09/2015
bool pdLinuxProcessDebugger::performHostStep(osThreadId threadId, StepType stepType)
{
    bool returnResult = false;

    bool before = false;

    gaUnLockDriverThreads();

    if (_isDuringInternalContinue)
    {
        trySuspendProcess(before);
    }

    int gdbId = -1;
    bool rcGDBId = getThreadGDBId(threadId, gdbId);
    GT_IF_WITH_ASSERT(rcGDBId)
    {
        m_triggeringThreadId = threadId;
    }

    osCallStack realCallStack;
    osCallStack visibleCallStack; // Call stack without spy functions
    int diff = 0;

    /*if (_isDuringInternalContinue)
    {
        stackThread = getOSThreadIdByGDBIndex(m_triggeringThreadId);
    }
    else
    {
        stackThread = m_triggeringThreadId;
    }*/

    /// Check if spy thread call stak present
    bool rcShow = getDebuggedThreadCallStack(threadId, realCallStack, false);
    bool rcHide = getDebuggedThreadCallStack(threadId, visibleCallStack, true);
    GT_IF_WITH_ASSERT(rcShow && rcHide)
    {
        diff = realCallStack.amountOfStackFrames() - visibleCallStack.amountOfStackFrames();
    }

    pdGDBCommandId cmdId = PD_LAST_GDB_CMD_INDEX;

    switch (stepType)
    {
        case PD_STEP_IN:
            cmdId = PD_GDB_STEP_INTO_CMD;
            m_lastStepKind = AP_STEP_IN_BREAKPOINT_HIT;
            break;

        case PD_STEP_OVER:
            cmdId = PD_GDB_STEP_OVER_CMD;
            m_lastStepKind = AP_STEP_OVER_BREAKPOINT_HIT;
            break;

        case PD_STEP_OUT:
            cmdId = PD_GDB_STEP_OUT_CMD;
            m_lastStepKind = AP_STEP_OUT_BREAKPOINT_HIT;
            break;

        default:
            returnResult = false;
    }

    GT_IF_WITH_ASSERT(PD_LAST_GDB_CMD_INDEX != cmdId && (0 < gdbId))
    {
        _isDebuggedProcssSuspended = false;

        auto stoppedThreads = _gdbDriver.GetStoppedThreads();

        /// Start all threads except debbugged thread
        for (const int& it : stoppedThreads)
        {
            if (it != gdbId)
            {
                gtASCIIString parameters;
                parameters.appendFormattedString("%d", it);

                GT_IF_WITH_ASSERT(_gdbDriver.executeGDBCommand(PD_SET_ACTIVE_THREAD_CMD, parameters, nullptr))
                {
                    GT_ASSERT(_gdbDriver.executeGDBCommand(PD_CONTINUE_THREAD_CMD, "", nullptr));
                }
            }
        }

        gtASCIIString parameters;
        parameters.appendFormattedString("%d", gdbId);

        pdGDBHostStepErrorInfoIndex* pStepResult = nullptr;

        /// Set active thread as debugged thread
        GT_IF_WITH_ASSERT(_gdbDriver.executeGDBCommand(PD_SET_ACTIVE_THREAD_CMD, parameters, nullptr))
        {
            if (diff == 0)
            {
                /// No spy thread present. Do step in/step over/step out

                returnResult = _gdbDriver.executeGDBCommand(cmdId, "", (const pdGDBData**)&pStepResult);

                if (!returnResult)
                {
                    m_lastStepKind = AP_FOREIGN_BREAK_HIT;
                }
                else
                {
                    if (PD_GDB_STEP_OUT_CMD == cmdId)
                    {
                        if (nullptr != pStepResult)
                        {
                            if (pStepResult->m_gdbErrorInfo.find("not meaningful in the outermost frame") != -1)
                            {
                                GT_IF_WITH_ASSERT(_gdbDriver.executeGDBCommand(PD_CONTINUE_CMD, "--all", nullptr))
                                {
                                    returnResult = true;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                /// Spy thread is present. Set temporary breakpoint desired place

                const osCallStackFrame* frame = nullptr;

                if (cmdId == PD_GDB_STEP_INTO_CMD || cmdId == PD_GDB_STEP_OVER_CMD)
                {
                    /// User try to do step over or step into. Temporary breakpoint will be at next line in the first visible frame
                    frame = visibleCallStack.stackFrame(0);
                }
                else
                {
                    /// User try to do step out. Temporary breakpoint will be at next line in the frame before first visible frame:
                    if (visibleCallStack.amountOfStackFrames() == 1)
                    {
                        /// Only one visible frame is present. (Main?) Just continue runnig.
                        GT_IF_WITH_ASSERT(_gdbDriver.executeGDBCommand(PD_CONTINUE_CMD, "--all", nullptr))
                        {
                            returnResult = true;
                        }
                    }
                    else
                    {
                        frame = visibleCallStack.stackFrame(1);
                    }
                }

                if (!returnResult)
                {
                    GT_IF_WITH_ASSERT(nullptr != frame)
                    {
                        gtString _fileName;

                        frame->sourceCodeFilePath().getFileNameAndExtension(_fileName);

                        gtASCIIString param = _fileName.asASCIICharArray();
                        param += ":";
                        int sourceCodeNumber = frame->sourceCodeFileLineNumber();
                        sourceCodeNumber++;
                        param += std::to_string(sourceCodeNumber).c_str();

                        returnResult = _gdbDriver.executeGDBCommand(PD_GDB_UNTIL_CMD, param, (const pdGDBData**)&pStepResult);
                    }
                }

                /// Start running after temporary breakpoint

                if (!_isUnderHostBreakpoint)
                {
                    _isDebuggedProcssSuspended = true;
                    GT_ASSERT(gaResumeDebuggedProcess());
                    _isDebuggedProcssSuspended = false;
                }

                GT_IF_WITH_ASSERT(_gdbDriver.executeGDBCommand(PD_CONTINUE_CMD, "--all", nullptr))
                {
                    returnResult = true;
                }

            }
        }
    }

    return returnResult;
}



// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::isDriverThread
// Description: Returns true iff the thread is a driver thread.
// Author:      Uri Shomroni
// Date:        2/2/2010
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::isDriverThread(const osThreadId& threadId)
{
    bool retVal = false;

    if (NULL != _pDebuggedProcessThreadsData)
    {
        // Iterate the debugged process threads:
        gtList<pdGDBThreadData>::const_iterator iter = _pDebuggedProcessThreadsData->_threadsDataList.begin();
        gtList<pdGDBThreadData>::const_iterator endIter = _pDebuggedProcessThreadsData->_threadsDataList.end();

        while (iter != endIter)
        {
            // If the current thread is the requested thread:
            if ((*iter)._OSThreadId == threadId)
            {
                // Return its status:
                retVal = (*iter)._isDriverThread;
                break;
            }

            iter++;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::switchGDBActiveThread
// Description: Switches GDB's active thread to a given thread id.
// Arguments: threadId - The thread to be GDB's active thread.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        17/1/2008
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::switchGDBActiveThread(osThreadId threadId, bool bSync)
{
    bool retVal = false;

    // Get GDB's id for the input thread:
    int gbdThreadId = getGDBThreadId(threadId);
    GT_IF_WITH_ASSERT(gbdThreadId != -1)
    {
        // Set GDB's current debugged executable to be the input executable path:
        gtASCIIString gbdThreadIdAsStr;
        gbdThreadIdAsStr.appendFormattedString("%d", gbdThreadId);

        bool rc1 = false;

        if (bSync)
        {
            rc1 = _gdbDriver.executeGDBCommand(PD_SET_ACTIVE_THREAD_CMD, gbdThreadIdAsStr);
        }
        else
        {
            rc1 = _gdbDriver.executeGDBCommand(PD_SET_ACTIVE_THREAD_ASYNC_CMD, gbdThreadIdAsStr);
        }

        if (rc1 && (NULL != _pDebuggedProcessThreadsData))
        {
            // Mark the new active thread:
            gtList<pdGDBThreadData>::iterator iter = _pDebuggedProcessThreadsData->_threadsDataList.begin();
            gtList<pdGDBThreadData>::iterator endIter = _pDebuggedProcessThreadsData->_threadsDataList.end();

            while (iter != endIter)
            {
                pdGDBThreadData& currThreadData = *iter;

                if (currThreadData._gdbThreadId == gbdThreadId)
                {
                    currThreadData._isGDBsActiveThread = true;
                }
                else
                {
                    currThreadData._isGDBsActiveThread = false;
                }

                iter++;
            }

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::outputHiddenSpyCallStack
// Description: Inputs a call stack and outputs a call stack that does not contain
//                    spy related frames.
// Author:      Yaki Tebeka
// Date:        20/3/2007
// ---------------------------------------------------------------------------
void pdLinuxProcessDebugger::outputHiddenSpyCallStack(const osCallStack& inputStack, osCallStack& outputStack)
{
    // Iterate the input stack frames:
    int stackFramesCount = inputStack.amountOfStackFrames();

    for (int i = 0; i < stackFramesCount; i++)
    {
        // Get the current stack frame:
        const osCallStackFrame* pCurrStackFrame = inputStack.stackFrame(i);
        GT_IF_WITH_ASSERT(pCurrStackFrame != NULL)
        {
            // If this is a spy related frame:
            if (pCurrStackFrame->isSpyFunction())
            {
                // Clear the output call stack:
                // (This also removes frames that appear below spy frames, which is the effect we want)
                outputStack.clearStack();
            }
            else
            {
                outputStack.addStackFrame(*pCurrStackFrame);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::waitForDebuggedProcessSuspensionCondition
//
// Description:
//  Is called by the main application thread. Causes the main application thread
//  to wait until the GDB listener thread notifies us that the debugged process was suspended.
//  (See also releaseDebuggedProcessSuspensionCondition).
//
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/1/2008
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::waitForDebuggedProcessSuspensionCondition()
{
    bool retVal = false;

    // Lock the condition:
    bool rc1 = _waitForDebuggedProcessSuspensionCondition.lockCondition();
    GT_IF_WITH_ASSERT(rc1)
    {
        // Wait until the debugged process is suspended:
        bool rc2 = _waitForDebuggedProcessSuspensionCondition.waitForCondition();
        GT_IF_WITH_ASSERT(rc2)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::releaseDebuggedProcessSuspensionCondition
// Description:
//   Is called by the gdb listener thread to notify the main application thread that
//   the debugged process was suspended.
//   (See also waitForDebuggedProcessSuspensionCondition)
//
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/1/2008
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::releaseDebuggedProcessSuspensionCondition()
{
    bool retVal = false;

    // If the condition is not locked:
    bool isConditionLocked = _waitForDebuggedProcessSuspensionCondition.isConditionLocked();

    while (!isConditionLocked)
    {
        // Wait until the condition is locked:
        bool dummyFlag = true;
        osWaitForFlagToTurnOff(dummyFlag, 200);
        isConditionLocked = _waitForDebuggedProcessSuspensionCondition.isConditionLocked();
    }

    // Unlock the condition on which the main application thread waits:
    bool rc1 = _waitForDebuggedProcessSuspensionCondition.unlockCondition();
    GT_IF_WITH_ASSERT(rc1)
    {
        // Signal the main application thread to notify it to check the condition status:
        bool rc2 = _waitForDebuggedProcessSuspensionCondition.signalAllThreads();
        GT_IF_WITH_ASSERT(rc2)
        {
            retVal = true;
        }
    }

    return retVal;
}

///////////////////////////////////////////////////////////////////////
/// \brief Prepare current debugged process to terminate.
///
/// \return true - success, false - fail
/// \author Vadim Entov
/// \date 01/02/2016
bool pdLinuxProcessDebugger::prepareProcessToTerminate()
{
    bool result = false;

    if (_gdbDriver.IsAllThreadsStopped(&m_GDBIdsOfThreadsToRelease))
    {
        for (auto& it : m_mapBpFileNameToIndex)
        {
            GT_ASSERT(deleteHostSourceBreakpoint(it.first.first, it.first.second));
        }

        _gdbDriver.StartBackgoundGDBListen();
    }

    return result;
}

///////////////////////////////////////////////////////////////////////
/// \brief Suspend process.
///
/// \return true - success, false - fail
/// \author Vadim Entov
/// \date 09/02/2016
bool pdLinuxProcessDebugger::suspendHostDebuggedProcess()
{
    bool result = false;
    bool bSuspendedBefore = false;

    gaLockDriverThreads();
    result = trySuspendProcess(bSuspendedBefore);

    GT_IF_WITH_ASSERT(result)
    {
        _isDebuggedProcssSuspended = true;
        _isDuringInternalContinue = true;
        _isUnderHostBreakpoint = true;
        m_hostBreakReason = AP_BREAK_COMMAND_HIT;

        GT_IF_WITH_ASSERT(updateDebuggedProcessThreadsData())
        {

            GT_IF_WITH_ASSERT(ReleaseSpyThread())
            {
                apBreakpointHitEvent* pBPEvent = new apBreakpointHitEvent(1, NULL);
                pBPEvent->setBreakReason(apBreakReason::AP_BREAK_COMMAND_HIT);

                apEventsHandler::instance().registerPendingDebugEvent(*pBPEvent);

                apDebuggedProcessRunSuspendedEvent* suspendEvent = new apDebuggedProcessRunSuspendedEvent(1, true);

                apEventsHandler::instance().registerPendingDebugEvent(*suspendEvent);
            }
        }
    }

    return true;
}




#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT

// This typedef is used in the below function to request the function from the library:
typedef bool (* PFNPDLAUNCHIPHONESIMULATORWITHAPPLICATION)(const gtString& applicationPath, const gtString& sdkRootPath, const gtVector<osEnvironmentVariable>& environment);

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::loadProcessDebuggerESLauncher
// Description: Loads the process debugger ES launcher, and gets the function
//              address pdLaunchiPhoneSimulatorWithApplication.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        23/9/2009
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::loadProcessDebuggerESLauncher()
{
    static bool stat_retVal = false;
    static bool triedToGetESLauncher = false;

    // Only try once, if the library can't be loaded, this machine doesn't have the iPhone Simulator:
    if (!triedToGetESLauncher)
    {
        // Construct the Module's file path by adding the module name to the binaries dir:
        osFilePath thisAppPath;
        bool rcApp = osGetCurrentApplicationPath(thisAppPath);

        gtString thisAppExt;
        thisAppPath.getFileExtension(thisAppExt);
        osFilePath thisAppBinariesDir = thisAppPath;

        // If this is a bundle, get the path to the binary inside:
        if (thisAppPath.isDirectory() && thisAppExt == OS_MAC_APPLICATION_BUNDLE_FILE_EXTENSION)
        {
            thisAppBinariesDir = osGetExecutableFromMacApplicationBundle(thisAppPath.asString());
        }

        // Get the dir itsself:
        thisAppBinariesDir.clearFileExtension();
        thisAppBinariesDir.clearFileName();

        // Calculate the bundle path:
        gtString processDebuggerESLauncherModulePath = thisAppBinariesDir.asString();

        // Make sure this only has one separator at the end:
        processDebuggerESLauncherModulePath.removeTrailing(osFilePath::osPathSeparator);
        processDebuggerESLauncherModulePath.append(osFilePath::osPathSeparator).append(PD_ES_LAUNCHER_MODULE_NAME);

        // Get the module handle if possible:
        osModuleHandle hProcessDebuggerESLauncherModule = NULL;

        bool rcModule = osLoadModule(processDebuggerESLauncherModulePath, hProcessDebuggerESLauncherModule);

        // Do not assert, we allow this to fail (for machines where the iPhone Simulator is not present:
        if (rcModule)
        {
            osGetProcedureAddress(hProcessDebuggerESLauncherModule, PD_LAUNCH_IPHONE_SIMULATOR_WITH_APP_FUNCTION_NAME, _pESLauncherHelperFunction);

            // We only succeed if we got here and got the pointer:
            stat_retVal = (_pESLauncherHelperFunction != NULL);
        }

        triedToGetESLauncher = true;
    }

    return stat_retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxProcessDebugger::launchApplicationWithiPhoneSimulator
// Description: Launched the debugged application with the iPhone simulator
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/6/2009
// ---------------------------------------------------------------------------
bool pdLinuxProcessDebugger::launchApplicationWithiPhoneSimulator(const osFilePath& executablePath, const apDebugProjectSettings& processCreationData)
{
    bool retVal = false;

    PFNPDLAUNCHIPHONESIMULATORWITHAPPLICATION pPDLaunchiPhoneSimulatorWithApplication = (PFNPDLAUNCHIPHONESIMULATORWITHAPPLICATION)_pESLauncherHelperFunction;

    // We shouldn't get here if the function was not found
    GT_IF_WITH_ASSERT(pPDLaunchiPhoneSimulatorWithApplication != NULL)
    {
        // Get the SDK root path (by using the fact that the ES framework is {SDKROOT}/Library/Frameworks/OpenGLES.framework/):
        osFilePath iphoneSDKRootPath(osGetOpenGLESFrameworkPath());
        osDirectory iphoneSDKRootDir;
        iphoneSDKRootPath.getFileDirectory(iphoneSDKRootDir);
        iphoneSDKRootDir.upOneLevel().upOneLevel().upOneLevel();

        // Make a vector out of the environment, so we can add any needed items that are missing:
        gtVector<osEnvironmentVariable> environment;
        gtList<osEnvironmentVariable>::const_iterator iter = processCreationData.environmentVariables().begin();
        gtList<osEnvironmentVariable>::const_iterator endIter = processCreationData.environmentVariables().end();

        bool wasDyldLibraryPathSet = false;
        bool wasDyldFrameworkPathSet = false;
        bool wasDyldInsertLibrariesSet = false;

        while (iter != endIter)
        {
            osEnvironmentVariable currentEnvVar = *iter;
            gtString varName = currentEnvVar._name;

            if (varName == s_ldLibraryPathEnvVariableName)
            {
                wasDyldLibraryPathSet = true;
                processLibraryPathEnvVariableValue(currentEnvVar._value);
            }
            else if (varName == s_ldPreloadEnvVariableName)
            {
                wasDyldInsertLibrariesSet = true;
                processPreloadEnvVariableValue(currentEnvVar._value);
            }
            else if (varName == s_dyldFrameworkPathEnvVariableName)
            {
                wasDyldFrameworkPathSet = true;
                processFrameworkPathEnvVariableValue(currentEnvVar._value);
            }

            if (varName != s_dyldFrameworkPathEnvVariableName)
            {
                environment.push_back(currentEnvVar);
            }

            iter++;
        }

        if (!wasDyldLibraryPathSet)
        {
            osEnvironmentVariable dummyEnvVar;
            dummyEnvVar._name = s_ldLibraryPathEnvVariableName;
            processLibraryPathEnvVariableValue(dummyEnvVar._value);
            environment.push_back(dummyEnvVar);
        }

        if (!wasDyldFrameworkPathSet)
        {
            osEnvironmentVariable dummyEnvVar;
            dummyEnvVar._name = s_dyldFrameworkPathEnvVariableName;
            processFrameworkPathEnvVariableValue(dummyEnvVar._value);
            environment.push_back(dummyEnvVar);
        }

        if (!wasDyldInsertLibrariesSet)
        {
            osEnvironmentVariable dummyEnvVar;
            dummyEnvVar._name = s_ldPreloadEnvVariableName;
            processPreloadEnvVariableValue(dummyEnvVar._value);
            environment.push_back(dummyEnvVar);
        }

        // Launch the debugged app on the iPhone Simulator:
        retVal = pPDLaunchiPhoneSimulatorWithApplication(executablePath.asString(), iphoneSDKRootDir.directoryPath().asString(), environment);

        if (retVal)
        {
            // Get the process creation time:
            osTime processCreationTime;
            processCreationTime.setFromCurrentTime();

            // Trigger "process created" event:
            apDebuggedProcessCreatedEvent processCreatedEvent(_debuggedProcessCreationData, processCreationTime, NULL);
            apEventsHandler::instance().registerPendingDebugEvent(processCreatedEvent);

            /*      // Clean up the old watcher thread if it still exists:
            delete _pLauncherProcessWatcherThread;

            // Start a launcher watcher thread to monitor the iPhone simulator:
            _pLauncherProcessWatcherThread = new pdLauncherProcessWatcherThread(childProcessId);
            _pLauncherProcessWatcherThread->execute();*/
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // Uri, 27/8/09: we Found a way to run the debugged process directly inside
    // the iPhone Simulator, using a private framework. Should Apple change the
    // API, this below code (which runs the iPhone Simulator) is left here:
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    /***********************************************************************
    int envVarsListLength = processCreationData.environmentVariables().length();
    const char** pEnv = new const char*[envVarsListLength + 4];
    gtString* pEnvAsGTStrings = new gtString[envVarsListLength + 3];



    pEnv[envVarsListLength] = NULL;
    pEnv[envVarsListLength + 1] = NULL;
    pEnv[envVarsListLength + 2] = NULL;
    pEnv[envVarsListLength + 3] = NULL;
    int i = 0;

    // Set the environment variables on the child process:
    gtList<osEnvironmentVariable>::const_iterator iter = processCreationData.environmentVariables().begin();
    gtList<osEnvironmentVariable>::const_iterator endIter = processCreationData.environmentVariables().end();

    while (iter != endIter)
    {
        gtString currentVariable;
        processEnvVariableToString(*iter, currentVariable);
        if (i < envVarsListLength)
        {
            pEnvAsGTStrings[i] = currentVariable;
            pEnv[i] = pEnvAsGTStrings[i].asCharArray();
            i++;

            gtString varName = (*iter)._name;
            if (varName == s_ldLibraryPathEnvVariableName)
            {
                wasDyldLibraryPathSet = true;
            }
            else if (varName == s_ldPreloadEnvVariableName)
            {
                wasDyldInsertLibrariesSet = true;
            }
            else if (varName == s_dyldFrameworkPathEnvVariableName)
            {
                wasDyldFrameworkPathSet = true;
            }
        }

        iter++;
    }

    if (!wasDyldLibraryPathSet)
    {
        gtString dyldLibraryPathString;
        osEnvironmentVariable dummyEnvValue;
        dummyEnvValue._name = s_ldLibraryPathEnvVariableName;
        processEnvVariableToString(dummyEnvValue, dyldLibraryPathString);
        pEnvAsGTStrings[i] = dyldLibraryPathString;
        pEnv[i] = pEnvAsGTStrings[i].asCharArray();
        i++;
    }

    if (!wasDyldFrameworkPathSet)
    {
        gtString dyldFrameworkPathString;
        osEnvironmentVariable dummyEnvValue;
        dummyEnvValue._name = s_dyldFrameworkPathEnvVariableName;
        processEnvVariableToString(dummyEnvValue, dyldFrameworkPathString);
        pEnvAsGTStrings[i] = dyldFrameworkPathString;
        pEnv[i] = pEnvAsGTStrings[i].asCharArray();
        i++;
    }

    if (!wasDyldInsertLibrariesSet)
    {
        gtString dyldInsertLibrariesString;
        osEnvironmentVariable dummyEnvValue;
        dummyEnvValue._name = s_ldPreloadEnvVariableName;
        processEnvVariableToString(dummyEnvValue, dyldInsertLibrariesString);
        pEnvAsGTStrings[i] = dyldInsertLibrariesString;
        pEnv[i] = pEnvAsGTStrings[i].asCharArray();
        i++;
    }

    pEnv[i] = NULL;

    // The -W switch causes the "open" application to keep running until the iPhone simulator exits.
    // We use this to identify when does the simulator exits.
    const char* args[1];
    gtString iPhoneSimulatorExecutableAsString = osGetExecutableFromMacApplicationBundle(OS_IPHONE_SIMULATOR_APP_PATH);

    args[0] = iPhoneSimulatorExecutableAsString.asCharArray();

    // Run the iPhone Simulator:
    pid_t childProcessId = ::fork();
    if (childProcessId == 0)
    {
        //::execvpe(args[0], (char* const*)args, pEnv);
        ::execle(args[0], args[0], NULL, pEnv);

        // We're not supposed to get here:
        ::exit (-1);
    }
    else
    {
        retVal = (childProcessId != -1);

        if (retVal)
        {
            // Get the process creation time:
            osTime processCreationTime;
            processCreationTime.setFromCurrentTime();

            // Trigger "process created" event:
            apDebuggedProcessCreatedEvent processCreatedEvent(_debuggedProcessCreationData, processCreationTime, NULL);
            apEventsHandler::instance().handleDebugEvent(processCreatedEvent);

            // Clean up the old watcher thread if it still exists:
            delete _pLauncherProcessWatcherThread;

            // Start a launcher watcher thread to monitor the iPhone simulator:
            _pLauncherProcessWatcherThread = new pdLauncherProcessWatcherThread(childProcessId);
            _pLauncherProcessWatcherThread->execute();
        }
    }

    delete[] pEnv;
    delete[] pEnvAsGTStrings;
    ***********************************************************************/

    return retVal;
}


#endif // AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT

