#include "VulkanFunctionDefs.h"
#include "Logger.h"

using namespace std;
using namespace GPULogger;

bool vulkanFunctionDefs::m_sIsInitialized = false;
map<string, VkFuncId> vulkanFunctionDefs::m_sVKAPIMap;
map<VkFuncId, string> vulkanFunctionDefs::m_sVKAPIStringsMap;
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
        InitDescriptorAPI();
        InitQueueSubmissionAPI();
        InitCmdBufProfiledAPI();
        InitCmdBufNonProfiledAPI();
        InitSyncAPI();
        InitKHRAPI();
    }
}

VkFuncId vulkanFunctionDefs::ToVKFuncType(const std::string& strName)
{
    VkFuncId retVal = FuncId_vkUNDEFINED;

    // Make sure mappings are initialized
    Initialize();

    // Find the API in the map
    std::map<std::string, VkFuncId>::iterator it = m_sVKAPIMap.find(strName);

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
    std::map<std::string, vkAPIType>::iterator it = m_sVKAPITypeMap.find(strAPIName);

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

bool vulkanFunctionDefs::vkAPITypeToString(VkFuncId apiType, std::string& strAPIName)
{
    bool retVal = false;

    // Make sure mappings are initialized
    Initialize();

    // Find the API in the map
    std::map<VkFuncId, std::string>::iterator it = m_sVKAPIStringsMap.find(apiType);

    if (it != m_sVKAPIStringsMap.end())
    {
        strAPIName = it->second;
        retVal = true;
    }

    return retVal;
}


vkAPIType vulkanFunctionDefs::GetAPIGroupFromAPI(VkFuncId inAPIFuncId)
{
    vkAPIType apiType = vkAPIType_Unknown;

    switch (inAPIFuncId)
    {
    case FuncId_vkCreateInstance:
    case FuncId_vkCreateDevice:
    case FuncId_vkCreateFence:
    case FuncId_vkCreateEvent:
    case FuncId_vkCreateBufferView:
    case FuncId_vkCreateQueryPool:
    case FuncId_vkCreateImage:
    case FuncId_vkCreateGraphicsPipelines:
    case FuncId_vkCreateComputePipelines:
    case FuncId_vkCreateBuffer:
    case FuncId_vkCreateImageView:
    case FuncId_vkCreateShaderModule:
    case FuncId_vkCreateSemaphore:
    case FuncId_vkCreatePipelineCache:
    case FuncId_vkCreateSampler:
    case FuncId_vkCreateDescriptorSetLayout:
    case FuncId_vkCreatePipelineLayout:
    case FuncId_vkCreateFramebuffer:
    case FuncId_vkCreateRenderPass:
    case FuncId_vkCreateCommandPool:
    case FuncId_vkCreateDescriptorPool:
        apiType = vkAPIType_Create;
        break;

    case FuncId_vkDestroyInstance:
    case FuncId_vkDestroyDevice:
    case FuncId_vkDestroySemaphore:
    case FuncId_vkDestroyEvent:
    case FuncId_vkDestroyBuffer:
    case FuncId_vkDestroyBufferView:
    case FuncId_vkDestroyPipeline:
    case FuncId_vkDestroyPipelineLayout:
    case FuncId_vkDestroySampler:
    case FuncId_vkDestroyDescriptorSetLayout:
    case FuncId_vkDestroyDescriptorPool:
    case FuncId_vkDestroyImageView:
    case FuncId_vkDestroyShaderModule:
    case FuncId_vkDestroyPipelineCache:
    case FuncId_vkDestroyFramebuffer:
    case FuncId_vkDestroyRenderPass:
    case FuncId_vkDestroyQueryPool:
    case FuncId_vkDestroyCommandPool:
    case FuncId_vkDestroyFence:
    case FuncId_vkDestroyImage:
        apiType = vkAPIType_Destroy;
        break;

    case FuncId_vkEnumeratePhysicalDevices:
    case FuncId_vkEnumerateInstanceExtensionProperties:
    case FuncId_vkEnumerateInstanceLayerProperties:
    case FuncId_vkEnumerateDeviceLayerProperties:
    case FuncId_vkGetPhysicalDeviceFeatures:
    case FuncId_vkGetPhysicalDeviceFormatProperties:
    case FuncId_vkGetPhysicalDeviceImageFormatProperties:
    case FuncId_vkGetPhysicalDeviceProperties:
    case FuncId_vkGetPhysicalDeviceQueueFamilyProperties:
    case FuncId_vkGetPhysicalDeviceMemoryProperties:
    case FuncId_vkGetDeviceQueue:
    case FuncId_vkGetBufferMemoryRequirements:
    case FuncId_vkGetImageMemoryRequirements:
    case FuncId_vkGetImageSparseMemoryRequirements:
    case FuncId_vkGetPhysicalDeviceSparseImageFormatProperties:
    case FuncId_vkGetQueryPoolResults:
    case FuncId_vkGetImageSubresourceLayout:
    case FuncId_vkGetPipelineCacheData:
    case FuncId_vkGetRenderAreaGranularity:
    case FuncId_vkGetDeviceMemoryCommitment:
        apiType = vkAPIType_Get;
        break;

    case FuncId_vkAllocateMemory:
    case FuncId_vkFreeMemory:
    case FuncId_vkMapMemory:
    case FuncId_vkUnmapMemory:
    case FuncId_vkBindBufferMemory:
    case FuncId_vkBindImageMemory:
    case FuncId_vkFlushMappedMemoryRanges:
    case FuncId_vkInvalidateMappedMemoryRanges:
    case FuncId_vkMergePipelineCaches:
        apiType = vkAPIType_Memory;
        break;

    case FuncId_vkResetDescriptorPool:
    case FuncId_vkAllocateDescriptorSets:
    case FuncId_vkFreeDescriptorSets:
    case FuncId_vkUpdateDescriptorSets:
        apiType = vkAPIType_DescriptorSet;
        break;

    case FuncId_vkQueueSubmit:
    case FuncId_vkQueueBindSparse:
    case FuncId_vkResetCommandPool:
    case FuncId_vkAllocateCommandBuffers:
    case FuncId_vkFreeCommandBuffers:
    case FuncId_vkBeginCommandBuffer:
    case FuncId_vkEndCommandBuffer:
    case FuncId_vkResetCommandBuffer:
        apiType = vkAPIType_QueueSubmission;
        break;

    case FuncId_vkCmdDraw:
    case FuncId_vkCmdDrawIndexed:
    case FuncId_vkCmdDrawIndirect:
    case FuncId_vkCmdDrawIndexedIndirect:
    case FuncId_vkCmdDispatch:
    case FuncId_vkCmdDispatchIndirect:
    case FuncId_vkCmdCopyBuffer:
    case FuncId_vkCmdCopyImage:
    case FuncId_vkCmdBlitImage:
    case FuncId_vkCmdCopyBufferToImage:
    case FuncId_vkCmdCopyImageToBuffer:
    case FuncId_vkCmdUpdateBuffer:
    case FuncId_vkCmdFillBuffer:
    case FuncId_vkCmdClearColorImage:
    case FuncId_vkCmdClearDepthStencilImage:
    case FuncId_vkCmdClearAttachments:
    case FuncId_vkCmdResolveImage:
    case FuncId_vkCmdResetQueryPool:
    case FuncId_vkCmdCopyQueryPoolResults:
    case FuncId_vkCmdExecuteCommands:
    case FuncId_vkCmdPipelineBarrier:
    case FuncId_vkCmdPushConstants:
    case FuncId_vkCmdBeginRenderPass:
    case FuncId_vkCmdNextSubpass:
    case FuncId_vkCmdEndRenderPass:
    case FuncId_vkCmdWaitEvents:
        apiType = vkAPIType_CmdBufProfiled;
        break;

    case FuncId_vkCmdBindPipeline:
    case FuncId_vkCmdSetViewport:
    case FuncId_vkCmdSetScissor:
    case FuncId_vkCmdSetLineWidth:
    case FuncId_vkCmdSetDepthBias:
    case FuncId_vkCmdSetBlendConstants:
    case FuncId_vkCmdSetDepthBounds:
    case FuncId_vkCmdSetStencilCompareMask:
    case FuncId_vkCmdSetStencilWriteMask:
    case FuncId_vkCmdSetStencilReference:
    case FuncId_vkCmdBindDescriptorSets:
    case FuncId_vkCmdBindIndexBuffer:
    case FuncId_vkCmdBindVertexBuffers:
    case FuncId_vkCmdSetEvent:
    case FuncId_vkCmdResetEvent:
    case FuncId_vkCmdBeginQuery:
    case FuncId_vkCmdEndQuery:
    case FuncId_vkCmdWriteTimestamp:
        apiType = vkAPIType_CmdBufNonProfiled;
        break;

    case FuncId_vkQueueWaitIdle:
    case FuncId_vkDeviceWaitIdle:
    case FuncId_vkResetFences:
    case FuncId_vkGetFenceStatus:
    case FuncId_vkWaitForFences:
    case FuncId_vkGetEventStatus:
    case FuncId_vkSetEvent:
    case FuncId_vkResetEvent:
        apiType = vkAPIType_Sync;
        break;

    case FuncId_vkDestroySurfaceKHR:
    case FuncId_vkCreateSwapchainKHR:
    case FuncId_vkDestroySwapchainKHR:
    case FuncId_vkGetSwapchainImagesKHR:
    case FuncId_vkAcquireNextImageKHR:
    case FuncId_vkQueuePresentKHR:
    case FuncId_vkGetPhysicalDeviceSurfaceSupportKHR:
    case FuncId_vkGetPhysicalDeviceSurfaceCapabilitiesKHR:
    case FuncId_vkGetPhysicalDeviceSurfaceFormatsKHR:
    case FuncId_vkGetPhysicalDeviceSurfacePresentModesKHR:
    case FuncId_vkCreateWin32SurfaceKHR:
    case FuncId_vkGetPhysicalDeviceWin32PresentationSupportKHR:
        apiType = vkAPIType_KHR;
        break;

    default:
        apiType = vkAPIType_Unknown;
    }
    return apiType;

}

void vulkanFunctionDefs::AddInterfaceFunction(const std::string& apiStr, VkFuncId functionType, vkAPIType apiType)
{
    // Make sure that the string and type were not added yet (avoid copy paste mistakes)
    SpAssert(m_sVKAPIMap.find(apiStr) == m_sVKAPIMap.end());
    SpAssert(m_sVKAPIStringsMap.find(functionType) == m_sVKAPIStringsMap.end());
    SpAssert(m_sVKAPITypeMap.find(apiStr) == m_sVKAPITypeMap.end());

    m_sVKAPIMap.insert(pair<string, VkFuncId>(apiStr, functionType));
    m_sVKAPIStringsMap.insert(pair<VkFuncId, string>(functionType, apiStr));
    m_sVKAPITypeMap.insert(pair<string, vkAPIType>(apiStr, apiType));
}


void vulkanFunctionDefs::InitCreateAPI()
{
    // Create API functions
    AddInterfaceFunction(string("vkCreateInstance"), FuncId_vkCreateInstance, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateDevice"), FuncId_vkCreateDevice, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateFence"), FuncId_vkCreateFence, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateEvent"), FuncId_vkCreateEvent, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateBufferView"), FuncId_vkCreateBufferView, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateQueryPool"), FuncId_vkCreateQueryPool, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateImage"), FuncId_vkCreateImage, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateGraphicsPipelines"), FuncId_vkCreateGraphicsPipelines, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateComputePipelines"), FuncId_vkCreateComputePipelines, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateBuffer"), FuncId_vkCreateInstance, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateImageView"), FuncId_vkCreateImageView, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateShaderModule"), FuncId_vkCreateShaderModule, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateSemaphore"), FuncId_vkCreateSemaphore, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreatePipelineCache"), FuncId_vkCreatePipelineCache, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateSampler"), FuncId_vkCreateSampler, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateDescriptorSetLayout"), FuncId_vkCreateDescriptorSetLayout, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreatePipelineLayout"), FuncId_vkCreateInstance, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateFramebuffer"), FuncId_vkCreateFramebuffer, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateRenderPass"), FuncId_vkCreateInstance, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateCommandPool"), FuncId_vkCreateCommandPool, vkAPIType_Create);
    AddInterfaceFunction(string("vkCreateDescriptorPool"), FuncId_vkCreateDescriptorPool, vkAPIType_Create);
}
void vulkanFunctionDefs::InitDestroyAPI()
{
    AddInterfaceFunction(string("vkDestroyInstance"), FuncId_vkDestroyInstance, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyDevice"), FuncId_vkDestroyDevice, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroySemaphore"), FuncId_vkDestroySemaphore, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyEvent"), FuncId_vkDestroyEvent, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyBuffer"), FuncId_vkDestroyBuffer, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyBufferView"), FuncId_vkDestroyBufferView, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyPipeline"), FuncId_vkDestroyPipeline, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyPipelineLayout"), FuncId_vkDestroyPipelineLayout, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroySampler"), FuncId_vkDestroySampler, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyDescriptorSetLayout"), FuncId_vkDestroyDescriptorSetLayout, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyDescriptorPool"), FuncId_vkDestroyDescriptorPool, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyImageView"), FuncId_vkDestroyImageView, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyShaderModule"), FuncId_vkDestroyShaderModule, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyPipelineCache"), FuncId_vkDestroyPipelineCache, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyFramebuffer"), FuncId_vkDestroyFramebuffer, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyRenderPass"), FuncId_vkDestroyRenderPass, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyQueryPool"), FuncId_vkDestroyQueryPool, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyCommandPool"), FuncId_vkDestroyCommandPool, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyFence"), FuncId_vkDestroyFence, vkAPIType_Destroy);
    AddInterfaceFunction(string("vkDestroyImage"), FuncId_vkDestroyImage, vkAPIType_Destroy);
}
void vulkanFunctionDefs::InitGetAPI()
{
    AddInterfaceFunction(string("vkEnumeratePhysicalDevices"), FuncId_vkEnumeratePhysicalDevices, vkAPIType_Get);
    AddInterfaceFunction(string("vkEnumerateInstanceExtensionProperties"), FuncId_vkEnumerateInstanceExtensionProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkEnumerateInstanceLayerProperties"), FuncId_vkEnumerateInstanceLayerProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkEnumerateDeviceLayerProperties"), FuncId_vkEnumerateDeviceLayerProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceFeatures"), FuncId_vkGetPhysicalDeviceFeatures, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceFormatProperties"), FuncId_vkGetPhysicalDeviceFormatProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceImageFormatProperties"), FuncId_vkGetPhysicalDeviceImageFormatProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceProperties"), FuncId_vkGetPhysicalDeviceProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceQueueFamilyProperties"), FuncId_vkGetPhysicalDeviceQueueFamilyProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceMemoryProperties"), FuncId_vkGetPhysicalDeviceMemoryProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetDeviceQueue"), FuncId_vkGetDeviceQueue, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetBufferMemoryRequirements"), FuncId_vkGetBufferMemoryRequirements, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetImageMemoryRequirements"), FuncId_vkGetImageMemoryRequirements, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetImageSparseMemoryRequirements"), FuncId_vkGetImageSparseMemoryRequirements, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPhysicalDeviceSparseImageFormatProperties"), FuncId_vkGetPhysicalDeviceSparseImageFormatProperties, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetQueryPoolResults"), FuncId_vkGetQueryPoolResults, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetImageSubresourceLayout"), FuncId_vkGetImageSubresourceLayout, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetPipelineCacheData"), FuncId_vkGetPipelineCacheData, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetRenderAreaGranularity"), FuncId_vkGetRenderAreaGranularity, vkAPIType_Get);
    AddInterfaceFunction(string("vkGetDeviceMemoryCommitment"), FuncId_vkGetDeviceMemoryCommitment, vkAPIType_Get);
}
void vulkanFunctionDefs::InitMemAPI()
{
    AddInterfaceFunction(string("vkAllocateMemory"), FuncId_vkAllocateMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkFreeMemory"), FuncId_vkFreeMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkMapMemory"), FuncId_vkMapMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkUnmapMemory"), FuncId_vkUnmapMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkBindBufferMemory"), FuncId_vkBindBufferMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkBindImageMemory"), FuncId_vkBindImageMemory, vkAPIType_Memory);
    AddInterfaceFunction(string("vkFlushMappedMemoryRanges"), FuncId_vkFlushMappedMemoryRanges, vkAPIType_Memory);
    AddInterfaceFunction(string("vkInvalidateMappedMemoryRanges"), FuncId_vkInvalidateMappedMemoryRanges, vkAPIType_Memory);
    AddInterfaceFunction(string("vkMergePipelineCaches"), FuncId_vkMergePipelineCaches, vkAPIType_Memory);
}
void vulkanFunctionDefs::InitDescriptorAPI()
{
    AddInterfaceFunction(string("vkResetDescriptorPool"), FuncId_vkResetDescriptorPool, vkAPIType_DescriptorSet);
    AddInterfaceFunction(string("vkAllocateDescriptorSets"), FuncId_vkAllocateDescriptorSets, vkAPIType_DescriptorSet);
    AddInterfaceFunction(string("vkFreeDescriptorSets"), FuncId_vkFreeDescriptorSets, vkAPIType_DescriptorSet);
    AddInterfaceFunction(string("vkUpdateDescriptorSets"), FuncId_vkUpdateDescriptorSets, vkAPIType_DescriptorSet);
}

