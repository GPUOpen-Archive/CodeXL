//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#ifndef _DC_SERVER_H_
#define _DC_SERVER_H_

/// \defgroup DCServer DCServer
/// This module is the DirectCompute backend for the profiler.
/// It provides the following services: \n
/// 1) Injecting code into a DirectCompute application through the use of
/// Microsoft Detours and virtual table patching. \n
/// 2) Interfacing with the GPUPerfAPI library to query and output
/// GPU performance counters. \n
/// 3) Keeping track of resources and creating backup resources to be used
/// when dispatching kernel multiple times. \n
/// 4) Recording commands executed on deferred contexts, flatten them and
/// execute them on the immediate context. \n
/// 5) Retrieving the DX assembly for the kernel.
///
/// \ingroup Backend
// @{
// @}

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the DCSERVER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// DCSERVER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef DCSERVER_EXPORTS
    #define DCSERVER_EXPORTS  __declspec( dllexport )
#else
    #define DCSERVER_EXPORTS  __declspec( dllexport )
#endif

#ifndef DCSERVER_PLUGIN_EXPORTS
    #define DCSERVER_PLUGIN_EXPORTS extern "C" __declspec( dllexport )
#endif

DCSERVER_PLUGIN_EXPORTS bool UpdateHooks();

#endif // _DC_SERVER_H_

