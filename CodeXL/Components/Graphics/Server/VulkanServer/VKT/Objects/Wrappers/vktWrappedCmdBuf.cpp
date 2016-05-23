//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktWrappedCmdBuf.cpp
/// \brief  A wrapper for command buffers.
//=============================================================================

#include "vktWrappedCmdBuf.h"
#include "../../vktLayerManager.h"
#include "../../vktInterceptManager.h"
#include "../../vktDefines.h"
#include "../../Profiling/vktCmdBufProfilerStatic.h"

//-----------------------------------------------------------------------------
/// Statically create a VktWrappedCmdBuf.
/// \param createInfo Creation struct used to construct this wrapper.
//-----------------------------------------------------------------------------
VktWrappedCmdBuf* VktWrappedCmdBuf::Create(const WrappedCmdBufCreateInfo& createInfo)
{
    VktWrappedCmdBuf* pOut = nullptr;

    if ((createInfo.device != VK_NULL_HANDLE) &&
        (createInfo.appCmdBuf != VK_NULL_HANDLE) &&
        (createInfo.pInterceptMgr != VK_NULL_HANDLE))
    {
        pOut = new VktWrappedCmdBuf(createInfo);
    }

    return pOut;
}

//-----------------------------------------------------------------------------
/// Constructor.
/// \param createInfo Creation struct used to construct this wrapper.
//-----------------------------------------------------------------------------
VktWrappedCmdBuf::VktWrappedCmdBuf(const WrappedCmdBufCreateInfo& createInfo) :
    m_pDynamicProfiler(nullptr),
    m_profiledCallCount(0),
    m_potentialProfiledCallCount(0),
    m_potentialProfiledCallCountHighest(0),
    m_pStaticProfiler(nullptr),
    m_fillId(0)
{
    memcpy(&m_createInfo, &createInfo, sizeof(m_createInfo));

#if MEASURE_WHOLE_CMD_BUFS
    m_pStaticProfiler = static_cast<VktCmdBufProfilerStatic*>(InitNewProfiler(PROFILER_TYPE_STATIC));
#endif
}

//-----------------------------------------------------------------------------
/// This command buffer is being freed.
//-----------------------------------------------------------------------------
void VktWrappedCmdBuf::Free()
{
#if GATHER_PROFILER_RESULTS_WITH_WORKERS
    FreeMT();
#else
    FreeST();
#endif
}

