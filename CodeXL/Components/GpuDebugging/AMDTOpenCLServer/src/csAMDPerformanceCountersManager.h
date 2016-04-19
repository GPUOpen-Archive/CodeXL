//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csAMDPerformanceCountersManager.h
///
//==================================================================================

//------------------------------ csAMDPerformanceCountersManager.h -----------------------------

#ifndef __CSATIPERFORMANCECOUNTERSMANAGER_H
#define __CSATIPERFORMANCECOUNTERSMANAGER_H

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSAPIWrappers/Include/oaATIFunctionWrapper.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTAPIClasses/Include/apCounterScope.h>

#ifdef OA_DEBUGGER_USE_AMD_GPA

// Forward declaration
class csAMDQueuePerformanceCountersReader;
class csCommandQueueMonitor;

// ----------------------------------------------------------------------------------
// Class Name:           csAMDPerformanceCountersManager
// General Description: Holds and manages ATI performance counters.
// Author:               Sigal Algranaty
// Creation Date:        28/2/2010
// ----------------------------------------------------------------------------------
class csAMDPerformanceCountersManager
{
public:
    csAMDPerformanceCountersManager(bool initialize);
    virtual ~csAMDPerformanceCountersManager();

    // Initialize:
    void initialize(int numberOfATICountersPerContext);

    const double* getCounterValues() const { return _pCountersValuesSnapshot; }
    int getCountersAmount() const {return _totalCountersAmount; };
    bool activateCounters(const gtVector<apCounterActivationInfo>& counterActivationInfosVec);

    // OpenCL Events:
    void onQueueCreatedEvent(csCommandQueueMonitor* pCommandQueueMonitor);
    void onQueueDeletedEvent(oaCLCommandQueueHandle queueHandle);

    void getCounterForActivation(int contextId, int queueCounterID, gtVector<apCounterActivationInfo>& counterActivationInfo);

    bool terminate();

private:

    // Indicates whether the ATI library is initialized:
    bool _isATILibraryInitialized;

    // Indicates whether csAMDPerformanceCountersManager has been initialized.
    // This indicates whether the first render context has been created
    bool _isInitialized;

    // The amount of ATI performance counters per context:
    int _ATIperContextCountersAmount;

    // The amount of performance counters:
    int _totalCountersAmount;

    // The performance counter values snapshot, last updated by updateCounterValues():
    double* _pCountersValuesSnapshot;

    // Contains a map from queue handle to Queue counters readers:
    gtMap<apCounterScope, csAMDQueuePerformanceCountersReader*> _existingQueuesPerfReaders;

    // Vector containing the existing queues:
    gtVector<apCounterScope> _existingQueues;

    // Vector of activation info the counters:
    gtVector<apCounterActivationInfo> _countersActivationVector;


};

#endif // OA_DEBUGGER_USE_AMD_GPA

#endif //__CSATIPERFORMANCECOUNTERSMANAGER_H

