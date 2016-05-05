//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktWrappedQueue.h
/// \brief  A wrapper for queues.
//=============================================================================

#ifndef __VKT_WRAPPED_QUEUE_H__
#define __VKT_WRAPPED_QUEUE_H__

#include "vktWrappedObject.h"
#include "../../Util/vktUtil.h"
#include "../../Profiling/vktWorkerInfo.h"

class VktInterceptManager;

//-----------------------------------------------------------------------------
/// Holds information used to create a queue wrapper.
//-----------------------------------------------------------------------------
struct WrappedQueueCreateInfo
{
    VkPhysicalDevice     physicalDevice;    ///< The queue's physical device
    VkDevice             device;            ///< The queue's device
    uint32_t             queueFamilyIndex;  ///< The queue's family index
    uint32_t             queueIndex;        ///< The queue's index
    VkQueue              appQueue;          ///< The app's handle to the queue
    UINT                 queueFlags;        ///< The queue's flags
    VktInterceptManager* pInterceptMgr;     ///< Pointer to our interception manager
};

//-----------------------------------------------------------------------------
/// Used to track all Vulkan API calls that are traced at runtime.
/// All API calls can be traced, and only some can be profiled.
//-----------------------------------------------------------------------------
class VktWrappedQueue : public VktWrappedObject
{
public:
    static VktWrappedQueue* Create(const WrappedQueueCreateInfo& createInfo);

    VktWrappedQueue(const WrappedQueueCreateInfo& createInfo);
    ~VktWrappedQueue() {}

    /// Store the app's handle in this wrapper
    virtual void StoreAppHandle(UINT64 hAppObject) { UNREFERENCED_PARAMETER(hAppObject); }

    /// Return the app's handle for this wrapper
    VkQueue AppHandle() { return m_createInfo.appQueue; }

    /// Return the parent device for this queue
    VkDevice ParentDevice() { return m_createInfo.device; }

    /// Return the queue's index
    UINT GetQueueIndex() { return m_createInfo.queueIndex; }

    /// Return the queue's creation flags
    UINT GetQueueFlags() { return m_createInfo.queueFlags; }

    double GetTimestampFrequency();

    void GatherWrappedCommandBufs(UINT commandBufferCount, const VkCommandBuffer* pCommandBuffers, std::vector<VktWrappedCmdBuf*>& wrappedCmdBufs);
    void SpawnWorker(CalibrationTimestampPair* pTimestampPair, VktWrappedQueue* pQueue, VkFence fenceToWaitOn, bool internalFence, const std::vector<VktWrappedCmdBuf*> wrappedCmdBufs);

    void EndCollection();

    /// Return the number of workers currently spawned by this wrapper
    UINT WorkerThreadCount() { return (UINT)m_workerThreadInfo.size(); }

    /// Return the thread handle for a particular worker
    HANDLE GetThreadHandle(int inIndex) { return m_workerThreadInfo[inIndex]->m_threadInfo.threadHandle; }

    /// Retrieve information about a particular worker
    VktWorkerInfo* GetWorkerInfo(int inIndex) { return m_workerThreadInfo[inIndex]; }

    VkResult QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
    VkResult QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);

    //-----------------------------------------------------------------------------
    /// ICD entry points.
    //-----------------------------------------------------------------------------
    VkResult QueueSubmit_ICD(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
    VkResult QueuePresentKHR_ICD(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
    VkResult QueueWaitIdle(VkQueue queue);
    VkResult QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence);

private:
    /// CmdBuf create info
    WrappedQueueCreateInfo m_createInfo;

    /// A vector of each VktWorkerInfo instance for each result collection worker thread.
    std::vector<VktWorkerInfo*> m_workerThreadInfo;

    /// A mutex used to serialize access to the worker thread info storage.
    mutex m_workerThreadInfoMutex;

    /// Track an ExecuteCommandBuffers ID.
    INT64 m_executionID;
};

#endif // __VKT_WRAPPED_QUEUE_H__