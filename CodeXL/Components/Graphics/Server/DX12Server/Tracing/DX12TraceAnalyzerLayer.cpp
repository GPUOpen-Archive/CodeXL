//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12TraceAnalyzerLayer.cpp
/// \brief  The DX12 TraceAnalyzer layer that manages API and GPU trace collection and response.
//=============================================================================
#include "DX12TraceAnalyzerLayer.h"
#include "DX12LayerManager.h"
#include "Interception/DX12Interceptor.h"
#include "Objects/IDX12InstanceBase.h"
#include "Objects/DX12ObjectDatabaseProcessor.h"
#include "Objects/DX12WrappedObjectDatabase.h"
#include "Tracing/DX12APIEntry.h"
#include "Tracing/DX12ThreadTraceData.h"
#include "SymbolSerializers/DX12Serializers.h"
#include "Util/DX12Utilities.h"
#include "DX12Defines.h"
#include "Profiling/DX12FrameProfilerLayer.h"
#include "../Common/TypeToString.h"
#include "../Objects/CustomWrappers/Wrapped_ID3D12CommandQueueCustom.h"

//-----------------------------------------------------------------------------
/// SortByStartTime
//-----------------------------------------------------------------------------
bool SortByStartTime(ProfilerResult*& lhs, ProfilerResult*& rhs)
{
    return lhs->timestampResult.rawClocks.start <= rhs->timestampResult.rawClocks.start;
}

