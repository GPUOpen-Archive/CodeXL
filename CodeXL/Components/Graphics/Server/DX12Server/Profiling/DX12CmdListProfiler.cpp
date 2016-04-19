//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12CmdListProfiler.cpp
/// \brief  DX12 command list profiler.
///         This standalone class injects queries into app command buffers
///         to determine GPU time and counter information.
//=============================================================================

#include "DX12CmdListProfiler.h"

//-----------------------------------------------------------------------------
/// Static method that instantiates a DX12 profiler.
/// \param pConfig A profiler configuration structure.
/// \returns A new DX12CmdListProfiler instance.
//-----------------------------------------------------------------------------
DX12CmdListProfiler* DX12CmdListProfiler::Create(const DX12CmdListProfilerConfig* pConfig)
{
    DX12CmdListProfiler* pOut = new DX12CmdListProfiler();

    if (pOut != nullptr)
    {
        if (pOut->Init(pConfig) != S_OK)
        {
            delete pOut;
            pOut = nullptr;
        }
    }

    return pOut;
}

//-----------------------------------------------------------------------------
/// Constructor.
//-----------------------------------------------------------------------------
DX12CmdListProfiler::DX12CmdListProfiler() :
    m_executionId(-1)
{
    ClearCmdListData();
}

//-----------------------------------------------------------------------------
/// Perform all profiler initialization.
/// \param pConfig A pointer to a profiler configuration structure.
/// \returns The result code for initialization.
//-----------------------------------------------------------------------------
HRESULT DX12CmdListProfiler::Init(const DX12CmdListProfilerConfig* pConfig)
{
    HRESULT result = E_FAIL;

    if ((pConfig != nullptr) && (pConfig->pDevice != nullptr) && (pConfig->pCmdList != nullptr))
    {
        if (pConfig->measurementsPerGroup > 0)
        {
            memcpy(&m_config, pConfig, sizeof(DX12CmdListProfilerConfig));

            result = S_OK;
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
DX12CmdListProfiler::~DX12CmdListProfiler()
{
    while (m_deletionQueue.empty() == false)
    {
        ReleaseStaleResourceGroup(m_deletionQueue.front());
    }

    m_cmdListData.measurementGroups.clear();

    ClearCmdListData();
}

//-----------------------------------------------------------------------------
/// Begin profiling a GPU command.
/// \param pIdInfo The identifying metadata for the new measurement.
/// \returns A profiler result code indicating measurement success.
//-----------------------------------------------------------------------------
ProfilerResultCode DX12CmdListProfiler::BeginCmdMeasurement(const ProfilerMeasurementId* pIdInfo)
{
    PROFILER_ASSERT(pIdInfo != nullptr);

    ProfilerResultCode profilerResultCode = PROFILER_FAIL;

    if (m_cmdListData.state != PROFILER_STATE_MEASUREMENT_BEGAN)
    {
        const UINT measurementId = m_cmdListData.cmdListMeasurementCount % m_config.measurementsPerGroup;

        // Create new measurement group if full
        if (measurementId == 0)
        {
            HRESULT result = E_FAIL;
            result = SetupNewMeasurementGroup();
            PROFILER_ASSERT(result == S_OK);
        }

        if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_PIPE_STATS)
        {
            // Begin pipe stats query
            m_config.pCmdList->BeginQuery(m_cmdListData.pActiveMeasurementGroup->gpuRes.pPipeStatsQueryHeap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, measurementId);
        }

        if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
        {
            const UINT offset = measurementId * 2;

            // Inject timestamp
            m_config.pCmdList->EndQuery(m_cmdListData.pActiveMeasurementGroup->gpuRes.pTimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, offset);
        }

        m_cmdListData.cmdListMeasurementCount++;

        // Add a new measurement
        ProfilerMeasurementInfo clientData = {};
        clientData.measurementNum = m_cmdListData.cmdListMeasurementCount;

        if (pIdInfo != nullptr)
        {
            memcpy(&clientData.idInfo, pIdInfo, sizeof(ProfilerMeasurementId));
        }

        m_cmdListData.pActiveMeasurementGroup->measurementInfos.push_back(clientData);

        m_cmdListData.pActiveMeasurementGroup->groupMeasurementCount++;

        m_cmdListData.state = PROFILER_STATE_MEASUREMENT_BEGAN;

        profilerResultCode = PROFILER_SUCCESS;
    }
    else
    {
        profilerResultCode = PROFILER_ERROR_MEASUREMENT_ALREADY_BEGAN;

        PROFILER_ASSERT_ALWAYS();
    }

    return profilerResultCode;
}

//-----------------------------------------------------------------------------
/// End profiling a GPU command.
/// \returns A profiler result code indicating measurement success.
//-----------------------------------------------------------------------------
ProfilerResultCode DX12CmdListProfiler::EndCmdMeasurement()
{
    ProfilerResultCode profilerResultCode = PROFILER_FAIL;

    if (m_cmdListData.state == PROFILER_STATE_MEASUREMENT_BEGAN)
    {
        const UINT measurementId = (m_cmdListData.cmdListMeasurementCount - 1) % m_config.measurementsPerGroup;

        // Inject timestamp
        if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
        {
            const UINT offset = (measurementId * 2) + 1;

            m_config.pCmdList->EndQuery(m_cmdListData.pActiveMeasurementGroup->gpuRes.pTimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, offset);

            if (m_config.resolveOnClose == false)
            {
                m_config.pCmdList->ResolveQueryData(
                    m_cmdListData.pActiveMeasurementGroup->gpuRes.pTimestampQueryHeap,
                    D3D12_QUERY_TYPE_TIMESTAMP,
                    (offset - 1),
                    2,
                    m_cmdListData.pActiveMeasurementGroup->gpuRes.pTimestampBuffer,
                    (offset - 1) * sizeof(UINT64));
            }
        }

        // End pipe stats query
        if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_PIPE_STATS)
        {
            m_config.pCmdList->EndQuery(m_cmdListData.pActiveMeasurementGroup->gpuRes.pPipeStatsQueryHeap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, measurementId);

            if (m_config.resolveOnClose == false)
            {
                m_config.pCmdList->ResolveQueryData(
                    m_cmdListData.pActiveMeasurementGroup->gpuRes.pPipeStatsQueryHeap,
                    D3D12_QUERY_TYPE_PIPELINE_STATISTICS,
                    measurementId,
                    1,
                    m_cmdListData.pActiveMeasurementGroup->gpuRes.pPipeStatsBuffer,
                    measurementId * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS));
            }
        }

        m_cmdListData.state = PROFILER_STATE_MEASUREMENT_ENDED;

        profilerResultCode = PROFILER_SUCCESS;
    }
    else
    {
        profilerResultCode = PROFILER_MEASUREMENT_NOT_STARTED;

        PROFILER_ASSERT_ALWAYS();
    }

    return profilerResultCode;
}

