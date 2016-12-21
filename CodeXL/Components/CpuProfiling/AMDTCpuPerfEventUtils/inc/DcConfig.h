//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DcConfig.h
/// \brief Data Collection configuration reader/writer class
///        (for custom profiling or every other profiling type).
///
//==================================================================================
#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "CpuPerfEventUtilsDLLBuild.h"
#include "EventEncoding.h"

#if defined (_WIN32)
// Error description: 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
// However, this warning is caused by private members that do not have DLL exports definitions.
#pragma warning( disable : 4251 )
#endif

struct DcEventConfig
{
    PERF_CTL pmc;               // PERF_CTL
    uint64_t eventCount = 0;    // Event count aka "sampling period"; this os PERF_CTR

    DcEventConfig() { pmc.perf_ctl = 0; }
};

// Do not use the following type in DLL client code
typedef std::vector<DcEventConfig> EventConfigList;

enum DcConfigType
{
    DCConfigInvalid,           // No or invalid DC configuration
    DCConfigTBP,               // Time-based profiling (TBP)
    DCConfigEBP,               // Event-based profiling (EBP)
    DCConfigIBS,               // Instruction-based sampling (IBS)
    DCConfigCLU,               // Cache line utilization (CLU)
    DCConfigMultiple
};

struct IbsConfig
{
    uint64_t fetchMaxCount = 0;          // Maximum value of periodic fetch counter
    uint64_t opMaxCount    = 0;          // Maximum value of periodic op counter
    bool fetchSampling     = false;      // Enable IBS fetch sampling
    bool opSampling        = false;      // Enable IBS op sampling
    bool opCycleCount      = true;       // Whether the op sampling is by cycle or dispatch
};

struct CluConfig
{
    uint64_t cluMaxCount = 0;        // Maximum value of periodic op counter while CLU profiling
    bool cluSampling     = false;    // Enable CLU sampling
    bool cluCycleCount   = false;    // Whether the op sampling is by cycle or dispatch while CLU profiling
};

////////////////////////////////////////////////////////////////////////////////////
// CDCConfig class declaration
////////////////////////////////////////////////////////////////////////////////////

class TiXmlElement;
class TiXmlDocument;

class CP_EVENT_API DcConfig
{
public:
    ///////////////////////////////////////////////////////////////////////////////
    // Constructors, destructors, assignment
    ///////////////////////////////////////////////////////////////////////////////
    DcConfig();
    DcConfig(const DcConfig& original);
    ~DcConfig() { m_ebpConfigList.clear(); };
    const DcConfig& operator=(const DcConfig& rhs);

    ///////////////////////////////////////////////////////////////////////////////
    // Client functions for all DC configuration types
    ///////////////////////////////////////////////////////////////////////////////
    // Return the configuration type (TBP, EBP, ...)
    DcConfigType GetConfigType() { return m_configType; };
    // Return the configuration name in configName
    void GetConfigName(std::string& configName) { configName = m_configName; };
    // Return the configuration tool type in toolTip
    void GetToolTip(std::string& toolTip) { toolTip = m_toolTip; };
    // Return the configuration description in description
    void GetDescription(std::string& description) { description = m_description; };

    // Set the configuration type
    void SetConfigType(DcConfigType configType) { m_configType = configType; };
    // Set the configuration name
    void SetConfigName(const std::string& configName) { m_configName = configName; };
    // Set the configuration tool tip
    void SetToolTip(const std::string& toolTip) { m_toolTip = toolTip; };
    // Set the configuration description
    void SetDescription(const std::string& description) { m_description = description; };

    // Read a DC configuration in XML from the specified file
    bool ReadConfigFile(const std::string& configFileName);
    // Write the current configuration in XML to the specified file
    bool WriteConfigFile(const std::string& configFileName, const std::string& pathToDtd);

    ///////////////////////////////////////////////////////////////////////////////
    // Client functions for TBP DC configurations
    ///////////////////////////////////////////////////////////////////////////////
    // Set the TBP timer interval (sample period)
    void SetTimerInterval(float interval) { m_tbpInterval = interval; };
    // Get the TBP timer interval
    float GetTimerInterval() { return m_tbpInterval; };

    ///////////////////////////////////////////////////////////////////////////////
    // Client functions for EBP DC configurations
    ///////////////////////////////////////////////////////////////////////////////
    // Set event groups and multiplex period for the configuration
    void SetEventInfo(const std::vector<DcEventConfig>& eventsConfigVector) { m_ebpConfigList = eventsConfigVector; };
    // Return the number of event groups in the configuration
    uint32_t GetNumberOfEvents() { return m_ebpConfigList.size(); };
    // Get the event groups; Copy data to caller-supplied vector event groups
    void GetEventInfo(std::vector<DcEventConfig>& eventsConfigVector) { eventsConfigVector = m_ebpConfigList; };

    ///////////////////////////////////////////////////////////////////////////////
    // Client functions for IBS DC configurations
    ///////////////////////////////////////////////////////////////////////////////
    // Set IBS configuration info using the specified struct
    void SetIBSInfo(const IbsConfig& inIBS) { m_ibsConfig = inIBS; };
    // Get IBS configuration info; Copy info to caller-supplied struct
    void GetIBSInfo(IbsConfig& outIBS) { outIBS = m_ibsConfig; };

    ///////////////////////////////////////////////////////////////////////////////
    // Client functions for CLU DC configurations
    ///////////////////////////////////////////////////////////////////////////////
    // Set CLU configuration info using the specified struct
    void SetCLUInfo(const CluConfig& inCLU) { m_cluConfig = inCLU; };
    // Get CLU configuration info; Copy info to caller-supplied struct
    void GetCLUInfo(CluConfig& outCLU) { outCLU = m_cluConfig; };

private:
    bool parseDomTree(TiXmlElement* pRoot);
    bool constructDomTree(TiXmlDocument* pConfigDoc);

private:
    DcConfigType     m_configType;        // Kind of configuration (TBP, EBP, ...)
    std::string      m_configName;        // Configuration name
    std::string      m_toolTip;           // Configuration's tool tip
    std::string      m_description;       // Description of the configuration
    float            m_tbpInterval;       // Timer interval (sample period)
    IbsConfig        m_ibsConfig;         // Holds IBS-specific configuration info
    CluConfig        m_cluConfig;         // Holds CLU-specific configuration info
    EventConfigList  m_ebpConfigList;     // Holds event configure as they come in
};
