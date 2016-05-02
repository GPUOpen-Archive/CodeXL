//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktCmdBufProfilerStatic.h
/// \brief  Vulkan command buffer profiler.
///         Special version of the default profiler. This one is limited to
///         a single measurement, whereas the other grows dynamically.
//==============================================================================

#ifndef __VKT_CMD_BUF_PROFILER_STATIC_H__
#define __VKT_CMD_BUF_PROFILER_STATIC_H__

#include "vktCmdBufProfiler.h"

//-----------------------------------------------------------------------------
/// The Vulkan command buffer profiler.
//-----------------------------------------------------------------------------
class VktCmdBufProfilerStatic : public VktCmdBufProfiler
{
public:
    static VktCmdBufProfilerStatic* Create(const VktCmdBufProfilerConfig& config);

    VktCmdBufProfilerStatic();

    virtual ~VktCmdBufProfilerStatic();

    virtual ProfilerResultCode BeginCmdMeasurement(const ProfilerMeasurementId* pIdInfo);

private:
    virtual VkResult ResetProfilerState() { return VK_SUCCESS; }

    bool m_createdAssets;

};
#endif // __VKT_CMD_BUF_PROFILER_STATIC_H__
