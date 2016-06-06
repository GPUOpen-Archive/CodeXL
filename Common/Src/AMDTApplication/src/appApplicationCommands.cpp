//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file appApplicationCommands.cpp
///
//==================================================================================

// Compiler warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <qtIgnoreCompilerWarnings.h>
// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtASCIIStringTokenizer.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acSourceCodeView.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afSourceCodeViewsManager.h>

// Local:
#include <src/appApplicationCommands.h>

// ---------------------------------------------------------------------------
// Name:        appApplicationCommands::appApplicationCommands
// Description: Constructor
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
appApplicationCommands::appApplicationCommands()
{
    afApplicationCommands::registerInstance(this);
}

// ---------------------------------------------------------------------------
// Name:        appApplicationCommands::~appApplicationCommands
// Description: Destructor
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
appApplicationCommands::~appApplicationCommands()
{

}
// ---------------------------------------------------------------------------
// Name:        appApplicationCommands::setShowWhiteSpaces
// Description: Show / Hide white spaces in all open source code views
// Arguments:   bool show
// Author:      Sigal Algranaty
// Date:        27/9/2011
// ---------------------------------------------------------------------------
void appApplicationCommands::setViewLineNumbers(bool show)
{
    // Get the source code views manager:
    afSourceCodeViewsManager::instance().setViewLineNumbers(show);
}

