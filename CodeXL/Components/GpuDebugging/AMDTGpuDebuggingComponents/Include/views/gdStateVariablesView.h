//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStateVariablesView.h
///
//==================================================================================

//------------------------------ gdStateVariablesView.h ------------------------------

#ifndef __GDSTATEVARIABLESVIEW
#define __GDSTATEVARIABLESVIEW

// Pre-declarations:
class afGlobalVariableChangedEvent;
class apMonitoredObjectsTreeSelectedEvent;
class apDebuggedProcessRunSuspendedEvent;

//Qt
#include <QtWidgets>

// Infra
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdStateVariablesView: public acListCtrl
// General Description:
//  A viewer that lists a log of the graphics API function calls.
//  (OpenGL, OpenGL extensions, Win32 API graphics functions).
//
// Author:               Yaki Tebeka
// Creation Date:        1/11/2003
// ----------------------------------------------------------------------------------
class GD_API gdStateVariablesView: public acListCtrl, public apIEventsObserver
{
    Q_OBJECT

public:
    gdStateVariablesView(QWidget* pParent);
    virtual ~gdStateVariablesView();

    // Overrides apIEventsObserver:
    virtual const wchar_t* eventObserverName() const { return L"StateVariablesView"; };

    void deleteAllStateVariables();
    void AddStateVariable(gtString& variableName);

    void selectRows(const QList<int>& rowToSelect);
    bool areItemsSelected();
    int numberOfVariableRows();

public slots:
    virtual void onAboutToShowContextMenu();
    void onDeleteSelected();

protected slots:

    void onSaveSnapShot();
    void openStateVariableDialog(const QString& activeStateVar = "");
    void onItemClicked(QTableWidgetItem* pItem);
    void itemDoubleClicked(QTableWidgetItem* pSelectedWidgetItem);
    void onCurrentItemChanged(QTableWidgetItem* pCurrent, QTableWidgetItem* pPrevious);
    virtual void mouseDoubleClickEvent(QMouseEvent* pMouseEvent);

protected:
    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

    // Overrides QWidget:
    virtual void focusInEvent(QFocusEvent* pFocusEvent);

    // Overrides acListCtrl:
    void extendContextMenu();

    // A struct which we attach to each list item:
    struct gdStateVarListItemData
    {
        // The state variable value as string:
        gtString _variableValue;
    };

    // Debugged process events:
    void onProcessCreated();
    void onProcessTerminated();
    void onProcessCreationFailed();
    void onProcessRunSuspended(const apDebuggedProcessRunSuspendedEvent& event);
    void onProcessRunResumed();
    void onGlobalVariableChanged(const afGlobalVariableChangedEvent& event);
    void onTreeItemSelection(const apMonitoredObjectsTreeSelectedEvent& eve);

    // wxWidgets events:
    void setListCtrlColumns();
    void updateStateVariablesList();

protected:

    // Context menu additional actions:
    QAction* m_pShowDialogAction;
    QAction* m_pExportStateVariablesAction;

    // Contain true while we're processing double click event:
    bool m_whileDoubleClick;

    // The current displayed context:
    apContextID _activeContextId;

};

#endif  // __GDSTATEVARIABLESVIEW
