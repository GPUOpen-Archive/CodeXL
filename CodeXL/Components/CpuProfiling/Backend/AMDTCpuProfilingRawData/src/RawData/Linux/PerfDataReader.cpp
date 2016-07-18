//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfDataReader.cpp
///
//==================================================================================

// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingRawData/src/RawData/Linux/PerfDataReader.cpp#7 $
// Last checkin:   $DateTime: 2016/04/14 02:12:02 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569056 $
//=====================================================================

#include <Linux/PerfDataReader.h>
#include <Linux/PerfData.h>
#include <AMDTCpuProfilingBackendUtils/3rdParty/linux/perfStruct.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

extern const char* _ca_perfRecNames[];

PerfDataReader::PerfDataReader()
{
    clear();
}


PerfDataReader::~PerfDataReader()
{
    deinit();
}


void PerfDataReader::clear()
{
    m_fd = -1;
    m_offset = 0;
    m_numCpus = 0;
    m_numAttrs = 0;
    m_numEventTypes = 0;
    m_sampleType = 0;
    m_curBufSz = 0;
    m_pBuf = NULL;
    m_dataStartOffset = 0;
    m_dataSize = 0;

    m_pPerfHdr = NULL;
    m_perfEventAttrVec.clear();
    m_perfEvtIdToEvtTypeMap.clear();
    m_perfFileSecVec.clear();
    m_perfEventTypeVec.clear();

    m_isEof = false;
}


HRESULT PerfDataReader::init(const std::string& perfDataPath)
{
    int retVal = S_OK;

    // Open the file
    if ((m_fd = open(perfDataPath.c_str(), O_RDONLY)) < 0)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Failed to open %hs", perfDataPath.c_str());
        return E_INVALIDPATH;
    }

    // Get the Perf header
    m_pPerfHdr = (struct perf_file_header*) mmap(NULL, sizeof(struct perf_file_header), PROT_READ, MAP_PRIVATE, m_fd, 0);

    if (!m_pPerfHdr)
    {
        return E_FAIL;
    }

    if (m_pPerfHdr->magic != PERF_MAGIC)
    {
        return E_INVALIDDATA;
    }

    m_dataStartOffset = m_pPerfHdr->data.offset;
    m_dataSize = m_pPerfHdr->data.size;


    if ((retVal = _processPerfAttributesSection()) != S_OK)
    {
        return retVal;
    }

    if ((retVal = _processPerfFileSection()) != S_OK)
    {
        return retVal;
    }

    if ((retVal = _processPerfEventTypesSection()) != S_OK)
    {
        return retVal;
    }

    // Set the default type values for IBS Fetch and Op
    m_ibsFetchType = 6;
    m_ibsFetchType = 7;

    return retVal;
}


void PerfDataReader::deinit()
{
    if (m_fd != -1)
    {
        close(m_fd);
        m_fd = -1;
    }

    if (m_pBuf)
    {
        free(m_pBuf);
        m_pBuf = NULL;
    }

    // clear the internal structures
    clear();
}


void PerfDataReader::dumpPerfHeader()
{
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"-------- PERF HEADER (size: %lu) --------", sizeof(struct perf_file_header));

    char* pMagic = (char*) & (m_pPerfHdr->magic);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"magic     : %c%c%c%c%c%c%c%c",
                               pMagic[0], pMagic[1], pMagic[2], pMagic[3],
                               pMagic[4], pMagic[5], pMagic[6], pMagic[7]);

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"size      : %llu", m_pPerfHdr->size);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"attr_size : %llu", m_pPerfHdr->attr_size);

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"section attrs (offset)       : 0x%llx", m_pPerfHdr->attrs.offset);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"section attrs (size)         : %llu", m_pPerfHdr->attrs.size);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"section event_types (offset) : 0x%llx", m_pPerfHdr->event_types.offset);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"section event_types (size)   : %llu", m_pPerfHdr->event_types.size);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"section data  (offset)       : 0x%llx", m_pPerfHdr->data.offset);
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"section data  (size)         : %llu", m_pPerfHdr->data.size);

    // [Suravee] Don't really know what these are for :(
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"adds_features (%u bits):", HEADER_FEAT_BITS);

    for (size_t i = 0 ; i < BITS_TO_LONGS(HEADER_FEAT_BITS); i++)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"0x%016lx\n", m_pPerfHdr->adds_features[i]);
    }

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"---- perf_file_section (%u section) ----", m_perfFileSecVec.size());
    PerfEvtIdToEvtTypeMap::iterator it, itEnd;
    it = m_perfEvtIdToEvtTypeMap.begin();
    itEnd = m_perfEvtIdToEvtTypeMap.end();

    for (; it != itEnd; it++)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"evId = 0x%llx -> evType = %u", it->first, it->second);
    }

}


