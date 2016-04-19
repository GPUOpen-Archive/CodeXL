//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDTEConnector.h
///
//==================================================================================

#ifndef vscDTEConnector_h__
#define vscDTEConnector_h__
#include "CodeXLVSPackageCoreDefs.h"
#include <ctime>

// *************************************************************************************************
// This is an interface to be used by the the VS package's vspDTEConnector in order to invoke
// core logic. In case that core components would like to utilize the VS package's vspDTEConnector,
// it would be possible through the vscVspDTRInvoker object.
// *************************************************************************************************
void* vscDTEConnector_CreateInstance();

void  vscDTEConnector_DestroyInstance(void*& pInstance);

void  vscDTEConnector_BuildOpenCLFile(const wchar_t* pFilePathStr);

void  vscDTEConnector_AddFileToOpenedList(void* pVscInstance, const wchar_t* filePath);

// void  vscDTEConnector_ParseAppxRecipe(void* pVscInstance, const wchar_t* filePath);

void  vscDTEConnector_ClearOpenedFiles(void* pVscInstance);

int vscDTEConnector_GetOpenedFilesCount(void* pVscInstance);

void  vscDTEConnector_GetOpenedFileAt(void* pVscInstance, int index, wchar_t*& pBuffer);

bool vscDTEConnector_GetFileModificationDate(void* pVscInstance, int fileIndex, time_t& modificationDate);

bool vscDTEConnector_IsHexDisplayMode();

bool vscDTEConnector_ParseAppxRecipe_IsPathExists(const wchar_t* pLayoutDirStr, char*& pPathAsUtf8);

void  vscDTEConnector_ChangeHexDisplayMode(bool isToHexMode);

#endif // vscDTEConnector_h__
