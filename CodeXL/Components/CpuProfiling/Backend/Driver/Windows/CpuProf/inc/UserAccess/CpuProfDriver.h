//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfDriver.h
/// \brief  CpuProf Driver IOCTL interaction definitions
///
/// \section returns Return Values
/// The return values for the IOCTL calls are available through GetLastError(),
///     if the DeviceIoControl () call failed:
/// ERROR_ACCESS_DENIED (STATUS_ACCESS_DENIED, STATUS_ALREADY_COMMITTED) The
///     client id was not valid, no client id was available, or the
///     configuration was already set
/// ERROR_INVALID_PARAMETER (STATUS_INVALID_PARAMETER) One of the given
///     configuration values was invalid
/// ERROR_GEN_FAILURE (STATUS_UNSUCCESSFUL) The profiler was not started
/// ERROR_INSUFFICIENT_BUFFER (STATUS_BUFFER_TOO_SMALL) The output buffer was
///     too small
/// ERROR_BAD_LENGTH (STATUS_INFO_LENGTH_MISMATCH) The input size was wrong
/// ERROR_INVALID_USER_BUFFER (STATUS_INVALID_USER_BUFFER) The buffer size
///     provided was insufficient
/// ERROR_NOT_READY (STATUS_DEVICE_NOT_READY) The profile client state was not
///     ready
/// ERROR_NOT_ENOUGH_MEMORY (STATUS_NO_MEMORY) There is not enough available
///     kernel memory
/// ERROR_NO_SYSTEM_RESOURCES (STATUS_INSUFFICIENT_RESOURCES) There were not
///     enough resources
/// ERROR_BUSY (STATUS_DEVICE_BUSY) The profiler was already started
/// ERROR_RESOURCE_TYPE_NOT_FOUND (STATUS_RESOURCE_TYPE_NOT_FOUND) No
///     configurations were added or the requested configuration was not added
/// ERROR_FILE_NOT_FOUND (STATUS_NO_SUCH_FILE) No output files were set
/// ERROR_NOT_SUPPORTED (STATUS_NOT_SUPPORTED) The IOCTL was not supported
/// ERROR_FILE_INVALID (STATUS_FILE_INVALID) The output file could not be written
/// ERROR_LOCK_VIOLATION (STATUS_LOCK_NOT_GRANTED) Some other process or service
///     has the hardware counters locked
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/Driver/Windows/CpuProf/inc/UserAccess/CpuProfDriver.h#10 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

#ifndef _CPUPROFDRIVER_H
#define _CPUPROFDRIVER_H

#include <windef.h>
#include <stdlib.h>

#include <WinDriverStackWalker\Include\UserAccess\CodeRange.h>
#include "CpuProfSharedObj.h"
#include "CpuProfVersion.h"

#pragma warning(push)
#pragma warning(disable:4201) // nameless struct/union
#pragma warning(disable:4091) //'typedef ': ignored on left of '' when no variable is declared
#include "PrdRecords.h"
#pragma warning(pop)

#ifndef CTL_CODE
    #pragma message ("CTL_CODE	undefined. Include winioctl.h")
#endif

/// \def SIZE_OF_32_ADDR The length of a 32-bit address
#define SIZE_OF_32_ADDR 4

/// \def _MAX_DRIVE_NAME hack for the /device/harddrivevolume1 name path
/// complication
#define _MAX_DRIVE_NAME 50

/// These enumerated masks cover the possible range of states the driver is in
typedef enum
{
    /// The current client is not configured for anything
    STATE_NOT_CONFIGURED    = 0,
    /// The output file has been set for the next profile
    STATE_OUTPUT_FILE_SET   = 1 << 0,
    /// The call-stack sampling has been set for the next profile
    STATE_CSS_SET           = 1 << 1,
    /// The process id filter has been set for the next profile
    STATE_PID_FILTER_SET    = 1 << 2,
    /// At least one event configuration has been added for the next profile
    STATE_EBP_SET           = 1 << 3,
    /// The timer configuration has been set for the next profile
    STATE_TBP_SET           = 1 << 4,
    /// The ibs configuration has been set for the next profile
    STATE_IBS_SET           = 1 << 5,
    /// The profile was started and is in process
    STATE_PROFILING         = 1 << 6,
    /// The profile was started and is currently paused
    STATE_PAUSED            = 1 << 7,
    /// The profile is currently stopping
    STATE_STOPPING          = 1 << 8
} CPUPROF_STATE;


