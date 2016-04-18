//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfHeader.h
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingRawData/inc/Linux/CaPerfHeader.h#6 $
// Last checkin:   $DateTime: 2016/04/14 01:08:15 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569054 $
//=====================================================================

#ifndef _CAPERFHEADER_H_
#define _CAPERFHEADER_H_

#include <stdint.h>
#include <sys/types.h>
#include <linux/perf_event.h>
// #include <linux/bitmap.h>

#include <linux/types.h>
#include <sys/utsname.h>
#include <limits.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

static const char caperf_magic_str[] = "_CAPERF_";
#define CAPERF_MAGIC            (*(gtUInt64*)caperf_magic_str)
#define CAPERF_MAGIC_ADDR       ((gtUInt64*)caperf_magic_str)

// We can also use MAJOR and MINOR VERSION NUMBER - 16 bits each and
// construct a 32-bit VERSION from these MAJOR and MINOR version numbers
#define CAPERF_VERSION      2

//This is defined to allow the "fake" timer workaround for the software timer sampling in kernels < 3.0
// since the call-stack is not correctly collected
#define ENABLE_FAKETIMER

typedef struct caperf_file_header
{
    gtUInt64  magic;
    gtUInt32  version;          // caperf_version
    gtUInt32  ca_version;       // CodeXL version

    // Section header table contains the list of section headers for various sections
    gtUInt32  section_hdr_off;  // offset of section header table in file;
    gtUInt32  section_hdr_size; // size of the section header table in file;

    gtUInt64  filler[5];        // future purpose
} caperf_file_header_t;


// A typical caperf file will be like
//
//         +----------------+
//         |   file hdr     |  - contains caperf header
//         +----------------+
//         |  section hdr   |  - contains section header table entries
//         |   table        |    (type, offset, length)
//         +----------------+
//         |  section -     |
//         |  run-info      |  - section contains runinfo (optional)
//         +----------------+
//         |  cpuinfo       |  - section contains cpuinfo
//         +----------------+
//         |  event config  |  - sections contains event configuration
//         +----------------+
//         |  target pid    |  - sections contains target pids.
//         +----------------+
//         |  translation   |
//         |      info      |  - sections contains translation info
//         +----------------+
//         |  kernel info   |  - sections contains kernel info
//         +----------------+
//         |  sample data   |  - section contains PERF sample records
//         +----------------+
//         | counter data   |  - section contains PMU counter values
//         +----------------+


// Section types
//
// Not all sections are mandatory...
//
//  - PERF Sampling mode requires - EVENT_ATTRIBUTE, SAMPLE_ID & SAMPLE_DATA sections
//  - PERF Counting mode requires - EVENT_ATTRIBUTE & COUNTER_DATA sections
//  - PERF Sampling and Sampling mode requires -
//        EVENT_ATTRIBUTE, SAMPLE_ID, SAMPLE_DATA & COUNTER_DATA sections
//
// Optional sections are - RUN_INFO and ??
//

#define CAPERF_MAX_SECTIONS    10

typedef enum caperf_section_type
{
    CAPERF_SECTION_RUN_INFO          = (1 << 0), // profile run info
    CAPERF_SECTION_CPU_INFO          = (1 << 1),
    CAPERF_SECTION_EVENT_ATTRIBUTE   = (1 << 2),
    CAPERF_SECTION_EVENT_ID          = (1 << 3), // ? sample ids
    CAPERF_SECTION_SAMPLE_DATA       = (1 << 4),
    CAPERF_SECTION_COUNTER_DATA      = (1 << 5),
    CAPERF_SECTION_TARGET_PIDS       = (1 << 6),
    CAPERF_SECTION_FAKE_TIMER_INFO   = (1 << 7),
    CAPERF_SECTION_TOPOLOGY          = (1 << 8),
    CAPERF_SECTION_DYNAMIC_PMU_TYPES = (1 << 9),
} caperf_section_type_t;


typedef struct caperf_section_hdr
{
    gtUInt32  type;       // caperf_section_type_t
    gtUInt32  misc;       // _UNUSED_ field
    gtUInt64  offset;
    gtUInt64  size;
} caperf_section_hdr_t;


// Run info Section
//
//   contains details about the machine, processor, OS
//   name/path of the application launched
//   start and end time of the profile
//
//   TBD: Do we need to save about
//      -  measurement type - SWP or Per-Process
//      -  application launched
//      -  list of pids profiled
//      -  list of CPUs profiled, if cpulist is specified
//
//
#define CA_NAME_MAX 64

typedef struct caperf_section_runinfo
{
    gtUInt64  start_time; // time_t ?
    gtUInt64  end_time;   // time_t ?

    // The can contain null terminated 64 byte length strings (65 chars)
    char      sysname[CA_NAME_MAX];    // OS implementation - Linux
    char      releases[CA_NAME_MAX];   // Release - 2.6.38-8-generic
    char      version[CA_NAME_MAX];    // version - Ubuntu
    char      nodename[CA_NAME_MAX];   // machine name - capike01
    char      machine[CA_NAME_MAX];    // hw type - x86_64

    char      filename[CA_NAME_MAX];   // launched application - should be the last field
} caperf_section_runinfo_t;


// cpu details
//
// TBD: Do we need vendor info ?
//
#define CA_CPUID_VALUE_MAX  4

