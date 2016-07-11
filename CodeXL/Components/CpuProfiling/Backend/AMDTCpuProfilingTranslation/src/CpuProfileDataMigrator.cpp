//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileDataMigrator.cpp
/// \brief EBP file format to CXLDB file migration class.
///
//==================================================================================

#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTCpuProfilingRawData/inc/CpuProfileReader.h>
#include <AMDTCpuCallstackSampling/inc/CssReader.h>
#include <CpuProfileDataMigrator.h>

gtString DataMigrator::GetSourceFilePath()
{
    return m_sourceFilePath.asString(true);
}

gtString DataMigrator::GetTargetFilePath()
{
    return m_targetFilePath.asString(true);
}

bool DataMigrator::Migrate(bool deleteSrcFiles)
{
    bool ret = false;

    ret = doValidate();
    ret = ret && doMigrate();

    if (ret && deleteSrcFiles)
    {
        ret = doDeleteSrcFiles();
    }

    return ret;
}

bool DataMigrator::doValidate()
{
    bool rc = false;
    gtString inFileExt;
    m_sourceFilePath.getFileExtension(inFileExt);

    // Input file type should be EBP
    if (inFileExt == SourceFileExt)
    {
        // File should be present
        if (m_sourceFilePath.exists())
        {
            rc = true;
        }
    }

    if (rc)
    {
        if (m_targetFilePath.exists())
        {
            osFile oldFile(m_targetFilePath);
            rc = oldFile.deleteFile();
        }
    }

    return rc;
}

bool DataMigrator::doMigrate()
{
    bool rc = false;

    CpuProfileInfo* pProfileInfo = nullptr;
    CoreTopologyMap* pTopMap = nullptr;
    PidProcessMap* pProcInfo = nullptr;
    NameModuleMap* pModMap = nullptr;

    CpuProfileReader profileReader;
    rc = profileReader.open(m_sourceFilePath.asString(true));

    if (rc)
    {
        pProfileInfo = profileReader.getProfileInfo();

        if (nullptr == pProfileInfo)
        {
            rc = false;
        }
    }

    if (rc)
    {
        pTopMap = profileReader.getTopologyMap();

        if (nullptr == pTopMap)
        {
            rc = false;
        }
    }

    if (rc)
    {
        pProcInfo = profileReader.getProcessMap();

        if (nullptr == pProcInfo)
        {
            rc = false;
        }
    }

    if (rc)
    {
        pModMap = profileReader.getModuleMap();

        if (nullptr == pModMap)
        {
            rc = false;
        }
    }

    if (rc)
    {
        for (auto& modIt : *pModMap)
        {
            gtUInt32 nextFunctionId = 1;

            // Read IMD file
            profileReader.getModuleDetail(modIt.first);

            // Populate module ID. Use module index as module id.
            // Module index is zero based, add 1 to it.
            modIt.second.m_moduleId = modIt.second.getImdIndex() + 1;

            // Populate function ID
            for (auto funcIt = modIt.second.getBeginFunction(); funcIt != modIt.second.getEndFunction(); ++funcIt)
            {
                funcIt->second.m_functionId = nextFunctionId++;
            }
        }
    }

    if (rc)
    {
        RunInfo runInfo;

        osFilePath riFilePath = m_sourceFilePath;
        riFilePath.setFileExtension(L"ri");
        fnReadRIFile(riFilePath.asString().asCharArray(), &runInfo);

        m_dbWriter.reset(new ProfilerDataDBWriter);

        if (m_dbWriter)
        {
            rc = m_dbWriter->Initialize(m_targetFilePath.asString(true));

            if (!rc)
            {
                m_dbWriter.reset(nullptr);
            }
        }

        if (m_dbWriter)
        {
            WriteSessionInfoIntoDB(*pProfileInfo, runInfo);
            WriteTopologyInfoIntoDB(*pTopMap);
            WriteSamplingEventInfoIntoDB(pProfileInfo->m_eventVec);
            WriteSamplingConfigInfoIntoDB(pProfileInfo->m_eventVec);
            WriteCoreSamplingConfigInfoIntoDB(pProfileInfo->m_eventVec, runInfo);
            WriteProcessInfoIntoDB(*pProcInfo);
            WriteThreadInfoIntoDB(*pModMap);
            WriteModuleInfoIntoDB(*pModMap);
            WriteModuleInstanceInfoIntoDB(*pModMap);
            WriteFunctionInfoIntoDB(*pModMap);
            WriteSampleProfileDataIntoDB(*pModMap);
            //WriteCallgraphProfileDataIntoDB(*pProcInfo, *pModMap);
            WriteJitInfoIntoDB(*pModMap);
            WriteFinish();
        }
    }

    profileReader.close();
    return rc;
}

