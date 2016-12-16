//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PrdRecords.h
/// \brief  CpuProf raw data record format
///
//==================================================================================

#ifndef _CPUPROFDATA_H
#define _CPUPROFDATA_H

#include <stdlib.h>

#ifndef CTL_CODE
    #pragma message ("CTL_CODE undefined. Include winioctl.h")
#endif

#ifndef _FILETIME_
/// \struct FILETIME Holds time mark information
typedef struct _FILETIME
{
    /// lower timer information
    unsigned long dwLowDateTime;
    /// higher timer information
    unsigned long dwHighDateTime;
} FILETIME;
#endif

/// the definitions of the profile record types generated and
/// written to the data file by the device driver. Note that each of the
/// different records is the same size (40 bytes), and that each of the
/// different records have the common fields in the same place.
/// \note The reason for the somewhat bizarre ordering and record types without
/// a corresponding data structure is legacy code.
typedef enum
{
    /// Debug record, to assist with debugging the device driver.
    PROF_REC_DEBUG = 0x01,
    /// APIC Timer interrupt record.  The data is in \ref PRD_APIC_DATA_RECORD
    PROF_REC_TIMER = 0x02,
    /// EVENT_CTR interrupt record.  The data is in \ref
    /// PRD_EVENT_CTR_DATA_RECORD
    PROF_REC_EVENT = 0x03,
    /// Dynamic configuration record
    PROF_REC_CONFIG = 0x04,
    /// User-defined record.  The data is in \ref PRD_USER_RECORD
    PROF_REC_USER = 0x05,
    /// EVENT_CTR configuration record.  The data is in \ref
    /// PRD_EVENT_CTR_CONFIG_RECORD
    PROF_REC_EVTCFG = 0x06,
    /// Call-stack sample record.  The data is in \ref PRD_CSS_DATA_RECORD
    PROF_REC_CSS = 0x07,
    /// APIC timer configuration record.  The data is in \ref
    /// PRD_APIC_CONFIG_RECORD
    PROF_REC_TIMERCFG = 0x08,
    /// IBS fetch basic record.  The data is in \ref
    /// PRD_IBS_FETCH_DATA_BASIC_RECORD
    PROF_REC_IBS_FETCH_BASIC = 0x09,
    /// IBS op basic record.  The data is in \ref PRD_IBS_OP_DATA_BASIC_RECORD
    PROF_REC_IBS_OP_BASIC = 0x0A,
    /// IBS op extended records. The data is in \ref
    /// PRD_IBS_OP_DATA_BASIC_RECORD and the sequential record in \ref
    /// PRD_IBS_OP_DATA_EXT_RECORD
    PROF_REC_IBS_OP_EXT = 0x0B,
    /// IBS fetch extended records. The data is in \ref
    /// PRD_IBS_FETCH_DATA_BASIC_RECORD and the sequential record in \ref
    /// PRD_IBS_FETCH_DATA_EXT_RECORD
    PROF_REC_IBS_FETCH_EXT = 0x0C,
    /// IBS configuration record.  The data is in \ref PRD_IBS_CONFIG_RECORD
    PROF_REC_IBSCFG = 0x0F,
    /// IBS configuration record.  The data is in \ref
    /// PRD_PAUSE_RESUME_DATA_RECORD
    PROF_REC_RESUME = 0x10,
    /// IBS configuration record.  The data is in \ref
    /// PRD_PAUSE_RESUME_DATA_RECORD
    PROF_REC_PAUSE = 0x11,
    /// Missed sample counts record.  The data is in \ref
    /// PRD_MISSED_DATA_RECORD
    PROF_REC_MISSED = 0x12,
    /// Obsolete cpu info record.  The data is in sTrcCPUInfoRecordSocket.
    PROF_REC_CPUINFO = 0x13,
    /// CpuId record.  The data is in \ref PRD_CPUINFO_RECORD.
    PROF_REC_CPUID = 0x14,
    /// Keeps track of all weights for used resources in the profile.  The
    /// data is in \ref PRD_WEIGHT_RECORD
    PROF_REC_WEIGHT = 0x15,
    /// PID filter record.  The data is in \ref PRD_PID_CONFIG_RECORD
    PROF_REC_PIDCFG = 0x20,
    /// Core Topology record.  The data is in \ref PRD_TOPOLOGY_RECORD
    PROF_REC_TOPOLOGY = 0x21,
    /// Extended header record.  The data is in \ref PRD_HEADER_EXT_RECORD
    PROF_REC_EXT_HEADER = 0x22,
    /// Configuration core mask record.  The data is in \ref PRD_CORE_MASK_RECORD
    PROF_REC_CORE_MASK = 0x22,
    /// User Call-stack header record.  The data is in \ref PRD_USER_CSS_DATA_RECORD
    PROF_REC_USER_CSS = 0x23,
    /// Kernel Call-stack header record.  The data is in \ref PRD_KERNEL_CSS_DATA_RECORD
    PROF_REC_KERNEL_CSS = 0x24,
    /// Virtual stack header record.  The data is in \ref PRD_VIRTUAL_STACK_RECORD
    PROF_REC_VIRTUAL_STACK = 0x25,
    /// Invalid record.
    PROF_REC_INVALID = 0xFF
} PRD_RECORD_TYPE;
/*****************************************************************************/