int PerfDataReader::_processPerfFileSection()
{
    int retVal = S_OK;
    ssize_t rdSz = 0;

    for (size_t i = 0 ; i < m_perfFileSecVec.size(); i++)
    {
        // Setup fd to the attribute Section
        lseek(m_fd, 0, SEEK_SET);

        if (lseek(m_fd, m_perfFileSecVec[i].offset, SEEK_SET) < 0)
        {
            return E_UNEXPECTED;
        }

        // NOTE: We assume that each id is per-core-per-event
        //       and all perfFileSec has the same number of CPUs.
        m_numCpus = m_perfFileSecVec[i].size / sizeof(gtUInt64);

        for (size_t j = 0; j < m_numCpus; j++)
        {
            gtUInt64 id = 0;
            rdSz = read(m_fd, &id, sizeof(gtUInt64));

            if (rdSz == 0)
            {
                // EOF
                break;
            }
            else if (rdSz < 0 || rdSz != sizeof(gtUInt64))
            {
                retVal = E_UNEXPECTED;
                break;
            }

            gtUInt64 data = j;
            data = (data << 32) | i;

            m_perfEvtIdToEvtTypeMap.insert(
                PerfEvtIdToEvtTypeMap::value_type(id, data));
        }
    }

    return retVal;
}


int PerfDataReader::_processPerfAttributesSection()
{
    int retVal = S_OK;
    ssize_t rdSz = 0;

    if (!isOpen())
    {
        return E_NOFILE;
    }

    m_numAttrs = m_pPerfHdr->attrs.size / m_pPerfHdr->attr_size;

    // Setup fd to the attribute Section
    lseek(m_fd, 0, SEEK_SET);

    if (lseek(m_fd, m_pPerfHdr->attrs.offset, SEEK_SET) < 0)
    {
        return E_UNEXPECTED;
    }

    struct perf_event_attr* pAttr = (struct perf_event_attr*) malloc(m_pPerfHdr->attr_size);

    do
    {
        /* NOTE:
         * In header.c, function perf_session__write_header()
         * writes out the event id per core per event.
         * The data structure perf_file_section
         * point to this location for each event.
         */

        // Reading perf_event_attr
        rdSz = read(m_fd, pAttr, m_pPerfHdr->attr_size);

        if (rdSz == 0)
        {
            // EOF
            break;
        }
        else if (rdSz < 0 || rdSz != (ssize_t)m_pPerfHdr->attr_size)
        {
            retVal = E_UNEXPECTED;
            break;
        }

        m_sampleType = pAttr->sample_type;

        m_perfEventAttrVec.push_back(*pAttr);

        // This is for the struct perf_file_section
        struct perf_file_section* pFileSec =
            (struct perf_file_section*)(((char*) pAttr) + sizeof(struct perf_event_attr));
        m_perfFileSecVec.push_back(*pFileSec);

    }
    while (m_perfEventAttrVec.size() < m_numAttrs);

    free(pAttr);
    return retVal;
}


