//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afMenuActionsExecutor.cpp
///
//==================================================================================

// QT:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afMenuActionsExecutor.h>

// ---------------------------------------------------------------------------
// Name:        afMenuActionsExecutor::afMenuActionsExecutor
// Description: Creator
// Author:      Sigal Algranaty
// Date:        2/4/2012
// ---------------------------------------------------------------------------
afMenuActionsExecutor::afMenuActionsExecutor() : m_pApplicationCommands(nullptr)
{
    m_pApplicationCommands = afApplicationCommands::instance();
    GT_ASSERT(m_pApplicationCommands != nullptr);
}

// ---------------------------------------------------------------------------
// Name:        afMenuActionsExecutor::~afMenuActionsExecutor
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        2/4/2012
// ---------------------------------------------------------------------------
afMenuActionsExecutor::~afMenuActionsExecutor()
{
}

// ---------------------------------------------------------------------------
// Name:        afMenuActionsExecutor::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void afMenuActionsExecutor::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(AF_ID_NEW_PROJECT);
    m_supportedCommandIds.push_back(AF_ID_OPEN_PROJECT);
    m_supportedCommandIds.push_back(AF_ID_CLOSE_PROJECT);
    m_supportedCommandIds.push_back(AF_ID_OPEN_FILE);
    m_supportedCommandIds.push_back(AF_ID_OPEN_STARTUP_PAGE);
    m_supportedCommandIds.push_back(AF_ID_SAVE_PROJECT);
    m_supportedCommandIds.push_back(AF_ID_SAVE_PROJECT_AS);
    m_supportedCommandIds.push_back(AF_ID_PROJECT_SETTINGS);
    m_supportedCommandIds.push_back(AF_ID_RESET_GUI_LAYOUTS);
    m_supportedCommandIds.push_back(AF_ID_TOOLS_SYSTEM_INFO);
    m_supportedCommandIds.push_back(AF_ID_TOOLS_OPTIONS);
    m_supportedCommandIds.push_back(AF_ID_HELP_USER_GUIDE);
    m_supportedCommandIds.push_back(AF_ID_HELP_QUICK_START);
    // m_supportedCommandIds.push_back(AF_ID_HELP_UPDATES); // updater is no longer supported
    m_supportedCommandIds.push_back(AF_ID_HELP_DEV_TOOLS_SUPPORT_FORUM);
    m_supportedCommandIds.push_back(AF_ID_HELP_LOAD_TEAPOT_SAMPLE);
    m_supportedCommandIds.push_back(AF_ID_HELP_ABOUT);
    m_supportedCommandIds.push_back(AF_ID_EXIT);
}

// ---------------------------------------------------------------------------
// Name:        initActionIcons::initApplicationIcons
// Description: Create the icons for the application commands
// Author:      Sigal Algranaty
// Date:        2/4/2012
// ---------------------------------------------------------------------------
void afMenuActionsExecutor::initActionIcons()
{

}


