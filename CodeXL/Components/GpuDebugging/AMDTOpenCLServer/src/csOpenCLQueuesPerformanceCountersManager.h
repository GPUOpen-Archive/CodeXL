//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLQueuesPerformanceCountersManager.h
///
//==================================================================================

//------------------------------ csOpenCLQueuesPerformanceCountersManager.h ------------------------------

#ifndef __CSOPENCLQUEUESPERFORMANCECOUNTERSMANAGER_H
#define __CSOPENCLQUEUESPERFORMANCECOUNTERSMANAGER_H

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/apCounterScope.h>

// Forward declarations:
class csCommandQueueMonitor;
class csOpenCLQueuePerformanceCountersReader;


// ----------------------------------------------------------------------------------
// Class Name:          csOpenCLQueuesPerformanceCountersManager
// General Description: Holds and manages all OpenCL queues performance counters.
// Author:              Sigal Algranaty
// Creation Date:       9/3/2010
// ----------------------------------------------------------------------------------
class csOpenCLQueuesPerformanceCountersManager
{
public:
    csOpenCLQueuesPerformanceCountersManager();
    virtual ~csOpenCLQueuesPerformanceCountersManager();

    int countersAmount() const { return _countersAmount; };
    bool updateCounterValues();
    const double* getCounterValues() const { return (const double*)_pCountersValuesSnapshot; }

    // OpenCL Events:
    void onQueueCreatedEvent(csCommandQueueMonitor* pCommandQueueMonitor);

private:
    // The amount of performance counters (combined for all queues):
    // This size is dynamic and is changed according to queues creation:
    int _countersAmount;

    // Pointer contains the performance counters' values.
    // This array is of _countersAmout size.
    double* _pCountersValuesSnapshot;

    // Contains a map from queue handle to Queue counters readers:
    gtMap<apCounterScope, csOpenCLQueuePerformanceCountersReader*> _existingQueuesPerfReaders;

    // Vector containing the existing queues:
    gtVector<apCounterScope> _existingQueues;
};

#endif //__CSOPENCLQUEUESPERFORMANCECOUNTERSMANAGER_H

