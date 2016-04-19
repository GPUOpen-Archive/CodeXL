//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains helper functions for the MicroDLL plugin.
//==============================================================================

#ifndef _PLUGIN_INFO_H_
#define _PLUGIN_INFO_H_

#include <string>
#include <windows.h>

/// \addtogroup MicroDLL
// @{

typedef bool (WINAPI* UpdateHooks_type)();

struct PluginInfo
{
    std::string strPluginPath;  ///< contains the plugin's file path
    std::string strPluginName;  ///< the plugin name
    HMODULE hModule;            ///< the module handle
    bool shouldForceLoad;       ///< should we call LoadLibrary on the DLL?

    /// Constructor.
    PluginInfo() : hModule(NULL), shouldForceLoad(false)
    {
    }

    /// Check whether the module has been loaded.
    /// \return true if the module has been loaded, false otherwise
    bool IsLoaded()
    {
        return (hModule != NULL);
    }
};

/// Load DCServer.dll before we detour LoadLibrary
void InitAvailablePluginInfo();

/// This function calls UpdateHooks for each plugin dll;
/// the function is called on every load library call detected in
/// the application.
void CheckOnLoadLibrary();

/// Checks if the lib being unloaded is one of our plugins, so we can mark it as unloaded
void CheckOnFreeLibrary();

/// Call the specified function defined in the plugin
/// \param[in] hLib  the plugin handle
/// \param[in] fucntionName  the name of the exported function to call
/// \return true if successful, false otherwise
bool CallFunction(HMODULE hLib, LPCSTR functionName);

/// This function calls OnExitProcess for each plugin dll
/// It is called when ExitProcess is called by the application
void NotifyOnExitProcess();

/// Check WinDir\System32\d3d11.dll, WinDir\SysWOW64\d3d11.dll
/// return true if file exists
bool CheckDXRuntime();

/// Check WinDir\System32\OpenCL.dll, WinDir\SysWOW64\OpenCL.dll and currentPath\OpenCL.dll
/// return true if file exists
bool CheckCLRuntime();

// @}

#endif