//-----------------------------------------------------------------------------
/// Protected constructor, because this is a singleton.
//-----------------------------------------------------------------------------
DX12TraceAnalyzerLayer::DX12TraceAnalyzerLayer()
    : MultithreadedTraceAnalyzerLayer()
{
    // The function name map is initialized when first traced.
    if (mFunctionIndexToNameString.empty())
    {
        // Included for cases where a non-profiled function is traced. Won't be visible within the client.
        mFunctionIndexToNameString[FuncId_UNDEFINED] = "UNDEFINED";

        mFunctionIndexToNameString[FuncId_D3D12GetDebugInterface] = "D3D12GetDebugInterface";
        mFunctionIndexToNameString[FuncId_D3D12CreateDevice] = "D3D12CreateDevice";
        mFunctionIndexToNameString[FuncId_D3D12SerializeRootSignature] = "D3D12SerializeRootSignature";
        mFunctionIndexToNameString[FuncId_D3D12CreateRootSignatureDeserializer] = "D3D12CreateRootSignatureDeserializer";

        mFunctionIndexToNameString[FuncId_IUnknown_QueryInterface] = "QueryInterface";
        mFunctionIndexToNameString[FuncId_IUnknown_AddRef] = "AddRef";
        mFunctionIndexToNameString[FuncId_IUnknown_Release] = "Release";

        mFunctionIndexToNameString[FuncId_ID3D12Object_GetPrivateData] = "GetPrivateData";
        mFunctionIndexToNameString[FuncId_ID3D12Object_SetPrivateData] = "SetPrivateData";
        mFunctionIndexToNameString[FuncId_ID3D12Object_SetPrivateDataInterface] = "SetPrivateDataInterface";
        mFunctionIndexToNameString[FuncId_ID3D12Object_SetName] = "SetName";

        mFunctionIndexToNameString[FuncId_ID3D12DeviceChild_GetDevice] = "GetDevice";

        mFunctionIndexToNameString[FuncId_ID3D12RootSignatureDeserializer_GetRootSignatureDesc] = "GetRootSignatureDesc";

        mFunctionIndexToNameString[FuncId_ID3D12Heap_GetDesc] = "GetDesc";

        mFunctionIndexToNameString[FuncId_ID3D12Resource_Map] = "Map";
        mFunctionIndexToNameString[FuncId_ID3D12Resource_Unmap] = "Unmap";
        mFunctionIndexToNameString[FuncId_ID3D12Resource_GetDesc] = "GetDesc";
        mFunctionIndexToNameString[FuncId_ID3D12Resource_GetGPUVirtualAddress] = "GetGPUVirtualAddress";
        mFunctionIndexToNameString[FuncId_ID3D12Resource_WriteToSubresource] = "WriteToSubresource";
        mFunctionIndexToNameString[FuncId_ID3D12Resource_ReadFromSubresource] = "ReadFromSubresource";
        mFunctionIndexToNameString[FuncId_ID3D12Resource_GetHeapProperties] = "GetHeapProperties";

        mFunctionIndexToNameString[FuncId_ID3D12CommandAllocator_Reset] = "Reset";

        mFunctionIndexToNameString[FuncId_ID3D12Fence_GetCompletedValue] = "GetCompletedValue";
        mFunctionIndexToNameString[FuncId_ID3D12Fence_SetEventOnCompletion] = "SetEventOnCompletion";
        mFunctionIndexToNameString[FuncId_ID3D12Fence_Signal] = "Signal";

        mFunctionIndexToNameString[FuncId_ID3D12PipelineState_GetCachedBlob] = "GetCachedBlob";

        mFunctionIndexToNameString[FuncId_ID3D12DescriptorHeap_GetDesc] = "GetDesc";
        mFunctionIndexToNameString[FuncId_ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart] = "GetCPUDescriptorHandleForHeapStart";
        mFunctionIndexToNameString[FuncId_ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart] = "GetGPUDescriptorHandleForHeapStart";

        mFunctionIndexToNameString[FuncId_ID3D12CommandList_GetType] = "GetType";

        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_Close] = "Close";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_Reset] = "Reset";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_ClearState] = "ClearState";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_DrawInstanced] = "DrawInstanced";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_DrawIndexedInstanced] = "DrawIndexedInstanced";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_Dispatch] = "Dispatch";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_CopyBufferRegion] = "CopyBufferRegion";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_CopyTextureRegion] = "CopyTextureRegion";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_CopyResource] = "CopyResource";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_CopyTiles] = "CopyTiles";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_ResolveSubresource] = "ResolveSubresource";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_IASetPrimitiveTopology] = "IASetPrimitiveTopology";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_RSSetViewports] = "RSSetViewports";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_RSSetScissorRects] = "RSSetScissorRects";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_OMSetBlendFactor] = "OMSetBlendFactor";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_OMSetStencilRef] = "OMSetStencilRef";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetPipelineState] = "SetPipelineState";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_ResourceBarrier] = "ResourceBarrier";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_ExecuteBundle] = "ExecuteBundle";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetDescriptorHeaps] = "SetDescriptorHeaps";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetComputeRootSignature] = "SetComputeRootSignature";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetGraphicsRootSignature] = "SetGraphicsRootSignature";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetComputeRootDescriptorTable] = "SetComputeRootDescriptorTable";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetGraphicsRootDescriptorTable] = "SetGraphicsRootDescriptorTable";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetComputeRoot32BitConstant] = "SetComputeRoot32BitConstant";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstant] = "SetGraphicsRoot32BitConstant";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetComputeRoot32BitConstants] = "SetComputeRoot32BitConstants";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstants] = "SetGraphicsRoot32BitConstants";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetComputeRootConstantBufferView] = "SetComputeRootConstantBufferView";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetGraphicsRootConstantBufferView] = "SetGraphicsRootConstantBufferView";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetComputeRootShaderResourceView] = "SetComputeRootShaderResourceView";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetGraphicsRootShaderResourceView] = "SetGraphicsRootShaderResourceView";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetComputeRootUnorderedAccessView] = "SetComputeRootUnorderedAccessView";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetGraphicsRootUnorderedAccessView] = "SetGraphicsRootUnorderedAccessView";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_IASetIndexBuffer] = "IASetIndexBuffer";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_IASetVertexBuffers] = "IASetVertexBuffers";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SOSetTargets] = "SOSetTargets";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_OMSetRenderTargets] = "OMSetRenderTargets";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_ClearDepthStencilView] = "ClearDepthStencilView";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_ClearRenderTargetView] = "ClearRenderTargetView";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewUint] = "ClearUnorderedAccessViewUint";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewFloat] = "ClearUnorderedAccessViewFloat";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_DiscardResource] = "DiscardResource";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_BeginQuery] = "BeginQuery";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_EndQuery] = "EndQuery";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_ResolveQueryData] = "ResolveQueryData";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetPredication] = "SetPredication";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_SetMarker] = "SetMarker";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_BeginEvent] = "BeginEvent";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_EndEvent] = "EndEvent";
        mFunctionIndexToNameString[FuncId_ID3D12GraphicsCommandList_ExecuteIndirect] = "ExecuteIndirect";

        mFunctionIndexToNameString[FuncId_ID3D12CommandQueue_UpdateTileMappings] = "UpdateTileMappings";
        mFunctionIndexToNameString[FuncId_ID3D12CommandQueue_CopyTileMappings] = "CopyTileMappings";
        mFunctionIndexToNameString[FuncId_ID3D12CommandQueue_ExecuteCommandLists] = "ExecuteCommandLists";
        mFunctionIndexToNameString[FuncId_ID3D12CommandQueue_SetMarker] = "SetMarker";
        mFunctionIndexToNameString[FuncId_ID3D12CommandQueue_BeginEvent] = "BeginEvent";
        mFunctionIndexToNameString[FuncId_ID3D12CommandQueue_EndEvent] = "EndEvent";
        mFunctionIndexToNameString[FuncId_ID3D12CommandQueue_Signal] = "Signal";
        mFunctionIndexToNameString[FuncId_ID3D12CommandQueue_Wait] = "Wait";
        mFunctionIndexToNameString[FuncId_ID3D12CommandQueue_GetTimestampFrequency] = "GetTimestampFrequency";
        mFunctionIndexToNameString[FuncId_ID3D12CommandQueue_GetClockCalibration] = "GetClockCalibration";
        mFunctionIndexToNameString[FuncId_ID3D12CommandQueue_GetDesc] = "GetDesc";

        mFunctionIndexToNameString[FuncId_ID3D12Device_GetNodeCount] = "GetNodeCount";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateCommandQueue] = "CreateCommandQueue";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateCommandAllocator] = "CreateCommandAllocator";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateGraphicsPipelineState] = "CreateGraphicsPipelineState";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateComputePipelineState] = "CreateComputePipelineState";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateCommandList] = "CreateCommandList";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CheckFeatureSupport] = "CheckFeatureSupport";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateDescriptorHeap] = "CreateDescriptorHeap";
        mFunctionIndexToNameString[FuncId_ID3D12Device_GetDescriptorHandleIncrementSize] = "GetDescriptorHandleIncrementSize";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateRootSignature] = "CreateRootSignature";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateConstantBufferView] = "CreateConstantBufferView";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateShaderResourceView] = "CreateShaderResourceView";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateUnorderedAccessView] = "CreateUnorderedAccessView";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateRenderTargetView] = "CreateRenderTargetView";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateDepthStencilView] = "CreateDepthStencilView";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateSampler] = "CreateSampler";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CopyDescriptors] = "CopyDescriptors";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CopyDescriptorsSimple] = "CopyDescriptorsSimple";
        mFunctionIndexToNameString[FuncId_ID3D12Device_GetResourceAllocationInfo] = "GetResourceAllocationInfo";
        mFunctionIndexToNameString[FuncId_ID3D12Device_GetCustomHeapProperties] = "GetCustomHeapProperties";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateCommittedResource] = "CreateCommittedResource";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateHeap] = "CreateHeap";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreatePlacedResource] = "CreatePlacedResource";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateReservedResource] = "CreateReservedResource";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateSharedHandle] = "CreateSharedHandle";
        mFunctionIndexToNameString[FuncId_ID3D12Device_OpenSharedHandle] = "OpenSharedHandle";
        mFunctionIndexToNameString[FuncId_ID3D12Device_OpenSharedHandleByName] = "OpenSharedHandleByName";
        mFunctionIndexToNameString[FuncId_ID3D12Device_MakeResident] = "MakeResident";
        mFunctionIndexToNameString[FuncId_ID3D12Device_Evict] = "Evict";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateFence] = "CreateFence";
        mFunctionIndexToNameString[FuncId_ID3D12Device_GetDeviceRemovedReason] = "GetDeviceRemovedReason";
        mFunctionIndexToNameString[FuncId_ID3D12Device_GetCopyableFootprints] = "GetCopyableFootprints";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateQueryHeap] = "CreateQueryHeap";
        mFunctionIndexToNameString[FuncId_ID3D12Device_SetStablePowerState] = "SetStablePowerState";
        mFunctionIndexToNameString[FuncId_ID3D12Device_CreateCommandSignature] = "CreateCommandSignature";
        mFunctionIndexToNameString[FuncId_ID3D12Device_GetResourceTiling] = "GetResourceTiling";
        mFunctionIndexToNameString[FuncId_ID3D12Device_GetAdapterLuid] = "GetAdapterLuid";

        mFunctionIndexToNameString[FuncId_IDXGISwapChain_Present] = "Present";
    }
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the parent LayerManager used by this tool.
/// \returns A pointer to the parent LayerManager used by this tool.
//-----------------------------------------------------------------------------
ModernAPILayerManager* DX12TraceAnalyzerLayer::GetParentLayerManager()
{
    return DX12LayerManager::Instance();
}

