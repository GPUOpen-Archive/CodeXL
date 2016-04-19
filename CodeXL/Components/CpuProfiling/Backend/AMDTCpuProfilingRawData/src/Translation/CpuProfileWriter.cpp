//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileWriter.cpp
///
//==================================================================================

// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingRawData/src/Translation/CpuProfileWriter.cpp#8 $
// Last checkin:   $DateTime: 2016/02/15 06:12:30 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 559634 $
//=====================================================================

#include <AMDTOSWrappers/Include/osFilePath.h>
#include <CpuProfileWriter.h>
#include "ImdWriter.h"

CpuProfileWriter::CpuProfileWriter() : m_pEventVec(nullptr)
{
}

CpuProfileWriter::~CpuProfileWriter()
{
    close();
}

bool CpuProfileWriter::Write(const gtString& path,
                             CpuProfileInfo* profileInfo,
                             const PidProcessMap* procMap,
                             const NameModuleMap* modMap,
                             const CoreTopologyMap* pTopMap)
{
    bool ret = open(path) &&
               WriteProfileInfo(profileInfo, procMap, pTopMap) &&
               WriteProcSection(procMap) &&
               WriteModSectionAndImd(modMap);

    if (ret)
    {
        close();
    }

    return ret;
}

bool CpuProfileWriter::WriteProfileInfo(CpuProfileInfo* profileInfo, const PidProcessMap* procMap, const CoreTopologyMap* pTopMap)
{
    if (!m_fileStream.isOpened())
    {
        return false;
    }

    if (m_stage != evOut_OK)
    {
        return false;
    }

    // save eventmask in case we need it
    m_pEventVec = &(profileInfo->m_eventVec);

    //order is VERY important
    // write version info
    WriteFormat(L"TBPFILEVERSION=%u\n", profileInfo->m_tbpVersion);

    if (profileInfo->m_tbpVersion > TBPVER_BEFORE_RI)
    {
        // Start to write runinfo section
        WriteFormat(RUNINFO_BEGIN);
        WriteFormat(L"\n");
        WriteFormat(L"TARGETPATH=%ls\n", profileInfo->m_targetPath.asCharArray());
        WriteFormat(L"WORKINGDIR=%ls\n", profileInfo->m_wrkDirectory.asCharArray());
        WriteFormat(L"CMDARGUMENTS=%ls\n", profileInfo->m_cmdArguments.asCharArray());
        WriteFormat(L"ENVVARIABLES=%ls\n", profileInfo->m_envVariables.asCharArray());
        WriteFormat(L"OSNAME=%ls\n", profileInfo->m_osName.asCharArray());
        WriteFormat(L"PROFILESCOPE=%ls\n", profileInfo->m_profScope.asCharArray());
        WriteFormat(L"PROFILETYPE=%ls\n", profileInfo->m_profType.asCharArray());
        WriteFormat(L"PROFILEDIR=%ls\n", profileInfo->m_profDirectory.asCharArray());
        WriteFormat(L"STARTTIME=%ls\n", profileInfo->m_profStartTime.asCharArray());
        WriteFormat(L"ENDTIME=%ls\n", profileInfo->m_profEndTime.asCharArray());
        WriteFormat(L"CSSENABLED=%ls\n", profileInfo->m_isCSSEnabled ? L"ENABLED" : L"DISABLED");
        WriteFormat(L"CSSUNWINDDEPTH=%u\n", profileInfo->m_cssUnwindDepth);

        if (CP_CSS_SCOPE_UNKNOWN != profileInfo->m_cssScope)
        {
            WriteFormat(L"CSSSCOPE=0x%X\n", static_cast<unsigned int>(profileInfo->m_cssScope));
        }

        WriteFormat(L"CSSFPO=%ls\n", profileInfo->m_isCssSupportFpo ? L"YES" : L"NO");
        WriteFormat(L"PROFILINGCLU=%ls\n", profileInfo->m_isProfilingCLU ? L"YES" : L"NO");

        // write end of runinfo section
        WriteFormat(RUNINFO_END);
        WriteFormat(L"\n\n");
    }

    // Start to write Environment section
    WriteFormat(ENV_BEGIN);
    WriteFormat(L"\n");
    WriteFormat(L"CPU=%u\n", profileInfo->m_numCpus);
    WriteFormat(L"NumEvents=%lu\n", profileInfo->m_numEvents);
    WriteFormat(L"MODULES=%u\n", profileInfo->m_numModules);
    WriteFormat(L"SAMPS=%u\n", profileInfo->m_numSamples);
    WriteFormat(L"TIMESTAMP=%ls\n", profileInfo->m_timeStamp.asCharArray());
    WriteFormat(L"MISSED=%u\n", profileInfo->m_numMisses);
    WriteFormat(L"CPUFAMILY=%lu,%lu\n", profileInfo->m_cpuFamily, profileInfo->m_cpuModel);

    // Event info
    EventEncodeVec::const_iterator eit = m_pEventVec->begin(), eitEnd = m_pEventVec->end();

    for (int i = 0; eit != eitEnd; ++eit, ++i)
    {
        // Format : Event <index>,<event+mask>,<norm>
        // Example: Event 0,118,250000.000000
        // Note: calculate event normalization here - although it was
        //      not used in current post processing
        //
        WriteFormat(L"Event %u,%u,%llu\n", i, eit->eventMask, eit->eventCount);

        if (m_evToIndexMap.find(eit->eventMask) != m_evToIndexMap.end())
        {
            return false;
        }

        m_evToIndexMap.insert(gtMap<EventMaskType, int>::value_type(eit->eventMask, i));
    }

    if (nullptr != pTopMap)
    {
        for (CoreTopologyMap::const_iterator tIt = pTopMap->begin(), tItEnd = pTopMap->end(); tIt != tItEnd; ++tIt)
        {
            // Format : TOPOLOGY,<logical core index>,<Processor>,<NUMA Node>
            // Example: TOPOLOGY,0,0,0
            WriteFormat(L"TOPOLOGY,%u,%u,%u\n", tIt->first, tIt->second.processor, tIt->second.numaNode);
        }
    }

    // write profile total
    WriteProfileTotal(procMap);

    // write end of environment section
    WriteFormat(ENV_END);
    WriteFormat(L"\n\n");
    m_fileStream.flush();

    return true;
}


