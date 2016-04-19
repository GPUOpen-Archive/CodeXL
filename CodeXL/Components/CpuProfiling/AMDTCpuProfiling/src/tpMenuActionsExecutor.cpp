//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpMenuActionsExecutor.cpp
///
//==================================================================================

//------------------------------ tpMenuActionsExecutor.h ------------------------------

// QT:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>

// Local:
#include <inc/tpMenuActionsExecutor.h>
#include <inc/StringConstants.h>

tpMenuActionsExecutor::tpMenuActionsExecutor() : afActionExecutorAbstract()
{
}

tpMenuActionsExecutor::~tpMenuActionsExecutor()
{
}

void tpMenuActionsExecutor::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_TP_MENU_ACTION_1);
    m_supportedCommandIds.push_back(ID_TP_MENU_ACTION_2);
}


void tpMenuActionsExecutor::initActionIcons()
{

}

bool tpMenuActionsExecutor::actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut)
{
    (void)keyboardShortcut;
    bool retVal = true;

    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {

        case ID_TP_MENU_ACTION_1:
            caption = CP_STR_menu_command_1;
            tooltip = CP_STR_menu_command_1;
            break;

        case ID_TP_MENU_ACTION_2:
            caption = CP_STR_menu_command_2;
            tooltip = CP_STR_menu_command_2;
            break;

        default:
            GT_ASSERT(false);
            retVal = false;
            break;

    };

    return retVal;
}

gtString tpMenuActionsExecutor::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {

        case ID_TP_MENU_ACTION_1:
        {
            retVal = CP_STR_menu_caption;
        }
        break;

        case ID_TP_MENU_ACTION_2:
        {
            retVal = CP_STR_menu_caption;
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
        }
        break;

        default:
            GT_ASSERT(false);
            break;
    };

    return retVal;
}


gtString tpMenuActionsExecutor::toolbarPosition(int actionIndex)
{
    (void)actionIndex;
    gtString retVal;
    return retVal;
}

void tpMenuActionsExecutor::handleTrigger(int actionIndex)
{
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_TP_MENU_ACTION_1:
        {
            acMessageBox::instance().information(acGTStringToQString(CP_STR_menu_command_1), acGTStringToQString(CP_STR_menu_command_1));
        }
        break;

        case ID_TP_MENU_ACTION_2:
        {
            acMessageBox::instance().information(acGTStringToQString(CP_STR_menu_command_1), acGTStringToQString(CP_STR_menu_command_2));
        }
        break;

        default:
            GT_ASSERT_EX(false, L"Unknown event id");
            break;
    }
}

void tpMenuActionsExecutor::handleUiUpdate(int actionIndex)
{
    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false;
    gtString updatedActionText;

    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_TP_MENU_ACTION_1:
        {
            static bool dummyBool = false;
            isActionCheckable = true;
            isActionEnabled = dummyBool;
            dummyBool = !dummyBool;

            if (dummyBool)
            {
                updatedActionText = CP_STR_menu_command_3;
            }
        }
        break;

        case ID_TP_MENU_ACTION_2:
        {
            isActionCheckable = false;
            isActionEnabled = true;
        }
        break;

        default:
            GT_ASSERT_EX(false, L"Unknown event id");
            break;
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

        // Update the text if needed:
        if (!updatedActionText.isEmpty())
        {
            pAction->setText(acGTStringToQString(updatedActionText));
        }
    }

}


void tpMenuActionsExecutor::groupAction(int actionIndex)
{
    (void)actionIndex;
}
