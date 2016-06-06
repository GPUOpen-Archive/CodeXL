//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osWin32DebugSymbolsManager.cpp
///
//=====================================================================

//------------------------------ osWin32DebugSymbolsManager.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Windows DebugHelp library:
#pragma warning( push )
#pragma warning( disable : 4091)
#include <dbghelp.h>
#pragma warning( pop )

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osWin32DebugSymbolsManager.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osMutexLocker.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>

// Static Variables:
// Holds the loaded module size (See pdEnumerateLoadedModulesProc64)
static ULONG stat_loadedModuleSize = 0;

// ---------------------------------------------------------------------------
// Name:        osWin32DebugSymbolsManager::osWin32DebugSymbolsManager
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
osWin32DebugSymbolsManager::osWin32DebugSymbolsManager()
{

}

// ---------------------------------------------------------------------------
// Name:        osWin32DebugSymbolsManager::~osWin32DebugSymbolsManager
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
osWin32DebugSymbolsManager::~osWin32DebugSymbolsManager()
{

}

// ---------------------------------------------------------------------------
// Name:        osWin32DebugSymbolsManager::loadModuleDebugSymbols
// Description: Loads a Module (exe or dll) debug symbols into the symbol engine.
// Arguments:   moduleName - The module name.
//              moduleLoadedAddress - The loaded address of the module (in debugged process address space).
//              moduleSize - The loaded module size (in memory).
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        17/10/2004
// ---------------------------------------------------------------------------
bool osWin32DebugSymbolsManager::loadModuleDebugSymbols(HANDLE hProcess, const osFilePath& modulePath, osInstructionPointer moduleLoadedAddress,
                                                        ULONG moduleSize)
{
    bool retVal = false;

    // Get the module path as a string:
    const gtString& moduleName = modulePath.asString();

    // Open the module file:
    HANDLE hModuleFile = CreateFile(moduleName.asCharArray(), GENERIC_READ, FILE_SHARE_READ,
                                    NULL, OPEN_EXISTING, 0, 0);

    // If we managed to open the file:
    if (hModuleFile != INVALID_HANDLE_VALUE)
    {
        // Load the DLL symbols into the symbol engine:
        DWORD64 dwModuleLoadedAddress = (DWORD64)moduleLoadedAddress;
        DWORD64 rc1 = SymLoadModuleEx(hProcess, hModuleFile, moduleName.asCharArray(), NULL, dwModuleLoadedAddress, moduleSize,
                                      NULL, 0);

        // If the debug info was loaded successfully:
        if (rc1 != 0)
        {
            retVal = true;
        }

        // Clean up:
        CloseHandle(hModuleFile);
    }

    if (!retVal)
    {
        // Trigger an assertion failure:
        gtString errorMessage = OS_STR_FailedToLoadedModuleDebugInfo;
        errorMessage += moduleName;
        GT_ASSERT_EX(false, errorMessage.asCharArray());
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osWin32DebugSymbolsManager::unloadModuleDebugSymbols
// Description: Unloads a given module debug information.
// Arguments: modulePath - The module path.
//            moduleLoadedAddress - The module base address.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        9/10/2005
// ---------------------------------------------------------------------------
bool osWin32DebugSymbolsManager::unloadModuleDebugSymbols(HANDLE hProcess, const osFilePath& modulePath, osInstructionPointer moduleLoadedAddress)
{
    bool retVal = false;

    // Unload the module debug information from the Win32 symbol handler:
    DWORD64 dwModuleLoadedAddress = (DWORD64)moduleLoadedAddress;
    BOOL rc = SymUnloadModule64(hProcess, dwModuleLoadedAddress);

    if (rc == TRUE)
    {
        retVal = true;
    }
    else
    {
        // Trigger an assertion failure:
        gtString errorMessage = OS_STR_FailedToUnloadedModuleDebugInfo;
        errorMessage += modulePath.asString();
        GT_ASSERT_EX(false, errorMessage.asCharArray());
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdEnumerateLoadedModulesProc64
// Description:
//   Is called by getLoadedModuleSize (by EnumerateLoadedModules64)
//   on each loaded module.
//   If the current module is the module that we search for, set stat_loadedModuleSize
//   to contain the module size and stops the modules iteration.
//
// Arguments: moduleName - The current module name.
//            moduleBase - The module base address.
//            moduleSize - The module size.
//            userData - Contains the start address of the module that we search for.
// Return Val: BOOL - TRUE - To continue the modules enumeration
//                    FALSE - To stop the modules enumeration.
// Author:      AMD Developer Tools Team
// Date:        29/9/2005
// ---------------------------------------------------------------------------
BOOL CALLBACK pdEnumerateLoadedModulesProc64(PSTR moduleName, DWORD64 moduleBase,
                                             ULONG moduleSize, PVOID userData)
{
    GT_UNREFERENCED_PARAMETER(moduleName);

    BOOL retVal = TRUE;

    // Get the base address of the module that we search for (we get it as our user data):
    DWORD64 searchedModuleBaseAddress = DWORD64(userData);

    // If the current module is the module that we are searching for:
    if (moduleBase == searchedModuleBaseAddress)
    {
        // Store the module size:
        stat_loadedModuleSize = moduleSize;

        // Exit the modules iteration:
        retVal = FALSE;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osWin32DebugSymbolsManager::getLoadedModuleSize
// Description: Input a loaded module path and outputs its memory size.
// Arguments: moduleLoadedAddress - The module loaded address.
//            moduleSize - Will get the module size in memory.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/9/2005
// ---------------------------------------------------------------------------
bool osWin32DebugSymbolsManager::getLoadedModuleSize(HANDLE hProcess, osInstructionPointer moduleLoadedAddress, ULONG& moduleSize)
{
    bool retVal = false;

    // Enumerate the loaded modules and search for the input module:
    stat_loadedModuleSize = 0;
    BOOL rc = ::EnumerateLoadedModules64(hProcess, (PENUMLOADED_MODULES_CALLBACKW64)pdEnumerateLoadedModulesProc64, (PVOID)moduleLoadedAddress);

    // If we managed to get the module size:
    if ((rc == TRUE) && (stat_loadedModuleSize > 0))
    {
        // Output the module size:
        moduleSize = stat_loadedModuleSize;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osWin32DebugSymbolsManager::initializeProcessSymbolsServer
// Description: Initialize the Windows Debug help symbol server for the debugged
//              process (a server that holds the Windows debug information).
// Arguments:   hProcess - The debugged process handle.
//              invadeProcess - if true, enumerates the loaded modules for the process
//                              and effectively calls the SymLoadModuleEx function for each module.
// Author:      AMD Developer Tools Team
// Date:        12/10/2004
// ---------------------------------------------------------------------------
bool osWin32DebugSymbolsManager::initializeProcessSymbolsServer(HANDLE hProcess, bool invadeProcess)
{
    bool retVal = false;

    // Translate the invade process argument to a BOOL:
    BOOL invadeProcessAsBOOL = FALSE;

    if (invadeProcess)
    {
        invadeProcessAsBOOL = TRUE;
    }

    // Get the Debug Symbols engine options:
    DWORD symOptions = SymGetOptions();

    // Add the following flags to the the Debug Symbols engine options:
    // - SYMOPT_DEFERRED_LOADS - Symbols are not loaded until a reference is made requiring
    //                           the symbols to be loaded.
    // - SYMOPT_LOAD_LINES - Loads line number information.
    // - SYMOPT_UNDNAME - All symbols are presented in undecorated form.

    // It seems that in 64-bit Windows apps, the lazy loading causes some issues with the debug information.
#if (AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE)
    DWORD forcedOptions = (SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
#elif (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE)
    DWORD forcedOptions = (SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
#else
#error Unknown address space size!
#endif
    SymSetOptions(symOptions | forcedOptions);

    // Initialize the DbgHelp library:
    if (SymInitialize(hProcess, NULL, invadeProcessAsBOOL) == TRUE)
    {
        retVal = true;
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osWin32DebugSymbolsManager::clearProcessSymbolsServer
// Description: Clears the Windows Debug help symbol server from a given
//              process symbols.
// Arguments:   hProcess - The given process handle.
// Author:      AMD Developer Tools Team
// Date:        12/10/2004
// ---------------------------------------------------------------------------
bool osWin32DebugSymbolsManager::clearProcessSymbolsServer(HANDLE hProcess)
{
    bool retVal = false;

    BOOL rc = SymCleanup(hProcess);
    retVal = (rc != FALSE);

    return retVal;
}
