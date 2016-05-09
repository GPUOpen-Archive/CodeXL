//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfDataWriter.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingRawData/src/RawData/Linux/CaPerfDataWriter.cpp#8 $
// Last checkin:   $DateTime: 2016/04/14 02:12:02 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569056 $
//=====================================================================

#include <Linux/CaPerfDataWriter.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

CaPerfDataWriter::CaPerfDataWriter()
{
    m_fd = -1;
    m_offset = 0;

    memset(&m_fileHeader, 0, sizeof(m_fileHeader));

    m_maxNbrSections = CAPERF_MAX_SECTIONS; // !! UNUSED !!
    m_nbrSections = 0;
    memset(&m_sectionHdrOffsets, 0, sizeof(m_sectionHdrOffsets));

    m_dataStartOffset = 0;
    m_dataSize = 0;

    m_pForkEvent = NULL;
    m_pCommEvent = NULL;
    m_pMmapEvent = NULL;

    m_sampleIdList.clear();

    m_guestOs = false;
    m_kVersion.clear();
    m_rootDir.clear();
    m_kernelSymbolFile.clear();
    m_kModulesPath.clear();
    m_kModuleNameMap.clear();
    m_kModuleAddrMap.clear();
}

CaPerfDataWriter::~CaPerfDataWriter()
{
    if (m_fd != -1)
    {
        // update the section header entry for CAPERF_SECTION_SAMPLE_DATA
        updateSectionHdr(CAPERF_SECTION_SAMPLE_DATA, m_dataStartOffset, m_dataSize);

        // TODO: update counter data
        close(m_fd);
    }

    if (m_pForkEvent)
    {
        free(m_pForkEvent);
        m_pForkEvent = NULL;
    }

    if (m_pCommEvent)
    {
        free(m_pCommEvent);
        m_pCommEvent = NULL;
    }

    if (m_pMmapEvent)
    {
        free(m_pMmapEvent);
        m_pMmapEvent = NULL;
    }

    m_sampleIdList.clear();

    m_kVersion.clear();
    m_rootDir.clear();
    m_kernelSymbolFile.clear();
    m_kModulesPath.clear();
    m_kModuleNameMap.clear();
    m_kModuleAddrMap.clear();
}

