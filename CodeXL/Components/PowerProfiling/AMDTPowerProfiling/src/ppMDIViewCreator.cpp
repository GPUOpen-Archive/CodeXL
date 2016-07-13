//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppMDIViewCreator.cpp
///
//==================================================================================

//------------------------------ ppMDIViewCreator.cpp ------------------------------

#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Framework:
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/StringConstants.h>

// Local:
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfiling/src/ppMDIViewCreator.h>
#include <AMDTPowerProfiling/src/ppSessionView.h>
#include <AMDTPowerProfiling/src/ppSessionActionsCreator.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::ppMDIViewCreator
// Description: Creator
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
ppMDIViewCreator::ppMDIViewCreator()
{
    // Create the view actions creator:
    _pViewActionCreator = new ppSessionActionsCreator();

    _pViewActionCreator->initializeCreator();
}

// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::~ppMDIViewCreator
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
ppMDIViewCreator::~ppMDIViewCreator()
{
}

// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::titleString
// Description: Get the title of the created view
// Arguments:   int viewIndex
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
void ppMDIViewCreator::titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused
    apMDIViewCreateEvent* pCurrentEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);

    if (nullptr != pCurrentEvent)
    {
        osDirectory fileDirectory;
        pCurrentEvent->filePath().getFileDirectory(fileDirectory);
        viewTitle = acQStringToGTString(ppAppController::instance().GetProjectNameFromSessionDir(fileDirectory));

        viewTitle.append(PP_STR_sessionViewCaption);
        viewMenuCommand = viewTitle;
    }
}


// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::associatedToolbar
// Description: Get the name of the toolbar associated with the requested view
// Arguments:   int viewIndex
// Return Val:  gtString
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
gtString ppMDIViewCreator::associatedToolbar(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused
    gtString retVal;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::type
// Description: Get view type
// Arguments:   int viewIndex
// Return Val:  afViewType
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
ppMDIViewCreator::afViewType ppMDIViewCreator::type(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused
    afViewCreatorAbstract::afViewType retDockArea = AF_VIEW_mdi;

    return retDockArea;
}

// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::dockArea
// Description: Get the docking area
// Arguments:   int viewIndex
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
int ppMDIViewCreator::dockArea(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused
    int retVal = AF_VIEW_DOCK_LeftDockWidgetArea;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::dockWidgetFeatures
// Description: Irrelevant for MDI views
// Return Val:  DockWidgetFeatures
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
QDockWidget::DockWidgetFeatures ppMDIViewCreator::dockWidgetFeatures(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused
    return QDockWidget::NoDockWidgetFeatures;
}

// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::initialSize
// Description: Get the initial size
// Arguments:   int viewIndex
// Return Val:  QSize - size of the view
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
QSize ppMDIViewCreator::initialSize(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused
    QSize retSize(0, 0);

    return retSize;
}

// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::visibility
// Description: Get the initial visibility of the view
// Arguments:   int viewIndex
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
bool ppMDIViewCreator::visibility(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused
    bool retVal = true;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::amountOfViewTypes
// Description: Get number of views that can be created by this creator
// Return Val:  int
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
int ppMDIViewCreator::amountOfViewTypes()
{
    return 1;
}

// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::createViewContent
// Description: Create the content for the view
// Arguments:   int viewIndex
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
bool ppMDIViewCreator::createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    bool retVal = false;

    apMDIViewCreateEvent* pCurrentEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);

    GT_IF_WITH_ASSERT(nullptr != pCurrentEvent)
    {
        // Get project name from the path
        ppAppController& appController = ppAppController::instance();

        // Extract the DB file path:
        osDirectory projectDirectory;
        gtString sessionName;
        pCurrentEvent->filePath().getFileDirectory(projectDirectory);
        sessionName = acQStringToGTString(appController.GetProjectNameFromSessionDir(projectDirectory));

        // Set the session state:
        auto lineNumber = pCurrentEvent->lineNumber();
        ppSessionController::SessionState createdSessionState = lineNumber != -1 ? static_cast<ppSessionController::SessionState>(lineNumber) : ppSessionController::PP_SESSION_STATE_COMPLETED;

        // Create the session view:
        ppSessionView* pSessionView = new ppSessionView(pQParent, createdSessionState);


        pSessionView->SetViewData(pCurrentEvent->filePath());
        pContentQWidget = pSessionView;

        m_createdViewsMap[pCurrentEvent->filePath().asString()] = pContentQWidget;

        afTreeItemType itemType = (afTreeItemType)pCurrentEvent->viewIndex();
        int viewIndex1 = (itemType == AF_TREE_ITEM_PP_SUMMARY) ? 1 : 0;
        pSessionView->DisplayTab(viewIndex1);
    }

    if (nullptr != pContentQWidget)
    {
        // add the created content widget to the created list:
        retVal = true;
        m_viewsCreated.push_back(pContentQWidget);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::getCurrentlyDisplayedFilePath
// Description: Get the currently displayed file path
// Arguments:   osFilePath& filePath
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
bool ppMDIViewCreator::getCurrentlyDisplayedFilePath(osFilePath& filePath)
{
    GT_UNREFERENCED_PARAMETER(filePath);

    bool retVal = false;

    apMDIViewCreateEvent* pCurrentEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);
    GT_IF_WITH_ASSERT(nullptr != pCurrentEvent)
    {
        filePath = pCurrentEvent->filePath();
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::displayExistingView
// Description:
// Arguments:   const apMDIViewCreateEvent& mdiViewEvent
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
bool ppMDIViewCreator::displayExistingView(const apMDIViewCreateEvent& mdiViewEvent)
{
    GT_UNREFERENCED_PARAMETER(mdiViewEvent); // unused

    bool retVal = false;

    apMDIViewCreateEvent* pCurrentEvent = dynamic_cast<apMDIViewCreateEvent*>((apMDIViewCreateEvent*)&mdiViewEvent);
    GT_IF_WITH_ASSERT(pCurrentEvent)
    {
        gtString filePathStr = pCurrentEvent->filePath().asString();

        if (m_createdViewsMap.find(filePathStr) != m_createdViewsMap.end())
        {
            retVal = true;
        }

        if (retVal)
        {
            ppSessionView* pSessionView = qobject_cast<ppSessionView*>(m_createdViewsMap[filePathStr]);
            GT_IF_WITH_ASSERT(nullptr != pSessionView)
            {
                // display the detailed file in case it is a different file then the one displayed there:
                afTreeItemType itemType = (afTreeItemType)pCurrentEvent->viewIndex();
                int viewIndex = (itemType == AF_TREE_ITEM_PP_TIMELINE) ? 0 : 1;
                pSessionView->DisplayTab(viewIndex);
            }

        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::onMDISubWindowClose
// Description: Handle sub window close
// Arguments:   afQMdiSubWindow* pMDISubWindow
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
bool ppMDIViewCreator::onMDISubWindowClose(afQMdiSubWindow* pMDISubWindow)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(pMDISubWindow != nullptr)
    {
        // Get the sub window widget:
        QWidget* pWidget = pMDISubWindow->widget();

        if (pWidget != nullptr)
        {
            gtMap<gtString, QWidget*>::iterator mapIterator;

            for (mapIterator = m_createdViewsMap.begin(); mapIterator != m_createdViewsMap.end(); mapIterator++)
            {
                if ((*mapIterator).second == pWidget)
                {
                    m_createdViewsMap.erase(mapIterator);
                    break;
                }
            }

            int existingViewIndex = -1;

            for (int i = 0; i < (int)m_viewsCreated.size(); i++)
            {
                if (pWidget == m_viewsCreated[i])
                {
                    existingViewIndex = i;
                    break;
                }
            }

            // Remove the view:
            if (existingViewIndex >= 0)
            {
                m_viewsCreated.removeItem(existingViewIndex);
            }
        }

        // Check if need to stop the online profiling
        osFilePath filePath = pMDISubWindow->filePath();
        osDirectory fileDirectory;
        filePath.getFileDirectory(fileDirectory);

        if (ppAppController::instance().GetExecutedSessionName() == ppAppController::instance().GetProjectNameFromSessionDir(fileDirectory))
        {
            SharedProfileManager::instance().stopCurrentRun();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::handleTrigger
// Description: Handle the action when it is triggered
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
void ppMDIViewCreator::handleTrigger(int viewIndex, int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);
    GT_UNREFERENCED_PARAMETER(actionIndex);

    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != nullptr)
    {
        // Get the current sub window:
        afQMdiSubWindow* pSubWindow = pMainWindow->activeMDISubWindow();
        GT_UNREFERENCED_PARAMETER(pSubWindow);

    }
}


// ---------------------------------------------------------------------------
// Name:        ppMDIViewCreator::handleUiUpdate
// Description: Handle UI update
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        25/8/14
// ---------------------------------------------------------------------------
void ppMDIViewCreator::handleUiUpdate(int viewIndex, int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false;
    QString updatedActionText;

    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(nullptr != pMainWindow)
    {
        // Get the current sub window:
        afQMdiSubWindow* pSubWindow = pMainWindow->activeMDISubWindow();
        GT_UNREFERENCED_PARAMETER(pSubWindow);
    }
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

        // Update the text if needed:
        if (!updatedActionText.isEmpty())
        {
            pAction->setText(updatedActionText);
        }
    }
}

void ppMDIViewCreator::OnSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDir)
{
    GT_UNREFERENCED_PARAMETER(oldSessionDir);

    GT_IF_WITH_ASSERT((pRenamedSessionData != nullptr) && (pRenamedSessionData->m_pParentData != nullptr))
    {
        // Get the profile file path:
        osFilePath filePathStr = pRenamedSessionData->m_pParentData->m_filePath;

        gtString oldSessionName;
        oldSessionFilePath.getFileName(oldSessionName);
        oldSessionName.append(PP_STR_sessionViewCaption);

        // Find the old session path in map:
        auto iter = m_createdViewsMap.find(oldSessionFilePath.asString());

        if (iter != m_createdViewsMap.end())
        {
            // Get the widget:
            QWidget* pOldWidget = iter->second;

            // Remove the old entrance in table:
            m_createdViewsMap.erase(iter);

            // Add the renamed session path:
            m_createdViewsMap[filePathStr.asString()] = pOldWidget;

            // Change the title:
            gtString viewTitle;
            osDirectory fileDirectory;
            filePathStr.getFileDirectory(fileDirectory);
            viewTitle = acQStringToGTString(ppAppController::instance().GetProjectNameFromSessionDir(fileDirectory));
            viewTitle.append(PP_STR_sessionViewCaption);

            // Check if the window is already created:
            if (afMainAppWindow::instance() != nullptr)
            {
                afQMdiSubWindow* pExistingSubwindow = afMainAppWindow::instance()->findMDISubWindow(oldSessionFilePath);

                if (pExistingSubwindow != nullptr)
                {
                    // Set the window title:
                    afMainAppWindow::instance()->renameMDIWindow(acGTStringToQString(oldSessionName), acGTStringToQString(viewTitle), acGTStringToQString(filePathStr.asString()));
                    pExistingSubwindow->setWindowTitle(acGTStringToQString(viewTitle));
                    pExistingSubwindow->setFilePath(filePathStr);
                }
            }

            // Get the session view for the renamed session:
            ppSessionView* pSessionView = qobject_cast<ppSessionView*>(pOldWidget);
            GT_IF_WITH_ASSERT(pSessionView != nullptr)
            {
                // Set the session new file path:
                pSessionView->SetSessionFilePath(pRenamedSessionData->m_pParentData->m_filePath);
            }
        }
    }
}

void ppMDIViewCreator::OnBeforeSessionRename(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage)
{
    isRenameEnabled = true;
    renameDisableMessage.clear();

    GT_IF_WITH_ASSERT((pAboutToRenameSessionData != nullptr) && (pAboutToRenameSessionData->m_pParentData != nullptr))
    {
        // Find the old session path in map:
        auto iter = m_createdViewsMap.find(pAboutToRenameSessionData->m_pParentData->m_filePath.asString());

        if (iter != m_createdViewsMap.end())
        {
            // If this is not a new session:
            if (pAboutToRenameSessionData->m_displayName != acGTStringToQString(PM_STR_NewSessionName))
            {
                // Get the widget:
                QWidget* pOldWidget = iter->second;

                // Get the session view for the renamed session:
                ppSessionView* pSessionView = qobject_cast<ppSessionView*>(pOldWidget);
                GT_IF_WITH_ASSERT(pSessionView != nullptr)
                {
                    // Set the session new file path:
                    pSessionView->StopDBConnection();
                }
            }
        }
    }
}

bool ppMDIViewCreator::ActivateSession(SessionTreeNodeData* pSessionData)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((pSessionData != nullptr) && (pSessionData->m_pParentData != nullptr))
    {
        // Find the old session path in map:
        auto iter = m_createdViewsMap.find(pSessionData->m_pParentData->m_filePath.asString());

        if (iter != m_createdViewsMap.end())
        {
            // Get the widget:
            QWidget* pOldWidget = iter->second;

            // Get the session view for the session to activate:
            ppSessionView* pSessionView = qobject_cast<ppSessionView*>(pOldWidget);
            GT_IF_WITH_ASSERT(pSessionView != nullptr)
            {
                pSessionView->ActivateSession();

                retVal = true;
            }
        }
    }

    return retVal;
}

void ppMDIViewCreator::OnSessionDelete(const gtString& deletedSessionFilePath)
{
    // Find the old session path in map:
    auto iter = m_createdViewsMap.find(deletedSessionFilePath);

    if (iter != m_createdViewsMap.end())
    {
        // Get the widget:
        QWidget* pWidget = iter->second;

        ppSessionView* pSessionView = qobject_cast<ppSessionView*>(pWidget);
        GT_IF_WITH_ASSERT(pSessionView != nullptr)
        {
            // Close all database connections to enable the file deletion from file system:
            pSessionView->SessionController().GetProfilerBL().CloseAllConnections();
        }

        if (afMainAppWindow::instance() != nullptr)
        {
            // If the MDI window is opened, close it:
            afQMdiSubWindow* pWindow = afMainAppWindow::instance()->findMDISubWindow(deletedSessionFilePath);

            if (pWindow != nullptr)
            {
                // Close the window:
                afMainAppWindow::instance()->closeMDISubWindow(pWindow);
            }
        }
    }

}

bool ppMDIViewCreator::displayOpenSession(const osFilePath& filePath, int lineNumber)
{
    bool retVal = false;

    gtString pathString = filePath.asString();
    if (m_createdViewsMap.count(pathString) > 0)
    {
        QWidget* pWidget = m_createdViewsMap[pathString];
        ppSessionView* pSessionView = dynamic_cast<ppSessionView*>(pWidget);
        if (pSessionView != nullptr)
        {
            pSessionView->DisplayTab(lineNumber == AF_TREE_ITEM_PP_SUMMARY ? 1 : 0);
            retVal = true;
        }
    }

    return retVal;
}
