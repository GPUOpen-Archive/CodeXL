//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwMenuActionsExecutor.cpp
///
//==================================================================================

//------------------------------ gwMenuActionsExecutor.h ------------------------------

// QT:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdImagesAndBuffersManager.h>

// Local:
#include <src/gwMenuActionsExecutor.h>
#include <AMDTGpuDebugging/Include/gwStringConstants.h>
#include <AMDTGpuDebugging/Include/gwgDEBuggerAppWrapper.h>

// ---------------------------------------------------------------------------
// Name:        gwMenuActionsExecutor::gwMenuActionsExecutor
// Description: Creator
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
gwMenuActionsExecutor::gwMenuActionsExecutor(void) : _pApplicationCommandsHandler(NULL)
{
    // Get the application commands handler:
    _pApplicationCommandsHandler = gdApplicationCommands::gdInstance();
    GT_ASSERT(_pApplicationCommandsHandler != NULL);
}

// ---------------------------------------------------------------------------
// Name:        gwMenuActionsExecutor::~gwMenuActionsExecutor
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
gwMenuActionsExecutor::~gwMenuActionsExecutor(void)
{
}

// ---------------------------------------------------------------------------
// Name:        gwMenuActionsExecutor::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void gwMenuActionsExecutor::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_SAVE_STATE_VARIABLES);

    // Debug menu:
    m_supportedCommandIds.push_back(ID_START_DEBUGGING);
    m_supportedCommandIds.push_back(ID_BREAK_DEBUGGING);
    m_supportedCommandIds.push_back(ID_STOP_DEBUGGING);
    m_supportedCommandIds.push_back(ID_DEBUG_MODE);
    m_supportedCommandIds.push_back(ID_API_STEP_DEBUGGING);
    m_supportedCommandIds.push_back(ID_DRAW_STEP_DEBUGGING);
    m_supportedCommandIds.push_back(ID_FRAME_STEP_DEBUGGING);
    m_supportedCommandIds.push_back(ID_STEP_IN_DEBUGGING);
    m_supportedCommandIds.push_back(ID_STEP_OVER_DEBUGGING);
    m_supportedCommandIds.push_back(ID_STEP_OUT_DEBUGGING);

    // Breakpoints menu:
    m_supportedCommandIds.push_back(ID_BREAKPOINTS_MENU_BREAK_ON_OPENGL_ERROR);
    m_supportedCommandIds.push_back(ID_BREAKPOINTS_MENU_BREAK_ON_OPENCL_ERROR);
    m_supportedCommandIds.push_back(ID_BREAKPOINTS_MENU_BREAK_ON_DETECTED_ERROR);
#if defined (__APPLE__)
    m_supportedCommandIds.push_back(ID_BREAKPOINTS_MENU_BREAK_ON_SOFTWARE_FALLBACKS);
#endif
    m_supportedCommandIds.push_back(ID_BREAKPOINTS_MENU_BREAK_ON_REDUNDANT_STATE_CHANGES);
    m_supportedCommandIds.push_back(ID_BREAKPOINTS_MENU_BREAK_ON_DEPRECATED_FUNCTIONS);
    m_supportedCommandIds.push_back(ID_BREAKPOINTS_MENU_BREAK_ON_MEMORY_LEAKS);

#if defined(_WIN32)
    m_supportedCommandIds.push_back(ID_BREAKPOINTS_MENU_GL_DEBUG_OUTPUT_BREAK);
    m_supportedCommandIds.push_back(ID_BREAKPOINTS_MENU_GL_DEBUG_OUTPUT_SETTINGS);
#endif

    m_supportedCommandIds.push_back(ID_BREAKPOINTS_MENU_ADD_REMOVE_BREAKPOINTS);
    m_supportedCommandIds.push_back(ID_BREAKPOINTS_MENU_ENABLE_DISABLE_ALL_BREAKPOINTS);

    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_IMAGE);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_IMAGE_AS);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_RAWDATA_AS);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_RAWDATA);

    m_supportedCommandIds.push_back(ID_DEBUG_SETTINGS);
}

