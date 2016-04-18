//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   Wrapped_ID3D12CommandQueueCustom.cpp
/// \brief  A special wrapper implementation extended from Wrapped_ID3D12CommandQueue
///         to be used for submitting GPU profiling commands.
//=============================================================================

#include "Wrapped_ID3D12CommandQueueCustom.h"
#include "Wrapped_ID3D12GraphicsCommandListCustom.h"
#include "../../Tracing/DX12TraceAnalyzerLayer.h"
#include "../../DX12LayerManager.h"
#include <AMDTOSWrappers/Include/osThread.h>

//-----------------------------------------------------------------------------
/// Profiler results collection worker function.
/// \param lpParam A void pointer to the incoming DX12WorkerInfo argument.
/// \returns Always 0.
//-----------------------------------------------------------------------------
DWORD WINAPI ThreadFunc(LPVOID lpParam)
{
    DX12WorkerInfo* pWorkerInfo = (DX12WorkerInfo*)lpParam;

    HANDLE hEvent = CreateEvent(nullptr, false, false, nullptr);

    if (hEvent != nullptr)
    {
        pWorkerInfo->m_threadInfo.workerThreadID = osGetCurrentThreadId();

        // Wait for GPU queue to come back
        DX12Util::WaitForFence(pWorkerInfo->m_inputs.fenceToWaitOn, pWorkerInfo->m_inputs.pQueueFence, hEvent);

        if (pWorkerInfo->m_inputs.timestampPair.mQueueCanBeTimestamped)
        {
            for (UINT i = 0; i < pWorkerInfo->m_inputs.commandListsWithProfiledCalls.size(); i++)
            {
                Wrapped_ID3D12GraphicsCommandListCustom* pCmdListCustom = static_cast<Wrapped_ID3D12GraphicsCommandListCustom*>(pWorkerInfo->m_inputs.commandListsWithProfiledCalls[i]);

                ProfilerResultCode profResult = pCmdListCustom->GetCmdListResultsMT(pWorkerInfo->m_inputs.pCommandQueue->mRealCommandQueue, pWorkerInfo->m_inputs.executionID, pWorkerInfo->m_outputs.results);

                if (profResult != PROFILER_SUCCESS)
                {
                    const char* profilerErrorCode = DX12CmdListProfiler::PrintProfilerResult(profResult);

                    // Report that a problem occurred in retrieving full profiler results.
                    Log(logERROR, "Failed to retrieve full profiler results: CmdList 0x%p, Queue 0x%p, ErrorCode %s\n",
                        pWorkerInfo->m_inputs.commandListsWithProfiledCalls[i], pWorkerInfo->m_inputs.pCommandQueue, profilerErrorCode);
                }
            }
        }

        CloseHandle(hEvent);

        // This will only be set to true if the GPU results have come back in time.
        pWorkerInfo->m_outputs.bResultsGathered = true;
    }
    else
    {
        Log(logWARNING, "Event creation in worker thread failed. Skipping result gathering.\n");
    }

    return 0;
}

//-----------------------------------------------------------------------------
/// A custom constructor for our wrapped ID3D12CommandQueue. Creates synchronization objects required for profiling.
//-----------------------------------------------------------------------------
Wrapped_ID3D12CommandQueueCustom::Wrapped_ID3D12CommandQueueCustom(ID3D12CommandQueue* inRealCommandQueue) :
    Wrapped_ID3D12CommandQueue(inRealCommandQueue),
    m_executionID(-1)
{
    ID3D12Device* pDevice = nullptr;
    HRESULT result = mRealCommandQueue->GetDevice(__uuidof(ID3D12Device), reinterpret_cast<void**>(&pDevice));

    if (result == S_OK)
    {
        result = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_QSD.pFence));

        if (result == S_OK)
        {
            m_QSD.pFence->SetName(L"Wrapped_ID3D12CommandQueueCustom fence");
            m_QSD.pFence->Signal(0);

            m_QSD.fenceEvent = CreateEvent(nullptr, false, false, nullptr);
            m_QSD.nextFenceValue = 1;
        }
    }
}

//-----------------------------------------------------------------------------
/// A custom destructor for our wrapped ID3D12CommandQueue. Destroys synchronization objects.
//-----------------------------------------------------------------------------
Wrapped_ID3D12CommandQueueCustom::~Wrapped_ID3D12CommandQueueCustom()
{
    CloseHandle(m_QSD.fenceEvent);
    m_QSD.pFence->Release();
}

//-----------------------------------------------------------------------------
/// Check if it is possible to collect timestamps for this Queue.
/// \returns True if it is possible to collect timestamps for this Queue.
//-----------------------------------------------------------------------------
bool Wrapped_ID3D12CommandQueueCustom::CanTimestampQueue()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = this->mRealCommandQueue->GetDesc();

    bool mayTimestamp = false;

#if TIMESTAMP_DIRECT_COMMAND_LISTS

    if (queueDesc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT)
    {
        mayTimestamp = true;
    }

#endif

#if TIMESTAMP_COMPUTE_COMMAND_LISTS

    if (queueDesc.Type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
    {
        mayTimestamp = true;
    }

#endif

    return mayTimestamp;
}

