//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PwrProfTranslateWin.cpp
///
//==================================================================================
#include <PwrProfTranslateWin.h>
#include <PowerProfileHelper.h>

// PwrGetProfileData: Provide process data based on process/module/ip profile type
AMDTResult PwrProfTranslateWin::PwrGetProfileData(CXLContextProfileType type, void** pData, AMDTUInt32* pCnt, AMDTFloat32* pPower)
{
    (void)type;
    (void)pData;
    (void)pCnt;
    (void)pPower;

    AMDTResult ret = AMDT_STATUS_OK;

#ifndef _WIN64
    char path[OS_MAX_PATH];
    AMDTFloat32 totalPower = 0;
    wchar_t str[OS_MAX_PATH];

    memset(str, 0, sizeof(wchar_t) * OS_MAX_PATH);

    if (PROCESS_PROFILE == type)
    {
        AMDTPwrProcessInfo process;
        memset(&process, 0, sizeof(AMDTPwrProcessInfo));
        m_processList.clear();

        for (auto processIter : m_systemTreeMap)
        {
            memset(&process, 0, sizeof(AMDTPwrProcessInfo));
            process.m_pid = processIter.first;

            memset(path, '\0', OS_MAX_PATH);

            if (S_OK == fnFindProcessName(process.m_pid, str, AMDT_PWR_EXE_PATH_LENGTH))
            {
                wcstombs(path, str, OS_MAX_PATH);
            }

            ExtractNameAndPath(path, process.m_name, process.m_path);

            process.m_sampleCnt = processIter.second.m_sampleCnt;
            process.m_power = processIter.second.m_power;
            totalPower += process.m_power;
            m_processList.push_back(process);
        }

        if (m_processList.size() > 0)
        {
            *pPower = totalPower;
            std::sort(m_processList.begin(), m_processList.end(),
            [](AMDTPwrProcessInfo const & a, AMDTPwrProcessInfo const & b) { return a.m_power > b.m_power; });
            *pData = &m_processList[0];
            *pCnt = static_cast<AMDTUInt32>(m_processList.size());
        }
    }

    if (MODULE_PROFILE == type)
    {
        AMDTPwrModuleData module;
        m_moduleList.clear();
        memset(&module, 0, sizeof(AMDTPwrModuleData));

        for (auto processIter : m_systemTreeMap)
        {
            for (auto moduleIter : processIter.second.m_modInstMap)
            {
                module.m_power = moduleIter.second.m_power;
                totalPower += module.m_power;

                ModuleInfoMap::iterator tabIter = m_moduleTable.find(moduleIter.first);

                if (tabIter != m_moduleTable.end())
                {
                    module.m_isKernel = tabIter->second.m_isKernel;
                    module.m_loadAddr = tabIter->second.m_moduleStartAddr;
                    module.m_size = tabIter->second.m_modulesize;
                    module.m_processId = static_cast<AMDTUInt32>(processIter.first);
                    ExtractNameAndPath(tabIter->second.m_pModulename, module.m_moduleName, module.m_modulePath);
                }
                else
                {
                    //PwrTrace("Error finding module pid %d module inst %d", (AMDTUInt32)processIter.first, moduleIter.first);
                }

                module.m_sampleCnt = moduleIter.second.m_sampleCnt;
                memset(path, '\0', OS_MAX_PATH);

                // Get process name and path
                if (S_OK == fnFindProcessName(module.m_processId, str, AMDT_PWR_EXE_PATH_LENGTH))
                {
                    wcstombs(path, str, OS_MAX_PATH);
                }

                ExtractNameAndPath(path, module.m_processName, module.m_processPath);

                m_moduleList.push_back(module);
            }
        }

        if (m_moduleList.size() > 0)
        {
            *pPower = totalPower;
            std::sort(m_moduleList.begin(), m_moduleList.end(),
            [](AMDTPwrModuleData const & a, AMDTPwrModuleData const & b) { return a.m_power > b.m_power; });
            *pData = &m_moduleList[0];
            *pCnt = static_cast<AMDTUInt32>(m_moduleList.size());
        }
    }

    if (*pCnt == 0)
    {
        ret = AMDT_ERROR_NODATA;
    }

#endif
    return ret;
}

// SetElapsedTime: Elapse time from the time when first record was collected
void PwrProfTranslateWin::SetElapsedTime(AMDTUInt64 raw, AMDTUInt64* pResult)
{
    *pResult = static_cast<AMDTUInt64>((AMDTUInt64)((raw - m_perfCounter) * 1000) / (AMDTUInt64)m_perfFreq);
}

