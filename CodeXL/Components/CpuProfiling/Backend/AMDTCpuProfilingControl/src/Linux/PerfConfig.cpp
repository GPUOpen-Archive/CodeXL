//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfConfig.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingControl/src/Linux/PerfConfig.cpp#3 $
// Last checkin:   $DateTime: 2016/04/14 02:12:20 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569057 $
//=====================================================================

// Standard headers
#include <string.h>
#include <stdlib.h>

// Project headers
#include "PerfConfig.h"
#include <AMDTOSWrappers/Include/osDebugLog.h>

//
//  class PerfConfig
//

PerfConfig::PerfConfig()
{
    // Profile Config State
    m_state = PERF_CONFIG_STATE_NOT_INITIALIZED;

    // Profile Config specific flags;
    m_flags = 0;

    // Counting Events
    m_nbrCtrEvents = 0;
    m_ctrEventList.clear();

    // TODO: Counter Groups are UNUSED now !!!
    m_nbrCtrGroups = 0;
    m_ctrEventGroupList.clear();
    m_pCurrentCtrEvtGroup = NULL;

    // Sampling Events
    m_nbrSamplingEvents = 0;
    m_samplingEventList.clear();

    m_sampleAttr = 0;
    m_readFormat = 0;

    // FIXME
    // m_wakeupByEvents = true;
    // m_waterMark = 1;
    m_wakeupByEvents = false;
    m_waterMark = 0;

    // Multiplexing
    m_isMultiplexing = false;
    m_muxPeriod = 1;

    // cpuAffinity
    m_cpuMask = 0;
    m_allCpus = true;

    // profile data file (output)
    m_outputFile.clear();
    m_overwriteOutFile = false;

#ifdef ENABLE_FAKETIMER
    m_timerHackIndex = -2;
    m_timerHackInfo.numCpu        = 0;
    m_timerHackInfo.timerNanosec  = 0;
    m_timerHackInfo.fakeTimerFds  = NULL;
    m_timerHackInfo.timerFds      = NULL;
#endif
}

PerfConfig::~PerfConfig()
{
    m_ctrEventList.clear();
    m_ctrEventGroupList.clear();

    m_samplingEventList.clear();
    m_outputFile.clear();

#ifdef ENABLE_FAKETIMER
    //Clear timer hack stuff
    m_timerHackIndex              = -2;
    m_timerHackInfo.numCpu        = 0;
    m_timerHackInfo.timerNanosec  = 0;

    if (NULL != m_timerHackInfo.timerFds)
    {
        delete [] m_timerHackInfo.timerFds;
        m_timerHackInfo.timerFds = NULL;
    }

    if (NULL != m_timerHackInfo.fakeTimerFds)
    {
        delete [] m_timerHackInfo.fakeTimerFds;
        m_timerHackInfo.fakeTimerFds = NULL;
    }

#endif
}

// copy constructor
PerfConfig::PerfConfig(const PerfConfig& cfg)
{
    copyCfg(cfg);
}

PerfConfig& PerfConfig::operator=(const PerfConfig& cfg)
{
    copyCfg(cfg);

    return *this;
}

void PerfConfig::copyCfg(const PerfConfig& cfg)
{
    m_state        = cfg.m_state;
    m_flags        = cfg.m_flags;

    m_nbrCtrEvents = cfg.m_nbrCtrEvents;
    m_ctrEventList = cfg.m_ctrEventList;

    // TODO: UNUSED now !!!
    m_nbrCtrGroups        = cfg.m_nbrCtrGroups;
    m_ctrEventGroupList   = cfg.m_ctrEventGroupList;
    m_pCurrentCtrEvtGroup = NULL;

    m_nbrSamplingEvents = cfg.m_nbrSamplingEvents;
    m_samplingEventList = cfg.m_samplingEventList;

    m_sampleAttr        = cfg.m_sampleAttr;
    m_readFormat        = cfg.m_readFormat;
    m_wakeupByEvents    = cfg.m_wakeupByEvents;
    m_waterMark         = cfg.m_waterMark;

    m_isMultiplexing    = cfg.m_isMultiplexing;
    m_muxPeriod         = cfg.m_muxPeriod;

    m_cpuMask           = cfg.m_cpuMask;
    m_allCpus           = cfg.m_allCpus;

    m_outputFile        = cfg.m_outputFile;
    m_overwriteOutFile  = cfg.m_overwriteOutFile;

#ifdef ENABLE_FAKETIMER
    m_timerHackIndex = cfg.m_timerHackIndex;
    m_timerHackInfo = cfg.m_timerHackInfo;

    if (NULL != cfg.m_timerHackInfo.timerFds)
    {
        m_timerHackInfo.timerFds = new uint32_t[m_timerHackInfo.numCpu];
        memcpy(m_timerHackInfo.timerFds, cfg.m_timerHackInfo.timerFds,
               m_timerHackInfo.numCpu * sizeof(uint32_t));
    }

    if (NULL != cfg.m_timerHackInfo.fakeTimerFds)
    {
        m_timerHackInfo.fakeTimerFds = new uint32_t[m_timerHackInfo.numCpu];
        memcpy(m_timerHackInfo.fakeTimerFds, cfg.m_timerHackInfo.fakeTimerFds,
               m_timerHackInfo.numCpu * sizeof(uint32_t));
    }

#endif
}

