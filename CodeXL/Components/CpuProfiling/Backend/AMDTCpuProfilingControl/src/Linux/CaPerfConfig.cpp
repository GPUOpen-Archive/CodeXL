//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfConfig.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingControl/src/Linux/CaPerfConfig.cpp#4 $
// Last checkin:   $DateTime: 2016/04/14 02:12:20 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569057 $
//=====================================================================

// system headers
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

// C++ headers
#include <list>

// project headers
#include "CaPerfConfig.h"
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

static int getCpuInfo(CACpuVec* cpuVec)
{
    FILE* cpuOnline;
    int cpu, prev;
    char sep;
    HRESULT ret;

    if (! cpuVec)
    {
        return E_INVALIDARG;
    }

    cpuVec->clear();

    cpuOnline = fopen("/sys/devices/system/cpu/online", "r");

    if (!cpuOnline)
    {
        return E_FAIL;
    }

    sep = 0;
    prev = -1;

    for (;;)
    {
        ret = fscanf(cpuOnline, "%u%c", &cpu, &sep);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"cpu(%u), sep(%c), ret(%d).\n", cpu, sep, ret);

        if (ret <= 0)
        {
            break;
        }

        if (prev >= 0)
        {
            while (++prev < cpu)
            {
                cpuVec->push_back(prev);
            }
        }

        cpuVec->push_back(cpu);

        if (ret == 2)
        {
            prev = (sep == '-') ? cpu : -1;
        }

        if (ret == 1 || sep == '\n')
        {
            break;
        }
    }

    fclose(cpuOnline);

    return cpuVec->size();
}


CaPerfConfig::CaPerfConfig(PerfConfig& cfg, PerfPmuTarget& tgt) : PerfConfig(cfg), m_pmuTgt(tgt)
{
    m_nbrCtrPerfEvents = 0;
    m_ctrPerfEventList.clear();
    m_nbrSamplingPerfEvents = 0;
    m_samplingPerfEventList.clear();

    m_sampleRecSize = 0;

    m_countData.clear();

    m_nbrFds = 0;
    m_nbrMmaps = 0;

    m_useIoctlRedirect = false;

    // Note: PERF expects the mmap pages for sample-data-buffer to be multiple of
    //       1 + 2^n pages. Hence, m_mmapPages should be multiple 1 + 2^n pages,
    //         - first page is a meta-data page (struct perf_event_mmap_page)
    //         - other pages form ring buffer
    m_pageSize = sysconf(_SC_PAGE_SIZE);

    // Baskar: Perf has added an ioctl option "PERF_EVENT_IOC_SET_OUTPUT"
    // This can be used to report the event notification to the specified
    // file descriptor, rather than the default one. But the file
    // descriptors must all be on the same CPU.
    //
    // If we use this ioctl-redirection, then we will have only one mmap
    // region per cpu. And hence we need more buffer pages. Where as if
    // we use mmap region per fd, then we don't need too many buffer pages.
    //
    // Note: When i tried the ioctl-redirection, sometimes CXL reports no samples
    // - even tough the caperf file exists. It seems some changes may be
    // requried in CaPerfTranslator. Hence currently ioctl-redirection is
    // set to false.
    //

    if (! m_useIoctlRedirect)
    {
        m_mmapPages = 16;
    }
    else
    {
        m_mmapPages = (512 * 1024) / m_pageSize; // 128 pages
    }

    m_mmapLen = (m_mmapPages + 1) * m_pageSize;
    m_mmap = NULL;

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"CaPerfConfig::CaPerfConfig: m_pageSize  = %d\n", m_pageSize);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"CaPerfConfig::CaPerfConfig: m_mmapPages = %d\n", m_mmapPages);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"CaPerfConfig::CaPerfConfig: m_mmapLen   = %d\n", m_mmapLen);
    m_pollFds = 0;
    m_samplingPollFds = NULL;

    m_profileCompleted = false;

    m_cpuFdMap.clear();
    m_cpuVec.clear();
    m_threadVec.clear();

    m_pData = new gtByte[m_mmapLen];

    m_commRecord = false;
}


CaPerfConfig::~CaPerfConfig()
{
    clear();
}


