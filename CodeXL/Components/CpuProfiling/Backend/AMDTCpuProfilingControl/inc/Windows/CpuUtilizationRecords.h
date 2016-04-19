//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuUtilizationRecords.h
///
//==================================================================================

#ifndef _CPUUTILIZATIONRECORDS_H_
#define _CPUUTILIZATIONRECORDS_H_

#define CUT_HEADER_SIGNATURE "AMDCPUUT"
const unsigned int CPUUTILIZATION_VERSION = 0x01;

#define CU_TIMESTAMP_RECORD     0x01
#define CORE_CPU_UTILIZATION    0x02
#define SYSTEM_MEMORY_USAGE     0x03
#define PROCESS_CPU_UTILIZATION 0x04
#define PROCESS_MEMORY_USAGE    0x05

struct CpuUtilHeader
{
    char            signature[8];   // this always be AMDCPUUT
    unsigned int    version;        // unsigned char version of CPU Utilization file
    unsigned int    num_cores;      // number of cpu processor cores
    unsigned int    num_Process;    // number of processes monitored
    unsigned int    num_Records;    // gtUInt32 number of overall records;
    unsigned int    phy_memory;     // total physical memory in KB;
    unsigned int    vir_memroy;     // total virtual memory in KB;
    gtUInt64        highResTime;    // high resolution time mark
    gtUInt64        systemTick;     // gettickcount; stored in 64bit value;
    gtUInt64        highResFreq;    // high resolution frequency
    gtUInt64        lastHrTime;     // last time mark
};

struct CU_TimestampRecord
{
    gtUInt16        recordType;     // CU_TIMESTAMP_RECORD  for core cpu utilization
    gtUInt16        padding1;       //
    unsigned int    padding2;
    gtUInt64        hrTimeStamp;    // high resolution time mark
};


struct CoreUtilRecord
{
    gtUInt16    recordType;     // CORE_CPU_UTILIZATION for core cpu utilization
    gtUInt16    numBytesFollow; // number of bytes following the record;
};
// unsigned int will be used to represent core cpu utilization (percentage).
// Although the BYTE will do the job. But don't want spend time on alignment.
// -Lei

struct ProcessMemUsage
{
    gtUInt16        recordType;     // PROCESS_MEMORY_USAGE for processs memory usage
    gtUInt16        padding;        //
    unsigned int    processID;      // process id
    unsigned int    workingSetSize; // current working set size, in bytes.
    unsigned int    privateUsage ;  // private memroy (in byte) not shared with other;
};

struct ProcessUtil
{
    gtUInt16        recordType;     // PROCESS_CPU_UTILIZATION for process cpu utilization
    gtUInt16        threadRecCnt;   // number of thread record followed; currently it's 0;
    unsigned int    processID;      // process id
    //  gtUInt64        elapsedTime;    // elapsed time in number of 100-nanosecond;
    unsigned int    procCPUUtil;    // process cpu utilization in percentage;
    unsigned int    padding;
};

// Since this record always follows the process utilization record, I don't put
// process id, time stamp into the structure.
// Note - Due to overhead issue, I don't implement thread cpu utilization in this
struct ThreadUtil
{
    unsigned int    threadID;       // thread id;
    unsigned int    threadUtil;     // thread cpu utilization
};

struct SysMemoryUsage
{
    gtUInt16    recordType;     // SYSTEM_MEMORY_USAGE  for system memory usage
    gtUInt16    memoryUsage;    // a percentage of physical memroy is in use;
};

#endif // _CPUUTILIZATIONRECORDS_H_
