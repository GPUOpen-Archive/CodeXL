//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscPackage.cpp
///
//==================================================================================

#include "stdafx.h"

#include <dbgmetric.h>

#include <Include/Public/vscPackage.h>
#include <src/vscApplicationCommands.h>
// Local:
#include <Include/Public/vscCoreUtils.h>
#include <Include/vscCoreInternalUtils.h>
#include <src/vscDebugEngine.h>
#include <src/vscBreakpointsManager.h>
#include <src/vspProgressBarWrapper.h>
#include <src/vspWindowsManager.h>

// VS Debugger:
#include <CodeXLVSPackageDebugger/Include/vsdPackageConnector.h>

// VS:
#include <Include/vspStringConstants.h>
#include <src/vspKernelAnalyzerEditorManager.h>
#include <src/vspPowerProfilingEditorManager.h>
#include <src/vscAppEventObserver.h>
#include <src/vspGRApiFunctions.h>
#include <src/vspProgressBarWrapper.h>
#include <src/vspProfileWindowsManager.h>
#include <src/vspQApplicationWrapper.h>
#include <src/vspSaveListDialog.h>
#include <src/vscBuildListDialog.h>
#include <Include/../../CodeXLVSPackageUi/CommandIds.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAssertionHandlers/Include/ahDialogBasedAssertionFailureHandler.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/apProjectSettings.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMessageBox.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afUnhandledExceptionHandler.h>
#include <AMDTApplicationFramework/Include/commands/afInitializeApplicationCommand.h>
#include <AMDTApplicationFramework/Include/commands/afSystemInformationCommand.h>
#include <AMDTApplicationFramework/Include/dialogs/afHelpAboutDialog.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdFrameworkConnectionManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>


// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdExecutionMode.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdGlobalDebugSettingsPage.h>
#include <AMDTGpuDebuggingComponents/Include/gdProjectSettingsExtension.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeHandler.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdThreadsEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMultiWatchView.h>

// Cpu Profiling:
#include <SharedProfileManager.h>
#include <StringConstants.h>
#include <AMDTCpuProfiling/Inc/AmdtCpuProfiling.h>

// GPU Profiling:
#include <AMDTGpuProfiling/AMDTGpuProfilerPlugin.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

// Kernel Analyzer
#include <AMDTKernelAnalyzer/src/kaAnalyzeSettingsPage.h>
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/src/kaGlobalVariableManager.h>
#include <AMDTKernelAnalyzer/src/kaProjectSettingsExtension.h>
#include <AMDTKernelAnalyzer/src/kaProjectSettingsExtensionOther.h>
#include <AMDTKernelAnalyzer/src/kaProjectSettingsShaderExtension.h>
#include <AMDTKernelAnalyzer/Include/kaExecutionMode.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

// Power Profiling
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfiling/src/ppProjectSettingsExtension.h>
#include <AMDTPowerProfiling/src/ppTreeHandler.h>

#include <AMDTApplicationComponents/Include/acSendErrorReportDialog.h>

// **************************************************************************************
// If you are developing the CodeXL extension - you can uncomment the next line
// (which defines the CXL_VS_EXT_DEVELOPMENT PP directive) and then Qt will look
// for the Windows platform plug-in (qwindows.dll in release and qwindowsd.dll in
// debug) in a directory named "platforms" which should be located at the same
// directory where the VS executable (devenv.exe) resides. The location of devenv.exe
// for VS 2010, 2012, and 2013 has been: <VS_DIR>\Common7\IDE.
// **************************************************************************************
//#define CXL_VS_EXT_DEVELOPMENT

class vscPackage
{
public:
    vscPackage() : m_pDebugEngine(nullptr),
        _pSendErrorReportDialog(nullptr),
        _pDialogBasedAssertionFailureHandler(nullptr),
        m_pEventObserver(nullptr),
        m_pProgressBar(nullptr) {}

    ~vscPackage()
    {
        // Unregister and delete the assertion failure handler:
        if (_pDialogBasedAssertionFailureHandler != nullptr)
        {
            gtUnRegisterAssertionFailureHandler(_pDialogBasedAssertionFailureHandler);
            delete _pDialogBasedAssertionFailureHandler;
            _pDialogBasedAssertionFailureHandler = nullptr;
        }

        delete m_pProgressBar;
    }

    void OnNewDebugEngine(vspCDebugEngine* pNewEngine)
    {
        // Release any previous engine we were holding:
        if (nullptr != m_pDebugEngine)
        {
            m_pDebugEngine->Release();
            m_pDebugEngine = nullptr;

            // Inform the breakpoints manager of the engine's destruction:
            vscBreakpointsManager::instance().setDebugEngine(nullptr);
        }

        // Keep a pointer to the new engine and retain it:
        if (nullptr != pNewEngine)
        {
            m_pDebugEngine = pNewEngine;
            m_pDebugEngine->AddRef();

            // Inform the breakpoints manager of the engine's creation:
            vscBreakpointsManager::instance().setDebugEngine(m_pDebugEngine);
        }
    }

    void CreateSendErrorReportDialog()
    {
        // Create the Send Error Report dialog instance:
        QPixmap iconPixMap;
        acSetIconInPixmap(iconPixMap, afGlobalVariablesManager::ProductIconID(), AC_64x64_ICON);
        _pSendErrorReportDialog = new acSendErrorReportDialog(nullptr, afGlobalVariablesManager::ProductNameA(), iconPixMap);

        // Register it for receiving debugged process events:
        _pSendErrorReportDialog->registerForRecievingDebuggedProcessEvents();
    }

    void CreateDialogBasedAssertionHandler()
    {
        // Get the main GUI thread id:
        osThreadId mainGUIThreadId = osGetCurrentThreadId();

        // Create and register the dialog based assertion failure handler:
        _pDialogBasedAssertionFailureHandler = new ahDialogBasedAssertionFailureHandler(mainGUIThreadId);


        // Register the dialog based assertion failure handler:
        gtRegisterAssertionFailureHandler(_pDialogBasedAssertionFailureHandler);
    }

    void PreClosing()
    {
        if (_pSendErrorReportDialog)
        {
            delete _pSendErrorReportDialog;
            _pSendErrorReportDialog = nullptr;
        }

        // Cleanup the application commands:
        afApplicationCommands::cleanupInstance();
        gdApplicationCommands::cleanupGDInstance(false);

        // Cleanup the progress bar:
        afProgressBarWrapper::cleanupInstance();

        // Save general settings needed for dialogs not to show:
        afGlobalVariablesManager::instance().saveGlobalSettingsToXMLFile();

        // If we still hold a debug engine, release it:
        if (nullptr != m_pDebugEngine)
        {
            m_pDebugEngine->Release();
            m_pDebugEngine = nullptr;
        }

        // Release the interfaces held by the breakpoints manager:
        vscBreakpointsManager::instance().releaseInterfaces();

        // Let the modes do any mode specific termination that is needed
        afExecutionModeManager::instance().TerminateModes();
    }

    void InitProgressBar()
    {
        if (m_pProgressBar == nullptr)
        {
            bool rcRegsister;

            m_pProgressBar = new vscProgressBarWrapper();

            // Register the application command single instance:
            rcRegsister = afProgressBarWrapper::registerInstance(m_pProgressBar);

            // Initialize the progress bar wrapper
            afProgressBarWrapper::instance().initialize(nullptr);

            // Only single instance of gdProgressBar should be registered
            assert(rcRegsister);
        }
    }

    // Assertion failure handler:
    ahDialogBasedAssertionFailureHandler* _pDialogBasedAssertionFailureHandler;

    // Send error report dialog:
    acSendErrorReportDialog* _pSendErrorReportDialog;

    // event observer:
    vscAppEventObserver* m_pEventObserver;

    gdFrameworkConnectionManager m_frameworkConnectionManager;

    // The last debug engine that we used:
    vspCDebugEngine* m_pDebugEngine;

    afProgressBarWrapper* m_pProgressBar;

};

void* vsc_CreateInstance()
{
    return new vscPackage();
}

void vsc_DestroyInstance(void*& pInstance)
{
    delete pInstance;
    pInstance = nullptr;
}

