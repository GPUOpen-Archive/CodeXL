//=====================================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief DXGIFactory wrapper
//=====================================================================================

#ifndef DXGI__FACTORY_WRAPPER_H
#define DXGI__FACTORY_WRAPPER_H

#include <dxgi1_4.h>
#include "DXGIHookSwapChain.h"
#include "..\common\IUnknownWrapperGUID.h"

/// Define the wrapper level for DXGIFactory
#define DXGI_FACTORY_WRAPPER_BASECLASS IDXGIFactory4

/// Use DXGI object wrapping
#define USE_DXGI_WRAPPER

#ifdef USE_DXGI_WRAPPER

//#define DEBUG_LOG

/// This class is responsible for wrapping all of the DXGI Factory types. We treat all DXGI factories as factory2's.
class IDXGIFactoryWrapper : public DXGI_FACTORY_WRAPPER_BASECLASS
{
    /// Store the real factory pointer
    DXGI_FACTORY_WRAPPER_BASECLASS* m_pReal;

public:

    /// Constructor
    /// \param pReal Real interface pointer
    IDXGIFactoryWrapper(void* pReal)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: Constructor: pReal 0x%p\n", pReal);
#endif

        IUnknown* pUnknown = static_cast<IUnknown*>(pReal);
        void* pTmp = NULL;

        if (pUnknown->QueryInterface(IID_IWrappedObject, &pTmp) == S_OK)
        {
            // The input object is already wrapped.
            Log(logDEBUG, "IDXGIFactoryWrapper: Constructor: pReal 0x%p is already wrapped!!\n", pReal);
        }

