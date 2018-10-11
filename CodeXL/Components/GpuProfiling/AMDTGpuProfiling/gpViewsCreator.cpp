//=====================================================================
// Copyright (c) 2012 2018 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file
/// \brief  This file contains the MDI views added by the GPU Profiler plugin
//
//=====================================================================

// Qt
#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Local:
#include <AMDTGpuProfiling/gpViewsCreator.h>
#include <AMDTGpuProfiling/GpuSessionActionsCreator.h>
#include <AMDTGpuProfiling/SessionControl.h>
#include <AMDTGpuProfiling/SessionManager.h>
#include <AMDTGpuProfiling/SessionWindow.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/TraceView.h>

gpViewsCreator::gpViewsCreator()
{
    m_lastCreatedMDIType = GPUWindowTypeUnknown;


    // Create the view actions creator:
    _pViewActionCreator = new GpuSessionActionsCreator;

    _pViewActionCreator->initializeCreator();

    // Check if there is an environment variable for the trace views UI. By default use the old UI
    m_traceUI = OLD_UI;

    gtString envVal;
    bool rc = osGetCurrentProcessEnvVariableValue(L"CODEXL_TRACE_VIEW_UI", envVal);

    if (rc)
    {
        if (envVal == L"NEW")
        {
            m_traceUI = NEW_UI;
        }

        if (envVal == L"PROMPT")
        {
            m_traceUI = PROMPT_THE_USER;
        }
    }
}

void gpViewsCreator::titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand)
{
    Q_UNUSED(viewIndex);
    viewTitle = m_lastSessionFileOpened;
    viewMenuCommand = m_lastSessionFileOpened;
}

gtString gpViewsCreator::associatedToolbar(int viewIndex)
{
    Q_UNUSED(viewIndex);
    gtString retVal;
    //TODO: implement this
    return retVal;
}

afViewCreatorAbstract::afViewType gpViewsCreator::type(int viewIndex)
{
    Q_UNUSED(viewIndex);
    afViewCreatorAbstract::afViewType retViewType = AF_VIEW_mdi;
    return retViewType;
}

int gpViewsCreator::dockArea(int viewIndex)
{
    Q_UNUSED(viewIndex);
    int retVal = AF_VIEW_DOCK_LeftDockWidgetArea;
    return retVal;
}

bool gpViewsCreator::getCurrentlyDisplayedFilePath(osFilePath& filePath)
{
    if ((_pCreationEvent != nullptr) && (_pCreationEvent->eventType() == apEvent::AP_MDI_CREATED_EVENT))
    {
        //Add filename
        // Down cast the event:
        apMDIViewCreateEvent* pGpuProfileEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);
        filePath = pGpuProfileEvent->filePath();
        return true;
    }

    //GT_ASSERT(_pCreationEvent);
    return true; //return true or the main app window asserts on return
}

bool gpViewsCreator::createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    bool retVal = false;

    if ((_pCreationEvent != nullptr) && (_pCreationEvent->eventType() == apEvent::AP_MDI_CREATED_EVENT))
    {
        //Add filename
        // Down cast the event:
        apMDIViewCreateEvent* pGpuProfileEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);
        GT_IF_WITH_ASSERT(pGpuProfileEvent != nullptr)
        {
            // Get the requested profile from the manager:
            m_lastCreatedMDIType = GPUWindowTypeFromFilePath(pGpuProfileEvent->filePath());

            retVal = GetWidgetForFilePath(m_lastCreatedMDIType, pQParent, pGpuProfileEvent->filePath().asString(), pContentQWidget);
        }
    }


    if (!retVal)
    {
        pContentQWidget = nullptr;
    }
    else
    {
        // Set the created window:
        m_viewsCreated.push_back(pContentQWidget);
    }

    return retVal;
}

int gpViewsCreator::amountOfViewTypes()
{
    return GPU_PROFILE_TYPES_COUNT;
}

