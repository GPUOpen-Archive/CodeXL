//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Utils.cpp
///
//==================================================================================

// Qt:
#include <QtGui>

// Backend
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>
#include <AMDTDisassembler/inc/LibDisassembler.h>

// Infra
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osApplication.h>

// Project
#include <Utils.h>


// Static Functions
static bool IsWindowsSystemModuleNoExt(const gtString& absolutePath);
static bool IsLinuxSystemModuleNoExt(const gtString& absolutePath);
static bool AuxIsLinuxSystemModule(const gtString& absolutePath);


bool GetEventEncodeVec(CpuProfileReader& profileReader, EventEncodeVec& eventVec)
{
    eventVec = profileReader.getProfileInfo()->m_eventVec;

    return true;
}

bool GetEventToIndexMap(CpuProfileReader& profileReader, EventToIndexMap& evtToIdexMap)
{
    bool retVal = true;

    EventEncodeVec eventVec;
    eventVec = profileReader.getProfileInfo()->m_eventVec;

    // Event info
    EventEncodeVec::const_iterator eit = eventVec.begin(), eitEnd = eventVec.end();

    for (int i = 0; eit != eitEnd; ++eit, ++i)
    {
        if (evtToIdexMap.find(eit->eventMask) != evtToIdexMap.end())
        {
            retVal = false;
        }

        evtToIdexMap.insert(gtMap<EventMaskType, int>::value_type(eit->eventMask, i));
    }

    return retVal;
}


int GetIndexForEvent(EventToIndexMap& evtToIdexMap, const EventMaskType& eventMask)
{
    int index = -1;
    gtMap<EventMaskType, int>::iterator eit;

    eit = evtToIdexMap.find(eventMask);

    if (eit != evtToIdexMap.end())
    {
        index = (*eit).second;
    }

    return index;
}


bool GetEventDetailForIndex(EventEncodeVec& eventVec, gtUInt32 index, EventMaskType& eventMask, gtUInt64& eventCount)
{
    bool rv = false;
    EventEncodeVec::const_iterator eit = eventVec.begin(), eitEnd = eventVec.end();

    for (int i = 0; eit != eitEnd; ++eit, ++i)
    {
        if ((*eit).sortedIndex == index)
        {
            eventMask = (*eit).eventMask;
            eventCount = (*eit).eventCount;
            rv = true;
            break;
        }
    }

    return rv;
}


bool GetEventDetailForEventMask(EventEncodeVec& eventVec, EventMaskType eventMask, gtUInt32& index, gtUInt64& eventCount)
{
    bool rv = false;
    EventEncodeVec::const_iterator eit = eventVec.begin(), eitEnd = eventVec.end();

    for (int i = 0; eit != eitEnd; ++eit, ++i)
    {
        if ((*eit).eventMask == eventMask)
        {
            index = (*eit).sortedIndex;
            eventCount = (*eit).eventCount;
            rv = true;
            break;
        }
    }

    return rv;
}


bool GetNumberOfProcesses(CpuProfileReader& profileReader, gtUInt32& nbrProcs)
{
    bool retVal = false;

    PidProcessMap* procMap = profileReader.getProcessMap();

    if (nullptr != procMap)
    {
        nbrProcs = procMap->size();
        retVal = true;
    }

    return retVal;
}


bool GetProcessInfoList(CpuProfileReader&    profileReader,
                        bool                 sepByCore, // UNUSED NOW
                        gtUInt64             coreMask,  // UNUSED NOW - aggregate samples only for the specified cores
                        ProcessInfoList&     procList,
                        gtVector<gtUInt64>&  totalDataVector)
{
    GT_UNREFERENCED_PARAMETER(coreMask);
    bool retVal = false;
    gtUInt32 numCores = profileReader.getProfileInfo()->m_numCpus;
    gtUInt32 dataSize = (sepByCore) ? (numCores * profileReader.getProfileInfo()->m_numEvents)
                        : profileReader.getProfileInfo()->m_numEvents;

    totalDataVector.resize(dataSize);

    PidProcessMap* procMap = profileReader.getProcessMap();

    if (nullptr != procMap)
    {
        for (PidProcessMap::const_iterator it = procMap->begin(); it != procMap->end(); it++)
        {
            ProcessInfo procInfo;

            ExtractFileName((*it).second.getPath(), procInfo.m_processName);
            procInfo.m_pid = (*it).first;
            procInfo.m_hasCSS = (*it).second.m_hasCss;
            procInfo.m_is32Bit = (*it).second.m_is32Bit;

            // Aggregate the samples..
            //   Get the EventToIndexMap - which is required to aggregate the samples
            EventToIndexMap evtToIdxMap;
            GetEventToIndexMap(profileReader, evtToIdxMap);

            procInfo.m_dataVector.resize(dataSize);

            AggregateSamples(profileReader,
                             (*it).second,
                             procInfo.m_dataVector,
                             totalDataVector,
                             evtToIdxMap,
                             false);

            procList.push_back(procInfo);
        }

        retVal = true;
    }

    return retVal;
}

