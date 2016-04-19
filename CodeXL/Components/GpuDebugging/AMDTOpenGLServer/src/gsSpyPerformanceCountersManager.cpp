//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsSpyPerformanceCountersManager.cpp
///
//==================================================================================

//------------------------------ gsSpyPerformanceCountersManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apCounterInfo.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <src/gsOpenGLMonitor.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsSpyPerformanceCountersManager.h>


// ---------------------------------------------------------------------------
// Name:        gsSpyPerformanceCountersManager::gsSpyPerformanceCountersManager
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        26/7/2005
// ---------------------------------------------------------------------------
gsSpyPerformanceCountersManager::gsSpyPerformanceCountersManager():
    _countersAmount(0), _pCountersValuesSnapshot(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        gsSpyPerformanceCountersManager::~gsSpyPerformanceCountersManager
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        26/7/2005
// ---------------------------------------------------------------------------
gsSpyPerformanceCountersManager::~gsSpyPerformanceCountersManager()
{
    // Delete counter values snapshot vector:
    if (_pCountersValuesSnapshot != NULL)
    {
        delete[] _pCountersValuesSnapshot;
        _pCountersValuesSnapshot = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        gsSpyPerformanceCountersManager::updateCounterValues
// Description: Updates the counters values snapshot.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        27/7/2005
// ---------------------------------------------------------------------------
bool gsSpyPerformanceCountersManager::updateCounterValues()
{
    bool retVal = true;
    int contextAmount = (int)_existingContextsPerfMgrs.size();

    // Iterate the supported render contexts:
    // (Notice that context 0 - the NULL context is skipped)
    for (int i = 0; i < contextAmount; i++)
    {
        // If context i exists:
        gsRenderContextPerformanceCountersManager* pCurrContextPerCountersMgr = _existingContextsPerfMgrs[i];

        if (pCurrContextPerCountersMgr)
        {
            // Calculate the first value for the current render context in the _pCountersValuesSnapshot array:
            double* pContextCountersValuesVec = _pCountersValuesSnapshot + (i * GS_RENDER_CONTEXTS_COUNTERS_AMOUNT);

            // Update context i counter values:
            bool rc = pCurrContextPerCountersMgr->updateCounterValues(pContextCountersValuesVec);

            retVal = retVal && rc;
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsSpyPerformanceCountersManager::onContextCreatedEvent
// Description: Is called when a context is created.
// Arguments:   contextSpyId - The spy id for the created context.
// Author:      Yaki Tebeka
// Date:        27/7/2005
// ---------------------------------------------------------------------------
void gsSpyPerformanceCountersManager::onContextCreatedEvent(int contextSpyId)
{
    // If this is a read context (not context 0):
    if (contextSpyId != AP_NULL_CONTEXT_ID)
    {
        // Get the created render context monitor:
        gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();
        gsRenderContextMonitor* pCreatedRenderContextMonitor = theOpenGLMonitor.renderContextMonitor(contextSpyId);
        GT_IF_WITH_ASSERT(pCreatedRenderContextMonitor != NULL)
        {
            // Add the render context performance counters manager to the list of managers:
            gsRenderContextPerformanceCountersManager& contextCountersMgr = pCreatedRenderContextMonitor->performanceCountersManager();
            _existingContextsPerfMgrs.push_back(&contextCountersMgr);

            // Calculate the new amount of spy performance counters:
            int oldCountersAmount = _countersAmount;
            int newContextsAmount = contextSpyId;
            int newCountersAmount = newContextsAmount * GS_RENDER_CONTEXTS_COUNTERS_AMOUNT;

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
            delete[] pOldCountersValuesSnapshot;
            pOldCountersValuesSnapshot = NULL;

            // Update the counters amount:
            _countersAmount = newCountersAmount;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsSpyPerformanceCountersManager::onContextDeletedEvent
// Description: Is called when a context is deleted.
// Arguments:   contextSpyId - The spy id for the deleted context.
// Author:      Yaki Tebeka
// Date:        27/7/2005
// ---------------------------------------------------------------------------
void gsSpyPerformanceCountersManager::onContextDeletedEvent(int contextSpyId)
{
    // If this is a supported context:
    if (contextSpyId > 0)
    {
        // Get the context's index in the vector:
        int vectorIndex = performanceCountersManagerByContextId(contextSpyId);

        // If we got a valid index:
        if ((0 < vectorIndex) && (vectorIndex < (int)_existingContextsPerfMgrs.size()))
        {
            // Get its counters manager:
            gsRenderContextPerformanceCountersManager* pPerfCountersMgr = _existingContextsPerfMgrs[vectorIndex];
            GT_IF_WITH_ASSERT(pPerfCountersMgr != NULL)
            {
                // Remove it from the existing counters managers vector:
                _existingContextsPerfMgrs[vectorIndex] = NULL;
            }

            // Calculate the render context offset (in the values pointer):
            int renderContextValuesOffset = vectorIndex * GS_RENDER_CONTEXTS_COUNTERS_AMOUNT;

            // Put 0 in the context values:
            for (int i = renderContextValuesOffset; i < renderContextValuesOffset + GS_RENDER_CONTEXTS_COUNTERS_AMOUNT; i++)
            {
                _pCountersValuesSnapshot[i] = 0.0;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsSpyPerformanceCountersManager::performanceCountersManagerByContextId
// Description: Iterates the counters managers and returns the index for the one
//              that manages context #contextId's counters
// Author:      Uri Shomroni
// Date:        31/1/2010
// ---------------------------------------------------------------------------
int gsSpyPerformanceCountersManager::performanceCountersManagerByContextId(int contextId) const
{
    int retVal = -1;

    // Iterate the managers:
    int numberOfManagers = (int)_existingContextsPerfMgrs.size();

    for (int i = 0; i < numberOfManagers; i++)
    {
        // Get the current manager (do not assert, as the context may have already been deleted):
        const gsRenderContextPerformanceCountersManager* pCurrentManager = _existingContextsPerfMgrs[i];

        if (pCurrentManager != NULL)
        {
            // If this is the right manager:
            int currentManagerContextId = pCurrentManager->renderContextId();

            if (currentManagerContextId == contextId)
            {
                // Return its index and stop looking:
                retVal = i;
                break;
            }
        }
    }

    return retVal;
}

