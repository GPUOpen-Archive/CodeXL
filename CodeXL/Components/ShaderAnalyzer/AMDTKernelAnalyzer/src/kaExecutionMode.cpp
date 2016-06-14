//------------------------------ kaExecutionMode.cpp ------------------------------

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAppWrapper.h>
#include <AMDTKernelAnalyzer/Include/kaExecutionMode.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>

kaExecutionMode::kaExecutionMode()
{

}

kaExecutionMode::~kaExecutionMode()
{

}

// ---------------------------------------------------------------------------
// Name:        kaExecutionMode::modeName
// Description: Mode name for identification
// Author:      Gilad Yarnitzky
// Date:        10/5/2012
// ---------------------------------------------------------------------------
gtString kaExecutionMode::modeName()
{
    return KA_STR_executionMode;
}

gtString kaExecutionMode::modeActionString()
{
    return KA_STR_executionModeAction;
}

gtString kaExecutionMode::modeVerbString()
{
    return KA_STR_executionModeVerb;
}

gtString kaExecutionMode::modeDescription()
{
    return KA_STR_executionModeDescription;
}

void kaExecutionMode::execute(afExecutionCommandId commandId)
{
    switch (commandId)
    {
        case AF_EXECUTION_ID_BUILD:
        {
            gtVector<osFilePath> filesToOpen;
            // close editor if it is open:
            kaApplicationTreeHandler::instance()->CloseEditor();

            GT_IF_WITH_ASSERT(kaApplicationCommands::instance().activeCLFiles(filesToOpen))
            {
                kaApplicationCommands::instance().buildCommand(filesToOpen);
            }
        }
        break;

        case AF_EXECUTION_ID_CANCEL_BUILD:
        {
            if (kaBackendManager::instance().isInBuild())
            {
                kaApplicationCommands::instance().cancelBuildCommand();
            }
        }
        break;

        case AF_EXECUTION_ID_START:
        case AF_EXECUTION_ID_BREAK:
        case AF_EXECUTION_ID_STOP:
        case AF_EXECUTION_ID_API_STEP:
        case AF_EXECUTION_ID_DRAW_STEP:
        case AF_EXECUTION_ID_FRAME_STEP:
        case AF_EXECUTION_ID_STEP_IN:
        case AF_EXECUTION_ID_STEP_OVER:
        case AF_EXECUTION_ID_STEP_OUT:
        case AF_EXECUTION_ID_CAPTURE:
        case AF_EXECUTION_ID_CAPTURE_CPU:
        case AF_EXECUTION_ID_CAPTURE_GPU:
        {
            break;
        }

        default: break;
    }
}


void kaExecutionMode::updateUI(afExecutionCommandId commandId, QAction* pAction)
{
    bool isActionEnabled = true;
    bool isActionVisible = true;

    GT_IF_WITH_ASSERT(NULL != pAction)
    {
        switch (commandId)
        {
            case AF_EXECUTION_ID_BUILD:
            {
                // set enable if not in build mode
                bool isInBuild = kaBackendManager::instance().isInBuild();
                const bool activeProgramHasFiles = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram() != nullptr && KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram()->HasFile();
                gtVector<osFilePath> filesToOpen;
                isActionEnabled = (!isInBuild &&  activeProgramHasFiles);
                break;
            }

            case AF_EXECUTION_ID_CANCEL_BUILD:
            {
                // set enable if not in build mode
                isActionEnabled = kaBackendManager::instance().isInBuild();
                break;
            }


            case AF_EXECUTION_ID_START:
            case AF_EXECUTION_ID_BREAK:
            case AF_EXECUTION_ID_STOP:
            case AF_EXECUTION_ID_API_STEP:
            case AF_EXECUTION_ID_DRAW_STEP:
            case AF_EXECUTION_ID_FRAME_STEP:
            case AF_EXECUTION_ID_STEP_IN:
            case AF_EXECUTION_ID_STEP_OVER:
            case AF_EXECUTION_ID_STEP_OUT:
            case AF_EXECUTION_ID_CAPTURE:
            case AF_EXECUTION_ID_CAPTURE_CPU:
            case AF_EXECUTION_ID_CAPTURE_GPU:
            {
                isActionVisible = false;
                isActionEnabled = false;
                break;
            }

            default: break;
        }

        pAction->setEnabled(isActionEnabled);
        pAction->setVisible(isActionVisible);

        // Use the start action different function:
        if (commandId == AF_EXECUTION_ID_START)
        {
            afExecutionModeManager::instance().UpdateStartActionVisibility(isActionVisible, isActionEnabled);
        }

    }
}

gtString kaExecutionMode::sessionTypeName(int sessionTypeIndex)
{
    gtString sessionName;

    GT_IF_WITH_ASSERT(sessionTypeIndex >= 0 && sessionTypeIndex < 1)
    {
        sessionName = KA_STR_executionSesionType;
    }

    return sessionName;

}

QPixmap* kaExecutionMode::sessionTypeIcon(int sessionTypeIndex)
{
    QPixmap* pPixmap = NULL;

    GT_IF_WITH_ASSERT(sessionTypeIndex >= 0 && sessionTypeIndex < 1)
    {
        pPixmap = new QPixmap;
        acSetIconInPixmap(*pPixmap, AC_ICON_BUILD_AND_ANALYZE_MODE);
    }

    return pPixmap;
}

bool kaExecutionMode::ExecuteStartupAction(afStartupAction action)
{
    bool retVal = false;

    if (action == AF_NO_PROJECT_USER_ACTION_NEW_FILE_FOR_ANALYZE)
    {
        // Create new file, and force project creation:
        osFilePath newFilePath;
        kaProgram* pProgram = nullptr;
        kaApplicationCommands::instance().NewFileCommand(true, newFilePath, pProgram, kaRenderingProgram::KA_PIPELINE_STAGE_NONE);

        if (newFilePath.exists())
        {
            kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

            if (pTreeHandler != nullptr && pProgram != nullptr)
            {
                afApplicationTreeItemData* pParentProgramItemData = pTreeHandler->GetProgramItemData(pProgram);
                // A program was associated with this file - add it to the program node
                pTreeHandler->AddFileNodeToProgramBranch(newFilePath, pParentProgramItemData, AF_TREE_ITEM_ITEM_NONE);
            }
        }

        retVal = true;
    }

    else if (action == AF_NO_PROJECT_USER_ACTION_ADD_FILE_FOR_ANALYZE)
    {
        gtVector<osFilePath> addedFilePaths;
        kaApplicationCommands::instance().AddFileCommand(addedFilePaths);
        retVal = true;
    }

    return retVal;
}

bool kaExecutionMode::IsStartupActionSupported(afStartupAction action)
{
    return ((action == AF_NO_PROJECT_USER_ACTION_ADD_FILE_FOR_ANALYZE) || (action == AF_NO_PROJECT_USER_ACTION_NEW_FILE_FOR_ANALYZE));
}

bool kaExecutionMode::isModeEnabled()
{
    return kaAppWrapper::s_loadEnabled;
}

gtString kaExecutionMode::HowToStartModeExecutionMessage()
{
    gtString retStr = KA_STR_PropertiesExecutionInformationSA;

    if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        retStr = KA_STR_PropertiesExecutionInformationVS;
    }

    return retStr;
}
