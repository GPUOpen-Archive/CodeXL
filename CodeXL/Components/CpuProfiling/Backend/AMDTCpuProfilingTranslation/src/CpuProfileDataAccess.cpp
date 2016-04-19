//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileDataAccess.cpp
/// \brief CPU profile translated data access interface implementation.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingTranslation/src/CpuProfileDataAccess.cpp#17 $
// Last checkin:   $DateTime: 2016/04/14 01:44:54 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569055 $
//=====================================================================

// Suppress Qt header warnings
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning(push)
    #pragma warning(disable : 4127 4718)
#endif
#include <QtWidgets>
#include <QDir>
#include <QMutexLocker>
#include <QDateTime>
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning(pop)
#endif

#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osProcess.h>

#include "CpuProfileDataAccess.h"
#include <AMDTCpuProfilingRawData/inc/CpuProfileWriter.h>
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>
#include <AMDTCpuProfilingRawData/inc/RunInfo.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <AMDTExecutableFormat/inc/ExecutableFile.h>

    // TODO: This will promoted into a library
    // #include "jpa.h" (Who uses it here)??

    const QString TI_FILE_EXT = ".ti";
    const QString PRD_FILE_EXT = ".prd";

#else //WINDOWS_ONLY


#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

#define KERNEL64_SPACE_START    0xfffff80000000000
#define KERNEL_SPACE_START  0x80000000


static wchar_t* CopyWcharStr(wchar_t* strDest, const wchar_t* strSource, size_t count)
{
    wchar_t* ret = wcsncpy(strDest, strSource, count);

    // To make sure that the string ends with NULL
    strDest[count] = L'\0';

    return ret;
}

///////////////////////////////////////////////////

ModuleAggregator::ModuleAggregator()
{
    m_hashCount = 0;
    m_hashArray = NULL;
}

ModuleAggregator::~ModuleAggregator()
{
    wchar_t* prevFileDelete = NULL;
    wchar_t* prevNameDelete = NULL;

    if (NULL != m_hashArray)
    {
        for (int i = 0; i < m_hashCount; i++)
        {
            QMap<InstructionKey, InstAggregate>::iterator it;

            for (it = (m_hashArray[i]).begin(); it != (m_hashArray[i]).end(); it++)
            {
                //since the same name may have been used multiple times, we
                // don't want to try to delete it more than once
                if ((NULL != it->instData.pJitDataFile) &&
                    (prevFileDelete != it->instData.pJitDataFile))
                {
                    prevFileDelete = it->instData.pJitDataFile;
                    delete [] it->instData.pJitDataFile;
                }

                if ((NULL != it->instData.pJitFunctionName) &&
                    (prevNameDelete != it->instData.pJitFunctionName))
                {
                    prevNameDelete = it->instData.pJitFunctionName;
                    delete [] it->instData.pJitFunctionName;
                }
            }
        }

        delete [] m_hashArray;
        m_hashArray = NULL;
        m_hashCount = 0;
    }

    gtList<InstAggregate>::iterator instIt;

    for (instIt = m_dataList.begin(); instIt != m_dataList.end(); instIt++)
    {
        //since the same name may have been used multiple times, we
        // don't want to try to delete it more than once
        if ((NULL != instIt->instData.pJitDataFile) &&
            (prevFileDelete != instIt->instData.pJitDataFile))
        {
            prevFileDelete = instIt->instData.pJitDataFile;
            delete [] instIt->instData.pJitDataFile;
        }

        if ((NULL != instIt->instData.pJitFunctionName) &&
            (prevNameDelete != instIt->instData.pJitFunctionName))
        {
            prevNameDelete = instIt->instData.pJitFunctionName;
            delete [] instIt->instData.pJitFunctionName;
        }
    }

    m_dataList.clear();
}


HRESULT ModuleAggregator::Initialize(const RawAggregate* pData)
{
    //allocate at least 1 hash
    //if it starts with "unknown" or we don't know the module
    m_hashCount = 1;

    m_address = pData->rawData.loadAddress;
    m_size = pData->rawData.moduleSize;

    m_is64Bit = ProfileDataSet::IsModule64Bit(pData->rawData.path, pData->rawData.address,
                                              pData->rawData.processId);

    m_type = pData->rawData.type;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    wcscpy_s(m_moduleName, OS_MAX_PATH, pData->rawData.path);

    ExecutableFile* pExecutable = ExecutableFile::Open(pData->rawData.path);

    if (NULL != pExecutable)
    {
        if (0 == m_size)
        {
            m_size = pExecutable->GetImageSize();
        }

        //if applicable, break the files into 4K chunks
        m_hashCount += pExecutable->GetCodeSize() / HASHSIZE;
        delete pExecutable;
        pExecutable = NULL;
    }

#else
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    m_hashArray = new QMap<InstructionKey, InstAggregate>[m_hashCount];

    if (NULL == m_hashArray)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}


void ModuleAggregator::AddSample(const RawAggregate* pData)
{
    int hashIndex = 0;
    gtUInt64 offset = pData->rawData.address - pData->rawData.loadAddress;

    if (1 != m_hashCount)
    {
        //note that the same module may be loaded multiple times at different
        // addresses
        hashIndex = static_cast<int>(offset / HASHSIZE);

        if (hashIndex >= m_hashCount)
        {
            hashIndex = m_hashCount - 1;
        }
    }

    gtVAddr modSampleAddr = offset + m_address;
    InstructionKey instKey;
    instKey.offset = offset;
    instKey.addr = pData->rawData.loadAddress;
    instKey.pid = pData->rawData.processId;
    instKey.thread_id = pData->rawData.threadId;

    QMap<InstructionKey, InstAggregate>& hashMap = m_hashArray[hashIndex];

    if (!hashMap.contains(instKey))
    {
        hashMap[instKey].instData.address = modSampleAddr;
        hashMap[instKey].instData.processId = pData->rawData.processId;
        hashMap[instKey].instData.threadId = pData->rawData.threadId;
        hashMap[instKey].instData.jitAddress = pData->rawData.loadAddress;

        if (NULL == pData->rawData.pJitDataFile)
        {
            hashMap[instKey].instData.pJitDataFile = NULL;
        }
        else
        {
            //differently loaded jit samples shouldn't have the same base
            hashMap[instKey].instData.address = pData->rawData.address;

            size_t len = wcslen(pData->rawData.pJitDataFile) + 1;
            hashMap[instKey].instData.pJitDataFile = new wchar_t[len];
            CopyWcharStr(hashMap[instKey].instData.pJitDataFile, pData->rawData.pJitDataFile, len);
        }

        if (NULL == pData->rawData.pJitFunctionName)
        {
            hashMap[instKey].instData.pJitFunctionName = NULL;
        }
        else
        {
            size_t len = wcslen(pData->rawData.pJitFunctionName) + 1;
            hashMap[instKey].instData.pJitFunctionName = new wchar_t[len];
            CopyWcharStr(hashMap[instKey].instData.pJitFunctionName, pData->rawData.pJitFunctionName, len);
        }
    }

    CpuProfileDataAccess::Aggregate(&(hashMap[instKey].sampleData), &(pData->sampleData));

    gtUInt32 pid = pData->rawData.processId;

    if (!m_aggProcessData.contains(pid))
    {
        m_aggProcessData[pid].modData.processId = pid;
    }

    CpuProfileDataAccess::Aggregate(&(m_aggProcessData[pid].sampleData), &(pData->sampleData));
}


void ModuleAggregator::AggregrateHashes()
{
    for (int i = 0; i < m_hashCount; i++)
    {
        QMap<InstructionKey, InstAggregate>::iterator it, itEnd;

        for (it = (m_hashArray[i]).begin(), itEnd = (m_hashArray[i]).end(); it != itEnd; ++it)
        {
            m_dataList.push_back(*it);
        }
    }

    delete [] m_hashArray;
    m_hashArray = NULL;
    m_hashCount = 0;
}

HRESULT ModuleAggregator::FillInstructionList(gtUInt64 coreRule,
                                              const QSet<gtUInt64>& rulePid,
                                              const QSet<gtUInt64>& ruleTid,
                                              gtList<InstAggregate>* pInstructionData,
                                              CpuProfileReader* pReader)
{
    gtList<InstAggregate>::iterator it;

    //if data not read yet...
    if ((0 == m_dataList.size()) && (NULL != pReader))
    {
        //read data
        gtString modName = gtString(m_moduleName);
        modName += L'\0';
        CpuProfileModule* pModDetail = pReader->getModuleDetail(modName);

        if (pModDetail == NULL)
        {
            return E_NOFILE;
        }

        m_type = (ModuleType)pModDetail->m_modType;
        m_address = pModDetail->getBaseAddr();

        AddrFunctionMultMap::const_iterator funcIt;

        for (funcIt = pModDetail->getBeginFunction();
             funcIt != pModDetail->getEndFunction(); ++funcIt)
        {
            InstAggregate inst;
            inst.instData.jitAddress = funcIt->second.getBaseAddr();

            gtString temp = funcIt->second.getFuncName();

            if (temp.length() > 0)
            {
                int length = temp.length() + 1;
                inst.instData.pJitFunctionName = new wchar_t[length];
                CopyWcharStr(inst.instData.pJitFunctionName, temp.asCharArray(), length);
            }

            temp = funcIt->second.getJncFileName();

            if (temp.length() > 0)
            {
                int length = temp.length() + 1;
                inst.instData.pJitDataFile = new wchar_t[length];
                CopyWcharStr(inst.instData.pJitDataFile, temp.asCharArray(), length);
            }

            AptAggregatedSampleMap::const_iterator ait;

            for (ait = funcIt->second.getBeginSample(); ait != funcIt->second.getEndSample(); ++ait)
            {
                inst.instData.address = ait->first.m_addr;

                if (inst.instData.address < pModDetail->m_base)
                {
                    inst.instData.address += pModDetail->m_base;
                }

                inst.instData.processId = ait->first.m_pid;
                inst.instData.threadId = ait->first.m_tid;

                CpuProfileSampleMap::const_iterator sampleIt = ait->second.getBeginSample();
                CpuProfileSampleMap::const_iterator sampleIt_end = ait->second.getEndSample();

                for (; sampleIt != sampleIt_end; ++sampleIt)
                {
                    SampleDatumKey sKey;
                    sKey.core = sampleIt->first.cpu;
                    sKey.event = sampleIt->first.event;
                    inst.sampleData.insert(SampleDataMap::value_type(sKey, sampleIt->second));
                }

                m_dataList.push_back(inst);
                inst.sampleData.clear();
            }
        }
    }

    for (it = m_dataList.begin(); it != m_dataList.end(); it++)
    {
        if ((rulePid.size() > 0) && (!rulePid.contains(it->instData.processId)))
        {
            continue;
        }

        if ((ruleTid.size() > 0) && (!ruleTid.contains(it->instData.threadId)))
        {
            continue;
        }

        InstAggregate oneAddress;
        oneAddress.instData.address = it->instData.address;
        oneAddress.instData.processId = it->instData.processId;
        oneAddress.instData.threadId = it->instData.threadId;
        oneAddress.instData.jitAddress = it->instData.jitAddress;
        oneAddress.instData.pJitFunctionName = it->instData.pJitFunctionName;
        oneAddress.instData.pJitDataFile = it->instData.pJitDataFile;
        SampleDataMap::iterator datum;

        for (datum = it->sampleData.begin(); datum != it->sampleData.end();
             datum++)
        {
            //is a valid core?
            if ((coreRule & (1ULL << datum->first.core)) != 0)
            {
                oneAddress.sampleData.insert(*datum);
            }
        }

        pInstructionData->push_back(oneAddress);
    }

    return S_OK;
}


HRESULT ModuleAggregator::FillModuleList(gtUInt64 coreRule, const QSet<gtUInt64>& rulePid, gtList<ModAggregate>* pSystemData)
{
    QMap<gtUInt32, ModAggregate>::const_iterator it, itEnd;

    for (it = m_aggProcessData.begin(), itEnd = m_aggProcessData.end(); it != itEnd; ++it)
    {
        if (!rulePid.empty() && (!rulePid.contains(it.key())))
        {
            continue;
        }

        ModAggregate oneProcess;
        oneProcess.modData.path = m_moduleName;
        oneProcess.modData.processId = it.key();
        oneProcess.modData.is64Bit = m_is64Bit;
        SampleDataMap::const_iterator datum, datEnd;

        for (datum = it.value().sampleData.begin(), datEnd = it.value().sampleData.end(); datum != datEnd; ++datum)
        {
            //is a valid core?
            if ((coreRule & (1ULL << datum->first.core)) != 0)
            {
                oneProcess.sampleData.insert(*datum);
            }
        }

        pSystemData->push_back(oneProcess);
    }

    return S_OK;
}


ProfileDataSet::ProfileDataSet()
{
    m_ready = false;
    m_pProfileReader = NULL;
}

ProfileDataSet::~ProfileDataSet()
{
    //free allocated memory
    gtList<SampleDatumKey*>::iterator keyIt = m_keyCache.begin();

    for (; keyIt != m_keyCache.end(); keyIt++)
    {
        delete [](*keyIt);
    }

    m_keyCache.clear();
    gtList<gtUInt64*>::iterator dataIt =  m_dataCache.begin();

    for (; dataIt != m_dataCache.end(); dataIt++)
    {
        delete [](*dataIt);
    }

    m_dataCache.clear();
    gtList<wchar_t*>::iterator strIt = m_stringCache.begin();

    for (; strIt != m_stringCache.end(); strIt++)
    {
        delete [](*strIt);
    }

    m_stringCache.clear();

    if (NULL != m_pProfileReader)
    {
        delete m_pProfileReader;
    }
}