void vsc_UpdateProjectSettingsFromVS(const wchar_t* executableFilePath,
                                     const wchar_t* workingDirPath, const wchar_t* cmdLineArgs, const wchar_t* env, bool& isDebuggingRequired, bool& isProfilingRequired, bool& isFrameAnalysisRequired)
{
    // Update the project settings if needed.
    apProjectSettings newProjectSettings = afProjectManager::instance().currentProjectSettings();

    if (newProjectSettings.executablePath().asString() != executableFilePath)
    {
        newProjectSettings.setExecutablePathFromString(executableFilePath);
    }

    if (newProjectSettings.workDirectory().asString() != workingDirPath)
    {
        newProjectSettings.setWorkDirectoryFromString(workingDirPath);
    }

    if (newProjectSettings.commandLineArguments() != cmdLineArgs)
    {
        newProjectSettings.setCommandLineArguments(cmdLineArgs);
    }

    newProjectSettings.clearEnvironmentVariables();

    newProjectSettings.addEnvironmentVariablesString(env, AF_STR_newProjectEnvironmentVariablesDelimiter);

    // Check which mode is on.
    isDebuggingRequired = false;
    isProfilingRequired = false;

    isDebuggingRequired = (newProjectSettings.lastActiveMode() == GD_STR_executionMode);
    isProfilingRequired = (newProjectSettings.lastActiveMode() == PM_STR_PROFILE_MODE);
    isFrameAnalysisRequired = (newProjectSettings.lastActiveMode() == PM_STR_FrameAnalysisMode);

    afProjectManager::instance().setProjectSettingsWithoutEvent(newProjectSettings);
}

bool vsc_ValidateDebugSettings(const wchar_t* executableFilePath, const wchar_t* workingDirPath, bool isProjectTypeValid)
{
    bool retVal = true;
    gtASCIIString errorMessage = VSP_STR_FailedToLaunchDebuggedProcess;

    // Verify this is a Visual C project:
    if (!isProjectTypeValid)
    {
        retVal = false;
        errorMessage = VSP_STR_NonSupportedProjectType;
    }

    // Verify the executable exists:
    osFilePath execPath(executableFilePath);

    if (retVal && !execPath.isExecutable())
    {
        retVal = false;

        if (executableFilePath != nullptr)
        {
            // Handle the case that no project has been loaded
            errorMessage = VSP_STR_NoExecutableSelectedForDebugging;
        }
        else
        {
            errorMessage = VSP_STR_DebuggedExecutableDoesNotExist;
            errorMessage.append('\"').append(executableFilePath).append('\"');
        }

        errorMessage.append(VSP_STR_PleaseCheckValuesInProjectSettings);
    }

    // Verify the working directory exists:
    osFilePath workDirPath(workingDirPath);

    if (retVal && (!workDirPath.exists()))
    {
        retVal = false;
        errorMessage = VSP_STR_WorkingDirectoryDoesNotExist;
        errorMessage.append('\"').append(workingDirPath).append('\"');
        errorMessage.append(VSP_STR_PleaseCheckValuesInProjectSettings);
    }

    // Verify we have the OpenCL DLL if we need it:
    bool testForMissingSystemLibraries = false;

    if (testForMissingSystemLibraries)
    {
        gdGDebuggerGlobalVariablesManager& theGlobalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
        gtString missingLibraries;
        gaSystemLibrariesExists(theGlobalVarsManager.currentDebugProjectSettings(), missingLibraries);
        missingLibraries.toLowerCase();

        if (retVal)
        {
            gtString clICDModuleName = OS_OPENCL_ICD_MODULE_ALTERNATIVE_NAME;
            clICDModuleName.toLowerCase();

            if (missingLibraries.find(clICDModuleName) > -1)
            {
                retVal = false;
                errorMessage = VSP_STR_OpenCLNotFound;
            }
        }

        // Verify we have the OpenGL DLL if we need it:
        if (retVal && (missingLibraries.find(OS_OPENGL_MODULE_NAME) > -1))
        {
            retVal = false;
            errorMessage = VSP_STR_OpenGLNotFound;
        }
    }

    // Verify if we need OpenCL dll that it is not locally with the executable
    osFilePath openCLPath(executableFilePath);
    openCLPath.setFileName(L"OpenCL");
    openCLPath.setFileExtension(L"dll");

    if (retVal && openCLPath.exists() && !vscIsCodeXLServerDll(openCLPath.asString().asCharArray()))
    {
        retVal = false;
        errorMessage = VSP_STR_OpenCLInSameDirectoryPrefix;
        errorMessage.append(openCLPath.fileDirectoryAsString().asASCIICharArray());
        errorMessage.append(VSP_STR_OpenCLInSameDirectorySuffix);
    }

    // Verify if we need OpenGL32 dll that it is not locally with the executable
    osFilePath openGLPath(executableFilePath);
    openGLPath.setFileName(L"opengl32");
    openGLPath.setFileExtension(L"dll");

    if (retVal && openGLPath.exists() && !vscIsCodeXLServerDll(openGLPath.asString().asCharArray()))
    {
        retVal = false;
        errorMessage = VSP_STR_OpenGLInSameDirectoryPrefix;
        errorMessage.append(openCLPath.fileDirectoryAsString().asASCIICharArray());
        errorMessage.append(VSP_STR_OpenGLInSameDirectorySuffix);
    }

    // If there's a problem, display a message to the user:
    if (!retVal)
    {
        acMessageBox::instance().critical(AF_STR_ErrorA, errorMessage.asCharArray());
    }

    return retVal;
}

void vsc_OnUpdateLaunchProfileAction(bool& isActionEnabled, bool& isActionChecked)
{
    bool isVisible(true);
    isActionEnabled = isActionEnabled && SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_START, isActionChecked, isVisible);
}

void vsc_OnUpdateLaunchFrameAnalysisAction(bool& isActionEnabled, bool& isActionChecked)
{
    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();

    if (pFrameAnalysisManager != nullptr)
    {
        // enable only if the frame analysis is not running
        isActionEnabled = isActionEnabled && (pFrameAnalysisManager->getCurrentRunModeMask() == 0);
        isActionChecked = false;
    }
}

bool vsc_CanStopCurrentRun()
{
    return SharedProfileManager::instance().canStopCurrentRun();
}

void vsc_ExecuteProfileSession()
{
    SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_START);
}

void vsc_ExecuteFrameAnalysisSession()
{
    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();

    if (pFrameAnalysisManager != nullptr)
    {
        pFrameAnalysisManager->OnStartFrameAnalysis();
    }
}

void vsc_RefreshFrameAnalysisSessionsFromServer()
{
    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();

    if (pFrameAnalysisManager != nullptr)
    {
        pFrameAnalysisManager->RefreshLoadedProjectSessionsFromServer();
    }
}

void vsc_DisplayDebugProcessLaunchFailureMessage(HRESULT failureHR)
{
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD

    if (failureHR == 0x89710016)
    {
        acMessageBox::instance().critical(AF_STR_ErrorA, "LaunchDebugTargets failed to create CodeXL extension debug engine.\n\nDid you remember to run CodeXLVSPackageRegistration.bat from the user extensions folder?\n\nC:\\Users\\<username>\\AppData\\Local\\Microsoft\\VisualStudio\\1*.0Exp\\Extensions\\...");
    }

#else
    GT_UNREFERENCED_PARAMETER(failureHR);
#endif
    // Display a message to the user:
    acMessageBox::instance().critical(AF_STR_ErrorA, VSP_STR_FailedToLaunchDebuggedProcess);
}

