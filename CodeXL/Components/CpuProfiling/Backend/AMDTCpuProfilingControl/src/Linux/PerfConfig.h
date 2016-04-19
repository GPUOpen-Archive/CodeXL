//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfConfig.h
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingControl/src/Linux/PerfConfig.h#3 $
// Last checkin:   $DateTime: 2016/04/14 02:12:20 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569057 $
//=====================================================================

#ifndef _PERFCONFIG_H_
#define _PERFCONFIG_H_

// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>
#include <wchar.h>

// C++ STL headers
#include <list>
#include <string>

// project headers
#include "PerfEventInternal.h"
#include <AMDTCpuProfilingRawData/inc/Linux/CaPerfHeader.h>

// class PerfConfig
//
// Profile Configuration
//   - contains profile event groups list;
//   - each event group will have a list of PMU events to be profiled
//   - Can be shared by any number of PerfPmuSession.
//
//   - !!! User Visible !!!
//
class PerfConfig
{
public:
    PerfConfig();

    // Destructor
    virtual ~PerfConfig();

    // copy ctor
    PerfConfig(const PerfConfig& cfg);

    // assignment operator
    PerfConfig& operator= (const PerfConfig& cfg);

    // CPU affinity for profiling
    void setCPUAffinity(uint64_t cpuMask)
    {
        m_cpuMask = cpuMask;

        // if cpuMask is zero, ie allCpus will set to zero;
        m_allCpus = (0 == m_cpuMask);
    }

    uint64_t getCPUAffinity() const { return m_cpuMask; }

    // adds a counter event
    // if groupLeader:
    //   'true' - new group is created with the given PerfEvent;
    //            and returns the groupId
    //   'false' - PerfEvent is added the current group;
    //
    void addCounterEvent(const PerfEvent& event, bool groupLeader = true);

    // The first event will be the group leader
    void addCounterEvent(const PerfEventList& eventList) { m_ctrEventGroupList.push_back(eventList); }

    // adds a sampling event
    void addSamplingEvent(const PerfEvent& event, uint64_t samplingValue, bool freq = false);

    // This is, in PERF, attr.sample_type - to request information in the
    // sample records. This is same for all the sampling events
    HRESULT setSampleAttribute(uint64_t sampleAttr)
    {
        if (sampleAttr < PERF_SAMPLE_IP || sampleAttr > PERF_SAMPLE_MAX)
        {
            return E_INVALIDARG;
        }

        m_sampleAttr = sampleAttr;
        return S_OK;
    }

    uint64_t getSampleAttribute() const { return m_sampleAttr; }

    HRESULT setReadFormat(uint64_t readformat)
    {
        if (readformat < PERF_FORMAT_TOTAL_TIME_ENABLED || readformat > PERF_FORMAT_MAX)
        {
            return E_INVALIDARG;
        }

        m_readFormat = readformat;
        return S_OK;
    }
    uint64_t getReadFormat() const { return m_readFormat; }

    // isByEvent is false, then the value will be bytes
    // isByEvent is true, then the value will be the number of events
    // happen before an overflow signal happens.
    void setWakeupWaterMark(uint64_t watermark, bool byEvent = true)
    {
        m_wakeupByEvents = byEvent;
        m_waterMark = watermark;
    }

    void setOutputFile(const std::string& file, bool overwrite = false)
    {
        m_outputFile = file;
        m_overwriteOutFile = overwrite;
    }
    const std::string& getOutputFile() const { return m_outputFile; }
    const std::string& getErrStr() const { return m_errStr; }
    bool isOverwrite() const { return m_overwriteOutFile; }

    void print();
    void clear();

#ifdef ENABLE_FAKETIMER
    void setTimerHackInfo(int timerHackIndex, uint64_t nSPeriod);
#endif

    enum ConfigState
    {
        PERF_CONFIG_STATE_NOT_INITIALIZED = -1,
        PERF_CONFIG_STATE_INITIALIZED     = 0,
        PERF_CONFIG_STATE_READY           = 1, // When an event gets added
    };

protected:
    ConfigState m_state;  // Profile Config specific flags;
    uint64_t    m_flags;  // TODO: UNUSED now ?

    // Counter events
    uint32_t              m_nbrCtrEvents; // Total counting events
    PerfEventList           m_ctrEventList;

    // IN PERF, Counters can be grouped together - a counter group scheduled
    // onto the CPU as a unit,
    // A counter group has one counter which is a group "leader". The leader
    // will be created first and the rest of the group counters will be
    // created subsequently.
    // TODO: support for event groups
    uint32_t              m_nbrCtrGroups;
    PerfEventGroupList      m_ctrEventGroupList;
    PerfEventList*          m_pCurrentCtrEvtGroup;

    // Sampling events
    uint32_t              m_nbrSamplingEvents;
    PerfEventList           m_samplingEventList;

    // Sample Attributes - specifies the information that needs to be
    // gathered while sampling
    uint64_t              m_sampleAttr;

    // format of the data returned by read() on a perf evebnt fd;
    uint64_t              m_readFormat;

    // how many events or bytes happen before an overflow signal happens..
    // if m_wakeupByEvents is
    //    "true" then the m_waterMark is number of events;
    //    "false" then the m_waterMark is number of bytes;
    bool                  m_wakeupByEvents;  // notify counter overflow events
    uint32_t              m_waterMark;

    // TODO:
    // Multiplexing:  Need to have more than one Event Group
    // Each Event Group can use all the event counters available in PMU
    bool                  m_isMultiplexing;
    uint32_t              m_muxPeriod;

    // cpuAffinity
    // Enable profiling on the specified CPUs
    // Shouldn't this be part of PerfPmuTarget ?
    bool                  m_allCpus; // No specific CPU affinity
    uint64_t              m_cpuMask; // MAX 64 CPUs ?

    bool          m_overwriteOutFile;
    std::string           m_outputFile;
    std::string           m_errStr;

#ifdef ENABLE_FAKETIMER
    //work around for a bug on kernels < 3.0
    int                 m_timerHackIndex;
    caperf_section_fake_timer_t m_timerHackInfo;
#endif

private:
    void copyCfg(const PerfConfig& cfg);
};

#endif // _PERFCONFIG_H_
