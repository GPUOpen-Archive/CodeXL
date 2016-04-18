//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsRenderContextPerformanceCountersManager.h
///
//==================================================================================

//------------------------------ gsRenderContextPerformanceCountersManager.h ------------------------------

#ifndef __GSRENDERCONTEXTPERFORMANCECOUNTERSMANAGER
#define __GSRENDERCONTEXTPERFORMANCECOUNTERSMANAGER

// Forward decelerations:
class gsRenderContextMonitor;

// Infra:
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLServerPerformanceCountersDefinitions.h>

// ----------------------------------------------------------------------------------
// Class Name:           gsRenderContextPerformanceCountersManager
// General Description:
//  Manages and calculated a single render context performance counters.
//
// Author:               Yaki Tebeka
// Creation Date:        26/7/2005
// ----------------------------------------------------------------------------------
class gsRenderContextPerformanceCountersManager
{
public:
    gsRenderContextPerformanceCountersManager();
    virtual ~gsRenderContextPerformanceCountersManager();

    void setMonitoredRenderContext(gsRenderContextMonitor& monitoredRenderContext);
    bool updateCounterValues(double* pCountersValuesArray);

    // Event callbacks:
    void onFrameTerminatorCall();
    void onMonitoredFunctionCall() { _currentFrameFunctionsCalls++; };

    // Accessors:
    int renderContextId() const {return _renderContextId;};

private:
    double getAvarageFramePerSecondRatio();
    double getAvarageFrameTime();
    double getAvarageFuncCallsPerFrameRatio();

    void restartPeriodicCounters();

private:
    // Represents the render context which I monitor:
    gsRenderContextMonitor* _pMonitoredRenderContext;

    // The id of the context who's performance is monitored:
    int _renderContextId;

    // A stop watch that measures the current frame elapsed time:
    osStopWatch _stopWatch;

    // A frame counter that counts the amount of frames passed since the last
    // time the counter values were updated:
    int _frameCounter;

    // Contains the sum of all time intervals consumed by frames that their
    // rendering was ended:
    double _endedFramesTimeIntervalsSum;

    // The amount of monitored function calls executed in the current frame:
    long _currentFrameFunctionsCalls;

    // The amount of monitored function calls executed in frames that were ended:
    long _endedFramesFunctionCalls;

    // A critical section that synchronize the access to this class members between:
    // a. The API thread
    // b. The thread that render into the render context that this class monitors
    osCriticalSection _threadSyncCriticalSection;

    // When we are running so slow that the performance counters rate is higher than the
    // frame rate, we might query performance counters before they have a real value.
    // if this is the case, we want to show the last real value we had.
    // This situation will typically happen in heavy apps in Analyze mode, or otherwise with a high
    // with a high slow motion delay time.
    double _lastRealFPSValue;
    double _lastRealFrameTimeValue;
    double _lastRealOGLCallsValue;
};


#endif  // __GSRENDERCONTEXTPERFORMANCECOUNTERSMANAGER