/// \def PRD_RECORD_SIZE
/// The record size for every record in the prd file
#define PRD_RECORD_SIZE 40
/// \def PRD_HDR_SIGNATURE
/// ascii for "perf" for m_Signature
#define PRD_HDR_SIGNATURE 0x066726570
/// \def CXL_HDR_VERSION
/// The current m_Version used
#define CXL_HDR_VERSION 0x0006
#define EXTENDED_CSS_HDR_VERSION 0x0007

/// \def CSS_DATA_PER_RECORD
/// The amount of CSS data that will fit into one record
#define CSS_DATA_PER_RECORD 4

/// \def TOPOLOGY_DATA_PER_RECORD
/// The amount of Topology data that will fit into one record
/// Assumes < 255 cores
/// 40 bytes - 1 (record type) = 39 / 3 ( 1 byte core + 1 byte processor +
///     1 byte NUMA node) = 13
#define TOPOLOGY_DATA_PER_RECORD 13

typedef enum
{
    /// File created on 32-bit Linux
    V1_HDR_SRC_LIN32 = 0x0001,
    /// File created on 64-bit Linux
    V1_HDR_SRC_LIN64 = 0x0002,
    /// File created on 32-bit Windows (NT/2000/XP/etc)
    V1_HDR_SRC_WIN32 = 0x0003,
    /// File created on 64-bit Windows
    V1_HDR_SRC_WIN64 = 0x0004,
    /// File created on Windows 98 or Windows ME, no longer supported
    V1_HDR_SRC_WIN98 = 0x0005
} PRD_SOURCE_TYPE;

/// The enumeration used to retrieve data from the __cpuid intrinsic
typedef enum
{
    /// The offset of EAX data
    EAX_OFF = 0,
    /// The offset of EBX data
    EBX_OFF,
    /// The offset of ECX data
    ECX_OFF,
    /// The offset of EDX data
    EDX_OFF,
    /// The number of values in the cpuid array
    MAX_CPUID_VALUE
};

/* ************************************************************************* */
// Configuration records

