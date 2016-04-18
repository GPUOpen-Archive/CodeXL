//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpTranslate.cpp
///
//==================================================================================

// Project headers
#include <tpInternalDataTypes.h>
#include <tpTranslate.h>
#include <tpTranslateDataTypes.h>

#include <AMDTOSWrappers/Include/osFilePath.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <tchar.h>
#endif

//
// Public Member functions
//

tpTranslate::tpTranslate() : m_verbose(false),
    m_userMode(false),
    m_isPassOneCompleted(false),
    m_allCSRecordsProcessed(false),
    m_handleAllPids(true)
{
    m_debugLogFP = stderr;
    m_numberOfProcessors = 0;

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    m_kernelImage.m_pExe = nullptr;
    m_isKernelImageAdded = false;
#endif

};

tpTranslate::~tpTranslate()
{
    auto processIt = m_pidProcessDataMap.begin();

    for (; processIt != m_pidProcessDataMap.end(); processIt++)
    {
        auto imageIt = (*processIt).second.m_imageMap.begin();

        for (; imageIt != (*processIt).second.m_imageMap.end(); imageIt++)
        {
            if (nullptr != (*imageIt).second.m_pExe)
            {
                // Either delete or close.. delete calls Close internally..
                // (*imageIt).second.m_pExe->Close();
                delete(*imageIt).second.m_pExe;
                (*imageIt).second.m_pExe = nullptr;
            }
        }
    }

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

    if (nullptr != m_kernelImage.m_pExe)
    {
        delete m_kernelImage.m_pExe;
        m_kernelImage.m_pExe = nullptr;
    }

#endif
}

AMDTResult tpTranslate::Initialize(const char* pLogFile, char* pDebugLogFile)
{
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;

    if (NULL != pLogFile)
    {
        mbstowcs(m_logFile, pLogFile, TP_MAX_ETL_PATH_LEN - 1);
        // TODO - should i check the validity of the path?

        retVal = AMDT_STATUS_OK;
    }

    if (NULL == pDebugLogFile)
    {
        // TODO.. open the file and set m_debugLogFP
        m_debugLogFP = stderr;
    }

    return retVal;
}

bool tpTranslate::IsValidProcess(AMDTProcessId pid) const
{
    return (m_pidProcessDataMap.find(pid) != m_pidProcessDataMap.end()) ? true : false;
}

bool tpTranslate::IsInterestingPid(AMDTProcessId pid)
{
    bool retVal = m_handleAllPids;

    //if (!retVal)
    //{
    //    auto it = std::find(m_interestingPidsList.begin(), m_interestingPidsList.end(), pid);
    //    retVal = (it != m_interestingPidsList.end()) ? true : false;
    //}

    if (!retVal)
    {
        retVal = (m_interestingPidsMap.end() != m_interestingPidsMap.find(pid)) ? true : false;
    }

    return retVal;
}

bool tpTranslate::AddInterestingPid(AMDTProcessId pid)
{
    bool retVal = false;

    //auto it = std::find(m_interestingPidsList.begin(), m_interestingPidsList.end(), pid);

    //if (it == m_interestingPidsList.end())
    //{
    //    m_interestingPidsList.push_back(pid);
    //    retVal = true;
    //}

    auto it = m_interestingPidsMap.find(pid);

    if (it == m_interestingPidsMap.end())
    {
        m_interestingPidsMap.insert(TPPidMap::value_type(pid, pid));
    }

    return retVal;
}

AMDTProcessId tpTranslate::GetPid(AMDTThreadId tid)
{
    AMDTProcessId pid = TP_ALL_PIDS;

    auto it = m_tidThreadDataMap.find(tid);

    if (it != m_tidThreadDataMap.end())
    {
        pid = (*it).second.m_processId;
    }

    return pid;
}

bool tpTranslate::AddInterestingTid(AMDTThreadId tid, AMDTProcessId pid)
{
    bool retVal = false;

    auto it = m_interestingTidsMap.find(tid);

    if (it == m_interestingTidsMap.end())
    {
        m_interestingTidsMap.insert(TPTidPidMap::value_type(tid, pid));
    }

    return retVal;
}

bool tpTranslate::IsInterestingTid(AMDTThreadId tid)
{
    bool retVal = m_handleAllPids;

    if (!retVal)
    {
        retVal = (m_interestingTidsMap.end() != m_interestingTidsMap.find(tid)) ? true : false;
    }

    return retVal;
}


bool tpTranslate::IsValidThread(AMDTThreadId tid) const
{
    return (m_tidThreadDataMap.find(tid) != m_tidThreadDataMap.end()) ? true : false;
}

