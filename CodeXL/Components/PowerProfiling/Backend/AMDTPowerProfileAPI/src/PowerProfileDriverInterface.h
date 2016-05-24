//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfileDriverInterface.h
///
//==================================================================================

#ifndef _POWERPROFILEDRIVERINTERFACE_H_
#define _POWERPROFILEDRIVERINTERFACE_H_

#include <AMDTDefinitions.h>
#include <AMDTPowerProfileInternal.h>
#include <AMDTRawDataFileHeader.h>
#include <AMDTPwrProfDriver.h>
#include <list>
#include <TaskInfoInterface.h>
#include <CpuProfilingTranslationDLLBuild.h>
#include <AMDTOSWrappers/Include/osApplication.h>

#define AMDT_PWR_STR_SYSTEM_IDLE "System Idle Process"
#define AMDT_PWR_STR_SYSTEM "System"
#define ERROR_READING_PROCESS_NAME "Unknown process"
#define ERROR_READING_PROCESS_PATH "Unable to read path"

#ifndef _LINUX
    #include <AMDTDriverControl/inc/DriverControl.h>
#endif

using namespace std;

enum DriverCommandType
{
    DRV_CMD_TYPE_IN,
    DRV_CMD_TYPE_OUT,
    DRV_CMD_TYPE_IN_OUT
};

// DriverSignalInfo: Signal information
typedef struct DriverSignalInfo
{
    AMDTUInt32 m_fill;
} DriverSignalInfo;

typedef void(*DRV_SIGINFO_CB)(DriverSignalInfo*);

// SharedBuffer: Hold the shared buffer information
typedef struct SharedBuffer
{
    AMDTUInt32 m_processedOffset;
    AMDTUInt32 m_size;
    AMDTUInt32 m_currentOffset;
    AMDTUInt32 m_consumedOffset;
    AMDTUInt32 m_maxValidOffset;
    AMDTUInt8* m_pBuffer;
} SharedBuffer;

// EventCfg: Driver event to indicate the data avialability
typedef struct EventCfg
{
    HANDLE         m_hdl;
    DRV_SIGINFO_CB m_eventCb;
    bool           m_isActive;
} EventCfg;

// OpenPowerProfileDriver: Open power profile driver
AMDTResult OpenPowerProfileDriver();

// RegisterClientWithDriver: Register a client with the driver
// Only one client is allowed at this point of time
AMDTResult RegisterClientWithDriver(AMDTUInt32* pClientId);

// GetPowerDriverClientId: provide the client id
AMDTResult GetPowerDriverClientId(AMDTUInt32* pClientId);

// IsPowerDriverOpened: check if driver is already open
bool IsPowerDriverOpened(void);

// CommandPowerDriver: driver ioctl command interface
AMDTResult CommandPowerDriver(DriverCommandType cmdType,
                              AMDTUInt32 cmd,
                              void* pInData,
                              AMDTUInt32 inLen,
                              void* pOutData,
                              AMDTUInt32 outlen,
                              void* pResult);

// DriverSignalInitialize: Driver sends signal based on data buffer availability
AMDTResult DriverSignalInitialize(DRV_SIGINFO_CB fnDrvSignalInfo, AMDTUInt32 samplingPeriod);

// DriverSignalClose: Close the signal handler
AMDTResult DriverSignalClose();

// InitializeSharedBuffer: Create and initialize shared buffer between driver and user spce
AMDTResult InitializeSharedBuffer();

// ReleaseSharedBuffer: Release  and unmap the shared buffer created
// between driver and user spce
AMDTResult ReleaseSharedBuffer();

// FlushDriverSignal: Send the signal to collect remaining data from driver
AMDTResult FlushDriverSignal();

// PrepareInitialProcessList: Prepare the initial process list names
AMDTResult PrepareInitialProcessList(list<ProcessName>& list);

// GetProcessNameFromPid: Get process name from a given PID
bool GetProcessNameFromPid(AMDTPwrProcessInfo* pInfo);


#endif //_POWERPROFILEDRIVERINTERFACE_H_

