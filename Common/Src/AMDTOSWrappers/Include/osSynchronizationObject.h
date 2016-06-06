//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSynchronizationObject.h
///
//=====================================================================

//------------------------------ osSynchronizationObject.h ------------------------------

#ifndef __OSSYNCHRONIZATIONOBJECT
#define __OSSYNCHRONIZATIONOBJECT

// Pre-declarations:
class osSynchronizationObjectImpl;

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           osSynchronizationObject
//
// General Description:
//   A threads synchronization object. Enables coordinated access to a resource that is
//   shared by few threads.
//   Only one thread can "lock" the synchronization object at a time. Other threads that
//   will try to "lock" the synchronization object will halt until the synchronization object
//   is "unlocked" by the thread that "owns" it.
//
//   Notice:
//     osSynchronizationObject enables locking the synchronization object from one thread and
//     unlocking it from another thread. If you don't require this ability, use osCriticalSection
//     or osMutex.
//
// Author:      AMD Developer Tools Team
// Creation Date:        21/07/2005
// ----------------------------------------------------------------------------------
class OS_API osSynchronizationObject
{
public:
    osSynchronizationObject();
    virtual ~osSynchronizationObject();
    bool lock();
    bool unlock();

private:
    // This class OS specific implementation:
    osSynchronizationObjectImpl* _pImplementation;
};


#endif  // __OSSYNCHRONIZATIONOBJECT
