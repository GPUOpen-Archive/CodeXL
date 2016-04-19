//=====================================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc. All rights reserved.
//
/// \file $File: $
/// \version $Revision: $
/// \brief
//
//=====================================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=====================================================================
#ifndef _PWRPROFINTERNAL_H
#define _PWRPROFINTERNAL_H
#include <ntifs.h>
#include <stdlib.h>
#include "..\..\..\..\..\..\..\amdpcore\include\pcoreinterface.h"
#include <AMDTAccessPmcData.h>
#include <AMDTPwrProfDriver.h>
#include <AMDTRawDataFileHeader.h>
#include <AMDTSharedBufferConfig.h>

struct ClientData;
struct CoreData;

// TODO: Need to check where following definitions are used
/// \def NT_DEVICE_NAME The device object name string
#define PWRPROF_NT_DEVICE_NAME L"\\Device\\AmdtPwrProf0"
/// \def DOS_DEVICE_NAME The symbolic link name string
#define PWRPROF_DOS_DEVICE_NAME L"\\??\\AmdtPwrProf0"

///Exclusive state definitions used for synchronization
typedef enum
{
    ///When the synchronization object is currently available
    SYNCH_AVAILABLE = 0,
    ///When the reaper for the client is currently writing data
    REAPER_WORKING,
    ///An application has registered with the client
    CLIENT_REGISTERED
};

/// \struct CONFIGURATION Holds the information for one configuration on
/// one core
typedef struct PcoreConfig
{
    ///Core id for which configuration is activated
    uint32 m_coreId;
    ///What type of resource does this configuration apply to
    PCORERESOURCETYPES m_type;
    ///The resource counter (mostly only applies to EVENT_CTR, NB_CTR and L2I_CTR)
    uint32 m_resourceId;
    ///The configuration to profile
    PCORE_CONFIGURATION m_cfg;
} PcoreConfig;

typedef struct OsClientCfg
{
    HANDLE       m_pcoreReg;
    PFILE_OBJECT m_userFileObj;
} OsClientCfg;

/// \struct OsCoreCfgData Holds all the profile settings and state for a
/// single client.
typedef struct OsCoreCfgData
{
    PcoreConfig m_pcoreCfg;
} OsCoreCfgData;

/// \struct PWRPROF_DEV_EXTENSION Holds all the driver state information and
/// extends the device
typedef struct _DEVICE_EXTENSION
{
    ///Saves the driver device object
    PDEVICE_OBJECT pDeviceObject;

    ///The array of clients, to allow for multiple apps simultaneously
    ///profiling
    ClientData* m_pClient;

    ///Obtained from \ref PcoreGetResourceCount
    ULONG resourceCounts[MAX_RESOURCE_TYPE];
    ///The maximum number of resources of any one type
    ULONG maxResources;
    ///The maximum configurations for any one client
    ULONG maxConfigs;

    // number of CPUs on system
    ULONG coreCount;

    ///Whether the driver can be unloaded
    ULONG CanUnload;

    PKEVENT m_userSpaceEvent;

} PWRPROF_DEV_EXTENSION, *PPWRPROF_DEV_EXTENSION;


// ***************************************************************************
// [MajorFunction] Type Function Prototypes

//  ==========================================================================
/// PwrProf driver entry point.  Called when it's first loaded.
///
/// \param[in] pDriverObject Pointer to the driver object.
/// \param[in] RegistryPath The registry path string
///
/// \return Whether the driver was successfully loaded.  If not, \ref
/// PwrProfUnload is not called.
///
// ===========================================================================
namespace PowerProf
{
DRIVER_INITIALIZE DriverEntry;
}

//  ==========================================================================
/// PwrProf driver unloaded.  All allocated memory and resources are freed.
///
/// \param[in] pDriverObject Pointer to the driver object.
///
// ===========================================================================
DRIVER_UNLOAD PwrProfUnload;

//  ==========================================================================
/// PwrProf device creation.  Unused
///
/// \param[in] pDeviceObject Pointer to the device object.
/// \param[in] Irp Pointer to the current IRP.
///
/// \return STATUS_SUCCESS
///
// ===========================================================================
__drv_dispatchType(IRP_MJ_CREATE) DRIVER_DISPATCH PwrProfCreate;

//  ==========================================================================
/// PwrProf device closing.  Don't need to do anything because \ref
/// PwrProfCleanup will handle any client registrations left open
///
/// \param[in] pDeviceObject Pointer to the device object.
/// \param[in] Irp Pointer to the current IRP.
///
/// \return STATUS_SUCCESS
///
// ===========================================================================
__drv_dispatchType(IRP_MJ_CLOSE) DRIVER_DISPATCH PwrProfClose;