CaPerfConfig::CaPerfConfig(const CaPerfConfig& cfg) : PerfConfig(cfg), m_pmuTgt(cfg.m_pmuTgt)
{
    m_nbrCtrPerfEvents = cfg.m_nbrCtrPerfEvents;
    m_ctrPerfEventList = cfg.m_ctrPerfEventList;
    m_nbrSamplingPerfEvents = cfg.m_nbrSamplingPerfEvents;
    m_samplingPerfEventList = cfg.m_samplingPerfEventList;

    m_nbrFds = cfg.m_nbrFds;
    m_nbrMmaps = cfg.m_nbrMmaps;
    m_mmapLen = cfg.m_mmapLen;

    m_sampleRecSize = cfg.m_sampleRecSize;

    m_cpuFdMap.clear();
    m_cpuFdMap = cfg.m_cpuFdMap;

    m_cpuVec.clear();
    m_cpuVec = cfg.m_cpuVec;

    m_threadVec.clear();
    m_threadVec = cfg.m_threadVec;

    m_pData = new gtByte[m_mmapLen];

    m_useIoctlRedirect = cfg.m_useIoctlRedirect;
    m_commRecord = cfg.m_commRecord;
}


CaPerfConfig& CaPerfConfig::operator=(const CaPerfConfig& cfg)
{
    if (&cfg == this)
    {
      return *this;
    }
    clear();
    PerfConfig::operator=(cfg);

    m_pmuTgt = cfg.m_pmuTgt;

    m_nbrCtrPerfEvents = cfg.m_nbrCtrPerfEvents;
    m_ctrPerfEventList = cfg.m_ctrPerfEventList;
    m_nbrSamplingPerfEvents = cfg.m_nbrSamplingPerfEvents;
    m_samplingPerfEventList = cfg.m_samplingPerfEventList;

    m_sampleRecSize = cfg.m_sampleRecSize;

    m_nbrFds = cfg.m_nbrFds;
    m_nbrMmaps = cfg.m_nbrMmaps;
    m_mmapLen = cfg.m_mmapLen;

    m_cpuFdMap.clear();
    m_cpuFdMap = m_cpuFdMap;

    m_cpuVec.clear();
    m_cpuVec = cfg.m_cpuVec;

    m_threadVec.clear();
    m_threadVec = cfg.m_threadVec;

    m_pData = new gtByte[m_mmapLen];

    m_useIoctlRedirect = cfg.m_useIoctlRedirect;
    m_commRecord = cfg.m_commRecord;

    return *this;
}


void CaPerfConfig::clear()
{
    m_ctrPerfEventList.clear();
    m_nbrCtrPerfEvents = 0;

    // munmap
    if (m_mmap)
    {
        for (int i = 0; i < m_nbrMmaps; i++)
        {
            if (m_mmap[i].m_base != NULL)
            {
                munmap(m_mmap[i].m_base, m_mmapLen);
                m_mmap[i].m_base = NULL;
            }
        }

        delete[] m_mmap;
        m_mmap = NULL;
    }

    m_pollFds = 0;

    if (m_samplingPollFds)
    {
        free(m_samplingPollFds);
        m_samplingPollFds = NULL;
    }

    m_samplingPerfEventList.clear();
    m_nbrSamplingPerfEvents = 0;

    m_countData.clear();

    m_nbrFds = 0;
    m_nbrMmaps = 0;
    m_mmapLen = 0;

    m_sampleRecSize = 0;

    if (NULL != m_pData)
    {
        delete [] m_pData;
        m_pData = NULL;
    }

    m_cpuFdMap.clear();
    m_cpuVec.clear();
    m_threadVec.clear();
}


uint16_t CaPerfConfig::getSampleRecSize(uint64_t sampleType, bool sampleIdAll)
{
    struct ca_sample_data* data;
    uint16_t size = 0;

    if (! sampleIdAll)
    {
        return size;
    }

    if (sampleType & PERF_SAMPLE_TID)
    {
        size += sizeof(data->tid) * 2;
    }

    if (sampleType & PERF_SAMPLE_TIME)
    {
        size += sizeof(data->time);
    }

    if (sampleType & PERF_SAMPLE_ID)
    {
        size += sizeof(data->id);
    }

    if (sampleType & PERF_SAMPLE_STREAM_ID)
    {
        size += sizeof(data->stream_id);
    }

    if (sampleType & PERF_SAMPLE_CPU)
    {
        size += sizeof(data->cpu) * 2;
    }

    return size;
}


