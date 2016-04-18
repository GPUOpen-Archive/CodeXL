//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWin32RemoteDLLLoader.cpp
///
//==================================================================================

//------------------------------ pdWin32RemoteDLLLoader.cpp ------------------------------

// Windows:
#include <tchar.h>

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTProcessDebugger/Include/pdWin32MemoryInjector.h>
#include <AMDTProcessDebugger/Include/pdWin32RemoteDLLLoader.h>

// The appropriate (unicode or ascii) LoadLibrary function name:
static LPCSTR static_LoadLibraryFunctionName =
#ifdef UNICODE
    "LoadLibraryW";
#else
    "LoadLibraryA";
#endif


    // ---------------------------------------------------------------------------
    // Name:        pdWin32RemoteDLLLoader::pdWin32RemoteDLLLoader
    // Description: Constructor
    // Arguments:   targetProcessHandle - The handle of the process into which the dll
    //                                    will be loaded.
    //              dllPath - The DLL path.
    // Author:      Yaki Tebeka
    // Date:        23/6/2004
    // ---------------------------------------------------------------------------
    pdWin32RemoteDLLLoader::pdWin32RemoteDLLLoader(HANDLE targetProcessHandle, const osFilePath& dllPath)
        : _targetProcessHandle(targetProcessHandle), _dllPath(dllPath),
          _pInjectedDLLNameAddress(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        pdWin32RemoteDLLLoader::execute
// Description: Does this class work - loads the DLL into the target process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/6/2004
// ---------------------------------------------------------------------------
bool pdWin32RemoteDLLLoader::execute()
{
    // Inject the DLL name into the target process:
    bool retVal = injectDLLPathIntoTargetProcess();

    if (retVal)
    {
        retVal = loadDLLIntoTargetProcess();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32RemoteDLLLoader::injectDLLPathIntoTargetProcess
// Description: Injects the DLL path into the target process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/6/2004
// ---------------------------------------------------------------------------
bool pdWin32RemoteDLLLoader::injectDLLPathIntoTargetProcess()
{
    bool retVal = false;

    // Copy the DLL path into a string and get a pointer to it:
    gtString dllPathString = _dllPath.asString();
    const void* pMemoryToInject = (void*)(dllPathString.asCharArray());
    unsigned long sizeOfMemoryToInject = dllPathString.length();

    // Allocate memory for the dll path:
    pdWin32MemoryInjector memoryInjector(_targetProcessHandle, pMemoryToInject,
                                         sizeOfMemoryToInject);
    retVal = memoryInjector.allocateSpaceForInjectedMemory();

    if (retVal)
    {
        // Inject the DLL path into the target process:
        retVal = memoryInjector.injectMemory();

        // Get the address (in target process VM address space) of the injected DLL
        // name:
        _pInjectedDLLNameAddress = memoryInjector.targetMemoryAddress();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32RemoteDLLLoader::loadDLLIntoTargetProcess
// Description: Loads the DLL into the target process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/6/2004
// Implementation notes:
//   To load the DLL into the target process we create a remote thread (that runs
//   in the target process). This thread will call LoadLibrary on the DLL to be
//   loaded and exit.
// ---------------------------------------------------------------------------
bool pdWin32RemoteDLLLoader::loadDLLIntoTargetProcess()
{
    bool retVal = false;

    // Will get the remote thread handle and id:
    HANDLE remoteThreadHandle = NULL;
    DWORD remoteThreadId = OS_NO_THREAD_ID;

    // Get the "LoadLibrary" function address:
    HMODULE kernel32ModuleHandle = GetModuleHandle(_T("Kernel32"));
    FARPROC fpLoadLibrary = GetProcAddress(kernel32ModuleHandle, static_LoadLibraryFunctionName);

    if (fpLoadLibrary)
    {
        // Create a remote thread that loads the DLL into the target process:
        remoteThreadHandle =
            CreateRemoteThread(_targetProcessHandle,                   // Handle to the process in which the thread
                               // is to be created
                               NULL,                                   // Use the default security descriptor.
                               0,                                      // Use default stack size.
                               (LPTHREAD_START_ROUTINE) fpLoadLibrary, // Thread start address.
                               (LPVOID)_pInjectedDLLNameAddress,       // Variable passed to the
                               // thread function.
                               0,                                     // Creation flags.
                               &remoteThreadId);                      // Will get the created
        // thread id.

        if (remoteThreadHandle != NULL)
        {
            // Wait for the thread to end its task:
            WaitForSingleObject(remoteThreadHandle, 8000);

            // Get the thread exit code:
            // (This is actually the LoadLibrary return code)
            DWORD threadExitCode = 0;
            BOOL rc = GetExitCodeThread(remoteThreadHandle, (LPDWORD)&threadExitCode);

            if (rc)
            {
                // If the remote LoadLibrary succeeded:
                if (threadExitCode != NULL)
                {
                    retVal = true;
                }
            }

            // Clean up:
            CloseHandle(remoteThreadHandle);
        }
    }

    return retVal;
}

