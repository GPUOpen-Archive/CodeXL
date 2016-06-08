//=============================================================================
//
// Author: AMD Developer Tools
//         AMD, Inc.
//
// This header file contains the basic data type definitions and the error
// codes used by AMD CodeXL Power Profile APIs.
//
//=============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.  All rights reserved.
//=============================================================================

#ifndef _AMDTDEFINITIONS_H_
#define _AMDTDEFINITIONS_H_

/** \file AMDTDefinitions.h
\brief Basic data type definitions and error codes used by the AMD CodeXL Power Profiler APIs
*/

#include <limits.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Windows specific system headers
#ifdef _WIN32
#include <wchar.h>
#include <Windows.h>
#endif // _WIN32

// Basic data type definitions

#ifdef _WIN32
typedef char AMDTInt8;
typedef unsigned char AMDTUInt8;
typedef __int16 AMDTInt16;
typedef unsigned __int16 AMDTUInt16;
typedef __int32 AMDTInt32;
typedef unsigned __int32 AMDTUInt32;
typedef __int64 AMDTInt64;
typedef unsigned __int64 AMDTUInt64;
typedef float AMDTFloat32;
typedef double AMDTFloat64;
typedef size_t AMDTSize;

#ifndef __cplusplus
typedef AMDTUInt8 bool;
#endif
#endif // _WIN32

#ifdef __linux__
typedef char AMDTInt8;
typedef short AMDTInt16;
typedef int AMDTInt32;
typedef long int AMDTInt64;
typedef unsigned int UINT;
typedef float   AMDTFloat32;
typedef double  AMDTFloat64;
typedef unsigned char AMDTUInt8;
typedef unsigned short AMDTUInt16;
typedef unsigned int AMDTUInt32;
typedef unsigned long int AMDTUInt64;
#ifndef __cplusplus
typedef AMDTUInt8 bool;
#endif
#endif // _LINUX

// Error Codes

typedef unsigned int AMDTResult;

/** Returned on success
*/
#define AMDT_STATUS_OK                          AMDTResult(0)

/**An internal error occurred.
*/
#define AMDT_ERROR_FAIL                         AMDTResult(0x80004005)

/**Invalid argument is passed.
*/
#define AMDT_ERROR_INVALIDARG                   AMDTResult(0x80070057)

/**Memory allocation failed.
*/
#define AMDT_ERROR_OUTOFMEMORY                  AMDTResult(0x8007000E)

/** An unexpected error occurred.
*/
#define AMDT_ERROR_UNEXPECTED                   AMDTResult(0x8000FFFF)

/**Profiler not available
*/
#define AMDT_ERROR_ACCESSDENIED                 AMDTResult(0x80070005)

/** Invalid handler is passed
*/
#define AMDT_ERROR_HANDLE                       AMDTResult(0x80070006)

/** Profiler aborted due to an internal error
*/
#define AMDT_ERROR_ABORT                        AMDTResult(0x80004004)

/** Requested profiler functionality is not yet implemented.
*/
#define AMDT_ERROR_NOTIMPL                      AMDTResult(0x80004001)

/** File not found.
*/
#define AMDT_ERROR_NOFILE                       AMDTResult(0x80070002)

/**Invalid file path specified.
*/
#define AMDT_ERROR_INVALIDPATH                  AMDTResult(0x80070003)

/**Invalid data is passed as a parameter.
*/
#define AMDT_ERROR_INVALIDDATA                  AMDTResult(0x8007000D)

/** Requested functionality or data is not yet available.
*/
#define AMDT_ERROR_NOTAVAILABLE                 AMDTResult(0x80075006)

/** No profile data is available.
*/
#define AMDT_ERROR_NODATA                       AMDTResult(0x800700E8)

/** Already locked.
*/
#define AMDT_ERROR_LOCKED                       AMDTResult(0x80070021)

/** Timeout.
*/
#define AMDT_ERROR_TIMEOUT                      AMDTResult(0x800705B4)

/** Profiler is currently active and the requested action is pending.
*/
#define AMDT_STATUS_PENDING                     AMDTResult(0x8000000A)

/** The requested functionality is not supported
*/
#define AMDT_ERROR_NOTSUPPORTED                 AMDTResult(0x8000FFFE)

/** Profiler is already initialized.
*/
#define AMDT_ERROR_DRIVER_ALREADY_INITIALIZED   AMDTResult(0x80080001)

/** Profile driver is not available.
*/
#define AMDT_ERROR_DRIVER_UNAVAILABLE           AMDTResult(0x80080002)

