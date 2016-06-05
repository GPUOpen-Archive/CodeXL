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

    std::string m_strOutputFile;       ///< output file
    bool        m_bIsProfilingEnabled; ///< flag indicating if profiling is currently enabled
};
#endif // _CL_OCCUPANCY_INFO_MANAGER_H_

