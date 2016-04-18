//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwgDEBuggerAppWrapperDLLBuild.h
///
//==================================================================================

//------------------------------ gwCodeXLAppWrapperDLLBuild.h ------------------------------

#ifndef __GWCodeXLAPPWRAPPERDLLBUILD_H
#define __GWCodeXLAPPWRAPPERDLLBUILD_H

// Under Win32 builds - define: GW_API to be:
// - When building AMDTCodeXLAppWrapper.dll:     __declspec(dllexport).
// - When building other projects:     __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDTGPUDEBUGGING_EXPORTS)
        #define GW_API __declspec(dllexport)
    #else
        #define GW_API __declspec(dllimport)
    #endif
#else
    #define GW_API
#endif


#endif  // __GWCodeXLAPPWRAPPERDLLBUILD_H
