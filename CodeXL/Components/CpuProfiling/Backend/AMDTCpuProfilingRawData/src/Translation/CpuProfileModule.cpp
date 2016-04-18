//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileModule.cpp
///
//==================================================================================

#include <CpuProfileModule.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

CpuProfileModule::CpuProfileModule()
{
    m_base      = 0;
    m_size      = 0;
    m_modType   = 0;
    m_checksum = 0;

    m_isImdRead  = false;
    m_is32Bit    = true;
    m_isIndirect = false;
    m_isSystemModule = false;
    m_symbolsLoaded = false;
    m_isDebugInfoAvailable = false;

    m_total     = 0;
    m_imdIndex  = -1;

    m_aggTotal  = 0;
    m_funcTotal = 0;
}

gtUInt64 CpuProfileModule::getTotal() const
{
    if (m_total > 0)
    {
        return m_total;
    }
    else if (m_aggTotal >= m_funcTotal)
    {
        return m_aggTotal;
    }
    else
    {
        return m_funcTotal;
    }
}

gtVAddr CpuProfileModule::getBaseAddr() const
{
    return m_base;
}

bool CpuProfileModule::extractFileName(gtString& fileName) const
{
    return ExtractFileName(m_path, fileName);
}

bool CpuProfileModule::isUnchartedFunction(const CpuProfileFunction& func) const
{
    bool ret;

    if (!func.getFuncName().isEmpty())
    {
        ret = (L'!' == func.getFuncName()[func.getFuncName().length() - 1]);
    }
    else
    {
        ret = (func.getBaseAddr() == func.getTopAddr() && func.getBaseAddr() == getBaseAddr());
    }

    return ret;
}

const CpuProfileFunction* CpuProfileModule::getUnchartedFunction() const
{
    const CpuProfileFunction* pUnchartedFunc = NULL;

    if (!m_funcMap.empty())
    {
        const CpuProfileFunction& func = m_funcMap.begin()->second;

        if (isUnchartedFunction(func))
        {
            pUnchartedFunc = &func;
        }
    }

    return pUnchartedFunc;
}

CpuProfileFunction* CpuProfileModule::getUnchartedFunction()
{
    CpuProfileFunction* pUnchartedFunc = NULL;

    if (!m_funcMap.empty())
    {
        CpuProfileFunction& func = m_funcMap.begin()->second;

        if (isUnchartedFunction(func))
        {
            pUnchartedFunc = &func;
        }
    }

    return pUnchartedFunc;
}

const CpuProfileFunction* CpuProfileModule::findFunction(gtVAddr addr) const
{
    const CpuProfileFunction* pFunc = NULL;

    AddrFunctionMultMap::const_iterator it = m_funcMap.upper_bound(addr);

    if (it != m_funcMap.begin())
    {
        pFunc = &(--it)->second;

        if (!pFunc->contains(addr) && !isUnchartedFunction(*pFunc))
        {
            pFunc = NULL;
        }
    }

    return pFunc;
}

CpuProfileFunction* CpuProfileModule::findFunction(gtVAddr addr)
{
    CpuProfileFunction* pFunc = NULL;

    AddrFunctionMultMap::iterator it = m_funcMap.upper_bound(addr);

    if (it != m_funcMap.begin())
    {
        pFunc = &(--it)->second;

        if (!pFunc->contains(addr) && !isUnchartedFunction(*pFunc))
        {
            pFunc = NULL;
        }
    }

    return pFunc;
}

bool CpuProfileModule::doesAddressBelongToModule(gtVAddr addr, const CpuProfileFunction** ppFunc) const
{
    for (AddrFunctionMultMap::const_reverse_iterator rit = m_funcMap.rbegin(), ritEnd = m_funcMap.rend(); rit != ritEnd; ++rit)
    {
        if (addr < rit->first)
        {
            continue;
        }

        const CpuProfileFunction* pFunc = &(rit->second);

        if (addr >= pFunc->getBaseAddr()
            || (pFunc->getEndSample() != pFunc->find(AptKey(addr - (pFunc->getBaseAddr()), 0, 0))))
        {
            *ppFunc = pFunc;
            return true;
        }
    }

    *ppFunc = NULL;
    return false;
}


