//=================================================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktCmdBufProfiler.h
/// \brief  Vulkan command buffer profiler.
///         This standalone class injects queries into app command buffers
///         to determine GPU time and counter information.
//=================================================================================================

#ifndef __VKT_CMD_BUF_PROFILER_H__
#define __VKT_CMD_BUF_PROFILER_H__

#include <vulkan.h>
#include <vector>
#include <queue>
#include "../Util/vktUtil.h"

class VktWrappedCmdBuf;

/// Set to 2 if only doing bottom/bottom timestamps, and set to 3 if doing top/bottom/bottom timestamps.
const UINT ProfilerTimestampsPerMeasurement = 3;

//-----------------------------------------------------------------------------
/// Define possible profiler types.
//-----------------------------------------------------------------------------
enum ProfilerType
{
    PROFILER_TYPE_UNDEFINED,
    PROFILER_TYPE_STATIC,
    PROFILER_TYPE_DYNAMIC,
};

//-----------------------------------------------------------------------------
/// Define possible measurement types.
//-----------------------------------------------------------------------------
enum ProfilerMeasurementTypeFlags
{
    PROFILER_MEASUREMENT_TYPE_NONE       = 0x0,
    PROFILER_MEASUREMENT_TYPE_TIMESTAMPS = 0x1,
};

//-----------------------------------------------------------------------------
/// Each command buffer being executed has a state to help track correct start/stop order.
//-----------------------------------------------------------------------------
enum ProfilerState
{
    PROFILER_STATE_INIT,
    PROFILER_STATE_MEASUREMENT_BEGAN,
    PROFILER_STATE_MEASUREMENT_ENDED,
    PROFILER_STATE_CMD_BUF_CLOSED,
};

//-----------------------------------------------------------------------------
/// Define return codes specific to profiler.
//-----------------------------------------------------------------------------
enum ProfilerResultCode
{
    PROFILER_SUCCESS,
    PROFILER_FAIL,
    PROFILER_THIS_CMD_BUF_WAS_NOT_MEASURED,
    PROFILER_THIS_CMD_BUF_WAS_NOT_CLOSED,
    PROFILER_MEASUREMENT_NOT_STARTED,
    PROFILER_ERROR_MEASUREMENT_ALREADY_BEGAN,
};

//-----------------------------------------------------------------------------
/// Configure the profiler to work in a certain way.
//-----------------------------------------------------------------------------
struct VktCmdBufProfilerConfig
{
    UINT             measurementsPerGroup;   ///< The initial number of measurements in a single group of profiled calls
    UINT             measurementTypeFlags;   ///< The type of measurement to collect using the profiler
    UINT             maxStaleResourceGroups; ///< The max number of stale resources that can pile up before being released
    VkPhysicalDevice physicalDevice;         ///< The physical device for this profiler
    VkDevice         device;                 ///< The device for this profiler
    VkCommandBuffer  cmdBuf;                 ///< The command buffer this profiler is measuring
    bool             mapTimestampMem;        ///< Use vkCmdCopyQueryPoolResults or vkGetQueryPoolResults
    bool             newMemClear;            ///< A flag used to indicate if we should clear the profiler's timestamp memory before profiling
    UINT64           newMemClearValue;       ///< The value to clear the profiler memory to
    UINT64           cmdBufFillId;           ///< The command buffer's fill ID
};

//-----------------------------------------------------------------------------
/// Helper struct used to point to start and end values.
//-----------------------------------------------------------------------------
struct ProfilerInterval
{
    UINT64 preStart; ///< The preStart timestamp (in GPU cycles) of a profiled command
    UINT64 start;    ///< The start timestamp (in GPU cycles) of a profiled command
    UINT64 end;      ///< The end timestamp (in GPU cycles) of a profiled command
};

//-----------------------------------------------------------------------------
/// Helper struct used to point to start and end values that have been aligned against the CPU timeline.
//-----------------------------------------------------------------------------
struct AlignedInterval
{
    double preStart; ///< The preStart time of a CPU-timeline-aligned GPU command
    double start;    ///< The start time of a CPU-timeline-aligned GPU command
    double end;      ///< The end time of a CPU-timeline-aligned GPU command
};

//-----------------------------------------------------------------------------
/// Lets the client store information about the current measurement.
//-----------------------------------------------------------------------------
struct ProfilerMeasurementInfo
{
    ProfilerMeasurementId idInfo;         ///< A set of info used to identify a profiler result's measurements
    UINT64                measurementNum; ///< The index of "this measurement" collected by the profiler
};

