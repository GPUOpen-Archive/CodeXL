//=====================================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DXGILayerManager.cpp
/// \brief  DXGI Layer Manager implementation
//=====================================================================================

#include "../Common/IServerPlugin.h"
#include "DXGILayerManager.h"
#include "DXGIHookDXGIFactory.h"
#include "DXGIHookSwapChain.h"
#include <list>
#include <d3d11_1.h>

#include "IDXGIFactoryWrapper.h"

/// Used to detect whene the executing code is being called from inside the runtime. It is used to mask out the effects of the runtime calling into our code.
RefTrackerCounter g_dwInsideDXGI;

/// A flag used to determine if a Server plugin's LayerManager has been added to the DXGILayerManager stack.
static bool s_bAPILayerManagerAlreadyInjected = false;

// The following exported functions are used by the DX11 and DX12 layers to tell the DXGIServer they exist.
// There are better ways of doing this but given the time constraints this should be ok.

/// Add a separate LayerManager to the DXGILayerManager's layer stack.
/// \param pLM The LayerManager to add to the DXGILayerManager's layer stack.
GPS_PLUGIN_API void SetLayerManager(LayerManager* pLM)
{
    if (DXGILayerManager::Instance() != NULL)
    {
        DXGILayerManager::Instance()->PushBack(pLM);

        s_bAPILayerManagerAlreadyInjected = true;
    }
}

/// Remove a separate LayerManager from the DXGILayerManager.
/// \param pLM The LayerManager to remove from the DXGILayerManager's layer stack.
GPS_PLUGIN_API void UnsetLayerManager(LayerManager* pLM)
{
    if (DXGILayerManager::Instance() != NULL)
    {
        DXGILayerManager::Instance()->Remove(pLM);
    }
}

//--------------------------------------------------------------------------
/// A function pointer type that will let us request a LayerManager pointer from a loaded Server plugin.
//--------------------------------------------------------------------------
typedef LayerManager* (*GetServerLayerManagerFunc)();

//--------------------------------------------------------------------------
/// In cases where no API servers have been connected automatically, attempt to discover and connect with an already-loaded server.
//--------------------------------------------------------------------------
void AttemptAttachToAPIServer()
{
    if (s_bAPILayerManagerAlreadyInjected == false)
    {
        Log(logTRACE, "DXGIServer searching for API servers to connect with.\n");

        HMODULE serverModule = NULL;
        HMODULE d3d11ServerModule = NULL;
        HMODULE d3d12ServerModule = NULL;
        // Attempt to latch on to a server module that may already be loaded.
#ifdef DLL_REPLACEMENT
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, "D3D11.dll", &d3d11ServerModule);
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, "D3D12.dll", &d3d12ServerModule);
#else
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, "DX11Server" AMDT_PROJECT_SUFFIX ".dll", &d3d11ServerModule);
#ifdef CODEXL_GRAPHICS
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, "CXLGraphicsServerDX12" AMDT_PROJECT_SUFFIX ".dll", &d3d12ServerModule);
#else // CODEXL_GRAPHICS
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, "DX12Server" AMDT_PROJECT_SUFFIX ".dll", &d3d12ServerModule);
#endif // CODEXL_GRAPHICS
#endif

        // Check if either of the modules were found within the process.
        if (d3d11ServerModule != NULL && d3d12ServerModule != NULL)
        {
            Log(logWARNING, "D3D11 and D3D12 found within process during AttempAttachToAPIServer.\n");
        }

        if (d3d11ServerModule != NULL)
        {
            serverModule = d3d11ServerModule;
        }

        if (d3d12ServerModule != NULL)
        {
            serverModule = d3d12ServerModule;
        }

        // If a module was found, we can attempt to include it in our Layer stack.
        if (serverModule != NULL)
        {
            // At this point, the module will be either the D3D11 or D3D12 server.
            const char* moduleName = (d3d11ServerModule != NULL) ? "DX11Server" : "DX12Server";

            // Latch onto an exported function to retrieve the server's main LayerManager.
            GetServerLayerManagerFunc getServerLayerManager = (GetServerLayerManagerFunc)GetProcAddress(serverModule, "GetServerLayerManager");

            if (getServerLayerManager != NULL)
            {
                // Now insert the server's LayerManager into the DXGILayerManager's stack.
                LayerManager* serverLayerManager = getServerLayerManager();
                SetLayerManager(serverLayerManager);

                Log(logTRACE, "Successfully inserted the '%s' instance to the DXGIServer stack.\n", moduleName);

                s_bAPILayerManagerAlreadyInjected = true;
            }
            else
            {
                Log(logERROR, "Discovered server module named '%s', but couldn't find 'GetServerLayerManager' export.\n", moduleName);
            }
        }
    }
}

