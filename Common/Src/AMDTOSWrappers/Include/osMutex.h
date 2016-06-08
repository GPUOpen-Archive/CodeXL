//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osMutex.h
///
//=====================================================================

//------------------------------ osMutex.h ------------------------------

#ifndef __OSMUTEX
#define __OSMUTEX

// Pre-declarations:
class osMutexImpl;

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           osMutex
//
// General Description:
//   A "mutex" object. Mutex objects enables coordinated access to a resource that
//   is shared by few threads.
//   "Mutex" comes from "Mutually-exclusive access" to a shared resource.
//   Only one thread at a time can "lock" the mutex. Other threads that will try to
//   "lock" the mutex will halt until the mutex is "unlocked" by the locking thread.
//
//   Notice:
//   a. Prefer using osMutexLocker on using osMutex directly (see osMutexLocker for more details).
//   b. On Windows, critical sections are only visible inside one process and a bit more
//      efficient than mutexes.
//      I.E: On Windows:
//      - If you need to synchronize threads within one process - use critical section objects.
//      - If you need to synchronize threads within few process - use mutex objects.
//
// Author:      AMD Developer Tools Team
// Creation Date:        29/3/2004
// ----------------------------------------------------------------------------------
class OS_API osMutex
{
public:
    osMutex();
    virtual ~osMutex();
    bool lock();
    bool unlock();

private:
    // This class OS specific implementation:
    osMutexImpl* _pImplementation;
};

#endif  // __OSMUTEX
