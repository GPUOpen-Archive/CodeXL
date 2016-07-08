//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktTraceAnalyzerLayer.h
/// \brief  Implementation file for Vulkan trace analyzer layer.
///         This source file contains two utility classes (VktAPIEntry, VktThreadTraceData),
///         and one main class (VtkTraceAnalyzerLayer). Essential work is
///         performed here, such as sending API/GPU trace information to client
///         and managing profiler results.
//==============================================================================

#include <thread>
#include "vktTraceAnalyzerLayer.h"
#include "vktThreadTraceData.h"
#include "../vktLayerManager.h"
#include "../vktInterceptManager.h"
#include "../vktDefines.h"
#include "../Profiling/vktFrameProfilerLayer.h"
#include "../../../Common/misc.h"
#include "../../../Common/TypeToString.h"
#include "../Interception/vktIntercept.h"
#include "../Objects/Wrappers/vktWrappedCmdBuf.h"
#include "../Objects/Wrappers/vktWrappedQueue.h"

#ifdef WIN32

//-----------------------------------------------------------------------------
/// SortByStartTime
//-----------------------------------------------------------------------------
bool SortByStartTime(ProfilerResult*& lhs, ProfilerResult*& rhs)
{
    return lhs->timestampResult.rawClocks.start < rhs->timestampResult.rawClocks.start;
}

#else

typedef int(*compfn)(const void*, const void*);

