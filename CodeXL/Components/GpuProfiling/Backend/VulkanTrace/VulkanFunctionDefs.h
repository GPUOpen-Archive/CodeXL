#ifndef _VK_FUNCTION_ENUM_DEFS_H_
#define _VK_FUNCTION_ENUM_DEFS_H_

#include <string>
#include <map>

using namespace std;




enum vkAPIType
{
    vkAPIType_Unknown = 0,
    vkAPIType_Command = 0x1,
    vkAPIType_CommandDraw = 0x2,
    vkAPIType_General = 0x4,
    vkAPIType_Create = 0x8,
    vkAPIType_Destroy = 0x10,
    vkAPIType_Get = 0x20,
    vkAPIType_Memory = 0x40,
    vkAPIType_Queue = 0x80,
    vkAPIType_Sync = 0x100,
    vkAPIType_KHR = 0x200,
};


enum vk_FUNC_TYPE
{
    vk_FUNC_TYPE_Unknown,
    vk_FUNC_TYPE_vkCreateInstance,
    vk_FUNC_TYPE_vkCreateDevice,
    vk_FUNC_TYPE_vkCreateFence,
    vk_FUNC_TYPE_vkCreateEvent,
    vk_FUNC_TYPE_vkCreateQueryPool,
    vk_FUNC_TYPE_vkCreateBuffer,
    vk_FUNC_TYPE_vkCreateBufferView,
    vk_FUNC_TYPE_vkCreateSemaphore,
    vk_FUNC_TYPE_vkCreateImage,
    vk_FUNC_TYPE_vkCreatePipelineLayout,
    vk_FUNC_TYPE_vkCreateShaderModule,
    vk_FUNC_TYPE_vkCreateSampler,
    vk_FUNC_TYPE_vkCreatePipelineCache,
    vk_FUNC_TYPE_vkCreateGraphicsPipelines,
    vk_FUNC_TYPE_vkCreateComputePipelines,
    vk_FUNC_TYPE_vkCreateShader,
    vk_FUNC_TYPE_vkCreateDescriptorSetLayout,
    vk_FUNC_TYPE_vkCreateCommandPool,
    vk_FUNC_TYPE_vkCreateCommandBuffer,
    vk_FUNC_TYPE_vkCreateFramebuffer,
    vk_FUNC_TYPE_vkCreateRenderPass,
    vk_FUNC_TYPE_vkCreateDescriptorPool,
    vk_FUNC_TYPE_vkCreateImageView,
    vk_FUNC_TYPE_vkDestroyInstance,
    vk_FUNC_TYPE_vkDestroyDevice,
    vk_FUNC_TYPE_vkDestroyEvent,
    vk_FUNC_TYPE_vkDestroySemaphore,
    vk_FUNC_TYPE_vkDestroyBuffer,
    vk_FUNC_TYPE_vkDestroyBufferView,
    vk_FUNC_TYPE_vkDestroyImage,
    vk_FUNC_TYPE_vkDestroyPipeline,
    vk_FUNC_TYPE_vkDestroyPipelineLayout,
    vk_FUNC_TYPE_vkDestroySampler,
    vk_FUNC_TYPE_vkDestroyFence,
    vk_FUNC_TYPE_vkDestroyCommandPool,
    vk_FUNC_TYPE_vkDestroyImageView,
    vk_FUNC_TYPE_vkDestroyShaderModule,
    vk_FUNC_TYPE_vkDestroyShader,
    vk_FUNC_TYPE_vkDestroyPipelineCache,
    vk_FUNC_TYPE_vkDestroyDescriptorSetLayout,
    vk_FUNC_TYPE_vkDestroyQueryPool,
    vk_FUNC_TYPE_vkDestroyCommandBuffer,
    vk_FUNC_TYPE_vkDestroyFramebuffer,
    vk_FUNC_TYPE_vkDestroyRenderPass,
    vk_FUNC_TYPE_vkDestroyDescriptorPool,
    vk_FUNC_TYPE_vkEnumeratePhysicalDevices,
    vk_FUNC_TYPE_vkEnumerateInstanceExtensionProperties,
    vk_FUNC_TYPE_vkEnumerateInstanceLayerProperties,
    vk_FUNC_TYPE_vkEnumerateDeviceLayerProperties,
    vk_FUNC_TYPE_vkGetPhysicalDeviceFeatures,
    vk_FUNC_TYPE_vkGetPhysicalDeviceFormatProperties,
    vk_FUNC_TYPE_vkGetPhysicalDeviceImageFormatProperties,
    vk_FUNC_TYPE_vkGetPhysicalDeviceProperties,
    vk_FUNC_TYPE_vkGetPhysicalDeviceQueueFamilyProperties,
    vk_FUNC_TYPE_vkGetPhysicalDeviceMemoryProperties,
    vk_FUNC_TYPE_vkGetDeviceQueue,
    vk_FUNC_TYPE_vkGetDeviceMemoryCommitment,
    vk_FUNC_TYPE_vkGetPhysicalDeviceSparseImageFormatProperties,
    vk_FUNC_TYPE_vkGetBufferMemoryRequirements,
    vk_FUNC_TYPE_vkGetImageMemoryRequirements,
    vk_FUNC_TYPE_vkGetImageSparseMemoryRequirements,
    vk_FUNC_TYPE_vkGetRenderAreaGranularity,
    vk_FUNC_TYPE_vkGetPipelineCacheSize,
    vk_FUNC_TYPE_vkGetPipelineCacheData,
    vk_FUNC_TYPE_vkGetQueryPoolResults,
    vk_FUNC_TYPE_vkGetImageSubresourceLayout,
    vk_FUNC_TYPE_vkAllocMemory,
    vk_FUNC_TYPE_vkFreeMemory,
    vk_FUNC_TYPE_vkMapMemory,
    vk_FUNC_TYPE_vkUnmapMemory,
    vk_FUNC_TYPE_vkFlushMappedMemoryRanges,
    vk_FUNC_TYPE_vkInvalidateMappedMemoryRanges,
    vk_FUNC_TYPE_vkBindBufferMemory,
    vk_FUNC_TYPE_vkBindImageMemory,
    vk_FUNC_TYPE_vkQueueBindSparseBufferMemory,
    vk_FUNC_TYPE_vkQueueBindSparseImageOpaqueMemory,
    vk_FUNC_TYPE_vkQueueBindSparseImageMemory,
    vk_FUNC_TYPE_vkMergePipelineCaches,
    vk_FUNC_TYPE_vkResetDescriptorPool,
    vk_FUNC_TYPE_vkAllocDescriptorSets,
    vk_FUNC_TYPE_vkFreeDescriptorSets,
    vk_FUNC_TYPE_vkUpdateDescriptorSets,
    vk_FUNC_TYPE_vkQueueSubmit,
    vk_FUNC_TYPE_vkResetCommandPool,
    vk_FUNC_TYPE_vkBeginCommandBuffer,
    vk_FUNC_TYPE_vkEndCommandBuffer,
    vk_FUNC_TYPE_vkResetCommandBuffer,
    vk_FUNC_TYPE_vkCmdDraw,
    vk_FUNC_TYPE_vkCmdDrawIndexed,
    vk_FUNC_TYPE_vkCmdDrawIndirect,
    vk_FUNC_TYPE_vkCmdDrawIndexedIndirect,
    vk_FUNC_TYPE_vkCmdDispatch,
    vk_FUNC_TYPE_vkCmdDispatchIndirect,
    vk_FUNC_TYPE_vkCmdBindPipeline,
    vk_FUNC_TYPE_vkCmdBindDescriptorSets,
    vk_FUNC_TYPE_vkCmdBindIndexBuffer,
    vk_FUNC_TYPE_vkCmdBindVertexBuffers,
    vk_FUNC_TYPE_vkCmdCopyBuffer,
    vk_FUNC_TYPE_vkCmdCopyImage,
    vk_FUNC_TYPE_vkCmdBlitImage,
    vk_FUNC_TYPE_vkCmdCopyBufferToImage,
    vk_FUNC_TYPE_vkCmdCopyImageToBuffer,
    vk_FUNC_TYPE_vkCmdUpdateBuffer,
    vk_FUNC_TYPE_vkCmdFillBuffer,
    vk_FUNC_TYPE_vkCmdClearColorImage,
    vk_FUNC_TYPE_vkCmdClearDepthStencilImage,
    vk_FUNC_TYPE_vkCmdClearColorAttachment,
    vk_FUNC_TYPE_vkCmdClearDepthStencilAttachment,
    vk_FUNC_TYPE_vkCmdResolveImage,
    vk_FUNC_TYPE_vkCmdSetEvent,
    vk_FUNC_TYPE_vkCmdResetEvent,
    vk_FUNC_TYPE_vkCmdWaitEvents,
    vk_FUNC_TYPE_vkCmdPipelineBarrier,
    vk_FUNC_TYPE_vkCmdBeginQuery,
    vk_FUNC_TYPE_vkCmdEndQuery,
    vk_FUNC_TYPE_vkCmdResetQueryPool,
    vk_FUNC_TYPE_vkCmdWriteTimestamp,
    vk_FUNC_TYPE_vkCmdCopyQueryPoolResults,
    vk_FUNC_TYPE_vkCmdBeginRenderPass,
    vk_FUNC_TYPE_vkCmdNextSubpass,
    vk_FUNC_TYPE_vkCmdPushConstants,
    vk_FUNC_TYPE_vkCmdEndRenderPass,
    vk_FUNC_TYPE_vkCmdExecuteCommands,
    vk_FUNC_TYPE_vkCmdSetViewport,
    vk_FUNC_TYPE_vkCmdSetScissor,
    vk_FUNC_TYPE_vkCmdSetLineWidth,
    vk_FUNC_TYPE_vkCmdSetDepthBias,
    vk_FUNC_TYPE_vkCmdSetBlendConstants,
    vk_FUNC_TYPE_vkCmdSetDepthBounds,
    vk_FUNC_TYPE_vkCmdSetStencilCompareMask,
    vk_FUNC_TYPE_vkCmdSetStencilWriteMask,
    vk_FUNC_TYPE_vkCmdSetStencilReference,
    vk_FUNC_TYPE_vkQueueWaitIdle,
    vk_FUNC_TYPE_vkQueueSignalSemaphore,
    vk_FUNC_TYPE_vkQueueWaitSemaphore,
    vk_FUNC_TYPE_vkResetFences,
    vk_FUNC_TYPE_vkGetFenceStatus,
    vk_FUNC_TYPE_vkWaitForFences,
    vk_FUNC_TYPE_vkGetEventStatus,
    vk_FUNC_TYPE_vkSetEvent,
    vk_FUNC_TYPE_vkResetEvent,
    vk_FUNC_TYPE_vkDeviceWaitIdle,
    vk_FUNC_TYPE_vkGetPhysicalDeviceSurfaceSupportKHR,
    vk_FUNC_TYPE_vkGetSurfacePropertiesKHR,
    vk_FUNC_TYPE_vkGetSurfaceFormatsKHR,
    vk_FUNC_TYPE_vkGetSurfacePresentModesKHR,
    vk_FUNC_TYPE_vkCreateSwapchainKHR,
    vk_FUNC_TYPE_vkDestroySwapchainKHR,
    vk_FUNC_TYPE_vkGetSwapchainImagesKHR,
    vk_FUNC_TYPE_vkAcquireNextImageKHR,
    vk_FUNC_TYPE_vkQueuePresentKHR
};


