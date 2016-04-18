//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A LayerManager responsible that acts as the entrypoint for the DX12
///         server plugin.
//=============================================================================

#ifndef DX12LAYERMANAGER_H
#define DX12LAYERMANAGER_H

#include "../Common/ModernAPILayerManager.h"
#include "../Common/ObjectDatabaseProcessor.h"
#include "../Common/defines.h"
#include "../Common/StreamLog.h"
#include "Interception/Hook_D3D12.h"
#include "DX12Defines.h"

class DX12Interceptor;

extern bool CheckUpdateHooks();

//-----------------------------------------------------------------------------
/// DX12LayerManager - A LayerManager with the special task of managing
/// layers specific to DX12's runtime stack.
//-----------------------------------------------------------------------------
class DX12LayerManager : public ModernAPILayerManager, public TSingleton < DX12LayerManager >
{
    //-----------------------------------------------------------------------------
    /// TSingleton is a friend of the DX12LayerManager.
    //-----------------------------------------------------------------------------
    friend TSingleton < DX12LayerManager >;

    //-----------------------------------------------------------------------------
    /// Default private constructor, where available layers are determined.
    //-----------------------------------------------------------------------------
    DX12LayerManager();

public:
    //-----------------------------------------------------------------------------
    /// Virtual destructor, where nothing important happens.
    //-----------------------------------------------------------------------------
    virtual ~DX12LayerManager() {}

    //-----------------------------------------------------------------------------
    /// Gets called after a frame has been presented.
    /// \param inSwapchain The swap chain instance used to present to the screen.
    //-----------------------------------------------------------------------------
    virtual void OnPresent(void* inSwapchain);

    //-----------------------------------------------------------------------------
    /// Gets called immediately after the real Present() is called
    /// \param SyncInterval The sync interval passed into the real Present() call.
    /// \param Flags The flags passed into the real Present() call.
    //-----------------------------------------------------------------------------
    virtual void OnPresent(UINT SyncInterval, UINT Flags);

    //-----------------------------------------------------------------------------
    /// Gets called when a swap chain is created.
    /// \param inSwapchain The swap chain instance that was just created.
    /// \param inDevice The device used to created the swap chain.
    //-----------------------------------------------------------------------------
    virtual void OnSwapchainCreated(void* inSwapchain, void* inDevice);

    //-----------------------------------------------------------------------------
    /// Retrieve a pointer to the TraceAnalyzer layer.
    /// \returns A pointer to the TraceAnalyzer layer.
    //-----------------------------------------------------------------------------
    virtual MultithreadedTraceAnalyzerLayer* GetTraceAnalyzerLayer();

    //-----------------------------------------------------------------------------
    /// Retrieve a pointer to the FrameDebugger layer.
    /// \returns A pointer to the FrameDebugger layer.
    //-----------------------------------------------------------------------------
    virtual ModernAPIFrameDebuggerLayer* GetFrameDebuggerLayer();

    //-----------------------------------------------------------------------------
    /// Retrieve a pointer to the FrameProfiler layer.
    /// \returns A pointer to the FrameProfiler layer.
    //-----------------------------------------------------------------------------
    virtual ModernAPIFrameProfilerLayer* GetFrameProfilerLayer();

    //-----------------------------------------------------------------------------
    /// Retrieve a pointer to the Object Database Processor.
    /// \returns A pointer to the Object Database Processor.
    //-----------------------------------------------------------------------------
    virtual ObjectDatabaseProcessor* GetObjectDatabaseProcessor();

    //-----------------------------------------------------------------------------
    /// Initialize the LayerManager by pushing available layers and starting the interceptors.
    //-----------------------------------------------------------------------------
    bool InitializeLayerManager();

    //-----------------------------------------------------------------------------
    /// Shutdown the LayerManager by detaching hooks and destroying instances.
    //-----------------------------------------------------------------------------
    bool ShutdownLayerManager();

    //-----------------------------------------------------------------------------
    /// Answer the "Has the DX12LayerManager been initialized?" question.
    /// \return True if the DX12LayerManager was initialized successfully.
    //-----------------------------------------------------------------------------
    inline bool HasBeenInitialized() const { return mbIsInitialized; }

    //-----------------------------------------------------------------------------
    /// Retrieve a pointer to the DX12Interceptor- the logic for call interception lives here.
    //-----------------------------------------------------------------------------
    inline DX12Interceptor* GetInterceptor() const { return mInterceptor; }

    //-----------------------------------------------------------------------------
    /// Retrieve a reference to the Hook_D3D12 instance used to hook intercepted entrypoints.
    /// \returns A reference to the Hook_D3D12 instance.
    //-----------------------------------------------------------------------------
    inline Hook_D3D12& GetModuleHook() { return mModuleHook; }

    /// Streamlog member used for debugging.
    StreamLog mStreamLog;

    //-----------------------------------------------------------------------------
    /// Obtain a streamlog reference
    /// \returns The StreamLog instance used for the DX12LayerManager.
    //-----------------------------------------------------------------------------
    StreamLog& GetStreamLog()
    {
        return mStreamLog;
    }

    //-----------------------------------------------------------------------------
    /// Send the incoming message to the DX12LayerManager's Streamlog.
    /// \param inLogString The string to log.
    //-----------------------------------------------------------------------------
    void StreamLog(char* inLogString)
    {
        mStreamLog.LogMsg(inLogString);
    }

    //-----------------------------------------------------------------------------
    /// Send the incoming message to the DX12LayerManager's Streamlog.
    /// \param inLogString The string to log.
    //-----------------------------------------------------------------------------
    void StreamLog(const char* inLogString)
    {
        mStreamLog.LogMsg(inLogString);
    }

    //-----------------------------------------------------------------------------
    /// Send the incoming message to the DX12LayerManager's Streamlog.
    /// \param inLogStringStream The string to log.
    //-----------------------------------------------------------------------------
    void StreamLog(std::stringstream& inLogStringStream)
    {
        mStreamLog.LogMsg(inLogStringStream);
    }

private:
    /// An instance of the DX12Interceptor- the piece that helps decide what to do with intercepted calls.
    DX12Interceptor* mInterceptor;

    /// This member is the object responsible for hooking D3D12 entry points.
    Hook_D3D12 mModuleHook;

    /// A true/false to indicate if the LayerManager was started successfully.
    bool mbIsInitialized;
};

//-----------------------------------------------------------------------------
/// Retrieve the DX12LayerManager instance.
/// \returns The DX12LayerManager singleton instance.
//-----------------------------------------------------------------------------
DX12LayerManager* GetDX12LayerManager();

#endif //DX12LAYERMANAGER_H