// dllmain.cpp : Defines the entry point for the DLL application.
#include "windows.h"
#include "AMDTActivityLogger.h"
#include "AMDTCpuProfileControl.h"

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                     )
{
    (void)(hModule); // UNUSED
    (void)(lpReserved); // UNUSED

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            amdtInitializeActivityLogger();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            amdtFinalizeActivityLogger();
            AMDTCpuProfileControlClose();
            break;
    }

    return TRUE;
}