size_t tpTranslate::GetNumberOfThreads(AMDTProcessId pid) const
{
    size_t size = 0;
    auto processIt = m_pidProcessDataMap.find(pid);

    if (processIt != m_pidProcessDataMap.end())
    {
        size = (*processIt).second.m_threadsList.size();
    }

    return size;
}



//
//  Data Retrievers
//

AMDTResult tpTranslate::GetProcessIds(AMDTUInt32 size, AMDTProcessId*& pProcesses)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    AMDTUInt32 nbrProcesses = m_pidProcessDataMap.size();

    if (nbrProcesses && (NULL != pProcesses))
    {
        auto processIt = m_pidProcessDataMap.begin();

        for (AMDTUInt32 i = 0; processIt != m_pidProcessDataMap.end() && i < size; processIt++, i++)
        {
            pProcesses[i] = (*processIt).second.m_pid;
        }
    }

    retVal = (nbrProcesses) ? AMDT_STATUS_OK : AMDT_ERROR_NODATA;

    return retVal;
}

AMDTResult tpTranslate::GetProcessData(AMDTProcessId pid, AMDTProcessData& processData)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    if (IsValidProcess(pid))
    {
        auto processIt = m_pidProcessDataMap.find(pid);

        TPProcessData& data = (*processIt).second;

        processData.m_pid = data.m_pid;
        processData.m_pCommand = data.m_executableName.asASCIICharArray();
        processData.m_processCreateTS = data.m_processCreateTS;
        processData.m_processTerminateTS = data.m_processTerminateTS;

        AMDTUInt32 nbrChilds = (*processIt).second.m_childProcessesList.size();
        processData.m_nbrChildProcesses = nbrChilds;

        if (nbrChilds > 0)
        {
            processData.m_pChildProcesses = (AMDTThreadId*)malloc(sizeof(AMDTProcessId) * nbrChilds);

            auto childProcessIt = (*processIt).second.m_childProcessesList.begin();

            for (AMDTUInt32 i = 0; childProcessIt != (*processIt).second.m_childProcessesList.end() && i < nbrChilds;
                 childProcessIt++, i++)
            {
                processData.m_pChildProcesses[i] = (*childProcessIt);
            }

            retVal = GetProcessIds(nbrChilds, processData.m_pChildProcesses);
        }

        AMDTUInt32 nbrThreads = (*processIt).second.m_threadsList.size();
        processData.m_nbrThreads = nbrThreads;

        if (nbrThreads > 0)
        {
            processData.m_pThreads = (AMDTThreadId*)malloc(sizeof(AMDTThreadId) * processData.m_nbrThreads);
            retVal = GetThreadIds(processData.m_pid, processData.m_nbrThreads, processData.m_pThreads);
        }
    }
    else
    {
        retVal = AMDT_ERROR_INVALIDDATA;
    }

    return retVal;
}

AMDTResult tpTranslate::GetThreadIds(AMDTProcessId pid, AMDTUInt32 size, AMDTThreadId*& pThreads)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    AMDTUInt32 nbrThreads = m_tidThreadDataMap.size();

    if (nbrThreads && (NULL != pThreads))
    {
        if (TP_ALL_PIDS == pid)
        {
            // get all the threads of all the processes
            auto threadIt = m_tidThreadDataMap.begin();

            for (AMDTUInt32 i = 0; threadIt != m_tidThreadDataMap.end() && i < size; threadIt++, i++)
            {
                pThreads[i] = (*threadIt).second.m_threadId;
            }
        }
        else
        {
            if (IsValidProcess(pid))
            {
                auto processIt = m_pidProcessDataMap.find(pid);
                TPProcessData& data = (*processIt).second;

                auto threadIt = data.m_threadsList.begin();

                for (AMDTUInt32 i = 0; threadIt != data.m_threadsList.end() && i < size; threadIt++, i++)
                {
                    pThreads[i] = (*threadIt);
                }
            }
        }
    }

    retVal = (nbrThreads) ? AMDT_STATUS_OK : AMDT_ERROR_NODATA;

    return retVal;
}

