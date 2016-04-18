//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfProfiler.h
///
//==================================================================================

#ifndef _CAPERFPROFILER_H_
#define _CAPERFPROFILER_H_

// Standard Headers
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <wchar.h>

// PERF header file
#ifdef __cplusplus
extern "C" {
#endif
#include <poll.h>
#include <linux/perf_event.h>
#include <pthread.h>
#ifdef __cplusplus
} // extern "C"
#endif

// Project HEaders
#include "CaPerfConfig.h"
#include "PerfProfiler.h"


// class CaPerf
//
//  class for PERF profiler
//
class CaPerfProfiler : public PerfProfiler
{
public:
    CaPerfProfiler() : PerfProfiler(), m_threadId(0), m_pPerfCfg(NULL) {}

    ~CaPerfProfiler()
    {
        // If the sample reader thread is still running, stop it.
        stopSampleReaderThread();

        if (m_pPerfCfg)
        {
            delete m_pPerfCfg;
        }
    }

    // initialize the internal data structures..
    // check if the relevant profiling subsystem is available
    HRESULT initialize(PerfConfig*  config, PerfPmuTarget* tgt = NULL);
    HRESULT setProfileConfig(PerfConfig*  config);
    HRESULT setPMUTarget(PerfPmuTarget* tgt);

    // Profile Control
    HRESULT startProfile(bool enable);
    HRESULT stopProfile();
    HRESULT enableProfile();
    HRESULT disableProfile();

    // Read the profile Data - counter values and sampled profile data
    HRESULT readCounters(PerfEventCountDataList** countData = NULL);
    // int getCounterValues();
    // int printCounterValues();

    int readSampleBuffers();

    // Clear the internal data structures
    void clear();

private:
    //This creates the thread to read PERF samples
    HRESULT createSampleReaderThread();
    HRESULT stopSampleReaderThread();

    pthread_t         m_threadId;

    // CaPerfConfig - PERF specific Profile Configuration
    // Contains:-
    //   - list of counting events
    //   - list of sampling events
    //   - target to be profiled - pids/cpus
    CaPerfConfig*        m_pPerfCfg;
};

#endif // _CAPERFPROFILER_H_
