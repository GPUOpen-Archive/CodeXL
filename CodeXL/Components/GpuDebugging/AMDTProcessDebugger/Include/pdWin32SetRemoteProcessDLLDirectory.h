//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWin32SetRemoteProcessDLLDirectory.h
///
//==================================================================================

//------------------------------ pdWin32SetRemoteProcessDLLDirectory.h ------------------------------

#ifndef __PDWIN32SETREMOTEPROCESSDLLDIRECTORY
#define __PDWIN32SETREMOTEPROCESSDLLDIRECTORY

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTProcessDebugger/Include/ProcessDebuggerDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           PD_API pdWin32SetRemoteProcessDLLDirectory
// General Description:
//  Calls the Win32 SetDllDirectory() function with an input dll directory path
//  in a remote process.
//
//   Notice:
//   ------
//   a. The execute() method should be called after KERNEL32.DLL was loaded into
//      the target process (it uses some of KERNEL32.DLL functions).
//   b. SetDllDirectory will be called only after the target process startup and staticly
//      linked DLL initialization routines are over.
//      (From CreateRemoteThread MSDN documentation: During process startup and DLL
//       initialization routines, new threads can be created, but they do not begin
//       execution until DLL initialization is done for the process).
//
// Author:               Yaki Tebeka
// Creation Date:        7/11/2004
// ----------------------------------------------------------------------------------
class PD_API pdWin32SetRemoteProcessDLLDirectory
{
public:
    pdWin32SetRemoteProcessDLLDirectory(HANDLE targetProcessHandle, const osFilePath& dllDirectory);
    bool execute();

private:
    bool injectDLLDirectoryStringIntoTargetProcess();
    bool callSetDLLDirectory();

    // Disallow use of default constructor, copy constructor and assignment operator:
    pdWin32SetRemoteProcessDLLDirectory() = delete;
    pdWin32SetRemoteProcessDLLDirectory(const pdWin32SetRemoteProcessDLLDirectory&) = delete;
    pdWin32SetRemoteProcessDLLDirectory& operator=(const pdWin32SetRemoteProcessDLLDirectory&) = delete;

private:
    // The handle of the process in which we will call :
    HANDLE _targetProcessHandle;

    // The input DLLs directory.
    const osFilePath _dllDirectory;

    // The address of the injected DLL directory path:
    void* _pInjectedDLLDirectoryStringAddress;
};


#endif  // __PDWIN32SETREMOTEPROCESSDLLDIRECTORY