AMDTResult tpTranslate::GetThreadData(AMDTThreadId tid, AMDTThreadData& threadData)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    if (IsValidThread(tid))
    {
        auto threadIt = m_tidThreadDataMap.find(tid);
        TPThreadData& data = (*threadIt).second;

        threadData.m_processId = data.m_processId;
        threadData.m_threadId = data.m_threadId;
        threadData.m_pThreadName = NULL; // FIXME
        threadData.m_affinity = data.m_affinity;
        threadData.m_threadCreateTS = data.m_threadCreateTS;
        threadData.m_threadTerminateTS = data.m_threadTerminateTS;
        threadData.m_nbrOfContextSwitches = data.m_nbrOfContextSwitches;
        threadData.m_nbrOfCoreSwitches = data.m_nbrOfCoreSwitches;
        threadData.m_totalExeTime = data.m_totalExeTime;
        threadData.m_totalWaitTime = data.m_totalWaitTime;
        threadData.m_totalTransitionTime = data.m_totalTransitionTime;
    }
    else
    {
        retVal = AMDT_ERROR_INVALIDDATA;
    }

    return retVal;
}

AMDTResult tpTranslate::GetThreadSampleData(AMDTThreadId       tid,
                                            AMDTUInt32*        pNbrRecords,
                                            AMDTThreadSample** ppThreadSampleData)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    if ((NULL != pNbrRecords) && (NULL != ppThreadSampleData))
    {
        if (IsValidThread(tid))
        {
            auto threadIt = m_tidThreadDataMap.find(tid);
            TPThreadData& data = (*threadIt).second;

            AMDTThreadSample* pSamples = NULL;
            AMDTUInt32 nbrSamples = data.m_sampleList.size();

            if (nbrSamples)
            {
                pSamples = (AMDTThreadSample*)malloc(sizeof(AMDTThreadSample) * nbrSamples);

                if (NULL != pSamples)
                {
                    AMDTUInt32 i = 0;

                    for (TPThreadSampleList::const_iterator it = data.m_sampleList.begin(), itEnd = data.m_sampleList.end();
                         it != itEnd; ++it, ++i)
                    {
                        pSamples[i].m_processId = data.m_processId;
                        pSamples[i].m_threadId = data.m_threadId;
                        pSamples[i].m_coreId = (*it).m_coreId;
                        pSamples[i].m_endTS = (*it).m_endTS;
                        pSamples[i].m_startTS = (*it).m_startTS;
                        pSamples[i].m_execTime = (*it).m_execTime;
                        pSamples[i].m_waitTime = (*it).m_waitTime;
                        pSamples[i].m_transitionTime = (*it).m_transitionTime;
                        pSamples[i].m_nbrStackFrames = (*it).m_nbrStackFrames;

                        //FIXME: should i alloc memory and copy.. or point to internal struct..
                        pSamples[i].m_pStackFrames = (*it).m_pStackFrames;

                        pSamples[i].m_threadState = (*it).m_threadState;
                        pSamples[i].m_waitReason = (*it).m_waitReason;
                        pSamples[i].m_waitMode = (*it).m_waitMode;
                    }

                    *pNbrRecords = nbrSamples;
                    *ppThreadSampleData = pSamples;
                }
                else
                {
                    retVal = AMDT_ERROR_OUTOFMEMORY;
                }
            }
        }
        else
        {
            retVal = AMDT_ERROR_INVALIDDATA;
        }
    }
    else
    {
        retVal = AMDT_ERROR_INVALIDARG;
    }

    return retVal;
} // GetThreadSampleData


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
bool tpTranslate::ConvertDeviceNameToFileName(const gtString& deviceName, gtString& fileName)
{
    bool retVal = false;

    // Get the size of the buffer needed for holding the logical drives mapping:
    int buffSize = GetLogicalDriveStrings(0, NULL);

    if (buffSize > 0)
    {
        // Allocate space for the logical drives map:
        wchar_t* pLogicalDriversMap = new wchar_t[buffSize + 5];
        pLogicalDriversMap[0] = NULL;

        // Get the logical drives map:
        // (See GetLogicalDriveStrings for this map structure)
        if (GetLogicalDriveStrings(buffSize + 4, pLogicalDriversMap))
        {
            wchar_t currentDriveString[3] = L" :";
            wchar_t* pMapCurrentPosition = pLogicalDriversMap;

            // Iterate the available drives mappings:
            do
            {
                // Copy the current drive letter into the drive string template:
                currentDriveString[0] = *pMapCurrentPosition;

                // Get the name of the DOS device that corresponds to this drive letter:
                wchar_t currentDeviceDosName[MAX_PATH];

                if (QueryDosDevice(currentDriveString, currentDeviceDosName, MAX_PATH))
                {
                    // Did we find the device that the file name contains:
                    size_t currentDeviceDosNameSize = wcslen(currentDeviceDosName);
                    retVal = (_tcsnicmp(deviceName.asCharArray(), currentDeviceDosName, currentDeviceDosNameSize) == 0);

                    if (retVal)
                    {
                        // Reconstruct the file name (replace the device name with the dos
                        // drive string)
                        wchar_t newFileName[MAX_PATH];
                        size_t currentDeviceDosNameLen = wcslen(currentDeviceDosName);
                        swprintf_s(newFileName, L"%ls%ls", currentDriveString, deviceName.asCharArray() + currentDeviceDosNameLen);
                        fileName = newFileName;
                    }
                }

                // Go to the next device in the device map (look for the next NULL character).
                while (*pMapCurrentPosition++) {};

            }
            while (!retVal && *pMapCurrentPosition);
        }

        // Clean up:
        delete[] pLogicalDriversMap;
    }

    return retVal;
}
#endif // AMDT_WINDOWS_OS

