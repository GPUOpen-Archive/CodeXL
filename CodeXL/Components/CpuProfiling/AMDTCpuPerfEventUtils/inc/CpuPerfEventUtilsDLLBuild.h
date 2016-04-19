//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuPerfEventUtilsDLLBuild.h
///
//==================================================================================

//------------------------------ CpuPerfEventUtilsDLLBuild.h ------------------------------

#ifndef _CPUPERFEVENTUTILSDLLBUILD_H_
#define _CPUPERFEVENTUTILSDLLBUILD_H_

// Under Win32 builds - define: CP_EVENT_API to be:
// - When building AMDTCpuPerfEventUtils.dll: __declspec(dllexport).
// - When building other projects:            __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDT_CPUPERFEVENTUTILS_EXPORTS)
        #define CP_EVENT_API __declspec(dllexport)
    #else
        #define CP_EVENT_API __declspec(dllimport)
    #endif
#else
    #define CP_EVENT_API
#endif


#endif  // _CPUPERFEVENTUTILSDLLBUILD_H_
