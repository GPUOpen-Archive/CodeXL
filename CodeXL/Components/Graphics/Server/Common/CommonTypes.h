//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Definitions of common types.
//==============================================================================

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#if defined _WIN32
    #include <Windows.h>

    typedef LARGE_INTEGER      GPS_TIMESTAMP;

    //Signed
    typedef char char8;
    typedef wchar_t char16;

    typedef char    int8;
    typedef short   int16;
    typedef int     int32;
    typedef __int64 int64;

    typedef float       float32;
    typedef double      float64;

    //Unsigned
    typedef unsigned char    uint8;
    typedef unsigned short   uint16;
    typedef unsigned int     uint32;
    typedef unsigned __int64 uint64;

#elif defined _LINUX
    #include <inttypes.h>
    #include <WinDefs.h>

    typedef LARGE_INTEGER      GPS_TIMESTAMP;
    typedef int8_t             INT8;
    typedef int16_t            INT16;
    typedef int32_t            INT32;
    typedef int64_t            INT64;
    typedef uint8_t            UINT8;
    typedef uint16_t           UINT16;
    typedef uint32_t           UINT32;
    typedef unsigned long long UINT64;
#endif

/// Enumeration for the stages of the pipeline
enum PIPELINE_STAGE
{
    PIPELINE_STAGE_NONE,
    PIPELINE_STAGE_IA,
    PIPELINE_STAGE_CS,
    PIPELINE_STAGE_VS,
    PIPELINE_STAGE_HS,
    PIPELINE_STAGE_DS,
    PIPELINE_STAGE_GS,
    PIPELINE_STAGE_PS,
    PIPELINE_STAGE_OM,
    PIPELINE_STAGE_SC,
};

#endif // COMMON_TYPES_H