HRESULT CaPerfConfig::initialize()
{
    // construct the CaPerfEvent objects from
    //    PerfConfig::m_ctrEventList
    //    PerfConfig::m_samplingEventList

    uint64_t sampleType = 0;
    bool     sampleIdAll = false;
    bool     firstEvent = true;

    // initialze the m_cpuVec;
    getCpuInfo(&m_cpuVec);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Number of CPUs : %d", m_cpuVec.size());

    // For per-process get the target pid's thread list;
    // TODO: Not sure whether we need to maintain per process CAThreadVec.
    // It may be a problem during PERF fd redirect ??
    size_t numPids;
    pid_t*  pids;

    if (! m_pmuTgt.isSWP())
    {
        m_pmuTgt.getPids(&numPids, &pids);

        for (size_t i = 0; i < numPids; i++)
        {
            osProcessThreadsEnumerator threadEnum;

            if (threadEnum.initialize(pids[i]))
            {
                gtUInt32 threadId;

                while (threadEnum.next(threadId))
                {
                    m_threadVec.push_back(threadId);
                }
            }
        }
    }

    // TODO: check whether the cpu-list in the pmutarget is valid or not ?

    // iterate over the counting event list...
    if (m_ctrEventList.size() != 0)
    {
        PerfEventList::iterator iter = m_ctrEventList.begin();

        for (; iter != m_ctrEventList.end(); iter++)
        {
            CaPerfEvent evt(*iter, this);

            m_ctrPerfEventList.push_back(evt);
        }
    }

    m_nbrCtrPerfEvents = m_ctrPerfEventList.size();

    if (m_samplingEventList.size() != 0)
    {
        PerfEventList::iterator iter = m_samplingEventList.begin();

        for (; iter != m_samplingEventList.end(); iter++)
        {
            CaPerfEvent evt(*iter, this);

            if (! firstEvent)
            {
                evt.setPerfAttrMmap(false);
#ifdef LINUX_PERF_MMAP_DATA_SUPPORT
                evt.setPerfAttrMmapData(false);
#endif
                evt.setPerfAttrComm(false);
            }

            firstEvent = false;

            m_samplingPerfEventList.push_back(evt);

            sampleType = evt.getSampleType();
#ifdef LINUX_PERF_SAMPLE_ID_ALL_SUPPORT
            sampleIdAll = evt.getSampleIdAll();
#endif
        }
    }

    m_nbrSamplingPerfEvents = m_samplingPerfEventList.size();

    // FIXME;
    if (sampleType)
    {
        m_sampleRecSize = getSampleRecSize(sampleType, sampleIdAll);
    }

    // FIXME: just for testing
    HRESULT ret = m_dataWriter.init(getOutputFile().c_str(), isOverwrite());

    if (S_OK != ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"initializing datafile failed... ret(%d)", ret);
        return E_FAIL;
    }

    OS_OUTPUT_DEBUG_LOG(L"initializing datafile succeedded...", OS_DEBUG_LOG_EXTENSIVE);

    return S_OK;
}

