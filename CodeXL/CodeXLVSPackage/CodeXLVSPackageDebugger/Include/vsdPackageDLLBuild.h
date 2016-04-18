//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdPackageDLLBuild.h
///
//==================================================================================

//------------------------------ vsdPackageDLLBuild.h ------------------------------

#ifndef __VSDPACKAGEDLLBUILD_H
#define __VSDPACKAGEDLLBUILD_H

// Under Win32 builds - define: VSD_API to be:
// - When building CodeXLVSPackageDebugger.dll:     __declspec(dllexport).
// - When building other projects:                  __declspec(dllimport).

#if defined(_WIN32)
    #if defined(VSPACKAGEDEBUGGERDLL_EXPORTS)
        #define VSD_API __declspec(dllexport)
    #else
        #define VSD_API __declspec(dllimport)
    #endif
#else
    #define VSD_API
#endif

#endif //__VSDPACKAGEDLLBUILD_H