// HOTSPOT
void CpuProfileModule::recordSample(const SampleInfo& sampleInfo,
                                    gtUInt32 sampleCnt,
                                    gtVAddr funcAddr,
                                    gtUInt32 funcSize,
                                    const gtString& funcName,
                                    const gtString& jncName,
                                    const gtString& srcFile,
                                    unsigned srcLine,
                                    gtUInt32 functionId)
{
    SampleKey sKey(sampleInfo.cpu, sampleInfo.event);

    m_total += sampleCnt;

    /////////////////
    // Update pid map
    PidAggregatedSampleMap::iterator ag_it = m_aggPidMap.find(sampleInfo.pid);

    if (ag_it == m_aggPidMap.end())
    {
        // Add new Pid
        AggregatedSample agSamp(sKey, sampleCnt);
        m_aggPidMap.insert(PidAggregatedSampleMap::value_type(sampleInfo.pid, agSamp));
    }
    else
    {
        ag_it->second.addSamples(sKey, sampleCnt);
    }

    /////////////////
    // Update Function (IMD) Map
    AddrFunctionMultMap::iterator fit = m_funcMap.find(funcAddr);

    if (fit == m_funcMap.end())
    {
        // Add new Function (IMD sub-section)
        CpuProfileFunction func(funcName, funcAddr, funcSize, jncName, srcFile, srcLine, functionId);
        fit = m_funcMap.insert(AddrFunctionMultMap::value_type(funcAddr, func));
        fit->second.insertSample(sampleInfo, sampleCnt);
    }
    else
    {
        fit->second.addSample(sampleInfo, sampleCnt);
    }
}


void CpuProfileModule::recordSample(ProcessIdType pid, const AggregatedSample* pAggSample)
{
    m_total += (*pAggSample).getTotal();

    // Update pid map
    PidAggregatedSampleMap::iterator ag_it = m_aggPidMap.find(pid);

    if (ag_it == m_aggPidMap.end())
    {
        // Add new Pid
        m_aggPidMap.insert(PidAggregatedSampleMap::value_type(pid, *pAggSample));
    }
    else
    {
        ag_it->second.addSamples(pAggSample);
    }
}


void CpuProfileModule::recordSample(gtVAddr addr, const CpuProfileFunction* pFunc)
{
    // Update Function (IMD) Map

    // TODO: Check if the function is available,
    // if not, m_funcMap.insert
    // else,
    //  iterate over the samples...
    //      find the function and addSample..

    // For each sample
    AptAggregatedSampleMap::const_iterator ait = pFunc->getBeginSample();
    AptAggregatedSampleMap::const_iterator aend = pFunc->getEndSample();

    for (; ait != aend; ++ait)
    {
        // AddrFunctionMultMap::iterator fit = m_funcMap.find(ait->first.m_addr);
        AddrFunctionMultMap::iterator fit = m_funcMap.find(addr);

        if (fit == m_funcMap.end())
        {
            // Add new Function (IMD sub-section)
            // CpuProfileFunction func(funcName, funcAddr, jncName, javaSrcName);
            fit = m_funcMap.insert(AddrFunctionMultMap::value_type(addr, *pFunc));
            // fit->second.insertSample(sampleInfo, sampleCnt);
        }
        else
        {
            fit->second.addSample(ait->first, (AggregatedSample*) & (ait->second));
        }
    }

}


void CpuProfileModule::recordFunction(gtVAddr addr, const CpuProfileFunction* pFunc)
{
    AddrFunctionMultMap::iterator fit = m_funcMap.find(addr);

    if (fit == m_funcMap.end())
    {
        fit = m_funcMap.insert(AddrFunctionMultMap::value_type(addr, *pFunc));
    }
} // recordFunction


void CpuProfileModule::insertModMetaData(const gtString& modPath, int imdIndex, ProcessIdType pid, const AggregatedSample& agSamp)
{
    m_path = modPath;
    m_imdIndex = imdIndex;
    insertModMetaData(pid, agSamp);
}

void CpuProfileModule::insertModMetaData(ProcessIdType pid, const AggregatedSample& agSamp)
{
    m_aggPidMap.insert(PidAggregatedSampleMap::value_type(pid, agSamp));
    m_aggTotal += agSamp.getTotal();
}

void CpuProfileModule::insertModDetailData(gtVAddr addr, const CpuProfileFunction& func)
{
    m_funcMap.insert(AddrFunctionMultMap::value_type(addr, func));
    m_funcTotal += func.getTotal();
}

void CpuProfileModule::clear()
{
    m_aggPidMap.clear();
    m_funcMap.clear();
}

