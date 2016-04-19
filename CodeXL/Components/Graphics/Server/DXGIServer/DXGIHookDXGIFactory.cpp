//=====================================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief DXGI Factory function hooking functions
//=====================================================================================


#include "DXGIHookDXGIFactory.h"
#ifndef DLL_REPLACEMENT
    #include <Interceptor.h>
#endif
#include "../DXCommonSource/HookHelpers.h"
#include "DXGILayerManager.h"
#include "../DXCommonSource/HookVtableImmediate.h"
//#include "DXGIFactoryRecorder.h"
#include "../Common/StreamLog.h"
#include "../Common/IUnknownWrapperGUID.h"
#include <dxgi1_2.h>
#include "IDXGIFactoryWrapper.h"

#define USE_VTABLE_PATCHING

/// Function pointer typedef for intercepted API functions
typedef HRESULT(WINAPI* CreateDXGIFactory_type)(REFIID riid, void** ppFactory);

/// Pointer to the real create factory function
static CreateDXGIFactory_type Real_CreateDXGIFactory = NULL;

/// Debug flag used to jump over the first factory creation. OFF by default.
bool g_ignoreFirstFactory = false;

/// Helper function to convert guid to string
/// \param pG  Input GUID
/// \result String version of GUID
std::string TranslateGUID(const GUID* pG)
{
    return FormatString("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", pG->Data1, pG->Data2, pG->Data3, pG->Data4[0], pG->Data4[1], pG->Data4[2], pG->Data4[3], pG->Data4[4], pG->Data4[5], pG->Data4[6], pG->Data4[7]);
}

/// Intercepted CreateDXGIFactory function
/// \param riid Interface ID
/// \param ppFactory Output factory pointer
/// \result HRESULT return code
static HRESULT WINAPI Mine_CreateDXGIFactory(REFIID riid, void** ppFactory)
{
    //Log(logDEBUG, "DXGIServer: Mine_CreateDXGIFactory\n" );

    LogTrace(traceENTER, TranslateGUID(&riid).c_str());

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "\nMine_CreateDXGIFactory:" << GetThreadString() << "--------------------------------------------" << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

    PsAssert(ppFactory != NULL);

    if (ppFactory == NULL)
    {
        Log(logERROR, "ppFactory is NULL\n");
        LogTrace(traceEXIT, "");
        return S_FALSE;
    }

    if (ppFactory)
    {
        LogTrace(traceMESSAGE, "*ppFactory = 0x%p", *ppFactory);
    }

    LogTrace(traceMESSAGE, "g_dwInsideDXGIFactory = %u", g_dwInsideDXGI.GetRef());

    HRESULT hRes;

    RefTracker rf(&g_dwInsideDXGI);

    // Create the original Factory for the app
    hRes = Real_CreateDXGIFactory(riid, ppFactory);

    //Log(logDEBUG, "DXGIServer: Real_CreateDXGIFactory has been called\n");

#ifdef USE_DXGI_WRAPPER
    IDXGIFactoryWrapper* pWrapper = new IDXGIFactoryWrapper(*ppFactory);
    *ppFactory = pWrapper;

    //Log(logDEBUG, "DXGIServer: *ppFactory has been wrapped\n");
#endif

    //Log(logDEBUG, "DXGIServer: Mine_CreateDXGIFactory Created factory 0x%p\n", *ppFactory );

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "Mine_CreateDXGIFactory:" << "Created Factory: " << *ppFactory << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

    // Ignore the first factory to be created
    if (g_ignoreFirstFactory == true)
    {
        Log(logERROR, "Ignoring first DXGIFactory.\n");
        g_ignoreFirstFactory = false;
        return hRes;
    }

    /// Tell each layer that a factory has been created
    if (riid == __uuidof(IDXGIFactory))
    {
        if (*ppFactory)
        {
            DXGILayerManager::Instance()->OnCreate(DXGI_FACTORY, *ppFactory);
            //Log(logDEBUG, "DXGIServer: Calling DXGILayerManager::Instance()->OnCreate()\n");
        }
    }

    LogTrace(traceEXIT, "returned 0x%08x, *ppFactory = 0x%p", hRes, *ppFactory);

    return hRes;
}

