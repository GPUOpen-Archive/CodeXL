//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file OsFileWrapper.cpp
///
//==================================================================================

#if ( defined (_WIN32) || defined (_WIN64) )
    #include <Windows.h>
#endif

#ifdef _LINUX
    #include <sys/time.h>
#endif
#include <clocale>
#include <cstdlib>     /* wcstombs, wchar_t(C) */
#include<cstring>

#include <AMDTDefinitions.h>
#include "OsFileWrapper.h"
//OSwrapper

bool PwrOpenFile(FILE** m_hFile, const char* pFileName, const char* pMode)
{
    bool ret = true;
#if ( defined (_WIN32) || defined (_WIN64) )

    if (0 != fopen_s(m_hFile, pFileName, pMode))
    {
        return false;
    }

#endif
#ifdef LINUX
    *m_hFile = fopen(pFileName, pMode);

    if (nullptr == *m_hFile)
    {
        ret = false;
    }

#endif
    return ret;
}

bool PwrWriteFile(FILE* m_hFile, AMDTUInt8* pData, AMDTInt32 len)
{
    bool ret = true;
    size_t size = 0;
    size = fwrite(pData, sizeof(AMDTUInt8), (size_t)len, m_hFile);

    if (size != (AMDTUInt32)len)
    {
        ret = false;
    }

    return ret;
}

bool PwrReadFile(FILE* m_hFile, AMDTUInt8* pData, AMDTInt32 len)
{
    bool ret = true;
    size_t size = 0;
    size = fread(pData, 1, (size_t)len, m_hFile);

    if (size != (size_t)len)
    {
        ret = false;
    }

    return ret;
}

bool PwrSeekFile(FILE* m_hFile, long size, AMDTInt32 mode)
{
    AMDTInt32 ret = 0;
    ret = fseek(m_hFile, size, mode);

    if (ret != size)
    {
        ret = false;
    }

    return true;
}

bool PwrCloseFile(FILE* m_hFile)
{
    bool ret = AMDT_STATUS_OK;

    if (m_hFile)
    {
        fclose(m_hFile);
    }

    return ret;
}

AMDTUInt64 GetOsTimeStamp(void)
{

#if ( defined (_WIN32) || defined (_WIN64) )
    return GetTickCount();
#endif
#ifdef _LINUX
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
#endif
}

