//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDTEConnector.cpp
///
//==================================================================================

#include "StdAfx.h"
#include <Include/Public/vscDTEConnector.h>
#include <Include/vscCoreInternalUtils.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtQueue.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>

#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

class vscDTEConnector
{
public:
    vscDTEConnector()
    {
        // Clear edit and continue data
        _openedFiles.clear();
    }
    ~vscDTEConnector() {}

    // private struct for edit and continue info
    struct vspEditAndContinue
    {
    public:
        osFilePath _filePath;
        time_t _fileModifiedDate;
    };

    void AddFileToOpenedList(const wchar_t* filePathStr)
    {
        // Add .cl files to the files to be checked if modified for edit and continue
        gtString fileExtension;
        osFilePath filePath(filePathStr);
        filePath.getFileExtension(fileExtension);

        if (fileExtension == AF_STR_clSourceFileExtension)
        {
            bool foundInList = false;
            // check if it is not already in the list
            int numOpenedFiles = _openedFiles.size();

            for (int nFile = 0; nFile < numOpenedFiles; nFile++)
            {
                // Get the current file checked
                osFilePath& currentFilePath = _openedFiles[nFile]._filePath;

                if (currentFilePath.asString() == filePath.asString())
                {
                    foundInList = true;
                }
            }

            if (!foundInList)
            {
                // Get the date of the file
                osStatStructure fileProperties;
                gtString fileName = filePath.asString();

                // Get the files status
                int rc1 = osWStat(fileName, fileProperties);

                GT_ASSERT(rc1 == 0);

                time_t lastModifiedFileTime = fileProperties.st_mtime;

                vspEditAndContinue openedFileData;
                openedFileData._filePath = filePath;
                openedFileData._fileModifiedDate = lastModifiedFileTime;

                // Store the opened document with its current modified date
                _openedFiles.push_back(openedFileData);
            }
        }
    }

    void ClearOpenedFiles()
    {
        _openedFiles.clear();
    }

    int GetOpenedFilesCount()
    {
        return _openedFiles.size();
    }

    void GetOpenedFileAt(wchar_t*& pBuffer, int index)
    {
        pBuffer = NULL;

        if (static_cast<unsigned int>(index) < _openedFiles.size())
        {
            const gtString& tmp = _openedFiles[index]._filePath.asString();
            pBuffer = vscAllocateAndCopy(tmp);
        }
    }

    bool GetFileModifiedDate(int fileIndex, time_t& modifiedDate)
    {
        bool ret = false;

        if (static_cast<unsigned int>(fileIndex) < _openedFiles.size())
        {
            modifiedDate = _openedFiles[fileIndex]._fileModifiedDate;
            ret = true;
        }

        return ret;
    }

    // edit and continue files that are opened
    gtVector<vspEditAndContinue> _openedFiles;
};

void vscDTEConnector_BuildOpenCLFile(const wchar_t* pFilePathStr)
{
    osFilePath filePath(pFilePathStr);
    gtVector <osFilePath> filePathsVector;
    filePathsVector.push_back(filePath);
    kaApplicationCommands::instance().buildCommand(filePathsVector);
}

void* vscDTEConnector_CreateInstance()
{
    static vscDTEConnector instance;
    return &instance;
}

void vscDTEConnector_AddFileToOpenedList(void* pVscInstance, const wchar_t* filePath)
{
    vscDTEConnector* pInstance = (vscDTEConnector*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        pInstance->AddFileToOpenedList(filePath);
    }
}

void vscDTEConnector_ClearOpenedFiles(void* pVscInstance)
{
    vscDTEConnector* pInstance = (vscDTEConnector*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        pInstance->ClearOpenedFiles();
    }
}

int vscDTEConnector_GetOpenedFilesCount(void* pVscInstance)
{
    int ret = 0;
    vscDTEConnector* pInstance = (vscDTEConnector*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        ret = pInstance->GetOpenedFilesCount();
    }
    return ret;
}

void vscDTEConnector_GetOpenedFileAt(void* pVscInstance, int index, wchar_t*& pBuffer)
{
    vscDTEConnector* pInstance = (vscDTEConnector*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        pInstance->GetOpenedFileAt(pBuffer, index);
    }
}

bool vscDTEConnector_GetFileModificationDate(void* pVscInstance, int fileIndex, time_t& modificationDate)
{
    bool ret = false;
    vscDTEConnector* pInstance = (vscDTEConnector*)pVscInstance;
    GT_IF_WITH_ASSERT(pInstance != NULL)
    {
        ret = pInstance->GetFileModifiedDate(fileIndex, modificationDate);
    }
    return ret;
}

bool vscDTEConnector_IsHexDisplayMode()
{
    return gaIsHexDisplayMode();
}

void vscDTEConnector_ChangeHexDisplayMode(bool isToHexMode)
{
    gaSetHexDisplayMode(isToHexMode);
}

void vscDTEConnector_DestroyInstance(void*& pInstance)
{
    delete pInstance;
    pInstance = NULL;
}

bool vscDTEConnector_ParseAppxRecipe_IsPathExists(const wchar_t* pLayoutDirStr, char*& pPathAsUtf8)
{
    osFilePath filePath;
    pPathAsUtf8 = NULL;
    filePath.setFileDirectory(pLayoutDirStr);
    filePath.setFileName(L"vs");
    filePath.setFileExtension(L"appxrecipe");
    std::string tmpPathStr;
    filePath.asString().asUtf8(tmpPathStr);

    // Allocate and copy output string.
    pPathAsUtf8 = vscAllocateAndCopy(tmpPathStr);

    return filePath.exists();
}
