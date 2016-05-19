//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktCmdBufProfilerStatic.h
/// \brief  Vulkan command buffer profiler.
///         Special version of the default profiler. This one is limited to
///         a fixed number of measurements, whereas the other grows dynamically.
//==============================================================================

#ifndef __VKT_CMD_BUF_PROFILER_STATIC_H__
#define __VKT_CMD_BUF_PROFILER_STATIC_H__

#include "vktCmdBufProfiler.h"

/// Only permit a pool of size 2
static const UINT StaticMeasurementCount = 2;

//-----------------------------------------------------------------------------
/// Holds information for one of the StaticMeasurementCount slots.
//-----------------------------------------------------------------------------
struct MeasurementSlot
{
    ProfilerState           state;            ///< Profiler state for this slot
    ProfilerGpuResources    gpuRes;           ///< The GPU resources used by this slot
    ProfilerMeasurementInfo measurementInfo;  ///< Measurement info for this slot
};

//-----------------------------------------------------------------------------
/// The Vulkan command buffer profiler.
//-----------------------------------------------------------------------------
class VktCmdBufProfilerStatic : public VktCmdBufProfiler
{
public:
    static VktCmdBufProfilerStatic* Create(const VktCmdBufProfilerConfig& config);

    VktCmdBufProfilerStatic();

    virtual ~VktCmdBufProfilerStatic();

    ProfilerResultCode BeginCmdMeasurement(const ProfilerMeasurementId* pIdInfo);
    ProfilerResultCode EndCmdMeasurement();
    void NotifyCmdBufClosure();
    ProfilerResultCode GetCmdBufResults(UINT64 fillId, std::vector<ProfilerResult>& results);

private:
    VkResult InternalInit();

    /// A reusable pool of measurements
    MeasurementSlot m_slots[StaticMeasurementCount];

    /// The currently active slot
    UINT m_activeSlot;

};
#endif // __VKT_CMD_BUF_PROFILER_STATIC_H__
