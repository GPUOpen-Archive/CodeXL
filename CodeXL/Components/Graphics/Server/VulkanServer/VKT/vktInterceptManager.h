//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktInterceptManager.h
/// \brief  Header file for Vulkan API interception manager.
///         This critical class implements InterceptorBase, and has the critical
///         role of determining what to do as different API functions are
///         intercepted.
//==============================================================================

#ifndef __VKT_INTERCEPT_MANAGER_H__
#define __VKT_INTERCEPT_MANAGER_H__

#include "../../Common/misc.h"
#include "../../Common/ModernAPILayerManager.h"

#include "Util/vktUtil.h"
#include "Profiling/vktFrameProfilerLayer.h"
#include "Profiling/vktCmdBufProfiler.h"
#include "Tracing/vktTraceAnalyzerLayer.h"

/// Typedef for a map containing information of each known device
typedef std::unordered_map<VkDevice, DeviceInfo> DeviceInfoMap;

//-----------------------------------------------------------------------------
///  Our Vulkan interceptor manager
//-----------------------------------------------------------------------------
class VktInterceptManager : public InterceptorBase
{
public:
    VktInterceptManager();
    virtual ~VktInterceptManager() {}

    virtual ModernAPILayerManager* GetParentLayerManager();

    /// Enable tracing
    virtual void SetCollectTrace(bool inbCollectTrace) { mbCollectApiTrace = inbCollectTrace; }

    /// Enable GPU profiling
    virtual void SetProfilingEnabled(bool inbProfilingEnabled) { m_profilerEnabled = inbProfilingEnabled; }

    /// Determine whether we are GPU-tracing
    inline bool ShouldCollectTrace() const { return VktTraceAnalyzerLayer::Instance()->ShouldCollectTrace(); }

    /// Determine whether we are API-tracing
    inline bool ShouldCollectGPUTime() const { return VktFrameProfilerLayer::Instance()->ShouldCollectGPUTime(); }

    VktAPIEntry* PreCall(FuncId funcId, ParameterEntry* pParams, int paramCount, VktWrappedCmdBuf* pWrappedCmdBuf = nullptr);

    void PostCall(VktAPIEntry* pNewEntry, int returnValue = FUNCTION_RETURNS_VOID);

    DeviceInfo InitializeDeviceInfo(VkDevice inDevice, VkPhysicalDevice inGpuHandle);
    DeviceInfo FindDeviceInfo(VkDevice inDevice);

    std::string GetQueueDesc(VkQueue queue);

private:
    /// Control whether our profiler is enabled
    bool m_profilerEnabled;

    /// Control whether we should control API trace
    bool mbCollectApiTrace;

    /// A map containing basic device info
    DeviceInfoMap m_deviceInfoMap;

    /// A mutex for m_deviceInfoMap
    mutex m_deviceInfoMapMutex;
};

#endif // __VKT_INTERCEPT_MANAGER_H__