bool DataMigrator::WriteSessionInfoIntoDB(const CpuProfileInfo& profileInfo, const RunInfo& runInfo)
{
    // Populate profile session info
    AMDTProfileSessionInfo *info = new (std::nothrow) AMDTProfileSessionInfo;

    if (nullptr != info)
    {
        info->m_cpuFamily = profileInfo.m_cpuFamily;
        info->m_cpuModel = profileInfo.m_cpuModel;
        info->m_targetAppPath = profileInfo.m_targetPath;
        info->m_targetAppWorkingDir = profileInfo.m_wrkDirectory;
        info->m_targetAppCmdLineArgs = profileInfo.m_cmdArguments;
        info->m_targetAppEnvVars = profileInfo.m_envVariables;
        info->m_sessionType = profileInfo.m_profType;
        info->m_sessionDir = profileInfo.m_profDirectory;
        info->m_sessionStartTime = profileInfo.m_profStartTime;
        info->m_sessionEndTime = profileInfo.m_profEndTime;
        info->m_cssEnabled = profileInfo.m_isCSSEnabled;
        info->m_unwindDepth = static_cast<gtUInt16>(profileInfo.m_cssUnwindDepth);
        info->m_unwindScope = static_cast<gtUInt16>(profileInfo.m_cssScope);
        info->m_cssFPOEnabled = profileInfo.m_isCssSupportFpo;
        info->m_systemDetails = profileInfo.m_osName;
        info->m_sessionScope = profileInfo.m_profScope;
        info->m_coreAffinity = runInfo.m_cpuAffinity;

        m_dbWriter->Push({ TRANSLATED_DATA_TYPE_SESSION_INFO, (void*)info });
        info = nullptr;
    }

    return true;
}

bool DataMigrator::WriteTopologyInfoIntoDB(const CoreTopologyMap& topMap)
{
    // Populate topology info
    CPAdapterTopologyMap *topologyInfo = new (std::nothrow) CPAdapterTopologyMap;

    if (nullptr != topologyInfo)
    {
        topologyInfo->reserve(topMap.size());

        for (const auto& topIt : topMap)
        {
            topologyInfo->emplace_back(topIt.first, topIt.second.processor, topIt.second.numaNode);
        }

        m_dbWriter->Push({ TRANSLATED_DATA_TYPE_TOPOLOGY_INFO, (void*)topologyInfo });
        topologyInfo = nullptr;
    }

    return true;
}

bool DataMigrator::WriteSamplingEventInfoIntoDB(const EventEncodeVec& eventVec)
{
    // Populate event info
    gtVector<EventMaskType> *eventInfo = new (std::nothrow) gtVector<EventMaskType>;

    if (nullptr != eventInfo)
    {
        eventInfo->reserve(eventVec.size());

        for (const auto& eventIt : eventVec)
        {
            eventInfo->emplace_back(eventIt.eventMask);
        }

        m_dbWriter->Push({ TRANSLATED_DATA_TYPE_EVENT_INFO, (void*)eventInfo });
        eventInfo = nullptr;
    }

    return true;
}

bool DataMigrator::WriteSamplingConfigInfoIntoDB(const EventEncodeVec& eventVec)
{
    // Populate sampling config info
    gtVector<std::pair<EventMaskType, gtUInt32>> *samplingConfigInfo = new (std::nothrow) gtVector<std::pair<EventMaskType, gtUInt32>>;

    if (nullptr != samplingConfigInfo)
    {
        samplingConfigInfo->reserve(eventVec.size());

        for (const auto& eventIt : eventVec)
        {
            samplingConfigInfo->emplace_back(eventIt.eventMask, static_cast<gtUInt32>(eventIt.eventCount));
        }

        m_dbWriter->Push({ TRANSLATED_DATA_TYPE_SAMPLINGCONFIG_INFO, (void*)samplingConfigInfo });
        samplingConfigInfo = nullptr;
    }

    return true;
}

