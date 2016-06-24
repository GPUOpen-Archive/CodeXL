//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktEnumerations.h
/// \brief  A set of declarations used to internally track Vulkan API call type,
///         function IDs, and API object types.
//==============================================================================

#ifndef __VKT_ENUMERATIONS_H__
#define __VKT_ENUMERATIONS_H__

//-----------------------------------------------------------------------------
/// An API Type enumeration that defines the group that the API call is classified into.
//-----------------------------------------------------------------------------
#ifndef CODEXL_INCLUDE
enum eAPIType
{
    kAPIType_Unknown           = 0,
    kAPIType_CmdBufNonProfiled = 0x1,
    kAPIType_CmdBufProfiled    = 0x2,
    kAPIType_DescriptorSet     = 0x4,
    kAPIType_Create            = 0x8,
    kAPIType_Destroy           = 0x10,
    kAPIType_Get               = 0x20,
    kAPIType_Memory            = 0x40,
    kAPIType_QueueSubmission   = 0x80,
    kAPIType_Sync              = 0x100,
    kAPIType_KHR               = 0x200,
};
#endif

//-----------------------------------------------------------------------------
/// An API Type enumeration that defines the group that the API call is classified into.
//-----------------------------------------------------------------------------
#ifdef CODEXL_INCLUDE
enum vkAPIType
{
    vkAPIType_Unknown = 0,
    vkAPIType_CmdBufNonProfiled = 0x1,
    vkAPIType_CmdBufProfiled = 0x2,
    vkAPIType_DescriptorSet = 0x4,
    vkAPIType_Create = 0x8,
    vkAPIType_Destroy = 0x10,
    vkAPIType_Get = 0x20,
    vkAPIType_Memory = 0x40,
    vkAPIType_QueueSubmission = 0x80,
    vkAPIType_Sync = 0x100,
    vkAPIType_KHR = 0x200,
};
#endif


