//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktWrappedObject.h
/// \brief  Super class for object wrappers.
//=============================================================================

#include "vktWrappedObject.h"

/// A listing of all entry points of this device dispatch table
const VkLayerDispatchTable DeviceDispatchTable =
{
    vkGetDeviceProcAddr,
    Mine_vkDestroyDevice,
    Mine_vkGetDeviceQueue,
    Mine_vkQueueSubmit,
    Mine_vkQueueWaitIdle,
    Mine_vkDeviceWaitIdle,
    Mine_vkAllocateMemory,
    Mine_vkFreeMemory,
    Mine_vkMapMemory,
    Mine_vkUnmapMemory,
    Mine_vkFlushMappedMemoryRanges,
    Mine_vkInvalidateMappedMemoryRanges,
    Mine_vkGetDeviceMemoryCommitment,
    Mine_vkGetImageSparseMemoryRequirements,
    Mine_vkGetImageMemoryRequirements,
    Mine_vkGetBufferMemoryRequirements,
    Mine_vkBindImageMemory,
    Mine_vkBindBufferMemory,
    Mine_vkQueueBindSparse,
    Mine_vkCreateFence,
    Mine_vkDestroyFence,
    Mine_vkGetFenceStatus,
    Mine_vkResetFences,
    Mine_vkWaitForFences,
    Mine_vkCreateSemaphore,
    Mine_vkDestroySemaphore,
    Mine_vkCreateEvent,
    Mine_vkDestroyEvent,
    Mine_vkGetEventStatus,
    Mine_vkSetEvent,
    Mine_vkResetEvent,
    Mine_vkCreateQueryPool,
    Mine_vkDestroyQueryPool,
    Mine_vkGetQueryPoolResults,
    Mine_vkCreateBuffer,
    Mine_vkDestroyBuffer,
    Mine_vkCreateBufferView,
    Mine_vkDestroyBufferView,
    Mine_vkCreateImage,
    Mine_vkDestroyImage,
    Mine_vkGetImageSubresourceLayout,
    Mine_vkCreateImageView,
    Mine_vkDestroyImageView,
    Mine_vkCreateShaderModule,
    Mine_vkDestroyShaderModule,
    Mine_vkCreatePipelineCache,
    Mine_vkDestroyPipelineCache,
    Mine_vkGetPipelineCacheData,
    Mine_vkMergePipelineCaches,
    Mine_vkCreateGraphicsPipelines,
    Mine_vkCreateComputePipelines,
    Mine_vkDestroyPipeline,
    Mine_vkCreatePipelineLayout,
    Mine_vkDestroyPipelineLayout,
    Mine_vkCreateSampler,
    Mine_vkDestroySampler,
    Mine_vkCreateDescriptorSetLayout,
    Mine_vkDestroyDescriptorSetLayout,
    Mine_vkCreateDescriptorPool,
    Mine_vkDestroyDescriptorPool,
    Mine_vkResetDescriptorPool,
    Mine_vkAllocateDescriptorSets,
    Mine_vkFreeDescriptorSets,
    Mine_vkUpdateDescriptorSets,
    Mine_vkCreateFramebuffer,
    Mine_vkDestroyFramebuffer,
    Mine_vkCreateRenderPass,
    Mine_vkDestroyRenderPass,
    Mine_vkGetRenderAreaGranularity,
    Mine_vkCreateCommandPool,
    Mine_vkDestroyCommandPool,
    Mine_vkResetCommandPool,
    Mine_vkAllocateCommandBuffers,
    Mine_vkFreeCommandBuffers,
    Mine_vkBeginCommandBuffer,
    Mine_vkEndCommandBuffer,
    Mine_vkResetCommandBuffer,
    Mine_vkCmdBindPipeline,
    Mine_vkCmdBindDescriptorSets,
    Mine_vkCmdBindVertexBuffers,
    Mine_vkCmdBindIndexBuffer,
    Mine_vkCmdSetViewport,
    Mine_vkCmdSetScissor,
    Mine_vkCmdSetLineWidth,
    Mine_vkCmdSetDepthBias,
    Mine_vkCmdSetBlendConstants,
    Mine_vkCmdSetDepthBounds,
    Mine_vkCmdSetStencilCompareMask,
    Mine_vkCmdSetStencilWriteMask,
    Mine_vkCmdSetStencilReference,
    Mine_vkCmdDraw,
    Mine_vkCmdDrawIndexed,
    Mine_vkCmdDrawIndirect,
    Mine_vkCmdDrawIndexedIndirect,
    Mine_vkCmdDispatch,
    Mine_vkCmdDispatchIndirect,
    Mine_vkCmdCopyBuffer,
    Mine_vkCmdCopyImage,
    Mine_vkCmdBlitImage,
    Mine_vkCmdCopyBufferToImage,
    Mine_vkCmdCopyImageToBuffer,
    Mine_vkCmdUpdateBuffer,
    Mine_vkCmdFillBuffer,
    Mine_vkCmdClearColorImage,
    Mine_vkCmdClearDepthStencilImage,
    Mine_vkCmdClearAttachments,
    Mine_vkCmdResolveImage,
    Mine_vkCmdSetEvent,
    Mine_vkCmdResetEvent,
    Mine_vkCmdWaitEvents,
    Mine_vkCmdPipelineBarrier,
    Mine_vkCmdBeginQuery,
    Mine_vkCmdEndQuery,
    Mine_vkCmdResetQueryPool,
    Mine_vkCmdWriteTimestamp,
    Mine_vkCmdCopyQueryPoolResults,
    Mine_vkCmdPushConstants,
    Mine_vkCmdBeginRenderPass,
    Mine_vkCmdNextSubpass,
    Mine_vkCmdEndRenderPass,
    Mine_vkCmdExecuteCommands,
    Mine_vkCreateSwapchainKHR,
    Mine_vkDestroySwapchainKHR,
    Mine_vkGetSwapchainImagesKHR,
    Mine_vkAcquireNextImageKHR,
    Mine_vkQueuePresentKHR,
};

