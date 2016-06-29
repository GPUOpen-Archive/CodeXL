//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktCmdBufProfilerStatic.cpp
/// \brief  Vulkan command buffer profiler.
///         Special version of the default profiler. This one is limited to
///         a fixed number of measurements, whereas the other grows dynamically.
//==============================================================================

#include "vktCmdBufProfilerStatic.h"

//-----------------------------------------------------------------------------
/// Static method that instantiates a VktCmdBufProfiler.
/// \param config A profiler configuration structure.
/// \returns A new VktCmdBufProfiler instance.
//-----------------------------------------------------------------------------
VktCmdBufProfilerStatic* VktCmdBufProfilerStatic::Create(const VktCmdBufProfilerConfig& config)
{
    VktCmdBufProfilerStatic* pOut = new VktCmdBufProfilerStatic();

    if (pOut != nullptr)
    {
        if ((pOut->Init(config) != VK_SUCCESS) || (pOut->InternalInit() != VK_SUCCESS))
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
VktCmdBufProfilerStatic::VktCmdBufProfilerStatic() :
    VktCmdBufProfiler(), m_activeSlot(0)
{
    memset(&m_slots, 0, sizeof(m_slots));
}

//-----------------------------------------------------------------------------
/// InternalInit
//-----------------------------------------------------------------------------
VkResult VktCmdBufProfilerStatic::InternalInit()
{
    VkResult result = VK_INCOMPLETE;

    for (UINT i = 0; i < StaticMeasurementCount; i++)
    {
        result = CreateGpuResourceGroup(m_slots[i].gpuRes);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
VktCmdBufProfilerStatic::~VktCmdBufProfilerStatic()
{
    for (UINT i = 0; i < StaticMeasurementCount; i++)
    {
        ReleaseGpuResourceGroup(m_slots[i].gpuRes);
    }
}

//-----------------------------------------------------------------------------
/// Begin profiling a GPU command.
/// \param pIdInfo The identifying metadata for the new measurement.
/// \returns A profiler result code indicating measurement success.
//-----------------------------------------------------------------------------
ProfilerResultCode VktCmdBufProfilerStatic::BeginCmdMeasurement(const ProfilerMeasurementId* pIdInfo)
{
    ProfilerResultCode profilerResultCode = PROFILER_FAIL;

    if (m_activeSlot == StaticMeasurementCount)
    {
        m_activeSlot = 0;
    }

    MeasurementSlot& currSlot = m_slots[m_activeSlot];

    if (currSlot.state != PROFILER_STATE_MEASUREMENT_BEGAN)
    {
        if (m_activeSlot == 0)
        {
            m_pDeviceDT->CmdResetQueryPool(
                m_config.cmdBuf,
                currSlot.gpuRes.timestampQueryPool,
                0,
                m_maxQueriesPerGroup);
        }

        if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
        {
            // Inject timestamp
            m_pDeviceDT->CmdWriteTimestamp(
                m_config.cmdBuf,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                currSlot.gpuRes.timestampQueryPool,
                0);

            // Inject timestamp
            m_pDeviceDT->CmdWriteTimestamp(
                m_config.cmdBuf,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                currSlot.gpuRes.timestampQueryPool,
                1);
        }

        m_activeSlot++;

        // Add a new measurement
        ProfilerMeasurementInfo clientData = ProfilerMeasurementInfo();
        clientData.measurementNum = m_activeSlot;

        if (pIdInfo != nullptr)
        {
            memcpy(&currSlot.measurementInfo.idInfo, pIdInfo, sizeof(ProfilerMeasurementId));
        }

        currSlot.state = PROFILER_STATE_MEASUREMENT_BEGAN;

        profilerResultCode = PROFILER_SUCCESS;
    }
    else
    {
        profilerResultCode = PROFILER_ERROR_MEASUREMENT_ALREADY_BEGAN;

        VKT_ASSERT_ALWAYS();
    }

    return profilerResultCode;
}

//-----------------------------------------------------------------------------
/// End profiling a GPU command.
/// \returns A profiler result code indicating measurement success.
//-----------------------------------------------------------------------------
ProfilerResultCode VktCmdBufProfilerStatic::EndCmdMeasurement()
{
    ProfilerResultCode profilerResultCode = PROFILER_FAIL;

    MeasurementSlot& currSlot = m_slots[m_activeSlot - 1];

    if (currSlot.state == PROFILER_STATE_MEASUREMENT_BEGAN)
    {
        // Inject timestamp
        if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
        {
            m_pDeviceDT->CmdWriteTimestamp(
                m_config.cmdBuf,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                currSlot.gpuRes.timestampQueryPool,
                2);
        }

        currSlot.state = PROFILER_STATE_MEASUREMENT_ENDED;

        profilerResultCode = PROFILER_SUCCESS;
    }
    else
    {
        profilerResultCode = PROFILER_MEASUREMENT_NOT_STARTED;

        VKT_ASSERT_ALWAYS();
    }

    return profilerResultCode;
}

//-----------------------------------------------------------------------------
/// Notify the profiler that this command buffer was closed.
//-----------------------------------------------------------------------------
void VktCmdBufProfilerStatic::NotifyCmdBufClosure()
{
    ProfilerResultCode result = PROFILER_FAIL;

    MeasurementSlot& currSlot = m_slots[m_activeSlot - 1];

    if (m_config.mapTimestampMem == true)
    {
        if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
        {
            if (currSlot.state == PROFILER_STATE_MEASUREMENT_ENDED)
            {
                m_pDeviceDT->CmdCopyQueryPoolResults(
                    m_config.cmdBuf,
                    currSlot.gpuRes.timestampQueryPool,
                    0,
                    ProfilerTimestampsPerMeasurement,
                    currSlot.gpuRes.timestampBuffer,
                    0,
                    sizeof(UINT64),
                    VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);

                result = PROFILER_SUCCESS;
            }
        }
    }

    if (result == PROFILER_FAIL)
    {
        Log(logERROR, "VktCmdBufProfilerStatic::NotifyCmdBufClosure(0 failed with PROFILER_FAIL\n");
    }

    currSlot.state = PROFILER_STATE_CMD_BUF_CLOSED;
}

//-----------------------------------------------------------------------------
/// We assume this will be called immediately after a command buffer has been submitted.
/// \param fillId An identifier for how many times a command buffer was filled in.
/// \param results A vector containing performance information for a given function.
/// \returns A code with the result of collecting profiler results for the CommandBuffer.
//-----------------------------------------------------------------------------
ProfilerResultCode VktCmdBufProfilerStatic::GetCmdBufResults(UINT64 fillId, std::vector<ProfilerResult>& results)
{
    ProfilerResultCode profilerResultCode = PROFILER_THIS_CMD_BUF_WAS_NOT_CLOSED;

    INT slot = -1;
    for (UINT i = 0; i < StaticMeasurementCount; i++)
    {
        if (m_slots[i].measurementInfo.idInfo.fillId == fillId)
        {
            slot = i;
            break;
        }
    }

    if (slot != -1)
    {
        MeasurementSlot& currSlot = m_slots[slot];

        if (currSlot.state == PROFILER_STATE_CMD_BUF_CLOSED)
        {
            VkResult result = VK_INCOMPLETE;

            ProfilerInterval* pTimestampData = nullptr;

            ProfilerInterval interval = ProfilerInterval();

            if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
            {
                // We use vkCmdCopyQueryPoolResults
                if (m_config.mapTimestampMem == true)
                {
                    result = m_pDeviceDT->MapMemory(
                        m_config.device,
                        currSlot.gpuRes.timestampMem,
                        0,
                        VK_WHOLE_SIZE,
                        0,
                        (void**)&pTimestampData);
                }

                // We use vkGetQueryPoolResults
                else
                {
                    result = m_pDeviceDT->GetQueryPoolResults(
                        m_config.device,
                        currSlot.gpuRes.timestampQueryPool,
                        0,
                        3,
                        sizeof(ProfilerInterval),
                        &interval,
                        sizeof(UINT64),
                        VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);

                    pTimestampData = &interval;
                }
            }

            if (result != VK_SUCCESS)
            {
                Log(logERROR, "VktCmdBufProfilerStatic::GetCmdBufResults() failed with %d\n", result);
            }

            // Report no results
            if (m_config.measurementTypeFlags == PROFILER_MEASUREMENT_TYPE_NONE)
            {
                ProfilerResult profilerResult = ProfilerResult();
                results.push_back(profilerResult);
            }

            // Fetch our results
            else
            {
                profilerResultCode = PROFILER_SUCCESS;

                ProfilerResult profilerResult = ProfilerResult();

                memcpy(&profilerResult.measurementInfo, &currSlot.measurementInfo, sizeof(ProfilerMeasurementInfo));

                if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
                {
                    UINT64* pTimerPreBegin = &pTimestampData->preStart;
                    UINT64* pTimerBegin = &pTimestampData->start;
                    UINT64* pTimerEnd = &pTimestampData->end;
                    UINT64 baseClock = pTimestampData->start;

                    // Store raw clocks
                    profilerResult.timestampResult.rawClocks.preStart = *pTimerPreBegin;
                    profilerResult.timestampResult.rawClocks.start = *pTimerBegin;
                    profilerResult.timestampResult.rawClocks.end = *pTimerEnd;

                    // Calculate adjusted clocks
                    profilerResult.timestampResult.adjustedClocks.preStart = 0;
                    profilerResult.timestampResult.adjustedClocks.start = *pTimerBegin - baseClock;
                    profilerResult.timestampResult.adjustedClocks.end = *pTimerEnd - baseClock;

                    // Calculate exec time
                    profilerResult.timestampResult.execMicroSecs = static_cast<double>(*pTimerEnd - *pTimerBegin) / m_gpuTimestampFreq;
                    profilerResult.timestampResult.execMicroSecs *= 1000000;

                    // Detected a zero timestamp. Allow this and continue, but some results are invalid.
                    VKT_ASSERT((*pTimerPreBegin != 0ULL) && (*pTimerBegin != 0ULL) && (*pTimerEnd != 0ULL));
                }

                results.push_back(profilerResult);
            }

            if (pTimestampData != nullptr)
            {
                if (m_config.mapTimestampMem == true)
                {
                    m_pDeviceDT->UnmapMemory(m_config.device, currSlot.gpuRes.timestampMem);
                }
            }
        }
    }

    return profilerResultCode;
}