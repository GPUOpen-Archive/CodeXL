//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionViewCreator.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/SessionViewCreator.cpp#88 $
// Last checkin:   eDateTime: $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

//QT
#include <QtCore>
#include <QtWidgets>
#include <QAction>

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

// Shared profiling:
#include <ProfileApplicationTreeHandler.h>

// local:
#include <inc/StringConstants.h>
#include <inc/CommandIds.h>
#include "inc/SessionViewCreator.h"
#include <inc/CommandsHandler.h>
#include <inc/SessionWindow.h>
#include <inc/SessionActions.h>


SessionViewCreator::SessionViewCreator() : afQtViewCreatorAbstract(), m_pCommandsHandler(nullptr), m_showInfoPanel(true)
{
    // Get the application commands handler:
    m_pCommandsHandler = CommandsHandler::instance();
    GT_ASSERT(m_pCommandsHandler != nullptr);

    // Create the view actions creator:
    _pViewActionCreator = new SessionActions;

    _pViewActionCreator->initializeCreator();
}

SessionViewCreator::~SessionViewCreator()
{
}
void SessionViewCreator::titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand)
{
    const CPUSessionTreeItemData* pCPUData = nullptr;

    if ((viewIndex >= 0) && (viewIndex < (int)m_sessionWindowsVector.size()))
    {
        CpuSessionWindow* pSessionWindow = m_sessionWindowsVector[viewIndex];

        if (pSessionWindow != nullptr)
        {
            pCPUData = pSessionWindow->displayedCPUSessionItemData();
        }
    }
    else
    {
        // Get the item data from profile file path:
        if (_pCreationEvent != nullptr)
        {
            GT_IF_WITH_ASSERT(_pCreationEvent->eventType() == apEvent::AP_MDI_CREATED_EVENT)
            {
                // Down cast the event:
                apMDIViewCreateEvent* pCpuProfileEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);
                GT_IF_WITH_ASSERT(pCpuProfileEvent != nullptr)
                {
                    // Get the item data for this session from tree:
                    afApplicationTreeItemData* pItemData = ProfileApplicationTreeHandler::instance()->FindItemByProfileFilePath(pCpuProfileEvent->filePath());

                    if (pItemData != nullptr)
                    {
                        pCPUData = qobject_cast<CPUSessionTreeItemData*>(pItemData->extendedItemData());
                    }
                }
            }
        }
    }

    if (pCPUData != nullptr)
    {
        // The view title is the display name:
        viewTitle = acQStringToGTString(pCPUData->m_displayName);
        gtString sessionType = acQStringToGTString(pCPUData->m_profileTypeStr);
        viewTitle.appendFormattedString(L" (CPU: %ls)", sessionType.asCharArray());
    }

    viewMenuCommand = viewTitle;
}


// Get the associated toolbar string:
gtString SessionViewCreator::associatedToolbar(int viewIndex)
{
    (void)(viewIndex); // unused
    gtString retVal;
    return retVal;
}

// Get view type:
afViewCreatorAbstract::afViewType SessionViewCreator::type(int viewIndex)
{
    (void)(viewIndex); // unused
    afViewCreatorAbstract::afViewType retDockArea = AF_VIEW_mdi;

    return retDockArea;
}

// Get the docking area:
int SessionViewCreator::dockArea(int viewIndex)
{
    (void)(viewIndex); // unused
    return AF_VIEW_DOCK_LeftDockWidgetArea;
}

// Get the docking features:
QDockWidget::DockWidgetFeatures SessionViewCreator::dockWidgetFeatures(int viewIndex)
{
    (void)(viewIndex); // unused
    return QDockWidget::NoDockWidgetFeatures;
}

// Get the initial size:
QSize SessionViewCreator::initialSize(int viewIndex)
{
    (void)(viewIndex); // unused
    return QSize(100, 200);
}

