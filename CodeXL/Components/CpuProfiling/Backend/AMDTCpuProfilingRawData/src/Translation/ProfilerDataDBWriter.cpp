//=====================================================================
// Copyright (c) 2013-2016 Advanced Micro Devices, Inc. All rights reserved.
//
/// \file $File: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingRawData/src/Translation/ProfilerDataDBWriter.cpp $
/// \version $Revision: $
/// \brief
//
//=====================================================================
// $Id: $
// Last checkin:   $ $
// Last edited by: $ $
// Change list:    $ $
//=====================================================================

#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>

#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>

#include <AMDTCommonProfileDataTypes.h>

#include <CpuProfileInfo.h>
#include <CpuProfileModule.h>
#include <CpuProfileProcess.h>

#include <ProfilerDataDBWriter.h>

static inline gtUInt32 generateFuncId(gtUInt16 moduleId, gtUInt16 funcSeq)
{
    gtUInt32 funcId = moduleId;
    funcId = (funcId << 16) | funcSeq;
    return funcId;
}

void ProfilerDataDBWriter::PackSessionInfo(const CpuProfileInfo& profileInfo, gtUInt64 cpuAffinity, AMDTProfileSessionInfo& sessionInfo)
{
    sessionInfo.m_targetAppPath = profileInfo.m_targetPath;
    sessionInfo.m_targetAppWorkingDir = profileInfo.m_wrkDirectory;
    sessionInfo.m_targetAppCmdLineArgs = profileInfo.m_cmdArguments;
    sessionInfo.m_targetAppEnvVars = profileInfo.m_envVariables;
    sessionInfo.m_systemDetails = profileInfo.m_osName;
    sessionInfo.m_sessionScope = profileInfo.m_profScope;
    sessionInfo.m_sessionType = profileInfo.m_profType;
    sessionInfo.m_sessionDir = profileInfo.m_profDirectory;
    sessionInfo.m_sessionStartTime = profileInfo.m_profStartTime;
    sessionInfo.m_sessionEndTime = profileInfo.m_profEndTime;
    sessionInfo.m_cssEnabled = profileInfo.m_isCSSEnabled;
    sessionInfo.m_unwindDepth = static_cast<gtUInt16>(profileInfo.m_cssUnwindDepth);
    sessionInfo.m_unwindScope = static_cast<gtUInt16>(profileInfo.m_cssScope);
    sessionInfo.m_cssFPOEnabled = profileInfo.m_isCssSupportFpo;
    sessionInfo.m_cpuFamily = profileInfo.m_cpuFamily;
    sessionInfo.m_cpuModel = profileInfo.m_cpuModel;
    sessionInfo.m_coreAffinity = cpuAffinity;
}

void ProfilerDataDBWriter::PackCoreTopology(const CoreTopologyMap& coreTopology, CPAdapterTopologyMap& cpaTopology)
{
    for (const auto& t : coreTopology)
    {
        cpaTopology.emplace_back(static_cast<gtUInt32>(t.first), t.second.processor, t.second.numaNode);
    }
}

void ProfilerDataDBWriter::DecodeSamplingEvent(EventMaskType encoded, gtUInt16& event, gtUByte& unitMask, bool& bitOs, bool& bitUsr)
{
    event = encoded & 0xFFFFU;
    unitMask = (encoded >> 16) & 0xFFU;
    bitOs = (encoded >> 25) & 1U;
    bitUsr = (encoded >> 24) & 1U;
}

bool ProfilerDataDBWriter::InitializeEventsXMLFile(gtUInt32 cpuFamily, gtUInt32 cpuModel, EventsFile& eventsFile)
{
    bool rv = true;

    osFilePath eventFilePath;
    EventEngine eventEngine;

    // Construct the path for family specific Events XML files
    if (osGetCurrentApplicationDllsPath(eventFilePath) || osGetCurrentApplicationPath(eventFilePath))
    {
        eventFilePath.clearFileName();
        eventFilePath.clearFileExtension();

        eventFilePath.appendSubDirectory(L"Data");
        eventFilePath.appendSubDirectory(L"Events");

        const gtString eventFilePathStr = eventFilePath.fileDirectoryAsString();

        if (!eventEngine.Initialize(QString::fromWCharArray(eventFilePathStr.asCharArray(), eventFilePathStr.length())))
        {
            rv = false;
        }
    }

    if (rv)
    {
        // Get event file path
        QString eventFile;
        eventFile = eventEngine.GetEventFilePath(cpuFamily, cpuModel);

        // Initialize event file
        if (!eventsFile.Open(eventFile))
        {
            rv = false;
        }
    }

    return rv;
}

