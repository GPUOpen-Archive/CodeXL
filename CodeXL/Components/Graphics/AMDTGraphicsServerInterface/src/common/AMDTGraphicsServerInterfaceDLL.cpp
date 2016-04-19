//------------------------------ AMDTGraphicsServerInterfaceDLL.cpp ------------------------------

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// GRBaseTools:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include "../../Include/GraphicsServerInterfaceDLLBuild.h"

// The Win32 socket support version the we will use:
#define REQUIRED_WIN32_SOCKET_SUPPORT_VERSION MAKEWORD( 2, 0 )

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
// Return Val:  BOOL - Success / failure
// Author:      Yaki Tebeka
// Date:        03/1/2004
// ---------------------------------------------------------------------------
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    GT_UNREFERENCED_PARAMETER(hModule);
    GT_UNREFERENCED_PARAMETER(lpReserved);

    BOOL retVal = FALSE;
    bool rc = false;

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            rc = true;
            break;

        case DLL_PROCESS_DETACH:
            rc = true;
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;
    }

    if (rc == true)
    {
        retVal = TRUE;
    }

    return retVal;
}