void vsc_GetCurrentSettings(wchar_t*& pExecutableFilePathBuffer, wchar_t*& pWorkingDirectoryPath, wchar_t*& pCommandLineArguments, wchar_t*& pEnvironment)
{
    // Init.
    pExecutableFilePathBuffer    = nullptr;
    pWorkingDirectoryPath        = nullptr;
    pCommandLineArguments        = nullptr;
    pEnvironment                 = nullptr;

    // Extract the strings.
    gtString executableFilePath = afProjectManager::instance().currentProjectSettings().executablePath().asString();
    gtString workingDirectoryPath = afProjectManager::instance().currentProjectSettings().workDirectory().asString();
    gtString commandLineArguments = afProjectManager::instance().currentProjectSettings().commandLineArguments();
    gtString environment;
    afProjectManager::instance().currentProjectSettings().environmentVariablesAsString(environment);

    // Allocate output strings.
    pExecutableFilePathBuffer = vscAllocateAndCopy(executableFilePath);
    pWorkingDirectoryPath = vscAllocateAndCopy(workingDirectoryPath);
    pCommandLineArguments = vscAllocateAndCopy(commandLineArguments);
    pEnvironment = vscAllocateAndCopy(environment);

}

bool vsc_IsInRunningMode()
{
    afPluginConnectionManager& connectionManager = afPluginConnectionManager::instance();
    return (connectionManager.getCurrentRunModeMask() == 0);
}

void vsc_CheckDebuggerInstallation()
{
    // Get the CodeXL extension installation dir and from there find the vs extensions installations:
    osFilePath codeXLExecuablePath(osFilePath::OS_CODEXL_BINARIES_PATH);

    osDirectory codeXLExecuableDir(codeXLExecuablePath);

    codeXLExecuableDir.upOneLevel(); // Go to the AMD directory

    // Look for the gDEBugger directory:
    gtString pathAsString = codeXLExecuablePath.fileDirectoryAsString();
    osFilePath amdFilePath = codeXLExecuableDir.directoryPath();
    amdFilePath.appendSubDirectory(VSP_STR_gDEBuggerName);

    // Add the gDEBuggger DLL name
    amdFilePath.setFileName(VSP_STR_gDEBuggerPackage);

    if (amdFilePath.exists())
    {
        // Notify the user he should remove gDEBugger:
        afMessageBox::instance().information(VSP_STR_gDEBuggerInstallerTitle, VSP_STR_gDEBuggerInstallerError);
    }

}

void vsc_InitKernelAnalyzerPlugin()
{
    // Create and register the global settings page:
    // initialize the kaBackendManager, must be initialized before the target device:
    QStringList asicTreeList;
    kaBackendManager::instance().getASICsTreeList(beKA::DeviceTableKind_OpenCL, false, &asicTreeList);
    kaGlobalVariableManager::instance().setDefaultTreeList(asicTreeList);
    kaAnalyzeSettingsPage* pKernelAnalyzerSettingsPage = new kaAnalyzeSettingsPage;

    afGlobalVariablesManager::instance().registerGlobalSettingsPage(pKernelAnalyzerSettingsPage);

    // Create and register the project settings object:
    kaProjectSettingsExtension* pGlobalKernelAnalyzerSettingExtension = new kaProjectSettingsExtension;

    afProjectManager::instance().registerProjectSettingsExtension(pGlobalKernelAnalyzerSettingExtension);

    kaProjectSettingsExtensionOther* pGlobalKernelAnalyzerSettingExtensionOther = new kaProjectSettingsExtensionOther;

    afProjectManager::instance().registerProjectSettingsExtension(pGlobalKernelAnalyzerSettingExtensionOther);

    kaProjectSettingsShaderExtension* pGlobalKernelAnalyzerSettingShaderExtension = new kaProjectSettingsShaderExtension;

    afProjectManager::instance().registerProjectSettingsExtension(pGlobalKernelAnalyzerSettingShaderExtension);

    // Create and Register the event and run mode manager:
    afPluginConnectionManager::instance().registerRunModeManager(&vspKernelAnalyzerEditorManager::instance());
    afPluginConnectionManager::instance().registerRunModeManager(&vspPowerProfilingEditorManager::instance());
}

void vsc_InitPowerProfilingPlugin()
{
    qRegisterMetaType<ppQtEventData>("ppQtEventData");

    // Create an event observer:
    ppAppController& appController = ppAppController::instance();

    // Register the run mode manager:
    afPluginConnectionManager::instance().registerRunModeManager(&appController);

    // Create and register the project settings object:
    ppProjectSettingsExtension* pProjectSettingsExtension = new ppProjectSettingsExtension;

    afProjectManager::instance().registerProjectSettingsExtension(pProjectSettingsExtension);

    // register the profile types
    SharedProfileManager::instance().registerProfileType(PP_STR_OnlineProfileName, &appController, PP_STR_projectSettingExtensionDisplayName, SPM_ALLOW_STOP | SPM_HIDE_PAUSE);

    // register the tree handler
    gtString offlineProfilingName(PP_STR_OnlineProfileName);
    ProfileApplicationTreeHandler::instance()->registerSessionTypeTreeHandler(offlineProfilingName.asASCIICharArray(), &ppTreeHandler::instance());
}

void vsc_InitDebuggerPlugin()
{
    // Create and register the project settings object:
    gdProjectSettingsExtension* pProjectSettingsExtension = new gdProjectSettingsExtension;

    afProjectManager::instance().registerProjectSettingsExtension(pProjectSettingsExtension);

    // Create and register the global settings page:
    gdGlobalDebugSettingsPage* pGlobalDebugSettingsPage = new gdGlobalDebugSettingsPage;

    afGlobalVariablesManager::instance().registerGlobalSettingsPage(pGlobalDebugSettingsPage);
}

void vsc_CreateSendErrorReportDialog(void* pVscInstance)
{
    GT_IF_WITH_ASSERT(pVscInstance != nullptr)
    {
        vscPackage* pInstance = (vscPackage*)pVscInstance;
        pInstance->CreateSendErrorReportDialog();
    }
}

void vsc_CreateDialogBasedAssertionFailureHandler(void* pVscInstance)
{
    GT_IF_WITH_ASSERT(pVscInstance != nullptr)
    {
        vscPackage* pInstance = (vscPackage*)pVscInstance;
        pInstance->CreateDialogBasedAssertionHandler();
    }
}

void vsc_PostSited_InitPackage(void* pVscInstance)
{

    static bool stat_callOnlyOnce = true;

    if (stat_callOnlyOnce)
    {
        // Initialize the application progress bar instance:
        GT_IF_WITH_ASSERT(pVscInstance != nullptr)
        {
            vscPackage* pInstance = (vscPackage*)pVscInstance;
            pInstance->InitProgressBar();
        }

        stat_callOnlyOnce = false;

        // Let the OSWrappers layer know that we are running in VS. The global variables manager
        // uses the OSWrappers layer so this info will propagate to it too:
        SetExecutedApplicationType(OS_VISUAL_STUDIO_PLUGIN_TYPE);

        // Create the API functions class and register it:
        vspGRApiFunctions::vspInstance();

        // Initialize the application command instance:
        vscApplicationCommands* pApplicationCommands = new vscApplicationCommands;

        // Register the application command single instance:
        bool rcRegsister = afApplicationCommands::registerInstance(pApplicationCommands);
        GT_ASSERT_EX(rcRegsister, L"Only single instance of gdApplicationCommands should be registered");

        // Register the application command single instance:
        rcRegsister = gdApplicationCommands::registerGDInstance(pApplicationCommands);
        GT_ASSERT_EX(rcRegsister, L"Only single instance of gdApplicationCommands should be registered");

        bool rcAPI;

        // Initialize the API package:
        rcAPI = gaIntializeAPIPackage(/*_shouldInitPerformanceCounters*/ false);

        // See GD_STR_LogMsg_FailedToInitAPI if it fails.
        assert(rcAPI);

        // Initialize the properties events handler:
        gdPropertiesEventObserver::instance();

        // Initialize the threads event handler:
        gdThreadsEventObserver::instance();

        // Register the modes
        gdExecutionMode* pDebuggerExecutionMode = new gdExecutionMode;


        // Initialize the profile windows manager instance:
        vspProfileWindowsManager::instance();

        // Initialize the KA execution mode:
        kaExecutionMode* pKAExecutionMode = new kaExecutionMode;

        afExecutionModeManager::instance().registerExecutionMode(pDebuggerExecutionMode);
        afExecutionModeManager::instance().registerExecutionMode(&SharedProfileManager::instance());
        afExecutionModeManager::instance().registerExecutionMode(pKAExecutionMode);

        afPluginConnectionManager::instance().registerRunModeManager(&SharedProfileManager::instance());

        // Set initial session type for the profile manager:
        SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_CPU_TIMER);

        // Initialize the debugger plug-in:
        vsc_InitDebuggerPlugin();

        afGlobalVariablesManager::SetProductIconID(AC_ICON_CODEXL_LOGO);

        // Initialize the AMTApplicationFramework library and its dependencies:
        // Do not initialize performance counters functionality:
        afInitializeApplicationCommand initAppCmd(VS_STR_CodeXL, VS_STR_VS);
        initAppCmd.execute();

        // Initialize the CPU Profiling component:
        AmdtCpuProfiling::instance().initializeStatic();

        //Initialize the GPU Profiling component
        GpuProfilerPlugin::Instance()->Initialize();

        vsc_InitKernelAnalyzerPlugin();

        vsc_InitPowerProfilingPlugin();
    }

    // Initialize event observer:
    GT_IF_WITH_ASSERT(pVscInstance != nullptr)
    {
        vscPackage* pInstance = (vscPackage*)pVscInstance;
        pInstance->m_pEventObserver = new vscAppEventObserver;
    }

}

