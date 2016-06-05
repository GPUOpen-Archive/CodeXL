//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfEvent.cpp
///
//==================================================================================

// standard headers
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <inttypes.h>

// project headers
#include "CaPerfEvent.h"
#include "CaPerfConfig.h"
#include <AMDTOSWrappers/Include/osDebugLog.h>

int sys_perf_event_open(struct perf_event_attr* attr, pid_t pid, int cpu, int group_fd, unsigned long flags)
{
    attr->size = sizeof(*attr);
    return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}

//
// Structure to hold pre-defined PERF 'type' and 'config' vlaues
//
struct defConfig
{
    uint32_t  type;
    uint32_t  config;
};

//
// Pre-defined PERF 'type' and 'config' vlaues
//
struct defConfig defaultCfgs[] =
{
    // Harwdare Types
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_BUS_CYCLES },

    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_CLOCK },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_MIGRATIONS },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MIN },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MAJ },
#if defined(LINUX_PERF_SW_FAULTS_SUPPORT)
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_ALIGNMENT_FAULTS },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_EMULATION_FAULTS },
#else
    { 0, 0 }, // dummy values
    { 0, 0 }, // dummy values
#endif

    { PERF_TYPE_TRACEPOINT, PERF_TYPE_TRACEPOINT },  // ??
#if defined(LINUX_PERF_BREAKPOINT_SUPPORT)
    { PERF_TYPE_BREAKPOINT, PERF_TYPE_BREAKPOINT },  // ??
#else
    { 0, 0 }, // dummy values
#endif

    { PERF_TYPE_RAW, 0 }, // config should be SET later ?

    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_L1D << 0) | (PERF_COUNT_HW_CACHE_OP_READ <<  8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_L1D << 0) | (PERF_COUNT_HW_CACHE_OP_READ <<  8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_L1D << 0) | (PERF_COUNT_HW_CACHE_OP_WRITE <<  8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_L1D << 0) | (PERF_COUNT_HW_CACHE_OP_WRITE <<  8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_L1D << 0) | (PERF_COUNT_HW_CACHE_OP_PREFETCH <<  8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_L1D << 0) | (PERF_COUNT_HW_CACHE_OP_PREFETCH <<  8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)) },

    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_L1I << 0) | (PERF_COUNT_HW_CACHE_OP_READ <<  8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_L1I << 0) | (PERF_COUNT_HW_CACHE_OP_READ <<  8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_L1I << 0) | (PERF_COUNT_HW_CACHE_OP_PREFETCH <<  8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_L1I << 0) | (PERF_COUNT_HW_CACHE_OP_PREFETCH <<  8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)) },

    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_LL << 0) | (PERF_COUNT_HW_CACHE_OP_READ <<  8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_LL << 0) | (PERF_COUNT_HW_CACHE_OP_READ <<  8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_LL << 0) | (PERF_COUNT_HW_CACHE_OP_WRITE <<  8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_LL << 0) | (PERF_COUNT_HW_CACHE_OP_WRITE <<  8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_LL << 0) | (PERF_COUNT_HW_CACHE_OP_PREFETCH <<  8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_LL << 0) | (PERF_COUNT_HW_CACHE_OP_PREFETCH <<  8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)) },

    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_DTLB << 0) | (PERF_COUNT_HW_CACHE_OP_READ <<  8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_DTLB << 0) | (PERF_COUNT_HW_CACHE_OP_READ <<  8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_DTLB << 0) | (PERF_COUNT_HW_CACHE_OP_WRITE <<  8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_DTLB << 0) | (PERF_COUNT_HW_CACHE_OP_WRITE <<  8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_DTLB << 0) | (PERF_COUNT_HW_CACHE_OP_PREFETCH <<  8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_DTLB << 0) | (PERF_COUNT_HW_CACHE_OP_PREFETCH <<  8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)) },

    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_ITLB << 0) | (PERF_COUNT_HW_CACHE_OP_READ <<  8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_ITLB << 0) | (PERF_COUNT_HW_CACHE_OP_READ <<  8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)) },

    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_BPU << 0) | (PERF_COUNT_HW_CACHE_OP_READ <<  8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)) },
    { PERF_TYPE_HW_CACHE, ((PERF_COUNT_HW_CACHE_BPU << 0) | (PERF_COUNT_HW_CACHE_OP_READ <<  8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16)) }
};


