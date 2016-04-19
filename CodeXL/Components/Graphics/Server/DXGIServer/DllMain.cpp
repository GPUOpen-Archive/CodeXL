//=====================================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief Initialization and hooking for the DXGI plugin dll
//=====================================================================================

#include "DllMain.h"
#include <windows.h>

#include "../Common/IServerPlugin.h"
#include "DXGIHookDXGIFactory.h"
#include "DXGIHookDXGIFactory1.h"
#include "DXGIHookDXGIFactory2.h"
#include "../Common/PerfStudioServer_Version.h"
#include "../Common/commandProcessor.h"
#include "../Common/logger.h"
#include "DXGILayerManager.h"
#include "ALVRInterception.h"
#include "../Common/LiquidVRSupport.h"
#include "../Common/Windows/DllReplacement.h"

#ifdef _MANAGED
    #pragma managed( push, off )
#endif

/// Record if hooked
static bool s_bHooked = false;

/// Record if inintialized
static bool s_bInitialized = false;

#ifdef DLL_REPLACEMENT
    static HINSTANCE s_hRealDXGI = 0;     // handle to real DXGI dll
#endif

/// DLL entry point
/// \param hinst Instance
/// \param dwReason Call reason
/// \param reserved Reserved
/// \return True for success, false for fail
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    (void)hinst;
    (void)reserved;

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        s_bHooked = s_bInitialized = false;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
#ifdef DLL_REPLACEMENT
        FreeLibrary(s_hRealDXGI);
#endif // DLL_REPLACEMENT
        /*
        LayerManager *pLM = GetLayerManager();
        if ( pLM != NULL )
        {
           if ( pLM->GetDeviceToMonitor() != NULL )
           {
              pLM->OnDetach();
           }
        }
        */

        if (s_bInitialized)
        {
            DeinitCommunication();
            LogFooter();
        }
    }

    return TRUE;
}

#ifdef _MANAGED
    #pragma managed( pop )
#endif

//=============================================================================
//
// EntryPoints for Request Processing
//
//=============================================================================


//-----------------------------------------------------------------------------
/// ProcessRequests
///
/// short
///
/// \param requestID The ID of a request that is in waiting to be processed.
///  This ID can be used to query the contents of an incoming request.
///
/// \return true if the request could be processed; false otherwise
//-----------------------------------------------------------------------------
GPS_PLUGIN_API bool ProcessRequest(CommunicationID requestID)
{
    CommandObject command(requestID, (char*) GetRequestText(requestID));

    LayerManager* pLM = DXGILayerManager::Instance();

    if (pLM != NULL)
    {
        return pLM->Process(command);
    }

    return false;
}

//=============================================================================
//
// EntryPoints for Plugin Registration
//
//=============================================================================

//-----------------------------------------------------------------------------
/// GetPluginVersion
///
/// Provides the version number of the implemented plugin. This will be exposed
/// to client-side plugins so that they can require a minimum version of a
/// particular server-side plugin. All efforts should be made to maintain
/// backwards compatibility.
///
/// \return pointer to the version string; cannot be NULL
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetPluginVersion()
{
    return PERFSTUDIOSERVER_VERSION_STRING;
}

//-----------------------------------------------------------------------------
/// GetShortDescription
///
/// This provides a unique short (one word) description of the functionality of
/// the plugin. Ex: A plugin which wraps an API may send back the name of the
/// API, and a plugin which profiles a piece of hardware may send back the name
/// of the hardware which it profiles. If multiple plugins exist which have the
/// same short description, only one of them will be loaded for use.
///
/// \return pointer to the short description; cannot be NULL
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetShortDescription()
{
    return "DXGI";
}

//-----------------------------------------------------------------------------
/// GetLongDescription
///
/// This provides a long (one sentence) description of the functionality of the
/// plugin. This string may be shown on the client-side so that users know the
/// functionality of the available server-side plugins.
///
/// \return pointer to the long description; cannot be NULL
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetLongDescription()
{
    return "Provides DXGI monitoring.";
}

//=============================================================================
//
// EntryPoints for Wrapper Plugins
//
//=============================================================================

//-----------------------------------------------------------------------------
/// GetWrappedDLLName
///
/// Provides the name of the dll which can be wrapped by this plugin.
///
/// \return pointer to the name of the dll; cannot be NULL
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetWrappedDLLName()
{
    return "dxgi.dll";
}

//-----------------------------------------------------------------------------
/// InitPlugin
///
/// Function which initializes the comms and profiler
///
/// \return true if the initialization was successful; false otherwise
//-----------------------------------------------------------------------------
bool InitPlugin()
{
    LogHeader();

    if (InitCommunication(GetShortDescription(), ProcessRequest) == false)
    {
        DeinitCommunication();
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Check that UpdateHooks has been called. If not, call it
//-----------------------------------------------------------------------------
bool CheckUpdateHooks()
{
    if (s_bInitialized == false)
    {
        return UpdateHooks();
    }

    // hooks already updated, nothing to do here
    return true;
}

//-----------------------------------------------------------------------------
/// UpdateHooks
///
/// Function which causes all necessary entrypoints to be hooked
///
/// \return true if hooking was successful; false will unload the wrapper
//-----------------------------------------------------------------------------
GPS_PLUGIN_API bool UpdateHooks()
{
    if (s_bInitialized == false)
    {
        if (InitPlugin() == true)
        {
            s_bInitialized = true;
        }

#ifdef DLL_REPLACEMENT

        // Load the real DXGI.dll and get a handle to it so that the real functions can
        // be obtained later
        if (s_hRealDXGI == 0)
        {
            s_hRealDXGI = DllReplacement::LoadRealLibrary("dxgi.dll");

            if (!s_hRealDXGI)
            {
                LogConsole(logERROR, "failed to load dxgi.dll\n");
                return false;
            }
        }

#endif // DLL_REPLACEMENT
    }

#ifdef DLL_REPLACEMENT
    // Get the real factory function pointers from the real DXGI dll handle
    ReplaceCreateDXGIFactoryFunction(s_hRealDXGI);
    ReplaceCreateDXGIFactory1Function(s_hRealDXGI);
    ReplaceCreateDXGIFactory2Function(s_hRealDXGI);
#else
    static HMODULE Old_hDXGI = NULL;

    HMODULE hDXGI = NULL;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, "dxgi.dll", &hDXGI);

    // Handle DXGI
    if (Old_hDXGI == NULL && hDXGI != NULL)
    {
        HookCreateDXGIFactoryFunction(hDXGI);
        HookCreateDXGIFactory1Function(hDXGI);
        HookCreateDXGIFactory2Function(hDXGI);
    }
    else if (hDXGI == NULL && Old_hDXGI != NULL)
    {
        UnhookCreateDXGIFactoryFunction(hDXGI);
        UnhookCreateDXGIFactory1Function(hDXGI);
        UnhookCreateDXGIFactory2Function(hDXGI);
    }
    else if (hDXGI != Old_hDXGI)
    {
        UnhookCreateDXGIFactoryFunction(NULL);
        UnhookCreateDXGIFactory1Function(NULL);
        UnhookCreateDXGIFactory2Function(NULL);

        HookCreateDXGIFactoryFunction(hDXGI);
        HookCreateDXGIFactory1Function(hDXGI);
        HookCreateDXGIFactory2Function(hDXGI);
    }

    Old_hDXGI = hDXGI;
#endif

#ifdef LIQUID_VR_SUPPORT
    HookLiquidVR();
#endif

    return true;
}

#ifdef _MANAGED
    #pragma managed(pop)
#endif


