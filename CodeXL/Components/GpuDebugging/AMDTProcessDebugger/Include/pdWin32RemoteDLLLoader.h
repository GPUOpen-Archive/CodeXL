//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWin32RemoteDLLLoader.h
///
//==================================================================================

//------------------------------ pdWin32RemoteDLLLoader.h ------------------------------

#ifndef __PDWIN32REMOTEDLLLOADER
#define __PDWIN32REMOTEDLLLOADER

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTProcessDebugger/Include/ProcessDebuggerDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           PD_API pdWin32RemoteDLLLoader
// General Description:
//   Loads a given DLL into a remote process.
//   The DLL is loading by creating a remote thread that executes LoadLibrary on
//   the input DLL.
//
//   Notice:
//   ------
//   a. The execute() method should be called after KERNEL32.DLL was loaded into
//      the target process (it uses some of KERNEL32.DLL functions).
//   b. The DLL will be loaded only after the target process startup and DLL initialization
//      routines are over.
//      (From CreateRemoteThread MSDN documentation: During process startup and DLL
//       initialization routines, new threads can be created, but they do not begin
//       execution until DLL initialization is done for the process).
//
// Author:               Yaki Tebeka
// Creation Date:        9/11/2003
// ----------------------------------------------------------------------------------
class PD_API pdWin32RemoteDLLLoader
{
public:
    pdWin32RemoteDLLLoader(HANDLE targetProcessHandle, const osFilePath& dllDirectory);
    bool execute();

private:
    bool injectDLLPathIntoTargetProcess();
    bool loadDLLIntoTargetProcess();

    // Disallow use of default constructor, copy constructor and assignment operator:
    pdWin32RemoteDLLLoader() = delete;
    pdWin32RemoteDLLLoader(const pdWin32RemoteDLLLoader&) = delete;
    pdWin32RemoteDLLLoader& operator=(const pdWin32RemoteDLLLoader&) = delete;

private:
    // The handle of the process into which we will load the DLL:
    HANDLE _targetProcessHandle;

    // The path of the DLL to be loaded.
    const osFilePath _dllPath;

    // The address of the injected dll name:
    void* _pInjectedDLLNameAddress;
};


#endif  // __PDWIN32REMOTEDLLLOADER
