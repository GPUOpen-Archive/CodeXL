//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLQueuePerformanceCountersReader.h
///
//==================================================================================

//------------------------------ csOpenCLQueuePerformanceCountersReader.h ------------------------------

#ifndef __CSOPENCLQUEUEPERFORMANCECOUNTERSREADER__H
#define __CSOPENCLQUEUEPERFORMANCECOUNTERSREADER__H

// Forward declarations:
class csCommandQueueMonitor;

// Infra:
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apOpenCLQueueCommandsCategories.h>

// ----------------------------------------------------------------------------------
// Class Name:          csOpenCLQueuePerformanceCountersReader
// General Description: Manages and calculated a single OpenCL queue performance counters.
// Author:              Sigal Algranaty
// Creation Date:       9/3/2010
// ----------------------------------------------------------------------------------
class csOpenCLQueuePerformanceCountersReader
{
public:
    csOpenCLQueuePerformanceCountersReader();
    virtual ~csOpenCLQueuePerformanceCountersReader();

    // Events:
    void onFrameTerminatorCall();

    bool updateCounterValues();

    // Set the command queue monitor:
    void setCommandQueueMonitor(csCommandQueueMonitor* pCommandQueueMonitor) {_pCommandQueueMonitor = pCommandQueueMonitor;}

    // Set the counters values pointer:
    void setCounterValuesPointer(double* pCounterValues);

    // Size values functions:
    void addToWorkItemsCount(gtUInt64 workItemCount) {_workItemCount += workItemCount;}
    void addToReadSize(gtUInt64 readSize) {_readSize += readSize;}
    void addToWriteSize(gtUInt64 writeSize) {_writeSize += writeSize;}
    void addToCopySize(gtUInt64 copySize) {_copySize += copySize;}

private:
    bool updateTimeTotals();

    double calculateAvarageFramePerSecondRatio();
    double calculateKernelUtilization();
    double calculateWriteUtilization();
    double calculateCopyUtilization();
    double calculateReadUtilization();
    double calculateOtherUtilization();
    double calculateIdleTime();
    double calculateWorkItemSize(double amountOfSeconds);
    double calculateReadSize(double amountOfSeconds);
    double calculateWriteSize(double amountOfSeconds);
    double calculateCopySize(double amountOfSeconds);

    void restartPeriodicCounters();

private:
    // The queue monitor related to this reader:
    csCommandQueueMonitor* _pCommandQueueMonitor;

    // Pointer used for counter values:
    double* _pCountersValues;

    // Counters value bases:
    gtUInt64 _totalTime;
    gtUInt64 _totalBusyTime;
    double _busyPercentage;
    gtUInt64 _totalBusyTimeWithDuplicates;
    gtUInt64 _busyTimeByCategory[AP_NUMBER_OF_QUEUE_COMMAND_CATEGORIES];
    gtUInt64 _workItemCount;
    gtUInt64 _readSize;
    gtUInt64 _writeSize;
    gtUInt64 _copySize;

    // Timer for the 'Per/Second' counters:
    osStopWatch _timeSinceLastCounterValuesUpdate;

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

    // Contains the sum of all time intervals consumed by frames that their
    // computation was ended:
    double _endedFramesTimeIntervalsSum;

    // A frame counter that counts the amount of frames passed since the last
    // time the counter values were updated:
    int _frameCounter;

    // Stop watch used for counting frames/sec:
    osStopWatch _fpsStopWatch;
};


#endif  // __CSOPENCLQUEUEPERFORMANCECOUNTERSREADER__H
