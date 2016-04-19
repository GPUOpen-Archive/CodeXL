//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdCallStackView.h
///
//==================================================================================

//------------------------------ gdCallStackView.h ------------------------------

#ifndef __GDCALLSTACKVIEW
#define __GDCALLSTACKVIEW

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Forward decelerations:
class apDebuggedProcessRunSuspendedEvent;
class afGlobalVariableChangedEvent;
class apExceptionEvent;

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// Local
#include <AMDTGpuDebuggingComponents/Include/views/gdCallsStackListCtrl.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdCallStackView: public wxListCtrl
// General Description:
//  A viewer that monitors the debugged process events.
//  It logs: Process creation and termination, dll load and unload, exceptions, etc.
//
// Author:               Yaki Tebeka
// Creation Date:        1/11/2003
// ----------------------------------------------------------------------------------
class GD_API gdCallStackView: public gdCallsStackListCtrl , public apIEventsObserver
{
    Q_OBJECT
public:

    gdCallStackView(QWidget* pParent);
    virtual ~gdCallStackView();

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"CallStackView"; };

private:
    // Debugged process events:
    void onProcessCreated();
    void onProcessRunSuspended(const apDebuggedProcessRunSuspendedEvent& event);
    void onProcessRunResumed();
    void onException(const apExceptionEvent& event);
    void onGlobalVariableChanged(const afGlobalVariableChangedEvent& event);

private:
    void upadteToSelectedThreadCallStack();
    void updateCallsStackList(osThreadId threadId);
    void updateKernelCallStackList(int wavefrontUserIndex);
};


#endif  // __GDCALLSTACKVIEW
