//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PrdOldRecords.h
/// \brief Depreciated PRD file record information.
/// \note This is based on amd-pcore-records.h
///
/// This file is intended to allow converting old prd files to the current format.
///
//==================================================================================

#ifndef _PRDOLDRECORDS_H_
#define _PRDOLDRECORDS_H_

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

/// \def V1_HDR_VERSION
/// The m_Version used or below to determine if \ref sFileHeaderV1 is valid
#define V1_HDR_VERSION 0x0003
/// \def V2_HDR_VERSION
/// The m_Version used to determine if \ref sFileHeaderV2 is valid
#define V2_HDR_VERSION 0x0004
/// \def V3_HDR_VERSION
/// The m_Version used or above to determine if \ref PRD_FILE_HEADER_V3 is valid
#define V3_HDR_VERSION 0x0005
/// \def CA_HDR_VERSION
/// The CodeAnalyst last used version
#define CA_HDR_VERSION V3_HDR_VERSION

// The PRD_RECORD_TYPE are the same as in CaPrdRecords.
typedef struct  _sFileHeaderV1
{
    gtUInt32 m_Signature; // Signature field, to verify that this is indeed a data  file
    gtUInt16 m_Version; // Version of the data, for runtime determination - version 1
    gtUInt16 m_Source; // Set to 1 : Linux32, 2 : Linux64, 3 : Win32, 4: Win64, 5 : Win98
    gtUByte  m_DateTime[16]; // Text date and time of the trace - fill  to 16 bytes of ascii space
    // Date format (ascii characters) yyyymmddhhmmsshh
    gtUByte  m_Comment[16]; // User supplied comment - fill to 16 bytes of ascii space
} sFileHeaderV1;

typedef struct  _sFileHeaderV2
{
    gtUInt32    m_Signature;    // Signature field, to verify that this is indeed a data file
    gtUInt16    m_Version;      // Version of the data, for runtime determination - version 2
    gtUByte     m_Source;       // Set to 1: Linux32, 2: Linux64, 3: Win32, 4: Win64, 5: Win98
    gtUByte     m_CPU_number;   // which CPU the TSC is from
    FILETIME    m_StartMark;    // Filetime mark for user time mark synchronization
    gtUInt64    m_TSC;          // time stamp counter value for user time mark synchronization
    gtUByte     m_Comment[16];  // User supplied comment - fill to 16 bytes of ascii space
} sFileHeaderV2;

typedef struct  _TrcCPUInfoRecordSocket // CPU information record for one socket
{
    gtUByte  m_Padding;         // PROF_REC_CPUINFO
    gtUByte  m_NumCPUs;         // Number of logical CPUs in socket1 | 80H if HTT
    gtUByte  m_StartCPUNum;     // System number of first CPU in this socket
    gtUByte  m_ClockSpeed;      // Clock speed (* 100MHZ)
    gtUInt32 m_FamilyModel;     // Family, model, stepping Ids from CPUID FN 1
} sTrcCPUInfoRecordSocket;

typedef struct  _TrcCPUInfoRecord       // CPU information record
{
    sTrcCPUInfoRecordSocket socket[5];
} sTrcCPUInfoRecord;

typedef struct  _TrcIbsConfRecord    // IBS Configuration record
{
    gtUByte  m_RecordType;           // PROF_REC_IBSCFG
    gtUByte  m_Padding0;
    gtUInt16 m_Padding1;
    gtUInt32 m_Padding2;
    gtUInt64 m_Padding3;
    gtUInt64 m_TimeStampCounter;     // Save the time stamp counter
    gtUInt64 m_IbsOpMaxCnt;          // IBS Op Max Count
    gtUInt64 m_IbsFetchMaxCnt;       // IBS Fetch Max Count
} sTrcIbsConfRecord;