//-----------------------------------------------------------------------------
/// Provides a chance to initialize states before a GPU trace is performed.
//-----------------------------------------------------------------------------
void DX12TraceAnalyzerLayer::BeforeGPUTrace()
{
    ModernAPIFrameProfilerLayer* frameProfiler = GetParentLayerManager()->GetFrameProfilerLayer();
    frameProfiler->ResetSampleIdCounter();
    frameProfiler->ClearProfilingResults();

    MultithreadedTraceAnalyzerLayer::BeforeGPUTrace();
}

//-----------------------------------------------------------------------------
/// Provides a chance to initialize states before a GPU trace is performed.
//-----------------------------------------------------------------------------
void DX12TraceAnalyzerLayer::AfterGPUTrace()
{
    MultithreadedTraceAnalyzerLayer::AfterGPUTrace();
}

//-----------------------------------------------------------------------------
/// Fill a vector with all available queues.
/// \param pFrameProfiler Pointer to the frame profiler.
/// \param customQueues Vector to be filled in with our queues.
//-----------------------------------------------------------------------------
void DX12TraceAnalyzerLayer::GetAvailableQueues(
    DX12FrameProfilerLayer*                         pFrameProfiler,
    std::vector<Wrapped_ID3D12CommandQueueCustom*>& customQueues)
{
    CommandQueueToCommandListMap& queues = pFrameProfiler->GetQueueToCommandListMap();

    for (CommandQueueToCommandListMap::iterator it = queues.begin(); it != queues.end(); ++it)
    {
        customQueues.push_back(static_cast<Wrapped_ID3D12CommandQueueCustom*>(it->first));
    }
}

