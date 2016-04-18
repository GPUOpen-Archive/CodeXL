//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsOpenGL32DLLExports.cpp
///
//==================================================================================

//------------------------------ gsOpenGL32DLLExports.cpp ------------------------------

// Windows:
// #define WIN32_LEAN_AND_MEAN // Excludes rarely-used stuff from Windows headers
// This is unneeded as the property files already define this
#include <Windows.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <src/gsOpenGLSpyInitFuncs.h>


// ---------------------------------------------------------------------------
// Name:        DllMain
// Description:
//   Main DLL function. Is called by Windows when:
//   a. Processes and threads that link statically to this DLL are initialized
//      and terminated
//   b. Upon calls to  LoadLibrary and FreeLibrary on this DLL.
//
// Arguments:   HANDLE hModule - Handle to the DLL. The value is the base address of the DLL.
//              DWORD  ul_reason_for_call - Specifies a flag indicating why this function
//                                          is being called
//              LPVOID lpReserved - Specifies further aspects of DLL initialization and
//                                  cleanup.
// Return Val:  BOOL - Success / Failure.
// Author:      Yaki Tebeka
// Date:        23/6/2003
// ---------------------------------------------------------------------------
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    GT_UNREFERENCED_PARAMETER(hModule);
    GT_UNREFERENCED_PARAMETER(lpReserved);

    bool retVal = true;

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_DETACH:
        {
            // When the debugged process is detached from this module, terminate the spy:
            retVal = gsOpenGLSpyTerminate();
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

