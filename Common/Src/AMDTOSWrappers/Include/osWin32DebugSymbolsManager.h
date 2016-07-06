//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osWin32DebugSymbolsManager.h
///
//=====================================================================

//------------------------------ osWin32DebugSymbolsManager.h ------------------------------

#ifndef __OSWIN32DEBUGSYMBOLSMANAGER_H
#define __OSWIN32DEBUGSYMBOLSMANAGER_H

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Forward Declarations:
class osFilePath;


// ----------------------------------------------------------------------------------
// Class Name:           OS_API osWin32DebugSymbolsManager
// General Description:
//   Wraps the Windows debug symbols engine with a convenient API.
// Author:      AMD Developer Tools Team
// Creation Date:        4/7/2010
// ----------------------------------------------------------------------------------
class OS_API osWin32DebugSymbolsManager
{
public:
    osWin32DebugSymbolsManager();
    ~osWin32DebugSymbolsManager();

    bool initializeProcessSymbolsServer(HANDLE hProcess, bool invadeProcess = false);
    bool clearProcessSymbolsServer(HANDLE hProcess);
    bool loadModuleDebugSymbols(HANDLE hProcess, const osFilePath& modulePath, osInstructionPointer moduleLoadedAddress, ULONG moduleSize);
    bool unloadModuleDebugSymbols(HANDLE hProcess, const osFilePath& modulePath, osInstructionPointer moduleLoadedAddress);
    bool getLoadedModuleSize(HANDLE hProcess, osInstructionPointer moduleLoadedAddress, ULONG& moduleSize);
    static BOOL CALLBACK dbghelpMessageCallbackFunction(HANDLE hProcess, ULONG ActionCode, ULONG64 CallbackData, ULONG64 UserContext);
};


#endif //__OSWIN32DEBUGSYMBOLSMANAGER_H
