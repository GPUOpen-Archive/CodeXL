//==============================================================================
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Responsible for Windows registry interactions
//==============================================================================

#include <windows.h>
#include <atlbase.h>
#include <shlobj.h>
#include <stdio.h>
#include <PathCch.h>

#include <AMDTBaseTools/Include/gtASCIIString.h>

#include "../Common/Logger.h"
#include "../MicroDLL/MicroDLLName.h"

#include "OSDependent.h"

#include "Registry.h"

// registry key to use for "Open with" option
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    #ifdef DEBUG
        #define PS_OPEN_WITH_KEY "*\\shell\\Open with GPU PerfServer (x64 Debug)"
    #else
        #define PS_OPEN_WITH_KEY "*\\shell\\Open with GPU PerfServer (x64)"
    #endif
#else // #if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    #ifdef DEBUG
        #define PS_OPEN_WITH_KEY "*\\shell\\Open with GPU PerfServer (Debug)" ///< registry key to use for "Open with" option
    #else
        #define PS_OPEN_WITH_KEY "*\\shell\\Open with GPU PerfServer" ///< registry key to use for "Open with" option
    #endif
#endif // #if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE

#define APPINIT_PATH    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows" ///< Registry path definition for AppInitDLL support keys
#define APPINIT_KEY     "AppInit_DLLs" ///< AppInitDLL key
#define LOADAPPINIT_KEY "LoadAppInit_DLLs" ///< Load AppInit key

/// Enable "Open with" functionality on right context menu by setting the appropriate registry key
void SetOpenWithRegistryKey(void)
{
    HKEY hk;
    char sCommand[ PS_MAX_PATH];
    DWORD len = GetModuleFileName(NULL, sCommand, PS_MAX_PATH);

    if ((len == 0) || (len == PS_MAX_PATH))
    {
        printf("Error: Unable to determine path to executable\n");
    }

    gtASCIIString sDefault = AddQuotesIfStringHasSpaces(sCommand);

    // This next line is necessary in order to handle the fact that windows dereferences
    // shortcuts when passing them to Open With commands. The Program being run is passed
    // to PerfServer via the first reference to appargs, the arguments are passed in the second.
    // all the quote and backslash nastiness are needed to ensure spaces are handled correctly.
    sDefault += " --appargs=\"\\\"%1\\\"\" --appargs=\"\\\"%*\\\"\"";

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT, PS_OPEN_WITH_KEY"\\Command", 0, NULL,
                                        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL))
    {
        printf("Error: Unable to create registry key HKEY_CLASSES_ROOT\\%s\n", PS_OPEN_WITH_KEY);
        printf("If you are running on Windows Vista, try running as Administrator.\n");
        return;
    }

    if (ERROR_SUCCESS != RegSetValueEx(hk, NULL, 0, REG_SZ, (LPBYTE)sDefault.asCharArray(), (DWORD)sDefault.length() + 1))
    {
        printf("Error: Unable to write registry key HKEY_CLASSES_ROOT\\%s\n", PS_OPEN_WITH_KEY);
        printf("If you are running on Windows Vista, try running as Administrator.\n");
        RegCloseKey(hk);
        return;
    }

    printf("Successfully wrote registry key HKEY_CLASSES_ROOT\\%s\n", PS_OPEN_WITH_KEY);
    RegCloseKey(hk);
}

/// Remove "Open with" registry key if it exists
void DeleteOpenWithRegistryKey()
{
    HKEY hKey;

    // does the key exist?
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, PS_OPEN_WITH_KEY, 0L, KEY_ALL_ACCESS, &hKey))
    {
        if (ERROR_SUCCESS != SHDeleteKey(HKEY_CLASSES_ROOT, PS_OPEN_WITH_KEY))
        {
            printf("Error: Unable to delete registry key HKEY_CLASSES_ROOT\\%s\n", PS_OPEN_WITH_KEY);
            return;
        }

        printf("Successfully deleted registry key HKEY_CLASSES_ROOT\\%s\n", PS_OPEN_WITH_KEY);
    }
}

/// Set the AppInit_DLLs registry entry with the 32 and 64 bit versions of MicroDLL, and set the LoadAppInit_DLLs entry to 1
void EnableAppInit()
{
    HKEY hKey;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, APPINIT_PATH, 0L, KEY_ALL_ACCESS, &hKey))
    {
        char curDir[PS_MAX_PATH] = {};
        DWORD len = GetModuleFileName(NULL, curDir, PS_MAX_PATH);
        Log(logMESSAGE, "PerfStudio folder is %s\n", curDir);

        if ((len == 0) || (len == PS_MAX_PATH))
        {
            Log(logERROR, "Error: Unable to determine path to executable\n");
        }

        // remove the filename from the path, leaving just the path
        PathRemoveFileSpec(curDir);

        char dlls[PS_MAX_PATH * 2] = {};

        char shortDir[PS_MAX_PATH] = {};

        // Use the short path name, since spaces are delimiters in the registry key and can't be used
        // in path names
        DWORD result = GetShortPathName(curDir, shortDir, PS_MAX_PATH);
        Log(logMESSAGE, "PerfStudio short folder is %s\n", shortDir);

        sprintf_s(dlls, "%s\\" MICRODLLNAME GDT_DEBUG_SUFFIX GDT_BUILD_SUFFIX ".dll %s\\" MICRODLLNAME "-x64" GDT_DEBUG_SUFFIX GDT_BUILD_SUFFIX ".dll", shortDir, shortDir);

        Log(logMESSAGE, "PerfStudio appinit reg key is %s\n", dlls);

        result = RegSetValueEx(hKey, APPINIT_KEY, 0, REG_SZ, (const BYTE*)dlls, (DWORD)(strlen(dlls) + 1));

        DWORD value = 1;
        result = RegSetValueEx(hKey, LOADAPPINIT_KEY, 0, REG_DWORD, (const BYTE*)&value, 4);

        RegCloseKey(hKey);
    }
}

/// Clear out the AppInit_DLLs registry entries
void RestoreAppInit()
{
    HKEY hKey;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, APPINIT_PATH, 0L, KEY_ALL_ACCESS, &hKey))
    {
        DWORD result = RegSetValueEx(hKey, APPINIT_KEY, 0, REG_SZ, (const BYTE*)"", 1);

        DWORD value = 0;
        result = RegSetValueEx(hKey, LOADAPPINIT_KEY, 0, REG_DWORD, (const BYTE*)&value, 4);
        RegCloseKey(hKey);
    }
}