//  CpuProf's error codes
typedef enum
{
    /// An error occurred
    PROF_ERROR = -1,
    /// No errors occurred
    PROF_SUCCESS = 0x00,
    /// An argument was invalid
    PROF_INVALID_ARG = 0x01,
    /// Unable to create a file with that name
    PROF_INVALID_FILENAME_FORMAT = 0x03,
    /// Unable to write to the file
    PROF_FILE_WRITE_ERROR = 0x05,
    /// The configuration is not available
    PROF_INVALID_OPERATION = 0x06,
    /// Memory is not available for buffer allocation
    PROF_BUFFER_NOT_ALLOCATED = 0x08,
    /// The client application crashed during a profile
    PROF_CRITICAL_ERROR = 0xDEAD
} CPUPROF_ERROR_CODES;

/// \def IOCTL_GET_VERSION returns the version encoded in a ULONG64 *
/// [63-56]: pcore major
/// [55-48]: pcore minor
/// [47-32]: pcore build
/// [31-24]: caprof major, [23-16]: caprof minor, [15-0]: caprof build
/// Use InvokeOut
/// Handled by \ref IoctlGetVersionHandler
#define IOCTL_GET_VERSION \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \struct TIMER_PROPERTIES Holds an APIC timer configuration
typedef struct
{
    /// The registered client id
    ULONG ulClientId;
    /// Interval of timer specified in 0.1 mS
    ULONG ulGranularity;
    /// The count of cores for the mask passed in.  0 means all cores
    ULONG ulCoreMaskCount;
    /// The array of masks of cores for which the configuration is valid
    /// The array size is (\ref ulCoreMaskCount / 64)
    ULARGE_INTEGER ullCpuMask;
    /// The return status
    ULONG ulStatus;
} TIMER_PROPERTIES, *PTIMER_PROPERTIES;

/// \def IOCTL_SET_TIMER_PROPERTIES sets the timer properties to profile when
/// the profiler starts.
/// Use InvokeInOut and \ref TIMER_PROPERTIES
/// Handled by \ref IoctlSetTimerPropertiesHandler
#define IOCTL_SET_TIMER_PROPERTIES \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_GET_TIMER_PROPERTIES is a test function to get the currently
/// set timer properties.
/// Use InvokeInOut and \ref TIMER_PROPERTIES
/// Handled by \ref IoctlGetTimerPropertiesHandler
#define IOCTL_GET_TIMER_PROPERTIES \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \struct PROFILER_PROPERTIES Holds information about the current profile
typedef struct
{
    /// The registered client id
    ULONG ulClientId;
    /// A mask of the profile’s current states: \ref CPUPROF_STATE
    ULONG ulProfilerState;
    /// A handle to the event created with CreateEvent for notification if the
    /// profile has to abort, for example: running out of disk space to write
    ULARGE_INTEGER hAbort;
    /// Ioctl return status
    ULONG ulStatus;
} PROFILER_PROPERTIES, *PPROFILER_PROPERTIES;

/// \def IOCTL_START_PROFILER starts the profile.
/// Use InvokeInOut and \ref PROFILER_PROPERTIES
/// Handled by \ref IoctlStartProfilerHandler
#define IOCTL_START_PROFILER \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_STOP_PROFILER stops the profile.  It will also clear the state
/// of the clientId.
/// Use InvokeInOut and ULONG client id in, ULONG profile status out
/// Handled by \ref IoctlStopProfilerHandler
#define IOCTL_STOP_PROFILER \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)


