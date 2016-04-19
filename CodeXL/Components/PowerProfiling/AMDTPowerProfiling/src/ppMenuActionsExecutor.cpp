//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppMenuActionsExecutor.cpp
///
//==================================================================================

// QT:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>

// AMDTSharedProfiling
#include <AMDTSharedProfiling/inc/StringConstants.h>

// Local:
#include <AMDTPowerProfiling/src/ppMenuActionsExecutor.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppCountersSelectionDialog.h>
#include <AMDTPowerProfiling/src/ppAppController.h>

// ---------------------------------------------------------------------------
ppMenuActionsExecutor::ppMenuActionsExecutor()
    : m_pApplicationCommandsHandler(nullptr)
{
    // Get the application commands handler:
    m_pApplicationCommandsHandler = afApplicationCommands::instance();
    GT_ASSERT(m_pApplicationCommandsHandler != nullptr);
}

// ---------------------------------------------------------------------------
ppMenuActionsExecutor::~ppMenuActionsExecutor()
{
}

// ---------------------------------------------------------------------------
// Name:        ppMenuActionsExecutor::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void ppMenuActionsExecutor::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(COMMAND_ID_COUNTERS_SELECTION);
}

// ---------------------------------------------------------------------------
bool ppMenuActionsExecutor::actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut)
{
    bool retVal = true;

    GT_UNREFERENCED_PARAMETER(keyboardShortcut);

    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case COMMAND_ID_COUNTERS_SELECTION:
            caption = PP_STR_CountersSelection;
            tooltip = PP_STR_CountersSelectionStatusbarString;
            break;

        default:
            GT_ASSERT(false);
            retVal = false;
            break;
    };

    return retVal;
}

// ---------------------------------------------------------------------------
gtString ppMenuActionsExecutor::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;
    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;

    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case COMMAND_ID_COUNTERS_SELECTION:
            retVal = AF_STR_ProfileMenuString;
            positionData.m_beforeActionMenuPosition = AF_STR_ProfileMenuString;
            positionData.m_beforeActionText = PM_STR_MENU_SETTINGS;
            break;

        default:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
gtString ppMenuActionsExecutor::toolbarPosition(int actionIndex)
{
    gtString retVal;

    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case COMMAND_ID_COUNTERS_SELECTION:
        default:
            /// not a tool bar item
            retVal = L"";
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void ppMenuActionsExecutor::handleTrigger(int actionIndex)
{
    GT_IF_WITH_ASSERT(m_pApplicationCommandsHandler != nullptr)
    {
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {
            case COMMAND_ID_COUNTERS_SELECTION:
                ppCountersSelectionDialog::OpenCountersSelectionDialog();
                break;

            default:
                GT_ASSERT_EX(false, L"Unsupported application command");
                break;
        }
    }
}

// ---------------------------------------------------------------------------
void ppMenuActionsExecutor::handleUiUpdate(int actionIndex)
{
    QString actionText;
    bool isActionEnabled = false;
    bool isActionChecked = false;
    bool isActionCheckable = false;

    afExecutionModeManager& executionMode = afExecutionModeManager::instance();
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case COMMAND_ID_COUNTERS_SELECTION:
            // Enable this command only if Power Profiling is supported according to this project's configuration
        {
            // Check if a session is already running.
            afPluginConnectionManager& thePluginConnectionManager = afPluginConnectionManager::instance();
            afRunModes currRunMode = thePluginConnectionManager.getCurrentRunModeMask();
            bool isSessionRunning = static_cast<int>(currRunMode) != 0;

            isActionEnabled = !isSessionRunning && executionMode.isActiveMode(PM_STR_PROFILE_MODE) && !ppAppController::instance().SessionIsOn();
        }
        break;

        default:
            GT_ASSERT_EX(false, L"Unknown event id");
            break;
    }

    // Wait for all events to be handled:
    if (!apEventsHandler::instance().areNoEventsPending())
    {
        isActionEnabled = false;
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

        if (!actionText.isEmpty())
        {
            pAction->setText(actionText);
        }
    }
}

// ---------------------------------------------------------------------------
void ppMenuActionsExecutor::groupAction(int actionIndex)
{
    // unused
    (void)(actionIndex);
}

// ---------------------------------------------------------------------------








