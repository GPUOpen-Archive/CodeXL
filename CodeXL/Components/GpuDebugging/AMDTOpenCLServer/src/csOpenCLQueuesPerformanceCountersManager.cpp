//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLQueuesPerformanceCountersManager.cpp
///
//==================================================================================

//------------------------------ csOpenCLQueuesPerformanceCountersManager.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apOpenCLQueuePerformanceCounters.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>

// Local:
#include <src/csOpenCLQueuesPerformanceCountersManager.h>
#include <src/csCommandQueueMonitor.h>
#include <src/csOpenCLQueuePerformanceCountersReader.h>

// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuesPerformanceCountersManager::csOpenCLQueuesPerformanceCountersManager
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
csOpenCLQueuesPerformanceCountersManager::csOpenCLQueuesPerformanceCountersManager():
    _countersAmount(0), _pCountersValuesSnapshot(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuesPerformanceCountersManager::~csOpenCLQueuesPerformanceCountersManager
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
csOpenCLQueuesPerformanceCountersManager::~csOpenCLQueuesPerformanceCountersManager()
{
    // Delete counter values snapshot vector:
    if (_pCountersValuesSnapshot != NULL)
    {
        delete[] _pCountersValuesSnapshot;
        _pCountersValuesSnapshot = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuesPerformanceCountersManager::updateCounterValues
// Description: Updates the counters values snapshot.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
bool csOpenCLQueuesPerformanceCountersManager::updateCounterValues()
{
    bool retVal = true;

    // Iterate the existing contexts counters reader:
    gtMap<apCounterScope, csOpenCLQueuePerformanceCountersReader*>::const_iterator iter = _existingQueuesPerfReaders.begin();
    gtMap<apCounterScope, csOpenCLQueuePerformanceCountersReader*>::const_iterator iterEnd = _existingQueuesPerfReaders.end();

    for (iter = _existingQueuesPerfReaders.begin(); iter != iterEnd; iter ++)
    {
        // If the current context exists:
        csOpenCLQueuePerformanceCountersReader* pCurrReader = (*iter).second;

        if (pCurrReader != NULL)
        {
            bool rc = pCurrReader->updateCounterValues();
            retVal = retVal && rc;
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuesPerformanceCountersManager::onQueueCreatedEvent
// Description: Is called when a queue is created
// Arguments:   int contextSpyId
//            int queueId
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
void csOpenCLQueuesPerformanceCountersManager::onQueueCreatedEvent(csCommandQueueMonitor* pCommandQueueMonitor)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pCommandQueueMonitor != NULL)
    {
        // Get the OpenCL Queue counters reader:
        csOpenCLQueuePerformanceCountersReader& openCLQueueCountersReader = pCommandQueueMonitor->openCLQueueCountersReader();

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
        _existingQueuesPerfReaders[newCounterScope] = &openCLQueueCountersReader;

        // Calculate the new amount of the OpenCL queue performance counters:
        int oldCountersAmount = _countersAmount;
        int newAmountOfQueues = (int)_existingQueuesPerfReaders.size();
        int newCountersAmount = newAmountOfQueues * AP_COMMAND_QUEUE_COUNTERS_AMOUNT;

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
            if ((j % AP_COMMAND_QUEUE_COUNTERS_AMOUNT) != AP_QUEUE_IDLE_COUNTER)
            {
                pNewCountersValuesSnapshot[j] = 0.0;
            }
            else // (j % AP_COMMAND_QUEUE_COUNTERS_AMOUNT) == AP_QUEUE_IDLE_COUNTER
            {
                // Initialize idle counters to 100%:
                pNewCountersValuesSnapshot[j] = 100.0;
            }
        }

        // Replace and remove the old space:
        _pCountersValuesSnapshot = pNewCountersValuesSnapshot;

        // Iterate the counter readers and update the counters values pointer:
        gtMap<apCounterScope, csOpenCLQueuePerformanceCountersReader*>::const_iterator iter = _existingQueuesPerfReaders.begin();
        gtMap<apCounterScope, csOpenCLQueuePerformanceCountersReader*>::const_iterator iterEnd = _existingQueuesPerfReaders.end();
        int i = 0;

        // Notify all readers that the pointer has changed:
        for (iter = _existingQueuesPerfReaders.begin(); iter != iterEnd; iter ++, i++)
        {
            // If context i exists:
            csOpenCLQueuePerformanceCountersReader* pCurrReader = (*iter).second;

            if (pCurrReader != NULL)
            {
                // Search for the reader's queue in the queue vector, to get the queue global index:
                apCounterScope currentReaderQueue = (*iter).first;

                // Find this queue within the queue vector:
                int queueIndex = -1;

                for (int ii = 0; ii < (int)_existingQueues.size(); ii++)
                {
                    if (_existingQueues[ii] == currentReaderQueue)
                    {
                        queueIndex = ii;
                        break;
                    }
                }

                GT_IF_WITH_ASSERT(queueIndex >= 0)
                {
                    // Calculate the first value for the current render context in the _pCountersValuesSnapshot array:
                    double* pContextCountersValuesVec = _pCountersValuesSnapshot + (queueIndex * AP_COMMAND_QUEUE_COUNTERS_AMOUNT);

                    // Set the counters vector. Update is done anyway, on each frame terminator:
                    pCurrReader->setCounterValuesPointer(pContextCountersValuesVec);
                }
            }
        }

        delete[] pOldCountersValuesSnapshot;
        pOldCountersValuesSnapshot = NULL;

        // Update the counters amount:
        _countersAmount = newCountersAmount;

    }
}