#pragma warning( disable : 4200 )
/// \struct EVENT_PROPERTIES Holds information about one event configuration
typedef struct
{
    /// The registered client id
    ULONG ulClientId;
    /// The count of cores for the mask passed in.  0 means all cores
    ULONG ulCoreMaskCount;
    /// The array of masks of cores for which the configuration is valid
    /// The array size is (\ref ulCoreMaskCount / 64)
    ULARGE_INTEGER ullCpuMask;
    /// Which performance counter to use, validity depends on the system
    ULONG ulCounterIndex;
    /// The encoded event configuration - \ref PERF_CTL, \ref NB_PERF_CTL or
    /// \ref L2I_PERF_CTL
    ULONG64 ullEventCfg;
    /// The starting count for a counting event or the interval of events between
    /// interrupts
    ULONG64 ullEventCount;
    /// The return status
    ULONG ulStatus;
} EVENT_PROPERTIES, *PEVENT_PROPERTIES;
#pragma warning( default : 4200 )

/// \def IOCTL_ADD_EVENT_PROPERTIES adds the event configuration to be profiled
/// when the profiler starts.
/// Use InvokeInOut and \ref EVENT_PROPERTIES
/// Handled by \ref IoctlSetEventPropertiesHandler
#define IOCTL_ADD_EVENT_PROPERTIES \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_GET_EVENT_PROPERTIES is a test function to get the currently
/// added event confifuration properties.
/// Use InvokeInOut and an array of \ref EVENT_PROPERTIES
/// Handled by \ref IoctlGetEventPropertiesHandler
#define IOCTL_GET_EVENT_PROPERTIES \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80A, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_REGISTER_CLIENT obtains the client id used for interactions,
/// both IOCTL and shared memory
/// Use InvokeOut and a ULONG *
/// Handled by \ref IoctlRegisterClient
#define IOCTL_REGISTER_CLIENT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80B, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_UNREGISTER_CLIENT frees the client id used for interactions,
/// it will immediately stop and finish any current profile
/// Use InvokeIn and a ULONG *
/// Handled by \ref IoctlUnegisterClient
#define IOCTL_UNREGISTER_CLIENT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80C, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \struct OUTPUT_FILE_DESCRIPTOR Holds the file names for the next profile
typedef struct
{
    /// The registered client id
    ULONG ulClientId;
    /// Pointer to path\filename of output PRD file, a NULL-terminated WCHAR_T
    /// string
    ULARGE_INTEGER uliPathName;
    /// Size of path\filename string
    ULONG ulPathSize;
    /// Pointer to path\filename of the temporary TI file, a NULL-terminated
    /// WCHAR_T string
    ULARGE_INTEGER uliTempTiPathName;
    /// Size of path\filename string
    ULONG ulTempTiSize;
    /// The return status
    ULONG ulStatus;
} OUTPUT_FILE_DESCRIPTOR, *POUTPUT_FILE_DESCRIPTOR;

/// \def IOCTL_SET_OUTPUT_FILE sets the file path and prd header for the
/// sampling output files
/// Use InvokeInOut and \ref OUTPUT_FILE_DESCRIPTOR
/// Handled by \ref IoctlSetOutputFileHandler
#define IOCTL_SET_OUTPUT_FILE   \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x811, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_PAUSE_PROFILER pauses the profile.
/// A faster method is to set the appropriate (1 << clientId) shared memory bit
/// or to set the shared memory key
/// Use InvokeInOut and ULONG client id in, ULONG profile state out
/// Handled by \ref IoctlPauseProfilerHandler
#define IOCTL_PAUSE_PROFILER \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x812, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_RESUME_PROFILER resumes the profile.
/// A faster method is to clear the appropriate (1 << clientId) shared memory
/// bit or to clear the shared memory key
/// Use InvokeInOut and ULONG client id in, ULONG profile state out
/// Handled by \ref IoctlResumeProfilerHandler
#define IOCTL_RESUME_PROFILER   \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x813, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_GET_OUTPUT_FILE is a test function to get the currently
/// set file paths and prd header for the output files
/// Use InvokeInOut and \ref OUTPUT_FILE_DESCRIPTOR
/// Handled by \ref IoctlGetOutputFileHandler
#define IOCTL_GET_OUTPUT_FILE   \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x816, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_GET_RECORD_COUNT is a test function to get the count of records
/// that have been written to the file
/// Use InvokeInOut, with In the ULONG * pClientID and Out ULONG *pCount
/// Handled by \ref IoctlGetRecordCountHandler
#define IOCTL_GET_RECORD_COUNT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x81C, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_GET_PROFILER_PROPERTIES is a test function to get the current
/// state and properties of the client of the profiler
/// Use InvokeInOut and \ref PROFILER_PROPERTIES
/// Handled by \ref IoctlGetProfilerPropertiesHandler
#define IOCTL_GET_PROFILER_PROPERTIES \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x826, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// Inclusively, which processor mode to perform call-stack sampling for
/// At least one type is required to obtain data
typedef enum
{
    /// Get user mode call-stack samples
    CSS_USER_MODE = 0x1,
    /// Get kernel mode call-stack samples
    CSS_KERNEL_MODE = 0x2
} CSS_SAMPLING_MODE;