HRESULT ProfileDataSet::WriteToFile(const wchar_t* pPathName,
                                    const wchar_t* pAggDir,
                                    gtUInt64 missedCount,
                                    gtUInt64 profileCoreMask,
                                    int cpuFamily,
                                    int cpuModel,
                                    const gtList<gtUInt64>& eventList,
                                    const gtList<gtUInt64>& periodList,
                                    int groupCount,
                                    QSet<gtUInt64>* pCssPids,
                                    CoreTopologyMap* pTopMap)
{
    (void)(missedCount); // unused
    (void)(groupCount); // unused
    //if any write fails, return E_ACCESSDENIED
    //if needed, create directory path
    QString tempFileName = QString::fromWCharArray(pPathName);
    QString tempDir = QString::fromWCharArray(pAggDir);

    //Replaces / with \ in directories
    tempFileName.replace('/', '\\');

    //recursively create directories, if needed
    int sections = tempFileName.count("\\");

    for (int sects = 1; sects < sections; sects ++)
    {
        QDir dir;
        dir.setPath(tempFileName.section("\\", 0, sects));

        if (!dir.exists())
        {
            dir.mkdir(dir.path());
        }
    }

    //Write profile file
    int coreCount = 0;

    for (int i = 63; i >= 0; i--)
    {
        if (((1ULL << i) & profileCoreMask) > 0)
        {
            coreCount = i + 1;
            break;
        }
    }

    CpuProfileWriter writer;
    CpuProfileInfo info;

    // Populate info
    info.m_numCpus = coreCount;
    info.m_numSamples = 0;
    info.m_numModules = m_aggModuleData.size();
    info.m_tbpVersion = TBPVER_BEFORE_RI;
    info.m_cpuFamily = cpuFamily;
    info.m_cpuModel = cpuModel;

    QString tmp = QDateTime::currentDateTime().toString();
    gtString timeStamp = tmp.toStdWString().c_str();
    info.setTimeStamp(timeStamp);

    //Add events to profile information
    gtList<gtUInt64>::const_iterator eventIt;
    gtList<gtUInt64>::const_iterator periodIt = periodList.begin();

    for (eventIt = eventList.begin(); eventIt != eventList.end(); ++eventIt, ++periodIt)
    {
        info.addEvent((EventMaskType)*eventIt, *periodIt);
    }

    PidProcessMap procMap;
    QMap<gtUInt32, ModAggregate>::iterator procIt;

    for (procIt = m_aggProcessData.begin(); procIt != m_aggProcessData.end(); ++procIt)
    {
        CpuProfileProcess temp_process;
        gtString temp = procIt->modData.path;
        temp_process.setPath(temp);
        temp_process.m_is32Bit = !procIt->modData.is64Bit;
        temp_process.m_hasCss = pCssPids->contains(procIt.key());

        SampleDataMap::iterator sampleIt;

        for (sampleIt = procIt->sampleData.begin(); sampleIt != procIt->sampleData.end(); ++sampleIt)
        {
            SampleKey sKey(sampleIt->first.core, (EventMaskType)(sampleIt->first.event));
            temp_process.insertSamples(sKey, (unsigned long)(sampleIt->second));
        }

        //Add samples to total
        info.m_numSamples += (unsigned int)temp_process.getTotal();
        procMap.insert(PidProcessMap::value_type(procIt.key(), temp_process));
    }

    NameModuleMap modMap;
    QMap<QString, ModuleAggregator>::iterator modIt;

    for (modIt = m_aggModuleData.begin(); modIt != m_aggModuleData.end(); ++modIt)
    {
        CpuProfileModule mod;
        gtString path = modIt.key().toStdWString().c_str();
        mod.setPath(path);
        mod.m_modType = modIt->m_type;
        mod.m_base = modIt->m_address;
        mod.m_size = (gtUInt32)(modIt->m_size);
        mod.m_is32Bit = !modIt->m_is64Bit;

        gtList<InstAggregate>::iterator instIt;

        for (instIt = modIt->m_dataList.begin(); instIt != modIt->m_dataList.end(); ++instIt)
        {
            gtVAddr address = 0;
            gtVAddr funcAddr = 0;
            gtUInt32 funcSize = 0;
            gtString funcName;
            gtString jncName;
            gtString javaSrcName;
            QString qJncName;

            switch (modIt->m_type)
            {
                case CpuProfileModule::JAVAMODULE:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                    jncName = instIt->instData.pJitDataFile;
                    qJncName = QString::fromWCharArray(instIt->instData.pJitDataFile);

                    if (!m_JncMap.contains(qJncName))
                    {
                        QString realPath = tempDir + "/" + qJncName;
                        m_JncMap[qJncName].Open(realPath.toStdWString().c_str());
                    }

                    javaSrcName = m_JncMap[qJncName].GetSourceName().toStdWString().c_str();
                    funcName = instIt->instData.pJitFunctionName;
                    funcAddr = instIt->instData.jitAddress;
                    address = instIt->instData.address;
#else
                    //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                    break;

                case CpuProfileModule::MANAGEDPE:
                case CpuProfileModule::OCLMODULE:
                    funcName = instIt->instData.pJitFunctionName;
                    jncName =  instIt->instData.pJitDataFile;
                    funcAddr = instIt->instData.jitAddress;
                    address = instIt->instData.address;
                    javaSrcName = L"UnknownJITSource";
                    break;

                default:
                    //use offset instead of address
                    funcAddr = 0;
                    address = instIt->instData.address - modIt->m_address;
                    break;
            }

            SampleDataMap::iterator sampleIt;

            for (sampleIt = instIt->sampleData.begin(); sampleIt != instIt->sampleData.end(); ++sampleIt)
            {
                SampleInfo sampInfo(address, instIt->instData.processId,
                                    instIt->instData.threadId,
                                    sampleIt->first.core,
                                    (EventMaskType)(sampleIt->first.event));

                mod.recordSample(sampInfo, (gtUInt32)(sampleIt->second), funcAddr, funcSize, funcName, jncName, javaSrcName);
            }
        }

        // Insert
        modMap.insert(NameModuleMap::value_type(modIt.key().toStdWString().c_str(), mod));
    }

    gtString path = pPathName;

    // Get the RI file path
    osFilePath riFilePath(path);
    riFilePath.setFileExtension(L"ri");

    // Read the data from the RI file
    RunInfo runInfo;
    HRESULT hr = fnReadRIFile(riFilePath.asString().asCharArray(), &runInfo);

    if (hr == S_OK)
    {
        // We have the RI data, set current TBP version
        info.m_tbpVersion = TBPVER_DEFAULT;

        // populate RI info
        info.m_targetPath = runInfo.m_targetPath;
        info.m_wrkDirectory = runInfo.m_wrkDirectory;
        info.m_cmdArguments = runInfo.m_cmdArguments;
        info.m_envVariables = runInfo.m_envVariables;
        info.m_profType = runInfo.m_profType;
        info.m_profDirectory = runInfo.m_profDirectory;
        info.m_profStartTime = runInfo.m_profStartTime;
        info.m_profEndTime = runInfo.m_profEndTime;
        info.m_isCSSEnabled = runInfo.m_isCSSEnabled;
        info.m_cssUnwindDepth = runInfo.m_cssUnwindDepth;
        info.m_cssScope = runInfo.m_cssScope;
        info.m_isCssSupportFpo = runInfo.m_isCssSupportFpo;
        info.m_osName = runInfo.m_osName;
        info.m_profScope = runInfo.m_profScope;
    }

    if (!writer.Write(path, &info, &procMap, &modMap, pTopMap))
    {
        return E_ACCESSDENIED;
    }

    //Copy call-stack files
    QDir inDir(tempDir);
    gtList<gtString> filesToCopyList;
    filesToCopyList.push_back(L"*.css");
    filesToCopyList.push_back(L"*.jnc");

    // Setup directory
    osDirectory osDir;
    osDir.setDirectoryFullPathFromString(tempDir.toStdWString().c_str());
    osDir.copyFilesToDirectory(tempFileName.section("\\", 0, -2).toStdWString().c_str(), filesToCopyList);

    return S_OK;
}

//
// ProfileDataSet::readFromFile() reads aggregated profile data from
// a .TBP/.EBP file. It populates the m_aggProcessData and
// m_aggModuleData maps for the dataset.
//
HRESULT ProfileDataSet::ReadFromFile(CpuProfileReader* pReader)
{
    m_pProfileReader = pReader;
    PidProcessMap* pAppMap = m_pProfileReader->getProcessMap();

    if (NULL == pAppMap)
    {
        return E_UNEXPECTED;
    }

    // Read in process data
    PidProcessMap::const_iterator pIt;

    for (pIt = pAppMap->begin(); pIt != pAppMap->end(); pIt++)
    {
        const CpuProfileProcess* pProcess = &(pIt->second);
        ModAggregate process;

        process.modData.is64Bit = !(pProcess->m_is32Bit);
        gtSize_t length = static_cast<gtSize_t>(pProcess->getPath().length()) + 1;
        process.modData.path = new wchar_t[length];
        CopyWcharStr(process.modData.path, pProcess->getPath().asCharArray(), length);
        process.modData.processId = pIt->first;

        for (CpuProfileSampleMap::const_iterator s_it = pProcess->getBeginSample();
             s_it != pProcess->getEndSample(); s_it++)
        {
            SampleDatumKey sKey;
            sKey.core = s_it->first.cpu;
            sKey.event = s_it->first.event;
            process.sampleData.insert(SampleDataMap::value_type(sKey, s_it->second));
        }

        m_aggProcessData.insert(pIt->first, process);
    }

    // Read in module data
    NameModuleMap* pModuleMap = m_pProfileReader->getModuleMap();

    if (NULL == pModuleMap)
    {
        return E_UNEXPECTED;
    }

    for (NameModuleMap::const_iterator mit = pModuleMap->begin(); mit != pModuleMap->end(); ++mit)
    {
        const CpuProfileModule* pModule = &(mit->second);
        QString modName = QString::fromWCharArray(pModule->getPath().asCharArray());

        m_aggModuleData[modName].m_address = pModule->getBaseAddr();
        m_aggModuleData[modName].m_is64Bit = !(pModule->m_is32Bit);
        CopyWcharStr(m_aggModuleData[modName].m_moduleName, modName.toStdWString().c_str(), (OS_MAX_PATH + 1));
        m_aggModuleData[modName].m_type = (ModuleType) pModule->m_modType;
        m_aggModuleData[modName].m_size = pModule->m_size;

        PidAggregatedSampleMap::const_iterator ait;

        for (ait = pModule->getBeginSample(); ait != pModule->getEndSample(); ++ait)
        {
            ModAggregate process;
            process.modData.path = NULL;
            process.modData.is64Bit = !(pModule->m_is32Bit);
            process.modData.processId = ait->first;
            CpuProfileSampleMap::const_iterator s_it;

            for (s_it = ait->second.getBeginSample(); s_it != ait->second.getEndSample(); ++s_it)
            {
                SampleDatumKey sKey;
                sKey.core = s_it->first.cpu;
                sKey.event = s_it->first.event;
                process.sampleData.insert(SampleDataMap::value_type(sKey, s_it->second));
            }

            m_aggModuleData[modName].m_aggProcessData.insert(ait->first, process);
        }
    }

    //note that we will read in instruction data as requested
    m_ready = true;

    return S_OK;
}

//
// ProfileDataSet::addSample() adds a raw sample to the m_aggProcessData
// and m_aggModuleData maps for the data set.
//
void ProfileDataSet::AddSample(const RawAggregate* pData, bool checkInterval)
{
    //First, if needed, verify the sample is in interesting intervals
    if ((checkInterval) && (!IsInInterval(&(pData->rawData.timeMark))))
    {
        return;
    }

    QString modKey = QString::fromWCharArray(pData->rawData.path);

    //If this is a new module, init
    if (!m_aggModuleData.contains(modKey))
    {
        m_aggModuleData[modKey].Initialize(pData);
    }

    m_aggModuleData[modKey].AddSample(pData);

    gtUInt32 pid = pData->rawData.processId;

    if (!m_aggProcessData.contains(pid))
    {
        m_aggProcessData[pid].modData.processId = pid;

        wchar_t processName[OS_MAX_PATH + 1];
        processName[0] = L'\0';

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        if (S_OK != fnFindProcessName(pid, processName, OS_MAX_PATH))
        {
            swprintf_s(processName, OS_MAX_PATH, L"unknown module pid (%d)", pid);
        }

#else
        //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        size_t length = wcslen(processName) + 1;
        m_aggProcessData[pid].modData.path = new wchar_t[length];
        CopyWcharStr(m_aggProcessData[pid].modData.path, processName, length);

        m_aggProcessData[pid].modData.is64Bit = IsModule64Bit(m_aggProcessData[pid].modData.path,
                                                              pData->rawData.address, pData->rawData.processId);
    }

    CpuProfileDataAccess::Aggregate(&(m_aggProcessData[pid].sampleData), &(pData->sampleData));
}


void ProfileDataSet::AggregrateMaps()
{
    QMap<QString, ModuleAggregator>::iterator it;

    for (it = m_aggModuleData.begin(); it != m_aggModuleData.end(); it++)
    {
        it->AggregrateHashes();
    }

    m_ready = true;
}


void ProfileDataSet::Clear()
{
    m_aggModuleData.clear();
    QMap<gtUInt32, ModAggregate>::iterator pIt = m_aggProcessData.begin(), pItEnd = m_aggProcessData.end();

    for (; pIt != pItEnd; ++pIt)
    {
        if (NULL != pIt->modData.path)
        {
            delete [] pIt->modData.path;
        }
    }

    m_aggProcessData.clear();
}

size_t ProfileDataSet::GetProcessDataCount(gtUInt64 coreRule, const QSet<gtUInt64>& rulePid)
{
    if ((coreRule != m_cacheCoreRule) || (rulePid != m_cacheRulePid))
    {
        m_moduleCache.clear();
        m_processCache.clear();
        m_instructionCache.clear();
    }
    else if (0 != m_processCache.size())
    {
        //If we're asking about the current data in the cache
        return m_processCache.size();
    }

    m_cacheCoreRule = coreRule;
    m_cacheRulePid = rulePid;

    QMap<gtUInt32, ModAggregate>::iterator it;

    for (it = m_aggProcessData.begin(); it != m_aggProcessData.end(); it++)
    {
        if ((rulePid.size() > 0) && (!rulePid.contains(it.key())))
        {
            continue;
        }

        ModAggregate oneProcess;
        oneProcess.modData.path = it.value().modData.path;
        oneProcess.modData.processId = it.key();
        oneProcess.modData.is64Bit = it.value().modData.is64Bit;
        SampleDataMap::iterator datum;

        for (datum = it.value().sampleData.begin();
             datum != it.value().sampleData.end();
             datum++)
        {
            //is a valid core?
            if ((coreRule & (1ULL << datum->first.core)) != 0)
            {
                oneProcess.sampleData.insert(*datum);
            }
        }

        m_processCache.push_back(oneProcess);
    }

    return m_processCache.size();
}


HRESULT ProfileDataSet::FillProcessData(gtUInt64 coreRule, const QSet<gtUInt64>& rulePid, unsigned int maxSize, ModuleDataType* pProcessData)
{
    //If the data isn't in the cache, put it there
    if ((coreRule != m_cacheCoreRule) || (rulePid != m_cacheRulePid))
    {
        GetProcessDataCount(coreRule, rulePid);
    }

    if (maxSize < m_processCache.size())
    {
        return E_OUTOFMEMORY;
    }

    gtList<ModAggregate>::iterator it = m_processCache.begin();

    for (int i = 0; it != m_processCache.end(); it++, i++)
    {
        pProcessData[i] = it->modData;
        pProcessData[i].data.count = (unsigned int)it->sampleData.size();
        size_t count = pProcessData[i].data.count;

        if (count > 0)
        {
            pProcessData[i].data.keyArray = new SampleDatumKey[count];

            if (NULL == pProcessData[i].data.keyArray)
            {
                pProcessData[i].data.count = 0;
                return E_OUTOFMEMORY;
            }

            pProcessData[i].data.dataArray = new gtUInt64[count];

            if (NULL == pProcessData[i].data.dataArray)
            {
                pProcessData[i].data.count = 0;
                delete [] pProcessData[i].data.keyArray;
                pProcessData[i].data.keyArray = NULL;
                return E_OUTOFMEMORY;
            }

            //save new for later cleanup
            m_keyCache.push_back(pProcessData[i].data.keyArray);
            m_dataCache.push_back(pProcessData[i].data.dataArray);

            int j = 0;
            SampleDataMap::iterator datum;

            for (datum = it->sampleData.begin(); datum != it->sampleData.end();
                 datum++, j++)
            {
                pProcessData[i].data.keyArray[j] = datum->first;
                pProcessData[i].data.dataArray[j] = datum->second;
            }
        }
    }

    return S_OK;
}