AMDTResult tpTranslate::SetSymbolSearchPath(const char* pSearchPath,
                                            const char* pServerList,
                                            const char* pCachePath)
{
    m_searchPath.fromASCIIString(pSearchPath);
    m_serverList.fromASCIIString(pServerList);
    m_cachePath.fromASCIIString(pCachePath);

    return AMDT_STATUS_OK;
}

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

static int hex(char ch)
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

static int hex2u64(const char* ptr, gtUInt64* long_val)
{
    const char* p = ptr;
    *long_val = 0;

    while (*p)
    {
        const int hex_val = hex(*p);

        if (hex_val < 0)
        {
            break;
        }

        *long_val = (*long_val << 4) | hex_val;
        p++;
    }

    return p - ptr;
}

// TODO: add the logic to provide size
AMDTResult tpTranslate::FindKernelSymbol(const char* pKernelSymFile,
                                         char**      ppSymbol,
                                         gtUInt64    startAddress)
{
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;
    FILE*  pFile = nullptr;
    char*  pLine = nullptr;

    char prevFuncName[KERNEL_SYMBOL_LEN];
    //gtUInt64 prevFuncAddr = 0x0;

    if ((nullptr != pKernelSymFile) && (nullptr != ppSymbol))
    {
        retVal = AMDT_ERROR_FAIL;

        if ((pFile = fopen(pKernelSymFile, "r")) != NULL)
        {
            // read the kernel symbol file /proc/kallsyms
            while (!feof(pFile))
            {
                gtUInt64  start;
                int       lineLen = 0;
                int       len;
                size_t    n;
                char      symbolType;
                char*     pKernelSymbol = nullptr;

                lineLen = getline(&pLine, &n, pFile);

                if (lineLen < 0)
                {
                    break;
                }

                pLine[--lineLen] = '\0'; /* \n */
                len = hex2u64(pLine, &start);
                len++;

                if (len + 2 >= lineLen)
                {
                    continue;
                }

                symbolType = toupper(pLine[len]);
                len += 2;
                pKernelSymbol = &pLine[len];

                // symbol length
                len = lineLen - len;

                if (len < KERNEL_SYMBOL_LEN)
                {
                    if (symbolType == 'T' || symbolType == 'W')
                    {
                        if (startAddress > start)
                        {
                            //prevFuncAddr = start;
                            strncpy(prevFuncName, pKernelSymbol, KERNEL_SYMBOL_LEN - 1);
                        }
                        else
                        {
                            *ppSymbol = strdup(prevFuncName);
                            break;
                        }
                    }
                    else
                    {
                        continue;
                    }
                }
            } // while loop
        } // if file open
    } // valid args

    if (nullptr != pLine)
    {
        free(pLine);
    }

    if (nullptr != pFile)
    {
        fclose(pFile);
    }

    return retVal;
}
#endif


