//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileSample.cpp
///
//==================================================================================

#include <CpuProfileSample.h>

SampleInfo::SampleInfo() : address(0), tid(0), pid(0), cpu(0), event(0)
{
}

SampleInfo::SampleInfo(gtVAddr a, ProcessIdType p, ThreadIdType t, gtUInt32 c, EventMaskType e) : address(a),
    tid(t),
    pid(p),
    cpu(c),
    event(e)
{
}


SampleKey::SampleKey() : cpu(0), event(0)
{
}

SampleKey::SampleKey(int c, EventMaskType e) : cpu(c), event(e)
{
}

bool SampleKey::operator<(const SampleKey& m) const
{
    return (cpu < m.cpu) || (cpu == m.cpu && event < m.event);
}

bool SampleKey::operator==(const SampleKey& m) const
{
    return cpu == m.cpu && event == m.event;
}


AggregatedSample::AggregatedSample() : m_total(0)
{
}

AggregatedSample::AggregatedSample(SampleKey& key, unsigned long count) : m_total(0)
{
    insertSamples(key, count);
}

AggregatedSample::~AggregatedSample()
{
    clear();
}

void AggregatedSample::clear()
{
    m_sampleMap.clear();
    m_total = 0;
}

void AggregatedSample::aggregateForAllCpus(AggregatedSample* agg) const
{
    for (CpuProfileSampleMap::const_iterator sit = m_sampleMap.begin(), sitEnd = m_sampleMap.end(); sit != sitEnd; ++sit)
    {
        // CPU -1 denotes for all CPUs.
        SampleKey key(-1, sit->first.event);
        agg->addSamples(key, sit->second);
    }
}

void AggregatedSample::insertSamples(SampleKey& key, unsigned long count)
{
    m_sampleMap.insert(CpuProfileSampleMap::value_type(key, count));
    m_total += count;
}

void AggregatedSample::addSamples(SampleKey& key, unsigned long count)
{
    CpuProfileSampleMap::iterator it = m_sampleMap.find(key);

    if (it == m_sampleMap.end())
    {
        m_sampleMap.insert(CpuProfileSampleMap::value_type(key, count));
    }
    else
    {
        it->second += count;
    }

    m_total += count;
}

void AggregatedSample::addSamples(const CpuProfileSampleMap* sampMap)
{
    if (NULL != sampMap)
    {
        for (CpuProfileSampleMap::const_iterator sit = sampMap->begin(), sitEnd = sampMap->end(); sit != sitEnd; ++sit)
        {
            CpuProfileSampleMap::iterator it = m_sampleMap.find(sit->first);

            if (it == m_sampleMap.end())
            {
                m_sampleMap.insert(CpuProfileSampleMap::value_type(sit->first, sit->second));
            }
            else
            {
                it->second += sit->second;
            }

            m_total += sit->second;
        }
    }
}

void AggregatedSample::addSamples(const AggregatedSample* aggSamp)
{
    if (NULL != aggSamp)
    {
        addSamples(&(aggSamp->m_sampleMap));
    }
}

bool AggregatedSample::compares(const AggregatedSample& p) const
{
    return m_total == p.m_total && m_sampleMap == p.m_sampleMap;
}


AptKey::AptKey() : m_addr(0), m_pid(0), m_tid(0)
{
}

AptKey::AptKey(gtVAddr a, ProcessIdType p, ThreadIdType t) : m_addr(a), m_pid(p), m_tid(t)
{
}

bool AptKey::operator<(const AptKey& other) const
{
    if (m_addr < other.m_addr)
    {
        return true;
    }
    else if (m_addr > other.m_addr)
    {
        return false;
    }
    else if (m_pid < other.m_pid)
    {
        return true;
    }
    else if (m_pid > other.m_pid)
    {
        return false;
    }
    else if (m_tid < other.m_tid)
    {
        return true;
    }

    return false;
}

bool AptKey::compares(const AptKey& other) const
{
    return m_addr == other.m_addr && m_pid  == other.m_pid && m_tid  == other.m_tid;
}