//-----------------------------------------------------------------------------
/// Track a command buffer call.
//-----------------------------------------------------------------------------
void VktWrappedCmdBuf::TrackCommandBufferCall(FuncId inFuncId)
{
    if ((inFuncId == FuncId_vkResetCommandBuffer) || (inFuncId == FuncId_vkBeginCommandBuffer) || (inFuncId == FuncId_vkEndCommandBuffer))
    {
#if TRACK_CMD_LIST_COMMANDS
        m_commands.clear();
#endif
        m_potentialProfiledCallCount = 0;
    }
    else
    {
#if TRACK_CMD_LIST_COMMANDS
        m_commands.push_back(inFuncId);
#endif

        if (VktFrameProfilerLayer::Instance()->ShouldProfileFunction(inFuncId))
        {
            // Update profiling state if this is the very first command
            if (m_potentialProfiledCallCount == 0)
            {
                if (VktFrameProfilerLayer::Instance()->ShouldCollectGPUTime() == true)
                {
                    m_pDynamicProfiler = InitNewProfiler(PROFILER_TYPE_DYNAMIC);
                }
            }

            m_potentialProfiledCallCount++;

            if (m_potentialProfiledCallCount > m_potentialProfiledCallCountHighest)
            {
                m_potentialProfiledCallCountHighest = m_potentialProfiledCallCount;
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Inject a "Before" timestamp measurement instruction for a profiled call.
/// \param pIdInfo The measurement information for the profiled call.
/// \returns A ProfilerResultCode indicating the operation's success.
//-----------------------------------------------------------------------------
ProfilerResultCode VktWrappedCmdBuf::BeginCmdMeasurement(const ProfilerMeasurementId* pIdInfo)
{
    ProfilerResultCode result = PROFILER_FAIL;

    if (m_pDynamicProfiler != nullptr)
    {
        result = m_pDynamicProfiler->BeginCmdMeasurement(pIdInfo);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Inject an "After" timestamp measurement instruction for a profiled call.
/// \returns A ProfilerResultCode indicating the operation's success.
//-----------------------------------------------------------------------------
ProfilerResultCode VktWrappedCmdBuf::EndCmdMeasurement()
{
    ProfilerResultCode result = PROFILER_FAIL;

    if (m_pDynamicProfiler != nullptr)
    {
        result = m_pDynamicProfiler->EndCmdMeasurement();

        if (result == PROFILER_SUCCESS)
        {
            m_profiledCallCount++;
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Use a single-threaded approach to get the results for the profiled workload on the given Queue.
/// \param outResults A vector of measurement results retrieved from the profiler.
/// \returns A ProfilerResultCode indicating the operation's success.
//-----------------------------------------------------------------------------
ProfilerResultCode VktWrappedCmdBuf::GetCmdBufResultsST(std::vector<ProfilerResult>& outResults)
{
    ProfilerResultCode result = PROFILER_SUCCESS;

    result = GetDynamicProfilerResults(m_fillId, outResults);

    result = GetStaticProfilerResults(m_fillId, m_profiledCallCount, outResults);

    DestroyDynamicProfilers();

    return result;
}

//-----------------------------------------------------------------------------
/// Use a multi-threaded approach to get the results for the profiled workload on the given Queue.
/// \param targetExecId The Id of the ExecuteCommandBuffers to retrieve profiling results for.
/// \param outResults A vector of measurement results retrieved from the profiler.
/// \returns A ProfilerResultCode indicating the operation's success.
//-----------------------------------------------------------------------------
ProfilerResultCode VktWrappedCmdBuf::GetCmdBufResultsMT(UINT64 targetFillId, UINT profiledCallCount, std::vector<ProfilerResult>& outResults)
{
    ProfilerResultCode result = PROFILER_SUCCESS;

    result = GetDynamicProfilerResultsMT(targetFillId, outResults);

    result = GetStaticProfilerResultsMT(targetFillId, profiledCallCount, outResults);

    return result;
}

//-----------------------------------------------------------------------------
/// Synchronized result fetch from our closed profilers.
/// \param targetExecId The Id of the ExecuteCommandBuffers to retrieve profiling results for.
/// \param outResults A vector of measurement results retrieved from the profiler.
/// \returns A ProfilerResultCode indicating the operation's success.
//-----------------------------------------------------------------------------
ProfilerResultCode VktWrappedCmdBuf::GetDynamicProfilerResults(UINT64 targetFillId, std::vector<ProfilerResult>& outResults)
{
    ProfilerResultCode result = PROFILER_SUCCESS;

    for (UINT i = 0; i < m_closedProfilers.size(); i++)
    {
        VktCmdBufProfiler* pProfiler = m_closedProfilers[i];

        if (pProfiler != nullptr)
        {
            if (targetFillId == pProfiler->GetFillId())
            {
                result = pProfiler->GetCmdBufResults(outResults);
            }
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Result fetch from our begin/end profiler.
/// \param outResults A vector of measurement results retrieved from the profiler.
/// \returns A ProfilerResultCode indicating the operation's success.
//-----------------------------------------------------------------------------
ProfilerResultCode VktWrappedCmdBuf::GetStaticProfilerResults(UINT64 fillId, UINT profiledCallCount, std::vector<ProfilerResult>& outResults)
{
    ProfilerResultCode result = PROFILER_SUCCESS;

#if MEASURE_WHOLE_CMD_BUFS
    if (m_pStaticProfiler != nullptr)
    {
#ifdef CODEXL_GRAPHICS
        if (profiledCallCount == 0)
        {
            m_pStaticProfiler->GetCmdBufResults(fillId, outResults);
        }
#else
        UNREFERENCED_PARAMETER(profiledCallCount);

        m_pStaticProfiler->GetCmdBufResults(fillId, outResults);
#endif
    }
#else
    UNREFERENCED_PARAMETER(fillId);
    UNREFERENCED_PARAMETER(outResults);
#endif

    return result;
}

//-----------------------------------------------------------------------------
/// Synchronized result fetch from our closed profilers.
/// \param targetFillId The Id of the ExecuteCommandBuffers to retrieve profiling results for.
/// \param outResults A vector of measurement results retrieved from the profiler.
/// \returns A ProfilerResultCode indicating the operation's success.
//-----------------------------------------------------------------------------
ProfilerResultCode VktWrappedCmdBuf::GetDynamicProfilerResultsMT(UINT64 targetFillId, std::vector<ProfilerResult>& outResults)
{
    ScopeLock lock(&m_closedProfilersMutex);

    ProfilerResultCode result = GetDynamicProfilerResults(targetFillId, outResults);

    return result;
}

//-----------------------------------------------------------------------------
/// Synchronized result fetch from our begin/end profiler.
/// \param outResults A vector of measurement results retrieved from the profiler.
/// \returns A ProfilerResultCode indicating the operation's success.
//-----------------------------------------------------------------------------
ProfilerResultCode VktWrappedCmdBuf::GetStaticProfilerResultsMT(UINT64 targetFillId, UINT profiledCallCount, std::vector<ProfilerResult>& outResults)
{
    ScopeLock lock(&m_staticProfilerMutex);

    ProfilerResultCode result = GetStaticProfilerResults(targetFillId, profiledCallCount, outResults);

    return result;
}

//-----------------------------------------------------------------------------
/// Single-threaded free ops.
//-----------------------------------------------------------------------------
void VktWrappedCmdBuf::FreeST()
{
    m_alive = false;

    DestroyDynamicProfilers();

#if MEASURE_WHOLE_CMD_BUFS
    ScopeLock lock(&m_staticProfilerMutex);

    if (m_pStaticProfiler != nullptr)
    {
        delete m_pStaticProfiler;
        m_pStaticProfiler = nullptr;
    }
#endif
}

//-----------------------------------------------------------------------------
/// Multi-threaded free ops.
//-----------------------------------------------------------------------------
void VktWrappedCmdBuf::FreeMT()
{
    ScopeLock lock(&m_deletionQueueMutex);

    m_alive = false;

    for (UINT i = 0; i < m_closedProfilers.size(); i++)
    {
        m_deletionQueue.push(m_closedProfilers[i]);
    }

    m_deletionQueue.push(m_pDynamicProfiler);

#if MEASURE_WHOLE_CMD_BUFS
    m_deletionQueue.push(m_pStaticProfiler);
#endif
}

//-----------------------------------------------------------------------------
/// Release resources when using thread workers
//-----------------------------------------------------------------------------
void VktWrappedCmdBuf::ReleaseProfilersMT()
{
    if (m_alive == false)
    {
        ScopeLock lock(&m_deletionQueueMutex);

        while (m_deletionQueue.empty() == false)
        {
            VktCmdBufProfiler* pProfiler = m_deletionQueue.front();

            if (pProfiler != nullptr)
            {
                delete pProfiler;
                pProfiler = nullptr;
            }

            m_deletionQueue.pop();
        }
    }
    else
    {
        DestroyDynamicProfilers();
    }
}

//-----------------------------------------------------------------------------
/// Safely clear closed profilers, only to be called after wiping queue.
//-----------------------------------------------------------------------------
void VktWrappedCmdBuf::ClearProfilersMT()
{
    ScopeLock lock(&m_closedProfilersMutex);

    m_closedProfilers.clear();
    m_pDynamicProfiler = nullptr;
    m_pStaticProfiler = nullptr;
}

//-----------------------------------------------------------------------------
/// Destroy all of the profiler objects used with this CommandBuffer.
//-----------------------------------------------------------------------------
void VktWrappedCmdBuf::DestroyDynamicProfilers()
{
    ScopeLock lock(&m_closedProfilersMutex);

    for (UINT i = 0; i < m_closedProfilers.size(); i++)
    {
        if ((m_closedProfilers[i] != nullptr) && (m_closedProfilers[i] != m_pDynamicProfiler))
        {
            delete m_closedProfilers[i];
            m_closedProfilers[i] = nullptr;
        }
    }

    m_closedProfilers.clear();

    if (m_pDynamicProfiler != nullptr)
    {
        delete m_pDynamicProfiler;
        m_pDynamicProfiler = nullptr;
    }
}

#if TRACK_CMD_LIST_COMMANDS
//-----------------------------------------------------------------------------
/// Debug function used to print out command buffer contents
//-----------------------------------------------------------------------------
void VktWrappedCmdBuf::PrintCommands()
{
    Log(logERROR, "===================================================\n");
    Log(logERROR, "CmdBuf: 0x%p\n", this);

    for (UINT i = 0; i < m_commands.size(); i++)
    {
        Log(logERROR, "%s\n", VktTraceAnalyzerLayer::Instance()->GetFunctionNameFromId(m_commands[i]));
    }
}
#endif

//-----------------------------------------------------------------------------
/// Initialize a profiler per-command buffer.
/// \param profilerType Either a static or dynamic profiler.
/// \returns A new profiler instance to be used for this command buffer.
//-----------------------------------------------------------------------------
VktCmdBufProfiler* VktWrappedCmdBuf::InitNewProfiler(ProfilerType profilerType)
{
    VktCmdBufProfiler* pProfiler = nullptr;

    UINT measurementsPerGroup = 256;

#if DYNAMIC_PROFILER_GROUP_SIZING

    if (measurementsPerGroup < m_potentialProfiledCallCountHighest)
    {
        measurementsPerGroup = m_potentialProfiledCallCountHighest;
    }

#endif

    VktCmdBufProfilerConfig config = {};
    config.measurementsPerGroup   = measurementsPerGroup;
    config.measurementTypeFlags   = PROFILER_MEASUREMENT_TYPE_TIMESTAMPS;
    config.maxStaleResourceGroups = 0;
    config.physicalDevice         = m_createInfo.physicalDevice;
    config.device                 = m_createInfo.device;
    config.cmdBuf                 = m_createInfo.appCmdBuf;
    config.mapTimestampMem        = false;
    config.newMemClear            = true;
    config.newMemClearValue       = 0;
    config.cmdBufFillId           = m_fillId;

    if (profilerType == PROFILER_TYPE_DYNAMIC)
    {
        pProfiler = VktCmdBufProfiler::Create(config);
    }
    else if (profilerType == PROFILER_TYPE_STATIC)
    {
        config.measurementsPerGroup = 1;
        pProfiler = VktCmdBufProfilerStatic::Create(config);
    }

    return pProfiler;
}

//-----------------------------------------------------------------------------
/// A custom implementation for BeginCommandBuffer.
/// \param commandBuffer The command buffer being closed.
/// \param pBeginInfo The command buffer begin info struct.
/// \returns A result code.
//-----------------------------------------------------------------------------
VkResult VktWrappedCmdBuf::BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo)
{
    VkResult result = BeginCommandBuffer_ICD(commandBuffer, pBeginInfo);

    m_fillId++;

    m_profiledCallCount = 0;

#if MEASURE_WHOLE_CMD_BUFS
    if (result == VK_SUCCESS)
    {
        if (m_pStaticProfiler != nullptr)
        {
            // SampleID is just an identifier, so we could use the command buffer handle plus the current submit number
            UINT cmdBufId = ((UINT64)m_createInfo.appCmdBuf & 0xFFFFFFFF) + (UINT)m_fillId;

            ProfilerMeasurementId measurementId = {};
            VktUtil::ConstructMeasurementInfo(FuncId_WholeCmdBuf, cmdBufId, this, VktLayerManager::GetLayerManager()->GetFrameCount(), m_fillId, measurementId);

            m_pStaticProfiler->BeginCmdMeasurement(&measurementId);
        }
    }
#endif

    return result;
}

//-----------------------------------------------------------------------------
/// A custom implementation for EndCommandBuffer.
/// \param commandBuffer The command buffer being closed.
/// \returns A result code.
//-----------------------------------------------------------------------------
VkResult VktWrappedCmdBuf::EndCommandBuffer(VkCommandBuffer commandBuffer)
{
#if MEASURE_WHOLE_CMD_BUFS
    if (m_pStaticProfiler != nullptr)
    {
        m_pStaticProfiler->EndCmdMeasurement();

        m_pStaticProfiler->NotifyCmdBufClosure();
    }
#endif

    if (m_pDynamicProfiler != nullptr)
    {
        ScopeLock lock(&m_closedProfilersMutex);

        // Update profiler state
        m_pDynamicProfiler->NotifyCmdBufClosure();

        // Stash the profiler
        m_closedProfilers.push_back(m_pDynamicProfiler);

        // We're now done with it
        m_pDynamicProfiler = nullptr;
    }

    return EndCommandBuffer_ICD(commandBuffer);
}

//-----------------------------------------------------------------------------
/// A custom implementation for ResetCommandBuffer.
/// \param commandBuffer The command buffer being closed.
/// \param flags The command buffer reset flags.
/// \returns A result code.
//-----------------------------------------------------------------------------
VkResult VktWrappedCmdBuf::ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
    if (m_pDynamicProfiler != nullptr)
    {
        m_pDynamicProfiler->NotifyCmdBufReset();
    }

    m_profiledCallCount = 0;

    return ResetCommandBuffer_ICD(commandBuffer, flags);
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
VkResult VktWrappedCmdBuf::BeginCommandBuffer_ICD(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo)
{
    const FuncId funcId = FuncId_vkBeginCommandBuffer;

    TrackCommandBufferCall(funcId);

    VkResult result = VK_INCOMPLETE;

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p", commandBuffer, pBeginInfo);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        result = device_dispatch_table(commandBuffer)->BeginCommandBuffer(commandBuffer, pBeginInfo);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(commandBuffer)->BeginCommandBuffer(commandBuffer, pBeginInfo);
    }

    return result;
}

VkResult VktWrappedCmdBuf::EndCommandBuffer_ICD(VkCommandBuffer commandBuffer)
{
    const FuncId funcId = FuncId_vkEndCommandBuffer;

    TrackCommandBufferCall(funcId);

    VkResult result = VK_INCOMPLETE;

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p", commandBuffer);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        result = device_dispatch_table(commandBuffer)->EndCommandBuffer(commandBuffer);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(commandBuffer)->EndCommandBuffer(commandBuffer);
    }

    return result;
}

VkResult VktWrappedCmdBuf::ResetCommandBuffer_ICD(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
    const FuncId funcId = FuncId_vkResetCommandBuffer;

    TrackCommandBufferCall(funcId);

    VkResult result = VK_INCOMPLETE;

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %s",
            commandBuffer,
            VktUtil::DecomposeCommandBufferResetFlagsEnumAsString(flags).c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        result = device_dispatch_table(commandBuffer)->ResetCommandBuffer(commandBuffer, flags);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry, result);
    }
    else
    {
        result = device_dispatch_table(commandBuffer)->ResetCommandBuffer(commandBuffer, flags);
    }

    return result;
}

void VktWrappedCmdBuf::CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
    const FuncId funcId = FuncId_vkCmdBindPipeline;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %s, 0x%p",
            commandBuffer,
            VktUtil::WritePipelineBindPointEnumAsString(pipelineBindPoint),
            pipeline);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    }
}

void VktWrappedCmdBuf::CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
{
    const FuncId funcId = FuncId_vkCmdSetViewport;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %u, %u, 0x%p",
            commandBuffer,
            firstViewport,
            viewportCount,
            PrintArrayWithFormatter(viewportCount, pViewports, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    }
}

void VktWrappedCmdBuf::CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
    const FuncId funcId = FuncId_vkCmdSetScissor;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %u, %u, 0x%p",
            commandBuffer,
            firstScissor,
            scissorCount,
            PrintArrayWithFormatter(scissorCount, pScissors, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    }
}

void VktWrappedCmdBuf::CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
    const FuncId funcId = FuncId_vkCmdSetLineWidth;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %f", commandBuffer, lineWidth);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdSetLineWidth(commandBuffer, lineWidth);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdSetLineWidth(commandBuffer, lineWidth);
    }
}

void VktWrappedCmdBuf::CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
    const FuncId funcId = FuncId_vkCmdSetDepthBias;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %f, %f, %f", commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }
}

