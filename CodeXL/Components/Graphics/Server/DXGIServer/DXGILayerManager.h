//=====================================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DXGILayerManager.h
/// \brief  DXGI Layer Manager interface declaration.
//=====================================================================================

#ifndef DXGI_LAYERMANAGER_H
#define DXGI_LAYERMANAGER_H

#include "../Common/LayerManager.h"
#include "../Common/TSingleton.h"
#include "../Common/StreamLog.h"
#include "DXGIHookDXGIFactory.h"
#include "DXGIHookSwapChain.h"
#include <list>

extern RefTrackerCounter g_dwInsideDXGI;
extern bool CheckUpdateHooks();

/// Specializes the base functionality of the LayerManager for use win the DXGI server.
/// It maintains a list of Layer Managers (i.e. one for DX10 and one for DX11). We use this list to communicate
/// with the DX10 and DX11 servers. The LayerManager is a singleton accessed from inside the loacl DllMain.cpp,
/// IDXGIFactory1_DXGIServer and IDXGIFactory_DXGIServer code.
class DXGILayerManager : public LayerManager, public TSingleton<DXGILayerManager>
{
    /// The base class needs to be a friend to access our constructor.
    friend TSingleton<DXGILayerManager>;

    /// List of active layer managers
    std::list<LayerManager*> m_LayersList;

    /// Constructor
    DXGILayerManager();

    /// Command to send the 3D API that is using the DXGI
    CommandResponse m_Info;

public:
    /// Default public destructor
    ~DXGILayerManager();

    /// Called when a known device / context is created
    /// \param type the creation object type.
    /// \param pPtr Pointer to the object that was just created.
    /// \return True if success, False if fail.
    bool OnCreate(CREATION_TYPE type, void* pPtr);

    /// Called when a known device / context is destroyed
    /// \param type the type of object that is being destroyed
    /// \param pPtr pointer to the object
    bool OnDestroy(CREATION_TYPE type, void* pPtr);

    /// Gets called 2nd after the swapchain present.
    void BeginFrame();

    /// Gets called 1st after the swapchain present.
    void EndFrame();

    /// Gets called after a frame has been presented.
    /// \param inSwapchain The swapchain instance used to present to the screen.
    void OnPresent(void* inSwapchain);

    /// Gets called immediately after the real Present() is called
    /// \param SyncInterval The sync interval passed into the real Present() call.
    /// \param Flags The flags passed into the real Present() call.
    void OnPresent(UINT SyncInterval, UINT Flags);

    /// Gets called when a swapchain is created.
    /// \param inSwapchain The swapchain instance that was just created.
    /// \param inDevice The device used to created the swapchain.
    void OnSwapchainCreated(void* inSwapchain, void* inDevice);

    /// Adds a layer manager to the list
    /// \param pLM The layer manager to add.
    void PushBack(LayerManager* pLM);

    /// Removes a layer manager from the list
    /// \param pLM The layer manager to remove.
    void Remove(LayerManager* pLM);

    /// Respond to Commands.
    /// \param pSwapChain The Swapchain instance that has just been presented to the screen.
    void RespondToCommands(IDXGISwapChain* pSwapChain);

    /// A StreamLog member to be used for multithreaded logging.
    StreamLog mStreamLog;
};

#endif