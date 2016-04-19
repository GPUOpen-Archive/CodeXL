//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DllMain.cpp
/// \brief  Defines the entry point for the DLL application
///
//==================================================================================

#include <windows.h>
#include <AMDTJITProfileAgent.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>


BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                     )
{
    GT_UNREFERENCED_PARAMETER(hModule);
    GT_UNREFERENCED_PARAMETER(lpReserved);

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            AMDTJitProfileInitialize(NULL);
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            AMDTJitProfileFinalize();
            break;
    }

    return TRUE;
}