//-----------------------------------------------------------------------------
/// Output structures containing results, to be read by client.
//-----------------------------------------------------------------------------
struct ProfilerTimestampResult
{
    ProfilerInterval rawClocks;                    ///< A set of intervals containing raw GPU clock cycles
    ProfilerInterval adjustedClocks;               ///< A set of intervals containing GPU clock cycles that have been offset by the overall start time
    AlignedInterval  alignedMillisecondTimestamps; ///< A set of intervals containing GPU timestamps that have been aligned against the CPU timeline
    double           execMicroSecs;                ///< The total execution (in Microseconds) time for the profiled call
};

//-----------------------------------------------------------------------------
/// Hold results for a measurement and related info.
//-----------------------------------------------------------------------------
struct ProfilerResult
{
    ProfilerMeasurementInfo measurementInfo; ///< The identifying metadata for this profiler results
    ProfilerTimestampResult timestampResult; ///< The timestamp measurements for this result
};

//-----------------------------------------------------------------------------
/// Encapsulate GPU resources in here.
//-----------------------------------------------------------------------------
struct ProfilerGpuResources
{
    VkQueryPool    timestampQueryPool; ///< Timestamp query pool
    VkBuffer       timestampBuffer;    ///< Timestamp buffer
    VkDeviceMemory timestampMem;       ///< Memory backing timestamp
};

//-----------------------------------------------------------------------------
/// Wrap what is required to timestamp commands in Vulkan.
//-----------------------------------------------------------------------------
struct ProfilerMeasurementGroup
{
    ProfilerGpuResources                  gpuRes;                ///< The set of resources required for measuring timestamps on the GPU
    std::vector<ProfilerMeasurementInfo>  measurementInfos;      ///< A vector of measurement info for the collected results
    UINT                                  groupMeasurementCount; ///< The size of the measured group
};

//-----------------------------------------------------------------------------
/// Holds information for each command buffer that is being measured.
//-----------------------------------------------------------------------------
struct ProfilerCmdBufData
{
    ProfilerState                         state;                   ///< The current state of the profiler for a single measured CommandBuffer.
    UINT                                  cmdBufMeasurementCount;  ///< The number of measurements in the CommandBuffer.
    std::vector<ProfilerMeasurementGroup> measurementGroups;       ///< A vector of all measurement groups in the CommandBuffer.
    ProfilerMeasurementGroup*             pActiveMeasurementGroup; ///< The currently-active measurement group to collect results for.
};

//-----------------------------------------------------------------------------
/// The Vulkan command buffer profiler.
//-----------------------------------------------------------------------------
class VktCmdBufProfiler
{
public:
    static VktCmdBufProfiler* Create(const VktCmdBufProfilerConfig& config);

    virtual ~VktCmdBufProfiler();

    ProfilerResultCode BeginCmdMeasurement(const ProfilerMeasurementId* pIdInfo);
    ProfilerResultCode EndCmdMeasurement();
    ProfilerResultCode GetCmdBufResults(std::vector<ProfilerResult>& results);

    void NotifyCmdBufClosure();
    void NotifyCmdBufReset();

    static const char* PrintProfilerResult(ProfilerResultCode resultCode);

    /// Return the command buffer's fill ID
    UINT64 GetFillId() { return m_config.cmdBufFillId; }

    VkResult ResetProfilerState();

protected:
    VktCmdBufProfiler();

    VkResult Init(const VktCmdBufProfilerConfig& config);

    VkResult SetupNewMeasurementGroup();
    VkResult CreateQueryBuffer(VkBuffer* pBuffer, VkDeviceMemory* pMemory, UINT size);
    VkResult CreateGpuResourceGroup(ProfilerGpuResources& gpuRes);
    VkResult ReleaseGpuResourceGroup(ProfilerGpuResources& gpuRes);
    void ClearCmdBufData();
    VkResult MemTypeFromProps(UINT typeBits, VkFlags reqsMask, UINT* pTypeIdx);

    /// Holds per-command buffer information for each begin-end measurement
    ProfilerCmdBufData m_cmdBufData;

    /// Profiler configuration
    VktCmdBufProfilerConfig m_config;

    /// Track stale resources.
    std::queue<ProfilerGpuResources> m_deletionQueue;

    /// Critical section object
    mutex m_mutex;

    /// GPU properties
    VkPhysicalDeviceProperties m_physicalDeviceProps;

    /// GPU memory heap properties
    VkPhysicalDeviceMemoryProperties m_memProps;

    /// GPU timestamp frequency
    double m_gpuTimestampFreq;

    /// Instance dispatch table
    VkLayerInstanceDispatchTable* m_pInstanceDT;

    /// Device dispatch table
    VkLayerDispatchTable* m_pDeviceDT;

    /// Track how many queries we will create per group
    UINT m_maxQueriesPerGroup;
};
#endif // __VKT_CMD_BUF_PROFILER_H__