size_t ProfileDataSet::GetModuleDataCount(gtUInt64 coreRule, const QSet<gtUInt64>& rulePid)
{
    if (coreRule != m_cacheCoreRule || rulePid != m_cacheRulePid)
    {
        m_processCache.clear();
        m_moduleCache.clear();
        m_instructionCache.clear();
    }
    else if (0 != m_moduleCache.size())
    {
        //If we're asking about the current data in the cache
        return m_moduleCache.size();
    }

    m_cacheCoreRule = coreRule;
    m_cacheRulePid = rulePid;

    QMap<QString, ModuleAggregator>::iterator it, itEnd;

    for (it = m_aggModuleData.begin(), itEnd = m_aggModuleData.end(); it != itEnd; ++it)
    {
        it.value().FillModuleList(coreRule, rulePid, &m_moduleCache);
    }

    return m_moduleCache.size();
}


HRESULT ProfileDataSet::FillModuleData(gtUInt64 coreRule, const QSet<gtUInt64>& rulePid, unsigned int maxSize, ModuleDataType* pSystemData)
{
    //If the data isn't in the cache, put it there
    if ((coreRule != m_cacheCoreRule) || (rulePid != m_cacheRulePid))
    {
        GetModuleDataCount(coreRule, rulePid);
    }

    if (maxSize < m_moduleCache.size())
    {
        return E_OUTOFMEMORY;
    }

    gtList<ModAggregate>::iterator it = m_moduleCache.begin();

    for (int i = 0; it != m_moduleCache.end(); it++, i++)
    {
        pSystemData[i] = it->modData;
        pSystemData[i].data.count = (unsigned int)it->sampleData.size();
        size_t count = pSystemData[i].data.count;

        if (count > 0)
        {
            pSystemData[i].data.keyArray = new SampleDatumKey[count];

            if (NULL == pSystemData[i].data.keyArray)
            {
                pSystemData[i].data.count = 0;
                return E_OUTOFMEMORY;
            }

            pSystemData[i].data.dataArray = new gtUInt64[count];

            if (NULL == pSystemData[i].data.dataArray)
            {
                pSystemData[i].data.count = 0;
                delete [] pSystemData[i].data.keyArray;
                pSystemData[i].data.keyArray = NULL;
                return E_OUTOFMEMORY;
            }

            //save new for later cleanup
            m_keyCache.push_back(pSystemData[i].data.keyArray);
            m_dataCache.push_back(pSystemData[i].data.dataArray);

            int j = 0;
            SampleDataMap::iterator datum;

            for (datum = it->sampleData.begin(); datum != it->sampleData.end();
                 datum++, j++)
            {
                pSystemData[i].data.keyArray[j] = datum->first;
                pSystemData[i].data.dataArray[j] = datum->second;
            }
        }
    }

    return S_OK;
}


HRESULT ProfileDataSet::GetInstructionDataCount(gtUInt64 coreRule,
                                                const QSet<gtUInt64>& rulePid,
                                                const QSet<gtUInt64>& ruleTid,
                                                const wchar_t* modulePath,
                                                unsigned int* pCount)
{
    QString qModulePath = QString::fromWCharArray(modulePath);

    if (coreRule != m_cacheCoreRule || rulePid != m_cacheRulePid || ruleTid != m_cacheRuleTid || m_cacheModName != qModulePath)
    {
        m_processCache.clear();
        m_moduleCache.clear();
        m_instructionCache.clear();
    }
    else if (0 != m_instructionCache.size())
    {
        //If we're asking about the current data in the cache
        if (NULL != pCount)
        {
            *pCount = (unsigned int)m_instructionCache.size();
        }

        return S_OK;
    }

    m_cacheCoreRule = coreRule;
    m_cacheRulePid = rulePid;
    m_cacheRuleTid = ruleTid;
    m_cacheModName = qModulePath;

    if (!m_aggModuleData.contains(qModulePath))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = m_aggModuleData[qModulePath].FillInstructionList(coreRule, rulePid, ruleTid, &m_instructionCache, m_pProfileReader);

    if (NULL != pCount)
    {
        *pCount = (unsigned int)m_instructionCache.size();
    }

    return hr;
}


HRESULT ProfileDataSet::FillInstructionData(gtUInt64 coreRule,
                                            const QSet<gtUInt64>& rulePid,
                                            const QSet<gtUInt64>& ruleTid,
                                            const wchar_t* modulePath,
                                            unsigned int maxSize,
                                            InstructionDataType* pInstructionData,
                                            gtUInt64* pModuleLoadAddress,
                                            ModuleType* pModuleType)
{
    HRESULT hr = S_OK;
    QString qModulePath = QString::fromWCharArray(modulePath);

    //If the data isn't in the cache, put it there
    if ((coreRule != m_cacheCoreRule) || (rulePid != m_cacheRulePid) ||
        (ruleTid != m_cacheRuleTid) ||
        (m_cacheModName != qModulePath))
    {
        hr = GetInstructionDataCount(coreRule, rulePid, ruleTid, modulePath, NULL);
    }

    if (maxSize < m_moduleCache.size())
    {
        return E_OUTOFMEMORY;
    }

    if (NULL != pModuleLoadAddress)
    {
        *pModuleLoadAddress = m_aggModuleData[qModulePath].m_address;
    }

    if (NULL != pModuleType)
    {
        *pModuleType = m_aggModuleData[qModulePath].m_type;
    }

    gtList<InstAggregate>::iterator it = m_instructionCache.begin(), itEnd = m_instructionCache.end();

    for (int i = 0; it != itEnd; ++it, ++i)
    {
        pInstructionData[i] = it->instData;
        pInstructionData[i].data.count = (unsigned int)it->sampleData.size();
        size_t count = pInstructionData[i].data.count;

        if (count > 0)
        {
            pInstructionData[i].data.keyArray = new SampleDatumKey[count];

            if (NULL == pInstructionData[i].data.keyArray)
            {
                pInstructionData[i].data.count = 0;
                return E_OUTOFMEMORY;
            }

            pInstructionData[i].data.dataArray = new gtUInt64[count];

            if (NULL == pInstructionData[i].data.dataArray)
            {
                pInstructionData[i].data.count = 0;
                delete [] pInstructionData[i].data.keyArray;
                pInstructionData[i].data.keyArray = NULL;
                return E_OUTOFMEMORY;
            }

            //save new for later cleanup
            m_keyCache.push_back(pInstructionData[i].data.keyArray);
            m_dataCache.push_back(pInstructionData[i].data.dataArray);

            int j = 0;

            for (SampleDataMap::const_iterator datum = it->sampleData.begin(), datEnd = it->sampleData.end();
                 datum != datEnd; ++datum, ++j)
            {
                pInstructionData[i].data.keyArray[j] = datum->first;
                pInstructionData[i].data.dataArray[j] = datum->second;
            }
        }
    }

    return hr;
}


HRESULT ProfileDataSet::GetJitData(gtUInt64 offset, const wchar_t* pJncDataFile,
                                   const wchar_t* pJncDir, int jitType,
                                   JITDataType* pJitData)
{
    if ((NULL == pJncDataFile) || (NULL == pJitData) || (NULL == pJncDir))
    {
        return E_INVALIDARG;
    }

    QString key = QString::fromWCharArray(pJncDataFile);

    if (CpuProfileModule::JAVAMODULE == jitType)
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        if (!m_JncMap.contains(key))
        {
            wchar_t fullPath[MAX_PATH];
            wcscpy_s(fullPath, MAX_PATH, pJncDir);
            wcscat_s(fullPath, MAX_PATH, pJncDataFile);

            if (!m_JncMap[key].Open(fullPath))
            {
                m_JncMap.remove(key);
                return E_ACCESSDENIED;
            }
        }

        pJitData->pNativeCodeBytes = m_JncMap[key].GetCodeBytesOfTextSection(NULL) + offset;

        int length = m_JncMap[key].GetSourceName().size();

        if (m_JncMap[key].GetSourceName().size() > 0)
        {
            pJitData->sourceFileName = new wchar_t [length + 1];

            if (NULL == pJitData->sourceFileName)
            {
                return E_OUTOFMEMORY;
            }

            //save new for later cleaning
            m_stringCache.push_back(pJitData->sourceFileName);

            wcscpy_s(pJitData->sourceFileName, length + 1, m_JncMap[key].GetSourceName().toStdWString().c_str());
        }
        else
        {
            pJitData->sourceFileName = NULL;
        }

        //Find the first entry after the key offset
        OffsetLinenumMap offsets = m_JncMap[key].GetOffsetLines();
        OffsetLinenumMap::iterator it = offsets.upperBound((unsigned int)offset);

        if (it != offsets.begin())
        {
            it--;
        }

        pJitData->lineNumber = *it;
    }
    else if (CpuProfileModule::MANAGEDPE == jitType)
    {
        if (!m_ClrMap.contains(key))
        {
            if (!m_ClrMap[key].Open(pJncDataFile))
            {
                m_ClrMap.remove(key);
                return E_ACCESSDENIED;
            }
        }

        unsigned int offsetToNative;
        pJitData->pNativeCodeBytes = m_ClrMap[key].GetCodeBytesOfTextSection(&offsetToNative, NULL) + offset;
#else
        (void)(offset); // unused
        //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    }
    else
    {
        return E_INVALIDARG;
    }

    return S_OK;
}


bool ProfileDataSet::IsReady() const
{
    return m_ready;
}


//static
bool ProfileDataSet::IsModule64Bit(wchar_t* pModuleName, gtUInt64 address, gtUInt32 pid)
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    //if it starts with "unknown", check kernel
    if (0 == wcsncmp(pModuleName, L"unknown", 7))
    {
        // it's in kernel space on 64-bit OS, it should be 64-bit module.
        if (address > KERNEL64_SPACE_START)
        {
            return true;
        }

        //otherwise it's in the system space of a 32-bit OS or unknown user-mode
        return false;
    }

    bool is64bit = false;
    ExecutableFile* pExecutable = ExecutableFile::Open(pModuleName);

    if (NULL != pExecutable)
    {
        is64bit = pExecutable->Is64Bit();
        delete pExecutable;
        pExecutable = NULL;
    }
    else
    {
        if (fnIsJITProcess32Bit(pid))
        {
            is64bit = false;
        }
        else
        {
            is64bit = (address > 0xffffffff);
        }
    }

    return is64bit;
#else
    // unused
    (void)(pModuleName);
    (void)(address);
    (void)(pid);
    //TODO: [Suravee] : This should be done in oswrapper
    return false;
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}


bool ProfileDataSet::IsInInterval(const CPA_TIME* pTime) const
{
    gtList<ProfileDataSetInterval>::const_iterator it, itEnd = m_intervals.end();

    for (it = m_intervals.begin(); it != itEnd; ++it)
    {

        //-1 means 1st time is earlier than the 2nd
        bool afterStart = (-1 != Compare_CPA_TIME(pTime, &(it->start)));
        bool beforeEnd = (-1 == Compare_CPA_TIME(pTime, &(it->end)));

        if (afterStart && beforeEnd)
        {
            break;
        }
    }

    //return if an appropriate interval was found
    return (itEnd != it);
}