#ifdef ENABLE_FAKETIMER
void PerfConfig::setTimerHackInfo(int timerHackIndex, uint64_t nSPeriod)
{
    m_timerHackIndex = timerHackIndex;
    m_timerHackInfo.timerNanosec = nSPeriod;
}
#endif

// PerfConfig::addCounterEvent
//
// adds a counter event
// if groupLeader:
//   'true' - new group is created with the given PerfEvent;
//            and returns the groupId
//   'false' - PerfEvent is added the current group;
//
void PerfConfig::addCounterEvent(const PerfEvent& event, bool groupLeader)
{
    // TODO: as of now, don't worry about counter groups now...
    // hence groupLeader is unused...
    PerfEventInternal  evt(event, groupLeader);

    // set the read format from here...
    evt.setReadFormat(m_readFormat);

    // Reset Sampling specific fields
    evt.setSamplingValue(0);
    evt.setSampleAttr(0);

    // FIXME: Only if nodelay set this..
    // evt.setWakeupWaterMark(1, true);

    m_ctrEventList.push_back(evt);
    m_nbrCtrEvents++;
}


// PerfConfig::addSamplingEvent
//
// adds a sampling event
//
void PerfConfig::addSamplingEvent(const PerfEvent& event, uint64_t samplingValue, bool freq)
{
    // Note: Groups are on;y for CoUnTiNg EvEnTs; Don't worry about
    // groups here.  Hence groupLeader is unused...
    PerfEventInternal  evt(event, false);

    // set the sampling specific values
    //   - sampling period
    //   - sample attributes to be recorded in sample records
    //   - watermark for overflow notification
    evt.setSamplingValue(samplingValue, freq);
    evt.setSampleAttr(m_sampleAttr);

    // FIXME: Only if nodelay set this..
    // evt.setWakeupWaterMark(1, true);

    // set the read format from here...
    evt.setReadFormat(m_readFormat);

    // Add to the list
    m_samplingEventList.push_back(evt);
    m_nbrSamplingEvents++;
}


void PerfConfig::print()
{
    // iterate over the lists and print them..
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Nbr of Counting Events : %d", m_nbrCtrEvents);

    if (m_ctrEventList.size() != 0)
    {
        PerfEventList::iterator iter = m_ctrEventList.begin();

        for (; iter != m_ctrEventList.end(); iter++)
        {
            iter->print();
        }
    }

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Nbr of Sampling Evenst : %d", m_nbrSamplingEvents);

    if (m_samplingEventList.size() != 0)
    {
        PerfEventList::iterator iter = m_samplingEventList.begin();

        for (; iter != m_samplingEventList.end(); iter++)
        {
            iter->print();
        }
    }
}

void PerfConfig::clear()
{
    // clear the Profile Config State
    m_state = PERF_CONFIG_STATE_NOT_INITIALIZED;

    // clear the Profile Config specific flags;
    m_flags = 0;

    // clear the Counting Events
    m_nbrCtrEvents = 0;
    m_ctrEventList.clear();

    // Counter Groups are UNUSED now !!!
    m_nbrCtrGroups = 0;
    m_ctrEventGroupList.clear();
    m_pCurrentCtrEvtGroup = NULL;

    // clear the Sampling Events
    m_nbrSamplingEvents = 0;
    m_samplingEventList.clear();

    m_sampleAttr = 0;
    m_readFormat = 0;
    m_wakeupByEvents = false;
    m_waterMark = 0;
    m_isMultiplexing = false;
    m_muxPeriod = 1;
    m_cpuMask = 0;
    m_allCpus = true;

    // clear the output profile data file
    m_outputFile.clear();
    m_overwriteOutFile = false;

#ifdef ENABLE_FAKETIMER
    // Baskar:
    // BUG365178: CodeXL ended data-translation prematurely and missing samples.
    // The fake-timer info section in caperf.data file should only be written
    // for TBP in linux kernel < 3.0. But due to incomplete clean-up, this fake-timer
    // section was written for non-TBP profiles too and hence we see many issues like
    //    - premature data-translation
    //    - CPU unhalted cycles not handled properly in access performance measurement
    //    - Any EBP based measurement that will be performed after a TBP measurement
    //      may produce incorrect profile data during data-translation.

    //Clear timer hack stuff
    m_timerHackIndex              = -2;
    m_timerHackInfo.numCpu        = 0;
    m_timerHackInfo.timerNanosec  = 0;

    if (NULL != m_timerHackInfo.timerFds)
    {
        delete [] m_timerHackInfo.timerFds;
        m_timerHackInfo.timerFds = NULL;
    }

    if (NULL != m_timerHackInfo.fakeTimerFds)
    {
        delete [] m_timerHackInfo.fakeTimerFds;
        m_timerHackInfo.fakeTimerFds = NULL;
    }

#endif
}
