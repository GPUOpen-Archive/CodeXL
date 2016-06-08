//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCondition.cpp
///
//=====================================================================

// ---------------------------------- osCondition.cpp ----------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osCondition.h>


// ---------------------------------------------------------------------------
// Name:        osCondition::osCondition
// Description:
//  Constructor. Initializes the condition to "unlocked" state.
// Author:      AMD Developer Tools Team
// Date:        14/11/2007
// ---------------------------------------------------------------------------
osCondition::osCondition()
    : _isConditionLocked(false)
{
    //  Creates the condition object:
    int rc = ::pthread_cond_init(&_conditionObj, NULL);
    GT_IF_WITH_ASSERT(rc == 0)
    {
        // Create the condition mutex:
        int rc = ::pthread_mutex_init(&_conditionMutex, NULL);
        GT_ASSERT(rc == 0);
    }
}


// ---------------------------------------------------------------------------
// Name:        osCondition::osCondition
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        14/11/2007
// ---------------------------------------------------------------------------
osCondition::~osCondition()
{
    // Unlock the condition (otherwise trying to delete it returns EBUSY)
    unlockCondition();

    // Destroy the condition object:
    int rc1 = ::pthread_cond_destroy(&_conditionObj);
    GT_ASSERT(rc1 == 0);

    // Destroy the condition mutex:
    int rc2 = ::pthread_mutex_destroy(&_conditionMutex);
    GT_ASSERT(rc2 == 0);
}


// ---------------------------------------------------------------------------
// Name:        osCondition::lockCondition
// Description:
//  Locks the condition. From this point on, all threads that will
//  call  waitForCondition()  will be blocked.
//
// Author:      AMD Developer Tools Team
// Date:        14/11/2007
// ---------------------------------------------------------------------------
bool osCondition::lockCondition()
{
    bool retVal = false;

    // Lock the condition mutex:
    int rc1 = ::pthread_mutex_lock(&_conditionMutex);
    GT_IF_WITH_ASSERT(rc1 == 0)
    {
        // Mark the condition as locked:
        _isConditionLocked = true;

        // Unlock the condition mutex:
        int rc2 = ::pthread_mutex_unlock(&_conditionMutex);
        GT_IF_WITH_ASSERT(rc2 == 0)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCondition::unlockCondition
// Description:
//  Unlocks the condition. From this point on, all threads that will
//  call  waitForCondition()  will not be blocked.
//
// Author:      AMD Developer Tools Team
// Date:        14/11/2007
// ---------------------------------------------------------------------------
bool osCondition::unlockCondition()
{
    bool retVal = false;

    // Lock the condition mutex:
    int rc1 = ::pthread_mutex_lock(&_conditionMutex);
    GT_IF_WITH_ASSERT(rc1 == 0)
    {
        // Mark the condition as unlocked:
        _isConditionLocked = false;

        // Unlock the condition mutex:
        int rc2 = ::pthread_mutex_unlock(&_conditionMutex);
        GT_IF_WITH_ASSERT(rc2 == 0)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCondition::osCondition
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        14/11/2007
// ---------------------------------------------------------------------------
bool osCondition::waitForCondition()
{
    bool retVal = false;

    // Lock the condition mutex:
    int rc = ::pthread_mutex_lock(&_conditionMutex);
    GT_IF_WITH_ASSERT(rc == 0)
    {
        // While the condition is locked:
        while (_isConditionLocked)
        {
            // Release the mutex and wait for _conditionObj to be signalled (it is .
            // signalled by signalSingleThread or signalAllThreads).
            // When it is signalled - lock again the mutex automatically:
            rc = ::pthread_cond_wait(&_conditionObj, &_conditionMutex);
            GT_ASSERT(rc == 0);
        }

        // Unlock the condition mutex:
        rc = ::pthread_mutex_unlock(&_conditionMutex);
        GT_IF_WITH_ASSERT(rc == 0)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCondition::signalSingleThread
// Description:
//  Signals one of the threads, waiting for this condition object to
//  check if the condition state was changed.
// Author:      AMD Developer Tools Team
// Date:        14/11/2007
// ---------------------------------------------------------------------------
bool osCondition::signalSingleThread()
{
    bool retVal = false;

    int rc1 = ::pthread_cond_signal(&_conditionObj);
    GT_IF_WITH_ASSERT(rc1 == 0)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCondition::signalAllThreads
// Description:
//  Signals all threads, waiting for this condition object to
//  check if the condition state was changed.
// Author:      AMD Developer Tools Team
// Date:        14/11/2007
// ---------------------------------------------------------------------------
bool osCondition::signalAllThreads()
{
    bool retVal = false;

    int rc1 = ::pthread_cond_broadcast(&_conditionObj);
    GT_IF_WITH_ASSERT(rc1 == 0)
    {
        retVal = true;
    }

    return retVal;
}