static int readn(int fd, void* buf, size_t n)
{
    void* buf_start = buf;

    while (n)
    {
        int ret = read(fd, buf, n);

        if (ret <= 0)
        {
            return ret;
        }

        n -= ret;
        // buf += ret;
        buf = (void*)((uintptr_t)buf + ret);
    }

    return (uintptr_t)buf - (uintptr_t)buf_start;
}


// getDynamicType
//
// This helper routine retreves the dynamic 'type' used
// by the PERF subsystem for various profile types.
// The 'type' is available at /sys/devices/<profile-type>/type
static HRESULT getDynamicType(uint32_t evtType, uint32_t* pType)
{
    char typeFile[PATH_MAX] = {'\0'};
    FILE* filePtr;
    char buffer[BUFSIZ];

    if (! pType)
    {
        return E_INVALIDARG;
    }

    // TODO: add "/sys/devices/software/type" for PERF_TYPE_SOFTWARE
    switch (evtType)
    {
        case PerfEvent::PERF_PROFILE_TYPE_TRACEPOINT:
            strcpy(typeFile, "/sys/devices/tracepoint/type");
            break;

        case PerfEvent::PERF_PROFILE_TYPE_RAW:
            strcpy(typeFile, "/sys/devices/cpu/type");
            break;

        case PerfEvent::PERF_PROFILE_TYPE_BREAKPOINT:
            strcpy(typeFile, "/sys/devices/breakpoint/type");
            break;

        case PerfEvent::PERF_PROFILE_TYPE_IBS_FETCH:
            strcpy(typeFile, "/sys/devices/ibs_fetch/type");
            break;

        case PerfEvent::PERF_PROFILE_TYPE_IBS_OP:
            strcpy(typeFile, "/sys/devices/ibs_op/type");
            break;

        default:
            return E_INVALIDARG;
    }

    if ((access(typeFile, F_OK)) < 0)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"File %hs not available", typeFile);
        return E_NOTAVAILABLE;
    }

    filePtr = fopen(typeFile, "r");

    if (NULL == filePtr)
    {
        return E_ACCESSDENIED;
    }

    if (fgets(buffer, sizeof(buffer), filePtr) == NULL)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"couldn't read %hs file", typeFile);
        fclose(filePtr);
        return E_FAIL;
    }

    *pType = atoi(buffer);

    fclose(filePtr);
    return S_OK;
}


CaPerfEvent::CaPerfEvent(PerfEventInternal& event, CaPerfConfig* profConfig) : PerfEventInternal(event),
    m_profConfig(profConfig),
    m_samplingEvent(false),
    m_hasCountData(false),
    m_pCountData(NULL)
{
    // initialize perf_event_attr structure from PerfEventInternal object
    initialize();

    m_eventDataList.clear();

    m_hasCountData  = false;
    m_groupId       = -1;   // _UNUSED_
}

//
//  class CaPerfEvent
//

