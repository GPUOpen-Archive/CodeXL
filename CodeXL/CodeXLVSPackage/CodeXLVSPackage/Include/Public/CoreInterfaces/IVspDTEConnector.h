//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IVspDTEConnector.h
///
//==================================================================================

#ifndef IVspDTEConnector_h__
#define IVspDTEConnector_h__
#include <Include/Public/CoreInterfaces/IDteTreeEventHandler.h>

class IVspDTEConnector
{
public:
    IVspDTEConnector() {};
    virtual ~IVspDTEConnector() {};
    virtual void vspDtecBuildSaveTree(IDteTreeEventHandler* pDteTreeEventHandler) = 0;
    virtual void vspDtecGetKernelSourceFilePath(wchar_t**& pProgramFilePaths, int& programFilePathsSize,  bool isOnlyFromStartupProject) = 0;
    virtual void vspDtecDeleteWCharStrBuffers(wchar_t**& pBuffersArr, int arrSize) = 0;
    virtual void vspDtecGetActiveDocumentFileFullPath(wchar_t*& pBuffer) = 0;
    virtual void vspDtecGetListOfFilesContainedAtDirectory(const wchar_t* pFolderStr, wchar_t**& pListOfFilePathContained, int& listOfPathSize) = 0;
    virtual void vspDtecSaveFileWithPath(const wchar_t* folderStr) = 0;
    virtual bool vspDtecResumeDebugging() = 0;
};

#endif // IVspDTEConnector_h__