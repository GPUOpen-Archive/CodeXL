//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osMutexLocker.h
///
//=====================================================================

//------------------------------ osMutexLocker.h ------------------------------

#ifndef __OSMUTEXLOCKER
#define __OSMUTEXLOCKER

// Local:
#include <AMDTOSWrappers/Include/osMutex.h>

// ----------------------------------------------------------------------------------
// Class Name:           OS_API osMutexLocker
// General Description:
//   Aid class that enables "exception safe" locking of a mutex.
//   Its constructor locks the mutex and destructor unlocks the mutex.
//   This causes mutex unlocking in case of an exception.
//   Example:
//     void foo(osMutex& myMutex)
//     {
//         osMutexLocker mutexLucker(myMutex);
//
//         < doing something >
//
//     }
//
//     In the above example, the mutex will be unlocked in the following scenarios:
//     a. The thread exits the function: Exiting the function executes the mutexLucker
//        destructor, which unlocks the mutex.
//     b. An exception is thrown while < doing something > is executed.
//        If there is no exception handler in the function, the exception will be "thrown out"
//        of the function, calling all the destructors of the function stack variables.
//        Among these destructors is the mutexLucker destructor, which will unlocks the mutex.
//
//     The user can also call unlockMutex() manually to unlock the mutex before the
//     osMutexLocker destructor is called.
//
// Author:      AMD Developer Tools Team
// Creation Date:        29/3/2004
// ----------------------------------------------------------------------------------
class OS_API osMutexLocker
{
public:
    osMutexLocker(osMutex& mutexObj);
    ~osMutexLocker();

    bool unlockMutex();

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    osMutexLocker() = delete;
    osMutexLocker(const osMutexLocker&) = delete;
    osMutexLocker& operator=(const osMutexLocker&) = delete;

    // The mutex on which this class operates:
    osMutex& _mutex;

    // Contains true iff the mutex was already unlocked by this class:
    bool _wasMutexUnlocked;
};

#endif  // __OSMUTEXLOCKER
