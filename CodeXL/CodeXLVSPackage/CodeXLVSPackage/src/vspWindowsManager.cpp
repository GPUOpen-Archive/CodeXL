//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspWindowsManager.cpp
///
//==================================================================================

//------------------------------ vspWindowsManager.cpp ------------------------------

#include "stdafx.h"

// QT
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/dialogs/afHelpAboutDialog.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>
#include <AMDTApplicationFramework/Include/dialogs/afGlobalSettingsDialog.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afSoftwareUpdaterWindow.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/afMessageBox.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdBreakpointsDialog.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdAPICallsHistoryView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageAndBufferView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMemoryView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStateVariablesView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMultiWatchView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsView.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdAPICallsHistoryPanel.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdStatisticsPanel.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// Local
#include <src/vspImagesAndBuffersManager.h>
#include <src/vspWindowsManager.h>
#include <src/vspSaveListDialog.h>
#include <CodeXLVSPackage/Include/vspCommandIDs.h>

// VS:
#include <Include/vspStringConstants.h>

#define TOOL_TIP_DELAY 400

// Static members initializations:
vspWindowsManager* vspWindowsManager::_pMySingleInstance = NULL;
IVscWindowsManagerOwner* vspWindowsManager::_pOwner = NULL;

// Forward declaration:
LRESULT WINAPI mainOverrideWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI mainOverrideQtWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

// original window proc for the modal window:
static WNDPROC stOriginalWindowProc = NULL;
static QDialog* stQtDialogToActivate = NULL;
static UINT_PTR stTimerID = NULL;

// A handle to the dialog that should be immediately closed.
static QDialog* gs_pDialogForImmediateClose = NULL;
const unsigned int DIALOG_IMMEDIATE_CLOSURE_INTERVAL_MS = 0;

#define MODAL_DIALOG_TIMER 123
#define MODAL_DIALOG_IMMEDIATE_CLOSURE_TIMER 124

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::vspWindowsManager
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        21/9/2010
// ---------------------------------------------------------------------------
vspWindowsManager::vspWindowsManager(): _pAPICallsHistoryPanel(NULL),
    _pPropertiesView(NULL), _pMonitoredObjectsTree(NULL), _pStatisticsPanel(NULL), _pMemoryView(NULL),
    _pStateVariablesView(NULL), _piUIShell(NULL)
{
    for (int i = 0; i < VSP_AMOUNT_OF_MULTIWATCH_VIEWS; i++)
    {
        _pMultiWatchViews[i] = NULL;
    }
}



// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::~vspWindowsManager
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        21/9/2010
// ---------------------------------------------------------------------------
vspWindowsManager::~vspWindowsManager()
{
    // Clear the UI shell interface:
    setUIShell(NULL);

    for (int i = 0; i < VSP_AMOUNT_OF_MULTIWATCH_VIEWS; i++)
    {
        if (NULL != _pMultiWatchViews[i])
        {
            _pMultiWatchViews[i]->deleteLater();
        }

        _pMultiWatchViews[i] = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Return Val:  vspWindowsManager&
// Author:      Sigal Algranaty
// Date:        21/9/2010
// ---------------------------------------------------------------------------
vspWindowsManager& vspWindowsManager::instance()
{
    // If my single instance was not created yet - create it:
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new vspWindowsManager;
    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::setUIShell
// Description: Sets or clears the IVsUIShell interface for this class
// Author:      Uri Shomroni
// Date:        6/4/2011
// ---------------------------------------------------------------------------
void vspWindowsManager::setUIShell(IVsUIShell* piUIShell)
{
    // Release any previous interface:
    if (_piUIShell != NULL)
    {
        _piUIShell->Release();
        _piUIShell = NULL;
    }

    // Set the pointer:
    _piUIShell = piUIShell;

    // Retain the new interface:
    if (_piUIShell != NULL)
    {
        _piUIShell->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::showOptionsDialog
// Description: Show the OpenCL debugger options dialog
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        22/9/2010
// ---------------------------------------------------------------------------
void vspWindowsManager::showOptionsDialog()
{
    afGlobalSettingsDialog::instance().showDialog();
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::showSystemInformationDialog
// Description: Show the OpenCL debugger system information dialog
// Arguments:   afSystemInformationDialog::InformationTabs selectedTab - the dialog selected tab
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        26/1/2011
// ---------------------------------------------------------------------------
void vspWindowsManager::showSystemInformationDialog(afSystemInformationDialog::InformationTabs selectedTab)
{
    // Create an system information dialog:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        afSystemInformationDialog dialog(NULL, selectedTab);
        pApplicationCommands->showModal(&dialog);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::showAboutDialog
// Description: Show the OpenCL debugger about dialog
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        1/2/2011
// ---------------------------------------------------------------------------
void vspWindowsManager::showAboutDialog()
{
    // Create an options dialog:
    afHelpAboutDialog dialog(OS_VISUAL_STUDIO_PLUGIN_TYPE, NULL);
    showModal(&dialog);
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::showAboutDialog
// Description: Show the OpenCL debugger about dialog
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        1/2/2011
// ---------------------------------------------------------------------------
void vspWindowsManager::showCheckForUpdateDialog()
{
    // Create an options dialog:
    afSoftwareUpdaterWindow dlg;
    dlg.displayDialog();
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::viewHelp
// Description: view help using the hh.exe application
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        1/2/2011
// ---------------------------------------------------------------------------
void vspWindowsManager::viewHelp()
{
    // Get the help file:
    osFilePath CodeXLHelpPath;
    bool rc = CodeXLHelpPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_VS_PACKAGE_HELP_FILE);
    GT_IF_WITH_ASSERT_EX(rc, L"Could not find user guide file")
    {
        // Open the tutorial file:
        osFileLauncher fileLauncher(CodeXLHelpPath.asString());
        rc = fileLauncher.launchFile();
    }

    if (!rc)
    {
        acMessageBox::instance().critical(AF_STR_ErrorA, AF_STR_HelpFileLoadErrorMessage);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::viewTutorial
// Description: view tutorial using the hh.exe application
// Author:      Uri Shomroni
// Date:        10/7/2011
// ---------------------------------------------------------------------------
void vspWindowsManager::viewQuickStartGuide()
{
    // Get the help file:
    osFilePath CodeXLTutorialPath;
    bool rc = CodeXLTutorialPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_QUICK_START_FILE);
    GT_IF_WITH_ASSERT_EX(rc, L"Could not find tutorial file")
    {
        // Open the tutorial file:
        osFileLauncher fileLauncher(CodeXLTutorialPath.asString());
        rc = fileLauncher.launchFile();
    }

    if (!rc)
    {
        acMessageBox::instance().critical(AF_STR_ErrorA, AF_STR_QuickStartFileLoadErrorMessage);
    }
}

void vspWindowsManager::OpenSample(afCodeXLSampleID sampleId)
{
    bool rc = true;

    // Find the sample dir ID and the solution file name according to the sample ID
    osFilePath::osApplicationSpecialDirectories sampleDirID = osFilePath::OS_CODEXL_TEAPOT_SAMPLE_PATH;
    gtString sampleSolutionFileName, sampleName;

    switch (sampleId)
    {
        case AF_TEAPOT_SAMPLE:
        {
            sampleDirID = osFilePath::OS_CODEXL_TEAPOT_SAMPLE_PATH;
            sampleSolutionFileName = AF_STR_CodeXLWindowTeapotSolutionName;
            sampleName = AF_STR_TeapotSampleProjectName;
        }
        break;

        case AF_MATMUL_SAMPLE:
        {
            sampleDirID = osFilePath::OS_CODEXL_MAT_MUL_SAMPLE_PATH;
            sampleSolutionFileName = AF_STR_CodeXLWindowMatMulSolutionName;
            sampleName = AF_STR_MatMulSampleProjectName;
        }
        break;

        case AF_D3D12MULTITHREADING_SAMPLE:
        {
            sampleDirID = osFilePath::OS_CODEXL_D3D_MT_SAMPLE_PATH;
            sampleSolutionFileName = AF_STR_CodeXLWindowD3D12MTSolutionName;
            sampleName = AF_STR_D3D12MultithreadingSampleProjectName;
        }
        break;

        default:
        {
            rc = false;
            GT_ASSERT_EX(false, L"Unsupported sample id");
        }
        break;
    }

    bool rcOpenSample = false;
    GT_IF_WITH_ASSERT(rc)
    {
        // Build the sample path from the sample dir ID
        osFilePath samplePath;
        rcOpenSample = samplePath.SetInstallRelatedPath(sampleDirID);
        gtString errorMessage;
        errorMessage.appendFormattedString(L"Could not find the samples root folder: %ls", samplePath.asString().asCharArray());
        GT_IF_WITH_ASSERT_EX(rcOpenSample, errorMessage.asCharArray())
        {
            // Get the VS version
            VsWindowsManagementMode vsVersion = GetVsWindowsManagementModeFromOwner();
            GT_IF_WITH_ASSERT_EX((vsVersion != VS_WMM_UNKNOWN), L"Unknown VS version")
            {
                // Find the suffix according to the VS version.
                gtString solutionSuffix;

                switch (vsVersion)
                {
                    case VS_WMM_VS10:
                        solutionSuffix = AF_STR_VS2010;
                        break;

                    case VS_WMM_VS11:
                        solutionSuffix = AF_STR_VS2012;
                        break;

                    case VS_WMM_VS12:
                        solutionSuffix = AF_STR_VS2013;
                        break;

                    case VS_WMM_VS14:
                        solutionSuffix = AF_STR_VS2015;
                        break;

                    case VS_WMM_UNKNOWN:
                        break;

                    default:
                        GT_ASSERT_EX(false, L"Unsupported VS version");
                        rc = false;
                        break;
                }

                if ((vsVersion < VS_WMM_VS14) && (sampleId == AF_D3D12MULTITHREADING_SAMPLE))
                {
                    acMessageBox::instance().critical(afGlobalVariablesManager::instance().ProductNameA(), VSP_STR_D3DMT_VS13_IDEWarning);
                }
                else
                {
                    gtString solutionFileName;
                    GT_IF_WITH_ASSERT(rc)
                    {
                        solutionFileName.appendFormattedString(sampleSolutionFileName.asCharArray(), solutionSuffix.asCharArray());
                    }

                    gtString sampleFullPath = samplePath.asString();
                    sampleFullPath.append(L"\\");
                    sampleFullPath.append(solutionFileName);

                    samplePath.setFullPathFromString(sampleFullPath);

                    // Check if the solution exists:
                    rcOpenSample = samplePath.exists();

                    OS_OUTPUT_DEBUG_LOG(samplePath.asString().asCharArray(), OS_DEBUG_LOG_INFO);

                    if (rcOpenSample)
                    {
                        bool userOK = true;

                        // Handle save files before project open:
                        vspSaveListDialog saveDialog(NULL);

                        if (saveDialog.hasItems())
                        {
                            // Open the save files dialog:
                            int action = saveDialog.exec();

                            if (action == QDialogButtonBox::Yes)
                            {
                                _pOwner->SaveChangedFiles(true);
                            }
                            else if (action == QDialogButtonBox::Cancel)
                            {
                                userOK = false;
                            }
                        }

                        if (userOK)
                        {
                            // Create the teapot cxlvs file before opening the solution:
                            afApplicationCommands::instance()->WriteSampleCXL(sampleId);

                            // Open the solution using the DTE:
                            rcOpenSample = _pOwner->OpenSolution(samplePath.asString().asCharArray());
                        }
                    }
                    else
                    {
                        gtString message;
                        message.appendFormattedString(VSP_STR_SampleNotFoundLogError, samplePath.asString().asCharArray());
                        OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_ERROR);
                    }
                }
            }
        }
    }

    if (!rcOpenSample)
    {
        QString errorMessage = QString(VSP_STR_SampleLoadErrorMessage).arg(acGTStringToQString(sampleName));
        acMessageBox::instance().critical(AF_STR_ErrorA, errorMessage);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::callsHistoryView
// Description: Return the single copy of the API calls history view
// Return Val:  gdAPICallsHistoryView*
// Author:      Sigal Algranaty
// Date:        17/10/2010
// ---------------------------------------------------------------------------
gdAPICallsHistoryPanel* vspWindowsManager::callsHistoryPanel(QWidget* pParentWindow, QSize viewSize)
{
    GT_UNREFERENCED_PARAMETER(viewSize);

    // If the view wasn't created yet:
    if (_pAPICallsHistoryPanel == NULL)
    {
        // Create the calls history view:
        _pAPICallsHistoryPanel = new gdAPICallsHistoryPanel(&afProgressBarWrapper::instance(), pParentWindow, true);

        // Initialize the view after it is open:
        if (_pAPICallsHistoryPanel->historyView())
        {
            _pAPICallsHistoryPanel->historyView()->initAfterCreation();
        }
    }

    return _pAPICallsHistoryPanel;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::propertiesView
// Description: Return the single copy of the properties view
// Author:      Sigal Algranaty
// Date:        17/10/2010
// ---------------------------------------------------------------------------
afPropertiesView* vspWindowsManager::propertiesView(QWidget* pParentWidget, QSize viewSize)
{
    GT_UNREFERENCED_PARAMETER(viewSize);

    // If the view wasn't created yet:
    if (_pPropertiesView == NULL)
    {
        // Create the calls history view:
        _pPropertiesView = new afPropertiesView(&afProgressBarWrapper::instance(), pParentWidget);

        if (_pStatisticsPanel != NULL)
        {
            gdStatisticsView* pStatisticsView = _pStatisticsPanel->statisticsView();
            GT_IF_WITH_ASSERT(pStatisticsView != NULL)
            {
                // Set the properties window for the statistics viewer:
                GT_ASSERT_EX(false, L"Create a statistics view HTML implementation");
                // pStatisticsView->setPropertiesView(_pPropertiesView);

                GT_IF_WITH_ASSERT(_pMonitoredObjectsTree != NULL)
                {
                    bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

                    if (isDebuggedProcessSuspended)
                    {
                        // Update the properties view with the currently displayed object:
                        const afApplicationTreeItemData* pSelectedTreeItemData = _pMonitoredObjectsTree->getCurrentlySelectedItemData();

                        // Display the item properties:
                        gdPropertiesEventObserver::instance().displayItemProperties((afApplicationTreeItemData*)pSelectedTreeItemData, false, false, false);
                    }
                    else
                    {
                        _pPropertiesView->clearView();
                    }
                }
            }
        }
    }

    return _pPropertiesView;
}


// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::memoryView
// Description: Return the single copy of the memory view
// Return Val:  gdMemoryView*
// Author:      Sigal Algranaty
// Date:        17/1/2011
// ---------------------------------------------------------------------------
gdMemoryView* vspWindowsManager::memoryView(QWidget* pParentWindow, QSize viewSize)
{
    // If the view wasn't created yet:
    if (_pMemoryView == NULL)
    {
        // Create the calls history view:
        _pMemoryView = new gdMemoryView(&afProgressBarWrapper::instance(), pParentWindow);
        _pMemoryView->resize(viewSize);
    }

    return _pMemoryView;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::stateVariablesView
// Description: Return the single copy of the State Variables view
// Return Val:  gdStateVariablesView*
// Author:      Yuri Rshtunique
// Date:        28/08/2014
// ---------------------------------------------------------------------------
gdStateVariablesView* vspWindowsManager::stateVariablesView(QWidget* pParentWindow, QSize viewSize)
{
    // If the view wasn't created yet:
    if (_pStateVariablesView == NULL)
    {
        // Create the state variables view:
        _pStateVariablesView = new gdStateVariablesView(pParentWindow);
        _pStateVariablesView->resize(viewSize);
    }

    return _pStateVariablesView;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::statisticsPanel
// Description: Return the single copy of the statistics panel
// Arguments:   wxWindow* pParentWindow
//              wxSize viewSize
// Return Val:  vspStatisticsView*
// Author:      Sigal Algranaty
// Date:        18/10/2010
// ---------------------------------------------------------------------------
gdStatisticsPanel* vspWindowsManager::statisticsPanel(QWidget* pParentWidget, QSize viewSize)
{
    GT_UNREFERENCED_PARAMETER(viewSize);

    // If the view wasn't created yet:
    if (_pStatisticsPanel == NULL)
    {
        // Create the statistics viewer:
        _pStatisticsPanel = new gdStatisticsPanel(&afProgressBarWrapper::instance(), pParentWidget, NULL);
    }

    return _pStatisticsPanel;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::multiwatchView
// Description: Return the requested multi watch view by its id
//              Possible ids: ID_MULTIWATCH_VIEW1 - ID_MULTIWATCH_VIEW5
// Arguments:   wxWindow* pParentWindow
//              QSize viewSize
//              int windowID
// Return Val:  gdMultiWatchView*
// Author:      Sigal Algranaty
// Date:        27/2/2011
// ---------------------------------------------------------------------------
gdMultiWatchView* vspWindowsManager::multiwatchView(QWidget* pParentWindow, QSize viewSize, int windowID)
{
    // Find the multi watch index:
    int windowIndex = windowID - ID_MULTIWATCH_VIEW_FIRST;

    GT_IF_WITH_ASSERT((windowIndex < VSP_AMOUNT_OF_MULTIWATCH_VIEWS) && (windowIndex >= 0))
    {
        // If the view wasn't created yet:
        if (_pMultiWatchViews[windowIndex] == NULL)
        {
            // Create the next available multi watch view:
            _pMultiWatchViews[windowIndex] = new gdMultiWatchView(pParentWindow, &afProgressBarWrapper::instance());

            // Initialize the view with the requested size:
            _pMultiWatchViews[windowIndex]->initialize(viewSize);
        }
    }

    return _pMultiWatchViews[windowIndex];
}


// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::monitoredObjectsTree
// Description: Return the monitored objects tree
// Arguments:   QWidget* pParentWindow
//              viewSize viewSize
// Return Val:  vspMonitoredObjectsTree*
// Author:      Sigal Algranaty
// Date:        17/10/2010
// ---------------------------------------------------------------------------
afApplicationTree* vspWindowsManager::monitoredObjectsTree(QWidget* pParentWindow, QSize viewSize)
{
    // We get here if some of the views or editors had requested the explorer and it wasn't created yet.
    // We should then ask the package to create it:
    GT_IF_WITH_ASSERT(_pOwner != NULL)
    {
        bool rcViews = _pOwner->ivwmoVerifyBaseViewsCreated();

        if (rcViews)
        {
            // If the view wasn't created yet:
            if (pParentWindow != NULL)
            {
                if (_pMonitoredObjectsTree == NULL)
                {
                    // Create the monitored objects tree:
                    _pMonitoredObjectsTree = new afApplicationTree(&afProgressBarWrapper::instance(), pParentWindow);

                    // Emit a signal stating that the tree was created:
                    afQtCreatorsManager::instance().EmitApplicationTreeCreatedSignal();

                    // force a "mode" change to get an update for the current active mode
                    _pMonitoredObjectsTree->onModeChanged();

                    _pMonitoredObjectsTree->resize(viewSize);

                    bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

                    if (isDebuggedProcessSuspended)
                    {
                        // Update the objects tree on initialization:
                        gdDebugApplicationTreeHandler::instance()->updateMonitoredObjectsTree();
                    }
                    else
                    {
                        _pMonitoredObjectsTree->clearTreeItems(true);
                    }

                    // Update the opened images / buffers windows:
                    vspImagesAndBuffersManager::instance().updateOpenedViewsOnEvent(isDebuggedProcessSuspended);
                }
            }
        }
    }

    return _pMonitoredObjectsTree;
}


// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::updateViewSize
// Description: The function is called after the VS owner window size event is
//              called, and is responsible for the update of the inner layout of
//              the view
// Arguments:   gdWindowCommandID - the view id
//              viewSize - the new view size
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/2/2011
// ---------------------------------------------------------------------------
bool vspWindowsManager::updateViewSize(int gdWindowCommandID, QSize viewSize)
{
    bool retVal = false;

    switch (gdWindowCommandID)
    {
        case ID_CALLS_HISTORY_LIST:
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(_pAPICallsHistoryPanel != NULL && _pAPICallsHistoryPanel->historyView() != NULL)
            {
                _pAPICallsHistoryPanel->historyView()->resize(viewSize);
                retVal = true;
            }
            break;
        }

        case ID_STATISTICS_VIEW:
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(_pStatisticsPanel != NULL)
            {
                gdStatisticsView* pStatisticsView = _pStatisticsPanel->statisticsView();
                GT_IF_WITH_ASSERT(pStatisticsView != NULL)
                {
                    pStatisticsView->resize(viewSize);
                    retVal = true;
                }
            }
            break;
        }


        default:
            // There is not special size event for this view:
            retVal = true;
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::updateMultiwatchViewShowStatus
// Description: Update a multi watch shown / hidden status
// Arguments:   int windowID - the multi watch window id
//              bool isShown - is the window shown / hidden
// Author:      Sigal Algranaty
// Date:        8/3/2011
// ---------------------------------------------------------------------------
void vspWindowsManager::updateMultiwatchViewShowStatus(int windowID, bool isShown)
{
    // Get the multi watch view;
    gdMultiWatchView* pMultiWatchView = multiwatchView(NULL, QSize(-1, -1), windowID);
    GT_IF_WITH_ASSERT(pMultiWatchView != NULL)
    {
        // Set the view shown flag:
        pMultiWatchView->setIsShown(isShown);
    }
}


// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::findFirstHiddenMultiWatchViewIndex
// Description: Return the first hidden multi-watch view. If the views are not yet
//              created, the function return the index of the first one.
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        8/3/2011
// ---------------------------------------------------------------------------
int vspWindowsManager::findFirstHiddenMultiWatchViewIndex()
{
    // By default, if all views used and shown, return the first:
    int retVal = -1;

    // Go through the views, and find the first not null and hidden view:
    for (int i = 0; i < VSP_AMOUNT_OF_MULTIWATCH_VIEWS; i++)
    {
        if (_pMultiWatchViews[i] != NULL)
        {
            if (!_pMultiWatchViews[i]->isShown())
            {
                // The view is both created, and hidden, use it:
                retVal = i;
                break;
            }
        }
        else if (retVal < 0)
        {
            // The view is not created yet, you can use it:
            retVal = i;
        }
    }

    if (retVal < 0)
    {
        // All views are shown, show the first one:
        retVal = 0;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        DialogImmediateClosureCallback
// Description: Callback to close the dialog which was marked to be immediately
//              closed. This is a WINAPI SetTimer() TIMERPROC.
// Author:      Amit Ben-Moshe
// Date:        1/7/2014
// ---------------------------------------------------------------------------
VOID CALLBACK DialogImmediateClosureCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    GT_UNREFERENCED_PARAMETER(hwnd);
    GT_UNREFERENCED_PARAMETER(idEvent);
    GT_UNREFERENCED_PARAMETER(dwTime);

    GT_IF_WITH_ASSERT(gs_pDialogForImmediateClose != NULL)
    {
        // Close the requested dialog.
        gs_pDialogForImmediateClose->done(0);

        // Kill the timer.
        KillTimer((HWND)gs_pDialogForImmediateClose->winId(), uMsg);

        gs_pDialogForImmediateClose->deleteLater();

        // Reset the requested dialog handle.
        gs_pDialogForImmediateClose = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::showModal
// Description: Show the dialog in a modal way
// Arguments:   QDialog* dialogWindow - the dialog to show
//              bool show - whether the dialog should be visible to the user,
//                          or closed immediately.
// Return Val:  Modal return value (button pressed)
// Author:      Gilad Yarnitzky
// Date:        9/5/2011
// ---------------------------------------------------------------------------
int vspWindowsManager::showModal(QDialog* dialogWindow, bool show /* = true */)
{
    int retVal = 0;

    GT_IF_WITH_ASSERT(dialogWindow != NULL && dialogWindow->winId())
    {
        // Correct set the parent window of the message box in case one is opened:
        acMessageBox::instance().setParentWidget(dialogWindow);

        // If a modal window is not already opened:
        if (stQtDialogToActivate == NULL)
        {
            // Get the VS dialog owner window
            // wxWindow* pDialogParent = NULL;
            HWND hwnd = NULL;
            IVsUIShell* piUIShell = getUIShell();
            GT_IF_WITH_ASSERT(piUIShell != NULL)
            {
                piUIShell->GetDialogOwnerHwnd(&hwnd);
            }

            // Prepare Modal status:
            if (piUIShell != NULL)
            {
                piUIShell->EnableModeless(FALSE);
            }

            ::EnableWindow(hwnd, false);

            // replace the main window proc of the dialog window:
            stOriginalWindowProc = (WNDPROC)::SetWindowLongPtr((HWND)dialogWindow->winId(), GWLP_WNDPROC, (LONG_PTR)&mainOverrideQtWindowProc);
            GT_ASSERT(stOriginalWindowProc != NULL);

            // Create a timer for the tooltips:
            stTimerID = SetTimer((HWND)dialogWindow->winId(), MODAL_DIALOG_TIMER, TOOL_TIP_DELAY, NULL);

            stQtDialogToActivate = dialogWindow;

            mModalDialogList.push_front(dialogWindow);

            if (!show)
            {
                // Move to the dialog way off screen so it will not be shown
                // set visibilty false will not work since exec will force it visible and
                // resize(1,1) will not work since it will not resize under dialog minimum size.
                dialogWindow->move(-10000, -10000);

                // Mark the dialog as to be immediately closed.
                gs_pDialogForImmediateClose = dialogWindow;

                // Set a timer that will invoke the function that should close the requested window.
                stTimerID = SetTimer((HWND)dialogWindow->winId(), MODAL_DIALOG_IMMEDIATE_CLOSURE_TIMER,
                                     DIALOG_IMMEDIATE_CLOSURE_INTERVAL_MS, DialogImmediateClosureCallback);
            }

            // Show modal:
            try
            {
                retVal = dialogWindow->exec();
            }
            catch (...)
            {
                // Just make sure everything is caught so main vs window
                // can be restored.
            }

            mModalDialogList.pop_front();

            // Kill the created timer:
            KillTimer((HWND)dialogWindow->winId(), MODAL_DIALOG_TIMER);
            stTimerID = NULL;

            // restore the original window proc of the visual studio window:
            ::SetWindowLongPtr((HWND)dialogWindow->winId(), GWLP_WNDPROC, (LONG_PTR)stOriginalWindowProc);
            stQtDialogToActivate = NULL;

            // Exit Modal Status:
            if (piUIShell != NULL)
            {
                piUIShell->EnableModeless(TRUE);
            }

            ::EnableWindow(hwnd, true);

            // Force the visual window to be the fore window and active:
            ::SetForegroundWindow(hwnd);

            // restore acMessage box VS window as parent:
            acMessageBox::instance().setParentWidget(NULL);
        }
        else
        {
            // open a modal within a modal:
            try
            {
                // Set the parent and modal handling correct:
                stQtDialogToActivate = dialogWindow;

                mModalDialogList.push_front(dialogWindow);

                retVal = dialogWindow->exec();

                // set the active dialog to the top of the list again:
                mModalDialogList.pop_front();

                stQtDialogToActivate = mModalDialogList.front();
                // Sanity check:
                GT_IF_WITH_ASSERT(stQtDialogToActivate != NULL)
                {
                    acMessageBox::instance().setParentHwnd(reinterpret_cast<HWND>(stQtDialogToActivate->winId()));
                }
            }
            catch (...)
            {
                // Just make sure everything is caught so main vs window
                // can be restored.
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
QMessageBox::StandardButton vspWindowsManager::ShowMessageBox(QMessageBox::Icon type, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox::StandardButton retVal = QMessageBox::Ok;

    HWND hwnd = NULL;
    IVsUIShell* piUIShell = getUIShell();

    GT_IF_WITH_ASSERT(piUIShell != NULL)
    {
        piUIShell->GetDialogOwnerHwnd(&hwnd);
    }

    // Prepare Modal status:
    if (piUIShell != NULL)
    {
        piUIShell->EnableModeless(FALSE);
    }

    ::EnableWindow(hwnd, false);

    // Show modal:
    try
    {
        retVal = afMessageBox::instance().ShowMessageBox(type, title, text, buttons, defaultButton);
    }
    catch (...)
    {
        // Just make sure everything is caught so main vs window
        // can be restored.
    }

    ::EnableWindow(hwnd, true);

    // Force the visual window to be the fore window and active:
    ::SetForegroundWindow(hwnd);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::updateMultiViewHexMode
// Description: Show message box in a modal way
// Return Val:  message box return value (button pressed)
// Author:      Gilad Yarnitzky
// Date:        16/5/2011
// ---------------------------------------------------------------------------
void vspWindowsManager::updateMultiWatchViewHexMode(bool hexDisplayMode)
{
    for (int i = 0; i < VSP_AMOUNT_OF_MULTIWATCH_VIEWS; i++)
    {
        // Get the current multi watch view:
        gdMultiWatchView* pMultiwatchView = _pMultiWatchViews[i];

        // If the view is already created:
        if (pMultiwatchView != NULL)
        {
            // Update the hexadecimal display flag for the multiwatch:
            pMultiwatchView->setHexDisplayMode(hexDisplayMode);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        vspWindowsManager::commandIdFromWidget
// Description: Get a window id from the requested QWidget object
// Arguments:   QWidget* pWidget
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        5/3/2012
// ---------------------------------------------------------------------------
int vspWindowsManager::commandIdFromWidget(QWidget* pWidget)
{
    int retVal = -1;

    if (_pAPICallsHistoryPanel != NULL)
    {
        if (pWidget == _pAPICallsHistoryPanel->historyView())
        {
            retVal = ID_CALLS_HISTORY_LIST;
        }
    }

    if (pWidget == _pAPICallsHistoryPanel)
    {
        retVal = ID_CALLS_HISTORY_LIST;
    }

    else if (pWidget == _pPropertiesView)
    {
        retVal = ID_PROPERTIES_VIEW;
    }

    else if (pWidget == _pStatisticsPanel)
    {
        retVal = ID_STATISTICS_VIEW;
    }

    else if (pWidget == _pMemoryView)
    {
        retVal = ID_MEMORY_ANALYSIS_VIEWER;
    }

    else if (pWidget == _pStateVariablesView)
    {
        retVal = ID_STATE_VARIABLES_VIEW;
    }



    return retVal;
}

VsWindowsManagementMode vspWindowsManager::GetVsWindowsManagementModeFromOwner() const
{
    VsWindowsManagementMode retVal = VS_WMM_UNKNOWN;
    GT_IF_WITH_ASSERT(_pOwner != nullptr)
    {
        bool rc = _pOwner->GetVsWindowsManagementMode(retVal);
        GT_ASSERT(rc);
    }
    return retVal;
}

void vspWindowsManager::setOwner(IVscWindowsManagerOwner* pOwner)
{
    _pOwner = pOwner;
}

LRESULT WINAPI mainOverrideQtWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = TRUE;

    if (Msg == WM_ACTIVATEAPP)
    {
        GT_IF_WITH_ASSERT(stQtDialogToActivate != NULL)
        {
            // Bring the dialog to the front only we the application is getting focus
            if (wParam == TRUE)
            {
                ::SetForegroundWindow((HWND)stQtDialogToActivate->winId());
            }
        }
    }

    // If we received the timer send the tooltip message:
    if (Msg == WM_TIMER && MODAL_DIALOG_TIMER == wParam)
    {
        HWND currentWinId = (HWND)stQtDialogToActivate->winId();
        POINT screenPos;
        POINT clientPos;
        GetCursorPos(&screenPos);
        clientPos = screenPos;
        ScreenToClient(currentWinId, &clientPos);

        // Move to QPoint:
        QPoint qtClientPos(clientPos.x, clientPos.y);
        QPoint qtScreenPos(screenPos.x, screenPos.y);

        // Find the lowest level child and move the clientPos to that child:
        QWidget* pChild = stQtDialogToActivate;
        QWidget* pCurrentWidget = stQtDialogToActivate;

        while (pChild)
        {
            pChild = pCurrentWidget->childAt(qtClientPos);

            if (pChild)
            {
                pCurrentWidget = pChild;
                // move client pos to the new current qwidget:
                qtClientPos = pCurrentWidget->mapFromGlobal(qtScreenPos);
            }
        }

        if (pCurrentWidget)
        {
            // Pass the QHelpEvent:
            QHelpEvent helpEvent(QEvent::ToolTip, qtClientPos, qtScreenPos);
            QApplication::sendEvent(pCurrentWidget, &helpEvent);
        }
    }

    if ((Msg == WM_MOUSEMOVE || Msg == WM_LBUTTONUP || Msg == WM_LBUTTONDOWN || Msg == WM_LBUTTONDBLCLK) && MODAL_DIALOG_TIMER == wParam)
    {
        KillTimer((HWND)stQtDialogToActivate->winId(), MODAL_DIALOG_TIMER);
        // if we have a mouse move reset the timer so the tooltip will appear after the correct time
        SetTimer((HWND)stQtDialogToActivate->winId(), MODAL_DIALOG_TIMER, TOOL_TIP_DELAY, NULL);
    }

    lResult = stOriginalWindowProc(hWnd, Msg, wParam, lParam);

    return lResult;
}