int ProfileDataSet::Compare_CPA_TIME(const CPA_TIME* pTime1, const CPA_TIME* pTime2) const
{
    if (NULL == pTime1 || NULL == pTime2)
    {
        return -2; //TODO: [Suravee] I don't like this
    }

    if (pTime1->second < pTime2->second)
    {
        return 1;
    }
    else if (pTime1->second > pTime2->second)
    {
        return -1;
    }
    else // For (pTime1->second == pTime2->second)
    {
        if (pTime1->microsec < pTime2->microsec)
        {
            return 1;
        }
        else if (pTime1->microsec == pTime2->microsec)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
}


CpuProfileDataAccess::CpuProfileDataAccess()
{
    m_ready = false;
    m_aggregating = false;
    m_pRawDataSet = NULL;
    m_profileCoreMask = static_cast<gtUInt64>(-1);
    m_coreCount = 0;
    m_rawProgress = 0;
    SetDefaultJitDir();
    m_aggOutDir[0] = L'\0';
    m_checkInterval = false;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    m_pPrdReader = NULL;
#else
    //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}


CpuProfileDataAccess::~CpuProfileDataAccess()
{
    m_eventsFile.Close();
#if SUPPORT_CSS
    ModAnalysisMap::iterator iter = NULL;

    for (iter = m_CssMap.begin(); iter != m_CssMap.end(); iter++)
    {
        CModAnalyzer* pAnalyzer = iter.value();
        delete pAnalyzer;
    }

    m_CssMap.clear();
#endif

    CleanupProfileReader();

    //free allocated memory
    gtList<SampleDatumKey*>::iterator keyIt = m_keyCache.begin();

    for (; keyIt != m_keyCache.end(); keyIt++)
    {
        delete [](*keyIt);
    }

    m_keyCache.clear();
    gtList<gtUInt64*>::iterator dataIt =  m_dataCache.begin();

    for (; dataIt != m_dataCache.end(); dataIt++)
    {
        delete [](*dataIt);
    }

    m_dataCache.clear();
    gtList<wchar_t*>::iterator strIt = m_stringCache.begin();

    for (; strIt != m_stringCache.end(); strIt++)
    {
        delete [](*strIt);
    }

    m_stringCache.clear();
}


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
HRESULT CpuProfileDataAccess::OpenPrdFile(const wchar_t* pPrdFile)
{
    HRESULT hr;
    struct _stati64 fileStat;
    _wstati64(pPrdFile, &fileStat);
    m_approxRecords = ((fileStat.st_size - sizeof(sFileHeaderV1)) / 40) + 1;

    size_t prdLen = wcslen(pPrdFile) - 4;

    if (0 != wcscmp(L".prd", &(pPrdFile[prdLen])))
    {
        return E_ACCESSDENIED;
    }

    hr = SetupProfileReader(pPrdFile);

    if (hr != S_OK)
    {
        return hr;
    }

    //get profile core mask
    m_cpuFamily = m_pPrdReader->GetCpuFamily();
    m_cpuModel = m_pPrdReader->GetCpuModel();
    m_profileCoreMask = 0;
    m_coreCount = m_pPrdReader->GetCoreCount();

    for (unsigned int i = 0; i < m_coreCount; i++)
    {
        m_profileCoreMask |= 1ULL << i;
    }

    //initialize
    m_ruleCoreMask = m_profileCoreMask;

    //open ti file too
    QString tiFile;
    tiFile = QString::fromWCharArray(pPrdFile);
    tiFile.replace(PRD_FILE_EXT, TI_FILE_EXT);

    hr = fnReadModuleInfoFile(tiFile.toStdWString().c_str());

    if (S_OK != hr)
    {
        delete m_pPrdReader;
        m_pPrdReader = NULL;
        return hr;
    }

    AddDataSet(&m_entire, 1, NULL);

    //load jit info
    fnReadCLRJitInformation(m_jitDir);
    fnReadOldJitInformation(m_jitDir);

    unsigned int types = m_pPrdReader->GetProfileType();

    if ((((types & PROF_IBS) > 0) || ((types & PROF_EBP) > 0)) &&
        (S_OK != OpenEventsFile(m_cpuFamily, m_cpuModel)))
    {
        delete m_pPrdReader;
        m_pPrdReader = NULL;
        return E_UNEXPECTED;
    }

    m_groupCount = 0;

    if ((types & PROF_TBP) > 0)
    {
        m_events.push_back(GetTimerEvent());
        m_groupCount = 1;

        m_labels.push_back(QString("Timer"));
        // GetTimerResultion() returns the timer interval in microseconds.
        m_sampIntvls.push_back(m_pPrdReader->GetTimerResultion());
    }

    if ((types & PROF_IBS) > 0)
    {
        unsigned int fetch;
        unsigned int ops;
        m_pPrdReader->GetIBSConfig(&fetch, &ops);

        if (fetch != 0) { AddIbsFetchEvents() ; }

        if (ops != 0) { AddIbsOpEvents() ; }

        m_groupCount = 1;
    }

    if ((types & PROF_EBP) > 0)
    {
        m_groupCount = m_pPrdReader->GetEventGroupCount();
        unsigned int cfgCnt = m_pPrdReader->GetEventCount();
        EventCfgInfo* pEvtCfg = (EventCfgInfo*) malloc(sizeof(EventCfgInfo) * cfgCnt);

        if (pEvtCfg)
        {
            if (S_OK == m_pPrdReader->GetEventInfo(pEvtCfg, cfgCnt))
            {
                PERF_CTL ctl;

                for (unsigned int i = 0; i < cfgCnt; i++)
                {
                    ctl.perf_ctl = pEvtCfg->ctl.perf_ctl;
                    unsigned int eventSelect = GetEvent12BitSelect(ctl);

                    EventMaskType encodedEvent = EncodeEvent((gtUInt16)eventSelect, ctl.ucUnitMask, ctl.bitOsEvents, ctl.bitUsrEvents);

                    m_events.push_back(encodedEvent);

                    CpuEvent oneEvent;
                    m_eventsFile.FindEventByValue(eventSelect, oneEvent);
                    m_labels.push_back(oneEvent.name);

                    m_sampIntvls.push_back(pEvtCfg->ctr);
                }
            }

            free(pEvtCfg);
            pEvtCfg = NULL;
        }
    }

    // If the number of event groups is zero, return an error
    if (0 == m_groupCount)
    {
        return E_ACCESSDENIED;
    }

    for (unsigned int j = 0; j < m_coreCount; j++)
    {
        CoreTopology topTemp;

        if (m_pPrdReader->GetTopology(j, &(topTemp.processor), &(topTemp.numaNode)))
        {
            m_topMap.insert(CoreTopologyMap::value_type(j, topTemp));
        }
    }

    return S_OK;
}
#else
//TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS


bool CpuProfileDataAccess::IsProfileDataAvailable() const
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    return (NULL != m_pPrdReader);
#else
    //TODO: [Suravee]
    return false;
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}


HRESULT CpuProfileDataAccess::ImportDataSet(const wchar_t* pProfileFile)
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    //use the profile reader to get data
    QString theProfile = QString::fromWCharArray(pProfileFile);
    theProfile.replace('/', '\\');

    //assume jnc files are in the same directory as the aggregated data file
    wcscpy_s(m_aggOutDir, OS_MAX_PATH,
             theProfile.section('\\', 0, -2, QString::SectionIncludeTrailingSep).toStdWString().c_str());

    CpuProfileReader* pReader = new CpuProfileReader;

    if (NULL == pReader)
    {
        return E_OUTOFMEMORY;
    }

    if (!pReader->open(theProfile.toStdWString().c_str()))
    {
        return E_ACCESSDENIED;
    }

    CpuProfileInfo* pSessionInfo = pReader->getProfileInfo();

    // Get the correct cpu family info
    m_cpuFamily = pSessionInfo->m_cpuFamily;
    m_cpuModel = pSessionInfo->m_cpuModel;

    if (S_OK != OpenEventsFile(m_cpuFamily, m_cpuModel))
    {
        CleanupProfileReader();
        return E_UNEXPECTED;
    }

    // Set the available data core mask (BUG166797)
    // This code is adapted from OpenPrdFile()
    m_profileCoreMask = 0;
    m_coreCount = pSessionInfo->m_numCpus;

    for (unsigned int i = 0; i < m_coreCount; i++)
    {
        m_profileCoreMask |= 1ULL << i;
    }

    // Initialize rule core mask
    m_ruleCoreMask = m_profileCoreMask;

    // Fill out the event list
    for (EventEncodeVec::const_iterator evIt = pSessionInfo->m_eventVec.begin(), evEnd = pSessionInfo->m_eventVec.end();
         evIt != evEnd; ++evIt)
    {
        m_events.push_back((*evIt).eventMask);
        gtUInt16 eventSelect;
        DecodeEvent((*evIt).eventMask, &eventSelect, NULL, NULL, NULL);

        if (IsTimerEvent(eventSelect))
        {
            // Handle the timer event explicitly (BUG184868)
            m_labels.push_back(QString("Timer"));
        }
        else
        {
            CpuEvent oneEvent;
            m_eventsFile.FindEventByValue(eventSelect, oneEvent);
            m_labels.push_back(oneEvent.name);
        }

        m_sampIntvls.push_back((*evIt).eventCount);
    }

    QString key; // Use the blank as the default dataset
    HRESULT hr = m_dataSets[key].ReadFromFile(pReader);

    if (S_OK == hr)
    {
        SetJitDir(theProfile.section('\\', 0, -2).toStdWString().c_str());
        m_ready = true;
    }

    return hr;
#else
    //TODO: [Suravee]
    (void)(pProfileFile); // unused
    return E_NOTIMPL;
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}


bool CpuProfileDataAccess::IsDataSetExists(const wchar_t* pDataSetName) const
{
    //NULL is the default data set
    QString key;

    if (NULL != pDataSetName)
    {
        key = QString::fromWCharArray(pDataSetName);
    }

    return m_dataSets.contains(key);
}


HRESULT CpuProfileDataAccess::SetJitDir(const wchar_t* pJitDirName)
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    //assume that the directory exists and is readable
    if (NULL != pJitDirName)
    {
        wcscpy_s(m_jitDir, OS_MAX_PATH, pJitDirName);
    }
    else
    {
        SetDefaultJitDir();
    }

    return fnReadOldJitInformation(m_jitDir);
#else
    //TODO: [Suravee]
    (void)(pJitDirName); // unused
    return E_NOTIMPL;
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}


HRESULT CpuProfileDataAccess::GetStartTimeMark(CPA_TIME* pTimeMark) const
{
    *pTimeMark = m_entire.start;
    return S_OK;
}


HRESULT CpuProfileDataAccess::GetEndTimeMark(CPA_TIME* pTimeMark) const
{
    *pTimeMark = m_entire.end;
    return S_OK;
}


HRESULT CpuProfileDataAccess::AddDataSet(const ProfileDataSetInterval* pIntervals, unsigned int count, const wchar_t* pDataSetName)
{
    if (!IsProfileDataAvailable())
    {
        return E_FAIL;
    }

    QString key;

    if (NULL != pDataSetName)
    {
        key = QString::fromWCharArray(pDataSetName);
    }

    for (unsigned int i = 0; i < count; i++)
    {
        m_dataSets[key].m_intervals.push_back(pIntervals[i]);
    }

    return S_OK;
}


HRESULT CpuProfileDataAccess::GetDataSets(gtList<wchar_t*>* pDataSets)
{
    QMap<QString, ProfileDataSet>::iterator it;

    for (it = m_dataSets.begin(); it != m_dataSets.end(); it++)
    {
        wchar_t* pName = NULL;
        int size = it.key().size();

        if (size > 0)
        {
            pName = new wchar_t[size + 1];

            if (NULL == pName)
            {
                return E_OUTOFMEMORY;
            }

            //save new for later cleaning
            m_stringCache.push_back(pName);

            CopyWcharStr(pName, it.key().toStdWString().c_str(), size + 1);
        }

        pDataSets->push_back(pName);
    }

    return S_OK;
}


HRESULT CpuProfileDataAccess::RemoveDataSet(const wchar_t* pDataSetName)
{
    //NULL is the default data set
    QString key;

    if (NULL != pDataSetName)
    {
        key = QString::fromWCharArray(pDataSetName);
    }

    if ((NULL == pDataSetName) || (!m_dataSets.contains(key)))
    {
        return E_INVALIDARG;
    }

    m_dataSets.remove(key);
    return S_OK;
}