// CaPerfEvent::initialize
//
// construct perf_event_attr object
//
void CaPerfEvent::initialize()
{
    HRESULT ret = S_OK;
    uint32_t attrType;

    // TODO: use compile options  -std=c++0x or -std=gnu++0x - do m_attr = { 0 };
    memset(&m_attr, 0, sizeof(m_attr));

    m_attr.size = sizeof(struct perf_event_attr);

    // get the dynamic type, if available
    ret = getDynamicType(m_event.getType(), &attrType);

    // handle special cases...
    switch (m_event.getType())
    {
        case PerfEvent::PERF_PROFILE_TYPE_RAW:
            m_attr.type = (S_OK == ret) ? attrType : (int)PERF_TYPE_RAW;
            m_attr.config = m_event.getConfig();
            break;

        case PerfEvent::PERF_PROFILE_TYPE_IBS_FETCH:
            // Note [Suravee]: 6 is Ibs Fetch. This is hidden in the kernel
            m_attr.type = (S_OK == ret) ? attrType : 6;
            m_attr.config = m_event.getConfig();
            break;

        case PerfEvent::PERF_PROFILE_TYPE_IBS_OP:
            // Note [Suravee]: 7 is Ibs Op. This is hidden in the kernel
            m_attr.type = (S_OK == ret) ? attrType : 7;
            m_attr.config = m_event.getConfig();
            break;

        default:
            m_attr.type = defaultCfgs[m_event.getType()].type;
            m_attr.config = defaultCfgs[m_event.getType()].config;
            break;
    }

    // Sampling Specification
    m_attr.sample_period = m_samplingValue;
    m_attr.freq = (m_samplebyFreq) ? 1 : 0;

    // if sampling event, specify the sample attributes - information that
    // needs to be gathered in the sample records..
    if (PerfEvent::PERF_PROFILE_TYPE_IBS_FETCH == m_event.getType()
        || PerfEvent::PERF_PROFILE_TYPE_IBS_OP == m_event.getType())
    {
        // m_attr.sample_type = m_sampleAttr | PERF_SAMPLE_RAW;
        m_attr.sample_type = m_sampleAttr;
    }
    else
    {
        m_attr.sample_type = (m_attr.sample_period) ? m_sampleAttr : 0;
    }

    // read format - format of the data returned by read() on a perf evebnt fd;
    m_attr.read_format = m_readFormat;

    uint64_t  taskFlags = m_event.getTaskFlags();
    uint64_t  profFlags = m_event.getPmuFlags();

    // set the flags
    m_attr.disabled       = 1;
    m_attr.inherit        = (taskFlags & PerfEvent::PERF_TASK_FLAG_INHERIT) ? 1 : 0;
    m_attr.pinned         = 0;
    m_attr.exclusive      = 0;

    m_attr.exclude_user   = (profFlags & PerfEvent::PERF_PMU_FLAG_EXCLUDE_USER) ? 1 : 0;
    m_attr.exclude_kernel = (profFlags & PerfEvent::PERF_PMU_FLAG_EXCLUDE_KERNEL) ? 1 : 0;
    m_attr.exclude_hv     = (profFlags & PerfEvent::PERF_PMU_FLAG_EXCLUDE_HYPERVISOR) ? 1 : 0;
    m_attr.exclude_idle   = (profFlags & PerfEvent::PERF_PMU_FLAG_EXCLUDE_IDLE) ? 1 : 0;

    // PERF's user-space-tool set mmap & comm to 1 for the first counter and 0 for
    // other counters; In CaPerfConfig::initialize, we disable this for other counters;
    m_attr.mmap           = (profFlags & PerfEvent::PERF_PMU_FLAG_INCLUDE_MMAP_DATA) ? 1 : 0;
    m_attr.comm           = (profFlags & PerfEvent::PERF_PMU_FLAG_INCLUDE_COMM_DATA) ? 1 : 0;

    m_attr.inherit_stat   = (taskFlags & PerfEvent::PERF_TASK_FLAG_INHERIT_STAT) ? 1 : 0;
    m_attr.enable_on_exec = (taskFlags & PerfEvent::PERF_TASK_FLAG_ENABLE_ON_EXEC) ? 1 : 0;
    m_attr.task           = (taskFlags & PerfEvent::PERF_TASK_FLAG_TRACE_TASK) ? 1 : 0;

    // In perf_event_attr struct, certain fields are added post 2.6.32 version.
#if defined(LINUX_PERF_PRECISE_IP_SUPPORT)
    m_attr.precise_ip     = m_event.getSkidFactor();
#endif

#if defined(LINUX_PERF_MMAP_DATA_SUPPORT)
    // non-exec mmap data; This is required to set only if the user has
    // asked for sample address PERF_SAMPLE_ADDR
    // Perf user-tool set this to 1 for the first counter and 0 for other
    // counters; In CaPerfConfig::initialize, we disable this for other counters;
    m_attr.mmap_data      = (m_sampleAttr & PERF_SAMPLE_ADDR) ? 1 : 0;
#endif

#if defined(LINUX_PERF_SAMPLE_ID_ALL_SUPPORT)
    // From /usr/include/linux/perf_event.h
    //
    // If perf_event_attr.sample_id_all is set then all event types will
    // have the sample_type selected fields related to where/when
    // (identity) an event took place (TID, TIME, ID, CPU, STREAM_ID)
    // described in PERF_RECORD_SAMPLE.
    //
    m_attr.sample_id_all  = 1;
#endif

    // watermark
    if (m_wakeupByEvents)
    {
        m_attr.watermark = 0;
        m_attr.wakeup_events = m_waterMark;
    }
    else
    {
        m_attr.watermark = 1;
        m_attr.wakeup_watermark = m_waterMark;
    }

    m_profConfig->getPMUTarget().print();
    // PerfPmuTarget &tgt = m_profConfig->getPMUTarget();
    // tgt.print();

    m_samplingEvent = (m_attr.sample_period) ? true : false;

    print();

    initEventData();
}


