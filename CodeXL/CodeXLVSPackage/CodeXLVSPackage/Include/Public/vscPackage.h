//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscPackage.h
///
//==================================================================================

#ifndef vscPackage_h__
#define vscPackage_h__
#include "CodeXLVSPackageCoreDefs.h"

void* vsc_CreateInstance();

void vsc_DestroyInstance(void*& pInstance);

void vsc_UpdateProjectSettingsFromVS(const wchar_t* executableFilePath, const wchar_t* workingDirPath, const wchar_t* cmdLineArgs, const wchar_t* env, bool& isDebuggingRequired, bool& isProfilingRequired, bool& isFrameAnalysisRequired);

bool vsc_ValidateDebugSettings(const wchar_t* executableFilePath, const wchar_t* workingDirPath, bool isProjectTypeValid);

void vsc_OnUpdateLaunchProfileAction(bool& isActionEnabled, bool& isActionChecked);

void vsc_OnUpdateLaunchFrameAnalysisAction(bool& isActionEnabled, bool& isActionChecked);

bool vsc_CanStopCurrentRun();

bool vsc_IsInRunningMode();

void vsc_CreateDialogBasedAssertionFailureHandler(void* pVscInstance);

void vsc_PostSited_InitPackage(void* pVscInstance);

void vsc_PostSited_InitAsDebugger();

void vsc_PostSited_InitBreakpointsManager(void* pDebugger);

void vsc_InitQtApp();

void vsc_InitDebugTreeHandler();

int vsc_OnAddKernelMultiWatchFromSourceCode();

int vsc_OnAddKernelMultiWatchFromLocalsView();

int vsc_OnAddKernelMultiWatchFromWatchView();

void vsc_OnViewQuickStart();

void vsc_OnModeClicked(int commandId);

void vsc_OpenBreakpointsDialog();

bool vsc_OnUpdateOpenCLBreakpoints();

void vsc_PreClosing(void* pVscInstance);

int vsc_GetObjectNavigationTreeId();

int vsc_GetPropertiesViewId();

int vsc_GetMemoryAnalysisViewerId();

int vsc_GetStateVariablesViewId();

int vsc_GetStatisticsViewId();

int vsc_GetFirstMultiWatchViewId();

int vsc_GetCommandQueuesViewerId();

int vsc_GetSecondMultiWatchViewId();

int vsc_GetThirdMultiWatchViewId();

int vsc_GetShadersSourceCodeViewerHtmlWindowId();

void vsc_OpenMultiWatchViewId(int viewID, const wchar_t* variableName);

void vsc_ShowSaveListDialog(bool& isChangedFilesSaveRequired, bool& isContinueDebugRequired, bool& isBuildProjectRequired);

bool vsc_IsDebuggedProcessExists();

void vsc_ValidateDriverAndGpu();

void vsc_GetProcessDllDirectoryFromCreationData(const wchar_t* exePath, const wchar_t*& o_dllDir);

void vsc_UpdateLaunchDebugAction(bool& isProcessExistsBuffer, bool& isProcessRunningBuffer);

void vsc_InitKernelAnalyzerPlugin();

void vsc_InitDebuggerPlugin();

void vsc_InitPowerProfilingPlugin();

void vsc_CreateSendErrorReportDialog(void* pVscInstance);

void vsc_ExecuteProfileSession();

void vsc_ExecuteFrameAnalysisSession();
void vsc_RefreshFrameAnalysisSessionsFromServer();

void vsc_DisplayDebugProcessLaunchFailureMessage(HRESULT failureHR);

void vsc_OpenMatMulExample();
void vsc_OpenD3D12MultithreadingExample();
void vsc_OpenTeapotExample();

void vscGetHelpDevToolsSupportForumURL(wchar_t*& pBuffer);

void vsc_GetCurrentSettings(wchar_t*& pExecutableFilePathBuffer, wchar_t*& pWorkingDirectoryPath, wchar_t*& pCommandLineArguments, wchar_t*& pEnvironment);