void gpViewsCreator::handleTrigger(int viewIndex, int actionIndex)
{
    (void)(viewIndex); // unused

    // Get the current active session window:
    SharedSessionWindow* pSession = GetCurrentActiveSessionWindow();

    if (nullptr != pSession)
    {
        // Get the command id:
        int commandId = actionIndexToCommandId(actionIndex);

        // Handle the action by its id:
        switch (commandId)
        {

            case ID_COPY:
                pSession->OnEditCopy();
                break;

            case ID_SELECT_ALL:
                pSession->OnEditSelectAll();
                break;

            case ID_FIND:
            {
                // Get the main window, and
                afMainAppWindow* pMainWindow = afMainAppWindow::instance();

                if (pMainWindow != nullptr)
                {
                    // Show the find widget, and do not respond to single characters click. We only want to search whole words, for performance reasons
                    pMainWindow->OnFind(false);
                }
            }
            break;

            case ID_FIND_NEXT:
            case ID_FIND_PREV:
                // When opening the find dialog, the find next & prev actions is connected, so we're connected to the
                // relevant slot:
                break;

            default:
            {
                GT_ASSERT_EX(false, L"Unsupported application command");
                break;
            }
        }
    }
}


QWidget* gpViewsCreator::CreateMDIWidget(QWidget* pParent, const osFilePath& sessionPath)
{
    QWidget* pRetVal = nullptr;

    osFilePath sessionFile(sessionPath);
    gtString fileExt;
    GPUWindowType windowType = GPUWindowTypeUnknown;

    if (sessionFile.getFileExtension(fileExt))
    {
        windowType = GPUWindowTypeFromFilePath(sessionPath);
    }

    if (!GetWidgetForFilePath(windowType, pParent, sessionPath, pRetVal))
    {
        pRetVal = nullptr;
    }

    return pRetVal;

}

SharedSessionWindow* gpViewsCreator::GetCurrentActiveSessionWindow()
{
    SharedSessionWindow* pRetVal = nullptr;

    // Get the current active sub window:
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();

    if (pMainWindow != nullptr)
    {
        // Get the current sub window:
        afQMdiSubWindow* pSubWindow = pMainWindow->activeMDISubWindow();

        if (pSubWindow != nullptr)
        {
            pRetVal = qobject_cast<SharedSessionWindow*>(pSubWindow->widget());
        }
    }

    return pRetVal;
}

