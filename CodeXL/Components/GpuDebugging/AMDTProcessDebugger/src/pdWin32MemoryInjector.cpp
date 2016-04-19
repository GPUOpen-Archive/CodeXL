//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWin32MemoryInjector.cpp
///
//==================================================================================

//------------------------------ pdWin32MemoryInjector.cpp ------------------------------

// GRBaseTools:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTProcessDebugger/Include/pdWin32MemoryInjector.h>


// ---------------------------------------------------------------------------
// Name:        pdWin32MemoryInjector::pdWin32MemoryInjector
// Description: Constructor
// Arguments:   targetProcessHandle - The target process handle.
//              pMemoryToInject - The memory to be injected.
//              sizeOfMemoryToInject - The size of the injected memory.
// Author:      Yaki Tebeka
// Date:        27/11/2003
// ---------------------------------------------------------------------------
pdWin32MemoryInjector::pdWin32MemoryInjector(HANDLE targetProcessHandle,
                                             const void* pMemoryToInject,
                                             unsigned long sizeOfMemoryToInject)
    : _targetProcessHandle(targetProcessHandle),
      _pMemoryToBeInjected(pMemoryToInject),
      _sizeOfMemoryToInject(sizeOfMemoryToInject),
      _pTargetMemoryAddress(NULL)
{
}


// Defining VirtualAllocEx function type.
// (We use a function pointer + GetModuleHandle for functions that does not
//  reside in all the Operating systems that we want to run on top
//  Otherwise - If we would used a hard coded function call, we would get
//  an unresolved symbol when running on operating systems that does not have
//  the hard coded function).
typedef LPVOID (__stdcall* PFN_VIRTUAL_ALLOC_EX)(HANDLE, LPVOID, SIZE_T, DWORD, DWORD);


// ---------------------------------------------------------------------------
// Name:        pdWin32MemoryInjector::allocateSpaceForInjectedMemory
// Description:
//   Allocated memory that will contain the injected memory
//   This memory will be accessible by the target process.
//
// Author:      Yaki Tebeka
// Date:        27/11/2003
//
// Implementation Notes:
//   - Under Windows NT product line (Win NT, Win2000, etc) - this memory will
//     be allocated in the target process address space.
//   - Under Windows 98X product line (Win 95, Win 98, etc) - this memory will
//     be allocates in a small memory mapped file.
//     (This is done because VirtualAllocEx is not available in this product line).
// ---------------------------------------------------------------------------
bool pdWin32MemoryInjector::allocateSpaceForInjectedMemory()
{
    bool retVal = false;

    // Get the operating system version (on which we run):
    OSVERSIONINFO osVersionInfo;
    ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFO));
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
    int rc = GetVersionEx(&osVersionInfo);

    if (rc != 0)
    {
        // If this the Windows NT product line:
        if (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            // We're on NT, so we can use VirtualAllocEx to allocate memory in
            // the target process address space.
            // (We can't just call VirtualAllocEx since it's not defined in
            //  the Windows 9X product line).

            // Get a pointer to the VirtualAllocEx function, defined in KERNEL32.DLL:
            PFN_VIRTUAL_ALLOC_EX pVirtualAllocEx =
                (PFN_VIRTUAL_ALLOC_EX)GetProcAddress(GetModuleHandle(L"KERNEL32.DLL"),
                                                     "VirtualAllocEx");

            if (pVirtualAllocEx)
            {
                // Allocate the memory in the target process address space:
                _pTargetMemoryAddress = pVirtualAllocEx(_targetProcessHandle,
                                                        0, _sizeOfMemoryToInject,
                                                        MEM_COMMIT, PAGE_READWRITE);
            }
        }
        else if (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        {
            // We're on Windows 9X. We will create a small memory mapped file.
            // On Windows 9X platform memory mapped files are above 2GB, and thus are
            // accessible by all processes.

            HANDLE hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, 0,
                                                    PAGE_READWRITE | SEC_COMMIT,
                                                    0, _sizeOfMemoryToInject,
                                                    0);

            if (hFileMapping)
            {
                _pTargetMemoryAddress = MapViewOfFile(hFileMapping, FILE_MAP_WRITE, 0, 0,
                                                      _sizeOfMemoryToInject);
            }
            else
            {
                // Failure clean up:
                CloseHandle(hFileMapping);
            }
        }
    }

    retVal = (_pTargetMemoryAddress != NULL);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWin32MemoryInjector::injectMemory
// Description:
//   Injects the memory into the target process.
//   This function should be called after allocateSpaceForInjectedMemory()
//   was called.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        27/11/2003
// ---------------------------------------------------------------------------
bool pdWin32MemoryInjector::injectMemory()
{
    bool retVal = false;

    // Verify that we have allocated space for the memory to inject:
    if (_pTargetMemoryAddress)
    {
        // Inject the memory into the target process:
        SIZE_T numberOfBytesWritten = 0;
        int rc = WriteProcessMemory(_targetProcessHandle,
                                    _pTargetMemoryAddress, (void*)_pMemoryToBeInjected,
                                    _sizeOfMemoryToInject, &numberOfBytesWritten);

        retVal = ((rc != 0) && (numberOfBytesWritten == _sizeOfMemoryToInject));
    }

    return retVal;
}
