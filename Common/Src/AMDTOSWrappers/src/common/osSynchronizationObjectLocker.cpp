//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSynchronizationObjectLocker.cpp
///
//=====================================================================

//------------------------------ osSynchronizationObjectLocker.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osSynchronizationObjectLocker.h>


// ---------------------------------------------------------------------------
// Name:        osSynchronizationObjectLocker::osSynchronizationObjectLocker
// Description: Constructor - locks the synchronization object.
// Arguments:   syncObj - The synchronization object to be locked.
// Author:      AMD Developer Tools Team
// Date:        30/3/2004
// ---------------------------------------------------------------------------
osSynchronizationObjectLocker::osSynchronizationObjectLocker(osSynchronizationObject& syncObj)
    : _syncObj(syncObj), _wasSyncObjUnlocked(false)
{
    // Lock the synchronization object:
    bool rc = _syncObj.lock();

    // Sanity check:
    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        osSynchronizationObjectLocker::~osSynchronizationObjectLocker
// Description: Destructor - Unlocks the synchronization object.
// Author:      AMD Developer Tools Team
// Date:        30/3/2004
// ---------------------------------------------------------------------------
osSynchronizationObjectLocker::~osSynchronizationObjectLocker()
{
    // If the synchronization object was not already unlocked by this class:
    if (!_wasSyncObjUnlocked)
    {
        bool rc = _syncObj.unlock();
        GT_ASSERT(rc);

        _wasSyncObjUnlocked = true;
    }
}


// ---------------------------------------------------------------------------
// Name:        osSynchronizationObjectLocker::unlockSyncObj
// Description: Unlocks the synchronization object manually.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/3/2004
// ---------------------------------------------------------------------------
bool osSynchronizationObjectLocker::unlockSyncObj()
{
    bool retVal = true;

    // If the synchronization object was not already unlocked by this class:
    if (!_wasSyncObjUnlocked)
    {
        retVal = _syncObj.unlock();
        GT_ASSERT(retVal);

        _wasSyncObjUnlocked = true;
    }

    return retVal;
}

