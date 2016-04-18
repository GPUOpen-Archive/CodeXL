//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afApplicationFrameworkDLLBuild.h
///
//==================================================================================

#ifndef __AFAPPLICATIONFRAMEWORKDLLBUILD_H
#define __AFAPPLICATIONFRAMEWORKDLLBUILD_H

// Under Win32 builds - define: AF_API to be:
// - When building APIClasses.dll:     __declspec(dllexport).
// - When building other projects:     __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDTApplicationFramework_EXPORTS)
        #define AF_API __declspec(dllexport)
    #else
        #define AF_API __declspec(dllimport)
    #endif
#else
    #define AF_API
#endif

#endif //__AFAPPLICATIONFRAMEWORKDLLBUILD_H