//
// Note: This does not aggregate the samples
//
bool GetProcessInfoMap(CpuProfileReader&    profileReader,
                       bool                 sepByCore,
                       gtUInt64             coreMask,
                       PidProcessInfoMap&   procInfoMap)
{
    GT_UNREFERENCED_PARAMETER(coreMask);
    bool retVal = false;
    gtUInt32 numCores = profileReader.getProfileInfo()->m_numCpus;
    gtUInt32 dataSize = (sepByCore) ? (numCores * profileReader.getProfileInfo()->m_numEvents)
                        : profileReader.getProfileInfo()->m_numEvents;

    PidProcessMap* procMap = profileReader.getProcessMap();

    if (nullptr != procMap)
    {
        for (PidProcessMap::const_iterator it = procMap->begin(); it != procMap->end(); it++)
        {
            ProcessInfo procInfo;
            ExtractFileName((*it).second.getPath(), procInfo.m_processName);
            procInfo.m_pid = (*it).first;
            procInfo.m_hasCSS = (*it).second.m_hasCss;
            procInfo.m_is32Bit = (*it).second.m_is32Bit;

            procInfo.m_dataVector.resize(dataSize);

            procInfoMap.insert(PidProcessInfoMap::value_type(procInfo.m_pid, procInfo));
        }

        retVal = true;
    }

    return retVal;
}


bool GetModuleInfoList(CpuProfileReader&    profileReader,
                       ProcessIdType        pid,     // if -1, aggregate across all the PIDs
                       gtUInt64             flags,   // ignore-system-modules, separate-by-core,
                       gtUInt64             coreMask, // UNUSED NOW, aggregate samples for the specified cores
                       ModuleInfoList&      modList,
                       gtVector<gtUInt64>&  totalDataVector,
                       PidProcessInfoMap*   pProcInfoMap)
{
    GT_UNREFERENCED_PARAMETER(coreMask);
    bool retVal = false;
    bool ignoreSysModule = ((flags & SAMPLE_IGNORE_SYSTEM_MODULES) == SAMPLE_IGNORE_SYSTEM_MODULES) ? true : false;
    bool sepByCore = ((flags & SAMPLE_SEPARATE_BY_CORE) == SAMPLE_SEPARATE_BY_CORE) ? true : false;

    gtUInt32 numCores = profileReader.getProfileInfo()->m_numCpus;
    gtUInt32 dataSize = (sepByCore) ? (numCores * profileReader.getProfileInfo()->m_numEvents)
                        : profileReader.getProfileInfo()->m_numEvents;

    totalDataVector.resize(dataSize);

    NameModuleMap* modMap = profileReader.getModuleMap();

    if (nullptr != modMap)
    {
        for (NameModuleMap::const_iterator it = modMap->begin(); it != modMap->end(); it++)
        {
            ModuleInfo modInfo;
            const CpuProfileModule* pMod = &((*it).second);

            if (pMod->isIndirect())
            {
                continue;
            }

            if (!pMod->m_isImdRead)
            {
                pMod = GetModuleDetail(profileReader, pMod->getPath(), nullptr);

                if (nullptr == pMod)
                {
                    continue;
                }
            }

            if (ignoreSysModule && pMod->isSystemModule())
            {
                continue;
            }

            pMod->extractFileName(modInfo.m_moduleName);
            modInfo.m_modulePath = pMod->getPath();
            modInfo.m_isSystemModule = pMod->isSystemModule();

            for (PidAggregatedSampleMap::const_iterator pidIt = pMod->getBeginSample(); pidIt != pMod->getEndSample(); pidIt++)
            {
                ProcessIdType sampPid = (*pidIt).first;

                if ((static_cast<ProcessIdType>(ALL_PROCESS_IDS) != pid) && (sampPid != pid))
                {
                    continue;
                }

                const AggregatedSample* pSamp = &((*pidIt).second);

                // Aggregate the samples..
                //   Get the EventToIndexMap - which is required to aggregate the samples
                EventToIndexMap evtToIdxMap;
                GetEventToIndexMap(profileReader, evtToIdxMap);

                modInfo.m_dataVector.resize(dataSize);

                gtVector<gtUInt64> tmpDataVec;
                tmpDataVec.resize(dataSize);

                // Aggregate across all the processes
                AggregateSamples(profileReader,
                                 *pSamp,
                                 tmpDataVec, // modInfo.m_dataVector,
                                 totalDataVector,
                                 evtToIdxMap,
                                 false);

                // To fill-in per process structure..
                modInfo.m_pidDataVector.insert(gtMap<ProcessIdType, gtVector<gtUInt64> >::value_type(sampPid, tmpDataVec));

                // Total samples for this module across all the processes
                for (gtUInt32 i = 0; i < dataSize; i++)
                {
                    modInfo.m_dataVector[i] += tmpDataVec[i];
                }

                // If ProcessInfoMap is provided, update the samples for this PID
                if (nullptr != pProcInfoMap)
                {
                    PidProcessInfoMap::iterator pIt = pProcInfoMap->find(sampPid);

                    if (pIt != pProcInfoMap->end())
                    {
                        for (gtUInt32 i = 0; i < dataSize; i++)
                        {
                            (*pIt).second.m_dataVector[i] += tmpDataVec[i];
                        }
                    }
                }
            }

            // only if there is a valid dataVector, add to moduleinfolist
            if (! modInfo.m_dataVector.empty())
            {
                modList.push_back(modInfo);
            }
        }

        retVal = true;
    }

    return retVal;
}


