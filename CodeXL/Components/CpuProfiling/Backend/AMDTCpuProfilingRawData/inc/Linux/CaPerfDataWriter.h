//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfDataWriter.h
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingRawData/inc/Linux/CaPerfDataWriter.h#4 $
// Last checkin:   $DateTime: 2016/04/14 01:08:15 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569054 $
//=====================================================================

#ifndef _CAPERFDATAWRITER_H_
#define _CAPERFDATAWRITER_H_

// System Headers
#include <linux/perf_event.h>

// Project Headers
#include "CaPerfHeader.h"
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTCpuProfilingControl/src/Linux/CaPerfEvent.h>

//
// Class CaPerfDataWriter
//
// Description
// This class writes the ca_perf.data file
//

struct kModule
{
    char        m_modName[OS_MAX_PATH];
    std::string m_absPath;

    gtUInt64    m_startAddress;
    gtUInt64    m_endAddress;

    void addAbsPath(const std::string& path)
    {
        m_absPath = path;
    }

    void setEndAddress(gtUInt64 addr)
    {
        m_endAddress = addr;
    }
};

typedef gtMap<std::string, kModule>  kModuleNameMap;
typedef gtMap<gtUInt64, kModule>  kModuleAddrMap;
typedef gtList<caperf_section_sample_id_t>  SampleInfoList;

class CaPerfDataWriter
{
public:
    CaPerfDataWriter();

    ~CaPerfDataWriter();

    HRESULT init(const char* filepath, bool overwrite = false);

    int writeTargetPids(gtUInt32 numPids, pid_t* pPids);

    HRESULT writeEventsConfig(CaPerfEvtList& countEventsList, CaPerfEvtList& sampleEventsList);

    HRESULT writePMUSampleData(void* data, ssize_t size);

    HRESULT writeSWPProcessMmaps(gtUInt16 sampleRecSize);

    HRESULT writePidMmaps(pid_t pid, pid_t tgid, gtUInt16 sampleRecSize);

    pid_t writePidComm(pid_t pid, gtUInt16 sampleRecSize);

    HRESULT writePidFork(pid_t pid, gtUInt16 sampleRecSize);

    HRESULT writeKernelInfo(gtUInt64 sampleRecSize);

    HRESULT writeFakeTimerInfo(caperf_section_fake_timer_t* pInfo);

    HRESULT writePMUCountData() { return E_NOTIMPL; }

private:
    HRESULT writeHeader();

    HRESULT updateFileHeader(gtUInt32 sect_hdr_offset, gtUInt32 sect_hdr_size);

    gtUInt32 getIdx(gtUInt32 section);

    HRESULT writeSectionHdr(caperf_section_type_t section, gtUInt64 startOffset = 0, gtUInt64 size = 0);

    HRESULT updateSectionHdr(caperf_section_type_t section, gtUInt64 startOffset, gtUInt64 size);

    HRESULT getCpuId(gtUInt32 fn, gtUInt32* pEax, gtUInt32* pEbx, gtUInt32* pEcx, gtUInt32* pEdx);

    HRESULT writeCpuInfo();

    HRESULT writeRunInfo();

    HRESULT writeCpuTopology();

    HRESULT writeCountEvent(CaPerfEvent& event);

    HRESULT writeSampleEvent(CaPerfEvent& event);

    HRESULT writeSampleEventsInfo();

    HRESULT findKernelSymbol(const char* symbol, gtUByte type, gtUInt64* startAddress);

    HRESULT writeKernelMmap(const char* symbolName, gtUInt16 sampleRecSize);

    HRESULT getKernelVersion();

    HRESULT readKernelModules();

    HRESULT setModulesPath();

    HRESULT setModulesAbsolutePath(const std::string& dirName);

    HRESULT writeKernelModules(gtUInt16 sampleRecSize);

    HRESULT writeGuestOsInfo() { return E_NOTIMPL; }

    HRESULT getDynamicPmuType(char* pFilePath, gtUInt32& type);

    HRESULT writeDynamicPmuTypes();

private: //Member Variables
    int m_fd;
    int m_offset;

    caperf_file_header_t  m_fileHeader;

    gtUInt64 m_nbrSections;
    gtUInt64 m_maxNbrSections;
    gtUInt64 m_sectionHdrOffsets[CAPERF_MAX_SECTIONS];

    // size of the sample records; based on
    // perf_event_attr::sample_type; This does not include
    // perf_event_header
    // Compute this when we write a sampling event
    gtUInt16 m_sampleRecSize;

    ssize_t m_dataStartOffset;
    ssize_t m_dataSize;

    ca_event_t* m_pForkEvent;
    ca_event_t* m_pCommEvent;
    ca_event_t* m_pMmapEvent;

    // list<struct caperf_section_sample_id_t>  m_sampleIdList;
    SampleInfoList m_sampleIdList;

    // TODO: all the kernel/kallsyms related stuff should be
    // in a separate class
    bool m_guestOs;
    std::string m_kVersion;
    std::string m_rootDir;
    std::string m_kernelSymbolFile;
    std::string m_kModulesPath;
    kModuleNameMap m_kModuleNameMap;
    kModuleAddrMap m_kModuleAddrMap;
};

#endif // _CAPERFDATAWRITER_H_