// ----------------------------------------------------------------------------------
// Class Name: vulkanFunctionDefs Utility class handling the types and strings
//             for vulkan API functions
// ----------------------------------------------------------------------------------
class vulkanFunctionDefs
{
public:

    /// Convert vulkan API name string to enum
    /// \param strName API name string
    /// \return enum representation of vulkan API
    static vk_FUNC_TYPE ToVKFuncType(const std::string& strName);

    /// Extract the function type from the function name
    /// \param strAPIName the API name
    /// \return vkAPIType enumeration describing the API function type
    static vkAPIType TovkAPIType(const std::string& strAPIName);

    /// Extract the function type from the function name
    /// \param apiType the API type
    /// \param strAPIName[output] the API function name
    /// \return true for success
    static bool vkAPITypeToString(vk_FUNC_TYPE apiType, std::string& strAPIName);

private:

    /// Initializes the maps
    static void Initialize();

    /// Utility function. Adds an interface function to the maps
    /// \param apiStr the function name
    /// \param the enumeration for the API function
    /// \param apiType the enumerated type of the API function
    static void AddInterfaceFunction(const std::string& apiStr, vk_FUNC_TYPE functionType, vkAPIType apiType);

    static void InitCreateAPI();
    static void InitDestroyAPI();
    static void InitGetAPI();
    static void InitMemAPI();
    static void InitGeneralAPI();
    static void InitQueueAPI();
    static void InitCmdDrawcallAPI();
    static void InitCmdGeneralAPI();
    static void InitSyncAPI();
    static void InitKHRAPI();

    /// True iff the static members are already initialized
    static bool m_sIsInitialized;

    // Map from string to vulkan function type
    static map<string, vk_FUNC_TYPE> m_sVKAPIMap;

    // Map from vulkan function type to string
    static map<vk_FUNC_TYPE, std::string> m_sVKAPIStringsMap;

    // Map from string to vulkan API type
    static map<string, vkAPIType> m_sVKAPITypeMap;
    static string m_sMissingInterfaces;
};


#endif //_VK_FUNCTION_ENUM_DEFS_H_