// Get the initial visibility of the view:
bool SessionViewCreator::visibility(int viewIndex)
{
    (void)(viewIndex); // unused

    if (nullptr == m_pCommandsHandler)
    {
        return true;
    }

    return true;
}

int SessionViewCreator::amountOfViewTypes()
{
    return m_sessionWindowsVector.size();
}


// Create the inner view:
bool SessionViewCreator::createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent)
{
    (void)(viewIndex); // unused
    (void)(pQParent); // unused
    bool retVal = false;
    pContentQWidget = nullptr;

    osFilePath requestedFilePath;

    GT_IF_WITH_ASSERT((_pCreationEvent != nullptr) && (_pCreationEvent->eventType() == apEvent::AP_MDI_CREATED_EVENT))
    {
        // Down cast the event:
        apMDIViewCreateEvent* pCpuProfileEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);
        GT_IF_WITH_ASSERT(pCpuProfileEvent != nullptr)
        {
            // Set the requested file path:
            requestedFilePath = pCpuProfileEvent->filePath();

            // Check if the created view is already in the vector:
            if (!m_sessionWindowsVector.empty())
            {
                CpuSessionWindow* pSessionWindow = m_sessionWindowsVector.back();

                if (pSessionWindow != nullptr)
                {
                    const CPUSessionTreeItemData* pItemData = pSessionWindow->displayedCPUSessionItemData();

                    if ((pItemData != nullptr) && (pItemData->m_pParentData != nullptr))
                    {
                        retVal = (pItemData->m_pParentData->m_filePath == pCpuProfileEvent->filePath());
                    }
                }
            }
        }
    }

    // View was not yet created:
    if (!retVal)
    {
        // Get the application mdi window instance:
        afMainAppWindow* pMainAppWindow = afMainAppWindow::instance();
        GT_IF_WITH_ASSERT(pMainAppWindow != nullptr)
        {
            GT_IF_WITH_ASSERT(ProfileApplicationTreeHandler::instance() != nullptr)
            {

                afApplicationTreeItemData* pSessionItemData = ProfileApplicationTreeHandler::instance()->FindItemByProfileFilePath(requestedFilePath);
                GT_IF_WITH_ASSERT(pSessionItemData != nullptr)
                {
                    // Create a new session window:
                    CpuSessionWindow* pSessionWindow = new CpuSessionWindow(pSessionItemData);


                    // Initialize the window:
                    pSessionWindow->initialize();

                    m_sessionWindowsVector.push_back(pSessionWindow);

                    gtString viewTitle, viewCommandName;
                    int viewIndexLast = m_sessionWindowsVector.size() - 1;
                    titleString(viewIndexLast, viewTitle, viewCommandName);
                    pSessionWindow->setWindowTitle(QString::fromWCharArray(viewTitle.asCharArray()));

                    // Display the relevant cpu item page:
                    if ((_pCreationEvent != nullptr) && (_pCreationEvent->eventType() == apEvent::AP_MDI_CREATED_EVENT))
                    {
                        //Add filename
                        // Down cast the event:
                        apMDIViewCreateEvent* pCpuProfileEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);
                        GT_IF_WITH_ASSERT(pCpuProfileEvent != nullptr)
                        {
                            QString errorMessage;
                            bool rc = pSessionWindow->DisplaySession(pCpuProfileEvent->filePath(), pCpuProfileEvent->filePath2(), (afTreeItemType)pCpuProfileEvent->lineNumber(), errorMessage);
                            GT_ASSERT(rc);

                            // If the window failed to open, let the user know what was the problem
                            if (!errorMessage.isEmpty())
                            {
                                acMessageBox::instance().critical(afGlobalVariablesManager::instance().ProductNameA(), errorMessage);
                            }
                        }
                    }

                    pContentQWidget = pSessionWindow;

                    retVal = true;
                }
            }
        }
    }

    // Set the created window:
    m_viewsCreated.push_back(pContentQWidget);

    return retVal;
}

