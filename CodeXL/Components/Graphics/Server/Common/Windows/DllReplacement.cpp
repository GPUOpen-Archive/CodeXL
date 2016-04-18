//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Miscellaneous helper functions for DLL Replacement functionality.
///         See header file for function definition help
//==============================================================================

/// Definition
#define WIN32_LEAN_AND_MEAN 1

#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include "../Logger.h"
#include "../SharedGlobal.h"

namespace DllReplacement
{

//--------------------------------------------------------------
/// LoadRealLibrary: Load in the real library from the Windows
/// system folder. Called by the replacement or proxy dll to get
/// the real function calls
///
/// \param libName The name of the library to load
/// \return handle to the opened library or NULL if error
//--------------------------------------------------------------
HINSTANCE LoadRealLibrary(const char* libName)
{
    HINSTANCE hl = 0;
    char sysFolder[MAX_PATH + 1];
    char fullName[MAX_PATH + 1];

    // returns length of buffer or 0 if error
    UINT length = GetSystemDirectory(sysFolder, MAX_PATH);

    if (length > 0)
    {
        sprintf_s(fullName, MAX_PATH, "%s\\%s", sysFolder, libName);
        hl = LoadLibrary(fullName);
    }

    return hl;
}

//--------------------------------------------------------------
/// Set the DLL directory to where the replaced system dll's reside.
/// The dll names are the same for 32 and 64 bit, so they are placed in
/// the Plugins folder in x86 and x64 subfolders. The Windows loader
/// will get the replaced Dll's rather than the system Dll's
///
/// \param is64Bit Whether the 64-bit directory is needed. Set to
/// true if 64-bit, false for 32-bit
//--------------------------------------------------------------
void SetDllDirectory(bool is64bit)
{
    // Get PerfStudio folder and add the subfolder
    gtASCIIString dllDir = SG_GET_PATH(ServerPath);
    dllDir += "Plugins\\x";

    // add correct subfolder
    if (is64bit)
    {
        dllDir += "64";
    }
    else
    {
        dllDir += "86";
    }

    ::SetDllDirectory(dllDir.asCharArray());
}

} // namespace DllReplacement