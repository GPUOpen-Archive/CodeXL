//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdDebuggedProcessEventsView.cpp
///
//==================================================================================

//------------------------------ gdDebuggedProcessEventsView.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessDetectedErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreationFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apInfrastructureFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleLoadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apTechnologyMonitorFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apMemoryLeakEvent.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTOpenGLServer/Include/gsPublicStringConstants.h>


// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afHTMLUtils.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdDebuggedProcessEventsView.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdStopDebuggingCommand.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdEventStringBuilder.h>

#define GD_DEBUGGED_PROCESS_EVENTS_SCROLL_TIMER_DURATION 100

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::gdDebuggedProcessEventsView
// Description: Constructor.
// Arguments:   parent - My parent window.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
gdDebuggedProcessEventsView::gdDebuggedProcessEventsView(QWidget* pParent)
    : acListCtrl(pParent), m_interceptionWarningShown(false),
      _wasGremedyOGLServerLoaded(false), _wasGremedyOGLESServerLoaded(false),
      _shouldDisplayEventProperties(true), m_pClearViewAction(NULL)
{
    // Create and load the image list:
    createAndLoadImageList();

    bool rcConnect = connect(this, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onDebugProcessEventsSelected(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(itemPressed(QTableWidgetItem*)), this, SLOT(onDebugProcessEventsSelected(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onDebugProcessEventsSelected(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onDebugProcessEventsSelected(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)), this, SLOT(onDebugProcessEventsCurrentItemChanged(QTableWidgetItem*, QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

    // Hide horizontal headers:
    horizontalHeader()->hide();
    verticalHeader()->hide();

    setColumnCount(1);

    // Extend the context menu:
    extendContextMenu();
}


// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::~gdDebuggedProcessEventsView
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
gdDebuggedProcessEventsView::~gdDebuggedProcessEventsView()
{
    // Uri, 23/8/09: When quitting CodeXL Mac (usually after debugging an iPhone app),
    // this function is sometimes called after the properties view was already destroyed
    // and throws a selection event. Avoid updating the (nonexistant) view in this case:
    _shouldDisplayEventProperties = false;

    // Delete just the items' data, wx will clean up the items themselves:
    clearItemsData();

    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}


// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::clearView
// Description: Clear the event view
// Return Val: void
// Author:      Sigal Algranaty
// Date:        6/7/2009
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::clearView()
{
    clearEventsList();
}


// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::createAndLoadImageList
// Description: Creates this view image list, and loads its images
//              from disk.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::createAndLoadImageList()
{
    // Create the pixmap items:
    QPixmap* pBreakOnDeprecated = new QPixmap;
    acSetIconInPixmap(*pBreakOnDeprecated, AC_ICON_DEBUG_EVENTSVIEW_BREAK_DEPRECATED);

    QPixmap* pBreakOnDetectedError = new QPixmap;
    acSetIconInPixmap(*pBreakOnDetectedError, AC_ICON_DEBUG_EVENTSVIEW_BREAK_DETECTED);

    QPixmap* pBreakOnMemoryLeak = new QPixmap;
    acSetIconInPixmap(*pBreakOnMemoryLeak, AC_ICON_DEBUG_EVENTSVIEW_BREAK_MEMORY);

    QPixmap* pBreakOnOpenGLError = new QPixmap;
    acSetIconInPixmap(*pBreakOnOpenGLError, AC_ICON_DEBUG_EVENTSVIEW_BREAK_GLERROR);

    QPixmap* pBreakOnOpenCLError = new QPixmap;
    acSetIconInPixmap(*pBreakOnOpenCLError, AC_ICON_DEBUG_EVENTSVIEW_BREAK_CLERROR);

    QPixmap* pBreakOnRedundant = new QPixmap;
    acSetIconInPixmap(*pBreakOnRedundant, AC_ICON_DEBUG_EVENTSVIEW_BREAK_REDUNDANT);

    QPixmap* pBreakOnSoftwareFallback = new QPixmap;
    acSetIconInPixmap(*pBreakOnSoftwareFallback, AC_ICON_DEBUG_EVENTSVIEW_BREAK_SWFALLBACK);

    QPixmap* pBreakpoint = new QPixmap;
    acSetIconInPixmap(*pBreakpoint, AC_ICON_DEBUG_EVENTSVIEW_BREAK_POINT);

    QPixmap* pBreak = new QPixmap;
    acSetIconInPixmap(*pBreak, AC_ICON_DEBUG_EVENTSVIEW_BREAK);

    QPixmap* pConnectionEnded = new QPixmap;
    acSetIconInPixmap(*pConnectionEnded, AC_ICON_DEBUG_EVENTSVIEW_CONNECTION_MINUS);

    QPixmap* pConnectionEstablished = new QPixmap;
    acSetIconInPixmap(*pConnectionEstablished, AC_ICON_DEBUG_EVENTSVIEW_CONNECTION_PLUS);

    QPixmap* pRenderContextCreated = new QPixmap;
    acSetIconInPixmap(*pRenderContextCreated, AC_ICON_DEBUG_EVENTSVIEW_GLCONTEXT_PLUS);

    QPixmap* pRenderContextDeleted = new QPixmap;
    acSetIconInPixmap(*pRenderContextDeleted, AC_ICON_DEBUG_EVENTSVIEW_GLCONTEXT_MINUS);

    QPixmap* pComputeContextCreated = new QPixmap;
    acSetIconInPixmap(*pComputeContextCreated, AC_ICON_DEBUG_EVENTSVIEW_CLCONTEXT_PLUS);

    QPixmap* pComputeContextDeleted = new QPixmap;
    acSetIconInPixmap(*pComputeContextDeleted, AC_ICON_DEBUG_EVENTSVIEW_CLCONTEXT_MINUS);

    QPixmap* pCLProgramBuild = new QPixmap;
    acSetIconInPixmap(*pCLProgramBuild, AC_ICON_DEBUG_EVENTSVIEW_CLPROGRAM_BUILD);

    QPixmap* pDetectedError = new QPixmap;
    acSetIconInPixmap(*pDetectedError, AC_ICON_DEBUG_EVENTSVIEW_DETECTED);

    QPixmap* pDrawStep = new QPixmap;
    acSetIconInPixmap(*pDrawStep, AC_ICON_DEBUG_EVENTSVIEW_BREAK_DRAWSTEP);

    QPixmap* pException = new QPixmap;
    acSetIconInPixmap(*pException, AC_ICON_DEBUG_EVENTSVIEW_EXCEPTION);

    QPixmap* pForeignBreak = new QPixmap;
    acSetIconInPixmap(*pForeignBreak, AC_ICON_DEBUG_EVENTSVIEW_BREAK_UNKNOWN);

    QPixmap* pFrameStep = new QPixmap;
    acSetIconInPixmap(*pFrameStep, AC_ICON_DEBUG_EVENTSVIEW_BREAK_FRAMESTEP);

    QPixmap* pModuleLoaded = new QPixmap;
    acSetIconInPixmap(*pModuleLoaded, AC_ICON_DEBUG_EVENTSVIEW_MODULE_PLUS);

    QPixmap* pModuleUnloaded = new QPixmap;
    acSetIconInPixmap(*pModuleUnloaded, AC_ICON_DEBUG_EVENTSVIEW_MODULE_MINUS);

    QPixmap* pOutputString = new QPixmap;
    acSetIconInPixmap(*pOutputString, AC_ICON_DEBUG_EVENTSVIEW_OUTPUT);

    QPixmap* pProcessCreated = new QPixmap;
    acSetIconInPixmap(*pProcessCreated, AC_ICON_DEBUG_EVENTSVIEW_PROCESS_PLUS);

    QPixmap* pProcessRunStarted = new QPixmap;
    acSetIconInPixmap(*pProcessRunStarted, AC_ICON_DEBUG_EVENTSVIEW_PROCESS_RUN);

    QPixmap* pProcessTerminated = new QPixmap;
    acSetIconInPixmap(*pProcessTerminated, AC_ICON_DEBUG_EVENTSVIEW_PROCESS_MINUS);

    QPixmap* pStep = new QPixmap;
    acSetIconInPixmap(*pStep, AC_ICON_DEBUG_EVENTSVIEW_BREAK_APISTEP);

    QPixmap* pThreadCreated = new QPixmap;
    acSetIconInPixmap(*pThreadCreated, AC_ICON_DEBUG_EVENTSVIEW_THREAD_PLUS);

    QPixmap* pThreadTerminated = new QPixmap;
    acSetIconInPixmap(*pThreadTerminated, AC_ICON_DEBUG_EVENTSVIEW_THREAD_MINUS);

    QPixmap* pGDBIcon = new QPixmap;
    acSetIconInPixmap(*pGDBIcon, AC_ICON_DEBUG_EVENTSVIEW_GDB);

    QPixmap* pYellowWarningIcon = new QPixmap;
    acSetIconInPixmap(*pYellowWarningIcon, AC_ICON_WARNING_YELLOW);

    QPixmap* pMemoryViewerIcon = new QPixmap;
    acSetIconInPixmap(*pMemoryViewerIcon, AC_ICON_DEBUG_VIEW_MEMORY);

    QPixmap* pOpenCLError = new QPixmap;
    acSetIconInPixmap(*pOpenCLError, AC_ICON_DEBUG_EVENTSVIEW_CLERROR);

    QPixmap* pqueueCreated = new QPixmap;
    acSetIconInPixmap(*pqueueCreated, AC_ICON_DEBUG_EVENTSVIEW_CLQUEUE_PLUS);

    QPixmap* pqueueDeleted = new QPixmap;
    acSetIconInPixmap(*pqueueDeleted, AC_ICON_DEBUG_EVENTSVIEW_CLQUEUE_MINUS);

    QPixmap* pclProgramCreated = new QPixmap;
    acSetIconInPixmap(*pclProgramCreated, AC_ICON_DEBUG_EVENTSVIEW_CLPROGRAM_PLUS);

    QPixmap* pclProgramDeleted = new QPixmap;
    acSetIconInPixmap(*pclProgramDeleted, AC_ICON_DEBUG_EVENTSVIEW_CLPROGRAM_MINUS);

    QPixmap* pdebugOutput = new QPixmap;
    acSetIconInPixmap(*pdebugOutput, AC_ICON_WARNING_YELLOW);


    // Add the icons to the image list, and add the icon index to a mapping for later use:
    _listIconsVec.push_back(pProcessCreated);
    _eventTypeToIconIndexMap[apEvent::AP_DEBUGGED_PROCESS_CREATED] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pProcessRunStarted);
    _eventTypeToIconIndexMap[apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pProcessTerminated);
    _eventTypeToIconIndexMap[apEvent::AP_DEBUGGED_PROCESS_TERMINATED] = _listIconsVec.size() - 1;
    _eventTypeToIconIndexMap[apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pThreadCreated);
    _eventTypeToIconIndexMap[apEvent::AP_THREAD_CREATED] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pThreadTerminated);
    _eventTypeToIconIndexMap[apEvent::AP_THREAD_TERMINATED] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pModuleLoaded);
    _eventTypeToIconIndexMap[apEvent::AP_MODULE_LOADED] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pModuleUnloaded);
    _eventTypeToIconIndexMap[apEvent::AP_MODULE_UNLOADED] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pRenderContextCreated);
    _eventTypeToIconIndexMap[apEvent::AP_RENDER_CONTEXT_CREATED_EVENT] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pRenderContextDeleted);
    _eventTypeToIconIndexMap[apEvent::AP_RENDER_CONTEXT_DELETED_EVENT] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pComputeContextCreated);
    _eventTypeToIconIndexMap[apEvent::AP_COMPUTE_CONTEXT_CREATED_EVENT] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pCLProgramBuild);
    _eventTypeToIconIndexMap[apEvent::AP_OPENCL_PROGRAM_BUILD_EVENT] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pComputeContextDeleted);
    _eventTypeToIconIndexMap[apEvent::AP_COMPUTE_CONTEXT_DELETED_EVENT] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pConnectionEstablished);
    _eventTypeToIconIndexMap[apEvent::AP_API_CONNECTION_ESTABLISHED] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pConnectionEnded);
    _eventTypeToIconIndexMap[apEvent::AP_API_CONNECTION_ENDED] = _listIconsVec.size() - 1;
    _eventTypeToIconIndexMap[apEvent::AP_TECHNOLOGY_MONITOR_FAILURE_EVENT] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pException);
    _eventTypeToIconIndexMap[apEvent::AP_EXCEPTION] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pOutputString);
    _eventTypeToIconIndexMap[apEvent::AP_DEBUGGED_PROCESS_OUTPUT_STRING] = _listIconsVec.size() - 1;
    _eventTypeToIconIndexMap[apEvent::AP_USER_WARNING] = _listIconsVec.size() - 1;
    _eventTypeToIconIndexMap[apEvent::AP_OUTPUT_DEBUG_STRING] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pdebugOutput);
    _eventTypeToIconIndexMap[apEvent::AP_GL_DEBUG_OUTPUT_MESSAGE] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pGDBIcon);
    _eventTypeToIconIndexMap[apEvent::AP_GDB_OUTPUT_STRING] = _listIconsVec.size() - 1;
    _eventTypeToIconIndexMap[apEvent::AP_GDB_ERROR] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pOpenCLError);
    _eventTypeToIconIndexMap[apEvent::AP_OPENCL_ERROR] = _listIconsVec.size() - 1;
    _listIconsVec.push_back(pBreakOnOpenCLError);

    // Add the breakpoints events icons:
    _listIconsVec.push_back(pBreak);
    _eventTypeToIconIndexMap[apEvent::AP_BREAKPOINT_HIT] = _listIconsVec.size() - 1;

    _listIconsVec.push_back(pStep);
    _listIconsVec.push_back(pDrawStep);
    _listIconsVec.push_back(pFrameStep);
    _listIconsVec.push_back(pBreakpoint);
    _listIconsVec.push_back(pBreakOnOpenGLError);
    _listIconsVec.push_back(pBreakOnRedundant);
    _listIconsVec.push_back(pBreakOnDeprecated);
    _listIconsVec.push_back(pBreakOnSoftwareFallback);
    _listIconsVec.push_back(pForeignBreak);
    _listIconsVec.push_back(pBreakOnMemoryLeak);

    // Memory leak (without break & with break):
    _listIconsVec.push_back(pYellowWarningIcon);
    _eventTypeToIconIndexMap[apEvent::AP_MEMORY_LEAK] = _listIconsVec.size() - 1;
    // Check for memory leaks:
    _listIconsVec.push_back(pMemoryViewerIcon);
    _eventTypeToIconIndexMap[apEvent::AP_SEARCHING_FOR_MEMORY_LEAKS] = _listIconsVec.size() - 1;

    // Detected error and break on detected error:
    _listIconsVec.push_back(pDetectedError);
    _eventTypeToIconIndexMap[apEvent::AP_DETECTED_ERROR_EVENT] = _listIconsVec.size() - 1;
    _listIconsVec.push_back(pBreakOnDetectedError);

    // Queue created and deleted events:
    _listIconsVec.push_back(pqueueCreated);
    _eventTypeToIconIndexMap[apEvent::AP_OPENCL_QUEUE_CREATED_EVENT] = _listIconsVec.size() - 1;
    _listIconsVec.push_back(pqueueDeleted);
    _eventTypeToIconIndexMap[apEvent::AP_OPENCL_QUEUE_DELETED_EVENT] = _listIconsVec.size() - 1;

    // OpenCL program created and deleted events:
    _listIconsVec.push_back(pclProgramCreated);
    _eventTypeToIconIndexMap[apEvent::AP_OPENCL_PROGRAM_CREATED_EVENT] = _listIconsVec.size() - 1;
    _listIconsVec.push_back(pclProgramDeleted);
    _eventTypeToIconIndexMap[apEvent::AP_OPENCL_PROGRAM_DELETED_EVENT] = _listIconsVec.size() - 1;

}


// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::clearEventsList
// Description: Clear the events list.
// Author:      Yaki Tebeka
// Date:        17/5/2007
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::clearEventsList()
{
    // Delete the items data:
    clearItemsData();

    // Delete Items from the list
    clearList();
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::clearItemsData
// Description: Deletes the data of all items. Note that this does not set the
//              items' data to NULL - use clearEventsList instead.
//              This function should only be used by itsself in the destructor,
//              where calling wxListCtrl::DeleteAllItems() causes a wx assert
//              on Mac.
// Author:      Uri Shomroni
// Date:        16/9/2009
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::clearItemsData()
{
    // Delete all item's attached data objects:
    int listSize = rowCount();

    for (int i = 0; i < listSize; i++)
    {
        apEvent* pEvent = (apEvent*)(getItemData(i));

        if (pEvent != NULL)
        {
            delete pEvent;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::onEvent
// Description: Is called when a debugged process event occurs.
//              Send the event to the appropriate event function for displaying
//              it to the user.
// Arguments:   event - The debugged process eve.
// Author:      Yaki Tebeka
// Date:        4/4/2004
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    // Build the string and add to the list view:
    gtString eventAsString;
    gdEventStringBuilder stringEventBuilder;
    stringEventBuilder.buildEventString(eve, eventAsString);

    if (!eventAsString.isEmpty())
    {
        insertListItem(eventAsString, eve);
    }

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // Down cast the event according to its type, and call the appropriate
    // event handling function:
    switch (eventType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            const apDebuggedProcessCreatedEvent& processCreatedEvent = (const apDebuggedProcessCreatedEvent&)eve;
            onProcessCreation(processCreatedEvent);
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            const apDebuggedProcessTerminatedEvent& processTermiatedEvent = (const apDebuggedProcessTerminatedEvent&)eve;
            onProcessTermination(processTermiatedEvent);
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            const apDebuggedProcessCreationFailureEvent& processCreationFailureEvent = (const apDebuggedProcessCreationFailureEvent&)eve;
            onProcessCreationFailure(processCreationFailureEvent);
        }
        break;

        case apEvent::AP_MODULE_LOADED:
        {
            const apModuleLoadedEvent& moduleLoadedEvent = (const apModuleLoadedEvent&)eve;
            onModuleLoad(moduleLoadedEvent);
        }
        break;

        case apEvent::AP_EXCEPTION:
        {
            focusLastItem();
        }
        break;

        case apEvent::AP_GDB_ERROR:
        {
            const apGDBErrorEvent& gdbErrorEvent = (const apGDBErrorEvent&)eve;
            onGDBError(gdbErrorEvent);
        }
        break;

        case apEvent::AP_BREAKPOINT_HIT:
        {
            const apBreakpointHitEvent& breakpointEvent = (const apBreakpointHitEvent&)eve;

            if (breakpointEvent.breakReason() == AP_MEMORY_LEAK_BREAKPOINT_HIT)
            {
                onMemoryLeakBreak(breakpointEvent);
            }
        }
        break;

        case apEvent::AP_DETECTED_ERROR_EVENT:
        {
            const apDebuggedProcessDetectedErrorEvent& errorEvent = (const apDebuggedProcessDetectedErrorEvent&)eve;

            // Focus the last item if break was generated:
            bool wasErrorBrokenOn = errorEvent.wasGeneratedByBreak();

            // If this is a break, highlight the item:
            if (wasErrorBrokenOn)
            {
                focusLastItem();
            }
        }
        break;

        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            // Down cast the event:
            const afGlobalVariableChangedEvent& variableChangedEvent = (const afGlobalVariableChangedEvent&)eve;

            // Handle it:
            if (variableChangedEvent.changedVariableId() == afGlobalVariableChangedEvent::CURRENT_PROJECT)
            {
                clearView();
            }
        }
        break;

        case apEvent::AP_INFRASTRUCTURE_FAILURE_EVENT:
        {
            const apInfrastructureFailureEvent& infraFailureEvent = (const apInfrastructureFailureEvent&)eve;
            onInfrastructureFailureEvent(infraFailureEvent);
        }
        break;

        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            onExecutionModeChangedEvent(eve);
        }
        break;

        default:
            // We do not report these events to the user.
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::onProcessCreation
// Description: Is called when a debugged process is created by our debugger.
//              Adds the process creation report to this view list.
// Arguments:   processCreatedEvent - A class representing the process creation event.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::onProcessCreation(const apDebuggedProcessCreatedEvent& processCreatedEvent)
{
    (void)(processCreatedEvent);  // unused
    // Get the current project name:
    gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();

    gtString logMsg;
    logMsg.appendFormattedString(GD_STR_LogMsg_processCreated, projectName.asCharArray());

    // Output an "Project name" log printout:
    OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_INFO);

    // Make the list empty:
    clearEventsList();
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::onInfrastructureFailureEvent
// Description: Is called when an infrastructure failure happens.
// Arguments: infraFailureEvent - A class containing the failure details.
// Author:      Yaki Tebeka
// Date:        5/8/2008
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::onInfrastructureFailureEvent(const apInfrastructureFailureEvent& infraFailureEvent)
{
    // Get the infrastructure failure reason:
    apInfrastructureFailureEvent::FailureReason failureReason = infraFailureEvent.failureReason();

    if (failureReason == apInfrastructureFailureEvent::FAILED_TO_INITIALIZE_GDB)
    {
        // Raise an appropriate message box:
        QString errString = GD_STR_ErrorGDBIsNotInstalled;

        acMessageBox::instance().critical(GD_STR_ErrorGDBIsNotInstalledMsgHeader, errString, QMessageBox::Ok);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::onProcessTermination
// Description: Is called when the debugged process terminates (exits).
//              Adds the process exit report to this view list
// Arguments:   processTerminatedEvent - A class representing the process termination event.
// Author:      Yaki Tebeka
// Date:        20/12/2003
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::onProcessTermination(const apDebuggedProcessTerminatedEvent& processTerminatedEvent)
{
    (void)(processTerminatedEvent);  // unused

    // Clear the interception failure cache:
    m_apiModuleLoaded.clear();
    m_interceptionWarningShown = false;

    // Initialize the Spy loading check
    _wasGremedyOGLServerLoaded = false;

    // Initialize the OpenGL ES Common dll flag:
    _wasGremedyOGLESServerLoaded = false;
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::onProcessCreationFailure
// Description: Is called when the debugged process terminates (exits).
//              Adds the process exit report to this view list
// Arguments:   processTermiatedEvent - A class representing the process termination event.
// Author:      Sigal Algranaty
// Date:        11/2/2010
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::onProcessCreationFailure(const apDebuggedProcessCreationFailureEvent& processCreationFailureEvent)
{
    // Get the failure reason:
    apDebuggedProcessCreationFailureEvent::ProcessCreationFailureReason failureReason = processCreationFailureEvent.processCreationFailureReason();

    switch (failureReason)
    {
        case apDebuggedProcessCreationFailureEvent::AUTOMATIC_CONFIGURATION_FAILED:
        {
            // Display a message box suggesting to switch to manual setup if the problem persists:
            acMessageBox::instance().critical(AF_STR_ErrorA, GD_STR_ErrorAutomaticConfigurationFailed, QMessageBox::Ok);
        }
        break;

        case apDebuggedProcessCreationFailureEvent::COULD_NOT_CREATE_PROCESS:
        {
            gtString errMsgGTString = GD_STR_ErrorCouldNotCreateProcess;
            errMsgGTString += L"\n";

            // Get the failure reason string:
            errMsgGTString += processCreationFailureEvent.processCreationError();

            if (processCreationFailureEvent.processCreationError().isEmpty())
            {
                if (afProjectManager::instance().currentProjectSettings().isRemoteTarget())
                {
                    // If we are in a remote session, let the user know that we have failed to connect to the remote agent.
                    errMsgGTString += AF_STR_REMOTE_GENERAL_ERROR_MSG;
                }
                else
                {
                    errMsgGTString += GD_STR_ErrorCouldNotCreateProcessGenericReason;
                }
            }

            QString errMsg = acGTStringToQString(errMsgGTString);
            acMessageBox::instance().critical(AF_STR_ErrorA, errMsg, QMessageBox::Ok);
        }
        break;

        case apDebuggedProcessCreationFailureEvent::REMOTE_HANDSHAKE_MISMATCH:
        {
            // Output the error to the user.
            QString errMsg = acGTStringToQString(processCreationFailureEvent.processCreationError());
            acMessageBox::instance().critical(AF_STR_ErrorA, errMsg, QMessageBox::Ok);
        }
        break;

        default:
        {
            // Unexpected value!
            GT_ASSERT(false);
        }
        break;
    }

    // Clear the interception failure cache:
    m_apiModuleLoaded.clear();
    m_interceptionWarningShown = false;

    // Initialize the Spy loading check
    _wasGremedyOGLServerLoaded = false;

    // Initialize the OpenGL ES Common dll flag:
    _wasGremedyOGLESServerLoaded = false;
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::onModuleLoad
// Description: Is called when the debugged process loads a DLL.
//              Updates this view list with the name of the loaded DLL.
// Arguments:   dllLoadedEvent - A class representing the dll load event.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::onModuleLoad(const apModuleLoadedEvent& dllLoadedEvent)
{
    // Get the loaded module path:
    const gtString& modulePath = dllLoadedEvent.modulePath();

    // Perform module load related sanity checks:
    checkLoadedModule(modulePath);
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::focusLastItem
// Description: Focus the last item added to the list
// Author:      Sigal Algranaty
// Date:        4/7/2011
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::focusLastItem()
{
    // Ensure the item is visible:
    int lastItem = rowCount() - 1;

    if (lastItem >= 0)
    {
        // Get the widget item:
        QTableWidgetItem* pItem = item(lastItem, 0);

        if (pItem != NULL)
        {
            scrollToItem(pItem, QAbstractItemView::EnsureVisible);
        }
    }

}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::onGDBError
// Description: Is called when a GDB Error occurs
// Author:      Uri Shomroni
// Date:        13/1/2009
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::onGDBError(const apGDBErrorEvent& gdbErrorEvent)
{
    (void)(gdbErrorEvent);  // unused
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    static gtString sMacArchitectureErrorString = "Architecture of file not recognized.";

    if (gdbErrorString == sMacArchitectureErrorString)
    {
        // The user tried to debug a PowerPC or 64-bit application, so notify them about this:
        acMessageBox::instance().critical(AF_STR_ErrorA, GD_STR_ErrorMacOSXArchitectureError, QMessageBox::Ok);
    }

#endif
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::onBreakpointHit
// Description: Is called when the debugged application hits a breakpoint.
//              Adds a "Breakpoint hit" event to the process events list.
// Arguments:   breakpointEvent - A class representing the breakpoint hit.
// Author:      Yaki Tebeka
// Date:        20/5/2004
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::onBreakpointHit(const apBreakpointHitEvent& breakpointEvent)
{
    // Get the break reason:
    apBreakReason breakReason = breakpointEvent.breakReason();

    // On most breakpoints (error types and foreign breakpoints) we don't want
    // this view to display the event properties in the properties view.
    // So, to we mark a flag that disables onDebugProcessEventsSelected() to display
    // the event properties (SetItemState calls indirectly onDebugProcessEventsSelected)
    if (!((breakReason == AP_OPENGL_ERROR_BREAKPOINT_HIT)
          || (breakReason == AP_OPENCL_ERROR_BREAKPOINT_HIT)
          || (breakReason == AP_DETECTED_ERROR_BREAKPOINT_HIT)
          || (breakReason == AP_SOFTWARE_FALLBACK_BREAKPOINT_HIT)
          || (breakReason == AP_FOREIGN_BREAK_HIT)
          || (breakReason == AP_REDUNDANT_STATE_CHANGE_BREAKPOINT_HIT)
          || (breakReason == AP_DEPRECATED_FUNCTION_BREAKPOINT_HIT)
         ))
    {
        _shouldDisplayEventProperties = false;
    }
    else
    {
        // Make me the window that receives keyboard input:
        setFocus();
    }

    // Focus the last item added:
    focusLastItem();
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::onMemoryLeakBreak
// Description: Is called when the debugged application hits a memory leak breakpoint.
//              Since we already handle memory leak when we get the memory leak event,
//              we only need to turn it into a breakpoint.
// Arguments:   breakpointEvent - A class representing the breakpoint hit.
// Author:      Yaki Tebeka
// Date:        20/5/2004
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::onMemoryLeakBreak(const apBreakpointHitEvent& breakpointEvent)
{
    // Make me the window that receives keyboard input:
    setFocus();

    // Look for the last memory leak event item:
    int lastMemoryLeakEventIndex = -1;

    // Get the last event index:
    int lastEventIndex = rowCount() - 1;

    for (int i = lastEventIndex; i >= 0; i--)
    {
        // Get the item data:
        QString text;
        (void) getItemText(i, 0, text);
        gtString itemText;
        itemText.fromASCIIString(text.toLatin1().data());

        if (itemText.startsWith(AP_STR_MemoryLeakStart))
        {
            // We found the memory leak event item:
            lastMemoryLeakEventIndex = i;
            break;
        }
    }


    // If the event was added:
    GT_IF_WITH_ASSERT_EX((lastMemoryLeakEventIndex >= 0), L"Memory leak breakpoint was raised, but no memory leak event was found")
    {
        QTableWidgetItem* pItem = item(lastMemoryLeakEventIndex, 0);

        if (pItem != NULL)
        {
            // Focus the memory leak event item:
            setItemSelected(pItem, true);
            scrollToItem(pItem, QAbstractItemView::EnsureVisible);

            // Get the icon for memory leaks break:
            int imageIndex = apEventToIconIndex(&breakpointEvent);

            if ((imageIndex >= 0) && (imageIndex < (int)_listIconsVec.size()))
            {
                // Get the pixmap:
                QPixmap* pPixmap = _listIconsVec[imageIndex];

                // Change the memory leak event image index:
                pItem->setIcon(QIcon(*pPixmap));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::onSetFocus
// Description: Is called when the view get focus
// Arguments:   event - get the event that triggered the context menu
// Author:      Avi Shapira
// Date:        29/6/2008
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::focusInEvent(QFocusEvent* pEvent)
{
    (void)(pEvent);  // unused

    // Get the currently selected item:
    if (!selectedItems().isEmpty())
    {
        // Get the first selected item:
        QTableWidgetItem* pSelected = selectedItems().first();

        if (pSelected != NULL)
        {
            // Call and process the event:
            onDebugProcessEventsSelected(pSelected);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::onDebugProcessEventsSelected
// Description: Write the details of the selected item to the properties view
// Arguments:   wxListEvent &eve
// Author:      Avi Shapira
// Date:        3/5/2004
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::onDebugProcessEventsSelected(QTableWidgetItem* pSelectedItem)
{
    if (_shouldDisplayEventProperties)
    {
        // If we selected more than one item, multipleItemsSelected = true
        bool multipleItemsSelected = (selectedItems().size() > 1);

        // Remove the recoding warning:

        if (multipleItemsSelected)
        {
            gdHTMLProperties htmlBuilder;
            gtString propertiesViewMessage;

            // Multiple items were selected, show the "Multiple items selected" message:
            afHTMLContent htmlContent;
            htmlBuilder.buildMultipleItemPropertiesString(GD_STR_ProcessEventsViewPropertiesTitle, GD_STR_ProcessEventsViewPropertiesDebugProcessEventName, htmlContent);
            htmlContent.toString(propertiesViewMessage);

            // Display the HTML message:
            gdPropertiesEventObserver::instance().setPropertiesFromText(acGTStringToQString(propertiesViewMessage));
        }
        else if (_shouldDisplayEventProperties)
        {
            // Id displaying the event properties was not temporarily disabled:

            gtString itemData;

            // Clear the properties view
            gdPropertiesEventObserver::instance().setPropertiesFromText(acGTStringToQString(gdHTMLProperties::emptyHTML()));

            // We selected only one item, show its properties
            if (pSelectedItem != NULL)
            {
                apEvent* pEvent = (apEvent*)(getItemData(pSelectedItem->row()));
                afHTMLContent htmlContent;

                if (pEvent != NULL)
                {
                    // Get the process creation data:
                    gdGDebuggerGlobalVariablesManager& theStateManager = gdGDebuggerGlobalVariablesManager::instance();
                    const apDebugProjectSettings& processCreationData = theStateManager.currentDebugProjectSettings();

                    // Define an HTML builder object:
                    gdHTMLProperties htmlBuilder;
                    afHTMLUtils htmlUtils;

                    // Define an HTML properties string (which is built according to event type):
                    gtString htmlPropertiesString;

                    apEvent::EventType eventType = pEvent->eventType();

                    switch (eventType)
                    {
                        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
                        {
                            const apDebuggedProcessCreatedEvent& processCreatedEvent = (*(apDebuggedProcessCreatedEvent*)pEvent);
                            htmlUtils.buildProcessCreatedEventPropertiesString(processCreationData, processCreatedEvent, htmlPropertiesString);
                            break;
                        }

                        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
                        {
                            const apDebuggedProcessRunStartedEvent& processRunStartedEvent = (*(apDebuggedProcessRunStartedEvent*)pEvent);
                            htmlUtils.buildProcessRunStartedEventPropertiesString(processCreationData, processRunStartedEvent, htmlPropertiesString);
                            break;
                        }

                        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
                        {
                            const apDebuggedProcessTerminatedEvent& processTerminatedEvent = (*(apDebuggedProcessTerminatedEvent*)pEvent);
                            htmlUtils.buildProcessTerminationEventPropertiesString(processCreationData, processTerminatedEvent, htmlPropertiesString);
                            break;
                        }

                        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
                        {
                            const apDebuggedProcessCreationFailureEvent& event = (*(apDebuggedProcessCreationFailureEvent*)pEvent);
                            htmlBuilder.buildProcessCreationFailureEventPropertiesString(processCreationData, event, htmlContent);
                            break;
                        }

                        case apEvent::AP_MODULE_LOADED:
                        {
                            const apModuleLoadedEvent& event = (*(apModuleLoadedEvent*)pEvent);
                            htmlBuilder.buildDLLLoadPropertiesString(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_MODULE_UNLOADED:
                        {
                            const apModuleUnloadedEvent& event = (*(apModuleUnloadedEvent*)pEvent);
                            htmlBuilder.buildDLLUnloadPropertiesString(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_EXCEPTION:
                        {
                            const apExceptionEvent& event = (*(apExceptionEvent*)pEvent);
                            htmlBuilder.buildExceptionPropertiesString(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_GDB_OUTPUT_STRING:
                        {
                            const apGDBOutputStringEvent& event = (*(apGDBOutputStringEvent*)pEvent);
                            htmlBuilder.buildGDBOutputStringEventPropertiesString(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_GDB_ERROR:
                        {
                            const apGDBErrorEvent& event = (*(apGDBErrorEvent*)pEvent);
                            htmlBuilder.buildGDBErrorPropertiesString(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_BREAKPOINT_HIT:
                        {
                            const apBreakpointHitEvent& breakpointEvent = (*(apBreakpointHitEvent*)pEvent);
                            gtString funcName, funcArgs;
                            bool rc = gdGetCurrentBreakpointFunction(breakpointEvent.breakedOnFunctionCall(), funcName, funcArgs);
                            apExecutionMode currentExecMode = AP_DEBUGGING_MODE;
                            rc = rc && gaGetDebuggedProcessExecutionMode(currentExecMode);

                            if (currentExecMode == AP_PROFILING_MODE)
                            {
                                // Profiling mode
                                funcArgs.makeEmpty();
                            }

                            htmlBuilder.buildBreakpointPropertiesString(funcName, funcArgs, breakpointEvent, htmlContent);
                            break;
                        }

                        case apEvent::AP_DEBUGGED_PROCESS_OUTPUT_STRING:
                        {
                            const apDebuggedProcessOutputStringEvent& event = (*(apDebuggedProcessOutputStringEvent*)pEvent);
                            htmlBuilder.buildDebuggedProcessOutputStringEventString(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_DETECTED_ERROR_EVENT:
                        {
                            const apDebuggedProcessDetectedErrorEvent& event = (*(apDebuggedProcessDetectedErrorEvent*)pEvent);
                            htmlBuilder.buildErrorEventPropertiesString(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_THREAD_CREATED:
                        {
                            const apThreadCreatedEvent& event = (*(apThreadCreatedEvent*)pEvent);
                            htmlBuilder.buildThreadCreatedEventPropertiesString(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_THREAD_TERMINATED:
                        {
                            const apThreadTerminatedEvent& event = (*(apThreadTerminatedEvent*)pEvent);
                            htmlBuilder.buildThreadTerminatedEventPropertiesString(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_MEMORY_LEAK:
                        {
                            const apMemoryLeakEvent& event = (*(apMemoryLeakEvent*)pEvent);
                            htmlBuilder.buildMemoryLeakEventHTMLPropertiesString(event, false, htmlContent);
                            break;
                        }

                        case apEvent::AP_SEARCHING_FOR_MEMORY_LEAKS:
                        {
                            const apSearchingForMemoryLeaksEvent& event = (*(apSearchingForMemoryLeaksEvent*)pEvent);
                            htmlBuilder.buildSearchingForMemoryLeakEventHTMLPropertiesString(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_API_CONNECTION_ESTABLISHED:
                        {
                            const apApiConnectionEstablishedEvent& event = (*(apApiConnectionEstablishedEvent*)pEvent);
                            htmlBuilder.buildAPIConnectionEstablishedEventProperties(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_OUTPUT_DEBUG_STRING:
                        {
                            const apOutputDebugStringEvent& event = (*(apOutputDebugStringEvent*)pEvent);
                            htmlBuilder.buildOutputDebugStringEventString(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_API_CONNECTION_ENDED:
                        {
                            const apApiConnectionEndedEvent& event = (*(apApiConnectionEndedEvent*)pEvent);
                            htmlBuilder.buildAPIConnectionEndedEventProperties(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_RENDER_CONTEXT_CREATED_EVENT:
                        {
                            const apRenderContextCreatedEvent& event = (*(apRenderContextCreatedEvent*)pEvent);
                            htmlBuilder.buildRenderContextCreatedEventProperties(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_RENDER_CONTEXT_DELETED_EVENT:
                        {
                            const apRenderContextDeletedEvent& event = (*(apRenderContextDeletedEvent*)pEvent);
                            htmlBuilder.buildRenderContextDeletedEventProperties(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_COMPUTE_CONTEXT_CREATED_EVENT:
                        {
                            const apComputeContextCreatedEvent& event = (*(apComputeContextCreatedEvent*)pEvent);
                            htmlBuilder.buildComputeContextCreatedEventProperties(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_COMPUTE_CONTEXT_DELETED_EVENT:
                        {
                            const apComputeContextDeletedEvent& event = (*(apComputeContextDeletedEvent*)pEvent);
                            htmlBuilder.buildComputeContextDeletedEventProperties(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_OPENCL_QUEUE_CREATED_EVENT:
                        {
                            const apOpenCLQueueCreatedEvent& event = (*(apOpenCLQueueCreatedEvent*)pEvent);
                            htmlBuilder.buildOpenCLQueueCreatedEventProperties(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_OPENCL_QUEUE_DELETED_EVENT:
                        {
                            const apOpenCLQueueDeletedEvent& event = (*(apOpenCLQueueDeletedEvent*)pEvent);
                            htmlBuilder.buildOpenCLQueueDeletedEventProperties(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_OPENCL_PROGRAM_CREATED_EVENT:
                        {
                            const apOpenCLProgramCreatedEvent& event = (*(apOpenCLProgramCreatedEvent*)pEvent);
                            htmlBuilder.buildOpenCLProgramCreatedEventProperties(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_OPENCL_PROGRAM_DELETED_EVENT:
                        {
                            const apOpenCLProgramDeletedEvent& event = (*(apOpenCLProgramDeletedEvent*)pEvent);
                            htmlBuilder.buildOpenCLProgramDeletedEventProperties(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_OPENCL_PROGRAM_BUILD_EVENT:
                        {
                            const apOpenCLProgramBuildEvent& event = (*(apOpenCLProgramBuildEvent*)pEvent);
                            htmlBuilder.buildOpenCLProgramBuildEventProperties(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_TECHNOLOGY_MONITOR_FAILURE_EVENT:
                        {
                            const apTechnologyMonitorFailureEvent& event = (*(const apTechnologyMonitorFailureEvent*)pEvent);
                            htmlBuilder.buildTechnologyMonitorFailureEventProperties(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_GL_DEBUG_OUTPUT_MESSAGE:
                        {
                            const apGLDebugOutputMessageEvent& event = (*(apGLDebugOutputMessageEvent*)pEvent);
                            htmlBuilder.buildGLDebugOutputMessageEventProperties(event, htmlContent);
                            break;
                        }

                        case apEvent::AP_OPENCL_ERROR:
                        {

                            const apOpenCLErrorEvent& event = (*(apOpenCLErrorEvent*)pEvent);

                            // Get the function name and arguments strings:
                            gtString funcName, funcArgs;
                            bool rc = gdGetCurrentBreakpointFunction(event.breakedOnFunctionCall(), funcName, funcArgs);

                            // Get the execution mode:
                            apExecutionMode currentExecMode = AP_DEBUGGING_MODE;
                            rc = rc && gaGetDebuggedProcessExecutionMode(currentExecMode);

                            if (currentExecMode == AP_PROFILING_MODE)
                            {
                                // Profiling mode
                                funcArgs.makeEmpty();
                            }


                            // Build the event string:
                            htmlBuilder.buildCLErrorEventProperties(funcName, funcArgs, event, htmlContent);
                        }

                        default:
                            // Do nothing...
                            break;
                    }

                    // Set the properties text:
                    htmlContent.toString(htmlPropertiesString);
                    gdPropertiesEventObserver::instance().setPropertiesFromText(acGTStringToQString(htmlPropertiesString));
                }
            }
        }
    }

    // Return the flag to its default state:
    _shouldDisplayEventProperties = true;
}


// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::onDebugProcessEventsCurrentItemChanged
// Description: Is handling the current item changed signal
// Arguments:   QTableWidgetItem* pCurrentItem
//              QTableWidgetItem* pPreviousItem
// Author:      Sigal Algranaty
// Date:        18/3/2012
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::onDebugProcessEventsCurrentItemChanged(QTableWidgetItem* pCurrentItem, QTableWidgetItem* pPreviousItem)
{
    (void)(pPreviousItem);  // unused
    // Call the selected event:
    onDebugProcessEventsSelected(pCurrentItem);
}


// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::insertListItem
// Description: Add the Items to the list and the data to the item data
// Arguments:   int itemIndex
//              wxString itemString
//              int itemImage
//              apEvent& event
// Author:      Avi Shapira
// Date:        3/5/2004
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::insertListItem(const gtString& itemString, const apEvent& event)
{
    // Add to log:
    OS_OUTPUT_DEBUG_LOG(itemString.asCharArray(), OS_DEBUG_LOG_INFO);

    // If any items are selected, de-select them:
    clearSelection();

    // Get the item image index:
    QPixmap* pItemPixmap = NULL;
    int imageIndex = apEventToIconIndex(&event);
    GT_IF_WITH_ASSERT((imageIndex >= 0) && (imageIndex < (int)_listIconsVec.size()))
    {
        // Get the pixmap for this item type:
        pItemPixmap = _listIconsVec[imageIndex];
    }

    QStringList itemStringList;
    itemStringList << QString::fromWCharArray(itemString.asCharArray());

    // Do not change the selection while adding events:
    blockSignals(true);

    // Insert the new item to the list
    apEvent* pEventCopy = event.clone();
    addRow(itemStringList, pEventCopy, false, Qt::Unchecked, pItemPixmap);

    // Ensure the item is visible:
    focusLastItem();

    blockSignals(false);
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::checkLoadedModule
// Description: Performs sanity checks that should be done after the debugged
//              process have loaded a new module (dll / shared library).
// Arguments: modulePath - The loaded module path.
// Author:      Yaki Tebeka
// Date:        14/5/2007
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::checkLoadedModule(const gtString& modulePath)
{
    // Transform the module path into a lower case format:
    gtString modulePathLowerCase = modulePath;
    modulePathLowerCase.toLowerCase();

    // See if interception failed for any API:
    checkInterceptionFailure(modulePathLowerCase);

    // Perform OpenGL module related tests:
    checkLoadedOpenGLModule(modulePathLowerCase);

    // Perform OpenGL ES module related tests:
    checkLoadedOpenGLESModule(modulePathLowerCase);
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::checkInterceptionFailure
// Description: When an API module is loaded, tests to see if the appropriate server
//              was loaded before it. If it was not, report a warning to the user.
// Arguments: modulePathLowerCase - The loaded module path, in lower case format.
// Author:      Uri Shomroni
// Date:        5/4/2015
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::checkInterceptionFailure(const gtString& modulePath)
{
    // Show this message once per application run:
    if (!m_interceptionWarningShown)
    {
        static gtString openCLModuleNames[2];

        if (openCLModuleNames[0].isEmpty())
        {
            openCLModuleNames[0] = OS_OPENCL_ICD_MODULE_NAME;
            openCLModuleNames[0].toLowerCase();
            openCLModuleNames[1] = OS_OPENCL_ICD_MODULE_ALTERNATIVE_NAME;
            openCLModuleNames[1].toLowerCase();
        }

        // If this is one of the API dlls:
        bool isOpenGLModule = (-1 != modulePath.find(OS_OPENGL_MODULE_NAME));
        bool isOpenCLModule = ((-1 != modulePath.find(openCLModuleNames[0])) || (-1 != modulePath.find(openCLModuleNames[0])));

        if (isOpenGLModule || isOpenCLModule)
        {
            // If the module was the server (loaded from the servers folder)
            bool isCodeXLServerModule = (-1 != modulePath.find(OS_SPIES_SUB_DIR_NAME));
            gtString currentProjectExecutableDir;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            // Windows also has a spies 64 folder:
            isCodeXLServerModule = isCodeXLServerModule || (-1 != modulePath.find(OS_SPIES_64_SUB_DIR_NAME));

            // We also consider a server in the executable folder, as this is the workaround we suggest for .NET applications:
            if (!isCodeXLServerModule)
            {
                osFilePath currentProjectExecutable = afProjectManager::instance().currentProjectSettings().executablePath();
                currentProjectExecutable.clearFileExtension().clearFileName();
                currentProjectExecutableDir = currentProjectExecutable.asString(true);
                currentProjectExecutableDir.toLowerCase();

                if (!currentProjectExecutableDir.isEmpty() && modulePath.startsWith(currentProjectExecutableDir))
                {
                    isCodeXLServerModule = true;
                }
            }

#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

            apAPIConnectionType connType = AP_AMOUNT_OF_API_CONNECTION_TYPES;

            if (isOpenCLModule != isOpenGLModule)
            {
                if (isOpenCLModule)
                {
                    connType = AP_OPENCL_API_CONNECTION;
                }
                else if (isOpenGLModule)
                {
                    connType = AP_OPENGL_API_CONNECTION;
                }
                else
                {
                    // Unexpected API type!
                    GT_ASSERT(false);
                }
            }
            else
            {
                // This is either both OpenCL and OpenGL (impossible), or neither (not supposed to happen), either way, this should not happen
                GT_ASSERT(false);
            }

            if (isCodeXLServerModule)
            {
                // Mark that the module was loaded:
                m_apiModuleLoaded[connType] = true;
            }
            else // !isCodeXLServerModule
            {
                // Check if the correct module was already loaded:
                bool wasLoaded = false;
                gtMap<apAPIConnectionType, bool>::const_iterator findIter = m_apiModuleLoaded.find(connType);

                if (m_apiModuleLoaded.end() != findIter)
                {
                    wasLoaded = m_apiModuleLoaded[connType];
                }

                if (!wasLoaded)
                {
                    // The teapot sample application loads the OpenCL runtime, specifically, in a way that works around our interception.
                    // In that case, it is expected to fail:
                    osFilePath examplesPath;
                    bool isExample = examplesPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_EXAMPLES_PATH, true);

                    if (isExample)
                    {
                        if (currentProjectExecutableDir.isEmpty())
                        {
                            osFilePath currentProjectExecutable = afProjectManager::instance().currentProjectSettings().executablePath();
                            currentProjectExecutable.clearFileExtension().clearFileName();
                            currentProjectExecutableDir = currentProjectExecutable.asString(true);
                            currentProjectExecutableDir.toLowerCase();
                        }

                        isExample = currentProjectExecutableDir.startsWith(examplesPath.asString(false));

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
                        // For debug mode, any application with the sample name can be considered the sample:
                        isExample = isExample || (-1 != currentProjectExecutableDir.find(OS_STR_CodeXLExmaplesDirName));
#endif
                    }

                    if (!(isExample && isOpenCLModule))
                    {
                        // Display a warning message to the user:
                        acMessageBox::instance().warning(AF_STR_WarningA, GD_STR_ErrorMessageWarningInterceptionFailure);
                        m_interceptionWarningShown = true;
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::checkLoadedOpenGLModule
// Description: Performs sanity checks that should be done after the debugged
//              process have loaded an OpenGL module.
// Arguments: modulePathLowerCase - The loaded module path, in lower case format.
// Author:      Yaki Tebeka
// Date:        14/5/2007
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::checkLoadedOpenGLModule(const gtString& modulePathLowerCase)
{
    // If our OpenGL / ES server was not loaded yet:
    if (!_wasGremedyOGLServerLoaded && !_wasGremedyOGLESServerLoaded)
    {
        // If the loaded module is an OpenGL module:
        if (modulePathLowerCase.find(OS_OPENGL_MODULE_NAME) != -1)
        {
            // If the OpenGL server was loaded in a project type that does not support OpenGL/ES, show a message to the user.
            // Uri, 22/3/10: The other case is currently not "else", since we don't separate Windows and Linux (LD_LIBRARY_PATH interception)
            // spies, so the OpenCL-only projects would still load the OpenGL spy in this case. If the "else" is uncommented, this message
            // appears twice in these cases, once for our spy and once for the real OpenGL, loaded by our spy.  Also, this message entirely
            // ignores licensing, does not allow a "don't show me again" feature, etc.

            // Yaki - 13/12/2010
            // From ~ Catalyst 10.8 and until 10.11 at least, AMD's atiocl.dll loads opengl32.dll. This causes us to tell the user that he uses OpenGL in an OpenCL only project
            // (but he doesn't!), So, until atiocl.dll is fixed, I commented out the below printout:
            /*
            if (!apDoesProjectTypeSupportOpenGLOrOpenGLES(currentProjectType))
            {
            acMessageBox::instance().warning(GD_STR_WarningA, GD_STR_ErrorMessageWarningOpenGLServerInOpenCLOnlyProject, QMessageBox::Ok);
            }
            else // apDoesProjectTypeSupportOpenGLOrOpenGLES(currentProjectType)
            */

            {
                // If this is the Windows system's OpenGL module:
                bool isWinOGLSystemModule = isWindowsSystemOGLModule(modulePathLowerCase);

                if (!isWinOGLSystemModule)
                {
                    // We assume that this module is our OpenGL server:
                    _wasGremedyOGLServerLoaded = true;
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::checkLoadedOpenGLESModule
// Description: Performs sanity checks that should be done after the debugged
//              process have loaded an OpenGL ES module.
// Arguments: modulePathLowerCase - The loaded module path, in lower case format.
// Author:      Yaki Tebeka
// Date:        14/5/2007
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::checkLoadedOpenGLESModule(const gtString& modulePathLowerCase)
{
    // Check if the first loaded libgles_cm.dll/libgles_cl.dll is our spy.
    if ((modulePathLowerCase.find(OS_OPENGL_ES_COMMON_DLL_NAME) != -1) ||
        (modulePathLowerCase.find(OS_OPENGL_ES_COMMON_LITE_DLL_NAME) != -1))
    {
        // Get the current project type:
        // gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();

        // If we are not in an OpenGL ES project:
        // Handle the situation in which an OpenGL ES module is loaded while in OpenGL project type:
        _wasGremedyOGLESServerLoaded = true;
        handleESModuleWhileInOGLProject();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::isGremedyOGLESServer
// Description: Inputs an OpenGL ES module path and returns true iff this is
//              gremedy's OpenGL ES server.
// Author:      Yaki Tebeka
// Date:        14/5/2007
// ---------------------------------------------------------------------------
bool gdDebuggedProcessEventsView::isGremedyOGLESServer(const gtString& modulePathLowerCase)
{
    bool retVal = false;

    // Get the OpenGL ES implementation directory:
    gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
    const osFilePath& oglESDLLsDirectory = globalVarsManager.getOpenGLESDLLsDirectory();
    const gtString& oglESDLLsDirectoryAsStr = oglESDLLsDirectory.asString();

    if (!oglESDLLsDirectoryAsStr.isEmpty())
    {
        // Get it's last path separator position:
        int lastPathSaperatorPos = oglESDLLsDirectoryAsStr.reverseFind(osFilePath::osPathSeparator);

        if (lastPathSaperatorPos != -1)
        {
            // Get the ES implementation directory path last directory name:
            gtString esImplementationDirName;
            oglESDLLsDirectoryAsStr.getSubString(lastPathSaperatorPos, oglESDLLsDirectoryAsStr.length(), esImplementationDirName);

            if (!esImplementationDirName.isEmpty())
            {
                // If the input module path contains the ES implementation directory path last dir name:
                esImplementationDirName.toLowerCase();

                if (modulePathLowerCase.find(esImplementationDirName) != -1)
                {
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::isWindowsSystemOGLModule
// Description: Inputs an OpenGL module path and returns true iff this is
//              Windows OpenGL system module.
// Author:      Yaki Tebeka
// Date:        14/5/2007
// ---------------------------------------------------------------------------
bool gdDebuggedProcessEventsView::isWindowsSystemOGLModule(const gtString& modulePathLowerCase)
{
    bool retVal = false;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    retVal = (modulePathLowerCase.find(L"system") != -1);
#else
    (void)(modulePathLowerCase); // Resolve the compiler warning for the Linux variant
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::handleForeignOpenGLESImplementation
// Description: Handles the case in which the debugged process loaded an OpenGL ES
//              implementation, but it is not the gremedy implementation.
// Author:      Yaki Tebeka
// Date:        14/5/2007
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::handleForeignOpenGLESImplementation(const gtString& modulePathLowerCase)
{
    // Stop the debugged process:
    gdStopDebuggingCommand stopDebuggingCmd;
    bool rc = stopDebuggingCmd.execute();
    GT_ASSERT(rc);

    // Display a message box:
    QString errorModule = acGTStringToQString(modulePathLowerCase);
    QString errorMessage = QString(GD_STR_ErrorMessageFailedToLoadTheESSpy).arg(errorModule).arg(errorModule);
    acMessageBox::instance().critical(AF_STR_ErrorA, errorMessage, QMessageBox::Ok);
}


// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::handleESModuleWhileInOGLProject
// Description: Handles the case in which an OpenGL ES module is loaded while
//              in OpenGL project type.
// Author:      Yaki Tebeka
// Date:        14/5/2007
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::handleESModuleWhileInOGLProject()
{
    // Output the message into the log file:
    OS_OUTPUT_DEBUG_LOG(GD_STR_ErrorMessageLoadTheWrongSpyOpenGLESCommonUnicode, OS_DEBUG_LOG_INFO);

    // Stop the debugged process:
    gdStopDebuggingCommand stopDebuggingCmd;
    bool rc = stopDebuggingCmd.execute();
    GT_ASSERT(rc);

    // Display an Error message:
    acMessageBox::instance().critical(AF_STR_ErrorA, GD_STR_ErrorMessageLoadTheWrongSpyOpenGLESCommon, QMessageBox::Ok);

    // Get the application commands instance:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        // Open the debug settings dialog:
        pApplicationCommands->OnProjectSettings();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::handleOGLModuleWhileInESProject
// Description: Handles the case in which our OpenGL server is loaded while
//              in OpenGL ES project type before the OpenGL ES server is loaded.
// Author:      Yaki Tebeka
// Date:        14/5/2007
// ---------------------------------------------------------------------------
void gdDebuggedProcessEventsView::handleOGLModuleWhileInESProject()
{
    // Stop the debugged process:
    gdStopDebuggingCommand stopDebuggingCmd;
    bool rc = stopDebuggingCmd.execute();
    GT_ASSERT(rc);

    // Output the message into the log file:
    OS_OUTPUT_DEBUG_LOG(GD_STR_ErrorMessageLoadTheWrongSpyOpenGL32Unicode, OS_DEBUG_LOG_INFO);

    // Load an Error message:
    acMessageBox::instance().critical(AF_STR_ErrorA, GD_STR_ErrorMessageLoadTheWrongSpyOpenGL32, QMessageBox::Ok);

    // Get the application commands instance:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        // Open the debug settings dialog:
        pApplicationCommands->OnProjectSettings();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::apEventToIconIndex
// Description: Return an icon index for the requested event object
// Arguments: apEvent* pEvent
// Return Val: int
// Author:      Sigal Algranaty
// Date:        21/2/2010
// ---------------------------------------------------------------------------
int gdDebuggedProcessEventsView::apEventToIconIndex(const apEvent* pEvent)
{
    int retVal = -1;
    GT_IF_WITH_ASSERT(pEvent != NULL)
    {
        // Get the event type:
        apEvent::EventType eventType = pEvent->eventType();

        // Try to get the icon index from the mapping:
        gtMap<apEvent::EventType, int>::const_iterator iter = _eventTypeToIconIndexMap.find(eventType);

        if (iter != _eventTypeToIconIndexMap.end())
        {
            // For most of the events, the icon is simply the mapping:
            retVal = (*iter).second;

            // Breakpoint event can have few icons:
            if (eventType == apEvent::AP_BREAKPOINT_HIT)
            {
                apBreakpointHitEvent* pBreakpointHitEvent = (apBreakpointHitEvent*)pEvent;
                GT_IF_WITH_ASSERT(pBreakpointHitEvent != NULL)
                {
                    switch (pBreakpointHitEvent->breakReason())
                    {
                        case AP_BREAK_COMMAND_HIT:
                            // Do nothing, this is the icon mapped:
                            break;

                        case AP_NEXT_MONITORED_FUNCTION_BREAKPOINT_HIT:

                        // TO_DO: choose icons for step over and in.
                        case AP_STEP_IN_BREAKPOINT_HIT:
                        case AP_STEP_OVER_BREAKPOINT_HIT:
                        case AP_STEP_OUT_BREAKPOINT_HIT:
                            retVal++; // Step:
                            break;

                        case AP_DRAW_MONITORED_FUNCTION_BREAKPOINT_HIT:
                            retVal += 2; // Draw:
                            break;

                        case AP_FRAME_BREAKPOINT_HIT:
                            retVal += 3; // FrameStep:
                            break;

                        case AP_MONITORED_FUNCTION_BREAKPOINT_HIT:
                        case AP_KERNEL_SOURCE_CODE_BREAKPOINT_HIT:
                        case AP_KERNEL_FUNCTION_NAME_BREAKPOINT_HIT:
                        case AP_HOST_BREAKPOINT_HIT:
                            retVal += 4; // Breakpoint:
                            break;

                        case AP_OPENGL_ERROR_BREAKPOINT_HIT:
                            retVal += 5; // BreakOnOpenGLError:
                            break;

                        case AP_REDUNDANT_STATE_CHANGE_BREAKPOINT_HIT:
                            retVal += 6; // BreakOnRedundant:
                            break;

                        case AP_DEPRECATED_FUNCTION_BREAKPOINT_HIT:
                            retVal += 7; // BreakOnDeprecated:
                            break;

                        case AP_SOFTWARE_FALLBACK_BREAKPOINT_HIT:
                            retVal += 8; // BreakOnSoftwareFallback:
                            break;

                        case AP_FOREIGN_BREAK_HIT:
                            retVal += 9; // ForeignBreak:
                            break;

                        case AP_MEMORY_LEAK_BREAKPOINT_HIT:
                            retVal += 10; // BreakOnMemoryLeak:
                            break;

                        default:
                        {
                            GT_ASSERT_EX(false, L"Unknown breakpoint type");
                            break;
                        }
                    }
                }
            }
            else if (eventType == apEvent::AP_MEMORY_LEAK)
            {
                apMemoryLeakEvent* pMemoryLeakEvent = (apMemoryLeakEvent*)pEvent;
                GT_IF_WITH_ASSERT(pMemoryLeakEvent != NULL)
                {
                    if (!pMemoryLeakEvent->memoryLeakExists())
                    {
                        // Try to get the icon index from the mapping:
                        gtMap<apEvent::EventType, int>::const_iterator iterEvents = _eventTypeToIconIndexMap.find(apEvent::AP_SEARCHING_FOR_MEMORY_LEAKS);

                        if (iterEvents != _eventTypeToIconIndexMap.end())
                        {
                            // For most of the events, the icon is simply the mapping:
                            retVal = (*iterEvents).second;
                        }
                    }
                }
            }
            else if (eventType == apEvent::AP_DETECTED_ERROR_EVENT)
            {
                bool breakOnDetectedError = false;
                // Down cast the event to a detected error event:
                apDebuggedProcessDetectedErrorEvent* pDetectedErrorEvent = (apDebuggedProcessDetectedErrorEvent*)pEvent;
                GT_IF_WITH_ASSERT(pDetectedErrorEvent != NULL)
                {
                    breakOnDetectedError = pDetectedErrorEvent->wasGeneratedByBreak();
                }

                if (breakOnDetectedError)
                {
                    retVal ++;
                }
            }
            else if (pEvent->eventType() == apEvent::AP_OPENCL_ERROR)
            {
                bool breakOnOpenCLErrors = false;
                // Down cast the event to a detected error event:
                apOpenCLErrorEvent* pOpenCLErrorEvent = (apOpenCLErrorEvent*)pEvent;
                GT_IF_WITH_ASSERT(pOpenCLErrorEvent != NULL)
                {
                    breakOnOpenCLErrors = pOpenCLErrorEvent->wasGeneratedByBreak();
                }

                if (breakOnOpenCLErrors)
                {
                    retVal ++;
                }
            }
        }
        else // iter == _eventTypeToIconIndexMap.end()
        {
            gtString errMsg;
            errMsg.appendFormattedString(L"Added an event of type %d to list, without icon assigned for this type.", eventType);
            GT_ASSERT_EX(false, errMsg.asCharArray());
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::getLastEventOfType
// Description: Return the last event of the given type
// Arguments:   apEvent::EventType eventType
// Return Val:  apEvent*
// Author:      Sigal Algranaty
// Date:        20/7/2010
// ---------------------------------------------------------------------------
apEvent* gdDebuggedProcessEventsView::getLastEventOfType(apEvent::EventType eventType)
{
    apEvent* pRetVal = NULL;

    // Go through the event items, and search for the first event with the requested type:
    int lastItemIndex = rowCount() - 1;

    for (int i = lastItemIndex ; i >= 0; i--)
    {
        // Get the current item data:
        apEvent* pCurrentEvent = (apEvent*)(getItemData(i));

        if (pCurrentEvent != NULL)
        {
            // If the current event has the requested type:
            if (pCurrentEvent->eventType() == eventType)
            {
                pRetVal = pCurrentEvent;
                break;
            }
        }
    }

    return pRetVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        onExecutionModeChangedEvent
/// \brief Description: Is handling CodeXL execution mode change
/// \param[in]          execChangedEvent
/// \return void
/// -----------------------------------------------------------------------------------------------
void gdDebuggedProcessEventsView::onExecutionModeChangedEvent(const apEvent& execChangedEvent)
{
    bool isEnabled = false;
    bool modeChanged = gdDoesModeChangeApplyToDebuggerViews(execChangedEvent, isEnabled);

    if (modeChanged)
    {
        if (!isEnabled)
        {
            clearView();
        }

        setEnabled(isEnabled);
    }
}


void gdDebuggedProcessEventsView::onAboutToShowContextMenu()
{
    // Call the base class implementation:
    acListCtrl::onAboutToShowContextMenu();

    GT_IF_WITH_ASSERT(m_pClearViewAction != NULL)
    {
        // Enable is there are items shown:
        m_pClearViewAction->setEnabled(rowCount() > 1);
    }
}

void gdDebuggedProcessEventsView::extendContextMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pContextMenu != NULL)
    {
        // Add a separator:
        m_pContextMenu->insertSeparator(m_pContextMenu->actions().first());

        // Add "Clear View" action:
        m_pClearViewAction = new QAction(GD_STR_ProcessEventsViewClear, m_pContextMenu);

        // Insert the action to the menu, at position 0:
        m_pContextMenu->insertAction(m_pContextMenu->actions().first(), m_pClearViewAction);

        // Connect the action to the slot:
        bool rc = connect(m_pClearViewAction, SIGNAL(triggered()), this, SLOT(clearView()));
        GT_ASSERT(rc);
    }
}
