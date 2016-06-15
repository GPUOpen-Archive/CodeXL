//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  TODO add documentation
//==============================================================================

#if !defined (_WIN32)

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "SafeCRT.h"
#include "misc.h"

errno_t fopen_s(FILE** pFile, const char* filename, const char* mode)
{
    if (pFile == 0 || filename == 0 || mode == 0)
    {
        return EINVAL;
    }

    *pFile = fopen(filename, mode);

    if (*pFile)
    {
        return errno;
    }

    return 0;
}

char* strtok_s(char* strToken, const char* strDelimit, char** context)
{
    return strtok_r(strToken, strDelimit, context);
}

errno_t memcpy_s(void* dest, size_t numberOfElements, const void* src, size_t count)
{
    if (dest == 0 || src == 0)
    {
        return EINVAL;
    }

    if (numberOfElements < count)
    {
        return ERANGE;
    }

    memcpy(dest, src, count);

    return 0;
}

errno_t strncpy_s(char* strDest, size_t numberOfElements, const char* strSource, size_t count)
{
    if (strDest == 0 || strSource == 0 || numberOfElements == 0)
    {
        return EINVAL;
    }

    if (numberOfElements < count)
    {
        if (numberOfElements >= 1)
        {
            strDest[0] = 0;
        }

        return EINVAL;
    }

    strncpy(strDest, strSource, count);

    return 0;
}

int sscanf_s(const char* buffer, const char* format, ...)
{
    int r = 0;

    if (buffer == 0 || format == 0)
    {
        return -1;
    }

    va_list args;
    va_start(args, format);
    r = vsscanf(buffer, format, args);
    va_end(args);

    return r;
}

int vsprintf_s(char* _DstBuf, size_t _SizeInBytes, const char* _Format, va_list _ArgList)
{
    return vsnprintf(_DstBuf, _SizeInBytes, _Format, _ArgList);
}

int sprintf_s(char* _DstBuf, size_t _SizeInBytes, const char* _Format, ...)
{
    int retVal;
    va_list arg_ptr;

    va_start(arg_ptr, _Format);
    retVal = vsnprintf(_DstBuf, _SizeInBytes, _Format, arg_ptr);
    va_end(arg_ptr);
    return retVal;
}

errno_t strcpy_s(char* _Dst, size_t _SizeInBytes, const char* _Src)
{
    PS_UNREFERENCED_PARAMETER(_SizeInBytes);
    strcpy(_Dst, _Src);
    return 0;
}

errno_t strncat_s(char* _Dst, size_t _SizeInBytes, const char* _Src, rsize_t _MaxCount)
{
    PS_UNREFERENCED_PARAMETER(_SizeInBytes);
    strncat(_Dst, _Src, _MaxCount);
    return 0;
}

errno_t strcat_s(char* _Dst, size_t _SizeInBytes, const char* _Src)
{
    PS_UNREFERENCED_PARAMETER(_SizeInBytes);
    strcat(_Dst, _Src);
    return 0;
}

int _snprintf_s(char* _DstBuf, size_t _SizeInBytes, size_t _MaxCount, const char* _Format, ...)
{
    int retVal;

    if (_MaxCount == _TRUNCATE)
    {
        _MaxCount = _SizeInBytes;
    }

    va_list arg_ptr;

    va_start(arg_ptr, _Format);
    retVal = vsnprintf(_DstBuf, _SizeInBytes, _Format, arg_ptr);
    va_end(arg_ptr);
    return retVal;
}

int vsnprintf_s(char* _DstBuf, size_t _SizeInBytes, size_t _MaxCount, const char* _Format, va_list _ArgList)
{
    PS_UNREFERENCED_PARAMETER(_MaxCount);
    return vsnprintf(_DstBuf, _SizeInBytes, _Format, _ArgList);
}

#endif // !_WIN32
