//=============================================================
// (c) 2012 Advanced Micro Devices, Inc.
//
/// \author franksw
/// \version $Revision: $
/// \brief  ioctl handler function definitions
//
//=============================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=============================================================
#ifndef _CPUPROF_IOCONTROL_H_
#define _CPUPROF_IOCONTROL_H_
#pragma once

/// Gets the hardcoded version number of the driver
//
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_GET_VERSION
//
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlGetVersionHandler(IN CpuProf::Device* pDevExt,
                                IN PIRP pIrp,
                                IN PIO_STACK_LOCATION pIrpStack);

/// Registers a profile client with the CpuProf personality driver
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_REGISTER_CLIENT
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlRegisterClient(IN CpuProf::Device* pDevExt, IN PIRP pIrp,
                             IN PIO_STACK_LOCATION pIrpStack);

/// Unregisters a profile client with the CpuProf personality driver
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_UNREGISTER_CLIENT
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlUnegisterClient(IN CpuProf::Device* pDevExt, IN PIRP pIrp,
                              IN PIO_STACK_LOCATION pIrpStack);

/// Starts a profile on the specified profile client
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_START_PROFILER
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlStartProfilerHandler(IN CpuProf::Device* pDevExt,
                                   PIRP pIrp, PIO_STACK_LOCATION pIrpStack);

/// Stops a profile on the specified profile client
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_STOP_PROFILER
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlStopProfilerHandler(IN CpuProf::Device* pDevExt,
                                  PIRP pIrp, PIO_STACK_LOCATION pIrpStack);

/// Adds an event configuration to the list to be profiled for the specified
/// profile client
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_ADD_EVENT_PROPERTIES
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlSetEventPropertiesHandler(IN CpuProf::Device* pDevExt,
                                        PIRP pIrp,
                                        PIO_STACK_LOCATION pIrpStack);

/// Gets the current list of event configurations to be profiled for the
/// specified profile client
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_GET_EVENT_PROPERTIES
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlGetEventPropertiesHandler(IN CpuProf::Device* pDevExt,
                                        PIRP pIrp, PIO_STACK_LOCATION pIrpStack);

/// Gets the current value of the counting event configuration during a profile
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_GET_EVENT_COUNT
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
///
/// IRQL Level: PASSIVE_LEVEL
NTSTATUS IoctlGetEventCountHandler(IN CpuProf::Device* pDevExt,
                                   PIRP pIrp, PIO_STACK_LOCATION pIrpStack);

/// This routine allows the user to configure the APIC timer for a profile with
/// resolution >= 0.1 mS for the specified profile client
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_SET_TIMER_PROPERTIES
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlSetTimerPropertiesHandler(IN CpuProf::Device* pDevExt,
                                        IN PIRP pIrp,
                                        IN PIO_STACK_LOCATION pIrpStack);

/// This routine allows the user to retrieve the timer configuration to be
/// profiled for the specified profile client.
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_GET_TIMER_PROPERTIES
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlGetTimerPropertiesHandler(IN CpuProf::Device* pDevExt,
                                        IN PIRP pIrp,
                                        IN PIO_STACK_LOCATION pIrpStack);

/// Set the profile data file names for the specified profile client
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_SET_OUTPUT_FILE
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlSetOutputFileHandler(IN CpuProf::Device* pDevExt,
                                   IN PIRP pIrp,
                                   IN PIO_STACK_LOCATION pIrpStack);

/// Retrieve the profile data file names from the specified profile client
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_GET_OUTPUT_FILE
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlGetOutputFileHandler(IN CpuProf::Device* pDevExt,
                                   IN PIRP pIrp,
                                   IN PIO_STACK_LOCATION pIrpStack);

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
NTSTATUS IoctlPauseProfilerHandler(IN CpuProf::Device* pDevExt,
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
NTSTATUS IoctlResumeProfilerHandler(IN CpuProf::Device* pDevExt,
                                    IN PIRP pIrp,
                                    IN PIO_STACK_LOCATION pIrpStack);

/// Gets the count of sample records from the specified profile
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_GET_RECORD_COUNT
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlGetRecordCountHandler(IN CpuProf::Device* pDevExt,
                                    PIRP pIrp, PIO_STACK_LOCATION pIrpStack);

/// Gets the current state of the profile client
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_GET_PROFILER_PROPERTIES
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlGetProfilerPropertiesHandler(IN CpuProf::Device* pDevExt,
                                           PIRP pIrp, PIO_STACK_LOCATION pIrpStack);

/// Allows the user to configure callstack sampling for a target application.
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_SET_CSS_PROPERTIES
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoclSetCSSPropertiesHandler(IN CpuProf::Device* pDevExt,
                                     IN PIRP pIrp,
                                     IN PIO_STACK_LOCATION pIrpStack);

/// Gets the current call-stack sampling configuration
///
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_GET_CSS_PROPERTIES
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoclGetCSSPropertiesHandler(IN CpuProf::Device* pDevExt,
                                     PIRP pIrp, PIO_STACK_LOCATION pIrpStack);

/// Configures ibs
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_SET_IBS_PROPERTIES
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlSetIbsPropertiesHandler(IN CpuProf::Device* pDevExt,
                                      IN PIRP pIRp,
                                      IN PIO_STACK_LOCATION pIrpStack);

/// Gets the IBS configuration for the profile
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_SET_IBS_PROPERTIES
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlGetIbsPropertiesHandler(IN CpuProf::Device* pDevExt,
                                      PIRP pIrp, PIO_STACK_LOCATION pIrpStack);

/// Configures PID filtering for target applications
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_SET_PID_PROPERTIES
///
/// Note that \ref IoctlSetOutputFileHandler should be called first
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlSetPIDPropertiesHandler(IN CpuProf::Device* pDevExt,
                                      PIRP pIrp, PIO_STACK_LOCATION pIrpStack);

/// Gets the previously configured PID filtering
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_GET_PID_PROPERTIES
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlGetPIDPropertiesHandler(IN CpuProf::Device* pDevExt,
                                      PIRP pIrp, PIO_STACK_LOCATION pIrpStack);

/// Clears all previous settings for the profile
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_CLEAR_PROFILER
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlClearProfiler(IN CpuProf::Device* pDevExt, PIRP pIrp,
                            PIO_STACK_LOCATION pIrpStack);

/// Gets the overhead spent during the last profile (in approximate cycle
/// count duration)
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_GET_OVERHEAD
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlGetOverhead(IN CpuProf::Device* pDevExt, PIRP pIrp,
                          PIO_STACK_LOCATION pIrpStack);

/// Gets the availability of the resource counters
/// Handles the IRP_MJ_DEVICE_CONTROL IoControlCode IOCTL_GET_AVAILABILITY
///
/// \param[in] pDevExt The driver device extension
/// \param[in] pIrp The IO request procedure
/// \param[in] pIrpStack The context of the IRP
NTSTATUS IoctlGetAvailability(IN CpuProf::Device* pDevExt, PIRP pIrp,
                              PIO_STACK_LOCATION pIrpStack);
#endif //_CPUPROF_IOCONTROL_H_
