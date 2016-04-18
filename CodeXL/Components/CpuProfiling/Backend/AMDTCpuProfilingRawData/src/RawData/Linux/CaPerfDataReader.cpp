//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfDataReader.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingRawData/src/RawData/Linux/CaPerfDataReader.cpp#7 $
// Last checkin:   $DateTime: 2016/04/14 02:12:02 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569056 $
//=====================================================================

#include <Linux/CaPerfDataReader.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

extern const char* _ca_perfRecNames[];

CaPerfDataReader::CaPerfDataReader() : PerfDataReader()
{
    clear();
}


CaPerfDataReader::~CaPerfDataReader()
{
    deinit();
}


void
CaPerfDataReader::clear()
{
    memset(&m_fileHeader, 0, sizeof(m_fileHeader));

    m_numSections = 0;
    m_pSectionHdrs = NULL;

    m_numEvents = 0;
    m_pEventCfg = NULL;

    m_numSampleIds = 0;
    m_pSampleIds = NULL;

    m_numTargetPids = 0;
    m_pTargetPids = NULL;

    m_pFakeTimerInfo = NULL;

    m_nbrCpus = 0;
    m_pTopology = NULL;
}


HRESULT CaPerfDataReader::init(const std::string& filepath)
{
    // if the file does not exist return error
    if (access(filepath.c_str(), F_OK))
    {
        return E_ACCESSDENIED;
    }

    // Open the file in read mode.
    m_fd = open(filepath.c_str(), O_RDONLY);

    if (-1 == m_fd)
    {
        return E_FAIL;
    }

    // read the caperf.data header
    HRESULT ret = readHeader();

    if (S_OK == ret)
    {
        // read the section headers
        ret = readSectionHdrs();

        if (S_OK == ret)
        {
            // read and process the various sections
            for (size_t i = 0; i < m_numSections; i++)
            {
                switch (m_pSectionHdrs[i].type)
                {
                    case CAPERF_SECTION_RUN_INFO:
                        OS_OUTPUT_DEBUG_LOG(L"Not Implemented", OS_DEBUG_LOG_DEBUG);
                        return E_FAIL;

                    case CAPERF_SECTION_CPU_INFO:
                        ret = readCpuInfoSection(m_pSectionHdrs[i].offset,
                                                 m_pSectionHdrs[i].size);
                        break;

                    case CAPERF_SECTION_EVENT_ATTRIBUTE:
                        ret = readEventConfigs(m_pSectionHdrs[i].offset,
                                               m_pSectionHdrs[i].size);
                        break;

                    case CAPERF_SECTION_EVENT_ID:
                        ret = readEventSampleIds(m_pSectionHdrs[i].offset,
                                                 m_pSectionHdrs[i].size);
                        break;

                    case CAPERF_SECTION_SAMPLE_DATA:
                        m_dataStartOffset = m_pSectionHdrs[i].offset;
                        m_dataSize = m_pSectionHdrs[i].size;
                        break;

                    case CAPERF_SECTION_COUNTER_DATA:
                        // TODO: !! not supported now !!
                        break;

                    case CAPERF_SECTION_TARGET_PIDS:
                        ret = readTargetPids(m_pSectionHdrs[i].offset,
                                             m_pSectionHdrs[i].size);
                        break;

                    case CAPERF_SECTION_FAKE_TIMER_INFO:
                        ret = readFakeTimerInfo(m_pSectionHdrs[i].offset,
                                                m_pSectionHdrs[i].size);
                        break;

                    case CAPERF_SECTION_TOPOLOGY:
                        ret = readCpuTopology(m_pSectionHdrs[i].offset,
                                              m_pSectionHdrs[i].size);
                        break;

                    case CAPERF_SECTION_DYNAMIC_PMU_TYPES:
                        ret = readDynamicPmuTypes(m_pSectionHdrs[i].offset,
                                                  m_pSectionHdrs[i].size);
                        break;

                    default:
                        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Warning: Unknown Section Header (0x%x)", m_pSectionHdrs[i].type);
                }

                if (S_OK != ret)
                {
                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error while reading Section(%d)", m_pSectionHdrs[i].type);
                    return E_FAIL;
                }
            }

            // if there are sample ids, assign them to the respective events
            ret = createEvtIdMap();

            if (S_OK != ret)
            {
                OS_OUTPUT_DEBUG_LOG(L"Error while creating EventIdMap", OS_DEBUG_LOG_ERROR);
                return E_FAIL;
            }
        }
    }

    return ret;
}


