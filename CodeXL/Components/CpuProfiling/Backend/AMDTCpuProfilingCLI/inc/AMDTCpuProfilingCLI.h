//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTCpuProfilingCLI.h
/// \brief This is Command Line Utility for CPU profiling.
///
//==================================================================================

#ifndef _AMDT_CPUPROFILING_CLI_H_
#define _AMDT_CPUPROFILING_CLI_H_

#pragma once

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osCpuid.h>

//
//     Macros
//

// un-comment this to enable Thread profiling "-m tp"
#define CXL_ENABLE_TP   1

// Name of the cpu-profiler Command Line tool
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define CODEXL_CPUPROFILER_CLI    "CodeXLCpuProfiler.exe"
    #define CLASSIC_EXAMPLE           "classic.exe"
#else
    #define CODEXL_CPUPROFILER_CLI    "CodeXLCpuProfiler"
    #define CLASSIC_EXAMPLE           "classic"
#endif

typedef enum CpuProfileCommands
{
    CPUPROF_COMMAND_UNDEFINED = 0,
    CPUPROF_COMMAND_COLLECT,
    CPUPROF_COMMAND_TRANSLATE,
    CPUPROF_COMMAND_REPORT,
    CPUPROF_COMMAND_UNKNOWN,
} CpuProfileCommand;


#endif // _AMDT_CPUPROFILING_CLI_H_