void vulkanFunctionDefs::InitQueueSubmissionAPI()
{
    AddInterfaceFunction(string("vkQueueSubmit"), FuncId_vkQueueSubmit, vkAPIType_QueueSubmission);
    AddInterfaceFunction(string("vkQueueBindSparse"), FuncId_vkQueueBindSparse, vkAPIType_QueueSubmission);
    AddInterfaceFunction(string("vkResetCommandPool"), FuncId_vkResetCommandPool, vkAPIType_QueueSubmission);
    AddInterfaceFunction(string("vkAllocateCommandBuffers"), FuncId_vkAllocateCommandBuffers, vkAPIType_QueueSubmission);
    AddInterfaceFunction(string("vkFreeCommandBuffers"), FuncId_vkFreeCommandBuffers, vkAPIType_QueueSubmission);
    AddInterfaceFunction(string("vkBeginCommandBuffer"), FuncId_vkBeginCommandBuffer, vkAPIType_QueueSubmission);
    AddInterfaceFunction(string("vkEndCommandBuffer"), FuncId_vkEndCommandBuffer, vkAPIType_QueueSubmission);
    AddInterfaceFunction(string("vkResetCommandBuffer"), FuncId_vkResetCommandBuffer, vkAPIType_QueueSubmission);
}

void vulkanFunctionDefs::InitCmdBufProfiledAPI()
{
    AddInterfaceFunction(string("vkCmdDraw"), FuncId_vkCmdDraw, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdDrawIndexed"), FuncId_vkCmdDrawIndexed, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdDrawIndirect"), FuncId_vkCmdDrawIndirect, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdDrawIndexedIndirect"), FuncId_vkCmdDrawIndexedIndirect, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdDispatch"), FuncId_vkCmdDispatch, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdDispatchIndirect"), FuncId_vkCmdDispatchIndirect, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdCopyBuffer"), FuncId_vkCmdCopyBuffer, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdCopyImage"), FuncId_vkCmdCopyImage, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdBlitImage"), FuncId_vkCmdBlitImage, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdCopyBufferToImage"), FuncId_vkCmdCopyBufferToImage, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdCopyImageToBuffer"), FuncId_vkCmdCopyImageToBuffer, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdUpdateBuffer"), FuncId_vkCmdUpdateBuffer, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdFillBuffer"), FuncId_vkCmdFillBuffer, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdClearColorImage"), FuncId_vkCmdClearColorImage, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdClearDepthStencilImage"), FuncId_vkCmdClearDepthStencilImage, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdClearAttachments"), FuncId_vkCmdClearAttachments, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdResolveImage"), FuncId_vkCmdResolveImage, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdResetQueryPool"), FuncId_vkCmdResetQueryPool, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdCopyQueryPoolResults"), FuncId_vkCmdCopyQueryPoolResults, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdExecuteCommands"), FuncId_vkCmdExecuteCommands, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdPipelineBarrier"), FuncId_vkCmdPipelineBarrier, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdPushConstants"), FuncId_vkCmdPushConstants, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdBeginRenderPass"), FuncId_vkCmdBeginRenderPass, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdNextSubpass"), FuncId_vkCmdNextSubpass, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdEndRenderPass"), FuncId_vkCmdEndRenderPass, vkAPIType_CmdBufProfiled);
    AddInterfaceFunction(string("vkCmdWaitEvents"), FuncId_vkCmdWaitEvents, vkAPIType_CmdBufProfiled);
}