//  ==========================================================================
/// PwrProf IOCTL handler.  Handles all IOCTLS specified in \ref cadddef.h
///
/// \param[in] pDeviceObject Pointer to the device object.
/// \param[in] Irp Pointer to the current IRP.
///
/// \return Return status of the IOCTL handler
///
// ===========================================================================
__drv_dispatchType(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH PwrProfDispatch;

//  ==========================================================================
/// PwrProf cleanup function.  Called when the user application closes the
/// driver or it crashes
///
/// \param[in] pDeviceObject Pointer to the device object.
/// \param[in] Irp Pointer to the current IRP.
///
/// \return Return status of the IOCTL handler
///
// ===========================================================================
__drv_dispatchType(IRP_MJ_CLEANUP) DRIVER_DISPATCH PwrProfCleanup;

//  ==========================================================================
/// The data sample callback from Pcore.  Stores the data in a buffer, if
/// possible
///
/// \param[in] pData Data passed from pcore in regards to a configuration
///
// ===========================================================================
VOID NTAPI SampleDataCallback(PCORE_DATA* pData);

// ***************************************************************************
//  Helper Functions

//  ==========================================================================
/// Checks whether the IOCTL is valid, considering the current state and the
/// system
///
/// \param[in] aResourceCounts The counts of available resources of each type
/// \param[in] ulProfilerState The current client profiling state
/// \param[in] ulIoctlOperation The IOCTL operation
///
/// \return Whether the check succeeded or not
///
// ===========================================================================
NTSTATUS CheckIfValidOperation(IN ULONG* aResourceCounts,
                               IN ULONG ulProfilerState,
                               IN ULONG ulIoctlOperation);

//  ==========================================================================
/// Determines whether the client is valid or not
///
/// \param[in] pDevExt The driver extension
/// \param[in] clientId The client id to check
///
/// \return Whether the client is valid
/// \retval TRUE The client is valid
/// \retval FALSE The client is invalid
///
// ===========================================================================
bool HelpCheckClient(IN PPWRPROF_DEV_EXTENSION pDevExt, uint32 clientId);

//  ==========================================================================
/// Stops any ongoing profile, clears and re-intializes the client.
///
/// \param[in] pDevExt PwrProf driver extension
/// \param[in] pClient The client to unregister
///
/// \return Success of unregistering
///
/// IRQL Level: PASSIVE_LEVEL
// ===========================================================================
NTSTATUS HelpUnregisterClient(ClientData* pClient);

//  ==========================================================================
/// Adds the configurations to the applicable pcore cores and resources.
///
/// \param[in] pClient The client whose configurations go to pcore
///
/// \return Success of adding the configurations
///
/// IRQL Level: PASSIVE_LEVEL
// ===========================================================================
NTSTATUS HelpAddPcoreCfgs(ClientData* pClient, CoreData* pCoreCfg);

//  ==========================================================================
/// Removes the configurations from pcore, saving the counts. We could use
/// PcoreRemoveAllConfigurations, but that won't save the counts when this is
/// called for pausing
///
/// \param[in] pClient The client whose configurations should be removed
///
/// \return Success of removing the configurations
///
/// IRQL Level: PASSIVE_LEVEL
// ===========================================================================
NTSTATUS HelpRemovePcoreCfgs(ClientData* pClient);

//  ==========================================================================
/// Stops profiling, flushes any partial buffers to the files, and clears the
/// previous profile's settings.
///
/// \param[in] pDevExt PwrProf driver extension
/// \param[in] pClient The client whose profile should stop
///
/// \return Success of stopping the profile
///
/// IRQL Level: PASSIVE_LEVEL or
// ===========================================================================
NTSTATUS HelpStopProfile(ClientData* pClient);

//  ==========================================================================
/// Clears the profile settings, except for abort reason, overhead data, and
/// profile record count
///
/// \param[in] pDevExt PwrProf driver extension
/// \param[in] pClient The client whose settings should be cleared
///
/// \return Success of clearing the settings
///
/// IRQL Level: PASSIVE_LEVEL
// ===========================================================================
NTSTATUS HelpClearClient(ClientData* pClient);

int32 AllocateAndInitDataBuffers(ProfileConfig* pProfileCfg, CoreData** ppCoreCfg);

// PwrProfControlMonitoringCb:
void NTAPI PwrProfControlMonitoringCb(PVOID pData, bool isActivated);

// GetPerformanceCounter
void GetPerformanceCounter(uint64* pPerfCounter, uint64* pFrequency);

#endif // _PWRPROFINTERNAL_H
