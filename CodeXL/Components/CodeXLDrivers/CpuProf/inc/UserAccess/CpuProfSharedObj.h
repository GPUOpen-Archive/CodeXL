//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfDriverSharedObj.h
/// \brief  Shared Object related macros used by CpuProf Driver, Backend
///         and CodeXLDriversLoadService.
///
//==================================================================================

#ifndef _CPUPROFSHAREDOBJ_H
#define _CPUPROFSHAREDOBJ_H

#include <stdlib.h>

///The maximum number of clients that CpuProf supports
#define MAX_CLIENT_COUNT 8

/// \struct SHARED_CLIENT Holds one client's shared information
typedef struct
{
    /// Whether the client is paused
    BOOLEAN paused;
    /// The string 'pause key' for a profile
    wchar_t pauseKey[_MAX_PATH];
    /// Process ID of the client
    DWORD clientPid;
    //Possible to be expanded later if something low-latency is required
} SHARED_CLIENT, *PSHARED_CLIENT;

/// \def CPU_PROF_SHARED_OBJ The shared object that various instances of
/// Cpu profilers will use to signal a pause state during sampling
#define CPU_PROF_SHARED_OBJ L"Global\\AMD_CPUPROF_SHARED_OBJ"

/// \def CA_SHARED_MEM_SIZE The size of the shared memory
#define CPU_PROF_SHARED_MEM_SIZE (sizeof(SHARED_CLIENT) * MAX_CLIENT_COUNT)

#endif // _CPUPROFSHAREDOBJ_H