void VktWrappedCmdBuf::CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
    const FuncId funcId = FuncId_vkCmdSetBlendConstants;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %f, %f, %f, %f", commandBuffer, blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdSetBlendConstants(commandBuffer, blendConstants);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdSetBlendConstants(commandBuffer, blendConstants);
    }
}

void VktWrappedCmdBuf::CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds)
{
    const FuncId funcId = FuncId_vkCmdSetDepthBounds;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %f, %f", commandBuffer, minDepthBounds, maxDepthBounds);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    }
}

void VktWrappedCmdBuf::CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask)
{
    const FuncId funcId = FuncId_vkCmdSetStencilCompareMask;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %s, %u",
            commandBuffer,
            VktUtil::DecomposeStencilFaceFlagsEnumAsString(faceMask).c_str(),
            compareMask);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    }
}

void VktWrappedCmdBuf::CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask)
{
    const FuncId funcId = FuncId_vkCmdSetStencilWriteMask;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %s, %u",
            commandBuffer,
            VktUtil::DecomposeStencilFaceFlagsEnumAsString(faceMask).c_str(),
            writeMask);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    }
}

void VktWrappedCmdBuf::CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference)
{
    const FuncId funcId = FuncId_vkCmdSetStencilReference;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %s, %u",
            commandBuffer,
            VktUtil::DecomposeStencilFaceFlagsEnumAsString(faceMask).c_str(),
            reference);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdSetStencilReference(commandBuffer, faceMask, reference);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdSetStencilReference(commandBuffer, faceMask, reference);
    }
}

