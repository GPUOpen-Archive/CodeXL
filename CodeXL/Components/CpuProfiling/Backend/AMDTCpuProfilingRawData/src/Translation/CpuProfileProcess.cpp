//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileProcess.cpp
///
//==================================================================================

#include <CpuProfileProcess.h>

CpuProfileProcess::CpuProfileProcess()
{
}

void CpuProfileProcess::clear()
{
    m_sampleMap.clear();
}

bool CpuProfileProcess::compares(const CpuProfileProcess& other, wchar_t* strerr, size_t maxlen) const
{
    if (m_is32Bit != other.m_is32Bit)
    {
        swprintf(strerr, maxlen, L"Error: m_is32Bit is different (%x, %x)\n", m_is32Bit, other.m_is32Bit);
        return false;
    }

    if (m_hasCss  != other.m_hasCss)
    {
        swprintf(strerr, maxlen, L"Error: m_hasCss is different (%x, %x)\n", m_hasCss, other.m_hasCss);
        return false;
    }

    if (m_fullPath != other.m_fullPath)
    {
        swprintf(strerr, maxlen, L"Error: m_fullPath is different (%ls, %ls)\n", m_fullPath.asCharArray(), other.m_fullPath.asCharArray());
        return false;
    }

    if (m_total != other.m_total)
    {
        swprintf(strerr, maxlen, L"Error: m_total is different (%llu, %llu)\n", m_total, other.m_total);
        return false;
    }

    if (m_sampleMap != other.m_sampleMap)
    {
        swprintf(strerr, maxlen, L"Error: m_sampleMap is different.\n");
        return false;
    }

    return true;
}

