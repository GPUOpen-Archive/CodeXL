//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Class for managing the kernel occupancy objects
//==============================================================================

#ifndef _CL_OCCUPANCY_INFO_MANAGER_H_
#define _CL_OCCUPANCY_INFO_MANAGER_H_

#include <map>
#include <list>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "TSingleton.h"
#include "../Common/TraceInfoManager.h"
#include "../Common/GlobalSettings.h"
#include "../Common/OSUtils.h"
#include "../CLCommon/CLCUInfoBase.h"
#include "../Common/ProfilerTimer.h"

/// Handle the response on the end of the timer
/// \param timerType type of the ending timer for which response have to be executed
void CLOccupancyAgentTimerEndResponse(ProfilerTimerType timerType);


class OccupancyInfoEntry : public ITraceEntry
{
public:
    /// Constructor
    OccupancyInfoEntry() :
        ITraceEntry(),
        m_pCLCUInfo(NULL) {}

    virtual ~OccupancyInfoEntry();

    /// To string
    /// \return string version
    std::string ToString();

    std::string m_strKernelName;     ///< name of kernel for which occupancy is being calculated
    std::string m_strDeviceName;     ///< name of device on which occupancy is being calculated

    size_t m_nWorkGroupItemCount;    ///< size of work-group
    size_t m_nWorkGroupItemCountMax; ///< maximum size of work-group
    size_t m_nGlobalItemCount;       ///< global work size
    size_t m_nGlobalItemCountMax;    ///< maximum global work size
    size_t m_nNumberOfComputeUnits;  ///< number of compute units
    CLCUInfoBase* m_pCLCUInfo;       ///< CLCU info object
    static char m_cListSeparator;    ///< List separator
};

class OccupancyInfoManager :
    public TraceInfoManager, public TSingleton<OccupancyInfoManager>
{
    friend class TSingleton<OccupancyInfoManager>;

public:
    /// Destructor
    ~OccupancyInfoManager();

    /// Set output file
    /// \param [in] strFileName output file name
    void SetOutputFile(const std::string& strFileName);

    /// Save occupancy data to tmp file (in timeout mode)
    /// \param [in] bForceFlush Force to write all data out no matter it's ready or not - used in Detach() only
    void FlushTraceData(bool bForceFlush = false);

    /// Write out the occupancy data (in non-timeout mode)
    /// \param sout [in]   output stream
    void WriteOccupancyData(std::ostream& sout);

    ///Save to occupancy file
    void SaveToOccupancyFile();

    /// Indicates whether or not profiling is currently enabled
    /// \return true if profiling is enabled, false otherwise
    bool IsProfilingEnabled() const { return m_bIsProfilingEnabled; }

    /// Enable to disable profiling
    /// \param doEnable, flag indicating whether to enable (true) or disable (false) profiling
    void EnableProfiling(bool doEnable) { m_bIsProfilingEnabled = doEnable; }

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
    /// \param timerType Type of the timer
    void startTimer(ProfilerTimerType timerType);

private:

    /// Constructor
    OccupancyInfoManager();

    /// Disable copy constructor
    /// \param [in] source CLOccupancyInfoManager object
    OccupancyInfoManager(const OccupancyInfoManager& infoMgr);

    /// Disable the assignment operator
    /// \param [in] source CLOccupancyInfoManager object
    /// \return assigned CLOccupancyInfoManager object
    OccupancyInfoManager& operator=(const OccupancyInfoManager& infoMgr);

    std::string             m_strOutputFile;                     ///< output file
    bool                    m_bIsProfilingEnabled;               ///< flag indicating if profiling is currently enabled
    bool                    m_bDelayStartEnabled;                ///< flag indicating whether or not the profiler should start with delay or not
    bool                    m_bProfilerDurationEnabled;          ///< flag indiacating whether profiler should only run for certain duration
    unsigned long           m_delayInMilliseconds;               ///< milliseconds to delay for profiler to start
    unsigned long           m_durationInMilliseconds;            ///< duration in milliseconds for which Profiler should run
    ProfilerTimer*          m_delayTimer;                        ///< timer for handling delay timer for the occupancy agent
    ProfilerTimer*          m_durationTimer;                     ///< timer for handling duration timer for the occupancy agent

};
#endif // _CL_OCCUPANCY_INFO_MANAGER_H_