gtString ProfilerDataDBWriter::ConvertQtToGTString(const QString& inputStr)
{
    gtString str;
    str.resize(inputStr.length());

    if (inputStr.length())
    {
        str.resize(inputStr.toWCharArray(&str[0]));
    }

    return str;
}

void ProfilerDataDBWriter::PackSamplingEvents(const CpuProfileInfo& profileInfo,
                                              AMDTProfileCounterDescVec& events,
                                              AMDTProfileSamplingConfigVec& samplingConfigs)
{
    EventsFile eventsFile;
    bool eventsFileAvbl = InitializeEventsXMLFile(profileInfo.m_cpuFamily, profileInfo.m_cpuModel, eventsFile);

    EventEncodeVec profileEvents = profileInfo.m_eventVec;

    for (const auto& samplingEvent : profileEvents)
    {
        gtUInt16 eventSel;
        gtUByte unitMask;
        bool user;
        bool os;

        AMDTProfileCounterDesc counterDesc;
        AMDTProfileSamplingConfig samplingConfig;

        DecodeSamplingEvent(samplingEvent.eventMask, eventSel, unitMask, os, user);

        // Use eventMask as id, clear the unused 6 MSBs in the eventMask value
        samplingConfig.m_id = samplingEvent.eventMask & 0x3FFFFFF;
        samplingConfig.m_hwEventId = eventSel;
        samplingConfig.m_unitMask = unitMask;
        samplingConfig.m_osMode = os;
        samplingConfig.m_userMode = user;
        samplingConfig.m_samplingInterval = samplingEvent.eventCount;

        counterDesc.m_hwEventId = eventSel;
        CpuEvent* pCpuEvent;
        gtString eventName;

        if (IsTimerEvent(eventSel))
        {
            counterDesc.m_name = L"Timer";
            counterDesc.m_description = L"Timer";
        }
        else if (eventsFileAvbl && eventsFile.FindEventByValue(eventSel, &pCpuEvent))
        {
            counterDesc.m_name = ConvertQtToGTString(pCpuEvent->name);
            counterDesc.m_description = ConvertQtToGTString(pCpuEvent->description);
        }
        else
        {
            counterDesc.m_name = L"Unknown";
            counterDesc.m_description = L"Unknown";
        }

        samplingConfigs.emplace_back(samplingConfig);

        // This can have duplicate events? But will be handled while inserting into db
        events.emplace_back(counterDesc);
    }

    eventsFile.Close();
}

void ProfilerDataDBWriter::PackProcessInfo(const PidProcessMap& processMap, CPAProcessList& processList)
{
    for (auto& pit : processMap)
    {
        processList.emplace_back(pit.first, pit.second.getPath(), pit.second.m_is32Bit);
    }
}

void ProfilerDataDBWriter::PackModuleInfo(const NameModuleMap& modMap, CPAModuleList& moduleList)
{
    for (auto& m : modMap)
    {
        moduleList.emplace_back(m.second.m_moduleId,
                                m.first,
                                m.second.m_size,
                                m.second.m_modType,
                                osIsSystemModule(m.first),
                                m.second.m_is32Bit,
                                m.second.m_isDebugInfoAvailable);
    }
}

void ProfilerDataDBWriter::PackModuleInstanceInfo(const NameModuleMap& modMap, const gtHashMap<gtUInt32, std::tuple<gtString, gtUInt64, gtUInt64>>& modInstanceInfoMap, CPAModuleInstanceList& moduleInstanceList)
{
    // modInstanceInfo : vector of <instanceId, modName, pid, loadAddr>
    for (const auto& modIt : modInstanceInfoMap)
    {
        gtUInt32 moduleId = 0;
        const auto& it = modMap.find(std::get<0>(modIt.second));

        if (it != modMap.end())
        {
            // Insert into DB only if the module has samples
            moduleId = it->second.m_moduleId;
            moduleInstanceList.emplace_back(modIt.first, moduleId, std::get<1>(modIt.second), std::get<2>(modIt.second));
        }
    }
}

