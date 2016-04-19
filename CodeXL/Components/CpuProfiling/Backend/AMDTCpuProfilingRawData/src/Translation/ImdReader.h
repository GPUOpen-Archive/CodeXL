//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ImdReader.h
/// \brief Interface for the ImdReader class.
///
//==================================================================================

#ifndef _IMDREADER_H_
#define _IMDREADER_H_

#include <CpuProfileInputStream.h>
#include <CpuProfileModule.h>
#include <CpuProfileInfo.h>

class ImdReader : public CpuProfileInputStream
{
public:
    ImdReader(CpuProfileInfo* pProfileInfo = NULL);

    ~ImdReader();

    bool readModSection(CpuProfileModule& mod);

private:
    bool parseModAttributes(const gtString& line, CpuProfileModule& mod);

    bool parseJitAttributes(const gtString& line, CpuProfileFunction& func);

    bool parseSubAttributes(const gtString& line, CpuProfileFunction& func);

    bool processInstSampleLine(const gtString& line,
                               AptKey& aptKey,
                               AggregatedSample& aggSamp,
                               gtString& javaFuncName,
                               gtVAddr& jitBaseAddr,
                               gtString& javaSrcFile,
                               gtString& jncFile);

    bool processInstSampleLine(const gtString& line, AptKey& aptKey, AggregatedSample& aggSamp, int& k);

    bool markSection(const gtString& name, fpos_t* pos);

    bool processDataBlock(gtString& line, CpuProfileModule& mod, CpuProfileFunction& func);

    bool processDataBlock_JavaTbp6(gtString& line, CpuProfileModule& mod);

    bool processJitBlock(gtString& line, CpuProfileModule& mod);

    bool processSubBlock(gtString& line, CpuProfileModule& mod);

private:
    unsigned int m_imdVersion;
    CpuProfileInfo* m_pProfileInfo;
};

#endif // _IMDREADER_H_
