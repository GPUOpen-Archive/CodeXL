//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SymbolEngine.cpp
///
//==================================================================================

#include "SymbolEngine.h"
#include <AMDTOSWrappers/Include/osFilePath.h>
#if AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
    #include <sys/stat.h>
#endif

static unsigned VerifyFile(const wchar_t* pPath, size_t lenPath, const wchar_t* pFile, size_t lenFile, wchar_t* pBuffer);

unsigned SymbolEngine::FindFile(const wchar_t* pSearchPath, const wchar_t* pFileName, wchar_t* pBuffer)
{
    unsigned len = 0U;

    if (NULL == pSearchPath)
    {
        pSearchPath = L"";
    }

    size_t lenFileName = wcslen(pFileName);
    const wchar_t* pChr = pSearchPath;
    const wchar_t* pChrNext;

    while (NULL != (pChrNext = wcschr(pChr, osFilePath::osEnvironmentVariablePathsSeparator)))
    {
        size_t lenPath = pChrNext - pChr;
        len = VerifyFile(pChr, lenPath, pFileName, lenFileName, pBuffer);

        if (0U != len)
        {
            break;
        }

        pChr = pChrNext + 1;
    }

    if (0U == len)
    {
        size_t lenPath = wcslen(pChr);
        len = VerifyFile(pChr, lenPath, pFileName, lenFileName, pBuffer);
    }

    return len;
}

static unsigned VerifyFile(const wchar_t* pPath, size_t lenPath, const wchar_t* pFile, size_t lenFile, wchar_t* pBuffer)
{
    if (0 != lenPath)
    {
        memcpy(pBuffer, pPath, lenPath * sizeof(wchar_t));
        pBuffer[lenPath++] = osFilePath::osPathSeparator;
    }

    memcpy(pBuffer + lenPath, pFile, lenFile * sizeof(wchar_t));
    lenPath += lenFile;
    pBuffer[lenPath] = L'\0';

    bool exists;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    struct _stat st;
    exists = (_wstat(pBuffer, &st) == 0);
#else
    char pathMb[OS_MAX_PATH];

    if (size_t(-1) != wcstombs(pathMb, pBuffer, OS_MAX_PATH))
    {
        struct stat st;
        exists = (stat(pathMb, &st) == 0);
    }
    else
    {
        exists = false;
    }

#endif
    return exists ? static_cast<unsigned>(lenPath) : 0U;
}
