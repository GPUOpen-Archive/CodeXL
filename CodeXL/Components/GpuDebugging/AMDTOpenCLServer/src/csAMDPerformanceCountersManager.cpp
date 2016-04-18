//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csAMDPerformanceCountersManager.cpp
///
//==================================================================================

//------------------------------ csAMDPerformanceCountersManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <src/csOpenCLMonitor.h>

// Local:
#include <src/csAMDPerformanceCountersManager.h>
#include <src/csAMDQueuePerformanceCountersReader.h>

#ifdef OA_DEBUGGER_USE_AMD_GPA

// ---------------------------------------------------------------------------
// Name:        csAMDPerformanceCountersManager::csAMDPerformanceCountersManager
// Description: Constructor.
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
csAMDPerformanceCountersManager::csAMDPerformanceCountersManager(bool initialize)
    : _isInitialized(false), _isATILibraryInitialized(false), _ATIperContextCountersAmount(0), _totalCountersAmount(0), _pCountersValuesSnapshot(NULL)
{
    if (initialize)
    {
        // Initialize the ATI library:
        GPA_Status gpaStatus = oaATIFunctionWrapper::cl_instance().GPA_Initialize();
        GT_IF_WITH_ASSERT(gpaStatus == GPA_STATUS_OK)
        {
            _isATILibraryInitialized = true;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDPerformanceCountersManager::~csAMDPerformanceCountersManager
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
csAMDPerformanceCountersManager::~csAMDPerformanceCountersManager()
{
    // Delete counter values snapshot vector
    if (_pCountersValuesSnapshot != NULL)
    {
        delete[] _pCountersValuesSnapshot;
        _pCountersValuesSnapshot = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDPerformanceCountersManager::onQueueCreatedEvent
// Description: Event handler for queue creation
// Arguments:   csCommandQueueMonitor* pCommandQueueMonitor - the queues monitor
// Return Val: void
// Author:      Sigal Algranaty
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void csAMDPerformanceCountersManager::onQueueCreatedEvent(csCommandQueueMonitor* pCommandQueueMonitor)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pCommandQueueMonitor != NULL)
    {
        // Get the AMD counters reader:
        csAMDQueuePerformanceCountersReader& AMDQueueCountersReader = pCommandQueueMonitor->AMDPerformanceCountersReader();

        // Get the queue context & queue id:
        int contextId = pCommandQueueMonitor->contextSpyId();
        int queueId = pCommandQueueMonitor->commandQueueIndex();
        apCounterScope newCounterScope(contextId, queueId);

        // Add the new queue to the existing queues:
        _existingQueues.push_back(newCounterScope);

        // Sort the existing queues vector (this vector should be kept sorted, in order to
        // map the actual global index for each queue):
        gtSort(_existingQueues.begin(), _existingQueues.end());

        // Insert this counter reader to the map:
        _existingQueuesPerfReaders[newCounterScope] = &AMDQueueCountersReader;

        // If the counter values snapshot is already initialized, reallocate the counter values with the new amount of values.
        // If we get here before the first call to 'onFirstTimeContextMadeCurrent' of one of the readers, the total counters amount would be 0
        // since we do not know yet what is the amount of counters per context:
        if (_totalCountersAmount > 0)
        {
            // Calculate the new amount of spy performance counters:
            int oldCountersAmount = _totalCountersAmount;
            int newContextsAmount = pCommandQueueMonitor->contextSpyId();
            int newCountersAmount = newContextsAmount * _ATIperContextCountersAmount;

            // Allocate new (larger) space to contain counter values:
            double* pOldCountersValuesSnapshot = _pCountersValuesSnapshot;
            double* pNewCountersValuesSnapshot = new double[newCountersAmount];


            // Copy the old performance counters values to the new space:
            for (int i = 0; i < oldCountersAmount; i++)
            {
                pNewCountersValuesSnapshot[i] = pOldCountersValuesSnapshot[i];
            }

            // Initialize new counter values to contain 0:
            for (int j = oldCountersAmount; j < newCountersAmount; j++)
            {
                pNewCountersValuesSnapshot[j] = 0;
            }

            // Replace and remove the old space:
            _pCountersValuesSnapshot = pNewCountersValuesSnapshot;

            // Iterate the counter readers and update the counters values pointer:
            gtMap<apCounterScope, csAMDQueuePerformanceCountersReader*>::const_iterator iter = _existingQueuesPerfReaders.begin();
            gtMap<apCounterScope, csAMDQueuePerformanceCountersReader*>::const_iterator iterEnd = _existingQueuesPerfReaders.end();
            int i = 0;

            // Notify all readers that the pointer has changed:
            for (iter = _existingQueuesPerfReaders.begin(); iter != iterEnd; iter ++, i++)
            {
                // If context i exists:
                csAMDQueuePerformanceCountersReader* pCurrReader = (*iter).second;

                if (pCurrReader != NULL)
                {
                    // Search for the reader's queue in the queue vector, to get the queue global index:
                    int queueGlobalIndex = -1;
                    apCounterScope currentReaderQueue = (*iter).first;

                    // Find this queue within the queue vector:
                    int queueIndex = -1;

                    for (int i = 0; i < (int)_existingQueues.size(); i++)
                    {
                        if (_existingQueues[i] == currentReaderQueue)
                        {
                            queueIndex = i;
                            break;
                        }
                    }

                    GT_IF_WITH_ASSERT(queueIndex >= 0)
                    {
                        // Calculate the first value for the current render context in the _pCountersValuesSnapshot array:
                        double* pContextCountersValuesVec = _pCountersValuesSnapshot + (queueIndex * _ATIperContextCountersAmount);
                        // Set the counters vector. Update is done anyway, on each frame terminator:

                        pCurrReader->setCounterValuesPointer(pContextCountersValuesVec);
                    }
                }
            }

            delete[] pOldCountersValuesSnapshot;
            pOldCountersValuesSnapshot = NULL;

            // Update the counters amount:
            _totalCountersAmount = newCountersAmount;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDPerformanceCountersManager::onContextDeletedEvent
// Description: Is called when a render context is deleted.
// Arguments: contextSpyId - The deleted context spy id.
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Name:        csAMDPerformanceCountersManager::onQueueDeletedEvent
// Description: Is called when a queue is deleted
// Arguments: oaCLCommandQueueHandle queueHandle
// Return Val: void
// Author:      Sigal Algranaty
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void csAMDPerformanceCountersManager::onQueueDeletedEvent(oaCLCommandQueueHandle queueHandle)
{
    // If this is a supported context:
    GT_IF_WITH_ASSERT(queueHandle != NULL)
    {
        // Get the command queue monitor:
        csCommandQueueMonitor* pCommandQueueMtr = csOpenCLMonitor::instance().commandQueueMonitor((oaCLCommandQueueHandle)queueHandle);
        GT_IF_WITH_ASSERT(pCommandQueueMtr != NULL)
        {
            // Get the spy id:
            int contextId = pCommandQueueMtr->contextSpyId();
            int queueId = pCommandQueueMtr->commandQueueIndex();
            apCounterScope deletedCounterScope(contextId, queueId);

            // Get its counters manager:
            csAMDQueuePerformanceCountersReader* pPerfCountersMgr = _existingQueuesPerfReaders[deletedCounterScope];
            GT_IF_WITH_ASSERT(pPerfCountersMgr != NULL)
            {
                // Remove it from the existing counters managers vector:
                _existingQueuesPerfReaders[deletedCounterScope] = NULL;
            }

            // Calculate the render context offset (in the values pointer):
            int renderContextValuesOffset = contextId * _ATIperContextCountersAmount;

            // Put 0 in the context counters values:
            for (int i = renderContextValuesOffset; i < renderContextValuesOffset + _ATIperContextCountersAmount; i++)
            {
                _pCountersValuesSnapshot[i] = 0;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDPerformanceCountersManager::getCounterForActivation
// Description: Get all counter waiting for activation for the requested
//              context + queue id
// Arguments: int contextId
//            int queueCounterID
//            gtVector<apCounterActivationInfo>& counterActivationInfo
// Return Val: void
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
void csAMDPerformanceCountersManager::getCounterForActivation(int contextId, int queueCounterID, gtVector<apCounterActivationInfo>& counterActivationInfo)
{
    gtVector<int> indicesToRemove;

    for (int j = 0; j < (int)_countersActivationVector.size(); j++)
    {
        // Get the current counter activation info:
        apCounterActivationInfo activationInfo = _countersActivationVector[j];

        if (activationInfo._counterId._counterScope._contextID._contextId == contextId)
        {
            // If this counter belongs to the current context, add it to the current list:
            counterActivationInfo.push_back(activationInfo);

            // The counter is handled, remove it:
            indicesToRemove.push_back(j);
        }
    }

    // After adding the counters to the reader, remove the indices from the global vector:
    int numberofIndicesToRemove = (int)indicesToRemove.size();

    for (int i = (numberofIndicesToRemove - 1); i >= 0 ; i--)
    {
        _countersActivationVector.removeItem(indicesToRemove[i]);
    }

}

// ---------------------------------------------------------------------------
// Name:        csAMDPerformanceCountersManager::initialize
// Description: Initialize the ATI performance counters usage.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
void csAMDPerformanceCountersManager::initialize(int numberOfATICountersPerContext)
{
    GT_IF_WITH_ASSERT(_isATILibraryInitialized)

    {
        if (!_isInitialized)
        {
            // If ATI counters are supported:
            if (0 < numberOfATICountersPerContext)
            {
                // Get the amount of supported counters (per render context):
                _ATIperContextCountersAmount = numberOfATICountersPerContext;

                // get number of contexts
                unsigned int numOfContexts = (unsigned int)_existingQueuesPerfReaders.size();

                // Calculate the amount of counters (both per context and total):
                _totalCountersAmount = numOfContexts * _ATIperContextCountersAmount;

                // Sanity check (make sure we get here only once):
                GT_ASSERT(_pCountersValuesSnapshot == NULL);

                // Allocate space for the counters values:
                _pCountersValuesSnapshot = new double[_totalCountersAmount];


                GT_IF_WITH_ASSERT(_pCountersValuesSnapshot != NULL)
                {
                    // Initialize counter values to 0:
                    for (int i = 0; i < _totalCountersAmount; i++)
                    {
                        _pCountersValuesSnapshot[i] = 0;
                    }
                }

                // Iterate the counter readers and update the counters values pointer:
                gtMap<apCounterScope, csAMDQueuePerformanceCountersReader*>::const_iterator iter = _existingQueuesPerfReaders.begin();
                gtMap<apCounterScope, csAMDQueuePerformanceCountersReader*>::const_iterator iterEnd = _existingQueuesPerfReaders.end();
                int i = 0;

                // Notify all readers that the pointer has changed:
                for (iter = _existingQueuesPerfReaders.begin(); iter != iterEnd; iter ++, i++)
                {
                    // If context i exists:
                    csAMDQueuePerformanceCountersReader* pCurrReader = (*iter).second;

                    if (pCurrReader != NULL)
                    {
                        // Search for the reader's queue in the queue vector, to get the queue global index:
                        int queueGlobalIndex = -1;
                        apCounterScope currentReaderQueue = (*iter).first;

                        // Find this queue within the queue vector:
                        int queueIndex = -1;

                        for (int i = 0; i < (int)_existingQueues.size(); i++)
                        {
                            if (_existingQueues[i] == currentReaderQueue)
                            {
                                queueIndex = i;
                                break;
                            }
                        }

                        GT_IF_WITH_ASSERT(queueIndex >= 0)
                        {
                            // Calculate the first value for the current render context in the _pCountersValuesSnapshot array:
                            double* pContextCountersValuesVec = _pCountersValuesSnapshot + (queueIndex * _ATIperContextCountersAmount);

                            // Set the counters vector. Update is done anyway, on each frame terminator:
                            pCurrReader->setCounterValuesPointer(pContextCountersValuesVec);
                        }
                    }
                }

                _isInitialized = true;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csAMDPerformanceCountersManager::activateCounters
// Description: Activates a counter
// Arguments: apCounterID counterID
//            bool isCounterActive
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/2/2010
// ---------------------------------------------------------------------------
bool csAMDPerformanceCountersManager::activateCounters(const gtVector<apCounterActivationInfo>& counterActivationInfosVec)
{
    bool retVal = true;
    GT_IF_WITH_ASSERT(_isATILibraryInitialized)

    {
        if (_isInitialized)
        {
            // Iterate the counter readers:
            gtMap<apCounterScope, csAMDQueuePerformanceCountersReader*>::const_iterator iter = _existingQueuesPerfReaders.begin();
            gtMap<apCounterScope, csAMDQueuePerformanceCountersReader*>::const_iterator iterEnd = _existingQueuesPerfReaders.end();

            // Go through each of the command queues, and activate its counters:
            for (iter = _existingQueuesPerfReaders.begin() ; iter != iterEnd ; iter++)
            {
                // If context i exists:
                csAMDQueuePerformanceCountersReader* pCurrCommmandQueue = (*iter).second;

                if (pCurrCommmandQueue != NULL)
                {
                    gtVector<apCounterActivationInfo> currentRenderContextActivationInfosVec;

                    for (int j = 0 ; j < (int)counterActivationInfosVec.size(); j++)
                    {
                        // Get the current reader context id:
                        int contextId = pCurrCommmandQueue->contextSpyId();

                        // Get the current context command queue id:
                        int commandQueueId = pCurrCommmandQueue->commandQueueId();

                        // Get the current counter activation info:
                        apCounterActivationInfo activationInfo = counterActivationInfosVec[j];

                        if ((activationInfo._counterId._counterScope._contextID._contextId == contextId) &&
                            (activationInfo._counterId._counterScope._queueId == commandQueueId))
                        {
                            // If this counter belongs to the current context, add it to the current list:
                            currentRenderContextActivationInfosVec.push_back(activationInfo);
                        }
                    }

                    bool rc = pCurrCommmandQueue->registerCountersForActivation(currentRenderContextActivationInfosVec);
                    retVal = retVal && rc;
                }
            }
        }
        else
        {
            // Just push them back to the vector, and activate later:
            for (int j = 0 ; j < (int)counterActivationInfosVec.size(); j++)
            {
                // Get the current counter activation info:
                apCounterActivationInfo activationInfo = counterActivationInfosVec[j];
                _countersActivationVector.push_back(activationInfo);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csAMDPerformanceCountersManager::terminate
// Description: Terminate the connection with the ATI library
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/5/2010
// ---------------------------------------------------------------------------
bool csAMDPerformanceCountersManager::terminate()
{
    bool retVal = false;
    oaATIFunctionWrapper& functionWrapper = oaATIFunctionWrapper::gl_instance();
    GPA_Status gpaStatus = functionWrapper.GPA_Destroy();
    GT_IF_WITH_ASSERT(gpaStatus == GPA_STATUS_OK)
    {
        retVal = true;
    }
    return retVal;
}

#endif // OA_DEBUGGER_USE_AMD_GPA