HRESULT CaPerfDataWriter::init(const char* filepath, bool overwrite)
{
    int numSection = 0;

    if (!filepath)
    {
        return E_INVALIDARG;
    }

    // if the file already exists return error
    if (overwrite && (! access(filepath, F_OK)))
    {
        return E_ACCESSDENIED;
    }

    // Open the file in write mode.
    m_fd = open(filepath,
                O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (-1 == m_fd)
    {
        return E_FAIL;
    }

    // write the header
    HRESULT ret = writeHeader();

    if (S_OK != ret)
    {
        return ret;
    }

    // CPU_INFO stuff
    // write section header entry for cpu info and update the fileheader
    ret = writeSectionHdr(CAPERF_SECTION_CPU_INFO, 0, 0);

    if (S_OK != ret)
    {
        return ret;
    }

    numSection++;

    // Write the CoreTopology stuff
    // write section header entry for cpu topology and update the fileheader
    ret = writeSectionHdr(CAPERF_SECTION_TOPOLOGY, 0, 0);

    if (S_OK != ret)
    {
        return ret;
    }

    numSection++;

    // TARGET_PIDS stuff
    // write the CAPERF_SECTION_TARGET_PIDS section header
    lseek(m_fd, 0, SEEK_END);
    ret = writeSectionHdr(CAPERF_SECTION_TARGET_PIDS, 0, 0);

    if (S_OK != ret)
    {
        return ret;
    }

    numSection++;

    // FAKE_TIMER_INFO stuff
    // write the CAPERF_SECTION_FAKE_TIMER_INFO section header
    lseek(m_fd, 0, SEEK_END);
    ret = writeSectionHdr(CAPERF_SECTION_FAKE_TIMER_INFO, 0, 0);

    if (S_OK != ret)
    {
        return ret;
    }

    numSection++;

    // CAPERF_SECTION_DYNAMIC_PMU_TYPES
    // write the dynamic pmu type ids used by PERF
    lseek(m_fd, 0, SEEK_END);
    ret = writeSectionHdr(CAPERF_SECTION_DYNAMIC_PMU_TYPES, 0, 0);

    if (S_OK != ret)
    {
        return ret;
    }

    numSection++;

    ret = updateFileHeader(0, numSection * sizeof(caperf_section_hdr_t));

    return ret;
}


HRESULT CaPerfDataWriter::writeHeader()
{
    if (-1 == m_fd)
    {
        return E_FAIL;
    }

    lseek(m_fd, 0, SEEK_SET);

    memcpy((void*)(&m_fileHeader.magic), (void*)CAPERF_MAGIC_ADDR, sizeof(gtUInt64));
    m_fileHeader.version = 1;

    gtUInt32 major = 3, minor = 3, micro = 1;
    gtUInt32 ver = ((major << 24) | (minor << 16) | (micro & 0xffff));

    m_fileHeader.ca_version = ver;
    m_fileHeader.section_hdr_off = sizeof(struct caperf_file_header);
    m_fileHeader.section_hdr_size = 0;
    memset(m_fileHeader.filler, 0, sizeof(m_fileHeader.filler));

    ssize_t ret = write(m_fd, (const void*)&m_fileHeader, sizeof(m_fileHeader));

    if (ret != sizeof(m_fileHeader))
    {
        OS_OUTPUT_DEBUG_LOG(L"Error while writing the file header", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }

    // set the offset
    m_offset = lseek(m_fd, 0, SEEK_CUR);

    return S_OK;
}


HRESULT CaPerfDataWriter::updateFileHeader(gtUInt32 sect_hdr_offset, gtUInt32 sect_hdr_size)
{
    caperf_file_header_t  hdr;
    ssize_t ret;

    lseek(m_fd, 0, SEEK_SET);
    ret = read(m_fd, &hdr, sizeof(hdr));

    if (ret != sizeof(hdr))
    {
        OS_OUTPUT_DEBUG_LOG(L"Error while updating file header", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }

    // m_fileHeader should be in sync with file header contents;
    // we don't need to read the header from the file, to update the section-hdr
    // offset/size fields;

    // TODO: we don't need to update the section header offset, every time
    // only the section-header-table size should be updated
    m_fileHeader.section_hdr_off += sect_hdr_offset;
    m_fileHeader.section_hdr_size += sect_hdr_size;

    lseek(m_fd, 0, SEEK_SET);
    ret = write(m_fd, (const void*)&m_fileHeader, sizeof(m_fileHeader));

    if (ret != sizeof(m_fileHeader))
    {
        OS_OUTPUT_DEBUG_LOG(L"Error while updating file header", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CaPerfDataWriter::getCpuId(gtUInt32 fn, gtUInt32* pEax, gtUInt32* pEbx, gtUInt32* pEcx, gtUInt32* pEdx)
{
    gtUInt32  eax, ebx, ecx, edx;

    // copied from Suravee's implementation in PerfDataReader.cpp

#if defined(__i386__) && defined(__PIC__)
    /* %ebx may be the PIC register.  */

#define __cpuid(level, a, b, c, d)              \
    __asm__ ("xchgl\t%%ebx, %1\n\t"             \
             "cpuid\n\t"                 \
             "xchgl\t%%ebx, %1\n\t"              \
             : "=a" (a), "=r" (b), "=c" (c), "=d" (d)    \
             : "0" (level))
#else
#define __cpuid(level, a, b, c, d)          \
    __asm__ ("cpuid\n\t"                \
             : "=a" (a), "=b" (b), "=c" (c), "=d" (d)    \
             : "0" (level))
#endif

    /* CPUID Fn0000_0001_EAX Family, Model, Stepping */
    __cpuid(fn, eax, ebx, ecx, edx);

    *pEax = eax;
    *pEbx = ebx;
    *pEcx = ecx;
    *pEdx = edx;

    return S_OK;
}

HRESULT CaPerfDataWriter::writeCpuInfo()
{
    caperf_section_cpuinfo_t  cpuinfo;
    memset((void*)&cpuinfo, 0, sizeof(caperf_section_cpuinfo_t));

    gtUInt32  fn = 1;
    gtUInt32  eax, ebx, ecx, edx;

    getCpuId(fn, &eax, &ebx, &ecx, &edx);

    cpuinfo.function = fn;
    cpuinfo.value[0] = eax;
    cpuinfo.value[1] = ebx;
    cpuinfo.value[2] = ecx;
    cpuinfo.value[3] = edx;

    m_offset = lseek(m_fd, 0, SEEK_END);
    ssize_t sectionStOffset = m_offset;

    ssize_t rc = write(m_fd, (const void*)&cpuinfo, sizeof(cpuinfo));
    GT_ASSERT(rc != -1);

    // TODO: add L1 data cache identifier; CPUID Fn 8000_00005 for CLU ?

    m_offset = lseek(m_fd, 0, SEEK_CUR);

    // update the section header entry for CAPERF_SECTION_CPU_INFO
    updateSectionHdr(CAPERF_SECTION_CPU_INFO,
                     sectionStOffset,
                     (m_offset - sectionStOffset));

    return S_OK;
}


HRESULT CaPerfDataWriter::writeCpuTopology()
{
    caperf_section_topology_t*  topology;
    long numCpus = sysconf(_SC_NPROCESSORS_ONLN);

    if (numCpus <= 0)
    {
        return E_FAIL;
    }

    topology = (caperf_section_topology_t*)malloc(sizeof(caperf_section_topology_t) * numCpus);

    if (NULL == topology)
    {
        return E_FAIL;
    }

    size_t size = sizeof(caperf_section_topology_t) * numCpus;
    memset((void*)topology, 0, size);

#define PATH_SYS_SYSTEM    "/sys/devices/system"

    // count the number of CPUs
    char filename[OS_MAX_PATH];
    int cnt = 0;
    bool done = false;

    do
    {
        memset(filename, 0, sizeof(filename));
        snprintf(filename, sizeof(filename), "%s/cpu/cpu%d", PATH_SYS_SYSTEM, cnt);

        if (0 == (access(filename, F_OK)))
        {
            cnt++;
        }
        else
        {
            done = true;
        }
    }
    while (false == done);

    // numCpus should be equal to i
    if (numCpus != cnt)
    {
        free(topology);
        return E_FAIL;
    }

    char buffer[BUFSIZ];
    int addedCpus = 0;

    for (int i = 0; i < cnt; i++)
    {
        int coreid = 0;
        int phyid = 0;
        int numa = 0;

        // read the core id
        memset(buffer, 0, sizeof(buffer));
        memset(filename, 0, sizeof(filename));
        snprintf(filename, sizeof(filename), "%s/cpu/cpu%d/topology/core_id", PATH_SYS_SYSTEM, i);

        if ((access(filename, F_OK)))
        {
            continue;
        }

        FILE* fp = fopen(filename, "r");

        if (NULL == fp)
        {
            free(topology);
            return E_FAIL;
        }

        bool bRes = fgets(buffer, sizeof(buffer), fp) != NULL;

        fclose(fp);
        fp = NULL;

        if (!bRes)
        {
            free(topology);
            return E_FAIL;
        }

        coreid = atoi(buffer);

        // read the physical core id
        memset(buffer, 0, sizeof(buffer));
        memset(filename, 0, sizeof(filename));
        snprintf(filename, sizeof(filename), "%s/cpu/cpu%d/topology/physical_package_id", PATH_SYS_SYSTEM, i);

        if ((access(filename, F_OK)))
        {
            continue;
        }

        fp = fopen(filename, "r");

        if (NULL == fp)
        {
            free(topology);
            return E_FAIL;
        }

        bRes = fgets(buffer, sizeof(buffer), fp) != NULL;

        fclose(fp);
        fp = NULL;

        if (!bRes)
        {
            free(topology);
            return E_FAIL;
        }

        phyid = atoi(buffer);

        topology[addedCpus].core_id      = (gtUInt32) coreid;
        topology[addedCpus].processor_id = (gtUInt16) phyid;
        topology[addedCpus].numa_node_id = (gtUInt16) numa;
        addedCpus++;
    }

    m_offset = lseek(m_fd, 0, SEEK_END);
    ssize_t sectionStOffset = m_offset;

    size_t rv = write(m_fd, (const void*)topology, (sizeof(caperf_section_topology_t) * addedCpus));

    if (rv != (sizeof(caperf_section_topology_t) * addedCpus))
    {
        free(topology);
        return E_FAIL;
    }

    m_offset = lseek(m_fd, 0, SEEK_CUR);

    // update the section header entry for CAPERF_SECTION_TOPOLOGY
    updateSectionHdr(CAPERF_SECTION_TOPOLOGY,
                     sectionStOffset,
                     (m_offset - sectionStOffset));

    free(topology);
    return S_OK;
}


// This helper routine retreves the dynamic 'type' used
// by the PERF subsystem for various profile types.
// The 'type' is available at /sys/devices/<profile-type>/type
HRESULT CaPerfDataWriter::getDynamicPmuType(char* pFilePath, gtUInt32& type)
{
    FILE* filePtr = NULL;
    char buffer[BUFSIZ];
    int ret = E_FAIL;

    if (!(access(pFilePath, F_OK)))
    {
        filePtr = fopen(pFilePath, "r");

        if (NULL != filePtr)
        {
            if (NULL != fgets(buffer, sizeof(buffer), filePtr))
            {
                type = atoi(buffer);
                ret = S_OK;
            }

            fclose(filePtr);
        }
    }

    return ret;
}


HRESULT CaPerfDataWriter::writeDynamicPmuTypes()
{
    uint32_t type = 0;
    int ret = E_FAIL;
    char buffer[BUFSIZ] = { '\0' };

    caperf_section_pmu_types_t  pmuTypes;
    memset((void*)&pmuTypes, 0, sizeof(caperf_section_pmu_types_t));

    // Get the dynamic pmu type for IBS fetch
    strncpy(buffer, "/sys/devices/ibs_fetch/type", BUFSIZ - 1);
    ret = getDynamicPmuType(buffer, type);
    pmuTypes.ibs_fetch = (S_OK == ret) ? type : 6;

    // Get the dynamic pmu type for IBS fetch
    strncpy(buffer, "/sys/devices/ibs_op/type", BUFSIZ - 1);
    ret = getDynamicPmuType(buffer, type);
    pmuTypes.ibs_op = (S_OK == ret) ? type : 7;

    m_offset = lseek(m_fd, 0, SEEK_END);
    ssize_t sectionStOffset = m_offset;

    ssize_t rc = write(m_fd, (const void*)&pmuTypes, sizeof(pmuTypes));
    GT_ASSERT(rc != -1);

    m_offset = lseek(m_fd, 0, SEEK_CUR);

    // update the section header entry for CAPERF_SECTION_DYNAMIC_PMU_TYPES
    updateSectionHdr(CAPERF_SECTION_DYNAMIC_PMU_TYPES,
                     sectionStOffset,
                     (m_offset - sectionStOffset));

    return S_OK;
}


// CaPerfDataWriter::getIdx(caperf_section_type_t section)
//
// returns event idx for the known section, otherwise UINT_MAX
//
gtUInt32 CaPerfDataWriter::getIdx(gtUInt32 section)
{
    int idx = ffs(section);

    // idx cannot be zero and greater than CAPERF_MAX_SECTIONS
    return ((idx > 0) && (idx <= CAPERF_MAX_SECTIONS)) ? (idx - 1) : UINT_MAX;
}


HRESULT CaPerfDataWriter::writeSectionHdr(caperf_section_type_t section, gtUInt64 startOffset, gtUInt64 size)
{
    int ret = S_OK;
    struct caperf_section_hdr hdr;
    memset((void*)&hdr, 0, sizeof(caperf_section_hdr));

    hdr.type = section;
    hdr.offset = startOffset;
    hdr.size = size;
    hdr.misc = 0;

    gtUInt32 idx = getIdx(section);

    if (CAPERF_MAX_SECTIONS > idx)
    {
        // lseek(m_fd, m_offset, SEEK_SET);
        m_offset = lseek(m_fd, 0, SEEK_CUR);

        ssize_t ret = write(m_fd, (const void*)&hdr, sizeof(hdr));

        if (ret == sizeof(hdr))
        {
            m_sectionHdrOffsets[idx] = m_offset;
            m_nbrSections++;

            // set the offset
            m_offset = lseek(m_fd, 0, SEEK_CUR);
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(L"Error while writing section header", OS_DEBUG_LOG_ERROR);
            ret = E_FAIL;
        }
    }

    return ret;
}


HRESULT CaPerfDataWriter::updateSectionHdr(caperf_section_type_t section, gtUInt64 startOffset, gtUInt64 size)
{
    struct caperf_section_hdr hdr;
    memset((void*)&hdr, 0, sizeof(caperf_section_hdr));

    int ret = S_OK;
    gtUInt32 idx = getIdx(section);

    if (CAPERF_MAX_SECTIONS > idx)
    {
        lseek(m_fd, m_sectionHdrOffsets[idx], SEEK_SET);
        ssize_t rc = read(m_fd, &hdr, sizeof(hdr));
        GT_ASSERT(rc != -1);

        hdr.offset = startOffset;
        hdr.size = size;

        lseek(m_fd, m_sectionHdrOffsets[idx], SEEK_SET);

        ssize_t ret = write(m_fd, (const void*)&hdr, sizeof(hdr));

        if (ret != sizeof(hdr))
        {
            OS_OUTPUT_DEBUG_LOG(L"Error while updating section header", OS_DEBUG_LOG_ERROR);
            ret = E_FAIL;
        }
    }

    return ret;
}

HRESULT CaPerfDataWriter::writeCountEvent(CaPerfEvent& event)
{
    caperf_section_evtcfg  evtCfg;

    evtCfg.event_config = event.getAttribute();
    evtCfg.start_idx = 0;
    evtCfg.number_entries = 0;

    // TODO: Set the name of the event
    memset(evtCfg.name, 0, sizeof(evtCfg.name));

    m_offset = lseek(m_fd, 0, SEEK_END);
    ssize_t rc = write(m_fd, (const void*)&evtCfg, sizeof(evtCfg));
    GT_ASSERT(rc != -1);
    m_offset = lseek(m_fd, 0, SEEK_CUR);

    return S_OK;
}


HRESULT CaPerfDataWriter::writeSampleEvent(CaPerfEvent& event)
{
    caperf_section_evtcfg  evtCfg;
    const PerfEventDataList* eventDataList = event.getEventDataList();
    caperf_section_sample_id_t  sinfo;

    evtCfg.event_config   = event.getAttribute();
    evtCfg.start_idx      = m_sampleIdList.size();
    evtCfg.number_entries = eventDataList->size();

    // TODO: Set the name of the event
    memset(evtCfg.name, 0, sizeof(evtCfg.name));

    ssize_t rc = write(m_fd, (const void*)&evtCfg, sizeof(evtCfg));
    GT_ASSERT(rc != -1);

    // save the sample-id/cpu-id stuff in m_sampleIdList;
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"writeSampleEvent - eventDataList size(%d)", eventDataList->size());

    PerfEventDataList::const_iterator iter = eventDataList->begin();

    for (; iter != eventDataList->end(); iter++)
    {
        sinfo.sample_id = iter->m_sampleId;
        sinfo.cpuid = iter->m_cpu;
        sinfo.misc = 0;

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"sinfo- smaple_id(0x%lx), cpuid(%d)", sinfo.sample_id, sinfo.cpuid);

        m_sampleIdList.push_back(sinfo);
    }

    return S_OK;
}


// function to write the section headers
//
// Expects counting-events-list and sampling-events-List
//
HRESULT CaPerfDataWriter::writeEventsConfig(CaPerfEvtList& countEventsList, CaPerfEvtList& sampleEventsList)
{
    gtUByte   numberSections = 0;
    HRESULT ret = S_OK;

    if (countEventsList.size() || sampleEventsList.size())
    {
        // write the CAPERF_SECTION_EVENT_ATTRIBUTE section header
        lseek(m_fd, 0, SEEK_END);
        ret = writeSectionHdr(CAPERF_SECTION_EVENT_ATTRIBUTE, 0, 0);
        numberSections++;
    }

    if (sampleEventsList.size())
    {
        // write the CAPERF_SECTION_EVENT_ID section header
        lseek(m_fd, 0, SEEK_END);
        ret = writeSectionHdr(CAPERF_SECTION_EVENT_ID, 0, 0);

        // write the CAPERF_SECTION_SAMPLE_DATA section header
        lseek(m_fd, 0, SEEK_END);
        ret = writeSectionHdr(CAPERF_SECTION_SAMPLE_DATA, 0, 0);
        numberSections += 2;
    }

    if (countEventsList.size())
    {
        // write the CAPERF_SECTION_COUNTER_DATA section header
        lseek(m_fd, 0, SEEK_END);
        ret = writeSectionHdr(CAPERF_SECTION_COUNTER_DATA, 0, 0);
        numberSections++;
    }

    // Since, we have added new sections in section-header-table, update the file header
    // TODO: modify updateFileHeader signature ?
    updateFileHeader(0, numberSections * sizeof(caperf_section_hdr_t));

    // write the cpu-info here, just before writing the counter details.
    ret = writeCpuInfo();

    if (S_OK != ret)
    {
        return ret;
    }

    // Write the cpu topology
    ret = writeCpuTopology();

    if (S_OK != ret)
    {
        return ret;
    }

    // Write the dynamic PMU types for IBS fetch and Op
    if (S_OK != (ret = writeDynamicPmuTypes()))
    {
        return ret;
    }

    // Add Counter Events
    ssize_t sectionStOffset = lseek(m_fd, 0, SEEK_END);

    CaPerfEvtList::iterator iter = countEventsList.begin();

    for (; iter != countEventsList.end(); iter++)
    {
        writeCountEvent(*iter);
    }

    // Add Sampling Events
    iter = sampleEventsList.begin();

    for (; iter != sampleEventsList.end(); iter++)
    {
        writeSampleEvent(*iter);
    }

    ssize_t sectionEndOffset = lseek(m_fd, 0, SEEK_END);

    // update the section header entry for CAPERF_SECTION_EVENT_ATTRIBUTE
    updateSectionHdr(CAPERF_SECTION_EVENT_ATTRIBUTE,
                     sectionStOffset,
                     (sectionEndOffset - sectionStOffset));

    // Now write CAPERF_SECTION_EVENT_ID and update the corresponding section header entry
    if (m_sampleIdList.size())
    {
        writeSampleEventsInfo();
    }

    // set the offset
    m_offset = lseek(m_fd, m_offset, SEEK_CUR);

    return S_OK;
}

HRESULT CaPerfDataWriter::writeSampleEventsInfo()
{
    size_t numEntries = 0;

    if (! m_sampleIdList.size())
    {
        return E_FAIL;
    }

    m_offset = lseek(m_fd, 0, SEEK_END);

    SampleInfoList::iterator iter = m_sampleIdList.begin();

    for (; iter != m_sampleIdList.end(); iter++)
    {
        caperf_section_sample_id_t  sinfo = *iter;

        ssize_t rc = write(m_fd, (const void*)&sinfo, sizeof(caperf_section_sample_id_t));
        GT_ASSERT(rc != -1);
        numEntries++;
    }

    // update the section header entry for CAPERF_SECTION_EVENT_ID
    updateSectionHdr(CAPERF_SECTION_EVENT_ID,
                     m_offset,
                     numEntries * sizeof(caperf_section_sample_id_t));

    m_offset = lseek(m_fd, 0, SEEK_CUR);

    return S_OK;
}


HRESULT CaPerfDataWriter::writePMUSampleData(void* data, ssize_t size)
{
    if (! m_dataStartOffset)
    {
        m_dataStartOffset = lseek(m_fd, 0, SEEK_END);

        updateSectionHdr(CAPERF_SECTION_SAMPLE_DATA,
                         m_dataStartOffset,
                         m_dataSize);

        m_offset = lseek(m_fd, 0, SEEK_END);
    }

    int ret = write(m_fd, (const void*)data, size);

    if (ret != size)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error writing PMU Sample Data. ret(%d)", ret);
        return E_FAIL;
    }

    m_offset = lseek(m_fd, 0, SEEK_END);
    m_dataSize += size;
    return S_OK;
}

//
// In Systwm Wide mode, we need to add the /proc entries in the caperf.data file
//

// sampleRecSize is without perf_event_header
//
HRESULT CaPerfDataWriter::writeSWPProcessMmaps(gtUInt16 sampleRecSize)
{
    DIR* proc;
    struct dirent dirent;
    struct dirent* next;
    HRESULT ret = E_FAIL;

    proc = opendir("/proc");

    if (proc == NULL)
    {
        return ret;
    }

    while (!readdir_r(proc, &dirent, &next) && next)
    {
        char* end;
        pid_t pid = strtol(dirent.d_name, &end, 10);

        // only interested in proper numerical dirents
        if (*end)
        {
            continue;
        }

        if (!SUCCEEDED(writePidFork(pid, sampleRecSize)))
        {
            continue;
        }

        pid_t tgid = writePidComm(pid, sampleRecSize);

        if (S_OK == writePidMmaps(pid, tgid, sampleRecSize))
        {
            continue;
        }
    }

    closedir(proc);
    return S_OK;
}

// using a specific size_t instead of typeof is because c++11 does not support
// typeof with no type and in all cases where the macro is used size_t is the type used
#define ALIGN_FOR_SIZE_T(x,a) __ALIGN_MASK(x,(size_t)(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

static int hex(char ch)
{
    if ((ch >= '0') && (ch <= '9'))
    {
        return ch - '0';
    }

    if ((ch >= 'a') && (ch <= 'f'))
    {
        return ch - 'a' + 10;
    }

    if ((ch >= 'A') && (ch <= 'F'))
    {
        return ch - 'A' + 10;
    }

    return -1;
}

// FIXME
static int hex2u64(const char* ptr, gtUInt64* long_val)
{
    const char* p = ptr;
    *long_val = 0;

    while (*p)
    {
        const int hex_val = hex(*p);

        if (hex_val < 0)
        {
            break;
        }

        *long_val = (*long_val << 4) | hex_val;
        p++;
    }

    return p - ptr;
}

static char*
strxfrchar(char* s, char from, char to)
{
    char* p = s;

    while ((p = strchr(p, from)) != NULL)
    {
        *p++ = to;
    }

    return s;
}

HRESULT CaPerfDataWriter::writePidMmaps(pid_t pid, pid_t tgid, gtUInt16  sampleRecSize)
{
    char filename[OS_MAX_PATH];
    FILE* fp;

    if (! m_pMmapEvent)
    {
        m_pMmapEvent = (ca_event_t*)calloc(1, sizeof(m_pMmapEvent->mmap) + sampleRecSize);

        if (m_pMmapEvent == NULL)
        {
            return E_FAIL;
        }
    }

    snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);

    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        /*
         * We raced with a task exiting - just return:
         */
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"couldn't open %hs", filename);
        return -1;
    }

    m_pMmapEvent->header.type = PERF_RECORD_MMAP;
    /*
     * Just like the kernel, see __perf_event_mmap in kernel/perf_event.c
     */
    m_pMmapEvent->header.misc = PERF_RECORD_MISC_USER;

    while (1)
    {
        char bf[BUFSIZ], *pbf = bf;
        int n;
        size_t size;

        if (fgets(bf, sizeof(bf), fp) == NULL)
        {
            break;
        }

        /* 00400000-0040c000 r-xp 00000000 fd:01 41038  /bin/cat */
        n = hex2u64(pbf, &m_pMmapEvent->mmap.start);

        if (n < 0)
        {
            continue;
        }

        pbf += n + 1;
        n = hex2u64(pbf, &m_pMmapEvent->mmap.len);

        if (n < 0)
        {
            continue;
        }

        pbf += n + 3;

        if (*pbf == 'x')   /* vm_exec */
        {
            char* execname = strchr(bf, '/');

            /* Catch VDSO */
            if (execname == NULL)
            {
                execname = strstr(bf, "[vdso]");
            }

            if (execname == NULL)
            {
                continue;
            }

            pbf += 3;
            n = hex2u64(pbf, &m_pMmapEvent->mmap.pgoff);

            size = strlen(execname);
            execname[size - 1] = '\0'; /* Remove \n */
            memcpy(m_pMmapEvent->mmap.filename, execname, size);
            size = ALIGN_FOR_SIZE_T(size, sizeof(gtUInt64));
            m_pMmapEvent->mmap.len -= m_pMmapEvent->mmap.start;
            m_pMmapEvent->mmap.header.size = (sizeof(m_pMmapEvent->mmap) -
                                              (sizeof(m_pMmapEvent->mmap.filename) - size));
            memset(m_pMmapEvent->mmap.filename + size, 0, sampleRecSize);
            m_pMmapEvent->mmap.header.size += sampleRecSize;
            m_pMmapEvent->mmap.pid = tgid;
            m_pMmapEvent->mmap.tid = pid;

            writePMUSampleData(m_pMmapEvent, m_pMmapEvent->header.size);
        }
    }

    fclose(fp);
    return 0;
}


