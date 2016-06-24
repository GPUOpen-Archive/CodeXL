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

// counteris, DerivedCounter
typedef gtMap<AMDTUInt32, PwrDerivedCounter> PwrDerivedCounterMap;
static PwrDerivedCounterMap g_derivedCounter;

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
    g_derivedCounter.clear();
}

// AddToHistogram: Add counter values to histogram
void AddToCumulativeCounter(AMDTUInt32 counterId, AMDTFloat32 value)
{
    PwrDerivedCounterMap:: iterator iter = g_derivedCounter.find(counterId);
    PwrDerivedCounter counter;

    if (iter != g_derivedCounter.end())
    {
        iter->second.m_value += value;
    }
    else
    {
        counter.m_value = value;
        g_derivedCounter.insert(PwrDerivedCounterMap::value_type(counterId, counter));
    }
}

// AddToHistogram: Add counter values to histogram
AMDTResult AddToHistogram(AMDTUInt32 counterId, AMDTFloat32 value)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTUInt32 bucket = 0;
    AMDTFloat32 step = 200;
    PwrDerivedCounterMap::iterator iter = g_derivedCounter.find(counterId);

    bucket = (AMDTUInt32)((value + step - 1) / step - 1);

    if (iter != g_derivedCounter.end())
    {
        iter->second.m_histogram.m_pBins[bucket]++;
    }
    else
    {
        PwrDerivedCounter counter;
        AMDTUInt32 loop = 0;
        memset(&counter, 0, sizeof(PwrDerivedCounter));
        counter.m_histogram.m_counterId = counterId;
        counter.m_histogram.m_numOfBins = FREQ_BIN_CNT;
        counter.m_histogram.m_pRange = (AMDTFloat32*)GetMemoryPoolBuffer(&g_transPool, (MAX_BIN_CNT + 1) * sizeof(AMDTFloat32));
        memset(counter.m_histogram.m_pRange, 0, MAX_BIN_CNT * sizeof(AMDTFloat32));
        counter.m_histogram.m_pBins = (AMDTFloat32*)GetMemoryPoolBuffer(&g_transPool, MAX_BIN_CNT * sizeof(AMDTFloat32));
        memset(counter.m_histogram.m_pBins, 0, MAX_BIN_CNT * sizeof(AMDTFloat32));

        for (loop = 1; loop <= FREQ_BIN_CNT + 1; loop++)
        {
            counter.m_histogram.m_pRange[loop] = (AMDTFloat32)(loop * 200 + 1);
        }

        counter.m_histogram.m_pBins[bucket] = 1;
        g_derivedCounter.insert(PwrDerivedCounterMap::value_type(counterId, counter));
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
    PwrDerivedCounterMap::iterator iter = g_derivedCounter.find(counterId);

    if (iter != g_derivedCounter.end())
    {
        return &iter->second.m_value;
    }
    else
    {
        return nullptr;
    }

}

// GetHistogramCounter: Get the value of histogram counter
AMDTPwrHistogram* GetHistogramCounter(AMDTUInt32 counterId)
{
    PwrDerivedCounterMap::iterator iter = g_derivedCounter.find(counterId);

    if (iter != g_derivedCounter.end())
    {
        return &iter->second.m_histogram;
    }
    else
    {
        return nullptr;
    }

}

