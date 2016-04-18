//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileReader.cpp
///
//==================================================================================

#include <CpuProfileReader.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include "ImdReader.h"

CpuProfileReader::CpuProfileReader() : m_isProcessDataRead(false), m_isModDataRead(false)
{
}

CpuProfileReader::~CpuProfileReader()
{
    close();
}

bool CpuProfileReader::open(const gtString& path)
{
    if (!CpuProfileInputStream::open(path))
    {
        return false;
    }

    if (!checkVersion())
    {
        return false;
    }

    if (m_profileInfo.m_tbpVersion > TBPVER_BEFORE_RI)
    {
        if (!readRunInfoSection())
        {
            m_profileInfo.m_tbpVersion = TBPVER_UNKNOWN;
            return false;
        }
    }

    if (!readEnvSection())
    {
        m_profileInfo.m_tbpVersion = TBPVER_UNKNOWN;
        return false;
    }

    return true;
}

void CpuProfileReader::close()
{
    CpuProfileInputStream::close();

    // Mohit: Ideally m_profileInfo should be a pointer to CpuProfileInfo and that should be deleted here
    // CpuProfileInfo destructor should take care of all the clean up
    // For now, clearing event vector here (solving Bug 409947)
    m_profileInfo.m_eventVec.clear();
}

CpuProfileInfo* CpuProfileReader::getProfileInfo()
{
    return (m_profileInfo.m_tbpVersion >= TBPVER_MIN) ? &m_profileInfo : NULL;
}


bool CpuProfileReader::checkVersion()
{
    gtString line;

    if (!resetStream())
    {
        return false;
    }

    if (readLine(line) < 0)
    {
        return false;
    }

    if (isEof() || line.isEmpty())
    {
        return false;
    }

    int tmp = line.find(L"TBPFILEVERSION");

    if (tmp == -1 || tmp != 0)
    {
        return false;
    }

    gtString str;

    if (!section(str, line, L"=", 1) || !str.toUnsignedIntNumber(m_profileInfo.m_tbpVersion))
    {
        return false;
    }

    if (m_profileInfo.m_tbpVersion < TBPVER_MIN ||  m_profileInfo.m_tbpVersion > TBPVER_DEFAULT)
    {
        m_profileInfo.m_tbpVersion = TBPVER_UNKNOWN;
        return false;
    }

    return true;
}

bool CpuProfileReader::readRunInfoSection()
{
    gtString line;
    bool ret = true;

    if (!resetStream())
    {
        return false;
    }

    /***********************************************************
     * This section parses all lines from [RUNINFO] to [END] or the EOF
     * or blank line whichever one comes first.
     */
    do
    {
        if (readLine(line) < 0)
        {
            return false;
        }

    }
    while (line.find(RUNINFO_BEGIN) == -1 &&  !isEof());

    markPos(RUNINFO_BEGIN);

    // Process [RUNINFO] section
    do
    {
        if (readLine(line) < 0)
        {
            return false;
        }

        if (line.find(RUNINFO_END) != -1 || line.isEmpty() || line[0] == L'\0')
        {
            break;
        }

        processRunInfoLine(line);

    }
    while (!isEof());

    //***********************************************************

    if (isEof())
    {
        ret = false;
    }

    return ret;
}

