//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktLayerManager.cpp
/// \brief  Implementation file for Vulkan layer manager.
///         This is a high level class which maintains a stack of available
///         and enabled layers in the Vulkan server. It also provides a means to
///         perform custom work at create, destroy, and begin-frame time.
//==============================================================================

#include "vktLayerManager.h"

#include "Tracing/vktTraceAnalyzerLayer.h"
#include "FrameDebugger/vktFrameDebuggerLayer.h"
#include "Profiling/vktFrameProfilerLayer.h"
#include "vktInterceptManager.h"

#include "Util/vktUtil.h"
#include "Objects/vktObjectDatabaseProcessor.h"

#include "../../Common/TimeControlLayer.h"
#include "../../Common/WrappedObjectDatabase.h"
#include "../../Common/IServerPlugin.h"
#include "../../Common/SharedGlobal.h"

#ifdef CODEXL_GRAPHICS

//-----------------------------------------------------------------------------
/// Initial configuration of layers that we can use within the VulkanServer DLL for CodeXL.
//-----------------------------------------------------------------------------
static LAYERDESC s_LayerList[] =
{
    { "Logger",         "Trace Analyzer",   "LOG",  NO_DISPLAY,     VktTraceAnalyzerLayer::Instance(),         VktTraceAnalyzerLayer::Instance() },
    { "ObjectDatabase", "Object Database",  "DB",   DISPLAY,        VktObjectDatabaseProcessor::Instance(),    VktObjectDatabaseProcessor::Instance() },
    { "FrameDebugger",  "Frame Debugger",   "FD",   NO_DISPLAY,     VktFrameDebuggerLayer::Instance(),         VktFrameDebuggerLayer::Instance() },
    { "FrameProfiler",  "Frame Profiler",   "FP",   NO_DISPLAY,     VktFrameProfilerLayer::Instance(),         VktFrameProfilerLayer::Instance() }
};

#else

//-----------------------------------------------------------------------------
/// Initial configuration of layers that we can use within the VulkanServer DLL for GPS.
//-----------------------------------------------------------------------------
static LAYERDESC s_LayerList[] =
{
    { "Logger",         "Trace Analyzer",   "LOG",  NO_DISPLAY,     VktTraceAnalyzerLayer::Instance(),         VktTraceAnalyzerLayer::Instance() },
    { "ObjectDatabase", "Object Database",  "DB",   DISPLAY,        VktObjectDatabaseProcessor::Instance(),    VktObjectDatabaseProcessor::Instance() },
    { "FrameDebugger",  "Frame Debugger",   "FD",   NO_DISPLAY,     VktFrameDebuggerLayer::Instance(),         VktFrameDebuggerLayer::Instance() },
    { "FrameProfiler",  "Frame Profiler",   "FP",   NO_DISPLAY,     VktFrameProfilerLayer::Instance(),         VktFrameProfilerLayer::Instance() },
    { "TimeControl",    "Time Control",     "TC",   NO_DISPLAY,     TimeControlLayer::Instance(),              TimeControlLayer::Instance() }
};

#endif

