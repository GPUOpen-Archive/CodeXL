//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktWrappedCmdBuf.h
/// \brief  A wrapper for command buffers.
//=============================================================================

#ifndef __VKT_WRAPPED_CMD_BUF_H__
#define __VKT_WRAPPED_CMD_BUF_H__

#include "vktWrappedObject.h"
#include "../../Util/vktUtil.h"
#include "../../Profiling/vktCmdBufProfiler.h"

class VktInterceptManager;

//-----------------------------------------------------------------------------
/// Holds information used to create a command buffer wrapper.
//-----------------------------------------------------------------------------
struct WrappedCmdBufCreateInfo
{
    VkPhysicalDevice            physicalDevice; ///< The command buffer's physical device
    VkDevice                    device;         ///< The command buffer's device
    VkCommandBufferAllocateInfo allocInfo;      ///< The allocInfo used to create the command buffer
    VkCommandBuffer             appCmdBuf;      ///< The app's command buffer handle
    VktInterceptManager*        pInterceptMgr;  ///< Pointer to interception manager
};

//-----------------------------------------------------------------------------
/// Used to track all Vulkan API calls that are traced at runtime.
/// All API calls can be traced, and only some can be profiled.
//-----------------------------------------------------------------------------
class VktWrappedCmdBuf : public VktWrappedObject
{
public:
    static VktWrappedCmdBuf* Create(const WrappedCmdBufCreateInfo& createInfo);

    VktWrappedCmdBuf(const WrappedCmdBufCreateInfo& createInfo);
    ~VktWrappedCmdBuf() {}

    void Free();

    /// Store the app's handle to this wrapper
    virtual void StoreAppHandle(UINT64 hAppObject) { UNREFERENCED_PARAMETER(hAppObject); }

    /// Return the app's handle to this wrapper
    VkCommandBuffer AppHandle() { return m_createInfo.appCmdBuf; }

    void TrackCommandBufferCall(FuncId inFuncId);

    ProfilerResultCode BeginCmdMeasurement(const ProfilerMeasurementId* pIdInfo);
    ProfilerResultCode EndCmdMeasurement();
    ProfilerResultCode GetCmdBufResultsST(std::vector<ProfilerResult>& outResults);
    ProfilerResultCode GetCmdBufResultsMT(INT64 targetExecId, std::vector<ProfilerResult>& outResults);
    ProfilerResultCode GetDynamicProfilerResultsMT(INT64 targetExecId, std::vector<ProfilerResult>& outResults);
    ProfilerResultCode GetStaticProfilerResults(std::vector<ProfilerResult>& outResults);
    ProfilerResultCode GetStaticProfilerResultsMT(std::vector<ProfilerResult>& outResults);

    /// Determine if this command buffer is currently being profiled
    bool IsProfilingEnabled() { return m_pDynamicProfiler != nullptr; }

    /// Return the number of profiled commands for this CmdBuf
    UINT GetProfiledCallCount() const { return m_profiledCallCount; }

    void DestroyDynamicProfilers();
    void SetProfilerExecutionId(INT64 executionId);

    VkResult BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
    VkResult EndCommandBuffer(VkCommandBuffer commandBuffer);
    VkResult ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);

    void IncrementSubmitCount() { m_submitNumber++; }

#if TRACK_CMD_LIST_COMMANDS
    void PrintCommands();
#endif

    //-----------------------------------------------------------------------------
    /// ICD entry points.
    //-----------------------------------------------------------------------------
    VkResult BeginCommandBuffer_ICD(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
    VkResult EndCommandBuffer_ICD(VkCommandBuffer commandBuffer);
    VkResult ResetCommandBuffer_ICD(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);

    void CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
    void CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports);
    void CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors);
    void CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);
    void CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
    void CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
    void CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
    void CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
    void CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
    void CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
    void CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets);
    void CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
    void CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
    void CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    void CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
    void CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
    void CmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
    void CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
    void CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);
    void CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions);
    void CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter);
    void CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions);
    void CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);
    void CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const uint32_t* pData);
    void CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);
    void CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges);
    void CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges);
    void CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects);
    void CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions);
    void CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
    void CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
    void CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
    void CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
    void CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags);
    void CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query);
    void CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
    void CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query);
    void CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags);
    void CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues);
    void CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents);
    void CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
    void CmdEndRenderPass(VkCommandBuffer commandBuffer);
    void CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);

private:
    VktCmdBufProfiler* InitNewProfiler(ProfilerType profilerType);

    /// The active profiler object for this CommandBuffer.
    VktCmdBufProfiler* m_pDynamicProfiler;

    /// A vector of closed profiler objects for this CommandBuffer.
    std::vector<VktCmdBufProfiler*> m_closedProfilers;

    /// Mutex to lock the usage of the internal profiler instances.
    mutex m_closedProfilersMutex;

    /// A profiler dedicated to measuring the span of this entire command buffer.
    VktCmdBufProfiler* m_pStaticProfiler;

    /// Mutex to lock the usage of profilers used to measure full command buffer duration.
    mutex m_staticProfilerMutex;

    /// The number of GPU commands profiled within this CommandBuffer.
    UINT m_profiledCallCount;

    /// The potential number of profiled calls we might expect to see in this CommandBuffer.
    UINT m_potentialProfiledCallCount;

    /// The highest number of profiled calls we've seen for this CommandBuffer.
    UINT m_potentialProfiledCallCountHighest;

    /// CmdBuf create info
    WrappedCmdBufCreateInfo m_createInfo;

    /// Track number of submits
    UINT m_submitNumber;

    /// Track aliveness
    bool m_alive;
};

#endif // __VKT_WRAPPED_CMD_BUF_H__