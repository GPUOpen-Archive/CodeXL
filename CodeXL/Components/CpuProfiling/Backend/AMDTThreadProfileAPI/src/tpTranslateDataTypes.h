//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpTranslateDataTypes.h
///
//==================================================================================

#ifndef _TPTRANSLATEDATATYPES_H_
#define _TPTRANSLATEDATATYPES_H_

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Base headers
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtList.h>

// Project Headers
#include <tpInternalDataTypes.h>
#include <AMDTThreadProfileDataTypes.h>


//
// Data Structures
//

// Event types
enum ThreadProfileEvent
{
    TP_EVENT_UNKNOWN = -1,
    TP_EVENT_INFO = 0,
    TP_EVENT_PROCESS = 1,
    TP_EVENT_IMAGE = 2,
    TP_EVENT_THREAD = 3,
    TP_EVENT_CSWITCH = 4,
    TP_EVENT_STACK = 5
};

enum ThreadProfileEventType
{
    TP_EVENT_TYPE_UNKNOWN = -1,
    TP_EVENT_TYPE_INFO,
    TP_EVENT_TYPE_PROCESS_START,
    TP_EVENT_TYPE_PROCESS_STOP,
    TP_EVENT_TYPE_PROCESS_DEFUNCT,
    TP_EVENT_TYPE_IMAGE_LOAD,
    TP_EVENT_TYPE_IMAGE_UNLOAD,
    TP_EVENT_TYPE_THREAD_START,
    TP_EVENT_TYPE_THREAD_STOP,
    TP_EVENT_TYPE_THREAD_READY,
    TP_EVENT_TYPE_CSWITCH,
    TP_EVENT_TYPE_STACK,
};

// header data in each ETL record
//
struct ThreadProfileEventInfo
{
    ThreadProfileEvent      m_event;
    ThreadProfileEventType  m_eventType;

    gtUByte                 m_version;
    gtUByte                 m_opcode;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    LARGE_INTEGER           m_timeStamp;
#else
    gtUInt64                m_timeStamp;
#endif

    gtUInt32                m_flags;
    gtUInt32                m_processorId;
    gtUInt32                m_processId;
    gtUInt32                m_threadId;
    gtUInt64                m_procTime;
    gtUInt32                m_kernelTime;
    gtUInt32                m_userTime;

    gtUInt32                m_pointerSize;
};


typedef struct ThreadProfileEventGeneric
{
    gtUInt64  m_timeStamp;    // from ThreadProfileEventInfo
    gtUInt32  m_processorId;  // from ThreadProfileEventInfo

    gtUInt32  m_numberOfProcessors;
} ThreadProfileEventGeneric;


// Process_TypeGroup1 record
//
// Event:       EVENT_TRACE_FLAG_PROCESS
//    Enables "Process" event type - "Process_TypeGroup1"
//
// Guid:        ProcessGuid (3d6fa8d0-fe05-11d0-9dda-00c04fd7ba7c), EventVersion(3), class Process : MSNT_SystemTrace
//
// Process and thread start events may be logged in the context of the parent process or thread.
// As a result, the ProcessId and ThreadId members of EVENT_TRACE_HEADER may not correspond to the
// process and thread being created.This is why these events contain the process and thread identifiers
// in the event data.
//
// The DCStart and DCEnd event types enumerate the process that are currently running, including idle
// and system process, at the time the kernel session starts and ends, respectively
//
typedef struct ThreadProfileEventProcess
{
    gtUInt64                m_timeStamp;    // from ThreadProfileEventInfo
    gtUInt32                m_processorId;  // from ThreadProfileEventInfo

    // The address of the process object in the kernel
    gtUInt64                m_uniqueProcessKey;

    // Global process identifier that you can use to identify a process
    gtUInt32                m_processId;

    // Unique identifier of the process that creates this process
    gtUInt32                m_parentId;

    // Unique identifier that an operating system generates when
    // it creates a new session. A session spans a period of time
    // from log on until log off from a specific system
    gtUInt32                m_sessionId;

    // Exit status of the stopped process
    gtInt32                 m_exitStatus;
    bool                    m_isActive;

    // The physical address of the page table of the process
    gtUInt64                m_directoryTableBase;

    // Security identifier (SID) for the user context under which the event happens
    void*                   m_userSID;             // FIXME ??

    // Path to the executable file of the process
    gtString                m_imageFileName;

    // Full command line of the process
    gtString                m_commandLine;
} ThreadProfileEventProcess;

// ThreadProfileEventImage
//
// Event:       EVENT_TRACE_FLAG_IMAGE_LOAD
//              Enables "Image" event type - "Image_Load"
//
// Guid : ImageLoadGuid(2cb15d1d - 5fc1 - 11d2 - abe1 - 00a0c911f518) EventVersion(2), class Image : MSNT_SystemTrace
//
// EventType : (10, 2, 3, 4), EventTypeName("Load", "Unload", "DCStart", "DCEnd")]class Image : Image
//               EVENT_TRACE_TYPE_LOAD(10) : Image load event.Generated when a DLL or executable file
//                                           is loaded.The provider generates only one event for the first time a given DLL is loaded.
//               EVENT_TRACE_TYPE_END(2) : Image unload event.generates only one event for the
//                                         last time a given DLL is unloaded.
//               EVENT_TRACE_TYPE_DC_START(3) : Data collection start event.Enumerates all loaded images at the beginning of the trace
//               EVENT_TRACE_TYPE_DC_END(4) : Data collection end event.Enumerates all loaded images at the end of the trace
//
//               class : Image_Load
// Notes: The DCStart and DCEnd events enumerate all loaded images at the beginning and end of the trace respectively.
//
typedef struct ThreadProfileEventImage
{
    // Header
    gtUInt64  m_timeStamp;    // from ThreadProfileEventInfo
    gtUInt32  m_processorId;  // from ThreadProfileEventInfo

    // Base address of the application in which the image is loaded
    gtUInt64  m_imageBase;

    gtUInt64  m_imageSize;

    // Identifies the process into which the image is loaded
    gtUInt32  m_processId;

    gtUInt32  m_imageCheckSum;

    // Time and date that the image was loaded or unloaded
    gtUInt32  m_timeDateStamp;
    gtUInt32  m_reserved0;

    // Default base address.
    gtUInt64  m_defaultBase;
    gtUInt32  m_reserved1;
    gtUInt32  m_reserved2;
    gtUInt32  m_reserved3;
    gtUInt32  m_reserved4;

    // File name and extension of the DLL or executable image
    gtString  m_fileName;
} ThreadProfileEventImage;