//-----------------------------------------------------------------------------
/// Get a pointer to this layer manager.
/// \return A pointer to this layer manager.
//-----------------------------------------------------------------------------
VktLayerManager* VktLayerManager::GetLayerManager()
{
    static VktLayerManager sInstance;
    return (VktLayerManager*)&sInstance;
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the TraceAnalyzer layer.
/// \returns A pointer to the TraceAnalyzer layer.
//-----------------------------------------------------------------------------
MultithreadedTraceAnalyzerLayer* VktLayerManager::GetTraceAnalyzerLayer()
{
    return VktTraceAnalyzerLayer::Instance();
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the TraceAnalyzer layer.
/// \returns A pointer to the TraceAnalyzer layer.
//-----------------------------------------------------------------------------
ModernAPIFrameDebuggerLayer* VktLayerManager::GetFrameDebuggerLayer()
{
    return VktFrameDebuggerLayer::Instance();
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the FrameProfiler layer.
/// \returns A pointer to the FrameProfiler layer.
//-----------------------------------------------------------------------------
ModernAPIFrameProfilerLayer* VktLayerManager::GetFrameProfilerLayer()
{
    return VktFrameProfilerLayer::Instance();
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the ObjectDatabase layer.
/// \returns A pointer to the ObjectDatabase layer.
//-----------------------------------------------------------------------------
ObjectDatabaseProcessor* VktLayerManager::GetObjectDatabaseProcessor()
{
    return VktObjectDatabaseProcessor::Instance();
}

//-----------------------------------------------------------------------------
/// Constructor where available layers are pushed.
//-----------------------------------------------------------------------------
VktLayerManager::VktLayerManager() :
    m_isInitialized(false)
{
    m_LayerList = (LAYERDESC*)& s_LayerList;
    m_LayerListSize = sizeof(s_LayerList) / sizeof(s_LayerList[0]);

    // Start with the object database layer enabled, as we want to track object instances from the start.
    m_AvailableLayers.push_back(VktObjectDatabaseProcessor::Instance());

    // If AutoCapture is enabled in the log file, automatically push the TraceAnalyzer to the stack.
    bool bPushTraceAnalyzer = (SG_GET_INT(OptionTraceType) != kTraceType_None);

    if (bPushTraceAnalyzer)
    {
        VktTraceAnalyzerLayer* pTraceLayer = VktTraceAnalyzerLayer::Instance();
        m_AvailableLayers.push_back(pTraceLayer);
        PushLayer(*pTraceLayer, &m_pushLayer);
    }
}

//-----------------------------------------------------------------------------
/// Initialize the LayerManager.
/// \return True if the LayerManager was initialized successfully.
//-----------------------------------------------------------------------------
bool VktLayerManager::InitializeLayerManager()
{
    // Keep the TimeControlLayer to the bottom of the stack;
    m_AvailableLayers.push_back(TimeControlLayer::Instance());

    // Initialize all of the layers that the plugin requires. They'll be pushed on first use.
    bool success = OnCreate(VULKAN_DEVICE, nullptr);

    if (success == true)
    {
        // If the LayerManager was initialized successfully, attempt to initialize the interceptor.
        m_pInterceptMgr = new VktInterceptManager();

        // If the interceptor has initialized, we're off to a good start. Try to Init communication with the PerfServer.
        if (success == true)
        {
            success = UpdateHooks();
        }
    }

    m_isInitialized = success;

    return success;
}

//-----------------------------------------------------------------------------
/// Shutdown the LayerManager by detaching hooks and destroying instances.
/// \returns True if the LayerManager was destroyed fully and successfully.
//-----------------------------------------------------------------------------
bool VktLayerManager::ShutdownLayerManager()
{
    DeinitCommunication();

    bool success = OnDestroy(VULKAN_DEVICE, nullptr);

    if (success == false)
    {
        Log(logERROR, "Failed to destroy the VtkLayerManager during shutdown.\n");
    }

    return success;
}

//--------------------------------------------------------------------------
/// Process a new request received by the server plugin.
/// \param inRequestId The Id associated with the new incoming request.
/// \return True if the request was handled successfully.
//--------------------------------------------------------------------------
bool VktLayerManager::ProcessRequestFromCommId(CommunicationID inRequestId)
{
    CommandObject command(inRequestId, (char*)GetRequestText(inRequestId));

    return Process(command);
}

//--------------------------------------------------------------------------
/// Destroy all layers tracked by this LayerManager.
/// \param type The creation type.
/// \param pPtr Output pointer.
/// \return True if successful.
//--------------------------------------------------------------------------
bool VktLayerManager::OnDestroy(CREATION_TYPE type, void* pPtr)
{
    bool success = false;

    // tell each layer that the context was destroyed; Leave the last layer TimeControl layer destroyed later
    for (uint32 i = 0; i < m_AvailableLayers.size() - 1; i++)
    {
        ILayer* pLayer = m_AvailableLayers[m_AvailableLayers.size() - i - 1];

        success = pLayer->OnDestroy(type, pPtr);

        if (success == false)
        {
            // Something went wrong when we tried to destroy the layer- report it here.
            Log(logERROR, "Layer with index '%u' failed in OnDestroy call.\n", i);
        }
    }

    return success;
}

//-----------------------------------------------------------------------------
/// he signal that a new frame is beginning. Let all of the layers know that this is happening.
//-----------------------------------------------------------------------------
void VktLayerManager::BeginFrame()
{
    // Call into the base class to deal with basic layer management.
    ModernAPILayerManager::BeginFrame();
}

//-----------------------------------------------------------------------------
/// A handler invoked when Autocapture mode has been triggered.
//-----------------------------------------------------------------------------
void VktLayerManager::AutocaptureTriggered()
{
    bool validTraceMode = SG_GET_INT(OptionTraceType) != kTraceType_None;

    if (validTraceMode)
    {
        m_AutoCapture = true;
    }
}