void CaPerfDataReader::deinit()
{
    if (m_fd != -1)
    {
        close(m_fd);
        m_fd = -1;
    }

    if (m_pSectionHdrs)
    {
        free(m_pSectionHdrs);
        m_pSectionHdrs = NULL;
    }

    if (m_pEventCfg)
    {
        free(m_pEventCfg);
        m_pEventCfg = NULL;
    }

    if (m_pSampleIds)
    {
        free(m_pSampleIds);
        m_pSampleIds = NULL;
    }

    if (m_pBuf)
    {
        free(m_pBuf);
        m_pBuf = NULL;
    }

    if (m_pTargetPids)
    {
        free(m_pTargetPids);
        m_pTargetPids = NULL;
    }

    if (m_pFakeTimerInfo)
    {
        free(m_pFakeTimerInfo);
        m_pFakeTimerInfo = NULL;
    }

    if (NULL != m_pTopology)
    {
        free(m_pTopology);
        m_pTopology = NULL;
    }

    // clear the internal structures
    clear();
}


HRESULT CaPerfDataReader::readHeader()
{
    if (-1 == m_fd)
    {
        return E_FAIL;
    }

    lseek(m_fd, 0, SEEK_SET);

    int ret = read(m_fd, &m_fileHeader, sizeof(m_fileHeader));

    if (ret != sizeof(m_fileHeader))
    {
        OS_OUTPUT_DEBUG_LOG(L"Error while reading the file header", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }

    // set the offset
    m_offset = lseek(m_fd, 0, SEEK_CUR);

    return S_OK;
}

HRESULT CaPerfDataReader::readSectionHdrs()
{
    lseek(m_fd, m_fileHeader.section_hdr_off, SEEK_SET);

    m_pSectionHdrs = (caperf_section_hdr_t*)malloc(m_fileHeader.section_hdr_size);

    ssize_t ret = read(m_fd, m_pSectionHdrs, m_fileHeader.section_hdr_size);

    if (ret != (ssize_t) m_fileHeader.section_hdr_size)
    {
        OS_OUTPUT_DEBUG_LOG(L"Error while reading the section headers", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }

    m_numSections = m_fileHeader.section_hdr_size / sizeof(caperf_section_hdr_t);

    return S_OK;
}

HRESULT CaPerfDataReader::readCpuInfoSection(gtUInt64 offset, gtUInt64 size)
{
    off_t ret = lseek(m_fd, offset, SEEK_SET);

    if (-1 == ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Invalid offset (0x%lx) in readCpuInfo", offset);
        return E_FAIL;
    }

    // Currently there is only one CPUID fn 0000_0001 is written
    // in caperf.data file.

    ssize_t rv = read(m_fd, &m_cpuFamilyInfo, size);

    if (rv != (ssize_t) size)
    {
        OS_OUTPUT_DEBUG_LOG(L"Error while reading the cpu info", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }

    return S_OK;
}


HRESULT CaPerfDataReader::readCpuTopology(gtUInt64 offset, gtUInt64 size)
{
    off_t ret = lseek(m_fd, offset, SEEK_SET);

    if (-1 == ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Invalid offset (0x%lx) in readCpuTopology", offset);
        return E_FAIL;
    }

    caperf_section_topology_t*  topology = NULL;
    topology = (caperf_section_topology_t*)malloc(size);

    ssize_t rv = read(m_fd, topology, size);

    if (rv != static_cast<ssize_t>(size))
    {
        OS_OUTPUT_DEBUG_LOG(L"Error while reading the cpu topology", OS_DEBUG_LOG_ERROR);
        free(topology);
        return E_FAIL;
    }

    m_pTopology = topology;
    m_nbrCpus = size / sizeof(caperf_section_topology_t);

#if 0

    for (unsigned int i = 0; i < m_nbrCpus; i++)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"core(%d), phy(%d), numa(%d)\n",
                                   m_pTopology[i].core_id,
                                   m_pTopology[i].processor_id,
                                   m_pTopology[i].numa_node_id);
    }

#endif //0

    return S_OK;
}


HRESULT CaPerfDataReader::readDynamicPmuTypes(gtUInt64 offset, gtUInt64 size)
{
    off_t ret = lseek(m_fd, offset, SEEK_SET);

    if (-1 == ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Invalid offset (0x%lx) in readDynamicPmuTypes", offset);
        return E_FAIL;
    }

    caperf_section_pmu_types_t  pmuTypes;
    ssize_t rv = read(m_fd, &pmuTypes, size);

    if (rv != (ssize_t) size)
    {
        OS_OUTPUT_DEBUG_LOG(L"Error while reading the dynamic pmu types info", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }

    m_ibsFetchType = pmuTypes.ibs_fetch;
    m_ibsOpType = pmuTypes.ibs_op;

    return S_OK;
}


HRESULT CaPerfDataReader::readEventConfigs(gtUInt64 offset, gtUInt64 size)
{
    off_t ret = lseek(m_fd, offset, SEEK_SET);

    if (-1 == ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Invalid offset (0x%lx) in readEventConfigs", offset);
        return E_FAIL;
    }

    // alloc memory to hold the event configs (attributes in PERF's terms)
    m_pEventCfg = (caperf_section_evtcfg_t*)malloc(size);

    ssize_t rv = read(m_fd, m_pEventCfg, size);

    if (rv != (ssize_t) size)
    {
        OS_OUTPUT_DEBUG_LOG(L"Error while reading the event configuration", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }

    m_numEvents = size / sizeof(caperf_section_evtcfg_t);

    return S_OK;
}


HRESULT CaPerfDataReader::readEventSampleIds(gtUInt64 offset, gtUInt64 size)
{
    off_t ret = lseek(m_fd, offset, SEEK_SET);

    if (-1 == ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Invalid offset (0x%lx) in readEventSampleIds", offset);
        return E_FAIL;
    }

    // alloc memory to hold the event configs (attributes in PERF's terms)
    m_pSampleIds = (caperf_section_sample_id_t*)malloc(size);

    ssize_t rv = read(m_fd, m_pSampleIds, size);

    if (rv != (ssize_t) size)
    {
        OS_OUTPUT_DEBUG_LOG(L"Error while reading the event configuration", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }

    m_numSampleIds = size / sizeof(caperf_section_sample_id_t);

    return S_OK;
}

HRESULT CaPerfDataReader::createEvtIdMap()
{
    if (! m_numEvents)
    {
        return E_INVALIDARG;
    }

    gtUInt32  idx = 0;

    for (size_t i = 0; i < m_numEvents; i++)
    {
        // push event_config (attribute) into PerfEventAttrVec
        m_perfEventAttrVec.push_back(m_pEventCfg[i].event_config);

        // sample-ids are per-core-per-event.
        if (! m_numCpus && m_pEventCfg[i].number_entries)
        {
            m_numCpus = m_pEventCfg[i].number_entries;
        }

        // get the sampleType info
        if (m_pEventCfg[i].event_config.sample_type)
        {
            m_sampleType = m_pEventCfg[i].event_config.sample_type;
        }

        // set the eventId-to-eventtype map
        if (! m_pEventCfg[i].number_entries)
        {
            // no event Id (sample Id) for this event
            continue;
        }

        for (size_t j = 0; j < m_pEventCfg[i].number_entries; j++)
        {
            // add entries into m_perfEvtIdToEvtTypeMap
            gtUInt64 id = m_pSampleIds[idx + j].sample_id;
            gtUInt64 data = m_pSampleIds[idx + j].cpuid;
            data = (data << 32) | i;

            m_perfEvtIdToEvtTypeMap.insert(
                PerfEvtIdToEvtTypeMap::value_type(id, data));
        }

        idx += m_pEventCfg[i].number_entries;
    }

    return S_OK;
}


HRESULT CaPerfDataReader::dumpCaPerfHeader()
{
    gtUInt32 major, minor, micro;
    gtUInt32 ver = m_fileHeader.ca_version;

    major = (ver >> 24);
    minor = (ver >> 16) & 0x00ff;
    micro = ver & 0x0000ffff;

    OS_OUTPUT_DEBUG_LOG(L"CaPerf Header :-", OS_DEBUG_LOG_INFO);
    OS_OUTPUT_DEBUG_LOG(L"-------------", OS_DEBUG_LOG_INFO);

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"CA Version : %d.%d.%d", major, minor, micro);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Version : %d", m_fileHeader.version);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"section header table offset : 0x%x", m_fileHeader.section_hdr_off);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"section header table size : 0x%x", m_fileHeader.section_hdr_size);

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Number of Sections  : %d", m_numSections);

    for (size_t i = 0; i < m_numSections; i++)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"section(%d), misc(%d), offset(0x%lx), size(0x%lx)",
                                   m_pSectionHdrs[i].type,
                                   m_pSectionHdrs[i].misc,
                                   m_pSectionHdrs[i].offset,
                                   m_pSectionHdrs[i].size);
    }

    gtUInt32 family = 0;
    gtUInt32 model = 0;
    gtUInt32 stepping = 0;
    getCpuInfo(&family, &model, &stepping);

    OS_OUTPUT_DEBUG_LOG(L"CPU Info :-", OS_DEBUG_LOG_INFO);
    OS_OUTPUT_DEBUG_LOG(L"--------", OS_DEBUG_LOG_INFO);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Number of CPUs : %d", getNumCpus());
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"CPU Family : 0x%x", family);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"CPU Model : %u", model);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"CPU Stepping : %u", stepping);

    return S_OK;
}

