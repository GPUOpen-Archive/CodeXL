//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTThreadProfileDataTypes.h
///
//==================================================================================

#ifndef _AMDTTHREADPROFILEDATATYPES_H_
#define _AMDTTHREADPROFILEDATATYPES_H_

#include <AMDTCommonHeaders/AMDTDefinitions.h>
//#include <AMDTExecutableFormat/inc/SymbolEngine.h>

typedef AMDTUInt32 AMDTProcessId;
typedef AMDTUInt32 AMDTThreadId;
typedef void* AMDTThreadProfileDataHandle;

// AMDTThreadProfileState
//
// AMDT_THREAD_PROFILE_STATE_UNINITIALIZED  - uninitiaized state before calling any thread profile apis
// AMDT_THREAD_PROFILE_STATE_INITIALIZED    - thread profiler is initialized by AMDTSetThreadProfileConfiguration()
// AMDT_THREAD_PROFILE_STATE_STARTED        - thread profiler is configured and profiling is in progress
// AMDT_THREAD_PROFILE_STATE_ABORTED        - this state occurs due to any internal errors (Windows API failures)
//                                            subsequent call to AMDTSetThreadProfileConfiguration()
//                                            will cleanup the internal structures
typedef enum
{
    AMDT_THREAD_PROFILE_STATE_UNINITIALIZED = 0,    // before AMDTSetThreadProfileConfiguration
    AMDT_THREAD_PROFILE_STATE_INITIALIZED   = 1,    // after AMDTSetThreadProfileConfiguration
    AMDT_THREAD_PROFILE_STATE_STARTED       = 2,    // after AMDTStartThreadProfile
    AMDT_THREAD_PROFILE_STATE_STOPPED       = 3,
    AMDT_THREAD_PROFILE_STATE_ABORTED       = 4,
} AMDTThreadProfileState;


// AMDTThreadProfileEvents
//
// Following thread profile modes are supported:
//  AMDT_TP_EVENT_TRACE_CSWITCH      - trace context switch events
//  AMDT_TP_EVENT_TRACE_CALLSTACK    - collect callstack during context switch event tracing
//  AMDT_TP_EVENT_TRACE_SYSTEMCALL   - trace system calls
//
typedef enum
{
    AMDT_TP_EVENT_TRACE_CSWITCH     = (1 << 0),
    AMDT_TP_EVENT_TRACE_CALLSTACK   = (1 << 1),
    AMDT_TP_EVENT_TRACE_MAX         = (1 << 2),
} AMDTThreadProfileEvents;


typedef enum
{
    AMDT_THREAD_STATE_INITIALIZED,
    AMDT_THREAD_STATE_READY,
    AMDT_THREAD_STATE_RUNNING,
    AMDT_THREAD_STATE_STANDBY,
    AMDT_THREAD_STATE_TERMINATED,
    AMDT_THREAD_STATE_WAITING,
    AMDT_THREAD_STATE_TRANSITION,
    AMDT_THREAD_STATE_DEFERREDREADY,
    AMDT_THREAD_STATE_GATEWAIT,
    AMDT_THREAD_STATE_MAXIMUM
} AMDTThreadState;


