//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afRecentProjectsActionsExecutor.cpp
///
//==================================================================================

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afRecentProjectsActionsExecutor.h>

// ---------------------------------------------------------------------------
// Name:        afRecentProjectsActionsExecutor::afRecentProjectsActionsExecutor
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
afRecentProjectsActionsExecutor::afRecentProjectsActionsExecutor()
{
    // Get the application commands handler:
    m_pApplicationCommandsHandler = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(m_pApplicationCommandsHandler != nullptr)
    {
        gtString appName;
        m_pApplicationCommandsHandler->FillRecentlyUsedProjectsNames(m_recentlyUsedProjectsNames, appName);
    }
}

// ---------------------------------------------------------------------------
// Name:        afRecentProjectsActionsExecutor::~afRecentProjectsActionsExecutor
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
afRecentProjectsActionsExecutor::~afRecentProjectsActionsExecutor(void)
{

}

// ---------------------------------------------------------------------------
// Name:        afRecentProjectsActionsExecutor::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void afRecentProjectsActionsExecutor::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_RECENTLY_USED_PROJECT_0);
    m_supportedCommandIds.push_back(ID_RECENTLY_USED_PROJECT_1);
    m_supportedCommandIds.push_back(ID_RECENTLY_USED_PROJECT_2);
    m_supportedCommandIds.push_back(ID_RECENTLY_USED_PROJECT_3);
    m_supportedCommandIds.push_back(ID_RECENTLY_USED_PROJECT_4);
    m_supportedCommandIds.push_back(ID_RECENTLY_USED_PROJECT_5);
    m_supportedCommandIds.push_back(ID_RECENTLY_USED_PROJECT_6);
    m_supportedCommandIds.push_back(ID_RECENTLY_USED_PROJECT_7);
    m_supportedCommandIds.push_back(ID_RECENTLY_USED_PROJECT_8);
    m_supportedCommandIds.push_back(ID_RECENTLY_USED_PROJECT_9);
    m_supportedCommandIds.push_back(ID_RECENT_PROJECTS_SUB_MENU);
}