void VktWrappedCmdBuf::CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
    const FuncId funcId = FuncId_vkCmdBindDescriptorSets;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %s, 0x%p, %u, %u, 0x%p, %u, 0x%p",
            commandBuffer,
            VktUtil::WritePipelineBindPointEnumAsString(pipelineBindPoint),
            layout,
            firstSet,
            descriptorSetCount,
            PrintArrayWithFormatter(descriptorSetCount, pDescriptorSets, "0x%p").c_str(),
            dynamicOffsetCount,
            PrintArrayWithFormatter(dynamicOffsetCount, pDynamicOffsets, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }
}

void VktWrappedCmdBuf::CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
    const FuncId funcId = FuncId_vkCmdBindIndexBuffer;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %llu, %s",
            commandBuffer,
            buffer,
            offset,
            VktUtil::WriteIndexTypeEnumAsString(indexType));

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    }
}

void VktWrappedCmdBuf::CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
    const FuncId funcId = FuncId_vkCmdBindVertexBuffers;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %u, %u, 0x%p, 0x%p",
            commandBuffer,
            firstBinding,
            bindingCount,
            PrintArrayWithFormatter(bindingCount, pBuffers, "0x%p").c_str(),
            PrintArrayWithFormatter(bindingCount, pOffsets, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    }
}