HRESULT CaPerfEvent::initEventData()
{
    int cnt = 0;
    PerfPmuTarget& pmuTgt = m_profConfig->getPMUTarget();
    const CAThreadVec& threadVec = m_profConfig->getThreadVec();

    if (m_pCountData)
    {
        return E_FAIL;
    }

    size_t nbr = pmuTgt.isSWP() ? pmuTgt.m_nbrCpus
                 : pmuTgt.m_nbrCpus * threadVec.size();

    m_nbrfds = nbr;
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"m_nbrfds : %d" , m_nbrfds);

    m_pCountData = new PerfEventCountData[nbr];

    for (int i = 0; i < m_nbrfds; i++)
    {
        m_pCountData[i].m_type              = m_attr.type;
        m_pCountData[i].m_config            = m_attr.config;
        m_pCountData[i].m_fd                = -1;
        m_pCountData[i].m_sampleId          = UINT_MAX;
        m_pCountData[i].m_hasValidCountData = false;
        m_pCountData[i].m_nbrValues         = 0;
        memset(m_pCountData[i].m_values, 0, sizeof(m_pCountData[i].m_values));

        // set pid and cpu values for SWP mode
        if (pmuTgt.isSWP())
        {
            m_pCountData[i].m_pid = -1;
            m_pCountData[i].m_cpu = pmuTgt.m_pCpus[i];
        }
    }

    if (!pmuTgt.isSWP())
    {
        // Set the details for Per-Process mode
        for (size_t i = 0; i < pmuTgt.m_nbrCpus; i++)
        {
            int cpu = pmuTgt.m_pCpus[i];

            for (size_t j = 0; j < threadVec.size(); j++, cnt++)
            {
                m_pCountData[cnt].m_pid = threadVec[j];
                m_pCountData[cnt].m_cpu = cpu;
            }
        }
    }

    return S_OK;
}

CaPerfEvent::CaPerfEvent(const CaPerfEvent& evt) : PerfEventInternal(evt), m_profConfig(evt.m_profConfig)
{
    memcpy(&m_attr, &(evt.m_attr), sizeof(m_attr));

    m_samplingEvent = evt.m_samplingEvent;
    m_hasCountData  = evt.m_hasCountData;

    m_eventDataList.clear();
    m_eventDataList = evt.m_eventDataList;

    m_nbrfds        = evt.m_nbrfds;

    if (evt.m_pCountData)
    {
        m_pCountData = new PerfEventCountData[m_nbrfds];

        for (int i = 0; i < m_nbrfds; i++)
        {
            m_pCountData[i] = evt.m_pCountData[i];
        }
    }

    m_groupId       = evt.m_groupId;
}