HRESULT CaPerfConfig::initSamplingEvents()
{
    // number of tgts
    size_t tgts;
    tgts  = (m_pmuTgt.isSWP()) ? m_pmuTgt.m_nbrCpus
            : m_cpuVec.size() * m_threadVec.size();

    int maxMmaps = tgts;

    if (m_samplingPerfEventList.size())
    {
        maxMmaps *= m_samplingPerfEventList.size();
    }

    if (! tgts)
    {
        return E_UNEXPECTED;
    }

    // allocate memory for mmap
    // m_mmap = new CaPerfMmap[tgts];
    m_mmap = new CaPerfMmap[maxMmaps];

    if (! m_mmap)
    {
        OS_OUTPUT_DEBUG_LOG(L"failed to allocate m_mmap.", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }

    if (! m_samplingPerfEventList.size())
    {
        return E_UNEXPECTED;
    }

    m_pollFds = 0;
    m_samplingPollFds = (struct pollfd*)malloc(sizeof(struct pollfd) * maxMmaps);

    if (! m_samplingPollFds)
    {
        OS_OUTPUT_DEBUG_LOG(L"failed to allocate m_samplingPollFds.", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }


    // TODO: i need to revist the logic for using ioctl-redirect..
    // It seems, we can use ioctl redirect only the fd's corresponding to one particular cpu..
    // i may have change the internal structures..
    int idx = 0;
    CaPerfEvtList::iterator iter = m_samplingPerfEventList.begin();

    for (; iter != m_samplingPerfEventList.end(); iter++, idx++)
    {
        PerfEventCountData* data;
        int ret = iter->getEventData(&data);

        if (ret <= 0)
        {
            OS_OUTPUT_DEBUG_LOG(L"Error in CAPErfCfg::initSamplingEvents.", OS_DEBUG_LOG_ERROR);
            return E_INVALIDDATA;
        }

#ifdef ENABLE_FAKETIMER

        if (idx == m_timerHackIndex)
        {
            m_timerHackInfo.numCpu = tgts;
            m_timerHackInfo.timerFds = new uint32_t[tgts];
            m_timerHackInfo.fakeTimerFds = new uint32_t[tgts];
        }

#endif

        int first_fd = -1;

        for (int i = 0; i < ret; i++)
        {
            int fd = data[i].m_fd;
            int cpu = data[i].m_cpu;

            if (-1 == fd)
            {
                OS_OUTPUT_DEBUG_LOG(L"Invalid fd in PerfEventCountData.", OS_DEBUG_LOG_ERROR);
                return E_INVALIDDATA;
            }

#ifdef ENABLE_FAKETIMER

            if (idx == m_timerHackIndex)
            {
                m_timerHackInfo.timerFds[cpu] = data[i].m_sampleId;
            }
            else if (idx == (m_timerHackIndex + 1))
            {
                m_timerHackInfo.fakeTimerFds[cpu] = data[i].m_sampleId;
            }

#endif

            // check we have the first-fd for this cpu
            if (m_useIoctlRedirect)
            {
                CpuFdMap::iterator iter;
                iter = m_cpuFdMap.find(cpu);

                if (iter == m_cpuFdMap.end())
                {
                    m_cpuFdMap.insert(CpuFdMap::value_type(cpu, fd));

                    // This is the first file-descriptor for this cpu, create
                    // a mmap region. Subsequent fd's for this cpu will redirect
                    // the event-notification to this mmaped region
                    first_fd = -1;

                    iter = m_cpuFdMap.find(cpu);

                    if (iter == m_cpuFdMap.end())
                    {
                        OS_OUTPUT_DEBUG_LOG(L"Insert into m_cpufdMap failed...", OS_DEBUG_LOG_ERROR);
                    }
                }
                else
                {
                    // There is already a mmap'd region assocaited with a
                    // file-descriptor for this cpu. If requested use
                    // ioctl-redirection. All the other fds on the same CPU
                    // will use the same mmap'd region.
                    first_fd = iter->second;
                }
            }

            if (first_fd == -1)
            {
                first_fd = fd;

                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"fd(%d), m_mmapLen(%d), pagesize(%d)", fd, m_mmapLen, m_pageSize);

                //
                // We need to allocate a writable page as we need to write to data_tail.
                // Internally in the kernel ring buffer code, the data_tail is used as
                // one of the inputs in perf_output_space() to make sure that the new
                // records are not overwriting the older ones that have not been consumed
                // by us.
                //
                void* base = mmap(NULL,
                                  m_mmapLen,
                                  PROT_READ | PROT_WRITE,
                                  MAP_SHARED,
                                  fd,
                                  0);

                if (MAP_FAILED == base)
                {
                    // Get the name of the error
                    char* errName = strerror(errno);

                    // Dump the error to the log file
                    if (NULL == errName)
                    {
                        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"mmap failed in CAPErfCfg::initSamplingEvents. errno(%d): No description - strerror() returned NULL for error description ",
                                                   errno);
                    }
                    else
                    {
                        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"mmap failed in CAPErfCfg::initSamplingEvents. errno(%d): %hs",
                                                   errno, errName);
                    }

                    return E_INVALIDDATA;
                }

                OS_OUTPUT_DEBUG_LOG(L"mmap succeedded...", OS_DEBUG_LOG_EXTENSIVE);

                // mask used to handle the ring-buffer boundary
                int mask = (m_mmapPages * m_pageSize) - 1;
                m_mmap[m_nbrMmaps].set(base, mask, 0);
                m_nbrMmaps++;

                // add it to pollfd list
                fcntl(fd, F_SETFL, O_NONBLOCK);

                m_samplingPollFds[m_pollFds].fd = fd;
                m_samplingPollFds[m_pollFds].events = POLLIN;
                m_pollFds++;

                // if we are not going to use ioctl redirect, reset first_fd to -1;
                if (! m_useIoctlRedirect)
                {
                    first_fd = -1;
                }
            }
            else
            {
                // ioctl redirect
                int rv = ioctl(fd, PERF_EVENT_IOC_SET_OUTPUT, first_fd);

                if (0 != rv)
                {
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error while ioctl - ret(%d) fd(%d), first_fd(%d), errno(%d)",
                                               rv, fd, first_fd, errno);
                    return E_UNEXPECTED;

                }
            }
        }
    }

    // The state is maintained by CaPerfProfiler and PerfPmuSession objects.
    return S_OK;
}