#ifndef DLL_REPLACEMENT
/// Helper function to hook the CreateFactory1 interface
/// \param hDXGI Input DXGI module
void HookCreateDXGIFactoryFunction(HMODULE hDXGI)
{
    //Log(logERROR, "DXGIServer: HookCreateDXGIFactoryFunction\n" );

    LogTrace(traceENTER, "");

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "\nHookCreateDXGIFactoryFunction:" << "Hooking HMODULE: " << hDXGI << GetThreadString() << "--------------------------------------------" << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

    // don't attach twice
    if (Real_CreateDXGIFactory != NULL)
    {
        LogTrace(traceEXIT, "already hooked");
#ifdef USE_DXGI_STREAMLOG
        {
            std::stringstream infoMsg;
            infoMsg << "HookCreateDXGIFactoryFunction: HMODULE already hooked - returning." << "\n";
            DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
        }
#endif
        return;
    }

    Real_CreateDXGIFactory = (CreateDXGIFactory_type)GetProcAddress(hDXGI, "CreateDXGIFactory");

    LONG error;
    AMDT::BeginHook();
    HOOK(Real_CreateDXGIFactory, Mine_CreateDXGIFactory);

    if (AMDT::EndHook() != NO_ERROR)
    {
        Log(logERROR, "HookCreateDXGIFactoryFunction Failed\n");
    }

    LogTrace(traceEXIT, "");

    return;
}

/// Helper function to unhook the CreateFactory2 interface
/// \param hDXGI Input DXGI module
void UnhookCreateDXGIFactoryFunction(HMODULE hDXGI)
{
    //Log(logERROR, "DXGIServer: UnhookCreateDXGIFactoryFunction\n" );

    LogTrace(traceENTER, "");

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "\nUnhookCreateDXGIFactory: HMODULE: " << hDXGI << GetThreadString() << " --------------------------------------------" << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

    // don't detach twice
    if (Real_CreateDXGIFactory == NULL)
    {
        Log(logERROR, "Attempting to call UnhookCreateDXGIFactory() twice\n");

#ifdef USE_DXGI_STREAMLOG
        {
            std::stringstream infoMsg;
            infoMsg << "UnhookCreateDXGIFactory: " << "Attempting to call UnhookCreateDXGIFactory() twice" << "\n";
            DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
        }
#endif
        LogTrace(traceEXIT, "");
        return;
    }

    if (hDXGI != NULL)
    {
        LONG error;
        AMDT::BeginHook();

        UNHOOK(Real_CreateDXGIFactory, Mine_CreateDXGIFactory);

        if (AMDT::EndHook() != NO_ERROR)
        {
            Log(logERROR, "UnhookCreateDXGIFactory Failed\n");
#ifdef USE_DXGI_STREAMLOG
            {
                std::stringstream infoMsg;
                infoMsg << "UnhookCreateDXGIFactory: HMODULE: " << hDXGI << "UnhookCreateDXGIFactory Failed" << "\n";
                DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
            }
#endif
        }
    }

    Real_CreateDXGIFactory = NULL;
    LogTrace(traceEXIT, "");

    return;
}
#endif // DLL_REPLACEMENT

/// Function pointer typedef for intercepted function
typedef HRESULT(STDMETHODCALLTYPE* IUnknown_QueryInterface_type)(IDXGIFactory* pFactory, REFIID riid, __RPC__deref_out void __RPC_FAR* __RPC_FAR* ppvObject);
/// Function pointer typedef for intercepted function
typedef ULONG(WINAPI* IDXGIFactory_Release_type)(IDXGIFactory* pFactory);
/// Function pointer typedef for intercepted function
typedef HRESULT(WINAPI* IDXGIFactory_CreateSwapChain_type)(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);


#ifdef USE_VTABLE_PATCHING
    static ULONG WINAPI Mine_IDXGIFactory_Release(IDXGIFactory* pFactory); ///<  Function pointer for intercepted function
    static HookVtableImmediate HookDXGIFactory("DXGIFactory", 2, (ptrdiff_t*)Mine_IDXGIFactory_Release); ///<  Function pointer for intercepted function
#else
    static IDXGIFactory_Release_type Real_IDXGIFactory_Release = NULL; ///<  Function pointer for intercepted function
    static IDXGIFactory_CreateSwapChain_type Real_IDXGIFactory_CreateSwapChain = NULL; ///<  Function pointer for intercepted function
    static IUnknown_QueryInterface_type Real_IDXGIFactory_QueryInterface = NULL; ///<  Function pointer for intercepted function