//-----------------------------------------------------------------------------
/// We assume this will be called immediately after a command list has been submitted.
/// \param pCmdQueue The Queue to collect profiler results from.
/// \param results A vector containing performance information for a given function.
/// \returns A code with the result of collecting profiler results for the CommandList.
//-----------------------------------------------------------------------------
ProfilerResultCode DX12CmdListProfiler::GetCmdListResults(
    ID3D12CommandQueue*          pCmdQueue,
    std::vector<ProfilerResult>& results)
{
    ScopeLock lock(&m_mutex);

    ProfilerResultCode profilerResultCode = PROFILER_THIS_CMD_LIST_WAS_NOT_CLOSED;

    HRESULT result = E_FAIL;

    if (m_cmdListData.state == PROFILER_STATE_CMD_LIST_CLOSED)
    {
        UINT64 queueFrequency = 0;
        pCmdQueue->GetTimestampFrequency(&queueFrequency);

        profilerResultCode = PROFILER_THIS_CMD_LIST_WAS_NOT_MEASURED;

        bool containsZeroTimestamp = false;

        // Loop through all measurements for this command list
        for (UINT i = 0; i < m_cmdListData.measurementGroups.size(); i++)
        {
            ProfilerMeasurementGroup& currGroup = m_cmdListData.measurementGroups[i];

            ProfilerInterval* pTimestampData = nullptr;
            D3D12_QUERY_DATA_PIPELINE_STATISTICS* pPipelineStatsData = nullptr;

            if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
            {
                D3D12_RANGE mapRange = {};
                mapRange.Begin = 0;
                mapRange.End = m_config.measurementsPerGroup * sizeof(ProfilerInterval);
                result = currGroup.gpuRes.pTimestampBuffer->Map(0, &mapRange, reinterpret_cast<void**>(&pTimestampData));
            }

            if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_PIPE_STATS)
            {
                D3D12_RANGE mapRange = {};
                mapRange.Begin = 0;
                mapRange.End = m_config.measurementsPerGroup * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS);
                result = currGroup.gpuRes.pPipeStatsBuffer->Map(0, &mapRange, reinterpret_cast<void**>(&pPipelineStatsData));
            }

            // Report no results
            if (m_config.measurementTypeFlags == PROFILER_MEASUREMENT_TYPE_NONE)
            {
                for (UINT j = 0; j < currGroup.groupMeasurementCount; j++)
                {
                    ProfilerResult profilerResult = {};
                    results.push_back(profilerResult);
                }
            }

            // Fetch our results
            else
            {
                profilerResultCode = PROFILER_SUCCESS;

                for (UINT j = 0; j < currGroup.groupMeasurementCount; j++)
                {
                    ProfilerResult profilerResult = {};

                    memcpy(&profilerResult.measurementInfo, &currGroup.measurementInfos[j], sizeof(ProfilerMeasurementInfo));

                    if (pTimestampData != nullptr)
                    {
                        UINT64 baseClock = pTimestampData[0].start;

                        ProfilerInterval* pCurrInterval = (pTimestampData + j);
                        UINT64 timerBegin = pCurrInterval->start;
                        UINT64 timerEnd   = pCurrInterval->end;

                        // Store raw clocks
                        profilerResult.timestampResult.rawClocks.start = timerBegin;
                        profilerResult.timestampResult.rawClocks.end = timerEnd;

                        // Calculate adjusted clocks
                        profilerResult.timestampResult.adjustedClocks.start = timerBegin - baseClock;
                        profilerResult.timestampResult.adjustedClocks.end = timerEnd - baseClock;

                        // Calculate exec time
                        profilerResult.timestampResult.execMicroSecs = static_cast<double>(timerEnd - timerBegin) / queueFrequency;
                        profilerResult.timestampResult.execMicroSecs *= 1000000;

                        // Detected a zero timestamp. Allow this and continue, but some results are invalid.
                        if ((timerBegin == 0ULL) || (timerEnd == 0ULL))
                        {
                            containsZeroTimestamp = true;
                        }
                    }

                    if (pPipelineStatsData != nullptr)
                    {
                        memcpy(&profilerResult.pipeStatsResult, pPipelineStatsData, sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS));
                    }

                    results.push_back(profilerResult);
                }

                D3D12_RANGE unmapRange = {};

                if (pPipelineStatsData != nullptr)
                {
                    currGroup.gpuRes.pPipeStatsBuffer->Unmap(0, &unmapRange);
                }

                if (pTimestampData != nullptr)
                {
                    currGroup.gpuRes.pTimestampBuffer->Unmap(0, &unmapRange);
                }
            }
        }

        if (containsZeroTimestamp == true)
        {
            profilerResultCode = PROFILER_ERROR_MEASUREMENT_CONTAINED_ZEROES;
        }
    }

    // We're done profiling this command list, so reset and we'll start over next time.
    result = ResetProfilerState();
    PROFILER_ASSERT(result == S_OK);

    return profilerResultCode;
}

