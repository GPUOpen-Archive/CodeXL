//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JitTaskInfo.cpp
/// \brief JIT task information interface implementation.
///
//==================================================================================

// Suppress Qt header warnings
#pragma warning(push)
#pragma warning(disable : 4127 4718)
#include <QFile>
#include <QDir>
#pragma warning(pop)

#include "JitTaskInfo.h"
#include <cstdio>
#include <cassert>
#include <cwchar>
#include <climits>
#include <string>

#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTProfilingAgentsData/inc/JclReader.h>
#include <AMDTProfilingAgentsData/inc/JclWriter.h>


// Map of <process id, jcl*>
typedef gtMap<gtUInt64, JclWriter*> JclMap;


// This is the constructor of a class
JitTaskInfo::JitTaskInfo()
{
    // at least 1 CPU
    m_affinity = 1;

    m_jnc_counter = 0;
    m_JitModCount = 0;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    LARGE_INTEGER ctr;
    QueryPerformanceFrequency(&ctr);
    m_hrFreq = ctr.QuadPart;

    m_is32on64Sys = false;
#if AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE
    IsWow64Process(GetCurrentProcess(), &m_is32on64Sys);
#endif

#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}

// destructor
JitTaskInfo::~JitTaskInfo()
{
    Cleanup();
}

////////////////////////////////////////////////////////////////////////
// JitTaskInfo::Cleanup()
//  Clean up the process map, user mode module map, and kernel mode module
//  map. Clean up the time marks.
//
//  Param: void.
//  Redsc: void
//
void JitTaskInfo::Cleanup()
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    m_JitClrMap.clear();
#endif

    m_tiModMap.clear();
    m_JitInfoMap.clear();

    m_JitModCount = 0;
    m_jnc_counter = 0;

    m_bitnessMap.clear();
}

//TODO: Linux: Baskar:
//Given a high resolution tick, give the delta from start
TiTimeType JitTaskInfo::CalculateDeltaTick(gtUInt64 rawTick) const
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    TiTimeType result = 0;

    if (rawTick >= m_startHr)
    {
        result = (TiTimeType)(rawTick - m_startHr);
    }

    return result;
#else
    return rawTick;
#endif
}

bool CopyFilesToDestionDir(QString srcDirName, QString destDirName, QStringList nameFilter = QStringList())
{
    bool retValue = true;

    QDir srcDir(srcDirName);

    if (!srcDir.exists())
    {
        return false;
    }

    QDir destDir(destDirName);

    //Make the destination if it doesn't exist
    if ((!destDir.exists()) && (!destDir.mkpath(destDirName)))
    {
        return false;
    }

    QFileInfoList di_list = srcDir.entryInfoList(nameFilter, QDir::Files);

    for (int i = 0; i < di_list.size(); i++)
    {
        QString srcFile = srcDirName + QString(osFilePath::osPathSeparator) + di_list.at(i).fileName();
        QString destFile = destDirName + QString(osFilePath::osPathSeparator) + di_list.at(i).fileName();

        if (!QFile::exists(destFile))
        {
            bool ret = false;
            int retry = 0;

            do
            {
                ret = QFile::copy(srcFile, destFile);
            }
            while (!ret && (++retry < 5));
        }
    }

    return retValue;
}

