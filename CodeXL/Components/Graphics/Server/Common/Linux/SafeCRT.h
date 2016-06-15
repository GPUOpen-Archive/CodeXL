//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  TODO add documentation
//==============================================================================

#ifndef GPS_SAFECRT_H
#define GPS_SAFECRT_H

#include <stdio.h>

#if !defined (_WIN32)
    #include <errno.h>
    #include <string.h>
    #include "WinDefs.h"

    typedef int errno_t;
    typedef size_t rsize_t;

    errno_t fopen_s(FILE** pFile, const char* filename, const char* mode);
    char* strtok_s(char* strToken, const char* strDelimit, char** context);
    errno_t memcpy_s(void* dest, size_t numberOfElements, const void* src, size_t count);
    errno_t strncpy_s(char* strDest, size_t numberOfElements, const char* strSource, size_t count);
    int sscanf_s(const char* buffer, const char* format, ...);
    int vsprintf_s(char* _DstBuf, size_t _SizeInBytes, const char* _Format, va_list _ArgList);
    int sprintf_s(char* _DstBuf, size_t _SizeInBytes, const char* _Format, ...);
    errno_t strcpy_s(char* _Dst, size_t _SizeInBytes, const char* _Src);
    errno_t strncat_s(char* _Dst, size_t _SizeInBytes, const char* _Src, rsize_t _MaxCount);
    errno_t strcat_s(char* _Dst, size_t _SizeInBytes, const char* _Src);
    int _snprintf_s(char* _DstBuf, size_t _SizeInBytes, size_t _MaxCount, const char* _Format, ...);
    int vsnprintf_s(char* _DstBuf, size_t _SizeInBytes, size_t _MaxCount, const char* _Format, va_list _ArgList);

    #define _stricmp strcasecmp
    #define _strnicmp strncasecmp

#endif // !_WIN32

#endif // GPS_SAFECRT_H
