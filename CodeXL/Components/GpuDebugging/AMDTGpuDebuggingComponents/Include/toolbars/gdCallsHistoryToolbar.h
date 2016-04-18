//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdCallsHistoryToolbar.h
///
//==================================================================================

//------------------------------ gdCallsHistoryToolbar.h ------------------------------

#ifndef __GDCALLSHISTORYTOOLBAR
#define __GDCALLSHISTORYTOOLBAR

// Qt:
#include <QAction>

// Pre-declarations:
class afGlobalVariableChangedEvent;
class apMonitoredObjectsTreeSelectedEvent;
class gdApplicationCommands;

// Infra:
#include <AMDTApplicationComponents/Include/acToolBar.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdCallsHistoryToolbar : public acToolBar
// General Description: The "Calls History" toolbar.
// Author:               Avi Shapira
// Creation Date:        6/5/2004
// ----------------------------------------------------------------------------------
class GD_API gdCallsHistoryToolbar : public acToolBar, public apIEventsObserver
{
    Q_OBJECT

public:
    gdCallsHistoryToolbar(QWidget* pParent);

    virtual ~gdCallsHistoryToolbar();

    // Overrides apIEventsObserver:
    virtual const wchar_t* eventObserverName() const { return L"CallsHistoryToolbar"; };

    void onUpdateToolbar();

protected slots:

    void addToolbarTools();

    // Command buttons status enabling:
    void onUpdateRecord(bool& shouldEnable, bool& shouldCheck);
    void onUpdateOpenRecord(bool& shouldEnable);
    void onUpdateMarker(bool& shouldEnable);

    // Command buttons pressed handling:
    void onRecordingClick();
    void onOpenRecordedFileClick();
    void onNextMarkerClick();
    void onPreviousMarkerClick();

protected:

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

private:
    // Do not allow the use of:
    gdCallsHistoryToolbar();
    gdCallsHistoryToolbar(const gdCallsHistoryToolbar& other);

    void onGlobalVariableChanged(const afGlobalVariableChangedEvent& event);
    void onTreeItemSelection(const apMonitoredObjectsTreeSelectedEvent& eve);

protected:

    // Application commands instance:
    gdApplicationCommands* _pApplicationCommands;

    // Actions:
    QAction* _pRecordAction;
    QAction* _pOpenRecordAction;
    QAction* _pNextMarkerAction;
    QAction* _pPrevMarkerAction;

    apContextID _selectedContextId;
    int _amountOfRenderContexts;
    osFilePath _recordFilePath;

    // Contains true iff the debugged process is suspended:
    bool _isDebuggedProcessSuspended;
    bool _isDebuggedProcessRunning;
    bool _isRecordFileExist;
};

#endif  // __GDCALLSHISTORYTOOLBAR