HRESULT CpuProfileDataAccess::AggregateDataSets(const bool* pCancel, const wchar_t* pJitDataDirectory, float* pPercentComplete)
{
    if (!IsProfileDataAvailable())
    {
        return S_FALSE;
    }

    m_aggregating = true;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (NULL != pJitDataDirectory)
    {
        wcscpy_s(m_aggOutDir, OS_MAX_PATH, pJitDataDirectory);
        wcscat_s(m_aggOutDir, OS_MAX_PATH, L"\\");
    }
    else
    {
        //else, need temp dir!
        GetTempPath(OS_MAX_PATH, m_aggOutDir);
    }

#else
    //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    //clean output directory
    QString dirPath = QString::fromWCharArray(m_aggOutDir);
    QDir outDir(dirPath);
    QStringList f_list = outDir.entryList(QStringList("*.css"));

    for (int i = 0; i < f_list.size(); i++)
    {
        QString filePath = dirPath + "\\" + f_list.at(i);
        QFile::remove(filePath);
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    fnReadJitInformation(m_aggOutDir);
#else
    //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    //initalize member variables
    m_cssPids.clear();

    QMap<QString, ProfileDataSet>::iterator it;
    RawAggregate data;
    HRESULT hr = GetFirstRawAggregateRecord(NULL, &data);

    if (S_OK != hr)
    {
        m_aggregating = false;
        return hr;
    }

    do
    {
        //pass the info to each data set
        for (it = m_dataSets.begin(); it != m_dataSets.end(); it++)
        {
            //don't do double duty on ready sets
            if (!it->IsReady())
            {
                it->AddSample(&data, m_checkInterval);
            }
        }

        hr = GetNextRawAggregateRecord(&data, pPercentComplete);

        if (NULL != pCancel && *pCancel)
        {
            for (it = m_dataSets.begin(); it != m_dataSets.end(); it++)
            {
                //clear half-aggregated sets
                if (!it->IsReady())
                {
                    it->Clear();
                }
            }

            m_aggregating = false;
            return S_FALSE;
        }

        if (!SUCCEEDED(hr))
        {
            break;
        }
    }
    while (hr != S_FALSE);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    //in case it's an old prd
    if (0 == m_profileCoreMask)
    {
        int coreCount = m_pPrdReader->GetCoreCount();

        for (int i = 0; i < coreCount; i++)
        {
            m_profileCoreMask |= 1ULL << i;
        }
    }

    //aggregate the fast hashes into one large list in each module in each set
    for (it = m_dataSets.begin(); it != m_dataSets.end(); it++)
    {
        it->AggregrateMaps();
    }

#else
    //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

#if SUPPORT_CSS
    //write the css files to the default directory
    fnWriteCallSiteFiles(m_aggOutDir);
#endif

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    //write the jit files to the default directory
    fnWriteJncFiles(m_aggOutDir);
#else
    (void)(pJitDataDirectory); // unused
    //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    m_aggregating = false;
    return S_OK;
}


bool CpuProfileDataAccess::IsDataSetReady(const wchar_t* pDataSetName) const
{
    QString key;

    if (NULL != pDataSetName)
    {
        key = QString::fromWCharArray(pDataSetName);
    }

    if (!m_dataSets.contains(key))
    {
        return false;
    }

    return m_dataSets[key].IsReady();
}


HRESULT CpuProfileDataAccess::WriteSetToFile(const wchar_t* pDataSetName, const wchar_t* pFileName)
{
    QString key;

    if (NULL != pDataSetName)
    {
        key = QString::fromWCharArray(pDataSetName);
    }

    if (!m_dataSets.contains(key))
    {
        return E_INVALIDARG;
    }

    if (!m_dataSets[key].IsReady())
    {
        return E_FAIL;
    }

    return m_dataSets[key].WriteToFile(pFileName, m_aggOutDir, m_missedCount,
                                       m_profileCoreMask, m_cpuFamily, m_cpuModel,
                                       m_events, m_sampIntvls, m_groupCount,
                                       &m_cssPids, &m_topMap);
}


HRESULT CpuProfileDataAccess::AppendDataSets(const wchar_t* pDestinationDataSet, const wchar_t* pSourceDataSet)
{
    //assumes that the data sets exist and are ready
    QString key;

    if (NULL != pDestinationDataSet)
    {
        key = QString::fromWCharArray(pDestinationDataSet);
    }

    ProfileDataSet* pDest = &(m_dataSets[key]);
    key.clear();

    if (NULL != pSourceDataSet)
    {
        key = QString::fromWCharArray(pSourceDataSet);
    }

    ProfileDataSet* pSrc = &(m_dataSets[key]);

    QMap<gtUInt32, ModAggregate>::iterator procIt;

    for (procIt = pSrc->m_aggProcessData.begin();
         procIt != pSrc->m_aggProcessData.end(); procIt++)
    {
        if (pDest->m_aggProcessData.contains(procIt.key()))
        {
            pDest->m_aggProcessData.insert(procIt.key(), procIt.value());
        }
        else
        {
            Aggregate(&(pDest->m_aggProcessData[procIt.key()].sampleData),
                      &(procIt.value().sampleData));
        }
    }

    QMap<QString, ModuleAggregator>::iterator modIt;

    for (modIt = pSrc->m_aggModuleData.begin();
         modIt != pSrc->m_aggModuleData.end(); modIt++)
    {
        if (pDest->m_aggModuleData.contains(modIt.key()))
        {
            pDest->m_aggModuleData.insert(modIt.key(), modIt.value());
        }
        else
        {
            //aggregate processes within the module
            QMap<gtUInt32, ModAggregate>* pDestMap =
                &(pDest->m_aggModuleData[modIt.key()].m_aggProcessData);
            QMap<gtUInt32, ModAggregate>* pSrcMap =    &(modIt.value().m_aggProcessData);

            for (procIt = pSrcMap->begin(); procIt != pSrcMap->end(); procIt++)
            {
                if (pDestMap->contains(procIt.key()))
                {
                    pDestMap->insert(procIt.key(), procIt.value());
                }
                else
                {
                    Aggregate(&((*pDestMap)[procIt.key()].sampleData),
                              &(procIt.value().sampleData));
                }
            }

            //aggregate instructions within the module
            gtList<InstAggregate>::iterator destIt = pDest->m_aggModuleData[modIt.key()].m_dataList.begin();
            gtList<InstAggregate>::iterator destEndIt = pDest->m_aggModuleData[modIt.key()].m_dataList.end();
            gtList<InstAggregate>::iterator srcIt;

            for (srcIt = modIt.value().m_dataList.begin();
                 srcIt != modIt.value().m_dataList.end(); srcIt++)
            {
                while ((destIt != destEndIt) && (*destIt < *srcIt))
                {
                    destIt++;
                }

                if ((destIt == destEndIt) || ((*srcIt) < (*destIt)))
                {
                    pDest->m_aggModuleData[modIt.key()].m_dataList.insert(destIt, *srcIt);
                }
                else
                {
                    //At this point dest == src, so aggregate data
                    Aggregate(&((*destIt).sampleData), &((*srcIt).sampleData));
                }
            }
        }
    }

    return S_OK;
}


HRESULT CpuProfileDataAccess::GetAvailableCoreData(gtUInt64* pCoreMask) const
{
    if (NULL == pCoreMask)
    {
        return E_INVALIDARG;
    }

    *pCoreMask = m_profileCoreMask;
    return S_OK;
}


HRESULT CpuProfileDataAccess::GetAvailableCoreCount(unsigned int* pCoreCount) const
{
    if (NULL == pCoreCount)
    {
        return E_INVALIDARG;
    }

    *pCoreCount = m_coreCount;
    return S_OK;
}


HRESULT CpuProfileDataAccess::SetRuleCoreData(gtUInt64 coreMask)
{
    if (0 == coreMask)
    {
        return E_INVALIDARG;
    }

    if ((gtUInt64) - 1 == coreMask)
    {
        // Select all available cores
        m_ruleCoreMask = m_profileCoreMask ;
    }
    else if (coreMask > m_profileCoreMask)
    {
        // Requested core mask is invalid
        return E_INVALIDARG ;
    }
    else
    {
        // Requested core mask is valid, use it
        m_ruleCoreMask = coreMask ;
    }

    return S_OK;
}


HRESULT CpuProfileDataAccess::SetRuleForProcesses(unsigned int count, const unsigned int* pProcessIdList)
{
    if (0 == count)
    {
        return E_INVALIDARG;
    }

    m_ruleProcessIdList.clear();

    for (unsigned int i = 0; i < count; i++)
    {
        m_ruleProcessIdList.insert(pProcessIdList[i]);
    }

    return S_OK;
}

HRESULT CpuProfileDataAccess::SetRuleForThreads(unsigned int count, const unsigned int* pThreadIdList)
{
    if (0 == count)
    {
        return E_INVALIDARG;
    }

    m_ruleThreadIdList.clear();

    for (unsigned int i = 0; i < count; i++)
    {
        m_ruleThreadIdList.insert(pThreadIdList[i]);
    }

    return S_OK;
}


HRESULT CpuProfileDataAccess::ResetRules()
{
    m_ruleCoreMask = m_profileCoreMask;
    m_ruleProcessIdList.clear();
    m_ruleThreadIdList.clear();
    return S_OK;
}

size_t CpuProfileDataAccess::GetDataEventCount() const
{
    return m_events.size();
}

HRESULT CpuProfileDataAccess::GetDataEvents(
    /*in*/ unsigned int maxSize,
    /*out*/ gtUInt64* pPerformanceEvents,
    /*out*/ wchar_t** ppDataLabels,
    /*out*/ gtUInt64* pSampIntvls)
{
    if (maxSize < m_events.size())
    {
        return E_OUTOFMEMORY;
    }

    if (NULL != pPerformanceEvents)
    {
        gtList<gtUInt64>::iterator it = m_events.begin();

        for (int i = 0; it != m_events.end(); it++, i++)
        {
            pPerformanceEvents[i] = *it;
        }
    }

    if (NULL != ppDataLabels)
    {
        gtList<QString>::iterator it = m_labels.begin();

        for (int i = 0; it != m_labels.end(); it++, i++)
        {
            int size = (*it).size();
            wchar_t* pName = new wchar_t[size + 1];

            if (NULL == pName)
            {
                return E_OUTOFMEMORY;
            }

            //save new for later cleaning
            m_stringCache.push_back(pName);

            CopyWcharStr(pName, (*it).toStdWString().c_str(), size + 1);
            ppDataLabels[i] = pName;
        }
    }

    if (NULL != pSampIntvls)
    {
        gtList<gtUInt64>::iterator it = m_sampIntvls.begin();

        for (int i = 0; it != m_sampIntvls.end(); it++, i++)
        {
            pSampIntvls[i] = *it;
        }
    }

    return S_OK;
}


HRESULT CpuProfileDataAccess::Aggregate(SampleDataMap* pDestination, const SampleDataMap* pSource)
{
    if ((NULL == pDestination) || (NULL == pSource))
    {
        return E_INVALIDARG;
    }

    for (SampleDataMap::const_iterator it = pSource->begin(), itEnd = pSource->end(); it != itEnd; ++it)
    {
        SampleDataMap::iterator found = pDestination->find(it->first);

        if (found == pDestination->end())
        {
            pDestination->insert(*it);
        }
        else
        {
            found->second += it->second;
        }
    }

    return S_OK;
}


size_t CpuProfileDataAccess::GetModuleDataCount(const wchar_t* pDataSetName)
{
    QString key;

    if (NULL != pDataSetName)
    {
        key = QString::fromWCharArray(pDataSetName);
    }

    if (!m_dataSets.contains(key))
    {
        return 0;
    }

    if (!m_dataSets[key].IsReady())
    {
        return 0;
    }

    return m_dataSets[key].GetModuleDataCount(m_ruleCoreMask, m_ruleProcessIdList);
}


HRESULT CpuProfileDataAccess::GetModuleData(const wchar_t* pDataSetName, unsigned int maxSize, ModuleDataType* pSystemData)
{
    if (NULL == pSystemData)
    {
        return E_INVALIDARG;
    }

    QString key;

    if (NULL != pDataSetName)
    {
        key = QString::fromWCharArray(pDataSetName);
    }

    if (!m_dataSets.contains(key))
    {
        return E_INVALIDARG;
    }

    if (!m_dataSets[key].IsReady())
    {
        return E_FAIL;
    }

    return m_dataSets[key].FillModuleData(m_ruleCoreMask, m_ruleProcessIdList, maxSize, pSystemData);
}


size_t CpuProfileDataAccess::GetProcessDataCount(const wchar_t* pDataSetName)
{
    QString key;

    if (NULL != pDataSetName)
    {
        key = QString::fromWCharArray(pDataSetName);
    }

    if (!m_dataSets.contains(key))
    {
        return 0;
    }

    if (!m_dataSets[key].IsReady())
    {
        return 0;
    }

    return m_dataSets[key].GetProcessDataCount(m_ruleCoreMask, m_ruleProcessIdList);
}


HRESULT CpuProfileDataAccess::GetProcessData(const wchar_t* pDataSetName, unsigned int maxSize, ModuleDataType* pProcessData)
{
    if (NULL == pProcessData)
    {
        return E_INVALIDARG;
    }

    QString key;

    if (NULL != pDataSetName)
    {
        key = QString::fromWCharArray(pDataSetName);
    }

    if (!m_dataSets.contains(key))
    {
        return E_INVALIDARG;
    }

    if (!m_dataSets[key].IsReady())
    {
        return E_FAIL;
    }

    return m_dataSets[key].FillProcessData(m_ruleCoreMask, m_ruleProcessIdList, maxSize, pProcessData);
}


HRESULT CpuProfileDataAccess::GetInstructionDataCount(const wchar_t* pDataSetName, const wchar_t* modulePath, unsigned int* pCount)
{
    *pCount = 0;
    QString key;

    if (NULL != pDataSetName)
    {
        key = QString::fromWCharArray(pDataSetName);
    }

    if (!m_dataSets.contains(key))
    {
        return E_INVALIDARG;
    }

    if (!m_dataSets[key].IsReady())
    {
        return E_PENDING;
    }

    return m_dataSets[key].GetInstructionDataCount(m_ruleCoreMask, m_ruleProcessIdList, m_ruleThreadIdList, modulePath, pCount);
}


HRESULT CpuProfileDataAccess::GetInstructionData(const wchar_t* pDataSetName,
                                                 const wchar_t* modulePath,
                                                 unsigned int maxSize,
                                                 InstructionDataType* pInstructionData,
                                                 gtUInt64* pModuleLoadAddress,
                                                 ModuleType* pModuleType)
{
    if (NULL == pInstructionData)
    {
        return E_INVALIDARG;
    }

    QString key;

    if (NULL != pDataSetName)
    {
        key = QString::fromWCharArray(pDataSetName);
    }

    if (!m_dataSets.contains(key))
    {
        return E_INVALIDARG;
    }

    if (!m_dataSets[key].IsReady())
    {
        return E_FAIL;
    }

    return m_dataSets[key].FillInstructionData(m_ruleCoreMask, m_ruleProcessIdList, m_ruleThreadIdList, modulePath, maxSize,
                                               pInstructionData, pModuleLoadAddress, pModuleType);
}

HRESULT CpuProfileDataAccess::GetJitData(const wchar_t* pDataSetName,
                                         gtUInt64 address,
                                         const wchar_t* pJncDataFile,
                                         int jitType,
                                         JITDataType* pJitData)
{
    if (NULL == pJitData)
    {
        return E_INVALIDARG;
    }

    QString key;

    if (NULL != pDataSetName)
    {
        key = QString::fromWCharArray(pDataSetName);
    }

    if (!m_dataSets.contains(key))
    {
        return E_INVALIDARG;
    }

    if (!m_dataSets[key].IsReady())
    {
        return E_FAIL;
    }

    return m_dataSets[key].GetJitData(address, pJncDataFile, m_aggOutDir, jitType, pJitData);
}


HRESULT CpuProfileDataAccess::GetFirstRawRecord(const wchar_t* pDataSetName, RawDataType* pData)
{
    if (NULL == pData)
    {
        return E_INVALIDARG;
    }

    RawAggregate tempData;
    HRESULT hr = GetFirstRawAggregateRecord(pDataSetName, &tempData);

    if (S_OK == hr)
    {
        *pData = tempData.rawData;
        pData->data.count = static_cast<unsigned int>(tempData.sampleData.size());
        size_t count = pData->data.count;

        if (count > 0)
        {
            pData->data.keyArray = new SampleDatumKey[count];

            if (NULL == pData->data.keyArray)
            {
                pData->data.count = 0;
                return E_OUTOFMEMORY;
            }

            pData->data.dataArray = new gtUInt64[count];

            if (NULL == pData->data.dataArray)
            {
                pData->data.count = 0;
                delete [] pData->data.keyArray;
                pData->data.keyArray = NULL;
                return E_OUTOFMEMORY;
            }

            //save new for later cleanup
            m_keyCache.push_back(pData->data.keyArray);
            m_dataCache.push_back(pData->data.dataArray);

            int j = 0;
            SampleDataMap::iterator datum, datEnd;

            for (datum = tempData.sampleData.begin(), datEnd = tempData.sampleData.end(); datum != datEnd; ++datum, ++j)
            {
                pData->data.keyArray[j] = datum->first;
                pData->data.dataArray[j] = datum->second;
            }
        }
    }

    return hr;
}


HRESULT CpuProfileDataAccess::GetFirstRawAggregateRecord(const wchar_t* pDataSetName, RawAggregate* pData)
{
    if (NULL == pData)
    {
        return E_INVALIDARG;
    }

    QString key;

    if (NULL != pDataSetName)
    {
        key = QString::fromWCharArray(pDataSetName);
    }

    if (!m_dataSets.contains(key))
    {
        return E_INVALIDARG;
    }

    if (!IsProfileDataAvailable())
    {
        return E_ACCESSDENIED;
    }

    m_mutex.lock();
    m_pRawDataSet = &(m_dataSets[key]);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    //reset data to beginning
    m_pPrdReader->Close();
    m_pPrdReader->Initialize(m_inputFile);
#else
    //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    m_rawProgress = 1;
    m_rawCss.stack.clear();
    m_rawCss.dataAvailable = false;
    m_mutex.unlock();
    return GetNextRawAggregateRecord(pData, NULL);
}


HRESULT CpuProfileDataAccess::GetNextRawRecord(RawDataType* pData, float* pPercentComplete)
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (NULL == pData)
    {
        return E_INVALIDARG;
    }

    RawAggregate tempData;
    HRESULT hr = GetNextRawAggregateRecord(&tempData, pPercentComplete);

    if (S_OK == hr)
    {
        *pData = tempData.rawData;
        pData->data.count = (unsigned int)tempData.sampleData.size();

        if (pData->data.count > 0)
        {
            pData->data.keyArray = new SampleDatumKey[pData->data.count];

            if (NULL == pData->data.keyArray)
            {
                pData->data.count = 0;
                return E_OUTOFMEMORY;
            }

            pData->data.dataArray = new gtUInt64[pData->data.count];

            if (NULL == pData->data.dataArray)
            {
                pData->data.count = 0;
                delete [] pData->data.keyArray;
                pData->data.keyArray = NULL;
                return E_OUTOFMEMORY;
            }

            //save new for later cleanup
            m_keyCache.push_back(pData->data.keyArray);
            m_dataCache.push_back(pData->data.dataArray);

            int j = 0;
            SampleDataMap::iterator datum;

            for (datum = tempData.sampleData.begin();
                 datum != tempData.sampleData.end();
                 datum++, j++)
            {
                pData->data.keyArray[j] = datum->first;
                pData->data.dataArray[j] = datum->second;
            }
        }
    }

    return hr;
#else
    (void)(pData); // unused
    (void)(pPercentComplete); // unused
    //TODO: [Suravee]
    return E_NOTIMPL;
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}