//-----------------------------------------------------------------------------
/// Notify the profiler that this command list was closed.
//-----------------------------------------------------------------------------
void DX12CmdListProfiler::NotifyCmdListClosure()
{
    m_cmdListData.state = PROFILER_STATE_CMD_LIST_CLOSED;

    if (m_config.resolveOnClose == true)
    {
        for (UINT i = 0; i < m_cmdListData.measurementGroups.size(); i++)
        {
            ProfilerMeasurementGroup& currGroup = m_cmdListData.measurementGroups[i];

            m_config.pCmdList->ResolveQueryData(
                currGroup.gpuRes.pTimestampQueryHeap,
                D3D12_QUERY_TYPE_TIMESTAMP,
                0,
                currGroup.groupMeasurementCount * 2,
                currGroup.gpuRes.pTimestampBuffer,
                0);
        }
    }
}

//-----------------------------------------------------------------------------
/// Notify the profiler that this command list was manually reset.
//-----------------------------------------------------------------------------
void DX12CmdListProfiler::NotifyCmdListReset()
{
    PROFILER_ASSERT((m_cmdListData.state == PROFILER_STATE_INIT) ||
                    (m_cmdListData.state == PROFILER_STATE_CMD_LIST_CLOSED));
}

//-----------------------------------------------------------------------------
/// A utility function that will return result codes as a string.
/// \param resultCode The result code to convert to a string.
/// \returns A stringified version of the incoming result code.
//-----------------------------------------------------------------------------
const char* DX12CmdListProfiler::PrintProfilerResult(ProfilerResultCode resultCode)
{
    const char* pResult = "";

    switch (resultCode)
    {
        case PROFILER_SUCCESS:
            pResult = "PROFILER_SUCCESS";
            break;

        case PROFILER_FAIL:
            pResult = "PROFILER_FAIL";
            break;

        case PROFILER_THIS_CMD_LIST_WAS_NOT_MEASURED:
            pResult = "PROFILER_THIS_CMD_LIST_WAS_NOT_MEASURED";
            break;

        case PROFILER_MEASUREMENT_NOT_STARTED:
            pResult = "PROFILER_MEASUREMENT_NOT_STARTED";
            break;

        case PROFILER_ERROR_MEASUREMENT_ALREADY_BEGAN:
            pResult = "PROFILER_STATE_MEASUREMENT_ALREADY_BEGAN";
            break;

        case PROFILER_THIS_CMD_LIST_WAS_NOT_CLOSED:
            pResult = "PROFILER_THIS_CMD_LIST_WAS_NOT_CLOSED";
            break;

        case PROFILER_ERROR_MEASUREMENT_CONTAINED_ZEROES:
            pResult = "PROFILER_ERROR_MEASUREMENT_CONTAINED_ZEROES";
            break;
    }

    return pResult;
}

