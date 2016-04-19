//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfilingRawDataDLLBuild.h
///
//==================================================================================

//------------------------------ CpuProfilingRawDataDLLBuild.h ------------------------------

#ifndef _CPUPROFILINGRAWDATADLLBUILD_H_
#define _CPUPROFILINGRAWDATADLLBUILD_H_

// Under Win32 builds - define: CP_RAWDATA_API to be:
// - When building AMDTCpuProfilingRawData.dll: __declspec(dllexport).
// - When building other projects:              __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDT_CPU_PROFILING_RAWDATA_EXPORTS)
        #define CP_RAWDATA_API __declspec(dllexport)
    #else
        #define CP_RAWDATA_API __declspec(dllimport)
    #endif
#else
    #define CP_RAWDATA_API
#endif


#endif  // _CPUPROFILINGRAWDATADLLBUILD_H_