/// The maximum number of CSS records accommodated in a 4K page ((102 - 2 data - 1 weight change - 1 CSS header) = 98)
#define MAX_CSS_RECORDS (102 - 2 - 1 - 1)
/// The maximum number of CSS values accommodated in a 4K page
#define MAX_CSS_VALUES  (MAX_CSS_RECORDS * CSS_DATA_PER_RECORD)

/// \def InitialCodeRangeBufferSize initial size of CSS code range buffer in bytes
#define InitialCodeRangeBufferSize 16384

/// \struct CSS_PROPERTIES Holds the settings for call-stack sampling
typedef struct
{
    /// The registered client id
    ULONG ulClientId;
    /// The initial PID to grab CSS data from
    ULONG64 ulTargetPid;
    /// How often to track the CSS information
    ULONG ulCSSInterval;
    /// Whether to track kernel or user mode CSS information
    UCHAR ucTargetSamplingMode;
    /// Whether to capture and build the virtual stack
    BOOLEAN bCaptureVirtualStack;
    /// Reserved (currently only for padding)
    USHORT usReserved;
    /// The depth of the call stack to descend, Maximum value is MAX_CSS_VALUES
    ULONG ulCSSDepth;
    /// The address length in bytes of the PID, 4 for 32-bit, 8 for 64-bit
    ULONG ulAddressLen;
    /// The number of code ranges, whose information is available in aCodeRangeInfo
    ULONG ulNumCodeRange;
    /// The array of code ranges for the PID, saving space for size as first entry
    CSS_CodeRange aCodeRangeInfo[(InitialCodeRangeBufferSize / sizeof(CSS_CodeRange)) - 1];
    /// The return status
    ULONG ulStatus;
} CSS_PROPERTIES, *PCSS_PROPERTIES;

/// \def IOCTL_SET_CSS_PROPERTIES sets the call-stack sampling properties for
/// the profile when the profiler starts.
/// Use InvokeInOut and \ref CSS_PROPERTIES
/// Handled by \ref IoclSetCSSPropertiesHandler
#define IOCTL_SET_CSS_PROPERTIES    \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x827, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_GET_CSS_PROPERTIES is a test function to get the CSS
/// settings
/// Use InvokeInOut and \ref CSS_PROPERTIES
/// Handled by \ref IoclGetCSSPropertiesHandler
#define IOCTL_GET_CSS_PROPERTIES    \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x828, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// This enum represents the different values of IBS_FETCH data that are
/// available. Minor name changes from PcoreInterface.h \ref IBS_FETCH_MASK
typedef enum
{
    /// IBS Fetch Physical Address Register
    CXL_FETCH_PHYSICAL_ADDR_MASK = 0x02,
    /// IBS Fetch Control Extended Register
    CXL_FETCH_CONTROL_EXTD_MASK = 0x04
} CXL_IBS_FETCH_MASK;

/// This enum represents the different values of IBS_OP data that are
/// available.  Minor name changes from pcore-interface.h \ref IBS_OP_MASK
typedef enum
{
    /// IBS Op Data 2 Register
    CXL_OP_DATA_2_MASK = 0x04,
    /// IBS Op Data 3 Register
    CXL_OP_DATA_3_MASK = 0x08,
    /// IBS Op DC Linear Address Register
    CXL_OP_DC_LINEAR_ADDR_MASK = 0x10,
    /// IBS Op DC Physical Address Register
    CXL_OP_DC_PHYSICAL_ADDR_MASK = 0x20,
    /// IBS Op Branch Target Address Register
    CXL_OP_BR_ADDR_MASK = 0x40,
    /// IBS Op Data 4 Register
    CXL_OP_DATA_4_MASK = 0x80
} CXL_IBS_OP_MASK;