typedef struct  _TrcPIDConfRecord // PID Configuration record
{
    gtUByte     m_RecordType;               // PROF_REC_PIDCFG
    gtUByte     m_Padding0;
    gtUInt16    m_Padding1;
    gtUInt32    m_Padding2;
    gtUInt64    m_PID_Array[MAX_PID_COUNT]; // PID array of values
} sTrcPIDConfRecord;

typedef struct  _TrcEvtConfRecord   // Event Configuration record
{
    gtUByte      m_RecordType;          // PROF_REC_EVTCFG
    gtUByte      m_GroupIndex;          // Group number
    gtUInt16     m_Padding0;
    gtUInt32     m_ModeFlags;           // Sampling mode flags 1=count 2=load initial value */
    gtUInt64     m_EventSelReg;         // The performance event select register
    gtUInt64     m_EventCount;          // The event count number
    gtUInt64     m_TimeStampCounter;    // Save the time stamp counter
    gtUInt64     m_EvtMultiplexTicks;   // duration of clock ticks for event multiplex
} sTrcEvtConfRecord;

typedef struct  _TrcTimerConfRecord  // Event Configuration record
{
    gtUByte      m_RecordType;       // PROF_REC_TIMERCFG
    gtUByte      m_Padding0;
    gtUInt16     m_Padding1;
    gtUInt32     m_Padding2;
    gtUInt64     m_Padding3;
    gtUInt64     m_Padding4;
    gtUInt64     m_TimeStampCounter; // Save the time stamp counter
    gtUInt64     m_TimerGranularity; // Timer Profile Granularity in microseconds
} sTrcTimerConfRecord;

typedef struct  _TrcCallStackRecord  // call stack record type*/
{
    union
    {
        struct
        {
            gtUByte   m_RecordType;         // PROF_REC_CSS
            gtUByte   m_ProcessorNumber;    // cpu #
            gtUInt16  m_Padding0;
            gtUInt32  m_Padding1;
        };

        gtUInt64 m_Offsets;
    };
    gtUInt64    m_CallStack[4];
} sTrcCallStackRecord, *psTrcCallStackRecord;

typedef struct _TrcTimerRecord   // Timer triggered record
{
    gtUByte   m_RecordType;           // PROF_REC_TIMER
    gtUByte   m_ProcessorNumber;      // The number of the processor the event occured on
    gtUInt16  m_CodeSegment;          // Save the cs register value
    gtUByte   m_Padding0;             // padding
    gtUByte   m_Padding1;             // Padding
    gtUInt16  m_StatusBits;           // Bits : 0 eflags/rx, 1 eflags/vm, 2 cs/l, 3 cs/d, 4-15 res 0
    gtUInt64  m_InstructionPointer;   // Save the rip register value
    gtUInt64  m_TimeStampCounter;     // save the time stamp counter
    gtUInt64  m_ProcessHandle;        // save the interrupted threads process id
    gtUInt64  m_ThreadHandle;         // save the interrupted threads thread id
} sTrcTimerRecord;

typedef struct _TrcPerfCountRecord   // Performance counter triggered record
{
    gtUByte   m_RecordType;           // PROF_REC_EVENT
    gtUByte   m_ProcessorNumber;      // The number of the processor the event occured on
    gtUInt16  m_CodeSegment;          // Save the cs register value
    gtUByte   m_Padding0;             // padding
    gtUByte   m_UnitMask;             // Unit Mask
    gtUInt16  m_EventSelect;          // EventSelect[0:11]
    gtUInt64  m_InstructionPointer;   // Save the rip register value
    gtUInt64  m_TimeStampCounter;     // Save the time stamp counter
    gtUInt64  m_ProcessHandle;        // Save the interrupted threads process id
    gtUInt64  m_ThreadHandle;         // Save the interrupted threads thread id
} sTrcPerfCountRecord;

