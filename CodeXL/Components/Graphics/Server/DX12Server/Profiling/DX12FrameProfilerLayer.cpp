//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12FrameProfilerLayer.cpp
/// \brief  The DX12-specific Frame Profiler layer implementation.
//=============================================================================

#include "DX12FrameProfilerLayer.h"
#include "../DX12LayerManager.h"
#include "../Tracing/DX12TraceAnalyzerLayer.h"
#include "../Objects/DX12ObjectDatabaseProcessor.h"
#include "../Objects/CustomWrappers/Wrapped_ID3D12GraphicsCommandListCustom.h"
#include "../Objects/CustomWrappers/Wrapped_ID3D12CommandQueueCustom.h"
#include "../Objects/BaseWrappers/Wrapped_ID3D12Device.h"
#include "../Objects/BaseWrappers/Wrapped_ID3D12CommandList.h"
#include "../Tracing/DX12APIEntry.h"

//-----------------------------------------------------------------------------
/// Default constructor for DX12FrameProfilerLayer.
//-----------------------------------------------------------------------------
DX12FrameProfilerLayer::DX12FrameProfilerLayer() :
    ModernAPIFrameProfilerLayer()
{
    SetProfiledFuncs();
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the parent LayerManager used by this tool.
/// \returns A pointer to the parent LayerManager used by this tool.
//-----------------------------------------------------------------------------
ModernAPILayerManager* DX12FrameProfilerLayer::GetParentLayerManager()
{
    return DX12LayerManager::Instance();
}

//-----------------------------------------------------------------------------
/// Called to indicate that a resource is being created
/// The layer must create its resources and hook functions here
/// \param type the type of resource that is being created
/// \param pPtr pointer to the resource that is being created
/// \return should return false on error; true otherwise
//-----------------------------------------------------------------------------
bool DX12FrameProfilerLayer::OnCreate(CREATION_TYPE type, void* pPtr)
{
    PS_UNREFERENCED_PARAMETER(type);
    PS_UNREFERENCED_PARAMETER(pPtr);

    bool bInitSuccessful = false;

    // Don't initialize the Frame Profiler more than once.
    static bool bAlreadyInitialized = false;

    if (bAlreadyInitialized == false)
    {
        bInitSuccessful = true;
        bAlreadyInitialized = true;
    }

    return bInitSuccessful;
}

//-----------------------------------------------------------------------------
/// Called to indicate that a resource is being destroyed
/// detaches from anything that was attached in OnCreate
/// \param type the type of resource that is being destroyed
/// \param pPtr pointer to the resource that is being destroyed
/// \return should return false on error; true otherwise
//-----------------------------------------------------------------------------
bool DX12FrameProfilerLayer::OnDestroy(CREATION_TYPE type, void* pPtr)
{
    PS_UNREFERENCED_PARAMETER(pPtr);
    PS_UNREFERENCED_PARAMETER(type);

    return true;
}

//-----------------------------------------------------------------------------
/// Clear out all of the ProfilerResults collected by the DX12Profiler.
/// IMPORTANT NOTE: All worker threads accessing profile results data
/// must have terminated and results must have been gathered and sent back
/// to the client before calling this function
//-----------------------------------------------------------------------------
void DX12FrameProfilerLayer::ClearProfilingResults()
{
    ProfilerResultsMap::iterator itemIter;
    ProfilerResultsMap::iterator beginIter = mEntriesWithProfilingResults.begin();
    ProfilerResultsMap::iterator endIter = mEntriesWithProfilingResults.end();

    for (itemIter = beginIter; itemIter != endIter; ++itemIter)
    {
        QueueWrapperToProfilingResultsMap threadMap = itemIter->second;
        QueueWrapperToProfilingResultsMap::iterator threadIter;
        QueueWrapperToProfilingResultsMap::iterator threadBegin = threadMap.begin();
        QueueWrapperToProfilingResultsMap::iterator threadEnd = threadMap.end();

        // Kill the map associated with each Queue type.
        for (threadIter = threadBegin; threadIter != threadEnd; ++threadIter)
        {
            SampleIdToProfilerResultMap* pResultMap = threadIter->second;

            SampleIdToProfilerResultMap::iterator resultIter;
            SampleIdToProfilerResultMap::iterator resultBegin = pResultMap->begin();
            SampleIdToProfilerResultMap::iterator resultEnd = pResultMap->end();

            for (resultIter = resultBegin; resultIter != resultEnd; ++resultIter)
            {
                SAFE_DELETE(resultIter->second);
            }

            SAFE_DELETE(pResultMap);
        }

        threadMap.clear();
    }

    // It should be safe to kill this now that all of the entries are destroyed.
    mEntriesWithProfilingResults.clear();

    mSampleIdToEntry.clear();
}

//-----------------------------------------------------------------------------
/// Construct a measurement info structure for each call that will be profiled.
/// \param inFuncId The function ID for the function being profiled.
/// \param inSampleId The SampleID associated with the profiled command.
/// \param pCmdListWrapped The command list that executed the profiled command.
/// \param out A ProfilerMeasurementId containing metadata for the new measurement.
//-----------------------------------------------------------------------------
void DX12FrameProfilerLayer::ConstructMeasurementInfo(FuncId inFuncId, UINT64 inSampleId, Wrapped_ID3D12GraphicsCommandListCustom* pCmdListWrapped, ProfilerMeasurementId& out)
{
    out.pWrappedCmdList = pCmdListWrapped;
    out.mCmdListType    = pCmdListWrapped->GetCmdListType();
    out.mSampleId       = inSampleId;
    out.mFrameIndex     = GetParentLayerManager()->GetCurrentFrameIndex();
    out.mFunctionId     = inFuncId;
}

//-----------------------------------------------------------------------------
/// Validate the contents of a profiler result.
/// \param result The result to validate.
/// \returns True if the profiler result looks good.
//-----------------------------------------------------------------------------
bool DX12FrameProfilerLayer::ValidateProfilerResult(const ProfilerResult& result)
{
    bool validResult = true;

    // Verify that the raw clock timestamps are non-zero.
    if ((result.timestampResult.rawClocks.start == 0ULL) || (result.timestampResult.rawClocks.end == 0ULL))
    {
        validResult = false;
    }

    // Verify that start < end.
    if (result.timestampResult.rawClocks.start > result.timestampResult.rawClocks.end)
    {
        validResult = false;
    }

    return validResult;
}

//-----------------------------------------------------------------------------
/// Verify, align, and store the new profiler results.
/// \param pQueue The Queue used to collect the results to verify.
/// \param results The vector of profiler results to verify.
/// \param pTimestampPair A pair of calibration timestamps used to align CPU and GPU timestamps.
/// \param threadID The ThreadId that the results are collected with.
/// \param frameStartTime The start time of the frame as collected on the CPU.
//-----------------------------------------------------------------------------
void DX12FrameProfilerLayer::VerifyAlignAndStoreResults(
    Wrapped_ID3D12CommandQueue*  pQueue,
    std::vector<ProfilerResult>& results,
    CalibrationTimestampPair*    pTimestampPair,
    UINT32                       threadID,
    GPS_TIMESTAMP                frameStartTime)
{
    SampleIdToProfilerResultMap* pResultMap = FindOrCreateProfilerResultsMap(pQueue, threadID);
    PsAssert(pResultMap != nullptr);

    if (pResultMap != nullptr)
    {
        for (size_t resultIndex = 0; resultIndex < results.size(); ++resultIndex)
        {
            ProfilerResult& currentResult = results[resultIndex];

            const UINT64 sampleId = currentResult.measurementInfo.idInfo.mSampleId;

            // Verify that the timestamps retrieved from the profiler appear to be valid.
            if (ValidateProfilerResult(currentResult) == true)
            {
                // Assign single clock duration, for equal bottom-bottom clock case
                if (currentResult.timestampResult.rawClocks.start == currentResult.timestampResult.rawClocks.end)
                {
                    currentResult.timestampResult.rawClocks.end++;
                }

                // Now attempt to align the profiled GPU timestamps with the traced API calls on the CPU.
                bool bAlignedSuccessfully = AlignProfilerResultWithCPUTimeline(currentResult, pTimestampPair, frameStartTime);

                if (bAlignedSuccessfully)
                {
                    // Store the final adjusted profiler results if they're valid.
                    ProfilerResult* pNewResult = new ProfilerResult;
                    CopyProfilerResult(pNewResult, &currentResult);
                    (*pResultMap)[sampleId] = pNewResult;
                }
                else
                {
                    Log(logERROR, "Command with SampleId %d failed to align with CPU timeline.\n", sampleId);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Set the internal flag that determines if GPU command profiling is enabled.
/// \param inbProfilingEnabled The flag used to enable or disable profiling.
//-----------------------------------------------------------------------------
void DX12FrameProfilerLayer::SetProfilingEnabled(bool inbProfilingEnabled)
{
    ModernAPIFrameProfilerLayer::SetProfilingEnabled(inbProfilingEnabled);

    for (CommandQueueToCommandListMap::iterator it = mCommandQueueTracker.begin();
         it != mCommandQueueTracker.end();
         ++it)
    {
        Wrapped_ID3D12Device* pDevice = static_cast<Wrapped_ID3D12Device*>(DX12Util::SafeGetDevice(it->first));

        if (pDevice != nullptr)
        {
            HRESULT stateSet = E_FAIL;
            stateSet = pDevice->mRealDevice->SetStablePowerState(inbProfilingEnabled);
            PsAssert(stateSet == S_OK);

            pDevice->Release();
        }
    }
}

//-----------------------------------------------------------------------------
/// Insert an APIEntry into the list of entries with profiler results.
/// \param inEntry An APIEntry with a sampled GPU time.
//-----------------------------------------------------------------------------
void DX12FrameProfilerLayer::StoreProfilerResult(DX12APIEntry* inEntry)
{
    UINT32 threadId = osGetCurrentThreadId();

    if (mSampleIdToEntry.find(threadId) == mSampleIdToEntry.end())
    {
        // create a new entry
        SampleIdToAPIEntryMap newMap;
        newMap[inEntry->mSampleId] = inEntry;

        // adding a new thread to the map so need to lock
        ScopeLock profilerResultsLock(&mProfilingResultsMutex);

        mSampleIdToEntry[threadId] = newMap;
    }
    else
    {
        SampleIdToAPIEntryMap& mapEntry = mSampleIdToEntry[threadId];
        mapEntry[inEntry->mSampleId] = inEntry;
    }
}

//-----------------------------------------------------------------------------
/// Given a SampleId, find the DX12APIEntry that was logged while API tracing.
/// \param inSampleId The SampleId to search with.
/// \return The buffered DX12APIEntry with a matching SampleId, or nullptr if it doesn't exist for some reason.
//-----------------------------------------------------------------------------
DX12APIEntry* DX12FrameProfilerLayer::FindInvocationBySampleId(UINT64 inSampleId)
{
    DX12APIEntry* apiEntry = nullptr;

    ThreadSafeSampleIdToAPIEntryMap::iterator threadIter;

    for (threadIter = mSampleIdToEntry.begin(); threadIter != mSampleIdToEntry.end(); ++threadIter)
    {
        SampleIdToAPIEntryMap& entryMap = threadIter->second;

        SampleIdToAPIEntryMap::iterator sampleIdIter = entryMap.find(inSampleId);

        if (sampleIdIter != entryMap.end())
        {
            apiEntry = sampleIdIter->second;
            break;
        }
    }

    return apiEntry;
}

//-----------------------------------------------------------------------------
/// Associate a set of CommandLists with the CommandQueue that they'll be executed through.
/// \param inCommandQueueWrapper A CommandQueue instance used to execute CommandLists.
/// \param inNumCommandLists The number of CommandLists in the incoming array.
/// \param inCommandLists An array of CommandLists to associate with the incoming CommandQueue.
//-----------------------------------------------------------------------------
void DX12FrameProfilerLayer::TrackParentCommandQueue(Wrapped_ID3D12CommandQueue* inCommandQueueWrapper, UINT inNumCommandLists, Wrapped_ID3D12GraphicsCommandList* const* inCommandLists)
{
    CommandQueueToCommandListMap::iterator wrappedQueueIter = mCommandQueueTracker.find(inCommandQueueWrapper);

    if (wrappedQueueIter != mCommandQueueTracker.end())
    {
        // Already found it in the map. Add to the existing vector.
        WrappedCommandListSet& commandListVector = wrappedQueueIter->second;
        commandListVector.insert(inCommandLists, inCommandLists + inNumCommandLists);
    }
    else
    {
        // Not in the map yet- create a new vector and add these instances.
        ScopeLock commandQueueLocker(&mCommandQueueLockMutex);
        WrappedCommandListSet& commandListVector = mCommandQueueTracker[inCommandQueueWrapper];
        commandListVector.insert(inCommandLists, inCommandLists + inNumCommandLists);
    }
}

//-----------------------------------------------------------------------------
/// Handles operations that need to occur before profiling an API call.
/// \param inWrappedInterface The interface pointer used to invoke the API call.
/// \param inFunctionId The FuncId corresponding to the API call being traced.
//-----------------------------------------------------------------------------
void DX12FrameProfilerLayer::PreCall(IUnknown* inWrappedInterface, FuncId inFunctionId)
{
    Wrapped_ID3D12GraphicsCommandListCustom* pWrappedGraphicsCommandListCustom = static_cast<Wrapped_ID3D12GraphicsCommandListCustom*>(inWrappedInterface);

    // Check if we intend to collect GPU time
    if (pWrappedGraphicsCommandListCustom->IsProfilingEnabled() && ShouldProfileFunction(inFunctionId))
    {
        osThreadId threadId = osGetCurrentThreadId();
        SampleInfo* pSampleInfo = GetSampleInfoForThread(threadId);

        if (pSampleInfo != nullptr)
        {
            // Don't let new sampling begin until this sample is finished with a call to EndSample.
            Wrapped_ID3D12CommandList* pCmdList = static_cast<Wrapped_ID3D12CommandList*>(inWrappedInterface);

            // Only some types of CommandLists can be timestamped. Check the type before proceeding.
            if (CommandListSupportsTimestamping(pCmdList) == true)
            {
                UINT64 nextSampleId = SetNextSampleId(pSampleInfo);

                ProfilerMeasurementId measurementId = {};
                ConstructMeasurementInfo(inFunctionId, nextSampleId, pWrappedGraphicsCommandListCustom, measurementId);

                ProfilerResultCode beginResult = pWrappedGraphicsCommandListCustom->BeginCmdMeasurement(&measurementId);

                if (beginResult == PROFILER_SUCCESS)
                {
                    pSampleInfo->mSampleId = measurementId.mSampleId;
                    pSampleInfo->mbBeginSampleSuccessful = true;
                }
                else
                {
                    Log(logERROR, "Failed BeginCmdMeasurement. CmdList='0x%p' SampleId='%d'\n", pWrappedGraphicsCommandListCustom->mRealGraphicsCommandList, measurementId.mSampleId);
                }
            }
        }
        else
        {
            Log(logERROR, "Failed to find or create SampleInfo instance for Thread %d\n", threadId);
        }
    }
}

//-----------------------------------------------------------------------------
/// Handler used after the real runtime implementation of an API call has been invoked.
/// \param inNewAPIEntry The new APIEntry created for the API call invocation.
/// \param inWrappedInterface The interface pointer used to invoke the API call.
/// \param inFunctionId The FuncId corresponding to the API call being traced.
//-----------------------------------------------------------------------------
void DX12FrameProfilerLayer::PostCall(DX12APIEntry* inNewAPIEntry, IUnknown* inWrappedInterface, FuncId inFunctionId)
{
    Wrapped_ID3D12GraphicsCommandListCustom* pWrappedGraphicsCommandListCustom = static_cast<Wrapped_ID3D12GraphicsCommandListCustom*>(inWrappedInterface);

    // Wait and gather results
    if (pWrappedGraphicsCommandListCustom->IsProfilingEnabled() && ShouldProfileFunction(inFunctionId))
    {
        Wrapped_ID3D12GraphicsCommandList* pWrappedGraphicsCommandList = static_cast<Wrapped_ID3D12GraphicsCommandList*>(inWrappedInterface);

        osThreadId threadId = osGetCurrentThreadId();

        Wrapped_ID3D12CommandList* pCmdList = static_cast<Wrapped_ID3D12CommandList*>(inWrappedInterface);

        if (CommandListSupportsTimestamping(pCmdList) == true)
        {
            SampleInfo* pSampleInfo = GetSampleInfoForThread(threadId);

            if (pSampleInfo != nullptr)
            {
                if (pSampleInfo->mbBeginSampleSuccessful == true)
                {
                    Wrapped_ID3D12GraphicsCommandListCustom* pCmdListCustom = static_cast<Wrapped_ID3D12GraphicsCommandListCustom*>(inWrappedInterface);

                    ProfilerResultCode endResult = pCmdListCustom->EndCmdMeasurement();

                    if (endResult == PROFILER_SUCCESS)
                    {
                        inNewAPIEntry->mSampleId = pSampleInfo->mSampleId;
                        StoreProfilerResult(inNewAPIEntry);
                    }
                    else
                    {
                        Log(logERROR, "Failed EndCmdMeasurement. CmdList='0x%p' SampleId='%d'\n", pWrappedGraphicsCommandList->mRealGraphicsCommandList, pSampleInfo->mSampleId);
                    }
                }
                else
                {
                    Log(logERROR, "Didn't call EndMeasurement because BeginMeasurement wasn't successful.\n");
                }
            }
            else
            {
                Log(logERROR, "Didn't call EndSample because there was no SampleInfo for Thread %d\n", threadId);
            }
        }
        else
        {
            Log(logERROR, "Failed to find or create SampleInfo instance for Thread %d\n", threadId);
        }
    }
}

//-----------------------------------------------------------------------------
/// Initialize the list of functions that we want to profile for GPU time.
/// Functions set to 'true' within this list be profiled and presented within the GPU trace.
//-----------------------------------------------------------------------------
void DX12FrameProfilerLayer::SetProfiledFuncs()
{
    // Set the whole block of switches to false.
    memset(&mbProfiledFuncs, 0, sizeof(bool) * FuncId_MAX);

    // Draws
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_DrawInstanced] = true;
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_DrawIndexedInstanced] = true;

    // Compute
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_Dispatch] = true;

    // Copies
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_CopyBufferRegion] = true;
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_CopyTextureRegion] = true;
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_CopyResource] = true;

    // Execute
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_ExecuteBundle] = true;
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_ExecuteIndirect] = true;

    // Resolves
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_ResolveSubresource] = true;
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_ResolveQueryData] = true;

    // Clears
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_ClearRenderTargetView] = true;
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_ClearDepthStencilView] = true;
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewUint] = true;
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_ClearUnorderedAccessViewFloat] = true;

    // Barrier
    mbProfiledFuncs[FuncId_ID3D12GraphicsCommandList_ResourceBarrier] = true;
}