void VktWrappedCmdBuf::CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    const FuncId funcId = FuncId_vkCmdDraw;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %u, %u, %u, %u", commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }
}

void VktWrappedCmdBuf::CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    const FuncId funcId = FuncId_vkCmdDrawIndexed;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %u, %u, %u, %u, %u", commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
}

void VktWrappedCmdBuf::CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
    const FuncId funcId = FuncId_vkCmdDrawIndirect;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %llu, %u, %u", commandBuffer, buffer, offset, drawCount, stride);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
}

void VktWrappedCmdBuf::CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
    const FuncId funcId = FuncId_vkCmdDrawIndexedIndirect;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %llu, %u, %u", commandBuffer, buffer, offset, drawCount, stride);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
}

void VktWrappedCmdBuf::CmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z)
{
    const FuncId funcId = FuncId_vkCmdDispatch;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %u, %u, %u", commandBuffer, x, y, z);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdDispatch(commandBuffer, x, y, z);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdDispatch(commandBuffer, x, y, z);
    }
}

void VktWrappedCmdBuf::CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
    const FuncId funcId = FuncId_vkCmdDispatchIndirect;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %llu", commandBuffer, buffer, offset);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdDispatchIndirect(commandBuffer, buffer, offset);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdDispatchIndirect(commandBuffer, buffer, offset);
    }
}

