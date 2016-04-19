//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSpiesUtilitiesDLLMainFunc.cpp
///
//==================================================================================

//------------------------------ suSpiesUtilitiesDLLMainFunc.cpp ------------------------------

// Windows:
// This is defined in the props file, no need to define it again here:
// #define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <src/suSpiesUtilitiesDLLInitializationFunctions.h>


// ---------------------------------------------------------------------------
// Name:        DllMain
//
// Description:
//   Main DLL function. Is called by Windows when:
//   a. Processes and threads that link statically to this DLL are initialized
//      and terminated.
//   b. Upon calls to LoadLibrary and FreeLibrary on this DLL.
//
// Arguments:   HANDLE hModule - Handle to the DLL. The value is the base address of the DLL.
//              DWORD  callReason - Specifies a flag indicating why this function is being called.
//              LPVOID lpReserved - Specifies further aspects of DLL initialization and cleanup.
//
// Return Val:  BOOL - Success / Failure.
// Author:      Yaki Tebeka
// Date:        22/9/2009
// ---------------------------------------------------------------------------
BOOL APIENTRY DllMain(HANDLE hModule, DWORD callReason, LPVOID lpReserved)
{
    GT_UNREFERENCED_PARAMETER(hModule);
    GT_UNREFERENCED_PARAMETER(lpReserved);

    switch (callReason)
    {
        case DLL_PROCESS_ATTACH:
        {
        }
        break;

        case DLL_PROCESS_DETACH:
        {
            // Report, to my debugger, that the debugged process is about to terminate:
            suReportDebuggedProcessTermination();

            suTerminateSpiesUtilitiesModule();
        }
        break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}