HRESULT CpuProfileDataAccess::GetNextRawAggregateRecord(RawAggregate* pData, float* pPercentComplete)
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    QMutexLocker locker(&m_mutex);

    if (NULL == pData)
    {
        return E_INVALIDARG;
    }

    if (0 == m_rawProgress)
    {
        return E_POINTER;
    }

    if (!IsProfileDataAvailable())
    {
        return E_ACCESSDENIED;
    }

    pData->rawData.address = 0;
    pData->sampleData.clear();
    pData->rawData.loadAddress = 0;
    pData->rawData.moduleSize = 0;
    pData->rawData.path = NULL;
    pData->rawData.pJitDataFile = NULL;
    pData->rawData.processId = 0;
    pData->rawData.threadId = 0;
    pData->rawData.type = InvalidModType;

    bool notData = true;

    do
    {
        if (m_rawProgress > m_approxRecords)
        {
            if (NULL != pPercentComplete)
            {
                *pPercentComplete = 100.0;
            }

            m_rawProgress = 0;
            return S_FALSE;
        }

        RawPRDRecord tRawRec, tRawRec2;
        tRawRec.rawRecordsData[0] = 0;
        unsigned int recNum = 0;
        HRESULT hr = m_pPrdReader->GetNextRawRecords(&tRawRec, &tRawRec2, &recNum);
        unsigned char recordType = tRawRec.rawRecordsData[0];

        //the first record after the CSS records end, save the data
        if ((m_rawCss.dataAvailable) && (recordType != PROF_REC_CSS))
        {
            if (IsRawOkayByRules(m_rawCss.pid, m_rawCss.tid,
                                 m_rawCss.core, &(pData->rawData.timeMark), m_checkInterval))
            {
                //Ignore css data that is in the System process
                if ((4 != m_rawCss.pid) && (8 != m_rawCss.pid))
                {
                    ProcessCssData();
                }
            }

            // clean up data;
            m_rawCss.stack.clear();
            m_rawCss.dataAvailable = false;
        }

        if (S_OK != hr)
        {
            if (NULL != pPercentComplete)
            {
                *pPercentComplete = 100.0;
            }

            m_rawProgress = 0;
            return S_FALSE;
        }

        m_rawProgress += recNum;

        TiModuleInfo modInfo;
        memset(&modInfo, 0, sizeof(TiModuleInfo));

        switch (recordType)
        {
            case PROF_REC_CSS:
            {
                //if we're not aggregating, ignore the css records
                if (!m_aggregating)
                {
                    break;
                }

                PRD_CSS_DATA_RECORD* pCSSRec = (PRD_CSS_DATA_RECORD*)(&tRawRec);

                for (int j = 0; j < CSS_DATA_PER_RECORD; j++)
                {
                    if (pCSSRec->m_CallStack[j])
                    {
                        m_rawCss.stack.push_back(pCSSRec->m_CallStack[j]);
                        m_rawCss.dataAvailable = true;
                    }
                }

                // Continue to process next record
                break;
            }
            break;

            case PROF_REC_EVENT:
            case PROF_REC_TIMER:
            {
                RecordDataStruct prdRecord;

                if (S_OK != m_pPrdReader->ConvertSampleData(tRawRec,
                                                            &prdRecord))
                {
                    break;
                }

                //synchronize start TIMEMARK, ti's mS, and the timestamp
                pData->rawData.timeMark = SynchronizeCPA(m_entire.start,
                                                         prdRecord.m_DeltaTick, prdRecord.m_ProcessorID);
                m_rawCss.timeStamp = prdRecord.m_DeltaTick;
                m_rawCss.pid = pData->rawData.processId = (unsigned int)prdRecord.m_PID;
                m_rawCss.tid = pData->rawData.threadId = (unsigned int)prdRecord.m_ThreadHandle;
                m_rawCss.core = prdRecord.m_ProcessorID;

                if (!IsRawOkayByRules(prdRecord.m_PID,
                                      prdRecord.m_ThreadHandle, prdRecord.m_ProcessorID,
                                      &(pData->rawData.timeMark), m_checkInterval))
                {
                    break;
                }

                m_rawCss.addr = pData->rawData.address = prdRecord.m_RIP;

                SampleDatumKey key;
                key.core = prdRecord.m_ProcessorID;
                //bit 17 OS, bit 16 USR >> 16 = prdRecord.m_eventBitMask
                key.event = EncodeEvent(prdRecord.m_EventType,
                                        prdRecord.m_EventUnitMask,
                                        (prdRecord.m_eventBitMask & 2) != 0,
                                        (prdRecord.m_eventBitMask & 1) != 0);
                pData->sampleData.insert(SampleDataMap::value_type(key, 1));

                GetModInfoHelper(&prdRecord, &modInfo, recordType);
                pData->rawData.path = modInfo.pModulename;
                pData->rawData.loadAddress = modInfo.ModuleStartAddr;
                pData->rawData.moduleSize = modInfo.Modulesize;
                pData->rawData.pJitDataFile = modInfo.pJncName;
                pData->rawData.pJitFunctionName = modInfo.pFunctionName;

                switch (modInfo.moduleType)
                {
                    case evJavaModule:
                        pData->rawData.type = JavaModType;
                        break;

                    case evManaged:
                        pData->rawData.type = ManagedModType;
                        break;

                    case evPEModule:
                        pData->rawData.type = UnmanagedModType;
                        break;

                    default:
                        pData->rawData.type = InvalidModType;
                        break;
                }

                notData = false;
                break;
            }

            case PROF_REC_IBS_FETCH_BASIC:
            case PROF_REC_IBS_FETCH_EXT:
            {
                IBSFetchRecordData ibsFetch;

                if (S_OK != m_pPrdReader->ConvertIBSFetchData(&tRawRec,
                                                              &tRawRec2, &ibsFetch))
                {
                    break;
                }

                //synchronize start TIMEMARK, ti's mS, and the timestamp
                pData->rawData.timeMark = SynchronizeCPA(m_entire.start,
                                                         ibsFetch.m_DeltaTick, ibsFetch.m_ProcessorID);
                m_rawCss.timeStamp = ibsFetch.m_DeltaTick;
                m_rawCss.pid = ibsFetch.m_PID;
                m_rawCss.tid = ibsFetch.m_ThreadHandle;
                m_rawCss.core = ibsFetch.m_ProcessorID;

                if (!IsRawOkayByRules(ibsFetch.m_PID, ibsFetch.m_ThreadHandle,
                                      ibsFetch.m_ProcessorID, &(pData->rawData.timeMark), m_checkInterval))
                {
                    break;
                }

                GetModInfoHelper(&ibsFetch, &modInfo, recordType);
                TranslateIbsFetch(ibsFetch, &modInfo, pData);
                notData = false;
            }
            break;

            case PROF_REC_IBS_OP_BASIC:
            case PROF_REC_IBS_OP_EXT:
            {
                IBSOpRecordData ibsOp;

                if (S_OK != m_pPrdReader->ConvertIBSOpData(&tRawRec,
                                                           &tRawRec2, &ibsOp))
                {
                    break;
                }

                //synchronize start TIMEMARK, ti's mS, and the timestamp
                pData->rawData.timeMark = SynchronizeCPA(m_entire.start,
                                                         ibsOp.m_DeltaTick, ibsOp.m_ProcessorID);
                m_rawCss.timeStamp = ibsOp.m_DeltaTick;
                m_rawCss.pid = ibsOp.m_PID;
                m_rawCss.tid = ibsOp.m_ThreadHandle;
                m_rawCss.core = ibsOp.m_ProcessorID;

                if (!IsRawOkayByRules(ibsOp.m_PID, ibsOp.m_ThreadHandle,
                                      ibsOp.m_ProcessorID, &(pData->rawData.timeMark),
                                      m_checkInterval))
                {
                    break;
                }

                GetModInfoHelper(&ibsOp, &modInfo, recordType);

                TranslateIbsOp(ibsOp, &modInfo, pData);
                notData = false;
            }
            break;

            case PROF_REC_MISSED:

            //Ignored, now
            default:
                // Continue to process next record
                break;
        }
    }
    while (notData);

    if (NULL != pPercentComplete)
    {
        *pPercentComplete = (100.0F * m_rawProgress) / m_approxRecords;
    }

    return S_OK;
#else
    //TODO: [Suravee]
    (void)(pData); // unused
    (void)(pPercentComplete); // unused
    return E_NOTIMPL;
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}


void CpuProfileDataAccess::SetDefaultJitDir()
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    //set the Jit Dir to the registry default
    GetTempPath(MAX_PATH, m_jitDir);
#else
    //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}