CpuProfileModule* GetModuleDetail(CpuProfileReader&  profileReader,
                                  const gtString&    modulePathGt,
                                  ExecutableFile**   ppExe)
{
    GT_UNREFERENCED_PARAMETER(ppExe);
    CpuProfileModule* pModule = nullptr;

    pModule = profileReader.getModuleDetail(modulePathGt);

    // Set the system module
    if (nullptr != pModule)
    {
        pModule->setSystemModule(IsSystemModule(pModule->getPath()));
    }

    return pModule;
}


//
// Get the functions for the given PID/TID/MODULE
// If PID is -1, get functions from all the PIDs
// If TID is -1 get functions from all the PIDS
//
bool GetFunctionInfoList(CpuProfileReader&        profileReader,
                         ProcessIdType            pid,
                         ThreadIdType             tid,
                         gtUInt64                 flags,   // ignore-system-modules, separate-by-core, etc
                         gtUInt64                 coreMask,
                         FunctionInfoList&        funcInfoList)
{
    bool retVal = false;
    bool ignoreSysModule = ((flags & SAMPLE_IGNORE_SYSTEM_MODULES) == SAMPLE_IGNORE_SYSTEM_MODULES) ? true : false;

    NameModuleMap* pNameModuleMap = profileReader.getModuleMap();

    if (nullptr != pNameModuleMap)
    {
        AggregatedSample parentSamples;
        // funcIdxMap maps baseAddress of a function to the
        // corresponding index of the FunctionInfo in the funcInfoList.
        // As funcInfoList is not sorted, funcIdxMap helps in function search.
        FunctionIdxMap funcIdxMap;

        NameModuleMap::iterator mit = pNameModuleMap->begin(), mEnd = pNameModuleMap->end();

        for (; mit != mEnd; ++mit)
        {
            // Get the current module:
            CpuProfileModule* pModule = &(mit->second);

            if (pModule->isIndirect())
            {
                continue;
            }

            if (!pModule->m_isImdRead)
            {
                pModule = GetModuleDetail(profileReader, pModule->getPath(), nullptr);

                if (nullptr == pModule)
                {
                    continue;
                }
            }

            if (ignoreSysModule && pModule->isSystemModule())
            {
                continue;
            }

            if (CpuProfileModule::UNMANAGEDPE == pModule->m_modType ||
                CpuProfileModule::JAVAMODULE  == pModule->m_modType ||
                CpuProfileModule::MANAGEDPE   == pModule->m_modType)
            {
                retVal = GetFunctionInfoList(profileReader,
                                             pid,
                                             tid,
                                             *pModule,
                                             pModule->getPath(),
                                             flags,
                                             coreMask,
                                             funcInfoList,
                                             funcIdxMap);
            }
        }
    }

    return retVal;
}


