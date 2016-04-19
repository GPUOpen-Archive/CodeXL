//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProcessDebuggerDLLBuild.h
///
//==================================================================================

//------------------------------ ProcessDebuggerDLLBuild.h ------------------------------

#ifndef __PROCESSDEBUGGERDLLBUILD
#define __PROCESSDEBUGGERDLLBUILD

// Under Win32 builds - define: PD_API to be:
// - When building GRProcessDebugger.dll:  __declspec(dllexport).
// - When building other projects:        __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDTPROCESSDEBUGGER_EXPORTS)
        #define PD_API __declspec(dllexport)
    #else
        #define PD_API __declspec(dllimport)
    #endif
#else
    #define PD_API
#endif



#endif  // __PROCESSDEBUGGERDLLBUILD
