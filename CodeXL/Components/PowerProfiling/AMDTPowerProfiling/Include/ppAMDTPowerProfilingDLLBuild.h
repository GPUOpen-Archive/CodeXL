//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppAMDTPowerProfilingDLLBuild.h
///
//==================================================================================

//------------------------------ ppAMDPowerProfilingDLLBuild.h ------------------------------

#ifndef __PPAMDPOWERPROFILINGDLLBUILD_H
#define __PPAMDPOWERPROFILINGDLLBUILD_H

// Under Win32 builds - define: GW_API to be:
// - When building AMDTPowerProfiling.dll:     __declspec(dllexport).
// - When building other projects:     __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDTPOWERPROFILING_EXPORTS)
        #define PP_API __declspec(dllexport)
    #else
        #define PP_API __declspec(dllimport)
    #endif
#else
    #define PP_API
#endif


#endif  // __PPAMDPOWERPROFILINGDLLBUILD_H