//Note that the directory string should not end in a "\"
HRESULT JitTaskInfo::ReadJavaJitInformation(const wchar_t* pDirectory, const wchar_t* pSessionDir)
{
    HRESULT hr = S_OK;

    QString dirQstr = QString::fromWCharArray(pDirectory);

    //for each process id directory in the given directory
    QDir search_dir(dirQstr);

    if (search_dir.exists())
    {
        QFileInfoList di_list = search_dir.entryInfoList(QDir::AllDirs);

        //while there are directories left,
        for (int i = 0; i < di_list.size(); i++)
        {
            unsigned long tPid = di_list.at(i).fileName().toULong();

            if (0 == tPid)
            {
                continue;
            }

            JitRecordType jitRecordType;
            std::wstring javaApp;

            QString tempPath = dirQstr + QString(osFilePath::osPathSeparator) + di_list.at(i).fileName();
            // QString oldPath = tempPath;

            QString jcl_file = tempPath + QString(osFilePath::osPathSeparator) + di_list.at(i).fileName();
            jcl_file += ".jcl";

            if (!QFile::exists(jcl_file))
            {
                continue;
            }

            QString sess_pid = QString::fromWCharArray(pSessionDir) + QString(osFilePath::osPathSeparator) + di_list.at(i).fileName();
            QDir sess_pid_dir;
            sess_pid_dir.mkpath(sess_pid);

            // copy file from temp dir to session dir;
            CopyFilesToDestionDir(dirQstr + QString(osFilePath::osPathSeparator) + di_list.at(i).fileName(), sess_pid);

            //TODO: Baskar: remove the jcl file too..
            // QFile::remove(jcl_file);

            //read the copied jcl file
            jcl_file = sess_pid + QString(osFilePath::osPathSeparator) + di_list.at(i).fileName() + ".jcl";
            JclReader jclReader(jcl_file.toStdWString().c_str());

            if (!jclReader.ReadHeader(&javaApp))
            {
                jclReader.Close();
                continue;
            }

            bool bitness = jclReader.Is32Bit();
            gtUInt64 jitProcessID = di_list.at(i).fileName().toULongLong();
            m_bitnessMap[jitProcessID] = bitness;

            while (jclReader.ReadNextRecordType(&jitRecordType))
            {
                if (JIT_LOAD == jitRecordType)
                {
                    JitLoadRecord jitLoadBlock;

                    if (!jclReader.ReadLoadRecord(&jitLoadBlock))
                    {
                        break;
                    }

                    jitLoadBlock.loadTimestamp = CalculateDeltaTick(jitLoadBlock.loadTimestamp);

                    const ModuleKey t_modKey(tPid, jitLoadBlock.blockStartAddr, (TiTimeType)jitLoadBlock.loadTimestamp);

                    ModuleValue t_modValue(0, jitLoadBlock.blockEndAddr - jitLoadBlock.blockStartAddr,
                                           TI_TIMETYPE_MAX, jitLoadBlock.classFunctionName, evJavaModule);
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
                    t_modValue.instanceId = m_nextModInstanceId++;
#endif

                    m_tiModMap.insert(ModuleMap::value_type(t_modKey, t_modValue));
                    m_JitModCount++;

                    QString repPath = QString::fromWCharArray(jitLoadBlock.jncFileName);
                    //TODO: Baskar: before replacing remove the old file
                    // QFile::remove(QString::fromWCharArray(jit_load_block.jncFileName));

                    repPath.replace(tempPath, sess_pid);
                    wcscpy(jitLoadBlock.jncFileName, repPath.toStdWString().c_str());

                    const JitBlockValue tJitBlockValue(javaApp.c_str(), jitLoadBlock.jncFileName, jitLoadBlock.srcFileName);
                    m_JitInfoMap.insert(JitBlockInfoMap::value_type(t_modKey, tJitBlockValue));
                }
                else
                {
                    JitUnloadRecord jitUnloadBlock;

                    if (!jclReader.ReadUnLoadRecord(&jitUnloadBlock))
                    {
                        break;
                    }

                    jitUnloadBlock.unloadTimestamp = CalculateDeltaTick(jitUnloadBlock.unloadTimestamp);

                    //find old record module, and alter unload time.
                    ModuleMap::iterator it;

                    for (it = m_tiModMap.begin(); it != m_tiModMap.end(); ++it)
                    {
                        ModuleMap::value_type& item1 = *it;

                        //note that item.first is the key, and item.second is the value
                        if (item1.first.processId != tPid)
                        {
                            continue;
                        }

                        if (item1.first.moduleLoadAddr != jitUnloadBlock.blockStartAddr)
                        {
                            continue;
                        }

                        if (item1.first.moduleLoadTime >= jitUnloadBlock.unloadTimestamp)
                        {
                            continue;
                        }

                        item1.second.moduleUnloadTime = (TiTimeType)jitUnloadBlock.unloadTimestamp;
                    }
                }
            } //end while ReadNextRecordType

            // scope closes jcl file

            //TODO: Baskar: delete the tmp dir that contains the jcl/jnc files
            // QDir dir(oldPath);
            // if (dir.exists(oldPath)) {
            //  dir.rmdir(oldPath);
            // }
        }
    }

    return hr;
}