#define PERF_EVENT_PARANOID "/proc/sys/kernel/perf_event_paranoid"

#define PERF_EVENT_PARANOID_MINUS_1_ERR \
    "Profiling as non-root user with current configuration requires\n" \
    PERF_EVENT_PARANOID \
    "\nto be set to -1 by privilege users."

void CaPerfConfig::_checkPerfEventParanoid()
{
    //-----------------------------------------------------------
    // Reading perf_event_paranoid
    int fd = open(PERF_EVENT_PARANOID, O_RDONLY);

    if (fd >= 0)
    {
        char paranoid_buf[10] = {'\0'};
        size_t rdSz = read(fd, paranoid_buf, 10);
        close(fd);

        if (rdSz <= 0)
        {
            return;
        }

        paranoid_buf[rdSz] = '\0';

        //-----------------------------------------------------------
        // Check user id.
        uid_t uid = getuid();

        // NOTE [Suravee]:
        // For non-root users, paranoid must be set to -1 when:
        // - Using more than 1 counters
        // - Using system-wide mode
        if (uid != 0 && strcmp("-1", paranoid_buf))
        {
            m_errStr = m_errStr + PERF_EVENT_PARANOID_MINUS_1_ERR;
        }

    }

    return;
}


HRESULT CaPerfConfig::startProfile(bool enable)
{
    //------------------------------------
    // Setting up events
    HRESULT ret;

    if (m_ctrPerfEventList.size() != 0)
    {
        CaPerfEvtList::iterator iter = m_ctrPerfEventList.begin();

        for (; iter != m_ctrPerfEventList.end(); iter++)
        {
            ret = iter->startEvent(false);

            if (S_OK != ret)
            {
                _checkPerfEventParanoid();
                return ret;
            }
        }
    }

    if (m_samplingPerfEventList.size() != 0)
    {
        CaPerfEvtList::iterator iter = m_samplingPerfEventList.begin();

        for (; iter != m_samplingPerfEventList.end(); iter++)
        {
            // Don't start the profile here, even if the user has requested so
            // We need the PERF's "file descriptor" of sampling-events
            // to mmap. This is handled in initSamplingEvents()
            ret = iter->startEvent(false);

            if (S_OK != ret)
            {
                _checkPerfEventParanoid();
                return ret;
            }
        }
    }

    //------------------------------------
    // Event info stuff

    // If there are any sampling events, initialize the respective
    // internal structures.
    ret = initSamplingEvents();

    if (S_OK != ret)
    {
        _checkPerfEventParanoid();

        // TODO: cleanup m_samplingPerfEventList, m_ctrPerfEventList, etc
        return ret;
    }

    ret = m_dataWriter.writeEventsConfig(
              m_ctrPerfEventList,
              m_samplingPerfEventList);

    if (S_OK != ret)
    {
        return ret;
    }

    //------------------------------------
    // Target Pid stuff
    size_t numPids;
    pid_t*  pids;

    m_pmuTgt.getPids(&numPids, &pids);

    m_dataWriter.writeTargetPids(numPids, pids);

#ifdef ENABLE_FAKETIMER
    //------------------------------------
    // Fake timer info stuff
    m_dataWriter.writeFakeTimerInfo(&m_timerHackInfo);
#endif

    //------------------------------------
    // Kernel info stuff
    m_dataWriter.writeKernelInfo(m_sampleRecSize);

    if (enable)
    {
        enableProfile();
    }

    return S_OK;
}


