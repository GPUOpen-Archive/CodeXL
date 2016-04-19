//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileSample.h
///
//==================================================================================

#ifndef _CPUPROFILESAMPLE_H_
#define _CPUPROFILESAMPLE_H_

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include "CpuProfilingRawDataDLLBuild.h"
#include "CpuProfileDataTranslationInfo.h"
#include <AMDTCpuPerfEventUtils/inc/EventEncoding.h>

/**********************************************************
 * Class SampleInfo
 *
 * Description:
 * This is a container for passing around information
 * related to a particular sample.
 */
class CP_RAWDATA_API SampleInfo
{
public:
    SampleInfo();
    SampleInfo(gtVAddr a, ProcessIdType p, ThreadIdType t, gtUInt32 c, EventMaskType e);

public:
    gtVAddr       address;
    ThreadIdType       tid;
    ProcessIdType       pid;
    gtUInt32      cpu;
    EventMaskType event;
};


/**********************************************************
 * Class SampleKey
 *
 * Description:
 * This is the key for CpuProfileSampleMap.
 */
class CP_RAWDATA_API SampleKey
{
public:
    SampleKey();
    SampleKey(int c, EventMaskType e);

    bool operator<(const SampleKey& m) const;
    bool operator==(const SampleKey& m) const;

public:
    int cpu;
    EventMaskType event;
};

/**********************************************************
 * CpuProfileSampleMap
 *
 * Description:
 * This is the smallest granularity map
 * which contain sample per cpu per event.
 */
typedef gtMap<SampleKey, unsigned long> CpuProfileSampleMap;



/**********************************************************
 * Class AggregatedSample
 *
 * Description:
 * This is the general purpose container for storing samples.
 * It contains a map and provide keep track of total number
 * of samples within the map.
 */
class CP_RAWDATA_API AggregatedSample
{
public:
    AggregatedSample();
    AggregatedSample(SampleKey& key, unsigned long count);
    virtual ~AggregatedSample();

    void clear();

    gtUInt64 getTotal() const { return m_total; }

    unsigned int getSampleMapSize() const { return static_cast<unsigned int>(m_sampleMap.size()); }

    void aggregateForAllCpus(AggregatedSample* agg) const;


    //////////////////////////////////////////////////////////
    /* NOTE: Inserting assumes the key doesn't exist
     *       in the current map
     */
    void insertSamples(SampleKey& key, unsigned long count);


    //////////////////////////////////////////////////////////
    /* NOTE: Adding assumes the key might already exist
     *       in the current map. Need to search!!!
     */
    void addSamples(SampleKey& key, unsigned long count);

    void addSamples(const CpuProfileSampleMap* sampMap);
    void addSamples(const AggregatedSample* aggSamp);

    CpuProfileSampleMap::const_iterator getBeginSample() const { return m_sampleMap.begin(); }
    CpuProfileSampleMap::const_iterator getEndSample() const { return m_sampleMap.end(); }
    CpuProfileSampleMap::const_iterator find(SampleKey& key) const { return m_sampleMap.find(key); }

    bool compares(const AggregatedSample& p) const;

protected:
    gtUInt64 m_total;
    CpuProfileSampleMap m_sampleMap;
};


/**********************************************************
 * PidAggregatedSampleMap
 *
 * Description:
 * This map contain samples for each PID. Mainly used for
 * the data of [MODULE] section of the TBP/EBP file.
 */
typedef gtMap<ProcessIdType, AggregatedSample> PidAggregatedSampleMap;


/**********************************************************
 * class AptKey:
 *
 * Description:
 * This class is the key containing "Addr + Pid + Tid" (APT)
 */
class CP_RAWDATA_API AptKey
{
public:
    AptKey();
    AptKey(gtVAddr a, ProcessIdType p, ThreadIdType t);

    bool operator<(const AptKey& other) const;

    bool compares(const AptKey& other) const;

    gtVAddr m_addr;
    ProcessIdType m_pid;
    ThreadIdType m_tid;
};

/**********************************************************
 * This map represent the data in [SUB] section of the IMD file.
 * This contains the samples per address per pid per tid.
 */
typedef gtMap<AptKey, AggregatedSample> AptAggregatedSampleMap;

#endif //_CPUPROFILESAMPLE_H_
