//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osMutexImpl.cpp
///
//=====================================================================

//------------------------------ osMutexImpl.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <linux/osMutexImpl.h>


// ---------------------------------------------------------------------------
// Name:        osMutexImpl::osMutexImpl
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        5/11/2006
// ---------------------------------------------------------------------------
osMutexImpl::osMutexImpl()
{
    // Create a mutex attributes objects that creates a recursive mutex:
    // (A recursive mutex enables locking the mutex several times by the same thread,
    //  maintaining a lock count)
    pthread_mutexattr_t  mutexAttributes;
    pthread_mutexattr_init(&mutexAttributes);
    pthread_mutexattr_settype(&mutexAttributes, PTHREAD_MUTEX_RECURSIVE);

    // Create the POSIX mutex:
    int rc = ::pthread_mutex_init(&_posixMutex, &mutexAttributes);
    GT_ASSERT(rc == 0);
}


// ---------------------------------------------------------------------------
// Name:        osMutexImpl::osMutexImpl
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        5/11/2006
// ---------------------------------------------------------------------------
osMutexImpl::~osMutexImpl()
{
    // Destroy the mutex:
    int rc = ::pthread_mutex_destroy(&_posixMutex);
    GT_ASSERT(rc == 0);
}


// ---------------------------------------------------------------------------
// Name:        osMutexImpl::lock
// Description: Locks the mutex and mark it as owned by the calling thread.
//              If the Mutex is already locked - waits (halts the calling thread)
//              until it will be unlocked by its owning thread.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/11/2006
// ---------------------------------------------------------------------------
bool osMutexImpl::lock()
{
    bool retVal = false;

    // Lock the POSIX mutex:
    int rc = ::pthread_mutex_lock(&_posixMutex);
    GT_IF_WITH_ASSERT(rc == 0)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osMutexImpl::tryLocking
// Description:
//   Tries to locks the mutex.
//   - If the mutex is already locked by another thread, this function will fail,
//     returning false.
//   - If the mutex is not locked, this function will lock the mutex, mark it as owned
//     by the calling thread and return true.
// Author:      AMD Developer Tools Team
// Date:        31/8/2009
// ---------------------------------------------------------------------------
bool osMutexImpl::tryLocking()
{
    bool retVal = false;

    // Try to lock the mutex:
    int rc1 = pthread_mutex_trylock(&_posixMutex);

    if (rc1 == 0)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osMutexImpl::unlock
// Description: Frees (unlocks) the mutex for the use of other threads.
//              If other threads are waiting for this mutex (called lock() while it
//              was owned and locked by this thread), the first thread that was
//              blocked by lock() call will gain the mutex ownership.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/11/2006
// ---------------------------------------------------------------------------
bool osMutexImpl::unlock()
{
    bool retVal = false;

    // Unlock the POSIX mutex:
    int rc = ::pthread_mutex_unlock(&_posixMutex);
    GT_IF_WITH_ASSERT(rc == 0)
    {
        retVal = true;
    }

    return retVal;
}