//-----------------------------------------------------------------------------
/// Get the map used to associate queue with its command lists.
/// \param inQueue Input queue.
/// \returns The CommandQueueToCommandListMap used to associate a Queue with the CommandLists it utilizes.
//-----------------------------------------------------------------------------
D3D12_COMMAND_LIST_TYPE DX12FrameProfilerLayer::GetCommandListTypeFromCommandQueue(Wrapped_ID3D12CommandQueue* inQueue)
{
    // Use the metadata object to retrieve the cached CreateInfo, and then pull the type out. Start with "Direct".
    D3D12_COMMAND_LIST_TYPE commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;

    PsAssert(inQueue != nullptr);

    if (inQueue != nullptr)
    {
        DX12ObjectDatabaseProcessor* databaseProcessor = DX12ObjectDatabaseProcessor::Instance();
        DX12WrappedObjectDatabase* objectDatabase = static_cast<DX12WrappedObjectDatabase*>(databaseProcessor->GetObjectDatabase());

        IDX12InstanceBase* queueMetadata = objectDatabase->GetMetadataObject((IUnknown*)inQueue);
        Wrapped_ID3D12CommandQueueCreateInfo* queueCreateInfo = static_cast<Wrapped_ID3D12CommandQueueCreateInfo*>(queueMetadata->GetCreateInfoStruct());
        D3D12_COMMAND_QUEUE_DESC* queueDesc = queueCreateInfo->GetDescription();
        commandListType = queueDesc->Type;
    }

    return commandListType;
}

