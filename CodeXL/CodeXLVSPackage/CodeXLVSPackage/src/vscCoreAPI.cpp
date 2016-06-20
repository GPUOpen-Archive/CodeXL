//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscCoreAPI.cpp
///
//==================================================================================

#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <Include/Public/CoreInterfaces/IVscCoreAPI.h>

// Exported interfaces:
#include <Include/Public/vscCoreUtils.h>
#include <Include/Public/vscDTEConnector.h>
#include <Include/Public/vscEditorDocument.h>
#include <Include/Public/vscGRApiFunctions.h>
#include <Include/Public/vscImageAndBuffersEditorDocument.h>
#include <Include/Public/vscKernelAnalyzerEditorDocument.h>
#include <Include/Public/vscOwnerRegistrationManager.h>
#include <Include/Public/vscPackage.h>
#include <Include/Public/vscPackageCommandHandler.h>
#include <Include/Public/vscProfileSessionEditorDocument.h>
#include <Include/Public/vscSourceCodeViewer.h>
#include <Include/Public/vscTimer.h>
#include <Include/Public/vscToolWindow.h>
#include <Include/Public/vscUtils.h>
#include <Include/Public/vscVspDTEInvoker.h>
#include <Include/Public/vscWindowsManager.h>

// These functions from vspDLLInit.cpp have no header file:
BOOL APIENTRY vspDllMain(DWORD reason, LPVOID reserved, HINSTANCE hInstance);
void vscSetApplicationBinariesFolder(const char* pathStr);

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD

    #define VSC_BEFORE_GETTING_CORE_FUNCTIONS int funcs = 0;
    #define VSC_GET_CORE_FUNCTION(p, fn, retVal) p->fn = &fn; retVal = retVal && (nullptr != p->fn); ++funcs;
    #define VSC_AFTER_GETTING_CORE_FUNCTIONS GT_ASSERT( sizeof(IVscCoreAPI) == (funcs * sizeof(void*)));

#elif AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD

    #define VSC_BEFORE_GETTING_CORE_FUNCTIONS
    #define VSC_GET_CORE_FUNCTION(p, fn, retVal) p->fn = &fn; retVal = retVal && (nullptr != p->fn);
    #define VSC_AFTER_GETTING_CORE_FUNCTIONS

#else
    #error Unknown configuration!
#endif

