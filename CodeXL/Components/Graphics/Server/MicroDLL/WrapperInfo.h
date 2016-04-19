//=====================================================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief Functions that manage loading of plugins (graphics servers)
//=====================================================================================

#ifndef GPS_WRAPPERINFO_INCLUDED
#define GPS_WRAPPERINFO_INCLUDED

#include <windows.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

//--------------------------------------------------------------
// WrapperInfo Struct
/// This structure is smaller than the one needed by the
/// WebServer, but it is all the information that is needed about
/// the wrappers to be able to inject them into another process
//--------------------------------------------------------------
struct WrapperInfo
{
    gtASCIIString strPluginPath;       ///< full path to the dll
    gtASCIIString strPluginName;       ///< the name of the dll
    gtASCIIString strWrappedDll;       ///< the dll that gets wrapped by this wrapper
    HMODULE  hWrapperLoaded;      ///< handle to the currently loaded instance of the wrapped DLL
};

//--------------------------------------------------------------
/// Stores information about each of the available wrappers
/// in the wrapper array
//--------------------------------------------------------------
void CollectWrapperInfo();

/// Checks to see if a wrapper is needed when the application calls LoadLibrary
void CheckWrapperOnLoadLibrary();

/// Checks to see if a wrapper is still needed after the application calls FreeLibrary
void CheckWrapperOnFreeLibrary();

//--------------------------------------------------------------
/// Call UpdateHooks for a replaced Dll (if it's loaded) when
/// the application calls LoadLibrary
//--------------------------------------------------------------
void UpdateHooksOnLoadLibrary();

#endif // GPS_WRAPPERINFO_INCLUDED
