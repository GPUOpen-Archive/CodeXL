//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscUtils.cpp
///
//==================================================================================

#include <Include/Public/vscUtils.h>

// VS:
#include <Include/vspStringConstants.h>
#include <Include/vscCoreInternalUtils.h>
#include <Include/../CodeXLVSPackageUi/CommandIds.h>

// Infra:
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>

// AMDTSharedProfiling:
#include <SharedProfileManager.h>
#include <SharedProfileSettingPage.h>
#include <StringConstants.h>
#include <ProfileApplicationTreeHandler.h>

// FrameAnlysis
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/gpProjectSettings.h>

class vscUtils
{
public:
    vscUtils() {}
    ~vscUtils() {}
};

void vscUtilsGetStartActionCommandName(wchar_t*& verbNameBuffer, wchar_t*& actionCommandStrBuffer, bool addKeyboardShortcut /*= false*/)
{
    actionCommandStrBuffer = NULL;
    verbNameBuffer         = NULL;

    gtString actionCommandStr;
    gtString verbName;

    // Get the exe file name:
    gtString exeFileName;
    afProjectManager::instance().currentProjectSettings().executablePath().getFileNameAndExtension(exeFileName);

    // Check if we're in profile mode:
    bool isProfiling = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);
    bool isSystemWide = (SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope == PM_PROFILE_SCOPE_SYS_WIDE);
    bool isExeSet = !exeFileName.isEmpty();

    // Get the current active mode
    afIExecutionMode* pExecMode = afExecutionModeManager::instance().activeMode();
    GT_IF_WITH_ASSERT(NULL != pExecMode)
    {
        // Get the verb relevant to the active mode
        verbName = pExecMode->modeVerbString();
    }

    bool processExists = (0 != ((AF_DEBUGGED_PROCESS_EXISTS)& afPluginConnectionManager::instance().getCurrentRunModeMask()));

    if (processExists)
    {
        actionCommandStr = VSP_STR_Continue;
    }
    else // !processExists
    {
        actionCommandStr = VSP_STR_StartGeneric;
        GT_IF_WITH_ASSERT(NULL != pExecMode)
        {
            actionCommandStr = pExecMode->modeActionString();
            actionCommandStr.prepend(VSP_STR_StartPrefix);

            if (isProfiling && isSystemWide)
            {
                if (isExeSet)
                {
                    actionCommandStr.appendFormattedString(L" (%ls + System-wide)", exeFileName.asCharArray());
                }
                else
                {
                    actionCommandStr.append(L" (System-wide)");
                }
            }
            else if (isExeSet)
            {
                actionCommandStr.appendFormattedString(L" (%ls)", exeFileName.asCharArray());
            }
        }
    }

    if (addKeyboardShortcut)
    {
        actionCommandStr.prepend('&');
    }

    // Now allocate the output strings.
    actionCommandStrBuffer = vscAllocateAndCopy(actionCommandStr);
    verbNameBuffer = vscAllocateAndCopy(verbName);
}