        m_pReal = static_cast<DXGI_FACTORY_WRAPPER_BASECLASS*>(pReal);
    }

    /// Wrap the QueryInterface method
    /// \param riid Input interface ID
    /// \param ppvObject Output object pointer
    /// \return The HRESULT
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid,
        _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        // Check if real interface is required
        if (riid == IID_IWrappedObject)
        {
            //Log(logDEBUG, "IDXGIFactoryWrapper: QueryInterface(IID_IWrappedObject) - Returning m_pReal\n");
            *ppvObject = m_pReal;
            return S_OK;
        }

        if (riid == __uuidof(IDXGIFactory))
        {
            //Log(logDEBUG, "IDXGIFactoryWrapper: QueryInterface(IDXGIFactory)\n");

            HRESULT hRes = m_pReal->QueryInterface(riid, ppvObject);

            if (hRes != S_OK)
            {
                *ppvObject = NULL;
                return hRes;
            }

            *ppvObject = this;
            return hRes;
        }
        else if (riid == __uuidof(IDXGIFactory1))
        {
            //Log(logDEBUG, "IDXGIFactoryWrapper: QueryInterface(IDXGIFactory1)\n");

            HRESULT hRes = m_pReal->QueryInterface(riid, ppvObject);

            if (hRes != S_OK)
            {
                *ppvObject = NULL;
                return hRes;
            }

            *ppvObject = this;
            return hRes;
        }
        else if (riid == __uuidof(IDXGIFactory2))
        {
            //Log(logDEBUG, "IDXGIFactoryWrapper: QueryInterface(IDXGIFactory2)\n");

            HRESULT hRes = m_pReal->QueryInterface(riid, ppvObject);

            if (hRes != S_OK)
            {
                //Log(logDEBUG, "IDXGIFactoryWrapper: QueryInterface(IDXGIFactory2) - Returning E_NOINTERFACE\n");
                *ppvObject = NULL;
                return hRes;
            }

            *ppvObject = this;
            return S_OK;
        }
        else if (riid == __uuidof(IUnknown))
        {
            //Log(logDEBUG, "IDXGIFactoryWrapper: QueryInterface(IDXGIFactory2)\n");

            HRESULT hRes = m_pReal->QueryInterface(riid, ppvObject);

            if (hRes != S_OK)
            {
                //Log(logDEBUG, "IDXGIFactoryWrapper: QueryInterface(IDXGIFactory2) - Returning E_NOINTERFACE\n");
                *ppvObject = NULL;
                return hRes;
            }

            *ppvObject = this;
            return S_OK;
        }
        else
        {
            Log(logERROR, "IDXGIFactoryWrapper: QueryInterface(UNKNOWN) unsupported riid %ld\n", riid);
        }

        return m_pReal->QueryInterface(riid, ppvObject);
    }

    /// Add a reference
    /// \return Current ref count
    virtual ULONG STDMETHODCALLTYPE AddRef(void)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif
        return m_pReal->AddRef();
    }

    /// Release a reference
    /// \return Current ref count
    virtual ULONG STDMETHODCALLTYPE Release(void)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->Release();
    }

    /// Wrap the SetPrivateData method
    /// \param Name Input name
    /// \param DataSize Input datasize
    /// \param pData Input data
    /// \return The HRESULT
    virtual HRESULT STDMETHODCALLTYPE SetPrivateData(
        REFGUID Name,
        UINT DataSize,
        const void* pData)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif
        return m_pReal->SetPrivateData(Name, DataSize, pData);
    }

    /// Wrap the SetPrivateDataInterface method
    /// \param Name Input name
    /// \param pUnknown The input object
    /// \return The HRESULT
    virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(
        REFGUID Name,
        const IUnknown* pUnknown)
    {
        return m_pReal->SetPrivateDataInterface(Name, pUnknown);
    }

    /// Wrap the GetPrivateData method
    /// \param Name Input name
    /// \param pDataSize Output data size
    /// \param pData Output data
    /// \return The HRESULT
    virtual HRESULT STDMETHODCALLTYPE GetPrivateData(
        REFGUID Name,
        UINT* pDataSize,
        void* pData)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->GetPrivateData(Name, pDataSize, pData);
    }

    /// Wrap the GetParent method
    /// \param riid Input ref id of the interface
    /// \param ppParent Output interface
    /// \return The HRESULT
    virtual HRESULT STDMETHODCALLTYPE GetParent(
        REFIID riid,
        void** ppParent)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->GetParent(riid, ppParent);
    }

    /// Wrap the EnumAdapters member function
    virtual HRESULT STDMETHODCALLTYPE EnumAdapters(
        UINT Adapter,
        IDXGIAdapter** ppAdapter)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->EnumAdapters(Adapter, ppAdapter);
    }

    /// Wrap the MakeWindowAssociation member function
    virtual HRESULT STDMETHODCALLTYPE MakeWindowAssociation(
        HWND WindowHandle,
        UINT Flags)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->MakeWindowAssociation(WindowHandle, Flags);
    }

    /// Wrap the GetWindowAssociation member function
    virtual HRESULT STDMETHODCALLTYPE GetWindowAssociation(HWND* pWindowHandle)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif
        return m_pReal->GetWindowAssociation(pWindowHandle);
    }

    /// Wrap thye CreateSwapChain member function
    virtual HRESULT STDMETHODCALLTYPE CreateSwapChain(
        IUnknown* pDevice,
        DXGI_SWAP_CHAIN_DESC* pDesc,
        IDXGISwapChain** ppSwapChain)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        IUnknown* unwrappedDevice = NULL;
        HRESULT wasWrapped = pDevice->QueryInterface(IID_IWrappedObject, (void**)&unwrappedDevice);

        if (wasWrapped == S_OK)
        {
            pDevice = unwrappedDevice;
        }

        HRESULT hRes = m_pReal->CreateSwapChain(pDevice, pDesc, ppSwapChain);

        if (hRes == S_OK)
        {
            LayerManager* pLM = DXGILayerManager::Instance();
            pLM->OnSwapchainCreated(*ppSwapChain, pDevice);

            if (pLM != NULL)
            {
                //Log(logDEBUG, "IDXGIFactoryWrapper: CreateSwapChainForHwnd: DXGILayerManager::Instance()->OnCreate()\n");
                pLM->OnCreate(DX11_SWAPCHAIN, *ppSwapChain);
            }
        }

        return hRes;
    }

    /// Wrap the CreateSoftwareAdapter member function
    virtual HRESULT STDMETHODCALLTYPE CreateSoftwareAdapter(
        HMODULE Module,
        IDXGIAdapter** ppAdapter)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->CreateSoftwareAdapter(Module, ppAdapter);
    }

    /// Wrap the EnumAdapters1 member function
    virtual HRESULT STDMETHODCALLTYPE EnumAdapters1(
        UINT Adapter,
        IDXGIAdapter1** ppAdapter)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->EnumAdapters1(Adapter, ppAdapter);
    }

    /// Wrap the IsCurrent member function
    virtual BOOL STDMETHODCALLTYPE IsCurrent(void)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->IsCurrent();
    }

    /// Wrap the IsWindowedStereoEnabled member function
    virtual BOOL STDMETHODCALLTYPE IsWindowedStereoEnabled(void)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->IsWindowedStereoEnabled();
    }

    /// Wrap the CreateSwapChainForHwnd member function
    virtual HRESULT STDMETHODCALLTYPE CreateSwapChainForHwnd(
        IUnknown* pDevice,
        HWND hWnd,
        const DXGI_SWAP_CHAIN_DESC1* pDesc,
        const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc,
        IDXGIOutput* pRestrictToOutput,
        IDXGISwapChain1** ppSwapChain)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        IUnknown* unwrappedDevice = NULL;
        HRESULT wasWrapped = pDevice->QueryInterface(IID_IWrappedObject, (void**)&unwrappedDevice);

        if (wasWrapped == S_OK)
        {
            pDevice = unwrappedDevice;
        }

        HRESULT hRes = m_pReal->CreateSwapChainForHwnd(pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);

        if (hRes == S_OK)
        {
            LayerManager* pLM = DXGILayerManager::Instance();
            pLM->OnSwapchainCreated(*ppSwapChain, pDevice);

            if (pLM != NULL)
            {
                //Log(logDEBUG, "IDXGIFactoryWrapper: CreateSwapChainForHwnd: DXGILayerManager::Instance()->OnCreate()\n");
                pLM->OnCreate(DX11_SWAPCHAIN, *ppSwapChain);
            }
        }

        return hRes;
    }

    /// Wrap the CreateSwapChainForCoreWindow member function
    virtual HRESULT STDMETHODCALLTYPE CreateSwapChainForCoreWindow(
        IUnknown* pDevice,
        IUnknown* pWindow,
        const DXGI_SWAP_CHAIN_DESC1* pDesc,
        IDXGIOutput* pRestrictToOutput,
        IDXGISwapChain1** ppSwapChain)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        IUnknown* unwrappedDevice = NULL;
        HRESULT wasWrapped = pDevice->QueryInterface(IID_IWrappedObject, (void**)&unwrappedDevice);

        if (wasWrapped == S_OK)
        {
            pDevice = unwrappedDevice;
        }

        HRESULT hRes = m_pReal->CreateSwapChainForCoreWindow(pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);

        if (hRes == S_OK)
        {
            LayerManager* pLM = DXGILayerManager::Instance();
            pLM->OnSwapchainCreated(*ppSwapChain, pDevice);

            if (pLM != NULL)
            {
                //Log(logDEBUG, "IDXGIFactoryWrapper: CreateSwapChainForCoreWindow: DXGILayerManager::Instance()->OnCreate()\n");
                pLM->OnCreate(DX11_SWAPCHAIN, *ppSwapChain);
            }
        }

        return hRes;
    }

    /// Wrap the GetSharedResourceAdapterLuid member function
    virtual HRESULT STDMETHODCALLTYPE GetSharedResourceAdapterLuid(
        HANDLE hResource,
        LUID* pLuid)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->GetSharedResourceAdapterLuid(hResource, pLuid);
    }

    /// Wrap the RegisterStereoStatusWindow member function
    virtual HRESULT STDMETHODCALLTYPE RegisterStereoStatusWindow(
        HWND WindowHandle,
        UINT wMsg,
        DWORD* pdwCookie)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->RegisterStereoStatusWindow(WindowHandle, wMsg, pdwCookie);
    }

    /// Wrap the RegisterStereoStatusEvent member function
    virtual HRESULT STDMETHODCALLTYPE RegisterStereoStatusEvent(
        HANDLE hEvent,
        DWORD* pdwCookie)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->RegisterStereoStatusEvent(hEvent, pdwCookie);
    }

    /// Wrap the UnregisterStereoStatus member function
    virtual void STDMETHODCALLTYPE UnregisterStereoStatus(DWORD dwCookie)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->UnregisterStereoStatus(dwCookie);
    }

    /// Wrap the RegisterOcclusionStatusWindow member function
    virtual HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusWindow(
        HWND WindowHandle,
        UINT wMsg,
        DWORD* pdwCookie)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->RegisterOcclusionStatusWindow(WindowHandle, wMsg, pdwCookie);
    }

    /// Wrap the RegisterOcclusionStatusEvent member function
    virtual HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusEvent(
        HANDLE hEvent,
        DWORD* pdwCookie)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->RegisterOcclusionStatusEvent(hEvent, pdwCookie);
    }

    /// Wrap the UnregisterOcclusionStatus member function
    virtual void STDMETHODCALLTYPE UnregisterOcclusionStatus(DWORD dwCookie)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        return m_pReal->UnregisterOcclusionStatus(dwCookie);
    }

    /// Wrap the CreateSwapChainForComposition member function
    virtual HRESULT STDMETHODCALLTYPE CreateSwapChainForComposition(
        IUnknown* pDevice,
        const DXGI_SWAP_CHAIN_DESC1* pDesc,
        IDXGIOutput* pRestrictToOutput,
        IDXGISwapChain1** ppSwapChain)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        IUnknown* unwrappedDevice = NULL;
        HRESULT wasWrapped = pDevice->QueryInterface(IID_IWrappedObject, (void**)&unwrappedDevice);

        if (wasWrapped == S_OK)
        {
            pDevice = unwrappedDevice;
        }

        HRESULT hRes =  m_pReal->CreateSwapChainForComposition(pDevice, pDesc, pRestrictToOutput, ppSwapChain);

        if (hRes == S_OK)
        {
            LayerManager* pLM = DXGILayerManager::Instance();
            pLM->OnSwapchainCreated(*ppSwapChain, pDevice);

            if (pLM != NULL)
            {
                //Log(logDEBUG, "IDXGIFactoryWrapper: CreateSwapChainForCoreWindow: DXGILayerManager::Instance()->OnCreate()\n");
                pLM->OnCreate(DX11_SWAPCHAIN, *ppSwapChain);
            }
        }

        return hRes;
    }

    /// Wrap the GetCreationFlags member function
    virtual UINT STDMETHODCALLTYPE GetCreationFlags()
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        UINT flags = m_pReal->GetCreationFlags();

        return flags;
    }

    /// Wrap the EnumAdapterByLuid member function
    virtual HRESULT STDMETHODCALLTYPE EnumAdapterByLuid(
        LUID AdapterLuid,
        REFIID riid,
        void** ppvAdapter)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        HRESULT hRes = m_pReal->EnumAdapterByLuid(AdapterLuid, riid, ppvAdapter);

        return hRes;
    }

    /// Wrap the EnumWarpAdapter member function
    virtual HRESULT STDMETHODCALLTYPE EnumWarpAdapter(REFIID riid,  void** ppvAdapter)
    {
#ifdef DEBUG_LOG
        Log(logDEBUG, "IDXGIFactoryWrapper: %s:\n", __FUNCTION__);
#endif

        HRESULT hRes = m_pReal->EnumWarpAdapter(riid, ppvAdapter);

        return hRes;
    }
};

#endif // USE_DXGI_WRAPPER

#endif // DXGI__FACTORY_WRAPPER_H