//-----------------------------------------------------------------------------
/// Take one result from DX12CmdListProfiler and scale the timestamp based on the incoming calibration timestamps.
/// \param ioResult The ProfilerResult instance to align with the CPU timeline.
/// \param pTimestamps A calibration timestamp structure used to align GPU events along the CPU timeline.
/// \param inFrameStartTime The starting time for the frame, measured by the CPU.
/// \returns True or false, indicating the success of the alignment operation.
//-----------------------------------------------------------------------------
bool DX12FrameProfilerLayer::AlignProfilerResultWithCPUTimeline(ProfilerResult& ioResult, const CalibrationTimestampPair* pTimestamps, GPS_TIMESTAMP inFrameStartTime)
{
    bool bAlignmentResult = false;

    if (pTimestamps != nullptr)
    {
        // The "DeltaStartTime" represents
        double cpuClockFrequency = (double)pTimestamps->cpuFrequency.QuadPart;

        double cpuStartMillisecond = (double)(static_cast<double>(pTimestamps->mBeforeExecutionCPUTimestamp) * 1000.0) / cpuClockFrequency;
        double gpuMillisecondAtBeforeExecution = (static_cast<double>(pTimestamps->mBeforeExecutionGPUTimestamp) * 1000.0) / static_cast<double>(pTimestamps->mQueueFrequency);

        // Extract the raw clock cycles from the profiler result, and convert them into GPU Milliseconds.
        double gpuMillisecondAtResultStart = (static_cast<double>(ioResult.timestampResult.rawClocks.start)    * 1000.0) / static_cast<double>(pTimestamps->mQueueFrequency);
        double gpuMillisecondAtResultEnd = (static_cast<double>(ioResult.timestampResult.rawClocks.end)      * 1000.0) / static_cast<double>(pTimestamps->mQueueFrequency);

        // Now compute the GPU timeline's delta between the "Before Execution GPU Timestamp" and the Start and End millisecond in the GPU timeline.
        double gpuMillisecondAtDeltaStart = (gpuMillisecondAtResultStart - gpuMillisecondAtBeforeExecution);
        double gpuMillisecondAtDeltaEnd = (gpuMillisecondAtResultEnd - gpuMillisecondAtBeforeExecution);

        // Compute the final profiled command's Start and End time by adding the item duration to the "Before CPU Execution" start time.
        double alignedStart = gpuMillisecondAtDeltaStart + cpuStartMillisecond;
        double alignedEnd = gpuMillisecondAtDeltaEnd + cpuStartMillisecond;

        // Take the frame start time into account
        double frameStartOffset = (inFrameStartTime.QuadPart * 1000.0) / cpuClockFrequency;

        alignedStart -= frameStartOffset;
        alignedEnd -= frameStartOffset;

        // Verify that the timestamps are larger than zero.
        if (alignedStart >= 0.0 && alignedEnd >= 0.0)
        {
            ioResult.timestampResult.alignedMillisecondTimestamps.start = alignedStart;
            ioResult.timestampResult.alignedMillisecondTimestamps.end = alignedEnd;

            bAlignmentResult = true;
        }
    }

    return bAlignmentResult;
}