void PerfDataReader::dumpPerfAttributesSection()
{
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO,
                               L"-------- PERF Attributes Section (offset: 0x%lx, size: %lu, num of attrs: %u) --------",
                               m_pPerfHdr->attrs.offset, m_pPerfHdr->attrs.size, m_numAttrs);

    for (size_t i = 0 ; i < m_perfEventAttrVec.size(); i++)
    {
        struct perf_event_attr* pAttr = &m_perfEventAttrVec[i];
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"---- ATTR Index: %u ----", i);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"type             : 0x%x", pAttr->type);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"size             : 0x%x", pAttr->size);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"config           : 0x%llx", pAttr->config);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"sample_period    : %llu", pAttr->sample_period);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"sample_freq      : %llu", pAttr->sample_freq);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"sample_type      : 0x%llx", pAttr->sample_type);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"read_format      : 0x%llx", pAttr->read_format);
        OS_OUTPUT_DEBUG_LOG(L"-- begin bits --", OS_DEBUG_LOG_INFO);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    disabled         : %x", pAttr->disabled);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    inherit          : %x", pAttr->inherit);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    pinned           : %x", pAttr->pinned);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    exclusive        : %x", pAttr->exclusive);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    exclude_user     : %x", pAttr->exclude_user);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    exclude_kernel   : %x", pAttr->exclude_kernel);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    exclude_hv       : %x", pAttr->exclude_hv);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    exclude_idle     : %x", pAttr->exclude_idle);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    mmap             : %x", pAttr->mmap);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    comm             : %x", pAttr->comm);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    freq             : %x", pAttr->freq);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    inherit_stat     : %x", pAttr->inherit_stat);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    enable_on_exec   : %x", pAttr->enable_on_exec);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    task             : %x", pAttr->task);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    watermark        : %x", pAttr->watermark);
#if defined(LINUX_PERF_PRECISE_IP_SUPPORT)
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    precise_ip       : %x", pAttr->precise_ip);
#endif
#if defined(LINUX_PERF_MMAP_DATA_SUPPORT)
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    mmap_data        : %x", pAttr->mmap_data);
#endif
#if defined(LINUX_PERF_SAMPLE_ID_ALL_SUPPORT)
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    sample_id_all    : %x", pAttr->sample_id_all);
#endif

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"    __reserved_1     : %lx", pAttr->__reserved_1);
        OS_OUTPUT_DEBUG_LOG(L"-- end bits --", OS_DEBUG_LOG_INFO);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"wakeup_events    : %x", pAttr->wakeup_events);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"wakeup_watermark : %x", pAttr->wakeup_watermark);
#if defined(LINUX_PERF_BREAKPOINT_SUPPORT)
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"bp_type          : %x", pAttr->bp_type);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"bp_addr          : %llx", pAttr->bp_addr);
#endif
#if defined(LINUX_PERF_EXT_CONFIG_SUPPORT)
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"config1          : %llx", pAttr->config1);
#endif
#if defined(LINUX_PERF_BREAKPOINT_SUPPORT)
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"bp_len           : %llx", pAttr->bp_len);
#endif
#if defined(LINUX_PERF_EXT_CONFIG_SUPPORT)
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"config2          : %llx", pAttr->config2);
#endif

        struct perf_file_section* pFileSec = &m_perfFileSecVec[i];
        OS_OUTPUT_DEBUG_LOG(L"-- struct perf_file_sction --", OS_DEBUG_LOG_INFO);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"offset           : 0x%llx", pFileSec->offset);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"size             : 0x%llx", pFileSec->size);
    }
}