//-----------------------------------------------------------------------------
/// Clear out the internal profiled CommandList data.
//-----------------------------------------------------------------------------
void DX12CmdListProfiler::ClearCmdListData()
{
    m_cmdListData.state = PROFILER_STATE_INIT;
    m_cmdListData.cmdListMeasurementCount = 0;
    m_cmdListData.pActiveMeasurementGroup = nullptr;
}

//-----------------------------------------------------------------------------
/// Create a buffer to hold query results.
/// \param pResource The new resource containing the query results.
/// \param size The size of the new resource.
/// \returns The result code for creating a new query buffer.
//-----------------------------------------------------------------------------
HRESULT DX12CmdListProfiler::CreateQueryBuffer(ID3D12Resource** pResource, UINT size)
{
    HRESULT result = E_FAIL;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type                 = D3D12_HEAP_TYPE_READBACK;
    heapProps.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask     = 1;
    heapProps.VisibleNodeMask      = 1;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment          = 0;
    bufferDesc.Width              = size;
    bufferDesc.Height             = 1;
    bufferDesc.DepthOrArraySize   = 1;
    bufferDesc.MipLevels          = 1;
    bufferDesc.Format             = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count   = 1;
    bufferDesc.SampleDesc.Quality = 0;
    bufferDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    result = m_config.pDevice->CreateCommittedResource(
                 &heapProps,
                 D3D12_HEAP_FLAG_NONE,
                 &bufferDesc,
                 D3D12_RESOURCE_STATE_COPY_DEST,
                 nullptr,
                 IID_PPV_ARGS(&(*pResource)));

    if (m_config.newMemClear == true)
    {
        if (result == S_OK)
        {
            ProfilerInterval* pTimestampData = nullptr;

            D3D12_RANGE mapRange = {};
            mapRange.Begin = 0;
            mapRange.End   = size;
            result = (*pResource)->Map(0, &mapRange, reinterpret_cast<void**>(&pTimestampData));

            if (result == S_OK)
            {
                const ProfilerInterval storeVal = { m_config.newMemClearValue, m_config.newMemClearValue };
                const UINT numSets = size / sizeof(storeVal);

                for (UINT i = 0; i < numSets; i++)
                {
                    pTimestampData[i] = storeVal;
                }

                D3D12_RANGE unmapRange = {};
                (*pResource)->Unmap(0, &unmapRange);
            }
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Create a new query heap and memory pair for time stamping.
/// \returns The result code for creating a new measurement group for a CommandList.
//-----------------------------------------------------------------------------
HRESULT DX12CmdListProfiler::SetupNewMeasurementGroup()
{
    HRESULT result = S_OK;

    ProfilerMeasurementGroup measurementGroup = {};

    if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
    {
        result = CreateQueryBuffer(&measurementGroup.gpuRes.pTimestampBuffer,
                                   m_config.measurementsPerGroup * sizeof(ProfilerInterval));

        if (result == S_OK)
        {
            measurementGroup.gpuRes.pTimestampBuffer->SetName(L"Timestamp Buffer");

            D3D12_QUERY_HEAP_DESC queryHeapDesc;
            queryHeapDesc.Count    = m_config.measurementsPerGroup * 2;
            queryHeapDesc.NodeMask = 1;
            queryHeapDesc.Type     = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
            result = m_config.pDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&measurementGroup.gpuRes.pTimestampQueryHeap));

            if (result == S_OK)
            {
                measurementGroup.gpuRes.pTimestampQueryHeap->SetName(L"Timestamp QueryHeap");
            }
        }
    }

    if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_PIPE_STATS)
    {
        result = CreateQueryBuffer(&measurementGroup.gpuRes.pPipeStatsBuffer,
                                   m_config.measurementsPerGroup * sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS));

        if (result == S_OK)
        {
            measurementGroup.gpuRes.pPipeStatsBuffer->SetName(L"PipeStats Buffer");

            D3D12_QUERY_HEAP_DESC queryHeapDesc;
            queryHeapDesc.Count    = m_config.measurementsPerGroup;
            queryHeapDesc.NodeMask = 1;
            queryHeapDesc.Type     = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
            result = m_config.pDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&measurementGroup.gpuRes.pPipeStatsQueryHeap));

            if (result == S_OK)
            {
                measurementGroup.gpuRes.pPipeStatsQueryHeap->SetName(L"PipeStats QueryHeap");
            }
        }
    }

    if (result == S_OK)
    {
        m_cmdListData.measurementGroups.push_back(measurementGroup);
        m_cmdListData.pActiveMeasurementGroup = &m_cmdListData.measurementGroups.back();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Release collection of stale GPU resources.
/// \param gpuRes The set of GPU resources to release.
/// \returns The result code after attempting to release the incoming resources.
//-----------------------------------------------------------------------------
HRESULT DX12CmdListProfiler::ReleaseStaleResourceGroup(ProfilerGpuResources& gpuRes)
{
    UINT successfulFrees = 0;
    UINT totalFrees = 0;

    if (gpuRes.pTimestampQueryHeap != nullptr)
    {
        totalFrees++;

        if (gpuRes.pTimestampQueryHeap->Release() == S_OK)
        {
            gpuRes.pTimestampQueryHeap = nullptr;
            successfulFrees++;
        }
    }

    if (gpuRes.pTimestampBuffer != nullptr)
    {
        totalFrees++;

        if (gpuRes.pTimestampBuffer->Release() == S_OK)
        {
            gpuRes.pTimestampBuffer = nullptr;
            successfulFrees++;
        }
    }

    if (gpuRes.pPipeStatsQueryHeap != nullptr)
    {
        totalFrees++;

        if (gpuRes.pPipeStatsQueryHeap->Release() == S_OK)
        {
            gpuRes.pPipeStatsQueryHeap = nullptr;
            successfulFrees++;
        }
    }

    if (gpuRes.pPipeStatsBuffer != nullptr)
    {
        totalFrees++;

        if (gpuRes.pPipeStatsBuffer->Release() == S_OK)
        {
            gpuRes.pPipeStatsBuffer = nullptr;
            successfulFrees++;
        }
    }

    m_deletionQueue.pop();

    return (totalFrees == successfulFrees) ? S_OK : E_FAIL;
}

//-----------------------------------------------------------------------------
/// Destroy DX12 objects and memory created by the profiler.
/// \returns The result code returned after resetting the profiler state.
//-----------------------------------------------------------------------------
HRESULT DX12CmdListProfiler::ResetProfilerState()
{
    HRESULT result = S_OK;

    // Store GPU references to objects for future deletion
    for (UINT i = 0; i < m_cmdListData.measurementGroups.size(); i++)
    {
        m_deletionQueue.push(m_cmdListData.measurementGroups[i].gpuRes);
    }

    m_cmdListData.measurementGroups.clear();

    ClearCmdListData();

    // Release GPU objects at the end of deletion queue
    if (m_deletionQueue.size() > m_config.maxStaleResourceGroups)
    {
        result = ReleaseStaleResourceGroup(m_deletionQueue.front());
    }

    return result;
}