//
// Get the functions for the given PID/TID/MODULE
// If PID is -1, get functions from all the PIDs
// If TID is -1 get functions from all the PIDS
//
bool GetFunctionInfoList(CpuProfileReader&        profileReader,
                         ProcessIdType            pid,
                         ThreadIdType             tid,
                         CpuProfileModule&        module,
                         const gtString&          moduleFilePath,
                         gtUInt64                 flags,        // ignore-system-modules, separate-by-core, etc
                         gtUInt64                 coreMask,
                         FunctionInfoList&        funcInfoList,
                         FunctionIdxMap&          funcIdxMap)
{
    GT_UNREFERENCED_PARAMETER(coreMask);
    GT_UNREFERENCED_PARAMETER(moduleFilePath);
    FunctionInfo funcInfo;
    gtVector<gtUInt64> totalDataVector;

    bool ignoreSysModule = ((flags & SAMPLE_IGNORE_SYSTEM_MODULES) == SAMPLE_IGNORE_SYSTEM_MODULES) ? true : false;
    bool sepByCore = ((flags & SAMPLE_SEPARATE_BY_CORE) == SAMPLE_SEPARATE_BY_CORE) ? true : false;

    bool groupByModule = ((flags & SAMPLE_GROUP_BY_MODULE) == SAMPLE_GROUP_BY_MODULE) ? true : false;
    // TODO: - group by thread
    bool groupByThread = ((flags & SAMPLE_GROUP_BY_THREAD) == SAMPLE_GROUP_BY_THREAD) ? true : false;

    gtUInt32 numCores = profileReader.getProfileInfo()->m_numCpus;
    gtUInt32 dataSize = (sepByCore) ? (numCores * profileReader.getProfileInfo()->m_numEvents)
                        : profileReader.getProfileInfo()->m_numEvents;

    if (!ignoreSysModule || !module.isSystemModule())
    {
        // For each function
        AddrFunctionMultMap::iterator fit = module.getBeginFunction();
        AddrFunctionMultMap::iterator fEnd = module.getEndFunction();

        for (; fit != fEnd; ++fit)
        {
            CpuProfileFunction& function = fit->second;

            // For each sample
            AptAggregatedSampleMap::const_iterator sit = function.getBeginSample();
            AptAggregatedSampleMap::const_iterator sEnd = function.getEndSample();

            gtString funcName;
            CpuProfileFunction* pFunc = &function;

            for (; sit != sEnd; ++sit)
            {
                const AptKey& aptKey = sit->first;
                gtVAddr sampAddr = aptKey.m_addr;
                bool rc = false;
                ProcessIdType sampPid = (groupByModule) ? static_cast<ProcessIdType>(-1) : aptKey.m_pid;
                ThreadIdType sampTid = (groupByThread) ? aptKey.m_tid : static_cast<ThreadIdType>(-1);

                if (sampPid != pid)
                {
                    continue;
                }

                if (sampTid != tid)
                {
                    continue;
                }

                funcName.makeEmpty();

                switch (module.m_modType)
                {
                    // Normal PE module:
                    case CpuProfileModule::UNMANAGEDPE:
                        if (module.isUnchartedFunction(function))
                        {
                            sampAddr += module.getBaseAddr();

                            // Find the name of the function
                            rc = GetFunctionName(&module, sampAddr, funcName, &pFunc);

                            if (rc)
                            {
                                sampAddr = pFunc->getBaseAddr();
                            }
                        }
                        else
                        {
                            funcName = pFunc->getFuncName(); // FIXME, if the func name is empty ?
                            sampAddr = pFunc->getBaseAddr();
                        }

                        break;

                    case CpuProfileModule::JAVAMODULE:
                    case CpuProfileModule::MANAGEDPE:
                        // For now, Putting this specific check for Java/CLR
                        // At some point, need to re-structure this function to properly handle all module type
                        // For Java/CLR, func name/addr info is already present and just need to set in outer loop (for each func)
                        // Probably the better is to define separate functions for each module type like in old GUI code
                        sampAddr = function.getBaseAddr();
                        funcName = function.getFuncName();
                        break;

                    case CpuProfileModule::UNKNOWNMODULE:
                    case CpuProfileModule::UNKNOWNKERNELSAMPLES:
                        // TODO: Handle Unknown Kernel Samples and Unknown Module here
                        // Convert the "No symbol" to a wide char:
                        break;

                    default:
                        break;
                }

                // Find the function in the unique
                FunctionInfo* pFuncInfo = &funcInfo;
                bool isNewFunc = false;

                // Check whether the new function is already there in the funcInfoList
                pFuncInfo = FindFunction(funcInfoList, funcIdxMap, sampAddr, pid, tid);

                if (nullptr == pFuncInfo)
                {
                    pFuncInfo = &funcInfo;

                    funcInfo.m_baseAddress = sampAddr;
                    funcInfo.m_functionName = funcName;
                    funcInfo.m_pModule = &module;
                    funcInfo.m_dataVector.clear();

                    funcInfo.m_pid = pid;
                    funcInfo.m_tid = tid;

                    funcInfo.m_dataVector.resize(dataSize);
                    isNewFunc = true;
                }

                totalDataVector.resize(dataSize);

                // Aggregate the samples..
                //   Get the EventToIndexMap - which is required to aggregate the samples
                EventToIndexMap evtToIdxMap;
                GetEventToIndexMap(profileReader, evtToIdxMap);

                AggregateSamples(profileReader,
                                 (*sit).second,
                                 pFuncInfo->m_dataVector,
                                 totalDataVector,
                                 evtToIdxMap,
                                 sepByCore);

                if (isNewFunc)
                {
                    funcInfoList.push_back(funcInfo);
                    funcIdxMap.insert({ sampAddr, funcInfoList.size() - 1 });
                }
            } // AptAggregatedSampleMap
        } // AddrFunctionMultMap
    }

    return true;
}