// Handle the action when it is triggered
void SessionViewCreator::handleTrigger(int viewIndex, int actionIndex)
{
    (void)(viewIndex); // unused

    // Get the current active session window:
    CpuSessionWindow* pSession = GetCurrentActiveSessionWindow();
    // Get the current active sub window:
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();

    if (nullptr != pSession)
    {
        // Handle the action by its id:
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {

            case ID_COPY:
                pSession->OnEditCopy();
                break;

            case ID_FIND:
                if (nullptr != pMainWindow)
                {
                    pMainWindow->OnFind(true);
                }

                break;

            case ID_FIND_NEXT:
                // When opening the find dialog, the find next action is connected, so we're connected to the
                // relevant slot:
                break;

            case ID_SELECT_ALL:
                pSession->OnEditSelectAll();
                break;

            default:
            {
                GT_ASSERT_EX(false, L"Unsupported application command");
                break;
            }
        }
    }
}

// handle UI update
void SessionViewCreator::handleUiUpdate(int viewIndex, int actionIndex)
{
    (void)(viewIndex); // unused
    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false;

    // Get the current active session window:
    CpuSessionWindow* pSession = GetCurrentActiveSessionWindow();

    if (nullptr != pSession)
    {
        // Handle the action by its id:
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {
            case ID_COPY:
            case ID_FIND:
            case ID_FIND_NEXT:
            case ID_SELECT_ALL:
                isActionEnabled = true;
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

bool SessionViewCreator::displayExistingView(const apMDIViewCreateEvent& mdiViewEvent)
{
    bool retVal = false;

    // Down cast to apMDIViewCreateEvent:
    const apMDIViewCreateEvent& cpuProfileViewEvent = (const apMDIViewCreateEvent&)mdiViewEvent;

    // Look for the session window displaying the session file path:
    CpuSessionWindow* pSessionWindow = nullptr;

    for (CpuSessionWindow* pCurrentWindow : m_sessionWindowsVector)
    {
        if (pCurrentWindow != nullptr)
        {
            // If the current session display the requested file path:
            if (pCurrentWindow->displayedSessionFilePath() == mdiViewEvent.filePath())
            {
                pSessionWindow = pCurrentWindow;
                break;
            }
        }
    }


    if (pSessionWindow != nullptr)
    {
        QString errorMessage;
        retVal = pSessionWindow->DisplaySession(cpuProfileViewEvent.filePath(), cpuProfileViewEvent.filePath2(), (afTreeItemType)cpuProfileViewEvent.lineNumber(), errorMessage);

        // If the window failed to open, let the user know what was the problem
        if (!errorMessage.isEmpty() && !retVal)
        {
            acMessageBox::instance().critical(afGlobalVariablesManager::instance().ProductNameA(), errorMessage);
        }
    }

    return retVal;
}

bool SessionViewCreator::onMDISubWindowClose(afQMdiSubWindow* pMDISubWindow)
{
    GT_IF_WITH_ASSERT(pMDISubWindow != nullptr)
    {
        bool wasWindowFound = false;

        QWidget* pWidget = pMDISubWindow->widget();

        if (nullptr != pWidget)
        {
            CpuSessionWindow* pSession = qobject_cast<CpuSessionWindow*>(pWidget);

            if (nullptr != pSession)
            {
                gtVector<CpuSessionWindow*>::iterator it = m_sessionWindowsVector.begin();
                gtVector<CpuSessionWindow*>::iterator endIt = m_sessionWindowsVector.end();

                for (; it != endIt; it++)
                {
                    if (*it == pSession)
                    {
                        // If this is the window, remove it:
                        m_sessionWindowsVector.erase(it);
                        wasWindowFound = true;
                        break;
                    }
                }
            }
        }

        if (!wasWindowFound)
        {
            // In some cases, the MDI window is about to be closed when the widget is nullptr. In these cases,
            // we should look for the session window according to the file path, if the file path is a CPU session file path:


            // Get the closed session file path:
            osFilePath closedSessionPath = pMDISubWindow->filePath();
            gtString extension;
            closedSessionPath.getFileExtension(extension);

            if (extension == AF_STR_CpuProfileFileExtension)
            {
                // Go through each of the sessions, and erase the session window that is related to this file path:
                gtVector<CpuSessionWindow*>::iterator it = m_sessionWindowsVector.begin();
                gtVector<CpuSessionWindow*>::iterator endIt = m_sessionWindowsVector.end();

                for (; it != endIt; it++)
                {
                    CpuSessionWindow* pCPUSessionWindow = *it;

                    if (pCPUSessionWindow != nullptr)
                    {
                        if (pCPUSessionWindow->displayedSessionFilePath() == closedSessionPath)
                        {
                            // If this is the window, remove it:
                            m_sessionWindowsVector.erase(it);
                            wasWindowFound = true;
                            break;
                        }
                    }
                }
            }
        }

    }

    return true;
}

void SessionViewCreator::openSession(const gtString& sessionPath, const gtString& moduleFilePath, afTreeItemType cpuItemType)
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        // Translate the item type to line number:
        int lineNumber = -1;
        bool isTypeValid = ((cpuItemType >= AF_TREE_ITEM_PROFILE_SESSION) && (cpuItemType < AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODES));

        if (isTypeValid || (cpuItemType == AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODE))
        {
            lineNumber = (int)(cpuItemType);
        }

        if (moduleFilePath.isEmpty())
        {
            bool rc = pApplicationCommands->OpenFileAtLine(sessionPath, (int)cpuItemType, -1);
            GT_ASSERT(rc);

            if (!rc)
            {
                // Warn the user the session file can not be opened
                acMessageBox::instance().warning(CPU_PROF_MESSAGE, QString::fromWCharArray(L"Session file can not be opened"), QMessageBox::Ok);
                gtString message;
                message.appendFormattedString(L"Could not open session file. Session file path: %ls", sessionPath.asCharArray());
                OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_ERROR);
            }
        }
        else
        {
            bool isSessionOpened = false;

            for (int i = 0 ; i < (int)m_sessionWindowsVector.size(); i++)
            {
                if (m_sessionWindowsVector[i] != nullptr)
                {
                    if (m_sessionWindowsVector[i]->displayedSessionFilePath() == sessionPath)
                    {
                        isSessionOpened = true;
                        break;
                    }
                }
            }

            if (!isSessionOpened)
            {
                bool rc = pApplicationCommands->OpenFileAtLine(sessionPath, (int)cpuItemType, -1);
                GT_ASSERT(rc);

                if (!rc)
                {
                    // Warn the user the session file can not be opened
                    acMessageBox::instance().warning(CPU_PROF_MESSAGE, QString::fromWCharArray(L"Session file can not be opened"), QMessageBox::Ok);
                    gtString message;
                    message.appendFormattedString(L"Could not open session file. Session file path: %ls", sessionPath.asCharArray());
                    OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_ERROR);
                }
            }

            // Trigger a Cpu Profile view creation event:
            apMDIViewCreateEvent cpuProfileViewEvent(AF_STR_CPUProfileViewsCreatorID, sessionPath, L"", -1, lineNumber);
            cpuProfileViewEvent.SetSecondFilePath(moduleFilePath);
            apEventsHandler::instance().registerPendingDebugEvent(cpuProfileViewEvent);
        }
    }
}

