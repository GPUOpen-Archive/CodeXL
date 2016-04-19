//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuUtilization.h
///
//==================================================================================

#ifndef _CPUUTILIZATION_H_
#define _CPUUTILIZATION_H_

// CPU Utilization
#define MAX_PID_SUPPORT     8
#include <AMDTCpuProfilingControl/inc/CpuProfilingControlDLLBuild.h>
#include "CpuUtilizationRecords.h"

#define MONITOR_CPU     0x1
#define MONITOR_MEM     0x2

class CP_CTRL_API CpuUtilization
{
public:
    enum
    {
        evCU_OK = 0,
        evCU_NOT_INITIALIZED,
        evCU_FAILED_INITIALIZE,
        evCU_FILE_OPEN_FAILURE,
        evCU_Initialization_FAILURE,
        evCU_MAX_PID_ERROR,
        evCU_FAILED_OPEN_PROCESS_HANDLE,
        evCU_INVALID_PATH,
        evCU_ALREADY_IN_MONITORING_STATUS,
        evCU_NOT_IN_MONITORING
    };

    CpuUtilization();
    ~CpuUtilization();

    unsigned int Initialize(unsigned int monitorFlag = MONITOR_CPU | MONITOR_MEM);

    // The interval is ms; by default the interval is 500 ms;
    unsigned int SetInterval(unsigned int interval);

    // Monitor the process CPU utilization
    unsigned int AddPid(unsigned int newPid, bool bMonitorThread);

    // start the CPU utilization, memory consumption data collection.
    //caller setup the whole file path, name and extension.
    unsigned int StartCPUUtilMonitor(wchar_t* logFileName);

    unsigned int StopCPUUtilMonitor();
    unsigned int Pause();
    unsigned int Resume();

    unsigned int GetInterval() { return m_interval;};

    // clean up default configuration.
    void SetDefaultConfig();


    // This is entry point for monitoring thread.
    // DOn't call this from caller site.
    void MonitorThreadEntry();

private:
    void InitUtilHeader();

    // enents used for synchronization during data collection.
    bool CreateEvents();
    void ClearEvents();

    // collect data (cpu utilization, memory consumpation) for each snapshot.
    unsigned int CollectData();

    // make sure the current data buffer has enought space for records,
    // If not, it will flush the data into file first.
    void EnsureDataBufferAvailable(unsigned int dataSizeRequired);

    unsigned int LogTimeStampRecord();
    unsigned int LogSysMemoryUsage();
    unsigned int LogCoreCPUUtilization();
    unsigned int LogProcessCPUUtilization();
    unsigned int LogProcessMemoryConsumption();

    unsigned int GetCoreNumber();

    // data buffer is used to log data temporarily, in order to avoid many file IO.
    bool AllocateDataBuffer();
    void ReleaseDataBuffer();

    gtUInt64 GetFileTimeDelta(const FILETIME& oldTime, const FILETIME& newTime);

private:
    // used for configuration
    unsigned int    m_interval;

    // used to specify CPU utilizaiton or memory consumption options
    unsigned int m_monitorFlag;

    // processes info - pid
    unsigned int    m_pidArray[MAX_PID_SUPPORT];
    // process handle;
    HANDLE          m_procHandles[MAX_PID_SUPPORT];

    // to avoid overhead, I don't monitor thread
    // -Lei 12/03/2010.
    unsigned int    m_pidMonitorThread[MAX_PID_SUPPORT];

    FILETIME        m_pidftKernelTime[MAX_PID_SUPPORT];
    FILETIME        m_pidftUserTime[MAX_PID_SUPPORT];

    unsigned int    m_pidCnt;

    // used for data logging before writing to output file;
    unsigned char*  m_dataBuffer;
    // datasize: mnay bytes used in the data buffer;
    unsigned int    m_dataSize;

    // m_sysPerfData: used to query system performance info.
    unsigned char*  m_sysPerfData;
    unsigned char*  m_perfDataPrev;
    unsigned char*  m_perfDataCurr;

    // system times
    FILETIME        m_prevFTSysKernel;
    FILETIME        m_prevFTSysUser;

    // output file and header
    FILE*           m_fileStream;
    CpuUtilHeader   m_hdr;

    // worker thread and event to terminate thread;
    HANDLE          m_wokerThread;
    HANDLE          m_ThreadExitEvents;

    CRITICAL_SECTION m_CriticalSection;
    unsigned int    m_StatusFlag;
};

#endif // _CPUUTILIZATION_H_
