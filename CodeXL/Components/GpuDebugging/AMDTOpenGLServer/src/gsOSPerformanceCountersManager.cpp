//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsOSPerformanceCountersManager.cpp
///
//==================================================================================

//------------------------------ gsOSPerformanceCountersManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osSystemResourcesDataSampler.h>

// Local:
#include <src/gsOSPerformanceCountersManager.h>

// These values must match the values in pcRemoteOSCountersReader.cpp:
#define GS_AMOUNT_OF_COUNTERS_PER_CPU 5
#define GS_AMOUNT_OF_MEMORY_COUNTERS 8

// ---------------------------------------------------------------------------
// Name:        gsOSPerformanceCountersManager::gsOSPerformanceCountersManager
// Description: Constructor
// Author:      Uri Shomroni
// Date:        22/11/2009
// ---------------------------------------------------------------------------
gsOSPerformanceCountersManager::gsOSPerformanceCountersManager(osSystemResourcesDataSampler& dataSampler)
    : _dataSampler(dataSampler), _amountOfCounters(0), _pCounterValues(NULL), _amountOfCPUs(0)
{
    bool rc1 = initialize();
    GT_ASSERT(rc1);
}

// ---------------------------------------------------------------------------
// Name:        gsOSPerformanceCountersManager::~gsOSPerformanceCountersManager
// Description: Destructor
// Author:      Uri Shomroni
// Date:        22/11/2009
// ---------------------------------------------------------------------------
gsOSPerformanceCountersManager::~gsOSPerformanceCountersManager()
{
    // Delete and clear the vectors
    delete[] _pCounterValues;
    _pCounterValues = NULL;
}

// ---------------------------------------------------------------------------
// Name:        gsOSPerformanceCountersManager::updateCounterValues
// Description: Updates the OS counters' values
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool gsOSPerformanceCountersManager::updateCounterValues()
{
    bool rcCPU = false;
    bool rcMem = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pCounterValues != NULL)
    {
        // Update CPU Counters:
        GT_IF_WITH_ASSERT(_amountOfCPUs > 0)
        {
            // Sample CPU data:
            rcCPU = _dataSampler.updateCPUsData();
            GT_IF_WITH_ASSERT(rcCPU)
            {
                if (_amountOfCPUs == 1)
                {
                    // Create a dummy vector since we need to pass parameters to the aid function:
                    double values[GS_AMOUNT_OF_COUNTERS_PER_CPU];

                    // Sample the only CPU:
                    rcCPU = updateCPUCounterValues(0, values[0], values[1], values[2], values[3], values[4]);
                }
                else // _amountOfCPUs > 1
                {
                    double totalUtil = 0.0;
                    double totalUser = 0.0;
                    double totalNice = 0.0;
                    double totalSys = 0.0;
                    double totalIdle = 0.0;

                    // The last "CPU" is the average CPU, so we cannot sample it:
                    int numberOfRealCPUs = _amountOfCPUs - 1;

                    for (int i = 0; i < numberOfRealCPUs; i++)
                    {
                        // Set the per-CPU counters values and get them back:
                        double currentUtil = 0.0, currentUser = 0.0, currentNice = 0.0, currentSys = 0.0, currentIdle = 0.0;
                        rcCPU = updateCPUCounterValues(i, currentUtil, currentUser, currentNice, currentSys, currentIdle) && rcCPU;

                        // Add the values to the totals:
                        totalUtil += currentUtil;
                        totalUser += currentUser;
                        totalNice += currentNice;
                        totalSys += currentSys;
                        totalIdle += currentIdle;
                    }

                    // Set the average "CPU"'s values:
                    int avgCPUBaseIndex = numberOfRealCPUs * GS_AMOUNT_OF_COUNTERS_PER_CPU;
                    _pCounterValues[avgCPUBaseIndex] = totalUtil / (double)numberOfRealCPUs;        // Average Utilization
                    _pCounterValues[avgCPUBaseIndex + 1] = totalUser / (double)numberOfRealCPUs;    // Average User Mode
                    _pCounterValues[avgCPUBaseIndex + 2] = totalNice / (double)numberOfRealCPUs;    // Average Nice Mode
                    _pCounterValues[avgCPUBaseIndex + 3] = totalSys / (double)numberOfRealCPUs;     // Average System Mode
                    _pCounterValues[avgCPUBaseIndex + 4] = totalIdle / (double)numberOfRealCPUs;    // Average Idle
                }
            }
        }

        // Update memory counters:
        rcMem = updateMemoryCounterValues();
    }

    bool retVal = rcCPU && rcMem;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsOSPerformanceCountersManager::initialize
// Description: Initializes the members and vectors
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool gsOSPerformanceCountersManager::initialize()
{
    // Get the number of CPUs from the data sampler:
    _amountOfCPUs = _dataSampler.cpusAmount();

    // Initialize the CPU samples vector (this is done before adding 1,
    // since we do not sample the average "CPU"):
    osCPUSampledData emptyData;

    for (int i = 0; i < _amountOfCPUs; i++)
    {
        _sampledCPUsData.push_back(emptyData);
    }

    // Add a dummy "average" CPU if there are multiple CPUs:
    if (_amountOfCPUs > 1)
    {
        _amountOfCPUs++;
    }

    // Calculate the amount of counters and assign a vector for the values:
    _amountOfCounters = (_amountOfCPUs * GS_AMOUNT_OF_COUNTERS_PER_CPU) + GS_AMOUNT_OF_MEMORY_COUNTERS;
    _pCounterValues = new double[_amountOfCounters];


    // Initialize the counters values to 0:
    for (int i = 0; i < _amountOfCounters; i++)
    {
        _pCounterValues[i] = 0.0;
    }

    bool retVal = (_amountOfCPUs > 0);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsOSPerformanceCountersManager::updateCPUCounterValues
// Description: Gets the data for CPU #cpuIndex from the sampler, sets it into
//              the appropriate indices in the values array, and returns the
//              counters' values.
// Author:      Uri Shomroni
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool gsOSPerformanceCountersManager::updateCPUCounterValues(int cpuIndex, double& cpuUsage, double& cpuUser, double& cpuNice, double& cpuSystem, double& cpuIdle)
{
    //////////////////////////////////////////////////////////////////////////
    // WARNING! // this function should be used carefully, as it assumes    //
    //          // all parameters are valid                                 //
    //////////////////////////////////////////////////////////////////////////
    // Get the recently updated CPU sample and the last updated one:
    osCPUSampledData newCPUSample;
    bool retVal = _dataSampler.getCPUData(cpuIndex, newCPUSample);
    GT_IF_WITH_ASSERT(retVal)
    {
        const osCPUSampledData& oldCPUSample = _sampledCPUsData[cpuIndex];

        // Get the differences for the values since the last sample:
        gtUInt64 totTime = newCPUSample._totalClockTicks - oldCPUSample._totalClockTicks;
        gtUInt64 userTime = newCPUSample._userClockTicks - oldCPUSample._userClockTicks;
        gtUInt64 niceTime = newCPUSample._niceClockTicks - oldCPUSample._niceClockTicks;
        gtUInt64 sysTime = newCPUSample._sysClockTicks - oldCPUSample._sysClockTicks;
        gtUInt64 idleTime = newCPUSample._idleClockTicks - oldCPUSample._idleClockTicks;

        // Calculate the percentages:
        cpuUsage = (((double)totTime - (double)idleTime) / (double)totTime) * 100.0;    // CPU i Usage
        cpuUser = ((double)userTime / (double)totTime) * 100.0;                         // CPU i User Mode
        cpuNice = ((double)niceTime / (double)totTime) * 100.0;                         // CPU i Nice Mode
        cpuSystem = ((double)sysTime / (double)totTime) * 100.0;                        // CPU i System Mode
        cpuIdle = ((double)idleTime / (double)totTime) * 100.0;                         // CPU i Idle

        // Set the values into the values vector:
        int countersBaseIndex = cpuIndex * GS_AMOUNT_OF_COUNTERS_PER_CPU;
        _pCounterValues[countersBaseIndex] = cpuUsage;
        _pCounterValues[countersBaseIndex + 1] = cpuUser;
        _pCounterValues[countersBaseIndex + 2] = cpuNice;
        _pCounterValues[countersBaseIndex + 3] = cpuSystem;
        _pCounterValues[countersBaseIndex + 4] = cpuIdle;

        // Update the CPU sample:
        _sampledCPUsData[cpuIndex] = newCPUSample;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsOSPerformanceCountersManager::updateMemoryCounterValues
// Description: Update the memory counters values
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/11/2009
// ---------------------------------------------------------------------------
bool gsOSPerformanceCountersManager::updateMemoryCounterValues()
{
    bool retVal = false;

    bool rcUpdate = _dataSampler.updatePhysicalMemoryData();
    GT_IF_WITH_ASSERT(rcUpdate)
    {
        osPhysicalMemorySampledData memoryData;
        bool rcValues = _dataSampler.getPhysicalMemoryData(memoryData);
        GT_IF_WITH_ASSERT(rcValues)
        {
            // Calculate the first index of memory counters:
            int firstMemoryCounterIndex = _amountOfCPUs * GS_AMOUNT_OF_COUNTERS_PER_CPU;

            // Update memory counters values:
            _pCounterValues[firstMemoryCounterIndex] = memoryData._totalPhysicalMemory;
            _pCounterValues[firstMemoryCounterIndex + 1] = memoryData._freeSwapMemory;
            _pCounterValues[firstMemoryCounterIndex + 2] = memoryData._wiredPages;
            _pCounterValues[firstMemoryCounterIndex + 3] = memoryData._activePages;
            _pCounterValues[firstMemoryCounterIndex + 4] = memoryData._inactivePages;

            // Used memory - the summary of: wired, active and inactive:
            _pCounterValues[firstMemoryCounterIndex + 5] = (memoryData._inactivePages + memoryData._activePages + memoryData._wiredPages);

            // Pageins and Pagesouts:
            // If we have the pages in / out history:
            if (0 < _lastPageInCount)
            {
                // Get the time elapsed since our last measurement:
                double timeInterval = 0.0;
                bool rc1 = _timeSinceLastCounterValuesUpdate.getTimeInterval(timeInterval);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // Calculate the amount of paged in / out pages since the last measurement:
                    gtUInt32 amountOfPagedInPages = memoryData._pageIns - _lastPageInCount;
                    gtUInt32 amountOfPagedOutPages = memoryData._pageOuts - _lastPageOutCount;

                    // Calculate and export paged out / in per sec:
                    double pagein = (double)amountOfPagedInPages / timeInterval;
                    double pageout = (double)amountOfPagedOutPages / timeInterval;
                    _pCounterValues[firstMemoryCounterIndex + 6] = pagein;
                    _pCounterValues[firstMemoryCounterIndex + 7] = pageout;
                    retVal = true;
                }
                else
                {
                    // Error occurred:
                    _pCounterValues[firstMemoryCounterIndex + 6] = 0;
                    _pCounterValues[firstMemoryCounterIndex + 7] = 0;
                    retVal = false;
                }
            }
            else
            {
                // We don't have paged in / out history; export 0 counter values:
                _pCounterValues[firstMemoryCounterIndex + 6] = 0;
                _pCounterValues[firstMemoryCounterIndex + 7] = 0;
                retVal = true;
            }

            // Store the current paged in / out values:
            _lastPageInCount = memoryData._pageIns;
            _lastPageOutCount = memoryData._pageOuts;

            // Restart the timer:
            _timeSinceLastCounterValuesUpdate.start();
        }
    }

    return retVal;
}