#endif


#ifndef USE_DXGI_WRAPPER
//--------------------------------------------------------------------------
// Wrapper IDXGIFactory::QueryInterface
//--------------------------------------------------------------------------
static HRESULT STDMETHODCALLTYPE Mine_IDXGIFactory_QueryInterface(IDXGIFactory* pFactory, REFIID riid, __RPC__deref_out void __RPC_FAR* __RPC_FAR* ppvObject)
{
    //Log(logDEBUG, "DXGIServer: Mine_IDXGIFactory_QueryInterface %ld\n", pFactory );

    LogTrace(traceENTER, "*pFactory = 0x%p\n", pFactory);
    LogTrace(traceMESSAGE, "g_dwInsideDXGI = %u\n", g_dwInsideDXGI.GetRef());

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "\nMine_IDXGIFactory_QueryInterface:" << "IDXGIFactory: " << pFactory << GetThreadString() << " --------------------------------------------" << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

#if 0
    Log(logTRACE, "Mine_IDXGIFactory_QueryInterface( 0x%08x, %s )\n", pFactory, TranslateGUID(&riid).c_str());
#else
    Log(logTRACE, "Mine_IDXGIFactory_QueryInterface( 0x%08x )\n", pFactory);
#endif

#ifdef USE_VTABLE_PATCHING

    //if (riid == __uuidof(IDXGIFactory2))
    //{
    //    //return m_pReal->QueryInterface(riid, ppvObject);
    //    *ppvObject = NULL;
    //    return E_NOINTERFACE;
    //}

    void* pFn = HookDXGIFactory.GetRealFunction(pFactory, 0);
    PsAssert(pFn != NULL);
    ULONG hRes = ((IUnknown_QueryInterface_type) pFn)(pFactory, riid, ppvObject);
#else
    HRESULT hRes = Real_IDXGIFactory_QueryInterface(pFactory, riid, ppvObject);
#endif

    LogTrace(traceEXIT, "returned 0x%08x", hRes);

    return hRes;
}

#endif

#ifdef USE_VTABLE_PATCHING

/// Handle factory destruction
/// \param pFactory Input factory
static void Mine_IDXGIFactory_OnDestroy(IUnknown* pFactory)
{
    //Log(logDEBUG, "DXGIServer: Mine_IDXGIFactory_OnDestroy 0x%p\n", pFactory );

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "\nMine_IDXGIFactory_OnDestroy:" << "IDXGIFactory: " << pFactory << GetThreadString() << " --------------------------------------------" << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

    DXGILayerManager::Instance()->OnDestroy(DXGI_FACTORY, pFactory);
}
#endif


/// Handle factory release
/// \param pFactory Input factory
static ULONG WINAPI Mine_IDXGIFactory_Release(IDXGIFactory* pFactory)
{
    //Log(logDEBUG, "DXGIServer: Mine_IDXGIFactory_Release 0x%p\n", pFactory );

    LogTrace(traceENTER, "*pFactory = 0x%p\n", pFactory);
    LogTrace(traceMESSAGE, "g_dwInsideDXGI = %u\n", g_dwInsideDXGI.GetRef());

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "\nMine_IDXGIFactory_Release:" << "IDXGIFactory: " << pFactory << GetThreadString() << " --------------------------------------------" << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

    RefTracker rf(&g_dwInsideDXGI);

    ULONG rVal;

#ifdef USE_VTABLE_PATCHING
    rVal = HookDXGIFactory.RemoveAndDetach(pFactory, "DXGIServer");
#else

    if ((g_dwInsideWrapper == 1) && (pFactory == GetLayerManager()->GetFactoryToMonitor()))
    {
        pFactory->AddRef();
        ULONG RefCount = pFactory->Release();

        if (RefCount == 1)
        {
            GetLayerManager()->OnDestroy(DXGI_FACTORY, pFactory);
        }
    }

    if (Real_IDXGIFactory_Release != NULL)
    {
        rVal = Real_IDXGIFactory_Release(pFactory);
    }
    else
    {
        rVal = pFactory->Release();
    }

#endif

    LogTrace(traceEXIT, "returned 0x%08x", rVal);

    return rVal;
}