typedef struct caperf_section_cpuinfo
{
    gtUInt32  function;
    gtUInt32  value[CA_CPUID_VALUE_MAX]; // when CA_CPUID_VALUE_MAX gets modified, ensure
    // this struct is 64-bit aligned
    gtUInt32  filler[3];
} caperf_section_cpuinfo_t;


// cpu topology info
typedef struct caperf_section_topology
{
    gtUInt32  core_id;
    gtUInt16  processor_id;
    gtUInt16  numa_node_id;
    gtUInt32  filler[2];
} caperf_section_topology_t;


// dynamic pmu types
typedef struct caperf_section_pmu_types
{
    gtUInt32  ibs_fetch;
    gtUInt32  ibs_op;
    gtUInt32  filler[2];
} caperf_section_pmu_types_t;


// caperf_section_evtcfg
//   Event configuration section
//
// Each profiling event (either sampling or counting event) will have
// a record of type 'caperf_section_evtcfg_t' in this section -
// CAPERF_SECTION_EVENT_ATTRIBUTE
//
// Total size of this section is in the corresponding section header entry;
// Each record is a variable record; size of each record is
//  sizeof(perf_event_attr) + sizeof(name) + (nbr_sample_ids * sizeof(gtUInt64)
//

#define CAPERF_MAX_EVENT_NAME   64

typedef struct caperf_section_evtcfg
{
    perf_event_attr  event_config;  // current size =  9 * sizeof(gtUInt64)

    //start index in caperf_section_evt_misc table
    // this caperf_section_evt_misc table contains sample ids..
    gtUInt32  start_idx;
    gtUInt32  number_entries;      // in caperf_section_evt_misc;

    // name of the predefined events supported by PERF.
    char      name[CAPERF_MAX_EVENT_NAME];
} caperf_section_evtcfg_t;


// caper_section_sample_id
//
//   sample ids of various events are stored in this section
//   Sample ids are per fd; fds are event/per-thread or per-cpu.
typedef struct caperf_section_sample_id
{
    gtUInt64  sample_id;
    gtUInt32  cpuid;
    gtUInt32  misc; // ?
} caperf_section_sample_id_t;


// caperf_section_pmusample_data
//   PMU profile data from PERF subsystem
//
//   This section will dump the data from PERF in the same format
//   See "perf_event_type" and the respecective structures for various
//   PERF_RECORD_* types
//

// Notes:-
//   If there are more than one sampling event, we need to know the
//   sampling event that caused the sample. For that we need PERF_SAMPLE_ID.
//   sample-id is a unique per fd id. Its is not in the fixed location.
//   It depends on perf_event_attr.sample_type field.
//
//   We need to have a global sample_type - which is applicable to all the
//   sampling-events; Only then we can identify the PERF_SAMPLE_ID field
//   from the PERF_RECORD_SAMPLE record. Then find the corresponding
//   sampling-event from that unique sample-id.
//
//   Hence.. current PERF_RECORD_SAMPLE structure does NoT allow us to have
//   per event sample_type...
//


// caperf_section_pmuctr_data
//
typedef struct caperf_section_pmuctr_data
{
    gtUInt32  type;

    gtUInt32  pid;
    gtUInt32  tid;
    gtUInt32  cpuid;

    gtUInt64  nbr_event_ctrs;
    union
    {
        gtUInt64  values[5];
        struct
        {
            gtUInt64  event_id;
            gtUInt64  event_config;
            gtUInt64  cntr_value;
            gtUInt64  time_enabled;
            gtUInt64  time_running;
        };
    };
} caperf_section_pmu_ctrdata_t;


typedef struct caperf_section_fake_timer
{
    gtUInt32 numCpu;
    gtUInt32 timerNanosec;
    gtUInt32* timerFds;
    gtUInt32* fakeTimerFds;
} caperf_section_fake_timer_t;


// FIXME
// perf.data has something called BUILD_ID, TRACE_INFO; - need to look into
// these stuff

//
// perf data records stuff
//

#define PERF_MMAP_FILENAME_MAX_LEN    264

struct ca_mmap_event
{
    struct perf_event_header header;
    gtUInt32 pid, tid;
    gtUInt64 start;
    gtUInt64 len;
    gtUInt64 pgoff;
    char filename[PERF_MMAP_FILENAME_MAX_LEN];
};

struct ca_comm_event
{
    struct perf_event_header header;
    gtUInt32 pid, tid;
    char comm[16];
};

struct ca_fork_event
{
    struct perf_event_header header;
    gtUInt32 pid, ppid;
    gtUInt32 tid, ptid;
    gtUInt64 time;
};

struct ca_ip_callchain
{
    gtUInt64 nr;
    gtUInt64 ips[0];
};

struct ca_sample_data
{
    gtUInt64 ip;
    gtUInt32 pid, tid;
    gtUInt64 time;
    gtUInt64 addr;
    gtUInt64 id;
    gtUInt64 stream_id;
    gtUInt64 period;
    gtUInt32 cpu;
    gtUInt32 raw_size;
    void* raw_data;
    struct ca_ip_callchain* callchain;
};

typedef union ca_event_union
{
    struct perf_event_header header;
    struct ca_mmap_event mmap;
    struct ca_comm_event comm;
    struct ca_fork_event fork;
} ca_event_t;

#define MAX_EVENT_NAME 64
struct ca_perf_trace_event_type
{
    gtUInt64 event_id;
    char name[MAX_EVENT_NAME];
};

#endif  // _CAPERFHEADER_H_
