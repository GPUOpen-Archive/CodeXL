//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfilingTranslationDLLBuild.h
///
//==================================================================================

//------------------------------ CpuProfilingTranslationDLLBuild.h ------------------------------

#ifndef _CPUPROFILINGTRANSLATIONDLLBUILD_H_
#define _CPUPROFILINGTRANSLATIONDLLBUILD_H_

// Under Win32 builds - define: CP_TRANS_API to be:
// - When building AMDTCpuProfilingTranslation.dll: __declspec(dllexport).
// - When building other projects:                  __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDT_CPU_PROFILING_TRANSLATION_EXPORTS)
        #define CP_TRANS_API __declspec(dllexport)
    #else
        #define CP_TRANS_API __declspec(dllimport)
    #endif
#else
    #define CP_TRANS_API
#endif


#endif  // _CPUPROFILINGTRANSLATIONDLLBUILD_H_
