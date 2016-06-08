//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSynchronizationObjectLocker.h
///
//=====================================================================

//------------------------------ osSynchronizationObjectLocker.h ------------------------------

#ifndef __OSSYNCHRONIZATIONOBJECTLOCKER
#define __OSSYNCHRONIZATIONOBJECTLOCKER
#pragma once


// Local:
#include <AMDTOSWrappers/Include/osSynchronizationObject.h>

// ----------------------------------------------------------------------------------
// Class Name:           OS_API osSynchronizationObjectLocker
// General Description:
//   Aid class that enables "exception safe" locking of a synchronization object.
//   Its constructor locks the synchronization object and destructor unlocks the
//   synchronization object.
//   This causes synchronization object unlocking in case of an exception.
//   Example:
//     void foo(osSynchronizationObject& mySyncObj)
//     {
//         osSynchronizationObjectLocker syncObjLucker(mySyncObj);
//
//         < doing something >
//
//     }
//
//     In the above example, the synchronization object will be unlocked in the following scenarios:
//     a. The thread exits the function: Exiting the function executes the syncObjLucker
//        destructor, which unlocks the synchronization object.
//     b. An exception is thrown while < doing something > is executed.
//        If there is no exception handler in the function, the exception will be "thrown out"
//        of the function, calling all the destructors of the function stack variables.
//        Among these destructors is the syncObjLucker destructor, which will unlocks the
//        synchronization object.
//
//     The user can also call unlockSyncObj() manually to unlock the synchronization object
//     before the osSynchronizationObjectLocker destructor is called.
//
// Author:      AMD Developer Tools Team
// Creation Date:        29/3/2004
// ----------------------------------------------------------------------------------
class OS_API osSynchronizationObjectLocker
{
public:
    osSynchronizationObjectLocker(osSynchronizationObject& syncObj);
    ~osSynchronizationObjectLocker();

    bool unlockSyncObj();

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    osSynchronizationObjectLocker() = delete;
    osSynchronizationObjectLocker(const osSynchronizationObjectLocker&) = delete;
    osSynchronizationObjectLocker& operator=(const osSynchronizationObjectLocker&) = delete;

private:
    // The synchronization object on which this class operates:
    osSynchronizationObject& _syncObj;

    // Contains true iff the synchronization object was already unlocked by this class:
    bool _wasSyncObjUnlocked;
};


#endif  // __OSSYNCHRONIZATIONOBJECTLOCKER