// AMDTThreadWaitReason
//
// Only applicable when the thread is in wait state (AMDT_THREAD_STATE_WAITING)
//
typedef enum
{
    AMDT_THREAD_WAIT_REASON_EXECUTIVE,          // 0 - Waiting for a component of the Windows NT Executive
    AMDT_THREAD_WAIT_REASON_FREEPAGE,           // 1 - Waiting for a page to be freed
    AMDT_THREAD_WAIT_REASON_PAGEIN,             // 2 - Waiting for a page to be mapped or copied
    AMDT_THREAD_WAIT_REASON_POOLALLOCATION,     // 3 - Waiting for space to be allocated in the paged or nonpaged pool
    AMDT_THREAD_WAIT_REASON_DELAYEXECUTION,     // 4 - Waiting for an Execution Delay to be resolved
    AMDT_THREAD_WAIT_REASON_SUSPENDED,          // 5 - Suspended
    AMDT_THREAD_WAIT_REASON_USERREQUEST,        // 6 - Waiting for a user request
    AMDT_THREAD_WAIT_REASON_WREXECUTIVE,        // 7 - Waiting for a component of the Windows NT Executive
    AMDT_THREAD_WAIT_REASON_WRFREEPAGE,         // 8 - Waiting for a page to be freed
    AMDT_THREAD_WAIT_REASON_WRPAGEIN,           // 9 - Waiting for a page to be mapped or copied
    AMDT_THREAD_WAIT_REASON_WRPOOLALLOCATION,   // 10 - Waiting for space to be allocated in the paged or nonpaged pool
    AMDT_THREAD_WAIT_REASON_WRDELAYEXECUTION,   // 11 - Waiting for an Execution Delay to be resolved
    AMDT_THREAD_WAIT_REASON_WRSUSPENDED,        // 12 - Suspended
    AMDT_THREAD_WAIT_REASON_WRUSERREQUEST,      // 13 - Waiting for a user request
    AMDT_THREAD_WAIT_REASON_WRSPARE0,           // 14 - Waiting for an event pair high
    AMDT_THREAD_WAIT_REASON_WRQUEUE,            // 15 - Waiting for an event pair low
    AMDT_THREAD_WAIT_REASON_WRLPCRECEIVE,       // 16 - Waiting for an LPC Receive notice
    AMDT_THREAD_WAIT_REASON_WRLPCREPLY,         // 17 - Waiting for an LPC Reply notice
    AMDT_THREAD_WAIT_REASON_WRVIRTUALMEMORY,    // 18 - Waiting for virtual memory to be allocated
    AMDT_THREAD_WAIT_REASON_WRPAGEOUT,          // 19 - Waiting for a page to be written to disk
    AMDT_THREAD_WAIT_REASON_WRRENDEZVOUS,
    AMDT_THREAD_WAIT_REASON_WRKEYEDEVENT,
    AMDT_THREAD_WAIT_REASON_WRTERMINATED,       // 22 - Terminated
    AMDT_THREAD_WAIT_REASON_WRPROCESSINSWAP,
    AMDT_THREAD_WAIT_REASON_WRCPURATECONTROL,
    AMDT_THREAD_WAIT_REASON_WRCALLOUTSTACK,
    AMDT_THREAD_WAIT_REASON_WRKERNEL,
    AMDT_THREAD_WAIT_REASON_WRRESOURCE,
    AMDT_THREAD_WAIT_REASON_WRPUSHLOCK,
    AMDT_THREAD_WAIT_REASON_WRMUTEX,
    AMDT_THREAD_WAIT_REASON_WRQUANTUMEND,
    AMDT_THREAD_WAIT_REASON_WRDISPATCHINT,
    AMDT_THREAD_WAIT_REASON_WRPREEMPTED,
    AMDT_THREAD_WAIT_REASON_WRYIELDEXECUTION,
    AMDT_THREAD_WAIT_REASON_WRFASTMUTEX,
    AMDT_THREAD_WAIT_REASON_WRGUARDEDMUTEX,
    AMDT_THREAD_WAIT_REASON_WRRUNDOWN,
    AMDT_THREAD_WAIT_REASON_WRALERTBYTHREADID,
    AMDT_THREAD_WAIT_REASON_WRDEFERREDPREEMPT,
    AMDT_THREAD_WAIT_REASON_MAXIMUMWAITREASON
} AMDTThreadWaitReason;

typedef enum
{
    AMDT_THREAD_WAIT_MODE_KERNEL,
    AMDT_THREAD_WAIT_MODE_USER,
    AMDT_THREAD_WAIT_MODE_MAXIMUM
} AMDTThreadWaitMode;

// AMDTSystemTime
//
// This structure represents the system time in second and milliseconds
//
typedef struct AMDTSystemTime
{
    AMDTUInt64   m_second;
    AMDTUInt64   m_microSecond;
} AMDTSystemTime;


typedef struct AMDTProcessData
{
    AMDTProcessId   m_pid;
    const char*     m_pCommand;

    AMDTUInt64      m_processCreateTS;
    AMDTUInt64      m_processTerminateTS;

    AMDTUInt32      m_nbrChildProcesses;
    AMDTProcessId*  m_pChildProcesses;

    AMDTUInt32      m_nbrThreads;
    AMDTThreadId*   m_pThreads;
} AMDTProcessData;


typedef struct AMDTThreadData
{
    AMDTProcessId   m_processId;
    AMDTThreadId    m_threadId;

    char*           m_pThreadName;

    AMDTUInt64      m_affinity;

    AMDTUInt64      m_threadCreateTS;
    AMDTUInt64      m_threadTerminateTS;

    AMDTUInt64      m_totalExeTime;             // in micro seconds
    AMDTUInt64      m_totalWaitTime;            // in micro seconds
    AMDTUInt64      m_totalTransitionTime;      // in micro seconds

    AMDTUInt64      m_nbrOfContextSwitches;
    AMDTUInt64      m_nbrOfCoreSwitches;

    // AMDTUInt64   m_totalKernelTime   // in micro seconds
    // AMDTUInt64   m_totalUserTime     // in micro seconds
} AMDTThreadData;


// AMDTThreadSample
//
typedef struct AMDTThreadSample
{
    AMDTProcessId           m_processId;
    AMDTThreadId            m_threadId;
    AMDTUInt32              m_coreId;           // the core in which TID was running

    AMDTUInt64              m_startTS;          // can be of type AMDTSystemTime
    AMDTUInt64              m_endTS;            // can be of type AMDTSystemTime
    AMDTUInt64              m_execTime;         // actual running time in micro seconds
    AMDTUInt32              m_waitTime;         // wait time in micro seconds
    AMDTUInt32              m_transitionTime;   // ready state to run state transition time

    AMDTThreadState         m_threadState;      // running or waiting
    AMDTThreadWaitReason    m_waitReason;       // reason for the wait
    AMDTThreadWaitMode      m_waitMode;         // Kernel or User mode

    AMDTUInt32              m_nbrStackFrames;
    AMDTUInt64*             m_pStackFrames;

    // state, waitreason, wait mode, callstack
} AMDTThreadSample;

#endif //_AMDTTHREADPROFILEDATATYPES_H_
