//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IVscCoreAPI.h
///
//==================================================================================

#ifndef __IVSCCOREAPI_H
#define __IVSCCOREAPI_H

// Local:
#include "CodeXLVSPackageCoreDefs.h"

enum vscPackageCommandHandlerModes
{
    vsDebugMode = 0,
    vsProfileMode,
    vsKernelAnalyzeMode,
    vsFrameAnalyisMode
};

class IProfileSessionEditorDocVsCoreImplOwner;
class IVscTimerOwner;
class IDteTreeEventHandler;
class IVspDTEConnector;
class IVscApplicationCommandsOwner;
class IVscSourceCodeViewerOwner;
class IProgressBarEventHandler;
class IVscWindowsManagerOwner;
class IVscEventObserverOwner;
class IVscBreakpointsManagerOwner;
class IVscDebugEngineOwner;
class IVscGRApiFunctionsOwner;
struct IVsUIShell;

struct IVscCoreAPI
{
    // vspDLLInit.cpp
    BOOL (APIENTRY* vspDllMain)(DWORD reason, LPVOID reserved, HINSTANCE hInstance);
    void (*vscSetApplicationBinariesFolder)(const char* pathStr);

    // vscCoreUtils
    void (*vscDeleteWcharString)(wchar_t*& pStr);
    void (*vscDeleteWcharStringArray)(wchar_t**& pStr);
    void (*vscDeleteCharString)(char*& pStr);
    void (*vscDeleteUintBuffer)(unsigned int*& pBuffer);
    bool (*vscIsPathStringsEqual)(const wchar_t* pathStrA, const wchar_t* pathStrB);
    void (*vscExtractFileExtension)(const wchar_t* filePathStr, wchar_t*& pExtensionStrBuffer);
    bool (*vscIsPathExists)(const wchar_t* pathStr);
    void (*vscGetFileDirectoryAsString)(const wchar_t* pathStr, wchar_t*& pDirStrBuffer);
    void (*vscGetFileName)(const wchar_t* pathStr, wchar_t*& pFileNameStrBuffer);
    wchar_t (*vscGetOsPathSeparator)();
    wchar_t (*vscGetOsExtensionSeparator)();
    bool (*vscStartsWith)(const wchar_t* str, const wchar_t* substring);
    void (*vscPrintDebugMsgToDebugLog)(const wchar_t* msg);
    void (*vscPrintErrorMsgToDebugLog)(const wchar_t* msg);
    bool (*vscGetLastModifiedDate)(const wchar_t* pFileNameStr, time_t& result);
    bool (*vscIsCodeXLServerDll)(const wchar_t* dllFullPath);
    int (*vscDllRegisterServer)();
    int (*vscDllUnregisterServer)();
    void (*vscApplicationCommands_SetOwner)(IVscApplicationCommandsOwner* pOwner);

    // vscDTEConnector
    void* (*vscDTEConnector_CreateInstance)();
    void (*vscDTEConnector_DestroyInstance)(void*& pInstance);
    void (*vscDTEConnector_BuildOpenCLFile)(const wchar_t* pFilePathStr);
    void (*vscDTEConnector_AddFileToOpenedList)(void* pVscInstance, const wchar_t* filePath);
    // void (*vscDTEConnector_ParseAppxRecipe)(void* pVscInstance, const wchar_t* filePath);
    void (*vscDTEConnector_ClearOpenedFiles)(void* pVscInstance);
    int (*vscDTEConnector_GetOpenedFilesCount)(void* pVscInstance);
    void (*vscDTEConnector_GetOpenedFileAt)(void* pVscInstance, int index, wchar_t*& pBuffer);
    bool (*vscDTEConnector_GetFileModificationDate)(void* pVscInstance, int fileIndex, time_t& modificationDate);
    bool (*vscDTEConnector_IsHexDisplayMode)();
    bool (*vscDTEConnector_ParseAppxRecipe_IsPathExists)(const wchar_t* pLayoutDirStr, char*& pPathAsUtf8);
    void (*vscDTEConnector_ChangeHexDisplayMode)(bool isToHexMode);