/// Create swap chain interception
/// \param pFactory Input factory
/// \param pDevice Input device
/// \param pDesc Input Description
/// \param ppSwapChain Output swapchain pointer
/// \result HRESULT return code
static HRESULT WINAPI Mine_IDXGIFactory_CreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
    //Log(logDEBUG, "DXGIServer: Mine_IDXGIFactory_CreateSwapChain on Factory: 0x%p\n", pFactory );

    LogTrace(traceENTER, "pFactory = 0x%p, pDevice = 0x%p, pDesc = 0x%p, ppSwapChain = 0x%p)", pFactory, pDevice, pDesc, ppSwapChain);
    LogTrace(traceMESSAGE, "g_dwInsideDXGI = %d", g_dwInsideDXGI.GetRef());

    //Log(logDEBUG, "Mine_IDXGIFactory_CreateSwapChain:\n");

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "\nMine_IDXGIFactory_CreateSwapChain:" << "IDXGIFactory: " << pFactory << GetThreadString() << " --------------------------------------------" << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

    RefTracker rf(&g_dwInsideDXGI);

    IUnknown* tmpDevice;

    if (pDevice->QueryInterface(IID_IWrappedObject, (void**)&tmpDevice) == S_OK)
    {
        // Wrapped Device - use real instead
        pDevice = tmpDevice;
    }
    else
    {
        Log(logDEBUG, "Mine_IDXGIFactory_CreateSwapChain: Device not wrapped 0x%p\n", pDevice);
    }

#ifdef USE_VTABLE_PATCHING
    void* pFn = HookDXGIFactory.GetRealFunction(pFactory, 10);
    ULONG hres = ((IDXGIFactory_CreateSwapChain_type) pFn)(pFactory, pDevice, pDesc, ppSwapChain);

    //Log(logDEBUG, "DXGIServer: Mine_IDXGIFactory_CreateSwapChain: Real Function called 0x%p\n", pFn);

#else
    HRESULT hres = Real_IDXGIFactory_CreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
#endif

    if (g_dwInsideDXGI == 1)
    {
        DXGILayerManager* pLM = DXGILayerManager::Instance();

        if (pLM != NULL)
        {
            //Log(logDEBUG, "DXGIServer: Mine_IDXGIFactory_CreateSwapChain: DXGILayerManager::Instance()->OnCreate()\n");
            pLM->OnCreate(DX11_SWAPCHAIN, *ppSwapChain);
        }
    }

    //Log(logDEBUG, "DXGIServer: Mine_IDXGIFactory_CreateSwapChain created swapchain: 0x%p\n", *ppSwapChain );

    LogTrace(traceEXIT, "returned 0x%08x, *ppSwapChain = 0x%p", hres, *ppSwapChain);

    return hres;
}

/// Counts how many times the factory is hooked
static DWORD s_dwDXGIFactoryHookedTimes = 0;