HRESULT CaPerfConfig::enableProfile()
{
    HRESULT ret = S_OK;

    // write the FORK, COMM & MMAP records for the target pids;
    // If we don't write the COMM/MMAP records here, for "profile paused" case,
    // we won't get these records. And source correlation is not possible.
    if (! m_commRecord)
    {
        m_commRecord = true;

        if (m_pmuTgt.isSWP())
        {
            // For system-wide profiling mode.
            ret = m_dataWriter.writeSWPProcessMmaps(m_sampleRecSize);
        }
        else
        {
            // For per-process mode, write the FORK, COMM and MMAP records for each process.
            size_t numPids;
            pid_t*  pids;

            m_pmuTgt.getPids(&numPids, &pids);

            for (size_t i = 0; i < numPids; i++)
            {
                if (S_OK != m_dataWriter.writePidFork(pids[i], m_sampleRecSize))
                {
                    continue;
                }

                pid_t tgid = m_dataWriter.writePidComm(pids[i], m_sampleRecSize);

                if (S_OK != m_dataWriter.writePidMmaps(pids[i], tgid, m_sampleRecSize))
                {
                    continue;
                }
            }
        }
    }

    if (m_ctrPerfEventList.size() != 0)
    {
        CaPerfEvtList::iterator iter = m_ctrPerfEventList.begin();

        for (; iter != m_ctrPerfEventList.end(); iter++)
        {
            ret = iter->enableEvent();

            if (S_OK != ret)
            {
                OS_OUTPUT_DEBUG_LOG(L"Error in enableProfile - counting events", OS_DEBUG_LOG_ERROR);
                break;
            }
        }
    }

    if (m_samplingPerfEventList.size() != 0)
    {
        CaPerfEvtList::iterator iter = m_samplingPerfEventList.begin();

        for (; iter != m_samplingPerfEventList.end(); iter++)
        {
            ret = iter->enableEvent();

            if (S_OK != ret)
            {
                OS_OUTPUT_DEBUG_LOG(L"Error in enableProfile - sampling events", OS_DEBUG_LOG_ERROR);
                break;
            }
        }
    }

    return ret;
}

HRESULT CaPerfConfig::disableProfile()
{
    HRESULT ret = S_OK;

    if (m_ctrPerfEventList.size() != 0)
    {
        CaPerfEvtList::iterator iter = m_ctrPerfEventList.begin();

        for (; iter != m_ctrPerfEventList.end(); iter++)
        {
            ret = iter->disableEvent();

            if (S_OK != ret)
            {
                OS_OUTPUT_DEBUG_LOG(L"Error in disableProfile - counting event", OS_DEBUG_LOG_ERROR);
                break;
            }
        }
    }

    if (m_samplingPerfEventList.size() != 0)
    {
        CaPerfEvtList::iterator iter = m_samplingPerfEventList.begin();

        for (; iter != m_samplingPerfEventList.end(); iter++)
        {
            ret = iter->disableEvent();

            if (S_OK != ret)
            {
                OS_OUTPUT_DEBUG_LOG(L"Error in disableProfile - sampling event", OS_DEBUG_LOG_ERROR);
                break;
            }
        }
    }

    // While disabling the profile, don't set this flag,
    // m_profileCompleted = true;

    return ret;
}


