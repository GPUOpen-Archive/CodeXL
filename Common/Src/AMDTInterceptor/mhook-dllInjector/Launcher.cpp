//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of Process launcher. This launcher is used by the
///         dll Injector library when spawning 64 bit processes from 32 bit
///         launchers and vice versa. If a 32-bit application needs to create
///         a 64-bit target process, it creates a 64-bit launcher
///         and delegates the creation of the target process and dll injection
///         to the launcher.
///         The important thing to note here is that a 64-bit dll can only be
///         injected into a 64-bit process (the application being created)
///         via a separate 64-bit process (this launcher).
//==============================================================================

#include <windows.h>
#include <fstream>
#include <string>
#include <tchar.h>
#include <Interceptor.h>

#ifdef DEBUG_PRINT
    #define DEBUG_OUTPUT(x) printf x
#else
    #define DEBUG_OUTPUT(x)
#endif

extern BOOL InjectDLLIntoProcess(const char* lpDllName, DWORD processID);

/// Main application entry point
/// The first command line argument is the name of the dll to be injected
/// The second command line argument is the process ID of the target application.
/// This has already been started but suspended from the calling process.
int _tmain(int argc, _TCHAR* argv[])
{
    // get the command line arguments
    UNREFERENCED_PARAMETER(argc);
    char* lpDllName = argv[0];
    int processID = atoi(argv[1]);
    DEBUG_OUTPUT(("Launcher: injecting %s into process %d\n", lpDllName, processID));

    // inject the DLL into the target process
    if (FALSE == InjectDLLIntoProcess(lpDllName, processID))
    {
        DEBUG_OUTPUT(("Launcher error: can't inject into process %d\n", processID));
    }

    return 0;
}