int PerfDataReader::_processPerfEventTypesSection()
{
    int retVal = S_OK;
    ssize_t rdSz = 0;
    int perfEvTypeSize = sizeof(struct perf_trace_event_type);

    if (!isOpen())
    {
        return E_NOFILE;
    }

    m_numEventTypes = m_pPerfHdr->event_types.size / perfEvTypeSize;

    // Setup fd to the attribute Section
    lseek(m_fd, 0, SEEK_SET);

    if (lseek(m_fd, m_pPerfHdr->event_types.offset, SEEK_SET) < 0)
    {
        return E_UNEXPECTED;
    }

    struct perf_trace_event_type* pEvType = (struct perf_trace_event_type*) malloc(perfEvTypeSize);

    do
    {
        rdSz = read(m_fd, pEvType, perfEvTypeSize);

        if (rdSz == 0)
        {
            // EOF
            break;
        }
        else if (rdSz < 0 || rdSz != perfEvTypeSize)
        {
            retVal = E_UNEXPECTED;
            break;
        }

        m_perfEventTypeVec.push_back(*pEvType);

    }
    while (m_perfEventTypeVec.size() < m_numEventTypes);

    free(pEvType);
    return retVal;
}


HRESULT PerfDataReader::getFirstRecord(struct perf_event_header* pHdr, const void** ppBuf, gtUInt32* pOffset)
{
    // Go to the offset specified by m_dataStartOffset
    return getRecordFromOffset(m_dataStartOffset, pHdr, ppBuf, pOffset);
}


HRESULT PerfDataReader::getRecordFromOffset(gtUInt32 offset,
                                            struct perf_event_header* pHdr,
                                            const void** ppBuf,
                                            gtUInt32* pOffset)
{
    // Goto specified offset
    if ((m_offset = lseek(m_fd, offset, SEEK_SET)) < 0)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error: Invalid offset 0x%lx", offset);
        return E_UNEXPECTED;
    }

    return getNextRecord(pHdr, ppBuf, pOffset);
}


HRESULT PerfDataReader::getNextRecord(
    struct perf_event_header* pHdr,
    const void** ppBuf,
    gtUInt32* pOffset)
{
    HRESULT retVal = S_OK;
    ssize_t rdSz = 0;

    // Sanity check
    if (NULL == pHdr)
    {
        return E_INVALIDARG;
    }

    if (NULL != pOffset)
    {
        *pOffset = m_offset;
    }

    // Read the header
    rdSz = read(m_fd, pHdr, sizeof(struct perf_event_header));

    if (rdSz != sizeof(struct perf_event_header))
    {
        // check for end-of-file
        m_isEof = (rdSz == 0) ? true : false;

        return E_UNEXPECTED;
    }

    // Sanity check
    if (pHdr->size <= 0)
    {
        return E_INVALIDDATA;
    }

    // Adjust payload size
    if (pHdr->size > m_curBufSz)
    {
        m_pBuf = realloc(m_pBuf, pHdr->size + 10); // add buffer zone

        if (!m_pBuf)
        {
            return E_OUTOFMEMORY;
        }

        m_curBufSz = pHdr->size;
    }

    // Read the payload
    ssize_t sz = pHdr->size - sizeof(struct perf_event_header);

    rdSz = read(m_fd, m_pBuf, sz);

    if (rdSz != sz)
    {
        return E_UNEXPECTED;
    }

    char* tmp = (char*) m_pBuf;
    tmp[pHdr->size] = '\0';

    m_offset += rdSz + sizeof(struct perf_event_header);

    if (retVal == S_OK)
    {
        *ppBuf = m_pBuf;
    }

    return retVal;
}


void PerfDataReader::dumpPerfEventTypesSection()
{
    struct perf_trace_event_type* pEvType = NULL;

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO,
                               L"-------- PERF Event Types Section (offset: 0x%lx, size: %lu, num of attrs: %u) --------",
                               m_pPerfHdr->event_types.offset, m_pPerfHdr->event_types.size, m_numEventTypes);

    for (size_t i = 0 ; i < m_perfEventTypeVec.size(); i++)
    {
        pEvType = &m_perfEventTypeVec[i];
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"---- EVENT Index: %u ----", i);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"event_id : 0x%lx", pEvType->event_id);
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"name     : %hs", pEvType->name);
    }
}


