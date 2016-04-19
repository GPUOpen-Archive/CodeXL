//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12CmdListProfiler.h
/// \brief  DX12 command list profiler.
///         This standalone class injects queries into app command buffers
///         to determine GPU time and counter information.
//=============================================================================

#ifndef __DX12_CMD_LIST_PROFILER_H__
#define __DX12_CMD_LIST_PROFILER_H__

#include <d3d12.h>
#include <vector>
#include <queue>
#include "../../Common/mymutex.h"

#ifdef _DEBUG
    /// Helper assert that will break on bad condition
    #define PROFILER_ASSERT(__expr__) if (!(__expr__)) __debugbreak();

    /// Helper assert that will always break
    #define PROFILER_ASSERT_ALWAYS() __debugbreak();
#else
    /// Helper assert that will break on bad condition (disabled for release)
    #define PROFILER_ASSERT(__expr__)

    /// Helper assert that will always break (disabled for release)
    #define PROFILER_ASSERT_ALWAYS()
#endif

//-----------------------------------------------------------------------------
/// Define possible measurement types.
//-----------------------------------------------------------------------------
enum ProfilerMeasurementTypeFlags
{
    PROFILER_MEASUREMENT_TYPE_NONE       = 0x0,
    PROFILER_MEASUREMENT_TYPE_TIMESTAMPS = 0x1,
    PROFILER_MEASUREMENT_TYPE_PIPE_STATS = 0x2,
};

//-----------------------------------------------------------------------------
/// Each command list being executed has a state to help track correct start/stop order.
//-----------------------------------------------------------------------------
enum ProfilerState
{
    PROFILER_STATE_INIT,
    PROFILER_STATE_MEASUREMENT_BEGAN,
    PROFILER_STATE_MEASUREMENT_ENDED,
    PROFILER_STATE_CMD_LIST_CLOSED,
};

//-----------------------------------------------------------------------------
/// Define return codes specific to profiler.
//-----------------------------------------------------------------------------
enum ProfilerResultCode
{
    PROFILER_SUCCESS,
    PROFILER_FAIL,
    PROFILER_THIS_CMD_LIST_WAS_NOT_MEASURED,
    PROFILER_THIS_CMD_LIST_WAS_NOT_CLOSED,
    PROFILER_MEASUREMENT_NOT_STARTED,
    PROFILER_ERROR_MEASUREMENT_ALREADY_BEGAN,
    PROFILER_ERROR_MEASUREMENT_CONTAINED_ZEROES,
};

//-----------------------------------------------------------------------------
/// Configure the profiler to work in a certain way.
//-----------------------------------------------------------------------------
struct DX12CmdListProfilerConfig
{
    UINT                       measurementsPerGroup;   ///< The initial number of measurements in a single group of profiled calls.
    UINT                       measurementTypeFlags;   ///< The type of measurement to collect using the CmdListProfiler.
    UINT                       maxStaleResourceGroups; ///< The max number of stale resources that can pile up before being released.
    ID3D12Device*              pDevice;                ///< The device used to submit commands, and collect profiler results.
    ID3D12GraphicsCommandList* pCmdList;               ///< The Command List with calls to be profiled.
    bool                       resolveOnClose;         ///< A flag used to indicate when we should resolve the profiler's query data.
    bool                       newMemClear;            ///< A flag used to indicate if we should clear the profiler's timestamp memory before profiling.
    UINT64                     newMemClearValue;       ///< The value to clear the profiler memory to.
};

//-----------------------------------------------------------------------------
/// Helper struct used to point to start and end values.
//-----------------------------------------------------------------------------
struct ProfilerInterval
{
    UINT64 start; ///< The start timestamp (in GPU cycles) of a profiled command.
    UINT64 end;   ///< The end timestamp (in GPU cycles) of a profiled command.
};

//-----------------------------------------------------------------------------
/// Helper struct used to point to start and end values that have been aligned against the CPU timeline.
//-----------------------------------------------------------------------------
struct AlignedInterval
{
    double start; ///< The start time of a CPU-timeline-aligned GPU command.
    double end;   ///< The end time of a CPU-timeline-aligned GPU command.
};

//-----------------------------------------------------------------------------
/// The client should fill one of these in, to uniquely identify each measurement.
//-----------------------------------------------------------------------------
struct ProfilerMeasurementId
{
    UINT64                     mSampleId;       ///< The SampleId for this measurement.
    UINT                       mFunctionId;     ///< The FunctionId for the measurement GPU command.
    UINT                       mFrameIndex;     ///< The index of the frame that the profiled call occurred in.
    ID3D12GraphicsCommandList* pWrappedCmdList; ///< The CommandList wrapper that was used to submit the GPU command.
    D3D12_COMMAND_LIST_TYPE    mCmdListType;    ///< The type of CommandList being measured.
};

