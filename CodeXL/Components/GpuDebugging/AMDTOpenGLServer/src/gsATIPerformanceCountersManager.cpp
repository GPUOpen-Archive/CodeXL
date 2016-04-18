//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsATIPerformanceCountersManager.cpp
///
//==================================================================================

//------------------------------ gsATIPerformanceCountersManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <src/gsOpenGLMonitor.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsATIPerformanceCountersManager.h>
#include <src/gsATIRenderContextPerformanceCountersReader.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsGlobalVariables.h>

#ifdef OA_DEBUGGER_USE_AMD_GPA

// ---------------------------------------------------------------------------
// Name:        gsATIPerformanceCountersManager::gsATIPerformanceCountersManager
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        12/3/2006
// ---------------------------------------------------------------------------
gsATIPerformanceCountersManager::gsATIPerformanceCountersManager(bool initialize)
    : _isInitialized(false), _isATILibraryInitialized(false), _ATIperContextCountersAmount(0), _totalCountersAmount(0), _pCountersValuesSnapshot(NULL)
{
    if (initialize)
    {
        // Initialize the ATI library:
        GPA_Status gpaStatus = oaATIFunctionWrapper::gl_instance().GPA_Initialize();
        GT_IF_WITH_ASSERT(gpaStatus == GPA_STATUS_OK)
        {
            _isATILibraryInitialized = true;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsATIPerformanceCountersManager::~gsATIPerformanceCountersManager
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        12/3/2006
// ---------------------------------------------------------------------------
gsATIPerformanceCountersManager::~gsATIPerformanceCountersManager()
{
    // Delete counter values snapshot vector
    if (_pCountersValuesSnapshot != NULL)
    {
        delete[] _pCountersValuesSnapshot;
        _pCountersValuesSnapshot = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        gsATIPerformanceCountersManager::onContextCreatedEvent
// Description: Event handler for context creation
// Arguments: int contextSpyId
// Return Val: void
// Author:      Sigal Algranaty
// Date:        26/3/2008
// ---------------------------------------------------------------------------
void gsATIPerformanceCountersManager::onContextCreatedEvent(int contextSpyId)
{
    // If this is a supported context:
    GT_IF_WITH_ASSERT(contextSpyId != AP_NULL_CONTEXT_ID)
    {
        // Get the created render context monitor:
        gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();
        gsRenderContextMonitor* pCreatedRenderContextMonitor = theOpenGLMonitor.renderContextMonitor(contextSpyId);
        GT_IF_WITH_ASSERT(pCreatedRenderContextMonitor != NULL)
        {
            // Add its counters manager to our counters managers list:
            gsATIRenderContextPerformanceCountersReader& contextCountersMgr = pCreatedRenderContextMonitor->atiPerformanceCountersManager();
            _existingContextsPerfMgrs.push_back(&contextCountersMgr);

            // Set the render context reader manager (me):
            contextCountersMgr.setATIPerformanceCounterManager(this);

            // If the counter values snapshot is already initialized, reallocate the counter values with the new amount of values.
            // If we get here before the first call to 'onFirstTimeContextMadeCurrent' of one of the readers, the total counters amount would be 0
            // since we do not know yet what is the amount of counters per context:
            if (_totalCountersAmount > 0)
            {
                // Calculate the new amount of spy performance counters:
                int oldCountersAmount = _totalCountersAmount;
                int newContextsAmount = contextSpyId;
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

                // Notify all render context readers that the pointer has changed:
                for (int i = 0; i < (int)_existingContextsPerfMgrs.size(); i++)
                {
                    // If context i exists:
                    gsATIRenderContextPerformanceCountersReader* pCurrContextPerCountersMgr = _existingContextsPerfMgrs[i];

                    if (pCurrContextPerCountersMgr != NULL)
                    {
                        // Calculate the first value for the current render context in the _pCountersValuesSnapshot array:
                        double* pContextCountersValuesVec = _pCountersValuesSnapshot + (i * _ATIperContextCountersAmount);
                        // Set the counters vector. Update is done anyway, on each frame terminator:

                        pCurrContextPerCountersMgr->setCounterValuesPointer(pContextCountersValuesVec);
                    }
                }

                delete[] pOldCountersValuesSnapshot;
                pOldCountersValuesSnapshot = NULL;

                // Update the counters amount:
                _totalCountersAmount = newCountersAmount;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsATIPerformanceCountersManager::onContextDeletedEvent
// Description: Is called when a render context is deleted.
// Arguments: contextSpyId - The deleted context spy id.
// Author:      Yaki Tebeka
// Date:        15/3/2006
// ---------------------------------------------------------------------------
void gsATIPerformanceCountersManager::onContextDeletedEvent(int contextSpyId)
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
            gsATIRenderContextPerformanceCountersReader* pPerfCountersMgr = _existingContextsPerfMgrs[vectorIndex];
            GT_IF_WITH_ASSERT(pPerfCountersMgr != NULL)
            {
                // Remove it from the existing counters managers vector:
                _existingContextsPerfMgrs[vectorIndex] = NULL;
            }

            // Calculate the render context offset (in the values pointer):
            int renderContextValuesOffset = vectorIndex * _ATIperContextCountersAmount;

            // Put 0 in the context counters values:
            for (int i = renderContextValuesOffset; i < renderContextValuesOffset + _ATIperContextCountersAmount; i++)
            {
                _pCountersValuesSnapshot[i] = 0;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsATIPerformanceCountersManager::getCounterForActivation
// Description: Get all counter waiting for activation for the reuqested context id
// Arguments: int renderContextId - the render context id
//            gtVector<apCounterActivationInfo>& counterActivationInfo - vector containing the
//            counter waiting for activation for this render context
// Author:      Sigal Algranaty
// Date:        14/2/2010
// ---------------------------------------------------------------------------
void gsATIPerformanceCountersManager::getCounterForActivation(int renderContextId, gtVector<apCounterActivationInfo>& counterActivationInfo)
{
    gtVector<int> indicesToRemove;

    for (int j = 0; j < (int)_countersActivationVector.size(); j++)
    {
        // Get the current counter activation info:
        apCounterActivationInfo activationInfo = _countersActivationVector[j];
        int counterContextId = activationInfo._counterId._counterScope._contextID._contextId;

        if (counterContextId == renderContextId)
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
// Name:        gsATIPerformanceCountersManager::initialize
// Description: Initialize the ATI performance counters usage.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        13/3/2006
// ---------------------------------------------------------------------------
void gsATIPerformanceCountersManager::initialize(int numberOfATICountersPerContext)
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
                unsigned int numOfContexts = (unsigned int)_existingContextsPerfMgrs.size();

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

                // Notify all render context readers that the pointer has changed:
                for (int i = 0; i < (int)_existingContextsPerfMgrs.size(); i++)
                {
                    // If context i exists:
                    gsATIRenderContextPerformanceCountersReader* pCurrContextPerCountersMgr = _existingContextsPerfMgrs[i];

                    if (pCurrContextPerCountersMgr != NULL)
                    {
                        // Calculate the first value for the current render context in the _pCountersValuesSnapshot array:
                        double* pContextCountersValuesVec = _pCountersValuesSnapshot + (i * _ATIperContextCountersAmount);

                        // Set the counters vector. Update is done anyway, on each frame terminator:
                        pCurrContextPerCountersMgr->setCounterValuesPointer(pContextCountersValuesVec);
                    }
                }

                _isInitialized = true;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsATIPerformanceCountersManager::activateCounters
// Description: Activates a counter
// Arguments: apCounterID counterID
//            bool isCounterActive
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/1/2010
// ---------------------------------------------------------------------------
bool gsATIPerformanceCountersManager::activateCounters(const gtVector<apCounterActivationInfo>& counterActivationInfosVec)
{
    bool retVal = true;

    GT_IF_WITH_ASSERT(_isATILibraryInitialized)
    {
        if (_isInitialized)
        {
            // Go through each of the render context, and activate its counters:
            for (int i = 0 ; i < (int)_existingContextsPerfMgrs.size(); i++)
            {
                // If context i exists:
                gsATIRenderContextPerformanceCountersReader* pCurrContextPerCountersReader = _existingContextsPerfMgrs[i];

                if (pCurrContextPerCountersReader != NULL)
                {
                    // Get this reader's context:
                    int renderContext = pCurrContextPerCountersReader->spyId();

                    gtVector<apCounterActivationInfo> currentRenderContextActivationInfosVec;

                    for (int j = 0 ; j < (int)counterActivationInfosVec.size(); j++)
                    {
                        // Get the current counter activation info:
                        apCounterActivationInfo activationInfo = counterActivationInfosVec[j];
                        int counterContextIndex = activationInfo._counterId._counterScope._contextID._contextId;

                        if (counterContextIndex == renderContext)
                        {
                            // If this counter belongs to the current context, add it to the current list:
                            currentRenderContextActivationInfosVec.push_back(activationInfo);
                        }
                    }

                    bool rc = pCurrContextPerCountersReader->registerCountersForActivation(currentRenderContextActivationInfosVec);
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
// Name:        gsATIPerformanceCountersManager::terminate
// Description: Terminate the connection with the ATI library
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/5/2010
// ---------------------------------------------------------------------------
bool gsATIPerformanceCountersManager::terminate()
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


// ---------------------------------------------------------------------------
// Name:        gsSpyPerformanceCountersManager::performanceCountersManagerByContextId
// Description: Iterates the counters managers and returns the index for the one
//              that manages context #contextId's counters
// Author:      Sigal Algranaty
// Date:        27/7/2010
// ---------------------------------------------------------------------------
int gsATIPerformanceCountersManager::performanceCountersManagerByContextId(int contextId) const
{
    int retVal = -1;

    // Iterate the managers:
    int numberOfManagers = (int)_existingContextsPerfMgrs.size();

    for (int i = 0; i < numberOfManagers; i++)
    {
        // Get the current manager (do not assert, as the context may have already been deleted):
        const gsATIRenderContextPerformanceCountersReader* pCurrentReader = _existingContextsPerfMgrs[i];

        if (pCurrentReader != NULL)
        {
            // If this is the right manager:
            int currentManagerContextId = pCurrentReader->spyId();

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

#endif // OA_DEBUGGER_USE_AMD_GPA

