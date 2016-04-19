//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12FrameProfilerLayer.h
/// \brief  The DX12-specific Frame Profiler layer implementation.
//=============================================================================

#ifndef DX12FRAMEPROFILERLAYER_H
#define DX12FRAMEPROFILERLAYER_H

#include "../../Common/ModernAPIFrameProfilerLayer.h"
#include "DX12CmdListProfiler.h"
#include "../D3D12Enumerations.h"
#include <set>

class DX12APIEntry;
class Wrapped_ID3D12CommandQueue;
class Wrapped_ID3D12CommandQueueCustom;
class Wrapped_ID3D12CommandList;
class Wrapped_ID3D12GraphicsCommandList;
class Wrapped_ID3D12GraphicsCommandListCustom;

//-----------------------------------------------------------------------------
/// A pair of calibration timestamps used to align GPU events alonside the CPU timeline.
//-----------------------------------------------------------------------------
struct CalibrationTimestampPair
{
    UINT64        mBeforeExecutionCPUTimestamp; ///< A CPU timestamp.
    UINT64        mBeforeExecutionGPUTimestamp; ///< A GPU timestamp.
    GPS_TIMESTAMP cpuFrequency;                 ///< The clock frequency for the CPU that executed API calls.
    UINT64        mQueueFrequency;              ///< The clock frequency for the Queue that work was submitted through.
    bool          mQueueCanBeTimestamped;       ///< A flag used to determine if the Queue can be timestamped.
};

/// A set of command lists that have been executed through a Queue.
typedef std::set<Wrapped_ID3D12GraphicsCommandList*> WrappedCommandListSet;

/// A map of wrapped CommandQueue to a set of CommandLists that it has executed.
typedef std::unordered_map <Wrapped_ID3D12CommandQueue*, WrappedCommandListSet>  CommandQueueToCommandListMap;

/// Used to associate a SampleId with a ProfilerResult for a GPU-profiled function call.
typedef std::unordered_map<UINT64, ProfilerResult*> SampleIdToProfilerResultMap;

/// Associate a Queue type with all of the profiled calls that were submitted with the queue.
typedef std::unordered_map<Wrapped_ID3D12CommandQueue*, SampleIdToProfilerResultMap*> QueueWrapperToProfilingResultsMap;

/// Mapping of threadID to Profile results map
typedef std::unordered_map<UINT32, QueueWrapperToProfilingResultsMap> ProfilerResultsMap;

/// A map of SampleID to DX12APIEntry. Used for fast lookup of entries with profiling results.
typedef std::unordered_map<UINT64, DX12APIEntry*> SampleIdToAPIEntryMap;

/// Mapping of threadID to SampleIdToAPIEntryMap
typedef std::unordered_map<UINT32, SampleIdToAPIEntryMap> ThreadSafeSampleIdToAPIEntryMap;

//-----------------------------------------------------------------------------
/// The DX12-specific Frame Profiler layer implementation.
//-----------------------------------------------------------------------------
class DX12FrameProfilerLayer : public ModernAPIFrameProfilerLayer, public TSingleton < DX12FrameProfilerLayer >
{
    //-----------------------------------------------------------------------------
    /// TSingleton is a friend of the DX12FrameProfilerLayer.
    //-----------------------------------------------------------------------------
    friend TSingleton < DX12FrameProfilerLayer >;
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for DX12FrameProfilerLayer.
    //-----------------------------------------------------------------------------
    DX12FrameProfilerLayer();

    //-----------------------------------------------------------------------------
    /// Default destructor for DX12FrameProfilerLayer.
    //-----------------------------------------------------------------------------
    virtual ~DX12FrameProfilerLayer() {}

    //-----------------------------------------------------------------------------
    /// Retrieve a pointer to the parent LayerManager used by this tool.
    /// \returns A pointer to the parent LayerManager used by this tool.
    //-----------------------------------------------------------------------------
    virtual ModernAPILayerManager* GetParentLayerManager();

    //-----------------------------------------------------------------------------
    /// Called to indicate that a resource is being created
    /// The layer must create its resources and hook functions here
    /// \param type the type of resource that is being created
    /// \param pPtr pointer to the resource that is being created
    /// \return should return false on error; true otherwise
    //-----------------------------------------------------------------------------
    virtual bool OnCreate(CREATION_TYPE type, void* pPtr);

    //-----------------------------------------------------------------------------
    /// Called to indicate that a resource is being destroyed
    /// detaches from anything that was attached in OnCreate
    /// \param type the type of resource that is being destroyed
    /// \param pPtr pointer to the resource that is being destroyed
    /// \return should return false on error; true otherwise
    //-----------------------------------------------------------------------------
    virtual bool OnDestroy(CREATION_TYPE type, void* pPtr);