int PerfDataReader::dumpPerfData()
{
    if (!isOpen())
    {
        return E_NOFILE;
    }

    lseek(m_fd, 0, SEEK_SET);

    // Move to the data Section
    if (lseek(m_fd, m_pPerfHdr->data.offset, SEEK_SET) < 0)
    {
        return E_UNEXPECTED;
    }

    m_offset = m_pPerfHdr->data.offset;

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"-------- PERF DATA SECTION (offset:0x%lx, size:%lu)--------",
                               m_pPerfHdr->data.offset, m_pPerfHdr->data.size);
    int retVal = S_OK;
    struct perf_event_header hdr;
    void* pBuf = NULL;
    int curBufSz = 0;
    ssize_t rdSz = 0;
    unsigned int recCnt = 0;

    while ((rdSz = read(m_fd, &hdr, sizeof(struct perf_event_header))) > 0)
    {
        // Adjust payload size
        if (hdr.size > curBufSz)
        {
            void* pNewBuf = realloc(pBuf, hdr.size);

            if (NULL == pNewBuf)
            {
                retVal = E_OUTOFMEMORY;
                break;
            }

            pBuf = pNewBuf;
            curBufSz = hdr.size;
        }

        // Read the payload
        rdSz = read(m_fd, pBuf, hdr.size - sizeof(struct perf_event_header));

        if (rdSz == 0)
        {
            // EOF
            break;
        }
        else if (rdSz < 0 || rdSz != (ssize_t)(hdr.size - sizeof(struct perf_event_header)))
        {
            retVal = E_UNEXPECTED;
            break;
        }

        dump_PERF_RECORD(&hdr, pBuf, recCnt);
        m_offset += rdSz + sizeof(struct perf_event_header);
        recCnt++;
    }

    if (NULL != pBuf)
    {
        free(pBuf);
    }

    return retVal;
}


HRESULT PerfDataReader::dump_PERF_RECORD(struct perf_event_header* pHdr, void* ptr, int recNum)
{
    (void)(ptr); // unused
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"[rec#:%5d, offet:0x%05x size:%3d] PERF_RECORD_%hs",
                               recNum, m_offset, pHdr->size, _ca_perfRecNames[pHdr->type]);

    return S_OK;
}


HRESULT PerfDataReader::getEventAndCpuFromSampleId(gtUInt64 evId,
                                                   int* pCpu,
                                                   gtUInt32* pEvent,
                                                   gtUInt32* pUmask,
                                                   gtUInt32* pOs,
                                                   gtUInt32* pUsr)
{
    if (!pCpu || !pEvent)
    {
        return E_FAIL;
    }

    PerfEvtIdToEvtTypeMap::iterator it = m_perfEvtIdToEvtTypeMap.find(evId);

    if (it == m_perfEvtIdToEvtTypeMap.end())
    {
        return E_FAIL;
    }

    unsigned int evIndx = it->second & 0xFFFFFFFF;

    *pCpu = (it->second >> 32) & 0xFFFFFFFF;

    if (S_OK != getEventInfoFromEvTypeAndConfig(
            m_perfEventAttrVec[evIndx].type,
            m_perfEventAttrVec[evIndx].config,
            pEvent,
            pUmask))
    {
        return E_FAIL;
    }


    if (pOs)
    {
        *pOs = !m_perfEventAttrVec[evIndx].exclude_kernel;
    }

    if (pUsr)
    {
        *pUsr = !m_perfEventAttrVec[evIndx].exclude_user;
    }

    return S_OK;
}