void gpViewsCreator::handleUiUpdate(int viewIndex, int actionIndex)
{
    (void)(viewIndex); // unused
    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false;

    // Get the current active session window:
    SharedSessionWindow* pSession = GetCurrentActiveSessionWindow();

    if (nullptr != pSession)
    {
        // Handle the action by its id:
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {
            case ID_COPY:
            {
                pSession->onUpdateEdit_Copy(isActionEnabled);
            }
            break;

            case ID_SELECT_ALL:
            {
                pSession->onUpdateEdit_SelectAll(isActionEnabled);
            }
            break;

            case ID_FIND:
            {
                pSession->onUpdateEdit_Find(isActionEnabled);
            }
            break;

            case ID_FIND_NEXT:
            case ID_FIND_PREV:
            {
                pSession->onUpdateEdit_FindNext(isActionEnabled);
            }
            break;

            default:
                GT_ASSERT_EX(false, L"Unknown event id");
                isActionEnabled = false;
                break;
        };
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(_pViewActionCreator)
    {
        // Get the QT action:
        QAction* pAction = _pViewActionCreator->action(actionIndex);
        GT_IF_WITH_ASSERT(pAction != nullptr)
        {
            // Set the action enable / disable:
            pAction->setEnabled(isActionEnabled);

            // Set the action checkable state:
            pAction->setCheckable(isActionCheckable);

            // Set the action check state:
            pAction->setChecked(isActionChecked);
        }
    }
}

bool gpViewsCreator::displayExistingView(const apMDIViewCreateEvent& mdiViewEvent)
{
    bool retVal = false;

    // Down cast to apMDIViewCreateEvent:
    const apMDIViewCreateEvent& gpuProfileViewEvent = (const apMDIViewCreateEvent&)mdiViewEvent;
    osFilePath filePath = gpuProfileViewEvent.filePath();
    osFilePath fileToMap = filePath;

    // Get the window type from the file path
    GPUWindowType windowType = GPUWindowTypeFromFilePath(filePath);

    // Try to find an existing window for this file path
    SharedSessionWindow* pSessionWindow = FindMatchingWindow(filePath, fileToMap, windowType);

    if (pSessionWindow != nullptr)
    {
        TraceView* pTraceView = dynamic_cast<TraceView*>(pSessionWindow);

        if (pTraceView != nullptr)
        {
            pTraceView->DisplaySummaryPageType(gpuProfileViewEvent.viewIndex());
        }

        // Just display the session, already exist
        retVal = true;
    }

    if (!retVal)
    {
        ProfileApplicationTreeHandler* pTreeHandler = ProfileApplicationTreeHandler::instance();
        GT_IF_WITH_ASSERT(pTreeHandler != nullptr)
        {
            GPUSessionTreeItemData* pSessionData = GetSessionFromTempPCFile(mdiViewEvent.filePath());

            if (pSessionData == nullptr)
            {
                afApplicationTreeItemData* pItemData = pTreeHandler->FindItemByProfileFilePath(mdiViewEvent.filePath());

                if (pItemData != nullptr)
                {
                    pSessionData = qobject_cast<GPUSessionTreeItemData*>(pItemData->extendedItemData());
                }
            }

            if (pSessionData != nullptr)
            {
                // The view title is the display name:
                m_lastSessionFileOpened = acQStringToGTString(pSessionData->m_displayName);
                gtString sessionType = acQStringToGTString(pSessionData->m_profileTypeStr);

                if (windowType == GPUWindowTypePerformanceCounters)
                {
                    m_lastSessionFileOpened.appendFormattedString(L" (GPU: %ls)", sessionType.asCharArray());
                }
                else if (windowType == GPUWindowTypeAPITrace)
                {
                    m_lastSessionFileOpened.appendFormattedString(L" (%ls)", sessionType.asCharArray());
                }
            }
            else
            {
                m_lastSessionFileOpened = mdiViewEvent.filePath().asString();
            }
        }
    }

    return retVal;
}

SharedSessionWindow* gpViewsCreator::FindMatchingWindow(const osFilePath& filePath, osFilePath& fileToMap, GPUWindowType windowType)
{
    // Try to see if this session is already opened
    SharedSessionWindow* pRetVal = nullptr;
    fileToMap = filePath;

    if (m_filePathToSessionWindowsMap.count(filePath))
    {
        pRetVal = m_filePathToSessionWindowsMap[filePath];
    }
    else if (windowType == GPUWindowTypePerformanceCounters)
    {
        QString mappedFilePath;
        GetSessionFileFromTempPCFile(fileToMap, mappedFilePath);
        osFilePath mappedPath(acQStringToGTString(mappedFilePath));

        if (m_filePathToSessionWindowsMap.contains(mappedPath))
        {
            // Get the session window
            pRetVal = m_filePathToSessionWindowsMap[mappedPath];
            fileToMap = mappedPath;
        }
    }

    return pRetVal;
}

void gpViewsCreator::ShowSession(GPUSessionTreeItemData* pSession, afTreeItemType treeItemType)
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pSession != nullptr && pSession->m_pParentData != nullptr && pApplicationCommands != nullptr)
    {
        osFilePath sessionFile;

        if (pSession->GetProfileType() == PERFORMANCE)
        {
            // Get the temporary file name
            bool rc = GetTempPCFile(pSession->m_projectName, pSession->m_displayName, sessionFile);
            GT_ASSERT(rc);
        }
        else
        {
            sessionFile = pSession->m_pParentData->m_filePath;
        }

        pApplicationCommands->OpenFileAtLine(sessionFile, treeItemType, -1);
    }
}

void gpViewsCreator::CreateTempPCFile(GPUSessionTreeItemData* pSession)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pSession != nullptr) && (pSession->m_pParentData != nullptr))
    {
        gtString ext;
        pSession->m_pParentData->m_filePath.getFileExtension(ext);

        // If we're in VS
        if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio() && (ext == GP_CSV_FileExtensionW))
        {
            // Generate the temp pc file name
            osFilePath sessionFile;
            bool rc = GetTempPCFile(pSession->m_projectName, pSession->m_displayName, sessionFile);
            GT_IF_WITH_ASSERT(rc)
            {
                // Write the actual session file name to the temp file
                osFile objectfile;
                rc = objectfile.open(sessionFile, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
                GT_IF_WITH_ASSERT(rc)
                {
                    rc = objectfile.writeString(pSession->m_pParentData->m_filePath.asString());
                    GT_IF_WITH_ASSERT(rc)
                    {
                        objectfile.close();
                    }

                    // Set the temp file as file path in the item data
                    pSession->m_pParentData->m_filePath = sessionFile;

                    PerformanceCounterSession* pPerformanceCounterSession = qobject_cast<PerformanceCounterSession*>(pSession);
                    GT_IF_WITH_ASSERT(pPerformanceCounterSession != nullptr)
                    {
                        // Set the session temporary file:
                        pPerformanceCounterSession->SetSessionTempFile(sessionFile);
                    }
                }
            }
        }
    }
}

