//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktWrappedQueue.cpp
/// \brief  A wrapper for queues.
//=============================================================================

#include "vktWrappedQueue.h"
#include "vktWrappedCmdBuf.h"
#include "../../vktInterceptManager.h"
#include "../../vktLayerManager.h"
#include "../../vktDefines.h"
#include "../../FrameDebugger/vktFrameDebuggerLayer.h"

//-----------------------------------------------------------------------------
/// Profiler results collection worker function.
/// \param lpParam A void pointer to the incoming VktWorkerInfo argument.
/// \returns Always 0.
//-----------------------------------------------------------------------------
DWORD WINAPI ThreadFunc(LPVOID lpParam)
{
    VktWorkerInfo* pWorkerInfo = (VktWorkerInfo*)lpParam;

    pWorkerInfo->m_threadInfo.workerThreadID = osGetCurrentThreadId();

    VkDevice device = pWorkerInfo->m_inputs.pQueue->ParentDevice();

    VkQueue queue = pWorkerInfo->m_inputs.pQueue->AppHandle();
    device_dispatch_table(device)->QueueWaitIdle(queue);

    //device_dispatch_table(device)->WaitForFences(device, 1, &pWorkerInfo->m_inputs.fenceToWaitOn, true, FENCE_TIMEOUT_TIME);

    if (pWorkerInfo->m_inputs.timestampPair.mQueueCanBeTimestamped)
    {
        for (UINT i = 0; i < pWorkerInfo->m_inputs.commandListsWithProfiledCalls.size(); i++)
        {
            VktWrappedCmdBuf* pWrappedCmdBuf = pWorkerInfo->m_inputs.commandListsWithProfiledCalls[i];

            ProfilerResultCode profResult = pWrappedCmdBuf->GetCmdBufResultsMT(pWorkerInfo->m_inputs.executionID, pWorkerInfo->m_outputs.results);

            if (profResult != PROFILER_SUCCESS)
            {
                const char* profilerErrorCode = VktCmdBufProfiler::PrintProfilerResult(profResult);

                // Report that a problem occurred in retrieving full profiler results.
                Log(logERROR, "Failed to retrieve full profiler results: CmdBuf 0x%p, Queue 0x%p, ErrorCode %s\n",
                    pWorkerInfo->m_inputs.commandListsWithProfiledCalls[i], pWorkerInfo->m_inputs.pQueue, profilerErrorCode);
            }
        }
    }

    if (pWorkerInfo->m_inputs.internalFence)
    {
        device_dispatch_table(device)->DestroyFence(device, pWorkerInfo->m_inputs.fenceToWaitOn, nullptr);
    }

    // This will only be set to true if the GPU results have come back in time.
    pWorkerInfo->m_outputs.bResultsGathered = true;

    return 0;
}