bool GetImixInfoList(CpuProfileReader&    profileReader,
                     ProcessIdType        pid,
                     ThreadIdType         tid,
                     gtUInt64             flags,
                     gtUInt64             coreMask,
                     ModuleImixInfoList&  modImixInfoList,
                     gtUInt64&            totalSamples)
{
    totalSamples = 0;
    NameModuleMap* pNameModuleMap = profileReader.getModuleMap();

    if (nullptr != pNameModuleMap)
    {
        AggregatedSample parentSamples;
        auto mit = pNameModuleMap->begin();
        auto mEnd = pNameModuleMap->end();

        for (; mit != mEnd; ++mit)
        {
            // Get the current module
            CpuProfileModule* pModule = &(mit->second);

            if (pModule->isIndirect())
            {
                continue;
            }

            if (!pModule->m_isImdRead)
            {
                pModule = GetModuleDetail(profileReader, pModule->getPath(), nullptr);

                if (nullptr == pModule)
                {
                    continue;
                }
            }

            if (CpuProfileModule::UNMANAGEDPE == pModule->m_modType ||
                CpuProfileModule::JAVAMODULE == pModule->m_modType ||
                CpuProfileModule::MANAGEDPE == pModule->m_modType)
            {
                gtUInt64 modSamples = 0;

                GetImixInfoList(profileReader,
                                pid,
                                tid,
                                *pModule,
                                flags,
                                coreMask,
                                modImixInfoList,
                                modSamples);

                totalSamples += modSamples;
            }
        }
    }

    return true;
}