AMDTResult tpTranslate::GetFunctionName(AMDTProcessId pid,
                                        AMDTUInt64 pc,
                                        char** ppFuncName)
{
    AMDTResult retVal = AMDT_STATUS_OK;
    gtString funcName;

    auto processIt = m_pidProcessDataMap.find(pid);

    if (processIt != m_pidProcessDataMap.end())
    {
        TPProcessData& processData = (*processIt).second;

        auto modIt = processData.m_imageMap.lower_bound(pc);

        for (; modIt != processData.m_imageMap.end(); modIt--)
        {
            if (((*modIt).second.m_imageBase <= pc)
                && (((*modIt).second.m_imageBase + (*modIt).second.m_imageSize) >= pc))
            {
                break;
            }
        }

        if (modIt != processData.m_imageMap.end())
        {
            TPImageData& module = (*modIt).second;

            if (nullptr == module.m_pExe)
            {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                gtString fileName;
                gtString deviceName = module.m_fileName;
                ConvertDeviceNameToFileName(deviceName, fileName);

                ExecutableFile* pExe = new PeFile(fileName.asCharArray());
#else
                ExecutableFile* pExe = new ElfFile(module.m_fileName.asCharArray());
#endif

                if (nullptr != pExe)
                {
                    if (pExe->Open(module.m_imageBase))
                    {
                        const wchar_t* pSearchPath = m_searchPath.asCharArray();
                        const wchar_t* pServerList = m_serverList.asCharArray();
                        const wchar_t* pCachePath = m_cachePath.asCharArray();

                        if (pExe->InitializeSymbolEngine(pSearchPath, pServerList, pCachePath))
                        {
                            module.m_pExe = pExe;
                        }
                        else
                        {
                            pExe->Close();
                            delete pExe;
                        }
                    }
                }
            }

            if (nullptr != module.m_pExe)
            {
                gtString moduleBaseName;
                osFilePath path(module.m_fileName);
                path.getFileName(moduleBaseName);
                funcName.append(moduleBaseName);
                funcName.append(L"::");

                ExecutableFile* pExe = module.m_pExe;

                const FunctionSymbolInfo* pFuncInfo = (nullptr != pExe && nullptr != pExe->GetSymbolEngine()) ?
                                                      pExe->GetSymbolEngine()->LookupFunction(pExe->VaToRva((gtRVAddr)pc), nullptr, false) : nullptr;

                if (nullptr != pFuncInfo && nullptr != pFuncInfo->m_pName)
                {
                    funcName.append(pFuncInfo->m_pName);
                }
                else
                {
                    funcName.appendFormattedString(L"0x%llx", pc);
                }
            }
            else
            {
                funcName.append(L"UnknownMod::");
                funcName.appendFormattedString(L"0x%llx", pc);
            }
        }
        else
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            funcName.appendFormattedString(L"0x%llx", pc);
#else
            char* pKernelSymbol = nullptr;
            FindKernelSymbol("/proc/kallsyms",
                             &pKernelSymbol,
                             pc);

            if (nullptr != pKernelSymbol)
            {
                funcName.fromASCIIString(pKernelSymbol);
            }
            else
            {
                funcName.append(L"Kernel::");
                funcName.appendFormattedString(L"0x%llx", pc);
            }

#endif
        }
    }

    if (NULL != ppFuncName)
    {
        *ppFuncName = strdup(funcName.asASCIICharArray());
    }

    return retVal;
} // GetFunctionName


//
//  Helper Member functions
//

AMDTResult tpTranslate::AddInfoEvent(ThreadProfileEventGeneric& genericRec)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    m_numberOfProcessors = genericRec.m_numberOfProcessors;

    return retVal;
}

AMDTResult tpTranslate::AddProcessCreateEvent(ThreadProfileEventProcess& processRec)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    AMDTProcessId pid = processRec.m_processId;

    auto it = m_pidProcessDataMap.find(pid);

    if (it == m_pidProcessDataMap.end())
    {
        TPProcessData processData;

        processData.m_pid                = pid;
        processData.m_stillActive        = processRec.m_isActive;
        processData.m_processCreateTS    = processRec.m_timeStamp;
        processData.m_exitStatus         = processRec.m_exitStatus;
        processData.m_processTerminateTS = 0;

        m_pidProcessDataMap.insert(TPPidProcessDataMap::value_type(pid, processData));

        // if the parent process is found, add this pid to the parent's child processes list
        // !!   For process 0, parentPid is also 0; !!
        AMDTProcessId parentPid = processRec.m_parentId;

        if (parentPid != pid)
        {
            auto parentIt = m_pidProcessDataMap.find(parentPid);

            if (parentIt != m_pidProcessDataMap.end())
            {
                // (*parentIt).second.m_childProcessesList.push_back(pid);
                (*parentIt).second.AddChildProcess(pid);

                // If the parent is an interesting process, add the child too
                if (IsInterestingPid(parentPid))
                {
                    AddInterestingPid(pid);
                }
            }
        }
    }
    else
    {
        fprintf(stderr, "Entry for pid(%u) created already\n", pid);
    }

    return retVal;
} // AddProcessCreateEvent


