//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileFunction.cpp
///
//==================================================================================

#include <CpuProfileModule.h>

CpuProfileFunction::CpuProfileFunction() : m_baseAddr(0), m_topAddr(0), m_total(0), m_sourceLine(0)
{
}

CpuProfileFunction::CpuProfileFunction(const gtString& name,
                                       gtVAddr baseAddr,
                                       gtUInt32 size,
                                       const gtString& jncFileName,
                                       const gtString& sourceFile,
                                       unsigned lineNumber,
                                       gtUInt32 functionId) :
    m_baseAddr(baseAddr),
    m_topAddr(baseAddr + static_cast<gtVAddr>(size)),
    m_name(name),
    m_jncFileName(jncFileName),
    m_sourceFile(sourceFile),
    m_total(0),
    m_sourceLine(lineNumber)
{
#ifdef AMDT_ENABLE_CPUPROF_DB
    m_functionId = functionId;
#else
    GT_UNREFERENCED_PARAMETER(functionId);
#endif
}

void CpuProfileFunction::setBaseAddr(gtVAddr addr)
{
    gtUInt32 size = getSize();
    m_baseAddr = addr;
    setSize(size);
}

void CpuProfileFunction::setTopAddr(gtVAddr addr)
{
    if (addr >= m_baseAddr)
    {
        m_topAddr = addr;
    }
}

gtUInt32 CpuProfileFunction::getSize() const
{
    return static_cast<gtUInt32>(m_topAddr - m_baseAddr);
}

void CpuProfileFunction::setSize(gtUInt32 size)
{
    m_topAddr = m_baseAddr + static_cast<gtVAddr>(size);
}

bool CpuProfileFunction::contains(gtVAddr addr) const
{
    gtUInt32 offset = static_cast<gtUInt32>(addr - m_baseAddr);
    return (offset < getSize() || 0U == offset);
}

void CpuProfileFunction::getSourceInfo(gtString& sourceInfo) const
{
    if (ExtractFileName(m_sourceFile, sourceInfo) && 0U != m_sourceLine)
    {
        sourceInfo.append(L'(');
        sourceInfo.appendUnsignedIntNumber(m_sourceLine);
        sourceInfo.append(L')');
    }
}

void CpuProfileFunction::addMetadataSample(const AptKey& aKey, AggregatedSample agSamp)
{
    AptAggregatedSampleMap::iterator ait = m_aptMetadata.find(aKey);

    if (m_aptMetadata.end() == ait)
    {
        m_aptMetadata.insert(AptAggregatedSampleMap::value_type(aKey, agSamp));
    }
    else
    {
        ait->second.addSamples(&agSamp);
    }
}

void CpuProfileFunction::computeJavaAggregatedMetadata()
{
    for (AptAggregatedSampleMap::const_iterator ait  = m_aptMap.begin(), aend = m_aptMap.end(); ait != aend; ++ait)
    {
        AptKey aKey(0, ait->first.m_pid, ait->first.m_tid);

        AptAggregatedSampleMap::iterator mit = m_aptMetadata.find(aKey);

        if (m_aptMetadata.end() == mit)
        {
            m_aptMetadata.insert(AptAggregatedSampleMap::value_type(aKey, ait->second));
        }
        else
        {
            mit->second.addSamples(&(ait->second));
        }
    }
}

void CpuProfileFunction::addSample(gtVAddr a, ProcessIdType p, ThreadIdType t, int c, EventMaskType e, unsigned long data)
{
    AptKey aKey(a, p, t);
    SampleKey sKey(c, e);
    AggregatedSample agSamp(sKey, data);
    addSample(aKey, agSamp);
}

void CpuProfileFunction::addSample(const SampleInfo& sInfo, unsigned long data)
{
    addSample(sInfo.address, sInfo.pid, sInfo.tid, sInfo.cpu, sInfo.event, data);
}

void CpuProfileFunction::addSample(const AptKey& aKey, AggregatedSample agSamp)
{
    m_total += agSamp.getTotal();
    AptAggregatedSampleMap::iterator ait = m_aptMap.find(aKey);

    if (ait == m_aptMap.end())
    {
        m_aptMap.insert(AptAggregatedSampleMap::value_type(aKey, agSamp));
    }
    else
    {
        ait->second.addSamples(&agSamp);
    }
}