// ---------------------------------------------------------------------------
// Name:        initActionIcons::initApplicationIcons
// Description: Create the icons for the application commands
// Author:      Sigal Algranaty
// Date:        26/7/2011
// ---------------------------------------------------------------------------
void gwMenuActionsExecutor::initActionIcons()
{
    // Initialize the commands icons:

    // Debug run command:
    initSingleActionIcon(ID_START_DEBUGGING, AC_ICON_EXECUTION_PLAY);

    // Break command:
    initSingleActionIcon(ID_BREAK_DEBUGGING, AC_ICON_EXECUTION_PAUSE);

    // Stop debugging command:
    initSingleActionIcon(ID_STOP_DEBUGGING, AC_ICON_EXECUTION_STOP);

    // API step command:
    initSingleActionIcon(ID_API_STEP_DEBUGGING, AC_ICON_EXECUTION_API_STEP);

    // Draw step command:
    initSingleActionIcon(ID_DRAW_STEP_DEBUGGING, AC_ICON_EXECUTION_DRAW_STEP);

    // Frame step command:
    initSingleActionIcon(ID_FRAME_STEP_DEBUGGING, AC_ICON_EXECUTION_FRAME_STEP);

    // Step in command:
    initSingleActionIcon(ID_STEP_IN_DEBUGGING, AC_ICON_EXECUTION_STEP_IN);

    // Step over command:
    initSingleActionIcon(ID_STEP_OVER_DEBUGGING, AC_ICON_EXECUTION_STEP_OVER);

    // Step out command:
    initSingleActionIcon(ID_STEP_OUT_DEBUGGING, AC_ICON_EXECUTION_STEP_OUT);

}