//Note that the directory string should not end in a "\"
HRESULT JitTaskInfo::WriteJavaJncFiles(/*[in]*/ const wchar_t* pDirectory)
{
    HRESULT hr = S_OK;

    QString qtstrDir = QString::fromWCharArray(pDirectory);

    if (qtstrDir.isEmpty())
    {
        qtstrDir = "."; //set as local directory
    }

    JclMap jclMap;

    for (const auto& mapItem : m_JitInfoMap)
    {
        //If the block wasn't used, go to the next one.
        if (!wcslen(mapItem.second.movedJncFileName))
        {
            //delete the jit file, if not used
            QFile::remove(QString::fromWCharArray(mapItem.second.jncFileName));
            continue;
        }

        // Make directory for Jnc/Jcl if not already exist
        QString procDir = qtstrDir + QString(osFilePath::osPathSeparator) + QString::number(mapItem.first.processId) + QString(osFilePath::osPathSeparator);

        auto jclIt = jclMap.find(mapItem.first.processId);

        //If there is not a jcl file for this jnc, add it
        if (jclMap.end() == jclIt)
        {
            QString newJcl;
            newJcl = procDir + QString::number(mapItem.first.processId) + ".jcl";

            JclWriter* pWriter = new JclWriter(newJcl.toStdWString().c_str(),
                                               mapItem.second.categoryName.asCharArray(), (int)mapItem.first.processId);

            if (NULL == pWriter)
            {
                return E_FAIL;
            }

            if (!pWriter->Initialize())
            {
                delete pWriter;
                return E_FAIL;
            }

            auto res = jclMap.emplace(mapItem.first.processId, pWriter);
            jclIt = res.first;
        }

        //write the jit block to the jcl file
        JitLoadRecord element;
        element.blockStartAddr = mapItem.first.moduleLoadAddr;
        element.blockEndAddr = element.blockStartAddr + m_tiModMap[mapItem.first].moduleSize;
        element.loadTimestamp = mapItem.first.moduleLoadTime;
        wcscpy(element.classFunctionName, m_tiModMap[mapItem.first].moduleName);
        wcscpy(element.jncFileName, mapItem.second.movedJncFileName);
        wcscpy(element.srcFileName, mapItem.second.srcFileName);

        if (jclIt->second != nullptr)
        {
            jclIt->second->WriteLoadRecord(&element);
        }

        //rename from temp->jncFile to temp->translatedJncFile
        QString new_jnc = procDir + QString(osFilePath::osPathSeparator) +
                          QString::fromWCharArray(mapItem.second.movedJncFileName);
        QString oldFile = QString::fromWCharArray(mapItem.second.jncFileName);
        QFile::rename(oldFile, new_jnc);

        //add unload info, if known
        if (TI_TIMETYPE_MAX != m_tiModMap[mapItem.first].moduleUnloadTime)
        {
            JitUnloadRecord block;
            block.blockStartAddr = mapItem.first.moduleLoadAddr;
            block.unloadTimestamp = m_tiModMap[mapItem.first].moduleUnloadTime;

            if (jclIt->second != nullptr)
            {
                jclIt->second->WriteUnloadRecord(&block);
            }
        }
    }

    //delete and close the jcl writer(s)
    for (auto& jcl : jclMap)
    {
        delete jcl.second;
    }

    jclMap.clear();

    return hr;
}

//Note that the directory string should not end in a "\"
HRESULT JitTaskInfo::ReadOldJitInfo(/* [in] */ const wchar_t* pDirectory)
{
    HRESULT hr = S_OK;

    //for each process id directory in the given directory
    QString jitDir = QString::fromWCharArray(pDirectory);
    QDir search_dir(jitDir);

    if (!search_dir.exists())
    {
        return E_ACCESSDENIED;
    }

    QFileInfoList di_list = search_dir.entryInfoList(QStringList("*.jcl"));

    //while there are jcl files left,
    for (int i = 0; i < di_list.size(); i++)
    {
        unsigned long tPid = di_list.at(i).fileName().section(".", 0, 0).toULong();

        if (0 == tPid)
        {
            continue;
        }

        JitRecordType jit_record_type;
        std::wstring javaApp;

        //read the jcl file
        QString jclFile = jitDir + QString(osFilePath::osPathSeparator) + di_list.at(i).fileName();
        JclReader jclReader(jclFile.toStdWString().c_str());

        if (!jclReader.ReadHeader(&javaApp))
        {
            continue;
        }

        while (jclReader.ReadNextRecordType(&jit_record_type))
        {
            if (JIT_LOAD == jit_record_type)
            {
                JitLoadRecord jit_block;
                jclReader.ReadLoadRecord(&jit_block);
                ModuleKey t_modKey(tPid, jit_block.blockStartAddr,
                                   (TiTimeType)jit_block.loadTimestamp);

                ModuleValue t_modValue(0, jit_block.blockEndAddr - jit_block.blockStartAddr,
                                       TI_TIMETYPE_MAX, jit_block.classFunctionName, evJavaModule);
                m_tiModMap.insert(ModuleMap::value_type(t_modKey, t_modValue));
                m_JitModCount++;

                JitBlockValue tJitBlockValue(javaApp.c_str(), jit_block.jncFileName, jit_block.srcFileName);
                tJitBlockValue.bJncMoved = true;
                wcscpy(tJitBlockValue.movedJncFileName, tJitBlockValue.jncFileName);
                wcscpy(tJitBlockValue.jncFileName, pDirectory);
                const wchar_t pathSeparator[] = { osFilePath::osPathSeparator, '\0' };
                wcscat(tJitBlockValue.jncFileName, pathSeparator);
                wcscat(tJitBlockValue.jncFileName, tJitBlockValue.movedJncFileName);
                m_JitInfoMap.insert(JitBlockInfoMap::value_type(t_modKey, tJitBlockValue));
            }
            else
            {
                JitUnloadRecord jit_unload_block;
                jclReader.ReadUnLoadRecord(&jit_unload_block);

                //find old record module, and alter unload time.
                for (ModuleMap::iterator it = m_tiModMap.begin(), itEnd = m_tiModMap.end(); it != itEnd; ++it)
                {
                    ModuleMap::value_type& item1 = *it;

                    //note that item.first is the key, and item.second is the value
                    if (item1.first.processId != tPid)
                    {
                        continue;
                    }

                    if (item1.first.moduleLoadAddr != jit_unload_block.blockStartAddr)
                    {
                        continue;
                    }

                    if (item1.first.moduleLoadTime >= jit_unload_block.unloadTimestamp)
                    {
                        continue;
                    }

                    if (TI_TIMETYPE_MAX != item1.second.moduleUnloadTime)
                    {
                        continue;
                    }

                    item1.second.moduleUnloadTime = (TiTimeType)jit_unload_block.unloadTimestamp;
                }
            }
        }
    }

    return hr;
}

