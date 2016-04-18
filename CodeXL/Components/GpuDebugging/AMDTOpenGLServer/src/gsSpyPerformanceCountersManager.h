//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsSpyPerformanceCountersManager.h
///
//==================================================================================

//------------------------------ gsSpyPerformanceCountersManager.h ------------------------------

#ifndef __GSSPYPERFORMANCECOUNTERSMANAGER_H
#define __GSSPYPERFORMANCECOUNTERSMANAGER_H

// Pre-decelerations:
class gsContextCreatedEvent;
class gsContextDeletedEvent;

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLServerPerformanceCountersDefinitions.h>

// Local:
#include <src/gsRenderContextPerformanceCountersManager.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsSpyPerformanceCountersManager
// General Description: Holds and manages the performance counters.
// Author:               Yaki Tebeka
// Creation Date:        26/7/2005
// ----------------------------------------------------------------------------------
class gsSpyPerformanceCountersManager
{
public:
    gsSpyPerformanceCountersManager();
    virtual ~gsSpyPerformanceCountersManager();

    int countersAmount() const { return _countersAmount; };
    bool updateCounterValues();
    const double* getCounterValues() const { return (const double*)_pCountersValuesSnapshot; }

    // OpenGL Events:
    void onContextCreatedEvent(int contextSpyId);
    void onContextDeletedEvent(int contextSpyId);

private:
    int performanceCountersManagerByContextId(int contextId) const;

private:
    // The amount of performance counters (combined for all render context):
    // This size is dynamic and is changed according to context creation and deletion.
    int _countersAmount;

    // Pointer contains the performance counters' values.
    // This array is of _countersAmout size.
    double* _pCountersValuesSnapshot;

    // Contains pointers to per render context performance counters managers
    // Context performance managers are added and removed in context deletion and creation
    gtVector<gsRenderContextPerformanceCountersManager*> _existingContextsPerfMgrs;
};

#endif //__GSSPYPERFORMANCECOUNTERSMANAGER_H
