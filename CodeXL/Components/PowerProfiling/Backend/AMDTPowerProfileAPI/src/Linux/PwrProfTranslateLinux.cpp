// Project Headers
#include <PowerProfileHelper.h>
#include <Linux/PwrProfTranslateLinux.h>

// System Headers
#include <algorithm>

extern std::list <ProcessName> g_processNames;

#define PP_UNKNOWN_MODULE "Unknown Module"
#define PP_UNKNOWN_NAME "unknown name"
#define PP_UNKNOWN_PATH "unknown path"

// PwrCharToHex:
int PwrCharToHex(char ch)
{
    if ((ch >= '0') && (ch <= '9'))
    {
        return ch - '0';
    }

    if ((ch >= 'a') && (ch <= 'f'))
    {
        return ch - 'a' + 10;
    }

    if ((ch >= 'A') && (ch <= 'F'))
    {
        return ch - 'A' + 10;
    }

    return -1;
}

// PwrHexToUInt64:
int PwrHexToUInt64(const char* ptr, AMDTUInt64* longVal)
{
    const char* p = ptr;
    *longVal = 0;

    while (*p)
    {
        const int hexVal = PwrCharToHex(*p);

        if (hexVal < 0)
        {
            break;
        }

        *longVal = (*longVal << 4) | hexVal;
        p++;
    }

    return p - ptr;
}

// InitializeModuleMap: initialize the data structure for process/module profiling
void PwrProfTranslateLinux::InitializeModuleMap()
{
    // init process profiling structures
    m_procPidModTable.reserve(MAX_MODULE_PER_PROCESS);
    m_modSampleDataTable.reserve(m_phyCoreCnt);

    // init each core with dummy map
    for (AMDTUInt32 idx = 0 ; idx < m_phyCoreCnt; ++idx)
    {
        m_modSampleDataTable.push_back(ModuleSampleDataMap());
    }
}

// SetElapsedTime: Elapse time from the time when first record was collected
void PwrProfTranslateLinux::SetElapsedTime(AMDTUInt64 raw, AMDTUInt64* pResult)
{
    const unsigned int nsec_to_microSec = 1000;
    *pResult = static_cast<AMDTUInt64>(raw - m_perfCounter) / nsec_to_microSec;
}