HRESULT CaPerfDataWriter::writePidFork(pid_t pid, gtUInt16 sampleRecSize)
{
    if (NULL == m_pForkEvent)
    {
        m_pForkEvent = (ca_event_t*)calloc(1, sizeof(m_pForkEvent->fork) + sampleRecSize);

        if (m_pForkEvent == NULL)
        {
            return E_FAIL;
        }
    }

    HRESULT hr = E_FAIL;

    osProcessId processId = pid, parentProcessId;

    if (osGetProcessIdentificationInfo(processId, &parentProcessId))
    {
        memset(&m_pForkEvent->fork, 0, sizeof(m_pForkEvent->fork));

        m_pForkEvent->fork.pid = processId;
        m_pForkEvent->fork.ppid = parentProcessId;
        m_pForkEvent->fork.ptid = parentProcessId;
        m_pForkEvent->fork.header.type = PERF_RECORD_FORK;
        size_t size = 0;
        size = ALIGN_FOR_SIZE_T(size, sizeof(gtUInt64));
        m_pForkEvent->fork.header.size = (sizeof(m_pForkEvent->fork) + size + sampleRecSize);

        // For each thread of the process
        osProcessThreadsEnumerator threadEnum;

        if (threadEnum.initialize(pid))
        {
            gtUInt32 threadId;

            while (threadEnum.next(threadId))
            {
                m_pForkEvent->fork.tid = threadId;
                writePMUSampleData(m_pForkEvent, m_pForkEvent->header.size);
            }

            hr = S_OK;
        }
    }

    if (E_FAIL == hr)
    {
        // We raced with a task exiting - just return:
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Failed to get process %d information", pid);
    }

    return hr;
}


