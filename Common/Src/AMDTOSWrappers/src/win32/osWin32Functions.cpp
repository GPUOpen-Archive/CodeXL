//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osWin32Functions.cpp
///
//=====================================================================

//------------------------------ osWin32Functions.cpp ------------------------------

// Local:
#include <AMDTOSWrappers/Include/osWin32Functions.h>


// Contains true iff the relevant module functions were connected to the below function pointers:
static bool stat_areKernel32FunctionPointersConnected = false;


// Pointers to Kernel32.dll functions:
static osPROCSetDLLDirectoryW stat_pWin32SetDLLDirectoryW = NULL;


// ---------------------------------------------------------------------------
// Name:        osConnectKernel32FunctionPointers
// Description: Get pointers to functions that reside in Kernel32.dll
//              and connect them to the static pointers that we hold at the
//              top of this file.
// Author:      AMD Developer Tools Team
// Date:        31/10/2004
// ---------------------------------------------------------------------------
void osConnectKernel32FunctionPointers()
{
    if (!stat_areKernel32FunctionPointersConnected)
    {
        // Get Kernel32 module handle:
        HMODULE hKernel32Module = GetModuleHandle(L"KERNEL32.DLL");

        // Connect Kernel32 function pointers:
        stat_pWin32SetDLLDirectoryW = (osPROCSetDLLDirectoryW)GetProcAddress(hKernel32Module, "SetDllDirectoryW");


        // Mark that the kernel32.dll functions were connected:
        stat_areKernel32FunctionPointersConnected = true;
    }
}


// ---------------------------------------------------------------------------
// Name:        osGetWin32SetDLLDirectoryW
// Description: Returns a pointer to the win32 SetDLLDirectoryW function
//              (or NULL if it is not supported on this machine)
// Author:      AMD Developer Tools Team
// Date:        31/10/2004
// ---------------------------------------------------------------------------
osPROCSetDLLDirectoryW osGetWin32SetDLLDirectoryW()
{
    // Connect the Kernel32.dll functions:
    osConnectKernel32FunctionPointers();

    // Return the connected function pointer:
    return stat_pWin32SetDLLDirectoryW;
}
