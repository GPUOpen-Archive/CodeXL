//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This class interfacts with GPA to retrieve PMC and write the output file
//==============================================================================

#ifndef _HSA_GPA_PROFILE_H_
#define _HSA_GPA_PROFILE_H_

#include <string>
#include <unordered_map>
#include <set>
#include <hsa.h>
#include "../Common/GPAUtils.h"
#include "../Common/GlobalSettings.h"
#include "TSingleton.h"
#include "AMDTMutex.h"
#include "../Common/KernelStats.h"

//------------------------------------------------------------------------------------
/// This class interfaces with GPA to retrieve PMC and save to file
//------------------------------------------------------------------------------------
class HSAGPAProfiler : public TSingleton<HSAGPAProfiler>
{
    friend class TSingleton<HSAGPAProfiler>;
public:
    /// Initialize GPA, load counter file
    /// \param params Parameters set by CodeXLGpuProfiler
    /// \param strErrorOut Error string
    /// \return true if succeeded
    bool Init(const Parameters& params, std::string& strErrorOut);

    // Thread safe - concurrent sessions are serialized
    /// \param device HSA Agent
    /// \param pQueue HSA Queue object
    /// \param pAqlPacket the AQL dispatch packet
    /// \param pAqlTranslationHandle an opaque handle required to collect perf counters
    /// \return true if succeeded
    bool Begin(const hsa_agent_t             device,
               const hsa_queue_t*            pQueue,
               hsa_kernel_dispatch_packet_t* pAqlPacket,
               void*                         pAqlTranslationHandle);

    /// End PMC session, generate profiling result
    /// \param pQueue HSA Queue
    /// \param pAqlTranslationHandle aql translation handle (opaque pointer)
    /// \param signal signal object
    /// \return true if succeeded
    bool End(const hsa_agent_t  device,
             const hsa_queue_t* pQueue,
             void*              pAqlTranslationHandle,
             hsa_signal_t       signal);

    /// This function waits for a session for the specified queue and and writes out results if one is found
    /// \param queueId the ID of the queue whose sessions should be checked
    /// \param timeoutSeconds the number of seconds to wait for the session before timing out
    /// \return true if the session is complete, false if there is no session for the specified queue or if the session did not complete in the allotted time
    bool WaitForCompletedSession(uint64_t queueId, uint32_t timeoutSeconds = m_DEFAULT_MAX_SECONDS_TO_WAIT_FOR_SESSIONS);

    /// This function waits for all session for all queues and and writes out results if any are found
    /// \param timeoutSeconds the number of seconds to wait for any session before timing out
    void WaitForCompletedSessions(uint32_t timeoutSeconds = m_DEFAULT_MAX_SECONDS_TO_WAIT_FOR_SESSIONS);

    /// Determines whether the max number of kernels have been profiled already
    /// \return true if the max number of kernels have already been profiled
    bool HasKernelMaxBeenReached() const { return m_uiCurKernelCount >= m_uiMaxKernelCount; }

    /// Indicates whether or not profiling is currently enabled
    /// \return true if profiling is enabled, false otherwise
    bool IsProfilingEnabled() const { return m_isProfilingEnabled; }

    /// Enable to disable profiling
    /// \param doEnable, flag indicating whether to enable (true) or disable (false) profiling
    void EnableProfiling(bool doEnable) { m_isProfilingEnabled = doEnable; }

protected:
    /// Constructor
    HSAGPAProfiler(void);

    /// Destructor
    ~HSAGPAProfiler(void);

private:

    /// Helper function used to determine if the specified agent is a GPU device
    /// \param agent the agent to check
    /// \return true if the specified agent is a GPU device, false otherwise
    static bool IsGPUDevice(hsa_agent_t agent);

    /// Callback used to get all HSA-capable GPU devices on the system
    /// \param agent the agent to check
    /// \param pData the user-defined data (the vector of GPU device IDs)
    /// \return status code indicating whether to continue iterating (HSA_STATUS_SUCCESS) or not (any other value)
    static hsa_status_t GetGPUDeviceIDs(hsa_agent_t agent, void* pData);

    // Set output file
    /// \param strOutputFile Output file
    void SetOutputFile(const std::string& strOutputFile);

    /// Init header
    void InitHeader();

    /// Struct used in the session map
    typedef struct
    {
        gpa_uint32  m_sessionID;   ///< the GPA session ID for a given session
        KernelStats m_kernelStats; ///< the Kernel Statistics for a given session
        std::string m_agentName;   ///< the name of the agent (device)
    } SessionInfo;

    /// Writes the results from the specified session to the session output file
    /// \param sessionId the session whose results should be written
    /// \param kernelStats the kernel statistics to write with the session
    /// \return true is results are written successfully, false otherwise
    bool WriteSessionResult(const SessionInfo& sessionInfo);

    /// This function checks for a completed session for the specified queue and and writes out results if one is found
    /// \param queueId the ID of the queue whose sessions should be checked
    /// \return true if the session is complete, false if there is no session for the specified queue or if the session is not complete
    bool CheckForCompletedSession(uint64_t queueId);

    /// Populates the specified Kernel statistics structure with info from the specified dispatch packet
    /// \param[in] pAqlPacket the AQL Dispatch packet to get the kernel stats from
    /// \param[in] strAgentName the name of the device the kernel was dispatched to
    /// \param[out] kernelStats the kernelStats for the specified dispatch
    /// \return true on success, false if pAqlPacket is null or if the kernel code object is null
    bool PopulateKernelStatsFromDispatchPacket(hsa_kernel_dispatch_packet_t* pAqlPacket, const std::string& strAgentName, KernelStats& kernelStats);

    /// Add Occupancy entry
    /// \param kernelStats kernel stats structure which contains most of the info we need for occupancy
    /// \param deviceName the name of the device the kernel was dispatched to
    /// \param agent the HSA agent the kernel was dispatched to
    /// \return true if the kernel occupancy info was sucessfully gathered and added, false otherwise
    bool AddOccupancyEntry(const KernelStats& kernelStats, const std::string& deviceName, hsa_agent_t agent);

    /// Default value for Max number of seconds to wait for a session to complete before timing out
    static const uint32_t m_DEFAULT_MAX_SECONDS_TO_WAIT_FOR_SESSIONS = 10;

    /// Typedef for holding a map of active session per queue
    typedef std::unordered_map<uint64_t, SessionInfo> QueueSessionMap;

    std::string             m_strOutputFile;         ///< Output file
    GPAUtils                m_gpaUtils;              ///< common GPA utility functions
    AMDTMutex               m_mtx;                   ///< mutex
    QueueSessionMap         m_activeSessionMap;      ///< map of active session per queue
    unsigned int            m_uiCurKernelCount;      ///< number of kernels that have been profiled.
    unsigned int            m_uiMaxKernelCount;      ///< max number of kernels to profile.
    unsigned int            m_uiOutputLineCount;     ///< number of items written to the output file
    bool                    m_isProfilingEnabled;    ///< flag indicating if profiling is currently enabled
    bool                    m_isProfilerInitialized; ///< flag indicating if the profiler object has been initialized already
};

#endif  //_HSA_GPA_PROFILE_H_
