//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdDebugEventCallback.cpp
///
//==================================================================================

//------------------------------ vsdDebugEventCallback.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessOutputStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunStartedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apModuleLoadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleUnloadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadTerminatedEvent.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>

// Local:
#include <src/vsdDebugEventCallback.h>
#include <src/vsdProcessDebugger.h>

// Helper function
static void vsdGUIDAsString(const GUID& guid, gtString& o_str)
{
    o_str.appendFormattedString(L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
#define IF_IID_ADD_TO_STRING(iid) if (iid == guid) o_str.append('(').append(L#iid).append(')')
    // Debug Events:
    IF_IID_ADD_TO_STRING(IID_IDebugEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugSessionCreateEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugSessionDestroyEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugEngineCreateEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugProcessCreateEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugProcessDestroyEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugProgramCreateEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugProgramDestroyEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugThreadCreateEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugThreadDestroyEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugLoadCompleteEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugEntryPointEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugStepCompleteEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugCanStopEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugBreakEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugBreakpointEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugExceptionEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugOutputStringEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugModuleLoadEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugSymbolSearchEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugBeforeSymbolSearchEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugPropertyCreateEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugPropertyDestroyEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugBreakpointBoundEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugBreakpointUnboundEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugBreakpointErrorEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugExpressionEvaluationCompleteEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugReturnValueEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugNoSymbolsEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugProgramNameChangedEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugThreadNameChangedEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugMessageEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugErrorEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugActivateDocumentEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugInterceptExceptionCompleteEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugAttachCompleteEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugFuncEvalAbortedEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugStopCompleteEvent2); else
        IF_IID_ADD_TO_STRING(IID_IDebugSessionEvent2);
#undef IF_IID_ADD_TO_STRING
}


// ---------------------------------------------------------------------------
// Name:        vsdCDebugEventCallback::vsdCDebugEventCallback
// Description: Constructor
// Author:      Uri Shomroni
// Date:        26/12/2011
// ---------------------------------------------------------------------------
vsdCDebugEventCallback::vsdCDebugEventCallback()
{

}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugEventCallback::~vsdCDebugEventCallback
// Description: Destructor
// Author:      Uri Shomroni
// Date:        26/12/2011
// ---------------------------------------------------------------------------
vsdCDebugEventCallback::~vsdCDebugEventCallback()
{

}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugEventCallback::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        26/12/2011
// ---------------------------------------------------------------------------
ULONG vsdCDebugEventCallback::AddRef(void)
{
    return vsdCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugEventCallback::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        26/12/2010
// ---------------------------------------------------------------------------
ULONG vsdCDebugEventCallback::Release(void)
{
    return vsdCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugEventCallback::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        26/12/2011
// ---------------------------------------------------------------------------
HRESULT vsdCDebugEventCallback::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if (ppvObj == NULL)
    {
        retVal = E_POINTER;
    }
    else if (riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*)this;
        AddRef();
    }
    else if (riid == IID_IDebugEventCallback2)
    {
        *ppvObj = (IDebugEventCallback2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IDebugEventCallback2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// IDebugEventCallback2 methods
HRESULT vsdCDebugEventCallback::Event(IDebugEngine2* pEngine, IDebugProcess2* pProcess, IDebugProgram2* pProgram, IDebugThread2* pThread, IDebugEvent2* pEvent, REFIID riidEvent, DWORD dwAttrib)
{
    osCriticalSectionLocker eventCSLocker(m_eventCallbackCS);

    HRESULT retVal = S_OK;

    GT_UNREFERENCED_PARAMETER(pProcess);
    GT_UNREFERENCED_PARAMETER(pEngine);

    if (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity())
    {
        // Construct a log printout:
        gtString logMsg;
        gtString guidAsString;
        vsdGUIDAsString(riidEvent, guidAsString);
        logMsg.appendFormattedString(L"vsdCDebugEventCallback - Got debug event. Program: %p, Thread: %p, Event: %p, Event type: %ls, Attrib: %#x", pProgram, pThread, pEvent, guidAsString.asCharArray(), dwAttrib);
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
    }

    bool isSyncEvent = ((dwAttrib & EVENT_SYNCHRONOUS) != 0);
    bool continueEvent = isSyncEvent;
    // bool isStoppingEvent = ((dwAttrib & EVENT_STOPPING) != 0);
    apEventsHandler& theEventsHandler = apEventsHandler::instance();
    osTime now;
    now.setFromCurrentTime();
    pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();
    vsdProcessDebugger& theVSDProcessDebugger = vsdProcessDebugger::vsInstance();

    if (riidEvent == IID_IDebugEngineCreateEvent2)
    {
        // Should we verify the engine used is the native engine?
    }
    else if (riidEvent == IID_IDebugProgramCreateEvent2)
    {
        theVSDProcessDebugger.handleProgramCreated(pProgram);

        // Send a process created event:
        const apDebugProjectSettings* pProcessCreationData = theProcessDebugger.debuggedProcessCreationData();
        GT_IF_WITH_ASSERT(pProcessCreationData != NULL)
        {
            apDebuggedProcessCreatedEvent processCreatedEvent(*pProcessCreationData, now, 0);
            theEventsHandler.registerPendingDebugEvent(processCreatedEvent);
        }
    }
    else if (riidEvent == IID_IDebugProgramDestroyEvent2)
    {
        long processExitCode = 0;

        IDebugProgramDestroyEvent2* pProgramDestroyEvent = NULL;
        HRESULT hr = pEvent->QueryInterface(riidEvent, (void**)&pProgramDestroyEvent);

        if (SUCCEEDED(hr) && (NULL != pProgramDestroyEvent))
        {
            DWORD dwExitCode = 0;
            hr = pProgramDestroyEvent->GetExitCode(&dwExitCode);

            if (SUCCEEDED(hr))
            {
                processExitCode = (long)dwExitCode;
            }

            pProgramDestroyEvent->Release();
        }

        apDebuggedProcessTerminatedEvent processRunTerminatedEvent(processExitCode);
        theEventsHandler.registerPendingDebugEvent(processRunTerminatedEvent);

        theVSDProcessDebugger.handleProgramDestroyed(pProgram);
    }
    else if (riidEvent == IID_IDebugThreadCreateEvent2)
    {
        // Tell the process debugger a thread was created:
        theVSDProcessDebugger.addDebugThread(pThread);

        osThreadId createdThreadId = OS_NO_THREAD_ID;
        osInstructionPointer createdThreadStartAddress = (osInstructionPointer)0;
        GT_IF_WITH_ASSERT(pThread != NULL)
        {
            DWORD dwTid = 0;
            HRESULT hr = pThread->GetThreadId(&dwTid);

            if (SUCCEEDED(hr))
            {
                createdThreadId = (osThreadId)dwTid;
            }
        }

        bool rcAddr = theVSDProcessDebugger.getDebugThreadStartAddress(createdThreadId, createdThreadStartAddress);
        GT_ASSERT(rcAddr);

        apThreadCreatedEvent threadCreatedEvent(createdThreadId, createdThreadId, now, createdThreadStartAddress);
        theEventsHandler.registerPendingDebugEvent(threadCreatedEvent);
    }
    else if (riidEvent == IID_IDebugThreadDestroyEvent2)
    {
        // Notify the process debugger that a thread was destroyed:
        theVSDProcessDebugger.removeDebugThread(pThread);

        // Get the thread ID:
        osThreadId destroyedThreadId = OS_NO_THREAD_ID;
        GT_IF_WITH_ASSERT(NULL != pThread)
        {
            DWORD dwTid = 0;
            HRESULT hr = pThread->GetThreadId(&dwTid);

            if (SUCCEEDED(hr))
            {
                destroyedThreadId = (osThreadId)dwTid;
            }
        }

        long threadExitCode = 0L;
        IDebugThreadDestroyEvent2* pThreadDestroyEvent = NULL;
        HRESULT hr = pEvent->QueryInterface(riidEvent, (void**)&pThreadDestroyEvent);

        if (SUCCEEDED(hr) && (NULL != pThreadDestroyEvent))
        {
            DWORD dwExitCode = 0;
            hr = pThreadDestroyEvent->GetExitCode(&dwExitCode);

            if (SUCCEEDED(hr))
            {
                threadExitCode = (long)dwExitCode;
            }

            pThreadDestroyEvent->Release();
        }

        apThreadTerminatedEvent threadTerminatedEvent(destroyedThreadId, threadExitCode, now);
        theEventsHandler.registerPendingDebugEvent(threadTerminatedEvent);
    }
    else if (riidEvent == IID_IDebugModuleLoadEvent2)
    {
        IDebugModuleLoadEvent2* pModuleLoadEvent = NULL;
        HRESULT hr = pEvent->QueryInterface(riidEvent, (void**)&pModuleLoadEvent);

        if (SUCCEEDED(hr) && (NULL != pModuleLoadEvent))
        {
            IDebugModule2* piModule = nullptr;
            BOOL wasLoaded = FALSE;
            hr = pModuleLoadEvent->GetModule(&piModule, nullptr, &wasLoaded);
            GT_IF_WITH_ASSERT(SUCCEEDED(hr) && (nullptr != piModule))
            {
                MODULE_INFO moduleInfo = { 0 };
                hr = piModule->GetInfo(MIF_URL | MIF_LOADADDRESS, &moduleInfo);
                GT_IF_WITH_ASSERT(SUCCEEDED(hr))
                {
                    DWORD dwTid = 0;

                    if (nullptr != pThread)
                    {
                        hr = pThread->GetThreadId(&dwTid);
                        // Still send an event even if we didn't get the thread Id:
                        GT_ASSERT(SUCCEEDED(hr));
                    }

                    gtString modulePath = moduleInfo.m_bstrUrl;

                    if (TRUE == wasLoaded)
                    {
                        apModuleLoadedEvent loadEve((osThreadId)dwTid, modulePath, (osInstructionPointer)moduleInfo.m_addrLoadAddress);
                        theEventsHandler.registerPendingDebugEvent(loadEve);
                    }
                    else // (FALSE == wasLoaded)
                    {
                        apModuleUnloadedEvent unloadEve((osThreadId)dwTid, modulePath);
                        theEventsHandler.registerPendingDebugEvent(unloadEve);
                    }

                    // SysFreeString(moduleInfo.m_bstrUrl);
                }

                piModule->Release();
            }
        }
    }
    else if (riidEvent == IID_IDebugLoadCompleteEvent2)
    {
    }
    else if (riidEvent == IID_IDebugEntryPointEvent2)
    {
        theVSDProcessDebugger.handleEntryPointEvent(pThread);

        apDebuggedProcessRunStartedEvent processRunStartedEvent(theProcessDebugger.debuggedProcessId(), now);
        theEventsHandler.registerPendingDebugEvent(processRunStartedEvent);
    }
    else if (riidEvent == IID_IDebugOutputStringEvent2)
    {
        // Get the output string event interface:
        IDebugOutputStringEvent2* piOutputStringEvent = NULL;
        HRESULT hr = pEvent->QueryInterface(IID_IDebugOutputStringEvent2, (void**)(&piOutputStringEvent));

        if (SUCCEEDED(hr) && (piOutputStringEvent != NULL))
        {
            // Get the string:
            BSTR outputString = NULL;
            hr = piOutputStringEvent->GetString(&outputString);

            if (SUCCEEDED(hr) && (outputString != NULL))
            {
                apDebuggedProcessOutputStringEvent outputStringEvent(outputString);
                theEventsHandler.registerPendingDebugEvent(outputStringEvent);

                // Release the string:
                SysFreeString(outputString);
            }

            // Release the interface:
            piOutputStringEvent->Release();
        }
    }
    else if (riidEvent == IID_IDebugMessageEvent2)
    {
        // Get the debug message event interface:
        IDebugMessageEvent2* piDebugMessageEvent = NULL;
        HRESULT hr = pEvent->QueryInterface(IID_IDebugMessageEvent2, (void**)(&piDebugMessageEvent));

        if (SUCCEEDED(hr) && (piDebugMessageEvent != NULL))
        {
            MESSAGETYPE msgType = 0;
            BSTR msgStr = NULL;
            DWORD msgTyp = 0;
            BSTR msgHelpFile = NULL;
            DWORD msgHelpId = 0;
            hr = piDebugMessageEvent->GetMessageW(&msgType, &msgStr, &msgTyp, &msgHelpFile, &msgHelpId);

            if (SUCCEEDED(hr))
            {
                // TO_DO: handle any special outputs here
            }

            SysFreeString(msgStr);
            SysFreeString(msgHelpFile);

            // Release the interface:
            piDebugMessageEvent->Release();
        }
    }
    else if (riidEvent == IID_IDebugExceptionEvent2)
    {
        IDebugExceptionEvent2* piExceptionEvent = NULL;
        HRESULT hr = pEvent->QueryInterface(IID_IDebugExceptionEvent2, (void**)(&piExceptionEvent));

        if (SUCCEEDED(hr) && (piExceptionEvent != NULL))
        {
            theVSDProcessDebugger.handleExceptionEvent(pEvent, piExceptionEvent, pThread, continueEvent);

            piExceptionEvent->Release();
        }
    }
    else if (IID_IDebugBreakpointEvent2 == riidEvent)
    {
        IDebugBreakpointEvent2* piBreakpointEvent = NULL;
        HRESULT hr = pEvent->QueryInterface(IID_IDebugBreakpointEvent2, (void**)(&piBreakpointEvent));

        if (SUCCEEDED(hr) && (piBreakpointEvent != NULL))
        {
            theVSDProcessDebugger.handleBreakpointEvent(pEvent, piBreakpointEvent, pThread, continueEvent);

            piBreakpointEvent->Release();
        }
    }
    else if (IID_IDebugStepCompleteEvent2 == riidEvent)
    {
        theVSDProcessDebugger.handleStepCompleteEvent(pEvent, pThread, continueEvent);
    }
    else if (IID_IDebugBreakpointBoundEvent2 == riidEvent)
    {

    }
    else if ((riidEvent == IID_IDebugBeforeSymbolSearchEvent2) || (IID_IDebugThreadNameChangedEvent2 == riidEvent) || (IID_IDebugReturnValueEvent2 == riidEvent) || (IID_IDebugNoSymbolsEvent2 == riidEvent))
    {
        // Known and ignored events
    }
    else
    {
        // Unknown event type!
        retVal = E_FAIL;
        OS_OUTPUT_DEBUG_LOG(L"vsdCDebugEventCallback - unhandled event type", OS_DEBUG_LOG_EXTENSIVE);
    }

    if (isSyncEvent && continueEvent)
    {
        OS_OUTPUT_DEBUG_LOG(L"vsdCDebugEventCallback - continuing event", OS_DEBUG_LOG_EXTENSIVE);
        theVSDProcessDebugger.continueFromSynchronousEvent(pEvent);
    }

    OS_OUTPUT_DEBUG_LOG(L"vsdCDebugEventCallback - event handling end", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

void vsdCDebugEventCallback::WaitForEventHandlingCriticalSection()
{
    osCriticalSectionLocker eventCSLocker(m_eventCallbackCS);
}


