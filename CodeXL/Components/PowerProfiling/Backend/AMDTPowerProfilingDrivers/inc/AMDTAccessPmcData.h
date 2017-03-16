#ifndef _AMDTACCESSPMCDATA_H
#define _AMDTACCESSPMCDATA_H

#include <AMDTPwrProfAttributes.h>

typedef struct PmcCounters
{
    ACCESS_MSR   m_control;
    ACCESS_MSR64 m_data;
} PmcCounters;

// InitializePMCCounters
bool InitializePMCCounters(PmcCounters* pPmc);

// ReadPmcCounterData: Read PMC counter values
uint32 ReadPmcCounterData(PmcCounters* pPmc, uint64* pData);

// ResetPMCCounters: Reset PCM counter values
bool ResetPMCCounters(PmcCounters* pPmc);

// ResetPMCControl: Reset PMC counter control data
bool ResetPMCControl(PmcCounters* pPmc);
#endif
