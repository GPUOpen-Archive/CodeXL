//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12TraceAnalyzerLayer.h
/// \brief  The DX12 TraceAnalyzer layer that manages API and GPU trace collection and response.
//=============================================================================

#ifndef DX12TRACEANALYZERLAYER_H
#define DX12TRACEANALYZERLAYER_H

#include "../../Common/Tracing/MultithreadedTraceAnalyzerLayer.h"
#include "../Profiling/DX12FrameProfilerLayer.h"
#include "../Util/DX12Utilities.h"
#include "../DX12Defines.h"

//-----------------------------------------------------------------------------
/// Collects API/GPU Trace in a multi-threaded manner by mapping each submission
/// thread to its own buffer that it can dump logged calls to.
//-----------------------------------------------------------------------------
class DX12TraceAnalyzerLayer : public MultithreadedTraceAnalyzerLayer, public TSingleton < DX12TraceAnalyzerLayer >
{
    //-----------------------------------------------------------------------------
    /// TSingleton is a friend of the DX12TraceAnalyzerLayer.
    //-----------------------------------------------------------------------------
    friend TSingleton < DX12TraceAnalyzerLayer >;
public:
    //-----------------------------------------------------------------------------
    /// DX12TraceAnalyzerLayer destructor. Will clean up trace data.
    //-----------------------------------------------------------------------------
    virtual ~DX12TraceAnalyzerLayer() {}

    //-----------------------------------------------------------------------------
    /// Retrieve a pointer to the parent LayerManager used by this tool.
    /// \returns A pointer to the parent LayerManager used by this tool.
    //-----------------------------------------------------------------------------
    ModernAPILayerManager* GetParentLayerManager();

    //-----------------------------------------------------------------------------
    /// Identify which API this TraceAnalyzer subclass handles.
    /// \returns A string containing "DX12".
    //-----------------------------------------------------------------------------
    virtual char const* GetAPIString() { return "DX12"; }

    //-----------------------------------------------------------------------------
    /// Provides a chance to initialize states before a GPU trace is performed.
    //-----------------------------------------------------------------------------
    virtual void BeforeGPUTrace();

    //-----------------------------------------------------------------------------
    /// Provides a chance to initialize states before a GPU trace is performed.
    //-----------------------------------------------------------------------------
    virtual void AfterGPUTrace();

    //-----------------------------------------------------------------------------
    /// Fill a vector with all known queues.
    //-----------------------------------------------------------------------------
    void GetAvailableQueues(DX12FrameProfilerLayer* pFrameProfiler, std::vector<Wrapped_ID3D12CommandQueueCustom*>& customQueues);

    //-----------------------------------------------------------------------------
    /// Check return value from worker wait.
    /// \param waitRetVal Return value from WaitForMultipleObjects.
    /// \param numThreads Number of threads we waited on.
    //-----------------------------------------------------------------------------
    bool WaitSucceeded(DWORD waitRetVal, UINT numThreads);

    //-----------------------------------------------------------------------------
    /// Wait for workers to finish and close out their handles.
    //-----------------------------------------------------------------------------
    void WaitAndFetchResults(DX12FrameProfilerLayer* pFrameProfiler);

    //-----------------------------------------------------------------------------
    /// Convert profiler result data to string form.
    //-----------------------------------------------------------------------------
    void ProfilerResultToStr(ProfilerResult* pResult, gtASCIIString& profiledCommandsLinesStr);

    //-----------------------------------------------------------------------------
    /// Return GPU-time in text format, to be parsed by the Client and displayed as its own timeline.
    /// \return A line-delimited, ASCII-encoded, version of the GPU Trace data.
    //-----------------------------------------------------------------------------
    virtual std::string GetGPUTraceTXT();

    //-----------------------------------------------------------------------------
    /// Return the stringified function name based on input enum.
    /// \param inFunctionId An enumeration representing the function being invoked.
    //-----------------------------------------------------------------------------
    virtual const char* GetFunctionNameFromId(FuncId inFunctionId);

    //-----------------------------------------------------------------------------
    /// Retrieve the API group that an API call has been classified into.
    /// \param inAPIFuncId The FunctionId of an API call to retrieve the group for.
    /// \returns An API Type that a call has been classified as being part of.
    //-----------------------------------------------------------------------------
    eAPIType GetAPIGroupFromAPI(FuncId inAPIFuncId) const;

    //-----------------------------------------------------------------------------
    /// Call this to complete the call log. This will dump the logged data into a buffer for later use.
    /// Log a DX12 API call within the Trace Analyzer.
    /// \param inAPIEntry The APIEntry created for this API call
    //-----------------------------------------------------------------------------
    void LogAPICall(DX12APIEntry* inAPIEntry);

    //-----------------------------------------------------------------------------
    /// Gets called immediately after the real Present() is called
    /// \param SyncInterval The sync interval passed into the real Present() call.
    /// \param Flags The flags passed into the real Present() call.
    //-----------------------------------------------------------------------------
    virtual void OnPresent(UINT SyncInterval, UINT Flags);

protected:
    //-----------------------------------------------------------------------------
    /// Protected constructor, because this is a singleton.
    //-----------------------------------------------------------------------------
    DX12TraceAnalyzerLayer();

    //-----------------------------------------------------------------------------
    /// Create a new ThreadTraceData instance for use specifically with DX12 APIs.
    /// \returns A new ThreadTraceData instance to trace DX12 API calls.
    //-----------------------------------------------------------------------------
    virtual ThreadTraceData* CreateThreadTraceDataInstance();
};

#endif // DX12TRACEANALYZERLAYER_H