    // vscEditorDocument
    void (*vscEditorDocument_CreatePaneWindow)(void* pVscInstance, HWND hWndParent, int x, int y, int cx, int cy, HWND* phWnd);
    void (*vscEditorDocument_LoadDocData)(void* pVscInstance, const wchar_t* filePathStr);
    void (*vscEditorDocument_SetEditorCaption)(void* pInstance, const wchar_t* filePathStr, wchar_t*& itemNameStrBuffer);
    void (*vscEditorDocument_ClosePane)(void* pVscInstance);
    void (*vscEditorDocument_OnShow)(void* pVscInstance);
    void (*vscEditorDocument_OnUpdateEdit_Copy)(void* pVscInstance, bool& isCopyPossilble);
    void (*vscEditorDocument_Copy)(void* pVscInstance);
    void (*vscEditorDocument_OnUpdateEdit_SelectAll)(void* pVscInstance, bool& isSelectAllPossible);
    void (*vscEditorDocument_SelectAll)(void* pVscInstance);

    // vscGRApiFunctions
    void (*vscGRApiFunctions_SetOwner)(IVscGRApiFunctionsOwner* pOwner);

    // vscImageAndBuffersEditorDocument
    void* (*vscImageAndBuffersEditorDocument_CreateInstance)();
    bool (*vscCodeXLImageAndBufferCommandIDFromVSCommandId)(const GUID& cmdGuid, DWORD cmdId, long& cmdIdBuffer);
    void (*vscImageAndBuffersEditorDocument_OnImageAndBuffersAction)(const GUID& cmdGuid, DWORD cmdId);
    bool (*vscImageAndBuffersEditorDocument_OnQueryImageAndBufferAction_IsActionRequired)(const GUID& cmdGuid, DWORD cmdId);
    void (*vscImageAndBuffersEditorDocument_OnQueryImageAndBufferCheckedAction_IsActionRequired)(const GUID& cmdGuid, DWORD cmdId, bool& shouldEnableBuffer, bool& shouldCheckBuffer);
    void (*vscImageAndBuffersEditorDocument_OnQueryImageSizeChanged_IsActionRequired)(bool& shouldEnableBuffer);
    bool (*vscImageAndBuffersEditorDocument_GetAvailableZoomLevels)(unsigned int*& pOutBuffer, size_t& sizeBuffer);
    bool (*vscImageAndBuffersEditorDocument_ChangeZoomLevel)(const wchar_t* pZoomText, int& currentZoomLevelBuffer);

    // vscKernelAnalyzerEditorDocument
    void* (*vscKernelAnalyzerEditorDocument_CreateInstance)();

    // vscOwnerRegistrationManager
    void (*vscSetVscDebugEngineOwner)(IVscDebugEngineOwner* pVscDebugEngineOwner);
    void (*vscSetVscBreakpoinstManagerOwner)(IVscBreakpointsManagerOwner* pVscDebugEngineOwner);
    void (*vscSetVscEventsObserverOwner)(IVscEventObserverOwner* pVscDebugEngineOwner);
    void (*vscSetVscWindowsManagerOwner)(IVscWindowsManagerOwner* pOwner);
    void (*vscSetVscProgressBarWrapperOwner)(IProgressBarEventHandler* pOwner);

