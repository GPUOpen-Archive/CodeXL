//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpInternalDataTypes.h
///
//==================================================================================

#ifndef _TPINTERNALDATATYPES_H_
#define _TPINTERNALDATATYPES_H_

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define INITGUID  // Include this #define to use SystemTraceControlGuid/EventTraceGuid  in Evntrace.h.

    // System Headers
    #include <windows.h>
    #include <evntrace.h>
    #include <evntcons.h>
    #include <tdh.h>
#else
    #include <algorithm>
#endif // AMDT_WINDOWS_OSl
#include <AMDTExecutableFormat/inc/ExecutableFile.h>
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <AMDTExecutableFormat/inc/PeFile.h>
#else
    #include <AMDTExecutableFormat/inc/ElfFile.h>
#endif
#include <AMDTExecutableFormat/inc/SymbolEngine.h>


#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtHashMap.h>

// Project Headers
#include <AMDTThreadProfileDataTypes.h>

//
// Macros
//

#define TP_MAX_ETL_PATH_LEN        256
#define TP_MAX_NAME_LEN            256
#define TP_ETW_EVENT_TYPE_MAX      64

#define TP_ALL_PIDS                0xFFFFFFFFUL
#define TP_ALL_TIDS                0xFFFFFFFFUL

#define TP_STR_Unknown                  "Unknown"

#define TP_STR_ThreadStateInitialized   "Initialized"
#define TP_STR_ThreadStateReady         "Ready"
#define TP_STR_ThreadStateRunning       "Running"
#define TP_STR_ThreadStateStandby       "Standby"
#define TP_STR_ThreadStateTerminated    "Terminated"
#define TP_STR_ThreadStateWaiting       "Waiting"
#define TP_STR_ThreadStateTransition    "Transition"
#define TP_STR_ThreadStateDeferredReady "DeferredReady"
#define TP_STR_ThreadStateGatewait      "Gatewait"
#define TP_STR_ThreadStateMaximum       "Unknown"

#define TP_STR_ThreadWaitModeKernel     "Kernel"
#define TP_STR_ThreadWaitModeUser       "User"
#define TP_STR_ThreadWaitModeMAximum    "Unknown"

#define TP_STR_ThreadWaitReasonExecutive                "Executive"
#define TP_STR_ThreadWaitReasonFreepage                 "FreePage"
#define TP_STR_ThreadWaitReasonPageIn                   "PageIn"
#define TP_STR_ThreadWaitReasonPoolAllocation           "PoolAllocation"
#define TP_STR_ThreadWaitReasonDelayExecution           "DelayExecution"
#define TP_STR_ThreadWaitReasonSuspended                "Suspended"
#define TP_STR_ThreadWaitReasonUserRequest              "UserRequest"
#define TP_STR_ThreadWaitReasonWrExecutive              "WrExecutive"
#define TP_STR_ThreadWaitReasonWrFreePage               "WrFreePage"
#define TP_STR_ThreadWaitReasonWrPageIn                 "WrPageIn"
#define TP_STR_ThreadWaitReasonWrPoolAllocation         "WrPoolAllocation"
#define TP_STR_ThreadWaitReasonWrDelayExecution         "WrDelayExecution"
#define TP_STR_ThreadWaitReasonWrSuspended              "WrSuspended"
#define TP_STR_ThreadWaitReasonWrUserRequest            "WrUserRequest"
#define TP_STR_ThreadWaitReasonWrSpare0                 "WrSpare0"
#define TP_STR_ThreadWaitReasonWrQueue                  "WrQueue"
#define TP_STR_ThreadWaitReasonWrLPCReceive             "WrLPCReceive"
#define TP_STR_ThreadWaitReasonWrLPCReply               "WrLPCReply"
#define TP_STR_ThreadWaitReasonWrVirtualMemory          "WrVirtualMemory"
#define TP_STR_ThreadWaitReasonWrPageOut                "WrPageOut"
#define TP_STR_ThreadWaitReasonWrRendezvous             "WrRendezvous"
#define TP_STR_ThreadWaitReasonWrKeyedEvent             "WrKeyedEvent"
#define TP_STR_ThreadWaitReasonWrTerminated             "WrTerminated"
#define TP_STR_ThreadWaitReasonWrProcessInSwap          "WrProcessInSwap"
#define TP_STR_ThreadWaitReasonWrCpuRateControl         "WrCpuRateControl"
#define TP_STR_ThreadWaitReasonWrCalloutStack           "WrCalloutStack"
#define TP_STR_ThreadWaitReasonWrKernel                 "WrKernel"
#define TP_STR_ThreadWaitReasonWrResource               "WrResource"
#define TP_STR_ThreadWaitReasonWrPushLock               "WrPushLock"
#define TP_STR_ThreadWaitReasonWrMutex                  "WrMutex"
#define TP_STR_ThreadWaitReasonWrQuantumEnd             "WrQuantumEnd"
#define TP_STR_ThreadWaitReasonWrDispatchInt            "WrDispatchInt"
#define TP_STR_ThreadWaitReasonWrPreempted              "WrPreempted"
#define TP_STR_ThreadWaitReasonWrYieldExecution         "WrYieldExecution"
#define TP_STR_ThreadWaitReasonWrFastMutex              "WrFastMutex"
#define TP_STR_ThreadWaitReasonWrGuardedMutex           "WrGuardedMutex"
#define TP_STR_ThreadWaitReasonWrRunDown                "WrRunDown"
#define TP_STR_ThreadWaitReasonWrAlertByThreadid        "WrAlertByThreadid"
#define TP_STR_ThreadWaitReasonWrDeferredPreempt        "WrDeferredPreempt"
#define TP_STR_ThreadWaitReasonmaximumwaitreason        "Unknown"


