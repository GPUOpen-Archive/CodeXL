//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscPackageCommandHandler.h
///
//==================================================================================

#ifndef vscPackageCommandHandler_h__
#define vscPackageCommandHandler_h__
#include "CodeXLVSPackageCoreDefs.h"

// Use this function to free memory allocated by the core.
// Note that in order to avoid runtime mixing, memory which was allocated
// by the core, should be freed using this function, never by the package.
void vsc_FreeStrMemory(wchar_t*& pStr);

void vsc_OnUpdateProfileMode(unsigned long commandId, int activeMode, bool& enableCommand, bool& shouldShowCommand, bool& isChecked);

bool vsc_GetGlobalWorkSize(int currentCoordinateIndex, int& workDimension);

bool vsc_GetGlobalWorkOffset(int currentCoordinateIndex, int& workOffset);

bool vsc_GetCurrentWorkItemCoordinate(int coordinate, int& value);

bool vsc_SetCurrentWorkItemCoordinate(int coordinate, int value);

void vsc_OnHexDisplayMode();

bool vsc_OnUpdateHexDisplayMode();

int vsc_ActiveProfileMode();

void vsc_OnProfileMode(unsigned long commandId);

void vsc_OnBreak();

void vsc_OnAttach();

void vsc_OnStop();

void vsc_RefreshFrameAnalysisSessionsFromServer();

void vsc_OnCapture();

void vsc_OnOpenCLBuild();

void vsc_OnUpdateOpenCLBuild_IsActionEnabled(bool& isActionEnabled);

void vsc_OnUpdateOpenCLBuild_IsActionVisible(bool& isActionVisible);

bool vscIsInBuild();

void vsc_OnCancelBuild();

void vsc_OnUpdateCancelBuild_IsActionEnabled(bool& isActionEnabled);

void vsc_OnUpdateCancelBuild_IsActionVisible(bool& isActionVisible);


bool vsc_OnUpdateAddOpenCLFile_IsActionVisible();

void vsc_OnUpdateAddOpenCLFile_AddFileCommand();

bool vsc_OnUpdateCreateSourceFile_IsActionVisible();

void vsc_OnUpdateCreateSourceFile_CreateFileCommand();


void vsc_OnDeviceOptions();

void vsc_OnKernelSettingOptions();

bool vsc_onFunctionNameGetList(const wchar_t* pFilePath, wchar_t**& FunctionStringsArrayBuffer, int& arraySizeBuffer);

void vsc_OnFunctionNameChanged(const wchar_t* pFunctionNameStr, const wchar_t* activeTreeDocFileFullPath, const wchar_t* activeMDIDocPath);

void vsc_OnUpdateFunctionName(const wchar_t* pFilePathStr, int bufSize, wchar_t* pOutStr, size_t& outStrSizeBuffer, bool& isEnabled);

void vsc_GetSelectedFunctionName(wchar_t*& pFunctionNameBuffer);

void vsc_GetBuildOptionsStr(int maxSize, wchar_t*& pBuildOptionsBuffer, int& strSizeBuffer);

void vsc_OnBuildOptionsChanged_SetBuildOptions(const wchar_t* str);

void vsc_OnBuildOptionsChanged_GetBuildOptions(wchar_t*& pBuildOptionsStrBuffer);

void vsc_ShowOptionsDialog();

void vsc_ShowSystemInformationDialog();

bool vsc_ToInteger(const wchar_t* pStr, int& resultBuffer);

void vsc_OnUpdatePPSelectCounters(bool& isActionVisible, bool& isActionEnabled);

void vsc_OnOpenPPSelectCounters();

void vsc_OnProjectSettings(const wchar_t* pFilePathStr);

void vsc_DisplayNoVisualCProjectMsgBox();

void vsc_OnUpdateDebugSettings(bool& enableCommand);

void vsc_OnUpdateDebugSettings(bool& enableCommand);

void vsc_SetDXFolderModel(const wchar_t* pTargetNameStr, const wchar_t* pFilePathStr);

void vsc_GetDXFolderModel(wchar_t*& pTargetNameBuffer);

void vsc_SetSelectedDXShaderType(const wchar_t* pTypeNameStr, const wchar_t* pFilePathStr);

void vsc_GetSelectedDXShaderType(wchar_t*& pTypeNameBuffer);

void vsc_UpdateToolbarCommands();

void vsc_GetActiveStaticAnalyzerTreeDocument(wchar_t*& pDocumentNameBuffer, bool& wasFileChanged);

void vsc_GetBuildCommandString(wchar_t*& pBuildCommandBuffer);

bool vsc_IsDirectXFile(const wchar_t* pFilePathStr);

bool vsc_IsOpenGLFile(const wchar_t* pFilePathStr);

void vsc_SetSelectedBitness(const wchar_t* pBitnessStr);

void vsc_GetSelectedBitness(wchar_t*& pBitnessBuffer);

/// Obtains file extension from full path and searches it in specified platform files extensions
/// \param[in] filePathStr string containing full file path
/// \param[in] platformFilesExtensionsStr - string containing all possible extensions of specific platform kernels|shaders
/// \return true if current file extension is found in the list
bool vsc_IsPlatformFile(const wchar_t* pFilePathStr, const wchar_t* pPlatformFilesExtensions);

/// Checks if passed platform is currently selected platform
/// \param[in] platformStr candidate platform
/// \return true if current platform is the candidate
bool vsc_IsCurrentPlatform(const wchar_t* pCandidatePlatform);

/// Checks if a file from source pool is currently selected
bool vsc_IsSourceFileSelected();

/// Checks if DX shader is currently selected
bool vsc_IsDXShaderSelected();

/// Checks if CL file is currently selected
bool vsc_IsCLFileSelected();

/// Checks if DX Folder is currently selected
bool vsc_IsDXFolderSelected();

#endif // vscPackageCommandHandler_h__