CaPerfEvent& CaPerfEvent::operator=(const CaPerfEvent& evt)
{
	if (&evt == this)
	{
		return *this;
	}
    CaPerfEvent::operator=(evt);

    m_profConfig = evt.m_profConfig;

    memcpy(&m_attr, &(evt.m_attr), sizeof(m_attr));

    m_hasCountData  = evt.m_hasCountData;
    m_samplingEvent = evt.m_samplingEvent;

    m_nbrfds        = evt.m_nbrfds;

    if (m_pCountData)
    {
        delete[] m_pCountData;
        m_pCountData = NULL;
    }

    m_eventDataList.clear();
    m_eventDataList = evt.m_eventDataList;

    if (evt.m_pCountData)
    {
        m_pCountData = new PerfEventCountData[m_nbrfds];

        for (int i = 0; i < m_nbrfds; i++)
        {
            m_pCountData[i] = evt.m_pCountData[i];
        }
    }

    m_groupId       = evt.m_groupId;

    return *this;
}


HRESULT CaPerfEvent::startEvent(bool enable)
{
    PerfPmuTarget& pmuTgt = m_profConfig->getPMUTarget();
    const CAThreadVec& threadVec = m_profConfig->getThreadVec();

    size_t nbr = m_nbrfds;

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"nbr(%d) in startEvent", nbr);

    pid_t pid = -1;
    int group_fd = -1;
    int ret;
    int cpu;
    int fd;

    // Set perf_event_attr::disabled field
    m_attr.disabled = ~enable;

    if (pmuTgt.isSWP())
    {
        for (size_t i = 0; i < nbr; i++)
        {
            cpu = pmuTgt.m_pCpus[i];

            fd = sys_perf_event_open(&m_attr, pid, cpu, group_fd, 0);
            int err = errno;

            if (fd < 0)
            {
                OS_OUTPUT_DEBUG_LOG(L"sys_perf_event_open failed..", OS_DEBUG_LOG_ERROR);

                if ((EACCES == err) || (EPERM == err))
                {
                    OS_OUTPUT_DEBUG_LOG(L"sys_perf_event_open failed due to Access Permissions...", OS_DEBUG_LOG_EXTENSIVE);
                    return E_ACCESSDENIED;
                }
                else if (ENODEV == err)
                {
                    // invalid cpu list
                    OS_OUTPUT_DEBUG_LOG(L"sys_perf_event_open failed - ENODEV...", OS_DEBUG_LOG_EXTENSIVE);
                    return E_INVALIDARG;
                }
                else if (ENOENT == err)
                {
                    // invalid cpu list
                    OS_OUTPUT_DEBUG_LOG(L"sys_perf_event_open failed - ENOENT...", OS_DEBUG_LOG_EXTENSIVE);
                    return E_INVALIDARG;
                }
                else if (EINVAL == err)
                {
                    // invalid cpu list
                    OS_OUTPUT_DEBUG_LOG(L"sys_perf_event_open failed - EINVAL...", OS_DEBUG_LOG_EXTENSIVE);
                    return E_INVALIDARG;
                }
                else
                {
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"sys_perf_event_open failed - errno(%d)", err);
                    return E_FAIL;
                }
            }
            else
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"for cpu(%d) fd(%d)", cpu, fd);
            }

            m_pCountData[i].m_fd = fd;

            // If the event is sampling event, get the sample-id
            // We set the attr::read_format to PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_TOTAL_TIME_RUNNING
            // | PERF_FORMAT_ID;
            // for this the format of the data returned by read() on the fd is:
            //      struct read_format {
            //         u64      value;
            //         u64      time_enabled;
            //         u64      time_running;
            //         u64      id;
            //      }
            //
            if (m_samplingEvent)
            {
                ret = readn(fd, m_pCountData[i].m_values, (PERF_MAX_NBR_VALUES * sizeof(uint64_t)));

                // on error, return
                if (-1 == ret)
                {
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"error in reading samplie-id for event(%d, %lx), fd(%d) errno(%d)\n",
                                               m_attr.type, m_attr.config, fd, errno);
                    return E_FAIL;
                }

                m_pCountData[i].m_sampleId =  m_pCountData[i].m_values[3];

                // Now push this into m_eventDataList
                // TODO: Eventually we will discard pCountData stuff
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"push elemnt in m_eventDataList i(%d)\n", i);
                m_eventDataList.push_back(m_pCountData[i]);
            }
        }
    }
    else
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"nbrPids(%d) nbrThreads(%d).\n", pmuTgt.m_nbrPids, threadVec.size());

        int cnt = 0;

        for (size_t i = 0; i < pmuTgt.m_nbrCpus; i++)
        {
            cpu = pmuTgt.m_pCpus[i];

            for (size_t j = 0; j < threadVec.size(); j++, cnt++)
            {
                pid = threadVec[j];

                fd = sys_perf_event_open(&m_attr, pid, cpu, group_fd, 0);

                if (-1 == fd)
                {
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"sys_perf_event_open failed cpu(%d), pid(%d), cnt(%d)", cpu, pid, cnt);
                }
                else
                {
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"for cpu(%d), pid(%d), cnt(%d), fd(%d)", cpu, pid, cnt, fd);
                }

                m_pCountData[cnt].m_fd = fd;

                // If the event is sampling event, get the sample-id
                if (m_samplingEvent)
                {
                    ret = readn(fd, m_pCountData[cnt].m_values, (PERF_MAX_NBR_VALUES * sizeof(uint64_t)));

                    // on error, return
                    if (-1 == ret)
                    {
                        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"error in reading samplie-id for event(%d, %lx), fd(%d) errno(%d)",
                                                   m_attr.type, m_attr.config, fd, errno);
                        return E_FAIL;
                    }

                    // If the event is sampling event, get the sample-id
                    // We set the attr::read_format to PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_TOTAL_TIME_RUNNING
                    // | PERF_FORMAT_ID;
                    // for this the format of the data returned by read() on the fd is:
                    //      struct read_format {
                    //         u64      value;
                    //         u64      time_enabled;
                    //         u64      time_running;
                    //         u64      id;
                    //      }
                    //
                    m_pCountData[cnt].m_sampleId =  m_pCountData[cnt].m_values[3];

                    // Now push this into m_eventDataList
                    // TODO: Eventually we will discard pCountData stuff
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"push elemnt in m_eventDataList cnt(%d)\n", cnt);
                    m_eventDataList.push_back(m_pCountData[i]);
                }
            } // threads
        } // cpus
    } // per process

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"size of m_eventDataList (%d)\n", m_eventDataList.size());
    return S_OK;
}


