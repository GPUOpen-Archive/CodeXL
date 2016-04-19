//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTPowerProfileApiInternal.h
///
//==================================================================================

#ifndef AMDT_PWR_PROFILE_INTERNAL
#define AMDT_PWR_PROFILE_INTERNAL

// minimum sampling interval is 100 msec for public
// and 1 msec for internal/nda builds
#if defined(GDT_INTERNAL) || defined(GDT_NDA)
    #define MINIMAL_SAMPLING_PERIOD 1
#else
    #define MINIMAL_SAMPLING_PERIOD 10
#endif

#define MAX_OUTPUT_FILE_PATH    255
#define MAX_SUPPORTED_COUNTERS  150
#define MAX_SAMPLES_PER_QUERY   128
#define MAX_DEVICE_CNT          15
#define COUNTER_HIST_MAX_CNT    15


typedef enum
{
    // SMU7 APU specific counters
    COUNTER_SMU7_APU_PWR_CU,
    COUNTER_SMU7_APU_TEMP_CU,
    COUNTER_SMU7_APU_PWR_PCIE,
    COUNTER_SMU7_APU_PWR_DDR,
    COUNTER_SMU7_APU_PWR_PACKAGE,
    COUNTER_SMU7_APU_PWR_DISPLAY,
    COUNTER_SMU7_APU_PWR_IGPU,
    COUNTER_SMU7_APU_TEMP_IGPU,
    COUNTER_SMU7_APU_FREQ_IGPU,

    // SMU8 APU specific counters
    COUNTER_SMU8_APU_PWR_CU,
    COUNTER_SMU8_APU_TEMP_CU,
    COUNTER_SMU8_APU_C0_RES,
    COUNTER_SMU8_APU_C1_RES,
    COUNTER_SMU8_APU_CC6_RES,
    COUNTER_SMU8_APU_PC6_RES,
    COUNTER_SMU8_APU_PWR_APU,
    COUNTER_SMU8_APU_PWR_VDDGFX,
    COUNTER_SMU8_APU_TEMP_VDDFFX,
    COUNTER_SMU8_APU_FREQ_IGPU,
    COUNTER_SMU8_APU_PWR_VDDIO,
    COUNTER_SMU8_APU_PWR_VDDNB,
    COUNTER_SMU8_APU_PWR_VDDP,
    COUNTER_SMU8_APU_PWR_UVD,
    COUNTER_SMU8_APU_PWR_VCE,
    COUNTER_SMU8_APU_PWR_ACP,
    COUNTER_SMU8_APU_PWR_UNB,
    COUNTER_SMU8_APU_PWR_SMU,
    COUNTER_SMU8_APU_PWR_ROC,
    COUNTER_SMU8_APU_FREQ_ACLK,

    // Non SMU counters accessed by MSR/PCIe interface
    COUNTER_CORE_APU_PID,
    COUNTER_CORE_APU_TID,
    COUNTER_CORE_EFFECTIVE_FREQUENCY,
    COUNTER_CORE_PSTATE,
    COUNTER_NODE_TCTL_TEMPERATURE,
    COUNTER_NCORE_SVI2_CORE_TELEMETRY,
    COUNTER_NCORE_SVI2_NB_TELEMETRY,
    COUNTER_MAX
} BasicCounters;

typedef enum
{
    COUNTER_ACC_SMU_PWR_CU,
    COUNTER_ACC_SMU_PWR_PACKAGE,
    COUNTER_ACC_SMU_PWR_GPU,
    COUNTER_HIST_SMU_FREQ_GPU,
    COUNTER_HIST_CORE_EFFECTIVE_FREQUENCY,
} AggregatedCounters;

typedef struct
{
    AMDTUInt32 m_instanceId;
    AMDTUInt32                    m_backendId;
    BasicCounters                 m_basicCounterId;
    AMDTPwrAttributeInstanceType  m_instanceType;
    AMDTPwrCounterDesc*           m_pDesc;
} CounterInfo;

typedef struct
{
    AMDTUInt32 m_clientId;
    AMDTUInt32 m_backendId;
} CounterMapping;

typedef struct
{
    AMDTUInt32          m_counterId;
    AMDTUInt32          m_backendId;
    AMDTUInt32          m_aggrCounterId;
    AMDTUInt32          m_bins;
} AggregatedCounterInfo;

typedef struct
{
    AMDTUInt32 m_family;
    AMDTUInt32 m_model;
    char*      m_name;
} PlaformInfo;

#endif
