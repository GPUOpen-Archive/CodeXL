//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsATIPerformanceCountersManager.h
///
//==================================================================================

//------------------------------ gsATIPerformanceCountersManager.h -----------------------------
#ifndef __GSATIPERFORMANCECOUNTERSMANAGER_H
#define __GSATIPERFORMANCECOUNTERSMANAGER_H

// Pre-decelerations:
class gsContextCreatedEvent;
class gsContextDeletedEvent;

#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#ifdef OA_DEBUGGER_USE_AMD_GPA

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/oaATIFunctionWrapper.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apCounterID.h>

// Forward declaration
class gsATIRenderContextPerformanceCountersReader;

// ----------------------------------------------------------------------------------
// Class Name:           gsATIPerformanceCountersManager
// General Description: Holds and manages ATI performance counters.
// Author:               Yaki Tebeka
// Creation Date:        26/7/2005
// ----------------------------------------------------------------------------------
class gsATIPerformanceCountersManager
{
public:
    gsATIPerformanceCountersManager(bool initialize);
    virtual ~gsATIPerformanceCountersManager();

    // Initialize:
    void initialize(int numberOfATICountersPerContext);

    const double* getCounterValues() const { return _pCountersValuesSnapshot; }
    int getCountersAmount() const {return _totalCountersAmount; };
    bool activateCounters(const gtVector<apCounterActivationInfo>& counterActivationInfosVec);

    // OpenGL Events:
    void onContextCreatedEvent(int contextSpyId);
    void onContextDeletedEvent(int contextSpyId);

    void getCounterForActivation(int renderContextId, gtVector<apCounterActivationInfo>& counterActivationInfo);

    bool terminate();

private:

    int performanceCountersManagerByContextId(int contextId) const;

private:

    // Indicates whether the ATI library is initialized:
    bool _isATILibraryInitialized;

    // Indicates whether gsATIPerformanceCountersManager has been initialized.
    // This indicates whether the first render context has been created
    bool _isInitialized;

    // The amount of ATI performance counters per context:
    int _ATIperContextCountersAmount;

    // The amount of performance counters:
    int _totalCountersAmount;

    // The performance counter values snapshot, last updated by updateCounterValues():
    double* _pCountersValuesSnapshot;

    // Contains pointers to per render context performance counters managers
    // The readers are member of gsRenderContextMonitor class, and are managed here:
    gtVector<gsATIRenderContextPerformanceCountersReader*> _existingContextsPerfMgrs;

    // Vector of activation info the counters:
    gtVector<apCounterActivationInfo> _countersActivationVector;

};

#endif // OA_DEBUGGER_USE_AMD_GPA

#endif //__GSATIPERFORMANCECOUNTERSMANAGER_H