    //-----------------------------------------------------------------------------
    /// Clear out all of the ProfilerResults collected by the DX12CmdListProfiler.
    //-----------------------------------------------------------------------------
    virtual void ClearProfilingResults();

    //-----------------------------------------------------------------------------
    /// Construct a measurement info structure for each call that will be profiled.
    /// \param inFuncId The function ID for the function being profiled.
    /// \param inSampleId The SampleID associated with the profiled command.
    /// \param pCmdListWrapped The command list that executed the profiled command.
    /// \param out A ProfilerMeasurementId containing metadata for the new measurement.
    //-----------------------------------------------------------------------------
    void ConstructMeasurementInfo(FuncId inFuncId, UINT64 inSampleId, Wrapped_ID3D12GraphicsCommandListCustom* pCmdListWrapped, ProfilerMeasurementId& out);

    //-----------------------------------------------------------------------------
    /// Insert an APIEntry into the list of entries with profiler results.
    /// \param inEntry An APIEntry with a sampled GPU time.
    //-----------------------------------------------------------------------------
    void StoreProfilerResult(DX12APIEntry* inEntry);

    //-----------------------------------------------------------------------------
    /// Given a SampleId, find the DX12APIEntry that was logged while API tracing.
    /// \param inSampleId The SampleId to search with.
    /// \return The buffered DX12APIEntry with a matching SampleId, or nullptr if it doesn't exist for some reason.
    //-----------------------------------------------------------------------------
    DX12APIEntry* FindInvocationBySampleId(UINT64 inSampleId);

    //-----------------------------------------------------------------------------
    /// Associate a set of CommandLists with the CommandQueue that they'll be executed through.
    /// \param inCommandQueueWrapper A CommandQueue instance used to execute CommandLists.
    /// \param inNumCommandLists The number of CommandLists in the incoming array.
    /// \param inCommandLists An array of CommandLists to associate with the incoming CommandQueue.
    //-----------------------------------------------------------------------------
    void TrackParentCommandQueue(Wrapped_ID3D12CommandQueue* inCommandQueueWrapper, UINT inNumCommandLists, Wrapped_ID3D12GraphicsCommandList* const* inCommandLists);

    //-----------------------------------------------------------------------------
    /// A function used to check if an API function should be profiled.
    /// \param inFuncId The FuncId of the API call to check for profilabity.
    /// \returns True if the function is able to be profiled.
    //-----------------------------------------------------------------------------
    inline bool ShouldProfileFunction(FuncId inFuncId) const { return (true == mbProfiledFuncs[inFuncId]); }

    //-----------------------------------------------------------------------------
    /// Handles operations that need to occur before profiling an API call.
    /// \param inWrappedInterface The interface pointer used to invoke the API call.
    /// \param inFunctionId The FuncId corresponding to the API call being traced.
    //-----------------------------------------------------------------------------
    void PreCall(IUnknown* inWrappedInterface, FuncId inFunctionId);

    //-----------------------------------------------------------------------------
    /// Handler used after the real runtime implementation of an API call has been invoked.
    /// \param inNewAPIEntry The new APIEntry created for the API call invocation.
    /// \param inWrappedInterface The interface pointer used to invoke the API call.
    /// \param inFunctionId The FuncId corresponding to the API call being traced.
    //-----------------------------------------------------------------------------
    void PostCall(DX12APIEntry* inNewAPIEntry, IUnknown* inWrappedInterface, FuncId inFunctionId);

    //-----------------------------------------------------------------------------
    /// Retrieve the type of CommandList when given the Queue that executed it.
    /// \param inQueue A CommandQueue to retrieve the command list type for.
    /// \returns The type of CommandList utilized by the given queue.
    //-----------------------------------------------------------------------------
    D3D12_COMMAND_LIST_TYPE GetCommandListTypeFromCommandQueue(Wrapped_ID3D12CommandQueue* inQueue);

    //-----------------------------------------------------------------------------
    /// Take one result from DX12CmdListProfiler and scale the timestamp based on the incoming calibration timestamps.
    /// \param ioResult The ProfilerResult instance to align with the CPU timeline.
    /// \param pTimestamps A calibration timestamp structure used to align GPU events along the CPU timeline.
    /// \param inFrameStartTime The starting time for the frame, measured by the CPU.
    /// \returns True or false, indicating the success of the alignment operation.
    //-----------------------------------------------------------------------------
    bool AlignProfilerResultWithCPUTimeline(ProfilerResult& result, const CalibrationTimestampPair* pTimestamps, GPS_TIMESTAMP inFrameStartTime);