HRESULT PerfDataReader::getEventInfoFromEvTypeAndConfig(gtUInt32 evType, gtUInt64 evCfg, gtUInt32* pEvent, gtUInt32* pUmask)
{
    HRESULT ret = E_FAIL;

    switch (evType)
    {
        case PERF_TYPE_RAW:
            ret = _getRawEventInfo(evCfg, pEvent, pUmask);
            break;

        case PERF_TYPE_HARDWARE:
        case PERF_TYPE_SOFTWARE:
        case PERF_TYPE_TRACEPOINT:
        case PERF_TYPE_HW_CACHE:
#if defined(LINUX_PERF_BREAKPOINT_SUPPORT)
        case PERF_TYPE_BREAKPOINT:
#endif
            ret = _getPerfEventInfo(evType, evCfg, pEvent);
            *pUmask = 0;
            break;

        default:
            break;
    }

    // Handle dynamic PMU types for IBS profile
    if (evType == m_ibsFetchType)
    {
        *pEvent = 0xF000;
        *pUmask = 0;
        ret = S_OK;
    }
    else if (evType == m_ibsOpType)
    {
        *pEvent = 0xF100;
        *pUmask = 0;
        ret = S_OK;
    }

    return ret;
}


HRESULT PerfDataReader::_getPerfEventInfo(gtUInt32 evType, gtUInt64 evCfg, gtUInt32* pEvent)
{
    if (!pEvent)
    {
        return E_FAIL;
    }

    // For Perf-events, we use the range 0xE000-0xEFFF.
    // Also, we use evType as part of the encoding.
    // This encoding will allow upto 256 events per type.
    *pEvent = CA_PERF_EV_BASE | (evType << 8) | evCfg;

    return S_OK;
}


HRESULT PerfDataReader::_getRawEventInfo(gtUInt64 evCfg, gtUInt32* pEvent, gtUInt32* pUmask)
{
    if (!pEvent)
    {
        return E_FAIL;
    }

    // NOTE: evCfg = ((umask & 0xff) << 8 | (evId & ff) | ((evId & 0xff00) << 24);
    *pEvent = (gtUInt32)(((evCfg & 0xff00000000ULL) >> 24) | (evCfg & 0xff));

    if (pUmask)
    {
        *pUmask = (evCfg >> 8) & 0xff;
    }

    return S_OK;
}


void PerfDataReader::dumpHeaderSections()
{
    dumpPerfHeader();
    dumpPerfAttributesSection();
    dumpPerfEventTypesSection();
}


void PerfDataReader::dumpData()
{
    dumpPerfData();
}


#if defined(__i386__) && defined(__PIC__)
/* %ebx may be the PIC register.  */
#define __cpuid(level, a, b, c, d)                      \
    __asm__ ("xchgl\t%%ebx, %1\n\t"                       \
             "cpuid\n\t"                                  \
             "xchgl\t%%ebx, %1\n\t"                       \
             : "=a" (a), "=r" (b), "=c" (c), "=d" (d)     \
             : "0" (level))
#else
#define __cpuid(level, a, b, c, d)                      \
    __asm__ ("cpuid\n\t"                                  \
             : "=a" (a), "=b" (b), "=c" (c), "=d" (d)     \
             : "0" (level))
#endif

HRESULT PerfDataReader::getCpuInfo(unsigned int* pFamily, unsigned int* pModel, unsigned int* pStepping)
{
    if (!pFamily)
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

    unsigned ebx, ecx, edx;

    /* CPUID Fn0000_0001_EAX Family, Model, Stepping */
    //        v.eax = 0x00000001;
    //        asm("cpuid" : "=a" (v.eax)
    //      : "0" (1)
    //      : "ecx","ebx","edx");

    __cpuid(1, v.eax, ebx, ecx, edx);

    if (pFamily)
    {
        *pFamily   = v.family + v.ext_family;
    }

    if (pModel)
    {
        *pModel    = v.model + (v.ext_model << 4);
    }

    if (pStepping)
    {
        *pStepping = v.stepping;
    }

    return S_OK;
}


HRESULT PerfDataReader::getTopology(unsigned int core, gtUInt16* pProcessor, gtUInt16* pNuma)
{
    if ((core >= m_numCpus) || (NULL == pProcessor) || (NULL == pNuma))
    {
        return E_FAIL;
    }

    // Linux PERF file does not contain topology details
    *pProcessor = 0;
    *pNuma = 0;

    return S_OK;
}