/// Helper function to hook the CreateFactory interface
/// \param pFac Input DXGI factory
DWORD HookIDXGIFactory(IDXGIFactory* pFac)
{
    //Log(logDEBUG, "DXGIServer: HookIDXGIFactory 0x%p\n", pFac );

    LogTrace(traceENTER, "pFac = 0x%p", pFac);

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "\nHookIDXGIFactory:" << "IDXGIFactory: " << pFac << GetThreadString() << " --------------------------------------------" << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

#ifdef USE_DXGI_WRAPPER
    // Debug: to get the real pointer here
    IUnknown* pReal;

    // first check if we are trying to patch a wrapper object
    if (pFac->QueryInterface(IID_IWrappedObject, (void**)&pReal) == S_OK)
    {
        Log(logDEBUG, "HookIDXGIFactory: Input is a Factory Wrapper - going to use the real pointer.\n");
        pFac = (IDXGIFactory*)pReal;
    }

#endif

    PsAssert(pFac);

#ifdef USE_VTABLE_PATCHING

    //Log(logDEBUG, "HookIDXGIFactory: VTable patching QueryInterface() and CreateSwapChain()\n");

    if (HookDXGIFactory.AddAndHookIfUnique(pFac, true))
    {
#ifndef USE_DXGI_WRAPPER
        HookDXGIFactory.AddVtableFunctionToPatch(pFac, 0, (ptrdiff_t*)Mine_IDXGIFactory_QueryInterface);
#endif
        HookDXGIFactory.AddVtableFunctionToPatch(pFac, 10, (ptrdiff_t*)Mine_IDXGIFactory_CreateSwapChain);
    }

    HookDXGIFactory.SetOnReleaseCallBack(Mine_IDXGIFactory_OnDestroy);
    HookDXGIFactory.Attach();
#else

    if (s_dwDXGIFactoryHookedTimes == 0)
    {
        Real_IDXGIFactory_QueryInterface = (IUnknown_QueryInterface_type)GetD3D11StaticOffset(pFac, 0);
        Real_IDXGIFactory_Release = (IDXGIFactory_Release_type)GetD3D11StaticOffset(pFac, 2);
        Real_IDXGIFactory_CreateSwapChain = (IDXGIFactory_CreateSwapChain_type)GetD3D11StaticOffset(pFac, 10);       //create swapchain
        LONG error;
        AMDT::BeginHook();

        HOOK(Real_IDXGIFactory_CreateSwapChain, Mine_IDXGIFactory_CreateSwapChain);
        HOOK(Real_IDXGIFactory_QueryInterface, Mine_IDXGIFactory_QueryInterface);
        HOOK(Real_IDXGIFactory_Release, Mine_IDXGIFactory_Release);

        if (AMDT::EndHook() != NO_ERROR)
        {
            Log(logERROR, "HookIDXGIFactory failed\n");
        }
    }

#endif

    s_dwDXGIFactoryHookedTimes++;

    LogTrace(traceEXIT, "");

    return s_dwDXGIFactoryHookedTimes;
}

/// Helper function to unhook the CreateFactory interface
DWORD UnhookIDXGIFactory()
{
    //Log(logDEBUG, "DXGIServer: UnhookIDXGIFactory\n" );

    LogTrace(traceENTER, "");

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "\nUnhookIDXGIFactory:" << " IDXGIFactory: " << " Unknown" << GetThreadString() << " --------------------------------------------" << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

    PsAssert(s_dwDXGIFactoryHookedTimes > 0);

    s_dwDXGIFactoryHookedTimes--;

    Log(logMESSAGE, "%s() - s_dwDXGIFactoryHookedTimes %ld\n", __FUNCTION__, s_dwDXGIFactoryHookedTimes);

    if (s_dwDXGIFactoryHookedTimes == 0)
    {
#ifdef USE_VTABLE_PATCHING
        Log(logMESSAGE, "%s() - s_dwDXGIFactoryHookedTimes is zero - about to call  HookDXGIFactory.Detach()\n", __FUNCTION__);
        HookDXGIFactory.Detach();
#else
        LONG error;
        AMDT::BeginHook();

        UNHOOK(Real_IDXGIFactory_CreateSwapChain, Mine_IDXGIFactory_CreateSwapChain);
        UNHOOK(Real_IDXGIFactory_Release, Mine_IDXGIFactory_Release);
        UNHOOK(Real_IDXGIFactory_QueryInterface, Mine_IDXGIFactory_QueryInterface);

        if (AMDT::EndHook() != NO_ERROR)
        {
            Log(logERROR, "UnhookIDXGIFactory failed\n");
        }

        Real_IDXGIFactory_Release = NULL;
        Real_IDXGIFactory_CreateSwapChain = NULL;
#endif
    }

    LogTrace(traceEXIT, "");

    return s_dwDXGIFactoryHookedTimes;
}

#ifdef DLL_REPLACEMENT

//-----------------------------------------------------------------------------
/// ReplaceCreateDXGIFactoryFunction
/// Setup the real CreateDXGIFactory function
///
/// \param hDXGI The handle to the real DXGI dll
//-----------------------------------------------------------------------------
void ReplaceCreateDXGIFactoryFunction(HMODULE hDXGI)
{
    Real_CreateDXGIFactory = (CreateDXGIFactory_type)GetProcAddress(hDXGI, "CreateDXGIFactory");
}

/// Intercepted CreateDXGIFactory function
/// \param riid Interface ID
/// \param ppFactory Output factory pointer
/// \result HRESULT return code
HRESULT WINAPI CreateDXGIFactory(REFIID riid, _COM_Outptr_ void** ppFactory)
{
    CheckUpdateHooks();
    return Mine_CreateDXGIFactory(riid, ppFactory);
}

#endif