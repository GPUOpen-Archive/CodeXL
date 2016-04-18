//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfDataReader.h
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingRawData/inc/Linux/CaPerfDataReader.h#4 $
// Last checkin:   $DateTime: 2016/04/14 01:08:15 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569054 $
//=====================================================================

#ifndef _CAPERFDATAREADER_H_
#define _CAPERFDATAREADER_H_

#include <linux/perf_event.h>
#include "CaPerfHeader.h"
#include "PerfDataReader.h"

//
// Class CaPerfDataReader
//
// Description
// This class writes the ca_perf.data file
//
class CaPerfDataReader : public PerfDataReader
{
public:
    CaPerfDataReader();

    virtual ~CaPerfDataReader();

    virtual void clear();

    virtual HRESULT init(const std::string& filepath);

    virtual void deinit();

    //////////////////////////
    // Query Interfaces
    virtual gtUInt32 getNumEvents() const { return m_perfEventAttrVec.size(); }

    size_t getNumTargetPids() const { return m_numTargetPids; }

    HRESULT getTargetPids(size_t maxSize, pid_t* ppid);

    HRESULT getFakeTimerInfo(caperf_section_fake_timer_t* pInfo);

    HRESULT getEventCountData() { return E_NOTIMPL; }

    HRESULT getRunInfoSection() { return E_NOTIMPL; }

    HRESULT getCpuInfoSection() { return E_NOTIMPL; }

    unsigned int getNumCpus() const { return m_nbrCpus; }

    HRESULT getTopology(unsigned int core, gtUInt16* pProcessor, gtUInt16* pNode);

    virtual void dumpHeaderSections();

    virtual void dumpData();

    //////////////////////////
    // Dumping Interfaces

    HRESULT dumpCaPerfHeader();

    HRESULT dumpCaPerfEvents();

    HRESULT dumpCaPerfData();

    virtual HRESULT dump_PERF_RECORD(struct perf_event_header* pHdr, void* ptr, gtUInt32 recNum);

    virtual HRESULT getCpuInfo(unsigned int* pFamily, unsigned int* pModel = NULL, unsigned int* pStepping = NULL);

private:

    HRESULT readHeader();
    HRESULT readSectionHdrs();
    HRESULT readRunInfoSection() { return E_NOTIMPL; }
    HRESULT readCpuInfoSection(gtUInt64 offset, gtUInt64 size);
    HRESULT readCpuTopology(gtUInt64 offset, gtUInt64 size);
    HRESULT readDynamicPmuTypes(gtUInt64 offset, gtUInt64 size);
    HRESULT readEventConfigs(gtUInt64 offset, gtUInt64 size);
    HRESULT readEventSampleIds(gtUInt64 offset, gtUInt64 size);
    HRESULT readTargetPids(gtUInt64 offset, gtUInt64 size);
    HRESULT readFakeTimerInfo(gtUInt64 offset, gtUInt64 size);
    HRESULT createEvtIdMap();
    HRESULT readPMUCountData() { return E_NOTIMPL; }

    caperf_file_header_t        m_fileHeader;

    gtUInt32            m_numSections;
    caperf_section_hdr_t*        m_pSectionHdrs;

    caperf_section_cpuinfo_t    m_cpuFamilyInfo;

    caperf_section_topology_t*  m_pTopology;
    gtUInt32                    m_nbrCpus;

    gtUInt32            m_numEvents;
    caperf_section_evtcfg_t*     m_pEventCfg;

    // Note: PERF only allows to have same sampleType for all
    // the sampling events.
    gtUInt32            m_numSampleIds;
    caperf_section_sample_id_t*  m_pSampleIds;


    gtUInt32            m_numTargetPids;
    pid_t*              m_pTargetPids;

    void*                m_pFakeTimerInfo;
};

#endif // _CAPERFDATAREADER_H_