/// \struct PRD_FILE_HEADER_V3
/// Definition of the header of the profile data file generated by the device
/// driver. This header is filled in and passed down when the prd file name is
/// set, via an ioctl call by the CpuPerfAPI.
/// \note the header is the same size as a data record. The device driver will
/// merely place the header as the first record in the file buffer, and allow
/// it to be written out to disk when the buffer is flushed. The application
/// code reading the file should ignore further header fields found later.
typedef struct
{
    /// Signature field, to verify that this is indeed a data file
    ULONG m_Signature;
    /// Version of the data, for runtime determination - version 4 - 5
    USHORT m_Version;
    /// See \ref PRD_SOURCE_TYPE
    UCHAR m_Source;
    /// Padding to make the record size 40 bytes.
    UCHAR m_Padding0;
    /// Filetime mark for user time mark synchronization
    FILETIME m_StartMark;
    /// version 4: time stamp counter value for user time mark synchronization
    /// version 5: The system tick count for user time mark synchronization
    ULONG64 m_TickStamp;
    /// The full clock speed in MHz
    ULONG m_speed;
    /// The count of active cores on the system at profile time
    USHORT m_coreCount;
    /// The number of sockets in the system.
    UCHAR m_socketCount;
    /// Padding to make the record size 40 bytes.
    UCHAR m_Padding1;
    /// The count of resources in the profiled system
    /// Note that if the \ref MAX_RESOURCE_TYPE ever grows past 8, this
    /// structure needs to change
    UCHAR m_resourceCounts[8];
} PRD_FILE_HEADER_V3;

/** \struct PRD_CPUINFO_RECORD
    System data for record of type \ref PROF_REC_CPUID
*/
typedef struct
{
    /// The record type is \ref PROF_REC_CPUID
    UCHAR m_RecordType;
    /// Padding to make the record size 40 bytes.
    UCHAR m_Padding1;
    /// Padding to make the record size 40 bytes.
    USHORT m_Padding2;
    /// Which function cpuid was called with
    ULONG m_CpuId_function;
    /// The \ref EAX_OFF, \ref EBX_OFF, \ref ECX_OFF, and \ref EDX_OFF result
    /// of the cpuid function call
    int aRegisterInfo[MAX_CPUID_VALUE];
    /// Padding to make the record size 40 bytes.
    ULONG64 m_Padding3[2];
} PRD_CPUINFO_RECORD;

/** \struct
    Complete configuration record for type \ref PROF_REC_WEIGHT
*/
typedef struct
{
    /// The record type is \ref PROF_REC_WEIGHT
    UCHAR m_RecordType;
    /// Which core the configuration is for.
    UCHAR m_Core;
    /// The indexes of the weights of the base resources
    /// Note that if the \ref MAX_RESOURCE_TYPE ever grows, this needs to
    /// be updated,
    /// 6 will be MAX_RESOURCE_TYPE defined in PcoreInterface.h
    UCHAR m_indexes[6];
    /// If not 0, then this is the 1st weight record of a new buffer.  The count
    /// is the number of records in the buffer (including this one)
    UCHAR m_BufferRecordCount;
    /// The weights of resources, with the respective indexes being the base
    /// resource of each type.  Thus EVENT_CTR 2's weight is at
    /// m_Weights[m_indexes[EVENT_CTR] + 2]
    UCHAR  m_Weights[31];
} PRD_WEIGHT_RECORD;

/** \struct PRD_EVENT_CTR_CONFIG_RECORD
    Configuration data for record of type \ref PROF_REC_EVTCFG
*/
typedef struct
{
    /// The record type is \ref PROF_REC_EVTCFG
    UCHAR m_RecordType;
    /// Which performance event counter the configuration was for.
    UCHAR m_EventCounter;
    /// The number of cores the mask applies to.
    /// 0 means all cores used.  > 63 means that the extended core masks will be defined in the following
    ///  record of type \ref PRD_CORE_MASK_RECORD
    USHORT m_CoreMaskCount;
    /// Sampling mode flags 1=counting mode, 2=load initial value
    ULONG m_ModeFlags;
    /// The control value for an EVENT_CTR configuration
    ULONG64 m_EventSelReg;
    /// The system tick count for user time mark synchronization
    ULONG64 m_TickStamp;
    /// The count of the event
    ULONG64 m_EventCount;
    /// Which cores 0-63 the configuration is for
    ULONG64 m_CoreMask;
} PRD_EVENT_CTR_CONFIG_RECORD;