//-----------------------------------------------------------------------------
/// A FuncId enumeration that defines all Vulkan API calls that can possibly be traced.
//-----------------------------------------------------------------------------
#ifdef CODEXL_INCLUDE
enum VkFuncId : int
#else
enum FuncId : int
#endif
{
#ifdef CODEXL_INCLUDE
    FuncId_vkUNDEFINED = 0,
#else
    FuncId_UNDEFINED = 0,
#endif

    FuncId_vkCreateInstance,
    FuncId_vkDestroyInstance,
    FuncId_vkEnumeratePhysicalDevices,
    FuncId_vkGetPhysicalDeviceFeatures,
    FuncId_vkGetPhysicalDeviceFormatProperties,
    FuncId_vkGetPhysicalDeviceImageFormatProperties,
    FuncId_vkGetPhysicalDeviceProperties,
    FuncId_vkGetPhysicalDeviceQueueFamilyProperties,
    FuncId_vkGetPhysicalDeviceMemoryProperties,
    FuncId_vkCreateDevice,
    FuncId_vkDestroyDevice,
    FuncId_vkEnumerateInstanceExtensionProperties,
    FuncId_vkEnumerateInstanceLayerProperties,
    FuncId_vkEnumerateDeviceLayerProperties,
    FuncId_vkGetDeviceQueue,
    FuncId_vkQueueSubmit,
    FuncId_vkQueueWaitIdle,
    FuncId_vkDeviceWaitIdle,
    FuncId_vkAllocateMemory,
    FuncId_vkFreeMemory,
    FuncId_vkMapMemory,
    FuncId_vkUnmapMemory,
    FuncId_vkFlushMappedMemoryRanges,
    FuncId_vkInvalidateMappedMemoryRanges,
    FuncId_vkGetDeviceMemoryCommitment,
    FuncId_vkBindBufferMemory,
    FuncId_vkBindImageMemory,
    FuncId_vkGetBufferMemoryRequirements,
    FuncId_vkGetImageMemoryRequirements,
    FuncId_vkGetImageSparseMemoryRequirements,
    FuncId_vkGetPhysicalDeviceSparseImageFormatProperties,
    FuncId_vkQueueBindSparse,
    FuncId_vkCreateFence,
    FuncId_vkDestroyFence,
    FuncId_vkResetFences,
    FuncId_vkGetFenceStatus,
    FuncId_vkWaitForFences,
    FuncId_vkCreateSemaphore,
    FuncId_vkDestroySemaphore,
    FuncId_vkCreateEvent,
    FuncId_vkDestroyEvent,
    FuncId_vkGetEventStatus,
    FuncId_vkSetEvent,
    FuncId_vkResetEvent,
    FuncId_vkCreateQueryPool,
    FuncId_vkDestroyQueryPool,
    FuncId_vkGetQueryPoolResults,
    FuncId_vkCreateBuffer,
    FuncId_vkDestroyBuffer,
    FuncId_vkCreateBufferView,
    FuncId_vkDestroyBufferView,
    FuncId_vkCreateImage,
    FuncId_vkDestroyImage,
    FuncId_vkGetImageSubresourceLayout,
    FuncId_vkCreateImageView,
    FuncId_vkDestroyImageView,
    FuncId_vkCreateShaderModule,
    FuncId_vkDestroyShaderModule,
    FuncId_vkCreatePipelineCache,
    FuncId_vkDestroyPipelineCache,
    FuncId_vkGetPipelineCacheData,
    FuncId_vkMergePipelineCaches,
    FuncId_vkCreateGraphicsPipelines,
    FuncId_vkCreateComputePipelines,
    FuncId_vkDestroyPipeline,
    FuncId_vkCreatePipelineLayout,
    FuncId_vkDestroyPipelineLayout,
    FuncId_vkCreateSampler,
    FuncId_vkDestroySampler,
    FuncId_vkCreateDescriptorSetLayout,
    FuncId_vkDestroyDescriptorSetLayout,
    FuncId_vkCreateDescriptorPool,
    FuncId_vkDestroyDescriptorPool,
    FuncId_vkResetDescriptorPool,
    FuncId_vkAllocateDescriptorSets,
    FuncId_vkFreeDescriptorSets,
    FuncId_vkUpdateDescriptorSets,
    FuncId_vkCreateFramebuffer,
    FuncId_vkDestroyFramebuffer,
    FuncId_vkCreateRenderPass,
    FuncId_vkDestroyRenderPass,
    FuncId_vkGetRenderAreaGranularity,
    FuncId_vkCreateCommandPool,
    FuncId_vkDestroyCommandPool,
    FuncId_vkResetCommandPool,
    FuncId_vkAllocateCommandBuffers,
    FuncId_vkFreeCommandBuffers,
    FuncId_vkBeginCommandBuffer,
    FuncId_vkEndCommandBuffer,
    FuncId_vkResetCommandBuffer,
    FuncId_vkCmdBindPipeline,
    FuncId_vkCmdSetViewport,
    FuncId_vkCmdSetScissor,
    FuncId_vkCmdSetLineWidth,
    FuncId_vkCmdSetDepthBias,
    FuncId_vkCmdSetBlendConstants,
    FuncId_vkCmdSetDepthBounds,
    FuncId_vkCmdSetStencilCompareMask,
    FuncId_vkCmdSetStencilWriteMask,
    FuncId_vkCmdSetStencilReference,
    FuncId_vkCmdBindDescriptorSets,
    FuncId_vkCmdBindIndexBuffer,
    FuncId_vkCmdBindVertexBuffers,
    FuncId_vkCmdDraw,
    FuncId_vkCmdDrawIndexed,
    FuncId_vkCmdDrawIndirect,
    FuncId_vkCmdDrawIndexedIndirect,
    FuncId_vkCmdDispatch,
    FuncId_vkCmdDispatchIndirect,
    FuncId_vkCmdCopyBuffer,
    FuncId_vkCmdCopyImage,
    FuncId_vkCmdBlitImage,
    FuncId_vkCmdCopyBufferToImage,
    FuncId_vkCmdCopyImageToBuffer,
    FuncId_vkCmdUpdateBuffer,
    FuncId_vkCmdFillBuffer,
    FuncId_vkCmdClearColorImage,
    FuncId_vkCmdClearDepthStencilImage,
    FuncId_vkCmdClearAttachments,
    FuncId_vkCmdResolveImage,
    FuncId_vkCmdSetEvent,
    FuncId_vkCmdResetEvent,
    FuncId_vkCmdWaitEvents,
    FuncId_vkCmdPipelineBarrier,
    FuncId_vkCmdBeginQuery,
    FuncId_vkCmdEndQuery,
    FuncId_vkCmdResetQueryPool,
    FuncId_vkCmdWriteTimestamp,
    FuncId_vkCmdCopyQueryPoolResults,
    FuncId_vkCmdPushConstants,
    FuncId_vkCmdBeginRenderPass,
    FuncId_vkCmdNextSubpass,
    FuncId_vkCmdEndRenderPass,
    FuncId_vkCmdExecuteCommands,
    FuncId_vkDestroySurfaceKHR,
    FuncId_vkCreateSwapchainKHR,
    FuncId_vkDestroySwapchainKHR,
    FuncId_vkGetSwapchainImagesKHR,
    FuncId_vkAcquireNextImageKHR,
    FuncId_vkQueuePresentKHR,
    FuncId_vkGetPhysicalDeviceSurfaceSupportKHR,
    FuncId_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
    FuncId_vkGetPhysicalDeviceSurfaceFormatsKHR,
    FuncId_vkGetPhysicalDeviceSurfacePresentModesKHR,
    FuncId_vkCreateWin32SurfaceKHR,
    FuncId_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    FuncId_vkCreateXcbSurfaceKHR,
    FuncId_vkGetPhysicalDeviceXcbPresentationSupportKHR,
    FuncId_vkCreateXlibSurfaceKHR,
    FuncId_vkGetPhysicalDeviceXlibPresentationSupportKHR,
    FuncId_vkGetPhysicalDeviceDisplayPropertiesKHR,
    FuncId_vkGetPhysicalDeviceDisplayPlanePropertiesKHR,
    FuncId_vkGetDisplayPlaneSupportedDisplaysKHR,
    FuncId_vkGetDisplayModePropertiesKHR,
    FuncId_vkCreateDisplayModeKHR,
    FuncId_vkGetDisplayPlaneCapabilitiesKHR,
    FuncId_vkCreateDisplayPlaneSurfaceKHR,
    FuncId_vkCreateDebugReportCallbackEXT,
    FuncId_vkDestroyDebugReportCallbackEXT,
    FuncId_vkDebugReportMessageEXT,

    FuncId_WholeCmdBuf,

#ifdef CODEXL_INCLUDE
        FuncId_vkMAX,
#else
        FuncId_MAX,
#endif
};

