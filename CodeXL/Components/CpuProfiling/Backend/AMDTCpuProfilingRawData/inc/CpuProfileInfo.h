//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileInfo.h
///
//==================================================================================

#ifndef _CPUPROFILEINFO_H_
#define _CPUPROFILEINFO_H_

#include "CpuProfileDataTranslationInfo.h"
#include "CpuProfileSample.h"
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTCpuProfilingBackendUtils/Include/CpuProfileDefinitions.h>

struct CoreTopology
{
    gtUInt16 processor;
    gtUInt16 numaNode;
};

/***********************************************************
 * Description:
 * This map represents the logical core to physical and node mapping.
 */
typedef gtMap<unsigned int, CoreTopology> CoreTopologyMap;

/****************************************************
 * class CpuProfileInfo
 *
 * Description:
 * This class contain the metadata for each profile
 * session. This is equivalent to the [ENV] section
 * of TBP/EBP file
 */
class CP_RAWDATA_API CpuProfileInfo
{
public:
    CpuProfileInfo()
    {
        m_numNodes = 0;
        m_numCpus = 0;
        m_numEvents = 0;
        m_numSamples = 0;
        m_numMisses = 0;
        m_numModules = 0;
        m_tbpVersion = TBPVER_UNKNOWN;
        m_cpuFamily = 0;
        m_cpuModel = 0;
        m_cssUnwindDepth = CP_CSS_DEFAULT_UNWIND_DEPTH;
        m_cssScope = CP_CSS_SCOPE_UNKNOWN;
        m_isCssSupportFpo = false;
        m_isCSSEnabled = true;
        m_isProfilingCLU = false;
    }

    bool addEvent(EventMaskType m, gtUInt64 c)
    {
        EventEncodeType ev = { m, c, static_cast<unsigned int>(m_eventVec.size()) };
        m_eventVec.push_back(ev);
        m_numEvents++;
        return true;
    }

    void setTimeStamp(const gtString& dateTime) { m_timeStamp = dateTime; }

public:
    unsigned int        m_numNodes;
    unsigned int        m_numCpus;
    unsigned long       m_numEvents;
    unsigned int        m_numSamples;
    unsigned int        m_numMisses;
    unsigned int        m_numModules;
    unsigned int        m_tbpVersion;
    unsigned long       m_cpuFamily;
    unsigned long       m_cpuModel;
    gtString            m_timeStamp;
    EventEncodeVec      m_eventVec;
    CpuProfileSampleMap m_totalMap;

    gtString            m_targetPath;
    gtString            m_wrkDirectory;
    gtString            m_cmdArguments;
    gtString            m_envVariables;
    gtString            m_profType;
    gtString            m_profDirectory;
    gtString            m_profStartTime;
    gtString            m_profEndTime;
    unsigned int        m_cssUnwindDepth;
    CpuProfileCssScope  m_cssScope;
    bool                m_isCssSupportFpo;
    bool                m_isCSSEnabled;
    bool                m_isProfilingCLU;
    gtString            m_osName;
    gtString            m_profScope;
};

#endif //_CPUPROFILEINFO_H_