// ---------------------------------------------------------------------------
// Name:        afMenuActionsExecutor::caption
// Description: Get the caption of the action item
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        2/4/2012
// ---------------------------------------------------------------------------
bool afMenuActionsExecutor::actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut)
{
    bool retVal = true;

    // Get the command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {

        case AF_ID_NEW_PROJECT:
            caption = AF_STR_newProject;
            tooltip = AF_STR_newProjectStatusbarString;
            keyboardShortcut = AF_STR_keyboardShortcutNewMenu;
            break;

        case AF_ID_OPEN_PROJECT:
            caption = AF_STR_openProject;
            tooltip = AF_STR_openProjectStatusbarString;
            keyboardShortcut = AF_STR_keyboardShortcutOpenProjectMenu;
            break;

        case AF_ID_CLOSE_PROJECT:
            caption = AF_STR_closeProject;
            tooltip = AF_STR_closeProjectStatusbarString;
            break;

        case AF_ID_OPEN_FILE:
            caption = AF_STR_openFile;
            tooltip = AF_STR_openFileStatusbarString;
            keyboardShortcut = AF_STR_keyboardShortcutOpenFileMenu;
            break;

        case AF_ID_OPEN_STARTUP_PAGE:
            caption = AF_STR_welcomePage;
            tooltip = AF_STR_welcomePageStatusbarString;
            keyboardShortcut = AF_STR_keyboardShortcutStartupDialogMenu;
            break;

        case AF_ID_SAVE_PROJECT:
            caption = AF_STR_saveProject;
            tooltip = AF_STR_saveProjectStatusbarString;
            keyboardShortcut = AF_STR_keyboardShortcutSaveProjectMenu;
            break;

        case AF_ID_SAVE_PROJECT_AS:
            caption = AF_STR_saveProjectAs;
            tooltip = AF_STR_saveProjectAsStatusbarString;
            break;

        case AF_ID_PROJECT_SETTINGS:
            caption = AF_STR_ProjectSettings;
            tooltip = AF_STR_ProgressSettingsStatusbarString;
            keyboardShortcut = AF_STR_ProjectSettingsKeyBoardShortcut;
            break;

        case AF_ID_RESET_GUI_LAYOUTS:
            caption = AF_STR_ResetGUILayouts;
            tooltip = AF_STR_ResetGUILayoutsStatusbarString;
            break;

        case AF_ID_TOOLS_SYSTEM_INFO:
            caption = AF_STR_ToolsSystemInfo;
            tooltip = AF_STR_ToolsSystemInfoStatusbarString;
            break;

        case AF_ID_TOOLS_OPTIONS:
            caption = AF_STR_ToolsOptions;
            tooltip = AF_STR_ToolsOptionsStatusbarString;
            break;

        case AF_ID_HELP_USER_GUIDE:
            caption = AF_STR_HelpUserGuide;
            tooltip = AF_STR_HelpUserGuideStatusbarString;
            keyboardShortcut = AF_STR_HelpShortcut;
            break;

        case AF_ID_HELP_QUICK_START:
            caption = AF_STR_HelpQuickStart;
            tooltip = AF_STR_HelpQuickStartStatusbarString;
            break;

        case AF_ID_HELP_UPDATES:
            caption = AF_STR_HelpUpdates;
            tooltip = AF_STR_HelpUpdatesStatusbarString;
            break;

        case AF_ID_HELP_DEV_TOOLS_SUPPORT_FORUM:
            caption = AF_STR_HelpDevToolsSupportForum;
            tooltip = AF_STR_HelpDevToolsSupportForumStatusbarString;
            break;

        case AF_ID_HELP_LOAD_TEAPOT_SAMPLE:
            caption = AF_STR_LoadTeapotSample;
            tooltip = AF_STR_LoadTeapotSampleStatusbarString;
            break;

        case AF_ID_HELP_ABOUT:
            caption = AF_STR_HelpAbout;
            tooltip = AF_STR_HelpAboutStatusbarString;
            break;

        case AF_ID_EXIT:
            caption = AF_STR_exit;
            tooltip = AF_STR_exitStatusbarString;
            keyboardShortcut = AF_STR_keyboardShortcutExitMenu;
            break;

        default:
            GT_ASSERT(false);
            retVal = false;
            break;

    };

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afMenuActionsExecutor::menuPosition
// Description: Menu position
//              Each hierarchy on the ,emu include name/priority.
//              If separator is needed after the item then 's' after the priority is needed
//              in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
// Arguments:   int actionIndex
//              //              positionData - defines the item position within its parent menu
/// Author:      Sigal Algranaty
// Date:        2/4/2012
// ---------------------------------------------------------------------------
gtString afMenuActionsExecutor::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;

    int commandId = actionIndexToCommandId(actionIndex);

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;

    switch (commandId)
    {
        case AF_ID_OPEN_STARTUP_PAGE:
        case AF_ID_NEW_PROJECT:
        case AF_ID_OPEN_PROJECT:
        case AF_ID_CLOSE_PROJECT:
        case AF_ID_SAVE_PROJECT:
        case AF_ID_SAVE_PROJECT_AS:
        {
            retVal = AF_STR_FileMenuString;
            positionData.m_beforeActionMenuPosition = afActionPositionData::AF_POSITION_MENU_START_BLOCK;
        }
        break;

        case AF_ID_PROJECT_SETTINGS:
        {
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
            retVal = AF_STR_FileMenuString;
        }
        break;

        case AF_ID_EXIT:
        {
            retVal = AF_STR_FileMenuString;
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
            positionData.m_actionPosition = afActionPositionData::AF_POSITION_MENU_END_BLOCK;
        }
        break;

        case AF_ID_OPEN_FILE:
        {
            retVal = AF_STR_FileMenuString;
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
        }
        break;

        case AF_ID_RESET_GUI_LAYOUTS:
        {
            retVal = AF_STR_ViewMenuString;
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
        }
        break;

        /*    case AF_ID_ANALYZE_LAUNCH_KA:
                {
                    retVal = AF_STR_AnalyzeMenuString;
                }
                break;*/

        case AF_ID_TOOLS_SYSTEM_INFO:
        case AF_ID_TOOLS_OPTIONS:
        {
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
            retVal = AF_STR_ToolsMenuString;
        }
        break;

        case AF_ID_HELP_USER_GUIDE:
        case AF_ID_HELP_QUICK_START:
        case AF_ID_HELP_UPDATES:
        case AF_ID_HELP_DEV_TOOLS_SUPPORT_FORUM:
        {
            retVal = AF_STR_HelpMenuString;
        }
        break;

        case AF_ID_HELP_ABOUT:
        case AF_ID_HELP_LOAD_TEAPOT_SAMPLE:
        {
            retVal = AF_STR_HelpMenuString;
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
        }
        break;


        default:
            GT_ASSERT(false);
            break;
    };

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afMenuActionsExecutor::handleTrigger
// Description: Handle the action when it is triggered
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void afMenuActionsExecutor::handleTrigger(int actionIndex)
{
    GT_IF_WITH_ASSERT(m_pApplicationCommands != nullptr)
    {
        // Get the command id:
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {

            case AF_ID_NEW_PROJECT:
                m_pApplicationCommands->OnFileNewProject();
                break;

            case AF_ID_OPEN_PROJECT:
                m_pApplicationCommands->OnFileOpenProject(L"");
                break;

            case AF_ID_CLOSE_PROJECT:
                m_pApplicationCommands->OnFileCloseProject(true);
                break;

            case AF_ID_OPEN_FILE:
                m_pApplicationCommands->onFileOpenFile();
                break;

            case AF_ID_OPEN_STARTUP_PAGE:
                m_pApplicationCommands->OnFileOpenWelcomePage();
                break;

            case AF_ID_SAVE_PROJECT_AS:
                m_pApplicationCommands->OnFileSaveProjectAs();
                break;

            case AF_ID_SAVE_PROJECT:
                m_pApplicationCommands->OnFileSaveProject();
                break;

            case AF_ID_PROJECT_SETTINGS:
                m_pApplicationCommands->OnProjectSettings();
                break;

            case AF_ID_RESET_GUI_LAYOUTS:
                m_pApplicationCommands->onViewResetGUILayout();
                break;

            case AF_ID_TOOLS_SYSTEM_INFO:
                m_pApplicationCommands->onToolsSystemInfo();
                break;

            case AF_ID_TOOLS_OPTIONS:
                m_pApplicationCommands->onToolsOptions();
                break;

            case AF_ID_HELP_USER_GUIDE:
                m_pApplicationCommands->onHelpUserGuide();
                break;

            case AF_ID_HELP_QUICK_START:
                m_pApplicationCommands->onHelpQuickStart();
                break;

            case AF_ID_HELP_UPDATES:
                m_pApplicationCommands->onHelpUpdates();
                break;

            case AF_ID_HELP_DEV_TOOLS_SUPPORT_FORUM:
                m_pApplicationCommands->onHelpOpenURL(AF_STR_HelpDevToolsSupportForumURL);
                break;

            case AF_ID_HELP_LOAD_TEAPOT_SAMPLE:
                m_pApplicationCommands->LoadSample(AF_TEAPOT_SAMPLE);
                break;

            case AF_ID_HELP_ABOUT:
                m_pApplicationCommands->onHelpAbout();
                break;

            case AF_ID_EXIT:
                m_pApplicationCommands->onFileExit();
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
// Name:        afMenuActionsExecutor::handleUiUpdate
// Description: Handle UI update
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        2/4/2012
// ---------------------------------------------------------------------------
void afMenuActionsExecutor::handleUiUpdate(int actionIndex)
{
    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false, isActionVisible = true;

    int commandId = actionIndexToCommandId(actionIndex);
    GT_IF_WITH_ASSERT(m_pApplicationCommands != nullptr)
    {
        switch (commandId)
        {
            case AF_ID_HELP_USER_GUIDE:
            case AF_ID_HELP_QUICK_START:
            case AF_ID_HELP_UPDATES:
            case AF_ID_HELP_DEV_TOOLS_SUPPORT_FORUM:
            case AF_ID_HELP_ABOUT:
                isActionEnabled = true;
                break;

            case AF_ID_EXIT:
            case AF_ID_OPEN_FILE:
            {
                // These actions are always enabled:
                isActionEnabled = true;
                break;
            }

            case AF_ID_SAVE_PROJECT:
            {
                m_pApplicationCommands->onUpdateProjectSave(isActionEnabled);
                break;
            }

            case AF_ID_CLOSE_PROJECT:
            {
                m_pApplicationCommands->onUpdateProjectClose(isActionEnabled);
                break;
            }

            case AF_ID_SAVE_PROJECT_AS:
            {
                m_pApplicationCommands->onUpdateProjectSaveAs(isActionEnabled);
                break;
            }

            case AF_ID_RESET_GUI_LAYOUTS:
                isActionEnabled = true;
                break;

            case AF_ID_PROJECT_SETTINGS:
                m_pApplicationCommands->enableWhenNoProcess(isActionEnabled);
                break;

            case AF_ID_NEW_PROJECT:
            case AF_ID_OPEN_PROJECT:
            case AF_ID_OPEN_STARTUP_PAGE:
                m_pApplicationCommands->enableWhenNoProcess(isActionEnabled);
                break;

            case AF_ID_TOOLS_SYSTEM_INFO:
                isActionEnabled = true;
                break;

            case AF_ID_TOOLS_OPTIONS:
                m_pApplicationCommands->enableWhenNoProcess(isActionEnabled);
                break;

            case AF_ID_HELP_LOAD_TEAPOT_SAMPLE:
            {
                m_pApplicationCommands->enableWhenNoProcess(isActionEnabled);
                break;
            }

            default:
                GT_ASSERT_EX(false, L"Unknown event id");
                break;
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

        // Show / hide the action
        pAction->setVisible(isActionVisible);
    }
}


// ---------------------------------------------------------------------------
// Name:        afMenuActionsExecutor::toolbarPosition
// Description: Toolbar position
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
gtString afMenuActionsExecutor::toolbarPosition(int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(actionIndex);

    gtString retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afMenuActionsExecutor::groupAction
// Description: Group actions if needed
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
void afMenuActionsExecutor::groupAction(int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(actionIndex);
}
