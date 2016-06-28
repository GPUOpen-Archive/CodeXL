//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Interface for the server-side plugins of GPU PerfStudio. This declares
// the entrypoints that a dll must have so that it can be registered with the
// main server. These entry points are all that is needed by a global (or
// system profiling) plugin; Wrapper plugins must additionally implement the
// entry points that are declared in IWrapperPlugin.h
//==============================================================================

#ifndef GPS_PLUGIN_INTERFACE
#define GPS_PLUGIN_INTERFACE

//=============================================================================
//
// EntryPoints for RequestProcessing
//
// Implementations of a plugin must also implement those functions required
// by the IProcessRequest.h interface
//
//=============================================================================

#include "IProcessRequests.h"

//=============================================================================
//
// EntryPoints for Plugin Registration
//
//
//=============================================================================

//-----------------------------------------------------------------------------
/// GetPluginVersion
///
/// Provides the version number of the implemented plugin. This will be exposed
/// to client-side plugins so that they can require a minimum version of a
/// particular server-side plugin. All efforts should be made to maintain
/// backwards compatibility.
///
/// \return pointer to the version string; cannot be NULL
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetPluginVersion();

//-----------------------------------------------------------------------------
/// GetShortDescription
///
/// This provides a unique short (one word) description of the functionality of
/// the plugin. Ex: A plugin which wraps an API may send back the name of the
/// API, and a plugin which profiles a piece of hardware may send back the name
/// of the hardware which it profiles. If multiple plugins exist which have the
/// same short description, only one of them will be loaded for use.
///
/// \return pointer to the short description; cannot be NULL
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetShortDescription();

//-----------------------------------------------------------------------------
/// GetLongDescription
///
/// This provides a long (one sentence) description of the functionality of the
/// plugin. This string may be shown on the client-side so that users know the
/// functionality of the available server-side plugins.
///
/// \return pointer to the long description; cannot be NULL
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetLongDescription();

//=============================================================================
//
// EntryPoints for Wrapper Plugins
//
// Unhooking should happen on PROCESS_DETACH
//
//=============================================================================

//-----------------------------------------------------------------------------
/// GetWrappedDLLName
///
/// Provides the name of the dll which can be wrapped by this plugin.
///
/// \return pointer to the name of the dll; cannot be NULL
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetWrappedDLLName();

//-----------------------------------------------------------------------------
/// UpdateHooks
///
/// Function which causes all necessary entrypoints to be hooked
///
/// \return true if hooking was successful; false will unload the wrapper
//-----------------------------------------------------------------------------
GPS_PLUGIN_API bool UpdateHooks();

//-----------------------------------------------------------------------------
/// RegisterActivePlugin
///
/// Allows a plugin to register with the server to indicate that it is actively
/// used by the application. A prerequisite to being "actively used" is that
/// the plugin must be in a state that it can receive and respond to commands.
/// This function does not need to be implemented by a ServerPlugin.
///
/// \param strShortDescription Must be the same value
///        as is returned by GetShortDescription( )
/// \return true if the plugin could be registered; false otherwise
//-----------------------------------------------------------------------------
extern bool RegisterActivePlugin(const char* strShortDescription);

#endif // GPS_PLUGIN_INTERFACE