    // vscPackage
    void* (*vsc_CreateInstance)();
    void (*vsc_DestroyInstance)(void*& pInstance);
    void (*vsc_UpdateProjectSettingsFromVS)(const wchar_t* executableFilePath, const wchar_t* workingDirPath, const wchar_t* cmdLineArgs, const wchar_t* env, bool& isDebuggingRequired, bool& isProfilingRequired, bool& isFrameAnalysisRequired);
    bool (*vsc_ValidateDebugSettings)(const wchar_t* executableFilePath, const wchar_t* workingDirPath, bool isProjectTypeValid);
    void (*vsc_OnUpdateLaunchProfileAction)(bool& isActionEnabled, bool& isActionChecked);
    void (*vsc_OnUpdateLaunchFrameAnalysisAction)(bool& isActionEnabled, bool& isActionChecked);
    bool (*vsc_CanStopCurrentRun)();
    bool (*vsc_IsInRunningMode)();
    void (*vsc_CreateDialogBasedAssertionFailureHandler)(void* pVscInstance);
    void (*vsc_PostSited_InitPackage)(void* pVscInstance);
    void (*vsc_PostSited_InitAsDebugger)();
    void (*vsc_PostSited_InitBreakpointsManager)(void* pDebugger);
    void (*vsc_InitQtApp)();
    void (*vsc_InitDebugTreeHandler)();
    int (*vsc_OnAddKernelMultiWatchFromSourceCode)();
    int (*vsc_OnAddKernelMultiWatchFromLocalsView)();
    int (*vsc_OnAddKernelMultiWatchFromWatchView)();
    void (*vsc_OnViewQuickStart)();
    void (*vsc_OnModeClicked)(int commandId);
    void (*vsc_OpenBreakpointsDialog)();
    bool (*vsc_OnUpdateOpenCLBreakpoints)();
    void (*vsc_PreClosing)(void* pVscInstance);
    int (*vsc_GetObjectNavigationTreeId)();
    int (*vsc_GetPropertiesViewId)();
    int (*vsc_GetMemoryAnalysisViewerId)();
    int (*vsc_GetStateVariablesViewId)();
    int (*vsc_GetStatisticsViewId)();
    int (*vsc_GetFirstMultiWatchViewId)();
    int (*vsc_GetCommandQueuesViewerId)();
    int (*vsc_GetSecondMultiWatchViewId)();
    int (*vsc_GetThirdMultiWatchViewId)();
    int (*vsc_GetShadersSourceCodeViewerHtmlWindowId)();
    void (*vsc_OpenMultiWatchViewId)(int viewID, const wchar_t* variableName);
    void (*vsc_ShowSaveListDialog)(bool& isChangedFilesSaveRequired, bool& isContinueDebugRequired, bool& isBuildProjectRequired);
    bool (*vsc_IsDebuggedProcessExists)();
    void (*vsc_ValidateDriverAndGpu)();
    void (*vsc_GetProcessDllDirectoryFromCreationData)(const wchar_t* exePath, const wchar_t*& o_dllDir);
    void (*vsc_UpdateLaunchDebugAction)(bool& isProcessExistsBuffer, bool& isProcessRunningBuffer);
    void (*vsc_InitKernelAnalyzerPlugin)();
    void (*vsc_InitDebuggerPlugin)();
    void (*vsc_InitPowerProfilingPlugin)();
    void (*vsc_CreateSendErrorReportDialog)(void* pVscInstance);
    void (*vsc_ExecuteProfileSession)();
    void (*vsc_ExecuteFrameAnalysisSession)();
    void (*vsc_DisplayDebugProcessLaunchFailureMessage)(HRESULT failureHR);
    void (*vsc_OpenMatMulExample)();
    void (*vsc_OpenTeapotExample)();
    void (*vsc_OpenD3D12MultithreadingExample)();
    void (*vscGetHelpDevToolsSupportForumURL)(wchar_t*& pBuffer);
    void (*vsc_GetCurrentSettings)(wchar_t*& pExecutableFilePathBuffer, wchar_t*& pWorkingDirectoryPath, wchar_t*& pCommandLineArguments, wchar_t*& pEnvironment);
    void (*vsc_CheckDebuggerInstallation)();
    int (*vsc_GetCallsHistoryListId)();
    void (*vsc_OnCodeExplorerView)();
    bool (*vsc_OnUpdateMode_IsEnabled)();
    bool (*vsc_OnUpdateMode_DebugMode_IsChecked)();
    void (*vsc_OnUpdateMode_DebugMode_Text)(int maxBufSize, wchar_t*& pCmdNameBuffer, int& cmdNameStrLength);
    bool (*vsc_OnUpdateMode_CXLProfileDropDownMenu_IsChecked)();
    void (*vsc_OnUpdateMode_CXLProfileDropDownMenu_CoreLogic)(int maxBufSize, wchar_t*& pCmdNameBuffer, int& cmdNameStrLength);
    bool (*vsc_OnUpdateMode_FrameAnalysisMode_IsChecked)();
    void (*vsc_OnUpdateMode_FrameAnalysis_Text)(int maxBufSize, wchar_t*& pCmdNameBuffer, int& cmdNameStrLength);
    bool (*vsc_OnUpdateMode_AnalyzeMode_IsChecked)();
    void (*vsc_OnUpdateMode_AnalyzeMode_Text)(int maxBufSize, wchar_t*& pCmdNameBuffer, int& cmdNameStrLength);
    void (*vsc_OnLaunchOpenCLDebugging)();
    void (*vsc_GetCurrentModeStr)(wchar_t*& pBuf);
    const wchar_t* (*vsc_GetProfileModeStr)();
    const wchar_t* (*vsc_GetExecutionModeStr)();
    const wchar_t* (*vsc_GetFrameAnalysisModeStr)();
    bool (*vsc_OnFrameStep_OnUpdateMode_IsResumeDebuggingRequired)();
    bool (*vsc_OnAPIStep_OnUpdateMode_IsResumeDebuggingRequired)();
    bool (*vsc_OnFrameStep_OnUpdateMode_IsDebuggedProcExists)();
    void (*vsc_OnUpdateFrameStep)(bool& isProcessSuspended, bool& isProcessExists, bool& shouldShow);
    void (*vsc_OnUpdateStepOver)(bool& isProcessSuspended, bool& isProcessExists, bool& shouldShow);
    void (*vsc_OnUpdateStepInfo)(bool& isStepIntoEnabled, bool& shouldShow);
    void (*vsc_OnUpdateStepOut)(bool& isStepOutEnabled, bool& shouldShow);
    WCHAR* (*CCodeXLVSPackagePackage_GetVSRegistryRootPath)(bool tempRegPath, bool& shouldDeallocate);
    void (*CCodeXLVSPackagePackage_ValidateStartDebugAfterBuild)(wchar_t** pProjectNamesArr, int arrSize, bool& isBuildRequired, bool& isDebugContinueRequired);
    void (*vsc_OnNewDebugEngine)(void* pVscInstance, void* pNewDebugEngine);
    bool (*vsc_IsAnyDebugEngineAvailable)();
    void (*vsc_ViewHelp)();
    void (*vsc_ShowCheckForUpdateDialog)();
    void (*vsc_ShowAboutDialog)();
    REFGUID(*vsc_GetDebugEngineGuid)();
    void (*vsc_SetAppMessageBoxIcon)();
    void (*vsc_SetWindowsStoreAppUserModelID)(const wchar_t* appUserModelId);
    void (*vsc_InitPackageDebugger)(const wchar_t* vsRegistryRootPath, const wchar_t* vsTempRegistryRootPath);
    void (*vsc_InitAppEventObserver)(void* pVscInstance);
    const wchar_t* (*vsc_Get_GD_STR_executionMode)();
    void (*vsc_InitGuiComponents)();
    unsigned int (*vsc_GetInstalledComponentsBitmask)();
    void (*vsc_OnConfigureRemoteHost)();
    void (*vsc_OnStartButton)();
    void (*vsc_OnUpdateConfigureRemoteHost)(bool& isActionEnabled);

