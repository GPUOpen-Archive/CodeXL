//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLDLLMainFunc.cpp
///
//==================================================================================

//------------------------------ csOpenCLDLLMainFunc.cpp ------------------------------

// Windows:
// #define WIN32_LEAN_AND_MEAN
// This is uneeded as the props file defines it.
#include <Windows.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <src/csOpenCLServerInitialization.h>


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

    bool retVal = true;

    switch (callReason)
    {
        case DLL_PROCESS_DETACH:
        {
            // Terminate the OpenCL Spy dll:
            retVal = csTerminateOpenCLServer();
        }
        break;

        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    // Translate bool return value into BOOL:
    if (retVal)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