bool appApplicationCommands::OpenFileAtLine(const osFilePath& filePath, int lineNumber, int programCounterIndex, int viewIndex)
{
    bool retVal = false;

    // Get the file extension:
    gtString ext;
    filePath.getFileExtension(ext);

    if (filePath.exists())
    {
        gtString mdiCreatorType;

        // If this is an images / buffers file:
        if (ext == AF_STR_CodeXMLImageBuffersFilesExtension)
        {
            mdiCreatorType = AF_STR_ImageBuffersViewsCreatorID;

        }
        else if (ext == AF_STR_CpuProfileFileExtension)
        {
            mdiCreatorType = AF_STR_CPUProfileViewsCreatorID;
        }
        else if (ext == AF_STR_GpuProfileTraceFileExtension || ext == AF_STR_GpuProfileSessionFileExtension || ext == AF_STR_profileFileExtension4)
        {
            mdiCreatorType = AF_STR_GPUProfileViewsCreatorID;
        }
        else if (ext == AF_STR_profileFileExtension7)
        {
            mdiCreatorType = AF_STR_ThreadProfileViewsCreatorID;
        }
        else if (ext == AF_STR_profileFileExtension6)
        {
            mdiCreatorType = AF_STR_PowerProfileViewsCreatorID;
        }
        else if (ext == AF_STR_profileFileExtension8 || ext == AF_STR_profileFileExtension9 || ext == AF_STR_frameAnalysisDashboardFileExtension || ext == AF_STR_frameAnalysisOverviewFileExtension)
        {
            mdiCreatorType = AF_STR_GPUProfileViewsCreatorID;
        }

#ifdef GP_OBJECT_VIEW_ENABLE    // GP_OBJECT_VIEW_ENABLE(manual enable)
        else if (ext == AF_STR_profileFileExtension10)
        {
            mdiCreatorType = AF_STR_GPUProfileViewsCreatorID;
        }

#endif
        else if (ext == AF_STR_pngFileExtension)
        {
            mdiCreatorType = AF_STR_GenericMDIViewsCreatorID;
        }
        else
        {
            mdiCreatorType = AF_STR_GenericMDIViewsCreatorID;
        }

        // Trigger an image buffer view creation event:
        apMDIViewCreateEvent mdiCreateEvent(mdiCreatorType, filePath, AF_STR_Empty, viewIndex, lineNumber, programCounterIndex);
        apEventsHandler::instance().registerPendingDebugEvent(mdiCreateEvent);
        retVal = true;
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(filePath.asString().asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        closeFile
// Description: Closes the specified file if it is open
// Arguments:   osFilePath filePath
// Return Val:  bool - Success / failure.
// Author:      Chris Hesik
// Date:        31/5/2012
// ---------------------------------------------------------------------------
bool appApplicationCommands::closeFile(const osFilePath& filePath)
{
    bool retVal = false;
    afMainAppWindow* mainWindow = afMainAppWindow::instance();
    GT_ASSERT(mainWindow != NULL)
    afQMdiSubWindow* pMdiWindow = mainWindow->findMDISubWindow(filePath);

    if (pMdiWindow != NULL)
    {
        retVal = mainWindow->closeMDISubWindow(pMdiWindow);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        appApplicationCommands::applicationMainWindow
// Description: Return the main application window
// Return Val:  QWidget*
// Author:      Sigal Algranaty
// Date:        22/3/2012
// ---------------------------------------------------------------------------
QWidget* appApplicationCommands::applicationMainWindow()
{
    return afMainAppWindow::instance();
}


afApplicationTree::DragAction appApplicationCommands::DragActionForDropEvent(QDropEvent* pEvent)
{
    afApplicationTree::DragAction retVal = afApplicationTree::DRAG_NO_ACTION;

    if (pEvent != nullptr)
    {
        const QMimeData* pMimeData = pEvent->mimeData();

        GT_IF_WITH_ASSERT(nullptr != pMimeData)
        {
            if (pMimeData->hasUrls())
            {
                if (!pMimeData->urls().isEmpty())
                {
                    foreach (QUrl url, pMimeData->urls())
                    {
                        // Get the URL as file path:
                        QString qurlStr = url.toLocalFile();
                        gtString fileUrl;
                        fileUrl.fromASCIIString(qurlStr.toLatin1().data());
                        osFilePath path(fileUrl);
                        gtString extension;
                        path.getFileExtension(extension);

                        if (extension == AF_STR_exeFileExtension)
                        {
                            if (pMimeData->urls().size() == 1)
                            {
                                retVal = afApplicationTree::DRAG_NEW_PROJECT;
                            }

                            break;
                        }
                        else if (extension == AF_STR_projectFileExtension)
                        {
                            if (pMimeData->urls().size() == 1)
                            {
                                retVal = afApplicationTree::DRAG_OPEN_PROJECT;
                            }

                            break;
                        }
                        else if ((extension == AF_STR_profileFileExtension1) ||
                                 (extension == AF_STR_profileFileExtension2) ||
                                 (extension == AF_STR_profileFileExtension3) ||
                                 (extension == AF_STR_profileFileExtension4) ||
                                 (extension == AF_STR_profileFileExtension5) ||
                                 (extension == AF_STR_profileFileExtension6) ||
                                 (extension == AF_STR_profileFileExtension7) ||
                                 (extension == AF_STR_profileFileExtension8) ||
                                 (extension == AF_STR_profileFileExtension9) ||
                                 (extension == AF_STR_frameAnalysisArchivedFileExtension))
                        {
                            // Check if the list is mixed:
                            if ((retVal != afApplicationTree::DRAG_NO_ACTION) && (retVal != afApplicationTree::DRAG_ADD_SESSION_TO_TREE))
                            {
                                // Mixed list:
                                retVal = afApplicationTree::DRAG_NO_ACTION;
                                break;
                            }
                            else
                            {
                                retVal = afApplicationTree::DRAG_ADD_SESSION_TO_TREE;
                            }
                        }
                        else
                        {
                            retVal = afApplicationTree::DRAG_ADD_ANALYZED_FILE_TO_TREE;
                        }
                    }
                }
            }
        }
    }

    // Check if a process is currently running:
    bool isProcessRunning = (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_EXISTS);

    // Make sure that the action is possible while a process is running:
    if ((retVal != afApplicationTree::DRAG_NO_ACTION) && isProcessRunning)
    {
        if ((retVal == afApplicationTree::DRAG_ADD_SESSION_TO_TREE) || (retVal == afApplicationTree::DRAG_OPEN_PROJECT) || (retVal == afApplicationTree::DRAG_NEW_PROJECT))
        {
            retVal = afApplicationTree::DRAG_NO_ACTION;
        }
    }

    return retVal;
}

/// save all mdi windows that are related to the supplied filepath
void appApplicationCommands::SaveAllMDISubWindowsForFilePath(const osFilePath& filePath)
{
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();

    // Sanity check:
    GT_IF_WITH_ASSERT((pMainWindow != nullptr) && (pMainWindow->mdiArea() != nullptr) && (pApplicationCommands != nullptr))
    {
        // Get the MDI sub windows list:
        QList<QMdiSubWindow*> windowsSubList = pMainWindow->mdiArea()->subWindowList();

        foreach (QMdiSubWindow* pCurrentSubWindow, windowsSubList)
        {
            GT_IF_WITH_ASSERT(pCurrentSubWindow != nullptr)
            {
                // Get the widget from the window:
                afQMdiSubWindow* pAfQTSubWindow = qobject_cast<afQMdiSubWindow*>(pCurrentSubWindow);

                if (pAfQTSubWindow != nullptr)
                {
                    // Compare file names:
                    if (pAfQTSubWindow->filePath() == filePath)
                    {
                        acSourceCodeView* pSourceView = qobject_cast<acSourceCodeView*>(pAfQTSubWindow->widget());

                        if (pSourceView != nullptr && pSourceView->IsModified())
                        {
                            pApplicationCommands->saveMDIFile(filePath);
                            break;
                        }
                    }
                }
            }
        }
    }
}
