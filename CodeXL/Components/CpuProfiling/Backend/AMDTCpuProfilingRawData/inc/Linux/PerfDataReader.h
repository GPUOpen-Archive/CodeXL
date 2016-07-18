//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfDataReader.h
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingRawData/inc/Linux/PerfDataReader.h#6 $
// Last checkin:   $DateTime: 2016/04/14 01:08:15 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569054 $
//=====================================================================

#ifndef _PERFDATAREADER_H_
#define _PERFDATAREADER_H_

#include <stdint.h>

#include <string>
#include <vector>
#include <map>
#include "linux/perf_event.h"
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

typedef vector<struct perf_event_attr> PerfEventAttrVec;

typedef map<gtUInt64, gtUInt64> PerfEvtIdToEvtTypeMap;

struct perf_trace_event_type;

//
// Class PerfDataReader
//
// Descriptions:
// This class reads the perf.data file
//
class PerfDataReader
{
public:
    PerfDataReader();

    virtual ~PerfDataReader();

    virtual void clear();

    virtual HRESULT init(const string& perfDataPath);

    virtual void deinit();

    bool isOpen() const { return (m_fd != -1); }

    //////////////////////////
    // Query Interfaces

    virtual unsigned int getNumEvents() const { return m_numEventTypes; }

    virtual unsigned int getNumCpus() const { return m_numCpus; }

    virtual HRESULT getCpuInfo(unsigned int* pFamily, unsigned int* pModel = NULL, unsigned int* pStepping = NULL);

    virtual HRESULT getTopology(unsigned int core, gtUInt16* pProcessor, gtUInt16* pNode);

    gtUInt64 getAttrSampleType() { return m_sampleType; }

    virtual HRESULT getFirstRecord(struct perf_event_header* pHdr, const void** ppBuf, gtUInt32* pOffset = NULL);

    virtual HRESULT getRecordFromOffset(gtUInt32 offset,
                                        struct perf_event_header* pHdr,
                                        const void** ppBuf,
                                        gtUInt32* pOffset = NULL);

    virtual HRESULT getNextRecord(struct perf_event_header* pHdr, const void** ppBuf, gtUInt32* pOffset = NULL);

    int getCurrentFileOffset() const { return m_offset; }

    HRESULT getEventAndCpuFromSampleId(gtUInt64 evId,
                                       int* pCpu,
                                       gtUInt32* pEvent,
                                       gtUInt32* pUmask = NULL,
                                       gtUInt32* pOs = NULL,
                                       gtUInt32* pUsr = NULL);

    HRESULT getEventInfoFromEvTypeAndConfig(gtUInt32 evType, gtUInt64 evCfg, gtUInt32* pEvent, gtUInt32* pUmask = NULL);

    const PerfEventAttrVec* getPerfEventAttrVec() const { return &m_perfEventAttrVec; }

    virtual void dumpHeaderSections();

    virtual void dumpData();

    unsigned int getDataSectionSize() { return m_dataSize; }

    unsigned int getCurrentDataSize() const { return (m_offset - m_dataStartOffset); }

    bool isEndOfFile() const { return m_isEof; }

private:
    HRESULT _getRawEventInfo(gtUInt64 evCfg, gtUInt32* pEvent, gtUInt32* pUmask = NULL);

    HRESULT _getPerfEventInfo(gtUInt32 evType, gtUInt64 evCfg, gtUInt32* pEvent);

    //////////////////////////
    // Dumping Interfaces

    void dumpPerfHeader();

    void dumpPerfAttributesSection();

    void dumpPerfEventTypesSection();

    int dumpPerfData();

    virtual HRESULT dump_PERF_RECORD(struct perf_event_header* pHdr, void* ptr, int recNum);


protected:
    int _processPerfAttributesSection();
    int _processPerfFileSection();
    int _processPerfEventTypesSection();

    int                   m_fd;
    int                   m_offset;
    unsigned int          m_numCpus;
    unsigned int          m_numAttrs;
    unsigned int          m_numEventTypes;
    gtUInt64              m_sampleType;
    int                   m_curBufSz;
    void*                 m_pBuf;
    ssize_t               m_dataStartOffset;
    ssize_t               m_dataSize;

    // Dynamic PMU types used by PERF. We need get these type
    // id from /sys/devices/<profile-type>/type
    gtUInt32              m_ibsFetchType;
    gtUInt32              m_ibsOpType;

    struct perf_file_header*                    m_pPerfHdr;
    PerfEventAttrVec                            m_perfEventAttrVec;
    PerfEvtIdToEvtTypeMap                       m_perfEvtIdToEvtTypeMap;
    vector<struct perf_file_section>            m_perfFileSecVec;
    vector<struct perf_trace_event_type>        m_perfEventTypeVec;
    bool m_bSeenComm;

    bool m_isEof;
};

#endif //_PERFDATAREADER_H_
