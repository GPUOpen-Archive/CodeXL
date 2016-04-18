//=====================================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief DXGIFactory1 interception
//=====================================================================================

#include "DXGIHookDXGIFactory1.h"
#ifndef DLL_REPLACEMENT
    #include <Interceptor.h>
#endif
#include "../DXCommonSource/HookHelpers.h"
#include "DXGILayerManager.h"
#include "../DXCommonSource/HookVtableImmediate.h"
#include "../Common/StreamLog.h"
#include "IDXGIFactoryWrapper.h"

#define USE_VTABLE_PATCHING

/// Interception function pointer type def
typedef HRESULT(WINAPI* CreateDXGIFactory1_type)(REFIID riid, void** ppFactory1);

/// Pointer to the real create factory function
static CreateDXGIFactory1_type Real_CreateDXGIFactory1 = NULL;

/// Debug flag used to jump over the first factory creation. OFF by default.
bool g_ignoreFirstFactory1 = false;

/// Helper function to convert guid to string
/// \param pG  Input GUID
/// \result String version of GUID
std::string TranslateGUID_1(const GUID* pG)
{
    return FormatString("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", pG->Data1, pG->Data2, pG->Data3, pG->Data4[0], pG->Data4[1], pG->Data4[2], pG->Data4[3], pG->Data4[4], pG->Data4[5], pG->Data4[6], pG->Data4[7]);
}

/// Intercepted CreateDXGIFactory1 function
/// \param riid Interface ID
/// \param ppFactory1 Output factory pointer
/// \result HRESULT return code
static HRESULT WINAPI Mine_CreateDXGIFactory1(REFIID riid, void** ppFactory1)
{
    //Log(logDEBUG, "DXGIServer: Mine_CreateDXGIFactory1\n");

    LogTrace(traceENTER, TranslateGUID_1(&riid).c_str());

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "Mine_CreateDXGIFactory1:" << "--------------------------------------------" << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

    PsAssert(ppFactory1 != NULL);

    if (ppFactory1 == NULL)
    {
        Log(logERROR, "ppFactory1 is NULL\n");
        LogTrace(traceEXIT, "");
        return S_FALSE;
    }

    if (ppFactory1)
    {
        LogTrace(traceMESSAGE, "*ppFactory1 = 0x%p", *ppFactory1);
    }

    LogTrace(traceMESSAGE, "g_dwInsideDXGIFactory1 = %u", g_dwInsideDXGI.GetRef());

    // Create the original Factory for the app
    HRESULT hRes = Real_CreateDXGIFactory1(riid, ppFactory1);

    //Log(logDEBUG, "DXGIServer: Real_CreateDXGIFactory1 has been called 0x%p\n", *ppFactory1);

#ifdef USE_DXGI_WRAPPER
    IDXGIFactoryWrapper* pWrapper = new IDXGIFactoryWrapper(*ppFactory1);
    *ppFactory1 = pWrapper;

    //Log(logDEBUG, "DXGIServer: *ppFactory1 has been wrapped 0x%p\n", *ppFactory1);
#endif

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "Mine_CreateDXGIFactory1:" << "Created Factory1: " << *ppFactory1 << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

    // Ignore the first factory to be created
    if (g_ignoreFirstFactory1 == true)
    {
        Log(logERROR, "Ignoring first DXGIFactory1.\n");
        g_ignoreFirstFactory1 = false;
        return hRes;
    }

    LayerManager* pLM = DXGILayerManager::Instance();

    if (pLM != NULL)
    {
        if (*ppFactory1)
        {
            DXGILayerManager::Instance()->OnCreate(DXGI_FACTORY, *ppFactory1);
            //Log(logDEBUG, "DXGIServer: Calling DXGILayerManager::Instance()->OnCreate()\n");
        }
    }

    LogTrace(traceEXIT, "returned 0x%08x, *ppFactory1 = 0x%p", hRes, *ppFactory1);

    return hRes;
}

#ifndef DLL_REPLACEMENT
/// Helper function to hook the CreateFactory1 interface
/// \param hDXGI Input DXGI module
void HookCreateDXGIFactory1Function(HMODULE hDXGI)
{
    //Log(logERROR, "DXGIServer: HookCreateDXGIFactory1Function\n" );

    LogTrace(traceENTER, "");

    // don't attach twice
    if (Real_CreateDXGIFactory1 != NULL)
    {
        LogTrace(traceEXIT, "already hooked");
        return;
    }

    Real_CreateDXGIFactory1 = (CreateDXGIFactory1_type)GetProcAddress(hDXGI, "CreateDXGIFactory1");

    LONG error;
    AMDT::BeginHook();
    HOOK(Real_CreateDXGIFactory1, Mine_CreateDXGIFactory1);

    if (AMDT::EndHook() != NO_ERROR)
    {
        Log(logERROR, "HookCreateDXGIFactory1Function Failed\n");
    }

    LogTrace(traceEXIT, "");

    return;
}

/// Helper function to unhook the CreateFactory2 interface
/// \param hDXGI Input DXGI module
void UnhookCreateDXGIFactory1Function(HMODULE hDXGI)
{
    //Log(logERROR, "DXGIServer: UnhookCreateDXGIFactory1Function\n" );

    LogTrace(traceENTER, "");

    // don't detach twice
    if (Real_CreateDXGIFactory1 == NULL)
    {
        Log(logERROR, "Attempting to call UnhookCreateDXGIFactory1() twice\n");
        LogTrace(traceEXIT, "");
        return;
    }

    if (hDXGI != NULL)
    {
        LONG error;
        AMDT::BeginHook();

        LogTrace(traceEXIT, "UnhookCreateDXGIFactory1");

        UNHOOK(Real_CreateDXGIFactory1, Mine_CreateDXGIFactory1);

        if (AMDT::EndHook() != NO_ERROR)
        {
            Log(logERROR, "UnhookCreateDXGIFactory1 Failed\n");
        }
    }

    Real_CreateDXGIFactory1 = NULL;

    LogTrace(traceEXIT, "");

    return;
}
#endif // DLL_REPLACEMENT

#ifdef DLL_REPLACEMENT
//-----------------------------------------------------------------------------
/// ReplaceCreateDXGIFactory1Function
/// Setup the real CreateDXGIFactory1 function
///
/// \param hDXGI The handle to the real DXGI dll
//-----------------------------------------------------------------------------
void ReplaceCreateDXGIFactory1Function(HMODULE hDXGI)
{
    Real_CreateDXGIFactory1 = (CreateDXGIFactory1_type)GetProcAddress(hDXGI, "CreateDXGIFactory1");
}

/// Intercepted CreateDXGIFactory2 function
/// \param riid Interface ID
/// \param ppFactory2 Output factory pointer
/// \result HRESULT return code
HRESULT WINAPI CreateDXGIFactory1(REFIID riid, _COM_Outptr_ void** ppFactory)
{
    CheckUpdateHooks();
    return Mine_CreateDXGIFactory1(riid, ppFactory);
}
#endif