CXLVSCORE_API bool vscGetCoreAPI(IVscCoreAPI* o_pCoreAPI)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr != o_pCoreAPI)
    {
        retVal = true;

        VSC_BEFORE_GETTING_CORE_FUNCTIONS;

        // This list should exactly match the structure definition in IVscCoreAPI.h:
        // vspDLLInit.cpp
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vspDllMain, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscSetApplicationBinariesFolder, retVal);

        // vscCoreUtils
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDeleteWcharString, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDeleteWcharStringArray, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDeleteCharString, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDeleteUintBuffer, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscIsPathStringsEqual, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscExtractFileExtension, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscIsPathExists, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscGetFileDirectoryAsString, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscGetFileName, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscGetOsPathSeparator, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscGetOsExtensionSeparator, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscStartsWith, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscPrintDebugMsgToDebugLog, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscPrintErrorMsgToDebugLog, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscGetLastModifiedDate, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscIsCodeXLServerDll, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDllRegisterServer, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDllUnregisterServer, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscApplicationCommands_SetOwner, retVal);

        // vscDTEConnector
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDTEConnector_CreateInstance, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDTEConnector_DestroyInstance, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDTEConnector_BuildOpenCLFile, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDTEConnector_AddFileToOpenedList, retVal);
        // VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDTEConnector_ParseAppxRecipe, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDTEConnector_ClearOpenedFiles, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDTEConnector_GetOpenedFilesCount, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDTEConnector_GetOpenedFileAt, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDTEConnector_GetFileModificationDate, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDTEConnector_IsHexDisplayMode, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDTEConnector_ParseAppxRecipe_IsPathExists, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscDTEConnector_ChangeHexDisplayMode, retVal);

        // vscEditorDocument
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscEditorDocument_CreatePaneWindow, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscEditorDocument_LoadDocData, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscEditorDocument_SetEditorCaption, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscEditorDocument_ClosePane, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscEditorDocument_OnShow, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscEditorDocument_OnUpdateEdit_Copy, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscEditorDocument_Copy, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscEditorDocument_OnUpdateEdit_SelectAll, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscEditorDocument_SelectAll, retVal);

        // vscGRApiFunctions
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscGRApiFunctions_SetOwner, retVal);

        // vscImageAndBuffersEditorDocument
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscImageAndBuffersEditorDocument_CreateInstance, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscCodeXLImageAndBufferCommandIDFromVSCommandId, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscImageAndBuffersEditorDocument_OnImageAndBuffersAction, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscImageAndBuffersEditorDocument_OnQueryImageAndBufferAction_IsActionRequired, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscImageAndBuffersEditorDocument_OnQueryImageAndBufferCheckedAction_IsActionRequired, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscImageAndBuffersEditorDocument_OnQueryImageSizeChanged_IsActionRequired, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscImageAndBuffersEditorDocument_GetAvailableZoomLevels, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscImageAndBuffersEditorDocument_ChangeZoomLevel, retVal);

        // vscKernelAnalyzerEditorDocument
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscKernelAnalyzerEditorDocument_CreateInstance, retVal);

        // vscOwnerRegistrationManager
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscSetVscDebugEngineOwner, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscSetVscBreakpoinstManagerOwner, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscSetVscEventsObserverOwner, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscSetVscWindowsManagerOwner, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscSetVscProgressBarWrapperOwner, retVal);

        // vscPackage
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_CreateInstance, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_DestroyInstance, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_UpdateProjectSettingsFromVS, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_ValidateDebugSettings, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateLaunchProfileAction, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateLaunchFrameAnalysisAction, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_CanStopCurrentRun, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_IsInRunningMode, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_CreateDialogBasedAssertionFailureHandler, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_PostSited_InitPackage, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_PostSited_InitAsDebugger, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_PostSited_InitBreakpointsManager, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_InitQtApp, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_InitDebugTreeHandler, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnAddKernelMultiWatchFromSourceCode, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnAddKernelMultiWatchFromLocalsView, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnAddKernelMultiWatchFromWatchView, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnViewQuickStart, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnModeClicked, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OpenBreakpointsDialog, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateOpenCLBreakpoints, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_PreClosing, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetObjectNavigationTreeId, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetPropertiesViewId, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetMemoryAnalysisViewerId, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetStateVariablesViewId, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetStatisticsViewId, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetFirstMultiWatchViewId, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetCommandQueuesViewerId, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetSecondMultiWatchViewId, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetThirdMultiWatchViewId, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetShadersSourceCodeViewerHtmlWindowId, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OpenMultiWatchViewId, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_ShowSaveListDialog, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_IsDebuggedProcessExists, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_ValidateDriverAndGpu, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetProcessDllDirectoryFromCreationData, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_UpdateLaunchDebugAction, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_InitKernelAnalyzerPlugin, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_InitDebuggerPlugin, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_InitPowerProfilingPlugin, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_CreateSendErrorReportDialog, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_ExecuteProfileSession, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_ExecuteFrameAnalysisSession, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_DisplayDebugProcessLaunchFailureMessage, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OpenMatMulExample, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OpenTeapotExample, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OpenD3D12MultithreadingExample, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscGetHelpDevToolsSupportForumURL, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetCurrentSettings, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_CheckDebuggerInstallation, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetCallsHistoryListId, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnCodeExplorerView, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateMode_IsEnabled, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateMode_DebugMode_IsChecked, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateMode_DebugMode_Text, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateMode_CXLProfileDropDownMenu_IsChecked, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateMode_CXLProfileDropDownMenu_CoreLogic, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateMode_FrameAnalysisMode_IsChecked, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateMode_FrameAnalysis_Text, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateMode_AnalyzeMode_IsChecked, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateMode_AnalyzeMode_Text, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnLaunchOpenCLDebugging, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetCurrentModeStr, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetProfileModeStr, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetExecutionModeStr, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetFrameAnalysisModeStr, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnFrameStep_OnUpdateMode_IsResumeDebuggingRequired, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnAPIStep_OnUpdateMode_IsResumeDebuggingRequired, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnFrameStep_OnUpdateMode_IsDebuggedProcExists, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateFrameStep, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateStepOver, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateStepInfo, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateStepOut, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, CCodeXLVSPackagePackage_GetVSRegistryRootPath, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, CCodeXLVSPackagePackage_ValidateStartDebugAfterBuild, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnNewDebugEngine, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_IsAnyDebugEngineAvailable, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_ViewHelp, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_ShowCheckForUpdateDialog, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_ShowAboutDialog, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetDebugEngineGuid, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_SetAppMessageBoxIcon, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_SetWindowsStoreAppUserModelID, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_InitPackageDebugger, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_InitAppEventObserver, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_Get_GD_STR_executionMode, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_InitGuiComponents, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetInstalledComponentsBitmask, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnConfigureRemoteHost, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnStartButton, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateConfigureRemoteHost, retVal);

        // vscPackageCommandHandler
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_FreeStrMemory, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateProfileMode, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetGlobalWorkSize, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetGlobalWorkOffset, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetCurrentWorkItemCoordinate, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_SetCurrentWorkItemCoordinate, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnHexDisplayMode, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateHexDisplayMode, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_ActiveProfileMode, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnProfileMode, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnBreak, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnAttach, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnStop, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_RefreshFrameAnalysisSessionsFromServer, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_IsFrameAnalysisModeSelected, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnCapture, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnOpenCLBuild, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateOpenCLBuild_IsActionEnabled, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateOpenCLBuild_IsActionVisible, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscIsInBuild, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnCancelBuild, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateCancelBuild_IsActionEnabled, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateCancelBuild_IsActionVisible, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateAddOpenCLFile_IsActionVisible, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateAddOpenCLFile_AddFileCommand, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateCreateSourceFile_IsActionVisible, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateCreateSourceFile_CreateFileCommand, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnDeviceOptions, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnKernelSettingOptions, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_onFunctionNameGetList, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnFunctionNameChanged, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateFunctionName, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetSelectedFunctionName, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetBuildOptionsStr, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnBuildOptionsChanged_SetBuildOptions, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnBuildOptionsChanged_GetBuildOptions, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_ShowOptionsDialog, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_ShowSystemInformationDialog, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_ToInteger, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdatePPSelectCounters, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnOpenPPSelectCounters, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnProjectSettings, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_DisplayNoVisualCProjectMsgBox, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_OnUpdateDebugSettings, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_SetDXFolderModel, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetDXFolderModel, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_SetSelectedDXShaderType, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetSelectedDXShaderType, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_UpdateToolbarCommands, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetActiveStaticAnalyzerTreeDocument, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetBuildCommandString, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_IsDirectXFile, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_IsOpenGLFile, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_SetSelectedBitness, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_GetSelectedBitness, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_IsPlatformFile, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_IsCurrentPlatform, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_IsDXShaderSelected, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_IsDXFolderSelected, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_IsCLFileSelected, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vsc_IsSourceFileSelected, retVal);

        // vscProfileSessionEditorDocument
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscProfileSessionEditorDocument_CreateInstance, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscProfileSessionEditorDocument_SetVSCOwner, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscProfileSessionEditorDocument_LoadSession, retVal);
        // VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscProfileSessionEditorDocument_GetEditorCaption, retVal);

        // vscSourceCodeViewer
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscSourceCodeViewerOwner_SetOwner, retVal);

        // vscTimer
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscTimer_CreateInstance, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscTimer_DestroyInstance, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscTimer_SetOwner, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscTimer_OnClockTick, retVal);

        // vscToolWindow
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscToolWindow__CreateInstance, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscToolWindow__DestroyInstance, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscToolWindow_CreateQTPaneWindow, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscToolWindow_CreatePaneWindow, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscToolWindow_OnUpdateEditCommand, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscToolWindow_OnExecuteEditCommand, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscToolWindow_OnFrameShow, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscToolWindow_GetVersionCaption, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscToolWindow_SetToolShowFunction, retVal);

        // vscUtils
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscUtilsGetStartActionCommandName, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscUtilsUpdateProjectSettingsFromStartupProject, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscGetExecutionCommandName, retVal);

        // vscVspDTEInvoker
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscVspDTEInvoker_SetIVspDTEConnector, retVal);

        // vscWindowsManager
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscWindowsManager_SetIVsUiShell, retVal);
        VSC_GET_CORE_FUNCTION(o_pCoreAPI, vscWindowsManager_GetIVsUiShell, retVal);

        VSC_AFTER_GETTING_CORE_FUNCTIONS;
    }

    return retVal;
}