void CpuProfileFunction::addSample(const AptKey& aKey, AggregatedSample* agSamp)
{
    m_total += (*agSamp).getTotal();

    AptAggregatedSampleMap::iterator ait = m_aptMap.find(aKey);

    if (ait == m_aptMap.end())
    {
        m_aptMap.insert(AptAggregatedSampleMap::value_type(aKey, (*agSamp)));
    }
    else
    {
        ait->second.addSamples(agSamp);
    }
}

// Assuming the key is not already existed.
void CpuProfileFunction::insertSample(const SampleInfo& sInfo, unsigned long data)
{
    AptKey aKey(sInfo.address, sInfo.pid, sInfo.tid);
    SampleKey sKey(sInfo.cpu, sInfo.event);
    AggregatedSample agSamp(sKey, data);
    m_aptMap.insert(AptAggregatedSampleMap::value_type(aKey, agSamp));
}

// Assuming the key is not already existed.
void CpuProfileFunction::insertSample(AptKey& aKey, const AggregatedSample& agSamp)
{
    m_total += agSamp.getTotal();
    m_aptMap.insert(AptAggregatedSampleMap::value_type(aKey, agSamp));
}

void CpuProfileFunction::removeSample(AptAggregatedSampleMap::iterator ait)
{
    if (ait != m_aptMap.end())
    {
        m_total -= ait->second.getTotal();
        m_aptMap.erase(ait);
    }
}

void CpuProfileFunction::clearSample()
{
    m_total = 0;
    m_aptMap.clear();
    m_aptMetadata.clear();
}

bool CpuProfileFunction::compares(const CpuProfileFunction& f, wchar_t* strerr, size_t maxlen) const
{
    if (m_baseAddr != f.m_baseAddr)
    {
        swprintf(strerr, maxlen, L"Error: m_baseAddr is different(%llx, %llx)\n", m_baseAddr, m_baseAddr);
        return false;
    }

    if (m_topAddr != f.m_topAddr)
    {
        swprintf(strerr, maxlen, L"Error: m_topAddr is different(%llx, %llx)\n", m_topAddr, m_topAddr);
        return false;
    }

    if (m_name != f.m_name)
    {
        swprintf(strerr, maxlen, L"Error: m_name is different(%ls, %ls)\n", m_name.asCharArray(), f.m_name.asCharArray());
        return false;
    }

    if (m_jncFileName != f.m_jncFileName)
    {
        swprintf(strerr, maxlen, L"Error: m_jncFileName is different(%ls, %ls)\n", m_jncFileName.asCharArray(), f.m_jncFileName.asCharArray());
        return false;
    }

    if (m_sourceFile != f.m_sourceFile)
    {
        swprintf(strerr, maxlen, L"Error: m_javaSrcName is different(%ls, %ls)\n", m_sourceFile.asCharArray(), f.m_sourceFile.asCharArray());
        return false;
    }

    if (m_total != f.m_total)
    {
        swprintf(strerr, maxlen, L"Error: m_total is different(%llu, %llu)\n", m_total, f.m_total);
        return false;
    }


    if (m_aptMap.size() != f.m_aptMap.size())
    {
        swprintf(strerr, maxlen, L"Error: m_aptMap size is different\n");
        return false;
    }

    AptAggregatedSampleMap::const_iterator ait1  = m_aptMap.begin(), aend1 = m_aptMap.end(), ait2  = f.m_aptMap.begin();

    for (; ait1 != aend1; ++ait1, ++ait2)
    {
        if (!ait1->first.compares(ait2->first))
        {
            swprintf(strerr, maxlen, L"Error: AptKey is different (%llx:%d:%d, %llx:%d:%d) \n",
                     ait1->first.m_addr, ait1->first.m_pid, ait1->first.m_tid,
                     ait2->first.m_addr, ait2->first.m_pid, ait2->first.m_tid);
            return false;
        }

        if (!ait1->second.compares(ait2->second))
        {
            swprintf(strerr, maxlen, L"Error: Data for AptKey (%llx:%d:%d) is different.\n",
                     ait1->first.m_addr, ait1->first.m_pid, ait1->first.m_tid);
            return false;
        }
    }

    return true;
}
