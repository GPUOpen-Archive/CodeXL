//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ImdWriter.h
/// \brief Interface for the ImdWriter class.
///
//==================================================================================

#ifndef _IMDWRITER_H_
#define _IMDWRITER_H_

#include <CpuProfileOutputStream.h>
#include <CpuProfileModule.h>

class ImdWriter : public CpuProfileOutputStream
{
public:
    enum ImdOutputStage
    {
        evOut_ModDetail = 1,
        evOut_JitDetail,
    };

    ImdWriter();

    bool writeToFile(const CpuProfileModule& mod, gtMap<EventMaskType, int>* pEvToIndexMap);

private:
    bool writeModDetailProlog(const CpuProfileModule& mod);
    bool writeModDetailEpilog();
    bool writeModDetailLine(ProcessIdType pid, ThreadIdType tid, gtVAddr sampAddr, const AggregatedSample& agSamp);

    bool writeSubProlog(const CpuProfileFunction& func);
    bool writeSubEpilog();

    gtMap<EventMaskType, int>* m_pEvToIndexMap;
};

#endif // _IMDWRITER_H_
