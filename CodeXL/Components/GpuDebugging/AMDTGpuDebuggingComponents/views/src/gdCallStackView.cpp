//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdCallStackView.cpp
///
//==================================================================================

//------------------------------ gdCallStackView.cpp ------------------------------

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>


// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apCallStackFrameSelectedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>


// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdCallStackView.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        gdCallStackView::gdCallStackView
// Description: Constructor.
// Arguments:   parent - My parent window.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
gdCallStackView::gdCallStackView(QWidget* pParent):
    gdCallsStackListCtrl(pParent, true)
{
    // enable sending activation event (overwrite parent behavior)
    m_sendActivationEvents = true;

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
}

// ---------------------------------------------------------------------------
// Name:        gdCallStackView::~gdCallStackView
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
gdCallStackView::~gdCallStackView()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        gdCallStackView::onEvent
// Description: Is called when a debugged process event occurs.
//              Send the event to the appropriate event function for displaying
//              it to the user.
// Arguments:   eve - The debugged process event.
// Author:      Yaki Tebeka
// Date:        4/4/2004
// ---------------------------------------------------------------------------
void gdCallStackView::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // Handle the event according to its type:
    switch (eventType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            onProcessCreated();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            onProcessRunSuspended((const apDebuggedProcessRunSuspendedEvent&)eve);
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        {
            onProcessRunResumed();
        }
        break;

        case apEvent::AP_EXCEPTION:
        {
            onException((const apExceptionEvent&)eve);
        }
        break;

        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            // Down cast the event:
            const afGlobalVariableChangedEvent& variableChangedEvent = (const afGlobalVariableChangedEvent&)eve;

            // Handle it:
            onGlobalVariableChanged(variableChangedEvent);
        }
        break;


        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            // Enable the view only in debug mode:
            bool isEnabled = true;

            bool modeChanged = gdDoesModeChangeApplyToDebuggerViews(eve, isEnabled);

            if (modeChanged)
            {
                setEnabled(isEnabled);

                if (!isEnabled)
                {
                    clearList();
                }
            }
        }
        break;


        default:
            // Do nothing...
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdCallStackView::onProcessCreated
// Description: Is called when the debugged process is created.
// Author:      Avi Shapira
// Date:        11/10/2004
// ---------------------------------------------------------------------------
void gdCallStackView::onProcessCreated()
{
    // Delete Items from the list
    deleteListItems();
}


// ---------------------------------------------------------------------------
// Name:        gdCallStackView::onProcessRunSuspended
// Description: Is called when the debugged process is suspended.
//              Enable the listCTRL
//              Calls updateCallsStackList()
// Arguments: event - A class that represents the process suspension event.
// Author:      Avi Shapira
// Date:        11/10/2004
// ---------------------------------------------------------------------------
void gdCallStackView::onProcessRunSuspended(const apDebuggedProcessRunSuspendedEvent& event)
{
    (void)(event);  // unused
    // Update the call stack to the selected thread:
    upadteToSelectedThreadCallStack();
}

// ---------------------------------------------------------------------------
// Name:        gdCallStackView::onProcessRunResumed
// Description: Is called when the debugged process is resumed.
// Author:      Avi Shapira
// Date:        11/10/2004
// ---------------------------------------------------------------------------
void gdCallStackView::onProcessRunResumed()
{
    // Disable the list
    setEnabled(false);
}