void vulkanFunctionDefs::InitCmdBufNonProfiledAPI()
{
    AddInterfaceFunction(string("vkCmdBindPipeline"), FuncId_vkCmdBindPipeline, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdSetViewport"), FuncId_vkCmdSetViewport, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdSetScissor"), FuncId_vkCmdSetScissor, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdSetLineWidth"), FuncId_vkCmdSetLineWidth, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdSetDepthBias"), FuncId_vkCmdSetDepthBias, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdSetBlendConstants"), FuncId_vkCmdSetBlendConstants, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdSetDepthBounds"), FuncId_vkCmdSetDepthBounds, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdSetStencilCompareMask"), FuncId_vkCmdSetStencilCompareMask, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdSetStencilWriteMask"), FuncId_vkCmdSetStencilWriteMask, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdSetStencilReference"), FuncId_vkCmdSetStencilReference, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdBindDescriptorSets"), FuncId_vkCmdBindDescriptorSets, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdBindIndexBuffer"), FuncId_vkCmdBindIndexBuffer, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdBindVertexBuffers"), FuncId_vkCmdBindVertexBuffers, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdSetEvent"), FuncId_vkCmdSetEvent, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdResetEvent"), FuncId_vkCmdResetEvent, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdBeginQuery"), FuncId_vkCmdBeginQuery, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdEndQuery"), FuncId_vkCmdEndQuery, vkAPIType_CmdBufNonProfiled);
    AddInterfaceFunction(string("vkCmdWriteTimestamp"), FuncId_vkCmdWriteTimestamp, vkAPIType_CmdBufNonProfiled);
}