void gpViewsCreator::HideSession(const osFilePath& sessionPath)
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        osFilePath sessionFile = sessionPath;

        if (GPUWindowTypeFromFilePath(sessionPath) == GPUWindowTypePerformanceCounters)
        {
            SessionTreeNodeData* pSessionData = nullptr;
            afApplicationTreeItemData* pItemData = ProfileApplicationTreeHandler::instance()->FindItemByProfileFilePath(sessionPath);

            if (pItemData != nullptr)
            {
                pSessionData = qobject_cast<SessionTreeNodeData*>(pItemData->extendedItemData());
            }

            if (pSessionData != nullptr)
            {
                if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
                {
                    // In VS we want to remove the temp performance counters file
                    bool rc = GetTempPCFile(pSessionData->m_projectName, pSessionData->m_displayName, sessionFile);
                    GT_ASSERT(rc);
                }
            }
        }

        pApplicationCommands->closeFile(sessionFile);
    }
}

bool gpViewsCreator::GetWidgetForFilePath(GPUWindowType profileType, QWidget* pParent, const osFilePath& sessionPath, QWidget*& pWidget)
{
    bool retVal = false;

    QString errorMessage;
    SharedSessionWindow* pCreatedSessionWindow = nullptr;
    osFilePath pathToDisplay = sessionPath;

    switch (profileType)
    {
        case GPUWindowTypePerformanceCounters:
        {
            pCreatedSessionWindow = CreatePerfCountersSessionWindow(pParent, sessionPath, pathToDisplay);
            break;
        }

        case GPUWindowTypeAPITrace:
        {
            pCreatedSessionWindow = CreateAppTraceSessionWindow(pParent);
            break;
        }

        default:
        {
            GT_ASSERT(retVal);
        }
    }

    GT_IF_WITH_ASSERT(nullptr != pCreatedSessionWindow)
    {
        pWidget = pCreatedSessionWindow;

        // Get the requested item type for the display
        afTreeItemType itemType = AF_TREE_ITEM_ITEM_NONE;
        apMDIViewCreateEvent* pGpuProfileEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);

        if (pGpuProfileEvent != nullptr)
        {
            itemType = (afTreeItemType)pGpuProfileEvent->lineNumber();
        }

        if (itemType == AF_TREE_ITEM_ITEM_NONE)
        {
            itemType = AF_TREE_ITEM_PROFILE_GPU_BEST_PRACTICE_SUMMARY;
        }

        // Load the session into the view
        retVal = LoadFileToView(pCreatedSessionWindow, pathToDisplay, itemType, errorMessage);
    }

    if (retVal)
    {
        // Insert the session window to the map
        m_filePathToSessionWindowsMap[pathToDisplay] = pCreatedSessionWindow;
    }
    else
    {
        if (errorMessage.isEmpty())
        {
            errorMessage = GP_Str_ErrorWhileLoadingSession;
        }

        Util::ShowErrorBox(errorMessage);
    }

    return retVal;
}


GPUWindowType gpViewsCreator::GPUWindowTypeFromFilePath(const osFilePath& filePath)
{
    GPUWindowType retVal = GPUWindowTypeUnknown;

    if (filePath.IsMatchingExtension(AF_STR_GpuProfileTraceFileExtension))
    {
        retVal = GPUWindowTypeAPITrace;
    }
    else if (filePath.IsMatchingExtension(AF_STR_GpuProfileSessionFileExtension) || filePath.IsMatchingExtension(AF_STR_profileFileExtension4))
    {
        retVal = GPUWindowTypePerformanceCounters;
    }

    return retVal;
}

