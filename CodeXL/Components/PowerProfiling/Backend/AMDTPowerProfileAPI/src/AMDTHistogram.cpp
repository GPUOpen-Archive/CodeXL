//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTHistogram.cpp
///
//==================================================================================

#include <AMDTDefinitions.h>
#include <AMDTPowerProfileInternal.h>
#include <AMDTPowerProfileApiInternal.h>
#include <PowerProfileHelper.h>
#include <AMDTHistogram.h>
#include <stdio.h>
#include <string.h>

#define FREQ_BIN_CNT 10
static DerivedCounter g_derivedCounter[MAX_COUNTER_CNT];

CounterRange g_hisCounters[] =
{
    {COUNTERID_SMU7_APU_FREQ_IGPU, 0.0, 2000.0, 200.0},
    {COUNTERID_SMU8_APU_FREQ_IGPU, 0.0, 2000.0, 200.0},
    {COUNTERID_CEF, 0.00, 5000.0, 500.0},
    {DGPU_COUNTER_BASE_ID + COUNTERID_FREQ_DGPU, 0.00, 2000.0, 200.0},
    {DGPU_COUNTER_BASE_ID + DGPU_COUNTERS_MAX + COUNTERID_FREQ_DGPU, 0.00, 2000.0, 200.0},
    {DGPU_COUNTER_BASE_ID + 2 * DGPU_COUNTERS_MAX + COUNTERID_FREQ_DGPU, 0.00, 2000.0, 200.0},
    {COUNTERID_FREQ_DGPU, 0.00, 2000.0, 200.0},
};

// InitHistogram: Initialize histogram counters
void InitializeHistogram()
{
    AMDTUInt32 cnt = 0;
    AMDTUInt32 steps = 0;
    AMDTUInt32 loop = 0;

    memset(g_derivedCounter, 0, sizeof(DerivedCounter));

    for (cnt = 0; cnt < sizeof(g_hisCounters) / sizeof(CounterRange); cnt++)
    {
        AMDTUInt32 idx = g_hisCounters[cnt].m_id;

        for (loop = 0; loop < MAX_INSTANCE_CNT; loop++)
        {
            g_derivedCounter[idx].m_histogram[loop].m_binCnt = FREQ_BIN_CNT;
            g_derivedCounter[idx].m_histogram[loop].m_pRangeStartIndex[0] = 0;

            for (steps = 1; steps <= FREQ_BIN_CNT; steps++)
            {
                g_derivedCounter[idx].m_histogram[loop].m_pRangeStartIndex[steps] = steps * g_hisCounters[cnt].m_steps + 1;
            }
        }
    }
}

// AddToHistogram: Add counter values to histogram
void AddToCumulativeCounter(AMDTUInt32 counterId, AMDTUInt32 instanceId, AMDTFloat32 value)
{
    g_derivedCounter[counterId].m_value[instanceId] += value;
}

// AddToHistogram: Add counter values to histogram
AMDTResult AddToHistogram(AMDTUInt32 counterId, AMDTUInt32 instanceId, AMDTFloat32 value)
{
    AMDTResult ret = AMDT_ERROR_INTERNAL;
    AMDTUInt32 bucket = 0;
    AMDTUInt32 cnt = 0;
    AMDTFloat32 step = 0;

    for (cnt = 0; cnt < sizeof(g_hisCounters) / sizeof(CounterRange); cnt++)
    {
        if (counterId == g_hisCounters[cnt].m_id)
        {
            step = g_hisCounters[cnt].m_steps;
            bucket = (AMDTUInt32)((value + step - 1) / step - 1);
            g_derivedCounter[counterId].m_histogram[instanceId].m_pRangeValue[bucket]++;
            ret = AMDT_STATUS_OK;
            break;
        }
    }

    if (AMDT_STATUS_OK != ret)
    {
        PwrTrace("counter %d not set ret 0x%x", counterId, ret);
    }

    return ret;
}

// GetCumulativeCounter: Get the value of cumulative counter
AMDTFloat32* GetCumulativeCounter(AMDTUInt32 counterId, AMDTUInt32 instanceId)
{
    return &g_derivedCounter[counterId].m_value[instanceId];

}

// GetHistogramCounter: Get the value of histogram counter
Histogram* GetHistogramCounter(AMDTUInt32 counterId, AMDTUInt32 instanceId)
{
    return &g_derivedCounter[counterId].m_histogram[instanceId];
}