unsigned int CpuProfileDataAccess::GetProfileType() const
{
    if (!IsProfileDataAvailable())
    {
        return 0;
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    return m_pPrdReader->GetProfileType();
#else
    //TODO: [Suravee]
    return 0;
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}

void CpuProfileDataAccess::GetIbsConfig(gtUInt64* pFetchCount, gtUInt64* pOpsCount) const
{
    if (!IsProfileDataAvailable() || NULL == pFetchCount || NULL == pOpsCount)
    {
        return;
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    unsigned int fetch;
    unsigned int ops;

    m_pPrdReader->GetIBSConfig(&fetch, &ops);
    *pFetchCount = fetch;
    *pOpsCount = ops;
#else
    //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    return;
}


int CpuProfileDataAccess::GetCpuFamily() const
{
    return m_cpuFamily;
}

int CpuProfileDataAccess::GetCpuModel() const
{
    return m_cpuModel;
}

void CpuProfileDataAccess::AddIbsFetchEvents()
{
    AddOneIbsEvent(DE_IBS_FETCH_ALL);
    AddOneIbsEvent(DE_IBS_FETCH_KILLED);
    AddOneIbsEvent(DE_IBS_FETCH_ATTEMPTED);
    AddOneIbsEvent(DE_IBS_FETCH_COMPLETED);
    AddOneIbsEvent(DE_IBS_FETCH_ABORTED);
    AddOneIbsEvent(DE_IBS_L1_ITLB_HIT);
    AddOneIbsEvent(DE_IBS_ITLB_L1M_L2H);
    AddOneIbsEvent(DE_IBS_ITLB_L1M_L2M);
    AddOneIbsEvent(DE_IBS_IC_MISS);
    AddOneIbsEvent(DE_IBS_IC_HIT);
    AddOneIbsEvent(DE_IBS_FETCH_4K_PAGE);
    AddOneIbsEvent(DE_IBS_FETCH_2M_PAGE);
    AddOneIbsEvent(DE_IBS_FETCH_LATENCY);

    int cpuFamily = GetCpuFamily();
    int cpuModels = GetCpuModel() >> 4;

    // Only supported by family:0x15, models:0x60-0x6F
    if (FAMILY_OR == cpuFamily && 0x6 == cpuModels)
    {
        AddOneIbsEvent(DE_IBS_FETCH_L2C_MISS);
        AddOneIbsEvent(DE_IBS_ITLB_REFILL_LAT);
    }
}

void CpuProfileDataAccess::AddIbsOpEvents()
{
    AddOneIbsEvent(DE_IBS_OP_ALL) ;
    AddOneIbsEvent(DE_IBS_OP_TAG_TO_RETIRE) ;
    AddOneIbsEvent(DE_IBS_OP_COMP_TO_RETIRE) ;

    AddOneIbsEvent(DE_IBS_BRANCH_RETIRED) ;
    AddOneIbsEvent(DE_IBS_BRANCH_MISP) ;
    AddOneIbsEvent(DE_IBS_BRANCH_TAKEN) ;
    AddOneIbsEvent(DE_IBS_BRANCH_MISP_TAKEN) ;
    AddOneIbsEvent(DE_IBS_RETURN) ;
    AddOneIbsEvent(DE_IBS_RETURN_MISP) ;
    AddOneIbsEvent(DE_IBS_RESYNC) ;

    AddOneIbsEvent(DE_IBS_LS_ALL_OP) ;
    AddOneIbsEvent(DE_IBS_LS_LOAD_OP) ;
    AddOneIbsEvent(DE_IBS_LS_STORE_OP) ;
    AddOneIbsEvent(DE_IBS_LS_DTLB_L1H) ;
    AddOneIbsEvent(DE_IBS_LS_DTLB_L1M_L2H) ;
    AddOneIbsEvent(DE_IBS_LS_DTLB_L1M_L2M) ;
    AddOneIbsEvent(DE_IBS_LS_DC_MISS) ;
    AddOneIbsEvent(DE_IBS_LS_DC_HIT) ;
    AddOneIbsEvent(DE_IBS_LS_MISALIGNED) ;
    AddOneIbsEvent(DE_IBS_LS_BNK_CONF_LOAD) ;
    AddOneIbsEvent(DE_IBS_LS_BNK_CONF_STORE) ;
    AddOneIbsEvent(DE_IBS_LS_STL_FORWARDED) ;
    AddOneIbsEvent(DE_IBS_LS_STL_CANCELLED) ;
    AddOneIbsEvent(DE_IBS_LS_UC_MEM_ACCESS) ;
    AddOneIbsEvent(DE_IBS_LS_WC_MEM_ACCESS) ;
    AddOneIbsEvent(DE_IBS_LS_LOCKED_OP) ;
    AddOneIbsEvent(DE_IBS_LS_MAB_HIT) ;
    AddOneIbsEvent(DE_IBS_LS_L1_DTLB_4K) ;
    AddOneIbsEvent(DE_IBS_LS_L1_DTLB_2M) ;
    AddOneIbsEvent(DE_IBS_LS_L1_DTLB_1G) ;
    AddOneIbsEvent(DE_IBS_LS_L2_DTLB_4K) ;
    AddOneIbsEvent(DE_IBS_LS_L2_DTLB_2M) ;
    AddOneIbsEvent(DE_IBS_LS_L2_DTLB_1G) ;
    AddOneIbsEvent(DE_IBS_LS_DC_LOAD_LAT) ;

    int cpuFamily = GetCpuFamily();
    int cpuModels = GetCpuModel() >> 4;

    // Only supported by family:0x15, models:0x60-0x6F
    if (FAMILY_OR == cpuFamily && 0x6 == cpuModels)
    {
        AddOneIbsEvent(DE_IBS_LS_DC_LD_RESYNC);
    }

    AddOneIbsEvent(DE_IBS_NB_LOCAL) ;
    AddOneIbsEvent(DE_IBS_NB_REMOTE) ;
    AddOneIbsEvent(DE_IBS_NB_LOCAL_L3) ;
    AddOneIbsEvent(DE_IBS_NB_LOCAL_CACHE) ;
    AddOneIbsEvent(DE_IBS_NB_REMOTE_CACHE) ;
    AddOneIbsEvent(DE_IBS_NB_LOCAL_DRAM) ;
    AddOneIbsEvent(DE_IBS_NB_REMOTE_DRAM) ;
    AddOneIbsEvent(DE_IBS_NB_LOCAL_OTHER) ;
    AddOneIbsEvent(DE_IBS_NB_REMOTE_OTHER) ;
    AddOneIbsEvent(DE_IBS_NB_CACHE_STATE_M) ;
    AddOneIbsEvent(DE_IBS_NB_CACHE_STATE_O) ;
    AddOneIbsEvent(DE_IBS_NB_LOCAL_LATENCY) ;
    AddOneIbsEvent(DE_IBS_NB_REMOTE_LATENCY) ;
}


void CpuProfileDataAccess::AddOneIbsEvent(unsigned int eventSelect)
{
    //no unit masks for the IBS events
    EventMaskType encodedEvent = EncodeEvent((gtUInt16)eventSelect, 0, false, false);
    m_events.push_back(encodedEvent);

    CpuEvent oneEvent;
    m_eventsFile.FindEventByValue(eventSelect, oneEvent);
    m_labels.push_back(oneEvent.name);

    m_sampIntvls.push_back(1);
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
static wchar_t smoduleName[OS_MAX_PATH + 1];
static wchar_t sfunctionName [OS_MAX_PATH + 1];
static wchar_t sjncName [OS_MAX_PATH + 1];
static wchar_t sjavaSrcFileName [OS_MAX_PATH + 1];
//Thread safe since it's only called while a mutex is locked
void CpuProfileDataAccess::GetModInfoHelper(const void* pVoid, TiModuleInfo* pModInfo, unsigned char dataType)
{
    if (!pVoid || !pModInfo)
    {
        return;
    }

    smoduleName[0] = L'\0';
    sfunctionName[0] = L'\0';
    sjncName[0] = L'\0';
    sjavaSrcFileName[0] = L'\0';

    switch (dataType)
    {
        case PROF_REC_EVENT:
        case PROF_REC_TIMER:
        {
            const RecordDataStruct* prdRecord = static_cast<const RecordDataStruct*>(pVoid);
            pModInfo->processID = prdRecord->m_PID;
            m_rawCss.addr = pModInfo->sampleAddr = prdRecord->m_RIP;
            pModInfo->cpuIndex = prdRecord->m_ProcessorID;
            pModInfo->deltaTick = prdRecord->m_DeltaTick;
        }
        break;

        case PROF_REC_IBS_FETCH_BASIC:
        case PROF_REC_IBS_FETCH_EXT:
        {
            const IBSFetchRecordData* pIBSFetchRec = static_cast<const IBSFetchRecordData*>(pVoid);
            pModInfo->processID = pIBSFetchRec->m_PID;
            m_rawCss.addr = pModInfo->sampleAddr = pIBSFetchRec->m_RIP;
            pModInfo->cpuIndex = pIBSFetchRec->m_ProcessorID;
            pModInfo->deltaTick = pIBSFetchRec->m_DeltaTick;
        }
        break;

        case PROF_REC_IBS_OP_BASIC:
        case PROF_REC_IBS_OP_EXT:
        {
            const IBSOpRecordData* pIBSOpRec = static_cast<const IBSOpRecordData*>(pVoid);
            pModInfo->processID = pIBSOpRec->m_PID;
            m_rawCss.addr = pModInfo->sampleAddr = pIBSOpRec->m_RIP;
            pModInfo->cpuIndex = pIBSOpRec->m_ProcessorID;
            pModInfo->deltaTick = pIBSOpRec->m_DeltaTick;
        }
        break;
    }

    pModInfo->funNameSize = pModInfo->jncNameSize = pModInfo->namesize = OS_MAX_PATH;
    pModInfo->pModulename = smoduleName;
    pModInfo->pFunctionName = sfunctionName;
    pModInfo->pJncName = sjncName;
    pModInfo->srcfilesize = OS_MAX_PATH;
    pModInfo->pJavaSrcFileName = sjavaSrcFileName;
    pModInfo->moduleType = evPEModule;
    pModInfo->ModuleStartAddr = 0;

    //Get the info from the TI files
    if (fnGetModuleInfo(pModInfo) != S_OK)
    {
        //handle unknown module samples...
        gtUInt64 kernelSpace = KERNEL_SPACE_START;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        BOOL on64 = false;
        IsWow64Process(GetCurrentProcess(), &on64);

        if (on64)
        {
            kernelSpace = KERNEL64_SPACE_START;
        }

#endif

        //note that unknown user mode 64-bit samples may be considered kernel
        //  samples if imported on a 32-bit system
        if (pModInfo->sampleAddr > KERNEL_SPACE_START)
        {
            // Unknown kernel module
            swprintf_s(smoduleName, OS_MAX_PATH, L"unknown kernel samples");
        }
        else
        {
            // unknown user module
            swprintf_s(smoduleName, OS_MAX_PATH, L"unknown module pid (%lld)",
                       pModInfo->processID);
        }
    }

    if (0 == wcslen(sfunctionName))
    {
        pModInfo->pFunctionName = NULL;
    }

    if (0 == wcslen(sjncName))
    {
        pModInfo->pJncName = NULL;
    }

    if (0 == wcslen(sjavaSrcFileName))
    {
        pModInfo->pJavaSrcFileName = NULL;
    }
}
#else
//TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS


bool CpuProfileDataAccess::IsRawOkayByRules(gtUInt64 pid, gtUInt64 tid, int core, CPA_TIME* pTime, bool checkInterval) const
{
    if (!m_ruleProcessIdList.empty() && !m_ruleProcessIdList.contains(pid))
    {
        return false;
    }

    if (!m_ruleThreadIdList.empty() && !m_ruleThreadIdList.contains(tid))
    {
        return false;
    }

    //is a valid core?
    if (0 == (m_ruleCoreMask & (1ULL << core)))
    {
        return false;
    }

    if (checkInterval && !m_pRawDataSet->IsInInterval(pTime))
    {
        return false;
    }

    return true;
}


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
void CpuProfileDataAccess::TranslateIbsFetch(const IBSFetchRecordData& ibsFetchRec, const TiModuleInfo* pModInfo, RawAggregate* pData)
{
    pData->rawData.address = ibsFetchRec.m_RIP;
    pData->rawData.processId = (unsigned int)ibsFetchRec.m_PID;
    pData->rawData.threadId = (unsigned int)ibsFetchRec.m_ThreadHandle;

    SampleDatumKey key;
    key.core = ibsFetchRec.m_ProcessorID;

    pData->rawData.path = pModInfo->pModulename;
    pData->rawData.pJitDataFile = pModInfo->pJncName;
    pData->rawData.pJitFunctionName = pModInfo->pFunctionName;
    pData->rawData.loadAddress = pModInfo->ModuleStartAddr;
    pData->rawData.moduleSize = pModInfo->Modulesize;

    switch (pModInfo->moduleType)
    {
        case evJavaModule:
            pData->rawData.type = JavaModType;
            break;

        case evManaged:
            pData->rawData.type = ManagedModType;
            break;

        case evPEModule:
            pData->rawData.type = UnmanagedModType;
            break;

        default:
            pData->rawData.type = InvalidModType;
            break;
    }

    // IBS all fetch samples (kills + attempts)
    key.event = EncodeEvent(DE_IBS_FETCH_ALL, 0, false, false);
    pData->sampleData.insert(SampleDataMap::value_type(key, 1));

    // IBS killed fetches ("case 0") -- All interesting event
    // flags are clear
    if (ibsFetchRec.m_Killed)
    {
        key.event = EncodeEvent(DE_IBS_FETCH_KILLED, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
        // Take an early out with IBS killed fetches, effectively
        // filtering killed fetches out of the other event counts
        return;
    }

    // Any non-killed fetch is an attempted fetch
    key.event = EncodeEvent(DE_IBS_FETCH_ATTEMPTED, 0, false, false);
    pData->sampleData.insert(SampleDataMap::value_type(key, 1));

    if (ibsFetchRec.m_FetchCompletion)
    {
        // IBS Fetch Completed
        key.event = EncodeEvent(DE_IBS_FETCH_COMPLETED, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }
    else
    {
        // IBS Fetch Aborted
        key.event = EncodeEvent(DE_IBS_FETCH_ABORTED, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    // IBS L1 ITLB hit
    if (ibsFetchRec.m_L1TLBHit)
    {
        key.event = EncodeEvent(DE_IBS_L1_ITLB_HIT, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    // IBS L1 ITLB miss and L2 ITLB hit
    if (ibsFetchRec.m_ITLB_L1M_L2H)
    {
        key.event = EncodeEvent(DE_IBS_ITLB_L1M_L2H, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    // IBS L1 & L2 ITLB miss; complete ITLB miss
    if (ibsFetchRec.m_ITLB_L1M_L2M)
    {
        key.event = EncodeEvent(DE_IBS_ITLB_L1M_L2M, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    // IBS instruction cache miss
    if (ibsFetchRec.m_InstCacheMiss)
    {
        key.event = EncodeEvent(DE_IBS_IC_MISS, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    // IBS instruction cache hit
    if (ibsFetchRec.m_InstCacheHit)
    {
        key.event = EncodeEvent(DE_IBS_IC_HIT, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    // IBS page translations (valid when ibsFetchRec.m_PhysicalAddrValid is set)
    if (ibsFetchRec.m_PhysicalAddrValid)
    {
        switch (ibsFetchRec.m_TLBPageSize)
        {
            case L1TLB4K:
                key.event = EncodeEvent(DE_IBS_FETCH_4K_PAGE, 0, false, false);
                pData->sampleData.insert(SampleDataMap::value_type(key, 1));
                break;

            case L1TLB2M:
                key.event = EncodeEvent(DE_IBS_FETCH_2M_PAGE, 0, false, false);
                pData->sampleData.insert(SampleDataMap::value_type(key, 1));
                break;

            default:
                // DE_IBS_FETCH_1G_PAGE ;
                // DE_IBS_FETCH_XX_PAGE ;
                break;
        }
    }

    if (ibsFetchRec.m_FetchLatency)
    {
        key.event = EncodeEvent(DE_IBS_FETCH_LATENCY, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key,
                                                           ibsFetchRec.m_FetchLatency));
    }
}
#else
//TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
void CpuProfileDataAccess::TranslateIbsOp(const IBSOpRecordData& ibsOpRec, const TiModuleInfo* pModInfo, RawAggregate* pData)
{
    pData->rawData.address = ibsOpRec.m_RIP;
    pData->rawData.processId = (unsigned int)ibsOpRec.m_PID;
    pData->rawData.threadId = (unsigned int)ibsOpRec.m_ThreadHandle;

    SampleDatumKey key;
    key.core = ibsOpRec.m_ProcessorID;

    pData->rawData.path = pModInfo->pModulename;
    pData->rawData.pJitDataFile = pModInfo->pJncName;
    pData->rawData.pJitFunctionName = pModInfo->pFunctionName;
    pData->rawData.loadAddress = pModInfo->ModuleStartAddr;
    pData->rawData.moduleSize = pModInfo->Modulesize;

    switch (pModInfo->moduleType)
    {
        case evJavaModule:
            pData->rawData.type = JavaModType;
            break;

        case evManaged:
            pData->rawData.type = ManagedModType;
            break;

        case evPEModule:
            pData->rawData.type = UnmanagedModType;
            break;

        default:
            pData->rawData.type = InvalidModType;
            break;
    }

    // All IBS op samples
    key.event = EncodeEvent(DE_IBS_OP_ALL, 0, false, false);
    pData->sampleData.insert(SampleDataMap::value_type(key, 1));

    // Tally retire cycle counts for all sampled macro-ops
    if (ibsOpRec.m_TagToRetireCycles)
    {
        // IBS tagged to retire cycles
        key.event = EncodeEvent(DE_IBS_OP_TAG_TO_RETIRE, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key,
                                                           ibsOpRec.m_TagToRetireCycles));
    }

    if (ibsOpRec.m_CompToRetireCycles)
    {
        // IBS completion to retire cycles
        key.event = EncodeEvent(DE_IBS_OP_COMP_TO_RETIRE, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key,
                                                           ibsOpRec.m_CompToRetireCycles));
    }

    // Test for an IBS branch macro-op
    if (ibsOpRec.m_OpBranchRetired)
    {
        // IBS Branch retired op
        key.event = EncodeEvent(DE_IBS_BRANCH_RETIRED, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));

        // Test branch-specific event flags
        if (ibsOpRec.m_OpBranchMispredicted)
        {
            // IBS mispredicted Branch op
            key.event = EncodeEvent(DE_IBS_BRANCH_MISP, 0, false, false);
            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
        }

        if (ibsOpRec.m_OpBranchTaken)
        {
            // IBS taken Branch op
            key.event = EncodeEvent(DE_IBS_BRANCH_TAKEN, 0, false, false);
            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
        }

        if (ibsOpRec.m_OpBranchTaken && ibsOpRec.m_OpBranchMispredicted)
        {
            // IBS mispredicted taken branch op
            key.event = EncodeEvent(DE_IBS_BRANCH_MISP_TAKEN, 0, false, false);
            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
        }

        if (ibsOpRec.m_OpReturn)
        {
            // IBS return op
            key.event = EncodeEvent(DE_IBS_RETURN, 0, false, false);
            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
        }

        if (ibsOpRec.m_OpReturn && ibsOpRec.m_OpBranchMispredicted)
        {
            // IBS mispredicted return op
            key.event = EncodeEvent(DE_IBS_RETURN_MISP, 0, false, false);
            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
        }

        //Only add the address if we're not aggregating
        if ((!m_aggregating) && (0 != ibsOpRec.m_BranchTarget))
        {
            //IBS branch target address
            pData->rawData.ibsOpBranchAddress =  ibsOpRec.m_BranchTarget;
        }
    } // Branch and return op sample

    // Test for a resync macro-op
    if (ibsOpRec.m_OpBranchResync)
    {
        // IBS resync OP
        key.event = EncodeEvent(DE_IBS_RESYNC, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    if (!ibsOpRec.m_IbsLdOp && !ibsOpRec.m_IbsStOp)
    {
        // If no load or store operation, then take an early return
        // No more derived events need to be tallied
        return;
    }

    // Count the number of LS op samples
    key.event = EncodeEvent(DE_IBS_LS_ALL_OP, 0, false, false);
    pData->sampleData.insert(SampleDataMap::value_type(key, 1));

    // Count and handle load ops
    if (ibsOpRec.m_IbsLdOp)
    {
        // Tally an IBS load derived event
        key.event = EncodeEvent(DE_IBS_LS_LOAD_OP, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));

        // If the load missed in DC, tally the DC load miss latency
        if (ibsOpRec.m_IbsDcMiss)
        {
            // DC load miss latency is only reliable for load ops
            key.event = EncodeEvent(DE_IBS_LS_DC_LOAD_LAT, 0, false, false);
            pData->sampleData.insert(SampleDataMap::value_type(key,
                                                               ibsOpRec.m_IbsDcMissLat));
        }

        // Data forwarding info are valid only for load ops
        if (ibsOpRec.m_IbsDcStToLdFwd)
        {
            key.event = EncodeEvent(DE_IBS_LS_STL_FORWARDED, 0, false, false);
            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
        }

        if (ibsOpRec.m_IbsDcStToLdCan)
        {
            key.event = EncodeEvent(DE_IBS_LS_STL_CANCELLED, 0, false, false);
            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
        }

        // NB data is only guaranteed reliable for load operations
        // that miss in L1 and L2 cache. NB data arrives too late
        // to be reliable for store operations
        if (ibsOpRec.m_IbsDcMiss && (ibsOpRec.m_NbIbsReqSrc != 0))
        {
            // NB data is valid, so tally derived NB events
            if (ibsOpRec.m_NbIbsReqDstProc)
            {
                // Request was serviced by remote processor
                key.event = EncodeEvent(DE_IBS_NB_REMOTE, 0, false, false);
                pData->sampleData.insert(SampleDataMap::value_type(key, 1));

                key.event = EncodeEvent(DE_IBS_NB_REMOTE_LATENCY, 0, false, false);
                pData->sampleData.insert(SampleDataMap::value_type(key,
                                                                   ibsOpRec.m_IbsDcMissLat));

                switch (ibsOpRec.m_NbIbsReqSrc)
                {
                    case 0x2:
                        key.event = EncodeEvent(DE_IBS_NB_REMOTE_CACHE, 0, false, false);
                        pData->sampleData.insert(SampleDataMap::value_type(key, 1));

                        if (ibsOpRec.m_NbIbsCacheHitSt)
                        {
                            key.event = EncodeEvent(DE_IBS_NB_CACHE_STATE_O, 0, false, false);
                            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
                        }
                        else
                        {
                            key.event = EncodeEvent(DE_IBS_NB_CACHE_STATE_M, 0, false, false);
                            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
                        }

                        break ;

                    case 0x3:
                        key.event = EncodeEvent(DE_IBS_NB_REMOTE_DRAM, 0, false, false);
                        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
                        break ;

                    case 0x7:
                        key.event = EncodeEvent(DE_IBS_NB_REMOTE_OTHER, 0, false, false);
                        pData->sampleData.insert(SampleDataMap::value_type(key, 1));

                    default:
                        break ;
                }
            }
            else
            {
                // Request was serviced by local processor
                key.event = EncodeEvent(DE_IBS_NB_LOCAL, 0, false, false);
                pData->sampleData.insert(SampleDataMap::value_type(key, 1));

                key.event = EncodeEvent(DE_IBS_NB_LOCAL_LATENCY, 0, false, false);
                pData->sampleData.insert(SampleDataMap::value_type(key,
                                                                   ibsOpRec.m_IbsDcMissLat));

                switch (ibsOpRec.m_NbIbsReqSrc)
                {
                    case 0x1:
                        key.event = EncodeEvent(DE_IBS_NB_LOCAL_L3, 0, false, false);
                        pData->sampleData.insert(SampleDataMap::value_type(key, 1));

                    case 0x2:
                        key.event = EncodeEvent(DE_IBS_NB_LOCAL_CACHE, 0, false, false);
                        pData->sampleData.insert(SampleDataMap::value_type(key, 1));

                        if (ibsOpRec.m_NbIbsCacheHitSt)
                        {
                            key.event = EncodeEvent(DE_IBS_NB_CACHE_STATE_O, 0, false, false);
                            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
                        }
                        else
                        {
                            key.event = EncodeEvent(DE_IBS_NB_CACHE_STATE_M, 0, false, false);
                            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
                        }

                        break ;

                    case 0x3:
                        key.event = EncodeEvent(DE_IBS_NB_LOCAL_DRAM, 0, false, false);
                        pData->sampleData.insert(SampleDataMap::value_type(key, 1));

                    case 0x7:
                        key.event = EncodeEvent(DE_IBS_NB_LOCAL_OTHER, 0, false, false);
                        pData->sampleData.insert(SampleDataMap::value_type(key, 1));

                    default:
                        break ;
                }
            }
        }
    }

    // Count and handle store ops
    if (ibsOpRec.m_IbsStOp)
    {
        key.event = EncodeEvent(DE_IBS_LS_STORE_OP, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    if (ibsOpRec.m_IbsDcMiss)
    {
        key.event = EncodeEvent(DE_IBS_LS_DC_MISS, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }
    else
    {
        key.event = EncodeEvent(DE_IBS_LS_DC_HIT, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    if (ibsOpRec.m_IbsDcMisAcc)
    {
        key.event = EncodeEvent(DE_IBS_LS_MISALIGNED, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    if (ibsOpRec.m_IbsDcLdBnkCon)
    {
        key.event = EncodeEvent(DE_IBS_LS_BNK_CONF_LOAD, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    if (ibsOpRec.m_IbsDcStBnkCon)
    {
        key.event = EncodeEvent(DE_IBS_LS_BNK_CONF_STORE, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    if (ibsOpRec.m_IbsDcUcMemAcc)
    {
        key.event = EncodeEvent(DE_IBS_LS_UC_MEM_ACCESS, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    if (ibsOpRec.m_IbsDcWcMemAcc)
    {
        key.event = EncodeEvent(DE_IBS_LS_WC_MEM_ACCESS, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    if (ibsOpRec.m_IbsDcLockedOp)
    {
        key.event = EncodeEvent(DE_IBS_LS_LOCKED_OP, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    if (ibsOpRec.m_IbsDcMabHit)
    {
        key.event = EncodeEvent(DE_IBS_LS_MAB_HIT, 0, false, false);
        pData->sampleData.insert(SampleDataMap::value_type(key, 1));
    }

    //Only add the address if we're not aggregating
    if ((!m_aggregating) && (ibsOpRec.m_IbsDcPhyAddrValid))
    {
        if (0 != ibsOpRec.m_IbsDcPhysAd)
        {
            //IBS DC Physical Address
            pData->rawData.ibsOpDcPhysicalAddress =  ibsOpRec.m_IbsDcPhysAd;
        }
    }

    // IbsDcLinAddrValid is true when address translation was successful.
    // Some macro-ops do not perform an address translation and use only
    // a physical address.
    bool useL2TranslationSize = false ;

    if (ibsOpRec.m_IbsDcLinAddrValid)
    {
        //Only add the address if we're not aggregating
        if ((!m_aggregating) && (0 != ibsOpRec.m_IbsDcLinAd))
        {
            //IBS DC Linear Address
            pData->rawData.ibsOpDcLinearAddress =  ibsOpRec.m_IbsDcLinAd;
        }

        if (! ibsOpRec.m_IbsDcL1tlbMiss)
        {
            // L1 DTLB hit -- This is the most frequent case
            key.event = EncodeEvent(DE_IBS_LS_DTLB_L1H, 0, false, false);
            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
        }
        else if (ibsOpRec.m_IbsDcL2tlbMiss)
        {
            // L1 DTLB miss, L2 DTLB miss
            key.event = EncodeEvent(DE_IBS_LS_DTLB_L1M_L2M, 0, false, false);
            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
        }
        else
        {
            // L1 DTLB miss, L2 DTLB hit
            key.event = EncodeEvent(DE_IBS_LS_DTLB_L1M_L2H, 0, false, false);
            pData->sampleData.insert(SampleDataMap::value_type(key, 1));
            useL2TranslationSize = true ;
        }

        if (useL2TranslationSize)
        {
            // L2 DTLB page translation
            if (ibsOpRec.m_IbsDcL2tlbHit2M)
            {
                // 2M L2 DTLB page translation
                key.event = EncodeEvent(DE_IBS_LS_L2_DTLB_2M, 0, false, false);
                pData->sampleData.insert(SampleDataMap::value_type(key, 1));
            }
            else  if (ibsOpRec.m_IbsDcL2tlbHit1G)
            {
                // 1G L2 DTLB page translation
                key.event = EncodeEvent(DE_IBS_LS_L2_DTLB_1G, 0, false, false);
                pData->sampleData.insert(SampleDataMap::value_type(key, 1));
            }
            else
            {
                // 4K L2 DTLB page translation
                key.event = EncodeEvent(DE_IBS_LS_L2_DTLB_4K, 0, false, false);
                pData->sampleData.insert(SampleDataMap::value_type(key, 1));
            }
        }
        else
        {
            // L1 DTLB page translation
            if (ibsOpRec.m_IbsDcL1tlbHit2M)
            {
                // 2M L1 DTLB page translation
                key.event = EncodeEvent(DE_IBS_LS_L1_DTLB_2M, 0, false, false);
                pData->sampleData.insert(SampleDataMap::value_type(key, 1));
            }
            else  if (ibsOpRec.m_IbsDcL1tlbHit1G)
            {
                // 1G L1 DTLB page translation
                key.event = EncodeEvent(DE_IBS_LS_L1_DTLB_1G, 0, false, false);
                pData->sampleData.insert(SampleDataMap::value_type(key, 1));
            }
            else
            {
                // This is the most common case, unfortunately
                key.event = EncodeEvent(DE_IBS_LS_L1_DTLB_4K, 0, false, false);
                pData->sampleData.insert(SampleDataMap::value_type(key, 1));
            }
        }
    }
}
#else
//TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS


void CpuProfileDataAccess::ProcessCssData()
{
#if SUPPORT_CSS
    size_t itemcnt = m_rawCss.stack.size();

    if (itemcnt == 0)
    {
        return;
    }

    wchar_t moduleName[OS_MAX_PATH + 1];
    wchar_t functionName [OS_MAX_PATH + 1];
    wchar_t jncName [OS_MAX_PATH + 1];
    wchar_t javaSrcFileName [OS_MAX_PATH + 1];

    gtUInt64 currip = m_rawCss.addr;

    memset(moduleName, 0, ((OS_MAX_PATH + 1)* sizeof(wchar_t)));
    memset(functionName, 0, ((OS_MAX_PATH + 1) * sizeof(wchar_t)));
    memset(jncName, 0, ((OS_MAX_PATH + 1) * sizeof(wchar_t)));
    memset(javaSrcFileName, 0, ((OS_MAX_PATH + 1) * sizeof(wchar_t)));

    TiModuleInfo modInfo;
    memset(&modInfo, 0, sizeof(TiModuleInfo));
    modInfo.processID = m_rawCss.pid;
    modInfo.sampleAddr = m_rawCss.addr;
    modInfo.cpuIndex = m_rawCss.core;
    modInfo.deltaTick = m_rawCss.timeStamp;
    modInfo.funNameSize = modInfo.jncNameSize = modInfo.namesize = OS_MAX_PATH;
    modInfo.pModulename = moduleName;
    modInfo.pFunctionName = functionName;
    modInfo.pJncName = jncName;
    modInfo.srcfilesize = OS_MAX_PATH;
    modInfo.pJavaSrcFileName = javaSrcFileName;
    modInfo.moduleType = evPEModule;
    modInfo.ModuleStartAddr = 0;

    QString callerModName, calleeModName;
    gtUInt64 callerAddr = 0;
    gtUInt64 calleeAddr = 0;

    HRESULT hr = fnGetModuleInfo(&modInfo);

    if ((hr != S_OK) || (evPEModule != modInfo.moduleType))
    {
        return;
    }

    if (!m_cssPids.contains(m_rawCss.pid))
    {
        m_cssPids.insert(m_rawCss.pid);
    }

    fnAddProcMod(m_rawCss.pid, modInfo.ModuleStartAddr, moduleName);

    if (wcslen(moduleName) > 0)
    {
        calleeModName = QString::fromWCharArray(moduleName);
    }

    calleeAddr = modInfo.ModuleStartAddr;

    bool tmpSelf = true;
    list <gtUInt64>::iterator iter;
    ModAnalysisMap::iterator modIter = m_CssMap.end();

    unsigned int i = 0;

    for (iter = m_rawCss.stack.begin(); iter != m_rawCss.stack.end(); iter++)
    {
        gtUInt64 cssitemRip = *iter;

        memset(moduleName, 0, ((OS_MAX_PATH + 1)* sizeof(wchar_t)));
        memset(functionName, 0, ((OS_MAX_PATH + 1) * sizeof(wchar_t)));
        memset(jncName, 0, ((OS_MAX_PATH + 1) * sizeof(wchar_t)));
        memset(javaSrcFileName, 0, ((OS_MAX_PATH + 1) * sizeof(wchar_t)));

        modInfo.processID = m_rawCss.pid;
        modInfo.sampleAddr = cssitemRip;
        modInfo.cpuIndex = m_rawCss.core;
        modInfo.deltaTick = m_rawCss.timeStamp;
        modInfo.funNameSize = modInfo.jncNameSize = modInfo.namesize = OS_MAX_PATH;
        modInfo.pModulename = moduleName;
        modInfo.pFunctionName = functionName;
        modInfo.pJncName = jncName;
        modInfo.srcfilesize = OS_MAX_PATH;
        modInfo.pJavaSrcFileName = javaSrcFileName;
        modInfo.moduleType = evPEModule;
        modInfo.ModuleStartAddr = 0;

        HRESULT listHr = fnGetModuleInfo(&modInfo);

        if (S_OK !=  listHr)
        {
            continue;
        }

        if (callerAddr != modInfo.ModuleStartAddr)
        {
            if (wcslen(moduleName) > 0)
            {
                callerModName = QString::fromWCharArray(moduleName);
                modIter = m_CssMap.find(callerModName);
            }

            callerAddr = modInfo.ModuleStartAddr;
        }

        CModAnalyzer* pAnalyzer = NULL;

        if (modIter != m_CssMap.end())
        {
            pAnalyzer = modIter.value();
        }
        else
        {
            CModAnalyzer* pNewAnalyzer = new CModAnalyzer;

            if (pNewAnalyzer)
            {
                pAnalyzer = pNewAnalyzer;

                if (S_OK != pNewAnalyzer->InitializeAnalysis(moduleName))
                {
                    delete pNewAnalyzer;
                    continue;
                }

                if (S_OK != pNewAnalyzer->AnalyzeMod(evCallSiteAnalysis))
                {
                    delete pNewAnalyzer;
                    continue;
                }

                m_CssMap.insert(callerModName, pNewAnalyzer);
            }
        }

        if (!pAnalyzer)
        {
            continue;
        }

        gtUInt32 imageOffset = (gtUInt32)(cssitemRip - modInfo.ModuleStartAddr);
        gtUInt32 instLen = 0;

        if (pAnalyzer->IsValidCallRet(imageOffset, &instLen, (calleeAddr == callerAddr) ? true : false))
        {
            fnAddProcMod(m_rawCss.pid, modInfo.ModuleStartAddr, moduleName);
            fnAddCallInfo(m_rawCss.pid, m_rawCss.tid, cssitemRip - instLen,
                          currip, tmpSelf);
            tmpSelf = false;
            // update current RIP
            currip = cssitemRip - instLen;
            calleeModName = callerModName;
            calleeAddr = callerAddr;
        }
        else
        {
            // check if caller irp is the base of stack frame - first function's start address.
            // Check if this is last css data;
            if ((++i == itemcnt) && tmpSelf)
            {
                if (callerModName == calleeModName)
                {
                    fnAddProcMod(m_rawCss.pid, modInfo.ModuleStartAddr,
                                 moduleName);
                    fnAddCallInfo(m_rawCss.pid, m_rawCss.tid, cssitemRip,
                                  currip, tmpSelf);
                }
            }
        }
    }

    return ;
#endif
}



HRESULT CpuProfileDataAccess::GetEventPathFromRegistryKey(wchar_t* path)
{
    gtString pathVar;
    osGetCurrentProcessEnvVariableValue(L"CPUPerfAPIDataPath", pathVar);
    wcscpy(path, pathVar.asCharArray());
    return S_OK;
}

HRESULT CpuProfileDataAccess::OpenEventsFile(int cpuFamily, int cpuModel)
{
    HRESULT ret;
    wchar_t eventPath[OS_MAX_PATH + 1] = {'\0'};

    if (S_OK != (ret = GetEventPathFromRegistryKey(eventPath)))
    {
        return ret;
    }

    EventEngine* pEvEngine = new EventEngine();

    if (NULL == pEvEngine)
    {
        return E_OUTOFMEMORY;
    }

    // Initialize event engine with events file installation dir
    if (!pEvEngine->Initialize(QString::fromWCharArray(eventPath)))
    {
        delete pEvEngine;
        return E_UNEXPECTED;
    }

    // Get event file path
    QString eventFile;
    eventFile = pEvEngine->GetEventFilePath(cpuFamily, cpuModel);

    // Initialize event file
    if (!m_eventsFile.Open(eventFile))
    {
        delete pEvEngine;
        return E_UNEXPECTED;
    }

    delete pEvEngine;

    return S_OK;
}


CPA_TIME CpuProfileDataAccess::SynchronizeCPA(const CPA_TIME& start, gtUInt64 deltaTick, int core, unsigned int extraMs)
{
    CPA_TIME ret_CPA_TIME;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    FILETIME filetime_start;
    CPA_TIME_to_FILETIME(start, &filetime_start);

    FILETIME ret_FILETIME;
    ret_FILETIME = fnSynchronize(filetime_start, deltaTick, core, extraMs);

    FILETIME_to_CPA_TIME(ret_FILETIME, &ret_CPA_TIME);
#else
    // unused
    (void)(start);
    (void)(deltaTick);
    (void)(core);
    (void)(extraMs);
    //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    return ret_CPA_TIME;
}


HRESULT CpuProfileDataAccess::SetupProfileReader(const wchar_t* pFile)
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    m_pPrdReader = new PrdReader();

    if (NULL == m_pPrdReader)
    {
        return E_OUTOFMEMORY;
    }

    if (S_OK != m_pPrdReader->Initialize(pFile))
    {
        delete m_pPrdReader;
        m_pPrdReader = NULL;
        return E_ACCESSDENIED;
    }

    wcscpy_s(m_inputFile, OS_MAX_PATH, pFile);
    return S_OK;
#else
    (void)(pFile); // unused
    //TODO: [Suravee]
    return E_FAIL;
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}


void CpuProfileDataAccess::CleanupProfileReader()
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (NULL != m_pPrdReader)
    {
        delete m_pPrdReader;
        m_pPrdReader = NULL;
    }

#else
    //TODO: [Suravee]
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

}
