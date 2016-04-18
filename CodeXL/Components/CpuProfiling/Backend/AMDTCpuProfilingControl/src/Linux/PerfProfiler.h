//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfProfiler.h
///
//==================================================================================

#ifndef _PERFPROFILER_H_
#define _PERFPROFILER_H_

// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>
#include <wchar.h>

// C++ STL headers
#include <string>
#include <list>

// project headers
#include "PerfEvent.h"
#include "PerfConfig.h"
#include "PerfPmuTarget.h"


// class PerfProfiler
//
// Generic class to provide the profiling mechanism
//  Can be used to support PERF, Oprofile
//  Windows driver, OpenCL library tracing, GPU counter profiling ??
//
//  A Layer on top of OS perf layer - PERF/OProfile/Windows Driver etc
//
class PerfProfiler
{
public:
    PerfProfiler() : m_pProfileConfig(NULL), m_pTarget(NULL), m_state(PERF_PROFILER_STATE_NOT_INITIALIZED) {}

    virtual ~PerfProfiler() {}

    // initialize the internal data structures..
    // check if the relevant profiling subsystem is available
    virtual HRESULT initialize(PerfConfig*  config, PerfPmuTarget* tgt) { (void)(config); (void)(tgt); return E_FAIL; }
    virtual HRESULT setProfileConfig(PerfConfig*  config) { (void)(config); return E_FAIL;}
    virtual HRESULT setPMUTarget(PerfPmuTarget* tgt) { (void)(tgt); return E_FAIL;}

    // Profiling related APIs
    virtual HRESULT startProfile(bool enable) = 0;
    virtual HRESULT stopProfile() = 0;
    virtual HRESULT enableProfile() = 0;
    virtual HRESULT disableProfile() = 0;
    // virtual int pauseProfile() = 0;
    // virtual int resumeProfile() = 0;

    // Read the profile Data
    virtual HRESULT readCounters(PerfEventCountDataList** countData = NULL) = 0;
    // virtual int getCounterValues() { return E_NOTIMPL; }
    // virtual int printCounterValues() { return E_NOTIMPL; }

    // TODO: Provide a callback mechanism; The user can register
    // a callback function. If a callback function is registered
    // it will be invoked whenever a buffer overflow notification
    // occurs. The base address of the sample-buffer and size of
    // the sample buffer will be passed to the callback routine.
    //
    // Its upto the callback to do whatever it wants ..
    //   - write into a file
    //   - process and constructs histograms..
    //   - print/dump...
    //
    virtual int readSampleBuffers() = 0;

    // Clear the internal data structures
    // clears both PerfConfig and PerfPmuTarget
    void clear();

    void setOutputFile(const std::string& file, bool overwrite = false);

    const std::string& getOutputFile() const { return m_outputFile; }

    const std::string& getErrStr() const { return m_errStr; }

    // Profile(r) state
    enum  ProfileState
    {
        PERF_PROFILER_STATE_NOT_INITIALIZED = -1,

        // Set when a profile config is associated with the profiler
        PERF_PROFILER_STATE_INITIALIZED     =  1,

        // Set when a PMU Target is associated with the profiler,
        PERF_PROFILER_STATE_READY           =  2,

        PERF_PROFILER_STATE_ACTIVE          =  3,
        PERF_PROFILER_STATE_INACTIVE        =  4,
        PERF_PROFILER_STATE_ERROR           =  5,
        PERF_PROFILER_STATE_PAUSED          =  6
    };

protected:
    // Profile Configuration;
    PerfConfig* m_pProfileConfig;

    // Target to be profiled;
    PerfPmuTarget* m_pTarget;

    // Profile state
    ProfileState m_state;

    // Output profile data file
    std::string m_outputFile;

    // Error string
    std::string m_errStr;
};

#endif // _PERFPROFILER_H_