//-----------------------------------------------------------------------------
/// Generate and allocate an object handle for the app, that the server manages and translates.
/// \param pWrappedObject Input wrapper.
/// \return A new handle for the app.
//-----------------------------------------------------------------------------
UINT64 CreateAppHandle(VktWrappedObject* pWrappedObject)
{
    UINT64 appHandle = NULL;

    if (pWrappedObject != NULL)
    {
        IcdApiObject* pApiObject = new IcdApiObject;

        set_loader_magic_value(&pApiObject->loaderData.loaderMagic);

        pApiObject->loaderData.loaderData = (void*)&DeviceDispatchTable;
        pApiObject->pWrappedObject = pWrappedObject;

        appHandle = reinterpret_cast<UINT64>(pApiObject);

        pWrappedObject->StoreAppHandle(appHandle);
    }

    return appHandle;
}

//-----------------------------------------------------------------------------
/// Destroy a shim handle.
/// \param handle Input object handle.
//-----------------------------------------------------------------------------
void DestroyWrappedHandle(UINT64 handle)
{
    if (handle != NULL)
    {
        IcdApiObject* pApiObject = reinterpret_cast<IcdApiObject*>(handle);

        delete pApiObject;
    }
}

//-----------------------------------------------------------------------------
/// Obtain a shim object when given an app handle.
/// \param handle Input object handle.
/// \return Pointer to a wrapped object.
//-----------------------------------------------------------------------------
VktWrappedObject* WrappedObjectFromAppHandle(UINT64 handle)
{
    VktWrappedObject* pObj = NULL;

    if (handle != VK_NULL_HANDLE)
    {
        pObj = reinterpret_cast<IcdApiObject*>(handle)->pWrappedObject;
    }

    return pObj;
}

//-----------------------------------------------------------------------------
/// Obtain a shim object when given an app handle.
/// \param handle Input object handle.
/// \return Pointer to the object's dispatch table.
//-----------------------------------------------------------------------------
const VkLayerDispatchTable* DispatchTableFromHandle(UINT64 handle)
{
    return (VkLayerDispatchTable*)((reinterpret_cast<IcdApiObject*>(handle))->loaderData.loaderData);
}