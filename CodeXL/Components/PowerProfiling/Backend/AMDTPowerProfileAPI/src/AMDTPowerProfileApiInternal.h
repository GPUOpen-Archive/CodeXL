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
#define MAX_DEVICE_CNT          50

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