/// Constructor
DXGILayerManager::DXGILayerManager()
{
    m_LayersList.clear();

    AddCommand(CONTENT_TEXT, "Info", "Info", "Info.txt", NO_DISPLAY, INCLUDE, m_Info);
}

/// Default public destructor
DXGILayerManager::~DXGILayerManager()
{
}

/// Called when a known device / context is created
/// \param type the creation object type.
/// \param pPtr Pointer to the object that was just created.
/// \return True if success, False if fail.
bool DXGILayerManager::OnCreate(CREATION_TYPE type, void* pPtr)
{
    // DXGILayerManager is interested in Factory and Swapchain objects.
    if (type == DXGI_FACTORY)
    {
        // Attach to the new factory object.
        DWORD dwRefCnt = HookIDXGIFactory((IDXGIFactory4*) pPtr);
        Log(logDEBUG, "%s() - Factory 0x%p, created: %u\n", __FUNCTION__, pPtr, dwRefCnt);
        Log(logMESSAGE, "%s() - Factory 0x%p, created: %u\n", __FUNCTION__, pPtr, dwRefCnt);

        if (dwRefCnt == 1)
        {
            Log(logDEBUG, "RegisterActivePlugin(DXGI)\n");
            RegisterActivePlugin("DXGI");
        }
    }
    else if (type == DX11_SWAPCHAIN)
    {
        // Attach to the new swapchain object.
        IDXGISwapChain* pSch = (IDXGISwapChain*)pPtr;
        DWORD dwRef = HookIDXGISwapChain(pSch);
        Log(logMESSAGE, "%s() - Swapchains created: %u\n", __FUNCTION__, dwRef);
    }

    AttemptAttachToAPIServer();

    std::list <LayerManager*>::iterator layersIter;

    // Tell the other layers about the new object
    for (layersIter = m_LayersList.begin(); layersIter != m_LayersList.end(); ++layersIter)
    {
        (*layersIter)->OnCreate(type, pPtr);
    }

    return true;
}

/// Called when a known device / context is destroyed
/// \param type the type of object that is being destroyed
/// \param pPtr pointer to the object
bool DXGILayerManager::OnDestroy(CREATION_TYPE type, void* pPtr)
{
    Log(logMESSAGE, "%s() - ptr 0x%p, \n", __FUNCTION__, pPtr);

    // now detach and release layer manager stuff
    if (type == DXGI_FACTORY)
    {
#ifndef USE_DXGI_WRAPPER
        DWORD dwRefCnt = UnhookIDXGIFactory();
        Log(logMESSAGE, "%s() - Factory 0x%p, Remaining: %u\n", __FUNCTION__, pPtr, dwRefCnt);
#endif
    }
    else if (type == DX11_SWAPCHAIN)
    {
        // Need For Speed Most Wanted
        // For some reason the swapcchain is getting released just after startup
        DWORD dwRef = UnhookIDXGISwapChain();
        Log(logMESSAGE, "%s() - Swapchains created: %u\n", __FUNCTION__, dwRef);
    }

    AttemptAttachToAPIServer();

    std::list <LayerManager*>::iterator layersIter;

    for (layersIter = m_LayersList.begin(); layersIter != m_LayersList.end(); ++layersIter)
    {
        (*layersIter)->OnDestroy(type, pPtr);
    }

    return true;
}

