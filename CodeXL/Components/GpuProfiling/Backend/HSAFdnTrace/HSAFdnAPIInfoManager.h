//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This class manages all the traced API objects
//==============================================================================

#ifndef _HSA_FDN_API_INFO_MANAGER_H_
#define _HSA_FDN_API_INFO_MANAGER_H_

#include <hsa_ext_profiler.h>

#include <unordered_map>

#include <AMDTOSWrappers/Include/osThread.h>

#include <AMDTMutex.h>

#include "../Common/APIInfoManagerBase.h"
#include "HSAAPIBase.h"
#include "HSAAqlPacketInfo.h"
#include "../Common/ProfilerTimer.h"

/// Struct to hold AsyncCopy timestamps
struct AsyncCopyInfo
{
    osThreadId   m_threadId; ///< thread id of host thread that called the API using the signal being tracked
    hsa_signal_t m_signal;   ///< the signal being tracked
    uint64_t     m_start;    ///< the start timestamp
    uint64_t     m_end;      ///< the end timestamp

    AsyncCopyInfo(const osThreadId& threadId, const hsa_signal_t& signal) :
       m_threadId(threadId),
       m_signal(signal),
       m_start(0),
       m_end(0)
    {
    }
};

/// Struct that handles replacement signals for async data transfers
///
/// We need to replace signals to allow us to collect timestamps from the signals
/// from a thread.  By replacing the signal, we can manage when the user application
/// sees completion.  This allows us to get the timestamps from the signal without
/// needing to worry about the user application destroying the signal out from under
/// us.
struct HSAAsyncCopySignalReplacer
{
    hsa_signal_t                m_originalSignal;  ///< the original signal provided by the user application
    hsa_signal_t                m_profilerSignal;  ///< the replacement signal created by us that is actually given to the runtime

    /// Constructor
    HSAAsyncCopySignalReplacer(hsa_signal_t originalSignal,
                               hsa_signal_t profilerSignal) :
        m_originalSignal(originalSignal),
        m_profilerSignal(profilerSignal)
    {
    }

    /// Default Constructor
    HSAAsyncCopySignalReplacer() {}
};

typedef std::vector<AsyncCopyInfo*> AsyncCopyInfoList; ///< typedef for the async copy info list
typedef std::vector<HSAAqlPacketBase*> PacketList; ///< typedef for the packet list

/// Handle the response on the end of the timer
/// \param timerType type of the ending timer for which response have to be executed
void HSATraceAgentTimerEndResponse(ProfilerTimerType timerType);

