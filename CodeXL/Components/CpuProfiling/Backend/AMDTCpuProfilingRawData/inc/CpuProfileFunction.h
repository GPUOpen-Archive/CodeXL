//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileFunction.h
///
//==================================================================================

#ifndef _CPUPROFILEFUNCTION_H_
#define _CPUPROFILEFUNCTION_H_

#include "CpuProfilingRawDataDLLBuild.h"
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include "CpuProfileSample.h"

/*******************************************
 * class CpuProfileFunction
 *
 * Description:
 * This class represent the [SUB] section of the IMD file
 */
class CP_RAWDATA_API CpuProfileFunction
{
public:
    CpuProfileFunction();
    CpuProfileFunction(const gtString& name,
                       gtVAddr baseAddr,
                       gtUInt32 size,
                       const gtString& jncFileName,
                       const gtString& sourceFile,
                       unsigned lineNumber = 0U,
                       gtUInt32 functionId = 0);

    const gtString& getFuncName() const { return m_name; }
    void setFuncName(const gtString& name) { m_name = name; }

    gtVAddr getBaseAddr() const { return m_baseAddr; }
    void setBaseAddr(gtVAddr addr);

    gtVAddr getTopAddr() const { return m_topAddr; }
    void setTopAddr(gtVAddr addr);

    gtUInt32 getSize() const;
    void setSize(gtUInt32 size);

    bool contains(gtVAddr addr) const;

    const gtString& getJncFileName() const { return m_jncFileName; }
    void setJncFileName(const gtString& name) { m_jncFileName = name; }

    gtUInt64 getTotal() const { return m_total; }

    void getSourceInfo(gtString& sourceInfo) const;

    const gtString& getSourceFile() const { return m_sourceFile; }
    void setSourceFile(const gtString& filePath) { m_sourceFile = filePath; }

    unsigned getSourceLine() const { return m_sourceLine; }
    void setSourceLine(unsigned lineNumber) { m_sourceLine = lineNumber; }

    unsigned int getMetadataMapSize() const { return (unsigned int)m_aptMetadata.size(); }

    AptAggregatedSampleMap::const_iterator getBeginMetadata() const { return m_aptMetadata.begin(); }

    AptAggregatedSampleMap::const_iterator getEndMetadata() const  { return m_aptMetadata.end(); }

    void addMetadataSample(const AptKey& aKey, AggregatedSample agSamp);
    void computeJavaAggregatedMetadata();

    unsigned int getSampleMapSize() const { return static_cast<unsigned int>(m_aptMap.size()); }

    void addSample(gtVAddr a, ProcessIdType p, ThreadIdType t, // AptKey
                   int c,
                   EventMaskType e, // SampleKey
                   unsigned long data);
    void addSample(const SampleInfo& sInfo, unsigned long data);
    void addSample(const AptKey& aKey, AggregatedSample agSamp);
    void addSample(const AptKey& aKey, AggregatedSample* agSamp);

    // Assuming the key is not already existed.
    void insertSample(const SampleInfo& sInfo, unsigned long data);

    // Assuming the key is not already existed.
    void insertSample(AptKey& aKey, const AggregatedSample& agSamp);
    void removeSample(AptAggregatedSampleMap::iterator ait);

    AptAggregatedSampleMap::iterator getBeginSample() { return m_aptMap.begin(); }
    AptAggregatedSampleMap::iterator getEndSample() { return m_aptMap.end(); }

    AptAggregatedSampleMap::const_iterator getBeginSample() const { return m_aptMap.begin(); }
    AptAggregatedSampleMap::const_iterator getEndSample() const { return m_aptMap.end(); }
    AptAggregatedSampleMap::const_iterator getLowerBoundSample(const AptKey& key) const { return m_aptMap.lower_bound(key); }
    AptAggregatedSampleMap::const_iterator find(const AptKey& key) const { return m_aptMap.find(key); }

    void clearSample();

    bool compares(const CpuProfileFunction& f, wchar_t* strerr, size_t maxlen) const;

private:
    gtVAddr m_baseAddr;
    gtVAddr m_topAddr;
    gtString m_name;
    gtString m_jncFileName; // USED by java
    gtString m_sourceFile; // USED by java
    gtUInt64 m_total;
    AptAggregatedSampleMap m_aptMetadata;   // USED by java
    AptAggregatedSampleMap m_aptMap;    // [SUB] sections
    unsigned m_sourceLine;

public:
    gtUInt32 m_functionId = 0;
};

/****************************************
 * This contains all the [SUB] sections of IMD files
 */
typedef std::multimap<gtVAddr, CpuProfileFunction> AddrFunctionMultMap;

#endif // _CPUPROFILEFUNCTION_H_