/// \def MIN_IBS_CYCLE_COUNT Experimentally defined minimal cycle count.  With
/// a lower count, it will crash due to excessive interrupts.  (10,000)
#define MIN_IBS_CYCLE_COUNT 0x2710  //
/// \def MAX_IBS_CYCLE_COUNT BKDG defined maximum cycle count.
#define MAX_IBS_CYCLE_COUNT 0xFFFFF /* A 20 bit counter */
/// \def MAX_IBS_EXT_COUNT BKDG defined maximum cycle count, when the extended
/// IBS Op count is enabled.
#define MAX_IBS_EXT_COUNT 0x7FFFFFF

/// \struct IBS_PROPERTIES Holds the settings for the IBS configuration
typedef struct
{
    /// The registered client id
    ULONG ulClientId;
    /// TRUE to perform an IBS fetch profile
    BOOLEAN bProfileFetch;
    /// TRUE to perform an IBS op profile
    BOOLEAN bProfileOp;
    /// TRUE for an IBS op profile to count dispatched ops, FALSE to count
    /// clock cycles
    BOOLEAN bOpDispatch;
    /// Maximum value of Fetch Counter
    ULONG ulIbsFetchMaxCnt;
    /// Data values to read for IBS_FETCH, requiring
    /// \ref PROF_REC_IBS_FETCH_EXT records
    CXL_IBS_FETCH_MASK fetchDataMask;
    /// Maximum value of op counter
    ULONG ulIbsOpMaxCnt;
    /// Data values to read for IBS_OP, requiring
    /// \ref PRD_IBS_OP_DATA_EXT_RECORD records
    CXL_IBS_OP_MASK opDataMask;
    /// The count of cores for the mask passed in.  0 means all cores
    ULONG ulCoreMaskCount;
    /// The array of masks of cores for which the configuration is valid
    /// The array size is (\ref ulCoreMaskCount / 64)
    ULARGE_INTEGER ullCpuMask;
    /// The return status
    ULONG ulStatus;
} IBS_PROPERTIES, *PIBS_PROPERTIES;

/// \def IOCTL_SET_IBS_PROPERTIES sets the IBS profiling when the profiler
/// starts.
/// Use InvokeInOut and \ref IBS_PROPERTIES
/// Handled by \ref IoctlSetIbsPropertiesHandler
#define IOCTL_SET_IBS_PROPERTIES    \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x829, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_GET_IBS_PROPERTIES is a test function to get the IBS
/// settings
/// Use InvokeInOut and \ref IBS_PROPERTIES
/// Handled by \ref IoctlGetIbsPropertiesHandler
#define IOCTL_GET_IBS_PROPERTIES    \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x830, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def MAX_PID_COUNT
/// max number of process ids the driver can filter for
#define MAX_PID_COUNT 256

/// \struct PID_PROPERTIES Holds the settings for the PID filter
typedef struct
{
    /// The registered client id
    ULONG ulClientId;
    /// 0-terminated array of PIDS
    ULONG64 ullPidArray[MAX_PID_COUNT];
    /// Whether to add child processes to the filter
    BOOLEAN bAddChildrenToFilter;
    /// The return status
    ULONG ulStatus;
} PID_PROPERTIES, *PPID_PROPERTIES;

/// \def IOCTL_SET_PID_PROPERTIES sets the PID filter for when the profiler
/// starts.
/// Use InvokeInOut and \ref PID_PROPERTIES
/// Handled by \ref IoctlSetPIDPropertiesHandler
#define IOCTL_SET_PID_PROPERTIES    \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x831, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_GET_PID_PROPERTIES is a test function to get the PID filters
/// Use InvokeInOut and \ref PID_PROPERTIES
/// Handled by \ref IoctlGetPIDPropertiesHandler
#define IOCTL_GET_PID_PROPERTIES    \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x832, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \struct COUNT_PROPERTIES Holds the configuration to read the count
typedef struct
{
    /// The registered client id
    ULONG ulClientId;
    /// The core to get the count from
    ULONG64 ullCore;
    /// Which performance counter to use
    ULONG ulCounterIndex;
    /// The encoded event configuration; PERF_CTL, NB_PERF_CTL or L2I_PERF_CTL
    ULONG64 ullEventCfg;
    /// The returned count for a counting event
    ULONG64 ullEventCount;
    /// The return status
    ULONG ulStatus;
} COUNT_PROPERTIES, *PCOUNT_PROPERTIES;

