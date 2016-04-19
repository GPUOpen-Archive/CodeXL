//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsOSPerformanceCountersManager.h
///
//==================================================================================

//------------------------------ gsOSPerformanceCountersManager.h ------------------------------

#ifndef __GSOSPERFORMANCECOUNTERSMANAGER_H
#define __GSOSPERFORMANCECOUNTERSMANAGER_H

// Forward declarations:
class osSystemResourcesDataSampler;

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTOSWrappers/Include/osCPUSampledData.h>
#include <AMDTOSWrappers/Include/osPhysicalMemorySampledData.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>


// ----------------------------------------------------------------------------------
// Class Name:          gsOSPerformanceCountersManager
// General Description: Holds and manages OS counters on the spy side for remote
//                      debugging targets
// Author:              Uri Shomroni
// Creation Date:       22/11/2009
// ----------------------------------------------------------------------------------
class gsOSPerformanceCountersManager
{
public:
    gsOSPerformanceCountersManager(osSystemResourcesDataSampler& dataSampler);
    ~gsOSPerformanceCountersManager();

    bool updateCounterValues();
    int amountOfCounters() const {return _amountOfCounters;};
    const double* getCounterValues() const {return _pCounterValues;};

private:
    // Disallow use of my default constructor:
    gsOSPerformanceCountersManager();

    bool initialize();
    bool updateCPUCounterValues(int cpuIndex, double& cpuUsage, double& cpuUser, double& cpuNice, double& cpuSystem, double& cpuIdle);
    bool updateMemoryCounterValues();
private:
    // The data sampler:
    osSystemResourcesDataSampler& _dataSampler;

    // The number of counters we export:
    int _amountOfCounters;

    // An array of the counters' values:
    double* _pCounterValues;

    // The number of CPUs we monitor, including a dummy "average" CPU, if needed:
    int _amountOfCPUs;

    // The last values of CPU sampled data:
    gtVector<osCPUSampledData> _sampledCPUsData;

    // For Page in/out per second measurements, we need these items:
    gtUInt32 _lastPageInCount;
    gtUInt32 _lastPageOutCount;
    osStopWatch _timeSinceLastCounterValuesUpdate;
};

#endif //__GSOSPERFORMANCECOUNTERSMANAGER_H