pid_t CaPerfDataWriter::writePidComm(pid_t pid, gtUInt16 sampleRecSize)
{
    if (! m_pCommEvent)
    {
        m_pCommEvent = (ca_event_t*)calloc(1, sizeof(m_pCommEvent->comm) + sampleRecSize);

        if (m_pCommEvent == NULL)
        {
            return 0;
        }
    }

    memset(&m_pCommEvent->comm, 0, sizeof(m_pCommEvent->comm));

    size_t size = sizeof(m_pCommEvent->comm.comm);
    osProcessId processId = pid;
    pid_t tgid = 0;

    if (osGetProcessIdentificationInfo(processId, NULL, &tgid, m_pCommEvent->comm.comm, &size))
    {
        m_pCommEvent->comm.pid = tgid;
        m_pCommEvent->comm.header.type = PERF_RECORD_COMM;
        size = ALIGN_FOR_SIZE_T(size, sizeof(gtUInt64));
        memset(m_pCommEvent->comm.comm + size, 0, sampleRecSize);
        m_pCommEvent->comm.header.size = sizeof(m_pCommEvent->comm) - (sizeof(m_pCommEvent->comm.comm) - size) + sampleRecSize;

        // FIXME:
#if 0

        if (!full)
        {
            m_pCommEvent->comm.tid = pid;

            writePMUSampleData(m_pCommEvent, m_pCommEvent->header.size);
            fclose(fp);
            return tgid;
        }

#endif // 0

        // For each thread of the process
        osProcessThreadsEnumerator threadEnum;

        if (threadEnum.initialize(pid))
        {
            gtUInt32 threadId;

            while (threadEnum.next(threadId))
            {
                m_pCommEvent->comm.tid = threadId;
                writePMUSampleData(m_pCommEvent, m_pCommEvent->header.size);
            }
        }
        else
        {
            tgid = 0;
        }
    }

    return tgid;
}


