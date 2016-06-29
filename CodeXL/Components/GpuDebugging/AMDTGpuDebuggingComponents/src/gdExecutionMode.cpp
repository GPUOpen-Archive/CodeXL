//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdExecutionMode.cpp
///
//==================================================================================

//------------------------------ gdExecutionMode.cpp ------------------------------

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdExecutionMode.h>

// ---------------------------------------------------------------------------
// Name:        gdExecutionMode::gdExecutionMode
// Description: constructor
// Author:      Gilad Yarnitzky
// Date:        10/5/2012
// ---------------------------------------------------------------------------
gdExecutionMode::gdExecutionMode() : m_isModeEnabled(true)
{

}


// ---------------------------------------------------------------------------
// Name:        gdExecutionMode::~gdExecutionMode
// Description: destructor
// Author:      Gilad Yarnitzky
// Date:        10/5/2012
// ---------------------------------------------------------------------------
gdExecutionMode::~gdExecutionMode()
{

}

// ---------------------------------------------------------------------------
// Name:        gdExecutionMode::modeName
// Description: Mode name for identification
// Author:      Gilad Yarnitzky
// Date:        10/5/2012
// ---------------------------------------------------------------------------
gtString gdExecutionMode::modeName()
{
    return GD_STR_executionMode;
}

// ---------------------------------------------------------------------------
// Name:        gdExecutionMode::modeActionString
// Description: The name of the action the mode encompasses
// Return Val:  gtString
// Author:      Uri Shomroni
// Date:        23/5/2012
// ---------------------------------------------------------------------------
gtString gdExecutionMode::modeActionString()
{
    return GD_STR_executionModeAction;
}

// ---------------------------------------------------------------------------
// Name:        gdExecutionMode::modeVerbString
// Description: The action verb the mode encompasses
// Return Val:  gtString
// Author:      Doron Ofek
// Date:        03/10/2012
// ---------------------------------------------------------------------------
gtString gdExecutionMode::modeVerbString()
{
    return GD_STR_executionModeVerb;
}

// ---------------------------------------------------------------------------
// Name:        gdExecutionMode::modeDescription
// Description: The mode description for tooltips
// Author:      Uri Shomroni
// Date:        15/10/2013
// ---------------------------------------------------------------------------
gtString gdExecutionMode::modeDescription()
{
    return GD_STR_executionModeDescription;
}

