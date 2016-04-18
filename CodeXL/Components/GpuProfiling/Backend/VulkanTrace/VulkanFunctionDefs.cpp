#include "VulkanFunctionDefs.h"
#include "Logger.h"

using namespace std;
using namespace GPULogger;

bool vulkanFunctionDefs::m_sIsInitialized = false;
map<string, vk_FUNC_TYPE> vulkanFunctionDefs::m_sVKAPIMap;
map<vk_FUNC_TYPE, string> vulkanFunctionDefs::m_sVKAPIStringsMap;
map<string, vkAPIType> vulkanFunctionDefs::m_sVKAPITypeMap;
string vulkanFunctionDefs::m_sMissingInterfaces;

void vulkanFunctionDefs::Initialize()
{
    if (!m_sIsInitialized)
    {
        m_sIsInitialized = true;

        InitCreateAPI();
        InitDestroyAPI();
        InitGetAPI();
        InitMemAPI();
        InitGeneralAPI();
        InitQueueAPI();
        InitCmdDrawcallAPI();
        InitCmdGeneralAPI();
        InitSyncAPI();
        InitKHRAPI();
    }
}

vk_FUNC_TYPE vulkanFunctionDefs::ToVKFuncType(const std::string& strName)
{
    vk_FUNC_TYPE retVal = vk_FUNC_TYPE_Unknown;

    // Make sure mappings are initialized
    Initialize();

    // Find the API in the map
    map<string, vk_FUNC_TYPE>::iterator it = m_sVKAPIMap.find(strName);

    if (it != m_sVKAPIMap.end())
    {
        retVal = it->second;
    }

    return retVal;
}

vkAPIType vulkanFunctionDefs::TovkAPIType(const std::string& strAPIName)
{
    vkAPIType retVal = vkAPIType_Unknown;

    // Make sure mappings are initialized
    Initialize();

    // Find the API in the map
    map<string, vkAPIType>::iterator it = m_sVKAPITypeMap.find(strAPIName);

    if (it == m_sVKAPITypeMap.end())
    {
        m_sMissingInterfaces.append(strAPIName);
        m_sMissingInterfaces.append("\n");
    }

    if (it != m_sVKAPITypeMap.end())
    {
        retVal = it->second;
    }

    return retVal;
}

bool vulkanFunctionDefs::vkAPITypeToString(vk_FUNC_TYPE apiType, std::string& strAPIName)
{
    bool retVal = false;

    // Make sure mappings are initialized
    Initialize();

    // Find the API in the map
    map<vk_FUNC_TYPE, string>::iterator it = m_sVKAPIStringsMap.find(apiType);

    if (it != m_sVKAPIStringsMap.end())
    {
        strAPIName = it->second;
        retVal = true;
    }

    return retVal;
}

void vulkanFunctionDefs::AddInterfaceFunction(const std::string& apiStr, vk_FUNC_TYPE functionType, vkAPIType apiType)
{
    // Make sure that the string and type were not added yet (avoid copy paste mistakes)
    SpAssert(m_sVKAPIMap.find(apiStr) == m_sVKAPIMap.end());
    SpAssert(m_sVKAPIStringsMap.find(functionType) == m_sVKAPIStringsMap.end());
    SpAssert(m_sVKAPITypeMap.find(apiStr) == m_sVKAPITypeMap.end());

    m_sVKAPIMap.insert(pair<string, vk_FUNC_TYPE>(apiStr, functionType));
    m_sVKAPIStringsMap.insert(pair<vk_FUNC_TYPE, string>(functionType, apiStr));
    m_sVKAPITypeMap.insert(pair<string, vkAPIType>(apiStr, apiType));
}


