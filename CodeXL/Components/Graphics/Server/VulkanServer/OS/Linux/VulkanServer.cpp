//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   VulkanServer.cpp
/// \brief  Implementation file for Vulkan server main entry point.
///         Initializes and destroys the Vulkan layer manager.
//==============================================================================

#include "../../../Common/Linux/ServerUtils.h"

#include "../../VKT/vktLayerManager.h"

#define SERVER_NAME "VulkanServer"

static bool s_bInitialized = false;

//-----------------------------------------------------------------------------
/// Linux Shared Library Constructor
///
/// Only limited setup is allowed here. Trying to initialize global data, particularly
/// STL objects, will lead to a seg fault, probably because the shared library isn't
/// fully loaded at this point
//-----------------------------------------------------------------------------
__attribute__((constructor))
static void ctor()
{
    if (ServerUtils::CanBind(program_invocation_short_name))
    {
        Log(logMESSAGE, "VulkanServer's constructor hit\n");

        VktLayerManager* pLayerManager = VktLayerManager::GetLayerManager();

        if (pLayerManager->HasBeenInitialized() == false)
        {
            bool initialized = pLayerManager->InitializeLayerManager();

            if (initialized == false)
            {
                Log(logWARNING, "The VulkanLayerManager was not initialized successfully.\n");
            }
            else
            {
                s_bInitialized = true;
                ServerUtils::CheckForDebuggerAttach(SERVER_NAME, s_bInitialized);
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Linux Shared Library Destructor
//-----------------------------------------------------------------------------
__attribute__((destructor))
static void dtor()
{
    // Only shutdown the VulkanLayerManager if it was initialized.
    VktLayerManager* pLayerManager = VktLayerManager::GetLayerManager();

    if (pLayerManager->HasBeenInitialized())
    {
        if (pLayerManager->ShutdownLayerManager() == false)
        {
            Log(logWARNING, "The VulkanLayerManager was not shutdown successfully.\n");
        }
    }
    else
    {
        Log(logERROR, "VulkanServer shutdown: The VulkanLayerManager was not initialized.\n");
    }
}