//-----------------------------------------------------------------------------
/// Statically create a VktWrappedQueue
/// \param createInfo Creation struct used to construct this wrapper.
//-----------------------------------------------------------------------------
VktWrappedQueue* VktWrappedQueue::Create(const WrappedQueueCreateInfo& createInfo)
{
    VktWrappedQueue* pOut = nullptr;

    if ((createInfo.device != VK_NULL_HANDLE) &&
        (createInfo.appQueue != VK_NULL_HANDLE) &&
        (createInfo.pInterceptMgr != VK_NULL_HANDLE))
    {
        pOut = new VktWrappedQueue(createInfo);
    }

    return pOut;
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param createInfo Creation struct used to construct this wrapper.
//-----------------------------------------------------------------------------
VktWrappedQueue::VktWrappedQueue(const WrappedQueueCreateInfo& createInfo) :
    m_executionID(-1)
{
    memcpy(&m_createInfo, &createInfo, sizeof(m_createInfo));
}

//-----------------------------------------------------------------------------
/// Fill a vector with all wrapped command buffers.
/// \param commandBufferCount The number of CommandBuffers to gather results for.
/// \param pCommandBuffers The array of CommandBuffers to gather results for.
/// \param cmdBufsWithProfiledCalls An array of CommandBuffers with profiled calls.
//-----------------------------------------------------------------------------
void VktWrappedQueue::GatherWrappedCommandBufs(
    UINT                            commandBufferCount,
    const VkCommandBuffer*          pCommandBuffers,
    std::vector<VktWrappedCmdBuf*>& cmdBufsWithProfiledCalls)
{
    for (UINT i = 0; i < commandBufferCount; i++)
    {
        if (pCommandBuffers[i] != nullptr)
        {
            VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(pCommandBuffers[i]);

            if (pWrappedCmdBuf != nullptr)
            {
                cmdBufsWithProfiledCalls.push_back(pWrappedCmdBuf);
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Spawn a worker thread to gather GPU profiler results.
/// \param pTimestampPair A pair of calibration timestamps used to align CPU and GPU timelines.
/// \param pQueue The Queue responsible for executed the profiled workload.
/// \param fenceToWaitOn The fence we want to wait on
/// \param internalFence True if the fence we're waiting on was created by VulkanServer
/// \param cmdBufsWithProfiledCalls The array of CommandBuffers being executed.
//-----------------------------------------------------------------------------
void VktWrappedQueue::SpawnWorker(
    CalibrationTimestampPair*            pTimestampPair,
    VktWrappedQueue*                     pQueue,
    VkFence                              fenceToWaitOn,
    bool                                 internalFence,
    const std::vector<VktWrappedCmdBuf*> cmdBufsWithProfiledCalls)
{
    const UINT profiledCmdBufCount = (UINT)cmdBufsWithProfiledCalls.size();

    if (profiledCmdBufCount > 0)
    {
        static UINT32 s_threadID = 1;

        VktWorkerInfo* pWorkerInfo = new VktWorkerInfo();

        pWorkerInfo->m_inputs.internalFence = internalFence;
        pWorkerInfo->m_inputs.fenceToWaitOn = fenceToWaitOn;
        pWorkerInfo->m_inputs.pQueue        = pQueue;
        pWorkerInfo->m_inputs.executionID   = m_executionID;

        pWorkerInfo->m_threadInfo.workerThreadCountID = s_threadID++;
        pWorkerInfo->m_threadInfo.parentThreadID = osGetCurrentThreadId();

        for (size_t index = 0; index < profiledCmdBufCount; index++)
        {
            pWorkerInfo->m_inputs.commandListsWithProfiledCalls.push_back(cmdBufsWithProfiledCalls[index]);
        }

        memcpy(&pWorkerInfo->m_inputs.timestampPair, pTimestampPair, sizeof(pWorkerInfo->m_inputs.timestampPair));

        // The push onto m_workerThreadInfo needs to be thread safe
        {
            ScopeLock lock(&m_workerThreadInfoMutex);

            m_workerThreadInfo.push_back(pWorkerInfo);

            DWORD threadId = 0;
            pWorkerInfo->m_threadInfo.threadHandle = CreateThread(nullptr, 0, ThreadFunc, pWorkerInfo, 0, &threadId);
        }
    }
}

//-----------------------------------------------------------------------------
/// Kill all info retained by this thread.
//-----------------------------------------------------------------------------
void VktWrappedQueue::EndCollection()
{
    ScopeLock lock(&m_workerThreadInfoMutex);

    for (UINT i = 0; i < m_workerThreadInfo.size(); i++)
    {
        // Delete profiler memory
        for (UINT j = 0; j < m_workerThreadInfo[i]->m_inputs.commandListsWithProfiledCalls.size(); j++)
        {
            VktWrappedCmdBuf* pCmdBuf = m_workerThreadInfo[i]->m_inputs.commandListsWithProfiledCalls[j];

            if (pCmdBuf != nullptr)
            {
                pCmdBuf->DestroyProfilers();
            }
        }

        m_workerThreadInfo[i]->m_outputs.results.clear();

        CloseHandle(m_workerThreadInfo[i]->m_threadInfo.threadHandle);
        SAFE_DELETE(m_workerThreadInfo[i]);
    }

    m_workerThreadInfo.clear();
}

//-----------------------------------------------------------------------------
/// Get the physical device's timestamp frequency.
//-----------------------------------------------------------------------------
double VktWrappedQueue::GetTimestampFrequency()
{
    VkPhysicalDeviceProperties deviceProps = {};

    instance_dispatch_table(m_createInfo.physicalDevice)->GetPhysicalDeviceProperties(m_createInfo.physicalDevice, &deviceProps);

    return 1000000000.0f / (double)deviceProps.limits.timestampPeriod;
}

//-----------------------------------------------------------------------------
/// Submit command buffers and gather results.
/// \param queue The queue issued work to.
/// \param submitCount The number of submits.
/// \param pSubmits The submit info structures.
/// \param fence The fence wrapping this submit.
//-----------------------------------------------------------------------------
VkResult VktWrappedQueue::QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
    m_executionID++;

    VkResult result = VK_INCOMPLETE;

    VktTraceAnalyzerLayer* pTraceAnalyzer = VktTraceAnalyzerLayer::Instance();
    VktFrameProfilerLayer* pFrameProfiler = VktFrameProfilerLayer::Instance();

    // Use this calibration timestamp structure to convert GPU events to the CPU timeline.
    CalibrationTimestampPair calibrationTimestamps = {};
    calibrationTimestamps.mQueueCanBeTimestamped = true;

    VkFence fenceToWaitOn = fence;
    bool usingInternalFence = false;

    std::vector<VktWrappedCmdBuf*> wrappedCmdBufs;

    for (UINT i = 0; i < submitCount; i++)
    {
        const VkSubmitInfo& currSubmit = pSubmits[i];

        GatherWrappedCommandBufs(currSubmit.commandBufferCount, currSubmit.pCommandBuffers, wrappedCmdBufs);

        for (UINT j = 0; j < currSubmit.commandBufferCount; j++)
        {
            VktWrappedCmdBuf* pWrappedCmdBuf = GetWrappedCmdBuf(currSubmit.pCommandBuffers[j]);

            if (pWrappedCmdBuf != nullptr)
            {
                pWrappedCmdBuf->SetProfilerExecutionId(m_executionID);
                pWrappedCmdBuf->IncrementSubmitCount();
            }
        }
    }

    // Surround the execution of CommandBuffers with timestamps so we can determine when the GPU work occurred in the CPU timeline.
    if (pTraceAnalyzer->ShouldCollectTrace() && pFrameProfiler->ShouldCollectGPUTime())
    {
        // Collect calibration timestamps in case we need to align GPU events against the CPU timeline.
        if (calibrationTimestamps.mQueueCanBeTimestamped)
        {
            pFrameProfiler->CollectCalibrationTimestamps(this, &calibrationTimestamps);
        }
        else
        {
            Log(logTRACE, "Did not collect calibration timestamps for Queue '0x%p'\n", this);
        }

        if (fenceToWaitOn == VK_NULL_HANDLE)
        {
            // Create internal fence
            VkFenceCreateInfo fenceCreateInfo = {};
            VkResult fenceResult = VK_INCOMPLETE;
            fenceResult = device_dispatch_table(queue)->CreateFence(m_createInfo.device, &fenceCreateInfo, nullptr, &fenceToWaitOn);
            VKT_ASSERT(fenceResult == VK_SUCCESS);
        }
    }

    // Invoke the real call to execute on the GPU
    result = QueueSubmit_ICD(queue, submitCount, pSubmits, fenceToWaitOn);

    if (pTraceAnalyzer->ShouldCollectTrace() && pFrameProfiler->ShouldCollectGPUTime())
    {
        // Collect the CPU and GPU frequency to convert timestamps.
        QueryPerformanceFrequency(&calibrationTimestamps.cpuFrequency);

#if GATHER_PROFILER_RESULTS_WITH_WORKERS
        SpawnWorker(&calibrationTimestamps, this, fenceToWaitOn, usingInternalFence, wrappedCmdBufs);
#else
        device_dispatch_table(queue)->QueueWaitIdle(queue);
        //device_dispatch_table(queue)->WaitForFences(m_createInfo.device, 1, &fenceToWaitOn, true, FENCE_TIMEOUT_TIME);
#endif

#if GATHER_PROFILER_RESULTS_WITH_WORKERS == 0

        if (calibrationTimestamps.mQueueCanBeTimestamped)
        {
            // Put all results into thread ID 0 bucket
            const UINT32 threadID = 0;
            std::vector<ProfilerResult> results;

            for (UINT i = 0; i < wrappedCmdBufs.size(); i++)
            {
                ProfilerResultCode getResultsResult = PROFILER_FAIL;
                getResultsResult = wrappedCmdBufs[i]->GetCmdBufResultsST(results);
                VKT_ASSERT(getResultsResult != PROFILER_FAIL);
            }

            pFrameProfiler->VerifyAlignAndStoreResults(this, results, &calibrationTimestamps, threadID, VktTraceAnalyzerLayer::Instance()->GetFrameStartTime());

            if (usingInternalFence)
            {
                device_dispatch_table(m_createInfo.device)->DestroyFence(m_createInfo.device, fenceToWaitOn, nullptr);
            }
        }
        else
        {
            Log(logTRACE, "Didn't collect calibration timestamps for Queue '0x%p'.\n", this);
        }
#endif
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Called at present-time.
/// \param queue The queue we're using to present.
/// \param pPresentInfo Information about this present.
//-----------------------------------------------------------------------------
VkResult VktWrappedQueue::QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
{
    VkResult result = QueuePresentKHR_ICD(queue, pPresentInfo);

    VktTraceAnalyzerLayer::Instance()->OnPresent(queue, pPresentInfo);

    QueueInfo queueInfo = {};
    queueInfo.physicalDevice = m_createInfo.physicalDevice;
    queueInfo.device         = m_createInfo.device;
    queueInfo.queue          = queue;
    VktFrameDebuggerLayer::Instance()->OnPresent(queueInfo);

    VktLayerManager::GetLayerManager()->EndFrame();

    // Get new requests sent to the server.
    GetPendingRequests();

    VktLayerManager::GetLayerManager()->BeginFrame();

    return result;
}

//-----------------------------------------------------------------------------
/// ICD entry points.
//-----------------------------------------------------------------------------

#pragma warning (push)
#pragma warning (disable : 4477)

// This prevents VS2015 from complaining about imperfect "%" formatting when printing Vulkan objects.
// This only applies to the 32-bit version of VulkanServer.
#ifndef X64
#pragma warning (disable : 4313)
#endif

VkResult VktWrappedQueue::QueueSubmit_ICD(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
    const FuncId funcId = FuncId_vkQueueSubmit;

    VkResult result = VK_INCOMPLETE;

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %u, 0x%p, 0x%p",
            queue,
            submitCount,
            PrintArrayWithFormatter(submitCount, pSubmits, "0x%p").c_str(),
            fence);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(queue)->QueueSubmit(queue, submitCount, pSubmits, fence);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(queue)->QueueSubmit(queue, submitCount, pSubmits, fence);
    }

    return result;
}

VkResult VktWrappedQueue::QueuePresentKHR_ICD(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
{
    return device_dispatch_table(queue)->QueuePresentKHR(queue, pPresentInfo);
}

VkResult VktWrappedQueue::QueueWaitIdle(VkQueue queue)
{
    const FuncId funcId = FuncId_vkQueueWaitIdle;

    VkResult result = VK_INCOMPLETE;

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p", queue);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(queue)->QueueWaitIdle(queue);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(queue)->QueueWaitIdle(queue);
    }

    return result;
}

VkResult VktWrappedQueue::QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence)
{
    const FuncId funcId = FuncId_vkQueueBindSparse;

    VkResult result = VK_INCOMPLETE;

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %u, 0x%p, 0x%p",
            queue,
            bindInfoCount,
            PrintArrayWithFormatter(bindInfoCount, pBindInfo, "0x%p").c_str(),
            fence);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer);
        result = device_dispatch_table(queue)->QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(queue)->QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    }

    return result;
}

#pragma warning (pop)