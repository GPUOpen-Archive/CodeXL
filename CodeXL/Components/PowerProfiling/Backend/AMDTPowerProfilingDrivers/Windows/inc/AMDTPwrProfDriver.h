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

#ifndef _PWRPROFDRIVER_H
#define _PWRPROFDRIVER_H

#include <stdlib.h>

/// \def IOCTL_GET_VERSION returns the version encoded in a ULONG64 *
/// Use InvokeOut
/// Handled by \ref IoctlGetVersionHandler
#define IOCTL_GET_VERSION \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)


/// \def IOCTL_REGISTER_CLIENT obtains the client id used for interactions,
/// both IOCTL and shared memory
/// Use InvokeOut and a ULONG *
/// Handled by \ref IoctlRegisterClient
#define IOCTL_REGISTER_CLIENT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)


/// \def IOCTL_SET_OUTPUT_FILE sets the file path and prd header for the
/// sampling output files
/// Use InvokeInOut and \ref OUTPUT_FILE_DESCRIPTOR
/// Handled by \ref IoctlSetOutputFileHandler
#define IOCTL_SET_OUTPUT_FILE   \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)



/// \def IOCTL_ADD_PROF_CONFIGS sets the profiling configurations to profile when
/// the profiler starts.
/// Use InvokeInOut and \ref PROF_CONFIGS
/// Handled by \ref IoctlAddProfConfigsHandler
#define IOCTL_ADD_PROF_CONFIGS \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)


/// \def IOCTL_START_PROFILER starts the profile.
/// Use InvokeInOut and \ref PROFILER_PROPERTIES
/// Handled by \ref IoctlStartProfilerHandler
#define IOCTL_START_PROFILER \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)


/// \def IOCTL_PAUSE_PROFILER pauses the profile.
/// A faster method is to set the appropriate (1 << clientId) shared memory bit
/// or to set the shared memory key
/// Use InvokeInOut and ULONG client id in, ULONG profile state out
/// Handled by \ref IoctlPauseProfilerHandler
#define IOCTL_PAUSE_PROFILER \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)


/// \def IOCTL_RESUME_PROFILER resumes the profile.
/// A faster method is to clear the appropriate (1 << clientId) shared memory
/// bit or to clear the shared memory key
/// Use InvokeInOut and ULONG client id in, ULONG profile state out
/// Handled by \ref IoctlResumeProfilerHandler
#define IOCTL_RESUME_PROFILER   \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)


/// \def IOCTL_GET_FILE_HEADER_BUFFER is to get the file header data
/// Use InvokeInOut and \ref FILE_HEADER
/// Handled by \ref IoctlGetFileHeaderBufferHandler
#define IOCTL_GET_FILE_HEADER_BUFFER \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)


/// \def IOCTL_GET_DATA_BUFFER is to get the sample data buffer
/// Use InvokeInOut and \ref DATA_BUFFER
/// Handled by \ref IoctlGetDataBufferHandler
#define IOCTL_GET_DATA_BUFFER \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)


/// \def IOCTL_STOP_PROFILER stops the profile.  It will also clear the state
/// of the clientId.
/// Use InvokeInOut and ULONG client id in, ULONG profile status out
/// Handled by \ref IoctlStopProfilerHandler
#define IOCTL_STOP_PROFILER \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)


/// \def IOCTL_UNREGISTER_CLIENT frees the client id used for interactions,
/// it will immediately stop and finish any current profile
/// Use InvokeIn and a ULONG *
/// Handled by \ref IoctlUnegisterClient
#define IOCTL_UNREGISTER_CLIENT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80A, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_ACCESS_PCI_DEVICE provides access to PCI devices,
/// Use InvokeInOut and \ref ACCESS_PCI
/// Handled by \ref IoctlAccessPciDevice
#define IOCTL_ACCESS_PCI_DEVICE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80B, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_ACCESS_MSR provides access to MSR
/// Use InvokeInOut and \ref ACCESS_MSR
/// Handled by \ref IoctlAccessMSR
#define IOCTL_ACCESS_MSR \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80C, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_ACCESS_MMIO provides access to MMIO address space
/// Use InvokeInOut and \ref ACCESS_MMIO
/// Handled by \ref IoctlAccessMMIO
#define IOCTL_ACCESS_MMIO \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80D, METHOD_BUFFERED, FILE_ANY_ACCESS)

/// \def IOCTL_SET_EVENT sets events to communicate with userspace
/// Use InvokeInOut and ULONG*
/// Handled by \ref IoctlSetEvent
#define IOCTL_SET_EVENT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80E, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif  //_PWRPROFDRIVER_H

