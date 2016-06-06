//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSynchronizationObject.cpp
///
//=====================================================================

//------------------------------ osSynchronizationObject.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Per OS implementations of this class:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <win32/osSynchronizationObjectImpl.h>
#endif

// Local:
#include <AMDTOSWrappers/Include/osSynchronizationObject.h>


// ---------------------------------------------------------------------------
// Name:        osSynchronizationObject::osSynchronizationObject
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        21/6/2005
// ---------------------------------------------------------------------------
osSynchronizationObject::osSynchronizationObject()
{
    // Create our OS specific synchronization object implementation:
    _pImplementation = new osSynchronizationObjectImpl;
    GT_ASSERT(_pImplementation);
}


// ---------------------------------------------------------------------------
// Name:        osSynchronizationObject::~osSynchronizationObject
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        21/6/2005
// ---------------------------------------------------------------------------
osSynchronizationObject::~osSynchronizationObject()
{
    delete _pImplementation;
}


// ---------------------------------------------------------------------------
// Name:        osSynchronizationObject::lock
// Description: "Locks" (try to own) the synchronization object. If the synchronization
//              object is currently owned  by another thread, this function will halt
//              its calling thread until the synchronization object is "Unlocked".
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/6/2005
// ---------------------------------------------------------------------------
bool osSynchronizationObject::lock()
{
    return _pImplementation->lock();
}

// ---------------------------------------------------------------------------
// Name:        osSynchronizationObject::unlock
// Description: Unlocks the synchronization object. (Release its ownership).
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/6/2005
// ---------------------------------------------------------------------------
bool osSynchronizationObject::unlock()
{
    return _pImplementation->unlock();
}