void ProfilerDataDBWriter::PackProcessThreadInfo(const gtVector<std::tuple<gtUInt32, gtUInt32>>& processThreadList, CPAProcessThreadList& procThreadIdList)
{
    for (const auto& it : processThreadList)
    {
        gtUInt32 processId = 0;
        gtUInt32 threadId = 0;
        std::tie(processId, threadId) = it;

        gtUInt64 ptId = processId;
        ptId = (ptId << 32) | threadId;

        procThreadIdList.emplace_back(ptId, static_cast<gtUInt64>(processId), threadId);
    }
}

void ProfilerDataDBWriter::PackCoreSamplingConfigInfo(const NameModuleMap& modMap, CPACoreSamplingConfigList& coreConfigList)
{
    gtHashMap<gtUInt64, std::pair<gtUInt16, gtUInt32>> configs;
    gtUInt32 unusedBitsMask = 0x3FFFFFF;

    for (auto& m : modMap)
    {
        for (auto sit = m.second.getBeginSample(); sit != m.second.getEndSample(); ++sit)
        {
            for (auto cit = sit->second.getBeginSample(); cit != sit->second.getEndSample(); ++cit)
            {
                gtUInt64 id = cit->first.cpu;
                id = (id << 32) | (cit->first.event & unusedBitsMask);
                configs.emplace(id, std::make_pair(static_cast<gtUInt16>(cit->first.cpu), cit->first.event));
            }
        }
    }

    for (auto& it : configs)
    {
        coreConfigList.emplace_back(it.first, it.second.first, it.second.second);
    }
}

void ProfilerDataDBWriter::PackSampleInfo(const NameModuleMap& modMap, CPASampeInfoList& sampleList)
{
    gtUInt32 unusedBitsMask = 0x3FFFFFF;

    for (auto& module : modMap)
    {
        // TODO: module load address should not come from NameModuleMap as
        // this map keeps only one loadAddress for each unique module
        gtVAddr modLoadAddr = module.second.getBaseAddr();

        for (auto fit = module.second.getBeginFunction(); fit != module.second.getEndFunction(); ++fit)
        {
            gtUInt32 functionId = fit->second.m_functionId;

            for (auto aptIt = fit->second.getBeginSample(); aptIt != fit->second.getEndSample(); ++aptIt)
            {
                // sample address offset is w.r.t. module load address
                gtUInt64 sampleOffset = aptIt->first.m_addr - modLoadAddr;
                gtUInt64 pid = aptIt->first.m_pid;
                gtUInt32 threadId = aptIt->first.m_tid;
                gtUInt32 moduleInstanceid = 0ULL;

                for (const auto& it : module.second.m_moduleInstanceInfo)
                {
                    if (std::get<0>(it) == pid && std::get<1>(it) == modLoadAddr)
                    {
                        moduleInstanceid = std::get<2>(it);
                        break;
                    }
                }

                gtUInt64 processThreadId = pid;
                processThreadId = (processThreadId << 32) | threadId;

                for (auto skIt = aptIt->second.getBeginSample(); skIt != aptIt->second.getEndSample(); ++skIt)
                {
                    gtUInt64 sampleCount = skIt->second;
                    gtUInt64 coreSamplingConfigId = skIt->first.cpu;
                    coreSamplingConfigId = (coreSamplingConfigId << 32) | (skIt->first.event & unusedBitsMask);

                    sampleList.emplace_back(processThreadId, moduleInstanceid, coreSamplingConfigId, functionId, sampleOffset, sampleCount);
                }
            }
        }
    }
}


void ProfilerDataDBWriter::PackFunctionInfo(const NameModuleMap& modMap, CPAFunctionInfoList& funcInfoList)
{
    // Insert a dummy function as "Unknown Function"
    gtString unknownFuncName = L"Unknown Function";
    funcInfoList.emplace_back(UNKNOWN_FUNCTION_ID, UNKNOWN_MODULE_ID, unknownFuncName, 0, 0);

    for (auto& module : modMap)
    {
        gtUInt32 modId = module.second.m_moduleId;
        gtUInt64 modLoadAddr = module.second.getBaseAddr();

        for (auto fit = module.second.getBeginFunction(); fit != module.second.getEndFunction(); ++fit)
        {
            gtString funcName = fit->second.getFuncName();
            gtUInt64 startOffset = fit->second.getBaseAddr() - modLoadAddr;
            gtUInt64 size = fit->second.getSize();
            gtUInt32 funcId = generateFuncId(modId, fit->second.m_functionId);

            // If function name is empty, it will be considered as unknown-function, don't insert into DB.
            // else insert function info into DB
            if (!funcName.isEmpty())
            {
                funcInfoList.emplace_back(funcId, modId, funcName, startOffset, size);
            }

#if 0
            // if function name is empty, generate a stub function name "modulename+addr"
            const gtString& modulePath = module.first;

            if (funcName.isEmpty())
            {
                gtString modName = modulePath;
                int startPos = modulePath.findLastOf(osFilePath::osPathSeparator);

                if (-1 != startPos)
                {
                    modulePath.getSubString(startPos + 1, modulePath.length(), modName);
                }

                funcName.appendFormattedString(L"%s+0x%lx", modName.asCharArray(), fit->second.getBaseAddr());
            }

#endif
        }
    }
}

