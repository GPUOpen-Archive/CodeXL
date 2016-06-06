//==============================================================================
// Copyright (c) 2009-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Include this file prior to including GPUPerfAPI.h under linux or
///         use GPUPerfAPI-Internal.h instead
//==============================================================================

#ifndef _GPUPERFAPIOS_H_
#define _GPUPERFAPIOS_H_

#ifdef _LINUX

    #ifdef GPALIB_DECL
    #else
        #ifdef __cplusplus
            #define GPALIB_DECL extern "C"
        #else
            #define GPALIB_DECL
        #endif // _cplusplus
    #endif

    typedef char    gpa_int8;
    typedef short   gpa_int16;
    typedef int     gpa_int32;
    typedef long long gpa_int64;
    typedef unsigned int UINT;
    typedef float   gpa_float32;
    typedef double  gpa_float64;
    typedef unsigned char    gpa_uint8;
    typedef unsigned short   gpa_uint16;
    typedef unsigned int     gpa_uint32;
    typedef unsigned long long gpa_uint64;
    #ifndef __cplusplus
        typedef gpa_uint8 bool;
    #endif

    // Linux
    #define UNREFERENCED_PARAMETER(x)
    #define UNREFERECED_VAR(x)

    #define _strcmpi(a, b) strcasecmp(a, b)

    // for now, just use non secure version for Linux
    #define strcpy_s(dst, ndst, src) strcpy(dst, src)
    #define strcat_s(dst, ndst, src) strcat(dst, src)
    #define strtok_s(a, b, c) strtok(a, b)

#endif // _LINUX

#endif // _GPUPERFAPIOS_H_

