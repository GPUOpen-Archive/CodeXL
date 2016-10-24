//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains code to collect timestamps from AQL packets.
///         NOTE: the code here is a direct port of the libhsa-runtime-tools
///               code that performs the same function.  In discussions with
///               the runtime team, we've decided that this code belongs in
///               the profiler rather than in the runtime.  This code can
///               probably be cleaned up a bit from its current state, but that
///               can be done as a future step, after we've validated that
///               everything is working correctly.
//==============================================================================

#ifndef _HSA_AQL_PACKET_TIME_COLLECTOR_H_
#define _HSA_AQL_PACKET_TIME_COLLECTOR_H_

#include <stack>
#include <queue>
#include <unordered_map>

#include "hsa.h"

#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osCondition.h>

#include <AMDTMutex.h>
#include <TSingleton.h>

#include "HSAAqlPacketInfo.h"

/// Struct that handles replacement signals for AQL packets
///
/// We need to replace signals to allow us to collect timestamps from the signals
/// from a thread.  By replacing the signal, we can manage when the user application
/// sees completion.  This allows us to get the timestamps from the signal without
/// needing to worry about the user application destroying the signal out from under
/// us.
struct HSAPacketSignalReplacer
{
    HSAAqlKernelDispatchPacket* m_pAqlPacket;      ///< the AQL packet
    hsa_signal_t                m_originalSignal;  ///< the original signal provided by the user application
    hsa_signal_t                m_profilerSignal;  ///< the replacement signal created by us that is actually given to the runtime
    hsa_agent_t                 m_agent;           ///< the agent on which the packet is dispatched
    const hsa_queue_t*          m_pQueue;          ///< the queue on which the packet is dispatched

    /// Constructor
    HSAPacketSignalReplacer(HSAAqlKernelDispatchPacket* pAqlPacket,
                            hsa_signal_t originalSignal,
                            hsa_signal_t profilerSignal,
                            hsa_agent_t agent,
                            const hsa_queue_t* pQueue) :
        m_pAqlPacket(pAqlPacket),
        m_originalSignal(originalSignal),
        m_profilerSignal(profilerSignal),
        m_agent(agent),
        m_pQueue(pQueue)
    {        
    }
    
    /// Default Constructor
    HSAPacketSignalReplacer() {}
};

/// Singleton class for global vars used by the SignalCollector thread and supporting code
class HSATimeCollectorGlobals: public TSingleton<HSATimeCollectorGlobals>
{
    friend class TSingleton<HSATimeCollectorGlobals>;

public:
    hsa_signal_t              m_forceSignalCollection;  ///< signal used to indicate that the collector thread should collect timestamps for all remaining signals
    bool                      m_doQuit;                 ///< flag to indicate that the signal collector should finish
    osCondition               m_dispatchesInFlight;     ///< condition used to wake up the signal collector thread

private:
    /// Constructor
    HSATimeCollectorGlobals();
};

/// Singleton class to manage hsa signals (allows recycling of no-longer used
/// signals to avoid overhead of constantly creating/destroying signals)
class HSASignalPool : public TSingleton<HSASignalPool>
{
    friend class TSingleton<HSASignalPool>;
    
public:
    /// Destructor
    ~HSASignalPool();

    /// Get a signal that can be used for a kernel dispatch. Creates one if
    /// needed or uses an existing one from the pool
    /// \param initialValue the initial value for the signal
    /// \param the newly-created or recycled signal
    /// \return true if a signal is returned
    bool AcquireSignal(hsa_signal_value_t initialValue, hsa_signal_t& signal);

    /// Marks a signal as longer being used.  Makes it available in the pool
    /// for future dispatches
    /// \param the signal that is no longer needed.  It willbe added to the
    ///        pool for future dispatches
    /// \return true if a signal is released
    bool ReleaseSignal(hsa_signal_t signal);

    /// Destroys all signals in the pool and clears the pool
    void Clear();

private:
    static const int s_MAX_POOL_SIZE = 100; ///< only keep 100 unused signals in the pool

    /// Constructor
    HSASignalPool();

    /// Remove copy constructor and assignment operator
    HSASignalPool(const HSASignalPool&) = delete;
    const HSASignalPool& operator=(const HSASignalPool&) = delete;

    std::stack<hsa_signal_t> m_signalPool;    ///< stack of created-and-no-longer-used signals
    AMDTMutex                m_signalPoolMtx; ///< mutex to protect access to m_signalPool
};

/// Singleton class that holds list of kernel dispatch packets with replacement
/// signals that are in flight
class HSASignalQueue : public TSingleton<HSASignalQueue>
{
    friend class TSingleton<HSASignalQueue>;

public:
    /// Add a signal to the queue
    /// \param signal the replacer signal to add to the back of the queue
    void AddSignalToBack(const HSAPacketSignalReplacer& signal);

    /// Get the signal from the queue
    /// \param[out] signal the replacer signal retrieved from the front of the queue
    void GetSignalFromFront(HSAPacketSignalReplacer& outSignal);

    /// Gets the size of the queue
    /// \return the size of the queue
    size_t GetSize() const;

    /// Clears the queue
    void Clear();

private:
    std::queue<HSAPacketSignalReplacer> m_signalQueue;    ///< Queue holding the signal replacers
    AMDTMutex                           m_signalQueueMtx; ///< Mutex protecting access to m_signalList
};

/// Thread to collect timestamps for kernel dispatch packets.
class HSASignalCollectorThread : public osThread
{
public:
    /// Constructor
    HSASignalCollectorThread();

protected:
    /// overridden function that is the main thread entry point
    virtual int entryPoint();

private:
    static const unsigned int m_deferLen = 10;          ///< number of deferred signals to track
    HSAPacketSignalReplacer   m_deferList[m_deferLen];  ///< deferred signal list -- these are signals that are done, but we don't collect their timestamps until later
    unsigned int              m_index;                  ///< index used to track which signals we've already collected timestamps for
};

#endif // _HSA_AQL_PACKET_TIME_COLLECTOR_H_