    // vscPackageCommandHandler
    void (*vsc_FreeStrMemory)(wchar_t*& pStr);
    void (*vsc_OnUpdateProfileMode)(unsigned long commandId, int activeMode, bool& enableCommand, bool& shouldShowCommand, bool& isChecked);
    bool (*vsc_GetGlobalWorkSize)(int currentCoordinateIndex, int& workDimension);
    bool (*vsc_GetGlobalWorkOffset)(int currentCoordinateIndex, int& workOffset);
    bool (*vsc_GetCurrentWorkItemCoordinate)(int coordinate, int& value);
    bool (*vsc_SetCurrentWorkItemCoordinate)(int coordinate, int value);
    void (*vsc_OnHexDisplayMode)();
    bool (*vsc_OnUpdateHexDisplayMode)();
    int (*vsc_ActiveProfileMode)();
    void (*vsc_OnProfileMode)(unsigned long commandId);
    void (*vsc_OnBreak)();
    void (*vsc_OnAttach)();
    void (*vsc_OnStop)();
    void (*vsc_RefreshFrameAnalysisSessionsFromServer)();
    bool(*vsc_IsFrameAnalysisModeSelected)();
    void(*vsc_OnCapture)(int commandID);
    void (*vsc_OnOpenCLBuild)();
    void (*vsc_OnUpdateOpenCLBuild_IsActionEnabled)(bool& isActionEnabled);
    void (*vsc_OnUpdateOpenCLBuild_IsActionVisible)(bool& isActionVisible);
    bool (*vscIsInBuild)();
    void (*vsc_OnCancelBuild)();
    void (*vsc_OnUpdateCancelBuild_IsActionEnabled)(bool& isActionEnabled);
    void (*vsc_OnUpdateCancelBuild_IsActionVisible)(bool& isActionVisible);
    bool (*vsc_OnUpdateAddOpenCLFile_IsActionVisible)();
    void (*vsc_OnUpdateAddOpenCLFile_AddFileCommand)();
    bool(*vsc_OnUpdateCreateSourceFile_IsActionVisible)();
    void(*vsc_OnUpdateCreateSourceFile_CreateFileCommand)();
    void (*vsc_OnDeviceOptions)();
    void (*vsc_OnKernelSettingOptions)();
    bool (*vsc_onFunctionNameGetList)(const wchar_t* pFilePath, wchar_t**& FunctionStringsArrayBuffer, int& arraySizeBuffer);
    void (*vsc_OnFunctionNameChanged)(const wchar_t* pFunctionNameStr, const wchar_t* activeTreeDocFileFullPath, const wchar_t* activeMDIDocPath);
    void (*vsc_OnUpdateFunctionName)(const wchar_t* pFilePathStr, int bufSize, wchar_t* pOutStr, size_t& outStrSizeBuffer, bool& isEnabled);
    void (*vsc_GetSelectedFunctionName)(wchar_t*& pFunctionNameBuffer);
    void (*vsc_GetBuildOptionsStr)(int maxSize, wchar_t*& pBuildOptionsBuffer, int& strSizeBuffer);
    void (*vsc_OnBuildOptionsChanged_SetBuildOptions)(const wchar_t* str);
    void (*vsc_OnBuildOptionsChanged_GetBuildOptions)(wchar_t*& pBuildOptionsStrBuffer);
    void (*vsc_ShowOptionsDialog)();
    void (*vsc_ShowSystemInformationDialog)();
    bool (*vsc_ToInteger)(const wchar_t* pStr, int& resultBuffer);
    void (*vsc_OnUpdatePPSelectCounters)(bool& isActionVisible, bool& isActionEnabled);
    void (*vsc_OnOpenPPSelectCounters)();
    void (*vsc_OnProjectSettings)(const wchar_t* pFilePathStr);
    void (*vsc_DisplayNoVisualCProjectMsgBox)();
    void (*vsc_OnUpdateDebugSettings)(bool& enableCommand);
    void (*vsc_SetDXFolderModel)(const wchar_t* pTargetNameStr, const wchar_t* pFilePathStr);
    void (*vsc_GetDXFolderModel)(wchar_t*& pTargetNameBuffer);
    void (*vsc_SetSelectedDXShaderType)(const wchar_t* pTypeNameStr, const wchar_t* pFilePathStr);
    void (*vsc_GetSelectedDXShaderType)(wchar_t*& pTypeNameBuffer);
    void (*vsc_UpdateToolbarCommands)();
    void (*vsc_GetActiveStaticAnalyzerTreeDocument)(wchar_t*& pDocumentNameBuffer, bool& wasFileChanged);
    void (*vsc_GetBuildCommandString)(wchar_t*& pBuildCommandBuffer);
    bool (*vsc_IsDirectXFile)(const wchar_t* pFilePathStr);
    bool (*vsc_IsOpenGLFile)(const wchar_t* pFilePathStr);
    void (*vsc_SetSelectedBitness)(const wchar_t* pBitnessStr);
    void (*vsc_GetSelectedBitness)(wchar_t*& pBitnessBuffer);
    bool (*vsc_IsPlatformFile)(const wchar_t* pFilePathStr, const wchar_t* pPlatformFilesExtensions);
    bool (*vsc_IsCurrentPlatform)(const wchar_t* pCandidatePlatform);
    bool(*vsc_IsSourceFileSelected)();
    bool (*vsc_IsDXShaderSelected)();
    bool (*vsc_IsCLFileSelected)();
    bool (*vsc_IsDXFolderSelected)();