    //-----------------------------------------------------------------------------
    /// Collect and store calibration timestamps from the CPU and GPU to align execution results in a single timeline.
    /// \param inSubmissionQueue The Queue responsible for work submission.
    /// \param ioTimestamps The timestamps structure used to hold timestamps occurring before and after workload execution.
    //-----------------------------------------------------------------------------
    void CollectCalibrationTimestamps(Wrapped_ID3D12CommandQueueCustom* inSubmissionQueue, CalibrationTimestampPair* ioTimestamps);

    //-----------------------------------------------------------------------------
    /// Retrieve the map used to associate a CommandQueue with the related profiling results.
    /// \returns The map that associates a CommandQueue with profiling results.
    //-----------------------------------------------------------------------------
    ProfilerResultsMap& GetCmdListProfilerResultsMap() { return mEntriesWithProfilingResults; }

    //-----------------------------------------------------------------------------
    /// Get the map used to associate queue with its command lists.
    /// \returns The CommandQueueToCommandListMap used to associate a Queue with the CommandLists it utilizes.
    //-----------------------------------------------------------------------------
    CommandQueueToCommandListMap& GetQueueToCommandListMap() { return mCommandQueueTracker; }

    //-----------------------------------------------------------------------------
    /// Verify, align, and store the new profiler results.
    /// \param pQueue The Queue used to collect the results to verify.
    /// \param results The vector of profiler results to verify.
    /// \param pTimestampPair A pair of calibration timestamps used to align CPU and GPU timestamps.
    /// \param threadID The ThreadId that the results are collected with.
    /// \param frameStartTime The start time of the frame as collected on the CPU.
    //-----------------------------------------------------------------------------
    void VerifyAlignAndStoreResults(
        Wrapped_ID3D12CommandQueue*  pQueue,
        std::vector<ProfilerResult>& results,
        CalibrationTimestampPair*    pTimestampPair,
        UINT32                       threadID,
        GPS_TIMESTAMP                frameStartTime);

    //-----------------------------------------------------------------------------
    /// Set the internal flag that determines if GPU command profiling is enabled.
    /// \param inbProfilingEnabled The flag used to enable or disable profiling.
    //-----------------------------------------------------------------------------
    virtual void SetProfilingEnabled(bool inbProfilingEnabled);

private:
    //-----------------------------------------------------------------------------
    /// Initialize the set of functions that are able to be profiled.
    //-----------------------------------------------------------------------------
    void SetProfiledFuncs();

    //-----------------------------------------------------------------------------
    /// Copy a profiler result from one chunk of memory to another.
    /// \param inDestination The destination ProfilerResults memory to copy to.
    /// \param inSource The source memory to copy from.
    //-----------------------------------------------------------------------------
    void CopyProfilerResult(ProfilerResult* inDestination, const ProfilerResult* inSource);

    //-----------------------------------------------------------------------------
    /// Create a new map to associate a LinkId with profiling results.
    /// \param pWrappedQueue The Queue that was submitted in order to retrieve profiling results.
    /// \param inThreadId The thread being used to create a results map.
    /// \returns A SampleIdToProfilerResultMap instance used to store profiler results.
    //-----------------------------------------------------------------------------
    SampleIdToProfilerResultMap* FindOrCreateProfilerResultsMap(Wrapped_ID3D12CommandQueue* pWrappedQueue, UINT32 inThreadId);

    //-----------------------------------------------------------------------------
    /// Determine if a command list may be timestamped.
    /// \param pCmdList The incoming CommandList to check support for timestamping.
    /// \returns True if the CommandList can be timestamped. False if it cannot.
    //-----------------------------------------------------------------------------
    bool CommandListSupportsTimestamping(Wrapped_ID3D12CommandList* pCmdList);

    //-----------------------------------------------------------------------------
    /// Validate the contents of a profiler result.
    /// \param result The result to validate.
    /// \returns True if the profiler result looks good.
    //-----------------------------------------------------------------------------
    bool ValidateProfilerResult(const ProfilerResult& result);

    /// A map that associates a GPA SampleID with the APIEntry for the call.
    ThreadSafeSampleIdToAPIEntryMap mSampleIdToEntry;

    /// A mutex used to lock the CommandQueue->CommandLists association map.
    mutex mCommandQueueLockMutex;

    /// A map that associates CommandQueue instances with the CommandLists that they execute.
    CommandQueueToCommandListMap mCommandQueueTracker;

    /// A mutex to lock the profiling results container.
    mutex mProfilingResultsMutex;

    /// Entries with profiling results. The key is a command queue, and the value is a map of profiler results.
    ProfilerResultsMap mEntriesWithProfilingResults;

    /// An array of bools used to determine which FunctionIds are available for profiling.
    bool mbProfiledFuncs[FuncId_MAX];
};

#endif // DX12FRAMEPROFILERLAYER_H