HRESULT CaPerfEvent::enableEvent()
{
    int ret;
    int nbr_tgts = m_nbrfds;
    int fd;

    for (int i = 0; i < nbr_tgts; i++)
    {
        fd = m_pCountData[i].m_fd;

        if (-1 == fd)
        {
            continue;
        }

        ret = ioctl(fd, PERF_EVENT_IOC_ENABLE);

        // on error, return
        if (-1 == ret)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"error in enabling event(%u, 0x%llx), fd(%d)\n",
                                       m_attr.type, m_attr.config, fd);
            return E_FAIL;
        }
    }

    return S_OK;
}


HRESULT CaPerfEvent::disableEvent()
{
    int ret;
    int nbr_tgts = m_nbrfds;
    int fd;

    for (int i = 0; i < nbr_tgts; i++)
    {
        fd = m_pCountData[i].m_fd;

        ret = ioctl(fd, PERF_EVENT_IOC_DISABLE);

        // on error, return
        if (-1 == ret)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"error in disabling event(%u, %lu), fd(%d)\n", m_attr.type, m_attr.config, fd);
            return E_FAIL;
        }
    }

    return S_OK;
}


HRESULT CaPerfEvent::readCounterValue()
{
    int ret;
    int nbr_tgts = m_nbrfds;
    int fd;
    uint64_t*  values;

    for (int i = 0; i < nbr_tgts; i++)
    {
        fd = m_pCountData[i].m_fd;
        values = m_pCountData[i].m_values;

        if (fd < 0)
        {
            continue;
        }

        ret = readn(fd, values, (4 * sizeof(uint64_t)));

        // on error, return
        if (-1 == ret)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"error in reading event(%d, %lx), fd(%d) errno(%d)",
                                       m_attr.type, m_attr.config, fd, errno);
            return E_FAIL;
        }

        // PerfEventCountData has valid counter values read..
        m_pCountData[i].m_hasValidCountData = true;

        // print the counter values...
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L" Event(%d, %ld), Value(%ld, %ld, %ld, %d)",
                                   m_attr.type, m_attr.config, values[0], values[1], values[2], values[3]);
    }

    m_hasCountData = true;

    return S_OK;
}