void vulkanFunctionDefs::InitSyncAPI()
{
    AddInterfaceFunction(string("vkQueueWaitIdle"), FuncId_vkQueueWaitIdle, vkAPIType_Sync);
    AddInterfaceFunction(string("vkDeviceWaitIdle"), FuncId_vkDeviceWaitIdle, vkAPIType_Sync);
    AddInterfaceFunction(string("vkResetFences"), FuncId_vkResetFences, vkAPIType_Sync);
    AddInterfaceFunction(string("vkGetFenceStatus"), FuncId_vkGetFenceStatus, vkAPIType_Sync);
    AddInterfaceFunction(string("vkWaitForFences"), FuncId_vkWaitForFences, vkAPIType_Sync);
    AddInterfaceFunction(string("vkGetEventStatus"), FuncId_vkGetEventStatus, vkAPIType_Sync);
    AddInterfaceFunction(string("vkSetEvent"), FuncId_vkSetEvent, vkAPIType_Sync);
    AddInterfaceFunction(string("vkResetEvent"), FuncId_vkResetEvent, vkAPIType_Sync);
}
void vulkanFunctionDefs::InitKHRAPI()
{
    AddInterfaceFunction(string("vkCreateSwapchainKHR"), FuncId_vkCreateSwapchainKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkDestroySwapchainKHR"), FuncId_vkDestroySwapchainKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkGetSwapchainImagesKHR"), FuncId_vkGetSwapchainImagesKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkAcquireNextImageKHR"), FuncId_vkAcquireNextImageKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkQueuePresentKHR"), FuncId_vkQueuePresentKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkGetPhysicalDeviceSurfaceSupportKHR"), FuncId_vkGetPhysicalDeviceSurfaceSupportKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkGetPhysicalDeviceSurfaceCapabilitiesKHR"), FuncId_vkGetPhysicalDeviceSurfaceCapabilitiesKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkGetPhysicalDeviceSurfaceFormatsKHR"), FuncId_vkGetPhysicalDeviceSurfaceFormatsKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkGetPhysicalDeviceSurfacePresentModesKHR"), FuncId_vkGetPhysicalDeviceSurfacePresentModesKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkCreateWin32SurfaceKHR"), FuncId_vkCreateWin32SurfaceKHR, vkAPIType_KHR);
    AddInterfaceFunction(string("vkGetPhysicalDeviceWin32PresentationSupportKHR"), FuncId_vkGetPhysicalDeviceWin32PresentationSupportKHR, vkAPIType_KHR);

}