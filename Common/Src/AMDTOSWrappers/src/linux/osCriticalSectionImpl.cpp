//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCriticalSectionImpl.cpp
///
//=====================================================================

//------------------------------ osCriticalSectionImpl.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <linux/osCriticalSectionImpl.h>


// ---------------------------------------------------------------------------
// Name:        osCriticalSectionImpl::osCriticalSectionImpl
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        6/11/2006
// ---------------------------------------------------------------------------
osCriticalSectionImpl::osCriticalSectionImpl()
{
}


// ---------------------------------------------------------------------------
// Name:        osCriticalSectionImpl::osCriticalSectionImpl
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        6/11/2006
// ---------------------------------------------------------------------------
osCriticalSectionImpl::~osCriticalSectionImpl()
{
}


// ---------------------------------------------------------------------------
// Name:        osCriticalSectionImpl::enter
// Description: "Enter" the critical section and mark it as owned by the calling thread.
//              If the critical section is already owned - wait (halts the calling thread)
//              until it will be "Left" by its owning thread.
// Author:      AMD Developer Tools Team
// Date:        6/11/2006
// ---------------------------------------------------------------------------
void osCriticalSectionImpl::enter()
{
    // Lock the mutex:
    _mutexImpl.lock();
}


// ---------------------------------------------------------------------------
// Name:        osCriticalSectionImpl::tryEntering
// Description:
//   Tries to enter the critical section.
//   - If the critical section is already owned by another thread, this function will fail,
//     returning false.
//   - If the critical section is not owned by another thread, this function will "Enter"
//     the critical section, mark it as owned by the calling thread and return true.
// Author:      AMD Developer Tools Team
// Date:        31/8/2009
// ---------------------------------------------------------------------------
bool osCriticalSectionImpl::tryEntering()
{
    // Try to lock the mutex:
    bool retVal = _mutexImpl.tryLocking();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCriticalSectionImpl::leave
// Description: "Leaves" (release ownership) the critical section. Other threads
//              can now "Enter" the critical section.
// Author:      AMD Developer Tools Team
// Date:        6/11/2006
// ---------------------------------------------------------------------------
void osCriticalSectionImpl::leave()
{
    // Unlock the mutex:
    _mutexImpl.unlock();
}

