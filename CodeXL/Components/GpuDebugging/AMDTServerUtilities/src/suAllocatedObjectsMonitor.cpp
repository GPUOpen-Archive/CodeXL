//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suAllocatedObjectsMonitor.cpp
///
//==================================================================================

//------------------------------ suAllocatedObjectsMonitor.cpp ------------------------------

// Standard C:
#include <limits.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCallsStackReader.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <AMDTServerUtilities/Include/suAllocatedObjectsMonitor.h>


// Static members initializations:
suAllocatedObjectsMonitor* suAllocatedObjectsMonitor::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::instance
// Description: Returns the single instance of the suBreakpointsManager class
// Author:      Sigal Algranaty
// Date:        7/12/2009
// ---------------------------------------------------------------------------
suAllocatedObjectsMonitor& suAllocatedObjectsMonitor::instance()
{
    // If this class single instance was not already created:
    if (_pMySingleInstance == NULL)
    {
        // Create it:
        _pMySingleInstance = new suAllocatedObjectsMonitor;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        suAllocatedObjectsMonitor::suAllocatedObjectsMonitor
// Description: Constructor
// Author:      Uri Shomroni
// Date:        20/10/2008
// ---------------------------------------------------------------------------
suAllocatedObjectsMonitor::suAllocatedObjectsMonitor():
    _collectingAllocatedObjectsCreationCallsStacks(true)
{

}

// ---------------------------------------------------------------------------
// Name:        suAllocatedObjectsMonitor::~suAllocatedObjectsMonitor
// Description: Destructor
// Author:      Uri Shomroni
// Date:        20/10/2008
// ---------------------------------------------------------------------------
suAllocatedObjectsMonitor::~suAllocatedObjectsMonitor()
{
    clearObjects();
}

// ---------------------------------------------------------------------------
// Name:        suAllocatedObjectsMonitor::registerAllocatedObject
// Description: Registers an allocated object in the manager
// Author:      Uri Shomroni
// Date:        20/10/2008
// ---------------------------------------------------------------------------
bool suAllocatedObjectsMonitor::registerAllocatedObject(apAllocatedObject& allocObj)
{
    bool retVal = false;

    // Lock the access to the critical section:
    osCriticalSectionLocker csLocker(_allocatedObjectsCreationCallStacksCS);

    unsigned int objectLocation = numberOfAllocatedObjects();
    GT_IF_WITH_ASSERT(objectLocation < INT_MAX)
    {
        // if the object has not yet been registered by us:
        retVal = allocObj.setAllocatedObjectId(objectLocation, false);

        GT_IF_WITH_ASSERT(retVal)
        {
            osCallStack* pObjectCreationCallStack = NULL;
            osCallsStackReader callStackReader;

            // We don't collect the calls stacks if the user chose so, or we are in profile mode:
            apExecutionMode currentExecMode = suDebuggedProcessExecutionMode();

            if ((currentExecMode != AP_PROFILING_MODE) && _collectingAllocatedObjectsCreationCallsStacks)
            {
                pObjectCreationCallStack = new osCallStack;
                retVal = callStackReader.getCurrentCallsStack(*pObjectCreationCallStack);
            }
            else
            {
                // If we are in profile mode, register NULL, so we'll know this object doesn't have one:
                retVal = true;
            }

            _allocatedObjectsCreationCallStacks.push_back(pObjectCreationCallStack);
        }
    }

    // Unlock the access to the critical section:
    csLocker.leaveCriticalSection();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suAllocatedObjectsMonitor::registerAllocatedObjects
// Description: Registers a set of allocated objects in the manager
// Author:      Uri Shomroni
// Date:        4/11/2014
// ---------------------------------------------------------------------------
bool suAllocatedObjectsMonitor::registerAllocatedObjects(gtVector<apAllocatedObject*>& allocObjs)
{
    bool retVal = false;

    // Lock the access to the critical section:
    osCriticalSectionLocker csLocker(_allocatedObjectsCreationCallStacksCS);

    unsigned int objectLocation = numberOfAllocatedObjects();
    GT_IF_WITH_ASSERT(objectLocation < INT_MAX)
    {
        // if the object has not yet been registered by us:
        retVal = false;
        int numberOfObjectsToRegister = (int)allocObjs.size();

        for (int i = 0; i < numberOfObjectsToRegister; i++)
        {
            // Make sure at least one non-NULL item is being registered:
            bool rc = false;
            apAllocatedObject* pCurrentObj = allocObjs[i];
            GT_IF_WITH_ASSERT(NULL != pCurrentObj)
            {
                rc = pCurrentObj->setAllocatedObjectId(objectLocation, false);
            }
            retVal = retVal || rc;
        }

        GT_IF_WITH_ASSERT(retVal)
        {
            osCallStack* pObjectCreationCallStack = NULL;
            osCallsStackReader callStackReader;

            // We don't collect the calls stacks if the user chose so, or we are in profile mode:
            apExecutionMode currentExecMode = suDebuggedProcessExecutionMode();

            if ((currentExecMode != AP_PROFILING_MODE) && _collectingAllocatedObjectsCreationCallsStacks)
            {
                pObjectCreationCallStack = new osCallStack;

                retVal = callStackReader.getCurrentCallsStack(*pObjectCreationCallStack);
            }
            else
            {
                // If we are in profile mode, register NULL, so we'll know this object doesn't have one:
                retVal = true;
            }

            _allocatedObjectsCreationCallStacks.push_back(pObjectCreationCallStack);

            if (!retVal)
            {
                delete pObjectCreationCallStack;
            }
        }
    }

    // Unlock the access to the critical section:
    csLocker.leaveCriticalSection();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suAllocatedObjectsMonitor::getAllocatedObjectCreationCallStack
// Description: inserts into callStack the creation stack of object number index
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        20/10/2008
// ---------------------------------------------------------------------------
bool suAllocatedObjectsMonitor::getAllocatedObjectCreationCallStack(int index, const osCallStack*& o_pCallsStack) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((0 <= index) && ((int)_allocatedObjectsCreationCallStacks.size() > index))
    {
        o_pCallsStack = _allocatedObjectsCreationCallStacks[index];
        if (nullptr != o_pCallsStack)
        {
            retVal = true;
        }
    }
    else
    {
        o_pCallsStack = nullptr;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suAllocatedObjectsMonitor::collectAllocatedObjectsCreationCallsStacks
// Description: Sets the "Collect allocated objects creation calls stacks" mode
// Author:      Uri Shomroni
// Date:        3/2/2009
// ---------------------------------------------------------------------------
void suAllocatedObjectsMonitor::collectAllocatedObjectsCreationCallsStacks(bool collectCreationStacks)
{
    _collectingAllocatedObjectsCreationCallsStacks = collectCreationStacks;
}

// ---------------------------------------------------------------------------
// Name:        suAllocatedObjectsMonitor::clearObjects
// Description: Clears all the registered objects from the manager.
// Author:      Uri Shomroni
// Date:        20/10/2008
// ---------------------------------------------------------------------------
void suAllocatedObjectsMonitor::clearObjects()
{
    int n = (int)_allocatedObjectsCreationCallStacks.size();

    for (int i = 0; i < n; i++)
    {
        delete _allocatedObjectsCreationCallStacks[i];
        _allocatedObjectsCreationCallStacks[i] = NULL;
    }

    _allocatedObjectsCreationCallStacks.clear();
}

