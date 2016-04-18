//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwDebugViewsCreator.h
///
//==================================================================================

//------------------------------ gwDebugViewsCreator.h ------------------------------

#ifndef __GWDEBUGVIEWSCREATOR_H
#define __GWDEBUGVIEWSCREATOR_H

// Forward declaration:
class gdApplicationCommands;
class gdCommandQueuesView;
class gdAPICallsHistoryPanel;
class gdDebuggedProcessEventsView;
class gdPropertiesEventObserver;
class gdBreakpointsView;
class gdLocalsView;
class gdWatchView;
class acListCtrl;
class acVirtualListCtrl;

// Infra:
#include <AMDTApplicationFramework/Include/afQtViewCreatorAbstract.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// ----------------------------------------------------------------------------------
// Class Name:           gwDebugViewsCreator : public afQtViewCreatorAbstract
// General Description:  A class handling the creation of debug views
// Author:               Sigal Algranaty
// Creation Date:        6/9/2011
// ----------------------------------------------------------------------------------
class gwDebugViewsCreator : public afQtViewCreatorAbstract
{
public:
    gwDebugViewsCreator();
    ~gwDebugViewsCreator();

    // Virtual functions that needs to be implemented:

    // Get the title of the created view:
    virtual void titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand);

    // Get the execution mode of the view:
    virtual gtString executionMode(int viewIndex) { (void)(viewIndex); return GD_STR_executionMode; }

    // Get the menu position in the menu view:
    virtual gtString modeMenuPosition(int viewIndex, afActionPositionData& positionData);

    // If one of the views contain toolbar, this function updates it's commands:
    virtual void updateViewToolbarCommands();

    // Get the associated toolbar string:
    virtual gtString associatedToolbar(int viewIndex);

    // Get view type:
    virtual afViewCreatorAbstract::afViewType type(int viewIndex);

    // Get the docking area:
    virtual int dockArea(int viewIndex);

    // Get the dock view we want to dock with. "-" prefix means below that view
    virtual gtString dockWith(int viewIndex);

    // True iff the view is creating views in run time:
    virtual bool isDynamic() { return false;};

    // Docable widget features:
    virtual QDockWidget::DockWidgetFeatures dockWidgetFeatures(int viewIndex);

    // Initialize the creator:
    virtual void initialize();

    // Get the initial size:
    virtual QSize initialSize(int viewIndex);

    // Get the initial visibility of the view:
    virtual bool visibility(int viewIndex);

    // Get the initial activity of the view:
    virtual bool initiallyActive(int viewIndex);

    // Should add a separator after view command?  - false by default, override when a separator is needed:
    virtual bool addSeparator(int viewIndex);

    // Get number of views that can be created by this creator:
    virtual int amountOfViewTypes();

    // Create the inner view:
    virtual bool createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent);

    // Handle the action when it is triggered
    virtual void handleTrigger(int viewIndex, int actionIndex);

    void handleListControlAction(acListCtrl* pListCtrl, int actionIndex);
    void handleVirtualListControlAction(acVirtualListCtrl* pVirtualListCtrl, int actionIndex);

    // handle UI update
    virtual void handleUiUpdate(int viewIndex, int actionIndex);

    // The created views:
    gdPropertiesEventObserver* propertiesView() {return m_pPropertiesView;};
    gdBreakpointsView* breakpointsView() const { return m_pBreakpointsView;};
    gdLocalsView* localsView() const { return m_pLocalsView;}
    gdWatchView* watchView() const { return m_pWatchView;}
    gdStatisticsPanel* statisticsPanel() {return m_pStatisticsPanel;};
    gdStateVariablesView* stateVariablesView() {return m_pStateVariablesView;};
    gdCommandQueuesView* commandQueuesView() {return m_pCommandQueuesView;};
    gdAPICallsHistoryPanel* callsHistoryPanel() {return m_pAPICallsHistoryPanel;};
    gdDebuggedProcessEventsView* debuggedProcessEventsView() {return m_pDebuggedProcessEventsView;};
    gdCallStackView* callStackView() {return m_pCallStackView;};
    gdMemoryView* memoryView() {return m_pMemoryView;};
    gdMultiWatchView* multiWatchView1() {return m_pMultiWatchView1;};
    gdMultiWatchView* multiWatchView2() {return m_pMultiWatchView2;};
    gdMultiWatchView* multiWatchView3() {return m_pMultiWatchView3;};

protected:

    enum GD_DEBUG_VIEW_INDEX
    {
        gdAPICallsHistoryPanelIndex = 0,
        gdDebuggedProcessEventsViewIndex,
        gdCallStackViewIndex,

        gdLocalsViewIndex,
        gdWatchViewIndex,
        gdStateVariablesViewIndex,
        gdMultiWatch1Index,
        gdMultiWatch2Index,
        gdMultiWatch3Index,

        gdBreakpointsViewIndex,

        gdMemoryViewIndex,
        gdStatisticsPanelIndex,

        // Profiling: gdCommandQueuesViewIndex,
        gdDebugAmountOfViews
    };

    // Array with the view captions:
    gtString m_viewCaptions[gdDebugAmountOfViews];
    gtString m_viewMenuCommands[gdDebugAmountOfViews];

    // Array with the views docking areas:
    afDockingAreaFlag m_viewDockingAreas[gdDebugAmountOfViews];

    // Array with the views docking features:
    QDockWidget::DockWidgetFeatures m_viewDockingFeatures[gdDebugAmountOfViews];

    // Application command handler:
    gdApplicationCommands* m_pApplicationCommandsHandler;

    // The created views:
    gdPropertiesEventObserver* m_pPropertiesView;
    gdBreakpointsView* m_pBreakpointsView;
    gdLocalsView* m_pLocalsView;
    gdWatchView* m_pWatchView;
    gdStatisticsPanel* m_pStatisticsPanel;
    gdStateVariablesView* m_pStateVariablesView;
    gdCommandQueuesView* m_pCommandQueuesView;
    gdAPICallsHistoryPanel* m_pAPICallsHistoryPanel;
    gdDebuggedProcessEventsView* m_pDebuggedProcessEventsView;
    gdMemoryView* m_pMemoryView;
    gdCallStackView* m_pCallStackView;

    gdMultiWatchView* m_pMultiWatchView1;
    gdMultiWatchView* m_pMultiWatchView2;
    gdMultiWatchView* m_pMultiWatchView3;

};

#endif //__GWDEBUGVIEWSCREATOR_H

