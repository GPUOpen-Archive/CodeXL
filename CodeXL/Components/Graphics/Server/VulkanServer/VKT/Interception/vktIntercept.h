//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktIntercept.h
/// \brief  API interception happens here.
//=============================================================================

#ifndef __VKT_INTERCEPT_H__
#define __VKT_INTERCEPT_H__

#include "../Util/vktUtil.h"

class VktWrappedCmdBuf;
class VktWrappedQueue;

/// Typedef for map of command buffer wrappers
typedef std::unordered_map<VkCommandBuffer, VktWrappedCmdBuf*> WrappedCmdBufMap;

/// Typedef for map of queue wrappers
typedef std::unordered_map<VkQueue, VktWrappedQueue*> WrappedQueueMap;

VktWrappedCmdBuf* GetWrappedCmdBuf(VkCommandBuffer cmdBuffer);
VktWrappedQueue* GetWrappedQueue(VkQueue queue);
const WrappedQueueMap& GetWrappedQueues();

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pCount, VkExtensionProperties* pProperties);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkEnumerateInstanceLayerProperties(uint32_t* pCount, VkLayerProperties* pProperties);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pCount, VkLayerProperties* pProperties);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkQueueWaitIdle(VkQueue queue);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkDeviceWaitIdle(VkDevice device);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkUnmapMemory(VkDevice device, VkDeviceMemory memory);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetFenceStatus(VkDevice device, VkFence fence);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetEventStatus(VkDevice device, VkEvent event);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkSetEvent(VkDevice device, VkEvent event);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkResetEvent(VkDevice device, VkEvent event);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkEndCommandBuffer(VkCommandBuffer commandBuffer);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const uint32_t* pData);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdEndRenderPass(VkCommandBuffer commandBuffer);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);

#ifdef VK_USE_PLATFORM_WIN32_KHR
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
VK_LAYER_EXPORT VKAPI_ATTR VkBool32 VKAPI_CALL Mine_vkGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
#endif  // VK_USE_PLATFORM_WIN32_KHR

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL Mine_vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
VK_LAYER_EXPORT VKAPI_ATTR void VKAPI_CALL Mine_vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage);

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* funcName);
VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* funcName);

#endif // __VKT_INTERCEPT_H__