//-----------------------------------------------------------------------------
/// Fill a vector with all profiled command lists.
/// \param numCommandLists The number of CommandLists to gather results for.
/// \param ppCommandLists The array of CommandLists to gather results for.
/// \param cmdListsWithProfiledCalls An array of CommandLists with profiled calls.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12CommandQueueCustom::GatherProfiledCommandLists(
    UINT                                                   numCommandLists,
    ID3D12CommandList* const*                              ppCommandLists,
    std::vector<Wrapped_ID3D12GraphicsCommandListCustom*>& cmdListsWithProfiledCalls)
{
    for (UINT i = 0; i < numCommandLists; i++)
    {
        if (ppCommandLists[i] == nullptr)
        {
            DX12LayerManager::Instance()->StreamLog("nullptr CmdList passed to SpawnThread.");
        }
        else
        {
            Wrapped_ID3D12GraphicsCommandListCustom* pWrappedCmdList = static_cast<Wrapped_ID3D12GraphicsCommandListCustom*>(ppCommandLists[i]);

            if (pWrappedCmdList->GetProfiledCallCount() > 0)
            {
                cmdListsWithProfiledCalls.push_back(pWrappedCmdList);
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Spawn a worker thread to gather GPU profiler results.
/// \param pTimestampPair A pair of calibration timestamps used to align CPU and GPU timelines.
/// \param pQueue The Queue responsible for executed the profiled workload.
/// \param numCommandLists The number of CommandLists being profiled.
/// \param ppCommandLists The array of CommandLists being executed.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12CommandQueueCustom::SpawnWorker(
    CalibrationTimestampPair*           pTimestampPair,
    Wrapped_ID3D12CommandQueueCustom*   pQueue,
    UINT                                numCommandLists,
    ID3D12CommandList* const*           ppCommandLists)
{
    if (numCommandLists > 0)
    {
        static UINT32 s_threadID = 1;

        DX12WorkerInfo* pWorkerInfo = new DX12WorkerInfo();

        pWorkerInfo->m_inputs.fenceToWaitOn = m_QSD.nextFenceValue;
        pWorkerInfo->m_inputs.pQueueFence   = m_QSD.pFence;
        pWorkerInfo->m_inputs.pCommandQueue = pQueue;
        pWorkerInfo->m_inputs.executionID   = m_executionID;

        pWorkerInfo->m_threadInfo.workerThreadCountID = s_threadID++;
        pWorkerInfo->m_threadInfo.parentThreadID      = osGetCurrentThreadId();

        // Fill up a vector with pointers to CommandLists that definitely contain profiler measurement calls.
        std::vector<Wrapped_ID3D12GraphicsCommandListCustom*> cmdListsWithProfiledCalls;
        GatherProfiledCommandLists(numCommandLists, ppCommandLists, cmdListsWithProfiledCalls);

        for (size_t index = 0; index < numCommandLists; index++)
        {
            if (ppCommandLists[index] == nullptr)
            {
                DX12LayerManager::Instance()->StreamLog("nullptr CmdList passed to SpawnThread.");
            }
            else
            {
                Wrapped_ID3D12GraphicsCommandListCustom* pWrappedCmdList = static_cast<Wrapped_ID3D12GraphicsCommandListCustom*>(ppCommandLists[index]);

                if (pWrappedCmdList->mRealGraphicsCommandList->GetType() == D3D12_COMMAND_LIST_TYPE_COMPUTE)
                {
                    pWorkerInfo->m_inputs.bContainsComputeQueueWork = true;
                }
            }
        }

        // This is the number of CommandLists in the incoming array that we'll need to wait on results for.
        size_t numProfiledCmdLists = cmdListsWithProfiledCalls.size();

        if (numProfiledCmdLists > 0)
        {
            for (size_t index = 0; index < numProfiledCmdLists; index++)
            {
                pWorkerInfo->m_inputs.commandListsWithProfiledCalls.push_back(cmdListsWithProfiledCalls[index]);
            }

            memcpy(&pWorkerInfo->m_inputs.timestampPair, pTimestampPair, sizeof(pWorkerInfo->m_inputs.timestampPair));

            // The push onto m_workerThreadInfo needs to be thread safe
            {
                ScopeLock lock(&m_workerThreadInfoMutex);

                m_workerThreadInfo.push_back(pWorkerInfo);

                DWORD threadId = 0;
                pWorkerInfo->m_threadInfo.threadHandle = CreateThread(nullptr, 0, ThreadFunc, pWorkerInfo, 0, &threadId);
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// A custom override used to track which Queue is responsible for executing CommandLists.
/// \param NumCommandLists The number of CommandLists in the array to execute.
/// \param ppCommandLists An array of CommandLists to execute through the Queue.
//-----------------------------------------------------------------------------
void STDMETHODCALLTYPE Wrapped_ID3D12CommandQueueCustom::ExecuteCommandLists(UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    m_executionID++;

    DX12TraceAnalyzerLayer* traceAnalyzer = DX12TraceAnalyzerLayer::Instance();
    DX12FrameProfilerLayer* frameProfiler = DX12FrameProfilerLayer::Instance();

    // Use this calibration timestamp structure to convert GPU events to the CPU timeline.
    CalibrationTimestampPair calibrationTimestamps = {};
    calibrationTimestamps.mQueueCanBeTimestamped = CanTimestampQueue();

    // Surround the execution of CommandLists with timestamps so we can determine when the GPU work occurred in the CPU timeline.
    if (traceAnalyzer->ShouldCollectTrace() && frameProfiler->ShouldCollectGPUTime())
    {
        // Collect calibration timestamps in case we need to align GPU events against the CPU timeline.
        if (calibrationTimestamps.mQueueCanBeTimestamped)
        {
            frameProfiler->CollectCalibrationTimestamps(this, &calibrationTimestamps);
        }
        else
        {
            Log(logTRACE, "Did not collect calibration timestamps for Queue '0x%p'. It was not a Direct or Compute Queue.\n", this);
        }

        for (UINT i = 0; i < NumCommandLists; i++)
        {
            Wrapped_ID3D12GraphicsCommandListCustom* pCustomCmdList = static_cast<Wrapped_ID3D12GraphicsCommandListCustom*>(ppCommandLists[i]);

            if (pCustomCmdList != nullptr)
            {
                pCustomCmdList->SetProfilerExecutionId(m_executionID);
            }
        }
    }

    // Invoke the real call to execute on the GPU.
    Wrapped_ID3D12CommandQueue::ExecuteCommandLists(NumCommandLists, ppCommandLists);

    if (traceAnalyzer->ShouldCollectTrace() && frameProfiler->ShouldCollectGPUTime())
    {
        // Collect the CPU and GPU frequency to convert timestamps.
        QueryPerformanceFrequency(&calibrationTimestamps.cpuFrequency);

        // Signal the next fence value (with the GPU)
        mRealCommandQueue->Signal(m_QSD.pFence, m_QSD.nextFenceValue);

#if GATHER_PROFILER_RESULTS_WITH_WORKERS
        SpawnWorker(&calibrationTimestamps, this, NumCommandLists, ppCommandLists);
#else
        DX12Util::WaitForFence(m_QSD.nextFenceValue, m_QSD.pFence, m_QSD.fenceEvent);
#endif
        m_QSD.nextFenceValue++;

#if GATHER_PROFILER_RESULTS_WITH_WORKERS == 0

        if (calibrationTimestamps.mQueueCanBeTimestamped)
        {
            std::vector<Wrapped_ID3D12GraphicsCommandListCustom*> cmdListsWithProfiledCalls;
            GatherProfiledCommandLists(NumCommandLists, ppCommandLists, cmdListsWithProfiledCalls);

            // Put all results into thread ID 0 bucket
            const UINT32 threadID = 0;
            std::vector<ProfilerResult> results;

            for (UINT i = 0; i < cmdListsWithProfiledCalls.size(); i++)
            {
                ProfilerResultCode getResultsResult = PROFILER_FAIL;
                getResultsResult = cmdListsWithProfiledCalls[i]->GetCmdListResultsST(this->mRealCommandQueue, results);
                PROFILER_ASSERT(getResultsResult != PROFILER_FAIL);
            }

            frameProfiler->VerifyAlignAndStoreResults(this, results, &calibrationTimestamps, threadID, DX12TraceAnalyzerLayer::Instance()->GetFrameStartTime());
        }
        else
        {
            Log(logTRACE, "Didn't collect calibration timestamps for Queue '0x%p'. Wasn't a Direct or Compute Queue.\n", this);
        }

#endif
    }

    // The incoming array of CommandLists is tracked order to know which Queue executed which CommandLists.
    frameProfiler->TrackParentCommandQueue(this, NumCommandLists, reinterpret_cast<Wrapped_ID3D12GraphicsCommandList* const*>(ppCommandLists));
}

//-----------------------------------------------------------------------------
/// Kill all info retained by this thread.
//-----------------------------------------------------------------------------
void Wrapped_ID3D12CommandQueueCustom::EndCollection()
{
    ScopeLock lock(&m_workerThreadInfoMutex);

    for (UINT i = 0; i < m_workerThreadInfo.size(); i++)
    {
        // Delete profiler memory
        for (UINT j = 0; j < m_workerThreadInfo[i]->m_inputs.commandListsWithProfiledCalls.size(); j++)
        {
            Wrapped_ID3D12GraphicsCommandListCustom* pCmdList = static_cast<Wrapped_ID3D12GraphicsCommandListCustom*>(m_workerThreadInfo[i]->m_inputs.commandListsWithProfiledCalls[j]);

            if (pCmdList != nullptr)
            {
                pCmdList->DestroyProfilers();
            }
        }

        m_workerThreadInfo[i]->m_outputs.results.clear();

        CloseHandle(m_workerThreadInfo[i]->m_threadInfo.threadHandle);
        SAFE_DELETE(m_workerThreadInfo[i]);
    }

    m_workerThreadInfo.clear();
}