//-----------------------------------------------------------------------------
/// Collect and store calibration timestamps from the CPU and GPU to align execution results in a single timeline.
/// \param inSubmissionQueue The Queue responsible for work submission.
/// \param ioTimestamps The timestamps structure used to hold timestamps occurring before and after workload execution.
//-----------------------------------------------------------------------------
void DX12FrameProfilerLayer::CollectCalibrationTimestamps(Wrapped_ID3D12CommandQueueCustom* inSubmissionQueue, CalibrationTimestampPair* ioTimestamps)
{
    // Collect the GPU clock calibration timestamps.
    UINT64 gpuTime;
    UINT64 cpuTime;

    HRESULT gotClockCalibration = inSubmissionQueue->mRealCommandQueue->GetClockCalibration(&gpuTime, &cpuTime);

    if (gotClockCalibration != S_OK)
    {
        Log(logWARNING, "Call to ID3D12CommandQueue->GetClockCalibration failed when attempting to collect device calibration timestamps.\n");
    }

    // Collect the clock frequency for this GPU queue.
    UINT64 queueFrequency;
    HRESULT gotDeviceFrequency = inSubmissionQueue->mRealCommandQueue->GetTimestampFrequency(&queueFrequency);

    if (gotDeviceFrequency != S_OK)
    {
        Log(logERROR, "Failed to query GPU frequency. CPU and GPU timestamps may be misaligned as a side-effect.\n");
    }

    // Dump the results into the output structure
    ioTimestamps->mBeforeExecutionGPUTimestamp = gpuTime;
    ioTimestamps->mBeforeExecutionCPUTimestamp = cpuTime;

    ioTimestamps->mQueueFrequency = queueFrequency;
}

