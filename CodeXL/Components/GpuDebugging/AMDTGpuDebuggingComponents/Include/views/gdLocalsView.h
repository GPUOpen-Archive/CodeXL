//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdLocalsView.h
///
//==================================================================================

//------------------------------ gdLocalsView.h ------------------------------

#ifndef __GDLOCALSVIEW_H
#define __GDLOCALSVIEW_H

// Qt:
#include <QtWidgets>

// Forward declaration:
class gtString;
class gdApplicationCommands;

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          GD_API gdLocalsView : public acListCtrl, public apIEventsObserver
// General Description: Displays the local variables currently available in the debug
//                      session.
// Author:              Uri Shomroni
// Creation Date:       8/9/2011
// ----------------------------------------------------------------------------------
class GD_API gdLocalsView : public acTreeCtrl, public apIEventsObserver
{
    Q_OBJECT

public:
    gdLocalsView(QWidget* pParent);
    virtual ~gdLocalsView();

    // Overrides apIEventsObserver:
    virtual const wchar_t* eventObserverName() const { return L"LocalsView"; };

protected:
    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

protected slots:

    void onAddWatch();
    void onAddMultiWatch();
    void onAboutToShowTextContextMenu();
    void onItemSelected(QTreeWidgetItem* pCurrent, QTreeWidgetItem* pPrevious);

private:
    void populateLocalsList();
    void updateValueColumnHeader();
    void extendContextMenu();
    void updateCallStackDepth(const apEvent& eve);

    void recursivelyAddLocalItemChildren(QTreeWidgetItem* pItem, const gtString& currentVariableName, const int currentWorkItemCoord[3]);

protected:
    // Actions for context menu:
    QAction* m_pAddWatchAction;
    QAction* m_pAddMultiWatchAction;

    int m_stackDepth;

    gdApplicationCommands* m_pApplicationCommands;
};

#endif //__GDLOCALSVIEW_H

