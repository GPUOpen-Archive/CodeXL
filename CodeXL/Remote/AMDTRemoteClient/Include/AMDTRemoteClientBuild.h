//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTRemoteClientBuild.h
///
//==================================================================================

//------------------------------ AMDTRemoteClientBuild.h ------------------------------

#ifndef __AMDTREMOTECLIENTDLLBUILD
#define __AMDTREMOTECLIENTDLLBUILD

// Under Win32 builds - define: AP_API to be:
// - When building APIClasses.dll:     __declspec(dllexport).
// - When building other projects:     __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDTREMOTECLIENT_EXPORTS)
        #define AMDT_REMOTE_CLIENT_API __declspec(dllexport)
    #else
        #define AMDT_REMOTE_CLIENT_API __declspec(dllimport)
    #endif
#else
    #define AMDT_REMOTE_CLIENT_API
#endif


#endif  // __AMDTREMOTECLIENTDLLBUILD