//
// Global Externs
//


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    extern const GUID g_imageGuid;
    extern const GUID g_processGuid;
    extern const GUID g_threadGuid;
    extern const GUID g_stackWalkGuid;
    extern const GUID g_perfInfoGuid;
#endif // AMDT_WINDOWS_OS

//
// Data Structures
//

typedef gtList<AMDTProcessId> TPProcessIdList;
typedef gtList<AMDTThreadId> TPThreadIdList;

class TPImageData
{
public:
    // Identifies the process into which the image is loaded
    gtUInt32         m_processId;

    gtUInt64         m_loadTimestamp;
    gtUInt64         m_unloadTimestamp;

    // Base address of the application in which the image is loaded
    gtUInt64         m_imageBase;

    gtUInt64         m_imageSize;

    gtUInt32         m_imageCheckSum;

    // Default base address.
    gtUInt64         m_defaultBase;    //??

    // File name and extension of the DLL or executable image
    gtString         m_fileName;

    ExecutableFile*  m_pExe;
    bool             m_isSystemModule;

public:
    TPImageData() : m_processId(TP_ALL_PIDS), m_loadTimestamp(0), m_unloadTimestamp(0),
        m_imageBase(0), m_imageSize(0), m_imageCheckSum(0), m_defaultBase(0),
        m_pExe(nullptr), m_isSystemModule(false)
    { }

    TPImageData(gtUInt32 pid, gtUInt64 loadTS, gtUInt64 unloadTS, gtUInt64 base,
                gtUInt64 size, gtUInt32 cksum, gtInt64 defaultBase, gtString name, bool isSysMod)
    {
        m_processId       = pid;
        m_loadTimestamp   = loadTS;
        m_unloadTimestamp = unloadTS;
        m_imageBase       = base;
        m_imageSize       = size;
        m_imageCheckSum   = cksum;
        m_defaultBase     = defaultBase;
        m_fileName        = name;
        m_pExe            = nullptr;
        m_isSystemModule  = isSysMod;
    }

    ~TPImageData()
    {
        if (nullptr != m_pExe)
        {
            delete m_pExe;
            m_pExe = nullptr;
        }
    }
};

typedef gtMap<gtUInt64, TPImageData> TPLoadAddrImageDataMap;

// typedef struct TPProcessData
class TPProcessData
{
public:
    AMDTProcessId   m_pid;

    bool            m_stillActive;  // on windows if the exit status is 259
    gtUInt32        m_exitStatus;

    gtString        m_executableName;
    gtString        m_commandLine;

    // This is esstentially the time at which the Process DCStart record was created,
    // or Process Start record (Actual process creation)
    AMDTUInt64      m_processCreateTS;

    // This is esstentially the time at which the Process DCEnd record was created,
    // or Process Stop record (actual process termination)
    AMDTUInt64      m_processTerminateTS;

    TPProcessIdList m_childProcessesList;

    TPThreadIdList  m_threadsList;

    TPLoadAddrImageDataMap  m_imageMap;

public:
    TPProcessData() : m_pid(TP_ALL_PIDS), m_stillActive(false), m_exitStatus(0),
        m_processCreateTS(0), m_processTerminateTS(0)
    {};

    ~TPProcessData()
    {
        m_childProcessesList.clear();
        m_threadsList.clear();
        m_imageMap.clear();
    }

    void AddChildProcess(AMDTProcessId childPid)
    {
        auto it = std::find(m_childProcessesList.begin(), m_childProcessesList.end(), childPid);

        if (it == m_childProcessesList.end())
        {
            m_childProcessesList.push_back(childPid);
        }
    }

    void AddThread(AMDTThreadId tid)
    {
        auto it = std::find(m_threadsList.begin(), m_threadsList.end(), tid);

        if (it == m_threadsList.end())
        {
            m_threadsList.push_back(tid);
        }
    }