bool CpuProfileModule::compares(const CpuProfileModule& m) const
{
    if (m_base != m.m_base)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Warning: %ls: m_base is different (%lx, %lx)", m_path.asCharArray(), m_base, m.m_base);
        //return false;
    }

    if (m_size != m.m_size)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: m_size is different (%x, %x)", m_path.asCharArray(), m_size, m.m_size);
        return false;
    }

    if (m_modType != m.m_modType)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: m_modType is different (%x, %x)", m_path.asCharArray(), m_modType, m.m_modType);
        return false;
    }

    if (m_is32Bit != m.m_is32Bit)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: m_is32Bit is different (%x, %x)", m_path.asCharArray(), m_is32Bit, m.m_is32Bit);
        return false;
    }

    if (m_total != m.m_total)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: m_total is different (%llu, %llu)", m_path.asCharArray(), m_total, m.m_total);
        return false;
    }

    if (m_path != m.m_path)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: m_path is different (%ls, %ls)", m_path.asCharArray(), m_path.asCharArray(), m.m_path.asCharArray());
        return false;
    }

    if (m_imdIndex != m.m_imdIndex)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: m_imdIndex is different (%x, %x)", m_path.asCharArray(), m_imdIndex, m.m_imdIndex);
        return false;
    }

    if (m_aggTotal != m.m_aggTotal)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: m_aggTotal is different (%llx, %llx)", m_path.asCharArray(), m_aggTotal, m.m_aggTotal);
        return false;
    }

    if (m_funcTotal != m.m_funcTotal)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: m_funcTotal is different (%llx, %llx)", m_path.asCharArray(), m_funcTotal, m.m_funcTotal);
        return false;
    }

    if (m_aggPidMap.size() != m.m_aggPidMap.size())
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: m_aggPidMap size is different (%u, %u)", m_path.asCharArray(),
                                   static_cast<unsigned>(m_aggPidMap.size()), static_cast<unsigned>(m.m_aggPidMap.size()));
        return false;
    }

    PidAggregatedSampleMap::const_iterator pit1  = m_aggPidMap.begin(), pend1 = m_aggPidMap.end();
    PidAggregatedSampleMap::const_iterator pit2  = m.m_aggPidMap.begin();

    for (; pit1 != pend1; ++pit1, ++pit2)
    {
        if (pit1->first != pit2->first)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: m_aggPidMap pid is different (%u, %u)", m_path.asCharArray(),
                                       static_cast<unsigned>(pit1->first), static_cast<unsigned>(pit2->first));
            return false;
        }

        if (!pit1->second.compares(pit2->second))
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: AggregatedSample data for pid %u is different", m_path.asCharArray(),
                                       static_cast<unsigned>(pit1->first));
            return false;
        }
    }

    if (m_funcMap.size() != m.m_funcMap.size())
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: m_funcMap size is different (%u, %u)", m_path.asCharArray(),
                                   static_cast<unsigned>(m_funcMap.size()), static_cast<unsigned>(m.m_funcMap.size()));
        return false;
    }

    AddrFunctionMultMap::const_iterator fit1  = m_funcMap.begin(), fend1 = m_funcMap.end();
    AddrFunctionMultMap::const_iterator fit2  = m.m_funcMap.begin();

    for (; fit1 != fend1; ++fit1, ++fit2)
    {
        if (fit1->first != fit2->first)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: m_funcMap address is different (%llx, %llx)", m_path.asCharArray(),
                                       fit1->first, fit2->first);
            return false;
        }

        wchar_t strerr[1024] = { L'\0' };

        if (!fit1->second.compares(fit2->second, strerr, 1024))
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%ls: CpuProfileFunction comparison for base address %llx failed with reason:\n\t%ls",
                                       m_path.asCharArray(), fit1->first, strerr);
            return false;
        }
    }

    return true;
}

static const wchar_t* FindStartOfFileName(const wchar_t* pFullPath, int len)
{
    if (0 > len)
    {
        len = static_cast<int>(wcslen(pFullPath));
    }

    int pos = len - 1;
    const wchar_t* pExeName = pFullPath + pos;

    while (0 < pos)
    {
        if (L'\\' == *pExeName || L'/' == *pExeName)
        {
            ++pExeName;
            break;
        }

        --pos;
        --pExeName;
    }

    return pExeName;
}

bool ExtractFileName(const gtString& fullPath, gtString& fileName)
{
    bool ret = false;

    if (!fullPath.isEmpty())
    {
        const wchar_t* pModFileName = FindStartOfFileName(fullPath.asCharArray(), fullPath.length());
        int len = fullPath.length() - static_cast<int>(pModFileName - fullPath.asCharArray());

        if (0 != len)
        {
            fileName.assign(pModFileName, len);
            ret = true;
        }
    }

    return ret;
}