bool JitTaskInfo::GetUserJitModInfo(TiModuleInfo* pModInfo, TiTimeType systemTimeTick, ModuleMap::value_type& item)
{
    (void)(systemTimeTick); // unused

    pModInfo->ModuleStartAddr = item.first.moduleLoadAddr;
    pModInfo->Modulesize = 0;
    pModInfo->FunStartAddr = item.first.moduleLoadAddr;
    pModInfo->instanceId = item.second.instanceId;

    // The module name is the JIT function name.
    wcsncpy(pModInfo->pFunctionName, item.second.moduleName, pModInfo->funNameSize);

    JitBlockValue* pJitBlock = NULL;

    ModuleKey modKey(pModInfo->processID, item.first.moduleLoadAddr, item.first.moduleLoadTime);
    JitBlockInfoMap::iterator jitIter = m_JitInfoMap.find(modKey);

    if (m_JitInfoMap.end() != jitIter)
    {
        pJitBlock = &jitIter->second;
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    else
    {
        jitIter = m_JitClrMap.find(modKey);

        if (m_JitClrMap.end() != jitIter)
        {
            pJitBlock = &jitIter->second;
        }
    }

#endif

    bool found = (NULL != pJitBlock);

    if (found)
    {
        wcsncpy(pModInfo->pModulename, pJitBlock->categoryName.asCharArray(), pModInfo->namesize);
        wcsncpy(pModInfo->pJavaSrcFileName, pJitBlock->srcFileName, pModInfo->srcfilesize);

        pModInfo->Modulesize = item.second.moduleSize;

        if (!item.second.bNameConverted)
        {
            pJitBlock->jncIndex = m_jnc_counter;

            wchar_t tmpStr[64];
            swprintf(tmpStr, 64,  L"jnc_%d.jnc", m_jnc_counter++);

#if defined(TI_MULTITHREADED)
            osCriticalSectionLocker lock(m_TIMutexJIT);

            if (!item.second.bNameConverted)
#endif
            {
                wcscpy(pJitBlock->movedJncFileName, tmpStr);

                item.second.bNameConverted = true;
                m_tiModMap[item.first] = item.second;
            }
        }

        wcsncpy(pModInfo->pJncName, pJitBlock->movedJncFileName, pModInfo->jncNameSize);
    }

    return found;
}

HRESULT JitTaskInfo::GetUserModInfo(TiModuleInfo* pModInfo, TiTimeType systemTimeTick)
{
    HRESULT hr = S_FALSE;

    // this is user space, check module map.
    ModuleMap::iterator i = m_tiModMap.lower_bound(ModuleKey(pModInfo->processID, pModInfo->sampleAddr, TI_TIMETYPE_MAX));

    for (ModuleMap::iterator iEnd = m_tiModMap.end(); i != iEnd; ++i)
    {
        ModuleMap::value_type& item = *i;

        // different process
        if (item.first.processId != pModInfo->processID)
        {
            break;
        }

        // since the module map is sorted by the process id, module address and time.
        // if module load address is greater than sample address, we don't need
        // go farther.
        if ((item.first.moduleLoadAddr + item.second.moduleSize) < pModInfo->sampleAddr)
        {
            break;
        }

        //TODO: Linux: Baskar: timing between PERF and Agent
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        // if load time is not in module load frame, try next module
        if (0 != systemTimeTick && (item.first.moduleLoadTime >= systemTimeTick || item.second.moduleUnloadTime < systemTimeTick))
        {
            continue;
        }

#endif

        pModInfo->moduleType = item.second.moduleType;

        // Baskar: We will only handle evJavaModule here
        if (evJavaModule == item.second.moduleType)
        {
            if (GetUserJitModInfo(pModInfo, systemTimeTick, item))
            {
                hr = S_OK;
                break;
            }
        } // Java Module
    } // module iter

    return hr;
}

//////////////////////////////////////////////////////////////////////////////////////
// JitTaskInfo::GetModuleInfo( )
//  Get module info for a given sample record.
//
//  Param: [in]     gtUInt64    processID,          | process id
//  Param: [in]     unsigned    CSvalue,            | CS value
//  Param: [in]     gtUInt64    sampleAddr,         | sample address
//  Param: [in]     unsigned    cpuIndex,           | cpud number
//  Param: [in/out] gtUInt64    timestamp,          | sample time stamp
//  Param: [out]    gtUInt64    *pModuleStartAddr,  | module start address
//  Param: [out]    gtUInt64    *pModulesize,       | module image size
//  Param: [out]    char        *pModulename,       | module full name
//  Param: [in]     unsigned    namesize            | length of module name string
//
//Note that it will convert the timestamp to the system mS count
HRESULT JitTaskInfo::GetModuleInfo(TiModuleInfo* pModInfo)
{
    HRESULT hr = S_FALSE;

    if (NULL == pModInfo)
    {
        return hr;
    }

#if 0

    // check the cpu index
    if (static_cast<int>(pModInfo->cpuIndex) >= m_affinity)
    {
        return hr;
    }

#endif // 0

    TiTimeType t_time = static_cast<TiTimeType>(pModInfo->deltaTick);
    pModInfo->kernel = false;
    hr = GetUserModInfo(pModInfo, t_time);

    return hr;
}


bool JitTaskInfo::IsJitProcess32Bit(gtUInt64 jitProcID) const
{
    bool bRet = false;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) && (AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE)
    //Should return false by default for a 64-bit system, and true on 32-bit
    bRet = !m_is32on64Sys;
#endif

    BitnessMap::const_iterator bitIter = m_bitnessMap.find(jitProcID);

    if (bitIter != m_bitnessMap.end())
    {
        bRet = bitIter->second;
    }

    return bRet;
}