bool gpViewsCreator::GetSessionFileFromTempPCFile(const osFilePath& tempPCFile, QString& sessionFile)
{
    bool retVal = false;
    sessionFile = acGTStringToQString(tempPCFile.asString());

    gtString fileExt;
    sessionFile = acGTStringToQString(tempPCFile.asString());

    if (tempPCFile.getFileExtension(fileExt))
    {
        if (fileExt == AF_STR_GpuProfileSessionFileExtension)
        {
            // Open the input session file:
            osFile inputSessionFile;
            bool rc = inputSessionFile.open(tempPCFile, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);

            if (rc)
            {
                // Load the input source code into a string:
                gtString inputSession;
                rc = inputSessionFile.readIntoString(inputSession);

                // Close the file:
                inputSessionFile.close();

                // If we managed to read the file contents
                GT_IF_WITH_ASSERT(rc)
                {
                    retVal = true;
                    sessionFile = acGTStringToQString(inputSession);
                }
            }
        }
        else if (fileExt == AF_STR_profileFileExtension4)
        {
            retVal = true;
        }
    }

    return retVal;
}

GPUSessionTreeItemData* gpViewsCreator::GetSessionFromTempPCFile(const osFilePath& tempPCFile)
{
    GPUSessionTreeItemData* retVal = nullptr;
    QString sessionFile;

    if (GetSessionFileFromTempPCFile(tempPCFile, sessionFile))
    {
        ProfileApplicationTreeHandler* pTreeHandler = ProfileApplicationTreeHandler::instance();
        GT_IF_WITH_ASSERT(pTreeHandler != nullptr)
        {
            osFilePath filePath(acQStringToGTString(sessionFile));
            afApplicationTreeItemData* pItemData = pTreeHandler->FindItemByProfileFilePath(filePath);

            if (pItemData != nullptr)
            {
                retVal = qobject_cast<GPUSessionTreeItemData*>(pItemData->extendedItemData());
            }
        }
    }

    return retVal;
}

void gpViewsCreator::UpdateTitleString(const osFilePath& oldSessionFileName, const osFilePath& newSessionFileName)
{
    // Get the renamed session profile type
    GPUWindowType profileType = GPUWindowTypeFromFilePath(oldSessionFileName);

    SharedSessionWindow* pSessionWindow = nullptr;

    if (m_filePathToSessionWindowsMap.contains(oldSessionFileName))
    {
        pSessionWindow = m_filePathToSessionWindowsMap[oldSessionFileName];
    }

    if (pSessionWindow != nullptr)
    {
        // Remove the old map entrance and add a new one
        m_filePathToSessionWindowsMap.remove(oldSessionFileName);
        m_filePathToSessionWindowsMap[newSessionFileName] = pSessionWindow;

        // Update the renamed session
        pSessionWindow->UpdateRenamedSession(oldSessionFileName, newSessionFileName);

        // Find the sub window in the widget hierarchy
        QMdiSubWindow* pSubWindow = qobject_cast<QMdiSubWindow*>(pSessionWindow);
        QWidget* pWidget = (QWidget*)pSessionWindow;

        while ((pSubWindow == nullptr) && ((pWidget != nullptr && pWidget->parentWidget() != nullptr)))
        {
            pWidget = pWidget->parentWidget();
            pSubWindow = qobject_cast<QMdiSubWindow*>(pWidget);
        }

        if (pSubWindow != nullptr)
        {
            // setting the window title will cause the tab text to change
            QString newTitle;

            if (profileType == GPUWindowTypePerformanceCounters)
            {
                // Add GPU prefix for performance counters profiles:
                newTitle.append(GP_Str_ProfileSessionGPUPrefix);
            }

            gtString displayName;
            newSessionFileName.getFileName(displayName);
            newTitle.append(acGTStringToQString(displayName));
            QString oldTitle = pSubWindow->windowTitle();

            pWidget->setWindowTitle(newTitle);

            // If this is an SA, rename the window in framework:
            afMainAppWindow* pMainWindow = afMainAppWindow::instance();

            if (pMainWindow != nullptr)
            {
                pMainWindow->renameMDIWindow(oldTitle, newTitle, acGTStringToQString(newSessionFileName.asString()));
            }
        }
    }
}

