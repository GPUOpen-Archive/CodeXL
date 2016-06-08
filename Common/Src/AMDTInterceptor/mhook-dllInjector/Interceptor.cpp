//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Interceptor Interface function definitions
//==============================================================================

#include <Windows.h>
#include <AMDTInterceptor/Interceptor.h>
#include <mhook-lib/mhook.h>

// ---------------------------------------------------------------------------
/// Initialize anything before beginning API Hooking
/// \return 0 if successful, non-zero for error
// ---------------------------------------------------------------------------
LONG AMDT::BeginHook()
{
    Mhook_SuspendOtherThreads();
    return 0;
}

// ---------------------------------------------------------------------------
/// Perform any cleanup after API Hooking
/// \return 0 if successful, non-zero for error
// ---------------------------------------------------------------------------
LONG AMDT::EndHook()
{
    Mhook_ResumeOtherThreads();
    return 0;
}

// ---------------------------------------------------------------------------
/// Hook an API call.
/// \param ppRealFn The real function pointer
/// \param pMineFn The Hooked function pointer
/// \return 0 if successful, non-zero for error
// ---------------------------------------------------------------------------
LONG WINAPI AMDT::HookAPICall(_Inout_ PVOID* ppRealFn, _In_ PVOID pMineFn)
{
    if (Mhook_SetHook(ppRealFn, pMineFn) == TRUE)
    {
        return 0;
    }

    return 1;
}

// ---------------------------------------------------------------------------
/// Unhook an API call.
/// \param ppRealFn The real function pointer
/// \param pMineFn The Hooked function pointer
/// \return 0 if successful, non-zero for error
// ---------------------------------------------------------------------------
LONG WINAPI AMDT::UnhookAPICall(_Inout_ PVOID* ppRealFn, _In_ PVOID pMineFn)
{
    UNREFERENCED_PARAMETER(pMineFn);

    if (Mhook_Unhook(ppRealFn) == TRUE)
    {
        return 0;
    }

    return 1;
}