HRESULT CaPerfConfig::stopProfile()
{
    HRESULT ret;

    if (m_ctrPerfEventList.size() != 0)
    {
        CaPerfEvtList::iterator iter = m_ctrPerfEventList.begin(), iterEnd = m_ctrPerfEventList.end();

        for (; iter != iterEnd; ++iter)
        {
            ret = iter->disableEvent();

            if (S_OK != ret)
            {
                OS_OUTPUT_DEBUG_LOG(L"Error in stopProfile - counting event", OS_DEBUG_LOG_ERROR);
                break;
            }
        }
    }

    if (m_samplingPerfEventList.size() != 0)
    {
        CaPerfEvtList::iterator iter = m_samplingPerfEventList.begin(), iterEnd = m_samplingPerfEventList.end();

        for (; iter != iterEnd; ++iter)
        {
            ret = iter->disableEvent();

            if (S_OK != ret)
            {
                OS_OUTPUT_DEBUG_LOG(L"Error in stopProfile - sampling event", OS_DEBUG_LOG_ERROR);
                break;
            }
        }
    }

    m_profileCompleted = true;

    return S_OK;
    // return disableProfile();
}


HRESULT CaPerfConfig::printSample(void* data)
{
    struct perf_event_header*  hdr = (perf_event_header*)data;

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"type(%d), misc(%d), size(%d)", hdr->type, hdr->misc, hdr->size);

    return S_OK;
}


HRESULT CaPerfConfig::readMmapBuffers()
{
    CaPerfMmap* mmapBuffer;
    struct perf_event_mmap_page* mmapMetaData; // first page in mmapBuffer
    unsigned int dataHead;
    unsigned int dataTail;
    unsigned char* dataBuf;
    void* data;

    // Iterate over all the mmaps and read them..
    for (int i = 0; i < m_nbrMmaps; i++)
    {
        mmapBuffer = &m_mmap[i];

        if (mmapBuffer->m_base != NULL)
        {
            mmapMetaData = (struct perf_event_mmap_page*)mmapBuffer->m_base;
            dataHead = mmapMetaData->data_head;
            dataTail = mmapMetaData->data_tail;
            rmb();

            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"fd:%d, time_enabled = %llx, time_running = %llx",
                                       i, mmapMetaData->time_enabled, mmapMetaData->time_running);

            dataBuf = (unsigned char*)mmapMetaData + m_pageSize;
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"dataHead(%u), prevHead(%u)", dataHead, mmapBuffer->m_prev);

            if (dataHead == dataTail)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"No Samples collected so far..dataHead(%d), dataTail(%d)", dataHead, dataTail);
            }

            unsigned int prevHead = mmapBuffer->m_prev;

            if ((dataHead - prevHead) <= 0)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"Invalid data.. Current Head(%u), Prev Head(%u).", dataHead, prevHead);
                // dataHead = prevHead;
                continue;
            }

            unsigned long dataSize = dataHead - prevHead;

            if (((prevHead & mmapBuffer->m_mask) + dataSize)
                != (dataHead & mmapBuffer->m_mask))
            {
                data = &dataBuf[prevHead & mmapBuffer->m_mask];
                dataSize = mmapBuffer->m_mask + 1 - (prevHead & mmapBuffer->m_mask);
                prevHead += dataSize;
                errno = 0;

                // memcpy(m_pData, data, dataSize);
                // OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"memcpy.. errno(%d).", errno);
                m_dataWriter.writePMUSampleData(data, dataSize);

                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"1 - dataSize(%u), prevHead(%u)", dataSize, prevHead);
            }

            data = &dataBuf[prevHead & mmapBuffer->m_mask];
            dataSize = dataHead - prevHead;
            prevHead += dataSize;

            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"2 - dataSize(%u), prevHead(%u)", dataSize, prevHead);
            errno = 0;

            if (dataSize > 0)
            {
#if 0
                memcpy(m_pData + tmp, data, dataSize);
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"memcpy.. errno(%d).", errno);
                m_dataWriter.writePMUSampleData(m_pData, tmp + dataSize);
#endif // 0
                m_dataWriter.writePMUSampleData(data, dataSize);
            }
            else
            {
                OS_OUTPUT_DEBUG_LOG(L"Invalid sample buffer data...", OS_DEBUG_LOG_ERROR);
            }

            mmapBuffer->m_prev = prevHead;

            // The perf command always sets the data_tail, and we need to do the
            // same to tell the kernel where we are in terms of data consumption.
            //
            // Internally, the kernel uses data_tail to calculate whether there is
            // enough room to place new records into the memory buffer.
            mmapMetaData->data_tail = prevHead;

        }
    }

    return S_OK;
}