//
// Write the MMAP records for Kernel modules to attribute samples to kernel modules
//

struct kernelSymbolInfo
{
    char*        name;
    gtUByte     type;
    gtUInt64    startAddress;
    gtUInt64    endAddress;
};



// Length of the Kernenl symbol name in /proc/kallsyms
#define CA_KERNEL_SYMBOL_LEN    128

// the type can be function or variable
#define KERNEL_FUNCTION     0x1
#define KERNEL_VARIABLE     0x2

HRESULT CaPerfDataWriter::findKernelSymbol(const char* symbol, gtUByte type, gtUInt64* startAddress)
{
    (void)(symbol); // unused
    const char* filename = m_kernelSymbolFile.c_str();
    FILE*    file;
    char*    line = NULL;
    size_t  n;

    if (!filename || !startAddress)
    {
        return E_INVALIDARG;
    }

    if ((file = fopen(filename, "r")) == NULL)
    {
        return E_FAIL;
    }

    HRESULT ret = S_OK;

    // read the kernel symbol file /proc/kallsyms
    while (!feof(file))
    {
        gtUInt64  start;
        int   lineLen, len;
        char      symbolType;

        lineLen = getline(&line, &n, file);

        if (lineLen < 0 || !line)
        {
            break;
        }

        line[--lineLen] = '\0'; /* \n */

        len = hex2u64(line, &start);

        len++;

        if (len + 2 >= lineLen)
        {
            continue;
        }

        symbolType = toupper(line[len]);
        len += 2;
        len = lineLen - len;

        if (len >= CA_KERNEL_SYMBOL_LEN)
        {
            ret = E_FAIL;
            break;
        }

        // check for the symbol type
        if (KERNEL_FUNCTION == type)
        {
            if (symbolType == 'T' || symbolType == 'W')
            {
                *startAddress = start;
                break;
            }
        }
        else if (KERNEL_VARIABLE == type)
        {
            if (symbolType == 'D' || symbolType == 'd')
            {
                *startAddress = start;
                break;
            }
        }
        else
        {
            ret = E_FAIL;
            break;
        }

    }

    free(line);
    fclose(file);
    return ret;
}

