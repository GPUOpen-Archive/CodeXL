//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWin32MemoryInjector.h
///
//==================================================================================

//------------------------------ pdWin32MemoryInjector.h ------------------------------

#ifndef __PDWIN32MEMORYINJECTOR
#define __PDWIN32MEMORYINJECTOR

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Local:
#include <AMDTProcessDebugger/Include/ProcessDebuggerDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           PD_API pdWin32MemoryInjector
// General Description:
//   Injects an input memory piece into a target process.
//
//   Notice:
//   ------
//   The execute() method should be called after KERNEL32.DLL was loaded into
//   the target process (it uses some of KERNEL32.DLL functions).
//
// Author:               Yaki Tebeka
// Creation Date:        9/11/2003
// ----------------------------------------------------------------------------------
class PD_API pdWin32MemoryInjector
{
public:
    pdWin32MemoryInjector(HANDLE targetProcessHandle, const void* pMemoryToInject,
                          unsigned long sizeOfMemoryToInject);

    bool allocateSpaceForInjectedMemory();
    bool injectMemory();
    void* targetMemoryAddress() const { return _pTargetMemoryAddress; };

private:
    // Do not allow the use of my default constructor:
    pdWin32MemoryInjector();

private:
    // The handle of the process into which we will inject the memory:
    HANDLE _targetProcessHandle;

    // The memory to be injected:
    const void* _pMemoryToBeInjected;

    // The size of the memory to be injected:
    unsigned long _sizeOfMemoryToInject;

    // The address of the injected memory (In target process address space):
    void* _pTargetMemoryAddress;
};


#endif  // __PDWIN32MEMORYINJECTOR