void VktWrappedCmdBuf::CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions)
{
    const FuncId funcId = FuncId_vkCmdCopyBuffer;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, 0x%p, %u, 0x%p",
            commandBuffer,
            srcBuffer,
            dstBuffer,
            regionCount,
            PrintArrayWithFormatter(regionCount, pRegions, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    }
}

void VktWrappedCmdBuf::CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions)
{
    const FuncId funcId = FuncId_vkCmdCopyImage;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %s, 0x%p, %s, %u, 0x%p",
            commandBuffer,
            srcImage,
            VktUtil::WriteImageLayoutEnumAsString(srcImageLayout),
            dstImage,
            VktUtil::WriteImageLayoutEnumAsString(dstImageLayout),
            regionCount,
            PrintArrayWithFormatter(regionCount, pRegions, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }
}

void VktWrappedCmdBuf::CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter)
{
    const FuncId funcId = FuncId_vkCmdBlitImage;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %s, 0x%p, %s, %u, 0x%p, %s",
            commandBuffer,
            srcImage,
            VktUtil::WriteImageLayoutEnumAsString(srcImageLayout),
            dstImage,
            VktUtil::WriteImageLayoutEnumAsString(dstImageLayout),
            regionCount,
            PrintArrayWithFormatter(regionCount, pRegions, "0x%p").c_str(),
            VktUtil::WriteFilterEnumAsString(filter));

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    }
}

void VktWrappedCmdBuf::CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    const FuncId funcId = FuncId_vkCmdCopyBufferToImage;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, 0x%p, %s, %u, 0x%p",
            commandBuffer,
            srcBuffer,
            dstImage,
            VktUtil::WriteImageLayoutEnumAsString(dstImageLayout),
            regionCount,
            PrintArrayWithFormatter(regionCount, pRegions, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    }
}

