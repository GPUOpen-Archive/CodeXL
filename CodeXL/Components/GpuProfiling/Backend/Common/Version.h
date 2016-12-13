//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains the version number for the CodeXL GPU Profiler backend.
//==============================================================================

#ifndef _VERSION_H_
#define _VERSION_H_

/// \defgroup Backend Backend
/// This is the backend of the CodeXL GPU Profiler.
/// It handles code injection to the application process,
/// gathering GPU performance counters through GPUPerfAPI, collecting
/// statistics from the DX and CL run-time.

/// \defgroup sprofile sprofile
/// This is the console application for the backend.
/// It handles command line inputs from the user or client and use Detours
/// and virtual table patching to inject into user application
///
/// \ingroup Backend
// @{

#ifdef AMDT_INTERNAL
    #define GPUPROFILER_BACKEND_EXE_NAME "CodeXLGpuProfiler-Internal"
#elif defined( AMDT_NDA )
    #define GPUPROFILER_BACKEND_EXE_NAME "CodeXLGpuProfiler-NDA"
#else // public version
    #define GPUPROFILER_BACKEND_EXE_NAME "CodeXLGpuProfiler"
#endif

#define GPUPROFILER_BACKEND_MAJOR_VERSION 4
#define GPUPROFILER_BACKEND_MINOR_VERSION 0
#define GPUPROFILER_BACKEND_BUILD_NUMBER 0

#define GPUPROFILER_STRINGIFY(ARG) #ARG
#define GPUPROFILER_JOIN_STRINGS(ARG1,ARG2) GPUPROFILER_STRINGIFY(ARG1) GPUPROFILER_STRINGIFY(ARG2)

#define GPUPROFILER_BACKEND_VERSION GPUPROFILER_BACKEND_MAJOR_VERSION.GPUPROFILER_BACKEND_MINOR_VERSION.GPUPROFILER_BACKEND_BUILD_NUMBER

#define GPUPROFILER_BACKEND_VERSION_STRING GPUPROFILER_JOIN_STRINGS(V,GPUPROFILER_BACKEND_VERSION)

// @}

#endif