HRESULT CaPerfDataReader::dumpCaPerfEvents()
{
    OS_OUTPUT_DEBUG_LOG(L"Event Attributes :-", OS_DEBUG_LOG_INFO);
    OS_OUTPUT_DEBUG_LOG(L"----------------", OS_DEBUG_LOG_INFO);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Number of Events  : %d", getNumEvents());

    for (PerfEventAttrVec::iterator it = m_perfEventAttrVec.begin(), itEnd = m_perfEventAttrVec.end(); it != itEnd; ++it)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"type          : %d", it->type);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"config        : 0x%llx", it->config);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"sample_period : %lld", it->sample_period);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"sample_type   : 0x%llx", it->sample_type);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"read_format   : 0x%llx", it->read_format);
    }

    // print the eventIds
    int cpu;
    gtUInt32 event;
    gtUInt32 umask;
    gtUInt32 os;
    gtUInt32 usr;

    OS_OUTPUT_DEBUG_LOG(L"Sample IDs :-", OS_DEBUG_LOG_INFO);
    OS_OUTPUT_DEBUG_LOG(L"----------", OS_DEBUG_LOG_INFO);

    for (PerfEvtIdToEvtTypeMap::iterator it = m_perfEvtIdToEvtTypeMap.begin(), itEnd = m_perfEvtIdToEvtTypeMap.end(); it != itEnd; ++it)
    {
        getEventAndCpuFromSampleId(it->first, &cpu, &event, &umask, &os, &usr);

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"eventId(0x%016llx), event:umask:os:usr (0x%x:0x%x:0x%x:0x%x) cpu(%d)",
                                   it->first, event, umask, os, usr, cpu);
    }

    return S_OK;
}


