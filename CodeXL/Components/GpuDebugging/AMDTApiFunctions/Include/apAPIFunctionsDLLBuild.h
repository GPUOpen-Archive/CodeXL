//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apAPIFunctionsDLLBuild.h
///
//==================================================================================

//------------------------------ apAPIFunctionsDLLBuild.h ------------------------------

#ifndef __APAPIFUNCTIONSDLLBUILD_H
#define __APAPIFUNCTIONSDLLBUILD_H

// Under Win32 builds - define: AP_API to be:
// - When building APIFunctions.dll:     __declspec(dllexport).
// - When building other projects:     __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDTAPIFUNCTIONS_EXPORTS)
        #define GA_API __declspec(dllexport)
    #else
        #define GA_API __declspec(dllimport)
    #endif
#else
    #define GA_API
#endif


#endif //__APAPIFUNCTIONSDLLBUILD_H