// ---------------------------------------------------------------------------
// Name:        afRecentProjectsActionsExecutor::caption
// Description: Get the caption of the action item
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
bool afRecentProjectsActionsExecutor::actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut)
{
    GT_UNREFERENCED_PARAMETER(tooltip);
    GT_UNREFERENCED_PARAMETER(keyboardShortcut);

    bool retVal = true;

    // Get the command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {

        case ID_RECENTLY_USED_PROJECT_0:
        case ID_RECENTLY_USED_PROJECT_1:
        case ID_RECENTLY_USED_PROJECT_2:
        case ID_RECENTLY_USED_PROJECT_3:
        case ID_RECENTLY_USED_PROJECT_4:
        case ID_RECENTLY_USED_PROJECT_5:
        case ID_RECENTLY_USED_PROJECT_6:
        case ID_RECENTLY_USED_PROJECT_7:
        case ID_RECENTLY_USED_PROJECT_8:
        case ID_RECENTLY_USED_PROJECT_9:
        {
            if (actionIndex < (int)m_recentlyUsedProjectsNames.size())
            {
                // Define a file path in order to extract the file name:
                osFilePath filePath(m_recentlyUsedProjectsNames[actionIndex]);
                filePath.getFileName(caption);
                caption.prependFormattedString(L"%d ", (int)(actionIndex + 1));
            }
        }
        break;

        case ID_RECENT_PROJECTS_SUB_MENU:
        {
            caption = AF_STR_RecentProject;
        }
        break;

        default:
            GT_ASSERT(false);
            retVal = false;
            break;

    };

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afRecentProjectsActionsExecutor::menuPosition
// Description: Menu position
//              Each hierarchy on the ,emu include name/priority.
//              If separator is needed after the item then 's' after the priority is needed
//              in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
// Arguments:   int actionIndex
//              positionData - defines the item position within its parent menu
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
gtString afRecentProjectsActionsExecutor::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;

    // The recent projects menu items should come before the exit command:
    positionData.m_beforeActionMenuPosition = AF_STR_FileMenuString;
    positionData.m_beforeActionMenuPosition.append(AF_Str_MenuSeparator);
    positionData.m_beforeActionText.append(AF_STR_exit);

    // Get the command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_RECENTLY_USED_PROJECT_0:
        case ID_RECENTLY_USED_PROJECT_1:
        case ID_RECENTLY_USED_PROJECT_2:
        case ID_RECENTLY_USED_PROJECT_3:
        case ID_RECENTLY_USED_PROJECT_4:
        case ID_RECENTLY_USED_PROJECT_5:
        case ID_RECENTLY_USED_PROJECT_6:
        case ID_RECENTLY_USED_PROJECT_7:
        case ID_RECENTLY_USED_PROJECT_8:
        case ID_RECENTLY_USED_PROJECT_9:
        {
            positionData.m_beforeActionMenuPosition = AF_STR_FileMenuString;
            positionData.m_beforeActionText = AF_STR_exit;
            retVal = AF_STR_FileMenuString;
            retVal.append(AF_Str_MenuSeparator);
            retVal.append(AF_STR_RecentProject);
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND_GROUP;
            positionData.m_actionPosition = afActionPositionData::AF_POSITION_MENU_END_BLOCK;
        }
        break;

        case ID_RECENT_PROJECTS_SUB_MENU:
        {
            positionData.m_beforeActionMenuPosition = AF_STR_FileMenuString;
            positionData.m_beforeActionText = AF_STR_exit;
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
            retVal = AF_STR_FileMenuString;
            retVal.append(AF_Str_MenuSeparator);
            retVal.append(AF_STR_RecentProject);
        }
        break;

        default:
            GT_ASSERT(false);
            break;
    };

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afRecentProjectsActionsExecutor::toolbarPosition
// Description: Toolbar position
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
gtString afRecentProjectsActionsExecutor::toolbarPosition(int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(actionIndex);

    gtString retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afRecentProjectsActionsExecutor::handleTrigger
// Description: Handle the action when it is triggered
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
void afRecentProjectsActionsExecutor::handleTrigger(int actionIndex)
{
    // Get the command id:
    int commandId = actionIndexToCommandId(actionIndex);
    GT_IF_WITH_ASSERT(m_pApplicationCommandsHandler != nullptr)
    {
        switch (commandId)
        {
            case ID_RECENTLY_USED_PROJECT_0:
            case ID_RECENTLY_USED_PROJECT_1:
            case ID_RECENTLY_USED_PROJECT_2:
            case ID_RECENTLY_USED_PROJECT_3:
            case ID_RECENTLY_USED_PROJECT_4:
            case ID_RECENTLY_USED_PROJECT_5:
            case ID_RECENTLY_USED_PROJECT_6:
            case ID_RECENTLY_USED_PROJECT_7:
            case ID_RECENTLY_USED_PROJECT_8:
            case ID_RECENTLY_USED_PROJECT_9:
                m_pApplicationCommandsHandler->OnFileRecentProject(m_recentlyUsedProjectsNames, actionIndex);
                break;

            default:
            {
                GT_ASSERT_EX(false, L"Unsupported application command");
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afRecentProjectsActionsExecutor::handleUiUpdate
// Description: Handle UI update
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
void afRecentProjectsActionsExecutor::handleUiUpdate(int actionIndex)
{
    // Sanity check:
    gtString currentProjectPath;

    // Get the current project file name:
    currentProjectPath = afProjectManager::instance().currentProjectFilePath().asString();

    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false, isActionVisible = false;

    if (actionIndex < (int)m_recentlyUsedProjectsNames.size())
    {
        isActionVisible = true;

        // Get the command id:
        int commandId = actionIndexToCommandId(actionIndex);
        GT_IF_WITH_ASSERT(m_pApplicationCommandsHandler != nullptr)
        {
            switch (commandId)
            {
                case ID_RECENTLY_USED_PROJECT_0:
                case ID_RECENTLY_USED_PROJECT_1:
                case ID_RECENTLY_USED_PROJECT_2:
                case ID_RECENTLY_USED_PROJECT_3:
                case ID_RECENTLY_USED_PROJECT_4:
                case ID_RECENTLY_USED_PROJECT_5:
                case ID_RECENTLY_USED_PROJECT_6:
                case ID_RECENTLY_USED_PROJECT_7:
                case ID_RECENTLY_USED_PROJECT_8:
                case ID_RECENTLY_USED_PROJECT_9:
                case ID_RECENT_PROJECTS_SUB_MENU:
                {
                    // Disable the active project:
                    if (currentProjectPath == m_recentlyUsedProjectsNames[actionIndex])
                    {
                        isActionEnabled = false;
                    }
                    else
                    {
                        m_pApplicationCommandsHandler->enableWhenNoProcess(isActionEnabled);
                    }
                }
                break;

                default:
                    GT_ASSERT_EX(false, L"Unknown event id");
                    break;
            }
        }
    }

    // Get the QT action:
    QAction* pAction = action(actionIndex);
    GT_IF_WITH_ASSERT(pAction != nullptr)
    {
        // Set the action visibility:
        pAction->setVisible(isActionVisible);

        // Set the action enable / disable:
        pAction->setEnabled(isActionEnabled);

        // Set the action checkable state:
        pAction->setCheckable(isActionCheckable);

        // Set the action check state:
        pAction->setChecked(isActionChecked);

        gtString caption, tooltip, keyboardShortcut;
        bool rc = actionText(actionIndex, caption, tooltip, keyboardShortcut);

        if (rc)
        {
            // Set the action text (recently used projects names are changed):
            pAction->setText(acGTStringToQString(caption));
            pAction->setToolTip(acGTStringToQString(tooltip));

            QList<QKeySequence> shortcuts;
            QString qShortcut = QString::fromWCharArray(keyboardShortcut.asCharArray());

            // Create a key shortcut from the string:
            QKeySequence keySeauence(acGTStringToQString(keyboardShortcut));
            shortcuts.append(keySeauence);

            // Set the shortcuts:
            pAction->setShortcuts(shortcuts);

            // Set context to application context:
            pAction->setShortcutContext(Qt::ApplicationShortcut);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afRecentProjectsActionsExecutor::groupAction
// Description: If the requested action should be a part of a group, create
//              the action group and add the relevant actions to the group
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
void afRecentProjectsActionsExecutor::groupAction(int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(actionIndex);
}


// ---------------------------------------------------------------------------
// Name:        afRecentProjectsActionsExecutor::UpdateRecentlyUsedProjects
// Description: Update the recently used commands menu creator
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
bool afRecentProjectsActionsExecutor::UpdateRecentlyUsedProjects()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pApplicationCommandsHandler != nullptr)
    {
        gtString currentAppName;
        m_pApplicationCommandsHandler->FillRecentlyUsedProjectsNames(m_recentlyUsedProjectsNames, currentAppName);
        retVal = true;
    }
    return retVal;
}
