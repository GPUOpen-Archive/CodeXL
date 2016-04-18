//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWin32SetRemoteProcessDLLDirectory.cpp
///
//==================================================================================

//------------------------------ pdWin32SetRemoteProcessDLLDirectory.cpp ------------------------------

// Windows:
#include <tchar.h>

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osWin32Functions.h>

// Local:
#include <AMDTProcessDebugger/Include/pdWin32MemoryInjector.h>
#include <AMDTProcessDebugger/Include/pdWin32SetRemoteProcessDLLDirectory.h>



// ---------------------------------------------------------------------------
// Name:        pdWin32SetRemoteProcessDLLDirectory::pdWin32SetRemoteProcessDLLDirectory
// Description: Constructor
// Arguments:   targetProcessHandle - The handle of the process into which the dll
//                                    will be loaded.
//              dllDirectory - The DLL path.
// Author:      Yaki Tebeka
// Date:        7/11/2004
// ---------------------------------------------------------------------------
pdWin32SetRemoteProcessDLLDirectory::pdWin32SetRemoteProcessDLLDirectory(HANDLE targetProcessHandle, const osFilePath& dllDirectory)
    : _targetProcessHandle(targetProcessHandle), _dllDirectory(dllDirectory),
      _pInjectedDLLDirectoryStringAddress(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        pdWin32SetRemoteProcessDLLDirectory::execute
// Description: Does this class work - calls SetDLLDirectory in the target process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/11/2004
// ---------------------------------------------------------------------------
bool pdWin32SetRemoteProcessDLLDirectory::execute()
{
    // Inject the DLL directory string into the target process:
    bool retVal = injectDLLDirectoryStringIntoTargetProcess();

    if (retVal)
    {
        // Call SetDLLDirectory() in the remote process:
        retVal = callSetDLLDirectory();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32SetRemoteProcessDLLDirectory::injectDLLDirectoryStringIntoTargetProcess
// Description: Injects the DLL directory string into the target process.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/11/2004
// ---------------------------------------------------------------------------
bool pdWin32SetRemoteProcessDLLDirectory::injectDLLDirectoryStringIntoTargetProcess()
{
    bool retVal = false;

    // Get a pointer to the DLL directory string:
    const gtString& dllDirectoryString = _dllDirectory.asString();
    const void* pMemoryToInject = (void*)(dllDirectoryString.asCharArray());
    unsigned long sizeOfMemoryToInject = dllDirectoryString.lengthInBytes();

    // Allocate memory for the dll directory string (in the remote process address space):
    pdWin32MemoryInjector memoryInjector(_targetProcessHandle, pMemoryToInject,
                                         sizeOfMemoryToInject);
    retVal = memoryInjector.allocateSpaceForInjectedMemory();

    if (retVal)
    {
        // Inject the DLL directory string into the target process:
        retVal = memoryInjector.injectMemory();

        // Get the address (in target process VM address space) of the injected string:
        _pInjectedDLLDirectoryStringAddress = memoryInjector.targetMemoryAddress();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32SetRemoteProcessDLLDirectory::callSetDLLDirectory
// Description: Creates a remote thread at the target process that calls
//              SetDLLDirectory() with our DLL directory string as input.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/11/2004
// ---------------------------------------------------------------------------
bool pdWin32SetRemoteProcessDLLDirectory::callSetDLLDirectory()
{
    bool retVal = false;

    // Will get the remote thread handle and id:
    HANDLE remoteThreadHandle = NULL;
    DWORD remoteThreadId = OS_NO_THREAD_ID;

    // Get the "SetDLLDirectoryW" function address:
    osPROCSetDLLDirectoryW procSetDLLDirectory = osGetWin32SetDLLDirectoryW();

    if (procSetDLLDirectory)
    {
        // Create a remote thread that loads the DLL into the target process:
        remoteThreadHandle =
            CreateRemoteThread(_targetProcessHandle,                   // Handle to the process in which the thread
                               // is to be created
                               NULL,                                   // Use the default security descriptor.
                               0,                                      // Use default stack size.
                               (LPTHREAD_START_ROUTINE)procSetDLLDirectory, // Thread start address.
                               (LPVOID)_pInjectedDLLDirectoryStringAddress,  // Variable passed to the
                               // thread function.
                               0,                                     // Creation flags.
                               &remoteThreadId);                      // Will get the created
        // thread id.

        if (remoteThreadHandle != NULL)
        {
            retVal = true;

            // Clean up:
            CloseHandle(remoteThreadHandle);
        }
    }

    return retVal;
}

