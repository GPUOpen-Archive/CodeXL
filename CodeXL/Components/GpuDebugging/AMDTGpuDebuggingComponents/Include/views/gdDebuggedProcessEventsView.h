//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdDebuggedProcessEventsView.h
///
//==================================================================================

//------------------------------ gdDebuggedProcessEventsView.h ------------------------------

#ifndef __GDDEBUGGEDPROCESSEVENTSVIEW
#define __GDDEBUGGEDPROCESSEVENTSVIEW

// Qt:
#include <QtWidgets>
#include <QPixmap>

// Forward declarations:
class apApiConnectionEndedEvent;
class apApiConnectionEstablishedEvent;
class apBreakpointHitEvent;
class apComputeContextCreatedEvent;
class apComputeContextDeletedEvent;
class apDebuggedProcessDetectedErrorEvent;
class apDebuggedProcessOutputStringEvent;
class apDebuggedProcessRunStartedEvent;
class apExceptionEvent;
class apGDBErrorEvent;
class apGDBOutputStringEvent;
class apGLDebugOutputMessageEvent;
class apInfrastructureFailureEvent;
class apMemoryLeakEvent;
class apModuleLoadedEvent;
class apModuleUnloadedEvent;
class apOutputDebugStringEvent;
class apOpenCLErrorEvent;
class apOpenCLProgramCreatedEvent;
class apOpenCLProgramDeletedEvent;
class apOpenCLProgramBuildEvent;
class apOpenCLQueueCreatedEvent;
class apOpenCLQueueDeletedEvent;
class apDebuggedProcessCreatedEvent;
class apDebuggedProcessTerminatedEvent;
class apDebuggedProcessCreationFailureEvent;
class apRenderContextCreatedEvent;
class apRenderContextDeletedEvent;
class apSearchingForMemoryLeaksEvent;
class apThreadCreatedEvent;
class apThreadTerminatedEvent;

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdDebuggedProcessEventsView: public acWXListCtrl
// General Description:
//  A viewer that monitors the debugged process events.
//  It logs: Process creation and termination, dll load and unload, exceptions, etc.
//
// Author:               Yaki Tebeka
// Creation Date:        1/11/2003
// ----------------------------------------------------------------------------------
class GD_API gdDebuggedProcessEventsView: public acListCtrl , public apIEventsObserver
{
    Q_OBJECT

public:

    gdDebuggedProcessEventsView(QWidget* pParent);
    virtual ~gdDebuggedProcessEventsView();

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"DebuggedProcessEventsView"; };

    // Self functions:
    void onProcessCreation(const apDebuggedProcessCreatedEvent& processCreatedEvent);
    void onProcessTermination(const apDebuggedProcessTerminatedEvent& processTermiatedEvent);
    void onProcessCreationFailure(const apDebuggedProcessCreationFailureEvent& processCreationFailureEvent);
    void onModuleLoad(const apModuleLoadedEvent& dllLoadedEvent);
    void onGDBError(const apGDBErrorEvent& gdbErrorEvent);
    void onBreakpointHit(const apBreakpointHitEvent& breakpointEvent);
    void onMemoryLeakBreak(const apBreakpointHitEvent& breakpointEvent);
    void onDebuggedProcessErrorEvent(const apDebuggedProcessDetectedErrorEvent& debuggedProcessErrorEvent);
    void onDebuggedProcessThreadTerminatedEvent(const apThreadTerminatedEvent& threadTerminatedEvent);
    void onInfrastructureFailureEvent(const apInfrastructureFailureEvent& infraFailureEvent);
    void onExecutionModeChangedEvent(const apEvent& execChangedEvent);


    apEvent* getLastEventOfType(apEvent::EventType eventType);

public slots:

    // Override acListCtrl:
    virtual void onAboutToShowContextMenu();

protected slots:

    void onDebugProcessEventsSelected(QTableWidgetItem* pSelectedItem);
    void onDebugProcessEventsCurrentItemChanged(QTableWidgetItem* pCurrentItem, QTableWidgetItem* pPreviousItem);
    void clearView();

protected:

    // Overrides acListCtrl:
    virtual void focusInEvent(QFocusEvent* pEvent);

    // Icon Index:
    int apEventToIconIndex(const apEvent* pEvent);

    void focusLastItem();

    void createAndLoadImageList();
    void clearEventsList();
    void clearItemsData();
    void insertListItem(const gtString& itemString, const apEvent& event);

    void checkLoadedModule(const gtString& modulePath);
    void checkInterceptionFailure(const gtString& modulePath);
    void checkLoadedOpenGLModule(const gtString& modulePathLowerCase);
    void checkLoadedOpenGLESModule(const gtString& modulePathLowerCase);
    bool isGremedyOGLESServer(const gtString& modulePathLowerCase);
    bool isWindowsSystemOGLModule(const gtString& modulePathLowerCase);
    void handleForeignOpenGLESImplementation(const gtString& modulePathLowerCase);
    void handleESModuleWhileInOGLProject();
    void handleOGLModuleWhileInESProject();

    /// Extend the basic acListCtrl context menu:
    void extendContextMenu();

protected:
    // Used to identify interception failures:
    gtMap<apAPIConnectionType, bool> m_apiModuleLoaded;
    bool m_interceptionWarningShown;

    // Contains true iff our OpenGL server was loaded:
    bool _wasGremedyOGLServerLoaded;

    // Contains true iff our OpenGL ES server was loaded:
    bool _wasGremedyOGLESServerLoaded;

    // Contains true if we should display the event properties in the properties view:
    bool _shouldDisplayEventProperties;

    // Map hold an event type to icon id mapping:
    gtMap<apEvent::EventType, int> _eventTypeToIconIndexMap;

    // Icons:
    gtPtrVector<QPixmap*> _listIconsVec;

    /// Clear view action:
    QAction* m_pClearViewAction;

};


#endif  // End Guard
