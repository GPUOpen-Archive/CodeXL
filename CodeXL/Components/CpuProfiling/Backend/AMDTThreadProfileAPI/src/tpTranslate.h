//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpTranslate.h
///
//==================================================================================

#ifndef _TPTRANSLATE_H_
#define _TPTRANSLATE_H_

// Base headers
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtList.h>

// OS Wrappers headers
#include <AMDTOSWrappers/Include/osProcess.h>

// Project Headers
#include <tpInternalDataTypes.h>
#include <AMDTThreadProfileDataTypes.h>
#include <tpTranslateDataTypes.h>


#define KERNEL_SYMBOL_LEN    128

// the type can be function or variable
#define KERNEL_FUNCTION     0x1
#define KERNEL_VARIABLE     0x2

//
// tpTranslate
//


class tpTranslate
{
public:
    tpTranslate();

    virtual ~tpTranslate();

    AMDTResult Initialize(const char* pLogFile, char* debugLogFile = NULL);

    virtual AMDTResult OpenThreadProfileData(bool isPassOne) = 0;
    virtual AMDTResult ProcessThreadProfileData() = 0;
    virtual AMDTResult CloseThreadProfileData() = 0;

    bool IsPassOneCompleted() const { return m_isPassOneCompleted; }
    bool IsallCSRecordsProcessed() const { return m_allCSRecordsProcessed;  }

    void EnableHandleAllPids() { m_handleAllPids = true; }
    void DisableHandleAllPids() { m_handleAllPids = false; }
    bool IsHandleAllPids() const { return m_handleAllPids; }

    bool IsValidProcess(AMDTProcessId pid) const;
    bool IsInterestingPid(AMDTProcessId pid);
    bool AddInterestingPid(AMDTProcessId pid);
    AMDTProcessId GetPid(AMDTThreadId tid);

    gtUInt32 GetNumberOfProcessors() const { return m_numberOfProcessors; }

    size_t GetNumberOfProcesses() const { return m_pidProcessDataMap.size(); }
    AMDTResult GetProcessIds(AMDTUInt32 size, AMDTProcessId*& pProcesses);
    AMDTResult GetProcessData(AMDTProcessId pid, AMDTProcessData& processData);

    bool AddInterestingTid(AMDTThreadId tid, AMDTProcessId pid);
    bool IsInterestingTid(AMDTThreadId tid);

    bool IsValidThread(AMDTThreadId tid) const;
    AMDTResult GetThreadIds(AMDTProcessId pid, AMDTUInt32 size, AMDTThreadId*& ppThreads);
    AMDTResult GetThreadData(AMDTThreadId tid, AMDTThreadData& threadData);
    size_t GetNumberOfThreads() const { return m_tidThreadDataMap.size(); }
    size_t GetNumberOfThreads(AMDTProcessId pid) const;

    AMDTResult GetThreadSampleData(AMDTThreadId tid, AMDTUInt32* pNbrRecords, AMDTThreadSample** ppThreadSampleData);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // TODO: This should be in OSWrappers - a generic helper function
    bool ConvertDeviceNameToFileName(const gtString& deviceName, gtString& fileName);
#endif

    AMDTResult SetSymbolSearchPath(const char* pSearchPath,
                                   const char* pServerList,
                                   const char* pCachePath);

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    AMDTResult FindKernelSymbol(const char* pKernelSymFile,
                                char**      pSymbol,
                                gtUInt64    startAddress);
#endif
    AMDTResult GetFunctionName(AMDTProcessId pid, AMDTUInt64 pc, char** ppFuncName);

    // Helpers used internally
    AMDTResult AddInfoEvent(ThreadProfileEventGeneric& genericRec);

    AMDTResult AddProcessCreateEvent(ThreadProfileEventProcess& processRec);
    AMDTResult AddProcessStartEvent(ThreadProfileEventProcess& processRec);
    AMDTResult AddProcessStopEvent(ThreadProfileEventProcess& processRec);

    AMDTResult AddImageLoadEvent(ThreadProfileEventImage& imageRec);

    AMDTResult AddThreadStartEvent(ThreadProfileEventThread& threadRec);
    AMDTResult AddThreadStopEvent(ThreadProfileEventThread& threadRec);

    AMDTResult AddCSwitchEvent(ThreadProfileEventCSwitch& csRec);
    AMDTResult AddStackEvent(ThreadProfileEventStack& stackRec);

    void EnableDebugLog() { m_verbose = true; }
    void DisableDebugLog() { m_verbose = false; }

    AMDTResult PrintThreadProfileData();

protected:
    bool            m_verbose;
    bool            m_userMode;
    bool            m_isPassOneCompleted;
    bool            m_allCSRecordsProcessed;
    bool            m_handleAllPids;

    AMDTUInt64      m_timerResolution;
    AMDTUInt32      m_pointerSize;

    wchar_t         m_logFile[TP_MAX_ETL_PATH_LEN];
    char*           m_debugLogFile;

    FILE*           m_debugLogFP;

    gtUInt32        m_numberOfProcessors;

    // Symbol search paths
    gtString        m_searchPath;
    gtString        m_serverList;
    gtString        m_cachePath;

    // TPProcessIdList         m_interestingPidsList;
    TPPidMap                m_interestingPidsMap;
    TPTidPidMap             m_interestingTidsMap;

    TPPidProcessDataMap     m_pidProcessDataMap;
    TPTidThreadDataMap      m_tidThreadDataMap;

    // Kernel image
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    TPImageData             m_kernelImage;
    bool                    m_isKernelImageAdded;
#endif

};

#endif //_TPTRANSLATE_H_