AMDTResult tpTranslate::AddProcessStartEvent(ThreadProfileEventProcess& processRec)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    // fprintf(stderr, "in AddProcessStartEvent %u\n", processRec.m_parentId);

    AMDTProcessId pid = processRec.m_processId;

    auto it = m_pidProcessDataMap.find(pid);

    if (it == m_pidProcessDataMap.end())
    {
        TPProcessData processData;

        processData.m_pid                = pid;
        processData.m_stillActive        = processRec.m_isActive;
        processData.m_processCreateTS    = processRec.m_timeStamp;
        processData.m_exitStatus         = processRec.m_exitStatus;
        processData.m_processTerminateTS = 0;
        processData.m_executableName     = processRec.m_imageFileName;

        m_pidProcessDataMap.insert(TPPidProcessDataMap::value_type(pid, processData));

        // if the parent process is found, add this pid to the parent's child processes list
        // !!   For process 0, parentPid is also 0; !!
        AMDTProcessId parentPid = processRec.m_parentId;

        if (parentPid != pid)
        {
            auto parentIt = m_pidProcessDataMap.find(parentPid);

            if (parentIt != m_pidProcessDataMap.end())
            {
                // (*parentIt).second.m_childProcessesList.push_back(pid);
                (*parentIt).second.AddChildProcess(pid);

                // If the parent is an interesting process, add the child too
                if (IsInterestingPid(parentPid))
                {
                    AddInterestingPid(pid);
                }
            }
        }
    }
    else
    {
        // On Windows, we will get there when we handle the Process-DCEnd event
        // On Linux, we will get here when we handle the COMM event record.

        (*it).second.m_stillActive = processRec.m_isActive;
        (*it).second.m_exitStatus  = processRec.m_exitStatus;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        (*it).second.m_commandLine = processRec.m_commandLine;
#else
        // TODO: if executableName is not NULL, create a seperate list..
        // we will one FORK and 2 COMM events for the same PDI: For Ex: when CodeXLCpuProfiler
        // launches target-app we get a FORK and a COMM for CodeXLCpuProfiler and when it
        // execs the target-app, we get once more COMM event for the target-app
        //
        // FIXME: there will also be a leak here
        (*it).second.m_executableName = processRec.m_imageFileName;
#endif

        //fprintf(stderr, "found pid(%u)\n", pid);
    }

    return retVal;
} // AddProcessStartEvent


AMDTResult tpTranslate::AddProcessStopEvent(ThreadProfileEventProcess& processRec)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    // fprintf(stderr, "in AddProcessStopEvent %u\n", processRec.m_parentId);

    AMDTProcessId pid = processRec.m_processId;

    auto it = m_pidProcessDataMap.find(pid);

    if (it != m_pidProcessDataMap.end())
    {

        (*it).second.m_processTerminateTS = processRec.m_timeStamp;
        (*it).second.m_exitStatus         = processRec.m_exitStatus;
        (*it).second.m_stillActive        = processRec.m_isActive;
        (*it).second.m_commandLine        = processRec.m_commandLine;
    }
    else
    {
        retVal = AMDT_ERROR_FAIL;
    }

    return retVal;
} // AddProcessStopEvent


AMDTResult tpTranslate::AddImageLoadEvent(ThreadProfileEventImage& imageRec)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    AMDTProcessId pid = imageRec.m_processId;

    // Add thee image data only for the interesting PIDs
    if (IsInterestingPid(pid))
    {
        auto processDataIt = m_pidProcessDataMap.find(pid);

        if (processDataIt != m_pidProcessDataMap.end())
        {
            TPImageData imageData(imageRec.m_processId,
                                  imageRec.m_timeDateStamp,
                                  0,
                                  imageRec.m_imageBase,
                                  imageRec.m_imageSize,
                                  imageRec.m_imageCheckSum,
                                  imageRec.m_defaultBase,
                                  imageRec.m_fileName,
                                  false);

            (*processDataIt).second.AddImage(imageData);
        }
    }

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    // On Linux, add kernel image
    else if ((!m_isKernelImageAdded) && ((AMDTProcessId) - 1 == pid))
    {
        m_kernelImage.m_processId       = imageRec.m_processId;
        m_kernelImage.m_loadTimestamp   = imageRec.m_timeDateStamp;
        m_kernelImage.m_unloadTimestamp = 0;
        m_kernelImage.m_imageBase       = imageRec.m_imageBase;
        m_kernelImage.m_imageSize       = imageRec.m_imageSize;
        m_kernelImage.m_imageCheckSum   = imageRec.m_imageCheckSum;
        m_kernelImage.m_defaultBase     = imageRec.m_defaultBase;
        m_kernelImage.m_fileName        = imageRec.m_fileName;
        m_kernelImage.m_pExe            = nullptr;
        m_kernelImage.m_isSystemModule  = false;

        m_isKernelImageAdded = true;
    }

