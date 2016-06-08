//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ExecutableFormatDLLBuild.h
///
//==================================================================================

//------------------------------ ExecutableFormatDLLBuild.h ------------------------------

#ifndef _EXECUTABLEFORMATDLLBUILD_H_
#define _EXECUTABLEFORMATDLLBUILD_H_

// Under Win32 builds - define: EXE_API to be:
// - When building AMDTExecutableFormat.dll: __declspec(dllexport).
// - When building other projects:           __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDT_EXE_EXPORTS)
        #define EXE_API __declspec(dllexport)
    #else
        #define EXE_API __declspec(dllimport)
    #endif
#else
    #define EXE_API
#endif

#if defined(_WIN32)
    const wchar_t* GetExecutableFormatModulePath();
#endif

#endif  // _EXECUTABLEFORMATDLLBUILD_H_