void CpuProfileReader::processRunInfoLine(const gtString& line)
{
    if (line.find(L"TARGETPATH") != -1)
    {
        section(m_profileInfo.m_targetPath, line, L"=", 1);
    }
    else if (line.find(L"WORKINGDIR") != -1)
    {
        section(m_profileInfo.m_wrkDirectory, line, L"=", 1);
    }
    else if (line.find(L"CMDARGUMENTS") != -1)
    {
        section(m_profileInfo.m_cmdArguments, line, L"=", 1);
    }
    else if (line.find(L"ENVVARIABLES") != -1)
    {
        section(m_profileInfo.m_envVariables, line, L"=", 1);
    }
    else if (line.find(L"PROFILETYPE") != -1)
    {
        section(m_profileInfo.m_profType, line, L"=", 1);
    }
    else if (line.find(L"PROFILEDIR") != -1)
    {
        section(m_profileInfo.m_profDirectory, line, L"=", 1);
    }
    else if (line.find(L"STARTTIME") != -1)
    {
        section(m_profileInfo.m_profStartTime, line, L"=", 1);
    }
    else if (line.find(L"ENDTIME") != -1)
    {
        section(m_profileInfo.m_profEndTime, line, L"=", 1);
    }
    else if (line.find(L"CSSENABLED") != -1)
    {
        gtString cssEnabled;
        section(cssEnabled, line, L"=", 1);

        if (L"ENABLED" == cssEnabled)
        {
            m_profileInfo.m_isCSSEnabled = true;
        }
        else if (L"DISABLED" == cssEnabled)
        {
            m_profileInfo.m_isCSSEnabled = false;
        }
    }
    else if (line.find(L"CSSUNWINDDEPTH") != -1)
    {
        gtString str;
        section(str, line, L"=", 1);
        str.toUnsignedIntNumber(m_profileInfo.m_cssUnwindDepth);
    }
    else if (line.find(L"CSSSCOPE") != -1)
    {
        gtString str;
        section(str, line, L"=", 1);
        unsigned int cssScope = (unsigned int)(-1);
        str.toUnsignedIntNumber(cssScope);
        m_profileInfo.m_cssScope = static_cast<CpuProfileCssScope>(cssScope);
    }
    else if (line.find(L"CSSFPO") != -1)
    {
        gtString cssFpo;
        section(cssFpo, line, L"=", 1);

        if (L"YES" == cssFpo)
        {
            m_profileInfo.m_isCssSupportFpo = true;
        }
        else if (L"NO" == cssFpo)
        {
            m_profileInfo.m_isCssSupportFpo = false;
        }
    }
    else if (line.find(L"PROFILINGCLU") != -1)
    {
        gtString profilingClu;
        section(profilingClu, line, L"=", 1);

        if (L"YES" == profilingClu)
        {
            m_profileInfo.m_isProfilingCLU = true;
        }
        else if (L"NO" == profilingClu)
        {
            m_profileInfo.m_isProfilingCLU = false;
        }
    }
    else if (line.find(L"OSNAME") != -1)
    {
        section(m_profileInfo.m_osName, line, L"=", 1);
    }
    else if (line.find(L"PROFILESCOPE") != -1)
    {
        section(m_profileInfo.m_profScope, line, L"=", 1);
    }
}

bool CpuProfileReader::readEnvSection()
{
    gtString line;
    bool ret = true;

    if (!resetStream())
    {
        return false;
    }

    /***********************************************************
     * This section parses all lines from [ENV] to [END] or the EOF
     * or blank line whichever one comes first.
     */
    do
    {
        if (readLine(line) < 0)
        {
            return false;
        }

    }
    while (line.find(ENV_BEGIN) == -1
           &&  !isEof());

    markPos(ENV_BEGIN);

    // Process [ENV] section
    do
    {
        if (readLine(line) < 0)
        {
            return false;
        }

        if (line.find(ENV_END) != -1 || line.isEmpty() || line[0] == L'\0')
        {
            break;
        }

        processEnvLine(line);

    }
    while (!isEof());

    //If there was no topology information, assume 1 node == 1 core
    if (0 == m_profileInfo.m_numNodes)
    {
        m_profileInfo.m_numNodes = m_profileInfo.m_numCpus;
    }

    //***********************************************************

    if (isEof())
    {
        ret = false;
    }

    return ret;
}


