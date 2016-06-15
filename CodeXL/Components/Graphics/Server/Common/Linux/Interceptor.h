//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Linux specific implementation of DLL Injection (just process creation
/// since the 'injection' is handled by LD_PRELOAD)
//==============================================================================

#ifndef _INTERCEPTOR_H
#define _INTERCEPTOR_H
#include "WinDefs.h"
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osProcess.h>

struct PROCESS_INFORMATION
{
    osProcessHandle hProcess;
    osThreadHandle hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
};

BOOL CreateProcess(const char* lpApplicationName,
                   const char* lpCommandLine,
                   const char* lpCurrentDirectory,
                   PROCESS_INFORMATION* lpProcessInformation
                  );

#endif // def _INTERCEPTOR_H
