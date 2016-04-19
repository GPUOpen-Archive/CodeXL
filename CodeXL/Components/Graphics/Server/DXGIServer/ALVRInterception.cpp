//=============================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief Hooking support functions for ALVR
//=============================================================

#include "../Common/LiquidVRSupport.h"

#ifdef LIQUID_VR_SUPPORT

#include <windows.h>
#include <Interceptor.h>
#include <stdio.h>
#include "../Common/PerfStudioServer_Version.h"
#include "../Common/logger.h"
#include "ALVRInterception.h"
#include "ALVRFactoryWrapper.h"

/// Our Init function ptr
static ALVRInit_Fn Real_fnInit = NULL;

/// Intercept the Init function that creates factory
/// \param version ALVR Version
/// \param ppFactory ALVR Factory
/// \return ALVR error code
ALVR_RESULT Mine_fnInit(uint64_t version, void** ppFactory)
{
    // Compare our version of LiquidVR to the one being asked for
    if (ALVR_FULL_VERSION != version)
    {
        // The application and GPS are NOT using the same version of LiquidVR
        uint16 alvr_build = (uint16)(version & 0xFFFFFFFF);
        uint16 alvr_release = (uint16)((version >> 16ull) & 0xFFFFFFFF);
        uint16 alvr_minor = (uint16)((version >> 32ull) & 0xFFFFFFFF);
        uint16 alvr_major = (uint16)((version >> 48ull) & 0xFFFFFFFF);

        Log(logERROR, "The application and GPU PerfStudio are using different versions of the LiquidVR interface.\n");
        Log(logERROR, "The application is requesting LiquidVR version %d.%d.%d.%d\n", alvr_major, alvr_minor, alvr_release, alvr_build);
        Log(logERROR, "This version of GPU PerfStudio supports LiquidVR version %d.%d.%d.%d\n", ALVR_VERSION_MAJOR, ALVR_VERSION_MINOR, ALVR_VERSION_RELEASE, ALVR_VERSION_BUILD);

        ALVR_RESULT res = Real_fnInit(version, ppFactory);

        return res;
    }
    else
    {
        // The application and GPS are using the same version of LiquidVR
        ALVR_RESULT res = Real_fnInit(version, ppFactory);

        // Wrap the factory
        ALVRFactoryWrapper* pWrapper = new ALVRFactoryWrapper(static_cast<ALVRFactory*>(*ppFactory));

        // Hand the wrapper back to the app
        *ppFactory = pWrapper;

        return res;
    }
}

/// Hook the Init function so we can intercept it
void HookLiquidVR()
{
    LogTrace(traceENTER, "");

    if (Real_fnInit != NULL)
    {
        LogTrace(traceEXIT, "");
        return;
    }

    HMODULE hLiquidVRDLL = LoadLibraryW(ALVR_DLL_NAME);

    if (hLiquidVRDLL != NULL)
    {
        printf("DXGI Server HookLiquidVR: GetProcAddress(%s)\n", ALVR_INIT_FUNCTION_NAME);

        Real_fnInit = (ALVRInit_Fn)::GetProcAddress(hLiquidVRDLL, ALVR_INIT_FUNCTION_NAME);

        if (Real_fnInit != NULL)
        {
            AMDT::BeginHook();

            LONG error = AMDT::HookAPICall(&(PVOID&)Real_fnInit, Mine_fnInit);
            error;
            PsAssert(error == NO_ERROR);

            if (AMDT::EndHook() != NO_ERROR)
            {
                Log(logERROR, "HookLiquidVR() failed\n");
            }
        }
    }

    LogTrace(traceEXIT, "");
}

/// Unhook ALVR init function
void UnhookLiquidVR()
{
    LogTrace(traceENTER, "");

    if (Real_fnInit == NULL)
    {
        return;
    }

    AMDT::BeginHook();

    LONG error = AMDT::UnhookAPICall(&(PVOID&)Real_fnInit, Mine_fnInit);
    PsAssert(error == NO_ERROR);

    error = AMDT::EndHook();

    if (error != NO_ERROR)
    {
        Log(logERROR, "failed\n");
    }

    LogTrace(traceEXIT, "");

    return;
}

#endif // LIQUID_VR_SUPPORT