/** \struct PRD_APIC_CONFIG_RECORD
    Configuration data for record of type \ref PROF_REC_TIMERCFG
*/
typedef struct
{
    /// The record type is \ref PROF_REC_TIMERCFG
    UCHAR m_RecordType;
    /// Padding to make the record size 40 bytes.
    UCHAR  m_Padding0;
    /// The number of cores the mask applies to.
    /// 0 means all cores used.  > 63 means that the extended core masks will be defined in the following
    ///  record of type \ref PRD_CORE_MASK_RECORD
    USHORT m_CoreMaskCount;
    /// Padding to make the record size 40 bytes.
    ULONG m_Padding2;
    /// Which cores 0-63 the configuration is for
    ULONG64 m_CoreMask;
    /// The system tick count for user time mark synchronization
    ULONG64 m_TickStamp;
    /// Padding to make the record size 40 bytes.
    ULONG64 m_Padding4;
    /// The profile granularity in microseconds.
    ULONG64 m_TimerGranularity;
} PRD_APIC_CONFIG_RECORD;

/** \struct PRD_IBS_CONFIG_RECORD
    Configuration data for record of type \ref PROF_REC_IBSCFG
*/
typedef struct
{
    /// The record type is \ref PROF_REC_IBSCFG
    UCHAR m_RecordType;
    /// Padding to make the record size 40 bytes.
    UCHAR m_Padding0;
    /// The number of cores the mask applies to.
    /// 0 means all cores used.  > 63 means that the extended core masks will be defined in the following
    ///  record of type \ref PRD_CORE_MASK_RECORD
    USHORT m_CoreMaskCount;
    /// Padding to make the record size 40 bytes.
    ULONG m_Padding2;
    /// Which cores 0-63 the configuration is for
    ULONG64 m_CoreMask;
    /// The system tick count for user time mark synchronization
    ULONG64 m_TickStamp;
    /// The control value for an IBS_OP configuration
    ULONG64 m_IbsOpCtl;
    /// The control value for an IBS_FETCH configuration
    ULONG64 m_IbsFetchCtl;
} PRD_IBS_CONFIG_RECORD;

/** \struct PRD_MISSED_DATA_RECORD
    Data for record of type \ref PROF_REC_MISSED, giving the missed counts, 1
    per previous configuration record.
    These records must be the last in the file.
*/
typedef struct
{
    /// The record type is PROF_REC_MISSED
    UCHAR m_RecordType;
    /// The number of the core the interrupt occurred on.
    UCHAR m_Core;
    /// The configuration that is being reported on: \ref PROF_REC_TIMERCFG,
    /// \ref PROF_REC_EVTCFG, or \ref PROF_REC_IBSCFG
    UCHAR m_MissedType;
    /// Padding to make the record size 40 bytes.
    UCHAR m_Padding0;
    /// The count of data records missed due to insufficient buffers.
    ULONG m_MissedCount;
    /// Either m_TimerGranularity, m_EventCtl, or m_IbsOpCtl, allowing the user
    /// to match the configuration to the missed count
    ULONG64 m_ConfigCtl;
    /// The system tick count for user time mark synchronization
    ULONG64 m_TickStamp;
    /// If the m_MissedType is PROF_REC_IBSCFG
    ULONG64 m_IbsFetchMaxCnt;
    /// The count of data records missed due to insufficient buffers.
    ULONG m_MissedFetchCount;
    /// Padding to make the record size 40 bytes.
    ULONG m_Padding1;
} PRD_MISSED_DATA_RECORD;

/// \def PID_DATA_PER_RECORD
/// The amount of CSS data that will fit into one record
#define PID_DATA_PER_RECORD 4

