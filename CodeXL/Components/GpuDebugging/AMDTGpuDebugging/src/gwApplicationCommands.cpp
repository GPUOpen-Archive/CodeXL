//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwApplicationCommands.cpp
///
//==================================================================================

//------------------------------ gwApplicationCommands.cpp ------------------------------

// Warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtWidgets>
#include <QObject>

#include <AMDTApplicationFramework/Include/views/afSourceCodeView.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIStringTokenizer.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afSourceCodeViewsManager.h>
#include <AMDTApplicationFramework/Include/afQMdiSubWindow.h>
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdImagesAndBuffersManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStatisticsPanel.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdSaveProjectCommand.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdBreakpointsDialog.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdCallsStackListCtrl.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMemoryView.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMultiWatchView.h>

// Local:
#include <AMDTGpuDebugging/Include/gwgDEBuggerAppWrapperDLLBuild.h>
#include <AMDTGpuDebugging/Include/gwgDEBuggerAppWrapper.h>
#include <AMDTGpuDebugging/Include/gwStringConstants.h>
#include <src/gwApplicationCommands.h>
#include <src/gwImagesAndBuffersMDIViewCreator.h>


// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::gwApplicationCommands
// Description:
// Return Val:
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
gwApplicationCommands::gwApplicationCommands()
{

}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::~gwApplicationCommands
// Description:
// Return Val:
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
gwApplicationCommands::~gwApplicationCommands()
{
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::openBreakpointsDialog
// Description: Open the breakpoints dialog
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
bool gwApplicationCommands::openBreakpointsDialog()
{
    bool retVal = false;

    // Perform the command only if it is enabled:
    if (isBreakpointsDialogCommandEnabled())
    {
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(NULL != pApplicationCommands)
        {
            gdBreakpointsDialog dialog(NULL);
            int rc = dialog.exec();

            if (QDialog::Accepted == rc)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::isBreakpointsDialogCommandEnabled
// Description: Return true iff the breakpoints dialog command is enabled
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
bool gwApplicationCommands::isBreakpointsDialogCommandEnabled()
{
    bool retVal = false;

    if (afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode))
    {
        // Get current execution mode;
        apExecutionMode currentExecMode = AP_DEBUGGING_MODE;
        gaGetDebuggedProcessExecutionMode(currentExecMode);

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::propertiesEventObserver
// Description: Return the application properties view
// Return Val:  gdPropertiesEventObserver*
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
gdPropertiesEventObserver* gwApplicationCommands::propertiesEventObserver()
{
    gdPropertiesEventObserver* pRetVal = NULL;
    pRetVal = gwgDEBuggerAppWrapper::propertiesView();

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::callsHistoryPanel
// Description: Return the application calls history panel
// Return Val:  gdAPICallsHistoryPanel*
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
gdAPICallsHistoryPanel* gwApplicationCommands::callsHistoryPanel()
{
    gdAPICallsHistoryPanel* pRetVal = NULL;
    pRetVal = gwgDEBuggerAppWrapper::callsHistoryPanel();

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::callStackView
// Description: Return the application call stack view
// Return Val:  gdCallStackView*
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
gdCallStackView* gwApplicationCommands::callStackView()
{
    gdCallStackView* pRetVal = NULL;
    pRetVal = gwgDEBuggerAppWrapper::callStackView();

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::debuggedProcessEventsView
// Description: Return the application debugged process events view
// Return Val:  gdDebuggedProcessEventsView*
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
gdDebuggedProcessEventsView* gwApplicationCommands::debuggedProcessEventsView()
{
    gdDebuggedProcessEventsView* pRetVal = NULL;
    pRetVal = gwgDEBuggerAppWrapper::debuggedProcessEventsView();

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::stateVariablesView
// Description: Return the application state variable view
// Return Val:  gdStateVariablesView*
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
gdStateVariablesView* gwApplicationCommands::stateVariablesView()
{
    gdStateVariablesView* pRetVal = NULL;
    pRetVal = gwgDEBuggerAppWrapper::stateVariablesView();

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::commandQueuesView
// Description: Return the application command queues view
// Return Val:  gdCommandQueuesView*
// Author:      Sigal Algranaty
// Date:        15/12/2011
// ---------------------------------------------------------------------------
gdCommandQueuesView* gwApplicationCommands::commandQueuesView()
{
    gdCommandQueuesView* pRetVal = NULL;
    //  pRetVal = gwgDEBuggerAppWrapper::commandQueuesView();

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::statisticsView
// Description: Return the application statistics view
// Return Val:  gdStatisticsPanel*
// Author:      Sigal Algranaty
// Date:        18/9/2011
// ---------------------------------------------------------------------------
gdStatisticsPanel* gwApplicationCommands::statisticsPanel()
{
    gdStatisticsPanel* pRetVal = NULL;
    pRetVal = gwgDEBuggerAppWrapper::statisticsPanel();

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::memoryView
// Description: Return the application memory view
// Return Val:  gdMemoryView*
// Author:      Sigal Algranaty
// Date:        18/9/2011
// ---------------------------------------------------------------------------
gdMemoryView* gwApplicationCommands::memoryView()
{
    gdMemoryView* pRetVal = NULL;
    pRetVal = gwgDEBuggerAppWrapper::memoryView();

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::breakpointsView
// Description: Get the application breakpoints view
// Return Val:  gdBreakpointsView*
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
gdBreakpointsView* gwApplicationCommands::breakpointsView()
{
    gdBreakpointsView* pRetVal = NULL;
    pRetVal = gwgDEBuggerAppWrapper::breakpointsView();

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::watchView
// Description: Get the application watch view
// Return Val:  gdWatchView*
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
gdWatchView* gwApplicationCommands::watchView()
{
    gdWatchView* pRetVal = NULL;
    pRetVal = gwgDEBuggerAppWrapper::watchView();

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::multiWatchView
// Description: Get the application multi watch view
// Return Val:  gdMultiWatchView*
// Author:      Sigal Algranaty
// Date:        26/9/2011
// ---------------------------------------------------------------------------
gdMultiWatchView* gwApplicationCommands::multiWatchView(int viewIndex)
{
    gdMultiWatchView* pRetVal = gwgDEBuggerAppWrapper::multiWatchView1();

    if (viewIndex == 1)
    {
        pRetVal = multiWatchView2();
    }

    else if (viewIndex == 2)
    {
        pRetVal = multiWatchView3();
    }

    return pRetVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        multiWatchView2
/// \brief Description: Get the application multi watch view 2
/// \return gdMultiWatchView*
/// -----------------------------------------------------------------------------------------------
gdMultiWatchView* gwApplicationCommands::multiWatchView2()
{
    gdMultiWatchView* pRetVal = NULL;
    pRetVal = gwgDEBuggerAppWrapper::multiWatchView2();

    return pRetVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        multiWatchView3
/// \brief Description: Get the application multi watch view 3
/// \return gdMultiWatchView*
/// -----------------------------------------------------------------------------------------------
gdMultiWatchView* gwApplicationCommands::multiWatchView3()
{
    gdMultiWatchView* pRetVal = NULL;
    pRetVal = gwgDEBuggerAppWrapper::multiWatchView3();

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::localsView
// Description: Get the application locals view
// Return Val:  gdLocalsView*
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
gdLocalsView* gwApplicationCommands::localsView()
{
    gdLocalsView* pRetVal = NULL;
    pRetVal = gwgDEBuggerAppWrapper::localsView();

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::displayImageBufferObject
// Description: Display an image / buffer object in VS
// Arguments:   afApplicationTreeItemData* pItemData
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
bool gwApplicationCommands::displayImageBufferObject(afApplicationTreeItemData* pItemData, const gtString& itemText)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        if (pItemData->m_itemType != AF_TREE_ITEM_ITEM_NONE)
        {
            // Get the main application window:
            afMainAppWindow* pApplicationWindow = afMainAppWindow::instance();
            GT_IF_WITH_ASSERT(pApplicationWindow != NULL)
            {
                gtString fileName;
                bool rcFileName = gdHTMLProperties::objectDataToHTMLLink(*pItemData, -1, fileName);
                GT_IF_WITH_ASSERT(rcFileName)
                {
                    // Build the file path:
                    // Get the User AppData directory
                    osFilePath imageObjectsFilePath;
                    afGetUserDataFolderPath(imageObjectsFilePath);

                    // Add the VS_Cache files directory:
                    imageObjectsFilePath.appendSubDirectory(GW_STR_VSCacheFolderName);

                    // Get the project name:
                    gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();

                    // Get the debugged application name:
                    gtString projectName;
                    osFilePath currentProject = globalVarsManager.currentDebugProjectSettings().executablePath();
                    currentProject.getFileName(projectName);

                    // Create the folder if not created:
                    osDirectory directoryPath;
                    directoryPath.setDirectoryPath(imageObjectsFilePath);
                    bool rcCreateDir = directoryPath.create();
                    GT_IF_WITH_ASSERT(rcCreateDir)
                    {
                        // Add the VS_Cache files directory:
                        imageObjectsFilePath.appendSubDirectory(projectName);
                        directoryPath.setDirectoryPath(imageObjectsFilePath);
                        directoryPath.create();

                        // Create a text file with the description of the current buffer / image object:

                        // Write the files to the cache folder:
                        imageObjectsFilePath.setFileName(fileName);
                        imageObjectsFilePath.setFileExtension(AF_STR_CodeXMLImageBuffersFilesExtension);

                        // Just save the file:
                        osFile objectfile;
                        bool rc = objectfile.open(imageObjectsFilePath, osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_WRITE);
                        GT_IF_WITH_ASSERT(rc)
                        {
                            gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
                            GT_IF_WITH_ASSERT(pGDData != NULL)
                            {
                                // Trigger a source code view creation event:
                                gtString viewTitle;
                                pGDData->_contextId.toString(viewTitle);
                                viewTitle.appendFormattedString(L" %ls", itemText.asCharArray());
                                apMDIViewCreateEvent imageBufferViewEvent(AF_STR_ImageBuffersViewsCreatorID, imageObjectsFilePath, viewTitle, 0, -1);
                                apEventsHandler::instance().registerPendingDebugEvent(imageBufferViewEvent);
                            }

                            retVal = true;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::displayOpenCLProgramSourceCode
// Description: Display an OpenCL program source code
// Arguments:   afApplicationTreeItemData* pProgramItemData
// Author:      Sigal Algranaty
// Date:        2/8/2011
// ---------------------------------------------------------------------------
void gwApplicationCommands::displayOpenCLProgramSourceCode(afApplicationTreeItemData* pProgramItemData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pProgramItemData != NULL)
    {
        if (pProgramItemData->m_itemType != AF_TREE_ITEM_ITEM_NONE)
        {
            // Get the monitored object tree:
            gdDebugApplicationTreeHandler* pMonitoredObjectsTree = gdDebugApplicationTreeHandler::instance();
            GT_IF_WITH_ASSERT(pMonitoredObjectsTree != NULL)
            {
                gtString viewTitle;

                // Get the item text from the tree:
                gtString itemText = acQStringToGTString(pMonitoredObjectsTree->GetTreeItemText(pProgramItemData->m_pTreeWidgetItem));

                gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pProgramItemData->extendedItemData());
                GT_IF_WITH_ASSERT(pGDData != NULL)
                {
                    // Append the context string to the item string:
                    gtString itemNameWithContext;
                    pGDData->_contextId.toString(viewTitle);

                    // Append the context as string to the item name:
                    viewTitle.appendFormattedString(L"%ls ", itemText.asCharArray());

                    // Find the item file path:
                    osFilePath filePath;
                    int lineNumber = -1;
                    bool rc = gdFindObjectFilePath(pProgramItemData, filePath, lineNumber);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        // Check if the file should be displayed with program counter:

                        int displayedLineNumber = -1, displayedPCCounter = -1;
                        (void) afSourceCodeViewsManager::instance().getLineNumberAndProgramCounter(filePath, displayedLineNumber, displayedPCCounter);

                        // Check if the file is opened:
                        bool isFileOpen = afSourceCodeViewsManager::instance().isFileOpen(filePath);

                        if (!isFileOpen)
                        {
                            // Display the file with the original line number and program counter, and then display again with the new line number and no pc:
                            apMDIViewCreateEvent sourceCodeViewEvent(AF_STR_GenericMDIViewsCreatorID, filePath, viewTitle, 0, displayedLineNumber, displayedPCCounter);
                            apEventsHandler::instance().registerPendingDebugEvent(sourceCodeViewEvent);
                        }

                        // Trigger a source code view creation event:
                        apMDIViewCreateEvent sourceCodeViewEvent(AF_STR_GenericMDIViewsCreatorID, filePath, viewTitle, 0, lineNumber, -1);
                        apEventsHandler::instance().registerPendingDebugEvent(sourceCodeViewEvent);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::displayOpenGLSLShaderCode
// Description: Display an OpenGL shader source code
// Arguments:   afApplicationTreeItemData* pShaderItemData
// Author:      Sigal Algranaty
// Date:        2/8/2011
// ---------------------------------------------------------------------------
void gwApplicationCommands::displayOpenGLSLShaderCode(afApplicationTreeItemData* pShaderItemData)
{
    if (pShaderItemData->m_itemType != AF_TREE_ITEM_ITEM_NONE)
    {
        // Get the monitored object tree:
        gdDebugApplicationTreeHandler* pMonitoredObjectsTree = gdDebugApplicationTreeHandler::instance();
        GT_IF_WITH_ASSERT(pMonitoredObjectsTree != NULL)
        {
            gtString viewTitle;
            // Get the item text from the tree:
            gtString itemText = acQStringToGTString(pMonitoredObjectsTree->GetTreeItemText(pShaderItemData->m_pTreeWidgetItem));

            gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pShaderItemData->extendedItemData());
            GT_IF_WITH_ASSERT(pGDData != NULL)
            {
                // Append the context string to the item string:
                gtString itemNameWithContext;
                pGDData->_contextId.toString(viewTitle);

                // Append the context as string to the item name:
                viewTitle.appendFormattedString(L"%ls ", itemText.asCharArray());

                // Find the item file path:
                osFilePath filePath;
                int lineNumber = -1;
                bool rc = gdFindObjectFilePath(pShaderItemData, filePath, lineNumber);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Trigger a source code view creation event:
                    apMDIViewCreateEvent sourceCodeViewEvent(AF_STR_GenericMDIViewsCreatorID, filePath, viewTitle, 0, lineNumber, -1);
                    apEventsHandler::instance().registerPendingDebugEvent(sourceCodeViewEvent);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::updateToolbarCommands
// Description: Update the main frame toolbar commands
// Author:      Sigal Algranaty
// Date:        1/8/2011
// ---------------------------------------------------------------------------
void gwApplicationCommands::updateToolbarCommands()
{
    // Get the main frame:
    afMainAppWindow* pApplicationWindow = afMainAppWindow::instance();

    // NOTICE: Some of the actions are initialized before the main application
    // initialization is over. Do not assert this if:
    if (pApplicationWindow != NULL)
    {
        pApplicationWindow->updateToolbarsCommands();
    }
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::updateToolbarCommands
// Description: Update the main frame toolbars
// Author:      Sigal Algranaty
// Date:        1/8/2011
// ---------------------------------------------------------------------------
void gwApplicationCommands::updateToolbars()
{
    // Get the main frame:
    afMainAppWindow* pApplicationWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pApplicationWindow != NULL)
    {
        pApplicationWindow->updateToolbars();
    }
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::raiseStatisticsView
// Description: Raise statistics view through the package commands
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        31/7/2011
// ---------------------------------------------------------------------------
bool gwApplicationCommands::raiseStatisticsView()
{
    bool retVal = false;

    // Get the main window:
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != NULL)
    {
        gdStatisticsPanel* pStatisticsPanel = statisticsPanel();
        GT_IF_WITH_ASSERT(pStatisticsPanel != NULL)
        {
            // Get the widget parent:
            QDockWidget* pDockWidgetParent = qobject_cast<QDockWidget*>(pStatisticsPanel->parent());
            GT_IF_WITH_ASSERT(pDockWidgetParent != NULL)
            {
                pDockWidgetParent->show();
                pDockWidgetParent->setFocus();
                pDockWidgetParent->raise();
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::raiseCommandQueuesView
// Description: Raise command queues view through the application main window
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/1/2012
// ---------------------------------------------------------------------------
bool gwApplicationCommands::raiseCommandQueuesView()
{
    bool retVal = false;

    // Get the main window:
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != NULL)
    {
        /*      gdCommandQueuesView* pCommandQueuesView = commandQueuesView();
        GT_IF_WITH_ASSERT (pCommandQueuesView != NULL)
        {
        // Get the widget parent:
        QDockWidget* pDockWidgetParent = qobject_cast<QDockWidget*>(pCommandQueuesView->parent());
        GT_IF_WITH_ASSERT(pDockWidgetParent != NULL)
        {
        pDockWidgetParent->show();
        pDockWidgetParent->setFocus();
        pDockWidgetParent->raise();
        }
        }*/
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::raiseMemoryView
// Description: Raise memory view through the main window interface
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        31/7/2011
// ---------------------------------------------------------------------------
bool gwApplicationCommands::raiseMemoryView()
{
    bool retVal = false;

    // Get the main window:
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != NULL)
    {
        gdMemoryView* pMemoryView = memoryView();
        GT_IF_WITH_ASSERT(pMemoryView != NULL)
        {
            // Get the widget parent:
            QDockWidget* pDockWidgetParent = qobject_cast<QDockWidget*>(pMemoryView->parent());
            GT_IF_WITH_ASSERT(pDockWidgetParent != NULL)
            {
                pDockWidgetParent->show();
                pDockWidgetParent->setFocus();
                pDockWidgetParent->raise();
            }

            // Update the view:
            bool rc = pMemoryView->updateView(true);
            GT_ASSERT(rc);
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gwApplicationCommands::raiseMultiWatchView
// Description: Raise the requested multiwatch view
// Arguments:   gdMultiWatchView* pMultiWatchView
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/9/2011
// ---------------------------------------------------------------------------
bool gwApplicationCommands::raiseMultiWatchView(gdMultiWatchView* pMultiWatchView)
{
    bool retVal = false;

    // Get the main window:
    GT_IF_WITH_ASSERT(pMultiWatchView != NULL)
    {
        // Get the widget parent:
        QDockWidget* pDockWidgetParent = qobject_cast<QDockWidget*>(pMultiWatchView->parent());
        GT_IF_WITH_ASSERT(pDockWidgetParent != NULL)
        {
            pDockWidgetParent->show();
            pDockWidgetParent->setFocus();
            pDockWidgetParent->raise();
        }
    }

    return retVal;
}