HRESULT CaPerfDataWriter::getKernelVersion()
{
    char version[OS_MAX_PATH];
    FILE* file;
    char* name, *tmp;
    const char* prefix = "Linux version ";

    sprintf(version, "%s/proc/version", m_rootDir.c_str());
    file = fopen(version, "r");

    if (!file)
    {
        return E_FAIL;
    }

    version[0] = '\0';
    tmp = fgets(version, sizeof(version), file);
    fclose(file);

    name = strstr(version, prefix);

    if (!name)
    {
        return E_FAIL;
    }

    name += strlen(prefix);
    tmp = strchr(name, ' ');

    if (tmp)
    {
        *tmp = '\0';
    }

    m_kVersion = name;
    // return strdup(name);

    return S_OK;
}


HRESULT CaPerfDataWriter::setModulesAbsolutePath(const std::string& dirName)
{
    struct dirent* dent;
    const char* dirPath = dirName.c_str();
    DIR* dir = opendir(dirPath);
    HRESULT ret = S_OK;

    if (!dir)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"cannot open %hs dir", dirPath);
        return E_FAIL;
    }

    while ((dent = readdir(dir)) != NULL)
    {
        char path[OS_MAX_PATH];
        struct stat st;

        /*sshfs might return bad dent->d_type, so we have to stat*/
        sprintf(path, "%s/%s", dirPath, dent->d_name);

        if (stat(path, &st))
        {
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, ".."))
            {
                continue;
            }

            snprintf(path, sizeof(path), "%s/%s",
                     dirPath, dent->d_name);
            ret = setModulesAbsolutePath(path);

            if (ret < 0)
            {
                break;
            }
        }
        else
        {
            char* dot = strrchr(dent->d_name, '.');
            char  dso_name[OS_MAX_PATH];

            if (dot == NULL || strcmp(dot, ".ko"))
            {
                continue;
            }

            snprintf(dso_name, sizeof(dso_name), "[%.*s]", (int)(dot - dent->d_name), dent->d_name);

            strxfrchar(dso_name, '-', '_');

            // map = map_groups__find_by_name(self, MAP__FUNCTION, dso_name);
            // if (map == NULL)
            //  continue;
            kModuleNameMap::iterator it = m_kModuleNameMap.find(dso_name);

            if (it == m_kModuleNameMap.end())
            {
                continue;
            }

            snprintf(path, sizeof(path), "%s/%s", dirPath, dent->d_name);
            it->second.addAbsPath(path);
#if 0
            long_name = strdup(path);

            if (long_name == NULL)
            {
                ret = -1;
            }
            else
            {
                dso__set_long_name(map->dso, long_name);
                map->dso->lname_alloc = 1;
                dso__kernel_module_get_build_id(map->dso, "");
            }

#endif // 0
        }
    }

    // we are done dir
    closedir(dir);
    return ret;
}

