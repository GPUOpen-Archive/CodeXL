//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfConfig.h
///
//==================================================================================

#ifndef _CAPERFCONFIG_H_
#define _CAPERFCONFIG_H_

// Standard Headers
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <wchar.h>

#if defined(__i386__)
    // #include "../../arch/x86/include/asm/unistd.h"
    #define rmb()           asm volatile("lock; addl $0,0(%%esp)" ::: "memory")
    #define cpu_relax()     asm volatile("rep; nop" ::: "memory");
    // #define CPUINFO_PROC    "model name"
#endif

#if defined(__x86_64__)
    // #include "../../arch/x86/include/asm/unistd.h"
    #define rmb()           asm volatile("lfence" ::: "memory")
    #define cpu_relax()     asm volatile("rep; nop" ::: "memory");
    // #define CPUINFO_PROC    "model name"
#endif

// PERF header file
#ifdef __cplusplus
extern "C" {
#endif
#include <poll.h>
#include <linux/perf_event.h>
#ifdef __cplusplus
} // extern "C"
#endif

// Project Headers
#include "CaPerfEvent.h"
#include "PerfConfig.h"
#include "PerfPmuTarget.h"
#include <AMDTCpuProfilingRawData/inc/Linux/CaPerfDataWriter.h>

// Baskar:
// If this is defined, the PERF-Sample-Reader-Thread will use SIGUSR1.
#define CAPERF_USES_SIGUSR1    1

typedef gtMap<int, int> CpuFdMap;

struct CaPerfMmap
{
    void*     m_base;
    int       m_mask;   // TODO: Use proper name
    uint32_t  m_prev;

    void set(void* base, int mask, uint32_t prev)
    {
        m_base = base;
        m_mask = mask;
        m_prev = prev;
    }

    void setBase(void* base)
    {
        m_base = base;
    }

    void setMask(int mask)
    {
        m_mask = mask;
    }

    void setBase(uint32_t prev)
    {
        m_prev = prev;
    }
};

// class CaPerfConfig
//
// PERF specific Profile Event Configuration (attributes ?)
// Inherits PerfConfig to construct PERF specific structs.
//
//
class CaPerfConfig : public PerfConfig
{
public:
    CaPerfConfig(PerfConfig& cfg, PerfPmuTarget& tgt);
    ~CaPerfConfig();

    CaPerfConfig(const CaPerfConfig& cfg);
    CaPerfConfig& operator=(const CaPerfConfig& cfg);

    HRESULT initialize();
    HRESULT startProfile(bool enable = true);
    HRESULT stopProfile();
    HRESULT enableProfile();
    HRESULT disableProfile();

    int readSampleData();
    HRESULT readMmapBuffers();
    HRESULT readCounters(PerfEventCountDataList** countData = NULL);

    uint16_t getSampleRecSize(uint64_t sampleType, bool sampleIdAll);
    PerfPmuTarget& getPMUTarget() const { return m_pmuTgt; }
    const CACpuVec& getCpuVec() const { return m_cpuVec; }
    const CAThreadVec& getThreadVec() const { return m_threadVec; }

    void clear();

    // Debug APIs
    void print();
    HRESULT printSample(void* data);
    HRESULT printCounterValues();

private:
    HRESULT initSamplingEvents();
    void _checkPerfEventParanoid();

protected:
    // PMU Target
    PerfPmuTarget&        m_pmuTgt;

    uint32_t            m_nbrCtrPerfEvents;
    CaPerfEvtList       m_ctrPerfEventList;
    uint32_t            m_nbrSamplingPerfEvents;
    CaPerfEvtList       m_samplingPerfEventList;

    uint16_t            m_sampleRecSize;

    // Event Count Data
    PerfEventCountDataList  m_countData;

    int                 m_nbrFds; // UNUSED

    uint32_t            m_mmapPages;
    uint32_t            m_pageSize;
    size_t              m_mmapLen;  // length of the memory mmap
    int                 m_nbrMmaps;
    CaPerfMmap*         m_mmap;
    CpuFdMap          m_cpuFdMap;

    // Fds for reading PERF sample data buffers
    int                 m_pollFds;
    struct pollfd*      m_samplingPollFds;

    // cpus that are online in the system
    CACpuVec            m_cpuVec;
    // target-pids' threads
    CAThreadVec         m_threadVec;

    bool                m_profileCompleted;
    bool                m_useIoctlRedirect;
    bool                m_commRecord;

    // Testing..
    gtByte*             m_pData;

    //TODO: do it properly..
    CaPerfDataWriter    m_dataWriter;

    // TODO:
    // - support for event groups, Multiplexing
    // - mmap overwrite
    // - union perf_event event_copy; ??
    // - need a flag for perf output redirect ?
};

#endif //  _CAPERFCONFIG_H_
