//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains the functions to detour ExitProcess.
//==============================================================================

#include <windows.h>
#include "..\Common\Logger.h"
#include "DetourExitProcess.h"
#include "PluginInfo.h"
#include "Interceptor.h"
#include "DLLMain.h"

static bool s_bExitProcessAttached = false;
using namespace GPULogger;

typedef VOID(WINAPI* ExitProcess_type)(UINT uExitCode);

ExitProcess_type Real_ExitProcess = NULL;

DECLSPEC_NORETURN VOID WINAPI Mine_ExitProcess(UINT uExitCode)
{
    Log(traceMESSAGE, "Detoured ExitProcess called\n");
    NotifyOnExitProcess();

    if (NULL != Real_ExitProcess)
    {
        Real_ExitProcess(uExitCode);
    }
}

bool DetoursAttachExitProcess()
{
    Real_ExitProcess = ExitProcess;

    if (s_bExitProcessAttached)
    {
        return true;
    }

    LONG error = AMDT::BeginHook();

    if (NO_ERROR == error)
    {
        error |= AMDT::HookAPICall(&(PVOID&)Real_ExitProcess, Mine_ExitProcess);
        error |= AMDT::EndHook();
    }

    s_bExitProcessAttached = true;

    if (NO_ERROR != error)
    {
        return false;
    }

    return true;
}

bool DetoursDetachExitProcess()
{
    if (!s_bExitProcessAttached)
    {
        return true;
    }

    LONG error = AMDT::BeginHook();

    if (NO_ERROR == error)
    {
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_ExitProcess, Mine_ExitProcess);
        error |= AMDT::EndHook();
    }

    s_bExitProcessAttached = false;

    if (NO_ERROR != error)
    {
        return false;
    }

    return true;
}