bool vsc_IsDebuggedProcessExists()
{
    return gaDebuggedProcessExists();
}

void vsc_UpdateLaunchDebugAction(bool& isProcessExistsBuffer, bool& isProcessRunningBuffer)
{
    isProcessExistsBuffer = vsc_IsDebuggedProcessExists();
    isProcessRunningBuffer = isProcessExistsBuffer && (!gaIsDebuggedProcessSuspended());
}

void vsc_OnViewQuickStart()
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        pApplicationCommands->onHelpQuickStart();
    }

}

void vscGetHelpDevToolsSupportForumURL(wchar_t*& pBuffer)
{
    pBuffer = nullptr;
    gtString tmp(AF_STR_HelpDevToolsSupportForumURL);
    pBuffer = vscAllocateAndCopy(tmp);
}

void vsc_ValidateDriverAndGpu()
{
    gdApplicationCommands* pTheApplicationCommands = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pTheApplicationCommands != nullptr)
    {
        pTheApplicationCommands->validateDriverAndGPU();
    }
}

void vsc_GetProcessDllDirectoryFromCreationData(const wchar_t* exePath, const wchar_t*& o_dllDir)
{
    o_dllDir = nullptr;
    osFilePath exeFilePath(exePath);

    if (exeFilePath.exists())
    {
        bool is64 = false;
        bool rc64 = osIs64BitModule(exeFilePath, is64);

        if (rc64 && is64)
        {
            static gtString spies64Folder;

            if (spies64Folder.isEmpty())
            {
                osFilePath spies32;
                spies32.SetInstallRelatedPath(osFilePath::OS_CODEXL_SERVERS_PATH);
                osDirectory spiesDir;
                spies32.getFileDirectory(spiesDir);
                osFilePath spies64 = spiesDir.upOneLevel().asFilePath();
                spies64.appendSubDirectory(OS_SPIES_64_SUB_DIR_NAME).reinterpretAsDirectory();

                spies64Folder = spies64.asString();
            }

            o_dllDir = spies64Folder.asCharArray();
        }
    }
}

void vsc_InitQtApp()
{
#ifdef CXL_VS_EXT_DEVELOPMENT

    int argc = 0;
    char** argv = nullptr;

    // Take the Qt platform plug-in from the "platforms" directory which should be located
    // at the same directory where devenv.exe is located.
    vspQApplicationWrapper* pQApplication = new vspQApplicationWrapper(argc, argv);
    GT_UNREFERENCED_PARAMETER(pQApplication);

#else

    osFilePath extensionBinariesPath;
    bool rc1 = extensionBinariesPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_BINARIES_PATH);
    GT_ASSERT(rc1 && !extensionBinariesPath.asString().isEmpty());

    // This memory will be freed as the extension goes down (by OS).
    char* pExtensionFolder = vscAllocateAndCopy(extensionBinariesPath.asString().asASCIICharArray());

    char* argvFixed[] = { "CodeXL", "-platformpluginpath", pExtensionFolder, nullptr };
    int argcFixed = sizeof(argvFixed) / sizeof(char*) - 1;

    vspQApplicationWrapper* pQApplication = new vspQApplicationWrapper(argcFixed, argvFixed);
    pQApplication->installEventFilter(pQApplication);
    GT_UNREFERENCED_PARAMETER(pQApplication);

#endif // CXL_VS_EXT_DEVELOPMENT

}

void vsc_InitDebugTreeHandler()
{
    gdDebugApplicationTreeHandler::instance();
}

int vsc_GetObjectNavigationTreeId()
{
    return ID_OBJECT_NAVIGATION_TREE;
}

int vsc_GetPropertiesViewId()
{
    return ID_PROPERTIES_VIEW;
}

void vsc_ShowSaveListDialog(bool& isChangedFilesSaveRequired, bool& isContinueDebugRequired, bool& isBuildProjectRequired)
{
    isChangedFilesSaveRequired = false;
    isContinueDebugRequired = false;
    isBuildProjectRequired = false;
    vspSaveListDialog saveDialog(nullptr);

    if (saveDialog.hasItems())
    {
        vspWindowsManager::instance().showModal(&saveDialog);

        isBuildProjectRequired = saveDialog.userDecision();
        isContinueDebugRequired = saveDialog.result() != QDialog::Accepted;
        isChangedFilesSaveRequired = ((saveDialog.result() == QDialog::Accepted) && (saveDialog.userDecision() == true));
    }
}

void vsc_PreClosing(void* pVscInstance)
{
    GT_IF_WITH_ASSERT(pVscInstance != nullptr)
    {
        vscPackage* pInstance = (vscPackage*)pVscInstance;
        pInstance->PreClosing();
    }
}

int vsc_GetCallsHistoryListId()
{
    return ID_CALLS_HISTORY_LIST;
}

void vsc_OnCodeExplorerView()
{
    // Initialize the debug tree handler:
    gdDebugApplicationTreeHandler::instance();

    // Make sure the tree is in focus for keyboard events
    afApplicationTree* pObjectTree = vspWindowsManager::instance().monitoredObjectsTree(nullptr, QSize(0, 0));
    GT_IF_WITH_ASSERT(pObjectTree != nullptr)
    {
        pObjectTree->treeFocus();
    }
}

int vsc_GetMemoryAnalysisViewerId()
{
    return ID_MEMORY_ANALYSIS_VIEWER;
}

int vsc_GetStateVariablesViewId()
{
    return ID_STATE_VARIABLES_VIEW;
}

int vsc_GetStatisticsViewId()
{
    return ID_STATISTICS_VIEW;
}

int vsc_GetFirstMultiWatchViewId()
{
    return ID_MULTIWATCH_VIEW_FIRST;
}

void vsc_OpenMultiWatchViewId(int viewID, const wchar_t* variableName)
{
    // Get the view created:
    gdMultiWatchView* pMultiWatchView = vspWindowsManager::instance().multiwatchView(nullptr, QSize(-1, -1), viewID);
    GT_IF_WITH_ASSERT(pMultiWatchView != nullptr)
    {
        // Display the requested variable:
        pMultiWatchView->displayVariable(variableName);
    }
}

void vsc_OnModeClicked(int commandId)
{
    switch (commandId)
    {
        case cmdidDebugMode:
        {
            {
                afProjectManager::instance().setLastActiveMode(GD_STR_executionMode);
                apExecutionModeChangedEvent executionModeEvent(GD_STR_executionMode, 0);
                apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
            }

        }
        break;

        case cmdidProfileMode:
        case cmdidCodeXLProfileDropdownMenu:
        {
            afProjectManager::instance().setLastActiveMode(PM_STR_PROFILE_MODE);
            SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_PROFILE_MODE);

            // Get the last session type used for profiling:
            gtString lastSessionTypeStr = afExecutionModeManager::instance().lastSessionUsedForMode(PM_STR_PROFILE_MODE);
            apExecutionModeChangedEvent executionModeEvent(PM_STR_PROFILE_MODE, lastSessionTypeStr);
            apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
        }
        break;

        case cmdidFrameAnalysis:
        {
            afProjectManager::instance().setLastActiveMode(PM_STR_FrameAnalysisMode);
            apExecutionModeChangedEvent executionModeEvent(PM_STR_FrameAnalysisMode, 0);
            apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
        }
        break;

        case cmdidAnalyzeMode:
        {
            afProjectManager::instance().setLastActiveMode(KA_STR_executionMode);
            apExecutionModeChangedEvent executionModeEvent(KA_STR_executionMode, 0);
            apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
        }
        break;

        default:
        {
            // Unexpected value!
            assert(false);
        }
        break;
    }

    // Update the toolbar commands:
    afApplicationCommands::instance()->updateToolbarCommands();

}

