//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Module.cpp
///
//==================================================================================
#include "stdafx.h"

#include "CommonIncludes.h"
#include <src/vspCoreAPI.h>

class PackageModule :
    public CAtlDllModuleT<PackageModule>
{
public:
    BOOL APIENTRY DllMain(DWORD reason, LPVOID reserved, HINSTANCE hInstance)
    {
        //        MessageBox(NULL, L"HOLD", L"HOLD 001", MB_OK);

        // Load the core:
        bool loadSuccess = true;

        if (DLL_PROCESS_ATTACH == reason)
        {
            loadSuccess = vspInitializeCoreAPI((void*)hInstance);
        }

        BOOL res = (loadSuccess ? VSCORE(vspDllMain)(reason, reserved, hInstance) : FALSE);

        return res;
    }
};

PackageModule _AtlModule;

// This macro is used as default registry root when a NULL parameter is passed to VSDllRegisterServer
// or VSDllUnregisterServer. For sample code we set as default the experimental instance, but for production
// code you should change it to the standard VisualStudio instance that is LREGKEY_VISUALSTUDIOROOT.
#define DEFAULT_REGISTRY_ROOT LREGKEY_VISUALSTUDIOROOT

// Since this project defines an oleautomation interface, the typelib needs to be registered.
#define VSL_REGISTER_TYPE_LIB TRUE

// Must come after declaration of _AtlModule and DEFAULT_REGISTRY_ROOT
// #include <VSLPackageDllEntryPoints.cpp>
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Uri, 27/12/10 - the default VSLPackage DllMain does not pass the module
// handle to us, which we need. So, we cannot have this include here.
// Instead, We copied its contents and slightly modified the DllMain
// function to give us the handle.
// Code below this point is from that file.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifndef VSL_REGISTER_TYPE_LIB
    #define VSL_REGISTER_TYPE_LIB FALSE
#endif VSL_REGISTER_TYPE_LIB

#pragma warning(push) // Sometimes true, sometimes not.
#pragma warning(disable : 4702) // warning C4702: unreachable code

// Initializes ATL
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    //    MessageBox(NULL, L"HOLD", L"HOLD 000", MB_OK);

    VSL_STDMETHODTRY
    {
        return _AtlModule.DllMain(dwReason, lpReserved, hInstance);

    } VSL_STDMETHODCATCH()

    return FALSE;
}

// Used by COM to determine whether the DLL can be unloaded
STDAPI DllCanUnloadNow()
{
    VSL_STDMETHODTRY
    {

        return _AtlModule.DllCanUnloadNow();

    } VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}

// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    VSL_STDMETHODTRY
    {

        return _AtlModule.GetClassObject(rclsid, riid, ppv);

    } VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}


STDMETHODIMP VSDllRegisterServerInternal(_In_opt_ wchar_t* strRegRoot, bool shouldRegister, bool isRanu)
{
    VSL_STDMETHODTRY
    {

        VsRegistryUtilities::SetRegRoot(NULL == strRegRoot ? DEFAULT_REGISTRY_ROOT : strRegRoot, isRanu);

        //Set ATL to register the typelib as RANU if requested by the caller.
        AtlSetPerUserRegistration(isRanu);

        if (shouldRegister)
        {
            HRESULT hr = _AtlModule.RegisterServer(VSL_REGISTER_TYPE_LIB);

            if (SUCCEEDED(hr))
            {
                hr = (HRESULT)VSCORE(vscDllRegisterServer)();
            }

            return hr;
        }
        else
        {
            HRESULT hr = _AtlModule.UnregisterServer(VSL_REGISTER_TYPE_LIB);

            if (SUCCEEDED(hr) || (TYPE_E_REGISTRYACCESS == hr))
            {
                hr = (HRESULT)VSCORE(vscDllUnregisterServer)();
            }

            // If the type library was already unregistered, ignore the failure
            return (TYPE_E_REGISTRYACCESS == hr) ? S_OK : hr;
        }
    } VSL_STDMETHODCATCH()

    return VSL_GET_STDMETHOD_HRESULT();
}

// Registers COM objects normally and registers VS Packages to the specified VS registry hive under HKCU
STDAPI VSDllRegisterServerUser(_In_opt_ wchar_t* strRegRoot)
{
    return VSDllRegisterServerInternal(strRegRoot, true, true);
}

// Unregisters COM objects normally and unregisters VS Packages from the specified VS registry hive under HKCU
STDAPI VSDllUnregisterServerUser(__in_opt wchar_t* strRegRoot)
{
    return VSDllRegisterServerInternal(strRegRoot, false, true);
}

// Registers COM objects normally and registers VS Packages to the specified VS registry hive
STDAPI VSDllRegisterServer(__in_opt wchar_t* strRegRoot)
{
    return VSDllRegisterServerInternal(strRegRoot, true, false);
}

// Unregisters COM objects normally and unregisters VS Packages from the specified VS registry hive
STDAPI VSDllUnregisterServer(__in_opt wchar_t* strRegRoot)
{
    return VSDllRegisterServerInternal(strRegRoot, false, false);
}

// Registers COM objects normally and registers VS Packages to the default VS registry hive
STDAPI DllRegisterServer()
{
    return VSDllRegisterServer(NULL);
}

// Unregisters COM objects normally and unregisters VS Packages from the default VS registry hive
STDAPI DllUnregisterServer()
{
    return VSDllUnregisterServer(NULL);
}

#pragma warning(pop)
