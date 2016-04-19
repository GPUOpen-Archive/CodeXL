//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Used to share the LayerManager pointer with the DXGI server so that it
/// can pass the Present() API call into the LayerManager so it can be handled by
/// the various server layers.
//==============================================================================

#ifndef CONNECTWITHDXGI_H
#define CONNECTWITHDXGI_H

#include "LayerManager.h"

/// Function pointer used to set the layer manager on the DXGI server
typedef void (*SetLayerManager_type)(LayerManager* pLM);

/// Function pointer used to unset the layer manager on the DXGI server
typedef void (*UnsetLayerManager_type)(LayerManager* pLM);

/// A flag to prevent us from registering with the DXGI server more than once
static bool s_ConnectedWithDXGI = false;

/// Registers a LayerManager with the DXGI server
/// \param pLayerManager the layer manager to register
void ConnectWithDXGI(LayerManager* pLayerManager)
{
    if (s_ConnectedWithDXGI == false)
    {
        SetLayerManager_type SLM = NULL;

        HMODULE hDXGIModule = NULL;

        // Get the DXGI dll
#ifdef DLL_REPLACEMENT
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, "dxgi.dll", &hDXGIModule);
#else
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, "DXGIServer" GDT_PROJECT_SUFFIX ".dll", &hDXGIModule);
#endif

        if (hDXGIModule != NULL)
        {
            // Get the process address of the SetLayerManager function
            SLM = (SetLayerManager_type)GetProcAddress(hDXGIModule, "SetLayerManager");

            if (SLM != NULL)
            {
                // Set the layer manager pointer inside the DXGI server
                SLM(pLayerManager);
                s_ConnectedWithDXGI = true;
                Log(logTRACE, "Connected with DXGIServer successfully.\n");
            }
            else
            {
                Log(logWARNING, "Attempted connection with DXGIServer, but failed to find 'SetLayerManager' entrypoint in module.\n");
            }
        }
        else
        {
            Log(logWARNING, "Attempted connection with DXGIServer, but the module wasn't found loaded within the process.\n");
        }
    }
}

/// Unregisters a LayerManager from the DXGI server
/// \param pLayerManager the layer manager to unregister
void DisconnectFromDXGI(LayerManager* pLayerManager)
{
    if (s_ConnectedWithDXGI == true)
    {
        UnsetLayerManager_type USLM = NULL;

        HMODULE hDXGIModule = NULL;

        // Get the DXGI dll
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, "DXGIServer" GDT_PROJECT_SUFFIX ".dll", &hDXGIModule);

        // Get the process address of the UnsetLayerManager function
        USLM = (UnsetLayerManager_type)GetProcAddress(hDXGIModule, "UnsetLayerManager");

        if (USLM != NULL)
        {
            // Unset the layer manager pointer inside the DXGI server
            USLM(pLayerManager);
            s_ConnectedWithDXGI = false;
        }
    }
}

#endif