//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileWriter.h
///
//==================================================================================

#ifndef _CPUPROFILEWRITER_H_
#define _CPUPROFILEWRITER_H_

#include "CpuProfileOutputStream.h"
#include "CpuProfileInfo.h"
#include "CpuProfileModule.h"
#include "CpuProfileProcess.h"

/**********************************
 * class CaProfileWriter
 *
 * Description:
 * This class implements TBP/EBP writer which is the
 * main interface for creating profile data
 */
class CP_RAWDATA_API CpuProfileWriter : public CpuProfileOutputStream
{
public:

    CpuProfileWriter();
    ~CpuProfileWriter();

    bool Write(const gtString& path,
               CpuProfileInfo* profileInfo,
               const PidProcessMap* procMap,
               const NameModuleMap* modMap,
               const CoreTopologyMap* topMap = NULL);

private:

    enum ProfileOutputStage
    {
        evOut_TaskSummary = 1,
        evOut_ModSummary,
    };

    bool WriteProfileInfo(CpuProfileInfo* env, const PidProcessMap* procMap, const CoreTopologyMap* pTopMap);

    bool WriteProfileTotal(const PidProcessMap* procMap);

    bool WriteProcSection(const PidProcessMap* procMap);

    bool WriteModSectionAndImd(const NameModuleMap* modMap);

    bool WriteProcSectionProlog();

    bool WriteProcLineData(const CpuProfileProcess& proc, ProcessIdType pid);

    bool WriteProcSectionEpilog();

    bool WriteModSectionProlog();

    bool WriteModData(const CpuProfileModule& mod);

    bool WriteModLineData(const CpuProfileModule& mod, ProcessIdType pid, const AggregatedSample& agSamp);

    bool WriteModSectionEpilog();

private:
    EventEncodeVec* m_pEventVec;
    gtMap<EventMaskType, int> m_evToIndexMap;
};

#endif // _CPUPROFILEWRITER_H_