void CpuProfileReader::processEnvLine(const gtString& line)
{
    gtString tmp = L"";

    if (line.find(L"CPUFAMILY") != -1)
    {
        switch (m_profileInfo.m_tbpVersion)
        {
            case 6:
            {
                gtString str;
                section(str, line, L"=", 1);
                str.toUnsignedLongNumber(m_profileInfo.m_cpuFamily);
            }
            break;

            default:
            {
                gtString strFull, strVal;
                section(strFull, line, L"=", 1);

                section(strVal, strFull, L",", 0, 1);
                strVal.toUnsignedLongNumber(m_profileInfo.m_cpuFamily);

                section(strVal, strFull, L",", 1);
                strVal.toUnsignedLongNumber(m_profileInfo.m_cpuModel);
            }
            break;
        }
    }
    else if (line.find(L"CPU") != -1)
    {
        gtString str;
        section(str, line, L"=", 1);
        str.toUnsignedIntNumber(m_profileInfo.m_numCpus);
    }
    else if (line.find(L"NumEvents") != -1)
    {
        gtString str;
        section(str, line, L"=", 1);
        str.toUnsignedLongNumber(m_profileInfo.m_numEvents);
    }
    else if (line.find(L"MODULES") != -1)
    {
        gtString str;
        section(str, line, L"=", 1);
        str.toUnsignedIntNumber(m_profileInfo.m_numModules);
    }
    else if (line.find(L"SAMPS") != -1)
    {
        gtString str;
        section(str, line, L"=", 1);
        str.toUnsignedIntNumber(m_profileInfo.m_numSamples);
    }
    else if (line.find(L"Event") != -1)
    {
        gtString str;
        // FIXME: Should support float for older version!!!!!
        // format : Event <index>,<event+mask>, <norm>
        // Example: Event 0,64,250000.000
        // Note   : at this point, we are not using the norm in
        //          the calculation.
        EventEncodeType event;
        EventMaskTypeEnc encoded;
        section(str, line, L",", 1, 2);
        str.toUnsignedIntNumber(encoded.encodedEvent);

        //if usr and os aren't specified, assume true
        if ((0 == (encoded.bitOsEvents & 1)) && (0 == (encoded.bitUsrEvents & 1)))
        {
            encoded.bitOsEvents = encoded.bitUsrEvents = 1;
        }

        section(str, line, L",", 2);
        unsigned long long tmpVal = 0ULL;
        str.toUnsignedLongLongNumber(tmpVal);
        event.eventCount = tmpVal;
        event.eventMask = encoded.encodedEvent;
        event.sortedIndex = m_profileInfo.m_eventVec.size();
        m_profileInfo.m_eventVec.push_back(event);
    }
    else if (line.find(L"TIMESTAMP") != -1)
    {
        section(m_profileInfo.m_timeStamp, line, L"=", 1);
    }
    else if (line.find(L"TOTAL") != -1)
    {
        int k = 0;
        gtString eventsStr;
        section(eventsStr, line, L"=", 1);
        parseEvents(eventsStr, k, m_profileInfo.m_eventVec, m_profileInfo.m_totalMap);
    }
    else if (line.find(L"TOPOLOGY") != -1)
    {
        gtString str;
        // Format : TOPOLOGY,<logical core index>,<Processor>,<NUMA Node>
        unsigned int node;
        section(str, line, L",", 3);
        str.toUnsignedIntNumber(node);

        unsigned int core;
        section(str, line, L",", 1, 2);
        str.toUnsignedIntNumber(core);

        CoreTopology topology = { static_cast<gtUInt16>(core), static_cast<gtUInt16>(node) };

        unsigned int idx;
        section(str, line, L",", 1, 2);
        str.toUnsignedIntNumber(idx);

        m_topMap.insert(CoreTopologyMap::value_type(idx, topology));

        if (m_profileInfo.m_numNodes <= node)
        {
            m_profileInfo.m_numNodes = node + 1;
        }
    }
}

/*
 * Description: This function process each line withing [PROCESSDATA] section
 *
 * Format of each line in [PROCESSDATA] section in version 6:
 *       PID,TOTALSAMPLE,#EVENTSET,[CPU INDEX] #SAMPLE,32-BIT-FLAG,CSS-FLAG,
 *              MODULE-NAME
 *
 * Note: this is only for version 6 or higher.
 * Note: the MODULE-NAME must be the last item in the line, in case the path
 *  contains a delimiter.
 */
