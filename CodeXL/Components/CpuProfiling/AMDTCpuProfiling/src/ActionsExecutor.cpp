//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ActionsExecutor.cpp
///
//==================================================================================

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// Local:
#include <inc/AmdtCpuProfiling.h>
#include <inc/ActionsExecutor.h>
#include <inc/SessionViewCreator.h>
#include <inc/SessionWindow.h>
#include <inc/StringConstants.h>
#include <inc/CommandIds.h>


ActionsExecutor::ActionsExecutor()
{
}

ActionsExecutor::~ActionsExecutor()
{
}

// ---------------------------------------------------------------------------
// Name:        ActionsExecutor::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:  AMD Developer Tools Team
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void ActionsExecutor::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_SHOW_HIDE_INFO);
}

bool ActionsExecutor::actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut)
{
    (void)(keyboardShortcut); // Unused
    bool retVal = true;

    int commandId = actionIndexToCommandId(actionIndex);

    if (ID_SHOW_HIDE_INFO == commandId)
    {
        caption = CA_STR_MENU_SHOW_HIDE_INFO;
        tooltip = CA_STR_STATUS_SHOW_HIDE_INFO;
    }
    else
    {
        GT_ASSERT(false);
        retVal = false;
    }

    return retVal;
}

gtString ActionsExecutor::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    (void)(actionIndex); // Unused
    gtString retVal;

    int commandId = actionIndexToCommandId(actionIndex);

    if (ID_SHOW_HIDE_INFO == commandId)
    {
        positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;
        retVal = AF_STR_ViewMenuString;
    }
    else
    {
        GT_ASSERT(false);
    }

    return retVal;
}

gtString ActionsExecutor::toolbarPosition(int actionIndex)
{
    (void)(actionIndex); // Unused
    return L"";
}

void ActionsExecutor::handleTrigger(int actionIndex)
{
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();
    GT_IF_WITH_ASSERT(pSessionViewCreator != nullptr)
    {
        int commandId = actionIndexToCommandId(actionIndex);

        if (ID_SHOW_HIDE_INFO == commandId)
        {
            // Toggle show / hide panel value:
            bool showInfoPanel = pSessionViewCreator->showInfoPanel();
            showInfoPanel = !showInfoPanel;
            pSessionViewCreator->setShowInfoPanel(showInfoPanel);

            gtVector<CpuSessionWindow*> sessionWindows = pSessionViewCreator->openedSessionWindows();

            for (int i = 0; i < (int)sessionWindows.size(); i++)
            {
                if (sessionWindows[i] != nullptr)
                {
                    sessionWindows[i]->showInformationPanel(showInfoPanel);
                }
            }
        }
        else
        {
            GT_ASSERT(false);
        }
    }
}

void ActionsExecutor::handleUiUpdate(int actionIndex)
{
    bool showInfoPanel = false;
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();
    GT_IF_WITH_ASSERT(pSessionViewCreator != nullptr)
    {
        // Toggle show / hide panel value:
        showInfoPanel = pSessionViewCreator->showInfoPanel();
    }
    bool isActionEnabled = false, isActionChecked = showInfoPanel, isActionCheckable = true;

    // Check if the active window is a CPU session window:

    // Get the main window:
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != nullptr)
    {
        // Get the current sub window:
        afQMdiSubWindow* pSubWindow = pMainWindow->activeMDISubWindow();

        if (pSubWindow != nullptr)
        {
            // Check if this is a session window:
            gtString ext;
            pSubWindow->filePath().getFileExtension(ext);

            isActionEnabled = (ext == DATA_EXT);
        }
    }

    // Get the QT action:
    QAction* pAction = action(actionIndex);
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
