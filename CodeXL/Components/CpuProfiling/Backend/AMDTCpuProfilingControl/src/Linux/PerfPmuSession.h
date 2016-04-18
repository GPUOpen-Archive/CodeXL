//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfPmuSession.h
///
//==================================================================================

#ifndef _PERFPMUSESSION_H_
#define _PERFPMUSESSION_H_

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
#include "PerfEvent.h"
#include "PerfConfig.h"
#include "PerfPmuTarget.h"
#include "PerfProfiler.h"

// Info:-
//
// There are 2 layers in this DataCollection model:-
//
// 1. OS independent Layer:-
//       - classes that are visible to user;
//       - contains more generic details (irrespective of underlying
//         OS/profiling mechanism)
//       - classes :- PerfEvent
//                    PerfConfig
//                    PerfPmuTarget
//                    PerfEventCountData (output counter values)
//                    PerfProfiler (abstract class)
//
// 2. OS/Profiler Dependent Layer:-
//       - This layer is more specific to the underlying OS/Profiling mechanism.
//       - We need to extend "PerfProfiler" and implement the interfaces
///        to program the PMU, start/stop the profile & to collect
//         profile data.
//       - Each underlying profiling mechanism can have its own class
///        Ex: for PERF,
//              CaPerfProfiler : public PerfProfiler
//       - The CAProfileConfing and PerfPmuTarget is available to this profiler
//       - They can exten "PerfConfig" & "PerfEventInternal" to
//         create their own profiler specific internal data structures..
//
// Well there should one more layer - that provides APIs to gather Hardware
//  PMU details like..
//     - processor model, cpu family
//     - Nbr of PMU counters available
//     - for the given event-name, retrieve event-id, umask, etc
//
// A data-collection tool (caprofile) is a client/user of this DataCollection
// library. The tool can define a "PerfPmuSession" class that contains ..
//  - the profiler
//  - ProfilerConfig
//  - PMU Target
//  - Profile Data File
//  - etc
//

//
// Classes
//
// PerfEvent
//   - Generic class for Event Configuration
//   - contains event-id, unitmaks, pmu-specific flags, etc;
//   - visible to client
//
// PerfEventInternal
//   - Represnets either a Counting or Sampling Event to the backend
//     "OS/Profiler" specific layer.
//   - internal class

// PerfConfig
//   - Generic class for the Profile Configuration
//   - has list of counting and sampling events represented by
//     PerfEventInternal class
//   - visible to client
//
// PerfPmuTarget
//   - describes the profiling target - pids/cpus
//   - visible to client
//
// PerfProfiler
//   - Abstract class for a profiling mechanism
//   - Each profiling mechanism can inherit this to implement their
//     own start/stop profiling methods
//   - PERF / Oprofile / Windows driver / OpenCL ect
//
// PerfEventCountData
//   - holds the counter values (in count mode)
//     event id, target info (pid/cpu-id), counter value
//   - visible to client

// PerfPmuSession
//   - PMU Profile Session / Measurement Run
//   - associate PerfProfiler, PerfConfig and PerfPmuTarget;
//   - profile the target and gather the profile data
//   - For every data colelction run, a session object should be created
//   - write into profile data file
//
//  User Visisble


// If we implement for PERF
//
// CaPerfProfiler : public PerfProfiler
//   - PERF specific profile mechanism APIs
//   - has its own internal profile configuration object - CaPerfConfig
//
// CaPerfConfig : public PerfConfig
//   - inherit PerfConfig and implement PERF specific stuff
//   - not user visible
//
// CaPerfEvent : public PerfEventInternal
//   - inherit PerfEventInternal and implement PERF specific stuff
//   - maintain fds and mmaps and other PERF specific internals
//   - not user visible
//


// class PerfPmuSession
//  PMU Profile Session / Measurement Run
//  Should be associated with a PerfConfig object
//  Each session per active measurement run (data collection run)
//  For every data collection run, a session object should be created
//
//  User Visible
//
class PerfPmuSession
{
public:
    PerfPmuSession(PerfProfiler* profiler = NULL, PerfConfig* profConfig = NULL, PerfPmuTarget* tgt = NULL);

    ~PerfPmuSession() { }

    HRESULT initialize(PerfProfiler* profiler, PerfConfig* config, PerfPmuTarget* tgt);

    HRESULT setProfiler(PerfProfiler* profiler);
    HRESULT setProfileConfig(PerfConfig* config);
    HRESULT setPMUTarget(PerfPmuTarget* tgt);

    void setOutputFile(const std::string& file, bool overwrite = false)
    {
        m_outputFile = file;

        if (m_pProfiler)
        {
            m_pProfiler->setOutputFile(m_outputFile, overwrite);
        }
    }

    // Profile Output file
    // int setOutputFile(char *file);
    const std::string& getOutputFile() const { return m_outputFile; }

    const std::string& getErrStr() const { return m_errStr; }

    HRESULT startProfile(bool enable);
    HRESULT stopProfile();
    HRESULT enableProfile();
    HRESULT disableProfile();

    HRESULT readPMUCounters(PerfEventCountDataList** countData);
    int writePMUCounterValues(); // dumps counter values into OutputFile;
    int printPMUCounterValues(); // prints counter values into stderr/stdout

    // CPU topology details
    int getCPUTopology();
    int writeCPUTopology(); // dumps CPU Topology values into OutputFile;
    int printCPUTopology(); // prints CPU Topology values into stderr/stdout

    enum m_PerfPmuSessionStates
    {
        PERF_PMU_SESSION_STATE_UN_INTIALIZED  = -1,
        //  PERF_PMU_SESSION_STATE_INTIALIZED     = 1,  // when a profiler & config is added
        PERF_PMU_SESSION_STATE_READY          = 2,  // when a tgt is associated
        PERF_PMU_SESSION_STATE_ACTIVE         = 3,
        PERF_PMU_SESSION_STATE_INACTIVE       = 4,
        PERF_PMU_SESSION_STATE_ERROR          = 5,
        PERF_PMU_SESSION_STATE_PAUSED         = 6,
    };

    HRESULT clear();

private:
    HRESULT init();

    PerfProfiler*       m_pProfiler;
    PerfConfig*  m_pProfileConfig;
    PerfPmuTarget*    m_pPmuTarget;

    uint64_t          m_flags;        // Not Reqd ?
    uint64_t          m_state;        // session state

    std::string       m_outputFile;   // QFile ?
    std::string       m_errStr;
};

#endif // _PERFPMUSESSION_H_