bool DataMigrator::WriteCoreSamplingConfigInfoIntoDB(const EventEncodeVec& eventVec, const RunInfo& runInfo)
{
    // Populate core sampling config info
    CPACoreSamplingConfigList *coreSamplingConfigList = new (std::nothrow) CPACoreSamplingConfigList;

    if (nullptr != coreSamplingConfigList)
    {
        gtUInt32 coreAffinity = static_cast<gtUInt32>(runInfo.m_cpuAffinity);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        gtUInt32 numProfiledCores = __popcnt(coreAffinity);
#else //LINUX
        gtUInt32 numProfiledCores = __builtin_popcount(coreAffinity);
#endif

        coreSamplingConfigList->reserve(numProfiledCores * eventVec.size());

        const gtUInt32 unusedBitsMask = 0x3FFFFFF;
        gtUInt32 coreIndex = 0;

        while (numProfiledCores)
        {
            if (coreAffinity & (1 << coreIndex))
            {
                for (const auto& eventIt : eventVec)
                {
                    gtUInt32 eventMask = eventIt.eventMask;
                    gtUInt64 id = coreIndex;

                    id = (id << 32) | (eventMask & unusedBitsMask);
                    coreSamplingConfigList->emplace_back(id, coreIndex, eventMask);
                }

                --numProfiledCores;
            }

            ++coreIndex;
        }

        m_dbWriter->Push({ TRANSLATED_DATA_TYPE_CORECONFIG_INFO, (void*)coreSamplingConfigList });
        coreSamplingConfigList = nullptr;
    }

    return true;
}

bool DataMigrator::WriteProcessInfoIntoDB(const PidProcessMap& processMap)
{
    bool ret = false;

    if (m_dbWriter)
    {
        // Populate process info and insert into DB.
        CPAProcessList *pProcessList = new (std::nothrow) CPAProcessList;

        if (nullptr != pProcessList)
        {
            pProcessList->reserve(64);

            for (auto& proc : processMap)
            {
                if (proc.second.getTotal())
                {
                    pProcessList->emplace_back(proc.first, proc.second.getPath(), proc.second.m_is32Bit, proc.second.m_hasCss);
                }
            }

            ret = pProcessList->size() > 0 ? true : false;

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_PROCESS_INFO, (void*)pProcessList });
            pProcessList = nullptr;
        }
    }

    return ret;
}

bool DataMigrator::WriteThreadInfoIntoDB(const NameModuleMap& moduleMap)
{
    bool ret = false;

    if (m_dbWriter)
    {
        // Insert the thread info into db
        CPAProcessThreadList *pProcThreadIdList = new (std::nothrow) CPAProcessThreadList;

        if (nullptr != pProcThreadIdList)
        {
            pProcThreadIdList->reserve(64);
            gtSet<std::pair<ProcessIdType, ThreadIdType>> processThreadSet;

            for (const auto& modIt : moduleMap)
            {
                for (auto funcIt = modIt.second.getBeginFunction(); funcIt != modIt.second.getEndFunction(); ++funcIt)
                {
                    for (auto sampleIt = funcIt->second.getBeginSample(); sampleIt != funcIt->second.getEndSample(); ++sampleIt)
                    {
                        processThreadSet.insert({ sampleIt->first.m_pid, sampleIt->first.m_tid });
                    }
                }
            }

            for (const auto it : processThreadSet)
            {
                gtUInt64 pid = it.first;
                gtUInt32 threadId = it.second;
                gtUInt64 ptId = pid;

                ptId = (ptId << 32) | threadId;
                pProcThreadIdList->emplace_back(ptId, pid, threadId);
            }

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_THREAD_INFO, (void*)pProcThreadIdList });
            pProcThreadIdList = nullptr;
        }
    }

    return ret;
}

bool DataMigrator::WriteModuleInfoIntoDB(const NameModuleMap& moduleMap)
{
    bool ret = false;

    if (m_dbWriter)
    {
        // Populate module info
        CPAModuleList *pModuleList = new (std::nothrow) CPAModuleList;

        if (nullptr != pModuleList)
        {
            pModuleList->reserve(moduleMap.size());

            for (const auto& m : moduleMap)
            {
                if (m.second.getTotal())
                {
                    pModuleList->emplace_back(
                        m.second.m_moduleId,
                        m.first,
                        m.second.m_size,
                        m.second.m_modType,
                        m.second.m_isSystemModule,
                        m.second.m_is32Bit,
                        m.second.m_isDebugInfoAvailable);
                }
            }

            ret = pModuleList->size() > 0 ? true : false;

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_MODULE_INFO, (void*)pModuleList });
            pModuleList = nullptr;
        }
    }

    return ret;
}