void VktWrappedCmdBuf::CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    const FuncId funcId = FuncId_vkCmdCopyImageToBuffer;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %s, 0x%p, %u, 0x%p",
            commandBuffer,
            srcImage,
            VktUtil::WriteImageLayoutEnumAsString(srcImageLayout),
            dstBuffer,
            regionCount,
            PrintArrayWithFormatter(regionCount, pRegions, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    }
}

void VktWrappedCmdBuf::CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const uint32_t* pData)
{
    const FuncId funcId = FuncId_vkCmdUpdateBuffer;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %llu, %llu, 0x%p", commandBuffer, dstBuffer, dstOffset, dataSize, pData);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    }
}

void VktWrappedCmdBuf::CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data)
{
    const FuncId funcId = FuncId_vkCmdFillBuffer;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %llu, %llu, %u", commandBuffer, dstBuffer, dstOffset, size, data);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    }
}

void VktWrappedCmdBuf::CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    const FuncId funcId = FuncId_vkCmdClearColorImage;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %s, 0x%p, %u, 0x%p",
            commandBuffer,
            image,
            VktUtil::WriteImageLayoutEnumAsString(imageLayout),
            pColor,
            rangeCount,
            PrintArrayWithFormatter(rangeCount, pRanges, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    }
}

void VktWrappedCmdBuf::CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    const FuncId funcId = FuncId_vkCmdClearDepthStencilImage;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %s, 0x%p, %u, 0x%p",
            commandBuffer,
            image,
            VktUtil::WriteImageLayoutEnumAsString(imageLayout),
            pDepthStencil,
            rangeCount,
            PrintArrayWithFormatter(rangeCount, pRanges, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    }
}

void VktWrappedCmdBuf::CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects)
{
    const FuncId funcId = FuncId_vkCmdClearAttachments;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %u, 0x%p, %u, 0x%p",
            commandBuffer,
            attachmentCount,
            PrintArrayWithFormatter(attachmentCount, pAttachments, "0x%p").c_str(),
            rectCount,
            PrintArrayWithFormatter(rectCount, pRects, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    }
}

void VktWrappedCmdBuf::CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions)
{
    const FuncId funcId = FuncId_vkCmdResolveImage;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %s, 0x%p, %s, %u, 0x%p",
            commandBuffer,
            srcImage,
            VktUtil::WriteImageLayoutEnumAsString(srcImageLayout),
            dstImage,
            VktUtil::WriteImageLayoutEnumAsString(dstImageLayout),
            regionCount,
            PrintArrayWithFormatter(regionCount, pRegions, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }
}

void VktWrappedCmdBuf::CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
    const FuncId funcId = FuncId_vkCmdSetEvent;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %s",
            commandBuffer,
            event,
            VktUtil::DecomposePipelineStageFlagsEnumAsString(stageMask).c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdSetEvent(commandBuffer, event, stageMask);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdSetEvent(commandBuffer, event, stageMask);
    }
}

void VktWrappedCmdBuf::CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
    const FuncId funcId = FuncId_vkCmdResetEvent;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %s",
            commandBuffer,
            event,
            VktUtil::DecomposePipelineStageFlagsEnumAsString(stageMask).c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdResetEvent(commandBuffer, event, stageMask);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdResetEvent(commandBuffer, event, stageMask);
    }
}

void VktWrappedCmdBuf::CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    const FuncId funcId = FuncId_vkCmdWaitEvents;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %u, 0x%p, %s, %s, %u, 0x%p, %u, 0x%p, %u, 0x%p",
            commandBuffer,
            eventCount,
            PrintArrayWithFormatter(eventCount, pEvents, "0x%p").c_str(),
            VktUtil::DecomposePipelineStageFlagsEnumAsString(srcStageMask).c_str(),
            VktUtil::DecomposePipelineStageFlagsEnumAsString(dstStageMask).c_str(),
            memoryBarrierCount,
            PrintArrayWithFormatter(memoryBarrierCount, pMemoryBarriers, "0x%p").c_str(),
            bufferMemoryBarrierCount,
            PrintArrayWithFormatter(bufferMemoryBarrierCount, pBufferMemoryBarriers, "0x%p").c_str(),
            imageMemoryBarrierCount,
            PrintArrayWithFormatter(imageMemoryBarrierCount, pImageMemoryBarriers, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }
}