//-----------------------------------------------------------------------------
/// Lets the client store information about the current measurement.
//-----------------------------------------------------------------------------
struct ProfilerMeasurementInfo
{
    ProfilerMeasurementId idInfo;         ///< A set of info used to identify a profiler result's measurements.
    UINT64                measurementNum; ///< The index of "this measurement" collected by the profiler.
};

//-----------------------------------------------------------------------------
/// Output structures containing results, to be read by client.
//-----------------------------------------------------------------------------
struct ProfilerTimestampResult
{
    ProfilerInterval rawClocks;                    ///< A set of intervals containing raw GPU clock cycles.
    ProfilerInterval adjustedClocks;               ///< A set of intervals containing GPU clock cycles that have been offset by the overall start time.
    AlignedInterval  alignedMillisecondTimestamps; ///< A set of intervals containing GPU timestamps that have been aligned against the CPU timeline.
    double           execMicroSecs;                ///< The total execution (in Microseconds) time for the profiled call.
};

//-----------------------------------------------------------------------------
/// Hold results for a measurement and related info.
//-----------------------------------------------------------------------------
struct ProfilerResult
{
    ProfilerMeasurementInfo              measurementInfo; ///< The identifying metadata for this profiler results.
    ProfilerTimestampResult              timestampResult; ///< The timestamp measurements for this result.
    D3D12_QUERY_DATA_PIPELINE_STATISTICS pipeStatsResult; ///< The pipeline statistics measurements for this result.
};

//-----------------------------------------------------------------------------
/// Encapsulate GPU resources in here.
//-----------------------------------------------------------------------------
struct ProfilerGpuResources
{
    ID3D12QueryHeap* pTimestampQueryHeap; ///< The heap used in resolving timestamp query results.
    ID3D12Resource*  pTimestampBuffer;    ///< The timestamp buffer resource.
    ID3D12QueryHeap* pPipeStatsQueryHeap; ///< The heap used in resolving pipeline statistics results.
    ID3D12Resource*  pPipeStatsBuffer;    ///< The pipeline statistics buffer resource.
};

//-----------------------------------------------------------------------------
/// Wrap what is required to timestamp commands in DX12.
//-----------------------------------------------------------------------------
struct ProfilerMeasurementGroup
{
    ProfilerGpuResources                  gpuRes;                ///< The set of resources required for measuring timestamps on the GPU.
    std::vector<ProfilerMeasurementInfo>  measurementInfos;      ///< A vector of measurement info for the collected results.
    UINT                                  groupMeasurementCount; ///< The size of the measured group.
};

//-----------------------------------------------------------------------------
/// Holds information for each command list that is being measured.
//-----------------------------------------------------------------------------
struct ProfilerCmdListData
{
    ProfilerState                         state;                   ///< The current state of the profiler for a single measured CommandList.
    UINT                                  cmdListMeasurementCount; ///< The number of measurements in the CommandList.
    std::vector<ProfilerMeasurementGroup> measurementGroups;       ///< A vector of all measurement groups in the CommandList.
    ProfilerMeasurementGroup*             pActiveMeasurementGroup; ///< The currently-active measurement group to collect results for.
};

//-----------------------------------------------------------------------------
/// The DX12 command list profiler.
//-----------------------------------------------------------------------------
class DX12CmdListProfiler
{
public:
    //-----------------------------------------------------------------------------
    /// Static method that instantiates a DX12 profiler.
    /// \param pConfig A profiler configuration structure.
    /// \returns A new DX12CmdListProfiler instance.
    //-----------------------------------------------------------------------------
    static DX12CmdListProfiler* Create(const DX12CmdListProfilerConfig* pConfig);

    //-----------------------------------------------------------------------------
    /// Destructor.
    //-----------------------------------------------------------------------------
    ~DX12CmdListProfiler();

    //-----------------------------------------------------------------------------
    /// Begin profiling a GPU command.
    /// \param pIdInfo The identifying metadata for the new measurement.
    /// \returns A profiler result code indicating measurement success.
    //-----------------------------------------------------------------------------
    ProfilerResultCode BeginCmdMeasurement(const ProfilerMeasurementId* pIdInfo);