typedef struct  _TrcIbsFetchRecord
{
    gtUByte   m_RecordType;           // PROF_REC_IBS_FETCH_BASIC or PROF_REC_IBS_FETCH_EXT
    gtUByte   m_ProcessorNumber;      // Processor ID
    gtUInt16  m_Reserved;             // Reserved space
    gtUInt32  m_IbsFetchCtlHigh;      // IbsFetchCtl<63:32> (Fetch event flags/cycle counts)
    gtUInt64  m_InstructionPointer;   // IbsFetchLinAd<63:0>
    gtUInt64  m_TimeStampCounter;     // Time stamp counter
    gtUInt64  m_ProcessHandle;        // Process ID
    gtUInt64  m_ThreadHandle;         // Thread ID
} sTrcIbsFetchRecord;

typedef struct  _TrcIbsFetchRecordExt
{
    gtUInt64    m_IbsFetchPhysAd;  // IbsFetchPhysAd<63:0>
    gtUInt64    m_Reserved1;
    gtUInt64    m_Reserved2;
    gtUInt64    m_Reserved3;
    gtUInt64    m_Reserved4;
} sTrcIbsFetchRecordExt;

typedef struct  _TrcIbsOpRecord
{
    gtUByte   m_RecordType;           // PROF_REC_IBS_OP_BASIC or PROF_REC_IBS_OP_EXT
    gtUByte   m_ProcessorNumber;      // Processor ID
    gtUInt16  m_IbsOpDataHigh;        // IbsOpdata <37:32> Branch/return/resync flags
    gtUInt32  m_IbsOpDataLow;         // IbsOpData<31:0> Macro-op retire cycle counts
    gtUInt64  m_InstructionPointer;   // IbsOpRip<63:0> Macro-op linear address
    gtUInt64  m_TimeStampCounter;     // Time stamp counter
    gtUInt64  m_ProcessHandle;        // Process ID
    gtUInt64  m_ThreadHandle;         // Thread ID
} sTrcIbsOpRecord;

typedef struct  _TrcIbsOpRecordExt
{
    gtUInt64    m_IbsOpData3;   // IbsOpData3<63:0> Load/store flags/latency
    gtUInt64    m_IbsDcLinAd;   // IbsDcLinAd<63:0> IBS DC Linear Data Address
    gtUInt64    m_IbsDcPhyAd;   // IbsDcPhysAd<63:0> IBS DC Physical Data Address
    gtUInt32    m_Reserved3;
    gtUInt16    m_Reserved4;
    gtUByte     m_Reserved5;
    gtUByte     m_IbsOpData2;   // IbsOpData3<5:0> Northbridge data
    gtUInt64    m_Reserved6;    // Reserved for 64-bit branch target address
} sTrcIbsOpRecordExt;

#define MAX_COUNTERS 4

typedef struct  _TrcMissedCountRecord   // Missed performance counter sample record
{
    gtUByte     m_RecordType;                   // PROF_REC_MISSED
    gtUByte     m_GroupIndex;                   // Event Group Index
    gtUInt16    m_CPU;                          // CPU TSC was read on
    gtUInt32    m_Padding;
    gtUInt32    m_MissedEvent[MAX_COUNTERS];    // Missed event count
    gtUInt64    m_Padding2;                     // padding to 40
    gtUInt64    m_TSC;                          // TSC value when sampling ended
} sTrcMissedCountRecord;

typedef struct _TrcCaWeightRecord
{
    /// The record type is \ref PROF_REC_WEIGHT
    gtUByte m_RecordType;
    /// Which core the configuration is for.
    gtUByte m_Core;
    /// The indexes of the weights of the base resources
    /// Note that if the \ref MAX_RESOURCE_TYPE ever grows, this needs to
    /// be updated,
    gtUByte m_indexes[5];
    /// If not 0, then this is the 1st weight record of a new buffer.  The count
    /// is the number of records in the buffer (including this one)
    gtUByte m_BufferRecordCount;
    /// The weights of resources, with the respective indexes being the base
    /// resource of each type.  Thus EVENT_CTR 2's weight is at
    /// m_Weights[m_indexes[EVENT_CTR] + 2]
    gtUByte m_Weights[32];
} sTrcCaWeightRecord;

#endif //_PRDOLDRECORDS_H_
