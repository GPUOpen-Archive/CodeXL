//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscCoreUtils.cpp
///
//==================================================================================

#include "stdafx.h"
#include <src\vscApplicationCommands.h>
#include <Include\Public\vscCoreUtils.h>
#include <Include\vscCoreInternalUtils.h>
#include <AMDTBaseTools\Include\gtAssert.h>
#include <AMDTOSWrappers\Include\osFilePath.h>
#include <AMDTOSWrappers\Include\osDebugLog.h>
#include <src\vscDebugEngine.h>
#include <algorithm>

bool vscIsPathStringsEqual(const wchar_t* pathStrA, const wchar_t* pathStrB)
{
    osFilePath pathA(pathStrA);
    osFilePath pathB(pathStrB);
    return (pathA == pathB);
}

void vscExtractFileExtension(const wchar_t* filePathStr, wchar_t*& pExtensionStrBuffer)
{
    pExtensionStrBuffer = NULL;
    osFilePath filePath(filePathStr);
    gtString str;
    filePath.getFileExtension(str);
    pExtensionStrBuffer = vscAllocateAndCopy(str);
}

bool vscIsPathExists(const wchar_t* pathStr)
{
    bool ret = false;

    if (pathStr != NULL)
    {
        osFilePath path(pathStr);
        ret = path.exists();
    }

    return ret;
}

void vscGetFileDirectoryAsString(const wchar_t* pathStr, wchar_t*& pDirStrBuffer)
{
    pDirStrBuffer = NULL;
    osFilePath filePath(pathStr);
    pDirStrBuffer = vscAllocateAndCopy(filePath.fileDirectoryAsString());
}

wchar_t vscGetOsPathSeparator()
{
    return osFilePath::osPathSeparator;
}

bool vscStartsWith(const wchar_t* str, const wchar_t* substring)
{
    gtString gtStr(str);
    return gtStr.startsWith(substring);
}

void vscPrintErrorMsgToDebugLog(const wchar_t* msg)
{
    OS_OUTPUT_DEBUG_LOG(msg, OS_DEBUG_LOG_DEBUG);
}


void vscPrintDebugMsgToDebugLog(const wchar_t* msg)
{
    OS_OUTPUT_DEBUG_LOG(msg, OS_DEBUG_LOG_DEBUG);
}

bool vscGetLastModifiedDate(const wchar_t* pFileNameStr, time_t& result)
{
    bool ret = false;

    if (pFileNameStr != NULL)
    {
        // Get the date of the file
        gtString strFP(pFileNameStr);
        osFilePath fp(strFP);
        osFile file(fp);
        osStatStructure fileProperties;

        // Get the files status
        int rc1 = osWStat(pFileNameStr, fileProperties);
        GT_IF_WITH_ASSERT(rc1 == 0)
        {
            result = fileProperties.st_mtime;
            ret = true;
        }
    }

    return ret;
}

bool vscIsCodeXLServerDll(const wchar_t* dllFullPath)
{
    bool isCodeXLDll = false;

    DWORD dwDummy = 0;
    DWORD dwSize = ::GetFileVersionInfoSize(dllFullPath, &dwDummy);

    if (dwSize > 0)
    {
        // Allocate a memory for the version information:
        LPBYTE versionInfoBuffer = (LPBYTE)malloc(dwSize);


        BOOL rcGetInfo = ::GetFileVersionInfo(dllFullPath, 0, dwSize, versionInfoBuffer);
        GT_IF_WITH_ASSERT(rcGetInfo == TRUE)
        {
            gtString keyString;

            keyString.append(L"\\StringFileInfo\\");
            keyString.append(L"040904b0\\"); // block from VSPackageVersionInfo.rc2
            keyString.append(L"ProductName");

            UINT sectionLength = 0;
            LPVOID sectionPointer = NULL;

            if (::VerQueryValue((void*)versionInfoBuffer, keyString.asCharArray(), &sectionPointer, &sectionLength))
            {
                gtString valueString;
                valueString.append((wchar_t*)sectionPointer, sectionLength);

                // Remove trailing 0 character so name comparison can be done
                if (valueString[sectionLength - 1] == 0)
                {
                    valueString.removeTrailing(0);
                }

                static const gtString codexlProductNameLowercase = L"codexl";
                valueString.toLowerCase();

                if (-1 != valueString.find(codexlProductNameLowercase))
                {
                    isCodeXLDll = true;
                }
            }
        }
        free(versionInfoBuffer);
    }

    return isCodeXLDll;
}

int vscDllRegisterServer()
{
    HRESULT hr = vscRegisterDebugEngine();

    return (int)hr;
}

int vscDllUnregisterServer()
{
    HRESULT hr = vscUnregisterDebugEngine();

    return (int)hr;
}

void vscGetFileName(const wchar_t* pathStr, wchar_t*& pFileNameStrBuffer)
{
    pFileNameStrBuffer = NULL;
    osFilePath filePath(pathStr);
    gtString fileName;

    if (filePath.getFileName(fileName))
    {
        pFileNameStrBuffer = vscAllocateAndCopy(fileName);
    }
}

void vscDeleteWcharString(wchar_t*& pStr)
{
    delete[] pStr;
    pStr = NULL;
}

void vscDeleteWcharStringArray(wchar_t**& pStr)
{
    delete[] pStr;
    pStr = NULL;
}

void vscDeleteCharString(char*& pStr)
{
    delete[] pStr;
    pStr = NULL;
}

void vscDeleteUintBuffer(unsigned int*& pBuffer)
{
    delete[] pBuffer;
    pBuffer = NULL;
}

wchar_t vscGetOsExtensionSeparator()
{
    return osFilePath::osExtensionSeparator;
}

void vscApplicationCommands_SetOwner(IVscApplicationCommandsOwner* pOwner)
{
    vscApplicationCommands::setOwner(pOwner);
}