// PwrGetProfileData: Get the profile data
AMDTResult PwrProfTranslateLinux::PwrGetProfileData(CXLContextProfileType type,
                                                    void** pData,
                                                    AMDTUInt32* pCnt,
                                                    AMDTFloat32* pEnergy)
{
    if (PROCESS_PROFILE == type)
    {
        AMDTFloat32 totalEnergy = 0;

        AMDTPwrProcessInfo process;
        m_processList.clear();

        for (const auto& itr : m_aggrModSampleDataMap)
        {
            auto pidItr = std::find_if(std::begin(m_processList),
                                       std::end(m_processList),
            [ = ](const AMDTPwrProcessInfo & args) { return (args.m_pid == itr.second.m_processId); });

            if (pidItr != m_processList.end())
            {
                process.m_sampleCnt += itr.second.m_sampleCnt;
                process.m_power += itr.second.m_power;
            }
            else
            {
                memset(&process, 0, sizeof(AMDTPwrProcessInfo));
                process.m_pid = itr.second.m_processId;

                AMDTPwrProcessInfo info;
                info.m_pid = itr.second.m_processId;

                GetProcessName(&info);
                memcpy(process.m_name, &info.m_name, AMDT_PWR_EXE_NAME_LENGTH);
                memcpy(process.m_path, &info.m_path, AMDT_PWR_EXE_NAME_LENGTH);

                process.m_sampleCnt = itr.second.m_sampleCnt;
                process.m_power     = itr.second.m_power;

                m_processList.push_back(process);
            }

            totalEnergy += process.m_power;
        }

        if (m_processList.size() > 0)
        {
            *pEnergy = totalEnergy;
            std::sort(m_processList.begin(), m_processList.end(),
            [](AMDTPwrProcessInfo const & a, AMDTPwrProcessInfo const & b) { return a.m_power > b.m_power; });
            *pData = &m_processList[0];
            *pCnt = static_cast<AMDTUInt32>(m_processList.size());
        }
    }
    else if (MODULE_PROFILE == type)
    {
        AMDTPwrModuleData module;
        m_moduleList.clear();
        AMDTFloat32 totalEnergy = 0;

        for (const auto& itr : m_aggrModSampleDataMap)
        {
            // fill process info
            memset(&module, 0, sizeof(AMDTPwrModuleData));

            AMDTPwrProcessInfo info;
            info.m_pid = itr.second.m_processId;

            module.m_processId = (AMDTUInt32)itr.second.m_processId;

            GetProcessName(&info);
            memcpy(module.m_processName, &info.m_name, AMDT_PWR_EXE_NAME_LENGTH);
            memcpy(module.m_processPath, &info.m_path, AMDT_PWR_EXE_NAME_LENGTH);

            module.m_sampleCnt = itr.second.m_sampleCnt;
            module.m_isKernel = itr.second.m_isKernel;
            module.m_power = itr.second.m_power;
            totalEnergy += module.m_power;

            auto moduleItr = m_moduleIdInfoMap.find(itr.first);

            if (moduleItr != m_moduleIdInfoMap.end())
            {
                ExtractNameAndPath(moduleItr->second.m_pModulename, module.m_moduleName, module.m_modulePath);
                module.m_size = moduleItr->second.m_endAddress - moduleItr->second.m_startAddress;
                module.m_loadAddr = moduleItr->second.m_startAddress;
            }

            m_moduleList.push_back(module);
        }

        if (m_moduleList.size() > 0)
        {
            *pEnergy = totalEnergy;
            std::sort(m_moduleList.begin(), m_moduleList.end(),
            [](AMDTPwrModuleData const & a, AMDTPwrModuleData const & b) { return a.m_power > b.m_power; });
            *pData = &m_moduleList[0];

            // For system idle process set module size to zero
            // list is sorted, if process id 0 exists it should be always at index 0
            try
            {
                auto& findProcessZero = m_moduleList.at(0);

                if (0 == findProcessZero.m_processId)
                {
                    findProcessZero.m_size = 0;
                }
            }
            catch (const std::out_of_range& oor)
            {
                PwrTrace("Trying to access, non existent process id.");
            }


            *pCnt = static_cast<AMDTUInt32>(m_moduleList.size());
        }
    }

    return AMDT_STATUS_OK;
}

// AttributePowerToSample : Collect the power/ipc for each sample
AMDTResult PwrProfTranslateLinux::AttributePowerToSample()
{
    AMDTFloat32 energyPerCuSample = 0.0;
    AMDTUInt64 timeSpan = 0;
    AMDTFloat32 load = 0;

    try
    {
        for (AMDTUInt32 compIdx = 0; compIdx < m_phyCoreCnt; ++compIdx)
        {
            auto& moduleMapItr = m_modSampleDataTable.at(compIdx);

            if (PLATFORM_ZEPPELIN == GetSupportedTargetPlatformId())
            {
                energyPerCuSample = m_componentPower[compIdx];
            }
            else
            {
                timeSpan = (m_currentTs - m_prevTs);
                energyPerCuSample = (m_componentPower[compIdx] > 0) ? (m_componentPower[compIdx] * timeSpan / PwrGetCountsPerSecs()) : static_cast<AMDTFloat32>(0.0);
            }

            for (auto& modItr : moduleMapItr)
            {
                load = modItr.second.m_ipc / m_sampleIpcLoad[compIdx];
                modItr.second.m_power = energyPerCuSample * load;
            }
        }


        for (AMDTUInt32 compIdx = 0; compIdx < m_phyCoreCnt; ++compIdx)
        {
            auto& moduleMapItr = m_modSampleDataTable.at(compIdx);

            for (auto modItr : moduleMapItr)
            {
                auto found = m_aggrModSampleDataMap.find(modItr.first);

                if (found != m_aggrModSampleDataMap.end())
                {
                    found->second.m_ipc         += modItr.second.m_ipc;
                    found->second.m_sampleCnt   += modItr.second.m_sampleCnt;
                    found->second.m_power       += modItr.second.m_power;
                }
                else
                {
                    ModuleSampleData sample;
                    sample.m_ipc        = modItr.second.m_ipc;
                    sample.m_sampleCnt  = modItr.second.m_sampleCnt;
                    sample.m_processId  = modItr.second.m_processId;
                    sample.m_threadId   = modItr.second.m_threadId;
                    sample.m_isKernel   = modItr.second.m_isKernel;
                    sample.m_ip         = modItr.second.m_ip;
                    sample.m_power      = modItr.second.m_power;

                    m_aggrModSampleDataMap.insert({modItr.first, sample});
                }
            }

            m_sampleIpcLoad[compIdx] = 0;
        }
    }
    catch (const std::out_of_range& oor)
    {
        PwrTrace("Core index does not exists. ");
    }

    // clear the table
    m_modSampleDataTable.clear();

    for (AMDTUInt32 idx = 0 ; idx < m_phyCoreCnt; ++idx)
    {
        m_modSampleDataTable.push_back(ModuleSampleDataMap());
    }

    // reset power component powers
    memset(m_componentPower, 0, sizeof(AMDTFloat32) * MAX_PHYSICAL_CORE_CNT);

    return AMDT_STATUS_OK;
}