//AttributePowerToSample: Calculate the power for each sample and insert in the system tree
AMDTResult PwrProfTranslateWin::AttributePowerToSample()
{
    AMDTUInt32 coreIdx = 0;
    AMDTFloat32 timeDiv = 0;
    AMDTFloat32 totalComponentLoad[MAX_PHYSICAL_CORE_CNT] = { 0, };
    AMDTFloat32 energyPerIpcLoad = 0;
    timeDiv = (m_currentTs > m_prevTs1) ? ((m_currentTs - m_prevTs1) / PwrGetCountsPerSecs()) : m_samplingPeriod;
    memset(totalComponentLoad, 0, m_targetCoreCnt);
    m_prevTs1 = m_currentTs;

    for (coreIdx = 0; coreIdx < m_targetCoreCnt; coreIdx++)
    {
        totalComponentLoad[coreIdx / m_coresPerComponent] += m_sampleIpcLoad[coreIdx];
    }

    for (coreIdx = 0; coreIdx < m_targetCoreCnt; coreIdx++)
    {
        float energyPerSample = 0;
        AMDTUInt32 componentIdx = coreIdx / m_coresPerComponent;

        if (PLATFORM_ZEPPELIN == GetSupportedTargetPlatformId())
        {
            timeDiv = 1;
        }

        energyPerSample = (m_componentPower[componentIdx] * timeDiv);

        for (auto sampleIter : m_sampleMap[coreIdx])
        {
            energyPerIpcLoad = (sampleIter.second.m_ipc / totalComponentLoad[componentIdx]) * energyPerSample;
            InsertSampleToSystemTree(&sampleIter.second, energyPerIpcLoad, sampleIter.second.m_sampleCnt);
        }

        m_sampleMap[coreIdx].clear();
        m_sampleIpcLoad[coreIdx] = 0;
    }

    // reset power component powers
    memset(m_componentPower, 0, sizeof(AMDTFloat32) * MAX_PHYSICAL_CORE_CNT);
    return AMDT_STATUS_OK;
}

// ProcessSample: Process each data sample for process/module/ip profiling
AMDTResult PwrProfTranslateWin::ProcessSample(ContextData* pCtx, AMDTUInt32 coreId, AMDTUInt32 componentIdx)
{
    (void)pCtx;
    (void)coreId;
    (void)componentIdx;
    AMDTUInt64 rerdMOps = 0;
    AMDTUInt64 cpuNotHltd = 0;

    AMDTUInt64 key = 0;
    bool newModule = false;
    AMDTUInt64 deltaTick = 0;
    AMDTFloat32 ipc = 0;
    rerdMOps = pCtx->m_pmcData[PMC_EVENT_RETIRED_MICRO_OPS];
    cpuNotHltd = pCtx->m_pmcData[PMC_EVENT_CPU_CYCLE_NOT_HALTED];

    if (PLATFORM_ZEPPELIN == GetSupportedTargetPlatformId())
    {
        rerdMOps = (rerdMOps > m_prevIpcData[coreId].m_ipcData[PMC_EVENT_RETIRED_MICRO_OPS]) ?
                   (rerdMOps - m_prevIpcData[coreId].m_ipcData[PMC_EVENT_RETIRED_MICRO_OPS]) :
                   (rerdMOps + ~m_prevIpcData[coreId].m_ipcData[PMC_EVENT_RETIRED_MICRO_OPS]);

        cpuNotHltd = (cpuNotHltd > m_prevIpcData[coreId].m_ipcData[PMC_EVENT_CPU_CYCLE_NOT_HALTED]) ?
                     (cpuNotHltd - m_prevIpcData[coreId].m_ipcData[PMC_EVENT_CPU_CYCLE_NOT_HALTED]) :
                     (cpuNotHltd + ~m_prevIpcData[coreId].m_ipcData[PMC_EVENT_CPU_CYCLE_NOT_HALTED]);

        m_prevIpcData[coreId].m_ipcData[PMC_EVENT_RETIRED_MICRO_OPS] = rerdMOps;
        m_prevIpcData[coreId].m_ipcData[PMC_EVENT_CPU_CYCLE_NOT_HALTED] = cpuNotHltd;
    }

    ipc = (AMDTFloat32)((AMDTFloat64) rerdMOps / (AMDTFloat64) cpuNotHltd);

          SetElapsedTime(pCtx->m_timeStamp, (AMDTUInt64*)&deltaTick);

    AMDTUInt32 instanceId = 0;
    pCtx->m_ip = pCtx->m_ip & 0x0000FFFFFFFFFFFF;

    if (S_OK != fnGetModuleInstanceId(pCtx->m_processId, pCtx->m_ip, deltaTick, instanceId))
    {
        // PwrTrace("fnGetModuleInstanceId failed for PID %d", pCtx->m_processId);
    }

    key = (pCtx->m_ip & 0x0000FFFFFFFFFFFFULL) | ((AMDTUInt64)instanceId << 48);
    SampleMap::iterator sampleIter = m_sampleMap[coreId].find(key);

    if (sampleIter == m_sampleMap[coreId].end())
    {
        SampleData sample;
        sample.m_ip = pCtx->m_ip;
        sample.m_processId = pCtx->m_processId;
        sample.m_threadId = pCtx->m_threadId;
        sample.m_modInstance = instanceId;
        sample.m_timeStamp = deltaTick;
        sample.m_ipc = ipc;
        sample.m_sampleCnt = 1;
        m_sampleMap[coreId].insert(SampleMap::value_type(key, sample));
        newModule = true;
    }
    else
    {
        sampleIter->second.m_ipc += ipc;
        sampleIter->second.m_sampleCnt++;
    }

    m_sampleIpcLoad[coreId] += ipc;

    if (newModule)
    {
        ModuleInfoMap::iterator tabIter = m_moduleTable.find(instanceId);

        if (tabIter == m_moduleTable.end())
        {
            LoadModuleInfo mod;
            memset(&mod, 0, sizeof(LoadModuleInfo));
            fnGetModuleInfoByInstanceId(instanceId, &mod);
            m_moduleTable.insert(ModuleInfoMap::value_type(instanceId, mod));
        }
    }

    return AMDT_STATUS_OK;
}