// ---------------------------------------------------------------------------
// Name:        gdCallStackView::onException
// Description: Do Nothing
// Arguments: event - A class representing the exception event.
// Author:      Avi Shapira
// Date:        11/10/2004
// ---------------------------------------------------------------------------
void gdCallStackView::onException(const apExceptionEvent& event)
{
    // If the debugged process is about to die:
    bool isSecondChangeException = event.isSecondChance();
    bool isFatalSignal = event.isFatalLinuxSignal();

    if (isSecondChangeException || isFatalSignal)
    {
        // Get the id of the debugged process thread that triggered the event:
        osThreadId threadId = event.triggeringThreadId();

        // Update the call stack list:
        updateCallsStackList(threadId);
    }
    else
    {
        // An exception event is usually followed by a PROCESS_RUN_SUSSPENDED event.
        // This view will be updated on the PROCESS_RUN_SUSSPENDED event.
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallStackView::upadteToSelectedThreadCallStack
// Description: If the chosen thread is a host thread, shows its call stack.
//              If it is a wave front, shows the kernel debugging call stack.
// Author:      Uri Shomroni
// Date:        6/4/2013
// ---------------------------------------------------------------------------
void gdCallStackView::upadteToSelectedThreadCallStack()
{
    bool isInHSAKernelDebugging = gaIsInHSAKernelBreakpoint();

    if (isInHSAKernelDebugging)
    {
        osCallStack hsaStack;
        bool rcStk = gaHSAGetCallStack(hsaStack);
        GT_IF_WITH_ASSERT(rcStk)
        {
            updateCallsStack(hsaStack);
        }
        else
        {
            setEmptyCallStackString(GD_STR_CallsStackWavefrontCallStackError);
            static const osCallStack emptyStack;
            updateCallsStack(emptyStack);
        }
    }
    else
    {
        // Get the chosen thread index:
        gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
        int newThreadIndex = globalVarsManager.chosenThread();
        bool isKernelDebuggingThread = globalVarsManager.isKernelDebuggingThreadChosen();

        if (isKernelDebuggingThread)
        {
            updateKernelCallStackList(newThreadIndex);
        }
        else // !isKernelDebuggingThread
        {
            // Get the thread Id from the thread index:
            osThreadId threadId = OS_NO_THREAD_ID;
            bool rc = gaGetThreadId(newThreadIndex, threadId);

            if (!rc)
            {
                threadId = OS_NO_THREAD_ID;
            }

            // Update the call stack list with the new thread:
            updateCallsStackList(threadId);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdCallStackView::updateCallsStackList
// Description: Update the Calls Stack into the listCTRL
// Author:      Avi Shapira
// Date:        11/10/2004
// ---------------------------------------------------------------------------
void gdCallStackView::updateCallsStackList(osThreadId threadId)
{
    // Enable the list
    setEnabled(true);

    // Get the call stack from the thread id:
    osCallStack callStack;
    QString emptyStackMessage;

    if (threadId != OS_NO_THREAD_ID)
    {
        // Get the debugged process threads
        bool rc = gaGetThreadCallStack(threadId, callStack);
        GT_ASSERT(rc);
    }
    else
    {
        // Clear the view:
        deleteListItems();

        // Add a message to the call stack:
        emptyStackMessage = GD_STR_CallsSelectThreadMessage;
    }

    // Update the view with the call stack:
    setEmptyCallStackString(emptyStackMessage);
    updateCallsStack(callStack);

    // Send an event the a top frame was selected:
    apCallStackFrameSelectedEvent callStackFrameSelectedEvent(0);
    apEventsHandler::instance().registerPendingDebugEvent(callStackFrameSelectedEvent);
}

// ---------------------------------------------------------------------------
// Name:        updateKernelCallStackList::updateCallsStackList
// Description: Update the kernel debugging call stack into the list
// Author:      Uri Shomroni
// Date:        4/4/2013
// ---------------------------------------------------------------------------
void gdCallStackView::updateKernelCallStackList(int wavefrontUserIndex)
{
    // Enable the list
    setEnabled(true);

    // Get the call stack from the thread id:
    osCallStack callStack;
    QString emptyStackMessage;

    if (0 < wavefrontUserIndex)
    {
        // Since the persistent data manager has a higher event handling priority, we can safely assume
        // that the kernel work item selected is in our wavefront:
        bool rc = gaGetCurrentlyDebuggedKernelCallStack(callStack);
        GT_ASSERT(rc);
        emptyStackMessage = GD_STR_CallsStackWavefrontCallStackError;
    }
    else // 0 >= wavefrontUserIndex
    {
        // Clear the view:
        deleteListItems();

        // Add a message to the call stack:
        emptyStackMessage = GD_STR_CallsStackWavefrontInactive;
    }

    // Update the view with the call stack:
    setEmptyCallStackString(emptyStackMessage);
    updateCallsStack(callStack);

    // Send an event the a top frame was selected:
    apCallStackFrameSelectedEvent callStackFrameSelectedEvent(0);
    apEventsHandler::instance().registerPendingDebugEvent(callStackFrameSelectedEvent);
}

// ---------------------------------------------------------------------------
// Name:        gdCallStackView::onGlobalVariableChanged
// Description: Triggered when a global variable value is changed -
//              will update the list according to the thread id
// Arguments:   const gdCodeXLGlobalVariableChangeEvent& stateChangeEvent
// Author:      Avi Shapira
// Date:        11/5/2005
// ---------------------------------------------------------------------------
void gdCallStackView::onGlobalVariableChanged(const afGlobalVariableChangedEvent& variableChangedEvent)
{
    // Get id of the global variable that was changed:
    afGlobalVariableChangedEvent::GlobalVariableId variableId = variableChangedEvent.changedVariableId();

    // If the chosen THREAD was changed:
    if (variableId == afGlobalVariableChangedEvent::CHOSEN_THREAD_INDEX)
    {
        // Update to its call stack:
        upadteToSelectedThreadCallStack();
    }
    else if (variableId == afGlobalVariableChangedEvent::CURRENT_PROJECT)
    {
        clearList();
    }
}