/** \struct PRD_PID_CONFIG_RECORD
    Configuration data for record of type \ref PROF_REC_PIDCFG
    If a child PID is loaded during the profile, another record will be added
    amongst the data
*/
typedef struct  // _TrcPIDConfRecord
{
    /// The record type is \ref PROF_REC_PIDCFG
    UCHAR m_RecordType;
    /// Padding to make the record size 40 bytes.
    UCHAR m_Padding0;
    /// Padding to make the record size 40 bytes.
    USHORT      m_Padding1;
    /// Padding to make the record size 40 bytes.
    ULONG       m_Padding2;
    /// Array of PID values
    ULONG64     m_PID_Array[PID_DATA_PER_RECORD];
} PRD_PID_CONFIG_RECORD;

/* ************************************************************************* */
// Data records

/** \struct PRD_APIC_DATA_RECORD
    Data for record of type \ref PROF_REC_TIMER
*/
typedef struct
{
    /// The record type is \ref PROF_REC_TIMER
    UCHAR m_RecordType;
    /// The number of the core the interrupt occurred on.
    UCHAR m_Core;
    /// For 64-bit mode, Bits: [0] eflags/rx, [1] eflags/vm, [2] CS/L,
    /// [3] CS/D, [15:4] reserved 0
    USHORT m_StatusBits;
    /// Padding to make the record size 40 bytes.
    ULONG m_Padding0;
    /// The EIP or RIP of the interrupted thread
    ULONG64 m_InstructionPointer;
    /// The system tick count for user time mark synchronization
    ULONG64 m_TickStamp;
    /// The process id of the interrupted thread
    ULONG64 m_ProcessHandle;
    /// The thread id of the interrupted thread
    ULONG64 m_ThreadHandle;
} PRD_APIC_DATA_RECORD;

/** \struct PRD_EVENT_CTR_DATA_RECORD
    Data for record of type \ref PROF_REC_EVENT
*/
typedef struct
{
    /// The record type is \ref PROF_REC_EVENT
    UCHAR m_RecordType;
    /// The number of the core the interrupt occurred on.
    UCHAR m_Core;
    /// Which performance event counter the configuration was for.
    UCHAR m_EventCounter;
    /// Upper bits (11-8) of the event select
    UCHAR m_EventSelectHigh;
    /// Lower 32-bit part of the msr control value for the EVENT_CTR
    /// configuration
    ULONG m_EventCtl;
    /// The EIP or RIP of the interrupted thread
    ULONG64 m_InstructionPointer;
    /// The system tick count for user time mark synchronization
    ULONG64 m_TickStamp;
    /// The process id of the interrupted thread
    ULONG64 m_ProcessHandle;
    /// The thread id of the interrupted thread
    ULONG64 m_ThreadHandle;
} PRD_EVENT_CTR_DATA_RECORD;

/** \struct PRD_IBS_FETCH_DATA_BASIC_RECORD
    Data for record of type \ref PROF_REC_IBS_FETCH_BASIC or record 1 of 2 for
    type \ref PROF_REC_IBS_FETCH_EXT.  Any invalid data has a value is 0,
    whether by not being available or not requested.
*/
typedef struct
{
    /// The record type is \ref PROF_REC_IBS_FETCH_BASIC (1 of 1) or \ref
    /// PROF_REC_IBS_FETCH_EXT (1 of 2) if the immediate next record is a
    /// \ref PRD_IBS_FETCH_DATA_EXT_RECORD
    UCHAR m_RecordType;
    /// The number of the core the interrupt occurred on.
    UCHAR m_Core;
    /// Padding to make the record size 40 bytes.
    USHORT m_Padding0;
    /// IbsFetchCtl[63:32] (Fetch event flags/cycle counts)
    ULONG m_IbsFetchCtlHigh;
    /// IbsFetchLinerAddress[63:0]
    ULONG64 m_IbsFetchLinAd;
    /// The system tick count for user time mark synchronization
    ULONG64 m_TickStamp;
    /// The process id of the interrupted thread
    ULONG64 m_ProcessHandle;
    /// The thread id of the interrupted thread
    ULONG64 m_ThreadHandle;
} PRD_IBS_FETCH_DATA_BASIC_RECORD;

