//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csAMDQueuePerformanceCountersReader.h
///
//==================================================================================

//------------------------------ csAMDQueuePerformanceCountersReader.h ------------------------------

#ifndef __CSATIRENDERCONTEXTPERFORMANCECOUNTERSREADER_H
#define __CSATIRENDERCONTEXTPERFORMANCECOUNTERSREADER_H

// Forward decelerations:
class csAMDQueuePerformanceCountersReader;

#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#ifdef OA_DEBUGGER_USE_AMD_GPA

// ATI:
#include <GPUPerfAPIFunctionTypes.h>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>

// Pre declaration:
class csCommandQueueMonitor;
class csAMDPerformanceCountersManager;

// ----------------------------------------------------------------------------------
// Class Name:           csAMDQueuePerformanceCountersReader
// General Description:
//  Manages and calculated a single OpenCL queue ATI OpenCL performance counters.
// Author:               Sigal Algranaty
// Creation Date:        28/02/2010
// ----------------------------------------------------------------------------------
class csAMDQueuePerformanceCountersReader
{
public:
    csAMDQueuePerformanceCountersReader(int contextSpyId, int commandQueueIndex);
    virtual ~csAMDQueuePerformanceCountersReader();

    // Initialize & Terminate:
    void initialize();
    void terminate();

    // Activates counters:
    bool registerCountersForActivation(const gtVector<apCounterActivationInfo>& counterActivationInfos);

    // Set the counters values pointer:
    void setCounterValuesPointer(double* pCounterValues);

    // Event callbacks:
    void onFrameTerminatorCall();
    void onFirstTimeContextMadeCurrent();

    // Set command queue monitor:
    void setCommandQueueMonitor(csCommandQueueMonitor* pCommandQueueMonitor) {_pCommandQueueMonitor = pCommandQueueMonitor;};

    // Spy ID:
    int contextSpyId() const { return _contextSpyId; };

    // Command Queue id:
    int commandQueueId() const { return _commandQueueId; };

private:

    // Do not allow the use of my default constructor:
    csAMDQueuePerformanceCountersReader();

    // Sampling help functions:

    // Handles sampling on each frame terminator:
    bool performSampling();

    // Begins a sampling session:
    bool beginSamplingSession();
    bool endSamplingSession();

    // Checks the ATI card for counters results:
    bool checkIfResultsAreReady();

    // Queries the counter sampling values:
    bool getCountersValues();

    // Begin / End one sampling pass:
    bool beginOnePass();
    bool endOnePass();

    // Activates counters (the counters list for activation is initialized before, this function actually activates the counter):
    bool activateCounters();

private:

    // The class holds information regarding the sampling process. The sampling process
    // is handled as state machine, so this class holds the current sampling state, and other
    // data necessary for the sampling process:
    class csATISamplingInfo
    {
    public:

        // Enumeration holding the ATI sampling stateL
        enum csATISamplingState
        {
            CS_ATI_NO_COUNTERS_TO_SAMPLE,                       // No need to sample
            CS_ATI_COUNTERS_ARE_WAITING_FOR_ACTIVATION,         // There are counters that are waiting for activation:
            CS_ATI_SESSION_NOT_INITIALIZED,                     // Counters are initialized, and session should be initialized:
            CS_ATI_IN_SESSION,                                  // Within a session:
            CS_ATI_WAITING_FOR_RESULTS                          // Waiting for a session results:
        };

        // Constructor:
        csATISamplingInfo():
            _samplingState(CS_ATI_NO_COUNTERS_TO_SAMPLE),
            _currentWaitSessionID(1),
            _amountOfRequiredPasses(0),
            _amountOfPassesPerformedForThisSession(0),
            _shouldActivateCounters(false)
        {}

        // The current sampling state:
        csATISamplingState _samplingState;

        // Identifier holding the current ATI session id:
        gpa_uint32 _currentWaitSessionID;

        // In order to sample the current group of disabled counters, we need to open defined amount of passes.
        // This member consist of required the number of passes, and is updated whenever a counter is enabled / disabled:
        gpa_uint32 _amountOfRequiredPasses;

        // Holds the currently opened passes:
        gpa_uint32 _amountOfPassesPerformedForThisSession;

        // True iff new counters activation / deactivation should be performed:
        bool _shouldActivateCounters;
    };

private:

    // A flag that indicates whether this object has been initialized:
    bool _isInitialized;
    bool _initializationFailed;

    // A flag that tells us if this is the first frame terminator call:
    bool _isFirstFrameTerminatorCall;

    // Amount of existing counters:
    int _countersAmount;

    // Amount of enabled counters:
    gpa_uint32 _enabledCountersAmount;

    // Represents the command queue which I monitor:
    csCommandQueueMonitor* _pCommandQueueMonitor;

    // Pointer used for counter values:
    double* _pCountersValues;

    // A pointer to the general ATI Manager that manages me:
    csAMDPerformanceCountersManager* _pGeneralATIPerfomanceCounterManager;

    // A critical section that synchronize the access to this class members between:
    // a. The API thread:
    // b. Threads that render into the render context that this class monitors:
    osCriticalSection _threadSyncCriticalSection;

    // Contain information regarding the sampling process, which is handled in the frame terminator:
    csATISamplingInfo _samplingInfo;

    // Contain true iff counter i is enabled:
    bool* _pCounterIsEnabled;

    // Spy & command queue ids:
    int _contextSpyId;
    int _commandQueueId;

    // Timer used for counters monitoring:
    osStopWatch _stopWatch;
};

#endif // OA_DEBUGGER_USE_AMD_GPA

#endif //__csATIRenderContextPerformanceCountersReader_H