/// Closes an open session window, if applicable
bool SessionViewCreator::closeSessionBeforeDeletion(const gtString& sessionPath)
{
    // Close the session window, if it's open
    osFilePath filePath(sessionPath);

    // Find the session window for this session (we need to close it's profile reader, to enable the file deletion)
    auto itr = find_if(m_sessionWindowsVector.begin(), m_sessionWindowsVector.end(), [&](const CpuSessionWindow * pSessionWindow)
    {
        bool result = false;
        GT_IF_WITH_ASSERT(pSessionWindow != nullptr)
        {
            result = pSessionWindow->displayedSessionFilePath() == filePath;
        }
        return result;
    });

    if (itr != m_sessionWindowsVector.end())
    {
        (*itr)->OnBeforeDeletion();
    }

    // by closing the mdi view in SA or view by VS the Qt child window is deleted. no need to delete it locally.

    return afApplicationCommands::instance()->closeFile(filePath);
}

bool SessionViewCreator::getCurrentlyDisplayedFilePath(osFilePath& filePath)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((_pCreationEvent != nullptr) && (_pCreationEvent->eventType() == apEvent::AP_MDI_CREATED_EVENT))
    {
        // Down cast the event:
        apMDIViewCreateEvent* pCpuProfileEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);
        GT_IF_WITH_ASSERT(pCpuProfileEvent != nullptr)
        {
            filePath = pCpuProfileEvent->filePath();
            retVal = true;
        }
    }

    if (!retVal)
    {
        // Get the main window:
        afMainAppWindow* pMainWindow = afMainAppWindow::instance();

        if (pMainWindow != nullptr)
        {
            // Get the current focused widget:
            // Get the current sub window:
            afQMdiSubWindow* pSubWindow = pMainWindow->activeMDISubWindow();
            CpuSessionWindow* pSession = qobject_cast<CpuSessionWindow*>(pSubWindow->widget());

            if (nullptr != pSession)
            {
                filePath = pSubWindow->filePath();
                retVal = true;
            }
        }
    }

    return retVal;

}