    //-----------------------------------------------------------------------------
    /// End profiling a GPU command.
    /// \returns A profiler result code indicating measurement success.
    //-----------------------------------------------------------------------------
    ProfilerResultCode EndCmdMeasurement();

    //-----------------------------------------------------------------------------
    /// We assume this will be called immediately after a command list has been submitted.
    /// \param pCmdQueue The Queue to collect profiler results from.
    /// \param results A vector containing performance information for a given function.
    /// \returns A code with the result of collecting profiler results for the CommandList.
    //-----------------------------------------------------------------------------
    ProfilerResultCode GetCmdListResults(ID3D12CommandQueue* pCmdQueue, std::vector<ProfilerResult>& results);

    //-----------------------------------------------------------------------------
    /// Notify the profiler that this command list was closed.
    //-----------------------------------------------------------------------------
    void NotifyCmdListClosure();

    //-----------------------------------------------------------------------------
    /// Notify the profiler that this command list was manually reset.
    //-----------------------------------------------------------------------------
    void NotifyCmdListReset();

    //-----------------------------------------------------------------------------
    /// Use this to change whether we want timestamps/counters or both
    /// \param measurementTypeFlags The type of measurements to collect with the profiler.
    //-----------------------------------------------------------------------------
    void UpdateMeasurementType(ProfilerMeasurementTypeFlags measurementTypeFlags)
    {
        m_config.measurementTypeFlags = measurementTypeFlags;
    }

    //-----------------------------------------------------------------------------
    /// A utility function that will return result codes as a string.
    /// \param resultCode The result code to convert to a string.
    /// \returns A stringified version of the incoming result code.
    //-----------------------------------------------------------------------------
    static const char* PrintProfilerResult(ProfilerResultCode resultCode);

    //-----------------------------------------------------------------------------
    /// Let the profiler know which group of command lists this profiler belongs to.
    //-----------------------------------------------------------------------------
    void SetExecutionId(INT64 executionId) { m_executionId = executionId; }

    //-----------------------------------------------------------------------------
    /// Fetch the execution ID for this profiler.
    //-----------------------------------------------------------------------------
    INT64 GetExecutionId() { return m_executionId; }

private:
    //-----------------------------------------------------------------------------
    /// Constructor.
    //-----------------------------------------------------------------------------
    DX12CmdListProfiler();

    //-----------------------------------------------------------------------------
    /// Perform all profiler initialization.
    /// \param pConfig A pointer to a profiler configuration structure.
    /// \returns The result code for initialization.
    //-----------------------------------------------------------------------------
    HRESULT Init(const DX12CmdListProfilerConfig* pConfig);

    //-----------------------------------------------------------------------------
    /// Create a new query heap and memory pair for time stamping.
    /// \returns The result code for creating a new measurement group for a CommandList.
    //-----------------------------------------------------------------------------
    HRESULT SetupNewMeasurementGroup();

    //-----------------------------------------------------------------------------
    /// Create a buffer to hold query results.
    /// \param pResource The new resource containing the query results.
    /// \param size The size of the new resource.
    /// \returns The result code for creating a new query buffer.
    //-----------------------------------------------------------------------------
    HRESULT CreateQueryBuffer(ID3D12Resource** pResource, UINT size);

    //-----------------------------------------------------------------------------
    /// Destroy DX12 objects and memory created by the profiler.
    /// \returns The result code returned after resetting the profiler state.
    //-----------------------------------------------------------------------------
    HRESULT ResetProfilerState();

    //-----------------------------------------------------------------------------
    /// Release collection of stale GPU resources.
    /// \param gpuRes The set of GPU resources to release.
    /// \returns The result code after attempting to release the incoming resources.
    //-----------------------------------------------------------------------------
    HRESULT ReleaseStaleResourceGroup(ProfilerGpuResources& gpuRes);

    //-----------------------------------------------------------------------------
    /// Clear out the internal profiled CommandList data.
    //-----------------------------------------------------------------------------
    void ClearCmdListData();

    /// Holds per-command list information for each begin-end measurement.
    ProfilerCmdListData m_cmdListData;

    /// Profiler configuration.
    DX12CmdListProfilerConfig m_config;

    /// Track stale resources.
    std::queue<ProfilerGpuResources> m_deletionQueue;

    /// Control access to deletion queue and cmd list data.
    mutex m_mutex;

    /// Track which ExecuteCommandLists() group this profiler belongs to.
    INT64 m_executionId;

};
#endif // __DX12_CMD_LIST_PROFILER_H__