void vsc_CheckDebuggerInstallation();

int vsc_GetCallsHistoryListId();

void vsc_OnCodeExplorerView();

bool vsc_OnUpdateMode_IsEnabled();

bool vsc_OnUpdateMode_DebugMode_IsChecked();
void vsc_OnUpdateMode_DebugMode_Text(int maxBufSize, wchar_t*& pCmdNameBuffer, int& cmdNameStrLength);

bool vsc_OnUpdateMode_CXLProfileDropDownMenu_IsChecked();
void vsc_OnUpdateMode_CXLProfileDropDownMenu_CoreLogic(int maxBufSize, wchar_t*& pCmdNameBuffer, int& cmdNameStrLength);

bool vsc_IsFrameAnalysisModeSelected();
bool vsc_OnUpdateMode_FrameAnalysisMode_IsChecked();
void vsc_OnUpdateMode_FrameAnalysis_Text(int maxBufSize, wchar_t*& pCmdNameBuffer, int& cmdNameStrLength);

bool vsc_OnUpdateMode_AnalyzeMode_IsChecked();
void vsc_OnUpdateMode_AnalyzeMode_Text(int maxBufSize, wchar_t*& pCmdNameBuffer, int& cmdNameStrLength);

void vsc_OnLaunchOpenCLDebugging();

void vsc_GetCurrentModeStr(wchar_t*& pBuf);

const wchar_t* vsc_GetProfileModeStr();

const wchar_t* vsc_GetExecutionModeStr();

const wchar_t* vsc_GetFrameAnalysisModeStr();

void vsc_OnUpdateStepInfo(bool& isStepIntoEnabled, bool& shouldShow);

bool vsc_OnFrameStep_OnUpdateMode_IsResumeDebuggingRequired();

bool vsc_OnAPIStep_OnUpdateMode_IsResumeDebuggingRequired();

bool vsc_OnFrameStep_OnUpdateMode_IsDebuggedProcExists();

void vsc_OnUpdateFrameStep(bool& isProcessSuspended, bool& isProcessExists, bool& shouldShow);

void vsc_OnUpdateStepOver(bool& isProcessSuspended, bool& isProcessExists, bool& shouldShow);

void vsc_OnUpdateStepInfo(bool& isStepIntoEnabled, bool& shouldShow);
void vsc_OnUpdateStepOut(bool& isStepOutEnabled, bool& shouldShow);

WCHAR* CCodeXLVSPackagePackage_GetVSRegistryRootPath(bool tempRegPath, bool& shouldDeallocate);

void CCodeXLVSPackagePackage_ValidateStartDebugAfterBuild(wchar_t** pProjectNamesArr, int arrSize, bool& isBuildRequired, bool& isDebugContinueRequired);

void vsc_OnNewDebugEngine(void* pVscInstance, void* pNewDebugEngine);

bool vsc_IsAnyDebugEngineAvailable();

void vsc_ViewHelp();

void vsc_ShowCheckForUpdateDialog();

void vsc_ShowAboutDialog();

REFGUID vsc_GetDebugEngineGuid();

void vsc_SetAppMessageBoxIcon();

void vsc_SetWindowsStoreAppUserModelID(const wchar_t* appUserModelId);

void vsc_InitPackageDebugger(const wchar_t* vsRegistryRootPath, const wchar_t* vsTempRegistryRootPath);

void vsc_InitAppEventObserver(void* pVscInstance);

const wchar_t* vsc_Get_GD_STR_executionMode();

void vsc_InitGuiComponents();

unsigned int vsc_GetInstalledComponentsBitmask();

void vsc_OnConfigureRemoteHost();

void vsc_OnStartButton();

void vsc_OnUpdateConfigureRemoteHost(bool& isActionEnabled);
void vsc_OnUpdateStartButtonGetText(int maxBufSize, wchar_t*& pCmdNameBuffer, int& cmdNameStrLength);

#endif // vscPackage_h__