void vulkanFunctionDefs::InitCreateAPI()
{
    // create API functions
    AddInterfaceFunction(string("vkCreateInstance"), vk_FUNC_TYPE_vkCreateInstance, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateDevice"), vk_FUNC_TYPE_vkCreateDevice, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateFence"), vk_FUNC_TYPE_vkCreateFence, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateEvent"), vk_FUNC_TYPE_vkCreateEvent, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateQueryPool"), vk_FUNC_TYPE_vkCreateQueryPool, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateBuffer"), vk_FUNC_TYPE_vkCreateBuffer, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateBufferView"), vk_FUNC_TYPE_vkCreateBufferView, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateSemaphore"), vk_FUNC_TYPE_vkCreateSemaphore, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateImage"), vk_FUNC_TYPE_vkCreateImage, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreatePipelineLayout"), vk_FUNC_TYPE_vkCreatePipelineLayout, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateShaderModule"), vk_FUNC_TYPE_vkCreateShaderModule, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateSampler"), vk_FUNC_TYPE_vkCreateSampler, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreatePipelineCache"), vk_FUNC_TYPE_vkCreatePipelineCache, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateGraphicsPipelines"), vk_FUNC_TYPE_vkCreateGraphicsPipelines, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateComputePipelines"), vk_FUNC_TYPE_vkCreateComputePipelines, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateShader"), vk_FUNC_TYPE_vkCreateShader, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateDescriptorSetLayout"), vk_FUNC_TYPE_vkCreateDescriptorSetLayout, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateCommandPool"), vk_FUNC_TYPE_vkCreateCommandPool, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateCommandBuffer"), vk_FUNC_TYPE_vkCreateCommandBuffer, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateFramebuffer"), vk_FUNC_TYPE_vkCreateFramebuffer, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateRenderPass"), vk_FUNC_TYPE_vkCreateRenderPass, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateDescriptorPool"), vk_FUNC_TYPE_vkCreateDescriptorPool, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateImageView"), vk_FUNC_TYPE_vkCreateImageView, vkAPIType_Create);
}
void vulkanFunctionDefs::InitDestroyAPI()
{
    AddInterfaceFunction(string("vkDestroyInstance"), vk_FUNC_TYPE_vkDestroyInstance, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyDevice"), vk_FUNC_TYPE_vkDestroyDevice, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyEvent"), vk_FUNC_TYPE_vkDestroyEvent, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroySemaphore"), vk_FUNC_TYPE_vkDestroySemaphore, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyBuffer"), vk_FUNC_TYPE_vkDestroyBuffer, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyBufferView"), vk_FUNC_TYPE_vkDestroyBufferView, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyImage"), vk_FUNC_TYPE_vkDestroyImage, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyPipeline"), vk_FUNC_TYPE_vkDestroyPipeline, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyPipelineLayout"), vk_FUNC_TYPE_vkDestroyPipelineLayout, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroySampler"), vk_FUNC_TYPE_vkDestroySampler, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyFence"), vk_FUNC_TYPE_vkDestroyFence, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyCommandPool"), vk_FUNC_TYPE_vkDestroyCommandPool, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyImageView"), vk_FUNC_TYPE_vkDestroyImageView, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyShaderModule"), vk_FUNC_TYPE_vkDestroyShaderModule, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyShader"), vk_FUNC_TYPE_vkDestroyShader, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyPipelineCache"), vk_FUNC_TYPE_vkDestroyPipelineCache, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyDescriptorSetLayout"), vk_FUNC_TYPE_vkDestroyDescriptorSetLayout, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyQueryPool"), vk_FUNC_TYPE_vkDestroyQueryPool, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyCommandBuffer"), vk_FUNC_TYPE_vkDestroyCommandBuffer, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyFramebuffer"), vk_FUNC_TYPE_vkDestroyFramebuffer, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyRenderPass"), vk_FUNC_TYPE_vkDestroyRenderPass, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyDescriptorPool"), vk_FUNC_TYPE_vkDestroyDescriptorPool, vkAPIType_Destroy);
}
void vulkanFunctionDefs::InitGetAPI()
{
    AddInterfaceFunction(string("vkEnumeratePhysicalDevices"), vk_FUNC_TYPE_vkEnumeratePhysicalDevices, vkAPIType_Get);
    AddInterfaceFunction(string("vkEnumerateInstanceExtensionProperties"), vk_FUNC_TYPE_vkEnumerateInstanceExtensionProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkEnumerateInstanceLayerProperties"), vk_FUNC_TYPE_vkEnumerateInstanceLayerProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkEnumerateDeviceLayerProperties"), vk_FUNC_TYPE_vkEnumerateDeviceLayerProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceFeatures"), vk_FUNC_TYPE_vkGetPhysicalDeviceFeatures, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceFormatProperties"), vk_FUNC_TYPE_vkGetPhysicalDeviceFormatProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceImageFormatProperties"), vk_FUNC_TYPE_vkGetPhysicalDeviceImageFormatProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceProperties"), vk_FUNC_TYPE_vkGetPhysicalDeviceProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceQueueFamilyProperties"), vk_FUNC_TYPE_vkGetPhysicalDeviceQueueFamilyProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceMemoryProperties"), vk_FUNC_TYPE_vkGetPhysicalDeviceMemoryProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetDeviceQueue"), vk_FUNC_TYPE_vkGetDeviceQueue, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetDeviceMemoryCommitment"), vk_FUNC_TYPE_vkGetDeviceMemoryCommitment, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceSparseImageFormatProperties"), vk_FUNC_TYPE_vkGetPhysicalDeviceSparseImageFormatProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetBufferMemoryRequirements"), vk_FUNC_TYPE_vkGetBufferMemoryRequirements, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetImageMemoryRequirements"), vk_FUNC_TYPE_vkGetImageMemoryRequirements, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetImageSparseMemoryRequirements"), vk_FUNC_TYPE_vkGetImageSparseMemoryRequirements, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetRenderAreaGranularity"), vk_FUNC_TYPE_vkGetRenderAreaGranularity, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPipelineCacheSize"), vk_FUNC_TYPE_vkGetPipelineCacheSize, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPipelineCacheData"), vk_FUNC_TYPE_vkGetPipelineCacheData, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetQueryPoolResults"), vk_FUNC_TYPE_vkGetQueryPoolResults, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetImageSubresourceLayout"), vk_FUNC_TYPE_vkGetImageSubresourceLayout, vkAPIType_Get);
}
void vulkanFunctionDefs::InitMemAPI()
{
    AddInterfaceFunction(string("vkAllocMemory"), vk_FUNC_TYPE_vkAllocMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkFreeMemory"), vk_FUNC_TYPE_vkFreeMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkMapMemory"), vk_FUNC_TYPE_vkMapMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkUnmapMemory"), vk_FUNC_TYPE_vkUnmapMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkFlushMappedMemoryRanges"), vk_FUNC_TYPE_vkFlushMappedMemoryRanges, vkAPIType_Memory);
    AddInterfaceFunction(string("vkInvalidateMappedMemoryRanges"), vk_FUNC_TYPE_vkInvalidateMappedMemoryRanges, vkAPIType_Memory);
    AddInterfaceFunction(string("vkBindBufferMemory"), vk_FUNC_TYPE_vkBindBufferMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkBindImageMemory"), vk_FUNC_TYPE_vkBindImageMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkQueueBindSparseBufferMemory"), vk_FUNC_TYPE_vkQueueBindSparseBufferMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkQueueBindSparseImageOpaqueMemory"), vk_FUNC_TYPE_vkQueueBindSparseImageOpaqueMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkQueueBindSparseImageMemory"), vk_FUNC_TYPE_vkQueueBindSparseImageMemory, vkAPIType_Memory);
}
void vulkanFunctionDefs::InitGeneralAPI()
{
    AddInterfaceFunction(string("vkMergePipelineCaches"), vk_FUNC_TYPE_vkMergePipelineCaches, vkAPIType_General);
    AddInterfaceFunction(string("vkResetDescriptorPool"), vk_FUNC_TYPE_vkResetDescriptorPool, vkAPIType_General);
    AddInterfaceFunction(string("vkAllocDescriptorSets"), vk_FUNC_TYPE_vkAllocDescriptorSets, vkAPIType_General);
    AddInterfaceFunction(string("vkFreeDescriptorSets"), vk_FUNC_TYPE_vkFreeDescriptorSets, vkAPIType_General);
    AddInterfaceFunction(string("vkUpdateDescriptorSets"), vk_FUNC_TYPE_vkUpdateDescriptorSets, vkAPIType_General);
}
void vulkanFunctionDefs::InitQueueAPI()
{
    AddInterfaceFunction(string("vkQueueSubmit"), vk_FUNC_TYPE_vkQueueSubmit, vkAPIType_Queue);
    AddInterfaceFunction(string("vkResetCommandPool"), vk_FUNC_TYPE_vkResetCommandPool, vkAPIType_Queue);
    AddInterfaceFunction(string("vkBeginCommandBuffer"), vk_FUNC_TYPE_vkBeginCommandBuffer, vkAPIType_Queue);
    AddInterfaceFunction(string("vkEndCommandBuffer"), vk_FUNC_TYPE_vkEndCommandBuffer, vkAPIType_Queue);
    AddInterfaceFunction(string("vkResetCommandBuffer"), vk_FUNC_TYPE_vkResetCommandBuffer, vkAPIType_Queue);
}
void vulkanFunctionDefs::InitCmdDrawcallAPI()
{
    AddInterfaceFunction(string("vkCmdDraw"), vk_FUNC_TYPE_vkCmdDraw, vkAPIType_CommandDraw);
    AddInterfaceFunction(string("vkCmdDrawIndexed"), vk_FUNC_TYPE_vkCmdDrawIndexed, vkAPIType_CommandDraw);
    AddInterfaceFunction(string("vkCmdDrawIndirect"), vk_FUNC_TYPE_vkCmdDrawIndirect, vkAPIType_CommandDraw);
    AddInterfaceFunction(string("vkCmdDrawIndexedIndirect"), vk_FUNC_TYPE_vkCmdDrawIndexedIndirect, vkAPIType_CommandDraw);
    AddInterfaceFunction(string("vkCmdDispatch"), vk_FUNC_TYPE_vkCmdDispatch, vkAPIType_CommandDraw);
    AddInterfaceFunction(string("vkCmdDispatchIndirect"), vk_FUNC_TYPE_vkCmdDispatchIndirect, vkAPIType_CommandDraw);
}