int CaPerfConfig::readSampleData()
{
    int ret = -1;

#ifdef CAPERF_USES_SIGUSR1
    sigset_t sigset;
    sigfillset(&sigset);
    sigdelset(&sigset, SIGUSR1);
#endif // CAPERF_USES_SIGUSR1

    // read the contents from mmap buffers
    // wait for the poll event
    // if profiling is done,
    //    stop the profile
    for (; ;)
    {
        readMmapBuffers();

        if (m_profileCompleted)
        {
            break;
        }

#ifdef CAPERF_USES_SIGUSR1
        // ppoll() will return on either receiving SIGUSR1 or if the data is available in fds
        ret = ppoll(m_samplingPollFds, m_pollFds, NULL, &sigset);

        if (EINTR == errno)
        {
            OS_OUTPUT_DEBUG_LOG(L"ppoll interrupted by signal...", OS_DEBUG_LOG_ERROR);
        }

#else
        // poll will wait for 1000msecs, and then returns
        int wait_msecs = 1000;
        ret = poll(m_samplingPollFds, m_pollFds, wait_msecs);
#endif // CAPERF_USES_SIGUSR1

        if (m_profileCompleted)
        {
            stopProfile();
        }
    }

    return ret;
}


HRESULT CaPerfConfig::readCounters(PerfEventCountDataList** countData)
{
    HRESULT ret;

    if (m_ctrPerfEventList.size() != 0)
    {
        CaPerfEvtList::iterator iter = m_ctrPerfEventList.begin();

        for (; iter != m_ctrPerfEventList.end(); iter++)
        {
            ret = iter->readCounterValue();

            if (S_OK != ret)
            {
                OS_OUTPUT_DEBUG_LOG(L"Error in reading Counter Values", OS_DEBUG_LOG_ERROR);
                break;
            }
        }
    }

    // Now that we got the counter values, populate them:
    PerfEventCountData* data;

    if (m_ctrPerfEventList.size() != 0)
    {
        CaPerfEvtList::iterator iter = m_ctrPerfEventList.begin();

        for (; iter != m_ctrPerfEventList.end(); iter++)
        {
            ret = iter->getEventData(&data);

            if (S_OK != ret)
            {
                OS_OUTPUT_DEBUG_LOG(L"Error in retrieving Counter Values", OS_DEBUG_LOG_ERROR);
                break;
            }

            for (int i = 0; i < ret; i++)
            {
                // data[i].print();
                m_countData.push_back(data[i]);
            }
        }
    }

    if (countData)
    {
        *countData = &m_countData;
    }

    return S_OK;
}


HRESULT CaPerfConfig::printCounterValues()
{
    HRESULT ret = S_OK;

    PerfEventCountData* data;

    if (m_ctrPerfEventList.size() != 0)
    {
        CaPerfEvtList::iterator iter = m_ctrPerfEventList.begin();

        for (; iter != m_ctrPerfEventList.end(); iter++)
        {
            ret = iter->getEventData(&data);

            if (S_OK != ret)
            {
                OS_OUTPUT_DEBUG_LOG(L"Error in retrieving Counter Values", OS_DEBUG_LOG_ERROR);
                break;
            }

            for (int i = 0; i < ret; i++)
            {
                data[i].print();
            }
        }
    }

    return ret;
}

void CaPerfConfig::print()
{
    // iterate over the counting event list...
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"\nNbr of Counting PERF Events : %d", m_nbrCtrPerfEvents);

    if (m_ctrPerfEventList.size() != 0)
    {
        CaPerfEvtList::iterator iter = m_ctrPerfEventList.begin();

        for (; iter != m_ctrPerfEventList.end(); iter++)
        {
            iter->print();
        }
    }

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"\nNbr of Sampling PERF Events : %d", m_nbrSamplingPerfEvents);

    if (m_samplingPerfEventList.size() != 0)
    {
        CaPerfEvtList::iterator iter = m_samplingPerfEventList.begin();

        for (; iter != m_samplingPerfEventList.end(); iter++)
        {
            iter->print();
        }
    }

    return;
}