// ProcessSample: Process each data sample for process/module/ip profiling
AMDTResult PwrProfTranslateLinux::ProcessSample(ContextData* pCtx, AMDTUInt32 coreId, AMDTUInt32 componentIdx)
{
    (void)coreId;
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTFloat32 ipc = 1;
    AMDTUInt64 rerdMOps = 0;
    AMDTUInt64 cpuNotHltd = 0;

    if (MAX_PID_CNT < m_procPidModTable.size())
    {
        ret = AMDT_ERROR_UNEXPECTED;
        PwrTrace("MAX_PID_CNT reached");
    }

    if (PLATFORM_ZEPPELIN == GetSupportedTargetPlatformId())
    {
        rerdMOps = pCtx->m_pmcData[PMC_EVENT_RETIRED_MICRO_OPS];
        cpuNotHltd = pCtx->m_pmcData[PMC_EVENT_CPU_CYCLE_NOT_HALTED];
        rerdMOps = (rerdMOps > m_prevIpcData[coreId].m_ipcData[PMC_EVENT_RETIRED_MICRO_OPS]) ?
                   (rerdMOps - m_prevIpcData[coreId].m_ipcData[PMC_EVENT_RETIRED_MICRO_OPS]) :
                   (rerdMOps + ~m_prevIpcData[coreId].m_ipcData[PMC_EVENT_RETIRED_MICRO_OPS]);

        cpuNotHltd = (cpuNotHltd > m_prevIpcData[coreId].m_ipcData[PMC_EVENT_CPU_CYCLE_NOT_HALTED]) ?
                     (cpuNotHltd - m_prevIpcData[coreId].m_ipcData[PMC_EVENT_CPU_CYCLE_NOT_HALTED]) :
                     (cpuNotHltd + ~m_prevIpcData[coreId].m_ipcData[PMC_EVENT_CPU_CYCLE_NOT_HALTED]);

         ipc = (AMDTFloat32)((AMDTFloat64) rerdMOps / (AMDTFloat64) cpuNotHltd);

         m_prevIpcData[coreId].m_ipcData[PMC_EVENT_RETIRED_MICRO_OPS] = rerdMOps;
         m_prevIpcData[coreId].m_ipcData[PMC_EVENT_CPU_CYCLE_NOT_HALTED] = cpuNotHltd;
    }

    // search for the entry in /proc/pid/map details
    auto pidFound = std::find_if(std::begin(m_procPidModTable),
                                 std::end(m_procPidModTable),
    [ = ](const ProcPidModInfo & args) { return (args.m_processId == pCtx->m_processId); });

    if (pidFound != m_procPidModTable.end())
    {
        if (AMDT_ERROR_FAIL == UpdateModuleSampleMap(pidFound->m_modInfoTable, pCtx, componentIdx, ipc))
        {
            // update module table
            ReadProcPidMap(pCtx, *pidFound);

            // update the sample module table again
            UpdateModuleSampleMap(pidFound->m_modInfoTable, pCtx, componentIdx, ipc);
        }
    }
    else
    {
        // insert an entry in process => module module map
        ProcPidModInfo info;
        info.m_processId = pCtx->m_processId;
        ReadProcPidMap(pCtx, info);
        m_procPidModTable.push_back(info);

        // insert an entry in sample map
        UpdateModuleSampleMap(info.m_modInfoTable, pCtx, componentIdx, ipc);
    }

    m_sampleIpcLoad[componentIdx] += ipc;

    return ret;
}