bool CpuProfileWriter::WriteProfileTotal(const PidProcessMap* procMap)
{
    AggregatedSample aggSamp;

    for (PidProcessMap::const_iterator pit  = procMap->begin(), pend = procMap->end(); pit != pend; ++pit)
    {
        aggSamp.addSamples(&(pit->second));
    }

    WriteFormat(L"TOTAL=");

    CpuProfileSampleMap::const_iterator ait  = aggSamp.getBeginSample(), aend = aggSamp.getEndSample();

    while (ait != aend)
    {
        SampleKey key = ait->first;

        gtMap<EventMaskType, int>::const_iterator eit;

        if ((eit = m_evToIndexMap.find(key.event)) == m_evToIndexMap.end())
        {
            return false;
        }

        WriteFormat(L"[%u %d] %u", key.cpu, eit->second, ait->second);

        if (++ait != aend)
        {
            WriteFormat(L",");
        }
        else
        {
            WriteFormat(L"\n");
        }
    }

    return true;
}

bool CpuProfileWriter::WriteProcSectionProlog()
{
    if (!m_fileStream.isOpened())
    {
        return false;
    }

    if (m_stage != evOut_OK)
    {
        return false;
    }

    WriteFormat(PROCESSDATA_BEGIN);
    WriteFormat(L"\n");
    m_fileStream.flush();

    m_stage = evOut_TaskSummary;

    return true;
}

bool CpuProfileWriter::WriteProcSectionEpilog()
{
    if (!m_fileStream.isOpened())
    {
        return false;
    }

    if (m_stage != evOut_TaskSummary)
    {
        return false;
    }

    WriteFormat(PROCESSDATA_END);
    WriteFormat(L"\n\n");
    m_fileStream.flush();

    m_stage = evOut_OK;

    return true;
}

// Description: This function writes a line under [PROCESSDATA] section
//
// Format of the line in [PROCESSDATA] section in version 6:
//      PID,TOTALSAMPLE,#EVENTSET,[CPU INDEX] #SAMPLE,32-BIT-FLAG,CSS-FLAG,
//      MODULE-NAME
// Note: this is only for Version 6 or higher.
bool CpuProfileWriter::WriteProcLineData(const CpuProfileProcess& proc, ProcessIdType pid)
{
    if (!m_fileStream.isOpened())
    {
        return false;
    }

    if (m_stage != evOut_TaskSummary)
    {
        return false;
    }

    // PID : Note Linux uses taskID instead
    WriteFormat(L"%u", pid);

    // TOTALSAMPLE
    WriteFormat(L",%lu", proc.getTotal());

    // number of EventSet
    WriteFormat(L",%u", proc.getSampleMapSize());

    // [cpu EVENT-INDEX] #SAMPLE
    CpuProfileSampleMap::const_iterator it = proc.getBeginSample(), itEnd = proc.getEndSample();

    for (; it != itEnd; ++it)
    {
        SampleKey key = it->first;

        if (it->second)
        {
            gtMap<EventMaskType, int>::const_iterator eit;

            if ((eit = m_evToIndexMap.find(key.event)) == m_evToIndexMap.end())
            {
                return false;
            }

            WriteFormat(L",[%u %d] %u", key.cpu, eit->second, it->second);
        }
    }

    // 32-bit-Flag
    WriteFormat(L",%d", static_cast<int>(proc.m_is32Bit));

    // CSSFlag
    WriteFormat(L",%d", static_cast<int>(proc.m_hasCss));

    // task name
    WriteFormat(L",%ls\n", proc.getPath().asCharArray());

    return true;
}


