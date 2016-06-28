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
#include <PowerProfileHelper.h>
#include <stdio.h>
#include <string.h>

#define FREQ_BIN_CNT 10
extern MemoryPool g_transPool;

// counterid, AMDTPwrHistogram
typedef gtMap<AMDTUInt32, AMDTPwrHistogram> PwrHistogramCounterMap;

// counterid, AMDTFloat32
typedef gtMap<AMDTUInt32, AMDTFloat32> PwrCummulativeCounterMap;

static PwrHistogramCounterMap g_histogramCounter;
static PwrCummulativeCounterMap g_cummulativeCounter;


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
    g_histogramCounter.clear();
    g_cummulativeCounter.clear();
}

// AddToHistogram: Add counter values to histogram
void AddToCumulativeCounter(AMDTUInt32 counterId, AMDTFloat32 value)
{
    PwrCummulativeCounterMap:: iterator iter = g_cummulativeCounter.find(counterId);

    if (iter != g_cummulativeCounter.end())
    {
        iter->second += value;
    }
    else
    {
        g_cummulativeCounter.insert(PwrCummulativeCounterMap::value_type(counterId, value));
    }
}

// AddToHistogram: Add counter values to histogram
AMDTResult AddToHistogram(AMDTUInt32 counterId, AMDTFloat32 value)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 bucket = 0;
    AMDTFloat32 step = 200;
    PwrHistogramCounterMap::iterator iter = g_histogramCounter.find(counterId);

    bucket = (AMDTUInt32)((value + step - 1) / step - 1);

    if (iter != g_histogramCounter.end())
    {
        iter->second.m_bins[bucket]++;
    }
    else
    {
        AMDTPwrHistogram counter;
        AMDTUInt32 loop = 0;
        memset(&counter, 0, sizeof(AMDTPwrHistogram));
        counter.m_counterId = counterId;
        counter.m_numOfBins = FREQ_BIN_CNT;

        for (loop = 0; loop <= AMDT_PWR_HISTOGRAM_MAX_BIN_COUNT; loop++)
        {
            counter.m_range[loop] = (AMDTFloat32)(loop * 200 + 1);
        }

        counter.m_bins[bucket] = 1;
        g_histogramCounter.insert(PwrHistogramCounterMap::value_type(counterId, counter));
    }

    if (AMDT_STATUS_OK != ret)
    {
        PwrTrace("counter %d not set ret 0x%x", counterId, ret);
    }

    return ret;
}

// GetCumulativeCounter: Get the value of cumulative counter
AMDTFloat32* GetCumulativeCounter(AMDTUInt32 counterId)
{
    PwrCummulativeCounterMap::iterator iter = g_cummulativeCounter.find(counterId);

    if (iter != g_cummulativeCounter.end())
    {
        return &iter->second;
    }
    else
    {
        return nullptr;
    }

}

// GetHistogramCounter: Get the value of histogram counter
AMDTPwrHistogram* GetHistogramCounter(AMDTUInt32 counterId)
{
    PwrHistogramCounterMap::iterator iter = g_histogramCounter.find(counterId);

    if (iter != g_histogramCounter.end())
    {
        return &iter->second;
    }
    else
    {
        return nullptr;
    }

}