// InitializeModuleMap:
void PwrProfTranslateWin::InitializeModuleMap()
{
    m_prevTs1 = 0;
    m_perfFreq = m_rawFileHld->GetSessionPerfFreq();
}

// CleanupModuleMap:
void PwrProfTranslateWin::CleanupModuleMap()
{
#ifndef _WIN64
    fnCleanupMaps();
#endif
}

// InsertSampleToSystemTree: Insert each sample to system tree with corresponding power
AMDTResult PwrProfTranslateWin::InsertSampleToSystemTree(SampleData* pCtx, AMDTFloat32 power, AMDTUInt32 sampleCnt)
{
    AMDTResult ret = AMDT_STATUS_OK;
    IpAddressInfo ipSample;

    ProcessTreeMap::iterator processIter = m_systemTreeMap.find(pCtx->m_processId);

    if (processIter == m_systemTreeMap.end())
    {
        ipSample.m_sampleCnt = sampleCnt;
        ipSample.m_power = power;

        //Create ip address map
        ModuleIntanceInfo modInfo;

        modInfo.m_ipMap.insert(IpAddressMap::value_type(pCtx->m_ip, ipSample));
        modInfo.m_power = power;
        modInfo.m_sampleCnt = sampleCnt;
        //Create ModuleInstanceMap
        ProcessInfo processInfo;
        processInfo.m_power = power;
        processInfo.m_sampleCnt = sampleCnt;
        processInfo.m_modInstMap.insert(ModuleInstanceMap::value_type(pCtx->m_modInstance, modInfo));
        //Create m_systemTreeMap
        m_systemTreeMap.insert(ProcessTreeMap::value_type(pCtx->m_processId, processInfo));
    }
    else
    {
        ModuleInstanceMap::iterator moduleIter = processIter->second.m_modInstMap.find(pCtx->m_modInstance);

        if (moduleIter == processIter->second.m_modInstMap.end())
        {
            ModuleIntanceInfo instInfo;
            ipSample.m_sampleCnt = sampleCnt;
            ipSample.m_power = power;
            instInfo.m_power = power;
            instInfo.m_sampleCnt = sampleCnt;
            instInfo.m_ipMap.insert(IpAddressMap::value_type(pCtx->m_modInstance, ipSample));
            //Create
            processIter->second.m_modInstMap.insert(ModuleInstanceMap::value_type(pCtx->m_modInstance, instInfo));
            processIter->second.m_power += power;
        }
        else
        {
            IpAddressMap::iterator ipIter = moduleIter->second.m_ipMap.find(pCtx->m_ip);

            if (ipIter == moduleIter->second.m_ipMap.end())
            {
                ipSample.m_sampleCnt = sampleCnt;
                ipSample.m_power = power;
                moduleIter->second.m_ipMap.insert(IpAddressMap::value_type(pCtx->m_ip, ipSample));
                moduleIter->second.m_power += power;
                moduleIter->second.m_sampleCnt += sampleCnt;
                processIter->second.m_power += power;
                processIter->second.m_sampleCnt += sampleCnt;
            }
            else
            {
                ipIter->second.m_power += power;
                ipIter->second.m_sampleCnt += sampleCnt;
                moduleIter->second.m_power += power;
                moduleIter->second.m_sampleCnt += sampleCnt;
                processIter->second.m_power += power;
                ipIter->second.m_sampleCnt += sampleCnt;
                processIter->second.m_sampleCnt++;
            }
        }
    }

    return ret;
}

// ExtractNameAndPath: Extract path and name of the executable
void PwrProfTranslateWin::ExtractNameAndPath(char* pFullPath, char* pName, char* pPath)
{
    AMDTUInt32 cnt = 0;
    AMDTUInt32 begin = 0;
    char* path = pFullPath;
    bool prefix = false;

    while (path[cnt] != '\0')
    {
        if ('\\' == path[cnt])
        {
            begin = cnt;
            prefix = true;
        }

        cnt++;
    }

    if (prefix)
    {
        begin += 1;
    }

    memset(pName, 0, AMDT_PWR_EXE_NAME_LENGTH);
    memset(pPath, 0, AMDT_PWR_EXE_PATH_LENGTH);
    memcpy(pName, &path[begin], strlen(&path[begin]) + 1);
    memcpy(pPath, &path[0], begin);

    if (strlen(pName) == 0)
    {
        memcpy(pName, "unknown name", AMDT_PWR_EXE_NAME_LENGTH);
    }

    if (strlen(pPath) == 0)
    {
        memcpy(pPath, "unknown path", AMDT_PWR_EXE_PATH_LENGTH);
    }
}