    void AddImage(TPImageData& imageData)
    {
        auto it = m_imageMap.find(imageData.m_imageBase);
        // FIXME this does not handle if the same load address is used for another load module
        // (dlopen(libA), dlclose(libA), (dlopen(libB)

        if (it == m_imageMap.end())
        {
            m_imageMap.insert(TPLoadAddrImageDataMap::value_type(imageData.m_imageBase, imageData));
        }
        else
        {
            // FIXME Not sure whether this is correct
            TPImageData& iData = (*it).second;
            iData.m_loadTimestamp = imageData.m_loadTimestamp;
            iData.m_unloadTimestamp = imageData.m_unloadTimestamp;
            iData.m_imageBase = imageData.m_imageBase;
            iData.m_imageSize = imageData.m_imageSize;
            iData.m_fileName = imageData.m_fileName;
        }
    }
};


typedef struct TPThreadSample
{
    bool                    m_intialized;
    bool                    m_complete;

    AMDTUInt32              m_coreId;           // the core in which TID was running

    AMDTUInt64              m_startTS;          // can be of type AMDTSystemTime
    AMDTUInt64              m_endTS;            // can be of type AMDTSystemTime ??

    AMDTUInt64              m_execTime;         // in micro seconds
    AMDTUInt32              m_waitTime;         // in micro seconds
    AMDTUInt32              m_transitionTime;   // in micro seconds ready state to running state transition

    AMDTThreadState         m_threadState;      // running or waiting
    AMDTThreadWaitReason    m_waitReason;       // reason for the wait
    AMDTThreadWaitMode      m_waitMode;         // Kernel or User mode

    AMDTUInt32              m_nbrStackFrames;
    AMDTUInt64*             m_pStackFrames;

    TPThreadSample() : m_intialized(false), m_complete(false),
        m_coreId((AMDTUInt32) - 1), m_startTS(0), m_endTS(0),
        m_execTime(0), m_waitTime(0), m_transitionTime(0),
        m_threadState(AMDT_THREAD_STATE_MAXIMUM),
        m_waitReason(AMDT_THREAD_WAIT_REASON_MAXIMUMWAITREASON),
        m_waitMode(AMDT_THREAD_WAIT_MODE_MAXIMUM),
        m_nbrStackFrames(0), m_pStackFrames(NULL)
    {
    };

    void Clear()
    {
        m_intialized = false;
        m_complete = false;
        m_coreId = (AMDTUInt32) - 1;
        m_startTS = 0;
        m_endTS = 0;
        m_threadState = AMDT_THREAD_STATE_MAXIMUM;
        m_waitReason = AMDT_THREAD_WAIT_REASON_MAXIMUMWAITREASON;
        m_waitMode = AMDT_THREAD_WAIT_MODE_USER;
        m_nbrStackFrames = 0;
        m_pStackFrames = NULL;
    }

} TPThreadSample;

typedef gtList<TPThreadSample> TPThreadSampleList;

typedef struct TPThreadData
{
    AMDTProcessId       m_processId;
    AMDTThreadId        m_threadId;

    // gtString            m_pThreadName;

    AMDTUInt64          m_affinity;
    AMDTUInt64          m_threadCreateTS;
    AMDTUInt64          m_threadTerminateTS;

    AMDTUInt64          m_totalExeTime;         // in micro second
    AMDTUInt64          m_totalWaitTime;        // in micro second
    AMDTUInt64          m_totalTransitionTime;       // in micro second

    AMDTUInt64          m_totalKernelTime;      // in micro seconds - UNUSED
    AMDTUInt64          m_totalUserTime;        // in micro seconds - UNUSED

    AMDTUInt32          m_prevProcessorId;      // the core in which the thread was running in the prev context switch
    AMDTUInt32          m_nbrOfContextSwitches;
    AMDTUInt32          m_nbrOfCoreSwitches;

    TPThreadSample      m_currSample;
    TPThreadSample      m_prevSample;

    TPThreadSampleList  m_sampleList;

    TPThreadData() : m_processId(TP_ALL_PIDS), m_threadId(TP_ALL_TIDS), m_affinity(0),
        m_threadCreateTS(0), m_threadTerminateTS(0),
        m_totalExeTime(0), m_totalWaitTime(0), m_totalTransitionTime(0),
        m_totalKernelTime(0), m_totalUserTime(0),
        m_prevProcessorId(0), m_nbrOfContextSwitches(0), m_nbrOfCoreSwitches(0)
    {};

} TPThreadData;


typedef gtHashMap<AMDTProcessId, TPProcessData> TPPidProcessDataMap;
typedef gtHashMap<AMDTThreadId, TPThreadData> TPTidThreadDataMap;

// To find the interesting pid
typedef gtHashMap<AMDTProcessId, AMDTProcessId> TPPidMap;
// To find the interesting tid
typedef gtHashMap<AMDTThreadId, AMDTProcessId> TPTidPidMap;

#endif //_TPINTERNALDATATYPES_H_
