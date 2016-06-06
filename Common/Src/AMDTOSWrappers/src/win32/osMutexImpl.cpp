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
#include <win32/osMutexImpl.h>


// ---------------------------------------------------------------------------
// Name:        osMutexImpl::osMutexImpl
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        29/3/2004
// ---------------------------------------------------------------------------
osMutexImpl::osMutexImpl()
{
    // Create an unnamed Win32 mutex:
    _mutexHandle = ::CreateMutex(NULL, FALSE, NULL);

    // Sanity check:
    GT_ASSERT(_mutexHandle);
}


// ---------------------------------------------------------------------------
// Name:        osMutexImpl::osMutexImpl
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        29/3/2004
// ---------------------------------------------------------------------------
osMutexImpl::~osMutexImpl()
{
    if (_mutexHandle)
    {
        // Destroy the Win32 mutex:
        ::CloseHandle(_mutexHandle);
    }
}


// ---------------------------------------------------------------------------
// Name:        osMutexImpl::lock
// Description: Locks the mutex and mark it as owned by the calling thread.
//              If the Mutex is already locked - waits (halts the calling thread)
//              until it will be unlocked by its owning thread.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/3/2004
// ---------------------------------------------------------------------------
bool osMutexImpl::lock()
{
    bool retVal = false;

    // Lock (or wait for) the Win32 mutex:
    DWORD rc = ::WaitForSingleObject(_mutexHandle, INFINITE);

    // The mutex was not locked - we managed to lock and own the mutex:
    if (rc == WAIT_OBJECT_0)
    {
        retVal = true;
    }
    else if (rc == WAIT_ABANDONED)
    {
        // The thread that owned the mutex died without unlocking it.
        // This enabled us to lock and own the mutex:
        retVal = true;

        // TO_DO: Replace me by a log print:
        GT_ASSERT(0);
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
// Date:        29/3/2004
// ---------------------------------------------------------------------------
bool osMutexImpl::unlock()
{
    bool retVal = true;

    // Unlock the Win32 mutex:
    if (! ::ReleaseMutex(_mutexHandle))
    {
        retVal = false;
    }

    return retVal;
}