QWidget* SessionViewCreator::openMdiWidget(QWidget* pQParent, const gtString& sessionPath)
{
    QWidget* pRetMdi = nullptr;

    GT_IF_WITH_ASSERT(ProfileApplicationTreeHandler::instance() != nullptr)
    {
        afApplicationTreeItemData* pSessionItemData = ProfileApplicationTreeHandler::instance()->FindItemByProfileFilePath(sessionPath);

        // When Visual Studio opens a project, the MDI windows can be opened before the project itself is completely loaded
        // so it is possible that CodeL will not get the corresponding session item at this time. Therefore - do not assert if this pointer is null.
        if (pSessionItemData != nullptr)
        {
            CpuSessionWindow* pSessionWindow = new CpuSessionWindow(pSessionItemData, pQParent);

            pSessionWindow->initialize();
            m_sessionWindowsVector.push_back(pSessionWindow);

            pRetMdi = pSessionWindow;

            gtString viewTitle, viewCommandName;
            int viewIndex = m_sessionWindowsVector.size() - 1;
            titleString(viewIndex, viewTitle, viewCommandName);

            pRetMdi->setWindowTitle(QString::fromWCharArray(viewTitle.asCharArray()));
            pSessionWindow->show();

            // Down cast the event:
            apMDIViewCreateEvent* pCpuProfileEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);
            GT_IF_WITH_ASSERT(pCpuProfileEvent != nullptr)
            {
                // Display the requested item according to the line number in the event:
                if ((AF_TREE_ITEM_PROFILE_CPU_OVERVIEW == pCpuProfileEvent->lineNumber()) ||
                    (AF_TREE_ITEM_PROFILE_SESSION == pCpuProfileEvent->lineNumber()))
                {
                    QString errorMessage;
                    bool retVal = pSessionWindow->DisplaySession(pCpuProfileEvent->filePath(), pCpuProfileEvent->filePath2(), (afTreeItemType)pCpuProfileEvent->lineNumber(), errorMessage);

                    // If the window failed to open, let the user know what was the problem
                    if (!errorMessage.isEmpty() && !retVal)
                    {
                        acMessageBox::instance().critical(afGlobalVariablesManager::instance().ProductNameA(), errorMessage);
                    }
                }
            }
        }
    }

    return pRetMdi;
}