/// \def IOCTL_GET_EVENT_COUNT will retrieve the count for the specified
/// performance counter and configuration for a profile
/// Use InvokeInOut and \ref COUNT_PROPERTIES
/// Handled by \ref IoctlGetEventCountHandler
#define IOCTL_GET_EVENT_COUNT   \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x833, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_CLEAR_PROFILER clears the profiler settings for the given client
/// Use InvokeIn and a ULONG *
/// Handled by \ref IoctlClearProfiler
#define IOCTL_CLEAR_PROFILER \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x834, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \struct OVERHEAD_PROPERTIES Holds the overhead data for a client
typedef struct
{
    /// The cumulative count of cycles taken during the reaper thread while
    /// writing data to the file
    ULONG64 reaperOverhead;
    /// The cumulative count of cycles taken during the sampling interrupt
    /// callback
    ULONG64 samplingOverhead;
    /// The cumulative count of cycles taken during pcore's scheduling of ALL
    /// cores and clients
    ULONG64 pcoreSchedulingOverhead;
    /// The cumulative count of cycles taken during pcore's interrupt handler
    /// of ALL cores and clients
    ULONG64 pcoreInterruptOverhead;
    /// The return status
    ULONG ulStatus;
} OVERHEAD_PROPERTIES, *POVERHEAD_PROPERTIES;

/// \def IOCTL_GET_OVERHEAD is a test function to get the overhead of the
/// drivers, both CpuProf personality client and Pcore
/// Use InvokeInOut and \ref OVERHEAD_PROPERTIES
/// Handled by \ref IoctlGetOverhead
#define IOCTL_GET_OVERHEAD  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x835, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \struct RESOURCE_AVAILABILITY Holds the availability of resources
typedef struct
{
    /// The mask of event counters that are available
    ULONG pmcAvailable;
    /// The mask of Northbridge event counters that are available
    ULONG nbAvailable;
    /// The mask of L2I event counters that are available
    ULONG l2iAvailable;
} RESOURCE_AVAILABILITY, *PRESOURCE_AVAILABILITY;

/// \def IOCTL_GET_AVAILABILITY is a function to get the availability of the
/// resource counters by the drivers
/// Use InvokeOut and \ref RESOURCE_AVAILABILITY
/// Handled by \ref IoctlGetAvailability
#define IOCTL_GET_AVAILABILITY  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x836, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// The types of task info records
typedef enum
{
    /// Process creation record
    MONITOR_PROC_CREATE = 0x01,
    /// Process deletion record
    MONITOR_PROC_DELETE = 0x02,
    /// Module load record
    MONITOR_IMAGE_LOAD = 0x04,
    /// Thread creation record
    MONITOR_THREAD_CREATE = 0x08,
    /// Thread deletion record
    MONITOR_THREAD_DELETE = 0x10
} MONITOR_RECORD_TYPES;

/// \struct PRD_PAUSE_RESUME_DATA_RECORD a record of a pause or resume of type
/// \ref PROF_REC_RESUME or \ref PROF_REC_PAUSE
typedef struct  _TaskInfoRecord
{
    /// The record type is
    UCHAR m_RecordType;
    /// The number of the processor the event occurred on
    UCHAR m_Core;
    /// Purely PADDING because of byte alignment
    UCHAR m_Padding[6];
    /// The caller thread's process id
    ULONG64 m_ProcessHandle;
    /// Starting linear address
    ULONG64 m_StartAddress;
    /// Image size
    ULONG64 m_Size;
    /// The system tick count for time mark synchronization
    ULONG64 m_TickStamp;
    /// Path name to module, wide char string
    wchar_t m_PathName[_MAX_PATH + _MAX_DRIVE_NAME];
} TASK_INFO_RECORD;

#endif // _CPUPROFDRIVER_H

