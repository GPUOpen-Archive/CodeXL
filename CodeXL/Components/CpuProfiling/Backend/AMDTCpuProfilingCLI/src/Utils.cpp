//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Utils.cpp
///
//==================================================================================

// Infra
#include <AMDTOSWrappers/Include/osFilePath.h>

// Project
#include <Utils.h>

#if AMDT_CPCLI_ENABLE_IMIX
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

                    std::string dasmString(dasmArray);
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
#endif //0