//ReadProcPidMap : Read the /proc/$pid/maps file and populate process ==> module map
//                  for each process
AMDTResult PwrProfTranslateLinux::ReadProcPidMap(ContextData* pCtx, ProcPidModInfo& info)
{
    AMDTResult ret = AMDT_ERROR_FAIL;

    if ((0 == pCtx->m_processId) && (m_isSwapperPid == false))
    {
        ModInfo moduleInfo;
        memcpy(moduleInfo.m_pModulename, PP_UNKNOWN_MODULE, AMDT_PWR_EXE_NAME_LENGTH);

        moduleInfo.m_startAddress = 0;
        moduleInfo.m_endAddress = 0xFFFFFFFFFFFFFFFF;
        moduleInfo.m_moduleId = m_moduleCnt;
        m_moduleIdInfoMap.insert({m_moduleCnt, moduleInfo});
        m_moduleCnt++;
        info.m_modInfoTable.push_back(moduleInfo);
        m_isSwapperPid = true;
        ret = AMDT_STATUS_OK;
    }
    else
    {
        AMDTUInt64 modStartAddress = 0;
        AMDTUInt64 modEndAddress = 0;

        char filename[OS_MAX_PATH];
        snprintf(filename, sizeof(filename), "/proc/%d/maps", pCtx->m_processId);

        // open /proc/pid/maps file
        FILE* fp = fopen(filename, "r");

        if (fp != NULL)
        {
            while (1)
            {
                char buffer[OS_MAX_PATH];
                char* pBuffer = buffer;
                int index;
                size_t size;

                if (fgets(buffer, sizeof(buffer), fp) == NULL)
                {
                    break;
                }

                /* 00400000-0040c000 r-xp 00000000 fd:01 41038  /bin/cat */
                index = PwrHexToUInt64(pBuffer, &modStartAddress);

                if (index < 0)
                {
                    continue;
                }

                pBuffer += index + 1;
                index = PwrHexToUInt64(pBuffer, &modEndAddress);

                if (index < 0)
                {
                    continue;
                }

                pBuffer += index + 3;

                if (*pBuffer == 'x')   /* vm_exec */
                {
                    char* execName = strchr(buffer, '/');

                    /* Catch VDSO */
                    if (execName == NULL)
                    {
                        execName = strstr(buffer, "[vdso]");
                    }

                    if (execName == NULL)
                    {
                        continue;
                    }

                    pBuffer += 3;

                    size = strlen(execName);
                    execName[size - 1] = '\0'; /* Remove \n*/

                    auto found = std::find_if(std::begin(info.m_modInfoTable),
                                              std::end(info.m_modInfoTable),
                    [ = ](const ModInfo & args) {return ((modStartAddress == args.m_startAddress) && (modEndAddress == args.m_endAddress));});

                    if (found == info.m_modInfoTable.end())
                    {
                        ModInfo moduleInfo;

                        memcpy(moduleInfo.m_pModulename, execName, size);

                        moduleInfo.m_startAddress = modStartAddress;
                        moduleInfo.m_endAddress = modEndAddress;
                        moduleInfo.m_moduleId = m_moduleCnt;
                        m_moduleIdInfoMap.insert({m_moduleCnt, moduleInfo});
                        m_moduleCnt++;
                        info.m_modInfoTable.push_back(moduleInfo);
                    }
                }
            }

            ret = AMDT_STATUS_OK;
            fclose(fp);
        }
    }

    return ret;

}

