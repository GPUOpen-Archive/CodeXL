//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfileCLI.h
///
//==================================================================================

#ifndef _POWER_PROFILE_CLI_H_
#define _POWER_PROFILE_CLI_H_
#pragma once

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osMachine.h>

// For reporter
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>

// #include <AMDTOSWrappers/Include/osApplication.h>
// #include <AMDTOSWrappers/Include/osGeneralFunctions.h>

// API
#include <AMDTPowerProfileApi.h>

// SVI2 is available only in internal and NDA version
#if defined(AMDT_INTERNAL) || defined(AMDT_NDA)
    #define SVI2_COUNTERS_SUPPORTED
#endif

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <stdio.h>
    #include <unistd.h>
    #include <stdarg.h>
    #include <sys/types.h>
    #include <sys/utsname.h>
    #include <signal.h>
    #include <time.h>
#endif

//
//  Macros
//
#define PP_DEFAULT_OUTPUTFILE_NAME  L"CodeXL-PowerProfile"

#define PP_SAMPLING_INTERVAL_DEFAULT    100


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define STR_FORMAT L"%s"
    #define PATH_SEPARATOR L"\\"
    #define wcsncpy_truncate(dst, size, src) wcsncpy_s(dst, size, src, _TRUNCATE)
    #define wcsncat_truncate(dst, size, src) wcsncat_s(dst, size, src, _TRUNCATE)
    #define mbstowcs_truncate(dst, size, src) { size_t retVal; mbstowcs_s(&retVal, dst, size, src, _TRUNCATE); }
    #define wcsdup   _wcsdup
#else
    #define STR_FORMAT L"%S"
    #define PATH_SEPARATOR L"/"
    #define wcsncpy_truncate(dst, size, src) wcsncpy(dst, src, (size) - 1)
    #define wcsncat_truncate(dst, size, src) wcsncat(dst, src, (size) - 1 - wcslen(dst))
    #define mbstowcs_truncate(dst, size, src) mbstowcs(dst, src, (size) - 1)
#endif


#define PP_REPORT_EXTENSION_CSV     L"csv"
#define PP_REPORT_EXTENSION_TEXT    L"txt"
#define PP_REPORT_EXTENSION_XML     L"xml"

#define PP_RAWFILE_EXTENSION        L"data"

#define PP_PROFILE_ALL_COUNTERS     L"all"

// support options with -P
#define PP_COUNTER_GROUP_POWER            L"power"
#define PP_COUNTER_GROUP_TEMPERATURE      L"temperature"
#define PP_COUNTER_GROUP_FREQUENCY        L"frequency"
#define PP_COUNTER_GROUP_CU_POWER         L"cu_power"
#define PP_COUNTER_GROUP_CU_TEMPERATURE   L"cu_temperature"
#define PP_COUNTER_GROUP_GPU_POWER        L"gpu_power"
#define PP_COUNTER_GROUP_GPU_TEMPERATURE  L"gpu_temperature"
#define PP_COUNTER_GROUP_CORE             L"core"
#define PP_COUNTER_GROUP_SVI2             L"svi2"
#define PP_COUNTER_GROUP_DVFS             L"dvfs"
#define PP_COUNTER_GROUP_ALL              L"all"

typedef enum PowerProfileReportTypes
{
    PP_REPORT_TYPE_UNKNOW = 0,
    PP_REPORT_TYPE_TEXT    = 1,
    PP_REPORT_TYPE_CSV   = 2,
    PP_REPORT_TYPE_XML    = 3,
} PowerProfileReportType;

typedef struct PwrSupportedCounterDetails
{
    AMDTUInt32           m_counterID;
    AMDTUInt32           m_deviceId;
    char                 m_name[OS_MAX_PATH];
    char                 m_description[OS_MAX_PATH];
    AMDTPwrCategory      m_category;
    AMDTPwrAggregation   m_aggregation;
    AMDTFloat64          m_minValue;
    AMDTFloat64          m_maxValue;
    AMDTPwrUnit          m_units;
    char                 m_modifiedName[OS_MAX_PATH];
} PwrSupportedCounterDetails;

typedef gtMap<gtString, AMDTPwrDevice*> AMDTPwrDeviceNameDescMap; // Device Name - Device Desc Map
typedef gtVector<AMDTPwrDevice*> AMDTPwrDeviceIdDescVec; // Device ID-Desc vector
typedef gtVector<gtString> AMDTPwrDeviceIdNameVec; // Device ID-Name vector


typedef gtMap<gtString, AMDTPwrCounterDesc*> AMDTPwrCounterNameDescMap; // Name - Counter Desc Map
typedef gtVector<AMDTPwrCounterDesc*> AMDTPwrCounterIdDescVec; // ID-Desc vector

typedef gtMap<AMDTUInt32, AMDTPwrCounterDesc> AMDTPwrCounterMap;
typedef gtVector<gtString> AMDTPwrCounterIdNameVec; // CounterID-Name vector

typedef gtMap <AMDTUInt32, PwrSupportedCounterDetails> PwrSupportedCounterDetailsMap;
// Helper functions
extern bool ReportError(bool appendDriverError, const char* pFormatString, ...);
extern PwrSupportedCounterDetailsMap g_supportedCounters;


// USE CASES
//
// PowerProfileCLI    // print usage & exit
// PowerProfileCLI -v // print version & exit
// PowerProfileCLI -h // print help & exit
// PowerProfileCLI -P 0x1f -c 0x1 // DO we need to support this ?
// PowerProfileCLI -P cu_power:0x2,gpu_power,core_freq:0x4 classic.exe
// PowerProfileCLI -e cu_0_power,cu_1_power -C 0x1 classic.exe
//

#endif // _POWER_PROFILE_CLI_H_