// ---------------------------------------------------------------------------
// Name:        gwMenuActionsExecutor::caption
// Description: Get the caption of the action item
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
bool gwMenuActionsExecutor::actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut)
{
    bool retVal = true;

    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_SAVE_STATE_VARIABLES:
            caption = GD_STR_saveStateVariables;
            tooltip = GD_STR_saveStateVariablesStatusbarString;
            break;

        case ID_START_DEBUGGING:
            caption = GD_STR_StartDebugging;
            tooltip = GD_STR_StartDebuggingStatusbarString;
            //Shortcut taken care of by afExectionModeManager
            break;

        case ID_DEBUG_MODE:
            caption = GD_STR_SwitchToDebugMode;
            tooltip = GD_STR_DebugModeStatusbarString;
            break;

        case ID_API_STEP_DEBUGGING:
            caption = GD_STR_APIStepDebugging;
            tooltip = GD_STR_APIStepDebuggingStatusbarString;
            keyboardShortcut = GD_STR_keyboardShortcutAPIStepMenu;
            break;

        case ID_DRAW_STEP_DEBUGGING:
            caption = GD_STR_DrawStepDebugging;
            tooltip = GD_STR_DrawStepDebuggingStatusbarString;
            keyboardShortcut = GD_STR_keyboardShortcutDrawStepMenu;
            break;

        case ID_FRAME_STEP_DEBUGGING:
            caption = GD_STR_FrameStepDebugging;
            tooltip = GD_STR_FrameStepDebuggingStatusbarString;
            keyboardShortcut = GD_STR_keyboardShortcutFrameStepMenu;
            break;

        case ID_STEP_OVER_DEBUGGING:
            caption = GD_STR_StepOverDebugging;
            tooltip = GD_STR_StepOverDebuggingStatusbarString;
            keyboardShortcut = GD_STR_keyboardShortcutStepOverMenu;
            break;

        case ID_STEP_IN_DEBUGGING:
            caption = GD_STR_StepInDebugging;
            tooltip = GD_STR_StepInDebuggingStatusbarString;
            keyboardShortcut = GD_STR_keyboardShortcutStepInMenu;
            break;

        case ID_STEP_OUT_DEBUGGING:
            caption = GD_STR_StepOutDebugging;
            tooltip = GD_STR_StepOutDebuggingStatusbarString;
            keyboardShortcut = GD_STR_keyboardShortcutStepOutMenu;
            break;

        case ID_BREAK_DEBUGGING:
            caption = GD_STR_BreakDebugging;
            tooltip = GD_STR_BreakDebuggingStatusbarString;
            //Shortcut taken care of by afExectionModeManager
            break;

        case ID_STOP_DEBUGGING:
            caption = GD_STR_StopDebugging;
            tooltip = GD_STR_StopDebuggingStatusbarString;
            //Shortcut taken care of by afExectionModeManager
            break;

        case ID_BREAKPOINTS_MENU_BREAK_ON_OPENGL_ERROR:
            caption = GD_STR_BreakOnOpenGLError;
            tooltip = GD_STR_BreakOnOpenGLErrorStatusbarString;
            keyboardShortcut = GD_STR_keyboardShortcutErrorMenu;
            break;

        case ID_BREAKPOINTS_MENU_BREAK_ON_OPENCL_ERROR:
            caption = GD_STR_BreakOnOpenCLError;
            tooltip = GD_STR_BreakOnOpenCLErrorStatusbarString;
            break;

        case ID_BREAKPOINTS_MENU_BREAK_ON_DETECTED_ERROR:
            caption = GD_STR_BreakOnDetectedError;
            tooltip = GD_STR_BreakOnDetectedErrorStatusbarString;
            break;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case ID_BREAKPOINTS_MENU_BREAK_ON_SOFTWARE_FALLBACKS:
            caption = GD_STR_BreakOnSoftwareFallbacks;
            tooltip = GD_STR_BreakOnSoftwareFallbacksStatusbarString;
            break;
#endif

        case ID_BREAKPOINTS_MENU_BREAK_ON_REDUNDANT_STATE_CHANGES:
            caption = GD_STR_BreakOnRedundantStateChanges;
            tooltip = GD_STR_BreakOnRedundantStateChangesStatusbarString;
            break;

        case ID_BREAKPOINTS_MENU_BREAK_ON_DEPRECATED_FUNCTIONS:
            caption = GD_STR_BreakOnDeprecatedFunctions;
            tooltip = GD_STR_BreakOnDeprecatedFunctionsStatusbarString;
            break;

        case ID_BREAKPOINTS_MENU_BREAK_ON_MEMORY_LEAKS:
            caption = GD_STR_BreakOnMemoryLeaks;
            tooltip = GD_STR_BreakOnMemoryLeaksStatusbarString;
            break;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

        case ID_BREAKPOINTS_MENU_GL_DEBUG_OUTPUT_BREAK:
            caption = GD_STR_BreakOnGLDebugOutput;
            tooltip = GD_STR_BreakOnGLDebugOutputStatusbarString;
            break;

        case ID_BREAKPOINTS_MENU_GL_DEBUG_OUTPUT_SETTINGS:
            caption = GD_STR_GLDebugOutputSettings;
            tooltip = GD_STR_GLDebugOutputSettingsStatusbarString;
            break;
#endif

        case ID_BREAKPOINTS_MENU_ENABLE_DISABLE_ALL_BREAKPOINTS:
            caption = GD_STR_EnableAllBreakpoints;
            tooltip = GD_STR_EnableAllBreakpointsStatusbarString;
            break;

        case ID_BREAKPOINTS_MENU_ADD_REMOVE_BREAKPOINTS:
            caption = GD_STR_Breakpoints;
            tooltip = GD_STR_BreakpointsStatusbarString;
            keyboardShortcut = GD_STR_keyboardShortcutBreakpointsMenu;
            break;

        case ID_DEBUG_SETTINGS:
            caption = GD_STR_DebugSettings;
            keyboardShortcut = AF_STR_keyboardShortcutDebugMenu;
            tooltip = GD_STR_DebugSettingsStatusbarString;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_IMAGE_AS:
            caption = GD_STR_ImagesAndBuffersViewerMenuFileSaveImageAs;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_RAWDATA_AS:
            caption = GD_STR_ImagesAndBuffersViewerMenuFileSaveRawDataAs;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_IMAGE:
            caption = GD_STR_ImagesAndBuffersViewerMenuFileSaveAllAsImages;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_RAWDATA:
            caption = GD_STR_ImagesAndBuffersViewerMenuFileSaveAllAsRawData;
            break;

        default:
            GT_ASSERT(false);
            retVal = false;
            break;

    };

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwMenuActionsExecutor::menuPosition
// Description: Menu position
//              Each hierarchy on the ,emu include name/priority.
//              If separator is needed after the item then 's' after the priority is needed
//              in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
// Arguments:   int actionIndex
//              positionData - defines the item position within its parent menu
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
gtString gwMenuActionsExecutor::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;

    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_SAVE_STATE_VARIABLES:
        {
            retVal = AF_STR_FileMenuString;
            retVal.append(AF_Str_MenuSeparator);
            retVal.append(GD_STR_exportFileMenu);
            positionData.m_beforeActionMenuPosition = AF_STR_FileMenuString;
            positionData.m_beforeActionMenuPosition.append(AF_Str_MenuSeparator);
            positionData.m_beforeActionMenuPosition.append(AF_STR_RecentProject);

            positionData.m_beforeActionText = AF_STR_RecentProject;
        }
        break;

        case ID_START_DEBUGGING:
        case ID_BREAK_DEBUGGING:
        case ID_STOP_DEBUGGING:
        case ID_DEBUG_MODE:
        case ID_API_STEP_DEBUGGING:
        case ID_DRAW_STEP_DEBUGGING:
        case ID_FRAME_STEP_DEBUGGING:
        case ID_STEP_IN_DEBUGGING:
        case ID_STEP_OVER_DEBUGGING:
        case ID_STEP_OUT_DEBUGGING:
        {
            retVal = GD_STR_DebugMenuString;

            positionData.m_beforeActionMenuPosition = GD_STR_DebugMenuString;
            positionData.m_beforeActionText = AF_STR_DebugSettings;

            if (ID_DEBUG_MODE == commandId || ID_API_STEP_DEBUGGING == commandId || ID_STEP_IN_DEBUGGING == commandId)
            {
                positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
            }
        }
        break;

        case ID_BREAKPOINTS_MENU_BREAK_ON_OPENGL_ERROR:
        {
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND_GROUP;
            retVal = GD_STR_DebugMenuString;
            retVal.append(AF_Str_MenuSeparator);
            retVal.append(GD_STR_BreakPointsMenuString);
        }
        break;

        case ID_BREAKPOINTS_MENU_BREAK_ON_OPENCL_ERROR:
        case ID_BREAKPOINTS_MENU_BREAK_ON_DETECTED_ERROR:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        case ID_BREAKPOINTS_MENU_BREAK_ON_SOFTWARE_FALLBACKS:
