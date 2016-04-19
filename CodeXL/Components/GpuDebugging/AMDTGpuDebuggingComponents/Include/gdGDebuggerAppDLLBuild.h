//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdGDebuggerAppDLLBuild.h
///
//==================================================================================

//------------------------------ gdCodeXLAppDLLBuild.h ------------------------------

#ifndef __GDCodeXLAPPDLLBUILD
#define __GDCodeXLAPPDLLBUILD

// Under Win32 builds - define: GD_API to be:
// - When building gdCodeXLApp.dll:     __declspec(dllexport).
// - When building other projects:          __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDTGPUDEBUGGINGCOMPONENTS_EXPORTS)
        #define GD_API __declspec(dllexport)
    #else
        #define GD_API __declspec(dllimport)
    #endif
#else
    #define GD_API
#endif


#endif  // __GDCodeXLAPPDLLBUILD
