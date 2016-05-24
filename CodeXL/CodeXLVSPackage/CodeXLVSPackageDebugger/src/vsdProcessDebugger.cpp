//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdProcessDebugger.cpp
///
//==================================================================================

//------------------------------ vsdProcessDebugger.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessOutputStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunResumedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDeferredCommandEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>

// Local:
#include <src/vsdDebugEventCallback.h>
#include <src/vsdDebugPort.h>
#include <src/vsdProcessDebugger.h>
#include <CodeXLVSPackageDebugger/Include/vsdPackageConnector.h>

#define VSD_MAKE_THREAD_EXECUTE_TIMEOUT_MS 10000

// CODEXL-2207, CODEXL-2287: it seems that interfaces obtained from IEnumDebug*2 classes
// Under *_INFO structs should not be explicitly released:
// #define VSD_RELEASE_ENUMERATED_INTERFACES 1

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdProcessDebugger
// Description: Constructor
// Author:      Uri Shomroni
// Date:        25/12/2011
// ---------------------------------------------------------------------------
vsdProcessDebugger::vsdProcessDebugger(IDebugEngine2& riNativeDebugEngine, IDebugProgramProvider2& riNativeProgramProvider)
    : m_pProcessCreationData(NULL), m_debuggedProcessExists(false), m_isDebuggedProcessSuspended(false),
      m_isDebuggedProcessAtFatalException(false), m_processId(0), m_hProcess(nullptr),
      m_mainThreadId(OS_NO_THREAD_ID), m_spiesAPIThreadId(OS_NO_THREAD_ID), m_gotIs64Bit(false), m_is64Bit(false), m_suspendThreadOnEntryPoint(false),
      m_isKernelDebuggingAboutToStart(false), m_isDuringKernelDebugging(false), m_isKernelDebuggingJustFinished(false),
      m_bpThreadId(OS_NO_THREAD_ID), m_isAPIBreakpoint(false), m_hostBreakReason(AP_FOREIGN_BREAK_HIT),
      m_currentBreakpointRequestedLine(-1), m_lastStepKind(AP_FOREIGN_BREAK_HIT), m_waitingForDeferredStep(false),
      m_isWaitingForProcessSuspension(false), m_waitingForExecutedFunction(false), m_piEventToContinue(nullptr),
      m_skipContinueEvent(false), m_issuingStepCommand(false),
      m_piNativeDebugEngine(&riNativeDebugEngine), m_piNativeDebugEngineLaunch(NULL),
      m_piNativeProgramProvider(&riNativeProgramProvider),
      m_piDebugPort(NULL), m_piDebugDefaultPort(NULL),
      m_piProcess(NULL), m_piAttachedProgram(nullptr), m_piProgram(NULL), m_debuggedProcessSuspensionThread1(nullptr), m_debuggedProcessSuspensionThread2(nullptr),
      m_pDebugEventCallback(NULL)
{
    // Retain the debug interfaces:
    m_piNativeDebugEngine->AddRef();
    m_piNativeProgramProvider->AddRef();

    // Get the IDebugEngineLaunch2 interface:
    HRESULT hr = m_piNativeDebugEngine->QueryInterface(IID_IDebugEngineLaunch2, (void**)(&m_piNativeDebugEngineLaunch));
    GT_ASSERT(SUCCEEDED(hr));
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::~vsdProcessDebugger
// Description: Destructor
// Author:      Uri Shomroni
// Date:        25/12/2011
// ---------------------------------------------------------------------------
vsdProcessDebugger::~vsdProcessDebugger()
{
    initialize();

    // Clean up the Debug Engine interface:
    GT_IF_WITH_ASSERT(m_piNativeDebugEngine != NULL)
    {
        m_piNativeDebugEngine->Release();
        m_piNativeDebugEngine = NULL;
    }

    GT_IF_WITH_ASSERT(m_piNativeDebugEngineLaunch != NULL)
    {
        m_piNativeDebugEngineLaunch->Release();
        m_piNativeDebugEngineLaunch = NULL;
    }

    // Clean up the program provider:
    GT_IF_WITH_ASSERT(m_piNativeProgramProvider != NULL)
    {
        m_piNativeProgramProvider->Release();
        m_piNativeProgramProvider = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsInstance
// Description: Returns the type-specific instance of this class
// Author:      Uri Shomroni
// Date:        8/1/2012
// ---------------------------------------------------------------------------
vsdProcessDebugger& vsdProcessDebugger::vsInstance()
{
    // TO_DO: verify that no other process debugger is currently registered:
    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

    return (vsdProcessDebugger&)theProcessDebugger;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::setDebugPort
// Description: Resets all the fields, readying for a new debug session.
// Author:      Uri Shomroni
// Date:        10/1/2016
// ---------------------------------------------------------------------------
void vsdProcessDebugger::initialize()
{
    delete m_pProcessCreationData;
    m_pProcessCreationData = nullptr;
    m_debuggedProcessExists = false;
    m_isDebuggedProcessSuspended = false;
    m_isDebuggedProcessAtFatalException = false;
    m_processId = 0;

    if (nullptr != m_hProcess)
    {
        BOOL rcCls = ::CloseHandle(m_hProcess);
        GT_ASSERT(TRUE == rcCls);
        m_hProcess = nullptr;
    }

    m_mainThreadId = OS_NO_THREAD_ID;
    m_spiesAPIThreadId = OS_NO_THREAD_ID;
    m_gotIs64Bit = false;
    m_is64Bit = false;
    m_suspendThreadOnEntryPoint = false;
    m_isKernelDebuggingAboutToStart = false;
    m_isDuringKernelDebugging = false;
    m_isKernelDebuggingJustFinished = false;
    m_bpThreadId = OS_NO_THREAD_ID;
    m_isAPIBreakpoint = false;
    m_hostBreakReason = AP_FOREIGN_BREAK_HIT;
    m_currentBreakpointRequestedFile.clear();
    m_currentBreakpointRequestedLine = -1;
    m_lastStepKind = AP_FOREIGN_BREAK_HIT;

    // m_isWaitingForProcessSuspension and m_piEventToContinue should always be already cleared,
    // So assert their values (and reset them to be on the safe side.
    GT_ASSERT(!m_isWaitingForProcessSuspension);
    m_isWaitingForProcessSuspension = false;

    m_waitingForExecutedFunction = false;

    if (nullptr != m_piEventToContinue)
    {
        GT_ASSERT(false);
        m_piEventToContinue->Release();
        m_piEventToContinue = nullptr;
    }

    // m_piNativeDebugEngine, m_piNativeDebugEngineLaunch, m_piNativeProgramProvider
    // are cleared only in the destructor.

    if (m_piDebugPort != NULL)
    {
        m_piDebugPort->Release();
        m_piDebugPort = NULL;
    }

    if (m_piDebugDefaultPort != NULL)
    {
        m_piDebugDefaultPort->Release();
        m_piDebugDefaultPort = NULL;
    }

    if (m_piProcess != NULL)
    {
        m_piProcess->Release();
        m_piProcess = NULL;
    }

    vsdPackageConnector::instance().setDebuggedProcess(nullptr);

    if (m_piAttachedProgram != NULL)
    {
        m_piAttachedProgram->Release();
        m_piAttachedProgram = NULL;
    }

    if (m_piProgram != NULL)
    {
        m_piProgram->Release();
        m_piProgram = NULL;
    }

    m_debuggedProcessThreads.deleteElementsAndClear();

    if (nullptr != m_debuggedProcessSuspensionThread1)
    {
        delete m_debuggedProcessSuspensionThread1;
        m_debuggedProcessSuspensionThread1 = nullptr;
    }
    if (nullptr != m_debuggedProcessSuspensionThread2)
    {
        delete m_debuggedProcessSuspensionThread2;
        m_debuggedProcessSuspensionThread2 = nullptr;
    }

    // The process termination event often comes before the deletion of breakpoints by th VS SDM.
    // As such, we will not clear the breakpoints here, but rather when initializing for the next run.
    // m_hostBreakpoints.deleteElementsAndClear();

    if (m_pDebugEventCallback != NULL)
    {
        m_pDebugEventCallback->Release();
        m_pDebugEventCallback = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::setDebugPort
// Description: Sets the debug port, which allows process manipulation
// Author:      Uri Shomroni
// Date:        27/12/2011
// ---------------------------------------------------------------------------
void vsdProcessDebugger::setDebugPort(IDebugPort2* piDebugPort)
{
    // Clear out the previous port:
    if (m_piDebugPort != NULL)
    {
        m_piDebugPort->Release();
        m_piDebugPort = NULL;
    }

    if (m_piDebugDefaultPort != NULL)
    {
        m_piDebugDefaultPort->Release();
        m_piDebugDefaultPort = NULL;
    }

    m_piDebugPort = new vsdCDebugPort(*piDebugPort);
    // m_piDebugPort = piDebugPort;
    // m_piDebugPort->AddRef();

    HRESULT hr = m_piDebugPort->QueryInterface(IID_IDebugDefaultPort2, (void**)(&m_piDebugDefaultPort));

    if (!SUCCEEDED(hr))
    {
        m_piDebugDefaultPort = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::addDebugThread
// Description: Adds a thread to be monitored by the process debugger
// Author:      Uri Shomroni
// Date:        8/1/2012
// ---------------------------------------------------------------------------
void vsdProcessDebugger::addDebugThread(IDebugThread2* piDebugThread)
{
    GT_IF_WITH_ASSERT(piDebugThread != NULL)
    {
        // Create the thread object:
        vsdCDebugThread* pNewThread = new vsdCDebugThread(*piDebugThread);

        // Make sure it is not yet represented:
        osThreadId newThreadId = pNewThread->threadId();

        if ((0 == m_debuggedProcessThreads.size()) && (OS_NO_THREAD_ID == m_mainThreadId))
        {
            m_mainThreadId = newThreadId;
        }

        if (getThreadFromId(newThreadId) == NULL)
        {
            // If we are waiting for suspension, this is almost surely the thread created by ::DebugBreakProcess
            if (m_isWaitingForProcessSuspension)
            {
                // Internal suspension should only happen during external suspension:
                GT_IF_WITH_ASSERT(m_isDebuggedProcessSuspended)
                {
                    // We should not see two threads in this case:
                    GT_IF_WITH_ASSERT((nullptr == m_debuggedProcessSuspensionThread1) || (nullptr == m_debuggedProcessSuspensionThread2))
                    {
                        vsdCDebugThread*& pFirstAvailableSlot = (nullptr == m_debuggedProcessSuspensionThread1) ? m_debuggedProcessSuspensionThread1 : m_debuggedProcessSuspensionThread2;

                        // Save the thread separately:
                        pNewThread->setCanSuspendThread(false);
                        pFirstAvailableSlot = pNewThread;

                        // Mark that we have handled the thread:
                        pNewThread = nullptr;
                    }
                }
            }

            if (nullptr != pNewThread)
            {
                osCriticalSectionLocker threadDataCSLocker(m_threadDataCS);
                m_debuggedProcessThreads.push_back(pNewThread);

                // If a new thread was encountered while we are "suspended", suspend it as well:
                if (m_isDebuggedProcessSuspended)
                {
                    pNewThread->suspendThread();
                }

                // Disallow the spies thread from being suspended:
                if (m_spiesAPIThreadId == newThreadId)
                {
                    pNewThread->setCanSuspendThread(false);
                }
            }
        }
        else // getThreadFromId(newThreadId) != NULL
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Duplicate thread object detected, thread id: %#x", newThreadId);
            delete pNewThread;
        }

        if (m_gotIs64Bit && m_is64Bit)
        {
            // Also resume the thread:
            ULONG ignored = 0;
            HRESULT hr = piDebugThread->Resume(&ignored);
            GT_ASSERT(SUCCEEDED(hr));
        }
    }
}
bool vsdProcessDebugger::getDebugThreadStartAddress(osThreadId threadId, osInstructionPointer& threadStartAddr) const
{
    bool retVal = false;

    vsdCDebugThread* pThread = getThreadFromId(threadId);
    GT_IF_WITH_ASSERT(nullptr != pThread)
    {
        threadStartAddr = pThread->threadStartAddress();

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::removeDebugThread
// Description: Removes a thread that is monitored by the process debugger
// Author:      Uri Shomroni
// Date:        20/1/2016
// ---------------------------------------------------------------------------
void vsdProcessDebugger::removeDebugThread(IDebugThread2* piDebugThread)
{
    GT_IF_WITH_ASSERT(piDebugThread != NULL)
    {
        // Get the thread Id:
        DWORD dwTID = 0;
        HRESULT hr = piDebugThread->GetThreadId(&dwTID);
        GT_IF_WITH_ASSERT(SUCCEEDED(hr) && (0 != dwTID))
        {
            osThreadId threadId = (osThreadId)dwTID;
            bool found = false;

            // Check if this is the suspension thread:
            if (nullptr != m_debuggedProcessSuspensionThread1)
            {
                if (m_debuggedProcessSuspensionThread1->threadId() == threadId)
                {
                    delete m_debuggedProcessSuspensionThread1;
                    m_debuggedProcessSuspensionThread1 = nullptr;
                    found = true;
                }
            }
            if (nullptr != m_debuggedProcessSuspensionThread2)
            {
                if (m_debuggedProcessSuspensionThread2->threadId() == threadId)
                {
                    delete m_debuggedProcessSuspensionThread2;
                    m_debuggedProcessSuspensionThread2 = nullptr;
                    found = true;
                }
            }

            if (!found)
            {
                // Iterate the threads vector:
                osCriticalSectionLocker threadDataCSLocker((osCriticalSection&)m_threadDataCS);
                int numberOfThreads = (int)m_debuggedProcessThreads.size();

                for (int i = 0; i < numberOfThreads; i++)
                {
                    if (!found)
                    {
                        vsdCDebugThread* pCurrentThread = (vsdCDebugThread*)m_debuggedProcessThreads[i];
                        GT_IF_WITH_ASSERT(pCurrentThread != NULL)
                        {
                            if (pCurrentThread->threadId() == threadId)
                            {
                                // Delete the thread object:
                                delete pCurrentThread;
                                m_debuggedProcessThreads[i] = nullptr;
                                found = true;
                            }
                        }
                    }
                    else
                    {
                        // This cannot happen on the first iteration:
                        m_debuggedProcessThreads[i - 1] = m_debuggedProcessThreads[i];
                    }
                }

                GT_IF_WITH_ASSERT(found)
                {
                    // Remove the last vector item, which is either nullptr or a duplicate pointer:
                    m_debuggedProcessThreads.pop_back();
                }

                if (threadId == m_mainThreadId)
                {
                    m_mainThreadId = OS_NO_THREAD_ID;

                    if (0 < m_debuggedProcessThreads.size())
                    {
                        vsdCDebugThread* pFirstRemainingThread = m_debuggedProcessThreads[0];
                        GT_IF_WITH_ASSERT(nullptr != pFirstRemainingThread)
                        {
                            m_mainThreadId = pFirstRemainingThread->threadId();
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::setDllDirectory
// Description: Copies the dll directory into oldDir and sets it to newDir. Do
//              not set both references to the same variable
// Author:      Uri Shomroni
// Date:        11/1/2012
// ---------------------------------------------------------------------------
void vsdProcessDebugger::setDllDirectory(const gtString& newDir, gtString& oldDir)
{
    oldDir.makeEmpty();

    // Get the old dir's length:
    DWORD oldDirLen = ::GetDllDirectory(0, NULL);
    GT_IF_WITH_ASSERT(oldDirLen > 0)
    {
        // Get the old dir value:
        LPWSTR oldDirAsCharArray = new WCHAR[oldDirLen + 2];
        DWORD oldDirLen2 = ::GetDllDirectory(oldDirLen + 1, oldDirAsCharArray);
        oldDirAsCharArray[oldDirLen + 1] = '\0';
        oldDir = oldDirAsCharArray;
        GT_ASSERT((oldDirLen2 + 1) == oldDirLen);
    }

    // Set the new dir:
    BOOL rcNew = ::SetDllDirectory(newDir.asCharArray());
    GT_ASSERT(rcNew == TRUE);
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::suspendProcessThreads
// Description: Suspends all process threads except spy and driver threads.
//              If the kernel debugging thread triggered the breakpoint, it is also skipped.
// Author:      Uri Shomroni
// Date:        7/1/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::suspendProcessThreads()
{
    bool retVal = true;

    // If the Spies API thread does not yet exist, there is no need to use the internal resume mechanism:
    if (OS_NO_THREAD_ID != m_spiesAPIThreadId)
    {
        if (!m_isDebuggedProcessAtFatalException)
        {
            osCriticalSectionLocker threadDataCSLocker(m_threadDataCS);

            for (vsdCDebugThread* pThread : m_debuggedProcessThreads)
            {
                // If the suspending thread is a driver thread (this happens during kernel debugging)
                // We want it to be suspendable:
                if (m_isAPIBreakpoint && pThread->isDriverThread())
                {
                    osThreadId currentThreadId = pThread->threadId();

                    if (currentThreadId == m_bpThreadId)
                    {
                        pThread->setCanSuspendThread(true);
                    }
                }

                // Do not suspend driver threads and the spy API thread:
                retVal = pThread->suspendThread() && retVal;
            }
        }
    }

    GT_IF_WITH_ASSERT(retVal)
    {
        m_isDebuggedProcessSuspended = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::internalResumeProcess
// Description: Internally halts the process (without reporting an event)
//              should always match a call to internalResumeProcess
// Author:      Uri Shomroni
// Date:        12/1/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::internalHaltProcess()
{
    bool retVal = false;

    // If the Spies API thread does not yet exist, there is no need to use the internal resume mechanism:
    if (OS_NO_THREAD_ID == m_spiesAPIThreadId)
    {
        retVal = true;
    }
    else
    {
        // Stop the process:
        // Uri, 19/1/2016 - This returns S_FALSE.
        // Attempting to access through m_piProcess or m_piProgram causes our own debug
        // Engine to be called, which is a feedback loop.
        // Instead, we will break the process manually.
        // HRESULT hr = S_OK;
        // hr = m_piNativeDebugEngine->CauseBreak();

        osProcessHandle hProcess = debuggedProcessHandle();
        GT_IF_WITH_ASSERT(nullptr != hProcess)
        {
            // Trigger the break:
            m_isWaitingForProcessSuspension = true;
            BOOL rcBreak = ::DebugBreakProcess(hProcess);

            if (TRUE == rcBreak)
            {
                // Wait for the suspension to occur:
                bool rcWait = osWaitForFlagToTurnOff(m_isWaitingForProcessSuspension, ULONG_MAX);
                GT_ASSERT(rcWait);
                retVal = true;
            }
            else
            {
                // Clear the flag:
                m_isWaitingForProcessSuspension = false;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::internalResumeProcess
// Description: Internally resumes the process (without reporting an event)
//              should always match a call to internalHaltProcess
// Author:      Uri Shomroni
// Date:        12/1/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::internalResumeProcess()
{
    bool retVal = false;

    // We come here in two situations:
    // 1. API thread exists, and internalHaltProcess was called just before to match.
    // 2. API thread does not exist, and we are not using the internal resume mechanism
    //    (in which case this will resume the step / breakpoint event).
    GT_IF_WITH_ASSERT(nullptr != m_piEventToContinue)
    {
        if (!m_skipContinueEvent)
        {
            continueFromSynchronousEvent(m_piEventToContinue);
        }

        m_piEventToContinue->Release();
        m_piEventToContinue = nullptr;

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::debuggedProcessHandle
// Description: Returns a debugger-level access handle of the debugged process
// Author:      Uri Shomroni
// Date:        20/1/2016
// ---------------------------------------------------------------------------
osProcessHandle vsdProcessDebugger::debuggedProcessHandle()
{
    // Check the cached handle:
    if (nullptr == m_hProcess)
    {
        // Get the process id:
        osProcessId procId = debuggedProcessId();

        if (0 != procId)
        {
            // Get a process Handle:
            HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

            if (nullptr != hProcess)
            {
                m_hProcess = hProcess;
            }
        }
    }

    return m_hProcess;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::handleProgramCreated
// Description: Called when a program is created
// Author:      Uri Shomroni
// Date:        1/2/2016
// ---------------------------------------------------------------------------
void vsdProcessDebugger::handleProgramCreated(IDebugProgram2* piProgram)
{
    GT_IF_WITH_ASSERT(nullptr != piProgram)
    {
        if (nullptr == m_piProgram)
        {
            m_piProgram = new vsdCDebugProgram(*piProgram);
        }

        // Make sure we are not debugging two program objects:
        GT_ASSERT(((vsdCDebugProgram*)m_piProgram)->isUnderlyingProgram(piProgram));
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::handleProgramDestroyed
// Description: Called when a program is destroyed
// Author:      Uri Shomroni
// Date:        1/2/2016
// ---------------------------------------------------------------------------
void vsdProcessDebugger::handleProgramDestroyed(IDebugProgram2* piProgram)
{
    GT_IF_WITH_ASSERT(nullptr != piProgram)
    {
        if (nullptr != m_piAttachedProgram)
        {
            if (((vsdCDebugProgram*)m_piAttachedProgram)->isUnderlyingProgram(piProgram))
            {
                m_piAttachedProgram->Release();
                m_piAttachedProgram = nullptr;
            }
        }

        if (nullptr != m_piProgram)
        {
            if (((vsdCDebugProgram*)m_piProgram)->isUnderlyingProgram(piProgram))
            {
                m_piProgram->Release();
                m_piProgram = nullptr;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::handleEntryPointEvent
// Description: Called when the entry point event is received
// Author:      Uri Shomroni
// Date:        7/1/2016
// ---------------------------------------------------------------------------
void vsdProcessDebugger::handleEntryPointEvent(IDebugThread2* piThread)
{
    // Get the thread Id (should be the main thread):
    GT_IF_WITH_ASSERT(nullptr != piThread)
    {
        DWORD dwTID = 0;
        HRESULT hr = piThread->GetThreadId(&dwTID);
        GT_IF_WITH_ASSERT(SUCCEEDED(hr) && (0 != dwTID))
        {
            osThreadId entryPointTID = (osThreadId)dwTID;

            if (!m_suspendThreadOnEntryPoint)
            {
                // Launching the process for debug suspends the main thread for us.
                vsdCDebugThread* pThread = getThreadFromId(entryPointTID);
                GT_IF_WITH_ASSERT(nullptr != pThread)
                {
                    pThread->markThreadAsSuspendedProcessEntryPoint();
                }
            }
            else // m_suspendThreadOnEntryPoint
            {
                // Suspend the main thread until the gaPersistentDataManager resumes it:
                bool rcSusp = suspendDebuggedProcessThread(entryPointTID);
                GT_ASSERT(rcSusp);
            }

            // Also note that it's the main thread:
            GT_ASSERT((OS_NO_THREAD_ID == m_mainThreadId) || (entryPointTID == m_mainThreadId));
            m_mainThreadId = entryPointTID;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::handleExceptionEvent
// Description: Called when an exception is received in the debug event callback
// Author:      Uri Shomroni
// Date:        26/1/2012
// ---------------------------------------------------------------------------
void vsdProcessDebugger::handleExceptionEvent(IDebugEvent2* piEvent, IDebugExceptionEvent2* piExceptionEvent, IDebugThread2* piThread, bool& continueException)
{
    continueException = true;

    // Get the exception data:
    EXCEPTION_INFO exceptionInfo = {0};
    HRESULT hr = piExceptionEvent->GetException(&exceptionInfo);

    if (SUCCEEDED(hr))
    {
        bool isSecondChanceException = (exceptionInfo.dwState & EXCEPTION_STOP_SECOND_CHANCE) != 0;
        DWORD exceptionCode = exceptionInfo.dwCode;
        bool isBreakpoint = (EXCEPTION_BREAKPOINT == exceptionCode);

        osThreadId threadId = OS_NO_THREAD_ID;
        osInstructionPointer exceptionAddress = (osInstructionPointer)0;
        bool isSpyException = false;

        GT_IF_WITH_ASSERT(nullptr != piThread)
        {
            // Get the thread Id:
            DWORD dwTID = 0;
            hr = piThread->GetThreadId(&dwTID);
            GT_IF_WITH_ASSERT(SUCCEEDED(hr))
            {
                threadId = (osThreadId)dwTID;
            }

            // Get the exception address:
            FRAMEINFO_FLAGS frameInfoFlags = FIF_DEBUG_MODULEP | FIF_STACKRANGE;
            IEnumDebugFrameInfo2* piFramesInfo = nullptr;
            hr = piThread->EnumFrameInfo(frameInfoFlags, 10, &piFramesInfo);

            if (SUCCEEDED(hr) && (nullptr != piFramesInfo))
            {
                // Find the first non-spy frame:
                bool hideSpyFrames = true;
                FRAMEINFO frameInfo = { 0 };
                ULONG readFrames = 0;
                hr = piFramesInfo->Next(1, &frameInfo, &readFrames);

                while (SUCCEEDED(hr) && (S_FALSE != hr) && (0 < readFrames))
                {
                    osInstructionPointer frameAddress = (osInstructionPointer)frameInfo.m_addrMin;
                    bool isSpyModule = false;

                    // Query and release the current frame's info:
                    if (nullptr != frameInfo.m_pModule)
                    {
                        MODULE_INFO bpModuleInfo = { 0 };
                        hr = frameInfo.m_pModule->GetInfo(MIF_URL, &bpModuleInfo);

                        if (SUCCEEDED(hr) && (nullptr != bpModuleInfo.m_bstrUrl))
                        {
                            gtString moduleUrl = bpModuleInfo.m_bstrUrl;
                            isSpyException = isSpyException || IsSpyModulePath(moduleUrl);

                            // Release the string:
                            SysFreeString(bpModuleInfo.m_bstrUrl);
                            bpModuleInfo.m_bstrUrl = nullptr;
                        }

#ifdef VSD_RELEASE_ENUMERATED_INTERFACES
                        frameInfo.m_pModule->Release();
                        frameInfo.m_pModule = nullptr;
#endif
                    }

                    if (((!isSpyModule) || (!hideSpyFrames)) && ((osInstructionPointer)0 == exceptionAddress))
                    {
                        exceptionAddress = frameAddress;
                        // Don't break, since we need to free the module data for the other frames.
                    }

                    // Get the next frame
                    hr = piFramesInfo->Next(1, &frameInfo, &readFrames);
                }

                piFramesInfo->Release();
            }
        }

        // Uri, 6/1/16 - for some reason, we do not get first-chance notification for breakpoints...
        // This might be something we need to configure in the native debug engine.
        if (isBreakpoint)
        {
            if (m_isWaitingForProcessSuspension)
            {
                // Do not resume the process:
                continueException = false;

                // Release the calling thread:
                m_isWaitingForProcessSuspension = false;
            }
            else if (m_waitingForExecutedFunction)
            {
                // The function ended successfully, release the thread:
                continueException = false;
                m_waitingForExecutedFunction = false;
            }
            else if (m_isKernelDebuggingAboutToStart || m_isKernelDebuggingJustFinished)
            {
                GT_ASSERT(m_isKernelDebuggingAboutToStart != m_isKernelDebuggingJustFinished);

                // Ignore the internal breakpoints:
                m_isKernelDebuggingAboutToStart = false;
                m_isKernelDebuggingJustFinished = false;
            }
            else // !m_isWaitingForProcessSuspension &&
            {
                // TO_DO: handle kernel debugging entry and exit BPs
                m_bpThreadId = threadId;
                m_isAPIBreakpoint = isSpyException;

                // If this is a break triggered by the process debugger:
                if (AP_BREAK_COMMAND_HIT == m_hostBreakReason)
                {
                    // The breakpoint comes from a thread that CauseProcessBreak() creates.
                    // Instead, select the main thread:
                    osThreadId mainTid = mainThreadId();

                    if (OS_NO_THREAD_ID != mainTid)
                    {
                        threadId = mainTid;
                    }
                }
                else
                {
                    // Reset the break reason so that the spy is queried:
                    m_hostBreakReason = AP_FOREIGN_BREAK_HIT;
                }

                bool rcSusp = suspendProcessThreads();
                GT_IF_WITH_ASSERT(rcSusp)
                {
                    // Notify observers about the breakpoint event:
                    apBreakpointHitEvent event(threadId, exceptionAddress);
                    apEventsHandler::instance().registerPendingDebugEvent(event);

                    // Notify observers that the debugged process run was suspended:
                    apDebuggedProcessRunSuspendedEvent processSuspendedEvent(threadId);
                    apEventsHandler::instance().registerPendingDebugEvent(processSuspendedEvent);
                }
            }

            // Don't pass it to the debugged application:
            hr = piExceptionEvent->PassToDebuggee(FALSE);
            GT_ASSERT(SUCCEEDED(hr));
        }
        else if (isSecondChanceException)
        {
            // Mark that we are during a fatal exception
            m_isDebuggedProcessAtFatalException = true;
            bool rcSusp = suspendProcessThreads();
            GT_ASSERT(rcSusp);

            // Translate win32 exception code to ExceptionReason:
            osExceptionReason exceptionReason = osExceptionCodeToExceptionReason(exceptionCode);

            // Notify observers about the second chance exception event:
            apExceptionEvent exceptionEvent(threadId, exceptionReason, exceptionAddress, true);
            apEventsHandler::instance().registerPendingDebugEvent(exceptionEvent);

            // Notify observers that the debugged process run was suspended:
            apDebuggedProcessRunSuspendedEvent processSuspendedEvent(threadId);
            apEventsHandler::instance().registerPendingDebugEvent(processSuspendedEvent);

            // Continuing the exception would cause the process to die:
            continueException = false;
        }


        // Release the struct's members:
        if (exceptionInfo.pProgram != NULL)
        {
            exceptionInfo.pProgram->Release();
        }

        SysFreeString(exceptionInfo.bstrExceptionName);
        SysFreeString(exceptionInfo.bstrProgramName);
    }

    // If we do not continue the event, save it to be resolved later:
    if (!continueException)
    {
        GT_ASSERT(nullptr == m_piEventToContinue);

        if (nullptr != m_piEventToContinue)
        {
            m_piEventToContinue->Release();
        }

        m_skipContinueEvent = false;
        m_piEventToContinue = piEvent;
        GT_IF_WITH_ASSERT(nullptr != m_piEventToContinue)
        {
            m_piEventToContinue->AddRef();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::handleBreakpointEvent
// Description: Called when an breakpoint is received in the debug event callback
// Author:      Uri Shomroni
// Date:        12/1/2016
// ---------------------------------------------------------------------------
void vsdProcessDebugger::handleBreakpointEvent(IDebugEvent2* piEvent, IDebugBreakpointEvent2* piBreakpointEvent, IDebugThread2* piThread, bool& continueBreakpoint)
{
    continueBreakpoint = true;

    osThreadId threadId = OS_NO_THREAD_ID;
    osInstructionPointer bpAddress = (osInstructionPointer)0;

    GT_IF_WITH_ASSERT(nullptr != piThread)
    {
        // Get the thread Id:
        DWORD dwTID = 0;
        HRESULT hr = piThread->GetThreadId(&dwTID);
        GT_IF_WITH_ASSERT(SUCCEEDED(hr))
        {
            threadId = (osThreadId)dwTID;
        }

        // Get the exception address:
        FRAMEINFO_FLAGS frameInfoFlags = FIF_DEBUG_MODULEP | FIF_STACKRANGE;
        IEnumDebugFrameInfo2* piFramesInfo = nullptr;
        hr = piThread->EnumFrameInfo(frameInfoFlags, 10, &piFramesInfo);

        if (SUCCEEDED(hr) && (nullptr != piFramesInfo))
        {
            // Find the first non-spy frame:
            bool hideSpyFrames = true;
            FRAMEINFO frameInfo = { 0 };
            ULONG readFrames = 0;
            hr = piFramesInfo->Next(1, &frameInfo, &readFrames);

            while (SUCCEEDED(hr) && (S_FALSE != hr) && (0 < readFrames))
            {
                osInstructionPointer frameAddress = (osInstructionPointer)frameInfo.m_addrMin;
                bool isSpyModule = false;

                // Query and release the current frame's info:
                if (nullptr != frameInfo.m_pModule)
                {
#ifdef VSD_RELEASE_ENUMERATED_INTERFACES
                    frameInfo.m_pModule->Release();
                    frameInfo.m_pModule = nullptr;
#endif
                }

                if (((!isSpyModule) || (!hideSpyFrames)) && ((osInstructionPointer)0 == bpAddress))
                {
                    bpAddress = frameAddress;
                    // Don't break, since we need to free the module data for the other frames.
                }

                // Get the next frame
                hr = piFramesInfo->Next(1, &frameInfo, &readFrames);
            }

            piFramesInfo->Release();
        }
    }

    m_bpThreadId = threadId;
    m_isAPIBreakpoint = false;
    m_hostBreakReason = AP_HOST_BREAKPOINT_HIT;

    GT_IF_WITH_ASSERT(nullptr != piBreakpointEvent)
    {
        IEnumDebugBoundBreakpoints2* piHitBreakpoints = nullptr;
        HRESULT hr = piBreakpointEvent->EnumBreakpoints(&piHitBreakpoints);

        if (SUCCEEDED(hr) && (nullptr != piHitBreakpoints))
        {
            // Get the first bound breakpoint:
            ULONG gotBP = 0;
            IDebugBoundBreakpoint2* piHitBreakpoint = nullptr;
            hr = piHitBreakpoints->Next(1, &piHitBreakpoint, &gotBP);

            if (SUCCEEDED(hr) && (S_FALSE != hr) && (nullptr != piHitBreakpoint) && (1 == gotBP))
            {
                IDebugPendingBreakpoint2* piPendingBreakpoint = nullptr;
                hr = piHitBreakpoint->GetPendingBreakpoint(&piPendingBreakpoint);

                if (SUCCEEDED(hr) && (nullptr != piPendingBreakpoint))
                {
                    IDebugBreakpointRequest2* piBreakpointRequest = nullptr;
                    hr = piPendingBreakpoint->GetBreakpointRequest(&piBreakpointRequest);

                    if (SUCCEEDED(hr) && (nullptr != piBreakpointRequest))
                    {
                        BP_LOCATION_TYPE bpLocType = 0;
                        hr = piBreakpointRequest->GetLocationType(&bpLocType);
                        GT_ASSERT(SUCCEEDED(hr) && (BPLT_CODE_FILE_LINE == bpLocType));

                        BP_REQUEST_INFO bpReqInfo = { 0 };
                        BPREQI_FIELDS bpReqFields = BPREQI_BPLOCATION;
                        hr = piBreakpointRequest->GetRequestInfo(bpReqFields, &bpReqInfo);

                        if (SUCCEEDED(hr))
                        {
                            GT_ASSERT(bpReqInfo.bpLocation.bpLocationType == bpLocType);
                            IDebugDocumentPosition2* piDocumentPosition = bpReqInfo.bpLocation.bpLocation.bplocCodeFileLine.pDocPos;

                            if (piDocumentPosition != NULL)
                            {
                                // Get the file name:
                                BSTR fileNameAsBSTR = NULL;
                                hr = piDocumentPosition->GetFileName(&fileNameAsBSTR);

                                if (SUCCEEDED(hr) && fileNameAsBSTR != NULL)
                                {
                                    // Get the source file:
                                    m_currentBreakpointRequestedFile.setFullPathFromString(fileNameAsBSTR);
                                    SysFreeString(fileNameAsBSTR);
                                    fileNameAsBSTR = nullptr;
                                }

                                // Get the range in the code where the breakpoint is:
                                TEXT_POSITION startPos = { 0 };
                                TEXT_POSITION endPos = { 0 };
                                hr = piDocumentPosition->GetRange(&startPos, &endPos);

                                if (SUCCEEDED(hr))
                                {
                                    // TO_DO: for some reason, breakpoint requests (clicking the margin or pressing F9) give a
                                    // 10-line range which starts at the line before the breakpoint. So get the actual line #:
                                    m_currentBreakpointRequestedLine = (int)(startPos.dwLine) + 1;
                                }

                                piDocumentPosition->Release();
                                bpReqInfo.bpLocation.bpLocation.bplocCodeFileLine.pDocPos = nullptr;
                            }
                        }

                        piBreakpointRequest->Release();
                    }

                    piPendingBreakpoint->Release();
                }

                piHitBreakpoint->Release();
            }

            piHitBreakpoints->Release();
        }
    }

    bool rcSusp = suspendProcessThreads();
    GT_IF_WITH_ASSERT(rcSusp)
    {
        // Notify observers about the breakpoint event:
        apBreakpointHitEvent event(threadId, bpAddress);
        apEventsHandler::instance().registerPendingDebugEvent(event);

        // Notify observers that the debugged process run was suspended:
        apDebuggedProcessRunSuspendedEvent processSuspendedEvent(threadId);
        apEventsHandler::instance().registerPendingDebugEvent(processSuspendedEvent);
    }

    // If the Spies API thread does not yet exist, there is no need to use the internal resume mechanism:
    if (OS_NO_THREAD_ID == m_spiesAPIThreadId)
    {
        continueBreakpoint = false;
    }

    // If we do not continue the event, save it to be resolved later:
    if (!continueBreakpoint)
    {
        GT_ASSERT(nullptr == m_piEventToContinue);

        if (nullptr != m_piEventToContinue)
        {
            m_piEventToContinue->Release();
        }

        m_skipContinueEvent = false;
        m_piEventToContinue = piEvent;
        GT_IF_WITH_ASSERT(nullptr != m_piEventToContinue)
        {
            m_piEventToContinue->AddRef();
        }
    }
}

// Helper functions used to transfer IDebugThread2* objects through our event system:
void vsdDebugThreadReleaser(void* pData)
{
    if (nullptr != pData)
    {
        IDebugThread2* piThread = (IDebugThread2*)pData;
        piThread->Release();
    }
}

void* vsdDebugThreadCloner(const void* pData)
{
    void* retVal = nullptr;

    if (nullptr != pData)
    {
        IDebugThread2* piThread = (IDebugThread2*)pData;
        piThread->AddRef();
        retVal = (void*)piThread;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::handleStepCompleteEvent
// Description: Called when an step complete event is received in the debug event callback
// Author:      Uri Shomroni
// Date:        2/2/2016
// ---------------------------------------------------------------------------
void vsdProcessDebugger::handleStepCompleteEvent(IDebugEvent2* piEvent, IDebugThread2* piThread, bool& continueStep)
{
    // This event will sometimes arrive while the step command issuing is still being processed.
    // Wait for that operation to complete before handling the event:
    osWaitForFlagToTurnOff(m_issuingStepCommand, ULONG_MAX);

    continueStep = true;

    bool ignoreStep = false;

    osThreadId threadId = OS_NO_THREAD_ID;
    osInstructionPointer bpAddress = (osInstructionPointer)0;

    GT_IF_WITH_ASSERT(nullptr != piThread)
    {
        // Get the thread Id:
        DWORD dwTID = 0;
        HRESULT hr = piThread->GetThreadId(&dwTID);
        GT_IF_WITH_ASSERT(SUCCEEDED(hr))
        {
            threadId = (osThreadId)dwTID;
        }

        // Get the exception address:
        FRAMEINFO_FLAGS frameInfoFlags = FIF_DEBUG_MODULEP | FIF_STACKRANGE;
        IEnumDebugFrameInfo2* piFramesInfo = nullptr;
        hr = piThread->EnumFrameInfo(frameInfoFlags, 10, &piFramesInfo);

        if (SUCCEEDED(hr) && (nullptr != piFramesInfo))
        {
            // Find the first non-spy frame:
            bool hideSpyFrames = true;
            FRAMEINFO frameInfo = { 0 };
            ULONG readFrames = 0;
            hr = piFramesInfo->Next(1, &frameInfo, &readFrames);

            // Get the absolute path to the spies directory:
            osFilePath resolvedSpiesPath;
            GT_IF_WITH_ASSERT(nullptr != m_pProcessCreationData)
            {
                resolvedSpiesPath = m_pProcessCreationData->spiesDirectory().asString();
                resolvedSpiesPath.resolveToAbsolutePath().reinterpretAsDirectory();
            }

            while (SUCCEEDED(hr) && (S_FALSE != hr) && (0 < readFrames))
            {
                osInstructionPointer frameAddress = (osInstructionPointer)frameInfo.m_addrMin;
                bool isSpyModule = false;

                // Query and release the current frame's info:
                if (nullptr != frameInfo.m_pModule)
                {
                    MODULE_INFO moduleInfo = { 0 };
                    hr = frameInfo.m_pModule->GetInfo(MIF_URL, &moduleInfo);

                    if (SUCCEEDED(hr))
                    {
                        osFilePath modulePath = moduleInfo.m_bstrUrl;
                        modulePath.clearFileName().clearFileExtension().resolveToAbsolutePath();

                        if (modulePath == resolvedSpiesPath)
                        {
                            isSpyModule = true;
                        }
                    }

#ifdef VSD_RELEASE_ENUMERATED_INTERFACES
                    frameInfo.m_pModule->Release();
                    frameInfo.m_pModule = nullptr;
#endif
                }

                if (isSpyModule)
                {
                    if (hideSpyFrames)
                    {
                        bpAddress = (osInstructionPointer)0;
                    }

                    // Continue stepping until we exit the spy:
                    ignoreStep = true;
                }

                if (((!isSpyModule) || (!hideSpyFrames)) && ((osInstructionPointer)0 == bpAddress))
                {
                    bpAddress = frameAddress;
                    // Don't break, since we need to free the module data for the other frames.
                }

                // Get the next frame
                hr = piFramesInfo->Next(1, &frameInfo, &readFrames);
            }

            piFramesInfo->Release();
        }
    }

    // If we want to ignore the step:
    if (ignoreStep)
    {
        // Wait for the main thread to execute the previous step command:
        osWaitForFlagToTurnOff(m_waitingForDeferredStep, ULONG_MAX);

        // Continue the debug event:
        continueStep = false;

        // Set up the deferred step state:
        GT_ASSERT(!m_waitingForDeferredStep);
        m_waitingForDeferredStep = true;

        // Send a deferred command, so that the main thread continues stepping after this is done.
        // Calling IDebugProgram2::Step during IDebugEventCallback2::Event causes a deadlock.
        apDeferredCommandEvent deferredStepEve(apDeferredCommandEvent::AP_DEFERRED_COMMAND_INTERNAL_HOST_STEP, apDeferredCommandEvent::AP_VSD_PROCESS_DEBUGGER);
        deferredStepEve.setData((const void*)piThread, vsdDebugThreadReleaser, vsdDebugThreadCloner);
        apEventsHandler::instance().registerPendingDebugEvent(deferredStepEve);
    }

    if (!ignoreStep)
    {
        // Send a step complete event to the client:
        reportHostStep(threadId, bpAddress);

        // If the Spies API thread does not yet exist, there is no need to use the internal resume mechanism:
        if (OS_NO_THREAD_ID == m_spiesAPIThreadId)
        {
            continueStep = false;
        }
    }

    // If we do not continue the event, save it to be resolved later:
    if (!continueStep)
    {
        GT_ASSERT(nullptr == m_piEventToContinue);

        if (nullptr != m_piEventToContinue)
        {
            m_piEventToContinue->Release();
        }

        m_skipContinueEvent = false;
        m_piEventToContinue = piEvent;
        GT_IF_WITH_ASSERT(nullptr != m_piEventToContinue)
        {
            m_piEventToContinue->AddRef();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::reportHostStep
// Description: Sends all the events and members that a host step ending should
//              communicate outside this class
// Author:      Uri Shomroni
// Date:        10/2/2016
// ---------------------------------------------------------------------------
void vsdProcessDebugger::reportHostStep(osThreadId threadId, osInstructionPointer bpAddress)
{
    m_bpThreadId = threadId;
    m_isAPIBreakpoint = false;
    m_hostBreakReason = m_lastStepKind;
    m_lastStepKind = AP_FOREIGN_BREAK_HIT;

    bool rcSusp = suspendProcessThreads();
    GT_IF_WITH_ASSERT(rcSusp)
    {
        apEventsHandler& theEventsHandler = apEventsHandler::instance();

        // Notify observers about the breakpoint event:
        apBreakpointHitEvent event(threadId, bpAddress);
        theEventsHandler.registerPendingDebugEvent(event);

        // Notify observers that the debugged process run was suspended:
        apDebuggedProcessRunSuspendedEvent processSuspendedEvent(threadId);
        theEventsHandler.registerPendingDebugEvent(processSuspendedEvent);
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::continueFromSynchronousEvent
// Description: Continues the debugged process run after a synchronous event
// Author:      Uri Shomroni
// Date:        9/1/2012
// ---------------------------------------------------------------------------
void vsdProcessDebugger::continueFromSynchronousEvent(IDebugEvent2* piEvent)
{
    HRESULT hr = m_piNativeDebugEngine->ContinueFromSynchronousEvent(piEvent);
    GT_ASSERT(SUCCEEDED(hr));
}
void vsdProcessDebugger::onEvent(const apEvent& eve, bool& vetoEvent)
{
    apEvent::EventType eveType = eve.eventType();

    switch (eveType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            if (m_gotIs64Bit && m_is64Bit)
            {
                HRESULT hr = m_piProgram->Continue(nullptr);
                GT_ASSERT(SUCCEEDED(hr));
            }
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            if (!m_debuggedProcessExists)
            {
                // This should not happen under the VS process debugger, but veto it anyway:
                GT_ASSERT(m_debuggedProcessExists);
                vetoEvent = true;
            }

            initialize();
        }
        break;

        case apEvent::AP_BREAKPOINT_HIT:
        {
            const apBreakpointHitEvent& breakpointEve = (const apBreakpointHitEvent&)eve;
            apBreakReason breakReason = breakpointEve.breakReason();

            if ((AP_BEFORE_KERNEL_DEBUGGING_HIT == breakReason) ||
                (AP_AFTER_KERNEL_DEBUGGING_HIT == breakReason))
            {
                // The internal breakpoints are not used by the VSP process debugger, but should still be hidden from the debugger views:
                GT_ASSERT(false);
                vetoEvent = true;
            }
        }
        break;

        case apEvent::AP_DEFERRED_COMMAND_EVENT:
        {
            apDeferredCommandEvent deferredCommandEvent = (const apDeferredCommandEvent&)eve;
            apDeferredCommandEvent::apDeferredCommand command = deferredCommandEvent.command();
            apDeferredCommandEvent::apDeferredCommandTarget target = deferredCommandEvent.commandTarget();

            if (apDeferredCommandEvent::AP_VSD_PROCESS_DEBUGGER == target)
            {
                switch (command)
                {
                    case apDeferredCommandEvent::AP_DEFERRED_COMMAND_INTERNAL_HOST_STEP:
                    {
                        // Deferred internal step (stepping out of spy code):
                        // Make sure the state is as expected:
                        GT_IF_WITH_ASSERT(m_waitingForDeferredStep)
                        {
                            IDebugThread2* piThread = (IDebugThread2*)deferredCommandEvent.getData();
                            GT_IF_WITH_ASSERT(nullptr != piThread)
                            {
                                // Only ignore it if our step out operation worked
                                GT_IF_WITH_ASSERT(nullptr != m_piProgram)
                                {
                                    // We need to continue stepping out of the spy code:
                                    HRESULT hr = m_piProgram->Step(piThread, STEP_OUT, STEP_LINE);
                                    bool rcStp = (SUCCEEDED(hr) && (S_FALSE != hr));

                                    // If we could not perform the internal step, report the suspension to the client:
                                    if (!rcStp)
                                    {
                                        osThreadId threadId = OS_NO_THREAD_ID;
                                        DWORD dwTid = 0;
                                        hr = piThread->GetThreadId(&dwTid);

                                        if (SUCCEEDED(hr))
                                        {
                                            threadId = (osThreadId)dwTid;
                                        }

                                        reportHostStep(threadId, (osInstructionPointer)0);
                                    }
                                }
                            }

                            // Either way, we are currently waiting on an event, so release it:
                            if (nullptr != m_piEventToContinue)
                            {
                                internalResumeProcess();
                            }

                            // Release the command handler thread:
                            m_waitingForDeferredStep = false;
                        }

                        // Prevent this event from propagating further:
                        vetoEvent = true;
                    }
                    break;

                    default:
                        GT_ASSERT_EX(false, L"vsdProcessDebugger: Invalid deferred command parameter in deferred command event");
                        break;
                }
            }
            else
            {
                // Got an event meant for someone else, note it in the log:
                gtString logMsg;
                logMsg.appendFormattedString(L"vsdProcessDebugger got deferred command %d with target %d", command, target);
                OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
                GT_ASSERT(apDeferredCommandEvent::AP_UNKNOWN_COMMAND_TARGET != target);
            }
        }
        break;

        default:
            break;
    }
}
void vsdProcessDebugger::onEventRegistration(apEvent& eve, bool& vetoEvent)
{
    apEvent::EventType eveType = eve.eventType();

    switch (eveType)
    {
        case apEvent::AP_BEFORE_KERNEL_DEBUGGING_EVENT:
        {
            // Kernel debugging is about to start, mark this:
            m_isKernelDebuggingAboutToStart = true;
            m_isDuringKernelDebugging = true;
        }
        break;

        case apEvent::AP_AFTER_KERNEL_DEBUGGING_EVENT:
        {
            // Kernel debugging just ended, mark this:
            m_isDuringKernelDebugging = false;
            m_isKernelDebuggingJustFinished = true;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_OUTPUT_STRING:
        {
            // Check if this is a spy termination notification:
            const apDebuggedProcessOutputStringEvent& stringEve = (const apDebuggedProcessOutputStringEvent&)eve;
            static const gtString s_processTerminationMessage = OS_STR_debuggedProcessIsTerminating;
            const gtString& debugString = stringEve.debuggedProcessOutputString();

            if (0 == debugString.find(s_processTerminationMessage))
            {
                // A new thread is becoming the API calls handling thread. Get its id:
                static const int threadIsStartPos = s_processTerminationMessage.length();
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

                // This is an internal message, no need to pass it on:
                vetoEvent = true;
            }
        }
        break;

        default:
            break;
    }
}
// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::initializeDebugger
// Description: Sets up the debugger to launch the process
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        25/1/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::initializeDebugger(const apDebugProjectSettings& processCreationData)
{
    GT_ASSERT(nullptr == m_pProcessCreationData);
    delete m_pProcessCreationData;
    m_pProcessCreationData = new apDebugProjectSettings(processCreationData);

    // Clear breakpoints from previous instances:
    m_hostBreakpoints.deleteElementsAndClear();

    m_is64Bit = false;
    m_gotIs64Bit = osIs64BitModule(m_pProcessCreationData->executablePath(), m_is64Bit);
    GT_ASSERT(m_gotIs64Bit);

    return (nullptr != m_pProcessCreationData);
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::launchDebuggedProcessInSuspendedMode
// Description: Creates the debugged process but and starts its run. When
//              this function returns, debuggedProcessId() has a valid value.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        19/9/2010
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::launchDebuggedProcess()
{
    bool retVal = launchDebuggedProcessInSuspendedMode();

    GT_IF_WITH_ASSERT(retVal)
    {
        retVal = continueDebuggedProcessFromSuspendedCreation();
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::doesSupportTwoStepLaunching
// Description: This process debugger implements the launchDebuggedProcessInSuspendedMode
//              and continueDebuggedProcessFromSuspendedCreation functions.
// Author:      Uri Shomroni
// Date:        26/12/2011
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::doesSupportTwoStepLaunching() const
{
    return true;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::launchDebuggedProcessInSuspendedMode
// Description: Creates the debugged process but does not start its run. When
//              this function returns, debuggedProcessId() has a valid value.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        19/9/2010
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::launchDebuggedProcessInSuspendedMode()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr != m_pProcessCreationData)
    {
        // Get the process info:
        const gtString& exePath = m_pProcessCreationData->executablePath().asString();
        const gtString& cmdArgs = m_pProcessCreationData->commandLineArguments();
        const gtString& workDir = m_pProcessCreationData->workDirectory().asString();
        const gtString& spiesDir = m_pProcessCreationData->spiesDirectory().asString();
        gtString environmentAsGTString;
        m_pProcessCreationData->environmentVariablesAsString(environmentAsGTString);

        // The LaunchSuspended method requires an environment block, i.e. a block of null-terminated strings terminated by an empty string.
        // Since our array is delimited by semicolons, we will replace them with null characters in the string we send:
        environmentAsGTString.removeTrailing((wchar_t)'\n').append((wchar_t)'\n').append((wchar_t)'\n');
        BSTR envVars = SysAllocString(environmentAsGTString.asCharArray());
        int nextDelim = environmentAsGTString.find((wchar_t)'\n');

        while (nextDelim > -1)
        {
            envVars[nextDelim] = '\0';
            nextDelim = environmentAsGTString.find((wchar_t)'\n', nextDelim + 1);
        }

        if (m_pDebugEventCallback == NULL)
        {
            m_pDebugEventCallback = new vsdCDebugEventCallback;
        }

        LAUNCH_FLAGS launchFlags = LAUNCH_DEBUG | LAUNCH_MERGE_ENV;

        gtString originalDllPath;
        setDllDirectory(spiesDir, originalDllPath);
        HRESULT hr = m_piNativeDebugEngineLaunch->LaunchSuspended(NULL, m_piDebugPort, exePath.asCharArray(), cmdArgs.asCharArray(), workDir.asCharArray(), envVars, NULL, launchFlags, 0, 0, 0, m_pDebugEventCallback, &m_piProcess);
        gtString ignored;
        setDllDirectory(originalDllPath, ignored);

        // Get the program interface:
        if (SUCCEEDED(hr) && (m_piProcess != NULL))
        {
            // Set the process in the package connector:
            // TO_DO: add a query interface for this:
            vsdPackageConnector::instance().setDebuggedProcess((vsdCDebugProcess*)m_piProcess);

            retVal = true;

            // Uri, 5/1/16 - for some reason the native debug engine LaunchSuspended method returns before the process exists, so we cannot
            // Get the programs from it here and publish them.
            // Need to check if this is crucial or not.
#if 0
            // Get the program:
            IEnumDebugPrograms2* piPrograms = NULL;
            hr = m_piProcess->EnumPrograms(&piPrograms);

            if (SUCCEEDED(hr) && (piPrograms != NULL))
            {
                ULONG numberOfPrograms = 0;
                hr = piPrograms->GetCount(&numberOfPrograms);

                if (SUCCEEDED(hr) && (numberOfPrograms > 0))
                {
                    // Get the first program:
                    ULONG fetchedPrograms = 0;
                    hr = piPrograms->Next(1, &m_piAttachedProgram, &fetchedPrograms);

                    if (SUCCEEDED(hr) && (S_FALSE != hr) && (fetchedPrograms > 0) && (m_piAttachedProgram != NULL))
                    {
                        GT_ASSERT(fetchedPrograms == 1);

                        /*
                        // Publish this program, so we could later get a program node:
                        CComPtr<IDebugProgramPublisher2> ccpProgramPublisher;
                        hr = ccpProgramPublisher.CoCreateInstance(CLSID_ProgramPublisher);
                        if (SUCCEEDED(hr) && (ccpProgramPublisher != NULL))
                        {
                            IUnknown* piProgramAsUnknown = NULL;
                            hr = m_piAttachedProgram->QueryInterface(IID_IUnknown, (void**)(&piProgramAsUnknown));

                            if (SUCCEEDED(hr) && (piProgramAsUnknown != NULL))
                            {
                                CONST_GUID_ARRAY debugEngines = {0};
                                debugEngines.Members = &guidNativeOnlyEng;
                                debugEngines.dwCount = 1;
                                hr = ccpProgramPublisher->PublishProgram(debugEngines, L"Program", piProgramAsUnknown);

                                piProgramAsUnknown->Release();
                            }
                        }
                        */
                        retVal = true;
                    }
                }

                // Release the enum interface:
                piPrograms->Release();
            }

#endif // 0
        }

        // Release COM objects:
        // Restore the delimiters before releasing the string:
        nextDelim = environmentAsGTString.find((wchar_t)0xffff);

        while (nextDelim > -1)
        {
            envVars[nextDelim] = (wchar_t)0xffff;
            nextDelim = environmentAsGTString.find((wchar_t)0xffff, nextDelim + 1);
        }

        SysFreeString(envVars);

        if (retVal)
        {
            m_debuggedProcessExists = true;
        }
        else
        {
            // If we failed, delete the process creation data:
            delete m_pProcessCreationData;
            m_pProcessCreationData = NULL;
        }
    }

    return retVal;
}
bool vsdProcessDebugger::continueDebuggedProcessFromSuspendedCreation()
{
    bool retVal = false;
    HRESULT hr = S_OK;

    // Get the program to be attached:
    if (nullptr == m_piAttachedProgram)
    {
        // Get the programs in the process:
        IEnumDebugPrograms2* piPrograms = NULL;
        hr = ((vsdCDebugProcess*)m_piProcess)->UnderlyingProcessEnumPrograms(&piPrograms);

        if (SUCCEEDED(hr) && (piPrograms != NULL))
        {
            // See there is at least one:
            ULONG numberOfPrograms = 0;
            hr = piPrograms->GetCount(&numberOfPrograms);

            if (SUCCEEDED(hr) && (numberOfPrograms > 0))
            {
                // Get the first program:
                ULONG fetchedPrograms = 0;
                IDebugProgram2* piRealProgram = nullptr;
                hr = piPrograms->Next(1, &piRealProgram, &fetchedPrograms);

                // If successful:
                if (SUCCEEDED(hr) && (S_FALSE != hr) && (fetchedPrograms > 0) && (piRealProgram != NULL))
                {
                    // Wrap the program and save it as a member:
                    GT_ASSERT(fetchedPrograms == 1);

                    m_piAttachedProgram = new vsdCDebugProgram(*piRealProgram);

                    // Release the program interface:
                    piRealProgram->Release();
                }
            }

            // Release the enum interface:
            piPrograms->Release();
        }
    }

    // Create a process ID object:
    AD_PROCESS_ID processID = {0};
    processID.ProcessIdType = AD_PROCESS_ID_SYSTEM;
    processID.ProcessId.dwProcessId = debuggedProcessId();
    // TO_DO: get GUID from engine:
    GUID guidDebugEngine = guidNativeOnlyEng;
    UINT64 programId = 1;

    // Get the program node from the port notifier:
    IDebugProgramNode2* piProgramNode = nullptr;
    GT_IF_WITH_ASSERT(nullptr != m_piDebugPort)
    {
        piProgramNode = ((const vsdCDebugPort*)m_piDebugPort)->RegisteredProgramNode();
    }

    // Attempt to get the program node from the program provider:
    if (nullptr == piProgramNode)
    {
        PROVIDER_FLAGS getNodeFlags = PFLAG_NONE /* | PFLAG_DEBUGGEE | PFLAG_ATTACHED_TO_DEBUGGEE | PFLAG_REASON_WATCH | PFLAG_GET_PROGRAM_NODES | PFLAG_GET_IS_DEBUGGER_PRESENT */;
        hr = m_piNativeProgramProvider->GetProviderProgramNode(getNodeFlags, m_piDebugDefaultPort, processID, guidDebugEngine, programId, &piProgramNode);

        if (!SUCCEEDED(hr) || (NULL == piProgramNode))
        {
            // If that failed, try getting the program node from the process:
            CONST_GUID_ARRAY nativeOnlyEngineArray = { 0 };
            nativeOnlyEngineArray.dwCount = 1;
            nativeOnlyEngineArray.Members = &guidDebugEngine;
            PROVIDER_PROCESS_DATA processData = { 0 };
            PROVIDER_FLAGS getProcessDataFlags = PFLAG_GET_PROGRAM_NODES | PFLAG_DEBUGGEE;
            hr = m_piNativeProgramProvider->GetProviderProcessData(getProcessDataFlags, m_piDebugDefaultPort, processID, nativeOnlyEngineArray, &processData);

            if (SUCCEEDED(hr))
            {
                // Get the first program:
                if (processData.ProgramNodes.dwCount > 0)
                {
                    piProgramNode = processData.ProgramNodes.Members[0];
                }
            }
        }
    }

    // If we got a program node:
    if (SUCCEEDED(hr) && NULL != piProgramNode)
    {
        // Wrap it:
        IDebugProgramNode2* pProgramNode = new vsdCDebugProgramNode(*piProgramNode);
        piProgramNode->Release();

        GT_IF_WITH_ASSERT(nullptr != m_piAttachedProgram)
        {
            // Set exception handling to tell us of first-chance breakpoint exceptions:
            // TO_DO: figure out why this doesn't work. For now, we just take the second-chance exception, which works OK.
            // EXCEPTION_INFO firstChanceBreakpoint = { m_piAttachedProgram, nullptr, nullptr, EXCEPTION_BREAKPOINT, EXCEPTION_STOP_FIRST_CHANCE , guidNativeOnlyEng };
            EXCEPTION_INFO firstChanceBreakpoint = { nullptr, nullptr, nullptr, EXCEPTION_BREAKPOINT, EXCEPTION_STOP_ALL, guidNativeOnlyEng };
            hr = m_piNativeDebugEngine->SetException(&firstChanceBreakpoint);
            GT_ASSERT(SUCCEEDED(hr));

            hr = m_piNativeDebugEngine->Attach(&m_piAttachedProgram, &pProgramNode, 1, m_pDebugEventCallback, ATTACH_REASON_LAUNCH);
            pProgramNode->Release();

            /*
            IDebugProgram2* piProgram = nullptr;
            IEnumDebugPrograms2* piPrograms = nullptr;
            hr = m_piNativeDebugEngine->EnumPrograms(&piPrograms);
            if (SUCCEEDED(hr) && (nullptr != piPrograms))
            {
                ULONG gotProgram = 0;
                hr = piPrograms->Next(1, &piProgram, &gotProgram);
                if (SUCCEEDED(hr) && (S_FALSE != hr) && (nullptr != piProgram))
                {
                    OS_OUTPUT_DEBUG_LOG(L"Got program from debug engine", OS_DEBUG_LOG_DEBUG);
                }

                piPrograms->Release();
            }

            if (nullptr != piProgram)
            {
                handleProgramCreated(piProgram);

                piProgram->Release();
            }
            */

            if (SUCCEEDED(hr))
            {
                hr = m_piNativeDebugEngineLaunch->ResumeProcess(m_piProcess);
                retVal = SUCCEEDED(hr);

                if (!retVal || S_FALSE == hr)
                {
                    GT_IF_WITH_ASSERT((E_NOTIMPL == hr) || (S_FALSE == hr))
                    {
                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}
bool vsdProcessDebugger::debuggedProcessExists() const
{
    return m_debuggedProcessExists;
}
const apDebugProjectSettings* vsdProcessDebugger::debuggedProcessCreationData() const
{
    return m_pProcessCreationData;
}
bool vsdProcessDebugger::terminateDebuggedProcess()
{
    // Call the native debug engine:
    HRESULT hr = m_piNativeDebugEngineLaunch->TerminateProcess(m_piProcess);

    // If we have a pending event, release it:
    if (nullptr != m_piEventToContinue)
    {
        bool rcEve = internalResumeProcess();
        GT_ASSERT(rcEve);
    }

    return SUCCEEDED(hr);
}
bool vsdProcessDebugger::isDebugging64BitApplication(bool& is64Bit) const
{
    if (!m_gotIs64Bit)
    {
        if (nullptr != m_piProgram)
        {
            // Get the module list:
            IEnumDebugModules2* piModules = nullptr;
            HRESULT hr = m_piProgram->EnumModules(&piModules);
            GT_IF_WITH_ASSERT(SUCCEEDED(hr) && (nullptr != piModules))
            {
                // Iterate the module until we find one that tells us if it's 64-bit or not:
                IDebugModule2* piCurrentModule = nullptr;
                ULONG gotCount = 0;
                hr = piModules->Next(1, &piCurrentModule, &gotCount);

                while ((!m_gotIs64Bit) && SUCCEEDED(hr) && (S_FALSE != hr) && (nullptr != piCurrentModule) && (0 < gotCount))
                {
                    MODULE_INFO moduleInfo = { 0 };
                    hr = piCurrentModule->GetInfo(MIF_FLAGS, &moduleInfo);

                    if (SUCCEEDED(hr))
                    {
                        // Cache the result:
                        (bool&)m_gotIs64Bit = true;
                        (bool&)m_is64Bit = (0 != (moduleInfo.m_dwModuleFlags & MODULE_FLAG_64BIT));
                    }

                    // Get the next module:
                    piCurrentModule->Release();
                    piCurrentModule = nullptr;
                    gotCount = 0;

                    if (!m_gotIs64Bit)
                    {
                        hr = piModules->Next(1, &piCurrentModule, &gotCount);
                    }
                }

                piModules->Release();
            }
        }
    }

    is64Bit = m_is64Bit;

    return m_gotIs64Bit;
}
int vsdProcessDebugger::amountOfDebuggedProcessThreads() const
{
    osCriticalSectionLocker threadDataCSLocker((osCriticalSection&)m_threadDataCS);
    int retVal = (int)m_debuggedProcessThreads.size();

    return retVal;
}
bool vsdProcessDebugger::getBreakpointTriggeringThreadIndex(int& index) const
{
    bool retVal = false;

    if (OS_NO_THREAD_ID != m_bpThreadId)
    {
        index = getThreadIndexFromId(m_bpThreadId);
        retVal = (-1 < index);
    }

    return retVal;
}
bool vsdProcessDebugger::getThreadId(int threadIndex, osThreadId& threadId) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= threadIndex) && (amountOfDebuggedProcessThreads() > threadIndex))
    {
        // Note that m_debuggedProcessSuspensionThread is not reported externally, so it should not appear here:
        osCriticalSectionLocker threadDataCSLocker((osCriticalSection&)m_threadDataCS);
        vsdCDebugThread* pThread = m_debuggedProcessThreads[threadIndex];
        threadDataCSLocker.leaveCriticalSection();
        GT_IF_WITH_ASSERT(nullptr != pThread)
        {
            threadId = pThread->threadId();
            retVal = (OS_NO_THREAD_ID != threadId);
        }
    }

    return retVal;
}
void vsdProcessDebugger::setSpiesAPIThreadId(osThreadId spiesAPIThreadId)
{
    bool threadIdIsNew = (m_spiesAPIThreadId != spiesAPIThreadId);

    if (threadIdIsNew)
    {
        if (OS_NO_THREAD_ID != m_spiesAPIThreadId)
        {
            vsdCDebugThread* pOldSpiesThread = getThreadFromId(m_spiesAPIThreadId);
            GT_IF_WITH_ASSERT(nullptr != pOldSpiesThread)
            {
                // This thread is no longer used as the spy API thread, allow it to be suspended:
                pOldSpiesThread->setCanSuspendThread(true);
            }
        }

        // Set the new value:
        m_spiesAPIThreadId = spiesAPIThreadId;

        if (OS_NO_THREAD_ID != m_spiesAPIThreadId)
        {
            vsdCDebugThread* pNewSpiesThread = getThreadFromId(m_spiesAPIThreadId);
            GT_IF_WITH_ASSERT(nullptr != pNewSpiesThread)
            {
                // This thread is now used as the spy API thread, do not allow it to be suspended:
                pNewSpiesThread->setCanSuspendThread(false);
            }
        }
    }
}

int vsdProcessDebugger::spiesAPIThreadIndex() const
{
    int retVal = -1;

    // Iterate the threads:
    osCriticalSectionLocker threadDataCSLocker((osCriticalSection&)m_threadDataCS);
    int numberOfThreads = (int)m_debuggedProcessThreads.size();

    for (int i = 0; i < numberOfThreads; i++)
    {
        vsdCDebugThread* pCurrentThread = m_debuggedProcessThreads[i];
        GT_IF_WITH_ASSERT(pCurrentThread != NULL)
        {
            // If this is the API thread:
            if (pCurrentThread->threadId() == m_spiesAPIThreadId)
            {
                retVal = i;
                break;
            }
        }
    }

    return retVal;
}
osThreadId vsdProcessDebugger::mainThreadId() const
{
    return m_mainThreadId;
}
osThreadId vsdProcessDebugger::spiesAPIThreadId() const
{
    return m_spiesAPIThreadId;
}
osProcessId vsdProcessDebugger::debuggedProcessId() const
{
    // Check the cached value:
    if (0 == m_processId)
    {
        if (m_piProcess != NULL)
        {
            PROCESS_INFO processInformation = { 0 };
            HRESULT hr = m_piProcess->GetInfo(PIF_PROCESS_ID, &processInformation);

            if (SUCCEEDED(hr))
            {
                switch (processInformation.ProcessId.ProcessIdType)
                {
                    case AD_PROCESS_ID_SYSTEM:
                        (osProcessId&)m_processId = (osProcessId)processInformation.ProcessId.ProcessId.dwProcessId;
                        break;

                    case AD_PROCESS_ID_GUID:
                        // We don't currently support GUID process IDs.
                        GT_ASSERT(false);
                        break;

                    default:
                        // Unexpected value!
                        GT_ASSERT(false);
                        break;
                }
            }
        }
    }

    return m_processId;
}
bool vsdProcessDebugger::isSpiesAPIThreadRunning() const
{
    vsdCDebugThread* pAPIThread = getThreadFromId(m_spiesAPIThreadId);

    bool retVal = (pAPIThread != NULL);

    return retVal;
}
bool vsdProcessDebugger::suspendDebuggedProcess()
{
    bool retVal = suspendProcessThreads();

    if (retVal)
    {
        // Notify observers that the debugged process run was suspended:
        apDebuggedProcessRunSuspendedEvent processSuspendedEvent(OS_NO_THREAD_ID);
        apEventsHandler::instance().registerPendingDebugEvent(processSuspendedEvent);
    }

    return retVal;
}
bool vsdProcessDebugger::resumeDebuggedProcess()
{
    bool retVal = true;
    osThreadId bpThreadId = m_bpThreadId;
    m_bpThreadId = OS_NO_THREAD_ID;

    if (m_isDebuggedProcessAtFatalException)
    {
        terminateDebuggedProcess();
    }
    else
    {
        // Notify observers that the debugged process run is resumed:
        apDebuggedProcessRunResumedEvent processResumedEvent;
        apEventsHandler::instance().registerPendingDebugEvent(processResumedEvent);

        // Note that this must be outside the critical section locker, since the
        // ::DebugBreakProcess function may create a thread inside the debugged process.
        bool rcHlt = internalHaltProcess();
        GT_ASSERT(rcHlt);

        // US, 20/3/16 - halting the process when no threads are running might result in a GUI hang.
        // We only know at least one thread is (internally) running when the spy API thread exists.
        // If the Spies API thread does not yet exist, there is no need to use the internal resume mechanism:
        if (OS_NO_THREAD_ID != m_spiesAPIThreadId)
        {
            osCriticalSectionLocker threadDataCSLocker(m_threadDataCS);

            for (vsdCDebugThread* pThread : m_debuggedProcessThreads)
            {
                // Do not resume driver threads and the spy API thread:
                retVal = pThread->resumeThread() && retVal;

                // If this is the breakpoint thread during kernel debugging,
                // restore its status as a non-suspendable thread:
                if (pThread->isDriverThread())
                {
                    osThreadId currentThreadId = pThread->threadId();

                    if (currentThreadId == bpThreadId)
                    {
                        pThread->setCanSuspendThread(false);
                    }
                }
            }
        }

        bool rcRes = internalResumeProcess();
        GT_ASSERT(rcRes);

        // Mark the debugged process run as resumed (not suspended):
        m_isDebuggedProcessSuspended = false;
        m_isAPIBreakpoint = false;
        m_hostBreakReason = AP_FOREIGN_BREAK_HIT;
        m_currentBreakpointRequestedFile.clear();
        m_currentBreakpointRequestedLine = -1;
    }

    return retVal;
}
bool vsdProcessDebugger::isDebuggedProcssSuspended()
{
    return m_isDebuggedProcessSuspended;
}
bool vsdProcessDebugger::suspendDebuggedProcessThread(osThreadId threadId)
{
    bool retVal = false;

    vsdCDebugThread* pThread = getThreadFromId(threadId);

    GT_IF_WITH_ASSERT(nullptr != pThread)
    {
        retVal = pThread->suspendThread();
    }

    return retVal;
}
bool vsdProcessDebugger::resumeDebuggedProcessThread(osThreadId threadId)
{
    bool retVal = false;

    vsdCDebugThread* pThread = getThreadFromId(threadId);

    GT_IF_WITH_ASSERT(nullptr != pThread)
    {
        retVal = pThread->resumeThread();
    }

    return retVal;
}
bool vsdProcessDebugger::getDebuggedThreadCallStack(osThreadId threadId, osCallStack& callStack, bool hideSpyDLLsFunctions)
{
    bool retVal = false;

    vsdCDebugThread* pThread = getThreadFromId(threadId);
    GT_IF_WITH_ASSERT(nullptr != pThread)
    {
        retVal = pThread->getCallStack(callStack, hideSpyDLLsFunctions);
    }

    return retVal;
}
void vsdProcessDebugger::fillCallsStackDebugInfo(osCallStack& callStack, bool hideSpyDLLsFunctions)
{
    GT_UNREFERENCED_PARAMETER(callStack);
    GT_UNREFERENCED_PARAMETER(hideSpyDLLsFunctions);

    // We should get debug info from the native debug engine itself.
}
void vsdProcessDebugger::fillThreadCreatedEvent(apThreadCreatedEvent& event)
{
    GT_UNREFERENCED_PARAMETER(event);

    // IMPLEMENTME!
}
bool vsdProcessDebugger::canGetCallStacks()
{
    return true;
}
bool vsdProcessDebugger::canMakeThreadExecuteFunction(const osThreadId& threadId)
{
    bool retVal = (PD_NO_FUNCTION_EXECUTION != functionExecutionMode());

    if (retVal && (m_spiesAPIThreadId != threadId))
    {
        vsdCDebugThread* pThread = getThreadFromId(threadId);

        if (nullptr != pThread)
        {
            // CODEXL-2946 - direct execution mode is currently causing call stack trouble.
            // We can currently only execute functions at threads suspended on API functions:
            retVal = !pThread->isDriverThread() && (0 < pThread->skippedFrameCount(true));
        }
    }

    return retVal;
}
bool vsdProcessDebugger::makeThreadExecuteFunction(const osThreadId& threadId, osProcedureAddress64 funcAddress)
{
    bool retVal = false;

    vsdCDebugThread* pThread = getThreadFromId(threadId);
    GT_IF_WITH_ASSERT(nullptr != pThread)
    {
        retVal = pThread->executeFunction(funcAddress, m_waitingForExecutedFunction);
    }

    return retVal;
}
pdProcessDebugger::FunctionExecutionMode vsdProcessDebugger::functionExecutionMode() const
{
    // return m_isAPIBreakpoint ? PD_EXECUTION_IN_BREAK_MODE : PD_DIRECT_EXECUTION_MODE;
    // CODEXL-2946 - direct execution mode is currently causing call stack trouble. If we are at a host breakpoint, do not allow
    // making debugger thread execute functions
    return m_isAPIBreakpoint ? PD_EXECUTION_IN_BREAK_MODE : PD_NO_FUNCTION_EXECUTION;
}
void vsdProcessDebugger::afterAPICallIssued()
{
    // Nothing to do, as the API thread is continued internally.
}
bool vsdProcessDebugger::canGetHostVariables() const
{
    return true;
}
bool vsdProcessDebugger::getHostLocals(osThreadId threadId, int callStackFrameIndex, gtVector<gtString>& o_variables)
{
    bool retVal = false;

    vsdCDebugThread* pThread = getThreadFromId(threadId);
    GT_IF_WITH_ASSERT(nullptr != pThread)
    {
        retVal = pThread->listLocalsForStackFrame(callStackFrameIndex, o_variables);
    }

    return retVal;
}
bool vsdProcessDebugger::getHostVariableValue(osThreadId threadId, int callStackFrameIndex, const gtString& variableName, gtString& o_varValue, gtString& o_varValueHex, gtString& o_varType)
{
    bool retVal = false;

    vsdCDebugThread* pThread = getThreadFromId(threadId);
    GT_IF_WITH_ASSERT(nullptr != pThread)
    {
        int skippedFrames = pThread->skippedFrameCount(true);
        retVal = pThread->evaluateVariableInStackFrame(callStackFrameIndex + skippedFrames, variableName, o_varValue, o_varValueHex, o_varType, true);
    }

    return retVal;
}
bool vsdProcessDebugger::canPerformHostDebugging() const
{
    return true;
}
bool vsdProcessDebugger::isAtAPIOrKernelBreakpoint(osThreadId threadId) const
{
    bool isSuspendingThread = (m_bpThreadId == threadId) || (OS_NO_THREAD_ID == threadId);
    return isSuspendingThread && m_isAPIBreakpoint;
}
apBreakReason vsdProcessDebugger::hostBreakReason() const
{
    return m_hostBreakReason;
}
bool vsdProcessDebugger::getHostBreakpointLocation(osFilePath& bpFile, int& bpLine) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(AP_HOST_BREAKPOINT_HIT == m_hostBreakReason)
    {
        bpFile = m_currentBreakpointRequestedFile;
        bpLine = m_currentBreakpointRequestedLine;

        retVal = ((0 < m_currentBreakpointRequestedLine) || (!m_currentBreakpointRequestedFile.isEmpty()));
    }

    return retVal;
}
bool vsdProcessDebugger::setHostSourceBreakpoint(const osFilePath& fileName, int lineNumber)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((nullptr != m_piNativeDebugEngine) && (nullptr != m_piProgram))
    {
        vsdCDebugBreakpointRequest* pBPRequest = new vsdCDebugBreakpointRequest(*m_piProgram, fileName, lineNumber);
        IDebugPendingBreakpoint2* piPendingBreakpoint = nullptr;
        HRESULT hr = m_piNativeDebugEngine->CreatePendingBreakpoint(pBPRequest, &piPendingBreakpoint);

        if (SUCCEEDED(hr))
        {
            GT_IF_WITH_ASSERT(nullptr != piPendingBreakpoint)
            {
                IEnumDebugErrorBreakpoints2* piErrors = nullptr;
                hr = piPendingBreakpoint->CanBind(&piErrors);

                if (SUCCEEDED(hr) && (S_FALSE != hr))
                {
                    hr = piPendingBreakpoint->Enable(TRUE);
                    GT_ASSERT(SUCCEEDED(hr));
                    hr = piPendingBreakpoint->Bind();

                    if (SUCCEEDED(hr))
                    {
                        retVal = true;
                        vsdHostBreakpoint* pNewBP = new(std::nothrow)vsdHostBreakpoint(*piPendingBreakpoint, fileName, lineNumber);
                        GT_IF_WITH_ASSERT(nullptr != pNewBP)
                        {
                            m_hostBreakpoints.push_back(pNewBP);
                        }
                    }
                    else
                    {
                        hr = piPendingBreakpoint->Delete();
                        GT_ASSERT(SUCCEEDED(hr));
                    }
                }

                if (nullptr != piErrors)
                {
                    piErrors->Release();
                }

                piPendingBreakpoint->Release();
            }
        }

        pBPRequest->Release();
    }

    return retVal;
}
bool vsdProcessDebugger::deleteHostSourceBreakpoint(const osFilePath& fileName, int lineNumber)
{
    bool retVal = false;

    // Find the requested breakpoint(s):
    int found = 0;
    int bpCount = (int)m_hostBreakpoints.size();

    for (int i = 0; i < bpCount; ++i)
    {
        vsdHostBreakpoint* b = m_hostBreakpoints[i];

        bool sameLocation = false;

        if (nullptr != b)
        {
            const osFilePath& bPath = b->requestedPath();
            int bLine = b->requestedLineNumber();

            if ((lineNumber == bLine) && (fileName == bPath))
            {
                sameLocation = true;
            }
        }

        if (sameLocation)
        {
            // Destroy the breakpoint:
            bool rcRem = b->removeRealBreakpoint();
            GT_ASSERT(rcRem);

            delete b;
            m_hostBreakpoints[i] = nullptr;
            ++found;
        }
        else if (0 < found)
        {
            // Copy backwards, overwriting destroyed breakpoints. Note that i is always at least equal to found:
            m_hostBreakpoints[i - found] = b;
        }
    }

    GT_IF_WITH_ASSERT(0 < found)
    {
        // Remove as many (now destroyed) pointers as we have found:
        for (int i = 0; i < found; ++i)
        {
            m_hostBreakpoints.pop_back();
        }

        retVal = true;
    }

    return retVal;
}
bool vsdProcessDebugger::setHostFunctionBreakpoint(const gtString& funcName)
{
    GT_UNREFERENCED_PARAMETER(funcName);
    // IMPLEMENTME
    return false;
}
bool vsdProcessDebugger::performHostStep(osThreadId threadId, StepType stepType)
{
    bool retVal = false;

    // In some situations, the step ended event will appear during this function's execution.
    // Make the handling of that event wait until this operation completes (acting as a one-way critical section):
    m_issuingStepCommand = true;

    if (!m_isDebuggedProcessAtFatalException)
    {
        bool rcHlt = internalHaltProcess();
        GT_ASSERT(rcHlt);
    }

    GT_IF_WITH_ASSERT(nullptr != m_piProgram)
    {
        vsdCDebugThread* pThread = getThreadFromId(threadId);
        GT_IF_WITH_ASSERT(nullptr != pThread)
        {
            vsdCDebugProcess* pProcess = (vsdCDebugProcess*)m_piProcess;
            retVal = pThread->performStep(*(pProcess), *m_piProgram, stepType);
        }
    }

    if (retVal)
    {
        // Save the step kind:
        switch (stepType)
        {
            case pdProcessDebugger::PD_STEP_IN:     m_lastStepKind = AP_STEP_IN_BREAKPOINT_HIT;     break;

            case pdProcessDebugger::PD_STEP_OVER:   m_lastStepKind = AP_STEP_OVER_BREAKPOINT_HIT;   break;

            case pdProcessDebugger::PD_STEP_OUT:    m_lastStepKind = AP_STEP_OUT_BREAKPOINT_HIT;    break;

            default:                                m_lastStepKind = AP_FOREIGN_BREAK_HIT;          break;
        }
    }

    // If the Spies API thread does not yet exist, there is no need to use the internal resume mechanism:
    if (OS_NO_THREAD_ID == m_spiesAPIThreadId)
    {
        m_skipContinueEvent = true;
    }
    else
    {
        bool rcRes = internalResumeProcess();
        GT_ASSERT(rcRes);
    }

    // Now continue the process run:
    bool rcProc = resumeDebuggedProcess();
    GT_ASSERT(rcProc);

    m_issuingStepCommand = false;

    return retVal;
}

bool vsdProcessDebugger::suspendHostDebuggedProcess()
{
    bool retVal = false;

    osProcessHandle hProcess = debuggedProcessHandle();
    GT_IF_WITH_ASSERT(nullptr != hProcess)
    {
        GT_ASSERT(AP_FOREIGN_BREAK_HIT == m_hostBreakReason);
        m_hostBreakReason = AP_BREAK_COMMAND_HIT;

        BOOL rcBreak = ::DebugBreakProcess(hProcess);

        if (TRUE == rcBreak)
        {
            retVal = true;
        }
        else // TRUE != rcBreak
        {
            m_hostBreakReason = AP_FOREIGN_BREAK_HIT;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::getThreadFromId
// Description: Gets a debugged process thread from its id
// Author:      Uri Shomroni
// Date:        8/1/2012
// ---------------------------------------------------------------------------
vsdProcessDebugger::vsdCDebugThread* vsdProcessDebugger::getThreadFromId(osThreadId threadId) const
{
    vsdCDebugThread* retVal = NULL;

    if (threadId != OS_NO_THREAD_ID)
    {
        // Check if this is the suspension thread:
        if (nullptr != m_debuggedProcessSuspensionThread1)
        {
            if (m_debuggedProcessSuspensionThread1->threadId() == threadId)
            {
                retVal = m_debuggedProcessSuspensionThread1;
            }
        }

        if ((nullptr == retVal) && (nullptr != m_debuggedProcessSuspensionThread2))
        {
            if (m_debuggedProcessSuspensionThread2->threadId() == threadId)
            {
                retVal = m_debuggedProcessSuspensionThread2;
            }
        }

        if (nullptr == retVal)
        {
            // Iterate the threads vector:
            osCriticalSectionLocker threadDataCSLocker((osCriticalSection&)m_threadDataCS);
            int numberOfThreads = (int)m_debuggedProcessThreads.size();

            for (int i = 0; i < numberOfThreads; i++)
            {
                vsdCDebugThread* pCurrentThread = (vsdCDebugThread*)m_debuggedProcessThreads[i];
                GT_IF_WITH_ASSERT(pCurrentThread != NULL)
                {
                    if (pCurrentThread->threadId() == threadId)
                    {
                        retVal = pCurrentThread;

                        break;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::getThreadIndexFromId
// Description: Gets a debugged process thread index from its id
// Author:      Uri Shomroni
// Date:        28/3/2016
// ---------------------------------------------------------------------------
int vsdProcessDebugger::getThreadIndexFromId(osThreadId threadId) const
{
    int retVal = -1;

    if (threadId != OS_NO_THREAD_ID)
    {
        // Iterate the threads vector. Note that m_debuggedProcessSuspensionThread is not reported and therefore not indexed:
        osCriticalSectionLocker threadDataCSLocker((osCriticalSection&)m_threadDataCS);
        int numberOfThreads = (int)m_debuggedProcessThreads.size();

        for (int i = 0; i < numberOfThreads; i++)
        {
            vsdCDebugThread* pCurrentThread = (vsdCDebugThread*)m_debuggedProcessThreads[i];
            GT_IF_WITH_ASSERT(nullptr != pCurrentThread)
            {
                if (pCurrentThread->threadId() == threadId)
                {
                    retVal = i;

                    break;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::IsSpyModulePath
// Description: Returns true iff the given path is a spy module
// Author:      Uri Shomroni
// Date:        21/1/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::IsSpyModulePath(const gtString& modulePath) const
{
    gtString modulePathLower = modulePath;
    modulePathLower.toLowerCase();
    bool retVal = (-1 != modulePathLower.find(OS_SPY_UTILS_FILE_PREFIX));

    if (!retVal && (nullptr != m_pProcessCreationData))
    {
        gtString spiesFolderLower = m_pProcessCreationData->spiesDirectory().asString();
        spiesFolderLower.toLowerCase();
        retVal = (-1 != modulePathLower.find(spiesFolderLower));
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::IsSpySourcePath
// Description: Returns true iff the given path is a spy source path
// Author:      Uri Shomroni
// Date:        21/1/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::IsSpySourcePath(const gtString& sourcePath, bool& skipAnotherFrame) const
{
    static const gtString spyFileName1 = L"gsopenglwrappers.cpp";               // gsOpenGLWrappers.cpp
    static const gtString spyFileName2 = L"gsopenglmonitor.cpp";                // gsOpenGLMonitor.cpp
    static const gtString spyFileName3 = L"gsopenglextensionswrappers.cpp";     // gsOpenGLExtensionsWrappers.cpp
    static const gtString spyFileName4 = L"gswglwrappers.cpp";                  // gsWGLWrappers.cpp
    static const gtString spyFileName5 = L"csopenclwrappers.cpp";               // csOpenCLWrappers.cpp
    static const gtString spyFileName6 = L"csopenclextensionswrappers.cpp";     // csOpenCLExtensionsWrappers.cpp
    static const gtString spyFileName7 = L"csopenglintegrationwrappers.cpp";    // csOpenGLIntegrationWrappers.cpp
    static const gtString spyFileName8 = L"csopenclmonitor.cpp";                // csOpenCLMonitor.cpp
    static const gtString spyFileName9 = L"csdirectxintegrationwrappers.cpp";   // csDirectXIntegrationWrappers.cpp
    static const gtString spyFileName10 = L"hdhsawrappers.cpp";                 // hdHSAWrappers.cpp

    static const gtString spyFileName1WithExtraFrame = L"gsapifunctionsstubs.cpp";

    gtString sourcePathLower(sourcePath);
    sourcePathLower.toLowerCase();

    skipAnotherFrame = (-1 != sourcePathLower.find(spyFileName1WithExtraFrame));

    return (skipAnotherFrame ||
            (-1 != sourcePathLower.find(spyFileName1)) ||
            (-1 != sourcePathLower.find(spyFileName2)) ||
            (-1 != sourcePathLower.find(spyFileName3)) ||
            (-1 != sourcePathLower.find(spyFileName4)) ||
            (-1 != sourcePathLower.find(spyFileName5)) ||
            (-1 != sourcePathLower.find(spyFileName6)) ||
            (-1 != sourcePathLower.find(spyFileName7)) ||
            (-1 != sourcePathLower.find(spyFileName8)) ||
            (-1 != sourcePathLower.find(spyFileName9)) ||
            (-1 != sourcePathLower.find(spyFileName10)));
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::IsSpyFuncName
// Description: Returns true iff the given function name is a spy function
// Author:      Uri Shomroni
// Date:        21/1/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::IsSpyFuncName(const gtString& funcName) const
{
    return OS_SPIES_BREAKPOINT_FUNCTION_NAME == funcName;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::IsDriverAddress
// Description: Returns true iff the given address is a driver address
// Author:      Uri Shomroni
// Date:        7/3/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::IsDriverAddress(osInstructionPointer pc)
{
    bool retVal = false;

    // Sanity check:
    if ((osInstructionPointer)0 != pc)
    {
        // Get the modules
        IEnumDebugModules2* piModules = nullptr;
        HRESULT hr = m_piProgram->EnumModules(&piModules);

        if (SUCCEEDED(hr) && (nullptr != piModules))
        {
            // Iterate the modules until we find the one containing the address:
            IDebugModule2* piCurrentModule = nullptr;
            ULONG gotModule = 0;
            hr = piModules->Next(1, &piCurrentModule, &gotModule);
            bool goOn = true;
            MODULE_INFO curModuleInf = { 0 };

            while (goOn && SUCCEEDED(hr) && (S_FALSE != hr) && (nullptr != piCurrentModule) && (0 < gotModule))
            {
                // Get the module's address range:
                hr = piCurrentModule->GetInfo(MIF_LOADADDRESS | MIF_SIZE, &curModuleInf);

                if (SUCCEEDED(hr) && (MIF_LOADADDRESS | MIF_SIZE) == (curModuleInf.dwValidFields & (MIF_LOADADDRESS | MIF_SIZE)))
                {
                    // If the address is in range:
                    if ((0 < curModuleInf.m_addrLoadAddress) &&
                        (curModuleInf.m_addrLoadAddress <= pc) &&
                        (curModuleInf.m_addrLoadAddress + curModuleInf.m_dwSize > pc))
                    {
                        // Get the module's name:
                        hr = piCurrentModule->GetInfo(MIF_NAME, &curModuleInf);

                        if (SUCCEEDED(hr))
                        {
                            // If it's a driver module, the address is a driver address:
                            gtString moduleNameLower = curModuleInf.m_bstrName;
                            moduleNameLower.toLowerCase();
                            retVal = IsDriverModuleName(moduleNameLower);

                            // Release the string:
                            // SysFreeString(curModuleInf.m_bstrName);
                            curModuleInf.m_bstrName = nullptr;
                        }

                        // We have found the module containing the address:
                        goOn = false;
                    }
                }

                // Release the module:
                piCurrentModule->Release();
                piCurrentModule = nullptr;

                // If we continue:
                if (goOn)
                {
                    // Get the next module:
                    hr = piModules->Next(1, &piCurrentModule, &gotModule);
                }
            }

            // Release the enumerator:
            piModules->Release();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::IsDriverModuleName
// Description: Returns true iff the given module name is a driver module
// Author:      Uri Shomroni
// Date:        7/3/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::IsDriverModuleName(const gtString& moduleNameLower) const
{
    bool retVal = false;

    // Strings for comparison:
    static gtString systemGLModuleName;
    static gtString systemCLModuleName1;
    static gtString systemCLModuleName2;
    static gtString amdCLModuleName1;
    static gtString amdCLModuleName2;
    static bool systemNamesInitialized = false;

    // Init the strings only once:
    if (!systemNamesInitialized)
    {
        systemGLModuleName = OS_OPENGL_MODULE_NAME;
        systemGLModuleName.toLowerCase();
        systemCLModuleName1 = OS_OPENCL_ICD_MODULE_NAME;
        systemCLModuleName1.toLowerCase();
        systemCLModuleName2 = OS_OPENCL_ICD_MODULE_ALTERNATIVE_NAME;
        systemCLModuleName2.toLowerCase();
        amdCLModuleName1 = OS_AMD_OPENCL_RUNTIME_MODULE_NAME;
        amdCLModuleName1.toLowerCase();
        amdCLModuleName2 = OS_AMD_OPENCL_RUNTIME_MODULE_NAME_OTHER_BITNESS;
        amdCLModuleName2.toLowerCase();
        systemNamesInitialized = true;
    }

    // The the module name is one of the names, it's a driver module:
    retVal = ((-1 != moduleNameLower.find(systemGLModuleName))  ||
              (-1 != moduleNameLower.find(systemCLModuleName1)) ||
              (-1 != moduleNameLower.find(systemCLModuleName2)) ||
              (-1 != moduleNameLower.find(amdCLModuleName1))    ||
              (-1 != moduleNameLower.find(amdCLModuleName2)));

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::vsdCDebugThread
// Description: Constructor
// Author:      Uri Shomroni
// Date:        8/1/2012
// ---------------------------------------------------------------------------
vsdProcessDebugger::vsdCDebugThread::vsdCDebugThread(IDebugThread2& riThread)
    : m_isDriverThread(false), m_threadStartAddress((osInstructionPointer)0), m_piDebugThread(&riThread), m_hThread(nullptr), m_canSuspendThread(true), m_isSuspended(false)
{
    m_piDebugThread->AddRef();

    // Detect the thread's entry point:
    detectThreadEntryPoint();
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::~vsdCDebugThread
// Description: Destructor
// Author:      Uri Shomroni
// Date:        8/1/2012
// ---------------------------------------------------------------------------
vsdProcessDebugger::vsdCDebugThread::~vsdCDebugThread()
{
    GT_IF_WITH_ASSERT(m_piDebugThread != NULL)
    {
        if (m_isSuspended)
        {
            // Do not perform this operation during process termination:
            bool processExists = vsdProcessDebugger::vsInstance().debuggedProcessExists();

            if (processExists)
            {
                bool rcRes = resumeThread();
                GT_ASSERT(rcRes);
            }
        }

        m_piDebugThread->Release();
        m_piDebugThread = NULL;
    }

    if (nullptr != m_hThread)
    {
        BOOL rc = ::CloseHandle(m_hThread);
        GT_ASSERT(FALSE != rc);
        m_hThread = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::setCanSuspendThread
// Description: Suspends the thread
// Author:      Uri Shomroni
// Date:        7/1/2016
// ---------------------------------------------------------------------------
void vsdProcessDebugger::vsdCDebugThread::setCanSuspendThread(bool canSuspend)
{
    bool extensiveLog = (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity());

    if (extensiveLog)
    {
        gtString logMsg;
        logMsg.appendFormattedString(L"Attempting to set can suspend thread %#x to %c. Sus? %c, Can? %c", threadId(), canSuspend ? 'Y' : 'N', m_isSuspended ? 'Y' : 'N', m_canSuspendThread ? 'Y' : 'N');
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
    }

    bool forceResume = false;
    bool forceSuspend = false;

    if (canSuspend && (!m_canSuspendThread))
    {
        // If we are turning the option on:
        if (m_isSuspended)
        {
            forceSuspend = true;
        }
    }
    else if ((!canSuspend) && m_canSuspendThread)
    {
        // If we are turning the option off:
        if (m_isSuspended)
        {
            forceResume = true;
        }
    }

    if (forceResume)
    {
        // Resume internally:
        resumeThread();
        m_isSuspended = true;
    }

    m_canSuspendThread = canSuspend;

    if (forceSuspend)
    {
        // Suspend internally:
        m_isSuspended = false;
        suspendThread();
    }

    if (extensiveLog)
    {
        OS_OUTPUT_DEBUG_LOG(L"Finished setting can resume thread", OS_DEBUG_LOG_EXTENSIVE);
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::suspendThread
// Description: Suspends the thread
// Author:      Uri Shomroni
// Date:        7/1/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::vsdCDebugThread::suspendThread()
{
    bool retVal = false;

    bool extensiveLog = (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity());

    if (extensiveLog)
    {
        gtString logMsg;
        logMsg.appendFormattedString(L"Attempting to suspend thread %#x. Sus? %c, Can? %c", threadId(), m_isSuspended ? 'Y' : 'N', m_canSuspendThread ? 'Y' : 'N');
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
    }

    GT_IF_WITH_ASSERT(!m_isSuspended)
    {
        if (m_canSuspendThread)
        {
            DWORD newSuspendCount = 0;
            HRESULT hr = m_piDebugThread->Suspend(&newSuspendCount);
            GT_ASSERT(SUCCEEDED(hr));
            // GT_ASSERT(1 == newSuspendCount); // Do not assert this value, it is usually 0.

            retVal = SUCCEEDED(hr); // && (1 == newSuspendCount);

            if (extensiveLog && !retVal)
            {
                gtString logMsg;
                logMsg.appendFormattedString(L"Win32 failure code: %#x", hr);
                OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
            }
        }
        else
        {
            retVal = true;
        }
    }

    if (retVal)
    {
        m_isSuspended = true;
    }

    if (extensiveLog)
    {
        OS_OUTPUT_DEBUG_LOG(retVal ? L"Thread suspend succeeded" : L"Thread suspend failed", OS_DEBUG_LOG_EXTENSIVE);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::resumeThread
// Description: Resumes the thread
// Author:      Uri Shomroni
// Date:        7/1/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::vsdCDebugThread::resumeThread()
{
    bool retVal = false;

    bool extensiveLog = (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity());

    if (extensiveLog)
    {
        gtString logMsg;
        logMsg.appendFormattedString(L"Attempting to resume thread %#x. Sus? %c, Can? %c", threadId(), m_isSuspended ? 'Y' : 'N', m_canSuspendThread ? 'Y' : 'N');
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
    }

    GT_IF_WITH_ASSERT(m_isSuspended)
    {
        if (m_canSuspendThread)
        {
            DWORD newSuspendCount = 1;
            HRESULT hr = m_piDebugThread->Resume(&newSuspendCount);
            GT_ASSERT(SUCCEEDED(hr));
            // GT_ASSERT(0 == newSuspendCount); // Do not assert this value, it is sometimes 1

            retVal = SUCCEEDED(hr); // && (0 == newSuspendCount);

            if (extensiveLog && !retVal)
            {
                gtString logMsg;
                logMsg.appendFormattedString(L"Win32 failure code: %#x", hr);
                OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
            }
        }
        else
        {
            retVal = true;
        }
    }

    if (retVal)
    {
        m_isSuspended = false;
    }

    if (extensiveLog)
    {
        OS_OUTPUT_DEBUG_LOG(retVal ? L"Thread resume succeeded" : L"Thread resume failed", OS_DEBUG_LOG_EXTENSIVE);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::threadId
// Description: Returns the thread id
// Author:      Uri Shomroni
// Date:        8/1/2012
// ---------------------------------------------------------------------------
osThreadId vsdProcessDebugger::vsdCDebugThread::threadId()
{
    osThreadId retVal = OS_NO_THREAD_ID;

    DWORD dwThreadId = 0;
    HRESULT hr = m_piDebugThread->GetThreadId(&dwThreadId);

    if (SUCCEEDED(hr))
    {
        retVal = (osThreadId)dwThreadId;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::getCallStack
// Description: Gets the thread's call stack
// Author:      Uri Shomroni
// Date:        7/1/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::vsdCDebugThread::getCallStack(osCallStack& callStack, bool hideSpyDLLsFunctions)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(nullptr != m_piDebugThread)
    {
        IEnumDebugFrameInfo2* piFramesInfo = nullptr;
        FRAMEINFO_FLAGS requestedFields = FIF_FUNCNAME | FIF_FRAME | FIF_DEBUG_MODULEP;
        HRESULT hr = m_piDebugThread->EnumFrameInfo(requestedFields, 0, &piFramesInfo);
        GT_IF_WITH_ASSERT(SUCCEEDED(hr) && (nullptr != piFramesInfo))
        {
            retVal = true;

            const vsdProcessDebugger& theVSDProcessDebugger = vsdProcessDebugger::vsInstance();

            bool firstFrame = true;
            bool removedPreviousFrame = false;
            FRAMEINFO currentFrameInfo = { 0 };
            ULONG readFrames = 0;
            bool skipNextFrame = false;
            hr = piFramesInfo->Next(1, &currentFrameInfo, &readFrames);

            while (SUCCEEDED(hr) && (S_FALSE != hr) && (0 < readFrames))
            {
                osCallStackFrame currentStackFrame;
                gtString funcName = currentFrameInfo.m_bstrFuncName;
                currentStackFrame.setFunctionName(funcName);
                SysFreeString(currentFrameInfo.m_bstrFuncName);
                currentFrameInfo.m_bstrFuncName = nullptr;
                currentStackFrame.setKernelSourceCode(false);
                bool isSpyFunc = theVSDProcessDebugger.IsSpyFuncName(funcName);

                // If we successfully got the frame:
                if (nullptr != currentFrameInfo.m_pFrame)
                {
                    // Get the source location (document context):
                    IDebugDocumentContext2* piDocContext = nullptr;
                    hr = currentFrameInfo.m_pFrame->GetDocumentContext(&piDocContext);

                    if (SUCCEEDED(hr) && (nullptr != piDocContext))
                    {
                        // Source path:
                        BSTR bstrSrcPath = nullptr;
                        hr = piDocContext->GetName(GN_FILENAME, &bstrSrcPath);

                        if (!SUCCEEDED(hr) || (nullptr == bstrSrcPath))
                        {
                            hr = piDocContext->GetName(GN_URL, &bstrSrcPath);
                        }

                        if (SUCCEEDED(hr) && (nullptr != bstrSrcPath))
                        {
                            osFilePath srcPath(bstrSrcPath);
                            currentStackFrame.setSourceCodeFilePath(srcPath);

                            // Free the string:
                            SysFreeString(bstrSrcPath);

                            isSpyFunc = skipNextFrame;
                            skipNextFrame = false;
                            isSpyFunc = isSpyFunc || theVSDProcessDebugger.IsSpySourcePath(srcPath.asString(), skipNextFrame);
                        }

                        // Line number:
                        TEXT_POSITION textRange[2] = { { 0 }, { 0 } };
                        hr = piDocContext->GetStatementRange(&(textRange[0]), &(textRange[1]));

                        if (SUCCEEDED(hr))
                        {
                            int lineNum = (int)(textRange[0].dwLine + 1);
                            currentStackFrame.setSourceCodeFileLineNumber(lineNum);
                        }

                        // Release the interface:
                        piDocContext->Release();
                    }

                    // Get the call site (code context)
                    IDebugCodeContext2* piCodeContext = nullptr;
                    hr = currentFrameInfo.m_pFrame->GetCodeContext(&piCodeContext);

                    if (SUCCEEDED(hr) && (nullptr != piCodeContext))
                    {
                        CONTEXT_INFO ctxInfo = { 0 };
                        hr = piCodeContext->GetInfo(CIF_ADDRESS | CIF_ADDRESSOFFSET, &ctxInfo);

                        if (SUCCEEDED(hr))
                        {
                            // Get the stack
                            gtUInt64 u64Addr = 0;
                            gtString strAddr(ctxInfo.bstrAddress);

                            if (strAddr.toUnsignedInt64Number(u64Addr))
                            {
                                osInstructionPointer codeAddr = (osInstructionPointer)u64Addr;

                                if (0 != codeAddr)
                                {
                                    // Set the instruction pointer:
                                    currentStackFrame.setInstructionCounterAddress(codeAddr);
                                }
                            }

                            // Calculate the frame base address:
                            gtUInt64 u64Off = 0;
                            gtString strOff(ctxInfo.bstrAddressOffset);

                            if (strAddr.toUnsignedInt64Number(u64Off))
                            {
                                osInstructionPointer frameBase = (osInstructionPointer)(u64Addr > u64Off) ? (u64Addr - u64Off) : 0;

                                if (0 != frameBase)
                                {
                                    // Set it to the frame:
                                    currentStackFrame.setFunctionStartAddress(frameBase);
                                }
                            }

                            // Release strings:
                            SysFreeString(ctxInfo.bstrAddress);
                            SysFreeString(ctxInfo.bstrAddressOffset);
                        }

                        if (removedPreviousFrame && !firstFrame)
                        {
                            // Try to get the call address instead of the return address:
                            IDebugMemoryContext2* piCodeContextMinus1 = nullptr;
                            hr = piCodeContext->Subtract(1, &piCodeContextMinus1);

                            if (SUCCEEDED(hr) && (nullptr != piCodeContextMinus1))
                            {
                                // Get as code context:
                                IDebugCodeContext2* piCodeContextMinus1CodeContext = nullptr;
                                hr = piCodeContextMinus1->QueryInterface(IID_IDebugCodeContext2, (void**)(&piCodeContextMinus1CodeContext));

                                if (SUCCEEDED(hr) && (nullptr != piCodeContextMinus1CodeContext))
                                {
                                    // Get document context:
                                    IDebugDocumentContext2* piDocContextMinus1 = nullptr;
                                    hr = piCodeContextMinus1CodeContext->GetDocumentContext(&piDocContextMinus1);

                                    if (SUCCEEDED(hr) && (nullptr != piDocContextMinus1))
                                    {
                                        // Line number:
                                        TEXT_POSITION textRange[2] = { { 0 }, { 0 } };
                                        hr = piDocContextMinus1->GetStatementRange(&(textRange[0]), &(textRange[1]));

                                        if (SUCCEEDED(hr))
                                        {
                                            // Replace the previous value:
                                            int lineNum = (int)(textRange[0].dwLine + 1);

                                            if (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity())
                                            {
                                                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"Replacing call stack return line %d with call line %d for file %ls", currentStackFrame.sourceCodeFileLineNumber(), lineNum, currentStackFrame.sourceCodeFilePath().asString());
                                            }

                                            currentStackFrame.setSourceCodeFileLineNumber(lineNum);
                                        }

                                        piDocContextMinus1->Release();
                                    }

                                    piCodeContextMinus1CodeContext->Release();
                                }

                                piCodeContextMinus1->Release();
                            }
                        }

                        piCodeContext->Release();
                    }

#ifdef VSD_RELEASE_ENUMERATED_INTERFACES
                    // Release the interface:
                    currentFrameInfo.m_pFrame->Release();
                    currentFrameInfo.m_pFrame = nullptr;
#endif
                }

                // If we successfully got the module:
                if (nullptr != currentFrameInfo.m_pModule)
                {
                    MODULE_INFO_FIELDS moduleInfoFields = MIF_URL | MIF_LOADADDRESS;
                    MODULE_INFO moduleInfo = { 0 };
                    hr = currentFrameInfo.m_pModule->GetInfo(moduleInfoFields, &moduleInfo);
                    {
                        gtString modulePath = moduleInfo.m_bstrUrl;
                        currentStackFrame.setModuleFilePath(modulePath);
                        currentStackFrame.setModuleStartAddress((osInstructionPointer)moduleInfo.m_addrLoadAddress);

                        isSpyFunc = isSpyFunc || theVSDProcessDebugger.IsSpyModulePath(modulePath);
                    }

                    if (nullptr != moduleInfo.m_bstrUrl)
                    {
                        SysFreeString(moduleInfo.m_bstrUrl);
                        moduleInfo.m_bstrUrl = nullptr;
                    }

#ifdef VSD_RELEASE_ENUMERATED_INTERFACES
                    // Release the interface:
                    currentFrameInfo.m_pModule->Release();
                    currentFrameInfo.m_pModule = nullptr;
#endif
                }

                currentStackFrame.markAsSpyFunction(isSpyFunc);

                // Anything above a spy frame is considered a spy frame (e.g. c++ runtime
                // or system calls)
                if (isSpyFunc && hideSpyDLLsFunctions)
                {
                    callStack.clearStack();
                    removedPreviousFrame = true;
                }
                else
                {
                    callStack.addStackFrame(currentStackFrame);
                    removedPreviousFrame = false;
                }

                // Get the next frame:
                hr = piFramesInfo->Next(1, &currentFrameInfo, &readFrames);
                firstFrame = false;
            }

            piFramesInfo->Release();
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::skippedFrameCount
// Description: Returns the number of frames skipped by the spy hiding
// Author:      Uri Shomroni
// Date:        12/2/2016
// ---------------------------------------------------------------------------
int vsdProcessDebugger::vsdCDebugThread::skippedFrameCount(bool hideSpyDLLsFunctions)
{
    int retVal = 0;

    // No frames are skipped if we're not hiding spy functions:
    if (hideSpyDLLsFunctions)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(nullptr != m_piDebugThread)
        {
            IEnumDebugFrameInfo2* piFramesInfo = nullptr;
            FRAMEINFO_FLAGS requestedFields = FIF_FUNCNAME | FIF_FRAME | FIF_DEBUG_MODULEP;
            HRESULT hr = m_piDebugThread->EnumFrameInfo(requestedFields, 0, &piFramesInfo);
            GT_IF_WITH_ASSERT(SUCCEEDED(hr) && (nullptr != piFramesInfo))
            {
                const vsdProcessDebugger& theVSDProcessDebugger = vsdProcessDebugger::vsInstance();

                int frameCount = 0;

                FRAMEINFO currentFrameInfo = { 0 };
                ULONG readFrames = 0;
                bool skipNextFrame = false;
                hr = piFramesInfo->Next(1, &currentFrameInfo, &readFrames);

                while (SUCCEEDED(hr) && (S_FALSE != hr) && (0 < readFrames))
                {
                    ++frameCount;

                    gtString funcName = currentFrameInfo.m_bstrFuncName;
                    SysFreeString(currentFrameInfo.m_bstrFuncName);
                    currentFrameInfo.m_bstrFuncName = nullptr;
                    bool isSpyFunc = theVSDProcessDebugger.IsSpyFuncName(funcName);

                    // If we successfully got the frame:
                    if (nullptr != currentFrameInfo.m_pFrame)
                    {
                        // Get the source location (document context):
                        IDebugDocumentContext2* piDocContext = nullptr;
                        hr = currentFrameInfo.m_pFrame->GetDocumentContext(&piDocContext);

                        if (SUCCEEDED(hr) && (nullptr != piDocContext))
                        {
                            // Source path:
                            BSTR bstrSrcPath = nullptr;
                            hr = piDocContext->GetName(GN_FILENAME, &bstrSrcPath);

                            if (!SUCCEEDED(hr) || (nullptr == bstrSrcPath))
                            {
                                hr = piDocContext->GetName(GN_URL, &bstrSrcPath);
                            }

                            if (SUCCEEDED(hr) && (nullptr != bstrSrcPath))
                            {
                                osFilePath srcPath(bstrSrcPath);

                                // Free the string:
                                SysFreeString(bstrSrcPath);

                                isSpyFunc = skipNextFrame;
                                skipNextFrame = false;
                                isSpyFunc = isSpyFunc || theVSDProcessDebugger.IsSpySourcePath(srcPath.asString(), skipNextFrame);
                            }

                            // Release the interface:
                            piDocContext->Release();
                        }

#ifdef VSD_RELEASE_ENUMERATED_INTERFACES
                        // Release the interface:
                        currentFrameInfo.m_pFrame->Release();
                        currentFrameInfo.m_pFrame = nullptr;
#endif
                    }

                    // If we successfully got the module:
                    if (nullptr != currentFrameInfo.m_pModule)
                    {
                        MODULE_INFO_FIELDS moduleInfoFields = MIF_URL;
                        MODULE_INFO moduleInfo = { 0 };
                        hr = currentFrameInfo.m_pModule->GetInfo(moduleInfoFields, &moduleInfo);
                        {
                            gtString modulePath = moduleInfo.m_bstrUrl;

                            isSpyFunc = isSpyFunc || theVSDProcessDebugger.IsSpyModulePath(modulePath);
                        }

                        if (nullptr != moduleInfo.m_bstrUrl)
                        {
                            SysFreeString(moduleInfo.m_bstrUrl);
                            moduleInfo.m_bstrUrl = nullptr;
                        }

#ifdef VSD_RELEASE_ENUMERATED_INTERFACES
                        // Release the interface:
                        currentFrameInfo.m_pModule->Release();
                        currentFrameInfo.m_pModule = nullptr;
#endif
                    }

                    // Anything above a spy frame is considered a spy frame (e.g. c++ runtime
                    // or system calls)
                    if (isSpyFunc)
                    {
                        retVal = frameCount;
                    }

                    // Get the next frame:
                    hr = piFramesInfo->Next(1, &currentFrameInfo, &readFrames);
                }

                piFramesInfo->Release();
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::listLocalsForStackFrame
// Description: Lists the locals in the context of the indexed call stack frame
// Author:      Uri Shomroni
// Date:        10/1/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::vsdCDebugThread::listLocalsForStackFrame(int callStackFrameIndex, gtVector<gtString>& o_variables)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(nullptr != m_piDebugThread)
    {
        IEnumDebugFrameInfo2* piFramesInfo = nullptr;
        FRAMEINFO_FLAGS requestedFields = FIF_FRAME;
        HRESULT hr = m_piDebugThread->EnumFrameInfo(requestedFields, 0, &piFramesInfo);
        GT_IF_WITH_ASSERT(SUCCEEDED(hr) && (nullptr != piFramesInfo))
        {
            // Find the frame:
            int currentFrameIndex = -skippedFrameCount(true);

            FRAMEINFO currentFrameInfo = { 0 };
            ULONG readFrames = 0;
            hr = piFramesInfo->Next(1, &currentFrameInfo, &readFrames);

            while (SUCCEEDED(hr) && (S_FALSE != hr) && (0 < readFrames))
            {
                // If we successfully got the frame:
                if (nullptr != currentFrameInfo.m_pFrame)
                {
                    // Check if it's the correct one:
                    if (currentFrameIndex == callStackFrameIndex)
                    {
                        ULONG localsCount = 0;
                        IEnumDebugPropertyInfo2* piProperties = nullptr;
                        // Uri, 10/1/16 - DEBUGPROP_INFO_FULLNAME gives the fully qualified name and not the contextual name.
                        // Use 10 as the radix, since we only want a list of variables at this point.
                        hr = currentFrameInfo.m_pFrame->EnumProperties(DEBUGPROP_INFO_NAME, 10, guidFilterLocalsPlusArgs, VSD_EXPRESSION_EVAL_TIMEOUT_MS, &localsCount, &piProperties);
                        GT_IF_WITH_ASSERT(SUCCEEDED(hr) && (nullptr != piProperties))
                        {
                            // Get the variables:
                            retVal = true;

                            ULONG addedVars = 0;
                            ULONG gotProp = 0;
                            DEBUG_PROPERTY_INFO currentProp = { 0 };
                            hr = piProperties->Next(1, &currentProp, &gotProp);
                            gtString currentLocal;

                            while (SUCCEEDED(hr) && (S_FALSE != hr) && (0 < gotProp))
                            {
                                // Get the name:
                                currentLocal = currentProp.bstrName;
                                SysFreeString(currentProp.bstrName);
                                currentProp.bstrName = nullptr;

                                // Add it:
                                if (!currentLocal.isEmpty())
                                {
                                    o_variables.push_back(currentLocal);
                                    ++addedVars;
                                }

                                // Get the next:
                                hr = piProperties->Next(1, &currentProp, &gotProp);
                            }

                            GT_ASSERT(addedVars == localsCount);

                            // Release the properties enumerator interface:
                            piProperties->Release();
                        }
                    }

#ifdef VSD_RELEASE_ENUMERATED_INTERFACES
                    // Release the interface:
                    currentFrameInfo.m_pFrame->Release();
                    currentFrameInfo.m_pFrame = nullptr;
#endif
                }

                // Get the next frame:
                hr = piFramesInfo->Next(1, &currentFrameInfo, &readFrames);
                ++currentFrameIndex;
            }

            piFramesInfo->Release();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::evaluateVariableInStackFrame
// Description: Evaluates an expression in the context of the indexed call stack frame
// Author:      Uri Shomroni
// Date:        10/1/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::vsdCDebugThread::evaluateVariableInStackFrame(int callStackFrameIndex, const gtString& variableName, gtString& o_varValue, gtString& o_varValueHex, gtString& o_varType, bool evalHex)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(nullptr != m_piDebugThread)
    {
        IEnumDebugFrameInfo2* piFramesInfo = nullptr;
        FRAMEINFO_FLAGS requestedFields = FIF_FRAME;
        HRESULT hr = m_piDebugThread->EnumFrameInfo(requestedFields, 0, &piFramesInfo);
        GT_IF_WITH_ASSERT(SUCCEEDED(hr) && (nullptr != piFramesInfo))
        {
            // Find the frame:
            int currentFrameIndex = 0;

            FRAMEINFO currentFrameInfo = { 0 };
            ULONG readFrames = 0;
            hr = piFramesInfo->Next(1, &currentFrameInfo, &readFrames);

            while (SUCCEEDED(hr) && (S_FALSE != hr) && (0 < readFrames))
            {
                // If we successfully got the frame:
                if (nullptr != currentFrameInfo.m_pFrame)
                {
                    // Check if it's the correct one:
                    if (currentFrameIndex == callStackFrameIndex)
                    {
                        // TO_DO: consider caching values (also possible to get them from the GetLocals function
                        // which will often be called beforehand).
                        IDebugExpressionContext2* piExprContext = nullptr;
                        hr = currentFrameInfo.m_pFrame->GetExpressionContext(&piExprContext);

                        if (SUCCEEDED(hr) && (nullptr != piExprContext))
                        {
                            // Repeat for both bases:
                            const wchar_t* varNameWCStr = variableName.asCharArray();
                            PARSEFLAGS parsFlg = PARSE_EXPRESSION;
                            EVALFLAGS evalFlg = EVAL_NOEVENTS;
                            DEBUG_PROPERTY_INFO propInfo = { 0 };
                            DWORD radices[2] = { 10, 16 };

                            for (DWORD rdx : radices)
                            {
                                // Parse:
                                gtString& varValue = (10 == rdx) ? o_varValue : o_varValueHex;
                                IDebugExpression2* piExpr = nullptr;
                                BSTR errorText = nullptr;
                                UINT errorIndex = S_OK;
                                hr = piExprContext->ParseText(varNameWCStr, parsFlg, rdx, &piExpr, &errorText, &errorIndex);

                                if (SUCCEEDED(hr) && (nullptr != piExpr))
                                {
                                    // Evaluate:
                                    IDebugProperty2* piProp = nullptr;
                                    hr = piExpr->EvaluateSync(evalFlg, VSD_EXPRESSION_EVAL_TIMEOUT_MS, nullptr, &piProp);

                                    if (SUCCEEDED(hr) && (nullptr != piProp))
                                    {
                                        // Get the value:
                                        hr = piProp->GetPropertyInfo(DEBUGPROP_INFO_VALUE | DEBUGPROP_INFO_TYPE, rdx, VSD_EXPRESSION_EVAL_TIMEOUT_MS, nullptr, 0, &propInfo);
                                        GT_IF_WITH_ASSERT(SUCCEEDED(hr))
                                        {
                                            varValue = propInfo.bstrValue;
                                            SysFreeString(propInfo.bstrValue);
                                            propInfo.bstrValue = nullptr;

                                            if (o_varType.isEmpty())
                                            {
                                                o_varType = propInfo.bstrType;
                                            }

                                            SysFreeString(propInfo.bstrType);
                                            propInfo.bstrType = nullptr;
                                            retVal = true;
                                        }

                                        // Release the property:
                                        piProp->Release();
                                        piProp = nullptr;
                                    }

                                    // Release the expression:
                                    piExpr->Release();
                                    piExpr = nullptr;
                                }
                                else if (nullptr != errorText)
                                {
                                    varValue = errorText;
                                    retVal = true;

                                    // TO_DO: add logging.
                                }

                                SysFreeString(errorText);
                                errorText = nullptr;

                                if (!evalHex) { break; }
                            }

                            // Release the context:
                            piExprContext->Release();
                        }
                    }

#ifdef VSD_RELEASE_ENUMERATED_INTERFACES
                    // Release the interface:
                    currentFrameInfo.m_pFrame->Release();
                    currentFrameInfo.m_pFrame = nullptr;
#endif
                }

                // Get the next frame:
                hr = piFramesInfo->Next(1, &currentFrameInfo, &readFrames);
                ++currentFrameIndex;
            }

            piFramesInfo->Release();
        }
    }

    // If we didn't get the value for one of the radices, copy from the other one:
    if (retVal)
    {
        if (o_varValueHex.isEmpty())
        {
            o_varValueHex = o_varValue;
        }
        else if (o_varValue.isEmpty())
        {
            o_varValue = o_varValueHex;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::performStep
// Description: Performs a step of the selected kind:
// Author:      Uri Shomroni
// Date:        12/1/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::vsdCDebugThread::performStep(IDebugProcess3& riProcess3, IDebugProgram2& riProgram, StepType stepType)
{
    STEPKIND sk = STEP_INTO;

    switch (stepType)
    {
        case PD_STEP_IN:    sk = STEP_INTO;     break;

        case PD_STEP_OVER:  sk = STEP_OVER;     break;

        case PD_STEP_OUT:   sk = STEP_OUT;      break;

        default:            GT_ASSERT(false);   break;
    }

    /*    DWORD tid = threadId();
        IEnumDebugThreads2* pithds = nullptr;
        HRESULT h = riProcess3.EnumThreads(&pithds);
        if (S_OK == h)
        {
            IDebugThread2* pthd = nullptr;
            ULONG u = 0;
            h = pithds->Next(1, &pthd, &u);
            while (S_OK == h)
            {
                DWORD dwt = 0;
                pthd->GetThreadId(&dwt);
                if (dwt == tid)
                {
                    m_piDebugThread->Release();
                    m_piDebugThread = pthd;
                    pthd->AddRef();
                }
                pthd->Release();
                h = pithds->Next(1, &pthd, &u);
            }
            pithds->Release();
        }*/

    // S_FALSE is the return value if the thread object is not managed by the process object
    HRESULT hr = riProcess3.Step(m_piDebugThread, sk, STEP_LINE);

    if (!SUCCEEDED(hr) || (S_FALSE == hr))
    {
        hr = riProgram.Step(m_piDebugThread, sk, STEP_LINE);
    }

    bool retVal = SUCCEEDED(hr);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::executeFunction
// Description: Makes a thread execute a function
// Author:      Uri Shomroni
// Date:        7/4/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::vsdCDebugThread::executeFunction(osProcedureAddress64 funcAddress, bool& waitingForExecutedFunctionFlag)
{
    bool retVal = false;

    // If the thread is running, we cannot change its context:
    GT_IF_WITH_ASSERT(m_isSuspended && m_canSuspendThread)
    {
        FunctionExecutionMode execMode = vsdProcessDebugger::vsInstance().functionExecutionMode();

        if (PD_EXECUTION_IN_BREAK_MODE == execMode)
        {
            // Execute in break mode does not require any changes to the thread - just release it until the stub function ends.
            retVal = resumeThreadForExecution(waitingForExecutedFunctionFlag);
        }
        else if (PD_DIRECT_EXECUTION_MODE == execMode)
        {
            // Get a handle to the thread:
            if (nullptr == m_hThread)
            {
                m_hThread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, threadId());
            }

            GT_IF_WITH_ASSERT(nullptr != m_hThread)
            {
                // Get the thread's execution context:
                CONTEXT origCtx = { 0 };
                origCtx.ContextFlags = CONTEXT_FULL;
                BOOL rcCtx = ::GetThreadContext(m_hThread, &origCtx);
                GT_IF_WITH_ASSERT(FALSE != rcCtx)
                {
                    // Copy the stored thread context:
                    CONTEXT modifiedCtx = { 0 };
                    ::memcpy(&modifiedCtx, &origCtx, sizeof(CONTEXT));

                    // Move the thread instruction pointer to the input function address:
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
                    modifiedCtx.Eip = (DWORD)funcAddress;
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
                    modifiedCtx.Rip = (DWORD64)funcAddress;
#else
#error Unknown address space size!
#endif // AMDT_ADDRESS_SPACE_TYPE

                    // Set this to be the current thread execution context:
                    rcCtx = ::SetThreadContext(m_hThread, &modifiedCtx);
                    GT_IF_WITH_ASSERT(FALSE != rcCtx)
                    {
                        retVal = resumeThreadForExecution(waitingForExecutedFunctionFlag);
                    }

                    // Restore the original context:
                    rcCtx = ::SetThreadContext(m_hThread, &origCtx);
                    GT_ASSERT(FALSE != rcCtx);
                }
            }
        }
        else
        {
            // Unknown Execution Mode!
            GT_ASSERT(false);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::markThreadAsSuspendedProcessEntryPoint
// Description: Marks the thread as suspended by the process entry point.
//              This happens only once, only when debugging 64-bit apps,
//              and should not be called otherwise
// Author:      Uri Shomroni
// Date:        4/3/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::vsdCDebugThread::markThreadAsSuspendedProcessEntryPoint()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_canSuspendThread)
    {
        GT_IF_WITH_ASSERT(!m_isSuspended)
        {
            // The thread is suspended by launching with the debugger:
            m_isSuspended = true;
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::resumeThreadForExecution
// Description: Internal function called by executeFunction
// Author:      Uri Shomroni
// Date:        7/4/2016
// ---------------------------------------------------------------------------
bool vsdProcessDebugger::vsdCDebugThread::resumeThreadForExecution(bool& waitingForExecutedFunctionFlag)
{
    bool retVal = false;

    // Resume the thread run:
    waitingForExecutedFunctionFlag = true;
    bool rcRes = resumeThread();
    GT_IF_WITH_ASSERT(rcRes)
    {
        // Wait for the function execution to end:
        retVal = osWaitForFlagToTurnOff(waitingForExecutedFunctionFlag, VSD_MAKE_THREAD_EXECUTE_TIMEOUT_MS);

        // Release the flag:
        waitingForExecutedFunctionFlag = false;

        // Restore the thread's suspension state:
        bool rcSus = suspendThread();
        GT_ASSERT(rcSus);

        if (retVal)
        {
            // Resume / release the exception event:
            bool rcExc = vsdProcessDebugger::vsInstance().internalResumeProcess();
            GT_ASSERT(rcExc);
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugThread::detectThreadEntryPoint
// Description: Sets the value of m_threadStartAddress and m_isDriverThread members
// Author:      Uri Shomroni
// Date:        6/3/2016
// ---------------------------------------------------------------------------
void vsdProcessDebugger::vsdCDebugThread::detectThreadEntryPoint()
{
    vsdProcessDebugger& theVSDProcessDebugger = vsdProcessDebugger::vsInstance();

    // Sanity check:
    m_threadStartAddress = (osInstructionPointer)0;
    GT_IF_WITH_ASSERT(nullptr != m_piDebugThread)
    {
        // Get the thread's call stack:
        IEnumDebugFrameInfo2* piCallStack = NULL;
        HRESULT hr = m_piDebugThread->EnumFrameInfo(FIF_FRAME, 16, &piCallStack);

        if (SUCCEEDED(hr) && (piCallStack != NULL))
        {
            // Get the top frame (should be the generic CreateThread entry point at this time):
            FRAMEINFO topFrame = { 0 };
            ULONG numReturned = 0;
            hr = piCallStack->Next(1, &topFrame, &numReturned);

            if (SUCCEEDED(hr) && (S_FALSE != hr) && (numReturned == 1))
            {
                IDebugStackFrame2* piTopFrame = topFrame.m_pFrame;

                if (piTopFrame != NULL)
                {
                    // Look for the *AX register value, which is the thread entry point (which will be called)
                    static const gtString cpuRegGroup = L"CPU";
                    static const gtString eaxRegisterName = L"EAX";
                    static const gtString raxRegisterName = L"RAX";

                    bool is64 = false;
                    bool rc64 = theVSDProcessDebugger.isDebugging64BitApplication(is64);
                    GT_ASSERT(rc64);

                    const gtString& entryPointReg = is64 ? raxRegisterName : eaxRegisterName;

                    ULONG groupCount = 0;
                    IEnumDebugPropertyInfo2* piGrpProperties = nullptr;

                    // Would want this to be DEBUGPROP_INFO_FULLNAME | DEBUGPROP_INFO_VALUE, but in fact:
                    // "Seems to be 'undocumented' again...
                    //  Calling IDebugStackFrame2::EnumProperties with a refiid of guidFilterRegisters returns
                    //  an IEnumDebugPropertyInfo2 object which contains the enum of properties(IDebugProperty2)
                    //  for different register types.Each one of them represents a register type(like CPU, CPU segment, mmx)
                    //  and has children represent the registers.Calling IDebugProperty2::EnumChildren to get another enum for the registers."
                    // (Source: www[dot]cnblogs[dot]com[slash]atempcode[slash]archive[slash]2004[slash]07[slash]21[slash]26323[dot]html
                    hr = piTopFrame->EnumProperties(DEBUGPROP_INFO_NAME | DEBUGPROP_INFO_PROP, 16, guidFilterRegisters, VSD_EXPRESSION_EVAL_TIMEOUT_MS, &groupCount, &piGrpProperties);

                    if (SUCCEEDED(hr) && (nullptr != piGrpProperties))
                    {
                        // Find the CPU register group:
                        if (0 < groupCount)
                        {
                            DEBUG_PROPERTY_INFO grpProp = { 0 };
                            ULONG gotGroup = 0;
                            hr = piGrpProperties->Next(1, &grpProp, &gotGroup);
                            bool goOn = true;

                            while (goOn && SUCCEEDED(hr) && (S_FALSE != hr) && (0 < gotGroup))
                            {
                                // If this is the CPU registers group:
                                if (cpuRegGroup == grpProp.bstrName)
                                {
                                    // Get its children, the CPU registers:
                                    DBG_ATTRIB_FLAGS flg = DBG_ATTRIB_NONE;
                                    IEnumDebugPropertyInfo2* piRegProperties = nullptr;
                                    hr = grpProp.pProperty->EnumChildren(DEBUGPROP_INFO_NAME | DEBUGPROP_INFO_VALUE, 16, guidFilterRegisters, flg, nullptr, VSD_EXPRESSION_EVAL_TIMEOUT_MS, &piRegProperties);

                                    if (SUCCEEDED(hr) && (nullptr != piRegProperties))
                                    {
                                        // Find the *AX register:
                                        DEBUG_PROPERTY_INFO regProp = { 0 };
                                        ULONG gotReg = 0;
                                        hr = piRegProperties->Next(1, &regProp, &gotReg);

                                        while (goOn && SUCCEEDED(hr) && (S_FALSE != hr) && (0 < gotReg))
                                        {
                                            // If this is the correct register:
                                            if (entryPointReg == regProp.bstrName)
                                            {
                                                // Register values are given as hexadecimal numbers, but without the "ox" prefix:
                                                gtString strRegVal = regProp.bstrValue;
                                                strRegVal.prepend(L"0x");
                                                unsigned long long ullRegVal = 0;

                                                if (strRegVal.toUnsignedLongLongNumber(ullRegVal))
                                                {
                                                    m_threadStartAddress = (osInstructionPointer)ullRegVal;
                                                }

                                                goOn = false;
                                            }

                                            // Release strings:
                                            // SysFreeString(regProp.bstrName);
                                            regProp.bstrName = nullptr;
                                            // SysFreeString(regProp.bstrValue);
                                            regProp.bstrValue = nullptr;

                                            // If we go on:
                                            if (goOn)
                                            {
                                                // Get the next register:
                                                hr = piRegProperties->Next(1, &regProp, &gotReg);
                                            }
                                        }
                                    }

                                    // We found the group, stop looking:
                                    goOn = false;
                                }

                                // Release properties:
                                // SysFreeString(grpProp.bstrName);
                                grpProp.bstrFullName = nullptr;

                                if (nullptr != grpProp.pProperty)
                                {
#ifdef VSD_RELEASE_ENUMERATED_INTERFACES
                                    grpProp.pProperty->Release();
                                    grpProp.pProperty = nullptr;
#endif
                                }

                                // If we go on:
                                if (goOn)
                                {
                                    // Get the next group:
                                    hr = piGrpProperties->Next(1, &grpProp, &gotGroup);
                                }
                            }
                        }

                        piGrpProperties->Release();
                    }

                    // If we couldn't get the address from %EAX / %RAX, at least set the actual PC (of ::ThreadCreate[Ex])
                    if ((osInstructionPointer)0 == m_threadStartAddress)
                    {
                        // Get the code context:
                        IDebugCodeContext2* piTopFrameCodeContext = NULL;
                        hr = piTopFrame->GetCodeContext(&piTopFrameCodeContext);

                        if (SUCCEEDED(hr) && (piTopFrameCodeContext != NULL))
                        {
                            // Get the address:
                            CONTEXT_INFO codeContextInfo = { 0 };
                            hr = piTopFrameCodeContext->GetInfo(CIF_ADDRESS, &codeContextInfo);

                            if (SUCCEEDED(hr) && (codeContextInfo.bstrAddress != NULL))
                            {
                                // Translate to a number:
                                unsigned long long addressAsULongLong = 0;
                                gtString addressAsString = codeContextInfo.bstrAddress;

                                if (addressAsString.toUnsignedLongLongNumber(addressAsULongLong))
                                {
                                    m_threadStartAddress = (osInstructionPointer)addressAsULongLong;
                                }

                                // Release the string:
                                SysFreeString(codeContextInfo.bstrAddress);
                            }

                            // Release the code context:
                            piTopFrameCodeContext->Release();
                        }
                    }

                    // Release the frame interface:
#ifdef VSD_RELEASE_ENUMERATED_INTERFACES
                    // Release the interface:
                    topFrame.m_pFrame->Release();
                    topFrame.m_pFrame = nullptr;
#endif
                }
            }

            // Release the call stack:
            piCallStack->Release();
        }
    }

    // If the thread is starting at a driver address, we consider it a driver thread:
    m_isDriverThread = theVSDProcessDebugger.IsDriverAddress(m_threadStartAddress);

    // Do not allow suspending driver threads:
    if (m_isDriverThread)
    {
        setCanSuspendThread(false);
    }
}

//////////////////////////////////////////////////////////////////////////
// vsdCDebugBreakpointRequest
//////////////////////////////////////////////////////////////////////////
// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugBreakpointRequest::vsdCDebugBreakpointRequest
// Description: Constructor
// Author:      Uri Shomroni
// Date:        12/1/2016
// ---------------------------------------------------------------------------
vsdProcessDebugger::vsdCDebugBreakpointRequest::vsdCDebugBreakpointRequest(IDebugProgram2& riProgram, const osFilePath& srcPath, int lineNum)
    : m_sourcePath(srcPath), m_lineNumber(lineNum)
{
    ::memset(&m_bpRequest, 0, sizeof(BP_REQUEST_INFO));

    m_bpRequest.dwFields = BPREQI_BPLOCATION | BPREQI_LANGUAGE | BPREQI_PROGRAM | BPREQI_PROGRAMNAME /*| BPREQI_THREAD | BPREQI_THREADNAME*/ | BPREQI_PASSCOUNT | BPREQI_CONDITION | BPREQI_FLAGS;
    m_bpRequest.guidLanguage = guidCPPLang;
    m_bpRequest.bpLocation.bpLocationType = BPLT_CODE_FILE_LINE;
    m_bpRequest.bpLocation.bpLocation.bplocCodeFileLine.bstrContext = nullptr;
    m_bpRequest.bpLocation.bpLocation.bplocCodeFileLine.pDocPos = this;
    m_bpRequest.pProgram = &riProgram;
    m_bpRequest.pProgram->AddRef();
    riProgram.GetName(&(m_bpRequest.bstrProgramName));
}
// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugBreakpointRequest::~vsdCDebugBreakpointRequest
// Description: Destructor
// Author:      Uri Shomroni
// Date:        12/1/2016
// ---------------------------------------------------------------------------
vsdProcessDebugger::vsdCDebugBreakpointRequest::~vsdCDebugBreakpointRequest()
{
    if (nullptr != m_bpRequest.pProgram)
    {
        m_bpRequest.pProgram->Release();
        m_bpRequest.pProgram = nullptr;
    }

    if (nullptr != m_bpRequest.bstrProgramName)
    {
        SysFreeString(m_bpRequest.bstrProgramName);
        m_bpRequest.bstrProgramName = nullptr;
    }

    if (nullptr != m_bpRequest.pThread)
    {
        m_bpRequest.pThread->Release();
        m_bpRequest.pThread = nullptr;
    }

    if (nullptr != m_bpRequest.bstrThreadName)
    {
        SysFreeString(m_bpRequest.bstrThreadName);
        m_bpRequest.bstrThreadName = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugBreakpointRequest::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        12/1/2016
// ---------------------------------------------------------------------------
ULONG vsdProcessDebugger::vsdCDebugBreakpointRequest::AddRef(void)
{
    return vsdCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugBreakpointRequest::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        12/1/2016
// ---------------------------------------------------------------------------
ULONG vsdProcessDebugger::vsdCDebugBreakpointRequest::Release(void)
{
    return vsdCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vsdProcessDebugger::vsdCDebugBreakpointRequest::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        12/1/2016
// ---------------------------------------------------------------------------
HRESULT vsdProcessDebugger::vsdCDebugBreakpointRequest::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if (ppvObj == NULL)
    {
        retVal = E_POINTER;
    }
    else if (riid == IID_IUnknown)
    {
        // Since multiple interfaces inherit IUnknown, we need to cast through one of them.
        // Note that we have to cast through the same one each time, to be consistent.
        *ppvObj = (IUnknown*)((IDebugBreakpointRequest2*)this);
        AddRef();
    }
    else if (riid == IID_IDebugBreakpointRequest2)
    {
        *ppvObj = (IDebugBreakpointRequest2*)this;
        AddRef();
    }
    else if (riid == IID_IDebugDocumentPosition2)
    {
        *ppvObj = (IDebugDocumentPosition2*)this;
        AddRef();
    }
    else
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// IDebugBreakpointRequest2 methods
HRESULT vsdProcessDebugger::vsdCDebugBreakpointRequest::GetLocationType(BP_LOCATION_TYPE* pBPLocationType)
{
    HRESULT retVal = E_FAIL;

    if (nullptr != pBPLocationType)
    {
        *pBPLocationType = m_bpRequest.bpLocation.bpLocationType;
        retVal = S_OK;
    }
    else
    {
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vsdProcessDebugger::vsdCDebugBreakpointRequest::GetRequestInfo(BPREQI_FIELDS dwFields, BP_REQUEST_INFO* pBPRequestInfo)
{
    HRESULT retVal = E_FAIL;

    if (nullptr != pBPRequestInfo)
    {
        // Copy output fields:
        BPREQI_FIELDS dwAvailFields = (dwFields & m_bpRequest.dwFields);

        // Location:
        if (0 != (BPREQI_BPLOCATION & dwAvailFields))
        {
            switch (m_bpRequest.bpLocation.bpLocationType)
            {
                case BPLT_CODE_FILE_LINE:
                {
                    BSTR context = m_bpRequest.bpLocation.bpLocation.bplocCodeFileLine.bstrContext;
                    pBPRequestInfo->bpLocation.bpLocation.bplocCodeFileLine.bstrContext = (nullptr != context) ? SysAllocString(context) : nullptr;
                    pBPRequestInfo->bpLocation.bpLocation.bplocCodeFileLine.pDocPos = m_bpRequest.bpLocation.bpLocation.bplocCodeFileLine.pDocPos;

                    if (nullptr != pBPRequestInfo->bpLocation.bpLocation.bplocCodeFileLine.pDocPos)
                    {
                        pBPRequestInfo->bpLocation.bpLocation.bplocCodeFileLine.pDocPos->AddRef();
                    }
                }
                break;

                default:
                    dwAvailFields &= (~BPREQI_BPLOCATION);
                    GT_ASSERT(false);
                    break;
            }
        }

        // Program:
        if (0 != (BPREQI_PROGRAM & dwAvailFields))
        {
            pBPRequestInfo->pProgram = m_bpRequest.pProgram;

            if (nullptr != pBPRequestInfo->pProgram)
            {
                pBPRequestInfo->pProgram->AddRef();
            }
        }

        // Program name:
        if (0 != (BPREQI_PROGRAMNAME & dwAvailFields))
        {
            BSTR progNm = m_bpRequest.bstrProgramName;
            pBPRequestInfo->bstrProgramName = (nullptr != progNm) ? SysAllocString(progNm) : nullptr;
        }

        // Thread:
        if (0 != (BPREQI_THREAD & dwAvailFields))
        {
            pBPRequestInfo->pThread = m_bpRequest.pThread;

            if (nullptr != pBPRequestInfo->pThread)
            {
                pBPRequestInfo->pThread->AddRef();
            }
        }

        // Thread name:
        if (0 != (BPREQI_THREADNAME & dwAvailFields))
        {
            BSTR thdNm = m_bpRequest.bstrThreadName;
            pBPRequestInfo->bstrThreadName = (nullptr != thdNm) ? SysAllocString(thdNm) : nullptr;
        }

        // Language:
        if (0 != (BPREQI_LANGUAGE & dwAvailFields))
        {
            pBPRequestInfo->guidLanguage = m_bpRequest.guidLanguage;
        }

        // The following two are expected to be all 0. If we implement them, this should be modified:
        // Pass count:
        if (0 != (BPREQI_PASSCOUNT & dwAvailFields))
        {
            ::memcpy(&(pBPRequestInfo->bpPassCount), &(m_bpRequest.bpPassCount), sizeof(BP_PASSCOUNT));
            dwAvailFields &= (~BPREQI_PASSCOUNT);
        }

        // Condition:
        if (0 != (BPREQI_CONDITION & dwAvailFields))
        {
            ::memcpy(&(pBPRequestInfo->bpCondition), &(m_bpRequest.bpCondition), sizeof(BP_CONDITION));
            dwAvailFields &= (~BPREQI_CONDITION);
        }

        // Flags:
        if (0 != (BPREQI_FLAGS & dwAvailFields))
        {
            pBPRequestInfo->dwFlags = m_bpRequest.dwFlags;
        }

        // Copy the valid fields:
        pBPRequestInfo->dwFields = dwAvailFields;
        retVal = S_OK;
    }
    else
    {
        retVal = E_POINTER;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// IDebugDocumentPosition2 methods
HRESULT vsdProcessDebugger::vsdCDebugBreakpointRequest::GetFileName(BSTR* pbstrFileName)
{
    HRESULT retVal = E_FAIL;

    if (nullptr != pbstrFileName)
    {
        *pbstrFileName = SysAllocString(m_sourcePath.asString().asCharArray());
        retVal = (nullptr != *pbstrFileName) ? S_OK : E_OUTOFMEMORY;
    }
    else
    {
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vsdProcessDebugger::vsdCDebugBreakpointRequest::GetDocument(IDebugDocument2** ppDoc)
{
    GT_UNREFERENCED_PARAMETER(ppDoc);
    return E_NOTIMPL;
}
HRESULT vsdProcessDebugger::vsdCDebugBreakpointRequest::IsPositionInDocument(IDebugDocument2* pDoc)
{
    HRESULT retVal = E_FAIL;

    if (nullptr != pDoc)
    {
        retVal = S_FALSE;

        BSTR docPathStr = nullptr;
        HRESULT hr = pDoc->GetName(GN_URL, &docPathStr);

        if (SUCCEEDED(hr) && (nullptr != docPathStr))
        {
            osFilePath docPath(docPathStr);

            if (docPath == m_sourcePath)
            {
                retVal = S_OK;
            }

            SysFreeString(docPathStr);
        }
    }
    else
    {
        retVal = E_INVALIDARG;
    }

    return retVal;
}
HRESULT vsdProcessDebugger::vsdCDebugBreakpointRequest::GetRange(TEXT_POSITION* pBegPosition, TEXT_POSITION* pEndPosition)
{
    // Breakpoint requests are expected to be 1 line back and 8 lines forward (total of 10 line range)
#define VSD_BREAKPOINT_MAX_LOOKBACK 1
#define VSD_BREAKPOINT_MAX_LOOKAHEAD 8
    HRESULT retVal = S_OK;

    if (nullptr != pBegPosition)
    {
        pBegPosition->dwLine = m_lineNumber - (VSD_BREAKPOINT_MAX_LOOKBACK);
        pBegPosition->dwColumn = 0;
    }

    if (nullptr != pEndPosition)
    {
        pEndPosition->dwLine = m_lineNumber + VSD_BREAKPOINT_MAX_LOOKAHEAD;
        pEndPosition->dwColumn = 0xFFFFFFFF;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////
// vsdHostBreakpoint
//////////////////////////////////////////////////////////////////////////
vsdProcessDebugger::vsdHostBreakpoint::vsdHostBreakpoint(IDebugPendingBreakpoint2& riPendingBreakpoint, const osFilePath& requestedPath, int requestedLineNumber)
    : m_piPendingBreakpoint(&riPendingBreakpoint), m_requestedPath(requestedPath), m_requestedLineNumber(requestedLineNumber)
{
    m_piPendingBreakpoint->AddRef();
}
vsdProcessDebugger::vsdHostBreakpoint::~vsdHostBreakpoint()
{
    bool rcRem = removeRealBreakpoint();
    GT_ASSERT(rcRem);
}
bool vsdProcessDebugger::vsdHostBreakpoint::removeRealBreakpoint()
{
    bool retVal = (nullptr == m_piPendingBreakpoint);

    if (nullptr != m_piPendingBreakpoint)
    {
        IEnumDebugBoundBreakpoints2* piEnumBound = nullptr;
        HRESULT hr = m_piPendingBreakpoint->EnumBoundBreakpoints(&piEnumBound);

        if (SUCCEEDED(hr) && (nullptr != piEnumBound))
        {
            ULONG readBreakpoints = 0;
            IDebugBoundBreakpoint2* piBoundBP = nullptr;
            hr = piEnumBound->Next(1, &piBoundBP, &readBreakpoints);

            while (SUCCEEDED(hr) && (S_FALSE != hr) && (nullptr != piBoundBP))
            {
                hr = piBoundBP->Delete();
                GT_ASSERT(SUCCEEDED(hr));

                piBoundBP->Release();
                piBoundBP = nullptr;
                hr = piEnumBound->Next(1, &piBoundBP, &readBreakpoints);
            }
        }

        hr = m_piPendingBreakpoint->Delete();
        GT_ASSERT(SUCCEEDED(hr));
        retVal = SUCCEEDED(hr);

        m_piPendingBreakpoint->Release();
        m_piPendingBreakpoint = nullptr;
    }

    return retVal;
}
