//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwgDEBuggerAppWrapper.cpp
///
//==================================================================================

//------------------------------ gwgDEBuggerAppWrapper.h ------------------------------

#include <QtWidgets>
#include <Qsci/qsciscintilla.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeHandler.h>
#include <AMDTGpuDebuggingComponents/Include/gdExecutionMode.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// Application Framework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/commands/afInitializeApplicationCommand.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdProjectSettingsExtension.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdGlobalDebugSettingsPage.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeHandler.h>

// Local:
#include <AMDTGpuDebugging/Include/gwgDEBuggerAppWrapperDLLBuild.h>
#include <AMDTGpuDebugging/Include/gwKernelWorkItemToolbar.h>
#include <AMDTGpuDebugging/Include/gwgDEBuggerAppWrapper.h>
#include <AMDTGpuDebugging/Include/gwStringConstants.h>
#include <src/gwApplicationCommands.h>
#include <src/gwDebugViewsCreator.h>
#include <src/gwEventObserver.h>
#include <src/gwImagesAndBuffersActionsCreator.h>
#include <src/gwImagesAndBuffersMDIViewCreator.h>
#include <src/gwMenuActionsExecutor.h>
#include <src/gwStatisticsActionsExecutor.h>

// Static members:
gwgDEBuggerAppWrapper* gwgDEBuggerAppWrapper::m_spMySingleInstance = NULL;
gwDebugViewsCreator* gwgDEBuggerAppWrapper::m_pDebugViewsCreator = NULL;
gwImagesAndBuffersMDIViewCreator* gwgDEBuggerAppWrapper::m_pImageBuffersMDIViewCreator = NULL;
gwEventObserver* gwgDEBuggerAppWrapper::m_pApplicationEventObserver = NULL;
gwStatisticsActionsExecutor* gwgDEBuggerAppWrapper::m_pStatisticsActionsExecutor = NULL;
gwKernelWorkItemToolbar* gwgDEBuggerAppWrapper::m_pKernelWorkItemToolbar = NULL;
gdProjectSettingsExtension* gwgDEBuggerAppWrapper::m_pProjectSettingsExtension = NULL;
gdGlobalDebugSettingsPage* gwgDEBuggerAppWrapper::m_spGlobalDebugSettingsPage = NULL;
gdExecutionMode* gwgDEBuggerAppWrapper::m_pExecutionMode = NULL;
bool gwgDEBuggerAppWrapper::s_loadEnabled = false;

// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::gwgDEBuggerAppWrapper
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        20/7/2011
// ---------------------------------------------------------------------------
gwgDEBuggerAppWrapper::gwgDEBuggerAppWrapper()
{

}
// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::instance
// Description:
// Return Val:  gwgDEBuggerAppWrapper&
// Author:      Sigal Algranaty
// Date:        20/7/2011
// ---------------------------------------------------------------------------
gwgDEBuggerAppWrapper& gwgDEBuggerAppWrapper::instance()
{
    // If this class single instance was not already created:
    if (m_spMySingleInstance == NULL)
    {
        // Create it:
        m_spMySingleInstance = new gwgDEBuggerAppWrapper;
        GT_ASSERT(m_spMySingleInstance);
    }

    return *m_spMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        initialize
// Description: Entry point for initialize
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        17/7/2011
// ---------------------------------------------------------------------------
void gwgDEBuggerAppWrapper::initialize()
{
    // Initialize the application command instance:
    gwApplicationCommands* pApplicationCommands = new gwApplicationCommands;


    // Register the application command single instance:
    bool rcRegsister = gdApplicationCommands::registerGDInstance(pApplicationCommands);
    GT_ASSERT_EX(rcRegsister, L"Only single instance of gdApplicationCommands should be registered");

    // Initialize the API package:
    bool rcAPI = gaIntializeAPIPackage(/*_shouldInitPerformanceCounters*/ false);
    GT_ASSERT_EX(rcAPI, GD_STR_LogMsg_FailedToInitAPI);

    // 7/18/2012 Gilad: the execution modes needs to register before the gwEventObserver
    // register the execution mode manager
    m_pExecutionMode = new gdExecutionMode;
    m_pExecutionMode->SetModeEnabled(s_loadEnabled);

    afExecutionModeManager::instance().registerExecutionMode(m_pExecutionMode);

    // Create an event observer:
    m_pApplicationEventObserver = new gwEventObserver;


    // Create the main menu actions creator:
    gwMenuActionsExecutor* pActionsCreator = new gwMenuActionsExecutor;


    // Register the actions creator:
    afQtCreatorsManager::instance().registerActionExecutor(pActionsCreator);

    // Create a debug views creator:
    m_pDebugViewsCreator = new gwDebugViewsCreator;


    // Initialize:
    m_pDebugViewsCreator->initialize();

    // Register the views creator:
    afQtCreatorsManager::instance().registerViewCreator(m_pDebugViewsCreator);

    // Create a views creator:
    m_pImageBuffersMDIViewCreator = new gwImagesAndBuffersMDIViewCreator;


    // Initialize:
    m_pImageBuffersMDIViewCreator->initialize();

    // Register the views creator:
    afQtCreatorsManager::instance().registerViewCreator(m_pImageBuffersMDIViewCreator);

    // Create the statistics actions creator:
    m_pStatisticsActionsExecutor = new gwStatisticsActionsExecutor;


    // Register the actions creator:
    afQtCreatorsManager::instance().registerActionExecutor(m_pStatisticsActionsExecutor);

    // Create and register the project settings object:
    m_pProjectSettingsExtension = new gdProjectSettingsExtension;

    afProjectManager::instance().registerProjectSettingsExtension(m_pProjectSettingsExtension);

    // Create and register the global settings page:
    m_spGlobalDebugSettingsPage = new gdGlobalDebugSettingsPage;

    afGlobalVariablesManager::instance().registerGlobalSettingsPage(m_spGlobalDebugSettingsPage);

    // Initialize the properties events handler:
    gdPropertiesEventObserver::instance();
}

// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::propertiesView
// Description: Return the application properties view
// Return Val:  gdPropertiesEventObserver*
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
gdPropertiesEventObserver* gwgDEBuggerAppWrapper::propertiesView()
{
    gdPropertiesEventObserver* pRetVal = NULL;
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->propertiesView();
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::memoryView
// Description: Return the application memory view
// Return Val:  gdMemoryView*
// Author:      Sigal Algranaty
// Date:        18/9/2011
// ---------------------------------------------------------------------------
gdMemoryView* gwgDEBuggerAppWrapper::memoryView()
{
    gdMemoryView* pRetVal = NULL;
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->memoryView();
    }

    return pRetVal;
}



// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::statisticsPanel
// Description: Return the application statistics panel
// Return Val:  gdStatisticsPanel*
// Author:      Sigal Algranaty
// Date:        18/9/2011
// ---------------------------------------------------------------------------
gdStatisticsPanel* gwgDEBuggerAppWrapper::statisticsPanel()
{
    gdStatisticsPanel* pRetVal = NULL;
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->statisticsPanel();
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::callsHistoryPanel
// Description: Return the application calls history panel
// Return Val:  gdAPICallsHistoryPanel*
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
gdAPICallsHistoryPanel* gwgDEBuggerAppWrapper::callsHistoryPanel()
{
    gdAPICallsHistoryPanel* pRetVal = NULL;
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->callsHistoryPanel();
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::callStackView
// Description: Return the application call stack view
// Return Val:  gdCallStackView*
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
gdCallStackView* gwgDEBuggerAppWrapper::callStackView()
{
    gdCallStackView* pRetVal = NULL;
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->callStackView();
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::debuggedProcessEventsView
// Description: Return the application debugged process events view
// Return Val:  gdDebuggedProcessEventsView*
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
gdDebuggedProcessEventsView* gwgDEBuggerAppWrapper::debuggedProcessEventsView()
{
    gdDebuggedProcessEventsView* pRetVal = NULL;
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->debuggedProcessEventsView();
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::stateVariablesView
// Description: Return the application state variables view
// Return Val:  gdStateVariablesView*
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
gdStateVariablesView* gwgDEBuggerAppWrapper::stateVariablesView()
{
    gdStateVariablesView* pRetVal = NULL;
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->stateVariablesView();
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::commandQueuesView
// Description: Return the application command queues view
// Return Val:  gdCommandQueuesView*
// Author:      Sigal Algranaty
// Date:        15/12/2011
// ---------------------------------------------------------------------------
gdCommandQueuesView* gwgDEBuggerAppWrapper::commandQueuesView()
{
    gdCommandQueuesView* pRetVal = NULL;
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->commandQueuesView();
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::multiWatchView1
// Description: Return the application multi watch view 1
// Return Val:  gdMultiWatchView*
// Author:      Sigal Algranaty
// Date:        24/8/2011
// ---------------------------------------------------------------------------
gdMultiWatchView* gwgDEBuggerAppWrapper::multiWatchView1()
{
    gdMultiWatchView* pRetVal = NULL;
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->multiWatchView1();
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::multiWatchView2
// Description: Return the application multi watch view 2
// Return Val:  gdMultiWatchView*
// Author:      Sigal Algranaty
// Date:        24/8/2011
// ---------------------------------------------------------------------------
gdMultiWatchView* gwgDEBuggerAppWrapper::multiWatchView2()
{
    gdMultiWatchView* pRetVal = NULL;
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->multiWatchView2();
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::multiWatchView3
// Description: Return the application multi watch view 3
// Return Val:  gdMultiWatchView*
// Author:      Sigal Algranaty
// Date:        24/8/2011
// ---------------------------------------------------------------------------
gdMultiWatchView* gwgDEBuggerAppWrapper::multiWatchView3()
{
    gdMultiWatchView* pRetVal = NULL;
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->multiWatchView3();
    }

    return pRetVal;
}



// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::kernelWorkItemToolbar
// Description: Return the application kernel work item toolbar
// Return Val:  gwKernelWorkItemToolbar*
// Author:      Sigal Algranaty
// Date:        15/9/2011
// ---------------------------------------------------------------------------
gwKernelWorkItemToolbar* gwgDEBuggerAppWrapper::kernelWorkItemToolbar()
{
    return m_pKernelWorkItemToolbar;
}

// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::breakpointsView
// Description: Get the application breakpoints view
// Return Val:  gdBreakpointsView*
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
gdBreakpointsView* gwgDEBuggerAppWrapper::breakpointsView()
{
    gdBreakpointsView* pRetVal = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->breakpointsView();
    }
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::watchView
// Description: Get the application watch view
// Return Val:  gdWatchView*
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
gdWatchView* gwgDEBuggerAppWrapper::watchView()
{
    gdWatchView* pRetVal = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->watchView();
    }
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::localsView
// Description: Get the application locals view
// Return Val:  gdLocalsView*
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
gdLocalsView* gwgDEBuggerAppWrapper::localsView()
{
    gdLocalsView* pRetVal = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDebugViewsCreator != NULL)
    {
        pRetVal = m_pDebugViewsCreator->localsView();
    }
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::initializeIndependentWidgets
// Description: This function initializes all the widget items that are not
//              registered with the creators mechanism. These widgets are
//              responsible for its own callbacks and string
// Author:      Sigal Algranaty
// Date:        15/9/2011
// ---------------------------------------------------------------------------
void gwgDEBuggerAppWrapper::initializeIndependentWidgets()
{
    // Get the main application window:
    afMainAppWindow* pAppMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pAppMainWindow != NULL)
    {
        // Initialize kernel work item toolbar:
        m_pKernelWorkItemToolbar = new gwKernelWorkItemToolbar(pAppMainWindow);


        m_pKernelWorkItemToolbar->setVisible(true);

        // Get the image and buffers toolbar
        gtString toolbarName = AF_STR_ImagesAndBuffersToolbar;
        QString toolbarNameQstr = QString::fromWCharArray(toolbarName.asCharArray());
        toolbarNameQstr.append(AF_STR_toolbarPostfix);
        QToolBar* pImagesAndBuffersToolbar = pAppMainWindow->findChild<QToolBar*>(toolbarNameQstr);

        pAppMainWindow->addToolbar(m_pKernelWorkItemToolbar, (acToolBar*)pImagesAndBuffersToolbar);
    }
}


extern "C"
{
    // ---------------------------------------------------------------------------
    // Name:        CheckValidity
    // Description: check validity of the plug in
    // Return Val:  int - error type
    //              0 = no error
    //              != 0 error value and error string contains the error
    // Author:      Gilad Yarnitzky
    // Date:        15/7/2014
    // ---------------------------------------------------------------------------
    int CheckValidity(gtString& errString)
    {
        int retVal = 0;

        errString = L"";
        // check if we are remotely connected
        gtString sshVarEnvName(L"SSH_TTY");
        gtString sshVarEnvVal;
        osGetCurrentProcessEnvVariableValue(sshVarEnvName, sshVarEnvVal);

        if (!sshVarEnvVal.isEmpty())
        {
            retVal = 1;
            errString = GW_STR_componentSSHLoadError;
        }

        gwgDEBuggerAppWrapper::s_loadEnabled = (0 == retVal);

        return retVal;
    }

    // ---------------------------------------------------------------------------
    // Name:        initialize
    // Description: Entry point for initialize
    // Return Val:  void
    // Author:      Gilad Yarnitzky
    // Date:        17/7/2011
    // ---------------------------------------------------------------------------
    void initialize()
    {
        gwgDEBuggerAppWrapper::instance().initialize();
    }


    // ---------------------------------------------------------------------------
    // Name:        initializeIndependentWidgets
    // Description: initialize other gui items that can be done only after main window is alive
    // Return Val:  void GW_API
    // Author:      Gilad Yarnitzky
    // Date:        26/1/2012
    // ---------------------------------------------------------------------------
    void GW_API initializeIndependentWidgets()
    {
        gwgDEBuggerAppWrapper::instance().initializeIndependentWidgets();
    }

}; // extern "C"

