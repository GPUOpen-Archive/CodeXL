//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdMemoryViewBase.h
///
//==================================================================================

//------------------------------ gdMemoryViewBase.h ------------------------------

#ifndef __GDMEMORYVIEWBASE
#define __GDMEMORYVIEWBASE

// Forward Declarations:
class apBreakpointHitEvent;
class apDebuggedProcessRunSuspendedEvent;
class acChartWindow;
class gdAllocatedObjectsCreationStackView;
class gdMemoryAnalysisDetailsView;
class gdDebugApplicationTreeHandler;


// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTAPIClasses/Include/apBreakReason.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>


// Constants for the layout of the memory views:
#define GD_MEMORY_VIEWER_LIST_MIN_HEIGHT 150
#define GD_MEMORY_VIEWER_MIN_WIDTH 300
#define GD_MEMORY_VIEWER_DEFAULT_CHART_SIZE 150
#define GD_MEMORY_VIEWER_TREE_DEFAULT_WIDTH 200
#define GD_MEMORY_VIEWER_BOTTOM_VIEWS_DEFAULT_HEIGHT GD_MEMORY_VIEWER_DEFAULT_CHART_SIZE
#define GD_MEMORY_VIEWER_VIEWS_MIN_SIZE 50
#define GD_MEMORY_VIEWER_RENDER_CONTEXTS_HORIZONTAL_SPACING 5


// ----------------------------------------------------------------------------------
// Class Name:           gdMemoryViewBase: public acFrame,
//                       public apIEventsObserver
// General Description:  The memory analysis viewer.
// Author:               Sigal Algranaty
// Creation Date:        22/9/2008
// ----------------------------------------------------------------------------------
class GD_API gdMemoryViewBase : public apIEventsObserver, public afBaseView
{
public:
    // Constructor
    gdMemoryViewBase(afProgressBarWrapper* pProgressBar, gdDebugApplicationTreeHandler* pObjectsTree);

    // Destructor
    ~gdMemoryViewBase();

    // Overrides apIEventsObserver:
    virtual const wchar_t* eventObserverName() const { return L"gdMemoryViewBase"; };

    virtual bool isShown() {return false;};

    virtual bool setStatusText(const gtString& progressText, bool showGauge = true) { (void)(progressText); (void)(showGauge); return false;};

    // Overrides afBaseView:
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);
    virtual void onEdit_Copy();
    virtual void onEdit_SelectAll();


protected:

    // Virtual function:
    // Raise the view and show it as top window:
    virtual void raiseView() {};

    void initializeColors();

    // Layout creation functions:
    virtual void clearAllViewers(bool displayNAMessage = false, bool forgetLastSelection = true);
    virtual bool updateMemoryAnalysisViewers();
    virtual void updateChart(const afApplicationTreeItemData* pItemData);
    virtual void onGlobalVariableChanged(const apEvent& event) { (void)(event); };
    virtual void onTreeItemSelection(const apMonitoredObjectsTreeSelectedEvent& eve) { (void)(eve); };
    virtual void onExecutionModeChangedEvent(const apEvent& eve);

    // Debugged process event handlers:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    void onProcessRunSuspended(const apDebuggedProcessRunSuspendedEvent& event);
    void onProcessStarted();
    void onProcessTerminated();

    // List click event handlers:
    void onListColumnClicked(int clickedItemIndex);
    void onListItemActivated(QTableWidgetItem* pItem);
    void onListItemSelected(QTableWidgetItem* pItem);
    void onListItemDeselected(QTableWidgetItem* pItem);

    // Pie chart update:
    void updatePieChart(afApplicationTreeItemData* pMemoryItemData);
    void highlightSelectedItemsInChart();
    void clearPieChart();

    // Item callstack:
    void displayItemCallStack(const afApplicationTreeItemData* pMemoryItemData);
    void displaySelectedItemsMemorySize();

protected:

    // Object creation calls stack view:
    gdAllocatedObjectsCreationStackView* _pAllocatedObjectsCreationStackView;

    // Total statistics view object:
    gdMemoryAnalysisDetailsView* _pMemoryDetailsView;

    // Memory Chart:
    acChartWindow* _pChartWindow;

    // A vector to hold the colors for the effective / redundant pie chart
    gtVector<unsigned long> _memoryObjectsColors;

    // Flag that is set to true while suspended process data update is done:
    bool _updatingOnProcessSuspension;

    // Is used to avoid feedback loops in Mac:
    bool _isAutomaticallySelectingTreeItem;

    // Is the debugged process suspended?
    bool _isDebuggedProcessSuspended;

    // Is the information in the viewer up-to-date?
    bool _isInfoUpdated;

    // Are we currently updating the viewers?
    bool _isDuringViewsUpdate;

    // Contain true iff I'm responsible for updating the objects tree:
    bool _isTreeUpdatedByMe;

    // Is used for WX bug workaround:
    QTreeWidgetItem* m_pLastSelectedTreeItemId;

};

#endif  // __GDMEMORYVIEWBASE