//-----------------------------------------------------------------------------
/// An object type enumeration that defines the types of Vulkan objects that GPS can wrap.
//-----------------------------------------------------------------------------
#ifndef CODEXL_INCLUDE
enum eObjectType : int
{
    kObjectType_Undefined = -1,

    kObjectType_Instance,
    kObjectType_Device,
    kObjectType_Semaphore,
    kObjectType_Event,
    kObjectType_Buffer,
    kObjectType_BufferView,
    kObjectType_Pipeline,
    kObjectType_PipelineLayout,
    kObjectType_Sampler,
    kObjectType_DescriptorSetLayout,
    kObjectType_DescriptorPool,
    kObjectType_ImageView,
    kObjectType_ShaderModule,
    kObjectType_PipelineCache,
    kObjectType_Framebuffer,
    kObjectType_RenderPass,
    kObjectType_QueryPool,
    kObjectType_CommandPool,
    kObjectType_Fence,
    kObjectType_Image,
    kObjectType_SurfaceKHR,
    kObjectType_SwapChainKHR,

    kObjectType_Begin_Range = kObjectType_Instance,
    kObjectType_End_Range = kObjectType_SwapChainKHR,
    kObjectType_Count = (kObjectType_End_Range - kObjectType_Begin_Range + 1)
};
#endif

#endif // __VKT_ENUMERATIONS_H__