void vsc_OpenBreakpointsDialog()
{
    // Show the dialog:
    gdApplicationCommands* pApplicationCommandInstance = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pApplicationCommandInstance != nullptr)
    {
        pApplicationCommandInstance->openBreakpointsDialog();
    }
}

bool vsc_OnUpdateOpenCLBreakpoints()
{
    return afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode);
}


bool vsc_OnUpdateMode_IsEnabled()
{
    afPluginConnectionManager& connectionManager = afPluginConnectionManager::instance();
    bool isEnabled = (connectionManager.getCurrentRunModeMask() == 0);
    return isEnabled;
}


bool vsc_OnUpdateMode_DebugMode_IsChecked()
{
    bool isChecked = false;

    if (!afProjectManager::instance().currentProjectSettings().lastActiveMode().isEmpty())
    {
        isChecked = (afProjectManager::instance().currentProjectSettings().lastActiveMode() == GD_STR_executionMode);
    }

    return isChecked;
}


void vsc_OnUpdateMode_DebugMode_Text(int maxBufSize, wchar_t*& pCmdNameBuffer, int& cmdNameStrLength)
{
    pCmdNameBuffer = nullptr;
    cmdNameStrLength = 0;
    gtString commandName = GD_STR_DebugMode;

    bool isInDebugMode = afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode);

    if (!isInDebugMode)
    {
        commandName = GD_STR_SwitchToDebugMode;
    }

    // Remove accelerator from string:
    commandName.removeChar('&');

    // Truncate the string to the buffer's size:
    int bufferSize = maxBufSize;

    if (bufferSize < commandName.length())
    {
        commandName.truncate(0, bufferSize - 1);
    }

    // Allocate output strings.
    pCmdNameBuffer = vscAllocateAndCopy(commandName);
    cmdNameStrLength = commandName.length();
}

bool vsc_OnUpdateMode_CXLProfileDropDownMenu_IsChecked()
{
    return (afProjectManager::instance().currentProjectSettings().lastActiveMode() == PM_STR_PROFILE_MODE);
}

void vsc_OnUpdateMode_CXLProfileDropDownMenu_CoreLogic(int maxBufSize, wchar_t*& pCmdNameBuffer, int& cmdNameStrLength)
{
    pCmdNameBuffer = nullptr;
    cmdNameStrLength = 0;

    bool isInProfileMode = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);

    gtString commandName = isInProfileMode ? PM_STR_PROFILE_MODE_MENU_COMMAND_PREFIX : PM_STR_SWITCH_TO_PROFILE_MODE_MENU_COMMAND_PREFIX;

    // Remove accelerator from string:
    commandName.removeChar('&');

    // Get the currently profile session type:
    gtString lastSessionName = SharedProfileManager::instance().currentSelection();

    if (!afProjectManager::instance().currentProjectFilePath().asString().isEmpty())
    {
        lastSessionName = afExecutionModeManager::instance().lastSessionUsedForMode(PM_STR_PROFILE_MODE);
    }

    commandName.append(lastSessionName);

    // Truncate the string to the buffer's size:
    int bufferSize = maxBufSize;

    if (bufferSize < commandName.length())
    {
        commandName.truncate(0, bufferSize - 1);
    }

    // Allocate output strings.
    pCmdNameBuffer = vscAllocateAndCopy(commandName);
    cmdNameStrLength = commandName.length();
}


bool vsc_IsFrameAnalysisModeSelected()
{
    bool retVal = (afProjectManager::instance().currentProjectSettings().lastActiveMode() == PM_STR_FrameAnalysisMode);
    return retVal;
}

bool vsc_OnUpdateMode_FrameAnalysisMode_IsChecked()
{
    bool isChecked = false;

    if (!afProjectManager::instance().currentProjectSettings().lastActiveMode().isEmpty())
    {
        isChecked = (afProjectManager::instance().currentProjectSettings().lastActiveMode() == PM_STR_FrameAnalysisMode);
    }

    return isChecked;
}

void vsc_OnUpdateMode_FrameAnalysis_Text(int maxBufSize, wchar_t*& pCmdNameBuffer, int& cmdNameStrLength)
{
    pCmdNameBuffer = nullptr;
    cmdNameStrLength = 0;
    gtString commandName = PM_STR_FrameAnalysisMode;

    bool isInDebugMode = afExecutionModeManager::instance().isActiveMode(PM_STR_FrameAnalysisMode);

    if (!isInDebugMode)
    {
        commandName = PM_STR_SwitchToFrameAnalysisMode;
    }

    // Remove accelerator from string:
    commandName.removeChar('&');

    // Truncate the string to the buffer's size:
    int bufferSize = maxBufSize;

    if (bufferSize < commandName.length())
    {
        commandName.truncate(0, bufferSize - 1);
    }

    // Allocate output strings.
    pCmdNameBuffer = vscAllocateAndCopy(commandName);
    cmdNameStrLength = commandName.length();
}


void vsc_OnUpdateMode_AnalyzeMode_Text(int maxBufSize, wchar_t*& pCmdNameBuffer, int& cmdNameStrLength)
{
    pCmdNameBuffer = nullptr;
    cmdNameStrLength = 0;
    gtString commandName = KA_STR_executionMode;

    bool isInKAMode = afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode);

    if (!isInKAMode)
    {
        commandName = KS_STR_SwitchToAnalyzeMode;
    }

    // Remove accelerator from string:
    commandName.removeChar('&');

    // Truncate the string to the buffer's size:
    int bufferSize = maxBufSize;

    if (bufferSize < commandName.length())
    {
        commandName.truncate(0, bufferSize - 1);
    }

    // Allocate output strings.
    pCmdNameBuffer = vscAllocateAndCopy(commandName);
    cmdNameStrLength = commandName.length();
}

bool vsc_OnUpdateMode_AnalyzeMode_IsChecked()
{
    bool isChecked = false;

    if (!afProjectManager::instance().currentProjectSettings().lastActiveMode().isEmpty())
    {
        isChecked = (afProjectManager::instance().currentProjectSettings().lastActiveMode() == KA_STR_executionMode);
    }

    return isChecked;
}

void vsc_OnLaunchOpenCLDebugging()
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();
        GT_IF_WITH_ASSERT(pApplicationTree != nullptr)
        {
            pApplicationTree->updateTreeRootText();
        }
    }
}

void vsc_GetCurrentModeStr(wchar_t*& pBuf)
{
    // Get the current mode (or debug if the project is not loaded yet):
    gtString currentMode = afProjectManager::instance().currentProjectSettings().lastActiveMode();
    pBuf = vscAllocateAndCopy(currentMode);
}

const wchar_t* vsc_GetProfileModeStr()
{
    return PM_STR_PROFILE_MODE;
}

const wchar_t* vsc_GetExecutionModeStr()
{
    return GD_STR_executionMode;
}

const wchar_t* vsc_GetFrameAnalysisModeStr()
{
    return PM_STR_FrameAnalysisMode;
}