bool GetImixInfoList(CpuProfileReader&    profileReader,
                     ProcessIdType        pid,
                     ThreadIdType         tid,
                     CpuProfileModule&    module,
                     gtUInt64             flags,
                     gtUInt64             coreMask,
                     ModuleImixInfoList&  modImixInfoList,
                     gtUInt64&            totalSamples)
{
    GT_UNREFERENCED_PARAMETER(profileReader);
    GT_UNREFERENCED_PARAMETER(tid);
    GT_UNREFERENCED_PARAMETER(coreMask);

    bool rv = true;
    bool ignoreSysModule = ((flags & SAMPLE_IGNORE_SYSTEM_MODULES) == SAMPLE_IGNORE_SYSTEM_MODULES) ? true : false;
    bool groupByModule = ((flags & SAMPLE_GROUP_BY_MODULE) == SAMPLE_GROUP_BY_MODULE) ? true : false;

    totalSamples = 0;

    if (!ignoreSysModule || !module.isSystemModule())
    {
        gtString exePath = module.getPath();
        ExecutableFile* pExecutable = ExecutableFile::Open(exePath.asCharArray(), module.getBaseAddr());

        if (nullptr == pExecutable)
        {
            rv = false;
        }

        if (rv)
        {
            const gtUByte* pCode = nullptr;
            gtRVAddr sectionStartRva = 0, sectionEndRva = 0;
            unsigned int prevSectionIndex = static_cast<unsigned int>(-1);

            ModuleImixInfo modInfo;
            modInfo.m_pModule = &module;
            gtUInt64 totalModSamples = 0;

            // Setup disassembler
            LibDisassembler dasm;
            // if the code is 64-bits
            bool isLongMode = pExecutable->Is64Bit();
            dasm.SetLongMode(isLongMode);

            // For each function
            auto fit = module.getBeginFunction();
            auto fitEnd = module.getEndFunction();

            for (; fit != fitEnd; ++fit)
            {
                CpuProfileFunction& function = fit->second;

                // For each sample
                auto sit = function.getBeginSample();
                auto sitEnd = function.getEndSample();

                for (; sit != sitEnd; ++sit)
                {
                    const AptKey& aptKey = sit->first;
                    gtVAddr funcOffset = aptKey.m_addr;
                    ProcessIdType sampPid = (groupByModule) ? static_cast<ProcessIdType>(-1) : aptKey.m_pid;

                    if (sampPid != pid)
                    {
                        continue;
                    }

                    gtRVAddr startRVAddr = pExecutable->VaToRva(function.getBaseAddr() + funcOffset);
                    unsigned int sectionIndex = pExecutable->LookupSectionIndex(startRVAddr);

                    BYTE error_code;
                    UIInstInfoType temp_struct;
                    char dasmArray[256] = { 0 };
                    unsigned int strlength = 255;

                    if (pExecutable->GetSectionsCount() <= sectionIndex)
                    {
                        strcpy(dasmArray, "BAD DASM");
                    }
                    else if (sectionIndex == prevSectionIndex)
                    {
                        gtRVAddr codeOffset = startRVAddr - sectionStartRva;
                        const gtUByte* pCurrentCode = pCode + codeOffset;

                        // Get disassembly for the current pCode from the disassembler
                        HRESULT hr = dasm.UIDisassemble((BYTE*)pCurrentCode, (unsigned int*)&strlength, (BYTE*)dasmArray, &temp_struct, &error_code);

                        if (S_OK != hr)
                        {
                            strcpy(dasmArray, "BAD DASM");
                        }
                    }
                    else
                    {
                        pCode = pExecutable->GetSectionBytes(sectionIndex);
                        pExecutable->GetSectionRvaLimits(sectionIndex, sectionStartRva, sectionEndRva);

                        // GetCodeBytes return the pointer to the sectionStart
                        // We need to add the offset to the beginning of the function
                        gtRVAddr codeOffset = startRVAddr - sectionStartRva;
                        const gtUByte* pCurrentCode = pCode + codeOffset;

                        // Get disassembly for the current pCode from the disassembler
                        HRESULT hr = dasm.UIDisassemble((BYTE*)pCurrentCode, (unsigned int*)&strlength, (BYTE*)dasmArray, &temp_struct, &error_code);

                        if (S_OK != hr)
                        {
                            strcpy(dasmArray, "BAD DASM");
                        }

                        prevSectionIndex = sectionIndex;
                    }

                    string dasmString(dasmArray);
                    auto it = modInfo.m_InstMap.find(dasmString);
                    gtUInt64 sampleCount = sit->second.getTotal();

                    if (it == modInfo.m_InstMap.end())
                    {
                        modInfo.m_InstMap.insert({ dasmString, sampleCount });
                    }
                    else
                    {
                        it->second += sampleCount;
                    }

                    totalModSamples += sampleCount;
                }
            }

            delete pExecutable;

            modInfo.m_samplesCount = totalModSamples;
            modImixInfoList.push_back(modInfo);
            totalSamples += totalModSamples;
        }
    }

    return rv;
}