HRESULT CaPerfDataWriter::setModulesPath()
{
    HRESULT ret = S_OK;
    char modules_path[OS_MAX_PATH];

    getKernelVersion();

    if (m_kVersion.empty())
    {
        return E_FAIL;
    }

    snprintf(modules_path, sizeof(modules_path), "%s/lib/modules/%s/kernel", m_rootDir.c_str(), m_kVersion.c_str());

    m_kModulesPath = modules_path;

    setModulesAbsolutePath(m_kModulesPath);

    if (! m_kModuleNameMap.size())
    {
        return E_FAIL;
    }

    // Now iterate over the map and to m_kModuleAddrMap
    for (kModuleNameMap::iterator it = m_kModuleNameMap.begin(), itEnd = m_kModuleNameMap.end(); it != itEnd; ++it)
    {
        m_kModuleAddrMap.insert(kModuleAddrMap::value_type(it->second.m_startAddress, it->second));
    }

    if (! m_kModuleAddrMap.size())
    {
        return E_FAIL;
    }

    // Now set the end address
    kModuleAddrMap::iterator aiter = m_kModuleAddrMap.begin(), aiterEnd = m_kModuleAddrMap.end();

    kModule* prev = &(aiter->second);

    gtUInt64 endAddr = ~0ULL;

    while (++aiter != aiterEnd)
    {
        kModule* curr = &(aiter->second);

        endAddr = curr->m_startAddress - 1;
        prev->setEndAddress(endAddr);

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_EXTENSIVE, L"addr(0x%lx , 0x%lx)", prev->m_startAddress, prev->m_endAddress);
        prev = curr;
    }

    // set endAddress for last entry
    if (prev)
    {
        endAddr = ~0ULL;
        prev->setEndAddress(endAddr);
    }

    return ret;
}

HRESULT CaPerfDataWriter::readKernelModules()
{
    char* line = NULL;
    size_t n;
    FILE* file;
    const char* modules = NULL;
    char path[OS_MAX_PATH];

    if (! m_guestOs)
    {
        // sprintf(path, "%s/proc/modules", self->root_dir);
        sprintf(path, "/proc/modules");
        modules = path;
    }
    else
    {
#if 0
        // TODO:
        // modules = symbol_conf.default_guest_modules;
#else
        return E_FAIL;
#endif
    }

    file = fopen(modules, "r");

    if (file == NULL)
    {
        return E_FAIL;
    }

    while (!feof(file))
    {
        // char      name[OS_MAX_PATH];
        // gtUInt64  start;
        char*      sep;
        int       line_len;
        struct kModule module;

        line_len = getline(&line, &n, file);

        if (line_len < 0)
        {
            break;
        }

        if (!line)
        {
            return E_FAIL;
        }

        line[--line_len] = '\0'; /* \n */

        sep = strrchr(line, 'x');

        if (sep == NULL)
        {
            continue;
        }

        hex2u64(sep + 1, &module.m_startAddress);

        sep = strchr(line, ' ');

        if (sep == NULL)
        {
            continue;
        }

        *sep = '\0';

        snprintf(module.m_modName, sizeof(module.m_modName), "[%s]", line);
        m_kModuleNameMap.insert(kModuleNameMap::value_type(module.m_modName, module));

        // TODO: build id stuff ??
        // dso__kernel_module_get_build_id(map->dso, self->root_dir);
    }

    free(line);
    fclose(file);

    // set the modules path
    setModulesPath();

    return S_OK;
}


HRESULT CaPerfDataWriter::writeKernelMmap(const char* symbolName, gtUInt16 sampleRecSize)
{
    char        filename[OS_MAX_PATH];
    ca_event_t*  event = NULL;
    bool        isHost = true; // TODO: identify host or guest
    HRESULT     ret = S_OK;

    event = (ca_event_t*)calloc(1, sizeof(event->mmap) + sampleRecSize);

    if (event == NULL)
    {
        return E_OUTOFMEMORY;
    }

    //  PERF uses PERF_RECORD_MISC_KERNEL for kernel mmaps
    //  and PERF_RECORD_MISC_USER for user mmaps
    //
    if (isHost)
    {
        event->header.misc = PERF_RECORD_MISC_KERNEL;
        snprintf(filename, sizeof(filename), "/proc/kallsyms");
        m_kernelSymbolFile = filename;
    }
    else
    {
        // Guest Kernel
#if defined(LINUX_PERF_GUEST_KERNEL_SUPPORT)
        event->header.misc = PERF_RECORD_MISC_GUEST_KERNEL;
#else
        event->header.misc = PERF_RECORD_MISC_KERNEL;
#endif

        // TODO: we need to construct the path ?
        snprintf(filename, sizeof(filename), "/proc/kallsyms");
        // m_kernelSymbolFile = filename;
    }

    gtUInt64 startAddress;
    ret = findKernelSymbol(symbolName, KERNEL_FUNCTION, &startAddress);

    if (ret == E_FAIL)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Failed to find kernel symbol %hs", symbolName);
        free(event);
        return ret;
    }

    // FIXME - map;
    // map = machine->vmlinux_maps[MAP__FUNCTION];

    // FIXME - mmap_name
    // mmap_name = machine__mmap_name(machine, name_buff, sizeof(name_buff));
    // size = snprintf(event->mmap.filename, sizeof(event->mmap.filename),
    //      "%s%s", mmap_name, symbol_name) + 1;

    // TODO: Perf looks for a symbol "

    size_t size;
    size = snprintf(event->mmap.filename, sizeof(event->mmap.filename),
                    "[%s]%s", "kernel.kallsyms", symbolName) + 1;
    size = ALIGN_FOR_SIZE_T(size, sizeof(gtUInt64));
    event->mmap.header.type = PERF_RECORD_MMAP;
    event->mmap.header.size = (sizeof(event->mmap) -
                               (sizeof(event->mmap.filename) - size) + sampleRecSize);
    event->mmap.pid   = -1; // machine->pid;
    event->mmap.tid   = 0;
    event->mmap.start = 0; // map->start;    FIXME
    event->mmap.len   = 0xffffffff9fffffffULL; // map->end - event->mmap.start; FIXME
    event->mmap.pgoff = startAddress;

    writePMUSampleData(event, event->header.size);

    free(event);
    return ret;
}

