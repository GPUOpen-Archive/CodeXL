//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscVspDTEInvoker.h
///
//==================================================================================

#ifndef vscVspDTEInvoker_h__
#define vscVspDTEInvoker_h__
#include <Include/Public/CoreInterfaces/IVspDTEConnector.h>

// Registers the VS package's DTE connector.
void vscVspDTEInvoker_SetIVspDTEConnector(IVspDTEConnector* pVspDteConnector);

void vscVspDTEInvoker_BuildSaveTree(IDteTreeEventHandler* pDteTreeEventHandler);
void vscVspDTEInvoker_DeleteWcharStrBuffersArray(wchar_t**& pBuffersArray, int buffersArrSize);
void vscVspDTEInvoker_GetKernelSourceFilePath(wchar_t**& pProgramFilePaths, int& programFilePathsSize, bool isOnlyFromStartupProject);
void vscVspDTEInvoker_GetActiveDocumentFileFullPath(wchar_t*& pBuffer);
void vscVspDTEInvoker_GetListOfFilesContainedAtDirectory(const std::wstring& folderStr, std::list<std::wstring>& listOfFilePathContained);
void vscVspDTEInvoker_SaveFileWithPath(const std::wstring& folderStr);
bool vscVspDTEInvoker_ResumeDebugging();

#endif // vscVspDTEInvoker_h__
