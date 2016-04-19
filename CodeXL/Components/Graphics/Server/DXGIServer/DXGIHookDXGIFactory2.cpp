//=====================================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief DXGIFactory2 interception
//=====================================================================================

#include "DXGIHookDXGIFactory2.h"
#ifndef DLL_REPLACEMENT
    #include <Interceptor.h>
#endif
#include "../DXCommonSource/HookHelpers.h"
#include "DXGILayerManager.h"
#include "../DXCommonSource/HookVtableImmediate.h"
#include "../Common/StreamLog.h"
#include "IDXGIFactoryWrapper.h"

#define USE_VTABLE_PATCHING

/// Function pointer typedef for intercepted API functions
typedef HRESULT(WINAPI* CreateDXGIFactory2_type)(UINT flags, REFIID riid, void** ppFactory2);

/// Pointer to the real create factory function
static CreateDXGIFactory2_type Real_CreateDXGIFactory2 = NULL;

/// Debug flag used to jump over the first factory creation. OFF by default. 
bool g_ignoreFirstFactory2 = false;

/// Helper function to convert guid to string
/// \param pG  Input GUID
/// \result String version of GUID
std::string TranslateGUID_2(const GUID* pG)
{
    return FormatString("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", pG->Data1, pG->Data2, pG->Data3, pG->Data4[0], pG->Data4[1], pG->Data4[2], pG->Data4[3], pG->Data4[4], pG->Data4[5], pG->Data4[6], pG->Data4[7]);
}

/// Intercepted CreateDXGIFactory2 function
/// \param flags Input flags
/// \param riid Interface ID
/// \param ppFactory2 Output factory pointer
/// \result HRESULT return code
static HRESULT WINAPI Mine_CreateDXGIFactory2(UINT flags, REFIID riid, void** ppFactory2)
{
    //Log(logDEBUG, "DXGIServer: Mine_CreateDXGIFactory2\n");

    LogTrace(traceENTER, TranslateGUID_2(&riid).c_str());

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "Mine_CreateDXGIFactory2:" << "--------------------------------------------" << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

    PsAssert(ppFactory2 != NULL);

    if (ppFactory2 == NULL)
    {
        Log(logERROR, "ppFactory2 is NULL\n");
        LogTrace(traceEXIT, "");
        return S_FALSE;
    }

    if (ppFactory2)
    {
        LogTrace(traceMESSAGE, "*ppFactory2 = 0x%p", *ppFactory2);
    }

    LogTrace(traceMESSAGE, "g_dwInsideDXGIFactory2 = %u", g_dwInsideDXGI.GetRef());

    // Create the original Factory for the app
    HRESULT hRes = Real_CreateDXGIFactory2(flags, riid, ppFactory2);

#ifdef USE_DXGI_WRAPPER
    IDXGIFactoryWrapper* pWrapper = new IDXGIFactoryWrapper(*ppFactory2);
    *ppFactory2 = pWrapper;
#endif

#ifdef USE_DXGI_STREAMLOG
    {
        std::stringstream infoMsg;
        infoMsg << "Mine_CreateDXGIFactory2:" << "Created Factory2: " << *ppFactory2 << "\n";
        DXGILayerManager::Instance()->mStreamLog.LogMsg(infoMsg);
    }
#endif

    // Ignore the first factory to be created
    if (g_ignoreFirstFactory2 == true)
    {
        Log(logERROR, "Ignoring first DXGIFactory2.\n");
        g_ignoreFirstFactory2 = false;
        return hRes;
    }

    LayerManager* pLM = DXGILayerManager::Instance();

    if (pLM != NULL)
    {
        if (*ppFactory2)
        {
            DXGILayerManager::Instance()->OnCreate(DXGI_FACTORY, *ppFactory2);
        }
    }

    LogTrace(traceEXIT, "returned 0x%08x, *ppFactory2 = 0x%p", hRes, *ppFactory2);

    return hRes;
}

#ifndef DLL_REPLACEMENT
/// Helper function to hook the CreateFactory2 interface
/// \param hDXGI Input DXGI module
void HookCreateDXGIFactory2Function(HMODULE hDXGI)
{
    LogTrace(traceENTER, "");

    // don't attach twice
    if (Real_CreateDXGIFactory2 != NULL)
    {
        LogTrace(traceEXIT, "already hooked");
        return;
    }

    Real_CreateDXGIFactory2 = (CreateDXGIFactory2_type)GetProcAddress(hDXGI, "CreateDXGIFactory2");

    if (Real_CreateDXGIFactory2 != NULL)
    {
        LONG error;
        AMDT::BeginHook();
        HOOK(Real_CreateDXGIFactory2, Mine_CreateDXGIFactory2);

        if (AMDT::EndHook() != NO_ERROR)
        {
            Log(logERROR, "HookCreateDXGIFactory2 Failed\n");
        }
    }
    else
    {
        Log(logERROR, "HookCreateDXGIFactory2 Failed: Cannot find the CreateDXGIFactory2 in the current DXGI module. This is not a real error if you are using Windows7.\n");
    }

    LogTrace(traceEXIT, "");

    return;
}

/// Helper function to unhook the CreateFactory2 interface
/// \param hDXGI Input DXGI module
void UnhookCreateDXGIFactory2Function(HMODULE hDXGI)
{
    LogTrace(traceENTER, "");

    // don't detach twice
    if (Real_CreateDXGIFactory2 == NULL)
    {
        Log(logERROR, "Attempting to call UnhookCreateDXGIFactory2() twice\n");
        LogTrace(traceEXIT, "");
        return;
    }

    if (hDXGI != NULL)
    {
        LONG error;
        AMDT::BeginHook();

        LogTrace(traceEXIT, "UnhookCreateDXGIFactory2");

        UNHOOK(Real_CreateDXGIFactory2, Mine_CreateDXGIFactory2);

        if (AMDT::EndHook() != NO_ERROR)
        {
            Log(logERROR, "UnhookCreateDXGIFactory2 Failed\n");
        }
    }

    Real_CreateDXGIFactory2 = NULL;

    LogTrace(traceEXIT, "");

    return;
}
#endif // DLL_REPLACEMENT

#ifdef DLL_REPLACEMENT
//-----------------------------------------------------------------------------
/// ReplaceCreateDXGIFactory2Function
/// Setup the real CreateDXGIFactory2 function
///
/// \param hDXGI The handle to the real DXGI dll
//-----------------------------------------------------------------------------
void ReplaceCreateDXGIFactory2Function(HMODULE hDXGI)
{
    Real_CreateDXGIFactory2 = (CreateDXGIFactory2_type)GetProcAddress(hDXGI, "CreateDXGIFactory2");
}

/// Intercepted CreateDXGIFactory2 function
/// \param flags Input flags
/// \param riid Interface ID
/// \param ppFactory2 Output factory pointer
/// \result HRESULT return code
HRESULT WINAPI CreateDXGIFactory2(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory)
{
    CheckUpdateHooks();
    return Mine_CreateDXGIFactory2(Flags, riid, ppFactory);
}
#endif