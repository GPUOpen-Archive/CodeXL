//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A LayerManager responsible that acts as the entrypoint for the DX12
///         server plugin.
//=============================================================================

#include "DX12LayerManager.h"
#include "Interception/DX12Interceptor.h"
#include "Tracing/DX12TraceAnalyzerLayer.h"
#include "Objects/DX12ObjectDatabaseProcessor.h"
#include "FrameDebugger/DX12FrameDebuggerLayer.h"
#include "Profiling/DX12FrameProfilerLayer.h"

#include <AMDTOSWrappers/Include/osDirectory.h>

#include "../Common/TimeControlLayer.h"
#include "../Common/ConnectWithDXGI.h"
#include "../Common/IMonitor.h"
#include "../Common/IServerPlugin.h"
#include "../Common/PerfStudioServer_Version.h"
#include "../Common/SharedGlobal.h"
#include "../Common/Windows/DllReplacement.h"

//-----------------------------------------------------------------------------
/// A flag used to track if this server plugin has been hooked.
//-----------------------------------------------------------------------------
static bool s_bHooked = false;

//-----------------------------------------------------------------------------
/// A flag used to track if this server plugin has been initialized.
//-----------------------------------------------------------------------------
static bool s_bInitialized = false;

#ifdef DLL_REPLACEMENT
    static HINSTANCE s_hRealD3D12 = 0;       // handle to real D3D12 dll
#endif // DLL_REPLACEMENT

//-----------------------------------------------------------------------------
/// Exported function used in stepping the message pump for the server plugin.
/// \param requestID The ID of a request that is in waiting to be processed.
/// This ID can be used to query the contents of an incoming request.
/// \return true if the request could be processed; false otherwise
//-----------------------------------------------------------------------------
GPS_PLUGIN_API bool ProcessRequest(CommunicationID requestID)
{
    DX12LayerManager* dx12LayerManager = GetDX12LayerManager();
    return dx12LayerManager->ProcessRequestFromCommId(requestID);
}

//-----------------------------------------------------------------------------
/// Provides the version number of the implemented plugin. This will be exposed
/// to client-side plugins so that they can require a minimum version of a
/// particular server-side plugin. All efforts should be made to maintain
/// backwards compatibility.
/// \return pointer to the version string; cannot be nullptr
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetPluginVersion() { return PERFSTUDIOSERVER_VERSION_STRING; }

//-----------------------------------------------------------------------------
/// This provides a unique short (one word) description of the functionality of
/// the plugin. Ex: A plugin which wraps an API may send back the name of the
/// API, and a plugin which profiles a piece of hardware may send back the name
/// of the hardware which it profiles. If multiple plugins exist which have the
/// same short description, only one of them will be loaded for use.
/// \return pointer to the short description; cannot be nullptr
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetShortDescription() { return "DX12"; }

//-----------------------------------------------------------------------------
/// This provides a long (one sentence) description of the functionality of the
/// plugin. This string may be shown on the client-side so that users know the
/// functionality of the available server-side plugins.
/// \return pointer to the long description; cannot be nullptr
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetLongDescription() { return "Provides DirectX 12 Instrumentation."; }

//-----------------------------------------------------------------------------
/// Provides the name of the dll which can be wrapped by this plugin.
/// \return pointer to the name of the dll; cannot be nullptr
//-----------------------------------------------------------------------------
GPS_PLUGIN_API const char* GetWrappedDLLName() { return D3D12_DLL; }

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the DX12LayerManager that manages this API server.
/// \returns A pointer to the DX12LayerManager.
//-----------------------------------------------------------------------------
GPS_PLUGIN_API LayerManager* GetServerLayerManager() { return DX12LayerManager::Instance(); }