void SessionViewCreator::updateTitleString(const CPUSessionTreeItemData* pSessionData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pSessionData != nullptr) && (pSessionData->m_pParentData != nullptr))
    {
        // Go over the session windows and find the one with this item data:
        gtVector<CpuSessionWindow*>::iterator iter = m_sessionWindowsVector.begin();
        gtVector<CpuSessionWindow*>::iterator iterEnd = m_sessionWindowsVector.end();

        CpuSessionWindow* pMatchingSessionWindow = nullptr;

        for (; iter != iterEnd; iter++)
        {
            CpuSessionWindow* pWindow = *iter;

            if (pWindow != nullptr)
            {
                if (pWindow->displayedCPUSessionItemData() == pSessionData)
                {
                    pMatchingSessionWindow = pWindow;
                    break;
                }
            }
        }

        // If the session is open:
        if (pMatchingSessionWindow != nullptr)
        {

            QString newTitle = CPU_PREFIX_A, oldTitle;
            newTitle += pSessionData->m_displayName;

            if (nullptr != pMatchingSessionWindow->parentWidget())
            {
                QWidget* pWrapper = pMatchingSessionWindow->parentWidget();
                oldTitle = pWrapper->windowTitle();
                pWrapper->setWindowTitle(newTitle);
            }
            else
            {
                pMatchingSessionWindow->setWindowTitle(newTitle);
                oldTitle = pMatchingSessionWindow->windowTitle();
            }

            // If this is an SA, rename the window in framework:
            afMainAppWindow* pMainWindow = afMainAppWindow::instance();

            if (pMainWindow != nullptr)
            {
                pMainWindow->renameMDIWindow(oldTitle, newTitle, acGTStringToQString(pSessionData->m_pParentData->m_filePath.asString()));
            }
        }
    }
}

int SessionViewCreator::viewIndexByPath(const osFilePath& filePath)
{
    int retVal = -1;

    for (int i = 0; i < (int)m_sessionWindowsVector.size(); i++)
    {
        CpuSessionWindow* pWindow = m_sessionWindowsVector[i];

        if (pWindow != nullptr)
        {
            const CPUSessionTreeItemData* pData = pWindow->displayedCPUSessionItemData();

            if ((pData != nullptr) && (pData->m_pParentData != nullptr))
            {
                if (pData->m_pParentData->m_filePath == filePath)
                {
                    retVal = i;
                    break;
                }
            }
        }
    }

    return retVal;
}

bool SessionViewCreator::displayOpenSession(const osFilePath& filePath, int lineNumber)
{
    bool retVal = false;
    CpuSessionWindow* pSessionWindow = nullptr;

    for (int i = 0; i < (int)m_sessionWindowsVector.size(); i++)
    {
        if (m_sessionWindowsVector[i] != nullptr)
        {
            if (m_sessionWindowsVector[i]->displayedSessionFilePath() == filePath)
            {
                pSessionWindow = m_sessionWindowsVector[i];
                break;
            }
        }
    }

    if (pSessionWindow != nullptr)
    {
        QString errorMessage;
        retVal = pSessionWindow->DisplaySession(filePath, osFilePath(), (afTreeItemType)lineNumber, errorMessage);

        // If the window failed to open, let the user know what was the problem
        if (!errorMessage.isEmpty() && !retVal)
        {
            acMessageBox::instance().critical(afGlobalVariablesManager::instance().ProductNameA(), errorMessage);
        }
    }

    return retVal;
}

QPixmap* SessionViewCreator::iconAsPixmap(int viewIndex)
{
    (void)(viewIndex); // unused
    QPixmap* pRetVal = nullptr;

    if (!m_sessionWindowsVector.empty())
    {
        CpuSessionWindow* pSessionWindow = m_sessionWindowsVector.back();

        if (pSessionWindow != nullptr)
        {
            const CPUSessionTreeItemData* pData = pSessionWindow->displayedCPUSessionItemData();

            if (pData != nullptr)
            {
                if (pData->m_pParentData != nullptr)
                {
                    pRetVal = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(AF_TREE_ITEM_PROFILE_SESSION, pData->m_profileTypeStr);
                }
            }
        }
    }

    return pRetVal;
}

