//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osOSWrappersDLLBuild.cpp
///
//=====================================================================

//------------------------------ osOSWrappersDLLBuild.cpp ------------------------------

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <Winsock2.h>

// GRBaseTools:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>

// The Win32 socket support version the we will use:
#define REQUIRED_WIN32_SOCKET_SUPPORT_VERSION MAKEWORD( 2, 0 )

// ---------------------------------------------------------------------------
// Name:        initializeWin32SocketSupport
// Description: Initialize the Win32 sockets support.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool initializeWin32SocketSupport()
{
    bool retVal = false;

    // The highest highest version of Win32 sockets support that we can use:
    WORD reqiredSocketSupportVersion = REQUIRED_WIN32_SOCKET_SUPPORT_VERSION;

    // Will get the current Win32 socket support implementation details:
    // (The maximal socket support version that the installed WS2_32.DLL supports):
    WSADATA socketImplementationData = { 0 };

    // Initialize the Win32 Socket support:
    int rc = WSAStartup(reqiredSocketSupportVersion, &socketImplementationData);

    if (rc != 0)
    {
        // We failed to initialize the Win32 Socket support:
        // (Usually because we could not load WS2_32.DLL or Winsock.dll):
        GT_ASSERT(0);
    }
    else
    {
        // Verify that the installed Win32 Socket support match our requested version:
        if (LOBYTE(socketImplementationData.wVersion) != LOBYTE(reqiredSocketSupportVersion) ||
            HIBYTE(socketImplementationData.wVersion) != HIBYTE(reqiredSocketSupportVersion))
        {
            // The Win32 Socket support does not match our required version:
            GT_ASSERT(0);
        }
        else
        {
            // We managed to initialize the Win32 socket support, and get the version that
            // we required:
            retVal = true;
        }
    }

    if (!retVal)
    {
        // Failure clean up:
        WSACleanup();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        terminateWin32SocketSupport
// Description: Terminates the Win32 sockets support.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool terminateWin32SocketSupport()
{
    bool retVal = false;

    // Terminate the Win32 socket support:
    int rc = WSACleanup();

    if (rc != SOCKET_ERROR)
    {
        retVal = true;
    }
    else
    {
        // Failed to terminate the Win32 Socket support:
        GT_ASSERT(0);
    }

    return retVal;
}


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
// Author:      AMD Developer Tools Team
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
            rc = initializeWin32SocketSupport();
            break;

        case DLL_PROCESS_DETACH:
            rc = terminateWin32SocketSupport();
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