class HSAAPIInfoManager :
    public APIInfoManagerBase, public TSingleton<HSAAPIInfoManager>
{
    friend class TSingleton<HSAAPIInfoManager>;

public:
    /// Destructor
    virtual ~HSAAPIInfoManager();

    /// Add APIInfo to the list
    /// \param api APIInfo entry
    void AddAPIInfoEntry(APIBase* pApi);

    /// Add AqlPacket to the list
    /// \param aqlPacket AqlPacketBase entry
    void AddAqlPacketEntry(HSAAqlPacketBase* PAqlPacket);

    /// Check if the specified API should be intercepted
    /// \param type HSA function type
    /// \return true if API should be intercepted
    bool ShouldIntercept(HSA_API_Type type) const;

    /// Adds the specified queue to the queue map
    /// \param pQueue the queue to add
    void AddQueue(const hsa_queue_t* pQueue);

    /// Gets the index of the specified queue
    /// \param pQueue the queue whose index is needed
    /// \param[out] queueIndex the index of the specified queue (if found)
    /// \return true if the queueu is known, false otherwise
    bool GetQueueIndex(const hsa_queue_t* pQueue, size_t& queueIndex) const;

    /// Adds the specified signal to the list of async copy signals that need to be tracked.
    /// \param completionSignal the signal that should be tracked
    void AddAsyncCopyCompletionSignal(const hsa_signal_t& completionSignal);

    /// Add the orig/replacement signal to the map of replaced signals
    /// \param originalSignal the application-provided signal that is being replaced
    /// \param replacementSignal the replacement signal that is used instead of the original signal
    void AddReplacementAsyncCopySignal(const hsa_signal_t& originalSignal, const hsa_signal_t& replacementSignal);

    /// Gets the original signal for the specified replacement signal
    /// \param replacementSignal the replacement signal whose original signal is needed
    /// \param[out] originalSignal the original signal (if found)
    /// \return true if an original signal is found for the replacement signal, false otherwise
    bool GetOriginalAsyncCopySignal(const hsa_signal_t& replacementSignal, hsa_signal_t& originalSignal);

    /// Removes the replacement signal
    /// \param replacementSignal the signal to remove
    void RemoveAsyncCopySignal(const hsa_signal_t& replacementSignal);

    /// Enables or Disables the profiler delay
    /// \param doEnable true for enable and false for disable
    /// \param delayInMilliseconds milliseconds to delay the profiler
    void EnableProfileDelayStart(bool doEnable, unsigned long delayInMilliseconds = 0);

    /// Enables or Disables the profiler duration
    /// \param doEnable true for enable and false for disable
    /// \param durationInMilliseconds profiler duration in milliseconds
    void EnableProfileDuration(bool doEnable, unsigned long durationInMilliseconds = 0);

    /// Indicates whether profiler should run after delay or not
    /// \param delayInMilliseconds to return the amount by which profile set to be delayed
    /// \returns true if delay is enabled
    bool IsProfilerDelayEnabled(unsigned long& delayInMilliseconds);

    /// Indicates whether profiler should run only for set duration or not
    /// \param durationInMilliseconds to return the amount by which profile set to run
    /// \returns true if duration of the profiler is enabled
    bool IsProfilerDurationEnabled(unsigned long& durationInMilliseconds);

    /// Assigns the call back function
    /// \param timerType type of the timer
    /// \param timerEndHandler call back function pointer
    void SetTimerFinishHandler(ProfilerTimerType timerType, TimerEndHandler timerEndHandler);

    /// Creates the Profiler Timer
    /// \param timerType timer type of the starting timer
    /// \param timeIntervalInMilliseconds profiler duration or profiler delay in milliseconds
    void CreateTimer(ProfilerTimerType timerType, unsigned long timeIntervalInMilliseconds);

    /// Starts the timer
    /// \param timerType Type of the the timer
    void startTimer(ProfilerTimerType timerType);

    /// Return true if max number of APIs are traced.
    /// \return true if max number of APIs are traced.
    bool IsCapReached() const;

    /// Locks access to the signal replacer map
    void LockSignalMap();

    /// Unlocks access to the signal replacer map
    void UnlockSignalMap();

protected:
    /// Flush non-API timestamp data to the output stream
    /// \param pid the process id of the profiled process
    virtual void FlushNonAPITimestampData(const osProcessId& pid);

    /// Add the specified api to the list of APIs to filter
    /// \param strAPIName the name of the API to add to the filter
    void AddAPIToFilter(const std::string& strAPIName);

private:
    /// Constructor
    HSAAPIInfoManager();

    /// Write kernel timestamp data to stream
    /// \param sout the output stream
    /// \param record the kernel timestamp record to write to the stream
    /// \return true on success
    bool WriteKernelTimestampEntry(std::ostream& sout, const hsa_profiler_kernel_time_t& record);

    /// Write Async Copy timestamp data to stream
    /// \param sout the output stream
    /// \param pAsyncCopyInfo the async copy info to write to the stream
    /// \return true on success
    bool WriteAsyncCopyTimestamp(std::ostream& sout, const AsyncCopyInfo* pAsyncCopyInfo);

    /// Check if specified API is in API filter list
    /// \param type HSA function type
    /// \return true if API is in filter list
    bool IsInFilterList(HSA_API_Type type) const;

    typedef std::unordered_map<const hsa_queue_t*, size_t>   QueueIndexMap;        ///< typedef for the queue index map
    typedef std::pair<const hsa_queue_t*, size_t>            QueueIndexMapPair;    ///< typedef for the queue index pair
    typedef std::unordered_map<uint64_t, hsa_signal_t>       ReplacementSignalMap; ///< typedef for the replacement signal map

    unsigned int           m_tracedApiCount;                ///< number of APIs that have been traced, used to support max apis to trace option
    std::set<HSA_API_Type> m_filterAPIs;                    ///< HSA APIs that are not traced due to API filtering
    std::set<HSA_API_Type> m_mustInterceptAPIs;             ///< HSA APIs that must be intercepted (even when they are filtered out and not traced)
    QueueIndexMap          m_queueIndexMap;                 ///< map of a queue to that queue's index (basically creation order)
    AsyncCopyInfoList      m_asyncCopyInfoList;             ///< list of async copy information
    PacketList             m_packetList;                    ///< list of packets
    AMDTMutex              m_asyncTimeStampsMtx;            ///< mutex to guard access to m_asyncCopyInfoList
    AMDTMutex              m_packetTraceMtx;                ///< mutex to guard access to m_packetList
    bool                   m_bDelayStartEnabled;            ///< flag indicating whether or not the profiler should start with delay or not
    bool                   m_bProfilerDurationEnabled;      ///< flag indiacating whether profiler should only run for certain duration
    unsigned long          m_delayInMilliseconds;           ///< millieconds to delay for profiler to start
    unsigned long          m_durationInMilliseconds;        ///< duration in milliseconds for which Profiler should run
    ProfilerTimer*         m_delayTimer;                    ///< timer for handling delay timer for the profile agent
    ProfilerTimer*         m_durationTimer;                 ///< timer for handling duration timer for the profile agent
    ReplacementSignalMap   m_signalMap;                     ///< map of replacement signal handle to the original signal
    AMDTMutex              m_signalMapMtx;                  ///< mutex to guard access to m_signalMap
};

#endif // _HSA_FDN_API_INFO_MANAGER_H_