/** \struct PRD_IBS_FETCH_DATA_EXT_RECORD
    2nd record of data for record of type \ref PROF_REC_IBS_FETCH_EXT
*/
typedef struct  // _TrcIbsFetchRecordExt
{
    /// IbsFetchPhysicalAddress[63:0]
    ULONG64 m_IbsFetchPhysAd;
    /// IbsItlbRefillLat[15:0]
    USHORT m_IbsFetchCtlExtd;
    /// Padding to make the record size 40 bytes.
    USHORT m_Padding1;
    /// Padding to make the record size 40 bytes.
    ULONG32 m_Padding2;
    /// Padding to make the record size 40 bytes.
    ULONG64 m_Padding[3];
} PRD_IBS_FETCH_DATA_EXT_RECORD ;

/** \struct PRD_IBS_OP_DATA_BASIC_RECORD
    Data for record 1 of 2 for type \ref PRD_IBS_OP_DATA_EXT_RECORD
    Any invalid data has a value is 0, whether by not being available or not
    requested.
*/
typedef struct
{
    /// The record type is \ref PROF_REC_IBS_OP_BASIC (1 of 1) or \ref
    /// PROF_REC_IBS_OP_EXT (1 of 2) if the immediate next record is a
    /// \ref PRD_IBS_OP_DATA_EXT_RECORD
    UCHAR m_RecordType;
    /// The number of the core the interrupt occurred on.
    UCHAR m_Core;
    /// IbsOpData [37:32] Branch/return/resync flags
    USHORT m_IbsOpDataHigh;
    /// IbsOpData[31:0] Macro-op retire cycle counts
    ULONG m_IbsOpDataLow;
    /// IbsOpRip[63:0] Macro-op linear address
    ULONG64 m_IbsOpRip;
    /// The system tick count for user time mark synchronization
    ULONG64 m_TickStamp;
    /// The process id of the interrupted thread
    ULONG64 m_ProcessHandle;
    /// The thread id of the interrupted thread
    ULONG64 m_ThreadHandle;
} PRD_IBS_OP_DATA_BASIC_RECORD;

/** \struct PRD_IBS_OP_DATA_EXT_RECORD
    Data for record 2 of 2 for type \ref PRD_IBS_OP_DATA_EXT_RECORD
*/
typedef struct
{
    /// IbsOpData3[63:0] Load/store flags/latency
    ULONG64 m_IbsOpData3;
    /// IbsDcLinearAdddress[63:0] IBS DC Linear Data Address
    ULONG64 m_IbsDcLinAd;
    /// IbsDcPhysicalAddress[63:0] IBS DC Physical Data Address
    ULONG64 m_IbsDcPhyAd;
    /// Padding to make the record size 40 bytes.
    ULONG m_Padding0;
    /// Padding to make the record size 40 bytes.
    USHORT m_Padding1;
    /// IbsOpData4[0] Load Resync
    UCHAR m_IbsOpData4;
    /// IbsOpData2[5:0] Northbridge data
    UCHAR m_IbsOpData2;
    /// Ibs branch target address
    ULONG64 m_IbsBrTarget;
} PRD_IBS_OP_DATA_EXT_RECORD;

