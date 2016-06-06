//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osMutexLocker.cpp
///
//=====================================================================

//------------------------------ osMutexLocker.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osMutexLocker.h>


// ---------------------------------------------------------------------------
// Name:        osMutexLocker::osMutexLocker
// Description: Constructor - locks the mutex.
// Arguments:   mutexObj - The mutex to be locked.
// Author:      AMD Developer Tools Team
// Date:        30/3/2004
// ---------------------------------------------------------------------------
osMutexLocker::osMutexLocker(osMutex& mutexObj)
    : _mutex(mutexObj), _wasMutexUnlocked(false)
{
    // Lock the Mutex:
    bool rc = _mutex.lock();

    // Sanity check:
    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        osMutexLocker::~osMutexLocker
// Description: Destructor - Unlocks the mutex.
// Author:      AMD Developer Tools Team
// Date:        30/3/2004
// ---------------------------------------------------------------------------
osMutexLocker::~osMutexLocker()
{
    // If the mutex was not already unlocked by this class:
    if (!_wasMutexUnlocked)
    {
        bool rc = _mutex.unlock();
        GT_ASSERT(rc);

        _wasMutexUnlocked = true;
    }
}


// ---------------------------------------------------------------------------
// Name:        osMutexLocker::unlockMutex
// Description: Unlocks the mutex manually.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/3/2004
// ---------------------------------------------------------------------------
bool osMutexLocker::unlockMutex()
{
    bool retVal = true;

    // If the mutex was not already unlocked by this class:
    if (!_wasMutexUnlocked)
    {
        retVal = _mutex.unlock();
        GT_ASSERT(retVal);

        _wasMutexUnlocked = true;
    }

    return retVal;
}