void VktWrappedCmdBuf::CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    const FuncId funcId = FuncId_vkCmdPipelineBarrier;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %s, %s, %s, %u, 0x%p, %u, 0x%p, %u, 0x%p",
            commandBuffer,
            VktUtil::DecomposePipelineStageFlagsEnumAsString(srcStageMask).c_str(),
            VktUtil::DecomposePipelineStageFlagsEnumAsString(dstStageMask).c_str(),
            VktUtil::DecomposeDependencyFlagsEnumAsString(dependencyFlags).c_str(),
            memoryBarrierCount,
            PrintArrayWithFormatter(memoryBarrierCount, pMemoryBarriers, "0x%p").c_str(),
            bufferMemoryBarrierCount,
            PrintArrayWithFormatter(bufferMemoryBarrierCount, pBufferMemoryBarriers, "0x%p").c_str(),
            imageMemoryBarrierCount,
            PrintArrayWithFormatter(imageMemoryBarrierCount, pImageMemoryBarriers, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }
}

void VktWrappedCmdBuf::CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
{
    const FuncId funcId = FuncId_vkCmdBeginQuery;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %u, %s",
            commandBuffer,
            queryPool,
            query,
            VktUtil::DecomposeQueryControlFlagsEnumAsString(flags).c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdBeginQuery(commandBuffer, queryPool, query, flags);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdBeginQuery(commandBuffer, queryPool, query, flags);
    }
}

void VktWrappedCmdBuf::CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query)
{
    const FuncId funcId = FuncId_vkCmdEndQuery;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %u", commandBuffer, queryPool, query);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdEndQuery(commandBuffer, queryPool, query);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdEndQuery(commandBuffer, queryPool, query);
    }
}

void VktWrappedCmdBuf::CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
    const FuncId funcId = FuncId_vkCmdResetQueryPool;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %u, %u", commandBuffer, queryPool, firstQuery, queryCount);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    }
}

void VktWrappedCmdBuf::CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query)
{
    const FuncId funcId = FuncId_vkCmdWriteTimestamp;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %s, 0x%p, %u",
            commandBuffer,
            VktUtil::DecomposePipelineStageFlagsEnumAsString(pipelineStage).c_str(),
            queryPool,
            query);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    }
}

void VktWrappedCmdBuf::CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
{
    const FuncId funcId = FuncId_vkCmdCopyQueryPoolResults;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %u, %u, 0x%p, %llu, %llu, %s",
            commandBuffer,
            queryPool,
            firstQuery,
            queryCount,
            dstBuffer,
            dstOffset,
            stride,
            VktUtil::DecomposeQueryResultFlagsEnumAsString(flags).c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    }
}

void VktWrappedCmdBuf::CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues)
{
    const FuncId funcId = FuncId_vkCmdPushConstants;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %s, %u, %u, 0x%p",
            commandBuffer,
            layout,
            VktUtil::DecomposeShaderStageFlagsEnumAsString(stageFlags).c_str(),
            offset,
            size,
            pValues);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    }
}

void VktWrappedCmdBuf::CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
    const FuncId funcId = FuncId_vkCmdBeginRenderPass;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, 0x%p, %s",
            commandBuffer,
            pRenderPassBegin,
            VktUtil::WriteSubpassContentsEnumAsString(contents));

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    }
}

void VktWrappedCmdBuf::CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
    const FuncId funcId = FuncId_vkCmdNextSubpass;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %s",
            commandBuffer,
            VktUtil::WriteSubpassContentsEnumAsString(contents));

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdNextSubpass(commandBuffer, contents);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdNextSubpass(commandBuffer, contents);
    }
}

void VktWrappedCmdBuf::CmdEndRenderPass(VkCommandBuffer commandBuffer)
{
    const FuncId funcId = FuncId_vkCmdEndRenderPass;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p", this);

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdEndRenderPass(commandBuffer);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdEndRenderPass(commandBuffer);
    }
}

void VktWrappedCmdBuf::CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    const FuncId funcId = FuncId_vkCmdExecuteCommands;

    TrackCommandBufferCall(funcId);

    if (m_createInfo.pInterceptMgr->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "0x%p, %u, 0x%p",
            commandBuffer,
            commandBufferCount,
            PrintArrayWithFormatter(commandBufferCount, pCommandBuffers, "0x%p").c_str());

        VktAPIEntry* pNewEntry = m_createInfo.pInterceptMgr->PreCall(funcId, argumentsBuffer, this);
        device_dispatch_table(commandBuffer)->CmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
        m_createInfo.pInterceptMgr->PostCall(pNewEntry);
    }
    else
    {
        device_dispatch_table(commandBuffer)->CmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    }
}

#pragma warning (pop)