void vsc_OnUpdateStepInfo(bool& isStepIntoEnabled, bool& shouldShow)
{
    isStepIntoEnabled = false;
    shouldShow = false;

    // Check if debugged process exists:
    bool doesProcessExists = gaDebuggedProcessExists();

    // Check if debugged process is suspended:
    bool isProcessSuspended = gaIsDebuggedProcessSuspended();

    shouldShow = afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode);

    if (doesProcessExists && isProcessSuspended)
    {
        // Check if we're in kernel debugging session:
        isStepIntoEnabled = gaIsInKernelDebugging() || gaCanGetHostDebugging();

        if (!isStepIntoEnabled)
        {
            // Get the current triggering context id:
            apContextID contextID;
            bool rc = gaGetBreakpointTriggeringContextId(contextID);
            GT_IF_WITH_ASSERT(rc)
            {
                // Get the amount of function calls for the triggering context:
                int amountOfFunctions = 0;
                rc = gaGetAmountOfCurrentFrameFunctionCalls(contextID, amountOfFunctions);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Get the last function call for the current context:
                    gtAutoPtr<apFunctionCall> aptrFunctionCall;
                    rc = gaGetCurrentFrameFunctionCall(contextID, amountOfFunctions - 1, aptrFunctionCall);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        apMonitoredFunctionId funcId = aptrFunctionCall->functionId();

                        if ((ap_clEnqueueNDRangeKernel == funcId) || (ap_clEnqueueTask == funcId))
                        {
                            // When the breakpoint is in clEnqueueNDRangeKernel, enable step into:
                            isStepIntoEnabled = true;
                        }
                    }
                }
            }
        }
    }
}
void vsc_OnUpdateStepOut(bool& isStepOutEnabled, bool& shouldShow)
{
    isStepOutEnabled = shouldShow = false;

    // Check if debugged process exists:
    bool doesProcessExists = gaDebuggedProcessExists();

    // Check if debugged process is suspended:
    bool isProcessSuspended = gaIsDebuggedProcessSuspended();

    shouldShow = afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode);

    if (doesProcessExists && isProcessSuspended)
    {
        // Check if we're in kernel debugging session:
        isStepOutEnabled = gaIsInKernelDebugging() || gaCanGetHostDebugging();
    }
}

bool vsc_OnFrameStep_OnUpdateMode_IsResumeDebuggingRequired()
{
    // Set the break on next frame flag:
    gaBreakOnNextFrame();
    return (gaDebuggedProcessExists());
}

bool vsc_OnAPIStep_OnUpdateMode_IsResumeDebuggingRequired()
{
    // Set the break on next API call flag:
    gaBreakOnNextMonitoredFunctionCall();
    return (gaDebuggedProcessExists());
}

bool vsc_OnFrameStep_OnUpdateMode_IsDebuggedProcExists()
{
    // Set the break on next frame flag:
    gaBreakOnNextDrawFunctionCall();
    return (gaDebuggedProcessExists());
}

void vsc_OnUpdateFrameStep(bool& isProcessSuspended, bool& isProcessExists, bool& shouldShow)
{
    isProcessSuspended = gaIsDebuggedProcessSuspended();
    isProcessExists = gaDebuggedProcessExists();
    shouldShow = afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode);
}

void vsc_OnUpdateStepOver(bool& isProcessSuspended, bool& isProcessExists, bool& shouldShow)
{
    isProcessExists = gaDebuggedProcessExists();
    isProcessSuspended = gaIsDebuggedProcessSuspended();
    shouldShow = afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode);
}

WCHAR* CCodeXLVSPackagePackage_GetVSRegistryRootPath(bool tempRegPath, bool& shouldDeallocate)
{
    shouldDeallocate = false;

    WCHAR* retVal = nullptr;

    // Try to calculate the value from our location:
    osFilePath binariesPath;
    bool rcPth = binariesPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_BINARIES_PATH);

    if (rcPth)
    {
        // The binaries path should be "C:\Users\<username>\AppData\Local\Microsoft\VisualStudio\<VS version number [+ suffix]>\Extensions\AMD\CodeXL\<CodeXL version>\"
        // So, we want to go up 4 levels, then take the last directory's name:
        osDirectory vsVersionDirectory;
        binariesPath.getFileDirectory(vsVersionDirectory);
        vsVersionDirectory.upOneLevel().upOneLevel().upOneLevel().upOneLevel();

        // This is static so we could return its asCharArray() as a pointer:
        static gtString vsVersion;
        vsVersion = vsVersionDirectory.directoryPath().fileDirectoryAsString();
        int lastPathSeparatorLocation = vsVersion.removeTrailing(osFilePath::osPathSeparator).reverseFind(osFilePath::osPathSeparator);
        vsVersion.truncate(lastPathSeparatorLocation + 1, -1);

        // Verify this starts with a number and has a period:
        if ((vsVersion[0] <= '9') && (vsVersion[0] >= '0') && (vsVersion.find('.') > -1))
        {
            vsVersion.prepend(VSP_STR_VisualStudioRegistryRootPathPrefix);

            if (tempRegPath)
            {
                vsVersion.append(VSP_STR_VisualStudioRegistryRootConfigSuffix);
            }

            retVal = vscAllocateAndCopy(vsVersion.asCharArray());
            shouldDeallocate = true;
        }
    }

    return retVal;
}

void CCodeXLVSPackagePackage_ValidateStartDebugAfterBuild(wchar_t** pProjectNamesArr, int arrSize, bool& isBuildRequired, bool& isDebugContinueRequired)
{
    GT_IF_WITH_ASSERT(pProjectNamesArr != nullptr)
    {
        GT_IF_WITH_ASSERT(arrSize > 0)
        {
            gtVector<gtString> vec(arrSize);

            for (int i = 0; i < arrSize; i++)
            {
                gtString tmp((pProjectNamesArr[i] != nullptr) ? pProjectNamesArr[i] : L"");
                vec.push_back(tmp);
            }

            vscBuildListDialog buildDialog(nullptr, vec);
            vspWindowsManager::instance().showModal(&buildDialog);
            isBuildRequired = buildDialog.shouldBuild();
            isDebugContinueRequired = (buildDialog.result() == QDialog::Accepted);
        }
    }
}

void vsc_PostSited_InitAsDebugger()
{
    afProjectManager::instance().setLastActiveMode(GD_STR_executionMode);
}

int vsc_GetCommandQueuesViewerId()
{
    return ID_COMMAND_QUEUES_VIEWER;
}

int vsc_GetSecondMultiWatchViewId()
{
    return ID_MULTIWATCH_VIEW2;
}

int vsc_GetThirdMultiWatchViewId()
{
    return ID_MULTIWATCH_VIEW3;
}

int vsc_GetShadersSourceCodeViewerHtmlWindowId()
{
    return ID_SHADERS_SOURCE_CODE_VIEWER_HTML_WINDOW;
}

void vsc_OnNewDebugEngine(void* pVscInstance, void* pNewDebugEngine)
{
    vscPackage* pInstance = (vscPackage*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != nullptr)
    {
        pInstance->OnNewDebugEngine((vspCDebugEngine*)pNewDebugEngine);
    }
}

bool vsc_IsAnyDebugEngineAvailable()
{
    return vspCDebugEngine::areThereDebugEngines();
}

void vsc_PostSited_InitBreakpointsManager(void* pDebugger)
{
    vscBreakpointsManager::instance().setDebuggerInterface((IVsDebugger*)pDebugger);
}

int vsc_OnAddKernelMultiWatchFromSourceCode()
{
    // Get the index for the first hidden multi watch view:
    return vspWindowsManager::instance().findFirstHiddenMultiWatchViewIndex();
}

int vsc_OnAddKernelMultiWatchFromLocalsView()
{
    return vspWindowsManager::instance().findFirstHiddenMultiWatchViewIndex();
}

int vsc_OnAddKernelMultiWatchFromWatchView()
{
    return vspWindowsManager::instance().findFirstHiddenMultiWatchViewIndex();
}

void vsc_OpenTeapotExample()
{
    // Open the solution
    vspWindowsManager::instance().OpenSample(AF_TEAPOT_SAMPLE);
}

void vsc_OpenD3D12MultithreadingExample()
{
    // Open the solution
    vspWindowsManager::instance().OpenSample(AF_D3D12MULTITHREADING_SAMPLE);
}

void vsc_OpenMatMulExample()
{
    // Open the solution
    vspWindowsManager::instance().OpenSample(AF_MATMUL_SAMPLE);
}

void vsc_ViewHelp()
{
    vspWindowsManager::instance().viewHelp();
}

void vsc_ShowCheckForUpdateDialog()
{
    vspWindowsManager::instance().showCheckForUpdateDialog();
}

