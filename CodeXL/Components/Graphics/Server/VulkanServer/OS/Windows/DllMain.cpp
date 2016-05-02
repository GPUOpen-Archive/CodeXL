//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DllMain.cpp
/// \brief  Implementation file for Vulkan server DLL main.
///         Initializes and destroys the Vulkan layer manager.
//==============================================================================

#include "../../VKT/vktLayerManager.h"

//-----------------------------------------------------------------------------
///  Good old DllMain.
//-----------------------------------------------------------------------------
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD dwReason, VOID* pReserved)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(pReserved);

    Log(logMESSAGE, "VulkanServer's DllMain hit with reason '%d'\n", dwReason);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            VktLayerManager* pLayerManager = VktLayerManager::GetLayerManager();

            if (pLayerManager->HasBeenInitialized() == false)
            {
                bool initialized = pLayerManager->InitializeLayerManager();

                if (initialized == false)
                {
                    Log(logWARNING, "The VulkanLayerManager was not initialized successfully.\n");
                }
            }
        }
        break;

        case DLL_PROCESS_DETACH:
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
        break;
    }

    return true;
}