void vulkanFunctionDefs::InitCmdGeneralAPI()
{
    AddInterfaceFunction(string("vkCmdBindPipeline"), vk_FUNC_TYPE_vkCmdBindPipeline, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdBindDescriptorSets"), vk_FUNC_TYPE_vkCmdBindDescriptorSets, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdBindIndexBuffer"), vk_FUNC_TYPE_vkCmdBindIndexBuffer, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdBindVertexBuffers"), vk_FUNC_TYPE_vkCmdBindVertexBuffers, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdCopyBuffer"), vk_FUNC_TYPE_vkCmdCopyBuffer, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdCopyImage"), vk_FUNC_TYPE_vkCmdCopyImage, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdBlitImage"), vk_FUNC_TYPE_vkCmdBlitImage, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdCopyBufferToImage"), vk_FUNC_TYPE_vkCmdCopyBufferToImage, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdCopyImageToBuffer"), vk_FUNC_TYPE_vkCmdCopyImageToBuffer, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdUpdateBuffer"), vk_FUNC_TYPE_vkCmdUpdateBuffer, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdFillBuffer"), vk_FUNC_TYPE_vkCmdFillBuffer, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdClearColorImage"), vk_FUNC_TYPE_vkCmdClearColorImage, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdClearDepthStencilImage"), vk_FUNC_TYPE_vkCmdClearDepthStencilImage, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdClearColorAttachment"), vk_FUNC_TYPE_vkCmdClearColorAttachment, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdClearDepthStencilAttachment"), vk_FUNC_TYPE_vkCmdClearDepthStencilAttachment, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdResolveImage"), vk_FUNC_TYPE_vkCmdResolveImage, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdSetEvent"), vk_FUNC_TYPE_vkCmdSetEvent, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdResetEvent"), vk_FUNC_TYPE_vkCmdResetEvent, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdWaitEvents"), vk_FUNC_TYPE_vkCmdWaitEvents, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdPipelineBarrier"), vk_FUNC_TYPE_vkCmdPipelineBarrier, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdBeginQuery"), vk_FUNC_TYPE_vkCmdBeginQuery, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdEndQuery"), vk_FUNC_TYPE_vkCmdEndQuery, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdResetQueryPool"), vk_FUNC_TYPE_vkCmdResetQueryPool, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdWriteTimestamp"), vk_FUNC_TYPE_vkCmdWriteTimestamp, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdCopyQueryPoolResults"), vk_FUNC_TYPE_vkCmdCopyQueryPoolResults, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdBeginRenderPass"), vk_FUNC_TYPE_vkCmdBeginRenderPass, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdNextSubpass"), vk_FUNC_TYPE_vkCmdNextSubpass, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdPushConstants"), vk_FUNC_TYPE_vkCmdPushConstants, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdEndRenderPass"), vk_FUNC_TYPE_vkCmdEndRenderPass, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdExecuteCommands"), vk_FUNC_TYPE_vkCmdExecuteCommands, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdSetViewport"), vk_FUNC_TYPE_vkCmdSetViewport, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdSetScissor"), vk_FUNC_TYPE_vkCmdSetScissor, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdSetLineWidth"), vk_FUNC_TYPE_vkCmdSetLineWidth, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdSetDepthBias"), vk_FUNC_TYPE_vkCmdSetDepthBias, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdSetBlendConstants"), vk_FUNC_TYPE_vkCmdSetBlendConstants, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdSetDepthBounds"), vk_FUNC_TYPE_vkCmdSetDepthBounds, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdSetStencilCompareMask"), vk_FUNC_TYPE_vkCmdSetStencilCompareMask, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdSetStencilWriteMask"), vk_FUNC_TYPE_vkCmdSetStencilWriteMask, vkAPIType_Command);
    AddInterfaceFunction(string("vkCmdSetStencilReference"), vk_FUNC_TYPE_vkCmdSetStencilReference, vkAPIType_Command);
}
void vulkanFunctionDefs::InitSyncAPI()
{
    AddInterfaceFunction(string("vkQueueWaitIdle"), vk_FUNC_TYPE_vkQueueWaitIdle, vkAPIType_Sync);
    AddInterfaceFunction(string("vkQueueSignalSemaphore"), vk_FUNC_TYPE_vkQueueSignalSemaphore, vkAPIType_Sync);
    AddInterfaceFunction(string("vkQueueWaitSemaphore"), vk_FUNC_TYPE_vkQueueWaitSemaphore, vkAPIType_Sync);
    AddInterfaceFunction(string("vkResetFences"), vk_FUNC_TYPE_vkResetFences, vkAPIType_Sync);
    AddInterfaceFunction(string("vkGetFenceStatus"), vk_FUNC_TYPE_vkGetFenceStatus, vkAPIType_Sync);
    AddInterfaceFunction(string("vkWaitForFences"), vk_FUNC_TYPE_vkWaitForFences, vkAPIType_Sync);
    AddInterfaceFunction(string("vkGetEventStatus"), vk_FUNC_TYPE_vkGetEventStatus, vkAPIType_Sync);
    AddInterfaceFunction(string("vkSetEvent"), vk_FUNC_TYPE_vkSetEvent, vkAPIType_Sync);
    AddInterfaceFunction(string("vkResetEvent"), vk_FUNC_TYPE_vkResetEvent, vkAPIType_Sync);
    AddInterfaceFunction(string("vkDeviceWaitIdle"), vk_FUNC_TYPE_vkDeviceWaitIdle, vkAPIType_Sync);
}
void vulkanFunctionDefs::InitKHRAPI()
{
    AddInterfaceFunction(string("vkGetPhysicalDeviceSurfaceSupportKHR"), vk_FUNC_TYPE_vkGetPhysicalDeviceSurfaceSupportKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkGetSurfacePropertiesKHR"), vk_FUNC_TYPE_vkGetSurfacePropertiesKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkGetSurfaceFormatsKHR"), vk_FUNC_TYPE_vkGetSurfaceFormatsKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkGetSurfacePresentModesKHR"), vk_FUNC_TYPE_vkGetSurfacePresentModesKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkCreateSwapchainKHR"), vk_FUNC_TYPE_vkCreateSwapchainKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkDestroySwapchainKHR"), vk_FUNC_TYPE_vkDestroySwapchainKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkGetSwapchainImagesKHR"), vk_FUNC_TYPE_vkGetSwapchainImagesKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkAcquireNextImageKHR"), vk_FUNC_TYPE_vkAcquireNextImageKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkQueuePresentKHR"), vk_FUNC_TYPE_vkQueuePresentKHR, vkAPIType_KHR);

}