    // vscProfileSessionEditorDocument
    void* (*vscProfileSessionEditorDocument_CreateInstance)();
    void (*vscProfileSessionEditorDocument_SetVSCOwner)(void* pVscInstance, IProfileSessionEditorDocVsCoreImplOwner* handler);
    void (*vscProfileSessionEditorDocument_LoadSession)(void* pVscInstance);
    // void (*vscProfileSessionEditorDocument_GetEditorCaption)(void* pVscInstance, wchar_t*& pOutBuffer);

    // vscSourceCodeViewer
    void (*vscSourceCodeViewerOwner_SetOwner)(const IVscSourceCodeViewerOwner* pOwner);

    // vscTimer
    void* (*vscTimer_CreateInstance)();
    void (*vscTimer_DestroyInstance)(void* pVscTimer);
    void (*vscTimer_SetOwner)(void* pVscTimer, IVscTimerOwner* pOwner);
    void (*vscTimer_OnClockTick)(void* pVscTimer);

    // vscToolWindow
    void* (*vscToolWindow__CreateInstance)();
    void (*vscToolWindow__DestroyInstance)(void*& pInstance);
    bool (*vscToolWindow_CreateQTPaneWindow)(void* pVscInstance, int gdWindowCommandID);
    void (*vscToolWindow_CreatePaneWindow)(void* pVscInstance, HWND hwndParent, int x, int y, int cx, int cy, HWND* phWND, int gdWindowCommandID);
    void (*vscToolWindow_OnUpdateEditCommand)(void* pVscInstance, bool& isEnabled, bool& isFoundCommandHandler, int cmdId);
    void (*vscToolWindow_OnExecuteEditCommand)(void* pVscInstance, int senderId);
    void (*vscToolWindow_OnFrameShow)(void* pVscInstance, bool isFrameShown, int gdWindowCommandID);
    void (*vscToolWindow_GetVersionCaption)(wchar_t*& pCaptionBuffer);
    void (*vscToolWindow_SetToolShowFunction)(void* pToolWindow, void* pVspToolWindow, void* pShowFunction);

    // vscUtils
    void (*vscUtilsGetStartActionCommandName)(wchar_t*& verbNameBuffer, wchar_t*& actionCommandStrBuffer, bool addKeyboardShortcut /* = false */, bool fullString /* = true */);
    void (*vscUtilsUpdateProjectSettingsFromStartupProject)(const wchar_t* execPath, const wchar_t* workDir, const wchar_t* cmdArgs, const wchar_t* execEnv, bool isProjectOpened, bool isProjectTypeSupported, bool isNonNativeProject);
    bool (*vscGetExecutionCommandName)(DWORD commandId, wchar_t*& commandNameBuffer);

    // vscVspDTEInvoker
    void (*vscVspDTEInvoker_SetIVspDTEConnector)(IVspDTEConnector* pVspDteConnector);

    // vscWindowsManager
    void (*vscWindowsManager_SetIVsUiShell)(IVsUIShell* pUiShell);
    IVsUIShell* (*vscWindowsManager_GetIVsUiShell)();
};

extern "C"
{
    CXLVSCORE_API bool vscGetCoreAPI(IVscCoreAPI* o_pCoreAPI);
}

#endif // __IVSCCOREAPI_H