void vsc_ShowAboutDialog()
{
    vspWindowsManager::instance().showAboutDialog();
}

REFGUID vsc_GetDebugEngineGuid()
{
    return __uuidof(vspDebugEngine);
}

void vsc_SetAppMessageBoxIcon()
{
    QPixmap* pPixmap = new QPixmap;
    acSetIconInPixmap(*pPixmap, AC_ICON_CODEXL_LOGO, AC_64x64_ICON);
    acMessageBox::setApplicationIconPixmap(pPixmap);
}

//////////////////////////////////////////////////////////////////////////
// This internal class is needed in the below function:
//////////////////////////////////////////////////////////////////////////
MIDL_INTERFACE("10E4254C-6D73-4C38-B011-E0049B2E0A0F")
IVsLoader : public IDispatch
{
    STDMETHOD(Load)(BSTR strModule, const GUID * rclsid, IUnknown * UnkOuter, unsigned long dwClsContext, IUnknown** ppunk);
};

// ---------------------------------------------------------------------------
// Name:        vscCLSIDToIUnknownWithIVsLoader
//              (CCodeXLVSPackagePackage::clsidToIUnknownWithIVsLoader) helper function
// Description: The native engine and related interfaces are not normally exposed
//              through CoCreateInstance, so we have to look at the local version
//              of the registry "CLSID" key and get the interfaces from there.
// Author:      Uri Shomroni
// Date:        28/12/2011
// ---------------------------------------------------------------------------
void vscCLSIDToIUnknownWithIVsLoader(const wchar_t* vsRegistryRootPath, const GUID& classId, IUnknown*& rpiLoadedUnkown)
{
    rpiLoadedUnkown = NULL;

    gtString registryLocation = vsRegistryRootPath;
    OLECHAR CLSIDAsString[40] = { 0 }; // GUIDs are 39 characters long
    ::StringFromGUID2(classId, CLSIDAsString, 39);
    registryLocation.append(L"\\CLSID\\").append(CLSIDAsString);

    // Get the value:
    CRegKey key;
    WCHAR moduleName[256] = { 0 };

    // Search in HKCU:
    LSTATUS lRes = key.Open(HKEY_CURRENT_USER, registryLocation.asCharArray());

    if (ERROR_SUCCESS != lRes)
    {
        // Search in HKLM:
        lRes = key.Open(HKEY_LOCAL_MACHINE, registryLocation.asCharArray());

        if (ERROR_SUCCESS != lRes)
        {
            // Search in HKCR:
            lRes = key.Open(HKEY_CLASSES_ROOT, registryLocation.asCharArray());
        }
    }

    if (ERROR_SUCCESS == lRes)
    {
        DWORD n = 255;
        lRes = key.QueryStringValue(L"InprocServer32", moduleName, &n);

        if (ERROR_SUCCESS == lRes)
        {
            // Get the Visual Studio Apartment loader:
            GUID CLSID_VsApartmentLoader = { 0xCA554A15, 0x4410, 0x45C9, { 0xB5, 0xC1, 0x20, 0xDE, 0x05, 0x2D, 0x9C, 0xD3 } };
            CComPtr<IVsLoader> pLoader;
            pLoader.CoCreateInstance(CLSID_VsApartmentLoader);

            if (pLoader != NULL)
            {
                // Load the Unknown:
                HRESULT hr = pLoader->Load(moduleName, &classId, 0, CLSCTX_ALL, &rpiLoadedUnkown);
                GT_ASSERT(SUCCEEDED(hr) && (rpiLoadedUnkown != NULL));
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        vscLoadDebugEngineByGUID
// Description: Uses the AD7Metrics to determine an engine's CLSID from its GUID
//              and loads it with the VS loader
// Author:      Uri Shomroni
// Date:        2/8/2016
// ---------------------------------------------------------------------------
bool vscLoadDebugEngineByGUID(const wchar_t* vsRegistryRootPath, const wchar_t* vsTempRegistryRootPath, const GUID& engineGUID, IUnknown*& rpiDebugEngineAsUnknown)
{
    // Use this only if something is not working with the registry:
// #define VSC_USE_DEBUG_ENGINE_STATIC_CLSIDS 1
#ifndef VSC_USE_DEBUG_ENGINE_STATIC_CLSIDS
    CLSID CLSID_FreeUsedEngine = { 0x3B476D36, 0xA401, 0x11D2,{ 0xAA, 0xD4, 0x00, 0xC0, 0x4F, 0x99, 0x01, 0x71 } }; // Default to the native engine.

    HRESULT hr = ::GetMetric(nullptr, metrictypeEngine, engineGUID, metricCLSID, &CLSID_FreeUsedEngine, (const unsigned short*)vsTempRegistryRootPath);
    // TO_DO: Might need both root paths to be passed - but we don't expect the native engine to be temporary...
    if (!SUCCEEDED(hr))
    {
        hr = ::GetMetric(nullptr, metrictypeEngine, engineGUID, metricCLSID, &CLSID_FreeUsedEngine, (const unsigned short*)vsRegistryRootPath);
        if (!SUCCEEDED(hr))
        {
            GT_ASSERT_EX(false, L"Failed to find debug engine CLSID. Using native engine CLSID by default");
        }
    }


#else // defined VSC_USE_DEBUG_ENGINE_STATIC_CLSIDS
#define VSC_EQUAL_GUID(g1, g2) ((g1.Data1    == g2.Data1)    && (g1.Data2    == g2.Data2)    && (g1.Data3    == g2.Data3)    && (g1.Data4[0] == g2.Data4[0]) && \
                                (g1.Data4[1] == g2.Data4[1]) && (g1.Data4[2] == g2.Data4[2]) && (g1.Data4[3] == g2.Data4[3]) && (g1.Data4[4] == g2.Data4[4]) && \
                                (g1.Data4[5] == g2.Data4[5]) && (g1.Data4[6] == g2.Data4[6]) && (g1.Data4[7] == g2.Data4[7]))

    //////////////////////////////////////////////////////////////////////////
    // See http://romanmarusyk.livejournal.com/5046.html and http://social.msdn.microsoft.com/Forums/ar/vsx/thread/3b75164e-31c4-49c8-923f-2b91be49080a:
    //////////////////////////////////////////////////////////////////////////

    // Create the string for where the debug engine's dll is saved:
    // The CLSID of a debug engine can be found in the registry at
    // HKCU\Software\Microsoft\VisualStudio\X.0[_Config]\AD7Metrics\Engine\{Engine GUID} value "CLSID"
    // The values are:
    // Engine           Engine GUID                             Engine CLSID
    // CodeXL           {D0B58E07-BEA0-44BB-AB51-458FFEA38665}  {D0B58E07-BEA0-44BB-AB51-458FFEA38665}
    // Native Only      {3B476D35-A401-11D2-AAD4-00C04F990171}  {3B476D36-A401-11D2-AAD4-00C04F990171} *
    // New Native Only  {3B476D35-A401-11D2-AAD4-00C04F990171}  {9FD54F8A-66FA-47df-AEA7-CF486A791E54} *
    // Native + GPU     {F4453496-1DB8-47F8-A7D5-31EBDDC2EC96}  {014D0C8F-3FBD-4A8F-A6F1-14190DD7C090}
    // COM Plus Native  {92EF0900-2251-11D2-B72E-0000F87572EF}  {A8403605-9923-4605-BFBA-F47A4739AB43}
    //
    // * The old (natdbgde) and new (concord) native-only debug engines are differentiated by a compatibility
    //   flag in the SDM. Otherwise, they share the same engine ID.
    GUID CLSID_FreeNativeOnlyEngine = { 0x3B476D36, 0xA401, 0x11D2,{ 0xAA, 0xD4, 0x00, 0xC0, 0x4F, 0x99, 0x01, 0x71 } };
    GUID CLSID_FreeNativeAndGpuEngine = { 0x014D0C8F, 0x3FBD, 0x4A8F,{ 0xA6, 0xF1, 0x14, 0x19, 0x0D, 0xD7, 0xC0, 0x90 } };
    GUID CLSID_FreeNewNativeOnlyEngine = { 0x9FD54F8A, 0x66FA, 0x47DF,{ 0xAE, 0xA7, 0xCF, 0x48, 0x6A, 0x79, 0x1E, 0x54 } };
    GUID CLSID_FreeComPlusNativeEngine = { 0xA8403605, 0x9923, 0x4605,{ 0xBF, 0xBA, 0xF4, 0x7A, 0x47, 0x39, 0xAB, 0x43 } };

    // This value is copied rather than taken from msdbg110.h, to avoid a dependency on the VS11 libraries when running in VS10:
    GUID guidConcordGpuEng = { 0xF4453496, 0x1DB8, 0x47F8,{ 0xA7, 0xD5, 0x31, 0xEB, 0xDD, 0xC2, 0xEC, 0x96 } };

    GUID& CLSID_FreeUsedEngine = (VSC_EQUAL_GUID(guidConcordGpuEng, engineGUID) ? CLSID_FreeNativeAndGpuEngine :
                                 (VSC_EQUAL_GUID(guidNativeOnlyEng, engineGUID) ? CLSID_FreeNativeOnlyEngine :
                                 (VSC_EQUAL_GUID(guidCOMPlusNativeEng, engineGUID) ? CLSID_FreeComPlusNativeEngine : 
                                  CLSID_FreeNativeOnlyEngine))); // Default to the native value 
#undef VSC_EQUAL_GUID

#endif // VSC_USE_DEBUG_ENGINE_STATIC_CLSIDS

    vscCLSIDToIUnknownWithIVsLoader(vsTempRegistryRootPath, CLSID_FreeUsedEngine, rpiDebugEngineAsUnknown);

    if (nullptr == rpiDebugEngineAsUnknown)
    {
        vscCLSIDToIUnknownWithIVsLoader(vsRegistryRootPath, CLSID_FreeUsedEngine, rpiDebugEngineAsUnknown);
    }

    return (nullptr != rpiDebugEngineAsUnknown);
}

// ---------------------------------------------------------------------------
// Name:        vsc_InitPackageDebugger
//              (CCodeXLVSPackagePackage::sendParametersToVSPackageCode)
// Description: Gets the details needed to initialize the gDEBuggerVSPackageCode
//              project, and send it to them
// Author:      Uri Shomroni
// Date:        22/12/2011
// ---------------------------------------------------------------------------
void vsc_InitPackageDebugger(const wchar_t* vsRegistryRootPath, const wchar_t* vsTempRegistryRootPath)
{
    bool useNativeOnly = true;
    GUID& guidUsedEngine = useNativeOnly ? guidNativeOnlyEng : guidCOMPlusNativeEng;

    IUnknown* piDebugEngineAsUnknown = nullptr;
    bool rcEng = vscLoadDebugEngineByGUID(vsRegistryRootPath, vsTempRegistryRootPath, guidUsedEngine, piDebugEngineAsUnknown);
    GT_ASSERT(rcEng);

    GT_IF_WITH_ASSERT(nullptr != piDebugEngineAsUnknown)
    {
        // Query for the IDebugEngine2 interface:
        IDebugEngine2* piDebugEngine = nullptr;
        HRESULT hr = piDebugEngineAsUnknown->QueryInterface(IID_IDebugEngine2, (void**)&piDebugEngine);

        if (SUCCEEDED(hr) && (nullptr != piDebugEngine))
        {
            // Get the program provider's class ID (see http://msdn.microsoft.com/en-us/library/bb161298.aspx):
            CLSID clsidProvider = { 0 };
            HRESULT hr = ::GetMetric(nullptr, metrictypeEngine, guidUsedEngine, metricProgramProvider, &clsidProvider, (const unsigned short*)vsTempRegistryRootPath);

            // TO_DO: Might need both root paths to be passed - but we don't expect the native engine to be temporary...
            if (!SUCCEEDED(hr))
            {
                hr = ::GetMetric(nullptr, metrictypeEngine, guidUsedEngine, metricProgramProvider, &clsidProvider, (const unsigned short*)vsRegistryRootPath);
            }

            if (SUCCEEDED(hr) && (!IsEqualGUID(clsidProvider, GUID_NULL)))
            {
                // Get the native program provider (we can't just CoCreateInstance here, as it is not outwardly registered):
                IUnknown* piProgramProviderAsUnknown = nullptr;
                vscCLSIDToIUnknownWithIVsLoader(vsTempRegistryRootPath, clsidProvider, piProgramProviderAsUnknown);

                if (nullptr == piProgramProviderAsUnknown)
                {
                    vscCLSIDToIUnknownWithIVsLoader(vsRegistryRootPath, clsidProvider, piProgramProviderAsUnknown);
                }

                GT_IF_WITH_ASSERT(nullptr != piProgramProviderAsUnknown)
                {
                    // Query for the program provider interface:
                    IDebugProgramProvider2* piProgramProvider = nullptr;
                    hr = piProgramProviderAsUnknown->QueryInterface(IID_IDebugProgramProvider2, (void**)(&piProgramProvider));

                    if (SUCCEEDED(hr) && (nullptr != piProgramProvider))
                    {
                        // Pass it to the CodeXLVSPackageDebugger dll:
                        vsdPackageConnector::instance().initializeWithPackage(piDebugEngine, piProgramProvider);

                        // Release the program provider interface:
                        piProgramProvider->Release();
                    }

                    // Release the unknown interface:
                    piProgramProviderAsUnknown->Release();
                }
            }

            // Release the interface:
            piDebugEngine->Release();
        }

        // Release the IUnknown interface:
        piDebugEngineAsUnknown->Release();
    }
}

void vsc_InitAppEventObserver(void* pVscInstance)
{
    // Initialize event observer:
    GT_IF_WITH_ASSERT(pVscInstance != nullptr)
    {
        vscPackage* pInstance = (vscPackage*)pVscInstance;
        pInstance->m_pEventObserver = new vscAppEventObserver;
    }
}

void vsc_SetWindowsStoreAppUserModelID(const wchar_t* appUserModelId)
{
    afProjectManager::instance().setWindowsStoreAppUserModelID(appUserModelId);
}

const wchar_t* vsc_Get_GD_STR_executionMode()
{
    return GD_STR_executionMode;
}

void vsc_InitGuiComponents()
{
    afProgressBarWrapper::instance().initialize(nullptr);

    // This solves the BUG449461, which required the user to open
    // CodeXL->Help->About in order for the CodeXL GUI components to activate.
    vspSaveListDialog saveDialog(nullptr);
    vspWindowsManager::instance().showModal(&saveDialog, false);
}

unsigned int vsc_GetInstalledComponentsBitmask()
{
    unsigned int retVal = afGlobalVariablesManager::instance().InstalledAMDComponentsBitmask();
    gtString errorMsg = afGlobalVariablesManager::instance().InstalledAMDComponentsErrorMessage();

    if (!errorMsg.isEmpty())
    {
        QString errorStrAsQt = acGTStringToQString(errorMsg);
        afApplicationCommands::instance()->AddStringToInformationView(errorStrAsQt);
    }

    return retVal;
}

void vsc_OnRemoteHost(const wchar_t* pRemoteHostText)
{
    // Set the current remote host settings:
    apProjectSettings currentSettings = afProjectManager::instance().currentProjectSettings();
    currentSettings.SetRemoteTargetHostname(pRemoteHostText);
    afProjectManager::instance().setCurrentProject(currentSettings);
}

void vsc_OnStartButton()
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        // Open the remote host settings edit dialog:
        pApplicationCommands->OnProjectSettingsEditRemoteHost();
    }
}

void vsc_OnConfigureRemoteHost()
{
    afApplicationCommands::instance()->OnProjectSettingsEditRemoteHost();
    afApplicationCommands::instance()->updateToolbarCommands();
}

void vsc_OnUpdateConfigureRemoteHost(bool& isActionEnabled)
{
    isActionEnabled = afExecutionModeManager::instance().IsRemoteHostEnabled();
}

void vsc_OnUpdateStartButtonIsEnabled(bool& isActionEnabled)
{
    GT_UNREFERENCED_PARAMETER(isActionEnabled);
}