#endif
        case ID_BREAKPOINTS_MENU_BREAK_ON_REDUNDANT_STATE_CHANGES:
        case ID_BREAKPOINTS_MENU_BREAK_ON_DEPRECATED_FUNCTIONS:
        case ID_BREAKPOINTS_MENU_BREAK_ON_MEMORY_LEAKS:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
        case ID_BREAKPOINTS_MENU_GL_DEBUG_OUTPUT_SETTINGS:
        case ID_BREAKPOINTS_MENU_GL_DEBUG_OUTPUT_BREAK:
#endif
            {
                retVal = GD_STR_DebugMenuString;
                retVal.append(AF_Str_MenuSeparator);
                retVal.append(GD_STR_BreakPointsMenuString);
            }
            break;

        case ID_BREAKPOINTS_MENU_ENABLE_DISABLE_ALL_BREAKPOINTS:
        case ID_BREAKPOINTS_MENU_ADD_REMOVE_BREAKPOINTS:
        {
            retVal = GD_STR_DebugMenuString;
        }
        break;

        case ID_DEBUG_SETTINGS:
        {
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
            retVal = GD_STR_DebugMenuString;
        }
        break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_RAWDATA:
        case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_IMAGE:
        case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_RAWDATA_AS:
        case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_IMAGE_AS:
        {
            // The recent projects menu items should come before the exit command:
            retVal = AF_STR_FileMenuString;
            retVal.append(AF_Str_MenuSeparator);
            retVal.append(GD_STR_exportFileMenu);

            positionData.m_beforeActionMenuPosition = AF_STR_FileMenuString;
            positionData.m_beforeActionMenuPosition.append(AF_Str_MenuSeparator);
            positionData.m_beforeActionMenuPosition.append(GD_STR_exportFileMenu);
            positionData.m_beforeActionText = GD_STR_StatisticsViewerFileExportTotalStatistics;

        }
        break;

        default:
            GT_ASSERT(false);
            break;
    };

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwMenuActionsExecutor::toolbarPosition
// Description: Toolbar position
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
gtString gwMenuActionsExecutor::toolbarPosition(int actionIndex)
{
    gtString retVal;

    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {

        case ID_START_DEBUGGING:
        case ID_BREAK_DEBUGGING:
        case ID_STOP_DEBUGGING:
        case ID_DEBUG_MODE:
        case ID_API_STEP_DEBUGGING:
        case ID_DRAW_STEP_DEBUGGING:
        case ID_FRAME_STEP_DEBUGGING:
        case ID_STEP_IN_DEBUGGING:
        case ID_STEP_OVER_DEBUGGING:
        case ID_STEP_OUT_DEBUGGING:
        case ID_DEBUG_SETTINGS:

        /*      retVal = GD_STR_DebugMenuString;
        break;*/

        default:
            break;
    };

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwMenuActionsExecutor::handleTrigger
// Description: Handle the action when it is triggered
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gwMenuActionsExecutor::handleTrigger(int actionIndex)
{
    // Check if the triggered action is checked (relevant only for the actions that check is relevant):
    bool isActionChecked = false;

    // Get the QT action object:
    QAction* pAction = action(actionIndex);
    GT_IF_WITH_ASSERT(pAction != NULL)
    {
        isActionChecked = pAction->isChecked();
    }

    GT_IF_WITH_ASSERT(_pApplicationCommandsHandler != NULL)
    {
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {
            case ID_SAVE_STATE_VARIABLES:
                _pApplicationCommandsHandler->onFileSaveStateVariables();
                break;

            case ID_START_DEBUGGING:
                _pApplicationCommandsHandler->onDebugStart();
                break;

            case ID_DEBUG_MODE:
            {
                apExecutionModeChangedEvent executionModeEvent(GD_STR_executionMode, 0);
                apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
            }
            break;

            case ID_API_STEP_DEBUGGING:
                _pApplicationCommandsHandler->onDebugAPIStep();
                break;

            case ID_DRAW_STEP_DEBUGGING:
                _pApplicationCommandsHandler->onDebugDrawStep();
                break;

            case ID_FRAME_STEP_DEBUGGING:
                _pApplicationCommandsHandler->onDebugFrameStep();
                break;

            case ID_STEP_IN_DEBUGGING:
                _pApplicationCommandsHandler->onDebugStepIn();
                break;

            case ID_STEP_OVER_DEBUGGING:
                _pApplicationCommandsHandler->onDebugStepOver();
                break;

            case ID_STEP_OUT_DEBUGGING:
                _pApplicationCommandsHandler->onDebugStepOut();
                break;

            case ID_BREAK_DEBUGGING:
                _pApplicationCommandsHandler->onDebugBreak();
                break;

            case ID_STOP_DEBUGGING:
                _pApplicationCommandsHandler->onDebugStopDebugging();
                break;

            case ID_BREAKPOINTS_MENU_BREAK_ON_OPENGL_ERROR:
                _pApplicationCommandsHandler->onBreakGeneric(AP_BREAK_ON_GL_ERROR, isActionChecked);
                break;

            case ID_BREAKPOINTS_MENU_BREAK_ON_OPENCL_ERROR:
                _pApplicationCommandsHandler->onBreakGeneric(AP_BREAK_ON_CL_ERROR, isActionChecked);
                break;

            case ID_BREAKPOINTS_MENU_BREAK_ON_DETECTED_ERROR:
                _pApplicationCommandsHandler->onBreakGeneric(AP_BREAK_ON_DETECTED_ERROR, isActionChecked);
                break;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

            case ID_BREAKPOINTS_MENU_BREAK_ON_SOFTWARE_FALLBACKS:
                _pApplicationCommandsHandler->onBreakGeneric(AP_BREAK_ON_SOFTWARE_FALLBACK, isActionChecked);
                break;
#endif

            case ID_BREAKPOINTS_MENU_BREAK_ON_REDUNDANT_STATE_CHANGES:
                _pApplicationCommandsHandler->onBreakGeneric(AP_BREAK_ON_REDUNDANT_STATE_CHANGE, isActionChecked);
                break;

            case ID_BREAKPOINTS_MENU_BREAK_ON_DEPRECATED_FUNCTIONS:
                _pApplicationCommandsHandler->onBreakGeneric(AP_BREAK_ON_DEPRECATED_FUNCTION, isActionChecked);
                break;

            case ID_BREAKPOINTS_MENU_BREAK_ON_MEMORY_LEAKS:
                _pApplicationCommandsHandler->onBreakGeneric(AP_BREAK_ON_MEMORY_LEAK, isActionChecked);
                break;
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

            case ID_BREAKPOINTS_MENU_GL_DEBUG_OUTPUT_BREAK:
                _pApplicationCommandsHandler->onBreakGeneric(AP_BREAK_ON_DEBUG_OUTPUT, isActionChecked);
                break;

            case ID_BREAKPOINTS_MENU_GL_DEBUG_OUTPUT_SETTINGS:
                _pApplicationCommandsHandler->onBreakDebugOutputSetting();
                break;
#endif

            case ID_BREAKPOINTS_MENU_ENABLE_DISABLE_ALL_BREAKPOINTS:
                _pApplicationCommandsHandler->enableAllBreakpoints(isActionChecked);
                break;

            case ID_BREAKPOINTS_MENU_ADD_REMOVE_BREAKPOINTS:
                _pApplicationCommandsHandler->openBreakpointsDialog();
                break;

            case ID_DEBUG_SETTINGS:
                afApplicationCommands::instance()->OnProjectSettings(GD_STR_projectSettingsExtensionDisplayName);
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_IMAGE_AS:
            {
                afApplicationTreeItemData* pMonitoredItemData = gdImagesAndBuffersManager::instance().controller().getCurrentlySelectedViewerItem();

                if (pMonitoredItemData != NULL)
                {
                    gdImagesAndBuffersManager::instance().controller().onSaveImageAs(pMonitoredItemData);
                }

                break;
            }

            case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_RAWDATA_AS:
            {
                afApplicationTreeItemData* pMonitoredItemData = gdImagesAndBuffersManager::instance().controller().getCurrentlySelectedViewerItem();

                if (pMonitoredItemData != NULL)
                {
                    gdImagesAndBuffersManager::instance().controller().onSaveRawDataAs(pMonitoredItemData);
                }
            }
            break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_IMAGE:
                gdImagesAndBuffersManager::instance().controller().onSaveAllAsImage();
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_RAWDATA:
                gdImagesAndBuffersManager::instance().controller().onSaveAllAsRawData();
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
// Name:        gwMenuActionsExecutor::handleUiUpdate
// Description: Handle UI update
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gwMenuActionsExecutor::handleUiUpdate(int actionIndex)
{
    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false;

    QString actionText;

    GT_IF_WITH_ASSERT(_pApplicationCommandsHandler != NULL)
    {
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {
            case ID_SAVE_STATE_VARIABLES:
                _pApplicationCommandsHandler->onUpdateFileSaveStateVariables(isActionEnabled);
                break;

            case ID_START_DEBUGGING:
                _pApplicationCommandsHandler->onUpdateDebugStart(isActionEnabled);
                actionText = FindStartDebugActionText();
                break;

            case ID_DEBUG_MODE:
            {
                // Enable if the active mode is not debug mode:
                bool isInDebugMode = (afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode));
                afRunModes runModes = afPluginConnectionManager::instance().getCurrentRunModeMask();
                isActionEnabled = (!(isInDebugMode) && (runModes == 0) && gwgDEBuggerAppWrapper::s_loadEnabled);
                // If the debug mode is not enabled it means that it is checked:
                isActionChecked = isInDebugMode;
                isActionCheckable = true;

                actionText = isInDebugMode ? acGTStringToQString(GD_STR_DebugMode) : acGTStringToQString(GD_STR_SwitchToDebugMode);
            }
            break;

            case ID_API_STEP_DEBUGGING:
                _pApplicationCommandsHandler->onUpdateDebugStep(isActionEnabled);
                break;

            case ID_DRAW_STEP_DEBUGGING:
                _pApplicationCommandsHandler->onUpdateDebugStep(isActionEnabled);
                break;

            case ID_FRAME_STEP_DEBUGGING:
                _pApplicationCommandsHandler->onUpdateDebugStep(isActionEnabled);
                break;

            case ID_STEP_IN_DEBUGGING:
                _pApplicationCommandsHandler->onUpdateDebugStepIn(isActionEnabled);
                break;

            case ID_STEP_OVER_DEBUGGING:
                _pApplicationCommandsHandler->onUpdateDebugStepOut(isActionEnabled);
                break;

            case ID_STEP_OUT_DEBUGGING:
                _pApplicationCommandsHandler->onUpdateDebugStepOut(isActionEnabled);
                break;

            case ID_BREAK_DEBUGGING:
                _pApplicationCommandsHandler->onUpdateDebugBreak(isActionEnabled);
                break;

            case ID_STOP_DEBUGGING:
                _pApplicationCommandsHandler->onUpdateDebugStopDebugging(isActionEnabled);
                break;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

            case ID_BREAKPOINTS_MENU_GL_DEBUG_OUTPUT_SETTINGS:
                _pApplicationCommandsHandler->onUpdateOutputSettingDialog(isActionEnabled);
                break;
#endif

            case ID_BREAKPOINTS_MENU_ADD_REMOVE_BREAKPOINTS:
                isActionEnabled = _pApplicationCommandsHandler->isBreakpointsDialogCommandEnabled();
                break;

            case ID_BREAKPOINTS_MENU_ENABLE_DISABLE_ALL_BREAKPOINTS:
                isActionEnabled = _pApplicationCommandsHandler->isEnableAllBreakpointsCommandEnabled(isActionChecked);
                isActionCheckable = true;
                break;

            case ID_BREAKPOINTS_MENU_BREAK_ON_OPENCL_ERROR:
                _pApplicationCommandsHandler->onUpdateBreakGeneric(AP_BREAK_ON_CL_ERROR, isActionEnabled, isActionChecked);
                isActionCheckable = true;
                break;

            case ID_BREAKPOINTS_MENU_BREAK_ON_OPENGL_ERROR:
                _pApplicationCommandsHandler->onUpdateBreakGeneric(AP_BREAK_ON_GL_ERROR, isActionEnabled, isActionChecked);
                isActionCheckable = true;
                break;

            case ID_BREAKPOINTS_MENU_BREAK_ON_DETECTED_ERROR:
                _pApplicationCommandsHandler->onUpdateBreakGeneric(AP_BREAK_ON_DETECTED_ERROR, isActionEnabled, isActionChecked);
                isActionCheckable = true;
                break;

            case ID_BREAKPOINTS_MENU_BREAK_ON_REDUNDANT_STATE_CHANGES:
                _pApplicationCommandsHandler->onUpdateBreakGeneric(AP_BREAK_ON_REDUNDANT_STATE_CHANGE, isActionEnabled, isActionChecked);
                isActionCheckable = true;
                break;

            case ID_BREAKPOINTS_MENU_BREAK_ON_DEPRECATED_FUNCTIONS:
                _pApplicationCommandsHandler->onUpdateBreakGeneric(AP_BREAK_ON_DEPRECATED_FUNCTION, isActionEnabled, isActionChecked);
                isActionCheckable = true;
                break;

            case ID_BREAKPOINTS_MENU_BREAK_ON_MEMORY_LEAKS:
                _pApplicationCommandsHandler->onUpdateBreakGeneric(AP_BREAK_ON_MEMORY_LEAK, isActionEnabled, isActionChecked);
                isActionCheckable = true;
                break;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

            case ID_BREAKPOINTS_MENU_GL_DEBUG_OUTPUT_BREAK:
                _pApplicationCommandsHandler->onUpdateBreakGeneric(AP_BREAK_ON_DEBUG_OUTPUT, isActionEnabled, isActionChecked);
                isActionCheckable = true;
                break;
#endif

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

            case ID_BREAKPOINTS_MENU_BREAK_ON_SOFTWARE_FALLBACKS:
                _pApplicationCommandsHandler->onUpdateBreakGeneric(AP_BREAK_ON_SOFTWARE_FALLBACK, isActionEnabled, isActionChecked);
                isActionCheckable = true;
                break;
#endif

            case ID_DEBUG_SETTINGS:
            {
                bool isRunning = (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_EXISTS);
                bool isDebugMode = (afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode));

                isActionEnabled = !isRunning && isDebugMode;
            }
            break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_IMAGE:
            case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_RAWDATA:
                isActionEnabled = gdImagesAndBuffersManager::instance().controller().shouldEnableSaveAllCommand();
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_RAWDATA_AS:
                isActionEnabled = gdImagesAndBuffersManager::instance().controller().shouldEnableSaveRawDataCommand();
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_IMAGE_AS:
                isActionEnabled = gdImagesAndBuffersManager::instance().controller().shouldEnableImageViewingCommand();
                break;

            default:
                GT_ASSERT_EX(false, L"Unknown event id");
                break;
        }
    }

    // Wait for all events to be handled:
    if (!apEventsHandler::instance().areNoEventsPending())
    {
        isActionEnabled = false;
    }

    // Get the QT action:
    QAction* pAction = action(actionIndex);
    GT_IF_WITH_ASSERT(pAction != NULL)
    {
        // Set the action enable / disable:
        pAction->setEnabled(isActionEnabled);

        // Set the action checkable state:
        pAction->setCheckable(isActionCheckable);

        // Set the action check state:
        pAction->setChecked(isActionChecked);

        if (!actionText.isEmpty())
        {
            pAction->setText(actionText);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gwMenuActionsExecutor::groupAction
// Description: If the requested action should be a part of a group, create
//              the action group and add the relevant actions to the group
// Arguments:   int actionIndex
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void gwMenuActionsExecutor::groupAction(int actionIndex)
{
#if (GW_SUPPORT_PROFILING == 1)
    int commandId = actionIndexToCommandId(actionIndex);

    if (commandId == ID_ANALYZE_MODE_MENU)
    {
        // Get my action:
        QAction* pAction = action(actionIndex);
        GT_IF_WITH_ASSERT(pAction != NULL)
        {
            // Group the execution mode actions:
            QActionGroup* pActionsGroup = new QActionGroup(pAction->parentWidget());

            int newActionIndex;
            commandIdToActionIndex(ID_DEBUGGING_MODE_MENU, newActionIndex);
            // Add the actions to the group:

            QAction* pDebugAction = action(newActionIndex);
            GT_IF_WITH_ASSERT(pDebugAction != NULL)
            {
                pActionsGroup->addAction(pDebugAction);
                pDebugAction->setActionGroup(pActionsGroup);
                pDebugAction->setChecked(true);
            }

            commandIdToActionIndex(ID_PROFILING_MODE_MENU, newActionIndex);
            QAction* pProfileAction = action(newActionIndex);
            GT_IF_WITH_ASSERT(pProfileAction != NULL)
            {
                pActionsGroup->addAction(pProfileAction);
                pProfileAction->setActionGroup(pActionsGroup);
            }

            commandIdToActionIndex(ID_ANALYZE_MODE_MENU, newActionIndex);
            QAction* pAnalyzeAction = action(newActionIndex);
            GT_IF_WITH_ASSERT(pAnalyzeAction != NULL)
            {
                pActionsGroup->addAction(pAnalyzeAction);
                pAnalyzeAction->setActionGroup(pActionsGroup);
            }
        }
    }

#else
    (void)(actionIndex); // unused
#endif
}

QString gwMenuActionsExecutor::FindStartDebugActionText()
{
    // Set the action text:
    QString retVal = GD_STR_StartDebuggingA;

    if (!afProjectManager::instance().currentProjectSettings().executablePath().isEmpty())
    {
        gtString fileName;
        afProjectManager::instance().currentProjectSettings().executablePath().getFileNameAndExtension(fileName);
        QString argsStr = acGTStringToQString(fileName);
        retVal = QString(GD_STR_StartDebuggingWithParam).arg(argsStr);
    }

    return retVal;
}