//-----------------------------------------------------------------------------
/// SortByStartTime
//-----------------------------------------------------------------------------
int SortByStartTime(ProfilerResult* pLhs, ProfilerResult *pRhs)
{
    if (pLhs->timestampResult.rawClocks.start < pRhs->timestampResult.rawClocks.start)
    {
        return -1;
    }
    else if (pLhs->timestampResult.rawClocks.start > pRhs->timestampResult.rawClocks.start)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

#endif

//-----------------------------------------------------------------------------
/// Constructor.
//-----------------------------------------------------------------------------
VktTraceAnalyzerLayer::VktTraceAnalyzerLayer()
    : MultithreadedTraceAnalyzerLayer()
{
    mFunctionIndexToNameString[FuncId_vkCreateInstance] = "vkCreateInstance";
    mFunctionIndexToNameString[FuncId_vkDestroyInstance] = "vkDestroyInstance";
    mFunctionIndexToNameString[FuncId_vkEnumeratePhysicalDevices] = "vkEnumeratePhysicalDevices";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceFeatures] = "vkGetPhysicalDeviceFeatures";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceFormatProperties] = "vkGetPhysicalDeviceFormatProperties";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceImageFormatProperties] = "vkGetPhysicalDeviceImageFormatProperties";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceProperties] = "vkGetPhysicalDeviceProperties";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceQueueFamilyProperties] = "vkGetPhysicalDeviceQueueFamilyProperties";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceMemoryProperties] = "vkGetPhysicalDeviceMemoryProperties";
    mFunctionIndexToNameString[FuncId_vkCreateDevice] = "vkCreateDevice";
    mFunctionIndexToNameString[FuncId_vkDestroyDevice] = "vkDestroyDevice";
    mFunctionIndexToNameString[FuncId_vkEnumerateInstanceExtensionProperties] = "vkEnumerateInstanceExtensionProperties";
    mFunctionIndexToNameString[FuncId_vkEnumerateInstanceLayerProperties] = "vkEnumerateInstanceLayerProperties";
    mFunctionIndexToNameString[FuncId_vkEnumerateDeviceLayerProperties] = "vkEnumerateDeviceLayerProperties";
    mFunctionIndexToNameString[FuncId_vkGetDeviceQueue] = "vkGetDeviceQueue";
    mFunctionIndexToNameString[FuncId_vkQueueSubmit] = "vkQueueSubmit";
    mFunctionIndexToNameString[FuncId_vkQueueWaitIdle] = "vkQueueWaitIdle";
    mFunctionIndexToNameString[FuncId_vkDeviceWaitIdle] = "vkDeviceWaitIdle";
    mFunctionIndexToNameString[FuncId_vkAllocateMemory] = "vkAllocateMemory";
    mFunctionIndexToNameString[FuncId_vkFreeMemory] = "vkFreeMemory";
    mFunctionIndexToNameString[FuncId_vkMapMemory] = "vkMapMemory";
    mFunctionIndexToNameString[FuncId_vkUnmapMemory] = "vkUnmapMemory";
    mFunctionIndexToNameString[FuncId_vkFlushMappedMemoryRanges] = "vkFlushMappedMemoryRanges";
    mFunctionIndexToNameString[FuncId_vkInvalidateMappedMemoryRanges] = "vkInvalidateMappedMemoryRanges";
    mFunctionIndexToNameString[FuncId_vkGetDeviceMemoryCommitment] = "vkGetDeviceMemoryCommitment";
    mFunctionIndexToNameString[FuncId_vkBindBufferMemory] = "vkBindBufferMemory";
    mFunctionIndexToNameString[FuncId_vkBindImageMemory] = "vkBindImageMemory";
    mFunctionIndexToNameString[FuncId_vkGetBufferMemoryRequirements] = "vkGetBufferMemoryRequirements";
    mFunctionIndexToNameString[FuncId_vkGetImageMemoryRequirements] = "vkGetImageMemoryRequirements";
    mFunctionIndexToNameString[FuncId_vkGetImageSparseMemoryRequirements] = "vkGetImageSparseMemoryRequirements";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceSparseImageFormatProperties] = "vkGetPhysicalDeviceSparseImageFormatProperties";
    mFunctionIndexToNameString[FuncId_vkQueueBindSparse] = "vkQueueBindSparse";
    mFunctionIndexToNameString[FuncId_vkCreateFence] = "vkCreateFence";
    mFunctionIndexToNameString[FuncId_vkDestroyFence] = "vkDestroyFence";
    mFunctionIndexToNameString[FuncId_vkResetFences] = "vkResetFences";
    mFunctionIndexToNameString[FuncId_vkGetFenceStatus] = "vkGetFenceStatus";
    mFunctionIndexToNameString[FuncId_vkWaitForFences] = "vkWaitForFences";
    mFunctionIndexToNameString[FuncId_vkCreateSemaphore] = "vkCreateSemaphore";
    mFunctionIndexToNameString[FuncId_vkDestroySemaphore] = "vkDestroySemaphore";
    mFunctionIndexToNameString[FuncId_vkCreateEvent] = "vkCreateEvent";
    mFunctionIndexToNameString[FuncId_vkDestroyEvent] = "vkDestroyEvent";
    mFunctionIndexToNameString[FuncId_vkGetEventStatus] = "vkGetEventStatus";
    mFunctionIndexToNameString[FuncId_vkSetEvent] = "vkSetEvent";
    mFunctionIndexToNameString[FuncId_vkResetEvent] = "vkResetEvent";
    mFunctionIndexToNameString[FuncId_vkCreateQueryPool] = "vkCreateQueryPool";
    mFunctionIndexToNameString[FuncId_vkDestroyQueryPool] = "vkDestroyQueryPool";
    mFunctionIndexToNameString[FuncId_vkGetQueryPoolResults] = "vkGetQueryPoolResults";
    mFunctionIndexToNameString[FuncId_vkCreateBuffer] = "vkCreateBuffer";
    mFunctionIndexToNameString[FuncId_vkDestroyBuffer] = "vkDestroyBuffer";
    mFunctionIndexToNameString[FuncId_vkCreateBufferView] = "vkCreateBufferView";
    mFunctionIndexToNameString[FuncId_vkDestroyBufferView] = "vkDestroyBufferView";
    mFunctionIndexToNameString[FuncId_vkCreateImage] = "vkCreateImage";
    mFunctionIndexToNameString[FuncId_vkDestroyImage] = "vkDestroyImage";
    mFunctionIndexToNameString[FuncId_vkGetImageSubresourceLayout] = "vkGetImageSubresourceLayout";
    mFunctionIndexToNameString[FuncId_vkCreateImageView] = "vkCreateImageView";
    mFunctionIndexToNameString[FuncId_vkDestroyImageView] = "vkDestroyImageView";
    mFunctionIndexToNameString[FuncId_vkCreateShaderModule] = "vkCreateShaderModule";
    mFunctionIndexToNameString[FuncId_vkDestroyShaderModule] = "vkDestroyShaderModule";
    mFunctionIndexToNameString[FuncId_vkCreatePipelineCache] = "vkCreatePipelineCache";
    mFunctionIndexToNameString[FuncId_vkDestroyPipelineCache] = "vkDestroyPipelineCache";
    mFunctionIndexToNameString[FuncId_vkGetPipelineCacheData] = "vkGetPipelineCacheData";
    mFunctionIndexToNameString[FuncId_vkMergePipelineCaches] = "vkMergePipelineCaches";
    mFunctionIndexToNameString[FuncId_vkCreateGraphicsPipelines] = "vkCreateGraphicsPipelines";
    mFunctionIndexToNameString[FuncId_vkCreateComputePipelines] = "vkCreateComputePipelines";
    mFunctionIndexToNameString[FuncId_vkDestroyPipeline] = "vkDestroyPipeline";
    mFunctionIndexToNameString[FuncId_vkCreatePipelineLayout] = "vkCreatePipelineLayout";
    mFunctionIndexToNameString[FuncId_vkDestroyPipelineLayout] = "vkDestroyPipelineLayout";
    mFunctionIndexToNameString[FuncId_vkCreateSampler] = "vkCreateSampler";
    mFunctionIndexToNameString[FuncId_vkDestroySampler] = "vkDestroySampler";
    mFunctionIndexToNameString[FuncId_vkCreateDescriptorSetLayout] = "vkCreateDescriptorSetLayout";
    mFunctionIndexToNameString[FuncId_vkDestroyDescriptorSetLayout] = "vkDestroyDescriptorSetLayout";
    mFunctionIndexToNameString[FuncId_vkCreateDescriptorPool] = "vkCreateDescriptorPool";
    mFunctionIndexToNameString[FuncId_vkDestroyDescriptorPool] = "vkDestroyDescriptorPool";
    mFunctionIndexToNameString[FuncId_vkResetDescriptorPool] = "vkResetDescriptorPool";
    mFunctionIndexToNameString[FuncId_vkAllocateDescriptorSets] = "vkAllocateDescriptorSets";
    mFunctionIndexToNameString[FuncId_vkFreeDescriptorSets] = "vkFreeDescriptorSets";
    mFunctionIndexToNameString[FuncId_vkUpdateDescriptorSets] = "vkUpdateDescriptorSets";
    mFunctionIndexToNameString[FuncId_vkCreateFramebuffer] = "vkCreateFramebuffer";
    mFunctionIndexToNameString[FuncId_vkDestroyFramebuffer] = "vkDestroyFramebuffer";
    mFunctionIndexToNameString[FuncId_vkCreateRenderPass] = "vkCreateRenderPass";
    mFunctionIndexToNameString[FuncId_vkDestroyRenderPass] = "vkDestroyRenderPass";
    mFunctionIndexToNameString[FuncId_vkGetRenderAreaGranularity] = "vkGetRenderAreaGranularity";
    mFunctionIndexToNameString[FuncId_vkCreateCommandPool] = "vkCreateCommandPool";
    mFunctionIndexToNameString[FuncId_vkDestroyCommandPool] = "vkDestroyCommandPool";
    mFunctionIndexToNameString[FuncId_vkResetCommandPool] = "vkResetCommandPool";
    mFunctionIndexToNameString[FuncId_vkAllocateCommandBuffers] = "vkAllocateCommandBuffers";
    mFunctionIndexToNameString[FuncId_vkFreeCommandBuffers] = "vkFreeCommandBuffers";
    mFunctionIndexToNameString[FuncId_vkBeginCommandBuffer] = "vkBeginCommandBuffer";
    mFunctionIndexToNameString[FuncId_vkEndCommandBuffer] = "vkEndCommandBuffer";
    mFunctionIndexToNameString[FuncId_vkResetCommandBuffer] = "vkResetCommandBuffer";
    mFunctionIndexToNameString[FuncId_vkCmdBindPipeline] = "vkCmdBindPipeline";
    mFunctionIndexToNameString[FuncId_vkCmdSetViewport] = "vkCmdSetViewport";
    mFunctionIndexToNameString[FuncId_vkCmdSetScissor] = "vkCmdSetScissor";
    mFunctionIndexToNameString[FuncId_vkCmdSetLineWidth] = "vkCmdSetLineWidth";
    mFunctionIndexToNameString[FuncId_vkCmdSetDepthBias] = "vkCmdSetDepthBias";
    mFunctionIndexToNameString[FuncId_vkCmdSetBlendConstants] = "vkCmdSetBlendConstants";
    mFunctionIndexToNameString[FuncId_vkCmdSetDepthBounds] = "vkCmdSetDepthBounds";
    mFunctionIndexToNameString[FuncId_vkCmdSetStencilCompareMask] = "vkCmdSetStencilCompareMask";
    mFunctionIndexToNameString[FuncId_vkCmdSetStencilWriteMask] = "vkCmdSetStencilWriteMask";
    mFunctionIndexToNameString[FuncId_vkCmdSetStencilReference] = "vkCmdSetStencilReference";
    mFunctionIndexToNameString[FuncId_vkCmdBindDescriptorSets] = "vkCmdBindDescriptorSets";
    mFunctionIndexToNameString[FuncId_vkCmdBindIndexBuffer] = "vkCmdBindIndexBuffer";
    mFunctionIndexToNameString[FuncId_vkCmdBindVertexBuffers] = "vkCmdBindVertexBuffers";
    mFunctionIndexToNameString[FuncId_vkCmdDraw] = "vkCmdDraw";
    mFunctionIndexToNameString[FuncId_vkCmdDrawIndexed] = "vkCmdDrawIndexed";
    mFunctionIndexToNameString[FuncId_vkCmdDrawIndirect] = "vkCmdDrawIndirect";
    mFunctionIndexToNameString[FuncId_vkCmdDrawIndexedIndirect] = "vkCmdDrawIndexedIndirect";
    mFunctionIndexToNameString[FuncId_vkCmdDispatch] = "vkCmdDispatch";
    mFunctionIndexToNameString[FuncId_vkCmdDispatchIndirect] = "vkCmdDispatchIndirect";
    mFunctionIndexToNameString[FuncId_vkCmdCopyBuffer] = "vkCmdCopyBuffer";
    mFunctionIndexToNameString[FuncId_vkCmdCopyImage] = "vkCmdCopyImage";
    mFunctionIndexToNameString[FuncId_vkCmdBlitImage] = "vkCmdBlitImage";
    mFunctionIndexToNameString[FuncId_vkCmdCopyBufferToImage] = "vkCmdCopyBufferToImage";
    mFunctionIndexToNameString[FuncId_vkCmdCopyImageToBuffer] = "vkCmdCopyImageToBuffer";
    mFunctionIndexToNameString[FuncId_vkCmdUpdateBuffer] = "vkCmdUpdateBuffer";
    mFunctionIndexToNameString[FuncId_vkCmdFillBuffer] = "vkCmdFillBuffer";
    mFunctionIndexToNameString[FuncId_vkCmdClearColorImage] = "vkCmdClearColorImage";
    mFunctionIndexToNameString[FuncId_vkCmdClearDepthStencilImage] = "vkCmdClearDepthStencilImage";
    mFunctionIndexToNameString[FuncId_vkCmdClearAttachments] = "vkCmdClearAttachments";
    mFunctionIndexToNameString[FuncId_vkCmdResolveImage] = "vkCmdResolveImage";
    mFunctionIndexToNameString[FuncId_vkCmdSetEvent] = "vkCmdSetEvent";
    mFunctionIndexToNameString[FuncId_vkCmdResetEvent] = "vkCmdResetEvent";
    mFunctionIndexToNameString[FuncId_vkCmdWaitEvents] = "vkCmdWaitEvents";
    mFunctionIndexToNameString[FuncId_vkCmdPipelineBarrier] = "vkCmdPipelineBarrier";
    mFunctionIndexToNameString[FuncId_vkCmdBeginQuery] = "vkCmdBeginQuery";
    mFunctionIndexToNameString[FuncId_vkCmdEndQuery] = "vkCmdEndQuery";
    mFunctionIndexToNameString[FuncId_vkCmdResetQueryPool] = "vkCmdResetQueryPool";
    mFunctionIndexToNameString[FuncId_vkCmdWriteTimestamp] = "vkCmdWriteTimestamp";
    mFunctionIndexToNameString[FuncId_vkCmdCopyQueryPoolResults] = "vkCmdCopyQueryPoolResults";
    mFunctionIndexToNameString[FuncId_vkCmdPushConstants] = "vkCmdPushConstants";
    mFunctionIndexToNameString[FuncId_vkCmdBeginRenderPass] = "vkCmdBeginRenderPass";
    mFunctionIndexToNameString[FuncId_vkCmdNextSubpass] = "vkCmdNextSubpass";
    mFunctionIndexToNameString[FuncId_vkCmdEndRenderPass] = "vkCmdEndRenderPass";
    mFunctionIndexToNameString[FuncId_vkCmdExecuteCommands] = "vkCmdExecuteCommands";
    mFunctionIndexToNameString[FuncId_vkDestroySurfaceKHR] = "vkDestroySurfaceKHR";
    mFunctionIndexToNameString[FuncId_vkCreateSwapchainKHR] = "vkCreateSwapchainKHR";
    mFunctionIndexToNameString[FuncId_vkDestroySwapchainKHR] = "vkDestroySwapchainKHR";
    mFunctionIndexToNameString[FuncId_vkGetSwapchainImagesKHR] = "vkGetSwapchainImagesKHR";
    mFunctionIndexToNameString[FuncId_vkAcquireNextImageKHR] = "vkAcquireNextImageKHR";
    mFunctionIndexToNameString[FuncId_vkQueuePresentKHR] = "vkQueuePresentKHR";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceSurfaceSupportKHR] = "vkGetPhysicalDeviceSurfaceSupportKHR";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceSurfaceCapabilitiesKHR] = "vkGetPhysicalDeviceSurfaceCapabilitiesKHR";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceSurfaceFormatsKHR] = "vkGetPhysicalDeviceSurfaceFormatsKHR";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceSurfacePresentModesKHR] = "vkGetPhysicalDeviceSurfacePresentModesKHR";
    mFunctionIndexToNameString[FuncId_vkCreateWin32SurfaceKHR] = "vkCreateWin32SurfaceKHR";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceWin32PresentationSupportKHR] = "vkGetPhysicalDeviceWin32PresentationSupportKHR";
    mFunctionIndexToNameString[FuncId_vkCreateXcbSurfaceKHR] = "vkCreateXcbSurfaceKHR";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceXcbPresentationSupportKHR] = "vkGetPhysicalDeviceXcbPresentationSupportKHR";
    mFunctionIndexToNameString[FuncId_vkCreateXlibSurfaceKHR] = "vkCreateXlibSurfaceKHR";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceXlibPresentationSupportKHR] = "vkGetPhysicalDeviceXlibPresentationSupportKHR";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceDisplayPropertiesKHR] = "vkGetPhysicalDeviceDisplayPropertiesKHR";
    mFunctionIndexToNameString[FuncId_vkGetPhysicalDeviceDisplayPlanePropertiesKHR] = "vkGetPhysicalDeviceDisplayPlanePropertiesKHR";
    mFunctionIndexToNameString[FuncId_vkGetDisplayPlaneSupportedDisplaysKHR] = "vkGetDisplayPlaneSupportedDisplaysKHR";
    mFunctionIndexToNameString[FuncId_vkGetDisplayModePropertiesKHR] = "vkGetDisplayModePropertiesKHR";
    mFunctionIndexToNameString[FuncId_vkCreateDisplayModeKHR] = "vkCreateDisplayModeKHR";
    mFunctionIndexToNameString[FuncId_vkGetDisplayPlaneCapabilitiesKHR] = "vkGetDisplayPlaneCapabilitiesKHR";
    mFunctionIndexToNameString[FuncId_vkCreateDisplayPlaneSurfaceKHR] = "vkCreateDisplayPlaneSurfaceKHR";
    mFunctionIndexToNameString[FuncId_vkCreateDebugReportCallbackEXT] = "vkCreateDebugReportCallbackEXT";
    mFunctionIndexToNameString[FuncId_vkDestroyDebugReportCallbackEXT] = "vkDestroyDebugReportCallbackEXT";
    mFunctionIndexToNameString[FuncId_vkDebugReportMessageEXT] = "vkDebugReportMessageEXT";
    mFunctionIndexToNameString[FuncId_WholeCmdBuf] = "WholeCmdBuf";
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the parent LayerManager used by this tool.
/// \returns A pointer to the parent LayerManager used by this tool.
//-----------------------------------------------------------------------------
ModernAPILayerManager* VktTraceAnalyzerLayer::GetParentLayerManager()
{
    return VktLayerManager::GetLayerManager();
}