//-----------------------------------------------------------------------------
/// Check return value from worker wait.
/// \param waitRetVal Return value from WaitForMultipleObjects.
/// \param numThreads Number of threads we waited on.
//-----------------------------------------------------------------------------
bool DX12TraceAnalyzerLayer::WaitSucceeded(DWORD waitRetVal, UINT numThreads)
{
    return (waitRetVal >= WAIT_OBJECT_0) && (waitRetVal <= (WAIT_OBJECT_0 + numThreads - 1));
}

//-----------------------------------------------------------------------------
/// Wait for workers to finish and get their results.
/// \param pFrameProfiler Pointer to the frame profiler.
//-----------------------------------------------------------------------------
void DX12TraceAnalyzerLayer::WaitAndFetchResults(DX12FrameProfilerLayer* pFrameProfiler)
{
    // Gather all known queues
    std::vector<Wrapped_ID3D12CommandQueueCustom*> queues;
    GetAvailableQueues(pFrameProfiler, queues);

    if (queues.size() > 0)
    {
        std::vector<HANDLE> computeResultThreads;
        std::vector<HANDLE> directResultThreads;

        for (UINT i = 0; i < queues.size(); i++)
        {
            for (UINT j = 0; j < queues[i]->WorkerThreadCount(); j++)
            {
                DX12WorkerInfo* pWorkerInfo = queues[i]->GetWorkerInfo(j);

                if (pWorkerInfo != nullptr)
                {
                    if (pWorkerInfo->m_inputs.bContainsComputeQueueWork)
                    {
                        computeResultThreads.push_back(queues[i]->GetThreadHandle(j));
                    }
                    else
                    {
                        directResultThreads.push_back(queues[i]->GetThreadHandle(j));
                    }
                }
            }
        }

        const bool computeQueueResults = computeResultThreads.size() > 0;
        const bool directQueueResults = directResultThreads.size() > 0;

        // Gather all worker handles
        if (computeQueueResults || directQueueResults)
        {
            // Wait for workers with compute-queue results
            if (computeQueueResults)
            {
                const UINT numComputeWorkers = (UINT)computeResultThreads.size();
                DWORD retVal = WaitForMultipleObjects(numComputeWorkers, &computeResultThreads[0], TRUE, COMPUTE_QUEUE_RESULTS_TIMEOUT);

                if (WaitSucceeded(retVal, numComputeWorkers) == false)
                {
                    Log(logWARNING, "Detected failure condition when waiting for worker threads.\n");
                }
            }

            // Wait for other workers
            if (directQueueResults)
            {
                const UINT numDirectWorkers = (UINT)directResultThreads.size();
                DWORD retVal = WaitForMultipleObjects(numDirectWorkers, &directResultThreads[0], TRUE, DIRECT_QUEUE_RESULTS_TIMEOUT);

                if (WaitSucceeded(retVal, numDirectWorkers) == false)
                {
                    Log(logWARNING, "Detected failure condition when waiting for worker threads.\n");
                }
            }

            // Get results from thread
            for (UINT i = 0; i < queues.size(); i++)
            {
                for (UINT j = 0; j < queues[i]->WorkerThreadCount(); j++)
                {
                    DX12WorkerInfo* pWorkerInfo = queues[i]->GetWorkerInfo(j);

                    pFrameProfiler->VerifyAlignAndStoreResults(
                        queues[i],
                        pWorkerInfo->m_outputs.results,
                        &pWorkerInfo->m_inputs.timestampPair,
                        pWorkerInfo->m_threadInfo.workerThreadCountID,
                        mFramestartTime);
                }
            }

            // Close worker handles
            for (UINT i = 0; i < queues.size(); i++)
            {
                queues[i]->EndCollection();
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Convert profiler result data to string form.
/// \param pResult The profilerResult to convert.
/// \param profiledCommandsLinesStr The output string.
//-----------------------------------------------------------------------------
void DX12TraceAnalyzerLayer::ProfilerResultToStr(
    ProfilerResult* pResult,
    gtASCIIString&  profiledCommandsLinesStr)
{
    DX12APIEntry* pResultEntry = DX12FrameProfilerLayer::Instance()->FindInvocationBySampleId(pResult->measurementInfo.idInfo.mSampleId);

    gtASCIIString returnValueString;
    pResultEntry->PrintReturnValue(pResultEntry->mReturnValue, pResultEntry->mReturnValueFlags, returnValueString);

    double startMillisecond = pResult->timestampResult.alignedMillisecondTimestamps.start;
    double endMillisecond = pResult->timestampResult.alignedMillisecondTimestamps.end;

    // DX12 Response line format:
    // CommandQueuePtr CommandListType CommandListPtr APIType FuncId ID3D12InterfaceName_FuncName(Args) = ReturnValue StartTime EndTime SampleId

    profiledCommandsLinesStr += "0x";
    profiledCommandsLinesStr += UINT64ToHexString((UINT64)pResult->measurementInfo.idInfo.pCmdQueue);
    profiledCommandsLinesStr += " ";

    profiledCommandsLinesStr += IntToString(pResult->measurementInfo.idInfo.mCmdListType);
    profiledCommandsLinesStr += " ";

    profiledCommandsLinesStr += "0x";
    profiledCommandsLinesStr += UINT64ToHexString((UINT64)pResultEntry->mWrapperInterface);
    profiledCommandsLinesStr += " ";

    profiledCommandsLinesStr += IntToString(DX12TraceAnalyzerLayer::Instance()->GetAPIGroupFromAPI(pResultEntry->mFunctionId));
    profiledCommandsLinesStr += " ";

    profiledCommandsLinesStr += IntToString(pResultEntry->mFunctionId);
    profiledCommandsLinesStr += " ";

    profiledCommandsLinesStr += "ID3D12GraphicsCommandList_";
    profiledCommandsLinesStr += GetFunctionNameFromId(pResultEntry->mFunctionId);

    profiledCommandsLinesStr += "(";
    profiledCommandsLinesStr += pResultEntry->GetParameterString();
    profiledCommandsLinesStr += ") = ";

    profiledCommandsLinesStr += returnValueString.asCharArray();

    profiledCommandsLinesStr += " ";
    profiledCommandsLinesStr += DoubleToString(startMillisecond);

    profiledCommandsLinesStr += " ";
    profiledCommandsLinesStr += DoubleToString(endMillisecond);

    profiledCommandsLinesStr += " ";
    profiledCommandsLinesStr += UINT64ToString(pResultEntry->mSampleId);

    profiledCommandsLinesStr += "\n";
}

//-----------------------------------------------------------------------------
/// Return GPU-time in text format, to be parsed by the Client and displayed as its own timeline.
/// \return A line-delimited, ASCII-encoded, version of the GPU Trace data.
//-----------------------------------------------------------------------------
std::string DX12TraceAnalyzerLayer::GetGPUTraceTXT()
{
    gtASCIIString appendString = "";

    DX12FrameProfilerLayer* pFrameProfiler = DX12FrameProfilerLayer::Instance();

    WaitAndFetchResults(pFrameProfiler);

    // During QueueSubmit we stored ProfilerResults in mEntriesWithProfilingResults. Form a response using it here.
    ProfilerResultsMap& profiledCmdListResultsMap = pFrameProfiler->GetCmdListProfilerResultsMap();

    // Gather all profiler results
    if (!profiledCmdListResultsMap.empty())
    {
        std::vector<ProfilerResult*> flatResults;

        for (ProfilerResultsMap::iterator profIt = profiledCmdListResultsMap.begin(); profIt != profiledCmdListResultsMap.end(); ++profIt)
        {
            QueueWrapperToProfilingResultsMap& resultsPerThread = profIt->second;

            for (QueueWrapperToProfilingResultsMap::iterator queuesIt = resultsPerThread.begin();
                 queuesIt != resultsPerThread.end();
                 ++queuesIt)
            {
                const SampleIdToProfilerResultMap* pResults = queuesIt->second;

                for (SampleIdToProfilerResultMap::const_iterator sampleIdIt = pResults->begin();
                     sampleIdIt != pResults->end();
                     ++sampleIdIt)
                {
                    ProfilerResult* pResult = sampleIdIt->second;
                    pResult->measurementInfo.idInfo.pCmdQueue = queuesIt->first;

                    flatResults.push_back(pResult);
                }
            }
        }

        sort(flatResults.begin(), flatResults.end(), SortByStartTime);

        // We'll need to insert the GPU Trace section header before the response data, even if there aren't any results.
        appendString += "//==GPU Trace==";
        appendString += "\n";

        appendString += "//API=";
        appendString += GetAPIString();
        appendString += "\n";

        appendString += "//CommandListEventCount=";
        appendString += IntToString((INT)flatResults.size());
        appendString += "\n";

        for (UINT i = 0; i < flatResults.size(); i++)
        {
            ProfilerResultToStr(flatResults[i], appendString);
        }
    }
    else
    {
        appendString += "NODATA";
    }

    return appendString.asCharArray();
}

//-----------------------------------------------------------------------------
/// Create a new ThreadTraceData instance for use specifically with DX12 APIs.
/// \returns A new ThreadTraceData instance to trace DX12 API calls.
//-----------------------------------------------------------------------------
ThreadTraceData* DX12TraceAnalyzerLayer::CreateThreadTraceDataInstance()
{
    return new DX12ThreadTraceData();
}

//-----------------------------------------------------------------------------
/// Log a DX12 API call within the Trace Analyzer.
/// \param inAPIEntry The APIEntry created for this API call
//-----------------------------------------------------------------------------
void DX12TraceAnalyzerLayer::LogAPICall(DX12APIEntry* inAPIEntry)
{
    ScopeLock logAPICallLock(&mTraceMutex);

    ThreadTraceData* currentThreadData = FindOrCreateThreadData(inAPIEntry->mThreadId);

    if (currentThreadData->m_startTime.QuadPart == s_DummyTimestampValue)
    {
        const char* functionNameString = GetFunctionNameFromId(inAPIEntry->mFunctionId);
        Log(logERROR, "There was a problem setting the start time for API call '%s' on Thread with Id '%d'.\n", functionNameString, inAPIEntry->mThreadId);
    }

    // Create a new entry for this traced API call, and add it to the list for the current thread.
    currentThreadData->AddAPIEntry(currentThreadData->m_startTime, inAPIEntry);
}

//-----------------------------------------------------------------------------
/// Return the stringified function name base don the input enum.
/// \param inFunctionId An enumeration representing the function being invoked.
/// \returns A string containing the function name.
//-----------------------------------------------------------------------------
const char* DX12TraceAnalyzerLayer::GetFunctionNameFromId(FuncId inFunctionId)
{
    FuncIdToNamestringMap::const_iterator funcIter = mFunctionIndexToNameString.find(inFunctionId);
    FuncIdToNamestringMap::const_iterator endIter = mFunctionIndexToNameString.end();

    if (funcIter != endIter)
    {
        return (funcIter->second).c_str();
    }

    Log(logERROR, "Failed to stringify FuncId '%d'\n", inFunctionId);

    // Throw up a warning for development in case this list needs to get updated.
    PsAssert(funcIter != endIter);
    return "UNDEFINED";
}

//-----------------------------------------------------------------------------
/// Gets called immediately after the real Present() is called
/// \param SyncInterval The sync interval passed into the real Present() call.
/// \param Flags The flags passed into the real Present() call.
//-----------------------------------------------------------------------------
void DX12TraceAnalyzerLayer::OnPresent(UINT SyncInterval, UINT Flags)
{
    INT64 result = FUNCTION_RETURNS_VOID;
    DX12Interceptor* interceptor = DX12LayerManager::Instance()->GetInterceptor();

    if (interceptor->ShouldCollectTrace())
    {
        char argumentsBuffer[OS_MAX_PATH] = {};
        sprintf_s(argumentsBuffer, OS_MAX_PATH, "%d, 0x%x", SyncInterval, Flags);

        // precall
        BeforeAPICall();

        // postcall
        DWORD threadId = osGetCurrentThreadId();
        DX12APIEntry* pNewEntry = new DX12APIEntry(threadId, nullptr, FuncId_IDXGISwapChain_Present, argumentsBuffer, result, RETURN_VALUE_DECIMAL);
        LogAPICall(pNewEntry);
    }
}

//-----------------------------------------------------------------------------
/// Retrieve the API Type that an API call has been classified into.
/// \param inAPIFuncId The FunctionId of an API call to retrieve the group for.
/// \returns An API Type that a call has been classified as being part of.
//-----------------------------------------------------------------------------
eAPIType DX12TraceAnalyzerLayer::GetAPIGroupFromAPI(FuncId inAPIFuncId) const
{
    eAPIType apiType = kAPIType_Unknown;

    switch (inAPIFuncId)
    {
        case FuncId_ID3D12GraphicsCommandList_SetDescriptorHeaps:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRootSignature:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRootSignature:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRootDescriptorTable:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRootDescriptorTable:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRoot32BitConstant:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstant:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRoot32BitConstants:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstants:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRootConstantBufferView:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRootConstantBufferView:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRootShaderResourceView:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRootShaderResourceView:
        case FuncId_ID3D12GraphicsCommandList_SetComputeRootUnorderedAccessView:
        case FuncId_ID3D12GraphicsCommandList_SetGraphicsRootUnorderedAccessView:
            apiType = kAPIType_BindingCommand;
            break;

        case FuncId_ID3D12GraphicsCommandList_ClearState:
        case FuncId_ID3D12GraphicsCommandList_ClearDepthStencilView:
        case FuncId_ID3D12GraphicsCommandList_ClearRenderTargetView:
        case FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewUint:
        case FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewFloat:
            apiType = kAPIType_ClearCommand;
            break;

        case FuncId_ID3D12GraphicsCommandList_Close:
        case FuncId_ID3D12GraphicsCommandList_Reset:
        case FuncId_ID3D12GraphicsCommandList_ResolveSubresource:
        case FuncId_ID3D12GraphicsCommandList_SetPipelineState:
        case FuncId_ID3D12GraphicsCommandList_ExecuteBundle:
        case FuncId_ID3D12GraphicsCommandList_DiscardResource:
        case FuncId_ID3D12GraphicsCommandList_BeginQuery:
        case FuncId_ID3D12GraphicsCommandList_EndQuery:
        case FuncId_ID3D12GraphicsCommandList_ResolveQueryData:
        case FuncId_ID3D12GraphicsCommandList_SetPredication:
        case FuncId_ID3D12GraphicsCommandList_ExecuteIndirect:
            apiType = kAPIType_Command;
            break;

        case FuncId_ID3D12CommandQueue_CopyTileMappings:
        case FuncId_ID3D12Device_CopyDescriptors:
        case FuncId_ID3D12Device_CopyDescriptorsSimple:
            apiType = kAPIType_Copy;
            break;

        case FuncId_ID3D12Device_CreateCommandQueue:
        case FuncId_ID3D12Device_CreateCommandAllocator:
        case FuncId_ID3D12Device_CreateGraphicsPipelineState:
        case FuncId_ID3D12Device_CreateComputePipelineState:
        case FuncId_ID3D12Device_CreateCommandList:
        case FuncId_ID3D12Device_CreateDescriptorHeap:
        case FuncId_ID3D12Device_CreateRootSignature:
        case FuncId_ID3D12Device_CreateConstantBufferView:
        case FuncId_ID3D12Device_CreateShaderResourceView:
        case FuncId_ID3D12Device_CreateUnorderedAccessView:
        case FuncId_ID3D12Device_CreateRenderTargetView:
        case FuncId_ID3D12Device_CreateDepthStencilView:
        case FuncId_ID3D12Device_CreateSampler:
        case FuncId_ID3D12Device_CreateHeap:
        case FuncId_ID3D12Device_CreatePlacedResource:
        case FuncId_ID3D12Device_CreateReservedResource:
        case FuncId_ID3D12Device_CreateSharedHandle:
        case FuncId_ID3D12Device_CreateFence:
        case FuncId_ID3D12Device_CreateQueryHeap:
        case FuncId_ID3D12Device_CreateCommandSignature:
            apiType = kAPIType_Create;
            break;

        case FuncId_ID3D12Object_GetPrivateData:
        case FuncId_ID3D12Object_SetPrivateData:
        case FuncId_ID3D12Object_SetPrivateDataInterface:
        case FuncId_ID3D12Object_SetName:
        case FuncId_ID3D12GraphicsCommandList_SetMarker:
        case FuncId_ID3D12GraphicsCommandList_BeginEvent:
        case FuncId_ID3D12GraphicsCommandList_EndEvent:
        case FuncId_ID3D12CommandQueue_BeginEvent:
        case FuncId_ID3D12CommandQueue_EndEvent:
        case FuncId_ID3D12CommandQueue_SetMarker:
            apiType = kAPIType_Debug;
            break;

        case FuncId_ID3D12GraphicsCommandList_DrawInstanced:
        case FuncId_ID3D12GraphicsCommandList_DrawIndexedInstanced:
        case FuncId_ID3D12GraphicsCommandList_Dispatch:
            apiType = kAPIType_DrawCommand;
            break;

        case FuncId_IUnknown_QueryInterface:
        case FuncId_IUnknown_AddRef:
        case FuncId_IUnknown_Release:
        case FuncId_ID3D12DeviceChild_GetDevice:
        case FuncId_ID3D12Heap_GetDesc:
        case FuncId_ID3D12RootSignatureDeserializer_GetRootSignatureDesc:
        case FuncId_ID3D12Resource_GetHeapProperties:
        case FuncId_ID3D12CommandAllocator_Reset:
        case FuncId_ID3D12PipelineState_GetCachedBlob:
        case FuncId_ID3D12DescriptorHeap_GetDesc:
        case FuncId_ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart:
        case FuncId_ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart:
        case FuncId_ID3D12CommandList_GetType:
        case FuncId_ID3D12CommandQueue_ExecuteCommandLists:
        case FuncId_ID3D12CommandQueue_GetTimestampFrequency:
        case FuncId_ID3D12CommandQueue_GetClockCalibration:
        case FuncId_ID3D12CommandQueue_GetDesc:
        case FuncId_ID3D12Device_GetNodeCount:
        case FuncId_ID3D12Device_CheckFeatureSupport:
        case FuncId_ID3D12Device_GetDescriptorHandleIncrementSize:
        case FuncId_ID3D12Device_GetResourceAllocationInfo:
        case FuncId_ID3D12Device_GetCustomHeapProperties:
        case FuncId_ID3D12Device_OpenSharedHandle:
        case FuncId_ID3D12Device_OpenSharedHandleByName:
        case FuncId_ID3D12Device_GetDeviceRemovedReason:
        case FuncId_ID3D12Device_GetCopyableFootprints:
        case FuncId_ID3D12Device_SetStablePowerState:
        case FuncId_ID3D12Device_GetResourceTiling:
        case FuncId_ID3D12Device_GetAdapterLuid:
            apiType = kAPIType_General;
            break;

        case FuncId_ID3D12CommandQueue_UpdateTileMappings:
        case FuncId_ID3D12Device_MakeResident:
        case FuncId_ID3D12Device_Evict:
            apiType = kAPIType_Paging;
            break;

        case FuncId_ID3D12Resource_Map:
        case FuncId_ID3D12Resource_Unmap:
        case FuncId_ID3D12Resource_GetDesc:
        case FuncId_ID3D12Resource_GetGPUVirtualAddress:
        case FuncId_ID3D12Resource_ReadFromSubresource:
            apiType = kAPIType_Resource;
            break;

        case FuncId_ID3D12GraphicsCommandList_IASetPrimitiveTopology:
        case FuncId_ID3D12GraphicsCommandList_RSSetViewports:
        case FuncId_ID3D12GraphicsCommandList_RSSetScissorRects:
        case FuncId_ID3D12GraphicsCommandList_OMSetBlendFactor:
        case FuncId_ID3D12GraphicsCommandList_OMSetStencilRef:
        case FuncId_ID3D12GraphicsCommandList_IASetIndexBuffer:
        case FuncId_ID3D12GraphicsCommandList_IASetVertexBuffers:
        case FuncId_ID3D12GraphicsCommandList_SOSetTargets:
        case FuncId_ID3D12GraphicsCommandList_OMSetRenderTargets:
            apiType = kAPIType_StageCommand;
            break;

        case FuncId_ID3D12Fence_GetCompletedValue:
        case FuncId_ID3D12Fence_SetEventOnCompletion:
        case FuncId_ID3D12Fence_Signal:
        case FuncId_ID3D12GraphicsCommandList_ResourceBarrier:
        case FuncId_ID3D12CommandQueue_Signal:
        case FuncId_ID3D12CommandQueue_Wait:
            apiType = kAPIType_Synchronization;
            break;

        default:
            apiType = kAPIType_Unknown;
    }

    return apiType;
}