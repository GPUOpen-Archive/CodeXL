//=============================================================
// (c) 2013 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \version $Revision: $
/// \brief  Power profile control APIs
//
//=============================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=============================================================

#ifndef _PWRPROFIOCTLS_H
#define _PWRPROFIOCTLS_H

/// Gets the hardcoded version number of the driver
//
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_GET_VERSION
//
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlGetVersionHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                IN PIRP pIrp,
                                IN PIO_STACK_LOCATION pIrpStack);


/// Registers a profile client with the PwrProf personality driver
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_REGISTER_CLIENT
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlRegisterClient(IN PPWRPROF_DEV_EXTENSION pDevExt,
                             IN PIRP pIrp,
                             IN PIO_STACK_LOCATION pIrpStack);


/// Add the profile configuration for the specified profile client
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_ADD_PROF_CONFIGS
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlAddProfConfigsHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                    IN PIRP pIrp,
                                    IN PIO_STACK_LOCATION pIrpStack);


/// Starts a profile on the specified profile client
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_START_PROFILER
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlStartProfilerHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                   PIRP pIrp,
                                   PIO_STACK_LOCATION pIrpStack);


/// Pauses any profiling that is going on.
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_PAUSE_PROFILER
///
/// \note This is intended to be used primarily by the counting configuration
/// profiles, as sampling profiles can use the shared memory BOOLEANs faster
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
///
/// \retval STATUS_SUCCESS Success
/// \retval STATUS_ACCESS_DENIED The client id was not valid
/// \retval STATUS_INVALID_PARAMETER The buffers were wrong size
/// \retval STATUS_UNSUCCESSFUL The profiler was not started
///
/// IRQL Level: PASSIVE_LEVEL
NTSTATUS IoctlPauseProfilerHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                   IN PIRP pIrp,
                                   IN PIO_STACK_LOCATION pIrpStack);


/// Resumes any profiling that is going on.
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_RESUME_PROFILER
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
///
/// \retval STATUS_SUCCESS Success
///
/// IRQL Level: PASSIVE_LEVEL
NTSTATUS IoctlResumeProfilerHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                    IN PIRP pIrp,
                                    IN PIO_STACK_LOCATION pIrpStack);


/// Get a buffer containing the file header information for the specified profile client
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_GET_FILE_HEADER_BUFFER
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlGetFileHeaderBufferHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                         IN PIRP pIrp,
                                         IN PIO_STACK_LOCATION pIrpStack);


/// Stops a profile on the specified profile client
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_STOP_PROFILER
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlStopProfilerHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                  PIRP pIrp,
                                  PIO_STACK_LOCATION pIrpStack);


/// Unregisters a profile client with the PwrProf personality driver
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_UNREGISTER_CLIENT
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlUnegisterClient(IN PPWRPROF_DEV_EXTENSION pDevExt,
                              IN PIRP pIrp,
                              IN PIO_STACK_LOCATION pIrpStack);

/// Read/Write to Pci devices
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_ACCESS_PCI
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlAccessPciDevice(IN PPWRPROF_DEV_EXTENSION pDevExt,
                              IN PIRP pIrp,
                              IN PIO_STACK_LOCATION pIrpStack);

/// Read/Write to MSR devices
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_ACCESS_MSR
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlAccessMSR(IN PPWRPROF_DEV_EXTENSION pDevExt,
                        IN PIRP pIrp,
                        IN PIO_STACK_LOCATION pIrpStack);

/// Read/Write to MMIO space
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_ACCESS_MMIO
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlAccessMMIO(IN PPWRPROF_DEV_EXTENSION pDevExt,
                         IN PIRP pIrp,
                         IN PIO_STACK_LOCATION pIrpStack);


/// Set event
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_SET_EVENT
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlSetEvent(IN PPWRPROF_DEV_EXTENSION pDevExt,
                       IN PIRP pIrp,
                       IN PIO_STACK_LOCATION pIrpStack);

#endif //_PWRPROFIOCTLS_H
