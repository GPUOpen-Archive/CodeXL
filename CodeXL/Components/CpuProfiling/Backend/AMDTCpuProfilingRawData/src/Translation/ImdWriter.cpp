//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ImdWriter.cpp
/// \brief Implementation of the ImdWriter class.
///
//==================================================================================

#include "ImdWriter.h"
#include <CpuProfileDataTranslationInfo.h>

ImdWriter::ImdWriter() : m_pEvToIndexMap(NULL)
{
}

bool ImdWriter::writeToFile(const CpuProfileModule& mod, gtMap<EventMaskType, int>* pEvToIndexMap)
{
    if (!m_fileStream.isOpened() || !pEvToIndexMap)
    {
        return false;
    }

    m_pEvToIndexMap = pEvToIndexMap;

    if (!writeModDetailProlog(mod))
    {
        return false;
    }

    // For each [SUB] section
    AddrFunctionMultMap::const_iterator it = mod.getBeginFunction(), itEnd = mod.getEndFunction();

    for (; it != itEnd; ++it)
    {

        if (CpuProfileModule::JAVAMODULE == mod.getModType() || CpuProfileModule::MANAGEDPE == mod.getModType())
        {
            CpuProfileFunction* func = (CpuProfileFunction*) & (it->second);
            func->computeJavaAggregatedMetadata();
        }

        if (!writeSubProlog(it->second))
        {
            return false;
        }

        // For each sample
        AptAggregatedSampleMap::const_iterator ait = it->second.getBeginSample(), aend = it->second.getEndSample();

        for (; ait != aend; ++ait)
        {
            if (!writeModDetailLine(ait->first.m_pid,
                                    ait->first.m_tid,
                                    ait->first.m_addr - it->second.getBaseAddr(),
                                    ait->second))
            {
                return false;
            }
        }

        if (!writeSubEpilog())
        {
            return false;
        }
    }

    if (!writeModDetailEpilog())
    {
        return false;
    }

    return true;
}

bool ImdWriter::writeModDetailProlog(const CpuProfileModule& mod)
{
    if (!m_fileStream.isOpened())
    {
        return false;
    }

    if (m_stage != evOut_OK)
    {
        return false;
    }

    WriteFormat(L"[%ls]\n", mod.getPath().asCharArray());
    WriteFormat(L"BASE=0x%llx\n", mod.m_base);
    WriteFormat(L"SIZE=%u\n", mod.m_size);
    WriteFormat(L"SAMPS=%llu\n", mod.getTotal());
    WriteFormat(L"ModuleType=%d\n", mod.m_modType);
    WriteFormat(L"NUMSUBSECTIONS=%u\n", mod.getNumSubSection());
    m_fileStream.flush();

    m_stage = evOut_ModDetail;

    return true;
}


// This function writes a line under [each module] section.
// The format is:
// TGID,TID,TOTALSAMPLE,#EVENTSET,[CPU INDEX] #SAMPLE,OFFSET,FUNCNAME,
//      FUNCBASEA+DDR,SRCFILE,JNCFILE
//
// Note: Since verison 8, the JNCFile will only contain file name.
//      It's because the path can ge calculated by ebp file path + /jit/.
//  -Lei 08/06/2008.
//
bool ImdWriter::writeModDetailLine(ProcessIdType pid, ThreadIdType tid, gtVAddr sampAddr, const AggregatedSample& agSamp)
{
    if (!m_fileStream.isOpened())
    {
        return false;
    }

    if (m_stage != evOut_ModDetail && m_stage != evOut_JitDetail)
    {
        return false;
    }

    // PID,TID:
    WriteFormat(L"%u,%u", static_cast<unsigned>(pid), static_cast<unsigned>(tid));

    // total
    WriteFormat(L",%llu", agSamp.getTotal());

    // event set number
    WriteFormat(L",%u", agSamp.getSampleMapSize());

    CpuProfileSampleMap::const_iterator it = agSamp.getBeginSample(), itEnd = agSamp.getEndSample();

    //[CPU EVENT-INDEX] #Sample
    for (; it != itEnd; ++it)
    {
        SampleKey key = it->first;

        if (0UL != it->second)
        {
            gtMap<EventMaskType, int>::iterator eit;

            if ((eit = m_pEvToIndexMap->find(key.event)) == m_pEvToIndexMap->end())
            {
                return false;
            }

            WriteFormat(L",[%u %d] %u", key.cpu, eit->second, it->second);
        }
    }

    WriteFormat(L",0x%llx\n", sampAddr);
    return true;
}

bool ImdWriter::writeModDetailEpilog()
{
    if (!m_fileStream.isOpened())
    {
        return false;
    }

    if (m_stage != evOut_ModDetail)
    {
        return false;
    }

    WriteFormat(IMD_END);
    WriteFormat(L"\n\n");
    m_fileStream.flush();

    m_stage = evOut_OK;

    return true;
}


bool ImdWriter::writeSubProlog(const CpuProfileFunction& func)
{
    if (!m_fileStream.isOpened())
    {
        return false;
    }

    if (m_stage != evOut_ModDetail)
    {
        return false;
    }

    WriteFormat(L"\n");
    WriteFormat(SUB_BEGIN);
    WriteFormat(L"\n");

    if (!func.getJncFileName().isEmpty())
    {
        WriteFormat(L"BINARYFILE=%ls\n", func.getJncFileName().asCharArray());
    }

    if (!func.getSourceFile().isEmpty())
    {
        WriteFormat(L"SRCFILE=%ls\n", func.getSourceFile().asCharArray());
    }

    if (0U != func.getSourceLine())
    {
        WriteFormat(L"SRCLINE=%u\n", func.getSourceLine());
    }

    if (!func.getFuncName().isEmpty())
    {
        WriteFormat(L"SYMBOL=%ls\n", func.getFuncName().asCharArray());
    }

    if (0 != func.getBaseAddr())
    {
        WriteFormat(L"BASEADDR=0x%llx\n", func.getBaseAddr());
    }

    if (func.getBaseAddr() != func.getTopAddr())
    {
        WriteFormat(L"TOPADDR=0x%llx\n", func.getTopAddr());
    }

    // For each pid/tid aggregated sample metadata
    for (AptAggregatedSampleMap::const_iterator ait = func.getBeginMetadata(), aend = func.getEndMetadata(); ait != aend; ++ait)
    {
        WriteFormat(L"AGGREGATED=");

        if (!writeModDetailLine(ait->first.m_pid, ait->first.m_tid, ait->first.m_addr, ait->second))
        {
            return false;
        }
    }

    WriteFormat(L"LINECOUNT=%u\n", func.getSampleMapSize());
    m_fileStream.flush();

    m_stage = evOut_JitDetail;

    return true;
}


bool ImdWriter::writeSubEpilog()
{
    if (!m_fileStream.isOpened())
    {
        return false;
    }

    if (m_stage != evOut_JitDetail)
    {
        return false;
    }

    WriteFormat(SUB_END);
    WriteFormat(L"\n");
    m_fileStream.flush();

    m_stage = evOut_ModDetail;

    return true;
}