#endif

    return retVal;
} // AddImageLoadEvent


AMDTResult tpTranslate::AddThreadStartEvent(ThreadProfileEventThread& threadRec)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    // fprintf(stderr, "in AddThreadStartEvent pid(%u) tid(%u)..\n", threadRec.m_processId, threadRec.m_threadId);

    AMDTProcessId pid = threadRec.m_processId;
    AMDTThreadId tid = threadRec.m_threadId;

    // If the corresponding process is not found in m_pidProcessDataMap, shall we ignore this record?
    auto processIt = m_pidProcessDataMap.find(pid);

    if (processIt != m_pidProcessDataMap.end())
    {
        auto threadIt = m_tidThreadDataMap.find(tid);

        if (threadIt == m_tidThreadDataMap.end())
        {
            TPThreadData threadData;

            threadData.m_processId         = pid;
            threadData.m_threadId          = tid;
            threadData.m_affinity          = threadRec.m_affinity;
            threadData.m_threadCreateTS    = threadRec.m_timeStamp;
            threadData.m_threadTerminateTS = 0;
            threadData.m_prevProcessorId   = threadRec.m_processorId; // TODO: what is this ?

            m_tidThreadDataMap.insert(TPTidThreadDataMap::value_type(tid, threadData));

            // add this tid to the process
            (*processIt).second.AddThread(tid);

            // If this is the interesting process and this tid to interesting tids list
            if (IsInterestingPid(pid))
            {
                AddInterestingTid(tid, pid);
            }
        }
    }
    else
    {
        fprintf(stderr, "pid not found for tid(%u)\n", tid);
    }

    return retVal;
} // AddThreadStartEvent


AMDTResult tpTranslate::AddThreadStopEvent(ThreadProfileEventThread& threadRec)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    // fprintf(stderr, "in AddThreadStopEvent pid(%u) tid(%u)..\n", threadRec.m_processId, threadRec.m_threadId);

    AMDTProcessId pid = threadRec.m_processId;
    AMDTThreadId tid = threadRec.m_threadId;

    // If the corresponding process is not found in m_pidProcessDataMap, shall we ignore this record?
    auto processIt = m_pidProcessDataMap.find(pid);

    if (processIt != m_pidProcessDataMap.end())
    {
        auto threadIt = m_tidThreadDataMap.find(tid);

        if (threadIt != m_tidThreadDataMap.end())
        {
            (*threadIt).second.m_threadTerminateTS = threadRec.m_timeStamp;
        }
        else
        {
            fprintf(stderr, "AddThreadStopEvent - tid not found in map for tid(%u)\n", tid);
        }
    }
    else
    {
        fprintf(stderr, "AddThreadStopEvent - pid not found for tid(%u)\n", tid);
    }

    return retVal;
} // AddThreadStopEvent