bool CpuProfileWriter::WriteProcSection(const PidProcessMap* procMap)
{
    if (!WriteProcSectionProlog())
    {
        return false;
    }

    for (PidProcessMap::const_iterator it = procMap->begin(), itEnd = procMap->end(); it != itEnd; ++it)
    {
        // Only writing out the one that has samples
        if (0 != it->second.getTotal())
        {
            if (!WriteProcLineData(it->second, it->first))
            {
                return false;
            }
        }
    }

    if (!WriteProcSectionEpilog())
    {
        return false;
    }

    return true;
}


bool CpuProfileWriter::WriteModSectionProlog()
{
    if (!m_fileStream.isOpened())
    {
        return false;
    }

    if (m_stage != evOut_OK)
    {
        return false;
    }

    WriteFormat(MODDATA_BEGIN);
    WriteFormat(L"\n");
    m_fileStream.flush();

    m_stage = evOut_ModSummary;

    return true;

}
bool CpuProfileWriter::WriteModData(const CpuProfileModule& mod)
{
    if (!m_fileStream.isOpened())
    {
        return false;
    }

    if (m_stage != evOut_ModSummary)
    {
        return false;
    }

    // For each PID
    for (PidAggregatedSampleMap::const_iterator it = mod.getBeginSample(), end = mod.getEndSample(); it != end; ++it)
    {
        if (!WriteModLineData(mod, it->first, it->second))
        {
            return false;
        }
    }

    return true;
}

bool CpuProfileWriter::WriteModLineData(const CpuProfileModule& mod, ProcessIdType pid, const AggregatedSample& agSamp)
{
    if (!m_fileStream.isOpened())
    {
        return false;
    }

    if (m_stage != evOut_ModSummary)
    {
        return false;
    }

    // taskid
    WriteFormat(L"%u", pid);

    // TOTALSAMPLE
    WriteFormat(L",%llu", agSamp.getTotal());

    // #EVENTSET
    WriteFormat(L",%u", agSamp.getSampleMapSize());

    // [CPU EVENT-INDEX] #SAMPLE
    CpuProfileSampleMap::const_iterator it = agSamp.getBeginSample(), itEnd = agSamp.getEndSample();

    for (; it != itEnd; ++it)
    {
        SampleKey key = it->first;

        if (it->second)
        {
            gtMap<EventMaskType, int>::const_iterator eit;

            if ((eit = m_evToIndexMap.find(key.event)) == m_evToIndexMap.end())
            {
                return false;
            }

            WriteFormat(L",[%u %d] %u", key.cpu, eit->second, it->second);
        }
    }

    // 32-bit-Flag
    WriteFormat(L",%d", static_cast<int>(mod.m_is32Bit));

    // module name
    WriteFormat(L",%ls\n", mod.getPath().asCharArray());

    return true;
}

bool CpuProfileWriter::WriteModSectionEpilog()
{
    if (!m_fileStream.isOpened())
    {
        return false;
    }

    if (m_stage != evOut_ModSummary)
    {
        return false;
    }

    WriteFormat(MODDATA_END);
    WriteFormat(L"\n\n");
    m_fileStream.flush();

    m_stage = evOut_OK;

    return true;
}


bool CpuProfileWriter::WriteModSectionAndImd(const NameModuleMap* modMap)
{
    if (!WriteModSectionProlog())
    {
        return false;
    }

    /*
     * IMD File Naming Scheme: <index>.imd
     * The index is assigned using the order of the module
     * listed in the [MODDATA] section of TBP/EBP file.
     */
    int i = 0;

    // Write out each module
    for (NameModuleMap::const_iterator it = modMap->begin(), itEnd = modMap->end(); it != itEnd; ++it)
    {
        const CpuProfileModule& module = it->second;

        // We won't write out IMD file if module
        // contains no sample.
        if (module.getTotal() <= 0 && !module.isIndirect())
        {
            continue;
        }

        int index = module.getImdIndex();

        if (index == -1)
        {
            index = i;
        }

        // IMD file path
        osFilePath imdFilePath(m_path);
        wchar_t buf[10] = {L'\0'};
        swprintf(buf, 10, L"%u", index);
        imdFilePath.setFileName(buf);
        imdFilePath.setFileExtension(L"imd");

        // Write Module section lines
        if (!WriteModData(module))
        {
            return false;
        }

        // Write IMD file
        ImdWriter imdWriter;

        if (!imdWriter.open(imdFilePath.asString()))
        {
            return false;
        }

        if (!imdWriter.writeToFile(module, &m_evToIndexMap))
        {
            return false;
        }

        imdWriter.close();
        i++;
    }

    return WriteModSectionEpilog();
}
