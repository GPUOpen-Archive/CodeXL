//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file appApplicationDLLBuild.h
///
//==================================================================================

#ifndef __APPAPPLICATIONDLLBUILD_H
#define __APPAPPLICATIONDLLBUILD_H

// Under Win32 builds - define: AF_API to be:
// - When building APIClasses.dll:     __declspec(dllexport).
// - When building other projects:     __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDTAPPLICATION_EXPORTS)
        #define APP_API __declspec(dllexport)
    #else
        #define APP_API __declspec(dllimport)
    #endif
#else
    #define APP_API
#endif

#endif //__APPAPPLICATIONDLLBUILD_H

