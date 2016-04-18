//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfData.h
///
//==================================================================================

#ifndef _PERFDATA_H_
#define _PERFDATA_H_

#include <AMDTCpuProfilingBackendUtils/3rdParty/linux/perfStruct.h>

#define CA_PERF_RECORD_PATH_MAX 1024

// Task command name length
// From linux-src/include/linux/sched.h
#define TASK_COMM_LEN 16

#define CA_PERF_EV_BASE 0xE000

struct CA_PERF_RECORD_MMAP
{
    u32 pid, tid;
    u64 addr;
    u64 len;
    u64 pgoff;
    char filename[CA_PERF_RECORD_PATH_MAX];
};

struct CA_PERF_RECORD_COMM
{
    u32 pid, tid;
    char comm[TASK_COMM_LEN];
};

/* NOTE:
 * This structure is from perf.h
 */
struct ip_callchain
{
    uint64_t nr;
    uint64_t ips[0];
};


struct read_format
{
    u64 value;
    u64 time_enabled;  // PERF_FORMAT_TOTAL_TIME_ENABLED
    u64 time_running;  // PERF_FORMAT_TOTAL_TIME_RUNNING
    u64 id;            //PERF_FORMAT_ID
};

/* NOTE:
 * This structure should be the same as
 * struct perf_sample in event.h
 */
struct CA_PERF_RECORD_SAMPLE
{
    u64 ip;
    u32 pid, tid;
    u64 time;
    u64 addr;
    u64 id;
    u64 stream_id;
    u64 period;
    struct read_format values;
    u32 cpu;
    u32 raw_size;
    void* raw_data;
    struct ip_callchain* callchain;
};

struct CA_PERF_RECORD_FORK_EXIT
{
    u32 pid, ppid;
    u32 tid, ptid;
    u64 time;
};

struct CA_PERF_RECORD_READ
{
    u32 pid, tid;
    struct read_format values;
};

#endif //_PERFDATA_H_