HRESULT CaPerfDataWriter::writeKernelModules(gtUInt16 sampleRecSize)
{
    HRESULT ret = S_OK;

    ret = readKernelModules();

    if ((int)E_FAIL == ret)
    {
        OS_OUTPUT_DEBUG_LOG(L"reading kernel modules failed", OS_DEBUG_LOG_ERROR);
        return ret;
    }

    ca_event_t* event = (ca_event_t*)calloc(1, sizeof(event->mmap) + sampleRecSize);

    if (event == NULL)
    {
        OS_OUTPUT_DEBUG_LOG(L"allocation Failed", OS_DEBUG_LOG_ERROR);
        return E_FAIL;
    }

    event->header.type = PERF_RECORD_MMAP;

    // kernel uses 0 for user space maps, see kernel/perf_event.c
    // __perf_event_mmap
#if defined(LINUX_PERF_GUEST_KERNEL_SUPPORT)
    event->header.misc = (! m_guestOs) ? PERF_RECORD_MISC_KERNEL : PERF_RECORD_MISC_GUEST_KERNEL;
#else
    event->header.misc = PERF_RECORD_MISC_KERNEL;
#endif

    kModuleAddrMap::iterator aiter = m_kModuleAddrMap.begin();

    for (; aiter != m_kModuleAddrMap.end(); aiter++)
    {
        size_t size;

        size = ALIGN_FOR_SIZE_T(aiter->second.m_absPath.length() + 1,
                                sizeof(gtUInt64));
        event->mmap.header.type = PERF_RECORD_MMAP;
        event->mmap.header.size = (sizeof(event->mmap) -
                                   (sizeof(event->mmap.filename) - size));
        memset(event->mmap.filename + size, 0, sampleRecSize);
        event->mmap.header.size += sampleRecSize;
        event->mmap.start = aiter->second.m_startAddress;
        event->mmap.len   = aiter->second.m_endAddress -
                            aiter->second.m_startAddress;
        event->mmap.pid   = -1;

        memcpy(event->mmap.filename, aiter->second.m_absPath.c_str(),
               aiter->second.m_absPath.length() + 1);

        writePMUSampleData(event, event->header.size);
    }

    free(event);
    return ret;
}

HRESULT CaPerfDataWriter::writeKernelInfo(gtUInt64 sampleRecSize)
{
    HRESULT ret;

    ret = writeKernelMmap("_text", sampleRecSize);

    if (E_FAIL == ret)
    {
        OS_OUTPUT_DEBUG_LOG(L"Kernel Mmap for _text failed", OS_DEBUG_LOG_ERROR);
        ret = writeKernelMmap("_stext", sampleRecSize);
    }

    if (E_FAIL == ret)
    {
        OS_OUTPUT_DEBUG_LOG(L"Kernel Mmap for _stext failed", OS_DEBUG_LOG_ERROR);
        return ret;
    }

    ret = writeKernelModules(sampleRecSize);

    if (ret)
    {
        ret = writeGuestOsInfo();
    }

    return ret;
}


HRESULT CaPerfDataWriter::writeTargetPids(gtUInt32 numPids, pid_t* pPids)
{
    HRESULT ret = E_FAIL;

    if (!pPids || 0 == numPids)
    {
        return ret;
    }

    m_offset = lseek(m_fd, 0, SEEK_END);
    ssize_t sectionStOffset = m_offset;

    ssize_t rc = write(m_fd, (const void*)pPids, sizeof(pid_t) * numPids);
    GT_ASSERT(rc != -1);

    m_offset = lseek(m_fd, 0, SEEK_CUR);

    // update the section header entry for CAPERF_SECTION_TARGET_PIDS
    updateSectionHdr(CAPERF_SECTION_TARGET_PIDS,
                     sectionStOffset,
                     (m_offset - sectionStOffset));

    ret = S_OK;
    return ret;
}

HRESULT
CaPerfDataWriter::writeFakeTimerInfo(caperf_section_fake_timer_t* pInfo)
{
    HRESULT ret = E_FAIL;

    if ((!pInfo)
        || (0 == pInfo->numCpu)
        || (0 == pInfo->timerNanosec)
        || (NULL == pInfo->timerFds)
        || (NULL == pInfo->fakeTimerFds))
    {
        return ret;
    }

    m_offset = lseek(m_fd, 0, SEEK_END);
    ssize_t sectionStOffset = m_offset;

    // Following the structure of caperf_section_fake_timer
    ssize_t rc = write(m_fd, (const void*) &pInfo->numCpu, sizeof(gtUInt32));
    GT_ASSERT(rc != -1);
    rc = write(m_fd, (const void*) &pInfo->timerNanosec, sizeof(gtUInt32));
    GT_ASSERT(rc != -1);
    rc = write(m_fd, (const void*) pInfo->timerFds, sizeof(gtUInt32) * pInfo->numCpu);
    GT_ASSERT(rc != -1);
    rc = write(m_fd, (const void*) pInfo->fakeTimerFds, sizeof(gtUInt32) * pInfo->numCpu);
    GT_ASSERT(rc != -1);

    m_offset = lseek(m_fd, 0, SEEK_CUR);

    // update the section header entry
    updateSectionHdr(CAPERF_SECTION_FAKE_TIMER_INFO, sectionStOffset, (m_offset - sectionStOffset));

    ret = S_OK;
    return ret;
}
