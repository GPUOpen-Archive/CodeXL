//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWin32ProcessDebugger.cpp
///
//==================================================================================

//------------------------------ pdWin32ProcessDebugger.cpp ------------------------------


// About threads and their synchronization:
// ---------------------------------------
// General:
//  The pdWin32ProcessDebugger class uses 2 threads for its work:
//  a. A debugger thread - a thread that listens to debugged process events.
//     When a debugged process event arrives - it stores them in a _pendingEvents queue
//  b. The main CodeXL thread - pops the events from the _pendingEvents queue and
//     handles them.
//     (See also "Notification mechanism overview" at the top of pdProcessDebugger.cpp)
//
// Thread actions conflicts:
//  The debugger thread calls functions that operate on the debugged process - launch and
//  terminate it, read its data, allocate and write into its address space, etc.
//  This means that the debugged and the main thread should have conflict mainly in the
//  following areas:
//  - Launch and terminate the debugged process.
//  - Suspend / resume the debugged process.
//  - The _pendingEvents queue.
//
// Resolving these conflicts:
// There are two simple "mechanisms" we use to synchronize between these two threads:
// 1. The _pendingEvents queue is a thread safe queue.
// 2. The fact that this class can choose whether to act on debugger / main thread
//    is a powerful thread synchronization mechanism.
//    - handleDebuggedProcessEvent (or its descendants) - Is called by the debugger thread.
//    - handlePendingDebugEvent (or its descendants) - Is called by the main thread.
//
//    For example:
//    - When a breakpoint is hit -
//      - The debugger thread suspends all the debugged process threads, except the
//        API handling thread.
//      - The main thread resumes the debugged process run only AFTER the event handling
//        was processed by the CodeXL application.


// Ignore warnings:
#pragma warning( disable : 4786)

// C / C++:
#include <string.h>

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <tchar.h>
#include <mbstring.h>
#include <psapi.h>
#include <wchar.h>

// The name of the environment variable that contains the machine / current process path:
#define PD_PATH_ENV_VARIABLE_NAME L"PATH"
#define PD_DEBUG_HEAP_ENV_VARIABLE_NAME L"_NO_DEBUG_HEAP"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osSystemError.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osMutexLocker.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osSynchronizationObjectLocker.h>
#include <AMDTOSWrappers/Include/osWin32DebugInfoReader.h>
#include <AMDTOSWrappers/Include/osWin32CallStackReader.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessDetectedErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunStartedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunResumedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleLoadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleUnloadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOutputDebugStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreationFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apUserWarningEvent.h>

// Local:
#include <src/pdWin32DebuggerThread.h>
#include <src/pdWin32ProcessDebugger.h>
#include <src/pdStringConstants.h>
#include <AMDTProcessDebugger/Include/pdWin32SetRemoteProcessDLLDirectory.h>

// Represents an unknown index:
#define PD_UNKNOWN_INDEX -1

// The maximal amount of output string printouts:
#define PD_MAX_OUTPUT_STRING_PRINTOUTS 500

// -----------------------------------------------------------------------
//  !!      Functions that are used by the application thread      !!
// -----------------------------------------------------------------------



// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::pdWin32ProcessDebugger
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        9/11/2003
// ---------------------------------------------------------------------------
pdWin32ProcessDebugger::pdWin32ProcessDebugger()
    : pdProcessDebugger(), _pDebuggerThread(NULL), _pProcessCreationData(NULL), _pProcessInfo(NULL), _isIgnoringAMD_OCL_BUILD_OPTIONS(false)
{
    // Is this the first call to this function ?
    static bool isFirstTime = true;

    // On the first call to this function:
    if (isFirstTime)
    {
        // Add the installation directory to my path. The debugged process inherits my
        // environment block and therefore also this path addition:
        bool rc = addInstallDirToCurrentProcessPath();
        GT_ASSERT_EX(rc, PD_STR_FailedToAddInstallDirToPath);

        isFirstTime = false;
    }

    // Initialize the members of this class:
    initialize();
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::~pdWin32ProcessDebugger
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        9/11/2003
// ---------------------------------------------------------------------------
pdWin32ProcessDebugger::~pdWin32ProcessDebugger()
{
    if (_debuggedProcessExist)
    {
        terminateDebuggedProcess();
    }

    // Delete the process creation data:
    if (_pProcessCreationData != NULL)
    {
        delete _pProcessCreationData;
        _pProcessCreationData = NULL;
    }

    // Delete the debugger thread:
    if (_pDebuggerThread != NULL)
    {
        delete _pDebuggerThread;
        _pDebuggerThread = NULL;
    }

    // Delete the process info:
    if (_pProcessInfo != NULL)
    {
        delete _pProcessInfo;
        _pProcessInfo = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////
/// \brief Do host debugger (gdb, VS, etc..) initialization prerequesites
///
/// \param processCreationData a data needed for the process debugger creation and initialization
/// \return true - success, false - failed
/// \author Vadim Entov
/// \date 21/01/2016
bool pdWin32ProcessDebugger::initializeDebugger(const apDebugProjectSettings& processCreationData)
{
    // Hold a copy of the process creation data:
    _pProcessCreationData = new apDebugProjectSettings(processCreationData);

    return (nullptr != _pProcessCreationData);
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::launchDebuggedProcess
// Description: Launches the debugged process.
// Arguments:   processCreationData - Data needed for the process creation
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/11/2003
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::launchDebuggedProcess()
{
    // Launch the debugged process:
    bool rc = launchDebuggedProcessInSuspendedMode();

    if (rc)
    {
        // We are performing one-step launching, so call the second part as well::
        rc = continueDebuggedProcessFromSuspendedCreation();
    }

    return rc;
}
// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::doesSupportTwoStepLaunching
// Description: This process debugger implements the launchDebuggedProcessInSuspendedMode
//              and continueDebuggedProcessFromSuspendedCreation functions.
// Author:      Uri Shomroni
// Date:        19/9/2010
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::doesSupportTwoStepLaunching() const
{
    return true;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::launchDebuggedProcessInSuspendedMode
// Description: Creates the debugged process but does not start its run. When
//              this function returns, debuggedProcessId() has a valid value.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        19/9/2010
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::launchDebuggedProcessInSuspendedMode()
{
    bool rc = false;

    GT_IF_WITH_ASSERT(nullptr != _pProcessCreationData)
    {
        // Create the process info struct:
        _pProcessInfo = new PROCESS_INFORMATION;
        ::memset(_pProcessInfo, 0, sizeof(PROCESS_INFORMATION));

        // Create the debugger thread, which will launch the debugged process:
        _pDebuggerThread = new pdWin32DebuggerThread(*this);

        // Clear process breakpoint flags:
        _wasProcessCreationBreakPointHit = false;
        _wasMainThreadEntryPointBreakpointHit = false;

        bool rc64Bit = osIs64BitModule(_pProcessCreationData->executablePath(), _isDebugging64BitApplication);
        GT_ASSERT(rc64Bit);

        // Clear process related data:
        _mainThreadEntryPointAddress = NULL;
        _mainThreadEntryPointOpCode = 0;

        // Clear thread ids:
        _spiesAPIThreadId = OS_NO_THREAD_ID;
        _mainThreadId = OS_NO_THREAD_ID;

        // Launch the debugged process and wait for the creation process to finish:
        _waitingForDebuggedProcessCreation = true;
        rc = _pDebuggerThread->execute();
        GT_IF_WITH_ASSERT(rc)
        {
            // Wait up to 30 seconds:
            rc = osWaitForFlagToTurnOff(_waitingForDebuggedProcessCreation, 30 * 1000);
        }
    }

    return rc;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::continueDebuggedProcessFromSuspendedCreation
// Description: Completes the debugged process launching after
//              launchDebuggedProcessInSuspendedMode, by starting the debugged
//              process run.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        19/9/2010
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::continueDebuggedProcessFromSuspendedCreation()
{
    bool retVal = true;

    // Release the waiting flag:
    _waitingAtDebuggedProcessLaunch = false;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::debuggedProcessExists
// Description: Returns true iff there is a launched debugged process.
// Author:      Yaki Tebeka
// Date:        16/11/2003
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::debuggedProcessExists() const
{
    return _debuggedProcessExist;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::debuggedProcessCreationData
// Description: Returns the debugged process creation data.
// Author:      Yaki Tebeka
// Date:        16/11/2003
// ---------------------------------------------------------------------------
const apDebugProjectSettings* pdWin32ProcessDebugger::debuggedProcessCreationData() const
{
    return _pProcessCreationData;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::terminateDebuggedProcess
// Description: Terminates the debugged process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/11/2003
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::terminateDebuggedProcess()
{
    bool retVal = false;

    if (_pProcessInfo != NULL)
    {
        // Mark that the API thread is no longer active:
        _spiesAPIThreadId = OS_NO_THREAD_ID;

        // Terminates the debugged process:
        int rc = TerminateProcess(_pProcessInfo->hProcess, 0 /* = The process exit code */);

        retVal = (rc != 0);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::isDebugging64BitApplication
// Description: Query whether the debugged application is a 64-bit application.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        21/9/2009
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::isDebugging64BitApplication(bool& is64Bit) const
{
    bool retVal = true;

    is64Bit = _isDebugging64BitApplication;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::amountOfDebuggedProcessThreads
// Description: Returns the amount of debugged process threads.
// Author:      Yaki Tebeka
// Date:        8/5/2005
// ---------------------------------------------------------------------------
int pdWin32ProcessDebugger::amountOfDebuggedProcessThreads() const
{
    int retVal = 0;

    // Lock the mutex that controls the access to the threads list:
    osMutexLocker mutexLocker(((pdWin32ProcessDebugger*)this)->_threadsListAccessMutex);

    retVal = _debuggedProcessThreadsData.length();

    // Unlock the mutex that controls the access to the threads list:
    mutexLocker.unlockMutex();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::getBreakpointTriggeringThreadIndex
// Description: Returns the index of thread that triggered the current breakpoint.
// Author:      Uri Shomroni
// Date:        28/3/2016
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::getBreakpointTriggeringThreadIndex(int& index) const
{
    bool retVal = false;

    if (OS_NO_THREAD_ID != _breakpointTriggeringThreadId)
    {
        osMutexLocker mutexLocker(((pdWin32ProcessDebugger*)this)->_threadsListAccessMutex);

        int threadIdx = 0;

        for (const auto& iter : _debuggedProcessThreadsData)
        {
            if (iter._threadId == _breakpointTriggeringThreadId)
            {
                index = threadIdx;
                retVal = true;
                break;
            }

            ++threadIdx;
        }

        mutexLocker.unlockMutex();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::getThreadId
// Description: Inputs a thread index and returns its id.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        8/5/2005
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::getThreadId(int threadIndex, osThreadId& threadId) const
{
    bool retVal = false;
    threadId = OS_NO_THREAD_ID;

    // Lock the mutex that controls the access to the threads list:
    osMutexLocker mutexLocker(((pdWin32ProcessDebugger*)this)->_threadsListAccessMutex);

    // Iterate the debugged process threads list:
    gtList<pdDebuggedProcessThreadData>::const_iterator endIter = _debuggedProcessThreadsData.end();
    gtList<pdDebuggedProcessThreadData>::const_iterator iter = _debuggedProcessThreadsData.begin();
    int currentThreadIndex = 0;

    while (iter != endIter)
    {
        // If we reached the queried index:
        if (currentThreadIndex == threadIndex)
        {
            // Output the thread id:
            DWORD currentThreadId = (*iter)._threadId;
            threadId = currentThreadId;

            retVal = true;
            break;
        }

        currentThreadIndex++;
        iter++;
    }

    // Unlock the mutex that controls the access to the threads list:
    mutexLocker.unlockMutex();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::setSpiesAPIThreadId
// Description:
//   Sets the Spy API thread id.
//   (The Spy API thread is created by the Spy dll and serves API function calls)
// Arguments:   spiesAPIThreadId - The ID of the Spy API thread.
// Author:      Yaki Tebeka
// Date:        8/6/2004
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::setSpiesAPIThreadId(osThreadId spiesAPIThreadId)
{
    // Log the spy API thread id:
    _spiesAPIThreadId = spiesAPIThreadId;

    // Output debug log message:
    gtString dbgMsg = PD_STR_gotOGLServerAPIThreadId;
    dbgMsg.appendFormattedString(L": %u", _spiesAPIThreadId);
    OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::spiesAPIThreadIndex
// Description: Returns the spies API thread index (see setSpiesAPIThreadId).
// Author:      Yaki Tebeka
// Date:        11/9/2005
// ---------------------------------------------------------------------------
int pdWin32ProcessDebugger::spiesAPIThreadIndex() const
{
    int retVal = -1;

    // Lock the mutex that controls the access to the threads list:
    osMutexLocker mutexLocker(((pdWin32ProcessDebugger*)this)->_threadsListAccessMutex);

    // Iterate the debugged process threads list:
    gtList<pdDebuggedProcessThreadData>::const_iterator endIter = _debuggedProcessThreadsData.end();
    gtList<pdDebuggedProcessThreadData>::const_iterator iter = _debuggedProcessThreadsData.begin();
    int currentIndex = 0;

    while (iter != endIter)
    {
        // If the current thread is the Spy API thread:
        DWORD currentThreadId = (*iter)._threadId;

        if (currentThreadId == _spiesAPIThreadId)
        {
            retVal = currentIndex;
            break;
        }

        currentIndex++;
        iter++;
    }

    // Unlock the mutex that controls the access to the threads list:
    mutexLocker.unlockMutex();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::mainThreadId
// Description: Returns the debugged application main thread id, or OS_NO_THREAD_ID
//              if it does not exist.
// Author:      Yaki Tebeka
// Date:        11/9/2005
// ---------------------------------------------------------------------------
osThreadId pdWin32ProcessDebugger::mainThreadId() const
{
    return _mainThreadId;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::spiesAPIThreadId
// Description:
//   Returns the spy spies API thread Id, or 0 is the spy api thread id is not
//   known (see comment at setSpiesAPIThreadId).
// Author:      Yaki Tebeka
// Date:        8/6/2004
// ---------------------------------------------------------------------------
osThreadId pdWin32ProcessDebugger::spiesAPIThreadId() const
{
    return _spiesAPIThreadId;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::debuggedProcessId
// Description: Returns the debugged process ID.
// Author:      Uri Shomroni
// Date:        16/9/2010
// ---------------------------------------------------------------------------
osProcessId pdWin32ProcessDebugger::debuggedProcessId() const
{
    osProcessId retVal = 0;

    if (_pProcessInfo != NULL)
    {
        retVal = _pProcessInfo->dwProcessId;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::isSpiesAPIThreadRunning
// Description: Returns true iff the Spy API thread is running.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/7/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::isSpiesAPIThreadRunning() const
{
    return (spiesAPIThreadIndex() != -1);
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::suspendDebuggedProcess
// Description:
//   Suspends all the debugged process threads, except the Spy
//   API handling thread.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        8/6/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::suspendDebuggedProcess()
{
    // Suspend the debugged process run:
    bool retVal = suspendDebuggedProcess(OS_NO_THREAD_ID);

    if (retVal)
    {
        // Notify observers that the debugged process run was suspended:
        apDebuggedProcessRunSuspendedEvent processSuspendedEvent(OS_NO_THREAD_ID);
        apEventsHandler::instance().registerPendingDebugEvent(processSuspendedEvent);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::resumeDebuggedProcess
// Description: Resumes the run of a suspended debugged process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        20/5/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::resumeDebuggedProcess()
{
    bool retVal = true;

    // This is used in releasing the kernel debugging thread:
    osThreadId resumingFromThreadId = _breakpointTriggeringThreadId;

    // We will no longer be suspended:
    _breakpointTriggeringThreadId = OS_NO_THREAD_ID;

    // If the debugged process threw a second change exception - we will terminate
    // it now:
    if (_isDuringSecondChangeExceptionHandling)
    {
        terminateDebuggedProcess();
    }
    else
    {
        // Mark the debugged process run as resumed (not suspended):
        _isDebuggedProcssSuspended = false;

        // Notify observers that the debugged process run is resumed:
        apDebuggedProcessRunResumedEvent processResumedEvent;
        apEventsHandler::instance().registerPendingDebugEvent(processResumedEvent);

        // Verify that we are not during a function execution:
        osSynchronizationObjectLocker syncObjLocker(_executedFuncSyncObj);

        // Lock the mutex that controls the access to the threads list:
        osMutexLocker mutexLocker(_threadsListAccessMutex);

        // Iterate on the debugged process threads:
        gtList<pdDebuggedProcessThreadData>::const_iterator endIter = _debuggedProcessThreadsData.end();
        gtList<pdDebuggedProcessThreadData>::const_iterator iter = _debuggedProcessThreadsData.begin();

        while (iter != endIter)
        {
            // If the current thread is not a Spy API thread:
            osThreadId currentThreadId = (*iter)._threadId;
            bool isAPIThreadId = isSpyAPIThread(currentThreadId);

            if (!isAPIThreadId)
            {
                // If it is not a driver thread:
                // (See case and 1461, 1616)
                bool isKernelDebuggingThread = (_isDuringKernelDebugging && ((*iter)._threadId == resumingFromThreadId));

                if (!((*iter)._isDriverThread) || isKernelDebuggingThread)
                {
                    // Resume it:
                    bool rc = resumeDebuggedProcessThread((*iter)._threadHandle);

                    if (!rc)
                    {
                        retVal = false;
                    }
                }
            }

            iter++;
        }

        // Unlock the mutex that controls the access to the threads list:
        mutexLocker.unlockMutex();

        // Release the function execution synchronization object:
        syncObjLocker.unlockSyncObj();

        // Debug test:
        GT_ASSERT(retVal);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::isDebuggedProcssSuspended
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
bool pdWin32ProcessDebugger::isDebuggedProcssSuspended()
{
    return _isDebuggedProcssSuspended;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::onEvent
// Description: Is called by the main application thread to handle events
//              For more details - see "Notification mechanism overview" at the
//              top of pdProcessDebugger.cpp.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        22/6/2004
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::onEvent(const apEvent& eve, bool& vetoEvent)
{
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {
        // Before the event was handled by the debugged application:
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            onDebuggedProcessCreationEvent((const apDebuggedProcessCreatedEvent&)eve);
        }
        break;

        case apEvent::AP_EXCEPTION:
        {
            onExceptionEvent((const apExceptionEvent&)eve);
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            // Fix case 47: We sometimes get a process suspended event, but the process run
            // was already resumed:
            if (!isDebuggedProcssSuspended())
            {
                vetoEvent = true;
            }
        }
        break;

        // After the event was handled by the application:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
        {
            // Yaki 11/9/2005:
            // To make sure that the Spy API thread is initialized before
            // the debugged process run starts, we will resume the main application
            // thread only after the API was initialized.
            // This is done by gaPersistentDataManager::resumeDebuggedProcessMainThreadRun().
            // (See Case 173)
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            onDebuggedProcessTerminationEvent();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            onDebuggedProcessCreationFailureEvent();
        }
        break;

        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::onEventRegistration
// Description: Called as a debugged process event is being registered in the events handler
// Author:      Uri Shomroni
// Date:        28/10/2010
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::onEventRegistration(apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    apEvent::EventType eveType = eve.eventType();

    switch (eveType)
    {
        case apEvent::AP_BEFORE_KERNEL_DEBUGGING_EVENT:
        {
            // Kernel debugging is about to start, mark this:
            _isKernelDebuggingAboutToStart = true;
            _isDuringKernelDebugging = true;
        }
        break;

        case apEvent::AP_AFTER_KERNEL_DEBUGGING_EVENT:
        {
            // Kernel debugging just ended, mark this:
            _isDuringKernelDebugging = false;
            _isKernelDebuggingJustFinished = true;
        }
        break;

        default:
            break;
    }

}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::fillThreadCreatedEvent
// Description: Fills the details of the name, module, source code, etc
//              of the function that started the thread run.
// Arguments:   event - The event to be filled.
// Author:      Yaki Tebeka
// Date:        9/5/2005
// Implementation Notes:
//   We fill these details here, since we would like to use the debug symbols
//   engine only from the application thread.
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::fillThreadCreatedEvent(apThreadCreatedEvent& event)
{
    // Get the thread start address:
    osInstructionPointer startAddress = event.threadStartAddress();
    DWORD64 threadStartAddress = DWORD64(startAddress);

    // Use it to fill the start address debug info:
    // -------------------------------------------

    osWin32DebugInfoReader debugInfoReader(_pProcessInfo->hProcess);

    osFilePath moduleFilePath;
    osInstructionPointer ignored = (osInstructionPointer)NULL;
    bool rc = debugInfoReader.getModuleFromAddress(threadStartAddress, moduleFilePath, ignored);

    if (rc)
    {
        event.setThreadStartModuleFilePath(moduleFilePath);
    }

    DWORD64 functionStartAddress = 0;
    gtString functionName;
    rc = debugInfoReader.getFunctionFromAddress(threadStartAddress, functionStartAddress, functionName);

    if (rc)
    {
        event.setThreadStartFunctionName(functionName);
    }

    osFilePath sourceCodeFile;
    int lineNumber = 0;
    rc = debugInfoReader.getSourceCodeFromAddress(threadStartAddress, sourceCodeFile, lineNumber);

    if (rc)
    {
        event.setThreadStartSourceCodeDetails(sourceCodeFile, lineNumber);
    }
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::canGetCallStacks
// Description: Query whether this process debugger can get debugged process
//              calls stacks by itself (without the API
// Author:      Uri Shomroni
// Date:        25/1/2010
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::canGetCallStacks()
{
    // The Windows process debugger has access to the Win32 native debugger, so
    // it can get calls stacks:
    return true;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::canMakeThreadExecuteFunction
// Description: Returns whether this process debugger implementation supports
//              the "make thread execute function" mechanism.
// Author:      Uri Shomroni
// Date:        2/11/2009
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::canMakeThreadExecuteFunction(const osThreadId& threadId)
{
    // The Windows process debugger has access to the Win32 native debugger, so
    // it can make threads execute functions:
    bool retVal = true;

    // However, if we try to make a thread execute a function while it is in WaitForSingleObject, it gets stuck
    // (since we cannot make the wait end - we would need the wait handle for UnregisterWait(), and we would need
    // to restore the wait afterwards).
    osCallStack threadCallsStack;
    bool rcStack = getDebuggedThreadCallStack(threadId, threadCallsStack, false);
    GT_IF_WITH_ASSERT(rcStack && (threadCallsStack.amountOfStackFrames() > 0))
    {
        const osCallStackFrame* pThreadCallsStackTopFrame = threadCallsStack.stackFrame(0);
        GT_IF_WITH_ASSERT(pThreadCallsStackTopFrame != NULL)
        {
            // Most of the function names are taken from http://blogs.msdn.com/tess/archive/2005/12/20/505862.aspx:
            static const gtString waitFuncName1 = L"KiFastSystemCallRet";
            static const gtString waitFuncName2 = L"KiFastSystemCall";
            static const gtString waitFuncName3 = L"ZwDelayExecution";
            static const gtString waitFuncName4 = L"ZwWaitForSingleObject";
            static const gtString waitFuncName5 = L"ZwWaitForMultipleObjects";
            static const gtString waitFuncName6 = L"NtDelayExecution";
            static const gtString waitFuncName7 = L"NtWaitForSingleObject";
            static const gtString waitFuncName8 = L"NtWaitForMultipleObjects";
            static const gtString waitFuncName9 = L"SystemCallStub";
            const gtString& currentThreadTopStackFrameFuncName = pThreadCallsStackTopFrame->functionName();

            if ((currentThreadTopStackFrameFuncName == waitFuncName1) ||
                (currentThreadTopStackFrameFuncName == waitFuncName2) ||
                (currentThreadTopStackFrameFuncName == waitFuncName3) ||
                (currentThreadTopStackFrameFuncName == waitFuncName4) ||
                (currentThreadTopStackFrameFuncName == waitFuncName5) ||
                (currentThreadTopStackFrameFuncName == waitFuncName6) ||
                (currentThreadTopStackFrameFuncName == waitFuncName7) ||
                (currentThreadTopStackFrameFuncName == waitFuncName8) ||
                (currentThreadTopStackFrameFuncName == waitFuncName9))
            {
                // We are currently in a wait loop, don't allow execution:
                retVal = false;
            }

            // Debug log output (in DEBUG log level):
            if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
            {
                gtString threadIdAsString;
                osThreadIdAsString(threadId, threadIdAsString);
                gtString dbgMsg = L"Thread id ";
                dbgMsg += threadIdAsString;
                dbgMsg += L" is currently executing ";
                dbgMsg += currentThreadTopStackFrameFuncName;
                OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
            }
        }
    }

    // If we are using direct execution, make sure we are trying to execute the function on the trigger thread:
    FunctionExecutionMode funcExecMode = functionExecutionMode();

    if ((funcExecMode == PD_EXECUTION_IN_BREAK_MODE) && retVal)
    {
        retVal = (threadId == _breakpointTriggeringThreadId);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::makeThreadExecuteFunction
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
bool pdWin32ProcessDebugger::makeThreadExecuteFunction(const osThreadId& threadId,
                                                       osProcedureAddress64 funcAddress)
{
    bool retVal = false;

    FunctionExecutionMode funcExecMode = functionExecutionMode();

    switch (funcExecMode)
    {
        case PD_DIRECT_EXECUTION_MODE:
            retVal = makeThreadExecuteFunctionDirectly(threadId, funcAddress);
            break;

        case PD_EXECUTION_IN_BREAK_MODE:
            retVal = makeThreadExecuteFunctionInBreak(threadId, funcAddress);
            break;

        default:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::functionExecutionMode
// Description: Returns the function execution mode used by this process debugger.
//              Most windows machines use the default mode. Windows 7 64-bit uses
//              In break function execution.
// Author:      Uri Shomroni
// Date:        12/2/2010
// ---------------------------------------------------------------------------
pdProcessDebugger::FunctionExecutionMode pdWin32ProcessDebugger::functionExecutionMode() const
{
    FunctionExecutionMode retVal = pdProcessDebugger::functionExecutionMode();

    if (_isWindows7With64Bit)
    {
        retVal = PD_EXECUTION_IN_BREAK_MODE;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::afterAPICallIssued
// Description: Is called after an API call (and all its parameters) were sent
//              To the spy
// Date:        30/4/2009
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::afterAPICallIssued()
{

}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::getDebuggedThreadCallStack
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
// Date:        13/10/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::getDebuggedThreadCallStack(osThreadId threadId, osCallStack& callStack,
                                                        bool hideSpyDLLsFunctions)
{
    bool retVal = false;

    // We operate only when the debugged process is suspended:
    if (isDebuggedProcssSuspended())
    {
        // Load debug info for loaded modules (if required):
        _loadedModulesManager.loadLoadedModulesDebugSymbols();

        // Verify that we are not during a function execution:
        osSynchronizationObjectLocker syncObjLocker(_executedFuncSyncObj);

        // Get the thread handle:
        HANDLE hThread = threadIdToThreadHandle(threadId);

        if (hThread != NULL)
        {
            // Read the thread call stack:
            osWin32CallStackReader callStackReader(_pProcessInfo->hProcess, hThread, callStack);
            retVal = callStackReader.execute(hideSpyDLLsFunctions);
        }

        // Release the function execution synchronization object:
        syncObjLocker.unlockSyncObj();

        retVal = true;
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::fillCallsStackDebugInfo
// Description: Fills debug information into the calls stack
// Author:      Uri Shomroni
// Date:        26/10/2008
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::fillCallsStackDebugInfo(osCallStack& callStack, bool hideSpyDLLsFunctions)
{
    int numberOfStackFrames = callStack.amountOfStackFrames();
    osCallStackFrame currentStackFrame;
    const osCallStackFrame* pcCurrentStackFrame = NULL;
    osCallStack pendingCallsStack;
    osWin32DebugInfoReader debugInfoReader(_pProcessInfo->hProcess);
    int numberOfIgnoredFrames = 0;

    // Initialize the pending call stack:
    pendingCallsStack.setThreadId(callStack.threadId());
    pendingCallsStack.setAddressSpaceType(callStack.is64BitCallStack());

    // Fill the frames:
    for (int i = 0; i < numberOfStackFrames; i++)
    {
        pcCurrentStackFrame = callStack.stackFrame(i);
        GT_IF_WITH_ASSERT(pcCurrentStackFrame != NULL)
        {
            currentStackFrame = *pcCurrentStackFrame;
            debugInfoReader.fillStackFrame(currentStackFrame);

            if (hideSpyDLLsFunctions && currentStackFrame.isSpyFunction())
            {
                // Mark the total number of frames we are ignoring:
                numberOfIgnoredFrames = i + 1;

                // Remove all stack items from this call "upwards":
                pendingCallsStack.clearStack();
            }
            else
            {
                pendingCallsStack.addStackFrame(currentStackFrame);
            }
        }
    }

    // Currently we do not validate the pending calls stack's filling (using the return value of
    // osWin32DebugInfoReader::fillStackFrame , since we allow some of the modules to be without
    // debug information. However, we make sure the new stack is the right size:
    GT_IF_WITH_ASSERT((pendingCallsStack.amountOfStackFrames() + numberOfIgnoredFrames) == numberOfStackFrames)
    {
        callStack = pendingCallsStack;
    }
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::addInstallDirToCurrentProcessPath
// Description:
//   Add the installation directory to the current process (the debugger
//   process path). The debugged process inherits the debugger process
//   environment block, and therefore inherits this path addition.
//   This removes the need of adding the installation path to the machine
//   path (see case #7)
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/9/2005
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::addInstallDirToCurrentProcessPath()
{
    bool retVal = false;

    // Get the current process path:
    gtString currentProcessPath;
    bool rc = osGetCurrentProcessEnvVariableValue(PD_PATH_ENV_VARIABLE_NAME, currentProcessPath);
    GT_ASSERT_EX(rc, PD_STR_FailedToGetTheCurrentProcessPath);

    if (rc)
    {
        // Get the current application exe file path:
        osFilePath currentAppPath;
        bool isDllsDirSet = osGetCurrentApplicationDllsPath(currentAppPath);

        if (!isDllsDirSet)
        {
            rc = osGetCurrentApplicationPath(currentAppPath);
        }

        if (rc)
        {
            // Get the current application installation directory:
            osDirectory installDirPath;
            rc = currentAppPath.getFileDirectory(installDirPath);

            if (rc)
            {
                // Build a new path that contains the installation directory:
                gtString newProcessPath = installDirPath.directoryPath().asString();
                newProcessPath += osFilePath::osEnvironmentVariablePathsSeparator;
                newProcessPath += currentProcessPath;

                // Set the current process path to be the new path:
                osEnvironmentVariable pathEnvVariable;
                pathEnvVariable._name = PD_PATH_ENV_VARIABLE_NAME;
                pathEnvVariable._value = newProcessPath;
                retVal = osSetCurrentProcessEnvVariable(pathEnvVariable);

                // Check if AMD_OCL_BUILD_OPTIONS environment variables is set, and if it is, add the OpenCL kernel debugging
                // necessary flags:
                gtString envVariableValue;
                rc = osGetCurrentProcessEnvVariableValue(PD_STR_AMD_OCL_BUILD_OPTIONS, envVariableValue);

                if (rc && !envVariableValue.isEmpty())
                {
                    // Mark that this environment variable is ignored:
                    _isIgnoringAMD_OCL_BUILD_OPTIONS = true;

                    // Currently the back end does not support the option to append the user build options to the kernel debugging build options,
                    // so we clean this environment variable content, and warn the user:
                    envVariableValue.makeEmpty();

                    // Set the current process path to be the new path:
                    osEnvironmentVariable buildOptionsEnvVariable;
                    buildOptionsEnvVariable._name = PD_STR_AMD_OCL_BUILD_OPTIONS;
                    buildOptionsEnvVariable._value = envVariableValue;
                    rc = osSetCurrentProcessEnvVariable(buildOptionsEnvVariable);
                    GT_ASSERT(rc);
                }

                // Check if AMD_OCL_BUILD_OPTIONS_APPEND environment variables is set, and if it is, add the OpenCL kernel debugging
                // necessary flags:
                rc = osGetCurrentProcessEnvVariableValue(PD_STR_AMD_OCL_BUILD_OPTIONS_APPEND, envVariableValue);

                if (rc && !envVariableValue.isEmpty())
                {
                    // Mark that this environment variable is ignored:
                    _isIgnoringAMD_OCL_BUILD_OPTIONS = true;

                    // Currently the back end does not support the option to append the user build options to the kernel debugging build options,
                    // so we clean this environment variable content, and warn the user:
                    envVariableValue.makeEmpty();

                    // Set the current process path to be the new path:
                    osEnvironmentVariable buildOptionsEnvVariable;
                    buildOptionsEnvVariable._name = PD_STR_AMD_OCL_BUILD_OPTIONS_APPEND;
                    buildOptionsEnvVariable._value = envVariableValue;
                    rc = osSetCurrentProcessEnvVariable(buildOptionsEnvVariable);
                    GT_ASSERT(rc);
                }
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::setDllDirectoryToSpiesDirectory
// Description:
//   Sets the DLL directory to contain the spies directory. This causes the spies directory
//   to be searched by LoadDll functions before the system directory is searched and load
//   the spy DLLs instead of the system dlls they imitate.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/6/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::setDllDirectoryToSpiesDirectory()
{
    bool retVal = false;

    const osFilePath& spiesDirPath = _pProcessCreationData->spiesDirectory();
    retVal = setDllDirectory(spiesDirPath);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::resetDllDirectory
// Description: Resets the DLL directory to contain no directory.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/6/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::resetDllDirectory()
{
    bool retVal = false;

    osFilePath spiesDirPath(L"");
    retVal = setDllDirectory(spiesDirPath);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::setDllDirectory
// Description:
//   Sets the DLL directory to contains the input path.
//
//   Notice:
//   ------
//   The Win32 API function SetDllDirectory() was introduced in Windows XP service pack 1
//   as part of a mechanism that avoids viruses trojan DLLs.
//   For more details see:
//   - MSDN article: "Development Impacts of Security Changes in Windows Server 2003
//                    (Code Secure)"
//   - MSDN "SafeDllSearchMode" registry entry documentation (appears in MSDN LoadLibrary
//     documentation)
//
//   In Windows 2000 and Windows XP without service pack 1, this function does
//   not exist in Kernel32.dll, causing this function to fail.
//
// Arguments:   dllDirectory - The input path the will be set as the DLL directory.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/6/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::setDllDirectory(const osFilePath& dllDirectory, gtString* o_pOriginalValue)
{
    bool retVal = false;

    // Defining SetDllDirectory function type.
    typedef BOOL (__stdcall * PFN_SET_DLL_DIRECTORY)(LPCTSTR);

    // Get a pointer to the SetDllDirectory function (defined in KERNEL32.DLL):
    HMODULE hKernel32 = GetModuleHandle(L"KERNEL32.DLL");
    PFN_SET_DLL_DIRECTORY pSetDllDirectory = (PFN_SET_DLL_DIRECTORY)GetProcAddress(hKernel32, "SetDllDirectoryW");

    // If the function exists in KERNEL32.DLL:
    // (See "Notice" in this function documentation)
    if (NULL != pSetDllDirectory)
    {
        // Before setting the value, output the original value, if requested:
        if (NULL != o_pOriginalValue)
        {
            o_pOriginalValue->makeEmpty();

            // Get the "GetDllDirectory" function:
            typedef DWORD (__stdcall * PFN_GET_DLL_DIRECTORY)(DWORD, LPTSTR);
            PFN_GET_DLL_DIRECTORY pGetDllDirectory = (PFN_GET_DLL_DIRECTORY)GetProcAddress(hKernel32, "GetDllDirectoryW");

            if (NULL != pGetDllDirectory)
            {
                // Get the length of the current string:
                DWORD bufferLength = pGetDllDirectory(0, NULL);

                if (0 < bufferLength)
                {
                    TCHAR* pPath = new TCHAR[bufferLength + 1];
                    GT_IF_WITH_ASSERT(NULL != pPath)
                    {
                        // Get the path:
                        DWORD writtenChars = pGetDllDirectory(bufferLength, pPath);
                        GT_IF_WITH_ASSERT(0 < writtenChars)
                        {
                            // Copy the path to the output variable:
                            pPath[bufferLength] = (TCHAR)0;
                            *o_pOriginalValue = pPath;
                        }
                    }

                    delete[] pPath;
                }
            }
        }

        // Set the input directory as the Dll directory:
        const gtString& dllDirAsString = dllDirectory.asString();
        LPCTSTR lpDLLDirectoryPathName = NULL;

        if (!dllDirAsString.isEmpty())
        {
            lpDLLDirectoryPathName = dllDirAsString.asCharArray();
        }

        BOOL rc = pSetDllDirectory(lpDLLDirectoryPathName);

        if (rc != FALSE)
        {
            retVal = true;
        }
        else
        {
            // This should not happened:
            GT_ASSERT(false);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::updateThreadsData
// Description: Updated threads data that can only be updated after the debugged
//              process run started.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/4/2006
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::updateThreadsData()
{
    bool retVal = true;

    // Act only if we need to update the threads data:
    if (_needToUpdateThreadsData)
    {
        _needToUpdateThreadsData = false;

        // Lock the mutex that controls the access to the threads list:
        osMutexLocker mutexLocker(_threadsListAccessMutex);

        // Iterate the debugged process threads:
        gtList<pdDebuggedProcessThreadData>::iterator endIter = _debuggedProcessThreadsData.end();
        gtList<pdDebuggedProcessThreadData>::iterator iter = _debuggedProcessThreadsData.begin();

        while (iter != endIter)
        {
            // Get the current thread start address:
            osInstructionPointer currThreadStartAddress = (*iter)._threadStartAddress;

            // Check if the thread is a driver thread:
            bool isDriverThread = _loadedModulesManager.isDriverAddress(currThreadStartAddress);

            // If it is a driver thread:
            if (isDriverThread)
            {
                // Mark it as a driver thread:
                (*iter)._isDriverThread = true;

                // Output a log message:
                gtString msg = PD_STR_driverThreadCreation;
                msg.appendFormattedString(L" (thread id = %d, start address = %p)", (*iter)._threadId, currThreadStartAddress);
                OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_INFO);
            }

            iter++;
        }

        // Unlock the mutex that controls the access to the threads list:
        mutexLocker.unlockMutex();
    }

    GT_RETURN_WITH_ASSERT(retVal);
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::makeThreadExecuteFunctionDirectly
// Description: Makes a thread execute a function directly (using ::SetThreadContext)
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        12/2/2010
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::makeThreadExecuteFunctionDirectly(const osThreadId& threadId, osProcedureAddress64 funcAddress)
{
    bool retVal = false;

    // Mark that we are during function execution:
    // The synchronization object will be unlocked by handleBreakPoint(), when
    // the thread that executed the function will end its run:
    _executedFuncSyncObj.lock();

    // Output debug log:
    OS_OUTPUT_DEBUG_LOG(PD_STR_makeThreadExecuteFunctionStarted, OS_DEBUG_LOG_DEBUG);

    // Get the thread handle:
    HANDLE hThread = threadIdToThreadHandle(threadId);

    if (hThread != NULL)
    {
        // Store the thread execution context:
        bool rc = storeThreadExecutionContext(threadId);
        GT_IF_WITH_ASSERT(rc)
        {
            // Copy the stored thread context:
            CONTEXT execFuncContext;
            memcpy(&execFuncContext, &_storedThreadContext, sizeof(_storedThreadContext));

            // Move the thread instruction pointer to the input function address:
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
            execFuncContext.Eip = (DWORD)funcAddress;
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
            execFuncContext.Rip = (DWORD64)funcAddress;
#else
#error Unknown address space size!
#endif // AMDT_ADDRESS_SPACE_TYPE

            // Set this to be the current thread execution context:
            BOOL rc1 = ::SetThreadContext(hThread, &execFuncContext);
            GT_IF_WITH_ASSERT(rc1 != 0)
            {
                // Resume the thread run:
                _waitingForExecutedFunction = true;
                DWORD previousSuspensionCount = ::ResumeThread(hThread);

                rc = (previousSuspensionCount == 1);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Wait for the function execution to end:
                    retVal = waitForThreadFunctionExecution();
                }
            }
        }
    }

    // If we failed to execute the function, we need to return the thread to its original state:
    if (!retVal)
    {
        // Output failure debug log:
        OS_OUTPUT_DEBUG_LOG(PD_STR_remoteFuncFailed, OS_DEBUG_LOG_ERROR);

        // Suspend the thread run:
        bool rc1 = suspendDebuggedProcessThread(threadId);
        GT_ASSERT(rc1);

        // Restore the thread execution context, unless the application crashed during execution:
        if (!_executedFunctionCrashed)
        {
            bool rc2 = restoreThreadExecutionContext(threadId);
            GT_ASSERT(rc2);
        }

        // Unlock synchronization members:
        _executedFuncSyncObj.unlock();
        _waitingForExecutedFunction = false;
    }

    // Output debug log:
    OS_OUTPUT_DEBUG_LOG(PD_STR_makeThreadExecuteFunctionEnded, OS_DEBUG_LOG_DEBUG);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::makeThreadExecuteFunctionInBreak
// Description: Makes a thread execute a function using suBreakpointsManager::handleFunctionExecutionDuringBreak
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        12/2/2010
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::makeThreadExecuteFunctionInBreak(const osThreadId& threadId, osProcedureAddress64 funcAddress)
{
    bool retVal = false;

    // Mark that we are during function execution:
    // The synchronization object will be unlocked by handleBreakPoint(), when
    // the thread that executed the function will end its run:
    _executedFuncSyncObj.lock();

    // Make sure we didn't get a real function pointer:
    GT_IF_WITH_ASSERT(funcAddress == PD_EXECUTION_IN_BREAK_DUMMY_ADDRESS)
    {
        // Get the thread handle:
        HANDLE hThread = threadIdToThreadHandle(threadId);

        if (hThread != NULL)
        {
            // Resume the thread run:
            _waitingForExecutedFunction = true;
            DWORD previousSuspensionCount = ::ResumeThread(hThread);

            GT_IF_WITH_ASSERT(previousSuspensionCount == 1)
            {
                // Wait for the function execution to end:
                retVal = waitForThreadFunctionExecution();
            }
        }
    }

    if (!retVal)
    {
        // Output failure debug log:
        OS_OUTPUT_DEBUG_LOG(PD_STR_remoteFuncFailed, OS_DEBUG_LOG_ERROR);

        // Suspend the thread run:
        bool rc1 = suspendDebuggedProcessThread(threadId);
        GT_ASSERT(rc1);

        // Unlock synchronization members:
        _executedFuncSyncObj.unlock();
        _waitingForExecutedFunction = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::waitForThreadFunctionExecution
// Description: Waits for a debugged thread to finish executing a function.
//              (See makeThreadExecuteFunction() for more details)
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/10/2005
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::waitForThreadFunctionExecution()
{
    bool retVal = false;

    // Wait, in 1 second intervals, for a debugged process thread to finish its execution:
    bool shouldWait = true;

    while (shouldWait)
    {
        bool rc1 = osWaitForFlagToTurnOff(_waitingForExecutedFunction, 1000);

        if (rc1)
        {
            // The debugged process thread finished its execution:
            shouldWait = false;
            retVal = true;
        }
        else
        {
            // If the debugged process crashed while we are waiting for a thread to finish its execution:
            if (_executedFunctionCrashed)
            {
                _waitingForExecutedFunction = false;
                shouldWait = false;
                retVal = false;
            }
        }
    }

    return retVal;
}

// -----------------------------------------------------------------------
//  !!      Functions that are used by the debugger thread      !!
// -----------------------------------------------------------------------



// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::createDebuggedProcess
// Description:
//   Is called by the debugger thread.
//   Launches the debugged process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/11/2003
// Implementation notes:
//
//   Loading the Spy dlls:
//   --------------------
//   - Before launching the debugged process, we set the DLL directory (or current directory)
//     of this process to contain the spies directory.
//   - When the debugged process is launched, it copies this process environment (and the
//     above directories definition). This causes the staticly linked DLLs of the debugged
//     process to be the Spy dlls instead of the real dlls.
//   - After the debugged process is launched - we reset the DLL directory.
//   - When the debugged process loads DLLs dynamically (using the LoadLibrary function),
//     it ignores the above SetDLLDirectory() definition. To overcome this problem, when
//     the debugged process run starts, we create a remote thread in the debugged process
//     that calls SetDLLDirectory() with our spies path. This definition holds until the
//     debugged process exits.
//   - When the spy DLL is loaded, it sets the current directory back to the original
//     directory.
//
//   Notice !!!!
//   ------
//   The CodeXL shortcut should not contain a "Start in" directory.
//   If it does - the above SetDLLDirectory() trick does not work (I don't know why).
//
//   Debugging the OpenGL32.dll spy:
//   ------------------------------
//   If you would like to debug the debugged application process and the OpenGL32.dll
//   spy using Dev studio, you should:
//   a. Replace the below DEBUG_ONLY_THIS_PROCESS flag with NULL.
//      I.E: We create the debugged process, but we do not serve as a debugger to it.
//   b. Put a breakpoint at the end of gaSocketServerThread::initializeAPI()
//   c. Run the process.
//   d. After the "b" stage breakpoint is hit - you can attach to the debugged process
//      using another Dev studio.
//
//   Why do we do all these stages ?
//   - You cannot attach with DevStudio to a process that is debugged by another process.
//     (This is why we remove the DEBUG_ONLY_THIS_PROCESS flag).
//   - Duplicate handle does not work on handles from processes that are debugged by
//     another Dev Studio.
//     (This is why we attach to the debugged process after gaSocketServerThread::initializeAPI()
//      was called - it copies the Spy API thread handle).
//
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::createDebuggedProcess()
{
    bool retVal = false;

    // Sanity test:
    GT_IF_WITH_ASSERT((_pProcessInfo != NULL) && (_pProcessCreationData != NULL))
    {
        // Convert strings into LPTSTRs:
        LPTSTR LCSTRexecutablePath = (wchar_t*)_pProcessCreationData->executablePath().asString().asCharArray();
        LPTSTR LCSTRcommandLine = (wchar_t*)_pProcessCreationData->commandLineArguments().asCharArray();
        LPTSTR LCSTRworkDir = (wchar_t*)_pProcessCreationData->workDirectory().asString().asCharArray();

        // If we have a spies directory:
        if (!_pProcessCreationData->spiesDirectory().asString().isEmpty())
        {
            // Try to set the DLL directory to contain the spies directory:
            // (See "Loading the Spy dll" section above)
            bool dllDirectorySet = setDllDirectoryToSpiesDirectory();

            // If we failed to set the DLL directory:
            // (See setDllDirectoryToSpiesDirectory documentation for details):
            if (!dllDirectorySet)
            {
                // We will use the work directory instead of the DLL directory to make the spies load:
                LCSTRworkDir = (wchar_t*)_pProcessCreationData->spiesDirectory().asString().asCharArray();
            }
        }
        else
        {
            // We don't have a spies directory:
            resetDllDirectory();
        }

        // Initialize the process startup info structure members to 0:
        STARTUPINFO startupInfo;
        ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);

        // Set the argument that will be used as the first time the debugged program calls ShowWindow():
        // SW_SHOW - Activate the window and display it in its current size and position.
        // (See case 587: Alias Studio PLE 12 and CodeXL)
        startupInfo.dwFlags = STARTF_USESHOWWINDOW;
        startupInfo.wShowWindow = SW_SHOW;

        // Calculate the command line of the process to be created:
        gtString createdProcessCommandLine = L"\"";
        createdProcessCommandLine += LCSTRexecutablePath;
        createdProcessCommandLine += L"\" ";
        createdProcessCommandLine += LCSTRcommandLine;

        gtString outputFileName;
        gtString inputFileName;
        bool appendMode;

        BOOL handleInheritance = FALSE;

        if (osCheckForOutputRedirection(createdProcessCommandLine, outputFileName, appendMode))
        {
            m_outputFile.openFile(outputFileName, true, appendMode);
        }

        if (osCheckForInputRedirection(createdProcessCommandLine, inputFileName))
        {
            m_inputFile.openFile(inputFileName, false, false);
        }

        startupInfo.hStdOutput = m_outputFile.handle();
        startupInfo.hStdInput = m_inputFile.handle();

        // Activate the redirection mechanism only if there is a redirection file at all:
        if (m_outputFile.handle() != NULL || m_inputFile.handle() != NULL)
        {
            startupInfo.dwFlags |= STARTF_USESTDHANDLES;
            handleInheritance = TRUE;
        }

        // Set the debugged process environment variables:
        bool rc1 = setDebuggedProcessEnvVariables();
        GT_ASSERT(rc1);

        // Create the debugged process:
        int rc =
            CreateProcess(NULL,                     // Executable name / path.
                          (LPWSTR)createdProcessCommandLine.asCharArray(),  // Command line arguments.
                          NULL,                     // The created process will use the
                          //  default security descriptor.
                          NULL,                     // The main thread will use the default
                          //  security descriptor.
                          handleInheritance,        // Set handle inheritance to FALSE.
                          DEBUG_ONLY_THIS_PROCESS,  // The process will be created in Debug
                          //  mode. Its child processes will not be
                          //  debugged.
                          // Notice: Here we can add the following flags:
                          //        DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS
                          //        CREATE_SUSPENDED
                          //        Another option is to put NULL. This
                          //        creates a process that is not attached to
                          //        us as a debugger => We can now attach a
                          //        debugger to the OpenGL32.dll spy and debug it.
                          NULL,                     // The created process will inherit this process
                          // environment variables.
                          LCSTRworkDir,             // The process initial working directory will
                          // be the spies directory. This will make it
                          // load our spy DLLs instead of the read DLLs.

                          &startupInfo,             // Pointer to STARTUPINFO structure.
                          _pProcessInfo);           // Pointer to PROCESS_INFORMATION structure.

        if (rc != 0)
        {
            _isDebuggedProcssSuspended = false;
            retVal = true;
        }
        else
        {
            delete _pProcessInfo;
            _pProcessInfo = NULL;

            // Get the process creation error as string from OS:
            gtString processCreationError;
            osGetLastSystemErrorAsString(processCreationError);

            // Send a message to the log file for the process creation failure;
            gtString dbg;
            dbg.appendFormattedString(L"Failed to create process. Process Command Line: %ls. Process creation error: %ls. Process Working Directory: %ls", createdProcessCommandLine.asCharArray(), processCreationError.asCharArray(), (wchar_t*)LCSTRworkDir);
            OS_OUTPUT_DEBUG_LOG(dbg.asCharArray(), OS_DEBUG_LOG_ERROR);

            // Notify observers about the process creation failure:
            createdProcessCommandLine.replace(L"\"", L"");
            apDebuggedProcessCreationFailureEvent event(apDebuggedProcessCreationFailureEvent::COULD_NOT_CREATE_PROCESS, createdProcessCommandLine, LCSTRworkDir, processCreationError);
            apEventsHandler::instance().registerPendingDebugEvent(event);

        }

        // Remove the debugged process environment variables from this process
        // (See removeDebuggedProcessEnvVariables documentation for more details)
        bool rc2 = removeDebuggedProcessEnvVariables();
        GT_ASSERT(rc2);

        // Reset the DLL directory:
        resetDllDirectory();
    }

    // Mark that we attempted to create the debugged process (whether successfully or not):
    _waitingForDebuggedProcessCreation = false;

    // Start waiting for continueDebuggedProcessFromSuspendedCreation:
    _waitingAtDebuggedProcessLaunch = true;


    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::initialize
// Description: Initializes this class members.
// Author:      Yaki Tebeka
// Date:        5/7/2004
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::initialize()
{
    _debuggedProcessExist = false;

    _isDebuggedProcssSuspended = false;
    _isDebugging64BitApplication = false;
    _wasProcessCreationBreakPointHit = false;
    _wasMainThreadEntryPointBreakpointHit = false;
    _isDuringSecondChangeExceptionHandling = false;
    _isDuringDebuggedProcessTermination = false;

    // Lock the mutex that controls the access to the threads list:
    osMutexLocker mutexLocker(_threadsListAccessMutex);

    // Clear the threads related members:
    _debuggedProcessThreadsData.clear();

    _needToUpdateThreadsData = false;

    _unknownDllIndex = 1;

    _mainThreadId = OS_NO_THREAD_ID;
    _spiesAPIThreadId = OS_NO_THREAD_ID;

    _mainThreadEntryPointAddress = NULL;
    _mainThreadEntryPointOpCode = 0;

    _idOfThreadWithStoredContext = OS_NO_THREAD_ID;
    memset(&_storedThreadContext, 0, sizeof(_storedThreadContext));


    // Unlock the mutex that controls the access to the threads list:
    mutexLocker.unlockMutex();

    _waitingForExecutedFunction = false;
    _waitingForDebuggedProcessCreation = false;
    _waitingAtDebuggedProcessLaunch = false;
    _executedFunctionCrashed = false;
    _isKernelDebuggingAboutToStart = false;
    _isDuringKernelDebugging = false;
    _isKernelDebuggingJustFinished = false;

    _isWindows7With64Bit = false;

    // If this is a 64-bit build (inside the remote process debugger):
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    {
        // Get the Windows version:
        osWindowsVersion winVer = OS_WIN_XP;
        bool rcVer = osGetWindowsVersion(winVer);
        GT_IF_WITH_ASSERT(rcVer)
        {
            // If this is windows 7
            if (OS_WIN_7 <= winVer)
            {
                // Mark that we are on Windows 7 64-bit (activating the ::SetThreadContext workaround)
                _isWindows7With64Bit = true;
            }
        }
        else
        {
            // If the function failed, we'd rather assume that we are Windows 7 or higher:
            _isWindows7With64Bit = true;
        }
    }
#endif

    _breakpointTriggeringThreadId = OS_NO_THREAD_ID;

    _amountOfOutputStringPrintouts = 0;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::onDebuggedProcessCreationEvent
// Description: Is called when the debugged process is created.
// Arguments:   event - An event representing the process creation.
// Author:      Yaki Tebeka
// Date:        17/10/2004
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::onDebuggedProcessCreationEvent(const apDebuggedProcessCreatedEvent& event)
{
    _loadedModulesManager.setDebuggedProcessHandle(_pProcessInfo->hProcess);
    _loadedModulesManager.onDebuggedProcessCreation(event);
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::onDebuggedProcessTerminationEvent
// Description: Is called by the main thread when the debugged process is
//              terminated.
//              Performs clean ups:
// Author:      Yaki Tebeka
// Date:        22/4/2004
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::onDebuggedProcessTerminationEvent()
{
    // We do not expect to get here without process info, since that would imply we
    // got two process termination events for the same process run:
    GT_IF_WITH_ASSERT(_pProcessInfo != NULL)
    {
        // Notify the loaded modules manager:
        _loadedModulesManager.onDebuggedProcessTermination();

        // Delete the debugged process info structure:
        CloseHandle(_pProcessInfo->hProcess);
        CloseHandle(_pProcessInfo->hThread);
        delete _pProcessInfo;
        _pProcessInfo = NULL;
    }

    // Delete the process creation data:
    delete _pProcessCreationData;
    _pProcessCreationData = NULL;

    // Delete the debugger thread:
    delete _pDebuggerThread;
    _pDebuggerThread = NULL;

    m_inputFile.closeFile();
    m_outputFile.closeFile();

    // Re-initialize this class:
    initialize();
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::onDebuggedProcessCreationFailureEvent
// Description: Is called by the main thread when the debugged process creation
//              is failed
//              Performs clean ups:
// Author:      Sigal Algranaty
// Date:        11/2/2010
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::onDebuggedProcessCreationFailureEvent()
{
    // Delete the process creation data:
    delete _pProcessCreationData;
    _pProcessCreationData = NULL;

    // Delete the debugger thread:
    delete _pDebuggerThread;
    _pDebuggerThread = NULL;

    // Re-initialize this class:
    initialize();
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::onExceptionEvent
// Description: Is called when a second chance exception event occurs.
// Arguments:   event - A class representing the event.
// Author:      Yaki Tebeka
// Date:        19/10/2004
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::onExceptionEvent(const apExceptionEvent& event)
{
    GT_UNREFERENCED_PARAMETER(&event);

    // Mark the debugged process as during second change exception handling:
    _isDuringSecondChangeExceptionHandling = true;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::waitForDebugEvent
//
// Description: Wait for debug events generated by one of the debugged process
//              threads.
//              When an even arrives - handles it.
//
// Return Val:  bool - Return true iff the event was handled successfully.
// Author:      Yaki Tebeka
// Date:        22/11/2003
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::waitForDebugEvent()
{
    bool rc = true;

    // Wait for a debug event generated by one of the debugged process threads:
    DEBUG_EVENT debugEvent;

    if (WaitForDebugEvent(&debugEvent, INFINITE) != FALSE)
    {
        // Handle the current debug event:
        DWORD continueStatus = DBG_CONTINUE;
        bool rc1 = handleDebuggedProcessEvent(debugEvent, continueStatus);

        GT_UNREFERENCED_PARAMETER(rc1);
        // TO_DO: Check this:
        // gtAssert(rc1);

        // If this is a process creation event:
        if (debugEvent.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT)
        {
            // Wait for two-step process creation, as needed:
            osWaitForFlagToTurnOff(_waitingAtDebuggedProcessLaunch, ULONG_MAX);
        }

        // Continue running it:
        if (ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId,
                               continueStatus) == FALSE)
        {
            // An error occurred:
            rc = false;
        }
    }

    return rc;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::handleDebuggedProcessEvent
// Description: Is called to handle debugged process events.
// Arguments:
//   debugEvent - The win32 event struct.
//   win32ContinueStatus - Continue status that will be passed to
//                         the ContinueDebugEvent() win32 function.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        22/11/2003
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::handleDebuggedProcessEvent(const DEBUG_EVENT& debugEvent,
                                                        unsigned long& win32ContinueStatus)
{
    bool rc = true;

    // Default continue status:
    win32ContinueStatus = DBG_CONTINUE;

    // What was the event type ?
    switch (debugEvent.dwDebugEventCode)
    {
        // The debugged process was created
        case CREATE_PROCESS_DEBUG_EVENT:
            rc = handleProcessCreation(debugEvent);
            break;

        // The debugged process exit
        case EXIT_PROCESS_DEBUG_EVENT:
            rc = handleProcessExit(debugEvent);
            break;

        // A new thread was created by the debugged process:
        case CREATE_THREAD_DEBUG_EVENT:
            rc = handleThreadCreation(debugEvent);
            break;

        // A thread ended its run:
        case EXIT_THREAD_DEBUG_EVENT:
            rc = handleThreadExit(debugEvent);
            break;

        // The debugged process loaded a DLL:
        case LOAD_DLL_DEBUG_EVENT:
            rc = handleDLLLoad(debugEvent);
            break;

        // The debugged process unloaded a DLL:
        case UNLOAD_DLL_DEBUG_EVENT:
            rc = handleDLLUnload(debugEvent);
            break;

        // The debugged process threw an exception:
        case EXCEPTION_DEBUG_EVENT:
            rc = handleException(debugEvent, win32ContinueStatus);
            break;

        // The debugged process output a debug string:
        case OUTPUT_DEBUG_STRING_EVENT:
            rc = handleDebugString(debugEvent);
            break;

        // A debugging system error occur:
        case RIP_EVENT:
            rc = handleWin32DebuggerError(debugEvent);
            break;

        // Unknown debug event:
        default:
            GT_ASSERT_EX(false, PD_STR_UnknownDebugEventError);
            rc = false;
            break;
    }

    return rc;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::handleProcessCreation
// Description: Is called when the debugged process is created.
//              Notifies the registered observers about the process creation.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        20/12/2003
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::handleProcessCreation(const DEBUG_EVENT& debugEvent)
{
    bool retVal = false;

    // Notice - here we get a lot of information and handles that can be used to
    // manipulate the debugged process (process / thread / file / etc handles).
    // Currently - we don't do such manipulations, so we close all the handles that we get.
    // In future - it might be useful to store them and close them only on process
    // termination.

    // Store the process main thread handle:
    _mainThreadId = _pProcessInfo->dwThreadId;

    // Store the main thread entry point address:
    _mainThreadEntryPointAddress = debugEvent.u.CreateProcessInfo.lpStartAddress;

    // Mark the current time as the process creation time:
    osTime processCreationTime;
    processCreationTime.setFromCurrentTime();

    // Get the process loaded address:
    osInstructionPointer processLoadedAddress = (osInstructionPointer)debugEvent.u.CreateProcessInfo.lpBaseOfImage;

    // Register the process (as a module) in the modules vectors:
    const osFilePath& exeModulePath = _pProcessCreationData->executablePath();
    _loadedModulesManager.onModuleLoaded(exeModulePath, processLoadedAddress);

    // Get the main thread handle and id:
    DWORD threadId = debugEvent.dwThreadId;
    HANDLE threadHandle = debugEvent.u.CreateProcessInfo.hThread;

    // Notify observers about the process creation:
    apDebuggedProcessCreatedEvent event(*_pProcessCreationData, processCreationTime, processLoadedAddress);
    apEventsHandler::instance().registerPendingDebugEvent(event);

    // Mark that a debugged process is running:
    _debuggedProcessExist = true;

    // Log the main thread creation:
    handleThreadCreation(threadId, threadHandle, (osInstructionPointer)_mainThreadEntryPointAddress, true);

    // Release the debugged process debug info structure:
    CloseHandle(debugEvent.u.CreateProcessInfo.hFile);

    // The CreateProcessInfo.hProcess and CreateProcessInfo.hThread handles are closed
    //  by the debugging system when calling ContinueDebugEvent.
    // (See ContinueDebugEvent MSDN documentation)

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::handleProcessExit
// Description: Is called when the debugged process exists.
//              Notifies the registered about the process termination.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        20/12/2003
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::handleProcessExit(const DEBUG_EVENT& debugEvent)
{
    bool retVal = true;

    // Get the debugged process exit code:
    long exitCode = debugEvent.u.ExitProcess.dwExitCode;

    // If the main thread didn't exit yet:
    if (_mainThreadId != NULL)
    {
        // Notify observers that the main thread was terminated:
        handleThreadExit(_mainThreadId, exitCode);
    }

    // Notify observers about the process exit:
    apDebuggedProcessTerminatedEvent event(exitCode);
    apEventsHandler::instance().registerPendingDebugEvent(event);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::handleThreadCreation
// Description: Is called when the debugged process creates a new thread.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        8/6/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::handleThreadCreation(const DEBUG_EVENT& debugEvent)
{
    bool retVal = true;

    // Get the created thread id and handle:
    DWORD threadId = debugEvent.dwThreadId;
    HANDLE threadHandle = debugEvent.u.CreateThread.hThread;

    // The threads start address from the thread creation struct:
    // (This points RtlUserThreadStart in ntdll.dll, which is the function that launched the thread and not the address we are looking for)
    osInstructionPointer threadStartAddressFromCreationEvent = (osInstructionPointer)(debugEvent.u.CreateThread.lpStartAddress);
    GT_UNREFERENCED_PARAMETER(threadStartAddressFromCreationEvent);

    // Yaki 23/4/06:
    // The below fix a problem in which DEBUG_EVENT.u.CreateThread.lpStartAddress always points
    // an address that resides inside kernel32.dll.
    // A query in microsoft.public.windbg (Microsoft managed newsgroups) by "Matthew LeGendre"
    // and "gremedy" got the following reply:
    // > I don't believe there's a public API to make that query.  You could potentially try and
    // > recover it from doing a stack trace and looking at call stack parameters, but that'd have
    // > a number of potential issues.
    // > I've suggested to the OS team that the debug API should return start
    // > addresses consistent with thread create APIs (instead of returning the true
    // > start addresses, which are OS wrapper functions) but that isn't something
    // > that'll happen soon (the app compat impact has to be assessed) and it won't
    // > help you for shipped OS's.
    //
    // However, a post to this newsgroup item by "Skywing" tells the following:
    // > While this isn't documented, on all current x86 NT-based Windows versions
    // > the address of the actual program-specified start routine is initially in
    // > the EAX register in the thread context (for both
    // > kernel32!BaseProcessStartThunk and kernel32!BaseThreadStartThunk). In
    // > general you will only be able to look at this while you are handling the
    // > initial thread creation event while the thread is still suspended though.
    //
    // We asked him the following:
    // > The approach you suggested seems to be working in my case.
    // > Can you please tell me:
    // > a. Why does the Eax register contain the thread start address?
    // >    (Is this the return value of a kernel32.dll function / other)
    // > b. How reliable do you think it is.
    //
    // And got the below answer:
    // > Because internally, this is just the mechanism that Microsoft selected to
    // > pass the user requested start address for a Win32 thread on x86 to the
    // > kernel32 wrapper function which sets up the top level exception handler.
    // > They could have also done something like pass a stack parameter, but this
    // > was probably easier (just requires a SetThreadContext call vs playing around
    // > with the stack pointer and altering process memory via WriteProcessMemory
    // > (the create thread API is designed to assume that the thread being created
    // > may reside in a different process, a-la CreateRemoteThread)
    //
    // > As for how reliable - it's always been like this on NT-based systems to my
    // > knowledge on x86.  As with any undocumented thing, it could theoretically
    // > change in the future
    // > For debugging tools, though, I would not think it would be such a big deal
    // > as you are generally going to be needing to make other changes with new
    // > major OS releases anyway (updating utilities/extensions as internal
    // > structures change, or supporting new debugging functionality)
    //
    // So, to resolve this problem, we implement the method suggested by Skywing
    //
    // See also Cases 1461 and 1616, for which we had to have accurate thread start address.


    // Will get the thread's start address from the thread's context:
    osInstructionPointer threadStartAddress = 0;

    // Get the thread's context:
    CONTEXT threadContext;
    memset(&threadContext, 0, sizeof(threadContext));
    threadContext.ContextFlags = CONTEXT_FULL;
    BOOL rc = ::GetThreadContext(threadHandle, &threadContext);

    if (rc)
    {
        // Get the thread's actual start address:
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        threadStartAddress = (osInstructionPointer)(threadContext.Eax);
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        threadStartAddress = (osInstructionPointer)(threadContext.Rcx);
#else
#error Unknown address space size!
#endif // AMDT_ADDRESS_SPACE_TYPE
    }

    // Handle the thread creation event:
    handleThreadCreation(threadId, threadHandle, threadStartAddress);

    // Notice: There is no need to duplicate or close the thread handle (debugEvent.u.CreateThread.hThread),
    //         since it is closed by the debugging system when the process ends.
    //         (See ms-help://MS.MSDNQTR.2003JAN.1033/dnmag02/html/EscapefromDLLHell.htm)

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::handleThreadCreation
// Description: Handles the thread creation event
// Arguments:   threadId - The created thread id.
//              threadHandle - The created thread handle.
//              threadStartAddress - The thread start address.
//              isMainThread - Will get true iff the created thread is debugged process's the main thread (the initial thread of the process).
// Author:      Yaki Tebeka
// Date:        8/5/2005
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::handleThreadCreation(DWORD threadId, HANDLE threadHandle, osInstructionPointer threadStartAddress, bool isMainThread)
{
    // Lock the mutex that controls the access to the threads list:
    osMutexLocker mutexLocker(_threadsListAccessMutex);

    // If needed, output debug log printout:
    outputHandlingThreadCreationDebugLogMessage(threadStartAddress, isMainThread);

    // Add it to the list of debugged process threads:
    pdDebuggedProcessThreadData createdThreadData(threadId, threadHandle, threadStartAddress);
    _debuggedProcessThreadsData.push_back(createdThreadData);

    // Mark that we need to update the debugged process threads data:
    _needToUpdateThreadsData = true;

    // Unlock the mutex that controls the access to the threads list:
    mutexLocker.unlockMutex();

    // Get the thread creation time:
    osTime nowTime;
    nowTime.setFromCurrentTime();

    // There is no LWP ID on Windows:
    osThreadId lwpOSId = OS_NO_THREAD_ID;

    // Notify observers about the thread creation:
    apThreadCreatedEvent event(threadId, lwpOSId, nowTime, threadStartAddress);
    apEventsHandler::instance().registerPendingDebugEvent(event);
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::outputHandlingThreadCreationDebugLogMessage
// Description:
//   Is called when we are notified about debugged process thread creation.
//   Output a debug log printout, describing a created
// Arguments:   threadStartAddress - The created thread start address.
// Author:      Yaki Tebeka
// Date:        19/7/2010
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::outputHandlingThreadCreationDebugLogMessage(osInstructionPointer threadStartAddress, bool isMainThread)
{
    // If debug log severity is DEBUG:
    osDebugLogSeverity debugLogSeverity = osDebugLog::instance().loggedSeverity();

    if (OS_DEBUG_LOG_DEBUG <= debugLogSeverity)
    {
        // When the main thread is created we cannot load modules debug symbols:
        if (!isMainThread)
        {
            // Load debugged process modules symbols:
            _loadedModulesManager.loadLoadedModulesDebugSymbols();
        }

        // Get the module and function in which the thread starts:
        osWin32DebugInfoReader debugInfoReader(_pProcessInfo->hProcess);
        DWORD64 functionStartAddress = 0;
        gtString functionName;
        osFilePath moduleFilePath;
        osInstructionPointer ignored = (osInstructionPointer)NULL;
        debugInfoReader.getFunctionFromAddress(threadStartAddress, functionStartAddress, functionName);
        debugInfoReader.getModuleFromAddress(threadStartAddress, moduleFilePath, ignored);

        gtString dbgMsg = L"Handling debugged process thread creation. ";
        dbgMsg.appendFormattedString(L"Thread start address: %p", threadStartAddress);
        dbgMsg += L", module: ";
        dbgMsg += moduleFilePath.asString();
        dbgMsg += L", function: ";
        dbgMsg += functionName;

        if (isMainThread)
        {
            dbgMsg += L", this is the main thread";
        }

        OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_INFO);
    }
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::handleThreadExit
// Description: Is called when the debugged process thread exists.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        8/6/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::handleThreadExit(const DEBUG_EVENT& debugEvent)
{
    bool retVal = true;

    // Get the thread id:
    DWORD threadId = debugEvent.dwThreadId;

    // Get the thread exit code:
    long exitCode = debugEvent.u.ExitThread.dwExitCode;

    // Handle the thread termination event:
    handleThreadExit(threadId, exitCode);

    // Notice: There is no need to close the thread handle (debugEvent.u.CreateThread.hThread),
    //         since it is closed by the debugging system when the process ends.
    //         (See ms-help://MS.MSDNQTR.2003JAN.1033/dnmag02/html/EscapefromDLLHell.htm)

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::handleThreadExit
// Description: Handles the thread exit event.
// Arguments:   threadId - The id of the thread that was terminated.
//              exitCode - The thread exit code.
// Author:      Yaki Tebeka
// Date:        8/5/2005
// ---------------------------------------------------------------------------
void pdWin32ProcessDebugger::handleThreadExit(DWORD threadId, long exitCode)
{
    // Lock the mutex that controls the access to the threads list:
    osMutexLocker mutexLocker(_threadsListAccessMutex);

    // If this is the main thread id - the process is about to die:
    if (threadId == _mainThreadId)
    {
        _mainThreadId = OS_NO_THREAD_ID;
    }

    // If this is the spies API thread:
    osThreadId APIThreadId = spiesAPIThreadId();

    if (threadId == APIThreadId)
    {
        _spiesAPIThreadId = OS_NO_THREAD_ID;
    }

    // Remove the thread id from the debugged process threads lists:
    removeThreadFromThreadsList(threadId);

    // Unlock the mutex that controls the access to the threads list:
    mutexLocker.unlockMutex();

    // Get the current time:
    osTime currentTime;
    currentTime.setFromCurrentTime();

    // Notify observers about the thread termination:
    apThreadTerminatedEvent event(threadId, exitCode, currentTime);
    apEventsHandler::instance().registerPendingDebugEvent(event);
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::handleDLLLoad
// Description:
//   Is called when the debugged process loads a DLL.
//   a. Keeps the DLL information in _loadedModules and _loadedModulesMap.
//   b. Notifies the registered observers with the DLL name.
//
// Arguments:   debugEvent - The win32 Debug event struct.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        28/11/2003
// Implementation notes:
//   We cannot load the dll debug info here, since when we get this message,
//   we sometimes cannot get the dll loaded size and therefore cannot use
//   SymLoadModuleEx to load its debug info (see more details in case 405 resolution)
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::handleDLLLoad(const DEBUG_EVENT& debugEvent)
{
    bool retVal = true;

    // Get the dll base address (in debugged process address space):
    osInstructionPointer dllBaseAddressInDPAS = (osInstructionPointer)debugEvent.u.LoadDll.lpBaseOfDll;

    // Get a pointer to a pointer to the DLL file name (in debugged process address space):
    LPVOID pPtrLoadedDLLPathInDPAS = debugEvent.u.LoadDll.lpImageName;

    // Is this path a unicode string:
    bool isUnicodeStringPath = (debugEvent.u.LoadDll.fUnicode != 0);

    // Get the DLL name:
    gtString dllName;
    bool rc = getDebuggedProcessDLLName(dllBaseAddressInDPAS, pPtrLoadedDLLPathInDPAS, isUnicodeStringPath, dllName);

    // If we didn't manage to get the dll name - supply a "Unknown DLL index" for it:
    if (!rc)
    {
        dllName = L"Unknown DLL ";
        dllName.appendFormattedString(L"%d", _unknownDllIndex);
        _unknownDllIndex++;
    }

    // Special treatment for nt.dll:
    // - The win32 debugger gives it without its path.
    // - We will add it's path to it.
    if (dllName.compareNoCase(L"ntdll.dll") == 0)
    {
        // On 64-bit systems, we want to show "System32" if the application is 64-bit, and "SysWOW64" for 32-bit.
        // On 32-bit systems, the only value is System32.
        osFilePath kernel32Path(_isDebugging64BitApplication ? osFilePath::OS_SYSTEM_DIRECTORY : osFilePath::OS_SYSTEM_X86_DIRECTORY);
        kernel32Path.setFileName(L"ntdll");
        kernel32Path.setFileExtension(L"dll");
        dllName = kernel32Path.asString();
    }

    // Notify the loaded modules manager:
    _loadedModulesManager.onModuleLoaded(osFilePath(dllName), dllBaseAddressInDPAS);

    // Notify observers about the module load event:
    apModuleLoadedEvent event(debugEvent.dwThreadId, dllName, dllBaseAddressInDPAS);
    apEventsHandler::instance().registerPendingDebugEvent(event);

    // Close open handles:
    CloseHandle(debugEvent.u.LoadDll.hFile);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::handleDLLUnload
//   Is called when the debugged process unloads a DLL.
//   Notifies the registered observers with the DLL name.
//
// Arguments:   debugEvent - The win32 Debug event struct.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/12/2003
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::handleDLLUnload(const DEBUG_EVENT& debugEvent)
{
    bool retVal = true;

    // Get a pointer to a pointer to the unloaded DLL base address:
    osInstructionPointer dllBaseAddress = (osInstructionPointer)(debugEvent.u.UnloadDll.lpBaseOfDll);

    // Notify the loaded modules manager and get the unloaded dll path:
    osFilePath unLoadedDLLPath;
    _loadedModulesManager.onModuleUnloaded(dllBaseAddress, unLoadedDLLPath);

    if (!unLoadedDLLPath.asString().isEmpty())
    {
        // Notify observers about the module unload event:
        apModuleUnloadedEvent event(debugEvent.dwThreadId, unLoadedDLLPath.asString());
        apEventsHandler::instance().registerPendingDebugEvent(event);
    }

    // Do not assert retVal (always fails on 64bit):
    // GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::handleException
// Description:
//   Is called when the debugged process throws an exception.
//   Notifies the registered observers about second chance exceptions.
//
// Arguments:
//   debugEvent - The win32 event struct.
//   win32ContinueStatus - Continue status that will be passed to
//                         the ContinueDebugEvent() win32 function.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/11/2003
// Implementation Notes:
//   On Win32 exceptions can be breakpoints, access violation, etc.
//   When an exception occur:
//   a. The debugger gets a "first chance exception" notification,
//      (so it can "tweak" the debugged process behavior).
//   b. The debugged process gets to handle the exception (the execution search
//      for an exception handler, etc).
//   c. If the debugged process does not handle the exception (it does not have
//      an exception handler for it), then the debugger is notified on a
//      "second chance exception". If the debugger does not prevent it explicitly,
//      the OS runtime ends the debugged process execution.
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::handleException(const DEBUG_EVENT& debugEvent,
                                             unsigned long& win32ContinueStatus)
{
    bool retVal = true;
    win32ContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

    // Get the exception record:
    const EXCEPTION_RECORD& exceptionRec = debugEvent.u.Exception.ExceptionRecord;

    // Will get true iff this is a second chance exception:
    bool isSecondChanceException = false;

    // If this is a second chance exception:
    if (debugEvent.u.Exception.dwFirstChance != TRUE)
    {
        // This is a second chance exception:
        isSecondChanceException = true;
    }
    else
    {
        // This is a first change exception:
        // (The debugger gets the exception before the debugged process)

        // If its a breakpoint exception:
        if (exceptionRec.ExceptionCode == EXCEPTION_BREAKPOINT)
        {
            // For debugging:
            bool areWeDebugging = false;

            if (areWeDebugging)
            {
                // Read the thread call stack:
                osCallStack callStack;

                HANDLE hThread = threadIdToThreadHandle(debugEvent.dwThreadId);
                osWin32CallStackReader callStackReader(_pProcessInfo->hProcess, hThread, callStack);
                callStackReader.execute(false);
            }

            retVal = handleBreakPoint(debugEvent, win32ContinueStatus);
        }
        else if (OS_THREAD_NAMING_EXCEPTION_CODE == exceptionRec.ExceptionCode)
        {
            // This is a thread name change exception, ignore it!
            retVal = true;

            // Also avoid passing it to the debugged process:
            win32ContinueStatus = DBG_CONTINUE;

            // Log the fact we ignored this exception:
            gtString logMsg;
            logMsg.appendFormattedString(L"Ignoring first-chance thread naming exception 0x406D1388, from thread %d", debugEvent.dwThreadId);
            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

            if (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity())
            {
                // Get the current call stack:
                osCallStack threadCallsStack;
                bool wasAlreadySuspended = _isDebuggedProcssSuspended;
                _isDebuggedProcssSuspended = true;
                bool rcStack = getDebuggedThreadCallStack((osThreadId)debugEvent.dwThreadId, threadCallsStack, false);
                _isDebuggedProcssSuspended = wasAlreadySuspended;

                GT_IF_WITH_ASSERT(rcStack)
                {
                    // Print it to the log:
                    gtString stack;
                    gtString ignored;
                    bool ignored2 = false;
                    threadCallsStack.asString(ignored, stack, ignored2);
                    logMsg = L"Ignored thread naming exception call stack:\n";
                    logMsg.append(stack);
                    OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
                }
            }
        }
        else if (_waitingForExecutedFunction)
        {
            // An exception occurred while waiting for an executed function.
            // We will transform this event into a second chance exception, so that
            // we can get a crash stack from the users:
            isSecondChanceException = true;

            // Suspend the thread that executed the function:
            HANDLE threadHandle = threadIdToThreadHandle(_idOfThreadWithStoredContext);
            GT_IF_WITH_ASSERT(threadHandle != OS_NO_THREAD_ID)
            {
                DWORD threadPreviousSuspensionCount = ::SuspendThread(threadHandle);
                GT_ASSERT(threadPreviousSuspensionCount == 0);
            }

            // Mark that the debugged process crashed while executing a function:
            _executedFunctionCrashed = true;
        }
    }

    // If this is second chance exception:
    // (The debugger gets the exception after the debugged process)
    if (isSecondChanceException)
    {
        // Ignore thread rename exceptions:
        if (OS_THREAD_NAMING_EXCEPTION_CODE != exceptionRec.ExceptionCode)
        {
            // This means that either:
            // a. The debugged process didn't have a handler for this exception.
            // b. The exception was continued by the debugged process.
            // In both cases, the debugged process is going to die.

            // Suspend the debugged process:
            if (!_isDebuggedProcssSuspended)
            {
                retVal = suspendDebuggedProcess(debugEvent.dwThreadId);
            }

            // Get the exception code and address:
            unsigned long win32ExceptionCode = exceptionRec.ExceptionCode;
            osInstructionPointer exceptionAddress = (osInstructionPointer)exceptionRec.ExceptionAddress;

            // Translate win32 exception code to ExceptionReason:
            osExceptionReason exceptionReason = osExceptionCodeToExceptionReason(win32ExceptionCode);

            // Notify observers about the second chance exception event:
            apExceptionEvent exceptionEvent(debugEvent.dwThreadId, exceptionReason, exceptionAddress, true);
            apEventsHandler::instance().registerPendingDebugEvent(exceptionEvent);

            // Notify observers that the debugged process run was suspended:
            apDebuggedProcessRunSuspendedEvent processSuspendedEvent(debugEvent.dwThreadId);
            apEventsHandler::instance().registerPendingDebugEvent(processSuspendedEvent);
        }
        else // OS_THREAD_NAMING_EXCEPTION_CODE == exceptionRec.ExceptionCode
        {
            // This is a thread name change exception, ignore it!
            retVal = true;

            // Also avoid passing it to the debugged process:
            win32ContinueStatus = DBG_CONTINUE;

            // Log the fact we ignored this exception:
            gtString logMsg;
            logMsg.appendFormattedString(L"Ignoring second-chance thread naming exception 0x406D1388, from thread %d", debugEvent.dwThreadId);
            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::handleBreakPoint
// Description: Is called when the debugged process hits a breakpoint.
// Arguments:
//   debugEvent - The win32 event struct.
//   win32ContinueStatus - Continue status that will be passed to
//                         the ContinueDebugEvent() win32 function.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        20/5/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::handleBreakPoint(const DEBUG_EVENT& debugEvent, unsigned long& win32ContinueStatus)
{
    bool retVal = true;

    win32ContinueStatus = DBG_CONTINUE;
    bool isTrueBreakPoint = true;

    // If this is the process main thread creation break point:
    // (Windows debugger thro\ws a breakpoint at the beginning of the process main thread run)
    if (!_wasProcessCreationBreakPointHit)
    {
        isTrueBreakPoint = false;

        // Mark that we hit the process creation breakpoint:
        _wasProcessCreationBreakPointHit = true;

        // Inject the main thread entry point breakpoint:
        retVal = injectMainThreadEntryPointBreakPoint();
    }
    else if ((!_wasMainThreadEntryPointBreakpointHit) && (debugEvent.u.Exception.ExceptionRecord.ExceptionAddress == _mainThreadEntryPointAddress))
    {
        // We hit the entry point breakpoint (that we put):

        _wasMainThreadEntryPointBreakpointHit = true;
        isTrueBreakPoint = false;

        // Suspend the main thread:
        bool rc = suspendDebuggedProcessThread(_mainThreadId);
        GT_ASSERT(rc);

        // Restore the main thread entry point op code:
        retVal = restoreMainThreadEntryPointOpCode();
        GT_ASSERT(retVal);

        // Set the debugged process DLL directory to be the Spy directory:
        // (See the above "Loading the Spy DLL" comment)
        const osFilePath& spiesDirPath = _pProcessCreationData->spiesDirectory();
        setDebuggedProcessDLLDirectory(spiesDirPath);

        // Notify observers that the debugged process run started:
        osTime currentTime;
        currentTime.setFromCurrentTime();
        apDebuggedProcessRunStartedEvent event(_pProcessInfo->dwProcessId, currentTime);
        apEventsHandler::instance().registerPendingDebugEvent(event);

        if (_isIgnoringAMD_OCL_BUILD_OPTIONS)
        {
            // Send the user a warning that this environment variable is ignore:
            apUserWarningEvent eventTmp(PD_STR_AMD_OCL_BUILD_OPTIONS_IgnoredWarning);
            apEventsHandler::instance().registerPendingDebugEvent(eventTmp);
        }
    }
    else if (_waitingForExecutedFunction)
    {
        // We are waiting for a function execution (see makeThreadExecuteFunction()):

        // Debug printout:
        OS_OUTPUT_DEBUG_LOG(L"Entered pdWin32ProcessDebugger::handleBreakPoint waiting for exe func section", OS_DEBUG_LOG_DEBUG)

        // Verify that this breakpoint came from the thread that executes the function:
        bool isUsingExecutionInBreakpoint = (functionExecutionMode() == PD_EXECUTION_IN_BREAK_MODE);

        if (isUsingExecutionInBreakpoint)
        {
            // Get the thread handle:
            HANDLE threadHandle = threadIdToThreadHandle(_breakpointTriggeringThreadId);
            GT_IF_WITH_ASSERT(threadHandle != OS_NO_THREAD_ID)
            {
                // Suspend the thread that executed the function:
                DWORD threadPreviousSuspensionCount = ::SuspendThread(threadHandle);
                GT_ASSERT(threadPreviousSuspensionCount == 0);
            }

            isTrueBreakPoint = false;

            // Mark that we finished the function execution:
            _waitingForExecutedFunction = false;
            _executedFuncSyncObj.unlock();
        }
        else if (_idOfThreadWithStoredContext == debugEvent.dwThreadId)
        {
            // Get the thread handle:
            HANDLE threadHandle = threadIdToThreadHandle(_idOfThreadWithStoredContext);
            GT_IF_WITH_ASSERT(threadHandle != OS_NO_THREAD_ID)
            {
                // Suspend the thread that executed the function:
                DWORD threadPreviousSuspensionCount = ::SuspendThread(threadHandle);
                GT_ASSERT(threadPreviousSuspensionCount == 0);

                // Restore the thread execution context:
                restoreThreadExecutionContext(_idOfThreadWithStoredContext);
            }

            isTrueBreakPoint = false;

            // Mark that we finished the function execution:
            _waitingForExecutedFunction = false;
            _executedFuncSyncObj.unlock();
        }
        else // && (_idOfThreadWithStoredContext != debugEvent.dwThreadId)
        {
            // We got a foreign breakpoint while waiting for executed function:
            OS_OUTPUT_DEBUG_LOG(L"Foreign breakpoint was thrown while waiting for debugged process function execution", OS_DEBUG_LOG_ERROR);
        }
    }

    // If this is a true breakpoint (not the process creation breakpoint or the breakpoint
    // that we inject at the main thread entry point):
    if (isTrueBreakPoint)
    {
        // Get the exception address:
        const EXCEPTION_RECORD& exceptionRec = debugEvent.u.Exception.ExceptionRecord;
        osInstructionPointer exceptionAddress = (osInstructionPointer)exceptionRec.ExceptionAddress;

        // Check if this breakpoint is an API breakpoint:
        bool isSpiesBreakpoint = _loadedModulesManager.isSpyServerAddress(exceptionAddress);

        if (isSpiesBreakpoint)
        {
            // The breakpoints before and after kernel debugging should not trigger breakpoint and
            // suspension events, they are just internal so we could freeze all threads except the
            // API thread and the kernel debugging thread.
            if (_isKernelDebuggingAboutToStart)
            {
                // This is the breakpoint just before kernel debugging starts:
                _isKernelDebuggingAboutToStart = false;

                // Suspend all threads except the API thread and the kernel debugging thread (which is
                // the thread which caused the breakpoint):
                // TO_DO: This seems to cause some weird crash / hang, look into it:
                // suspendDebuggedProcess();
                // resumeDebuggedProcessThread(debugEvent.dwThreadId);
                retVal = true;
            }
            else if (_isKernelDebuggingJustFinished)
            {
                // This is the breakpoint just after kernel debugging ended:
                _isKernelDebuggingJustFinished = false;

                // Release all the threads suspended before:
                // TO_DO: This seems to cause some weird crash / hang, look into it:
                // suspendDebuggedProcessThread(debugEvent.dwThreadId);
                // resumeDebuggedProcess();
                retVal = true;
            }
            else // !(_isKernelDebuggingAboutToStart || _isKernelDebuggingJustFinished)
            {
                // Suspend the debugged process:
                retVal = suspendDebuggedProcess(debugEvent.dwThreadId);

                if (retVal)
                {
                    // Note which thread caused this breakpoint:
                    _breakpointTriggeringThreadId = debugEvent.dwThreadId;

                    // Notify observers about the breakpoint event:
                    apBreakpointHitEvent event(debugEvent.dwThreadId, exceptionAddress);
                    apEventsHandler::instance().registerPendingDebugEvent(event);

                    // Notify observers that the debugged process run was suspended:
                    apDebuggedProcessRunSuspendedEvent processSuspendedEvent(debugEvent.dwThreadId);
                    apEventsHandler::instance().registerPendingDebugEvent(processSuspendedEvent);
                }
            }
        }
        else
        {
            retVal = true;
        }
    }

    // Debug printout:
    OS_OUTPUT_DEBUG_LOG(L"pdWin32ProcessDebugger::handleBreakPoint finished working", OS_DEBUG_LOG_DEBUG);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::handleDebugString
// Description: Is called when the debugged process outputs a debug string.
//              Notifies the registered observers with the debug string.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/12/2003
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::handleDebugString(const DEBUG_EVENT& debugEvent)
{
    bool retVal = false;

    // A pointer to the string in the debugged process address space:
    void* stringAddress = debugEvent.u.DebugString.lpDebugStringData;

    // The string length:
    unsigned long stringSize = debugEvent.u.DebugString.nDebugStringLength;

    // Is it a unicode string ?
    bool isUnicodeString = (debugEvent.u.DebugString.fUnicode != 0);

    // Read the string from the debugged process memory:
    gtString debugString;
    bool rc = readStringFromDebuggedProcessMemory(stringAddress, stringSize, isUnicodeString, debugString);

    if (rc)
    {
        // If a Spy notifies us that the debugged process is going to terminate:
        static const gtString stat_debuggedProcessIsTerminatingString = OS_STR_debuggedProcessIsTerminating;

        if (debugString.find(stat_debuggedProcessIsTerminatingString) == 0)
        {
            // Mark that we are during debugged process termination:
            _isDuringDebuggedProcessTermination = true;

            // A new thread is becoming the API calls handling thread. Get its id:
            static const int threadIsStartPos = stat_debuggedProcessIsTerminatingString.length();
            int threadIsEndPos = debugString.length() - 1;
            gtString newAPIThreadIdAsStr;
            debugString.getSubString(threadIsStartPos, threadIsEndPos, newAPIThreadIdAsStr);
            osThreadId newAPIThreadId = OS_NO_THREAD_ID;
            bool rcNum = newAPIThreadIdAsStr.toUnsignedLongNumber(newAPIThreadId);
            GT_IF_WITH_ASSERT(rcNum)
            {
                // Mark it as the new API thread id:
                setSpiesAPIThreadId(newAPIThreadId);
            }
        }
        else
        {
            // Increment the amount of output string printouts:
            _amountOfOutputStringPrintouts++;

            if (_amountOfOutputStringPrintouts == PD_MAX_OUTPUT_STRING_PRINTOUTS)
            {
                // Tell the user that we reached the maximal debug printouts:
                static gtString maxReportsMsg;
                maxReportsMsg.appendFormattedString(PD_STR_OutputDebugStringMaxPrintoutsReached, PD_MAX_OUTPUT_STRING_PRINTOUTS);
                apOutputDebugStringEvent event(debugEvent.dwThreadId, maxReportsMsg);
                apEventsHandler::instance().registerPendingDebugEvent(event);
            }
            else if (_amountOfOutputStringPrintouts < PD_MAX_OUTPUT_STRING_PRINTOUTS)
            {
                // We got a debug string from the debugged process. Trigger an "Output debug string" event:
                apOutputDebugStringEvent event(debugEvent.dwThreadId, debugString);
                apEventsHandler::instance().registerPendingDebugEvent(event);
            }

            // Otherwise ignore the printouts:
        }

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::handleWin32DebuggerError
// Description: Handles Win32 debugger errors.
// Arguments: debugEvent - A struct that contains the error data.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/4/2006
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::handleWin32DebuggerError(const DEBUG_EVENT& debugEvent)
{
    bool retVal = true;

    // Output an error message to the log file:
    GT_ASSERT_EX(false, PD_STR_win32DebuggerError);

    // It it was an error that will cause the debugged process to fail:
    if (debugEvent.u.RipInfo.dwType == SLE_ERROR)
    {
        GT_ASSERT_EX(false, PD_STR_win32DebuggerFatalError);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::readPointerFromDebuggedProcessMemory
// Description: Reads a pointer value from the debugged process memory.
// Arguments:   pointerAddress - The address of the pointer in the debugged process
//                               address space.
//              pPointerValue - Pointer to a pointer that will get the read pointer value.
//                              Notice that this pointer is given in debugged process
//                              address space.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        28/11/2003
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::readPointerFromDebuggedProcessMemory(void* pointerAddress,
                                                                  void** pPointerValue) const
{
    bool retVal = false;

    // Read the pointer value into *pPointerValue:
    SIZE_T numberOfBytesRead = 0;
    size_t pointerSize = sizeof(void*);
    BOOL rc = ::ReadProcessMemory(_pProcessInfo->hProcess,
                                  pointerAddress, pPointerValue,
                                  pointerSize, &numberOfBytesRead);

    if ((rc != 0) && (numberOfBytesRead == pointerSize))
    {
        retVal = true;
    }

    return retVal;
}


// This uses a template with specializations since we cannot use the condition:
// "#if (sizeof(TCHAR) == 1)":
template<unsigned int SIZEOF_TCHAR> void copyTCHARStringFromString(TCHAR* readStringBuf, WCHAR* readStringInDPCSBuf, size_t& readStringBufSize, bool isUnicodeString);

template<>
void copyTCHARStringFromString<1>(TCHAR* readStringBuf, WCHAR* readStringInDPCSBuf, size_t& readStringBufSize, bool isUnicodeString)
{
    // This process uses ANSI strings (sizeof(TCHAR) == 1):
    if (isUnicodeString)
    {
        // Copy from Unicode:
        size_t i = wcstombs((CHAR*)readStringBuf, (WCHAR*)readStringInDPCSBuf, readStringBufSize);
        GT_ASSERT(i >= 0);
    }
    else
    {
        // Copy from ANSI:
        strncpy_s((CHAR*)readStringBuf, readStringBufSize, (CHAR*)readStringInDPCSBuf, readStringBufSize);
    }
}

template<>
void copyTCHARStringFromString<2>(TCHAR* readStringBuf, WCHAR* readStringInDPCSBuf, size_t& readStringBufSize, bool isUnicodeString)
{
    // This process uses unicode strings (sizeof(TCHAR) == 2):
    if (isUnicodeString)
    {
        // Copy from Unicode:
        wcsncpy((WCHAR*)readStringBuf, (WCHAR*)readStringInDPCSBuf, readStringBufSize);
    }
    else
    {
        // Copy from ANSI:
        size_t i = mbstowcs((WCHAR*)readStringBuf, (CHAR*)readStringInDPCSBuf, readStringBufSize);
        GT_ASSERT(i >= 0);
    }
}

// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::readStringFromDebuggedProcessMemory
// Description: Reads a string from the debugged process memory.
// Arguments:   stringAddress - The string address (In debugged process address space).
//              stringSize - The string size.
//              isUnicodeString - true - if the string in the debugged process memory
//                                       is represented in unicode format.
//                                false - if the string in the debugged process memory
//                                        is represented in ASCII format.
//              readString - Will get the read string.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        28/11/2003
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::readStringFromDebuggedProcessMemory(void* stringAddress, unsigned long stringSize, bool isUnicodeString, gtString& readString) const
{
    bool retVal = false;

    // A buffer that will contain the read string in debugged process character set:
    size_t readStringBufSize = stringSize + 1;
    WCHAR* readStringInDPCSBuf = new WCHAR[readStringBufSize];

    // A buffer that will contain the read string in this process character set:
    TCHAR* readStringBuf = new TCHAR[readStringBufSize];

    if (readStringInDPCSBuf && readStringBuf)
    {
        // Make the string buffers contains NULLs:
        ::ZeroMemory((LPVOID)readStringInDPCSBuf, readStringBufSize * sizeof(WCHAR));
        ::ZeroMemory(readStringBuf, readStringBufSize * sizeof(TCHAR));

        // Calculate the amount of characters to read:
        int amountOfCharsToRead = isUnicodeString ? stringSize * 2 : stringSize;

        // Read the string from the debugged process memory:
        SIZE_T numberOfBytesRead = 0;
        BOOL rc = ::ReadProcessMemory(_pProcessInfo->hProcess,
                                      stringAddress, (LPVOID)readStringInDPCSBuf,
                                      amountOfCharsToRead, &numberOfBytesRead);

        if ((rc != 0) && (numberOfBytesRead == stringSize))
        {
            // Handle separately if this process uses ANSI strings or unicode ones:
            copyTCHARStringFromString<sizeof(TCHAR)>(readStringBuf, readStringInDPCSBuf, readStringBufSize, isUnicodeString);

            // Translate the string into a gtString:
            // TO_DO: The below code is a bug when this process uses unicode strings:
            readString = readStringBuf;
            retVal = true;
        }

        // Clean up:
        delete[] readStringInDPCSBuf;
        delete[] readStringBuf;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::injectMainThreadEntryPointBreakPoint
// Description: Injects a breakpoint command at the main thread entry point.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        27/6/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::injectMainThreadEntryPointBreakPoint()
{
    bool retVal = false;

    // Store the main thread entry point Op code
    SIZE_T numberOfBytesRead = 0;
    size_t sizeOfByte = sizeof(BYTE);
    BOOL rc = ::ReadProcessMemory(_pProcessInfo->hProcess,
                                  _mainThreadEntryPointAddress,
                                  &_mainThreadEntryPointOpCode,
                                  sizeOfByte,
                                  &numberOfBytesRead);

    if ((rc != FALSE) && (numberOfBytesRead == sizeOfByte))
    {
        // Inject a breakpoint command at the main thread entry point:
        BYTE breakPointOpCode = 0xCC;
        SIZE_T numberOfBytesWritten = 0;
        rc = ::WriteProcessMemory(_pProcessInfo->hProcess,
                                  _mainThreadEntryPointAddress,
                                  &breakPointOpCode,
                                  sizeOfByte,
                                  &numberOfBytesWritten);

        if ((rc != FALSE) && (numberOfBytesWritten == sizeOfByte))
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::restoreMainThreadEntryPointOpCode
// Description:
//  Restores the main thread entry point OpCode.
//  This opcode was replaced by us to contain a breakpoint command.
//  (See injectMainThreadEntryPointBreakPoint()).
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        27/6/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::restoreMainThreadEntryPointOpCode()
{
    bool retVal = false;

    // Restore the main thread entry point OpCode:
    size_t sizeOfByte = sizeof(BYTE);
    SIZE_T numberOfBytesWritten = 0;
    BOOL rc = ::WriteProcessMemory(_pProcessInfo->hProcess,
                                   _mainThreadEntryPointAddress,
                                   &_mainThreadEntryPointOpCode,
                                   sizeOfByte,
                                   &numberOfBytesWritten);

    if ((rc != FALSE) && (numberOfBytesWritten == sizeOfByte))
    {
        // Get the main thread handle:
        HANDLE mainThreadHandle = threadIdToThreadHandle(_mainThreadId);

        if (mainThreadHandle != NULL)
        {
            // Get the current main thread context:
            CONTEXT mainThreadContext;
            memset(&mainThreadContext, 0, sizeof(mainThreadContext));
            mainThreadContext.ContextFlags = CONTEXT_FULL;
            rc = GetThreadContext(mainThreadHandle, &mainThreadContext);

            if (rc != FALSE)
            {
                // Move the main thread P.C back to the entry op code:
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
                mainThreadContext.Eip = mainThreadContext.Eip - 1;
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
                mainThreadContext.Rip = mainThreadContext.Rip - 1;
#else
#error Unknown address space size!
#endif // AMDT_ADDRESS_SPACE_TYPE
                rc = SetThreadContext(mainThreadHandle, &mainThreadContext);

                if (rc != FALSE)
                {
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::setDebuggedProcessDLLDirectory
// Description: Calls SetDLLDirectory in the debugged process with an input
//              directory path.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/11/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::setDebuggedProcessDLLDirectory(const osFilePath& dllDirectoryPath)
{
    bool retVal = false;

    pdWin32SetRemoteProcessDLLDirectory dllDirectoryAssinger(_pProcessInfo->hProcess, dllDirectoryPath);
    retVal = dllDirectoryAssinger.execute();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::getDebuggedProcessDLLName
// Description: Returns the name of a debugged process dll.
// Arguments:   dllBaseAddressInDPAS - The base address of the dll (in debugged process address space).
//              pPtrLoadedDLLPathInDPAS - The dll path (in debugged process address space).
//              isUnicodeStringPath - Is the dll path given as a unicode string.
//              dllName - Will get the DLL name.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/9/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::getDebuggedProcessDLLName(osInstructionPointer dllBaseAddressInDPAS,
                                                       LPVOID pPtrLoadedDLLPathInDPAS, bool isUnicodeStringPath,
                                                       gtString& dllName) const
{
    bool retVal = false;

    // If we have the loaded dll path name written in the debugged process address space:
    if (pPtrLoadedDLLPathInDPAS)
    {
        // A pointer to the DLL file name (in debugged process address space):
        LPVOID pLoadedDLLPathInDPAS = NULL;

        // Read the pointer to pointer value into pLoadedDLLPathInDPAS:
        bool rc = readPointerFromDebuggedProcessMemory(pPtrLoadedDLLPathInDPAS,
                                                       &pLoadedDLLPathInDPAS);

        if (rc && (pLoadedDLLPathInDPAS != NULL))
        {
            // Read the loaded DLL path:
            rc = readStringFromDebuggedProcessMemory(pLoadedDLLPathInDPAS, MAX_PATH, isUnicodeStringPath, dllName);

            retVal = rc;
        }
    }

    // If we didn't manage to get the dll name - we try to get it using GetMappedFileName:
    if (!retVal)
    {

        // Get the DLL name using GetMappedFileName:
        wchar_t dllNameBuff[MAX_PATH];
        DWORD dllNameSize = GetMappedFileName(_pProcessInfo->hProcess, (HMODULE)dllBaseAddressInDPAS, dllNameBuff, MAX_PATH);

        if (dllNameSize > 0)
        {
            dllName = dllNameBuff;

            // Try to translate the device name to dos drive letter:
            deviceNameToFileName(dllName, dllName);

            retVal = true;
        }
    }

    return retVal;
}




// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::deviceNameToFileName
// Description: Translated from device name into a file name.
// Arguments:   deviceName - The input device name.
//              fileName - The output file name.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        27/9/2004
// Implementation notes:
//   The "inspiration" for this function code was taken from the MSDN article
//   "Obtaining a File Name From a File Handle".
//   (ms-help://MS.MSDNQTR.2003JAN.1033/fileio/base/obtaining_a_file_name_from_a_file_handle.htm)
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::deviceNameToFileName(const gtString& deviceName, gtString& fileName) const
{
    bool retVal = false;

    // Get the size of the buffer needed for holding the logical drives mapping:
    int buffSize = GetLogicalDriveStrings(0, NULL);

    if (buffSize > 0)
    {
        // Allocate space for the logical drives map:
        wchar_t* pLogicalDriversMap = new wchar_t[buffSize + 5];
        pLogicalDriversMap[0] = NULL;

        // Get the logical drives map:
        // (See GetLogicalDriveStrings for this map structure)
        if (GetLogicalDriveStrings(buffSize + 4, pLogicalDriversMap))
        {
            wchar_t currentDriveString[3] = L" :";
            wchar_t* pMapCurrentPosition = pLogicalDriversMap;

            // Iterate the available drives mappings:
            do
            {
                // Copy the current drive letter into the drive string template:
                currentDriveString[0] = *pMapCurrentPosition;

                // Get the name of the DOS device that corresponds to this drive letter:
                wchar_t currentDeviceDosName[MAX_PATH];

                if (QueryDosDevice(currentDriveString, currentDeviceDosName, MAX_PATH))
                {
                    // Did we find the device that the file name contains:
                    size_t currentDeviceDosNameSize = wcslen(currentDeviceDosName);
                    retVal = (_tcsnicmp(deviceName.asCharArray(), currentDeviceDosName, currentDeviceDosNameSize) == 0);

                    if (retVal)
                    {
                        // Reconstruct the file name (replace the device name with the dos
                        // drive string)
                        wchar_t newFileName[MAX_PATH];
                        size_t currentDeviceDosNameLen = wcslen(currentDeviceDosName);
                        swprintf_s(newFileName, L"%ls%ls", currentDriveString, deviceName.asCharArray() + currentDeviceDosNameLen);
                        fileName = newFileName;
                    }
                }

                // Go to the next device in the device map (look for the next NULL character).
                while (*pMapCurrentPosition++) {};

            }
            while (!retVal && *pMapCurrentPosition);
        }

        // Clean up:
        delete[] pLogicalDriversMap;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::setDebuggedProcessEnvVariables
// Description:
//   Sets additional debugged process environment variables that:
//   a. Appear in the process creation data.
//   b. Add the process creation data working directory to the %PATH% environment variable.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        6/9/2005
// Implementation notes:
//   The easiest way to implement this functionality is to:
//   a. Add the additional environment variables to the current (debugger) process.
//   b. Create the debugged process in a way that it copies the current process
//      environment block (See MSDN documentation of CreateProcess and SetEnvironmentVariable).
//   c. After the debugged process was created, remove the added additional environment variables.
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::setDebuggedProcessEnvVariables()
{
    bool retVal = true;

    // a. Add additional environment variable values:
    // =============================================

    // Iterate the additional environment variables:
    const gtList<osEnvironmentVariable>& additionalEnvVariables = _pProcessCreationData->environmentVariables();
    gtList<osEnvironmentVariable>::const_iterator iter = additionalEnvVariables.begin();
    gtList<osEnvironmentVariable>::const_iterator endIter = additionalEnvVariables.end();
    bool wasDebugHeapVariableSet = false;

    while (iter != endIter)
    {
        // Get the current value the current environment variable:
        osEnvironmentVariable currentVal;
        currentVal._name = (*iter)._name;
        osGetCurrentProcessEnvVariableValue(currentVal._name, currentVal._value);
        _storedEnvironmentVariablesValues.push_back(currentVal);

        // If this is the _NO_DEBUG_HEAP variable, mark it down:
        if (currentVal._name == PD_DEBUG_HEAP_ENV_VARIABLE_NAME)
        {
            wasDebugHeapVariableSet = true;
        }

        // Set the current environment value in this process environment block:
        bool rc2 = osSetCurrentProcessEnvVariable(*iter);

        // In case of failure:
        if (!rc2)
        {
            // Generate an assertion failure:
            gtString errMessage = L"Error: Failed to set the environment variable: ";
            errMessage += (*iter)._name;
            GT_ASSERT_EX(false, errMessage.asCharArray());

            retVal = false;
        }

        ++iter;
    }


    // b. Add debugged process working directory to the %PATH% environment variable:
    // ============================================================================
    bool rc3 = addDebuggedProcessWorkDirToPath();
    retVal = retVal && rc3;

    // If the user didn't specify the _NO_DEBUG_HEAP variable:
    if (!wasDebugHeapVariableSet)
    {
        // Set it to 1:
        osEnvironmentVariable debugHeapEnvVar;
        debugHeapEnvVar._name = PD_DEBUG_HEAP_ENV_VARIABLE_NAME;
        debugHeapEnvVar._value = '1';
        osSetCurrentProcessEnvVariable(debugHeapEnvVar);

        // Mark it to be cleared afterwards:
        debugHeapEnvVar._value.makeEmpty();
        _storedEnvironmentVariablesValues.push_back(debugHeapEnvVar);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::addDebuggedProcessWordDirToPath
// Description:
//   Adds the debugged process work directory to the %PATH% environment variable.
//   Also updates _storedEnvironmentVariablesValues to contain the original %PATH%
//   environment variable value
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/7/2008
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::addDebuggedProcessWorkDirToPath()
{
    bool retVal = false;

    // The %PATH% environment variable name:
    static const gtString pathEnvVariableName(PD_PATH_ENV_VARIABLE_NAME);

    // Get the current %PATH% environment variable value:
    osEnvironmentVariable pathOriginalVal;
    pathOriginalVal._name = pathEnvVariableName;
    bool rc1 = osGetCurrentProcessEnvVariableValue(pathOriginalVal._name, pathOriginalVal._value);

    if (!rc1)
    {
        // We didn't have a PATH environment variable (should not happened on Windows OS):
        GT_ASSERT(false);
        pathOriginalVal._value.makeEmpty();
    }

    // Store its value so it can be resorted later:
    _storedEnvironmentVariablesValues.push_back(pathOriginalVal);

    // Add the debugged process work directory to the %PATH% environment variable value:
    osEnvironmentVariable pathModifiedVal;
    pathModifiedVal._name = pathEnvVariableName;
    pathModifiedVal._value = pathOriginalVal._value;
    pathModifiedVal._value += osFilePath::osEnvironmentVariablePathsSeparator;
    pathModifiedVal._value += _pProcessCreationData->workDirectory().asString();

    // Set the  %PATH% environment variable value to the modified value:
    bool rc2 = osSetCurrentProcessEnvVariable(pathModifiedVal);
    GT_IF_WITH_ASSERT(rc2)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::removeDebuggedProcessEnvVariables
// Description: Removes the added additional environment variables added
//              to this process by setDebuggedProcessEnvVariables.
//              (See setDebuggedProcessEnvVariables documentation for more details).
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/9/2005
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::removeDebuggedProcessEnvVariables()
{
    bool retVal = true;

    // Iterate the stored environment variables values:
    gtList<osEnvironmentVariable>::const_iterator iter = _storedEnvironmentVariablesValues.begin();
    gtList<osEnvironmentVariable>::const_iterator endIter = _storedEnvironmentVariablesValues.end();

    while (iter != endIter)
    {
        bool isCurVarValueStores = false;

        // Get the current environment variable stored value:
        const osEnvironmentVariable& currentStoredEnvVar = *iter;

        // It the variable had a value when it was stored:
        if (!(currentStoredEnvVar._value.isEmpty()))
        {
            isCurVarValueStores = osSetCurrentProcessEnvVariable(currentStoredEnvVar);
        }
        else
        {
            // The variable didn't exist when it was stored:
            isCurVarValueStores = osRemoveCurrentProcessEnvVariable(currentStoredEnvVar._name);
        }

        retVal = retVal && isCurVarValueStores;

        iter++;
    }

    // Clear the stored environment variables values list:
    _storedEnvironmentVariablesValues.clear();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::removeThreadFromThreadsList
// Description:
//   Removes a thread from this class threads list (_debuggedProcessThreadsData).
//
//   !! Notice !!!
//   The caller to this function is responsible for locking the threads list access
//   mutex (_threadsListAccessMutex).
//
// Arguments:  threadId - The thread to be removed.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        20/4/2006
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::removeThreadFromThreadsList(DWORD threadId)
{
    bool retVal = false;

    // Lock the mutex that controls the access to the threads list:
    osMutexLocker mutexLocker(_threadsListAccessMutex);

    // Iterate on the debugged process threads:
    gtList<pdDebuggedProcessThreadData>::iterator endIter = _debuggedProcessThreadsData.end();
    gtList<pdDebuggedProcessThreadData>::iterator iter = _debuggedProcessThreadsData.begin();

    while (iter != endIter)
    {
        // If this is the thread we are looking for:
        DWORD currentThreadId = (*iter)._threadId;

        if (currentThreadId == threadId)
        {
            // Delete it from the threads data list:
            _debuggedProcessThreadsData.erase(iter);

            // Exit the loop:
            retVal = true;
            break;
        }

        iter++;
    }

    // Unlock the mutex that controls the access to the threads list:
    mutexLocker.unlockMutex();

    GT_RETURN_WITH_ASSERT(retVal);
}


// -----------------------------------------------------------------------
//  !!      Functions that are used by both thread      !!
// -----------------------------------------------------------------------




// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::suspendDebuggedProcess
// Description:
//   Suspends all the debugged process threads, except the Spy
//   API handling thread.
//
// Arguments:
//   triggeringThreadId - The thread that triggered the process suspension
//                        (Or - PD_NO_THREAD_ID)
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        8/6/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::suspendDebuggedProcess(osThreadId triggeringThreadId)
{
    bool retVal = true;

    // Lock the mutex that controls the access to the threads list:
    osMutexLocker mutexLocker(_threadsListAccessMutex);

    // Update threads data:
    updateThreadsData();

    // Iterate on the debugged process threads:
    gtList<pdDebuggedProcessThreadData>::const_iterator endIter = _debuggedProcessThreadsData.end();
    gtList<pdDebuggedProcessThreadData>::const_iterator iter = _debuggedProcessThreadsData.begin();

    while (iter != endIter)
    {
        // If the current thread is not a Spy API thread:
        osThreadId currentThreadId = (*iter)._threadId;
        bool isAPIThreadId = isSpyAPIThread(currentThreadId);

        if (!isAPIThreadId)
        {
            // If it is not a driver thread:
            // (See case and 1461, 1616)
            bool isKernelDebuggingThread = (_isDuringKernelDebugging && ((*iter)._threadId == triggeringThreadId));

            if (!((*iter)._isDriverThread) || isKernelDebuggingThread)
            {
                // Suspend it:
                bool rc = suspendDebuggedProcessThread((*iter)._threadHandle);

                if (!rc)
                {
                    retVal = false;
                }
            }
        }

        iter++;
    }

    // Mark the debugged process as suspended:
    _isDebuggedProcssSuspended = true;

    // Unlock the mutex that controls the access to the threads list:
    mutexLocker.unlockMutex();

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::suspendDebuggedProcessThread
// Description: Suspends a given thread's run.
// Arguments: threadHandle - The handle of the thread to be suspended.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/4/2006
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::suspendDebuggedProcessThread(osThreadHandle threadHandle)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(threadHandle != NULL)
    {
        // Suspend the input thread:
        DWORD previousThreadSuspendCount = ::SuspendThread(threadHandle);
        GT_IF_WITH_ASSERT(previousThreadSuspendCount != -1)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::suspendDebuggedProcessThread
// Description: Suspends a debugged process thread.
// Arguments:   threadId - The input thread id.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/6/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::suspendDebuggedProcessThread(osThreadId threadId)
{
    bool retVal = false;

    // Find the input thread handle:
    HANDLE threadHandle = threadIdToThreadHandle(threadId);

    if (threadHandle != NULL)
    {
        // Suspend it:
        retVal = suspendDebuggedProcessThread(threadHandle);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::resumeDebuggedProcessThread
// Description: Resumes the run of a suspended thread.
// Arguments: threadHandle - The handle of the thread who's run should be resumed.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/4/2006
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::resumeDebuggedProcessThread(osThreadHandle threadHandle)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(threadHandle != NULL)
    {
        // Resume the thread run:
        DWORD previousThreadSuspendCount = ::ResumeThread(threadHandle);
        GT_IF_WITH_ASSERT(previousThreadSuspendCount != -1)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::resumeDebuggedProcessThread
// Description: Resumes a debugged process thread.
// Arguments:   threadId - The input thread id.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/6/2004
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::resumeDebuggedProcessThread(osThreadId threadId)
{
    bool retVal = false;

    // Find the input thread handle:
    HANDLE threadHandle = threadIdToThreadHandle(threadId);

    if (threadHandle != NULL)
    {
        retVal = resumeDebuggedProcessThread(threadHandle);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::storeThreadExecutionContext
// Description: Stores a given thread execution context.
// Arguments: threadId - The id of the thread who's execution context will be stored.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/10/2005
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::storeThreadExecutionContext(osThreadId threadId)
{
    bool retVal = false;

    // Get the input thread handle:
    HANDLE threadHandle = threadIdToThreadHandle(threadId);

    if (threadHandle != NULL)
    {
        // Store the id of the thread to which we store the execution context:
        _idOfThreadWithStoredContext = threadId;

        // Store the current thread execution context:
        memset(&_storedThreadContext, 0, sizeof(_storedThreadContext));
        _storedThreadContext.ContextFlags = CONTEXT_FULL;
        BOOL rc = ::GetThreadContext(threadHandle, &_storedThreadContext);

        if (rc != 0)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::restoreThreadExecutionContext
// Description: Restores a given thread execution context, previously stored by
//              storeThreadExecutionContext.
// Arguments: threadId - The id of the thread who's execution context will be stored.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/10/2005
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::restoreThreadExecutionContext(osThreadId threadId)
{
    bool retVal = false;

    // Verify that the stored context belongs to the input thread:
    GT_IF_WITH_ASSERT(threadId == _idOfThreadWithStoredContext)
    {
        // Get the input thread handle:
        HANDLE threadHandle = threadIdToThreadHandle(threadId);

        if (threadHandle != NULL)
        {
            // Move it back to the context in which it was before the call to makeThreadExecuteFunction:
            BOOL rc1 = ::SetThreadContext(threadHandle, &_storedThreadContext);
            retVal = (rc1 != FALSE);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::threadIdToThreadHandle
// Description:
//   Translated a thread Id to a thread Handle.
//   Returns NULL in case of failure.
//   Notice: The returned handle should not need to be released !!
// Author:      Yaki Tebeka
// Date:        18/8/2004
// ---------------------------------------------------------------------------
HANDLE pdWin32ProcessDebugger::threadIdToThreadHandle(DWORD threadId)
{
    HANDLE retVal = NULL;

    // Lock the mutex that controls the access to the threads list:
    osMutexLocker mutexLocker(_threadsListAccessMutex);

    // Iterate on the debugged process threads:
    gtList<pdDebuggedProcessThreadData>::const_iterator endIter = _debuggedProcessThreadsData.end();
    gtList<pdDebuggedProcessThreadData>::const_iterator iter = _debuggedProcessThreadsData.begin();

    while (iter != endIter)
    {
        // If this is the thread we are looking for:
        DWORD currentThreadId = (*iter)._threadId;

        if (currentThreadId == threadId)
        {
            // Output the thread handle:
            retVal = (*iter)._threadHandle;

            // Exit the loop:
            break;
        }

        iter++;
    }

    // Unlock the mutex that controls the access to the threads list:
    mutexLocker.unlockMutex();

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        pdWin32ProcessDebugger::isSpyAPIThread
// Description: Inputs a thread id and returns true iff the thread is a Spy API thread.
// Author:      Yaki Tebeka
// Date:        16/11/2009
// ---------------------------------------------------------------------------
bool pdWin32ProcessDebugger::isSpyAPIThread(osThreadId threadId) const
{
    bool retVal = false;

    // If this is the spies API thread:
    osThreadId OGLSpyAPIiThreadId = spiesAPIThreadId();

    if (threadId == OGLSpyAPIiThreadId)
    {
        retVal = true;
    }

    return retVal;
}


