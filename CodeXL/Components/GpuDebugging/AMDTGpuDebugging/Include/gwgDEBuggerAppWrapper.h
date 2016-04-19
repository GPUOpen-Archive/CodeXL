//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwgDEBuggerAppWrapper.h
///
//==================================================================================

//------------------------------ gwgDEBuggerAppWrapper.h ------------------------------

#ifndef __GWGDEBUGGERAPPWRAPPER_H
#define __GWGDEBUGGERAPPWRAPPER_H

// Forward declarations:
class gdPropertiesEventObserver;
class gdDebugApplicationTreeHandler;
class gdMemoryView;
class gdAPICallsHistoryPanel;
class gdCallStackView;
class gdCommandQueuesView;
class gdDebuggedProcessEventsView;
class gdMultiWatchView;
class gdStateVariablesView;
class gdStatisticsPanel;
class gdPerformanceGraphView;
class gdPerformanceDashboardView;
class gdProjectSettingsExtension;
class gwEventObserver;
class gwViewsCreator;
class gwDebugViewsCreator;
class gwStatisticsActionsExecutor;
class gwRecentProjectsActionsExecutor;
class gwImagesAndBuffersMDIViewCreator;
class gwKernelWorkItemToolbar;
class gdGlobalDebugSettingsPage;
class gdBreakpointsView;
class gdLocalsView;
class gdWatchView;
class gdExecutionMode;

// AMDTGpuDebuggingComponents:
class gdDebugApplicationTreeData;

// Local:
#include <AMDTGpuDebugging/Include/gwgDEBuggerAppWrapperDLLBuild.h>

class GW_API gwgDEBuggerAppWrapper
{
public:

    static gwgDEBuggerAppWrapper& instance();
    void initialize();
    void initializeIndependentWidgets();

    // Accessors for the view objects:
    static gdPropertiesEventObserver* propertiesView();
    static gdMemoryView* memoryView();
    static gdAPICallsHistoryPanel* callsHistoryPanel();
    static gdCallStackView* callStackView();
    static gdDebuggedProcessEventsView* debuggedProcessEventsView();
    static gdStateVariablesView* stateVariablesView();
    static gdCommandQueuesView* commandQueuesView();
    static gdStatisticsPanel* statisticsPanel();
    static gdMultiWatchView* multiWatchView1();
    static gdMultiWatchView* multiWatchView2();
    static gdMultiWatchView* multiWatchView3();
    static gwKernelWorkItemToolbar* kernelWorkItemToolbar();
    static gdBreakpointsView* breakpointsView();
    static gdWatchView* watchView();
    static gdLocalsView* localsView();

    // marks if the prerequisite of this plugin were met
    static bool s_loadEnabled;

protected:

    // Do not allow the use of my default constructor:
    gwgDEBuggerAppWrapper();

    static gwgDEBuggerAppWrapper* m_spMySingleInstance;

    // Contain the object that created the debug views:
    static gwDebugViewsCreator* m_pDebugViewsCreator;

    // Contain the image / buffers MDI view creator:
    static gwImagesAndBuffersMDIViewCreator* m_pImageBuffersMDIViewCreator;

    // Recent projects actions executor:
    static gwRecentProjectsActionsExecutor* m_pRecentProjectsActionsExecutor;

    // Statistics actions executor:
    static gwStatisticsActionsExecutor* m_pStatisticsActionsExecutor;

    // Kernel work items toolbar:
    static gwKernelWorkItemToolbar* m_pKernelWorkItemToolbar;

    // Project settings extension:
    static gdProjectSettingsExtension* m_pProjectSettingsExtension;

    // Global settings page:
    static gdGlobalDebugSettingsPage* m_spGlobalDebugSettingsPage;

    // Contain the application event observer:
    static gwEventObserver* m_pApplicationEventObserver;

    // global execution mode handler;
    static gdExecutionMode* m_pExecutionMode;
};

extern "C"
{
    // check validity of the plugin:
    int GW_API CheckValidity(gtString& errString);

    // initialize function for this wrapper:
    void GW_API initialize();

    // Initialize other items after main window creation:
    void GW_API initializeIndependentWidgets();
};


#endif //__GWGDEBUGGERAPPWRAPPER_H