/// \struct PRD_CSS_DATA_RECORD
///    Record data for record of type \ref PROF_REC_CSS or \ref PROF_REC_POTENTIAL_CSS
///    For a depth > 4, there should be multiple records in a row.  This record
///    applies to the preceding data record.  The 1st address is the location of
///    the call-stack
///
///    This struct is OBSOLETE. It has been replaced by PRD_KERNEL_CSS_DATA_RECORD and PRD_USER_CSS_DATA_RECORD
typedef struct
{
    union
    {
        struct
        {
            /// The record type is \ref PROF_REC_CSS or \ref PROF_REC_POTENTIAL_CSS
            UCHAR m_RecordType;
            /// The number of the core the interrupt occurred on.
            UCHAR m_Core;
            /// Padding to make the record size 40 bytes.
            USHORT m_Padding0;
            /// Padding to make the record size 40 bytes.
            ULONG m_Padding1;
        };

        struct
        {
            ULONG64           : 16;
            /// The offsets in the stack (limited to 4096).
            ULONG64 m_Offset0 : 12;
            ULONG64 m_Offset1 : 12;
            ULONG64 m_Offset2 : 12;
            ULONG64 m_Offset3 : 12;
        };

        ULONG64 m_Offsets;
    };

    /// The call-stack addresses, in order.
    ULONG64 m_CallStack[CSS_DATA_PER_RECORD];
} PRD_CSS_DATA_RECORD, *PPRD_CSS_DATA_RECORD;


#define PRD_CSS_DATA_RECORD_INIT_OFFSET(record, idx, offset) \
    (record)->m_Offsets |= (((ULONG64)(offset)) & 0xFFFULL) << (16 + ((idx) * 12))

#define PRD_CSS_DATA_RECORD_SET_OFFSET(record, idx, offset) \
    (record)->m_Offsets = ((record)->m_Offsets & ~(0xFFFULL << (16 + ((idx) * 12)))) \
                          | ((((ULONG64)(offset)) & 0xFFFULL) << (16 + ((idx) * 12)))

#define PRD_CSS_DATA_RECORD_GET_OFFSET(record, idx) \
    ((ULONG)((record)->m_Offsets >> (16 + ((idx) * 12)))) & 0xFFFUL


typedef struct
{
    /// The record type is \ref PROF_REC_KERNEL_CSS
    UCHAR m_RecordType;
    UCHAR m_Is64Bit;

    /// The depth of the call-stack. This is the number of elements in the array
    /// m_CallStack32 or m_CallStack64, as appropriate.
    /// If the depth exceeds the size of the array in this struct then the consecutive
    /// PRD records will contain nothing but the excess elements, without the other
    /// struct members (e.g. m_RecordType, etc.).
    USHORT m_Depth;

    ULONG m_Padding;

    union
    {
        /// The 32-bit call-stack addresses, in order.
        ULONG32 m_CallStack32[8];
        /// The 64-bit call-stack addresses, in order.
        ULONG64 m_CallStack64[4];
    };
} PRD_KERNEL_CSS_DATA_RECORD, *PPRD_KERNEL_CSS_DATA_RECORD;


typedef struct
{
    /// The record type is \ref PROF_REC_USER_CSS
    UCHAR m_RecordType;
    UCHAR m_Is64Bit;

    USHORT m_Depth;

    ULONG m_Padding;

    /// The system tick counts for user time mark synchronization
    /// This is the time when the sample was performed that triggered this CSS collection.
    /// Triggering can happen when system timer expires (TBP) or monitored event occurred (EBP, IBS).
    /// CSS collection is performed asynchronously sometime after the sample was taken. During the
    /// CSS collection, additional timer/events may occur and they will all be associated with this
    /// single CSS sample.
    ULONG64 m_TickStampBegin;

    /// The time when the CSS collection has ended (currently performed in an APC)
    ULONG64 m_TickStampEnd;

    /// The process id of the interrupted thread
    ULONG64 m_ProcessHandle;
    /// The thread id of the interrupted thread
    ULONG64 m_ThreadHandle;
} PRD_USER_CSS_DATA_RECORD, *PPRD_USER_CSS_DATA_RECORD;


typedef struct
{
    ULONG64 m_PrevUserCssRecordOffset;

    union
    {
        /// The 32-bit call-stack addresses, in order.
        ULONG32 m_CallStack32[8];
        /// The 64-bit call-stack addresses, in order.
        ULONG64 m_CallStack64[4];
    };
} PRD_USER_CSS_DATA_EXT_RECORD, *PPRD_USER_CSS_DATA_EXT_RECORD;