// UpdateModuleSampleMap: Update the moduleId ==> moduleInfo map for each sample collected
AMDTResult PwrProfTranslateLinux::UpdateModuleSampleMap(const std::vector<ModInfo>& modInfoTable,
                                                        ContextData* pCtx,
                                                        AMDTUInt32 componentIdx,
                                                        AMDTFloat32 ipc)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt64 ip = pCtx->m_ip;

    // true if IP lies within address range
    auto comp = [ = ](const ModInfo & args) {return ((ip >= args.m_startAddress) && (ip <= args.m_endAddress));};

    //search process ==> module table for ip entry
    auto moduleTableItr  = std::find_if(std::begin(modInfoTable), std::end(modInfoTable), comp);

    if (moduleTableItr != modInfoTable.end())
    {
        try
        {
            // get handle to the component map
            auto& moduleMapItr = m_modSampleDataTable.at(componentIdx);

            // search if module id in the component map
            auto module = moduleMapItr.find(moduleTableItr->m_moduleId);

            if (module != moduleMapItr.end())
            {
                module->second.m_ipc += ipc;
                module->second.m_sampleCnt++;
            }
            else
            {
                ModuleSampleData sampleData;
                sampleData.m_processId  = pCtx->m_processId;
                sampleData.m_threadId   = pCtx->m_threadId;
                sampleData.m_isKernel   = pCtx->m_isKernel;
                sampleData.m_ip         = pCtx->m_ip;
                sampleData.m_ipc        = ipc;
                sampleData.m_sampleCnt  = 1;
                moduleMapItr.insert({moduleTableItr->m_moduleId, sampleData});
            }
        }
        catch (const std::out_of_range& oor)
        {
            PwrTrace("Out of range.");
            ret = AMDT_ERROR_FAIL;
        }
    }
    else
    {
        // module entry not found in process ==> module table
        ret = AMDT_ERROR_FAIL;
    }

    return ret;
}

// CleanupModuleMap: cleanup process profiling structures
void PwrProfTranslateLinux::CleanupModuleMap()
{
    // clear the maps
    m_procPidModTable.clear();
    m_modSampleDataTable.clear();
    m_moduleIdInfoMap.clear();
    m_moduleCnt = 0;
    m_aggrModSampleDataMap.clear();
}

// GetProcessName: Get process name
void PwrProfTranslateLinux:: GetProcessName(AMDTPwrProcessInfo* pInfo)
{
    bool found = false;
    bool refreshed = false;

    if (nullptr != pInfo)
    {
        memset(pInfo->m_name, 0, AMDT_PWR_EXE_NAME_LENGTH);
        memset(pInfo->m_name, 0, AMDT_PWR_EXE_PATH_LENGTH);

        do
        {
            if (g_processNames.size() > 0)
            {
                for (auto itr : g_processNames)
                {
                    if (itr.m_pid == pInfo->m_pid)
                    {
                        memcpy(pInfo->m_name, itr.m_name, strlen(itr.m_name));
                        memcpy(pInfo->m_path, itr.m_path, strlen(itr.m_path));
                        found = true;
                        break;
                    }
                }
            }

            if (found == false)
            {
                found = GetProcessNameFromPid(pInfo);

                if (false == found)
                {
                    if (false == refreshed)
                    {
                        // Prepare the list again
                        PrepareInitialProcessList(g_processNames);
                        refreshed = true;
                    }
                    else
                    {
                        memcpy(pInfo->m_name, ERROR_READING_PROCESS_NAME, strlen(ERROR_READING_PROCESS_NAME) + 1);
                        memcpy(pInfo->m_path, ERROR_READING_PROCESS_PATH, strlen(ERROR_READING_PROCESS_PATH) + 1);
                        break;
                    }
                }
            }
        }
        while (false == found);
    }
}

//ExtractNameAndPath : Extrachr module name and path from full function
void PwrProfTranslateLinux::ExtractNameAndPath(char* pFullPath, char* pName, char* pPath)
{
    AMDTUInt32 cnt = 0;
    AMDTUInt32 begin = 0;
    char* path = pFullPath;
    bool prefix = false;

    while (path[cnt] != '\0')
    {
        if ('/' == path[cnt])
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
        memcpy(pName, PP_UNKNOWN_NAME, AMDT_PWR_EXE_NAME_LENGTH);
    }

    if (strlen(pPath) == 0)
    {
        memcpy(pPath, PP_UNKNOWN_PATH, AMDT_PWR_EXE_PATH_LENGTH);
    }
}