void CpuProfileReader::processProcLine(PidProcessMap& procMap, const gtString& line)
{
    int k = 0;
    CpuProfileProcess proc;

    // Get PID
    ProcessIdType pid = 0;

    if (!parsePid(line, k, pid))
    {
        return;
    }

    // NOTE: We should not use this value. Instead,
    //       we uses the aggregated value. (Suravee)
    // Get TOTALSAMPLE
    gtUInt64 total;

    if (!parseTotalSample(line, k, total))
    {
        return;
    }

    // Get num event set
    unsigned long numEventSet;

    if (!parseNumEventSet(line, k, numEventSet))
    {
        return;
    }

    // Get Events
    CpuProfileSampleMap samples;

    if (!parseEvents(line, k, m_profileInfo.m_eventVec, samples))
    {
        return;
    }

    proc.addSamples(&samples);

    // Get 32-BIT-FLAG
    if (!parseIs32Bit(line, k, proc.m_is32Bit))
    {
        return;
    }

    // Get CSS-FLAG
    if (!parseHasCss(line, k, proc.m_hasCss))
    {
        return;
    }

    // Get MODULE-NAME
    //Note that this must be the last item in the line, in case the path
    //  contains a delimiter.
    gtString tmp;

    if (!parseWstring(line, k, tmp, true))
    {
        return;
    }

    proc.setPath(tmp);

    procMap.insert(PidProcessMap::value_type(pid, proc));
}

/*
 * Description: This function process each line withing [MODDATA] section
 *
 * Format of each line in [MODDATA] section in version 3:
 *       TOTALSAMPLE,[CPU INDEX]#SAMPLE,32-BIT-SAMPLE,64-BIT-SAMPLE,MODULE-NAME
 *
 * Format of each line in [MODDATA] section in version 6:
 *       PID,TOTALSAMPLE,#EVENTSET,[CPU INDEX] #SAMPLE,32-BIT-FLAG,MODULE-NAME
 * Note: the MODULE-NAME must be the last item in the line, in case the path
 *  contains a delimiter.
 */
void CpuProfileReader::processModLine(NameModuleMap& modMap, const gtString& line)
{
    int k = 0;
    CpuProfileModule mod;

    // Get PID
    ProcessIdType pid = 0;

    if (m_profileInfo.m_tbpVersion >= 6)
    {
        // Only for version 6 or higher
        if (!parsePid(line, k, pid))
        {
            return;
        }
    }

    // NOTE: We should not use this value. Instead,
    //       we uses the aggregated value. (Suravee)
    // Get TOTALSAMPLE
    gtUInt64 total;

    if (!parseTotalSample(line, k, total))
    {
        return;
    }

    // Pass the #evnetSet part; for version 6 or later;
    unsigned long numEventSet = 0UL;

    if (m_profileInfo.m_tbpVersion >= 6)
    {
        if (!parseNumEventSet(line, k, numEventSet))
        {
            return;
        }
    }

    // Get Events
    CpuProfileSampleMap sampMap;

    if (0UL != numEventSet && !parseEvents(line, k, m_profileInfo.m_eventVec, sampMap))
    {
        return;
    }

    AggregatedSample agSamp;
    agSamp.addSamples(&sampMap);

    // Get is 32bit
    if (!parseIs32Bit(line, k, mod.m_is32Bit))
    {
        return;
    }

    // Get MODULE-NAME
    //Note that this must be the last item in the line, in case the path
    //  contains a delimiter.
    gtString modPath;

    if (!parseWstring(line, k, modPath, true))
    {
        return;
    }

    NameModuleMap::iterator mit = modMap.find(modPath);

    if (mit == modMap.end())
    {
        int imdIndex = modMap.size();

        if (0ULL != agSamp.getTotal())
        {
            mod.insertModMetaData(modPath, imdIndex, pid, agSamp);
        }

        modMap.insert(NameModuleMap::value_type(modPath, mod));
    }
    else
    {
        mit->second.insertModMetaData(pid, agSamp);
    }

}


