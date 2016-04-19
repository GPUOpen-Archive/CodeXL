//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Information required by worker threads to gather profiler results.
//=============================================================================

#ifndef DX12_WORKER_INFO_H
#define DX12_WORKER_INFO_H

#include "DX12FrameProfilerLayer.h"
#include <AMDTOSWrappers/Include/osThread.h>

//-----------------------------------------------------------------------------
/// Holds information passed to a worker thread.
//-----------------------------------------------------------------------------
struct WorkerInputs
{
    UINT64                            fenceToWaitOn;                 ///< The fence value to wait on after profiling.
    ID3D12Fence*                      pQueueFence;                   ///< The fence used to wait on profiler results.
    Wrapped_ID3D12CommandQueueCustom* pCommandQueue;                 ///< The Queue used to profile calls.
    CalibrationTimestampPair          timestampPair;                 ///< A pair of timestamps used to align CPU and GPU timelines.
    bool                              bContainsComputeQueueWork;     ///< A flag that indicates if a Compute Queue was profiled.
    std::vector<ID3D12CommandList*>   commandListsWithProfiledCalls; ///< A vector of CommandLists with profiled calls.
    INT64                             executionID;                   ///< Track which ExecuteCommandLists() call this worker is for
};

//-----------------------------------------------------------------------------
/// Holds information passed from a worker thread.
//-----------------------------------------------------------------------------
struct WorkerOutputs
{
    bool                        bResultsGathered; ///< A flag used to indicate if the results were gathered correctly.
    std::vector<ProfilerResult> results;          ///< A vector containing all collected profiler results.
};

//-----------------------------------------------------------------------------
/// Holds information about a worker thread.
//-----------------------------------------------------------------------------
struct WorkerThreadInfo
{
    UINT32     workerThreadCountID; ///< Incremented count per worker thread created - starts at 1.
    HANDLE     threadHandle;        ///< Handle to the thread that uses me.
    osThreadId parentThreadID;      ///< Thread ID of the parent that spawned us.
    osThreadId workerThreadID;      ///< The thread ID of the worker thread that uses me.
};

//-----------------------------------------------------------------------------
/// Holds information regarding a worker thread
//-----------------------------------------------------------------------------
class DX12WorkerInfo
{
public:
    //-----------------------------------------------------------------------------
    /// Constructor.
    //-----------------------------------------------------------------------------
    DX12WorkerInfo()
    {
        m_inputs.fenceToWaitOn             = 0;
        m_inputs.pQueueFence               = nullptr;
        m_inputs.pCommandQueue             = nullptr;
        m_inputs.bContainsComputeQueueWork = false;
        memset(&m_inputs.timestampPair, 0, sizeof(m_inputs.timestampPair));

        m_outputs.bResultsGathered = false;

        memset(&m_threadInfo, 0, sizeof(m_threadInfo));
    }

    /// The inputs required for collecting profiler results.
    WorkerInputs m_inputs;

    /// The output profiling results.
    WorkerOutputs m_outputs;

    /// Holds information about a worker thread.
    WorkerThreadInfo m_threadInfo;
};

#endif // DX12_WORKER_INFO_H