CpuSessionWindow* SessionViewCreator::findSessionWindow(const afApplicationTreeItemData* pSessionItemData)
{
    CpuSessionWindow* pRetVal = nullptr;
    // Sanity check:
    GT_IF_WITH_ASSERT(pSessionItemData != nullptr)
    {
        const gtVector<CpuSessionWindow*>& openedSessions = currentlyOpenedSessionWindows();

        for (int i = 0; i < (int)openedSessions.size(); i++)
        {
            if (openedSessions[i] != nullptr)
            {
                if (openedSessions[i]->displayedSessionFilePath() == pSessionItemData->m_filePath)
                {
                    pRetVal = openedSessions[i];
                }
            }
        }
    }

    return pRetVal;
}

CpuSessionWindow* SessionViewCreator::findSessionWindow(const osFilePath& sessionPath)
{
    CpuSessionWindow* pRetVal = nullptr;

    const gtVector<CpuSessionWindow*>& openedSessions = currentlyOpenedSessionWindows();

    for (int i = 0; i < (int)openedSessions.size(); i++)
    {
        if (openedSessions[i] != nullptr)
        {
            if (openedSessions[i]->displayedSessionFilePath() == sessionPath)
            {
                pRetVal = openedSessions[i];
            }
        }
    }

    return pRetVal;
}

void SessionViewCreator::removeDeletedSessionWindow(CpuSessionWindow* pDeletedWindow)
{
    // Go through each of the sessions, and erase the session window that is related to this file path:
    gtVector<CpuSessionWindow*>::iterator it = m_sessionWindowsVector.begin();
    gtVector<CpuSessionWindow*>::iterator endIt = m_sessionWindowsVector.end();

    for (; it != endIt; it++)
    {
        if (*it == pDeletedWindow)
        {
            // If this is the window, remove it:
            m_sessionWindowsVector.erase(it);
            break;
        }
    }
}

void SessionViewCreator::removeSessionWindow(const osFilePath& sessionFilePath, bool deleteView)
{
    CpuSessionWindow* pSessionWindow = nullptr;

    // Look for an existing window with the same session file path:
    for (int i = 0 ; i < (int)m_sessionWindowsVector.size(); i++)
    {
        CpuSessionWindow* pTempWindow = m_sessionWindowsVector[i];

        if (pTempWindow != nullptr)
        {
            if (pTempWindow->displayedSessionFilePath() == sessionFilePath)
            {
                pSessionWindow = pTempWindow;
                break;
            }
        }
    }

    // If there is a window for this session file path, remove it from the list of open windows:
    if (pSessionWindow != nullptr)
    {
        removeDeletedSessionWindow(pSessionWindow);
    }

    // delete the view when we are done if needed
    if (deleteView)
    {
        delete pSessionWindow;
    }
}

CpuSessionWindow* SessionViewCreator::GetCurrentActiveSessionWindow()
{
    CpuSessionWindow* pRetVal = nullptr;

    // Get the current active sub window:
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();

    if (pMainWindow != nullptr)
    {
        // Get the current sub window:
        afQMdiSubWindow* pSubWindow = pMainWindow->activeMDISubWindow();

        if (pSubWindow != nullptr)
        {
            pRetVal = qobject_cast<CpuSessionWindow*>(pSubWindow->widget());
        }
    }

    return pRetVal;
}

void SessionViewCreator::UpdateView(const osFilePath& filePath)
{
    // Update the view:
    CpuSessionWindow* pSessionWindow = findSessionWindow(filePath);

    if (pSessionWindow != nullptr)
    {
        pSessionWindow->UpdateDisplaySettings(true, UPDATE_TABLE_REBUILD);
    }
}

QWidget* SessionViewCreator::CreateSessionWindow(QWidget* pParent)
{
    QWidget* pRetVal = new CpuSessionWindow(nullptr, pParent);
    return pRetVal;
}