gtString gpViewsCreator::GetViewTitleForFile(const osFilePath& fileName)
{
    gtString retVal;
    ProfileApplicationTreeHandler* pTreeHandler = ProfileApplicationTreeHandler::instance();
    GT_IF_WITH_ASSERT(pTreeHandler != nullptr)
    {
        QString strFileName = acGTStringToQString(fileName.asString());
        GPUSessionTreeItemData* pSessionData = GetSessionFromTempPCFile(fileName);

        GPUProfileType profileType = NA_PROFILE_TYPE;

        if (pSessionData == nullptr)
        {
            afApplicationTreeItemData* pItemData = pTreeHandler->FindItemByProfileFilePath(fileName);

            if (pItemData != nullptr)
            {
                pSessionData = qobject_cast<GPUSessionTreeItemData*>(pItemData->extendedItemData());
            }
        }

        // if the fileName passed in does not represent a known session, then try to create a dummy session, read the display name from it, then destroy the dummy session
        // this is used when Visual Studio automatically loads previous session views when a solution is loaded
        QString strDisplayName;

        if (pSessionData == nullptr)
        {
            osFilePath localFilePath = fileName;
            // check if the fileName passed in is a .gpsession file, and if so, then extract the real session file from it
            QString strSessionFile;

            if (GetSessionFileFromTempPCFile(localFilePath, strSessionFile))
            {
                strFileName = strSessionFile;
                localFilePath.setFullPathFromString(acQStringToGTString(strFileName));
            }

            // check the profile type of the file -- if valid then create the dummy session, read the display name, and then destroy it
            profileType = SessionManager::Instance()->GetProfileType(strFileName);

            if (profileType != NA_PROFILE_TYPE)
            {
                osDirectory fileDir;
                localFilePath.getFileDirectory(fileDir);
                QString strDirName = acGTStringToQString(fileDir.directoryPath().asString());
                QDir dirSession(strDirName);
                pSessionData = new(std::nothrow) GPUSessionTreeItemData(dirSession.dirName(), "", "", "", profileType, false);

                strDisplayName = pSessionData->m_displayName;
                SAFE_DELETE(pSessionData);
            }
            else
            {
                // if the profile type is not valid, then just show the filename and extension
                gtString fileNameAndExt;
                fileName.getFileNameAndExtension(fileNameAndExt);
                strDisplayName = acGTStringToQString((fileNameAndExt));
            }
        }
        else
        {
            strDisplayName = pSessionData->m_displayName;
        }

        QString tabTitle;

        if (profileType == PERFORMANCE)
        {
            tabTitle.append(GP_Str_ProfileSessionGPUPrefix);
        }

        tabTitle.append(strDisplayName);
        retVal = acQStringToGTString(tabTitle);

    }
    return retVal;
}


QPixmap* gpViewsCreator::iconAsPixmap(int viewIndex)
{
    (void)(viewIndex); // unused
    QPixmap* pRetVal = nullptr;

    // Get the icon according to the last created window type
    GPUProfileType profileType = API_TRACE;

    if (m_lastCreatedMDIType == GPUWindowTypePerformanceCounters)
    {
        profileType = PERFORMANCE;
    }

    pRetVal = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(AF_TREE_ITEM_PROFILE_SESSION, Util::GetProfileTypeName(profileType));

    return pRetVal;
}

SharedSessionWindow* gpViewsCreator::CreatePerfCountersSessionWindow(QWidget* pParent, const osFilePath& sessionPath, osFilePath& pathToDisplay)
{
    SharedSessionWindow* pRetVal = nullptr;


    GPUSessionWindow* pSessionWindow = new GPUSessionWindow(pParent);
    pRetVal = pSessionWindow;

    // Get the session for this file:
    GPUSessionTreeItemData* pSessionData = GetSessionFromTempPCFile(sessionPath);

    if ((pSessionData != nullptr) && (pSessionData->m_pParentData != nullptr))
    {
        pathToDisplay = pSessionData->m_pParentData->m_filePath;
    }

    return pRetVal;
};

SharedSessionWindow* gpViewsCreator::CreateAppTraceSessionWindow(QWidget* pParent)
{
    SharedSessionWindow* pRetVal = nullptr;

    if (m_traceUI == OLD_UI)
    {
        // Create a new trace view window:
        pRetVal = new TraceView(pParent);
    }

    return pRetVal;
}

