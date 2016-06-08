//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaOSAPIWrappersDLLBuild.h
///
//=====================================================================

//------------------------------ oaOSAPIWrappersDLLBuild.h ------------------------------

#ifndef __OAOSWRAPPERSDLLBUILD
#define __OAOSWRAPPERSDLLBUILD

// Under Win32 builds - define: OW_API to be:
// - When building OSAPIWrappers.lib:     default
// - When building OSAPIWrappersDLL.dll:  __declspec(dllexport).
// - When building other projects:        __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDTOSAPIWRAPPERS_EXPORTS)
        #define OA_API __declspec(dllexport)
    #elif defined(AMDTOSAPIWRAPPERS_STATIC)
        #define OA_API
    #else
        #define OA_API __declspec(dllimport)
    #endif
#else
    #define OA_API
#endif


#endif  // __OAOSAPIWRAPPERSDLLBUILD
