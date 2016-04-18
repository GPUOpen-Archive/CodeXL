//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csAMDQueuePerformanceCountersReader.cpp
///
//==================================================================================

//------------------------------ csAMDQueuePerformanceCountersReader.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTAPIClasses/Include/apCounterID.h>

// Local:
#include <src/csAMDPerformanceCountersManager.h>
#include <src/csAMDQueuePerformanceCountersReader.h>
#include <src/csContextMonitor.h>
#include <src/csOpenCLMonitor.h>
#include <src/csStringConstants.h>

#ifdef OA_DEBUGGER_USE_AMD_GPA

#define CS_AMD_COUNTERS_TIME_INTERVAL 0.1


// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::csAMDQueuePerformanceCountersReader
// Description: Constructor
// Return Val:
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
csAMDQueuePerformanceCountersReader::csAMDQueuePerformanceCountersReader(int contextSpyId, int commandQueueIndex)
    : _isInitialized(false), _initializationFailed(false), _isFirstFrameTerminatorCall(true), _countersAmount(0), _enabledCountersAmount(0),
      _pCommandQueueMonitor(NULL), _pCountersValues(NULL), _pGeneralATIPerfomanceCounterManager(NULL), _pCounterIsEnabled(NULL),
      _contextSpyId(contextSpyId), _commandQueueId(commandQueueIndex), _stopWatch()
{
    csAMDPerformanceCountersManager& theAMDPerformanceCountersMgr = csOpenCLMonitor::instance().AMDPerformanceCountersManager();
    _pGeneralATIPerfomanceCounterManager = &theAMDPerformanceCountersMgr;
}


// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::~csAMDQueuePerformanceCountersReader
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
csAMDQueuePerformanceCountersReader::~csAMDQueuePerformanceCountersReader()
{
    // Terminate the reader:
    terminate();
}

// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::initialize
// Description: Asks ATI infra for information regarding the amount of counters per render context.
//              This call should be done after a render context has been obtained, otherwise it would crash,
//              due to call to ATI infra without a live context.
// Return Val: void
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
void csAMDQueuePerformanceCountersReader::initialize()
{
    // If we were not initialized:
    if ((!_isInitialized) && (!_initializationFailed))
    {
        // Output debug log message:
        OS_OUTPUT_DEBUG_LOG(CS_STR_initializingATICounters, OS_DEBUG_LOG_INFO);

        GT_IF_WITH_ASSERT(_pCommandQueueMonitor != NULL)
        {
            // Get the command queue handle;
            oaCLCommandQueueHandle hCommandQueue = _pCommandQueueMonitor->commandQueueInfo().commandQueueHandle();

            // Open the ATI performance counters library connection:
            oaATIFunctionWrapper& functionWrapper = oaATIFunctionWrapper::cl_instance();
            GPA_Status gpaStatus =  functionWrapper.GPA_OpenContext((void*)hCommandQueue);
            GT_IF_WITH_ASSERT(gpaStatus == GPA_STATUS_OK)
            {
                // Get amount of supported counters:
                gpa_uint32 amountOfCounters = 0;
                gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_GetNumCounters(&amountOfCounters);
                GT_IF_WITH_ASSERT(gpaStatus == GPA_STATUS_OK)
                {
                    // Reset amount of enabled counters:
                    _enabledCountersAmount = 0;

                    // Allocate space for counter activation vector:
                    _pCounterIsEnabled = new bool[amountOfCounters];


                    for (int i = 0; i < (int)amountOfCounters; i++)
                    {
                        _pCounterIsEnabled[i] = false;
                    }

                    // Initialize amount of counters:
                    _countersAmount = (int)amountOfCounters;

                    if (_countersAmount > 0)
                    {
                        GT_IF_WITH_ASSERT(_pGeneralATIPerfomanceCounterManager != NULL)
                        {
                            // Inform the general ATI manager of the amount of ATI counters per context:
                            _pGeneralATIPerfomanceCounterManager->initialize(_countersAmount);

                            // Mark that initialization is done:
                            _isInitialized = true;
                        }
                    }
                }
            }
        }

        // Output success / failure message to the debug log file:
        if (_isInitialized)
        {
            OS_OUTPUT_DEBUG_LOG(CS_STR_ATICountersInitialized, OS_DEBUG_LOG_INFO);
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(CS_STR_ATICountersInitializationFailed, OS_DEBUG_LOG_INFO);
            _initializationFailed = true;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::terminate
// Description: Deallocates the space that was allocated on initialization.
// Return Val: void
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
void csAMDQueuePerformanceCountersReader::terminate()
{
    if (_isInitialized)
    {
        _isInitialized = false;

        if (_pCounterIsEnabled != NULL)
        {
            delete [] _pCounterIsEnabled;
        }
    }

    // Stop the stop watch run:
    if (_stopWatch.isRunning())
    {
        _stopWatch.stop();
    }
}


// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::onFrameTerminatorCall
// Description: Is called when a frame terminator function call is executed
//              within the monitored context.
//              Frame terminator is handling the sampling process. On each frame terminator,
//              we advance the sampling process one step further
// Return Val: void
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
void csAMDQueuePerformanceCountersReader::onFrameTerminatorCall()
{
    // Verify that we talk to ATI infra only if the initialization was complete:
    if (_isInitialized)
    {
        // On first frame we do nothing:
        if (_isFirstFrameTerminatorCall)
        {
            // Mark that this function was called:
            _isFirstFrameTerminatorCall = false;
        }
        else
        {
            // Perform sampling:
            bool rcSampling = performSampling();
            GT_ASSERT(rcSampling);
        }
    }
    else
    {
        onFirstTimeContextMadeCurrent();
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::performSampling
// Description: This function perform one sampling state, according to the current
//              sampling process state
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
bool csAMDQueuePerformanceCountersReader::performSampling()
{
    bool retVal = false;

    // Check if the time slot had passed:
    double timePassed = 0.0;
    bool rc = _stopWatch.getTimeInterval(timePassed);
    bool doPerformSampling = (timePassed > CS_AMD_COUNTERS_TIME_INTERVAL);

    if (doPerformSampling)
    {
        // Check where we are within the sampling process:
        switch (_samplingInfo._samplingState)
        {

            case csATISamplingInfo::CS_ATI_NO_COUNTERS_TO_SAMPLE:
                // Do nothing, no counters:
                retVal = true;
                break;

            // There are counters waiting for activation, the process should be reset:
            case csATISamplingInfo::CS_ATI_COUNTERS_ARE_WAITING_FOR_ACTIVATION:
                retVal = activateCounters();
                break;

            // We need to initialize a sampling session:
            case csATISamplingInfo::CS_ATI_SESSION_NOT_INITIALIZED:
                retVal = beginSamplingSession();
                break;

            // Sampling session was initialized:
            case csATISamplingInfo::CS_ATI_IN_SESSION:
            {
                // There are still passes to perform:
                if (_samplingInfo._amountOfPassesPerformedForThisSession < _samplingInfo._amountOfRequiredPasses)
                {
                    retVal = beginOnePass();
                }

                // No more passes to perform, end pass and session:
                else if (_samplingInfo._amountOfPassesPerformedForThisSession == _samplingInfo._amountOfRequiredPasses)
                {
                    bool rc = endOnePass();

                    if (rc)
                    {
                        // End the sampling session:
                        retVal = endSamplingSession();
                    }
                }
            }
            break;

            // We wait for ATI counters results:
            case csATISamplingInfo::CS_ATI_WAITING_FOR_RESULTS:
            {
                bool isResultReady = checkIfResultsAreReady();

                if (isResultReady)
                {
                    retVal = getCountersValues();
                }
            }
            break;

            default:
                // We should not get here:
                GT_ASSERT_EX(false, CS_STR_ATICountersSamplingUnknownState);
                retVal = false;
                break;
        }

        // Restart the watch
        _stopWatch.stop();
        _stopWatch.start();
    }
    else
    {
        retVal = true;
    }


    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::checkIfResultsAreReady
// Description: Checks if the counters results are ready for the last ended session
// Return Val: void
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
bool csAMDQueuePerformanceCountersReader::checkIfResultsAreReady()
{
    bool retVal = false;

    // Check if the session result is ready:
    GPA_Status sessionStatus = oaATIFunctionWrapper::cl_instance().GPA_IsSessionReady(&retVal, _samplingInfo._currentWaitSessionID);

    if (sessionStatus != GPA_STATUS_OK)
    {
        retVal = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::getCountersValues
// Description: Get counters results from the ATI library.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
bool csAMDQueuePerformanceCountersReader::getCountersValues()
{
    bool retVal = true;

    // If counters values pointer is initialized:
    if (_pCountersValues != NULL)
    {
        // Lock the access to the threads common variables:
        osCriticalSectionLocker csLocker(_threadSyncCriticalSection);

        // Reset counters values:
        memset(_pCountersValues, 0, _countersAmount * sizeof(int));

        // Get amount of enabled counters:
        gpa_uint32 amountOfEnabledCounters = 0;
        GPA_Status gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_GetEnabledCount(&amountOfEnabledCounters);
        GT_IF_WITH_ASSERT(gpaStatus == GPA_STATUS_OK)
        {
            // Sanity check:
            GT_ASSERT(amountOfEnabledCounters == _enabledCountersAmount);

            // Iterate the counters:
            for (gpa_uint32 currentCounterIndex = 0 ; currentCounterIndex < amountOfEnabledCounters ; currentCounterIndex++)
            {
                // Get the ATI current enabled counter index (which is used for queries):
                gpa_uint32 enabledCounterIndex = 0;
                gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_GetEnabledIndex(currentCounterIndex, &enabledCounterIndex);

                // Get the current counter data type:
                GPA_Type atiCounterDataType = GPA_TYPE__LAST;
                GPA_Status gpaStatusDataType = oaATIFunctionWrapper::cl_instance().GPA_GetCounterDataType(enabledCounterIndex, &atiCounterDataType);
                GT_IF_WITH_ASSERT(gpaStatusDataType == GPA_STATUS_OK)
                {
                    // Sanity check:
                    bool isValidIndex = ((enabledCounterIndex >= 0) && (enabledCounterIndex < (gpa_uint32)_countersAmount));
                    GT_IF_WITH_ASSERT(isValidIndex)
                    {
                        GPA_Status sampleStatus = GPA_STATUS_ERROR_FAILED;

                        // Get the sample value according to the counter data type:
                        switch (atiCounterDataType)
                        {
                            case GPA_TYPE_UINT32:
                            {
                                // Get the value as 32bit int:
                                gpa_uint32 value = 0;
                                sampleStatus = oaATIFunctionWrapper::cl_instance().GPA_GetSampleUInt32(_samplingInfo._currentWaitSessionID, 1, enabledCounterIndex, &value);
                                _pCountersValues[enabledCounterIndex] = (double)value;
                            }
                            break;

                            case GPA_TYPE_UINT64:
                            {
                                // Get the value as 64bit uint:
                                gpa_uint64 value = 0;
                                sampleStatus = oaATIFunctionWrapper::cl_instance().GPA_GetSampleUInt64(_samplingInfo._currentWaitSessionID, 1, enabledCounterIndex, &value);
                                _pCountersValues[enabledCounterIndex] = (double)value;
                            }
                            break;

                            case GPA_TYPE_FLOAT32:
                            {
                                // Get the value as 32bit float:
                                gpa_float32 value = 0;
                                sampleStatus = oaATIFunctionWrapper::cl_instance().GPA_GetSampleFloat32(_samplingInfo._currentWaitSessionID, 1, enabledCounterIndex, &value);
                                _pCountersValues[enabledCounterIndex] = (double)value;
                            }
                            break;

                            case GPA_TYPE_FLOAT64:
                            {
                                // Get the value as 64bit float:
                                gpa_float64 value = 0;
                                sampleStatus = oaATIFunctionWrapper::cl_instance().GPA_GetSampleFloat64(_samplingInfo._currentWaitSessionID, 1, enabledCounterIndex, &value);
                                _pCountersValues[enabledCounterIndex] = (double)value;
                            }
                            break;

                            default:
                                GT_ASSERT_EX(false, CS_STR_ATICountersSamplingUnknownDataType);
                                break;

                        }

                        // Notify failure to log file:
                        GT_ASSERT_EX((sampleStatus == GPA_STATUS_OK), CS_STR_ATICountersSamplingCounterValueFailure);
                    }
                }
            }
        }

        // Unlock the access to the threads common variables:
        csLocker.leaveCriticalSection();
    }

    if (_samplingInfo._shouldActivateCounters)
    {
        // Next state is counters activation:
        _samplingInfo._samplingState = csATISamplingInfo::CS_ATI_COUNTERS_ARE_WAITING_FOR_ACTIVATION;
    }
    else
    {
        // Next step is session initialization:
        _samplingInfo._samplingState = csATISamplingInfo::CS_ATI_SESSION_NOT_INITIALIZED;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::onFirstTimeContextMadeCurrent
// Description: Called when this render context becomes current for the first time.
//              Initializes this object and initializes the ATI infra to Activate all the counters.
// Return Val: void
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
void csAMDQueuePerformanceCountersReader::onFirstTimeContextMadeCurrent()
{
    // Upon entering the call, we are sure to have an active render context (otherwise the call to initialize would crash)
    // initializing
    initialize();

    // Activate the counters registered for activation from the manager:
    GT_IF_WITH_ASSERT(_pGeneralATIPerfomanceCounterManager != NULL)
    {
        // Get counters waiting for activation:
        gtVector<apCounterActivationInfo> countersForActivation;
        _pGeneralATIPerfomanceCounterManager->getCounterForActivation(_contextSpyId, _commandQueueId, countersForActivation);

        // Register the counters for activation:
        registerCountersForActivation(countersForActivation);

        // Start the stop watch run:
        _stopWatch.start();
    }
}


// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::registerCountersForActivation
// Description: Activate / Deactivate ATI vector of counters
// Arguments: const gtVector<apCounterActivationInfo>& counterActivationInfos
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
bool csAMDQueuePerformanceCountersReader::registerCountersForActivation(const gtVector<apCounterActivationInfo>& counterActivationInfos)
{
    bool retVal = true;

    // Copy the counters activations info:
    for (int i = 0; i < (int)counterActivationInfos.size() ; i++)
    {
        const apCounterActivationInfo& activationInfo = counterActivationInfos[i];

        // Update the activated flag for this counter:
        int counterLocalIndex = activationInfo._counterId._counterLocalIndex;
        GT_IF_WITH_ASSERT((counterLocalIndex < _countersAmount) && (counterLocalIndex >= 0))
        {
            _pCounterIsEnabled[counterLocalIndex] = activationInfo._shouldBeActivated;
        }
    }

    // Notify the sampling info for counters waiting for activation:
    _samplingInfo._shouldActivateCounters = true;

    if (_samplingInfo._samplingState == csATISamplingInfo::CS_ATI_NO_COUNTERS_TO_SAMPLE)
    {
        _samplingInfo._samplingState = csATISamplingInfo::CS_ATI_COUNTERS_ARE_WAITING_FOR_ACTIVATION;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::activateCounters
// Description: Activate / Deactivate requested counter against the ATI library
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
bool csAMDQueuePerformanceCountersReader::activateCounters()
{
    bool retVal = true;

    // If we're in the middle of a sampling session:
    if (0 < _samplingInfo._amountOfPassesPerformedForThisSession)
    {
        // Before doing anything, close open session:
        bool rcCloseSession = endSamplingSession();
        GT_ASSERT(rcCloseSession);
    }

    // Disable all counters:
    GPA_Status gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_DisableAllCounters();
    GT_ASSERT(gpaStatus == GPA_STATUS_OK);

    // Go through all the counters:
    int counterATIIndex = 0;

    for (int counterLocalIndex = 0 ; counterLocalIndex < _countersAmount; counterLocalIndex++)
    {
        // Check if this counter should be activated:
        bool shouldBeActivated = _pCounterIsEnabled[counterLocalIndex];

        if (shouldBeActivated)
        {
            gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_EnableCounter(counterLocalIndex);
            retVal = retVal && (gpaStatus == GPA_STATUS_OK);
        }
    }

    // Update amount of enabled counters:
    gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_GetEnabledCount(&_enabledCountersAmount);
    GT_ASSERT(gpaStatus == GPA_STATUS_OK);

    if (_enabledCountersAmount > 0)
    {
        // Update the sampling info (the next sampling state is session initialization):
        _samplingInfo._samplingState = csATISamplingInfo::CS_ATI_SESSION_NOT_INITIALIZED;
    }
    else
    {
        // No counters:
        _samplingInfo._samplingState = csATISamplingInfo::CS_ATI_NO_COUNTERS_TO_SAMPLE;
    }

    // Turn off the flag that counters should be activated:
    _samplingInfo._shouldActivateCounters = false;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::beginSamplingSession
// Description: Opens a sampling session with the currently enabled counters
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
bool csAMDQueuePerformanceCountersReader::beginSamplingSession()
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(_samplingInfo._samplingState == csATISamplingInfo::CS_ATI_SESSION_NOT_INITIALIZED)
    {
        // Check how many passes are required with the currently enabled counters set:
        GPA_Status gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_GetPassCount(&_samplingInfo._amountOfRequiredPasses);
        GT_IF_WITH_ASSERT(gpaStatus == GPA_STATUS_OK)
        {
            // Open a sampling session:
            _samplingInfo._currentWaitSessionID = 1;
            gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_BeginSession(&_samplingInfo._currentWaitSessionID);
            GT_IF_WITH_ASSERT(gpaStatus == GPA_STATUS_OK)
            {
                _samplingInfo._samplingState = csATISamplingInfo::CS_ATI_IN_SESSION;
                _samplingInfo._amountOfPassesPerformedForThisSession = 0;
                retVal = beginOnePass();
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::endSamplingSession
// Description: Closes currently open session
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
bool csAMDQueuePerformanceCountersReader::endSamplingSession()
{
    bool retVal = false;

    // End the session:
    GPA_Status gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_EndSession();

    if (gpaStatus == GPA_STATUS_OK)
    {
        retVal = true;
    }

    // Reset sampling session arguments:
    _samplingInfo._amountOfPassesPerformedForThisSession = 0;

    // Update the session sampling state:
    _samplingInfo._samplingState = csATISamplingInfo::CS_ATI_WAITING_FOR_RESULTS;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::beginOnePass
// Description: Begins one sampling pass
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
bool csAMDQueuePerformanceCountersReader::beginOnePass()
{
    bool retVal = false;

    if (_samplingInfo._amountOfPassesPerformedForThisSession < _samplingInfo._amountOfRequiredPasses)
    {
        // If a pass had began, close it:
        if (_samplingInfo._amountOfPassesPerformedForThisSession > 0)
        {
            bool rc = endOnePass();
            GT_ASSERT(rc);
        }

        // Open another pass:
        GPA_Status gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_BeginPass();

        if (gpaStatus == GPA_STATUS_ERROR_SAMPLE_NOT_STARTED)
        {
            GT_ASSERT_EX(false, CS_STR_ATICountersSamplingNotStartFailure);
        }
        else if (gpaStatus == GPA_STATUS_OK)
        {
            retVal = true;
            gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_BeginSample(1);
            _samplingInfo._amountOfPassesPerformedForThisSession++;
            GT_ASSERT(gpaStatus == GPA_STATUS_OK);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::endOnePass
// Description: Ends one sampling pass
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
bool csAMDQueuePerformanceCountersReader::endOnePass()
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(_samplingInfo._amountOfPassesPerformedForThisSession > 0)
    {
        // Open another pass:
        GPA_Status gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_EndSample();
        GT_IF_WITH_ASSERT(gpaStatus == GPA_STATUS_OK)
        {
            gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_EndPass();
            GT_IF_WITH_ASSERT(gpaStatus == GPA_STATUS_OK)
            {
                retVal = true;
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csAMDQueuePerformanceCountersReader::setCounterValuesPointer
// Description: Sets the counter values pointer
// Arguments: double* pCounterValues
// Return Val: void
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
void csAMDQueuePerformanceCountersReader::setCounterValuesPointer(double* pCounterValues)
{
    // Set the counters new value pointer:
    _pCountersValues = pCounterValues;
}


#endif // OA_DEBUGGER_USE_AMD_GPA