void vscUtilsUpdateProjectSettingsFromStartupProject(const wchar_t* execPath, const wchar_t* workDir,
                                                     const wchar_t* cmdArgs, const wchar_t* execEnv, bool isProjectOpened, bool isProjectTypeSupported, bool isNonNativeProject)
{
    GT_UNREFERENCED_PARAMETER(isProjectOpened);
    // First clear currnet settings. This will make sure that all views are cleared from previous project +
    // will imitate SA mechanism -> first close current project then open the new one.
    apProjectSettings emptyProject;
    afProjectManager::instance().setCurrentProject(emptyProject);

    GT_IF_WITH_ASSERT(isProjectTypeSupported)
    {
        // Read the CodeXL VS project settings:
        osFilePath execFilePath(execPath);
        osFilePath workDirFilePath(workDir);

        if (!execFilePath.isEmpty())
        {
            // After the project was loaded, override the arguments that are stored by VS projects:
            afProjectManager& theProjectManager = afProjectManager::instance();

            // Get the current project name:
            gtString projectName = theProjectManager.currentProjectSettings().projectName();

            // Read the project settings from the XML file:
            osFilePath vsFilePath;
            afGetVisualStudioProjectFilePath(execFilePath, projectName, vsFilePath);

            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
            {
                // Check if the project exists:
                bool vsCXLProjectExists = vsFilePath.exists();

                if (vsCXLProjectExists)
                {
                    pApplicationCommands->OnFileOpenProject(vsFilePath.asString());

                    // After the project settings was loaded, go through each of the extensions,
                    // and make sure that the plugins store their data in their own data structures:
                    // Get the amount of extension pages and amount of extensions:
                    int amountOfExtensions = theProjectManager.amountOfProjectExtensions();

                    for (int i = 0; i < amountOfExtensions; i++)
                    {
                        // Save the project settings for each of the extensions:
                        bool rc = theProjectManager.saveCurrentProjectData(i);
                        GT_ASSERT(rc);
                    }
                }
                else
                {
                    // Let the project settings extension know that the current settings are cleared:
                    afProjectManager::instance().EmitClearCurretProjectSettings();

                    // Before loading a new project, restore the default settings for all extensions:
                    afProjectManager::instance().restoreDefaultExtensionsProjectSettings();
                }

                apProjectSettings newProjectSettings = theProjectManager.currentProjectSettings();
                gtString projectName;
                execFilePath.getFileName(projectName);
                newProjectSettings.setExecutablePath(execFilePath);
                newProjectSettings.setProjectName(projectName);
                newProjectSettings.setWorkDirectory(workDirFilePath);
                newProjectSettings.setCommandLineArguments(cmdArgs);
                newProjectSettings.clearEnvironmentVariables();
                newProjectSettings.addEnvironmentVariablesString(execEnv, AF_STR_newProjectEnvironmentVariablesDelimiter);
                newProjectSettings.setShouldDisableVSDebugEngine(isNonNativeProject);
                theProjectManager.setProjectSettingsWithoutEvent(newProjectSettings);

                // Emit an executable changed signal:
                afProjectManager::instance().EmitExecutableChanged(acGTStringToQString(execPath), true, false);

                // If the project does not exist, create it:
                if (!vsCXLProjectExists)
                {
                    pApplicationCommands->OnFileSaveProject();

                    // NOTICE: After saving the file, the file should be loaded, since there are operations
                    // that are done in framework load project:
                    pApplicationCommands->OnFileOpenProject(vsFilePath.asString());

                    // After the file is opened, we should set the generic project settings again:
                    apProjectSettings newProjectSettings = theProjectManager.currentProjectSettings();
                    gtString projectName;
                    execFilePath.getFileName(projectName);
                    newProjectSettings.setExecutablePath(execFilePath);
                    newProjectSettings.setProjectName(projectName);
                    newProjectSettings.setWorkDirectory(workDirFilePath);
                    newProjectSettings.setCommandLineArguments(cmdArgs);
                    newProjectSettings.clearEnvironmentVariables();
                    newProjectSettings.addEnvironmentVariablesString(execEnv, AF_STR_newProjectEnvironmentVariablesDelimiter);
                    newProjectSettings.setShouldDisableVSDebugEngine(isNonNativeProject);
                    theProjectManager.setProjectSettingsWithoutEvent(newProjectSettings);
                    theProjectManager.setCurrentProjectFilePath(vsFilePath);
                }
            }
        }
        else
        {
            // Solution is closed, clear the settings for the infra and plugins
            // Let the project settings extension know that the current settings are cleared:
            afProjectManager::instance().EmitClearCurretProjectSettings();

            // Before loading a new project, restore the default settings for all extensions:
            afProjectManager::instance().restoreDefaultExtensionsProjectSettings();
        }
    }
}

bool vscGetExecutionCommandName(DWORD commandId, wchar_t*& commandNameBuffer)
{
    bool retVal = false;
    commandNameBuffer = NULL;

    bool isInProfileMode = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);
    bool isInFrameAnalysisMode = afExecutionModeManager::instance().isActiveMode(GPU_STR_executionMode);

    gtString commandName;

    if (commandId == cmdidStopProfiling)
    {
        commandName = isInProfileMode ? VSP_STR_StopProfiling : (isInFrameAnalysisMode ? VSP_STR_StopFrameAnalysis : VSP_STR_StopDebugging);
        retVal = true;
    }
    else if (commandId == cmdidBreakProfiling)
    {
        gtString currentProfileName = SharedProfileManager::instance().currentSelection();

        if (isInProfileMode)
        {
            commandName = VSP_STR_PauseProfiling;

            if (!currentProfileName.startsWith(L"CPU:"))
            {
                commandName = VSP_STR_BreakProfiling;
            }
        }
        else
        {
            commandName = VSP_STR_BreakDebugging;
        }

        retVal = true;
    }
    if (commandId == cmdidCaptureFrame)
    {
        gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
        gpProjectSettings& settings = pFrameAnalysisManager->ProjectSettings();

        gtString buttonText;
        buttonText.appendFormattedString(VSP_STR_CaptureFrame, acQStringToGTString(settings.m_numFramesToCapture).asCharArray());
        commandName = buttonText;
        retVal = true;

    }
    // Allocate the output string.
    commandNameBuffer = vscAllocateAndCopy(commandName);

    return retVal;
}
