//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuCallstackSamplingDLLBuild.h
///
//==================================================================================

//------------------------------ CpuCallstackSamplingDLLBuild.h ------------------------------

#ifndef _CPUPCALLSTACKSAMPLINGDLLBUILD_H_
#define _CPUPCALLSTACKSAMPLINGDLLBUILD_H_

// Under Win32 builds - define: CP_CSS_API to be:
// - When building AMDTCpuCallstackSampling.dll: __declspec(dllexport).
// - When building other projects:               __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDT_CSS_EXPORTS)
        #define CP_CSS_API __declspec(dllexport)
    #else
        #define CP_CSS_API __declspec(dllimport)
    #endif
#else
    #define CP_CSS_API
#endif

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

class ProcessWorkingSetQuery
{
public:
    virtual class ExecutableFile* FindModule(gtVAddr va) = 0;
    virtual unsigned ForeachModule(void (*pfnProcessModule)(class ExecutableFile&, void*), void* pContext) = 0;
};

#endif  // _CPUPCALLSTACKSAMPLINGDLLBUILD_H_
