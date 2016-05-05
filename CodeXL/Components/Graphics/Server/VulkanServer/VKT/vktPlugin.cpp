//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktPlugin.cpp
/// \brief  Implementation file for a Vulkan-specific GPS plug-in.
///         All GPS plug-ins must implement this interface.
//==============================================================================

#include "vktLayerManager.h"

#include "../../Common/IServerPlugin.h"
#include "../../Common/PerfStudioServer_Version.h"
#include "../../Common/misc.h"
#include "../../Common/ICommunication.h"

//-----------------------------------------------------------------------------
/// Exported function used in stepping the message pump for the server plugin.
/// \param requestID The ID of a request that is in waiting to be processed.
/// This ID can be used to query the contents of an incoming request.
/// \return true if the request could be processed; false otherwise
//-----------------------------------------------------------------------------
GPS_PLUGIN_API bool ProcessRequest(CommunicationID requestID)
{
    return VktLayerManager::GetLayerManager()->ProcessRequestFromCommId(requestID);
}

//-----------------------------------------------------------------------------
/// Provides the version number of the implemented plugin. This will be exposed
/// to client-side plugins so that they can require a minimum version of a
/// particular server-side plugin. All efforts should be made to maintain
/// backwards compatibility.
/// \return pointer to the version string; cannot be nullptr
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetPluginVersion()
{
    return PERFSTUDIOSERVER_VERSION_STRING;
}

//-----------------------------------------------------------------------------
/// This provides a unique short (one word) description of the functionality of
/// the plugin. Ex: A plugin which wraps an API may send back the name of the
/// API, and a plugin which profiles a piece of hardware may send back the name
/// of the hardware which it profiles. If multiple plugins exist which have the
/// same short description, only one of them will be loaded for use.
/// \return pointer to the short description; cannot be nullptr
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetShortDescription()
{
    return "Vulkan";
}

//-----------------------------------------------------------------------------
/// This provides a long (one sentence) description of the functionality of the
/// plugin. This string may be shown on the client-side so that users know the
/// functionality of the available server-side plugins.
/// \return pointer to the long description; cannot be nullptr
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetLongDescription()
{
    return "Provides Vulkan API interception.";
}

//-----------------------------------------------------------------------------
/// Provides the name of the dll which can be wrapped by this plugin.
/// \return pointer to the name of the dll; cannot be nullptr
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetWrappedDLLName()
{
    return "vulkan.0.dll";
}

//-----------------------------------------------------------------------------
/// Function which causes all necessary entrypoints to be hooked.
/// \return true if hooking was successful; false will unload the wrapper
//-----------------------------------------------------------------------------
GPS_PLUGIN_API bool UpdateHooks()
{
    bool success = false;

    if (VktLayerManager::GetLayerManager()->HasBeenInitialized() == false)
    {
        bool commInitialized = InitCommunication(GetShortDescription(), ProcessRequest);

        if (commInitialized)
        {
            success = RegisterActivePlugin(GetShortDescription());
        }
        else
        {
            DeinitCommunication();
        }
    }
    else
    {
        success = true;
    }

    return success;
}