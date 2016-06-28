//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktFrameProfilerLayer.h
/// \brief  Header file for our Vulkan frame debugger layer.
//==============================================================================

#ifndef __VKT_FRAME_PROFILER_LAYER_H__
#define __VKT_FRAME_PROFILER_LAYER_H__

#include "../../../Common/ModernAPIFrameProfilerLayer.h"
#include "../Tracing/vktTraceAnalyzerLayer.h"
#include "../Util/vktUtil.h"
#include "vktCmdBufProfiler.h"
#include <set>

class VktAPIEntry;
class VktWrappedQueue;
class VktWrappedCmdBuf;

/// A set of command buffers that have been executed through a Queue
typedef std::set<VktWrappedCmdBuf*> WrappedCommandBufferSet;

/// A map of wrapped CommandQueue to a set of CommandBuffers that it has executed
typedef std::unordered_map <VktWrappedQueue*, WrappedCommandBufferSet>  CommandQueueToCommandBufferMap;

/// Used to associate a SampleId with a ProfilerResult for a GPU-profiled function call
typedef std::unordered_map<UINT64, ProfilerResult*> SampleIdToProfilerResultMap;

/// Associate a Queue type with all of the profiled calls that were submitted with the queue
typedef std::unordered_map<VktWrappedQueue*, SampleIdToProfilerResultMap*> QueueWrapperToProfilingResultsMap;

/// Mapping of threadID to Profile results map
typedef std::unordered_map<UINT32, QueueWrapperToProfilingResultsMap> ProfilerResultsMap;

/// A map of SampleID to VktAPIEntry. Used for fast lookup of entries with profiling results
typedef std::unordered_map<UINT64, VktAPIEntry*> SampleIdToAPIEntryMap;

/// Mapping of threadID to SampleIdToAPIEntryMap
typedef std::unordered_map<UINT32, SampleIdToAPIEntryMap> ThreadSafeSampleIdToAPIEntryMap;

//-----------------------------------------------------------------------------
/// The Vulkan-specific Frame Profiler layer implementation.
//-----------------------------------------------------------------------------
class VktFrameProfilerLayer : public ModernAPIFrameProfilerLayer, public TSingleton < VktFrameProfilerLayer >
{
    /// This class is a singleton
    friend TSingleton < VktFrameProfilerLayer >;

public:
    VktFrameProfilerLayer();

    virtual ~VktFrameProfilerLayer() {}
    virtual ModernAPILayerManager* GetParentLayerManager();

    /// Called when creating this layer
    virtual bool OnCreate(CREATION_TYPE type, void* pPtr) { GT_UNREFERENCED_PARAMETER(type); GT_UNREFERENCED_PARAMETER(pPtr); return true; }

    /// Called when destroying this layer
    virtual bool OnDestroy(CREATION_TYPE type, void* pPtr) { GT_UNREFERENCED_PARAMETER(type); GT_UNREFERENCED_PARAMETER(pPtr); return true; }

    void SetProfiledFuncs();
    virtual void SetProfilingEnabled(bool inbProfilingEnabled);

    /// Lookup whether this function should be profiled
    inline bool ShouldProfileFunction(FuncId inFuncId) const { return (m_profiledFuncs[inFuncId] == VK_TRUE); }

    /// Retrieve a map with profiler results
    ProfilerResultsMap& GetCmdBufProfilerResultsMap() { return mEntriesWithProfilingResults; }

    SampleIdToProfilerResultMap* FindOrCreateProfilerResultsMap(VktWrappedQueue* pWrappedQueue, UINT32 inThreadId);

    VktAPIEntry* FindInvocationBySampleId(UINT64 inSampleId);

    bool ValidateProfilerResult(const ProfilerResult& result);
    void StoreProfilerResult(VktAPIEntry* pEntry);
    void CopyProfilerResult(ProfilerResult* pDst, const ProfilerResult* pSrc);
    void VerifyAlignAndStoreResults(VktWrappedQueue* pQueue, std::vector<ProfilerResult>& results, CalibrationTimestampPair* pTimestampPair, UINT32 threadID, GPS_TIMESTAMP frameStartTime);
    bool AlignProfilerResultWithCPUTimeline(ProfilerResult& ioResult, const CalibrationTimestampPair* pTimestamps, GPS_TIMESTAMP inFrameStartTime);
    VkResult CollectCalibrationTimestamps(VktWrappedQueue* pWrappedQueue, CalibrationTimestampPair* pTimestamps);
    virtual void ClearProfilingResults();

    void PreCall(FuncId funcId, VktWrappedCmdBuf* pWrappedCmdBuf);
    void PostCall(VktAPIEntry* pNewAPIEntry, FuncId funcId, VktWrappedCmdBuf* pWrappedCmdBuf);

    /// A map that associates a GPA SampleID with the APIEntry for the call
    ThreadSafeSampleIdToAPIEntryMap mSampleIdToEntry;

    /// A mutex used to lock the CommandQueue->CommandBuffers association map
    mutex mCommandQueueLockMutex;

    /// A map that associates CommandQueue instances with the CommandBuffers that they execute
    CommandQueueToCommandBufferMap mCommandQueueTracker;

    /// A mutex to lock the profiling results container
    mutex mProfilingResultsMutex;

    /// Entries with profiling results
    ProfilerResultsMap mEntriesWithProfilingResults;

    /// A map which defines which entry points are profiled
    bool m_profiledFuncs[FuncId_MAX];
};

#endif // __VKT_FRAME_PROFILER_LAYER_H__