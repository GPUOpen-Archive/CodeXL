//=============================================================
// (c) 2016 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \version $Revision: $
/// \brief  Histogram prototypes for power profile
//
//=============================================================
#ifndef _AMDTHISTOGRAM_H_
#define _AMDTHISTOGRAM_H_
#include <AMDTDefinitions.h>
#include <AMDTPowerProfileDataTypes.h>
#include <AMDTPowerProfileInternal.h>

typedef struct CounterRange
{
    AMDTUInt32    m_id;
    AMDTFloat32   m_min;
    AMDTFloat32   m_max;
    AMDTFloat32   m_steps;
} CounterRange;

// InitHistogram: Initialize histogram counters
void InitializeHistogram();

// AddToHistogram: Add counter values to histogram
AMDTResult AddToHistogram(AMDTUInt32 counterId, AMDTFloat32 value);

// AddToHistogram: Add counter values to histogram
void AddToCumulativeCounter(AMDTUInt32 counterId, AMDTFloat32 value);

// GetCumulativeCounter: Ge the value of cumulative counter
AMDTFloat32* GetCumulativeCounter(AMDTUInt32 counterId);

// GetHistogramCounter: Get the value of histogram counter
AMDTPwrHistogram* GetHistogramCounter(AMDTUInt32 counterId);

#endif //_AMDTHISTOGRAM_H_