CaPerfEvent::~CaPerfEvent()
{
    // if fds are open close them
    // int nbr_tgts = (m_tgt.isSWP()) ? m_tgt.m_nbrCpus : m_tgt.m_nbrPids;
    int nbr_tgts = m_nbrfds;
    int fd;

    for (int i = 0; i < nbr_tgts; i++)
    {
        fd = m_pCountData[i].m_fd;

        if (fd < 0)
        {
            continue;
        }

        close(fd);
    }

    delete[] m_pCountData;
    m_eventDataList.clear();
}

void CaPerfEvent::clear()
{
    return;
}


void CaPerfEvent::print()
{
    // print m_attr fields..
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Event type      : 0x%x", m_attr.type);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Config          : 0x%llx", m_attr.config);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"sample by       : %ls", (m_attr.freq ? L"frequency" : L"period"));
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"sample value    : %llu", m_attr.sample_period);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"sample type     : 0x%llx",  m_attr.sample_type);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"read format     : 0x%llx", m_attr.read_format);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"disabled        : %ls", (m_attr.disabled ? L"true" : L"false"));
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"inherit         : %llu", m_attr.inherit);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"pinned          : %llu", m_attr.pinned);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"exclusive       : %llu", m_attr.exclusive);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"exclude_user    : %llu", m_attr.exclude_user);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"exclude_kernel  : %llu", m_attr.exclude_kernel);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"exclude_idle    : %llu", m_attr.exclude_idle);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"exclude_hv      : %llu", m_attr.exclude_hv);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"mmap            : %llu", m_attr.mmap);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"comm            : %llu", m_attr.comm);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"inherit_stat    : %llu", m_attr.inherit_stat);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"enable_on_exec  : %llu", m_attr.enable_on_exec);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"task            : %llu", m_attr.task);
#if defined(LINUX_PERF_PRECISE_IP_SUPPORT)
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"precise_ip      : %llu", m_attr.precise_ip);
#endif
#if defined(LINUX_PERF_MMAP_DATA_SUPPORT)
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"mmap_data       : %llu", m_attr.mmap_data);
#endif
#if defined(LINUX_PERF_SAMPLE_ID_ALL_SUPPORT)
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"sample_id_all   : %llu", m_attr.sample_id_all);
#endif

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"wakeup          : %ls", (m_attr.watermark ? L"bytes" : L"events"));
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"watermark       : %d", m_attr.wakeup_events);

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"nbr of fds      : %d", m_nbrfds);
    return;
}
