//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfilingControlDLLBuild.h
///
//==================================================================================

//------------------------------ CpuProfilingControlDLLBuild.h ------------------------------

#ifndef _CPUPROFILINGCONTROLDLLBUILD_H_
#define _CPUPROFILINGCONTROLDLLBUILD_H_

// Under Win32 builds - define: CP_CTRL_API to be:
// - When building AMDTCpuProfilingControl.dll: __declspec(dllexport).
// - When building other projects:              __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDT_CPU_PROFILING_CONTROL_EXPORTS)
        #define CP_CTRL_API __declspec(dllexport)
    #else
        #define CP_CTRL_API __declspec(dllimport)
    #endif
#else
    #define CP_CTRL_API
#endif


#endif  // _CPUPROFILINGCONTROLDLLBUILD_H_