void JitTaskInfo::GetJavaJitBlockInfo(gtVector<std::tuple<gtUInt32, gtString, gtUInt32, gtUInt64, gtUInt64, gtUInt64>>& jitBlockInfo)
{
    for (const auto& it : m_tiModMap)
    {
        auto jitIt = m_JitInfoMap.find(it.first);

        if (m_JitInfoMap.end() != jitIt && wcslen(jitIt->second.movedJncFileName) != 0)
        {
            gtUInt32 jncIndex = jitIt->second.jncIndex;
            gtString moduleName = jitIt->second.categoryName;
            gtUInt32 instanceId = it.second.instanceId;
            gtUInt64 pid = it.first.processId;
            gtUInt64 loadAddr = it.first.moduleLoadAddr;
            gtUInt64 size = it.second.moduleSize;

            // <jncIdx, moduleId, instanceId, pid, loadAddr>
            jitBlockInfo.emplace_back(jncIndex, moduleName, instanceId, pid, loadAddr, size);
        }
    }
}

void JitTaskInfo::GetJavaJncInfo(gtVector<std::tuple<gtUInt32, gtString, gtString>>& jncInfoList)
{
    for (const auto& it : m_JitInfoMap)
    {
        if (wcslen(it.second.movedJncFileName) != 0)
        {
            // <jncIdx, srcFilePath, jncFilePath>
            gtString jncFilePath(std::to_wstring(it.first.processId).c_str());
            jncFilePath.append(osFilePath::osPathSeparator);
            jncFilePath.append(it.second.movedJncFileName);
            jncInfoList.emplace_back(it.second.jncIndex, it.second.srcFileName, jncFilePath);
        }
    }
}