AMDTResult tpTranslate::AddCSwitchEvent(ThreadProfileEventCSwitch& csRec)
{
    AMDTResult retVal = AMDT_STATUS_OK;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // The Windows ticks are in 100 nanoseconds (10^-7).
    AMDTUInt32 timeDiv = 10;
#else
    // On Linux, the TS are in nanoseconds (10^-9).
    AMDTUInt32 timeDiv = 1000;
#endif

    //fprintf(stderr, "in AddCSwitchCreateEvent %d\n", csRec.m_newThreadId);

    AMDTThreadId oldTid = csRec.m_oldThreadId;

    // update the old thread
    // if (IsValidThread(oldTid) && IsInterestingPid(GetPid(oldTid)))
    if (IsInterestingTid(oldTid))
    {
        auto oldThreadIt = m_tidThreadDataMap.find(oldTid);

        if (oldThreadIt != m_tidThreadDataMap.end())
        {
            TPThreadSample& oldThreadSample = (*oldThreadIt).second.m_currSample;

            if (oldThreadSample.m_intialized)
            {
                // The Windows ticks are in 100 nanoseconds (10^-7).
#define WINDOWS_TICK_PER_SEC 10000000

                oldThreadSample.m_endTS = csRec.m_timeStamp;

                AMDTUInt64 execTime = (AMDTUInt64)((oldThreadSample.m_endTS - oldThreadSample.m_startTS) / timeDiv);

                (*oldThreadIt).second.m_totalExeTime += execTime;
                oldThreadSample.m_execTime    = execTime;
                oldThreadSample.m_waitMode    = static_cast<AMDTThreadWaitMode>(csRec.m_oldThreadWaitMode);
                oldThreadSample.m_waitReason  = static_cast<AMDTThreadWaitReason>(csRec.m_oldThreadWaitReason);
                oldThreadSample.m_threadState = static_cast<AMDTThreadState>(csRec.m_oldThreadState);

                // push this sample into the list
                (*oldThreadIt).second.m_sampleList.push_back(oldThreadSample);
                (*oldThreadIt).second.m_nbrOfContextSwitches++;
                //(*oldThreadIt).second.m_prevProcessorId = csRec.m_processorId;

                oldThreadSample.m_complete = true;
                (*oldThreadIt).second.m_prevSample = oldThreadSample; // presere the current complete sample

                oldThreadSample.Clear();
            }
            else
            {
                // TODO: what to do with this case
                // fprintf(stderr, "CS record for old tid(%u) without init\n", oldTid);
            }
        }
    }

    // update the new thread
    AMDTThreadId newTid = csRec.m_newThreadId;

    //if (IsValidThread(newTid) && IsInterestingPid(GetPid(newTid)))
    if (IsInterestingTid(newTid))
    {
        auto newThreadIt = m_tidThreadDataMap.find(newTid);

        if (newThreadIt != m_tidThreadDataMap.end())
        {
            TPThreadSample& newThreadSample = (*newThreadIt).second.m_currSample;
            TPThreadSample& newThreadPrevSample = (*newThreadIt).second.m_prevSample;

            if (!newThreadSample.m_intialized)
            {
                newThreadSample.m_startTS    = csRec.m_timeStamp;
                newThreadSample.m_coreId     = csRec.m_processorId;
                newThreadSample.m_intialized = true;

                if ((*newThreadIt).second.m_prevProcessorId != csRec.m_processorId)
                {
                    (*newThreadIt).second.m_nbrOfCoreSwitches++;
                    (*newThreadIt).second.m_prevProcessorId = csRec.m_processorId;
                }

                // update the wait time (the thread is waiting for this much
                // time, before getting scheduled
                if (newThreadPrevSample.m_complete)
                {
                    AMDTUInt32 waitTime = 0;
                    waitTime = (AMDTUInt32)((newThreadSample.m_startTS - newThreadPrevSample.m_endTS) / timeDiv);

                    if (AMDT_THREAD_STATE_WAITING == newThreadPrevSample.m_threadState)
                    {
                        newThreadSample.m_transitionTime = 0;
                        newThreadSample.m_waitTime = waitTime;
                        (*newThreadIt).second.m_totalWaitTime += waitTime;
                    }
                    else
                    {
                        newThreadSample.m_waitTime = 0;
                        newThreadSample.m_transitionTime = waitTime;
                        (*newThreadIt).second.m_totalTransitionTime += waitTime;
                    }

                    // clear the previous sample
                    newThreadPrevSample.Clear();
                }
            }
            else
            {
                // TODO: what to do with this case?
                // fprintf(stderr, "CS record for new tid(%u) with init\n", newTid);
            }
        }
    }

    return retVal;
} // AddCSwitchEvent


AMDTResult tpTranslate::AddStackEvent(ThreadProfileEventStack& stackRec)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    //fprintf(stderr, "in AddCSwitchCreateEvent %d\n", csRec.m_newThreadId);

    AMDTThreadId tid = stackRec.m_stackThread;
    AMDTProcessId pid = stackRec.m_stackProcess;

    // TODO: should we get the pid for this tid from our structure and
    // check with pid in the stack record?
    if (IsInterestingPid(pid))
    {
        auto threadIt = m_tidThreadDataMap.find(tid);

        if (threadIt != m_tidThreadDataMap.end())
        {
            TPThreadSample& threadSample = (*threadIt).second.m_currSample;

            if (threadSample.m_intialized)
            {
                threadSample.m_nbrStackFrames = stackRec.m_nbrFrames;

                threadSample.m_pStackFrames = (AMDTUInt64*)malloc(sizeof(AMDTUInt64) * stackRec.m_nbrFrames);

                for (AMDTUInt32 i = 0; i < stackRec.m_nbrFrames; i++)
                {
                    threadSample.m_pStackFrames[i] = stackRec.m_stacks[i];
                }

                // threadSample.m_pStackFrames - TODO: when will this freed ?
            }
            else
            {
                fprintf(stderr, "could not add the stack record for tid %d\n", tid);
            }
        }
    }

    return retVal;
} // AddStackEvent


//
//  Print routines
//

AMDTResult tpTranslate::PrintThreadProfileData()
{
    AMDTResult retVal = AMDT_STATUS_OK;

    return retVal;
} // PrintThreadProfileData