/// Gets called 2nd after the swapchain present
void DXGILayerManager::BeginFrame()
{
    AttemptAttachToAPIServer();

    std::list <LayerManager*>::iterator layersIter;

    for (layersIter = m_LayersList.begin(); layersIter != m_LayersList.end(); ++layersIter)
    {
        (*layersIter)->BeginFrame();
    }
}

/// Gets called 1st after the swapchain present.
void DXGILayerManager::EndFrame()
{
    AttemptAttachToAPIServer();

    std::list <LayerManager*>::iterator layersIter;

    for (layersIter = m_LayersList.begin(); layersIter != m_LayersList.end(); ++layersIter)
    {
        (*layersIter)->EndFrame();
    }
}

/// Gets called after a frame has been presented.
/// \param inSwapchain The swapchain instance used to present to the screen.
void DXGILayerManager::OnPresent(void* inSwapchain)
{
    AttemptAttachToAPIServer();

    std::list <LayerManager*>::iterator layersIter;

    for (layersIter = m_LayersList.begin(); layersIter != m_LayersList.end(); ++layersIter)
    {
        (*layersIter)->OnPresent(inSwapchain);
    }
}

/// Gets called immediately after the real Present() is called
/// \param SyncInterval The sync interval passed into the real Present() call.
/// \param Flags The flags passed into the real Present() call.
void DXGILayerManager::OnPresent(UINT SyncInterval, UINT Flags)
{
    std::list <LayerManager*>::iterator layersIter;

    for (layersIter = m_LayersList.begin(); layersIter != m_LayersList.end(); ++layersIter)
    {
        (*layersIter)->OnPresent(SyncInterval, Flags);
    }
}

/// Gets called when a swapchain is created.
/// \param inSwapchain The swapchain instance that was just created.
/// \param inDevice The device used to created the swapchain.
void DXGILayerManager::OnSwapchainCreated(void* inSwapchain, void* inDevice)
{
    AttemptAttachToAPIServer();

    std::list <LayerManager*>::iterator layersIter;

    for (layersIter = m_LayersList.begin(); layersIter != m_LayersList.end(); ++layersIter)
    {
        (*layersIter)->OnSwapchainCreated(inSwapchain, inDevice);
    }
}

/// Adds a layer manager to the list
/// \param pLM The layer manager to add.
void DXGILayerManager::PushBack(LayerManager* pLM)
{
    std::list<LayerManager*>::iterator findIter = std::find(m_LayersList.begin(), m_LayersList.end(), pLM);
    if (findIter == m_LayersList.end())
    {
        m_LayersList.push_back(pLM);
    }
    else
    {
        Log(logERROR, "Prevented duplicate Layer from being pushed within DXGILayerManager.\n");
    }
}

/// Removes a layer manager from the list
/// \param pLM The layer manager to remove.
void DXGILayerManager::Remove(LayerManager* pLM)
{
    if (m_LayersList.empty() == false)
    {
        m_LayersList.remove(pLM);
    }
}

/// Respond to Commands.
/// \param pSwapChain The Swapchain instance that has just been presented to the screen.
void DXGILayerManager::RespondToCommands(IDXGISwapChain* pSwapChain)
{
    GetPendingRequests();

    // Returns the graphics API the game is using.
    if (m_Info.IsActive())
    {
        IUnknown* pObj;
        char* pStr = "Unknown";

        if (pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pObj) == S_OK)
        {
            pStr = "ID3D11Device";
            pObj->Release();
        }
        else if (pSwapChain->GetDevice(__uuidof(ID3D10Device1), (void**)&pObj) == S_OK)
        {
            pStr = "ID3D10Device1";
            pObj->Release();
        }
        else if (pSwapChain->GetDevice(__uuidof(ID3D10Device), (void**)&pObj) == S_OK)
        {
            pStr = "ID3D10Device";
            pObj->Release();
        }

        m_Info.Send(pStr);
    }
}