bool GetImixSummaryMap(CpuProfileReader&    profileReader,
                       ModuleImixInfoList&  modImixInfoList,
                       ImixSummaryMap&      imixSummaryMap,
                       gtUInt64&            totalSamples)
{
    GT_UNREFERENCED_PARAMETER(profileReader);
    totalSamples = 0;

    for (const auto& mit : modImixInfoList)
    {
        for (const auto& it : mit.m_InstMap)
        {
            auto sit = imixSummaryMap.find(it.first);

            if (sit == imixSummaryMap.end())
            {
                imixSummaryMap.insert({ it.first, it.second });
            }
            else
            {
                sit->second += it.second;
            }
        }

        totalSamples += mit.m_samplesCount;
    }

    return true;
}

// Convert an aggregated sample to data array
gtUInt32 AggregateSamples(CpuProfileReader&        profileReader,
                          const AggregatedSample&  sm,
                          gtVector<gtUInt64>&      dataVector,
                          gtVector<gtUInt64>&      totalVector,
                          EventToIndexMap&         evtToIdxMap,
                          bool                     seperateByCore)
{
    gtUInt32 total = 0;

    {
        CpuProfileSampleMap::const_iterator sampleIt = sm.getBeginSample();
        CpuProfileSampleMap::const_iterator sampleItEnd = sm.getEndSample();

        for (; sampleIt != sampleItEnd; ++sampleIt)
        {
            gtUInt32 sampleCount = sampleIt->second;
            total += sampleCount;

            // If separateByCore, then idx = (number-of-events * eventidx)
            // Given cpu/event select from profile, find column index:
            int index = GetIndexForEvent(evtToIdxMap, sampleIt->first.event);

            if (seperateByCore)
            {
                index *= profileReader.getProfileInfo()->m_numCpus;
            }

            // TODO: if ((index >= 0) && (index < (int)dataVector.size()))
            if (index >= 0)
            {
                // Aggregate total:
                totalVector[index] += sampleCount;

                dataVector[index] += sampleCount;
            }
        }
    }

    return total;
}


bool GetFunctionName(const CpuProfileModule* pModule, gtVAddr va, gtString& name, CpuProfileFunction** ppFunc)
{
    bool ret = false;
    const CpuProfileFunction* pFunction = nullptr;

    if (nullptr != pModule)
    {
        pFunction = pModule->findFunction(va);
    }

    if ((nullptr != pFunction)
        && (!pFunction->getFuncName().isEmpty())
        && (!pModule->isUnchartedFunction(*pFunction)))
    {
        name = pFunction->getFuncName();

        if (nullptr != ppFunc)
        {
            *ppFunc = const_cast<CpuProfileFunction*>(pFunction);
        }

        ret = true;
    }
    else
    {
        // use module name
        gtString modFile;

        if ((nullptr != pModule) && (pModule->extractFileName(modFile)))
        {
            name = modFile;
            name += '!';
        }

        // Append the sample address
        name.appendFormattedString(L"0x%08llx", static_cast<unsigned long long>(va));
    }

    return ret;
}


bool IsSystemModule(const gtString& absolutePath)
{
    bool ret;

    if (absolutePath.length() > 4 && (absolutePath.endsWith(L".dll") ||
                                      absolutePath.endsWith(L".sys") ||
                                      absolutePath.endsWith(L".exe")))
    {
        ret = IsWindowsSystemModuleNoExt(absolutePath);
    }
    else
    {
        ret = AuxIsLinuxSystemModule(absolutePath);
    }

    return ret;
}