// ---------------------------------------------------------------------------
// Name:        gdExecutionMode::execute
// Description: Execute the command by id
// Arguments:   afExecutionCommandId commandId
// Author:      Gilad Yarnitzky
// Date:        10/5/2012
// ---------------------------------------------------------------------------
void gdExecutionMode::execute(afExecutionCommandId commandId)
{
    switch (commandId)
    {
        case AF_EXECUTION_ID_START:         gdApplicationCommands::gdInstance()->onDebugStart();            break;

        case AF_EXECUTION_ID_BREAK:         gdApplicationCommands::gdInstance()->onDebugBreak();            break;

        case AF_EXECUTION_ID_STOP:          gdApplicationCommands::gdInstance()->onDebugStopDebugging();    break;

        case AF_EXECUTION_ID_API_STEP:      gdApplicationCommands::gdInstance()->onDebugAPIStep();          break;

        case AF_EXECUTION_ID_DRAW_STEP:     gdApplicationCommands::gdInstance()->onDebugDrawStep();         break;

        case AF_EXECUTION_ID_FRAME_STEP:    gdApplicationCommands::gdInstance()->onDebugFrameStep();        break;

        case AF_EXECUTION_ID_STEP_IN:       gdApplicationCommands::gdInstance()->onDebugStepIn();           break;

        case AF_EXECUTION_ID_STEP_OVER:     gdApplicationCommands::gdInstance()->onDebugStepOver();         break;

        case AF_EXECUTION_ID_STEP_OUT:      gdApplicationCommands::gdInstance()->onDebugStepOut();          break;

        default: break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdExecutionMode::updateUI
// Description: update the UI of the action base on the command id
// Author:      Gilad Yarnitzky
// Date:        10/5/2012
// ---------------------------------------------------------------------------
void gdExecutionMode::updateUI(afExecutionCommandId commandId, QAction* pAction)
{
    bool isActionEnabled = true;
    bool isActionVisible = true;

    GT_IF_WITH_ASSERT(NULL != pAction)
    {
        switch (commandId)
        {
            case AF_EXECUTION_ID_CANCEL_BUILD:
            case AF_EXECUTION_ID_BUILD:
            case AF_EXECUTION_ID_CAPTURE:
            case AF_EXECUTION_ID_CAPTURE_CPU:
            case AF_EXECUTION_ID_CAPTURE_GPU:
            {
                isActionVisible = false;
            }
            break;

            case AF_EXECUTION_ID_START:
            {
                gdApplicationCommands::gdInstance()->onUpdateDebugStart(isActionEnabled);
                break;
            }

            case AF_EXECUTION_ID_API_STEP:
            {
                gdApplicationCommands::gdInstance()->onUpdateDebugStep(isActionEnabled);
                break;
            }

            case AF_EXECUTION_ID_DRAW_STEP:
            {
                gdApplicationCommands::gdInstance()->onUpdateDebugStep(isActionEnabled);
                break;
            }

            case AF_EXECUTION_ID_FRAME_STEP:
            {
                gdApplicationCommands::gdInstance()->onUpdateDebugStep(isActionEnabled);
                break;
            }

            case AF_EXECUTION_ID_STEP_IN:
            {
                gdApplicationCommands::gdInstance()->onUpdateDebugStepIn(isActionEnabled);
                break;
            }

            case AF_EXECUTION_ID_STEP_OVER:
            {
                gdApplicationCommands::gdInstance()->onUpdateDebugStepOut(isActionEnabled);
                break;
            }

            case AF_EXECUTION_ID_STEP_OUT:
            {
                gdApplicationCommands::gdInstance()->onUpdateDebugStepOut(isActionEnabled);
                break;
            }

            case AF_EXECUTION_ID_BREAK:
            {
                gdApplicationCommands::gdInstance()->onUpdateDebugBreak(isActionEnabled);
                pAction->setText(AF_STR_BreakDebugging);
                break;
            }

            case AF_EXECUTION_ID_STOP:
            {
                gdApplicationCommands::gdInstance()->onUpdateDebugStopDebugging(isActionEnabled);
                break;
            }

            default: break;
        }

        pAction->setEnabled(isActionEnabled);
        pAction->setVisible(isActionVisible);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdExecutionMode::sessionTypeName
// Description: Get the name of the session type by index
// Author:      Gilad Yarnitzky
// Date:        10/5/2012
// ---------------------------------------------------------------------------
gtString gdExecutionMode::sessionTypeName(int sessionTypeIndex)
{
    gtString sessionName;

    GT_IF_WITH_ASSERT(sessionTypeIndex >= 0 && sessionTypeIndex < 1)
    {
        sessionName = GD_STR_executionSesionType;
    }

    return sessionName;

}

// ---------------------------------------------------------------------------
// Name:        gdExecutionMode::sessionTypeIcon
// Description: Get the icon of the session type by index
// Author:      Gilad Yarnitzky
// Date:        10/5/2012
// ---------------------------------------------------------------------------
QPixmap* gdExecutionMode::sessionTypeIcon(int sessionTypeIndex)
{
    QPixmap* pPixmap = NULL;

    GT_IF_WITH_ASSERT(sessionTypeIndex >= 0 && sessionTypeIndex < 1)
    {
        pPixmap = new QPixmap;
        acSetIconInPixmap(*pPixmap, AC_ICON_DEBUG_MODE);
    }

    return pPixmap;
}

bool gdExecutionMode::ExecuteStartupAction(afStartupAction action)
{
    bool retVal = false;

    if (action == AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT_DEBUG)
    {
        // Open the new project settings dialog:
        afApplicationCommands::instance()->OnFileNewProject();
    }

    return retVal;
}


gtString gdExecutionMode::HowToStartModeExecutionMessage()
{
    gtString retStr = GD_STR_PropertiesExecutionInformationSA;

    if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        retStr = GD_STR_PropertiesExecutionInformationVS;
    }

    return retStr;
}

bool gdExecutionMode::IsRemoteEnabledForSessionType(const gtString& sessionType)
{
    GT_UNREFERENCED_PARAMETER(sessionType);

    // We only support remote debugging in standalone:
    bool retVal = !afGlobalVariablesManager::instance().isRunningInsideVisualStudio();
    return retVal;
}

bool gdExecutionMode::isModeEnabled()
{
    return m_isModeEnabled;
}

void gdExecutionMode::GetToolbarStartButtonText(gtString& buttonText, bool fullString /*= true*/)
{
    gtString exeFileName;
    afProjectManager::instance().currentProjectSettings().executablePath().getFileNameAndExtension(exeFileName);

    buttonText = GD_STR_executionStartButton;

    if (!exeFileName.isEmpty() && fullString)
    {
        buttonText.appendFormattedString(AF_STR_playButtonExeNameOnly, exeFileName.asCharArray());
    }
}