bool ProfilerDataDBWriter::Write(
    const gtString& path,
    CpuProfileInfo& profileInfo,
    gtUInt64 cpuAffinity,
    const PidProcessMap& procMap,
    gtVector<std::tuple<gtUInt32, gtUInt32>>& processThreadList,
    const NameModuleMap& modMap,
    const gtHashMap<gtUInt32, std::tuple<gtString, gtUInt64, gtUInt64>>& modInstanceInfoMap,
    const CoreTopologyMap* pTopMap)
{
    if (m_pCpuProfDbAdapter != nullptr)
    {
        // Remove trailing '.ebp' from path
        gtString dbPath = path;

        if (dbPath.endsWith(gtString(L".ebp")))
        {
            int pos = dbPath.reverseFind(L".");
            dbPath.extruct(pos, dbPath.length());
        }

        gtString dbExtn;
        m_pCpuProfDbAdapter->GetDbFileExtension(dbExtn);
        dbPath.append(dbExtn);
        m_pCpuProfDbAdapter->CreateDb(dbPath, AMDT_PROFILE_MODE_AGGREGATION);

        AMDTProfileSessionInfo sessionInfo;
        PackSessionInfo(profileInfo, cpuAffinity, sessionInfo);
        m_pCpuProfDbAdapter->InsertSessionInfo(sessionInfo);
        sessionInfo.Clear();

        if (pTopMap != nullptr)
        {
            CPAdapterTopologyMap topMap;
            PackCoreTopology(*pTopMap, topMap);
            m_pCpuProfDbAdapter->InsertTopology(topMap);
            topMap.clear();
        }

        AMDTProfileCounterDescVec eventVec;
        AMDTProfileSamplingConfigVec samplingVec;
        PackSamplingEvents(profileInfo, eventVec, samplingVec);

        m_pCpuProfDbAdapter->InsertSamplingEvents(eventVec, samplingVec);
        eventVec.clear();
        samplingVec.clear();

        CPAProcessList processList;
        PackProcessInfo(procMap, processList);
        m_pCpuProfDbAdapter->InsertProcessInfo(processList);
        processList.clear();

        CPAModuleList moduleList;
        PackModuleInfo(modMap, moduleList);
        m_pCpuProfDbAdapter->InsertModuleInfo(moduleList);
        moduleList.clear();

        CPAModuleInstanceList modInstanceList;
        PackModuleInstanceInfo(modMap, modInstanceInfoMap, modInstanceList);
        m_pCpuProfDbAdapter->InsertModuleInstanceInfo(modInstanceList);
        modInstanceList.clear();

        CPAProcessThreadList procThreadIdList;
        PackProcessThreadInfo(processThreadList, procThreadIdList);
        m_pCpuProfDbAdapter->InsertProcessThreadInfo(procThreadIdList);
        procThreadIdList.clear();

        CPACoreSamplingConfigList coreConfigList;
        PackCoreSamplingConfigInfo(modMap, coreConfigList);
        m_pCpuProfDbAdapter->InsertCoreSamplingConfigInfo(coreConfigList);
        coreConfigList.clear();

        CPAFunctionInfoList funcList;
        PackFunctionInfo(modMap, funcList);
        m_pCpuProfDbAdapter->InsertFunctionInfo(funcList);
        funcList.clear();

        CPASampeInfoList sampleList;
        PackSampleInfo(modMap, sampleList);
        m_pCpuProfDbAdapter->InsertSamples(sampleList);
        sampleList.clear();

        m_pCpuProfDbAdapter->CloseDb();
    }

    return true;
}