void gpViewsCreator::OnWindowClose(QWidget* pClosedSessionWindow)
{
    if (pClosedSessionWindow != nullptr)
    {
        SharedSessionWindow* pSessionWindow = qobject_cast<SharedSessionWindow*>(pClosedSessionWindow);

        if (pSessionWindow != nullptr)
        {
            // Remove the session window from the windows map
            if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())//patch for perfromance counters
            {
                osFilePath tempPCFilePath;
                SessionTreeNodeData* pSessionData = pSessionWindow->SessionTreeData();
                GT_IF_WITH_ASSERT(pSessionData != nullptr)
                {
                    bool rc = GetTempPCFile(pSessionData->m_projectName, pSessionData->m_displayName, tempPCFilePath);
                    GT_ASSERT(rc);

                    if (m_filePathToSessionWindowsMap.contains(tempPCFilePath))
                    {
                        m_filePathToSessionWindowsMap.remove(tempPCFilePath);
                    }
                }
            }

            if (m_filePathToSessionWindowsMap.contains(pSessionWindow->SessionFilePath()))
            {
                m_filePathToSessionWindowsMap.remove(pSessionWindow->SessionFilePath());
            }
        }

        TraceView* pTraceView = qobject_cast<TraceView*>(pClosedSessionWindow);

        if (pTraceView != nullptr)
        {
            if (pTraceView->TimelinePropertiesAreSet())
            {
                // If trace view was close, clear the properties view:
                afApplicationCommands::instance()->RestorePropertiesViewToCurrentlySelectedItem();
            }
        }
    }
}

bool gpViewsCreator::GetTempPCFile(const QString& projectName, const QString& sessionName, osFilePath& tempPCFile)
{
    bool retVal = false;

    afGetUserDataFolderPath(tempPCFile);

    // Add the VS_Cache files directory:
    tempPCFile.appendSubDirectory(AF_STR_CSV_FILE_CACHE);

    // Create the folder if not created:
    osDirectory directoryPath;
    directoryPath.setDirectoryPath(tempPCFile);
    bool rcCreateDir = directoryPath.create();
    GT_IF_WITH_ASSERT(rcCreateDir)
    {
        tempPCFile.appendSubDirectory(acQStringToGTString(projectName));
        directoryPath.setDirectoryPath(tempPCFile);
        directoryPath.create();

        // Create a text file which contains the name of the .csv file
        // Write the files to the cache folder:
        tempPCFile.setFileName(acQStringToGTString(sessionName));
        tempPCFile.setFileExtension(AF_STR_GpuProfileSessionFileExtension);
        retVal = true;
    }

    return retVal;
}

void gpViewsCreator::OnSessionDelete(const gtString& deletedSessionFilePath)
{
    GT_UNREFERENCED_PARAMETER(deletedSessionFilePath);
}

bool gpViewsCreator::LoadFileToView(SharedSessionWindow* pNewSessionWindow, const osFilePath& sessionFilePath, afTreeItemType displayItemInView, QString& errorMessage)
{
    bool retVal = false;

    ProfileApplicationTreeHandler* pTreeHandler = ProfileApplicationTreeHandler::instance();
    GT_IF_WITH_ASSERT(pTreeHandler != nullptr)
    {
        // Check if the tree already contain this item. If not, it means that the tree items are not ready, and the
        // session should be added for later load
        afApplicationTreeItemData* pItemData = pTreeHandler->FindItemByProfileFilePath(sessionFilePath);

        if (pItemData != nullptr)
        {
            GPUWindowType winType = GPUWindowTypeFromFilePath(sessionFilePath);

            if ((winType == GPUWindowTypeAPITrace) || (winType == GPUWindowTypePerformanceCounters))
            {
                // Get the requested profile from the manager
                retVal = pNewSessionWindow->DisplaySession(sessionFilePath, displayItemInView, errorMessage);
            }
        }
    }

    return retVal;

}

gtString gpViewsCreator::SessionFilePathToSessionName(const osFilePath& sessionFilePath)
{
    gtString retVal;

    if (sessionFilePath.IsMatchingExtension(AF_STR_GpuProfileSessionFileExtension) ||
        sessionFilePath.IsMatchingExtension(AF_STR_profileFileExtension4) ||
        sessionFilePath.IsMatchingExtension(AF_STR_GpuProfileTraceFileExtension))
    {
        sessionFilePath.getFileName(retVal);
    }

    return retVal;
}