//-----------------------------------------------------------------------------
/// Copy a profiler result from one chunk of memory to another.
/// \param inDestination The destination ProfilerResults memory to copy to.
/// \param inSource The source memory to copy from.
//-----------------------------------------------------------------------------
void DX12FrameProfilerLayer::CopyProfilerResult(ProfilerResult* inDestination, const ProfilerResult* inSource)
{
    // Copy the measurement info.
    memcpy(&inDestination->measurementInfo, &inSource->measurementInfo, sizeof(ProfilerMeasurementInfo));
    memcpy(&inDestination->timestampResult, &inSource->timestampResult, sizeof(ProfilerTimestampResult));
    //memcpy(&inDestination->pipeStatsResult, &inSource->pipeStatsResult, sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS));
}

//-----------------------------------------------------------------------------
/// Create a new map to associate a LinkId with profiling results.
/// \param pWrappedQueue The Queue that was submitted in order to retrieve profiling results.
/// \param inThreadId The thread being used to create a results map.
/// \returns A SampleIdToProfilerResultMap instance used to store profiler results.
//-----------------------------------------------------------------------------
SampleIdToProfilerResultMap* DX12FrameProfilerLayer::FindOrCreateProfilerResultsMap(Wrapped_ID3D12CommandQueue* pWrappedQueue, UINT32 inThreadId)
{
    SampleIdToProfilerResultMap* pResultMap = nullptr;

    if (mEntriesWithProfilingResults.count(inThreadId) == 0)
    {
        // make the new map data for this thread
        QueueWrapperToProfilingResultsMap queueMap;
        pResultMap = new SampleIdToProfilerResultMap();
        queueMap.insert(make_pair(pWrappedQueue, pResultMap));
        {
            // Lock before we insert something new into this map.
            ScopeLock profilerResultsLock(&mProfilingResultsMutex);
            mEntriesWithProfilingResults[inThreadId] = queueMap;
        }
    }
    else
    {
        QueueWrapperToProfilingResultsMap& queueMap = mEntriesWithProfilingResults[inThreadId];

        QueueWrapperToProfilingResultsMap::const_iterator queueIt = queueMap.find(pWrappedQueue);

        if (queueIt != queueMap.end())
        {
            return queueIt->second;
        }
        else
        {
            pResultMap = new SampleIdToProfilerResultMap();
            queueMap.insert(make_pair(pWrappedQueue, pResultMap));
        }
    }

    return pResultMap;
}

//-----------------------------------------------------------------------------
/// Determine if a command list may be timestamped.
/// \param pCmdList The incoming CommandList to check support for timestamping.
/// \returns True if the CommandList can be timestamped. False if it cannot.
//-----------------------------------------------------------------------------
bool DX12FrameProfilerLayer::CommandListSupportsTimestamping(Wrapped_ID3D12CommandList* pCmdList)
{
    bool timeStampSupported = false;

    const D3D12_COMMAND_LIST_TYPE cmdListType = pCmdList->mRealCommandList->GetType();

#if TIMESTAMP_DIRECT_COMMAND_LISTS

    if (cmdListType == D3D12_COMMAND_LIST_TYPE_DIRECT)
    {
        timeStampSupported = true;
    }

#endif

#if TIMESTAMP_COMPUTE_COMMAND_LISTS

    if (cmdListType == D3D12_COMMAND_LIST_TYPE_COMPUTE)
    {
        timeStampSupported = true;
    }

#endif

    return timeStampSupported;
}