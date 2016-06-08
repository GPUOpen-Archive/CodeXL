//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSynchronizationObjectImpl.cpp
///
//=====================================================================

//------------------------------ osSynchronizationObjectImpl.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <win32/osSynchronizationObjectImpl.h>


// ---------------------------------------------------------------------------
// Name:        osSynchronizationObjectImpl::osSynchronizationObjectImpl
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        21/6/2005
// Implementation notes:
//   We will use a Win32 "auto reset" event object that allows only one thread
//   to "own" the event object.
//
//   From MSDN documentation of PulseEvent:
//     For an auto-reset event object, the function resets the state to non-signaled
//     and returns after releasing a single waiting thread, even if multiple threads
//     are waiting.
// ---------------------------------------------------------------------------
osSynchronizationObjectImpl::osSynchronizationObjectImpl()
{
    // Create a Win32 "auto reset" event object:
    _eventObjHandle = ::CreateEvent(NULL, FALSE, TRUE, NULL);

    // Sanity check:
    GT_ASSERT(_eventObjHandle != NULL);
}


// ---------------------------------------------------------------------------
// Name:        osSynchronizationObjectImpl::osSynchronizationObjectImpl
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        21/6/2005
// ---------------------------------------------------------------------------
osSynchronizationObjectImpl::~osSynchronizationObjectImpl()
{
    if (_eventObjHandle)
    {
        // Destroy the Win32 event object:
        ::CloseHandle(_eventObjHandle);
    }
}


// ---------------------------------------------------------------------------
// Name:        osSynchronizationObjectImpl::lock
// Description: Locks the synchronization object. If the synchronization object
//              is already locked by another thread, waits (halts the calling thread)
//              until it will be unlocked.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/6/2005
// ---------------------------------------------------------------------------
bool osSynchronizationObjectImpl::lock()
{
    bool retVal = false;

    // Lock (or wait for) the Win32 event object:
    DWORD rc = ::WaitForSingleObject(_eventObjHandle, INFINITE);

    // The event object was not locked - we managed to lock it:
    if (rc == WAIT_OBJECT_0)
    {
        retVal = true;
    }
    else if (rc == WAIT_ABANDONED)
    {
        // The thread that owned the mutex died without unlocking it.
        // This enabled us to lock the mutex:
        retVal = true;

        // TO_DO: Replace me by a log print:
        GT_ASSERT(0);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSynchronizationObjectImpl::unlock
// Description: Frees (unlocks) the event object for the use of other threads.
//              If other threads are waiting for this event object (called lock()
//              while it was locked), the first thread that was blocked by lock()
//              call will resume its run and the event object will remain locked.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/6/2005
// ---------------------------------------------------------------------------
bool osSynchronizationObjectImpl::unlock()
{
    bool retVal = false;

    // Unlock the event object:
    if (::SetEvent(_eventObjHandle) != 0)
    {
        retVal = true;
    }

    return retVal;
}

