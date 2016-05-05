//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktTraceAnalyzerLayer.h
/// \brief  Header file for Vulkan trace analyzer layer.
///         The main class (VtkTraceAnalyzerLayer) performs essential work here,
///         such as sending API/GPU trace information to client and managing
///         profiler results.
//==============================================================================

#ifndef __VKT_TRACE_ANALYZER_LAYER_H__
#define __VKT_TRACE_ANALYZER_LAYER_H__

#include "../../../Common/Tracing/MultithreadedTraceAnalyzerLayer.h"
#include "../../../Common/TSingleton.h"
#include "../Util/vktUtil.h"
#include "vktAPIEntry.h"

struct ProfilerResult;
class VktFrameProfilerLayer;
class VktWrappedQueue;

//-----------------------------------------------------------------------------
/// Collects API/GPU Trace in a multi-threaded manner by mapping each submission
/// thread to its own buffer that it can dump logged calls to.
//-----------------------------------------------------------------------------
class VktTraceAnalyzerLayer : public MultithreadedTraceAnalyzerLayer, public TSingleton< VktTraceAnalyzerLayer >
{
    /// This is a singleton class
    friend TSingleton < VktTraceAnalyzerLayer >;

public:
    virtual ~VktTraceAnalyzerLayer() {}

    /// Return the API name
    virtual char const* GetAPIString() { return "Vulkan"; }

    virtual std::string GetGPUTraceTXT();
    virtual ThreadTraceData* CreateThreadTraceDataInstance();
    virtual const char* GetFunctionNameFromId(FuncId inFunctionId);
    virtual void OnPresent(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
    virtual ModernAPILayerManager* GetParentLayerManager();

    void LogAPICall(VktAPIEntry* pApiEntry);

    eAPIType GetAPIGroupFromAPI(FuncId inAPIFuncId);

    bool WaitSucceeded(DWORD waitRetVal, UINT numThreads);
    void WaitAndFetchResults(VktFrameProfilerLayer* pFrameProfiler);
    void GetAvailableQueues(std::vector<VktWrappedQueue*>& wrappedQueues);

protected:
    VktTraceAnalyzerLayer();

    virtual void BeforeGPUTrace();
    virtual void AfterGPUTrace();
};

#endif // __VKT_TRACE_ANALYZER_LAYER_H__