/** SMU is disabled.
*/
#define AMDT_WARN_SMU_DISABLED                  AMDTResult(0x80080003)

/**Internal GPU is disabled.
*/
#define AMDT_WARN_IGPU_DISABLED                 AMDTResult(0x80080004)

/** Driver is not yet initialized.
*/
#define AMDT_ERROR_DRIVER_UNINITIALIZED         AMDTResult(0x80080005)

/** Invalid device ID is passed as a parameter.
*/
#define AMDT_ERROR_INVALID_DEVICEID             AMDTResult(0x80080006)

/** Invalid profile counter id is passes as a parameter.
*/
#define AMDT_ERROR_INVALID_COUNTERID            AMDTResult(0x80080007)

/** Specified counter ID is already enabled.
*/
#define AMDT_ERROR_COUNTER_ALREADY_ENABLED      AMDTResult(0x80080008)

/** No write permission to create the specified profile data file.
*/
#define AMDT_ERROR_NO_WRITE_PERMISSION          AMDTResult(0x80080009)

/** Specified counter ID is not enabled.
*/
#define AMDT_ERROR_COUNTER_NOT_ENABLED          AMDTResult(0x8008000A)

/** Sampling timer is not set.
*/
#define AMDT_ERROR_TIMER_NOT_SET                AMDTResult(0x8008000B)

/** Profile data file is not set.
*/
#define AMDT_ERROR_PROFILE_DATAFILE_NOT_SET     AMDTResult(0x8008000C)

/** Profile was already started.
*/
#define AMDT_ERROR_PROFILE_ALREADY_STARTED      AMDTResult(0x8008000D)

/** Profile was not started.
*/
#define AMDT_ERROR_PROFILE_NOT_STARTED          AMDTResult(0x8008000E)

/** Profile is not in paused state.
*/
#define AMDT_ERROR_PROFILE_NOT_PAUSED           AMDTResult(0x8008000F)

/** Profile data is not yet available.
*/
#define AMDT_ERROR_PROFILE_DATA_NOT_AVAILABLE   AMDTResult(0x80080010)

/** This HW platform is not supported.
*/
#define AMDT_ERROR_PLATFORM_NOT_SUPPORTED       AMDTResult(0x80080011)

/** An Internal error occured.
*/
#define AMDT_ERROR_INTERNAL                     AMDTResult(0x80080012)

/** Mismatch between the expected and installed driver versions.
*/
#define AMDT_DRIVER_VERSION_MISMATCH            AMDTResult(0x80080013)

/** Bios needs to be upgraded in the system.
*/
#define AMDT_ERROR_BIOS_VERSION_NOT_SUPPORTED   AMDTResult(0x80080014)

/** Profile is already configured.
*/
#define AMDT_ERROR_PROFILE_ALREADY_CONFIGURED   AMDTResult(0x80080015)

/** Profile is not yet configured.
*/
#define AMDT_ERROR_PROFILE_NOT_CONFIGURED       AMDTResult(0x80080016)

/** Profile session already exists.
*/
#define AMDT_ERROR_PROFILE_SESSION_EXISTS       AMDTResult(0x80080017)

/** Could not access the configured profile counter due to access failure.
*/
#define AMDT_ERROR_SMU_ACCESS_FAILED            AMDTResult(0x80080018)

/** Could not start the profile session as counters are not enabled.
*/
#define AMDT_ERROR_COUNTERS_NOT_ENABLED         AMDTResult(0x80080019)

/** Previous profile session was not closed.
*/
#define AMDT_ERROR_PREVIOUS_SESSION_NOT_CLOSED  AMDTResult(0x80080020)

/** Counter does not have any hierarchical relationship
*/
#define AMDT_ERROR_COUNTER_NOHIERARCHY          AMDTResult(0x80080021)

/** Counter is not accessible
*/
#define AMDT_ERROR_COUNTER_NOT_ACCESSIBLE       AMDTResult(0x80080022)

/**  Profiling not supported on Hypervisor
*/
#define AMDT_ERROR_HYPERVISOR_NOT_SUPPORTED     AMDTResult(0x80080023)

/**  Process profiling not supported
*/
#define AMDT_WARN_PROCESS_PROFILE_NOT_SUPPORTED AMDTResult(0x80080024)

/**  Unable to configure the marker
*/
#define AMDT_ERROR_MARKER_NOT_SET                AMDTResult(0x80080025)

#ifdef __cplusplus
}
#endif

#endif // _AMDTDEFINITIONS_H_