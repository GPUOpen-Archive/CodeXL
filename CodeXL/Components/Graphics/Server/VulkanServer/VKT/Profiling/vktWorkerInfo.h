//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktWorkerInfo.h
/// \brief  Information required by worker threads to gather profiler results.
//=============================================================================

#ifndef __VKT_WORKER_INFO_H__
#define __VKT_WORKER_INFO_H__

#include "../Profiling/vktFrameProfilerLayer.h"
#include <AMDTOSWrappers/Include/osThread.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    typedef HANDLE WorkerThreadHandle;
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <thread>
    typedef std::thread* WorkerThreadHandle;
#endif

class VktWrappedQueue;

//-----------------------------------------------------------------------------
/// Holds information about a command buffer and its desired fill ID
//-----------------------------------------------------------------------------
struct WrappedCmdBufData
{
    VktWrappedCmdBuf* pCmdBuf;           ///< The profiled command buffer
    UINT64            targetFillID;      ///< An ID specifying how many times this command buffer was filled
    UINT              profiledCallCount; ///< The last-known number of times this cmdBuf was profiled
};

//-----------------------------------------------------------------------------
/// Holds information passed to a worker thread.
//-----------------------------------------------------------------------------
struct WorkerInputs
{
    bool                           internalFence;     ///< The fence passed here is internal to VulkanServer
    VkFence                        fenceToWaitOn;     ///< The fence we should be waiting on
    CalibrationTimestampPair       timestampPair;     ///< A pair of timestamps used to align CPU and GPU timelines
    VktWrappedQueue*               pQueue;            ///< The Queue used to profile calls
    std::vector<WrappedCmdBufData> cmdBufData;        ///< A vector of WrappedCmdBufData structs
    GPS_TIMESTAMP                  frameStartTime;    ///< Cache frame start time before launching threads
};

//-----------------------------------------------------------------------------
/// Holds information passed from a worker thread.
//-----------------------------------------------------------------------------
struct WorkerOutputs
{
    std::vector<ProfilerResult> results;      ///< A vector containing all collected profiler results
};



//-----------------------------------------------------------------------------
/// Holds information about a worker thread.
//-----------------------------------------------------------------------------
struct WorkerThreadInfo
{
    UINT32              workerThreadCountID; ///< Incremented count per worker thread created - starts at 1
    WorkerThreadHandle  threadHandle;        ///< On Windows: Handle to the thread that uses me. On Linux: Pointer to the thread object.
    osThreadId          parentThreadID;      ///< Thread ID of the parent that spawned us
    osThreadId          workerThreadID;      ///< The thread ID of the worker thread that uses me
};

//-----------------------------------------------------------------------------
/// Holds information regarding a worker thread
//-----------------------------------------------------------------------------
class VktWorkerInfo
{
public:
    //-----------------------------------------------------------------------------
    /// Constructor.
    //-----------------------------------------------------------------------------
    VktWorkerInfo()
    {
        memset(&m_inputs.timestampPair, 0, sizeof(m_inputs.timestampPair));
        m_inputs.internalFence = false;
        m_inputs.fenceToWaitOn = VK_NULL_HANDLE;
        m_inputs.pQueue        = nullptr;

        memset(&m_inputs.frameStartTime, 0, sizeof(m_inputs.frameStartTime));

        memset(&m_threadInfo, 0, sizeof(m_threadInfo));
    }

    /// The inputs required for collecting profiler results.
    WorkerInputs m_inputs;

    /// The output profiling results.
    WorkerOutputs m_outputs;

    /// Holds information about a worker thread.
    WorkerThreadInfo m_threadInfo;
};

#endif // __VKT_WORKER_INFO_H__