//-----------------------------------------------------------------------------
/// Provides a chance to initialize states before a GPU trace is performed.
//-----------------------------------------------------------------------------
void VktTraceAnalyzerLayer::BeforeGPUTrace()
{
    VktFrameProfilerLayer::Instance()->ResetSampleIdCounter();
    VktFrameProfilerLayer::Instance()->ClearProfilingResults();

    MultithreadedTraceAnalyzerLayer::BeforeGPUTrace();
}

//-----------------------------------------------------------------------------
/// Provides a chance to initialize states before a GPU trace is performed.
//-----------------------------------------------------------------------------
void VktTraceAnalyzerLayer::AfterGPUTrace()
{
    MultithreadedTraceAnalyzerLayer::AfterGPUTrace();
}

//-----------------------------------------------------------------------------
/// Fill a vector with all available queues.
/// \param wrappedQueues Vector to be filled in with our queues.
//-----------------------------------------------------------------------------
void VktTraceAnalyzerLayer::GetAvailableQueues(std::vector<VktWrappedQueue*>& wrappedQueues)
{
    const WrappedQueueMap& queues = GetWrappedQueues();

    for (WrappedQueueMap::const_iterator it = queues.begin(); it != queues.end(); ++it)
    {
        wrappedQueues.push_back(it->second);
    }
}