// Copied from Suravee's PerfDataReader implementation
HRESULT CaPerfDataReader::dump_PERF_RECORD(struct perf_event_header* pHdr, void* ptr, gtUInt32 recNum)
{
    (void)(ptr); // unused
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"[rec#:%5d, offet:0x%05x size:%3d] PERF_RECORD_%hs",
                               recNum, m_offset, pHdr->size, _ca_perfRecNames[pHdr->type]);

    return S_OK;
}

HRESULT CaPerfDataReader::dumpCaPerfData()
{
    if (!isOpen())
    {
        return E_NOFILE;
    }

    // Move to the data Section
    if ((m_offset = lseek(m_fd, m_dataStartOffset, SEEK_SET)) < 0)
    {
        return E_FAIL;
    }

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"-------- PERF DATA SECTION (offset:0x%lx, size:%lu)--------",
                               m_dataStartOffset, m_dataSize);

    int retVal = S_OK;
    struct perf_event_header hdr;
    void* pBuf = NULL;
    gtUInt32 recCnt = 0;

    if (getFirstRecord(&hdr, (const void**)(&pBuf)) != S_OK)
    {
        return E_FAIL;
    }

    do
    {
        if (hdr.size == 0 || !pBuf)
        {
            retVal = E_FAIL;
            break;
        }

        dump_PERF_RECORD(&hdr, pBuf, recCnt);

        recCnt++;
    }
    while (getNextRecord(&hdr, (const void**)(&pBuf)) == S_OK);

    return retVal;
}