bool DataMigrator::WriteModuleInstanceInfoIntoDB(NameModuleMap& moduleMap)
{
    bool ret = false;

    if (m_dbWriter)
    {
        // Populate module instance info
        CPAModuleInstanceList *pModuleInstanceList = new (std::nothrow) CPAModuleInstanceList;

        if (nullptr != pModuleInstanceList)
        {
            pModuleInstanceList->reserve(moduleMap.size());

            gtUInt32 nextInstanceId = 1;

            for (auto& modIt : moduleMap)
            {
                gtSet<ProcessIdType> uniquePids;

                for (auto funcIt = modIt.second.getBeginFunction(); funcIt != modIt.second.getEndFunction(); ++funcIt)
                {
                    for (auto sampleIt = funcIt->second.getBeginSample(); sampleIt != funcIt->second.getEndSample(); ++sampleIt)
                    {
                        uniquePids.insert(sampleIt->first.m_pid);
                    }
                }

                for (auto pid : uniquePids)
                {
                    modIt.second.m_moduleInstanceInfo.emplace_back(
                        static_cast<gtUInt64>(pid),
                        static_cast<gtUInt64>(modIt.second.getBaseAddr()),
                        nextInstanceId);

                    pModuleInstanceList->emplace_back(nextInstanceId, modIt.second.m_moduleId, pid, modIt.second.getBaseAddr());
                    ++nextInstanceId;
                }
            }

            ret = pModuleInstanceList->size() > 0 ? true : false;
            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_MODINSTANCE_INFO, (void*)pModuleInstanceList });
            pModuleInstanceList = nullptr;
        }
    }

    return ret;
}

bool DataMigrator::WriteFunctionInfoIntoDB(const NameModuleMap& moduleMap)
{
    bool ret = false;

    if (m_dbWriter)
    {
        // Populate function info
        CPAFunctionInfoList *pFuncInfoList = new (std::nothrow) CPAFunctionInfoList;

        if (nullptr != pFuncInfoList)
        {
            // Let us assume one function from each module
            pFuncInfoList->reserve(moduleMap.size());

            // Insert a dummy function as "Unknown Function"
            gtString unknownFuncName = L"Unknown Function";
            pFuncInfoList->emplace_back(UNKNOWN_FUNCTION_ID, UNKNOWN_MODULE_ID, unknownFuncName, 0, 0);

            for (auto& module : moduleMap)
            {
                gtUInt32 modId = module.second.m_moduleId;
                gtUInt64 modLoadAddr = module.second.getBaseAddr();

                for (auto fit = module.second.getBeginFunction(); fit != module.second.getEndFunction(); ++fit)
                {
                    gtString funcName = fit->second.getFuncName();
                    gtUInt64 startOffset = fit->second.getBaseAddr() - modLoadAddr;
                    gtUInt64 size = fit->second.getSize();
                    gtUInt32 funcId = modId;
                    funcId = (funcId << 16) | fit->second.m_functionId;

                    // If function name is empty, it will be considered as unknown-function, don't insert into DB.
                    // else insert function info into DB
                    if (!funcName.isEmpty())
                    {
                        pFuncInfoList->emplace_back(funcId, modId, funcName, startOffset, size);
                    }
                }
            }

            ret = pFuncInfoList->size() > 0 ? true : false;

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_FUNCTION_INFO, (void*)pFuncInfoList });
            pFuncInfoList = nullptr;
        }
    }

    return ret;
}