PidProcessMap* CpuProfileReader::getProcessMap()
{
    PidProcessMap* pProcMap = NULL;

    if (isOpen())
    {
        if (!m_isProcessDataRead)
        {
            m_isProcessDataRead = true;

            gtString line;
            fpos_t pos;

            if (!getPos(PROCESSDATA_BEGIN, &pos))
            {
                resetStream();

                /// Locate the PROCESSDATA section
                while (readLine(line) > 0)
                {
                    if (line.find(PROCESSDATA_BEGIN) != -1)
                    {
                        markPos(PROCESSDATA_BEGIN);
                        break;
                    }

                    if (isEof())
                    {
                        break;
                    }
                }
            }
            else
            {
                getCurrentPosition(&pos);
            }

            // Parse all the line in between [PROCESSDATA]
            while (!isEof() && readLine(line) > 0)
            {
                // Find [END], end of section, update position; STOP;
                if (line.find(PROCESSDATA_END) != -1)
                {
                    break;
                }

                processProcLine(m_procMap, line);
            }
        }

        if (m_procMap.size() != 0)
        {
            pProcMap = &m_procMap;
        }
    }

    return pProcMap;
}

NameModuleMap* CpuProfileReader::getModuleMap()
{
    NameModuleMap* pModMap = NULL;

    if (isOpen())
    {
        if (!m_isModDataRead)
        {
            m_isModDataRead = true;

            gtString line;
            fpos_t pos;

            if (!getPos(MODDATA_BEGIN, &pos))
            {
                resetStream();

                /// Locate the MODDATA section
                while (readLine(line) > 0)
                {
                    if (line.find(MODDATA_BEGIN) != -1)
                    {
                        markPos(MODDATA_BEGIN);
                        break;
                    }

                    if (isEof())
                    {
                        break;
                    }
                }
            }
            else
            {
                getCurrentPosition(&pos);
            }

            // Parse all the line in between [MODDATA]
            while (!isEof() && readLine(line) > 0)
            {
                // Find [END], end of section, update position; STOP;
                if (line.find(MODDATA_END) != -1)
                {
                    pModMap = &m_modMap;
                    break;
                }

                processModLine(m_modMap, line);
            }
        }

        if (m_modMap.size() != 0)
        {
            pModMap = &m_modMap;
        }
    }

    return pModMap;
}

CoreTopologyMap* CpuProfileReader::getTopologyMap()
{
    if (!isOpen())
    {
        return NULL;
    }

    if (m_topMap.size() != 0)
    {
        return &m_topMap;
    }

    return NULL;
}

CpuProfileModule* CpuProfileReader::getModuleDetail(const gtString& modName)
{
    if (modName.isEmpty())
    {
        return NULL;
    }

    if (!isOpen())
    {
        return NULL;
    }

    if (m_modMap.size() == 0 && getModuleMap() == NULL)
    {
        return NULL;
    }

    NameModuleMap::iterator mit = m_modMap.find(modName);

    if (mit == m_modMap.end())
    {
        return NULL;
    }

    if (mit->second.m_isImdRead)
    {
        return (&(mit->second));
    }

    if (m_profileInfo.m_tbpVersion >= 10)
    {
        if (!getImd(mit->second))
        {
            return NULL;
        }
    }
    else
    {
        /* NOTE: This is for backward compatibility (prior 10)
         *       Basically, everything is in one file
         */
        ImdReader imdRd(&m_profileInfo);
        resetStream();
        imdRd.setStream(this);

        if (!imdRd.readModSection(mit->second))
        {
            return NULL;
        }
    }

    mit->second.m_isImdRead = true;

    return (&(mit->second));
}


bool CpuProfileReader::getImd(CpuProfileModule& mod)
{
    bool bRet = true;

    if (m_profileInfo.m_tbpVersion < 10)
    {
        return false;
    }

    ImdReader imdRd(&m_profileInfo);

    // IMD file path
    osFilePath imdFilePath(m_path);
    wchar_t buf[10] = { L'\0' };
    swprintf(buf, 10, L"%u", mod.getImdIndex());
    imdFilePath.setFileName(buf);
    imdFilePath.setFileExtension(L"imd");

    if (!imdRd.open(imdFilePath.asString()))
    {
        return false;
    }

    if (!imdRd.readModSection(mod))
    {
        bRet = false;
    }

    imdRd.close();

    return bRet;
}