// ThreadProfileEventThread
//
// Event:       EVENT_TRACE_FLAG_THREAD
//              Enables the following "Thread" event type : "Thread_TypeGroup1"
//
// Guid : ThreadGuid(3d6fa8d1 - fe05 - 11d0 - 9dda - 00c04fd7ba7c)
//
// EventType : {1, 2, 3, 4}, EventTypeName{ "Start", "End", "DCStart", "DCEnd" }]class Thread_TypeGroup1 : Thread
//
//               EVENT_TRACE_TYPE_START(1) : Start thread event
//               EVENT_TRACE_TYPE_END(2) : End thread event
//               EVENT_TRACE_TYPE_DC_START(3) : Start data collection thread event.Enumerates threads that
//                                              are currently running at the time the kernel session starts
//               EVENT_TRACE_TYPE_DC_END(4) : End data collection thread event.Enumerates threads that
//                                            are currently running at the time the kernel session ends
//
//               For Thread_V2
//                 36 : Context switch event.The "CSwitch" MOF class defines the event data for this event.
//                 50 : Ready thread event.The "ReadyThread" MOF class defines the event data for this event
//
//              class : "Thread_TypeGroup1"
//
// Notes:
// The DCStart and DCEnd event types enumerate the threads that are currently running at the time the
// kernel session starts and ends, respectively
//
// Process and thread start events may be logged in the context of the parent process or thread.As a
// result, the ProcessId and ThreadId members of EVENT_TRACE_HEADER may not correspond to the process
// and thread being created.This is why these events contain the process and thread identifiers
// in the event data(in addition to those in the event header
//
typedef struct ThreadProfileEventThread
{
    gtUInt64  m_timeStamp;    // from ThreadProfileEventInfo
    gtUInt32  m_processorId;  // from ThreadProfileEventInfo

    // Process identifier of the thread involved in the event
    gtUInt32  m_processId;

    // Thread identifier of the thread involved in the event
    gtUInt32  m_threadId;

    // Base address of the thread's stack
    gtUInt64  m_stackBase;

    // Limit of the thread's stack
    gtUInt64  m_stackLimit;

    // Base address of the thread's user-mode stack
    gtUInt64  m_userStackBase;

    // Limit of the thread's user-mode stack
    gtUInt64  m_userStackLimit;

    // The set of processors on which the thread is allowed to run
    gtUInt64  m_affinity;

    // Starting address of the function to be executed by this thread
    gtUInt64  m_win32StartAddr;

    // Thread environment block base address
    gtUInt64  m_tebBase;

    // Identifies the service if the thread is owned by a service; otherwise, zero
    gtUInt32  m_subProcessTag;

    // The scheduler priority of the thread
    gtByte    m_basePriority;

    // A memory page priority hint for memory pages accessed by the thread
    gtByte    m_pagePriority;

    // An IO priority hint for scheduling IOs generated by the thread
    gtByte    m_ioPriority;

    // Not used.
    gtByte    m_threadFlags;
} ThreadProfileEventThread;


// Context Switch record
//
// Event:       EVENT_TRACE_FLAG_CSWITCH
//              Enables the following "Thread" event type : "CSwitch"
//
// Guid : ThreadGuid "{3d6fa8d1-fe05-11d0-9dda-00c04fd7ba7c}"), EventVersion(2)]class Thread_V2 : MSNT_SystemTrace
//
// EventType : {36}, EventTypeName{ "CSwitch" }]class CSwitch : Thread_V2
//             class : CSwitch
//
typedef struct ThreadProfileEventCSwitch
{
    gtUInt64  m_timeStamp;    // from ThreadProfileEventInfo;
    gtUInt32  m_processorId;  // from ThreadProfileEventInfo

    gtUInt32  m_newThreadId;
    gtUInt32  m_oldThreadId;
    gtByte    m_newThreadPriority;
    gtByte    m_oldThreadPriority;
    gtByte    m_previousCState;
    gtByte    m_spareByte;
    gtByte    m_oldThreadWaitReason;
    gtByte    m_oldThreadWaitMode;
    gtByte    m_oldThreadState;
    gtByte    m_oldThreadWaitIdealProcessor;
    gtUInt32  m_newThreadWaitTime;
    gtUInt32  m_reserved;
} ThreadProfileEventCSwitch;


typedef struct ThreadProfileEventStack
{
    gtUInt64  m_timeStamp;    // from ThreadProfileEventInfo
    gtUInt32  m_processorId;  // from ThreadProfileEventInfo

    // Original event time stamp from the event header. Use this
    // timestamp to match the stack to an event
    gtUInt64  m_eventTimeStamp;

    // The process identifier of the original event
    gtUInt32  m_stackProcess;

    // The thread identifier of the original event
    gtUInt32  m_stackThread;

    gtUInt32  m_nbrFrames;
    gtUInt64  m_stacks[200];    // FIXME
} ThreadProfileEventStack;

#endif // _TPTRANSLATEDATATYPES_H_