bool DataMigrator::WriteSampleProfileDataIntoDB(const NameModuleMap& modMap)
{
    bool ret = false;

    if (m_dbWriter)
    {
        // Populate sample info
        CPASampeInfoList *pSampleList = new (std::nothrow) CPASampeInfoList;

        if (nullptr != pSampleList)
        {
            const gtUInt32 unusedBitsMask = 0x3FFFFFF;

            // Assume minimum 1000 samples
            pSampleList->reserve(1000);

            for (auto& module : modMap)
            {
                gtUInt32 modSize = module.second.m_size;

                for (auto fit = module.second.getBeginFunction(); fit != module.second.getEndFunction(); ++fit)
                {
                    gtUInt32 funcId = module.second.m_moduleId;
                    funcId = (funcId << 16) | fit->second.m_functionId;

                    for (auto aptIt = fit->second.getBeginSample(); aptIt != fit->second.getEndSample(); ++aptIt)
                    {
                        gtUInt64 pid = aptIt->first.m_pid;
                        gtUInt32 threadId = aptIt->first.m_tid;
                        gtVAddr  sampleAddr = fit->second.getBaseAddr() + aptIt->first.m_addr;
                        gtUInt32 moduleInstanceid = 0ULL;
                        gtVAddr  modLoadAddr = 0ULL;

                        for (const auto& it : module.second.m_moduleInstanceInfo)
                        {
                            modLoadAddr = std::get<1>(it);
                            gtVAddr modEndAddr = modLoadAddr + modSize;

                            if ((std::get<0>(it) == pid))
                            {
                                // 1. sampleAddr falls within module size limit.
                                // 2. module size is 0, pick the first match.
                                if ((modLoadAddr <= sampleAddr) && ((sampleAddr < modEndAddr) || (0 == modSize)))
                                {
                                    moduleInstanceid = std::get<2>(it);
                                    break;
                                }
                            }
                        }

                        // sample address offset is w.r.t. module load address
                        gtUInt64 sampleOffset = sampleAddr - modLoadAddr;
                        gtUInt64 processThreadId = pid;
                        processThreadId = (processThreadId << 32) | threadId;

                        for (auto skIt = aptIt->second.getBeginSample(); skIt != aptIt->second.getEndSample(); ++skIt)
                        {
                            gtUInt64 sampleCount = skIt->second;
                            gtUInt64 coreSamplingConfigId = skIt->first.cpu;
                            coreSamplingConfigId = (coreSamplingConfigId << 32) | (skIt->first.event & unusedBitsMask);

                            pSampleList->emplace_back(processThreadId, moduleInstanceid, coreSamplingConfigId, funcId, sampleOffset, sampleCount);
                        }
                    }
                }
            }

            ret = pSampleList->size() > 0 ? true : false;

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_SAMPLE_INFO, (void*)pSampleList });
            pSampleList = nullptr;
        }
    }

    return ret;
}
#if 0
class MigratorCssCallback : public CssCallback
{
public:
    MigratorCssCallback() {}
    ~MigratorCssCallback() {}

    bool Initialize(osFilePath cssFilePath, gtUInt32 pid)
    {
        bool ret = false;

        gtString fileName;
        fileName.appendFormattedString(L"%u", pid);

        cssFilePath.setFileName(fileName);
        cssFilePath.setFileExtension(L"css");

        if (cssFilePath.exists())
        {
            CssReader reader(CP_CSS_MAX_UNWIND_DEPTH);
            ret = reader.Open(cssFilePath.asString().asCharArray());

            if (ret)
            {
                reader.SetCallback(this);
                ret = reader.Read(m_funcGraph, pid);
            }
        }

        return ret;
    }

    const FunctionGraph& GetFunctionGraph() const { return m_funcGraph; }

    FunctionGraph& GetFunctionGraph() { return m_funcGraph; }

    // Callback to CssCallback class
    bool AddModule(gtVAddr base, const wchar_t* pPath)
    {
        GT_UNREFERENCED_PARAMETER(base);
        GT_UNREFERENCED_PARAMETER(pPath);
        return false;
    }

    bool AddFunction(gtVAddr va, gtVAddr& startVa, gtVAddr& endVa)
    {
        GT_UNREFERENCED_PARAMETER(va);
        GT_UNREFERENCED_PARAMETER(startVa);
        GT_UNREFERENCED_PARAMETER(endVa);
        return false;
    }

    // This function must be called right after calling AddFunction
    bool AddMetadata(gtVAddr va, void** ppMetadata = nullptr)
    {
        GT_UNREFERENCED_PARAMETER(va);
        GT_UNREFERENCED_PARAMETER(ppMetadata);
        return false;
    }

private:
    FunctionGraph m_funcGraph;
};