void CaPerfDataReader::dumpHeaderSections()
{
    dumpCaPerfHeader();
    dumpCaPerfEvents();
}


void CaPerfDataReader::dumpData()
{
    dumpCaPerfData();
}


#define CPUID_FN_0000_0001  0x00000001
#define CPUID_FN_8000_0005  0x80000005

HRESULT CaPerfDataReader::getCpuInfo(gtUInt32* pFamily, gtUInt32* pModel, gtUInt32* pStepping)
{
    if (!pFamily)
    {
        return E_FAIL;
    }

    if (CPUID_FN_0000_0001 != m_cpuFamilyInfo.function)
    {
        return E_FAIL;
    }

    union
    {
        unsigned eax;
        struct
        {
            unsigned stepping : 4;
            unsigned model : 4;
            unsigned family : 4;
            unsigned res : 4;
            unsigned ext_model : 4;
            unsigned ext_family : 8;
            unsigned res2 : 4;
        };
    } v;

    v.eax = m_cpuFamilyInfo.value[0];

    *pFamily = v.family + v.ext_family;

    if (pModel)
    {
        *pModel = v.model + (v.ext_model << 4);
    }

    if (pStepping)
    {
        *pStepping = v.stepping;
    }

    return S_OK;
}


HRESULT CaPerfDataReader::readTargetPids(gtUInt64 offset, gtUInt64 size)
{
    off_t ret = lseek(m_fd, offset, SEEK_SET);

    if (-1 == ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Invalid offset (0x%lx) in readTargetPids", offset);
        return E_FAIL;
    }

    m_pTargetPids = (pid_t*)malloc(size);

    ssize_t rv = read(m_fd, m_pTargetPids, size);

    if (rv != (ssize_t) size)
    {
        OS_OUTPUT_DEBUG_LOG(L"Error while reading the event configuration", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }

    m_numTargetPids = size / sizeof(pid_t);

    return S_OK;
}

HRESULT CaPerfDataReader::getTargetPids(size_t maxSize, pid_t* ppid)
{
    if (!ppid || !m_pTargetPids || m_numTargetPids == 0)
    {
        return E_FAIL;
    }

    size_t tmp = sizeof(pid_t) * m_numTargetPids;
    memcpy(ppid, m_pTargetPids, (tmp > maxSize) ? maxSize : tmp);

    return S_OK;
}

HRESULT CaPerfDataReader::readFakeTimerInfo(gtUInt64 offset, gtUInt64 size)
{
    if (0 == size)
    {
        return S_OK;
    }

    off_t ret = lseek(m_fd, offset, SEEK_SET);

    if (-1 == ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Invalid offset (0x%lx) in readFakeTimerInfo", offset);
        return E_FAIL;
    }

    m_pFakeTimerInfo = malloc(size);

    ssize_t rv = read(m_fd, m_pFakeTimerInfo, size);

    if (rv != (ssize_t) size)
    {
        OS_OUTPUT_DEBUG_LOG(L"Error while reading the event configuration", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CaPerfDataReader::getFakeTimerInfo(caperf_section_fake_timer_t* pInfo)
{
    if (!m_pFakeTimerInfo)
    {
        return E_FAIL;
    }

    char* tmp = (char*)m_pFakeTimerInfo;

    pInfo->numCpu = *((gtUInt32*)tmp);
    tmp += 4;

    pInfo->timerNanosec = *((gtUInt32*)tmp);
    tmp += 4;

    pInfo->timerFds = ((gtUInt32*)tmp);
    tmp += (4 * pInfo->numCpu);

    pInfo->fakeTimerFds = ((gtUInt32*)tmp);

    return S_OK;
}


HRESULT CaPerfDataReader::getTopology(unsigned int core, gtUInt16* pProcessor, gtUInt16* pNode)
{
    if ((NULL == m_pTopology)
        || (core >= (unsigned int) m_nbrCpus)
        || (NULL == pProcessor)
        || (NULL == pNode))
    {
        return E_FAIL;
    }

    *pProcessor = m_pTopology[core].processor_id;
    *pNode      = m_pTopology[core].numa_node_id;

    return S_OK;
}