//-----------------------------------------------------------------------------
/// Check return value from worker wait.
/// \param waitRetVal Return value from WaitForMultipleObjects.
/// \param numThreads Number of threads we waited on.
//-----------------------------------------------------------------------------
bool VktTraceAnalyzerLayer::WaitSucceeded(DWORD waitRetVal, UINT numThreads)
{
    // @TODO - pull it up
#ifdef WIN32
    return (waitRetVal >= WAIT_OBJECT_0) && (waitRetVal <= (WAIT_OBJECT_0 + numThreads - 1));
#else
    GT_UNREFERENCED_PARAMETER(waitRetVal);
    GT_UNREFERENCED_PARAMETER(numThreads);
    return true;
#endif
}

//-----------------------------------------------------------------------------
/// Wait for workers to finish and get their results.
/// \param pFrameProfiler Pointer to the frame profiler.
//-----------------------------------------------------------------------------
void VktTraceAnalyzerLayer::WaitAndFetchResults(VktFrameProfilerLayer* pFrameProfiler)
{
    // Gather all known queues
    std::vector<VktWrappedQueue*> queues;
    GetAvailableQueues(queues);

    if (queues.size() > 0)
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        std::vector<HANDLE> queueThreads;
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
        std::vector<std::thread*> queueThreads;
#else
#error Unknown build target! No valid value for AMDT_BUILD_TARGET.
#endif

        for (UINT i = 0; i < queues.size(); i++)
        {
            for (UINT j = 0; j < queues[i]->WorkerThreadCount(); j++)
            {
                VktWorkerInfo* pWorkerInfo = queues[i]->GetWorkerInfo(j);

                if (pWorkerInfo != nullptr)
                {
                    queueThreads.push_back(queues[i]->GetThreadHandle(j));
                }
            }
        }

        const UINT queueWorkerCount = (UINT)queueThreads.size();

        // Gather all worker handles
        if (queueWorkerCount > 0)
        {
            // Wait for other workers
#ifdef WIN32
            DWORD retVal = WaitForMultipleObjects(queueWorkerCount, &queueThreads[0], TRUE, QUEUE_RESULTS_WORKER_TIMEOUT);
#else

            for (size_t it = 0; it < queueWorkerCount; it++)
            {
                queueThreads[it]->join();
            }

            DWORD retVal = 0;
#endif

            if (WaitSucceeded(retVal, queueWorkerCount) == false)
            {
                Log(logWARNING, "Detected failure condition when waiting for worker threads.\n");
            }

            // Get results from thread
            for (UINT i = 0; i < queues.size(); i++)
            {
                for (UINT j = 0; j < queues[i]->WorkerThreadCount(); j++)
                {
                    VktWorkerInfo* pWorkerInfo = queues[i]->GetWorkerInfo(j);

                    pFrameProfiler->VerifyAlignAndStoreResults(
                        queues[i],
                        pWorkerInfo->m_outputs.results,
                        &pWorkerInfo->m_inputs.timestampPair,
                        pWorkerInfo->m_threadInfo.workerThreadCountID,
                        pWorkerInfo->m_inputs.frameStartTime);
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
void VktTraceAnalyzerLayer::ProfilerResultToStr(
    ProfilerResult* pResult,
    gtASCIIString&  profiledCommandsLinesStr)
{
    // Convert timestamps to milliseconds by using the clock frequency.
#ifndef CODEXL_GRAPHICS
    const double timestampFrequency = pResult->measurementInfo.idInfo.pWrappedQueue->GetTimestampFrequency();

    double preStartTimestamp = (pResult->timestampResult.rawClocks.preStart / timestampFrequency) * 1000.0;
    double startTimestamp = (pResult->timestampResult.rawClocks.start / timestampFrequency) * 1000.0;
    double endTimestamp = (pResult->timestampResult.rawClocks.end / timestampFrequency) * 1000.0;
#else
    double startTimestamp = pResult->timestampResult.alignedMillisecondTimestamps.start;
    double endTimestamp = pResult->timestampResult.alignedMillisecondTimestamps.end;
#endif
    FuncId funcId = (FuncId)pResult->measurementInfo.idInfo.funcId;

    gtASCIIString funcName = GetFunctionNameFromId(funcId);
    gtASCIIString retVal = "void";
    gtASCIIString params = "";

    if (pResult->measurementInfo.idInfo.funcId != FuncId_WholeCmdBuf)
    {
        VktAPIEntry* pResultEntry = VktFrameProfilerLayer::Instance()->FindInvocationBySampleId(pResult->measurementInfo.idInfo.sampleId);

        if (pResultEntry != nullptr)
        {
            // Convert the functionID and return values from integers into full strings that we can use in the response.
            funcName = GetFunctionNameFromId(pResultEntry->mFunctionId);
            retVal = (pResultEntry->m_returnValue != -1) ? VktUtil::WriteResultCodeEnumAsString(pResultEntry->m_returnValue) : "void";
            params = pResultEntry->mParameters.asCharArray();
        }
    }

    // Vulkan Response line format:
    // QueueFamilyIndex QueueIndex CommandBuffer APIType FuncId Vulkan_FuncName(Args) = ReturnValue PreStartTime StartTime EndTime SampleId

    profiledCommandsLinesStr += IntToString(pResult->measurementInfo.idInfo.pWrappedQueue->GetQueueFamilyIndex());
    profiledCommandsLinesStr += " ";

    profiledCommandsLinesStr += IntToString(pResult->measurementInfo.idInfo.pWrappedQueue->GetQueueIndex());
    profiledCommandsLinesStr += " ";

    profiledCommandsLinesStr += POINTER_SUFFIX;
    profiledCommandsLinesStr += UINT64ToHexString((UINT64)pResult->measurementInfo.idInfo.pWrappedCmdBuf->AppHandle());
    profiledCommandsLinesStr += " ";

    profiledCommandsLinesStr += IntToString(VktTraceAnalyzerLayer::Instance()->GetAPIGroupFromAPI(funcId));
    profiledCommandsLinesStr += " ";

    profiledCommandsLinesStr += IntToString(funcId);
    profiledCommandsLinesStr += " ";

    profiledCommandsLinesStr += "Vulkan_";
    profiledCommandsLinesStr += funcName;

    profiledCommandsLinesStr += "(";
    profiledCommandsLinesStr += params;
    profiledCommandsLinesStr += ") = ";

    profiledCommandsLinesStr += retVal;

#ifndef CODEXL_GRAPHICS
    profiledCommandsLinesStr += " ";
    profiledCommandsLinesStr += DoubleToString(preStartTimestamp);
#endif

    profiledCommandsLinesStr += " ";
    profiledCommandsLinesStr += DoubleToString(startTimestamp);

    profiledCommandsLinesStr += " ";
    profiledCommandsLinesStr += DoubleToString(endTimestamp);

    profiledCommandsLinesStr += " ";
    profiledCommandsLinesStr += UINT64ToString(pResult->measurementInfo.idInfo.sampleId);

    profiledCommandsLinesStr += "\n";
}

//-----------------------------------------------------------------------------
/// Return GPU-time in text format, to be parsed by the Client and displayed as its own timeline.
/// \return A line-delimited, ASCII-encoded, version of the GPU Trace data.
//-----------------------------------------------------------------------------
std::string VktTraceAnalyzerLayer::GetGPUTraceTXT()
{
    gtASCIIString appendString = "";

    VktFrameProfilerLayer* pProfilerLayer = VktFrameProfilerLayer::Instance();

    WaitAndFetchResults(pProfilerLayer);

    // During QueueSubmit we stored ProfilerResults in mEntriesWithProfilingResults. Form a response using it here.
    ProfilerResultsMap& profiledCmdBufResultsMap = pProfilerLayer->GetCmdBufProfilerResultsMap();

    // Gather all profiler results
    if (!profiledCmdBufResultsMap.empty())
    {
        std::vector<ProfilerResult*> flatResults;

        for (ProfilerResultsMap::iterator profIt = profiledCmdBufResultsMap.begin(); profIt != profiledCmdBufResultsMap.end(); ++profIt)
        {
            QueueWrapperToProfilingResultsMap& resultsPerThread = profIt->second;

            for (QueueWrapperToProfilingResultsMap::iterator queuesIt = resultsPerThread.begin();
                 queuesIt != resultsPerThread.end();
                 ++queuesIt)
            {
                // This structure holds all of the profiler results that were collected at QueueSubmit. The form is LinkId->ProfilerResult.
                const SampleIdToProfilerResultMap* pResults = queuesIt->second;

                for (SampleIdToProfilerResultMap::const_iterator sampleIdIt = pResults->begin();
                     sampleIdIt != pResults->end();
                     ++sampleIdIt)
                {
                    ProfilerResult* pResult = sampleIdIt->second;
                    pResult->measurementInfo.idInfo.pWrappedQueue = queuesIt->first;

                    flatResults.push_back(pResult);
                }
            }
        }

        const UINT numResults = (UINT)flatResults.size();

        // We'll need to insert the GPU Trace section header before the response data, even if there aren't any results.
        appendString += "//==GPU Trace==";
        appendString += "\n";

        appendString += "//API=";
        appendString += GetAPIString();
        appendString += "\n";

        appendString += "//CommandBufEventCount=";
        appendString += IntToString((INT)numResults);
        appendString += "\n";

#ifdef WIN32
        sort(flatResults.begin(), flatResults.end(), SortByStartTime);

        for (UINT i = 0; i < numResults; i++)
        {
            ProfilerResultToStr(flatResults[i], appendString);
        }
#else
        ProfilerResult* pFlatResults = new ProfilerResult[numResults];

        for (UINT i = 0; i < numResults; i++)
        {
            pFlatResults[i] = *(flatResults[i]);
        }

        qsort(pFlatResults, numResults, sizeof(ProfilerResult), (compfn)SortByStartTime);

        for (UINT i = 0; i < numResults; i++)
        {
            ProfilerResultToStr(&pFlatResults[i], appendString);
        }

        delete[] pFlatResults;
        pFlatResults = nullptr;
#endif
    }
    else
    {
        appendString += "NODATA";
    }

    return appendString.asCharArray();
}

//-----------------------------------------------------------------------------
/// Create a new ThreadTraceData instance for use specifically with Vulkan.
/// \returns A new ThreadTraceData instance to trace Vulkan API calls.
//-----------------------------------------------------------------------------
ThreadTraceData* VktTraceAnalyzerLayer::CreateThreadTraceDataInstance()
{
    return new VktThreadTraceData();
}

//-----------------------------------------------------------------------------
/// Log a Vulkan API call within the Trace Analyzer.
/// \param pApiEntry The APIEntry created for this API call
//-----------------------------------------------------------------------------
void VktTraceAnalyzerLayer::LogAPICall(VktAPIEntry* pApiEntry)
{
    ScopeLock logAPICallLock(&mTraceMutex);

    ThreadTraceData* pCurrentThreadData = FindOrCreateThreadData(pApiEntry->mThreadId);

    UINT64 startTime = pCurrentThreadData->m_startTime.QuadPart;

    if (startTime == s_DummyTimestampValue)
    {
        const char* pFuncNameString = GetFunctionNameFromId(pApiEntry->mFunctionId);
        Log(logERROR, "There was a problem setting the start time for API call '%s' on Thread with Id '%d'.\n", pFuncNameString, pApiEntry->mThreadId);
    }

    pCurrentThreadData->AddAPIEntry(pCurrentThreadData->m_startTime, pApiEntry);
}

//-----------------------------------------------------------------------------
/// Return the stringified function name base don the input enum.
/// \param inFunctionId An enumeration representing the function being invoked.
/// \returns A string containing the function name.
//-----------------------------------------------------------------------------
const char* VktTraceAnalyzerLayer::GetFunctionNameFromId(FuncId inFunctionId)
{
    return mFunctionIndexToNameString[inFunctionId].c_str();
}

//-----------------------------------------------------------------------------
/// Gets called immediately after the real Present() is called
/// \param queue The queue used to present.
/// \param pPresentInfo Presentation info.
//-----------------------------------------------------------------------------
void VktTraceAnalyzerLayer::OnPresent(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
{
    VktInterceptManager* pInterceptor = VktLayerManager::GetLayerManager()->GetInterceptMgr();

    if (pInterceptor->ShouldCollectTrace())
    {
        char argumentsBuffer[ARGUMENTS_BUFFER_SIZE];
        sprintf_s(argumentsBuffer, ARGUMENTS_BUFFER_SIZE, "%s, %s",
                  VktUtil::WritePointerAsString(queue),
                  VktUtil::WritePointerAsString(pPresentInfo));

        // precall
        BeforeAPICall();

        // postcall
        DWORD threadId = osGetCurrentThreadId();
        VktAPIEntry* pNewEntry = new VktAPIEntry(threadId, FuncId_vkQueuePresentKHR, argumentsBuffer, nullptr);
        pInterceptor->PostCall(pNewEntry);
    }
}

//-----------------------------------------------------------------------------
/// Retrieve the API Type that an API call has been classified into.
/// \param inAPIFuncId The FunctionId of an API call to retrieve the group for.
/// \returns An API Type that a call has been classified as being part of.
//-----------------------------------------------------------------------------
eAPIType VktTraceAnalyzerLayer::GetAPIGroupFromAPI(FuncId inAPIFuncId)
{
    eAPIType apiType = kAPIType_Unknown;

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
            apiType = kAPIType_Create;
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
            apiType = kAPIType_Destroy;
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
            apiType = kAPIType_Get;
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
            apiType = kAPIType_Memory;
            break;

        case FuncId_vkResetDescriptorPool:
        case FuncId_vkAllocateDescriptorSets:
        case FuncId_vkFreeDescriptorSets:
        case FuncId_vkUpdateDescriptorSets:
            apiType = kAPIType_DescriptorSet;
            break;

        case FuncId_vkQueueSubmit:
        case FuncId_vkQueueBindSparse:
        case FuncId_vkResetCommandPool:
        case FuncId_vkAllocateCommandBuffers:
        case FuncId_vkFreeCommandBuffers:
        case FuncId_vkBeginCommandBuffer:
        case FuncId_vkEndCommandBuffer:
        case FuncId_vkResetCommandBuffer:
            apiType = kAPIType_QueueSubmission;
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
            apiType = kAPIType_CmdBufProfiled;
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
            apiType = kAPIType_CmdBufNonProfiled;
            break;

        case FuncId_vkQueueWaitIdle:
        case FuncId_vkDeviceWaitIdle:
        case FuncId_vkResetFences:
        case FuncId_vkGetFenceStatus:
        case FuncId_vkWaitForFences:
        case FuncId_vkGetEventStatus:
        case FuncId_vkSetEvent:
        case FuncId_vkResetEvent:
            apiType = kAPIType_Sync;
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
        case FuncId_vkCreateXcbSurfaceKHR:
        case FuncId_vkGetPhysicalDeviceXcbPresentationSupportKHR:
        case FuncId_vkCreateXlibSurfaceKHR:
        case FuncId_vkGetPhysicalDeviceXlibPresentationSupportKHR:
        case FuncId_vkGetPhysicalDeviceDisplayPropertiesKHR:
        case FuncId_vkGetPhysicalDeviceDisplayPlanePropertiesKHR:
        case FuncId_vkGetDisplayPlaneSupportedDisplaysKHR:
        case FuncId_vkGetDisplayModePropertiesKHR:
        case FuncId_vkCreateDisplayModeKHR:
        case FuncId_vkGetDisplayPlaneCapabilitiesKHR:
        case FuncId_vkCreateDisplayPlaneSurfaceKHR:
        case FuncId_vkCreateDebugReportCallbackEXT:
        case FuncId_vkDestroyDebugReportCallbackEXT:
        case FuncId_vkDebugReportMessageEXT:
            apiType = kAPIType_KHR;
            break;

        default:
            apiType = kAPIType_Unknown;
    }

    return apiType;
}