bool DataMigrator::WriteCallgraphProfileDataIntoDB(const PidProcessMap& processMap, const NameModuleMap& moduleMap)
{
    GT_UNREFERENCED_PARAMETER(moduleMap);
    bool ret = false;

    if (m_dbWriter)
    {
        for (const auto& procIt : processMap)
        {
            if (procIt.second.m_hasCss)
            {
                MigratorCssCallback cssReader;
                cssReader.Initialize(m_sourceFilePath, procIt.first);

                FunctionGraph& funcGraph = cssReader.GetFunctionGraph();

                gtUInt32 nextCallStackId = 1;
                CPACallStackFrameInfoList *csFrameInfoList = new (std::nothrow) CPACallStackFrameInfoList;
                CPACallStackLeafInfoList  *csLeafInfoList = new (std::nothrow) CPACallStackLeafInfoList;

                if (nullptr != csFrameInfoList && nullptr != csLeafInfoList)
                {
                    // This is just a safe random space reservation. Check if we can reserve more accurate space.
                    csFrameInfoList->reserve(500);
                    csLeafInfoList->reserve(100);

                    // TODO: process CSS data from reader

                    m_dbWriter->Push({ TRANSLATED_DATA_TYPE_CSS_FRAME_INFO, (void*)csFrameInfoList });
                    csFrameInfoList = nullptr;

                    m_dbWriter->Push({ TRANSLATED_DATA_TYPE_CSS_LEAF_INFO, (void*)csLeafInfoList });
                    csLeafInfoList = nullptr;

                    ret = true;
                }
                else
                {
                    // At least one of the memory allocations failed.
                    delete csFrameInfoList;
                    delete csLeafInfoList;
                    break;
                }
            }
        }
    }

    return ret;
}
#endif
bool DataMigrator::WriteJitInfoIntoDB(const NameModuleMap& modMap)
{
    bool ret = false;

    if (m_dbWriter)
    {
        // Populate Java/CLR Jnc and Jit blob info.
        CPAJitCodeBlobInfoList *jitCodeBlobInfoList = new (std::nothrow) CPAJitCodeBlobInfoList;
        CPAJitInstanceInfoList *jitInstanceInfoList = new (std::nothrow) CPAJitInstanceInfoList;

        if (jitInstanceInfoList != nullptr && jitCodeBlobInfoList != nullptr)
        {
            for (const auto& modIt : modMap)
            {
                if (modIt.second.m_modType == CpuProfileModule::JAVAMODULE)
                {
                    for (auto funcIt = modIt.second.getBeginFunction(); funcIt != modIt.second.getEndFunction(); ++funcIt)
                    {
                        gtString srcFilePath = funcIt->second.getSourceFile();
                        gtString jncFilePath = funcIt->second.getJncFileName();
                        gtUInt64 pid = 0;
                        gtUInt32 jitId = 0;

                        pid = wcstoul(jncFilePath.asCharArray(), nullptr, 10);

                        int idx = 0;
                        for (; jncFilePath[idx] != L'_' && jncFilePath[idx] != L'\0'; ++idx);

                        if (jncFilePath[idx] == L'_')
                        {
                            jitId = wcstoul(jncFilePath.asCharArray() + idx + 1, nullptr, 10);
                        }

                        // Compute function id.
                        gtUInt64 funcId = modIt.second.m_moduleId;
                        funcId = (funcId << 16) | funcIt->second.m_functionId;

                        gtUInt64 loadAddr = funcIt->second.getBaseAddr();
                        gtUInt32 size = static_cast<gtUInt32>(funcIt->second.getTopAddr() - funcIt->second.getBaseAddr());

                        jitInstanceInfoList->emplace_back(jitId, funcId, pid, loadAddr, size);
                        jitCodeBlobInfoList->emplace_back(jitId, srcFilePath, jncFilePath);
                    }
                }
            }

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_JITINSTANCE_INFO, (void*)jitInstanceInfoList });
            jitInstanceInfoList = nullptr;

            m_dbWriter->Push({ TRANSLATED_DATA_TYPE_JITCODEBLOB_INFO, (void*)jitCodeBlobInfoList });
            jitCodeBlobInfoList = nullptr;
        }
        else
        {
            delete jitInstanceInfoList;
            delete jitCodeBlobInfoList;
        }
    }

    return ret;
}

bool DataMigrator::WriteFinish()
{
    // Finish writing profile data
    m_dbWriter->Push({ TRANSLATED_DATA_TYPE_UNKNOWN_INFO, nullptr });
    return true;
}

bool DataMigrator::doDeleteSrcFiles()
{
    if (m_sourceFilePath.exists())
    {
        osFilePath oldFilePath(m_sourceFilePath);

        // delete old EBP file
        osFile oldFile(oldFilePath);
        oldFile.deleteFile();

        //TODO: delete IMD/RI/TI files.
    }

    return true;
}