//-----------------------------------------------------------------------------
/// Initialize the DX12Server plugin.
/// \returns True if initialization was successful, false if it failed.
//-----------------------------------------------------------------------------
bool InitPlugin()
{
    LogHeader();

    if (InitCommunication(GetShortDescription(), ProcessRequest) == false)
    {
        DeinitCommunication();
        return false;
    }

    Log(logTRACE, "DX12Server2's InitPlugin success.\n");
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
/// Function which causes all necessary entrypoints to be hooked.
/// \return true if hooking was successful; false will unload the wrapper
//-----------------------------------------------------------------------------
GPS_PLUGIN_API bool UpdateHooks()
{
    Log(logTRACE, "DX12Server2's UpdateHooks.\n");

    if (s_bInitialized == false)
    {
        if (InitPlugin() == false)
        {
            return false;
        }

        s_bInitialized = true;

#ifdef DLL_REPLACEMENT

        // Load the real D3D12.dll and get a handle to it so that the real functions can
        // be obtained later
        if (s_hRealD3D12 == 0)
        {
            s_hRealD3D12 = DllReplacement::LoadRealLibrary(D3D12_DLL);

            if (!s_hRealD3D12)
            {
                Log(logERROR, "failed to load D3D12.dll\n");
                return false;
            }
        }

#endif // DLL_REPLACEMENT
    }

    DX12LayerManager* dx12LayerManager = GetDX12LayerManager();
#ifndef USE_OLD_DXGI_HOOKING
    ConnectWithDXGI(dx12LayerManager);
#endif

    if (dx12LayerManager != nullptr && !dx12LayerManager->HasBeenInitialized())
    {
        if (!dx12LayerManager->InitializeLayerManager())
        {
            Log(logWARNING, "The DX12LayerManager was not initialized successfully.\n");
        }
    }

    return true;
}

#ifdef CODEXL_GRAPHICS

//-----------------------------------------------------------------------------
/// Initial configuration of layers that we can use within the DX12Server DLL for CodeXL.
//-----------------------------------------------------------------------------
static LAYERDESC s_LayerList[] =
{
    { "Logger",         "Trace Analyzer",   "LOG",  NO_DISPLAY,     DX12TraceAnalyzerLayer::Instance(),         DX12TraceAnalyzerLayer::Instance() },
    { "ObjectDatabase", "Object Database",  "DB",   DISPLAY,        DX12ObjectDatabaseProcessor::Instance(),    DX12ObjectDatabaseProcessor::Instance() },
    { "FrameDebugger",  "Frame Debugger",   "FD",   NO_DISPLAY,     DX12FrameDebuggerLayer::Instance(),         DX12FrameDebuggerLayer::Instance() },
    { "FrameProfiler",  "Frame Profiler",   "FP",   NO_DISPLAY,     DX12FrameProfilerLayer::Instance(),         DX12FrameProfilerLayer::Instance() }
};

#else

//-----------------------------------------------------------------------------
/// Initial configuration of layers that we can use within the DX12Server DLL for GPS.
//-----------------------------------------------------------------------------
static LAYERDESC s_LayerList[] =
{
    { "Logger",         "Trace Analyzer",   "LOG",  NO_DISPLAY,     DX12TraceAnalyzerLayer::Instance(),         DX12TraceAnalyzerLayer::Instance() },
    { "ObjectDatabase", "Object Database",  "DB",   DISPLAY,        DX12ObjectDatabaseProcessor::Instance(),    DX12ObjectDatabaseProcessor::Instance() },
    { "FrameDebugger",  "Frame Debugger",   "FD",   NO_DISPLAY,     DX12FrameDebuggerLayer::Instance(),         DX12FrameDebuggerLayer::Instance() },
    { "FrameProfiler",  "Frame Profiler",   "FP",   NO_DISPLAY,     DX12FrameProfilerLayer::Instance(),         DX12FrameProfilerLayer::Instance() },
    { "TimeControl",    "Time Control",     "TC",   NO_DISPLAY,     TimeControlLayer::Instance(),               TimeControlLayer::Instance() }
};

#endif

//-----------------------------------------------------------------------------
/// Retrieve the DX12LayerManager instance.
/// \returns The DX12LayerManager singleton instance.
//-----------------------------------------------------------------------------
DX12LayerManager* GetDX12LayerManager()
{
    return DX12LayerManager::Instance();
}

//-----------------------------------------------------------------------------
/// Constructor where available layers are pushed.
//-----------------------------------------------------------------------------
DX12LayerManager::DX12LayerManager()
    : ModernAPILayerManager()
    , mInterceptor(nullptr)
    , mbIsInitialized(false)
{
    m_LayerList = (LAYERDESC*) & s_LayerList;
    m_LayerListSize = sizeof(s_LayerList) / sizeof(s_LayerList[0]);

    // Start with the object database layer enabled, as we want to track object instances from the start.
    m_AvailableLayers.push_back(DX12ObjectDatabaseProcessor::Instance());

    m_AvailableLayers.push_back(DX12FrameDebuggerLayer::Instance());

    m_AvailableLayers.push_back(DX12TraceAnalyzerLayer::Instance());

    // If AutoCapture is enabled in the config file, automatically push the TraceAnalyzer to the stack.
    bool bPushTraceAnalyzer = (SG_GET_INT(OptionTraceType) != kTraceType_None);

    if (bPushTraceAnalyzer)
    {
        // Add the TraceAnalyzer so it can AutoCapture at a specified frame.
        DX12TraceAnalyzerLayer* traceLayer = DX12TraceAnalyzerLayer::Instance();
        PushLayer(*traceLayer, &m_pushLayer);
    }

    osFilePath logFilePath;
    logFilePath.setPath(osFilePath::OS_TEMP_DIRECTORY);
    char fullPath[PS_MAX_PATH] = {};
    sprintf(fullPath, "%s\\%s\\GPS", logFilePath.asString().asASCIICharArray(), GetPerfStudioDirName());
    mStreamLog.SetBaseNamePath(fullPath);
}

//-----------------------------------------------------------------------------
/// Gets called after a frame has been presented.
/// \param inSwapchain The swap chain instance used to present to the screen.
//-----------------------------------------------------------------------------
void DX12LayerManager::OnPresent(void* inSwapchain)
{
    // Forward the OnPresent to the FrameDebugger layer for now. Can likely be moved with DX12CaptureLayer exists.
    DX12FrameDebuggerLayer::Instance()->OnPresent(inSwapchain);
}

//-----------------------------------------------------------------------------
/// Gets called immediately after the real Present() is called
/// \param SyncInterval The sync interval passed into the real Present() call.
/// \param Flags The flags passed into the real Present() call.
//-----------------------------------------------------------------------------
void DX12LayerManager::OnPresent(UINT SyncInterval, UINT Flags)
{
    // Forward the OnPresent to the TraceAnalyzer layer for now.
    DX12TraceAnalyzerLayer::Instance()->OnPresent(SyncInterval, Flags);
}

//-----------------------------------------------------------------------------
/// Gets called when a swap chain is created.
/// \param inSwapchain The swap chain instance that was just created.
/// \param inDevice The device used to created the swap chain.
//-----------------------------------------------------------------------------
void DX12LayerManager::OnSwapchainCreated(void* inSwapchain, void* inDevice)
{
    // Forward the Swapchain creation to the FrameDebugger layer for now. Can likely be moved with DX12CaptureLayer exists.
    DX12FrameDebuggerLayer::Instance()->OnSwapchainCreated(inSwapchain, inDevice);
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the TraceAnalyzer layer.
/// \returns A pointer to the TraceAnalyzer layer.
//-----------------------------------------------------------------------------
MultithreadedTraceAnalyzerLayer* DX12LayerManager::GetTraceAnalyzerLayer()
{
    return DX12TraceAnalyzerLayer::Instance();
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the TraceAnalyzer layer.
/// \returns A pointer to the TraceAnalyzer layer.
//-----------------------------------------------------------------------------
ModernAPIFrameDebuggerLayer* DX12LayerManager::GetFrameDebuggerLayer()
{
    return DX12FrameDebuggerLayer::Instance();
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the FrameProfiler layer.
/// \returns A pointer to the FrameProfiler layer.
//-----------------------------------------------------------------------------
ModernAPIFrameProfilerLayer* DX12LayerManager::GetFrameProfilerLayer()
{
    return DX12FrameProfilerLayer::Instance();
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the ObjectDatabase layer.
/// \returns A pointer to the ObjectDatabase layer.
//-----------------------------------------------------------------------------
ObjectDatabaseProcessor* DX12LayerManager::GetObjectDatabaseProcessor()
{
    return DX12ObjectDatabaseProcessor::Instance();
}

//-----------------------------------------------------------------------------
/// Initialize the LayerManager.
/// \return True if the LayerManager was initialized successfully.
//-----------------------------------------------------------------------------
bool DX12LayerManager::InitializeLayerManager()
{
    // Initialize all of the layers that the plugin requires. They'll be pushed on first use.
    bool bLayerManagerInitialized = OnCreate(DX12_DEVICE, nullptr);

    if (bLayerManagerInitialized)
    {
        bool bPluginRegistered = RegisterActivePlugin(GetShortDescription());

        if (!bPluginRegistered)
        {
            Log(logERROR, "Failed to register DX12Server plugin.\n");
        }
        else
        {
            // If the LayerManager was initialized successfully, attempt to initialize the interceptor.
            mInterceptor = new DX12Interceptor();
#ifdef DLL_REPLACEMENT

            // Pass the handle to the system's D3D12 dll to the Hook_D3D12 module so that the real
            // functions can be obtained later.
            mModuleHook.SetRealDllHandle(s_hRealD3D12);
#endif
            // Hook the entrypoints we'd like to intercept.
            bLayerManagerInitialized = mModuleHook.HookInterceptor();

            if (!bLayerManagerInitialized)
            {
                Log(logERROR, "Failed to hook D3D12 entrypoints.\n");
            }
        }
    }
    else
    {
        Log(logERROR, "Failed to initialize DX12Server plugin.\n");
    }

    mbIsInitialized = bLayerManagerInitialized;
    return bLayerManagerInitialized;
}

//-----------------------------------------------------------------------------
/// Shutdown the LayerManager by detaching hooks and destroying instances.
/// \returns True if the LayerManager was destroyed fully and successfully.
//-----------------------------------------------------------------------------
bool DX12LayerManager::ShutdownLayerManager()
{
    bool bShutdownSuccess = false;

    DeinitCommunication();

    // Try to shutdown the DX12Interceptor if it was created during initialization.
    if (mInterceptor != nullptr)
    {
        if ((bShutdownSuccess = mInterceptor->Shutdown()) == false)
        {
            Log(logERROR, "Failed to clean up the DX12Interceptor during shutdown.\n");
        }

        SAFE_DELETE(mInterceptor);
    }

    bool bUnhookSuccessful = mModuleHook.UnhookInterceptor();

    if (!bUnhookSuccessful)
    {
        Log(logERROR, "Failed to unhook intercepted D3D12 entrypoints during shutdown.\n");
    }

    if ((bShutdownSuccess = OnDestroy(DX12_DEVICE, nullptr)) == false)
    {
        Log(logERROR, "Failed to destroy the DX12LayerManager during shutdown.\n");
    }

    return bShutdownSuccess;
}

//-----------------------------------------------------------------------------
/// Entrypoint for the DX12Server plugin. Short rundown:
/// On DLL_PROCESS_ATTACH, initialize the DX12LayerManager.
/// On DLL_PROCESS_DETACH, kill and destroy the DX12LayerManager.
/// \param hModule The module associated with this invocation of *this* DllMain.
/// \param dwReason The reason that this function is being invoked.
/// \param pReserved Reserved mystery pointer.
/// \returns See http://msdn.microsoft.com/en-us/library/windows/desktop/ms682583(v=vs.85).aspx to understand this.
//-----------------------------------------------------------------------------
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD dwReason, VOID* pReserved)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(pReserved);

    Log(logTRACE, "DX12Server2's DllMain hit with reason '%d'\n", dwReason);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            s_bHooked = s_bInitialized = false;
        }
        break;

        case DLL_PROCESS_DETACH:
        {
#ifdef DLL_REPLACEMENT
            FreeLibrary(s_hRealD3D12);
#endif // DLL_REPLACEMENT
            // Only shutdown the DX12LayerManager if it was initialized.
            DX12LayerManager* layerManager = GetDX12LayerManager();

            if (layerManager->HasBeenInitialized())
            {
                if (layerManager->ShutdownLayerManager() == false)
                {
                    Log(logWARNING, "The DX12LayerManager was not shutdown successfully.\n");
                }

                // Disconnect from the running DXGI server before we kill the plugin.
#ifndef USE_OLD_DXGI_HOOKING
                DisconnectFromDXGI(layerManager);
#endif
            }
            else
            {
                Log(logERROR, "DX12Server shutdown: The DX12LayerManager was not initialized.\n");
            }
        }
        break;
    }

    return true;
}