typedef struct
{
    /// The record type is \ref PROF_REC_VIRTUAL_STACK
    UCHAR m_RecordType;

    UCHAR m_Padding1;

    USHORT m_ValuesCount;

    /// The offset of the EBP or RBP value from the stack pointer
    ULONG m_FramePointerOffset;

    ULONG64 m_StackPointer;

    ULONG32 m_Values[6];
} PRD_VIRTUAL_STACK_RECORD, *PPRD_VIRTUAL_STACK_RECORD;


/** \struct PRD_PAUSE_RESUME_DATA_RECORD
    a record of a pause or resume of type \ref PROF_REC_RESUME or \ref
    PROF_REC_PAUSE
*/
typedef struct
{
    /// The record type is \ref PROF_REC_RESUME or \ref PROF_REC_PAUSE
    UCHAR m_RecordType;
    /// The number of the core the interrupt occurred on.
    UCHAR m_Core;
    /// Padding to make the record size 40 bytes.
    USHORT m_Padding0;
    /// Padding to make the record size 40 bytes.
    ULONG m_Padding1;
    /// Padding to make the record size 40 bytes.
    ULONG64 m_Padding2;
    /// The system tick count for user time mark synchronization
    ULONG64 m_TickStamp;
    /// Padding to make the record size 40 bytes.
    ULONG64 m_Padding[2];
} PRD_PAUSE_RESUME_DATA_RECORD;

/** \struct PRD_TOPOLOGY_RECORD
    a record of core topologies of type \ref PROF_REC_TOPOLOGY
    where index = 0..TOPOLOGY_DATA_PER_RECORD
    The value in m_Core should be strictly increasing
*/
typedef struct
{
    /// The record type is \ref PROF_REC_TOPOLOGY
    UCHAR m_RecordType;
    /// The core at index i
    UCHAR m_Core[TOPOLOGY_DATA_PER_RECORD];
    /// The processor id for the logical core at index i
    UCHAR m_Processor[TOPOLOGY_DATA_PER_RECORD];
    /// The NUMA node id for the logical core at index i
    UCHAR m_Node[TOPOLOGY_DATA_PER_RECORD];
} PRD_TOPOLOGY_RECORD;

/** \struct PRD_HEADER_EXT_RECORD
    System data for record of type \ref PROF_REC_EXT_HEADER
*/
typedef struct
{
    /// The record type is \ref PROF_REC_EXT_HEADER
    UCHAR m_RecordType;
    /// Padding to make the record size 40 bytes.
    UCHAR m_Padding1;
    /// Padding to make the record size 40 bytes.
    USHORT m_Padding2;
    /// Padding to make the record size 40 bytes.
    ULONG m_Padding3;
    /// The high-resolution frequency as reported by KeQueryPerformanceCounter
    ULONG64 m_HrFrequency;

    ULONG64 m_LastUserCssRecordOffset;
    /// Padding to make the record size 40 bytes.
    ULONG64 m_Padding4[2];
} PRD_HEADER_EXT_RECORD;

/** \struct PRD_CORE_MASK_RECORD
    System data for record of type \ref PROF_REC_CORE_MASK
*/
typedef struct
{
    /// The record type is \ref PROF_REC_CORE_MASK
    UCHAR m_RecordType;
    /// Padding to make the record size 40 bytes.
    UCHAR m_Padding1;
    /// The starting index of the core mask
    ///  For example, 1 means that \ref m_CoreMasks[0] contains the masks for cores 64-127
    USHORT m_StartingIndex;
    /// Padding to make the record size 40 bytes.
    ULONG m_Padding3;
    /// The core masks
    ULONG64 m_CoreMasks[4];
} PRD_CORE_MASK_RECORD;

#endif // _CPUPROFDATA_H
