//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktCmdBufProfilerStatic.cpp
/// \brief  Vulkan command buffer profiler.
///         Special version of the default profiler. This one is limited to
///         a single measurement, whereas the other grows dynamically.
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
VktCmdBufProfilerStatic::VktCmdBufProfilerStatic() :
    VktCmdBufProfiler(),
    m_createdAssets(false)
{
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
VktCmdBufProfilerStatic::~VktCmdBufProfilerStatic()
{
    // This should only be 1-element long
    VKT_ASSERT(m_cmdBufData.measurementGroups.size() == 1);

    for (UINT i = 0; i < m_cmdBufData.measurementGroups.size(); i++)
    {
        ReleaseStaleResourceGroup(m_cmdBufData.measurementGroups[i].gpuRes);
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

    // This profiler only supports 1 measurement.
    m_config.measurementsPerGroup = 1;
    m_cmdBufData.cmdBufMeasurementCount = 1;

    if (m_cmdBufData.state != PROFILER_STATE_MEASUREMENT_BEGAN)
    {
        if (m_createdAssets == false)
        {
            VkResult result = VK_INCOMPLETE;
            result = SetupNewMeasurementGroup();
            VKT_ASSERT(result == VK_SUCCESS);

            m_createdAssets = true;
        }

        m_pDeviceDT->CmdResetQueryPool(m_config.cmdBuf, m_cmdBufData.pActiveMeasurementGroup->gpuRes.timestampQueryPool, 0, 3);

        if (m_config.measurementTypeFlags & PROFILER_MEASUREMENT_TYPE_TIMESTAMPS)
        {
            // Inject timestamp
            m_pDeviceDT->CmdWriteTimestamp(
                m_config.cmdBuf,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                m_cmdBufData.pActiveMeasurementGroup->gpuRes.timestampQueryPool,
                0);

            // Inject timestamp
            m_pDeviceDT->CmdWriteTimestamp(
                m_config.cmdBuf,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                m_cmdBufData.pActiveMeasurementGroup->gpuRes.timestampQueryPool,
                1);
        }

        // Add a new measurement
        ProfilerMeasurementInfo clientData = {};
        clientData.measurementNum = m_cmdBufData.cmdBufMeasurementCount;

        if (pIdInfo != nullptr)
        {
            memcpy(&clientData.idInfo, pIdInfo, sizeof(ProfilerMeasurementId));
        }

        // Only fill in the first slot since we'll only ever have 1 measurement with this profiler
        if (m_cmdBufData.pActiveMeasurementGroup->measurementInfos.size() == 1)
        {
            m_cmdBufData.pActiveMeasurementGroup->measurementInfos[0] = clientData;
        }
        else
        {
            m_cmdBufData.pActiveMeasurementGroup->measurementInfos.push_back(clientData);
        }

        m_cmdBufData.pActiveMeasurementGroup->groupMeasurementCount = 1;

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