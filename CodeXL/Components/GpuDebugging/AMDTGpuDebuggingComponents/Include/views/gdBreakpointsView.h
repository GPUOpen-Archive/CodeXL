//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdBreakpointsView.h
///
//==================================================================================

//------------------------------ gdBreakpointsView.h ------------------------------

#ifndef __GDBREAKPOINTSVIEW_H
#define __GDBREAKPOINTSVIEW_H

// Forward declaration:
class gdPropertiesEventObserver;

// Infra:
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          GD_API gdBreakpointsView : public acListCtrl, public apIEventsObserver
// General Description: Display breakpoints view in run time. Display the current
//                      breakpoints items in the debugged process
// Author:              Sigal Algranaty
// Creation Date:       6/9/2011
// ----------------------------------------------------------------------------------
class GD_API gdBreakpointsView : public acListCtrl, public apIEventsObserver
{
    Q_OBJECT

public:
    gdBreakpointsView(QWidget* pParent);
    virtual ~gdBreakpointsView();

    // Overrides apIEventsObserver:
    virtual const wchar_t* eventObserverName() const { return L"BreakpointsView"; };

public slots:
    // Override acListCtrl:
    virtual void onAboutToShowContextMenu();

protected:
    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

    // acListCtrl overrides:
    virtual void onBeforeRemoveRow(int row);

    // Utilities:
    bool updateBreakpoints();
    void initColumns();
    void onBreakpointHitEvent(const apBreakpointHitEvent& breakpointHitEvent);
    gtString breakpointHitEventToString(const apBreakpointHitEvent& breakpointHitEvent);
    void extendContextMenu();
    bool updateSingleBreakpoint(int updatedBreakpointIndex);
    void addDoubleClickMessageRow();

    void displayCurrentlySelectedBreakpoints();

protected slots:
    void openBreakpintsDialog();
    void onItemClicked(QTableWidgetItem* pItem);
    virtual void onItemsCheckedChange();
    virtual void mouseDoubleClickEvent(QMouseEvent* pMouseEvent);
    void onBreakpointSelected(QTableWidgetItem* pCurrent, QTableWidgetItem* pPrevious);

protected:
    // Icons:
    QPixmap* m_pDisabledBreakpintPixmap;
    QPixmap* m_pEnabledBreakpintPixmap;

    // Context menu actions:
    QAction* m_pShowDialogAction;
};


#endif //__GDBREAKPOINTSVIEW_H

