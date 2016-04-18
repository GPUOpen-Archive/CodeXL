//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpMDIViewCreator.cpp
///
//==================================================================================

//------------------------------ tpMDIViewCreator.cpp ------------------------------

#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

// Local:
#include <inc/tpMDIViewCreator.h>
#include <inc/tpSessionView.h>
#include <inc/StringConstants.h>
#include <inc/tpAppController.h>

tpMDIViewCreator* tpMDIViewCreator::m_spMySingleInstance = nullptr;

tpMDIViewCreator::tpMDIViewCreator()
{

}

tpMDIViewCreator::~tpMDIViewCreator()
{
}

tpMDIViewCreator& tpMDIViewCreator::Instance()
{
    // If this class single instance was not already created:
    if (m_spMySingleInstance == nullptr)
    {
        // Create it:
        m_spMySingleInstance = new tpMDIViewCreator;
        GT_ASSERT(m_spMySingleInstance);
    }

    return *m_spMySingleInstance;
}
void tpMDIViewCreator::titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused
    apMDIViewCreateEvent* pCurrentEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);

    if (nullptr != pCurrentEvent)
    {
        osDirectory fileDirectory;
        pCurrentEvent->filePath().getFileDirectory(fileDirectory);
        viewTitle = acQStringToGTString(ProfileApplicationTreeHandler::GetProjectNameFromSessionDir(fileDirectory));

        viewTitle.append(CP_STR_sessionViewCaption);
        viewMenuCommand = viewTitle;
    }
}

gtString tpMDIViewCreator::associatedToolbar(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused
    gtString retVal;
    return retVal;
}

tpMDIViewCreator::afViewType tpMDIViewCreator::type(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused

    // This creator is only creating MDI views:
    afViewCreatorAbstract::afViewType retDockArea = AF_VIEW_mdi;

    return retDockArea;
}

int tpMDIViewCreator::dockArea(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused
    int retVal = AF_VIEW_DOCK_LeftDockWidgetArea;
    return retVal;
}

QDockWidget::DockWidgetFeatures tpMDIViewCreator::dockWidgetFeatures(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused
    return QDockWidget::NoDockWidgetFeatures;
}

QSize tpMDIViewCreator::initialSize(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused
    QSize retSize(0, 0);

    return retSize;
}

bool tpMDIViewCreator::visibility(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex); // unused
    bool retVal = true;

    return retVal;
}

int tpMDIViewCreator::amountOfViewTypes()
{
    return 1;
}

bool tpMDIViewCreator::createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    bool retVal = false;

    apMDIViewCreateEvent* pCurrentEvent = dynamic_cast<apMDIViewCreateEvent*>(_pCreationEvent);

    GT_IF_WITH_ASSERT(nullptr != pCurrentEvent)
    {
        // Create the session view:
        tpSessionView* pSessionView = new tpSessionView(pQParent, pCurrentEvent->filePath());
        pContentQWidget = pSessionView;

        // Display the tab as requested in the event:
        pSessionView->DisplayTab(pCurrentEvent->viewIndex());

        m_createdViewsMap[pCurrentEvent->filePath().asString()] = pContentQWidget;
    }

    if (nullptr != pContentQWidget)
    {
        // add the created content widget to the created list:
        retVal = true;
        m_viewsCreated.push_back(pContentQWidget);
    }

    return retVal;
}

bool tpMDIViewCreator::getCurrentlyDisplayedFilePath(osFilePath& filePath)
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

bool tpMDIViewCreator::displayExistingView(const apMDIViewCreateEvent& mdiViewEvent)
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
            tpSessionView* pSessionView = qobject_cast<tpSessionView*>(m_createdViewsMap[filePathStr]);
            GT_IF_WITH_ASSERT(nullptr != pSessionView)
            {
                // Display the tab as requested in the event:
                pSessionView->DisplayTab(pCurrentEvent->viewIndex());
            }

        }
    }

    return retVal;
}

bool tpMDIViewCreator::onMDISubWindowClose(afQMdiSubWindow* pMDISubWindow)
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

    }

    return retVal;
}

void tpMDIViewCreator::handleTrigger(int viewIndex, int actionIndex)
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

void tpMDIViewCreator::handleUiUpdate(int viewIndex, int actionIndex)
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

void tpMDIViewCreator::OnSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDir)
{
    GT_UNREFERENCED_PARAMETER(pRenamedSessionData);
    GT_UNREFERENCED_PARAMETER(oldSessionFilePath);
    GT_UNREFERENCED_PARAMETER(oldSessionDir);
#pragma message ("TODO: TP :")
}

void tpMDIViewCreator::OnBeforeSessionRename(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage)
{
    GT_UNREFERENCED_PARAMETER(pAboutToRenameSessionData);
    GT_UNREFERENCED_PARAMETER(isRenameEnabled);
    GT_UNREFERENCED_PARAMETER(renameDisableMessage);
#pragma message ("TODO: TP :")
}

bool tpMDIViewCreator::ActivateSession(SessionTreeNodeData* pSessionData)
{
    GT_UNREFERENCED_PARAMETER(pSessionData);
#pragma message ("TODO: TP :")
    return false;
}

void tpMDIViewCreator::OnSessionDelete(const gtString& deletedSessionFilePath)
{
    GT_UNREFERENCED_PARAMETER(deletedSessionFilePath);
#pragma message ("TODO: TP :")
}
