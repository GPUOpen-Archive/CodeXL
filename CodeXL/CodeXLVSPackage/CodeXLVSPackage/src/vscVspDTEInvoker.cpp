//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscVspDTEInvoker.cpp
///
//==================================================================================

#include "stdafx.h"
#include <Include\Public\vscVspDTEInvoker.h>
#include <AMDTBaseTools\Include\gtAssert.h>

class vscVspDTEInvoker
{
public:
    ~vscVspDTEInvoker() {}

    static vscVspDTEInvoker& GetInstance()
    {
        static vscVspDTEInvoker instance;
        return instance;
    }

    IVspDTEConnector* m_pVspDteConnector;

private:
    vscVspDTEInvoker() : m_pVspDteConnector(NULL) {}

};

void vscVspDTEInvoker_SetIVspDTEConnector(IVspDTEConnector* pVspDteConnector)
{
    GT_ASSERT(pVspDteConnector != NULL);
    vscVspDTEInvoker::GetInstance().m_pVspDteConnector = pVspDteConnector;
}

void vscVspDTEInvoker_BuildSaveTree(IDteTreeEventHandler* pDteTreeEventHandler)
{
    GT_IF_WITH_ASSERT(pDteTreeEventHandler != NULL)
    {
        vscVspDTEInvoker& instance = vscVspDTEInvoker::GetInstance();
        GT_IF_WITH_ASSERT(instance.m_pVspDteConnector != NULL)
        {
            instance.m_pVspDteConnector->vspDtecBuildSaveTree(pDteTreeEventHandler);
        }
    }
}

void vscVspDTEInvoker_GetKernelSourceFilePath(wchar_t**& pProgramFilePaths, int& programFilePathsSize, bool isOnlyFromStartupProject)
{
    vscVspDTEInvoker& instance = vscVspDTEInvoker::GetInstance();
    GT_IF_WITH_ASSERT(instance.m_pVspDteConnector != NULL)
    {
        instance.m_pVspDteConnector->vspDtecGetKernelSourceFilePath(pProgramFilePaths, programFilePathsSize, isOnlyFromStartupProject);
    }
}

void vscVspDTEInvoker_DeleteWcharStrBuffersArray(wchar_t**& pBuffersArray, int buffersArrSize)
{
    vscVspDTEInvoker& instance = vscVspDTEInvoker::GetInstance();
    GT_IF_WITH_ASSERT(instance.m_pVspDteConnector != NULL)
    {
        instance.m_pVspDteConnector->vspDtecDeleteWCharStrBuffers(pBuffersArray, buffersArrSize);
    }
}

void vscVspDTEInvoker_GetActiveDocumentFileFullPath(wchar_t*& pBuffer)
{
    vscVspDTEInvoker& instance = vscVspDTEInvoker::GetInstance();
    GT_IF_WITH_ASSERT(instance.m_pVspDteConnector != NULL)
    {
        instance.m_pVspDteConnector->vspDtecGetActiveDocumentFileFullPath(pBuffer);
    }
}

void vscVspDTEInvoker_GetListOfFilesContainedAtDirectory(const std::wstring& folderStr, std::list<std::wstring>& listOfFilePathContained)
{
    vscVspDTEInvoker& instance = vscVspDTEInvoker::GetInstance();
    GT_IF_WITH_ASSERT(instance.m_pVspDteConnector != NULL)
    {
        wchar_t** pListOfFilePathContained;
        int listOfPathSize;

        instance.m_pVspDteConnector->vspDtecGetListOfFilesContainedAtDirectory(folderStr.c_str(), pListOfFilePathContained, listOfPathSize);

        // copy the buffers to strings
        for (int nFolder = 0; nFolder < listOfPathSize; nFolder++)
        {
            listOfFilePathContained.push_back(std::wstring(pListOfFilePathContained[nFolder]));
        }

        // release memory
        vscVspDTEInvoker_DeleteWcharStrBuffersArray(pListOfFilePathContained, listOfPathSize);
    }
}

void vscVspDTEInvoker_SaveFileWithPath(const std::wstring& folderStr)
{
    vscVspDTEInvoker& instance = vscVspDTEInvoker::GetInstance();
    GT_IF_WITH_ASSERT(instance.m_pVspDteConnector != NULL)
    {
        instance.m_pVspDteConnector->vspDtecSaveFileWithPath(folderStr.c_str());
    }
}

bool vscVspDTEInvoker_ResumeDebugging()
{
    bool retVal = false;

    vscVspDTEInvoker& instance = vscVspDTEInvoker::GetInstance();
    GT_IF_WITH_ASSERT(NULL != instance.m_pVspDteConnector)
    {
        retVal = instance.m_pVspDteConnector->vspDtecResumeDebugging();
    }

    return retVal;
}


