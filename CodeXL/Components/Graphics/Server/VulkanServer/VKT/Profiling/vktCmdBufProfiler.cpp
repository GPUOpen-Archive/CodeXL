//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktCmdBufProfiler.cpp
/// \brief  Vulkan command buffer profiler.
///         This standalone class injects queries into app command buffers
///         to determine GPU time and counter information.
//==============================================================================

#include "vktCmdBufProfiler.h"

//-----------------------------------------------------------------------------
/// Static method that instantiates a VktCmdBufProfiler.
/// \param config A profiler configuration structure.
/// \returns A new VktCmdBufProfiler instance.
//-----------------------------------------------------------------------------
VktCmdBufProfiler* VktCmdBufProfiler::Create(const VktCmdBufProfilerConfig& config)
{
    VktCmdBufProfiler* pOut = new VktCmdBufProfiler();

    if (pOut != nullptr)
    {
        if (pOut->Init(config) != VK_SUCCESS)
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
VktCmdBufProfiler::VktCmdBufProfiler() :
    m_gpuTimestampFreq(0),
    m_executionId(-1)
{
}

//-----------------------------------------------------------------------------
/// Perform all profiler initialization.
/// \param config A pointer to a profiler configuration structure.
/// \returns The result code for initialization.
//-----------------------------------------------------------------------------
VkResult VktCmdBufProfiler::Init(const VktCmdBufProfilerConfig& config)   ///< [in] Pointer to profiler configuration
{
    VkResult result = VK_INCOMPLETE;

    if ((config.physicalDevice != VK_NULL_HANDLE) &&
        (config.device != VK_NULL_HANDLE))
    {
        memcpy(&m_config, &config, sizeof(m_config));

        m_pInstanceDT = instance_dispatch_table(config.physicalDevice);
        m_pDeviceDT = device_dispatch_table(config.device);

        m_pInstanceDT->GetPhysicalDeviceMemoryProperties(config.physicalDevice, &m_memProps);

        m_pInstanceDT->GetPhysicalDeviceProperties(config.physicalDevice, &m_physicalDeviceProps);

        m_gpuTimestampFreq = 1000000000.0f / m_physicalDeviceProps.limits.timestampPeriod;

        ClearCmdBufData();

        result = VK_SUCCESS;
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
VktCmdBufProfiler::~VktCmdBufProfiler()
{
    while (m_deletionQueue.empty() == false)
    {
        ReleaseStaleResourceGroup(m_deletionQueue.front());

        m_deletionQueue.pop();
    }
}

//-----------------------------------------------------------------------------
/// Begin profiling a GPU command.
/// \param pIdInfo The identifying metadata for the new measurement.
/// \returns A profiler result code indicating measurement success.
//-----------------------------------------------------------------------------
ProfilerResultCode VktCmdBufProfiler::BeginCmdMeasurement(const ProfilerMeasurementId* pIdInfo)
{
    ProfilerResultCode profilerResultCode = PROFILER_FAIL;

    if (m_cmdBufData.state != PROFILER_STATE_MEASUREMENT_BEGAN)
    {
        const UINT measurementId = m_cmdBufData.cmdBufMeasurementCount % m_config.measurementsPerGroup;

        // Create new measurement group if full
        if (measurementId == 0)
        {
            VkResult result = VK_INCOMPLETE;
            result = SetupNewMeasurementGroup();
            VKT_ASSERT(result == VK_SUCCESS);
        }

        if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
        {
            const UINT offset = measurementId * ProfilerTimestampsPerMeasurement;

            // Inject timestamp
            m_pDeviceDT->CmdWriteTimestamp(
                m_config.cmdBuf,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                m_cmdBufData.pActiveMeasurementGroup->gpuRes.timestampQueryPool,
                offset);

            // Inject timestamp
            m_pDeviceDT->CmdWriteTimestamp(
                m_config.cmdBuf,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                m_cmdBufData.pActiveMeasurementGroup->gpuRes.timestampQueryPool,
                offset+1);
        }

        m_cmdBufData.cmdBufMeasurementCount++;

        // Add a new measurement
        ProfilerMeasurementInfo clientData = {};
        clientData.measurementNum = m_cmdBufData.cmdBufMeasurementCount;

        if (pIdInfo != nullptr)
        {
            memcpy(&clientData.idInfo, pIdInfo, sizeof(ProfilerMeasurementId));
        }

        m_cmdBufData.pActiveMeasurementGroup->measurementInfos.push_back(clientData);

        m_cmdBufData.pActiveMeasurementGroup->groupMeasurementCount++;

        m_cmdBufData.state = PROFILER_STATE_MEASUREMENT_BEGAN;

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
ProfilerResultCode VktCmdBufProfiler::EndCmdMeasurement() ///< [in] Handle to cmd buf being measured
{
    ProfilerResultCode profilerResultCode = PROFILER_FAIL;

    if (m_cmdBufData.state == PROFILER_STATE_MEASUREMENT_BEGAN)
    {
        const UINT measurementId = (m_cmdBufData.cmdBufMeasurementCount - 1) % m_config.measurementsPerGroup;

        // Inject timestamp
        if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
        {
            const UINT offset = (measurementId * ProfilerTimestampsPerMeasurement) + (ProfilerTimestampsPerMeasurement - 1);

            m_pDeviceDT->CmdWriteTimestamp(
                m_config.cmdBuf,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                m_cmdBufData.pActiveMeasurementGroup->gpuRes.timestampQueryPool,
                offset);
        }

        m_cmdBufData.state = PROFILER_STATE_MEASUREMENT_ENDED;

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
void VktCmdBufProfiler::NotifyCmdBufClosure()
{
    ProfilerResultCode result = PROFILER_FAIL;

    if (m_config.mapTimestampMem == true)
    {
        if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
        {
            if (m_cmdBufData.state == PROFILER_STATE_MEASUREMENT_ENDED)
            {
                for (UINT i = 0; i < m_cmdBufData.measurementGroups.size(); i++)
                {
                    ProfilerMeasurementGroup& currGroup = m_cmdBufData.measurementGroups[i];

                    m_pDeviceDT->CmdCopyQueryPoolResults(
                        m_config.cmdBuf,
                        currGroup.gpuRes.timestampQueryPool,
                        0,
                        currGroup.groupMeasurementCount * ProfilerTimestampsPerMeasurement,
                        currGroup.gpuRes.timestampBuffer,
                        0,
                        sizeof(UINT64),
                        VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);
                }

                result = PROFILER_SUCCESS;
            }
        }
    }

    m_cmdBufData.state = PROFILER_STATE_CMD_BUF_CLOSED;
}

//-----------------------------------------------------------------------------
/// We assume this will be called immediately after a command buffer has been submitted.
/// \param results A vector containing performance information for a given function.
/// \returns A code with the result of collecting profiler results for the CommandBuffer.
//-----------------------------------------------------------------------------
ProfilerResultCode VktCmdBufProfiler::GetCmdBufResults(std::vector<ProfilerResult>& results)
{
    ScopeLock lock(&m_mutex);

    ProfilerResultCode profilerResultCode = PROFILER_THIS_CMD_BUF_WAS_NOT_CLOSED;

    VkResult result = VK_INCOMPLETE;

    if (m_cmdBufData.state == PROFILER_STATE_CMD_BUF_CLOSED)
    {
        profilerResultCode = PROFILER_THIS_CMD_BUF_WAS_NOT_MEASURED;

        bool containsZeroTimestamp = false;

        // Loop through all measurements for this command buffer
        for (UINT i = 0; i < m_cmdBufData.measurementGroups.size(); i++)
        {
            ProfilerMeasurementGroup& currGroup = m_cmdBufData.measurementGroups[i];

            ProfilerInterval* pTimestampData = nullptr;

            if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
            {
                // We use vkCmdCopyQueryPoolResults
                if (m_config.mapTimestampMem == true)
                {
                    result = m_pDeviceDT->MapMemory(
                                 m_config.device,
                                 currGroup.gpuRes.timestampMem,
                                 0,
                                 VK_WHOLE_SIZE,
                                 0,
                                 (void**)&pTimestampData);
                }

                // We use vkGetQueryPoolResults
                else
                {
                    pTimestampData = new ProfilerInterval[currGroup.groupMeasurementCount]();

                    result = m_pDeviceDT->GetQueryPoolResults(
                                 m_config.device,
                                 currGroup.gpuRes.timestampQueryPool,
                                 0,
                                 currGroup.groupMeasurementCount * ProfilerTimestampsPerMeasurement,
                                 currGroup.groupMeasurementCount * sizeof(ProfilerInterval),
                                 pTimestampData,
                                 sizeof(UINT64),
                                 VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);
                }
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

                    if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
                    {
                        UINT64* pTimerPreBegin = &pTimestampData[j].preStart;
                        UINT64* pTimerBegin = &pTimestampData[j].start;
                        UINT64* pTimerEnd = &pTimestampData[j].end;
                        UINT64 baseClock = pTimestampData[0].start;

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
                        if ((*pTimerPreBegin == 0ULL) || (*pTimerBegin == 0ULL) || (*pTimerEnd == 0ULL))
                        {
                            containsZeroTimestamp = true;
                        }
                    }

                    results.push_back(profilerResult);
                }
            }

            if (pTimestampData != nullptr)
            {
                if (m_config.mapTimestampMem == true)
                {
                    m_pDeviceDT->UnmapMemory(m_config.device, currGroup.gpuRes.timestampMem);
                }
                else
                {
                    delete [] pTimestampData;
                    pTimestampData = nullptr;
                }
            }

            if (containsZeroTimestamp == true)
            {
                profilerResultCode = PROFILER_ERROR_MEASUREMENT_CONTAINED_ZEROES;
            }
        }
    }

    // We're done profiling this command buffer, so reset and we'll start over next time.
    result = ResetProfilerState();
    VKT_ASSERT(result == S_OK);

    return profilerResultCode;
}

//-----------------------------------------------------------------------------
/// Notify the profiler that this command buffer was manually reset.
//-----------------------------------------------------------------------------
void VktCmdBufProfiler::NotifyCmdBufReset()
{
    VKT_ASSERT((m_cmdBufData.state == PROFILER_STATE_INIT) || (m_cmdBufData.state == PROFILER_STATE_CMD_BUF_CLOSED));
}

//-----------------------------------------------------------------------------
/// A utility function that will return result codes as a string.
/// \param resultCode The result code to convert to a string.
/// \returns A stringified version of the incoming result code.
//-----------------------------------------------------------------------------
const char* VktCmdBufProfiler::PrintProfilerResult(ProfilerResultCode resultCode)
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

    case PROFILER_THIS_CMD_BUF_WAS_NOT_MEASURED:
        pResult = "PROFILER_THIS_CMD_BUF_WAS_NOT_MEASURED";
        break;

    case PROFILER_MEASUREMENT_NOT_STARTED:
        pResult = "PROFILER_MEASUREMENT_NOT_STARTED";
        break;

    case PROFILER_ERROR_MEASUREMENT_ALREADY_BEGAN:
        pResult = "PROFILER_STATE_MEASUREMENT_ALREADY_BEGAN";
        break;

    case PROFILER_THIS_CMD_BUF_WAS_NOT_CLOSED:
        pResult = "PROFILER_THIS_CMD_BUF_WAS_NOT_CLOSED";
        break;

    case PROFILER_ERROR_MEASUREMENT_CONTAINED_ZEROES:
        pResult = "PROFILER_ERROR_MEASUREMENT_CONTAINED_ZEROES";
        break;
    }

    return pResult;
}

//-----------------------------------------------------------------------------
/// Create a buffer to hold query results.
/// \param pBuffer The new resource containing the query results.
/// \param pMemory The new memory containing the query results.
/// \param size The size of the new resource.
/// \returns The result code for creating a new query buffer.
//-----------------------------------------------------------------------------
VkResult VktCmdBufProfiler::CreateQueryBuffer(
    VkBuffer*       pBuffer,
    VkDeviceMemory* pMemory,
    UINT            size)
{
    VkResult result = VK_INCOMPLETE;

    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext                 = nullptr;
    bufferCreateInfo.usage                 = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferCreateInfo.size                  = size;
    bufferCreateInfo.queueFamilyIndexCount = 0;
    bufferCreateInfo.pQueueFamilyIndices   = nullptr;
    bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.flags                 = 0;
    result = m_pDeviceDT->CreateBuffer(m_config.device, &bufferCreateInfo, nullptr, pBuffer);

    if (result == VK_SUCCESS)
    {
        VkMemoryRequirements memReqs = {};
        m_pDeviceDT->GetBufferMemoryRequirements(m_config.device, *pBuffer, &memReqs);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext          = nullptr;
        allocInfo.allocationSize = memReqs.size;

        result = MemTypeFromProps(
                     memReqs.memoryTypeBits,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                     &allocInfo.memoryTypeIndex);

        if (result == VK_SUCCESS)
        {
            result = m_pDeviceDT->AllocateMemory(m_config.device, &allocInfo, nullptr, pMemory);

            if (result == VK_SUCCESS)
            {
                result = m_pDeviceDT->BindBufferMemory(m_config.device, *pBuffer, *pMemory, 0);

                if (m_config.newMemClear == true)
                {
                    if (result == VK_SUCCESS)
                    {
                        void* pMappedMem = nullptr;

                        result = m_pDeviceDT->MapMemory(
                            m_config.device,
                            *pMemory,
                            0,
                            VK_WHOLE_SIZE,
                            0,
                            &pMappedMem);

                        ProfilerInterval* pTimestampData = (ProfilerInterval*)pMappedMem;

                        if (result == VK_SUCCESS)
                        {
                            const ProfilerInterval storeVal = { m_config.newMemClearValue, m_config.newMemClearValue };
                            const UINT numSets = size / sizeof(storeVal);

                            for (UINT i = 0; i < numSets; i++)
                            {
                                pTimestampData[i] = storeVal;
                            }

                            m_pDeviceDT->UnmapMemory(m_config.device, *pMemory);
                        }
                    }
                }
            }
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Create a new query heap and memory pair for time stamping.
/// \returns The result code for creating a new measurement group for a CommandBuffer.
//-----------------------------------------------------------------------------
VkResult VktCmdBufProfiler::SetupNewMeasurementGroup()
{
    VkResult result = VK_SUCCESS;

    ProfilerMeasurementGroup measurementGroup = {};

    if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
    {
        const UINT queryCount = m_config.measurementsPerGroup * ProfilerTimestampsPerMeasurement;

        VkQueryPoolCreateInfo queryPoolCreateInfo = {};
        queryPoolCreateInfo.sType      = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        queryPoolCreateInfo.pNext      = nullptr;
        queryPoolCreateInfo.queryType  = VK_QUERY_TYPE_TIMESTAMP;
        queryPoolCreateInfo.queryCount = queryCount;
        result = m_pDeviceDT->CreateQueryPool(m_config.device, &queryPoolCreateInfo, nullptr, &measurementGroup.gpuRes.timestampQueryPool);

        m_pDeviceDT->CmdResetQueryPool(m_config.cmdBuf, measurementGroup.gpuRes.timestampQueryPool, 0, queryCount);

        if (result == VK_SUCCESS)
        {
            result = CreateQueryBuffer(
                         &measurementGroup.gpuRes.timestampBuffer,
                         &measurementGroup.gpuRes.timestampMem,
                         m_config.measurementsPerGroup * sizeof(ProfilerInterval));
        }
    }

    if (result == VK_SUCCESS)
    {
        m_cmdBufData.measurementGroups.push_back(measurementGroup);
        m_cmdBufData.pActiveMeasurementGroup = &m_cmdBufData.measurementGroups.back();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Release collection of stale GPU resources.
/// \param gpuRes The set of GPU resources to release.
/// \returns The result code after attempting to release the incoming resources.
//-----------------------------------------------------------------------------
VkResult VktCmdBufProfiler::ReleaseStaleResourceGroup(ProfilerGpuResources& gpuRes)
{
    if (gpuRes.timestampQueryPool != NULL)
    {
        m_pDeviceDT->DestroyQueryPool(m_config.device, gpuRes.timestampQueryPool, nullptr);
        gpuRes.timestampQueryPool = NULL;
    }

    if (gpuRes.timestampBuffer != NULL)
    {
        m_pDeviceDT->DestroyBuffer(m_config.device, gpuRes.timestampBuffer, nullptr);
        gpuRes.timestampBuffer = NULL;
    }

    if (gpuRes.timestampMem != NULL)
    {
        m_pDeviceDT->FreeMemory(m_config.device, gpuRes.timestampMem, nullptr);
        gpuRes.timestampMem = NULL;
    }

    return VK_SUCCESS;
}

//-----------------------------------------------------------------------------
/// Reset internal profiled CommandBuffer data.
//-----------------------------------------------------------------------------
void VktCmdBufProfiler::ClearCmdBufData()
{
    m_cmdBufData.state = PROFILER_STATE_INIT;
    m_cmdBufData.cmdBufMeasurementCount = 0;
    m_cmdBufData.pActiveMeasurementGroup = nullptr;
    m_cmdBufData.measurementGroups.clear();
}

//-----------------------------------------------------------------------------
/// Destroy Vulkan objects and memory created by the profiler.
/// \returns The result code returned after resetting the profiler state.
//-----------------------------------------------------------------------------
VkResult VktCmdBufProfiler::ResetProfilerState()
{
    VkResult result = VK_SUCCESS;

    // Store GPU references to objects for future deletion
    for (UINT i = 0; i < m_cmdBufData.measurementGroups.size(); i++)
    {
        m_deletionQueue.push(m_cmdBufData.measurementGroups[i].gpuRes);
    }

    ClearCmdBufData();

    // Release GPU objects at the end of deletion queue
    if (m_deletionQueue.size() > m_config.maxStaleResourceGroups)
    {
        result = ReleaseStaleResourceGroup(m_deletionQueue.front());
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Find a memory type from available heaps.
/// \param typeBits The requested mem type.
/// \param reqsMask Mem requirements.
/// \param pTypeIdx The output heap index.
/// \returns A Vulkan result code.
//-----------------------------------------------------------------------------
VkResult VktCmdBufProfiler::MemTypeFromProps(
    UINT    typeBits,
    VkFlags reqsMask,
    UINT*   pTypeIdx)
{
    // Search memory types to find first index with those properties
    for (UINT i = 0; i < 32; i++)
    {
        if ((typeBits & 1) == 1)
        {
            // Type is available, does it match user properties?
            if ((m_memProps.memoryTypes[i].propertyFlags & reqsMask) == reqsMask)
            {
                *pTypeIdx = i;
                return VK_SUCCESS;
            }
        }

        typeBits >>= 1;
    }

    // No memory types matched, return failure
    return VK_INCOMPLETE;
}