FunctionInfo* FindFunction(
    FunctionInfoList& funcInfoList,
    FunctionIdxMap& funcIdxMap,
    gtVAddr baseAddr,
    ProcessIdType pid,
    ThreadIdType tid)
{
    FunctionInfo* pFunc = nullptr;
    size_t count = funcIdxMap.count(baseAddr);

    // Most of the time count is zero, i.e. a new function found.
    // Add (count != 0) check to return from this function early.
    if (count != 0)
    {
        if (count == 1)
        {
            gtUInt32 idx = funcIdxMap.find(baseAddr)->second;
            pFunc = &(funcInfoList[idx]);

            if (pFunc->m_pid != pid || pFunc->m_tid != tid)
            {
                pFunc = nullptr;
            }
        }
        else if (count > 1)
        {
            auto range = funcIdxMap.equal_range(baseAddr);

            for (auto it = range.first; it != range.second; ++it)
            {
                pFunc = &(funcInfoList[it->second]);

                if (pFunc->m_pid == pid && pFunc->m_tid == tid)
                {
                    break;
                }

                pFunc = nullptr;
            }
        }
    }

    return pFunc;
}


static bool IsWindowsSystemModuleNoExt(const gtString& absolutePath)
{
    bool ret = false;

    // 21 is the minimum of: "\\windows\\system\\*.***"
    if (absolutePath.length() >= 21)
    {
        gtString lowerAbsolutePath = absolutePath;

        for (int i = 0, e = lowerAbsolutePath.length(); i != e; ++i)
        {
            wchar_t& wc = lowerAbsolutePath[i];

            if ('/' == wc)
            {
                wc = '\\';
            }
            else
            {
                // Paths in windows are case insensitive
                wc = tolower(wc);
            }
        }

        int rootPos = lowerAbsolutePath.find(L"\\windows\\");

        if (-1 != rootPos)
        {
            // 9 is the length of "\\windows\\"
            rootPos += 9;

            if (lowerAbsolutePath.compare(rootPos, 3, L"sys") == 0)
            {
                rootPos += 3;

                if (lowerAbsolutePath.compare(rootPos, 4, L"tem\\")   == 0 || // "\\windows\\system\\"
                    lowerAbsolutePath.compare(rootPos, 6, L"tem32\\") == 0 || // "\\windows\\system32\\"
                    lowerAbsolutePath.compare(rootPos, 6, L"wow64\\") == 0)   // "\\windows\\syswow64\\"
                {
                    ret = true;
                }
            }
            else
            {
                if (lowerAbsolutePath.compare(rootPos, 7, L"winsxs\\") == 0)
                {
                    ret = true;
                }
            }
        }
    }

    return ret;
}

static bool IsLinuxSystemModuleNoExt(const gtString& absolutePath)
{
    // Kernel samples
    bool ret = (absolutePath.find(L"[kernel.kallsyms]") != -1);

    if (!ret && L'/' == absolutePath[0])
    {
        if (absolutePath.compare(1, 3, L"lib") == 0)
        {
            ret = true;
        }
        else
        {
            if (absolutePath.compare(1, 4, L"usr/") == 0)
            {
                if (absolutePath.compare(5, 3,  L"lib")       ||
                    absolutePath.compare(5, 9,  L"local/lib") ||
                    absolutePath.compare(5, 10, L"share/gdb") == 0)
                {
                    ret = true;
                }
            }
        }
    }

    return ret;
}

// This function tries to tell whether a given module name is a Linux system library.
//
// The special name "[kernel.kallsyms]" is the module name for samples within the kernel.
// Then, if the path does not start with '/' we assume it's not a system library.
// The name must then start with "lib" and have ".so" within it.
// If so, we consider these files to be system libraries if they are from:
//          /lib*
//          /usr/lib*
//          /usr/local/lib*
//          /usr/share/gdb*
//
bool AuxIsLinuxSystemModule(const gtString& absolutePath)
{
    bool ret;

    int len = absolutePath.length();

    if (len > 3 && 0 == memcmp(absolutePath.asCharArray() + len - 3, L".so", 3 * sizeof(wchar_t)))
    {
        ret = IsLinuxSystemModuleNoExt(absolutePath);
    }
    else
    {
        ret = false;
    }

    return ret;
}


bool InitializeEventsXMLFile(EventsFile& eventsFile)
{
    bool rv = true;
    osCpuid cpuInfo;
    int model = cpuInfo.getModel();

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

        if (!eventEngine.Initialize(convertToQString(eventFilePathStr)))
        {
            rv = false;
        }
    }

    if (rv)
    {
        // Get event file path
        QString eventFile;
        eventFile = eventEngine.GetEventFilePath(cpuInfo.getFamily(), model);

        // Initialize event file
        if (!eventsFile.Open(eventFile))
        {
            rv = false;
        }
    }

    return rv;
}
