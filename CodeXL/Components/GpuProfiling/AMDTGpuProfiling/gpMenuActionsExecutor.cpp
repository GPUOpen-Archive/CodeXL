//------------------------------ cpMenuActionsExecutor.h ------------------------------

#include <qtIgnoreCompilerWarnings.h>

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
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
// Local:
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/gpMenuActionsExecutor.h>
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTGpuProfiling/ProfileManager.h>

gpMenuActionsExecutor::gpMenuActionsExecutor() : afActionExecutorAbstract(), m_pModeManager(nullptr)
{
    m_pModeManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    GT_ASSERT(m_pModeManager);
}

gpMenuActionsExecutor::~gpMenuActionsExecutor()
{
}

void gpMenuActionsExecutor::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_GP_START);
    m_supportedCommandIds.push_back(ID_GP_CAPTURE);
    m_supportedCommandIds.push_back(ID_GP_STOP);
    m_supportedCommandIds.push_back(ID_GP_REFRESH_FROM_SERVER);
    m_supportedCommandIds.push_back(ID_GP_MODE);
    m_supportedCommandIds.push_back(ID_GP_SETTINGS);
}


void gpMenuActionsExecutor::initActionIcons()
{
    // Initialize commands icons
    initSingleActionIcon(ID_GP_START, AC_ICON_EXECUTION_PLAY);
    initSingleActionIcon(ID_GP_CAPTURE, AC_ICON_EXECUTION_CAPTURE);
    initSingleActionIcon(ID_GP_STOP, AC_ICON_EXECUTION_STOP);
}

bool gpMenuActionsExecutor::actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut)
{
    (void)keyboardShortcut;
    bool retVal = true;

    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_GP_MODE:
            caption = GPU_STR_executionModeMenu;
            tooltip = GPU_STR_executionModeStatusbarString;
            break;

        case ID_GP_START:
            caption = GPU_STR_executionModeStart;
            tooltip = GPU_STR_executionModeStartStatus;
            break;

        case ID_GP_CAPTURE:
            caption = GPU_STR_executionModeCapture;
            tooltip = GPU_STR_executionModeCaptureStatus;
            break;

        case ID_GP_STOP:
            caption = GPU_STR_executionModeStop;
            tooltip = GPU_STR_executionModeStopStatus;
            break;

        case ID_GP_REFRESH_FROM_SERVER:
            caption = GPU_STR_executionModeRefreshFromServer;
            tooltip = GPU_STR_executionModeRefreshFromServerStatus;
            break;

        case ID_GP_SETTINGS:
            caption = GPU_STR_executionModeSetting;
            tooltip = GPU_STR_executionModeSettingsStatus;
            break;

        default:
            GT_ASSERT(false);
            retVal = false;
            break;

    };

    return retVal;
}

gtString gpMenuActionsExecutor::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_GP_START:
        case ID_GP_CAPTURE:
        case ID_GP_STOP:
        {
            retVal = AF_STR_FrameAnalysisMenuString;
        }
        break;

        case ID_GP_MODE:
        case ID_GP_SETTINGS:
        case ID_GP_REFRESH_FROM_SERVER:
        {
            retVal = AF_STR_FrameAnalysisMenuString;
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
        }
        break;

        default:
            GT_ASSERT(false);
            break;
    };

    return retVal;
}


gtString gpMenuActionsExecutor::toolbarPosition(int actionIndex)
{
    (void)actionIndex;
    gtString retVal;
    return retVal;
}

void gpMenuActionsExecutor::handleTrigger(int actionIndex)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pModeManager != nullptr)
    {
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {
            case ID_GP_MODE:
            {
                apExecutionModeChangedEvent executionModeEvent(GPU_STR_executionMode, 0);
                apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
            }
            break;

            case ID_GP_START:
            {
                m_pModeManager->OnStartFrameAnalysis();
            }
            break;

            case ID_GP_CAPTURE:
            {
                m_pModeManager->CaptureFrame(gpExecutionMode::FrameAnalysisCaptureType_LinkedTrace);
            }
            break;
            case ID_GP_CAPTURE_CPU:
            {
                m_pModeManager->CaptureFrame(gpExecutionMode::FrameAnalysisCaptureType_APITrace);
            }
            break;
            case ID_GP_CAPTURE_GPU:
            {
                m_pModeManager->CaptureFrame(gpExecutionMode::FrameAnalysisCaptureType_GPUTrace);
            }
            break;
            case ID_GP_STOP:
            {
                m_pModeManager->stopCurrentRun();
            }
            break;

            case ID_GP_REFRESH_FROM_SERVER:
            {
                m_pModeManager->RefreshLoadedProjectSessionsFromServer();
            }
            break;

            case ID_GP_SETTINGS:
            {
                afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
                GT_IF_WITH_ASSERT(NULL != pApplicationCommands)
                {
                    pApplicationCommands->OnProjectSettings(GPU_STR_projectSettingExtensionDisplayName);
                }
            }
            break;

            default:
                GT_ASSERT_EX(false, L"Unknown event id");
                break;
        }
    }
}

void gpMenuActionsExecutor::handleUiUpdate(int actionIndex)
{
    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false;
    gtString updatedActionText;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pModeManager != nullptr)
    {
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {
            case ID_GP_STOP:
            {
                isActionEnabled = m_pModeManager->IsStopEnabled();
            }
            break;

            case ID_GP_START:
            {
                isActionEnabled = m_pModeManager->IsStartEnabled(updatedActionText);
            }
            break;

            case ID_GP_CAPTURE:
            {
                isActionEnabled = m_pModeManager->IsCaptureEnabled();
            }
            break;

            case ID_GP_MODE:
            {
                // Enable if the active mode is not debug mode:
                bool isGPUModeActive = (afExecutionModeManager::instance().isActiveMode(GPU_STR_executionMode));
                afRunModes runModes = afPluginConnectionManager::instance().getCurrentRunModeMask();
                isActionEnabled = ((!isGPUModeActive) && (runModes == 0));
                isActionChecked = isGPUModeActive;

                updatedActionText = isGPUModeActive ? GPU_STR_executionModeMenu : GPU_STR_SwitchToAnalyzeMode;
            }
            break;

            case ID_GP_SETTINGS:
            {
                isActionEnabled = (afExecutionModeManager::instance().isActiveMode(GPU_STR_executionMode));
                isActionEnabled = isActionEnabled && (!m_pModeManager->IsSessionRunning());
            }
            break;

            case ID_GP_REFRESH_FROM_SERVER:
            {
                bool isProjectLoaded = !afProjectManager::instance().currentProjectSettings().projectName().isEmpty();
                isActionEnabled = (afExecutionModeManager::instance().isActiveMode(GPU_STR_executionMode));
                isActionEnabled = isActionEnabled && isProjectLoaded;
            }
            break;

            default:
                GT_ASSERT_EX(false, L"Unknown event id");
                break;
        }
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

        // Update the text if needed:
        if (!updatedActionText.isEmpty())
        {
            pAction->setText(acGTStringToQString(updatedActionText));
        }
    }

}


void gpMenuActionsExecutor::groupAction(int actionIndex)
{
    (void)actionIndex;
}
