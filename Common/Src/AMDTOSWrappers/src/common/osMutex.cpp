//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osMutex.cpp
///
//=====================================================================

//------------------------------ osMutex.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Per OS implementations of this class:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <win32/osMutexImpl.h>
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <linux/osMutexImpl.h>
#else
    #error Unknown build configuration!
#endif

// Local:
#include <AMDTOSWrappers/Include/osMutex.h>


// ---------------------------------------------------------------------------
// Name:        osMutex::osMutex
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        29/3/2004
// ---------------------------------------------------------------------------
osMutex::osMutex()
{
    // Create our OS specific Mutex implementation:
    _pImplementation = new osMutexImpl;

}


// ---------------------------------------------------------------------------
// Name:        osMutex::~osMutex
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        29/3/2004
// ---------------------------------------------------------------------------
osMutex::~osMutex()
{
    delete _pImplementation;
    _pImplementation = NULL;
}


// ---------------------------------------------------------------------------
// Name:        osMutex::lock
// Description: "Locks" (try to own) the Mutex. If the Mutex is currently owned
//              by another thread, this function will halt its calling thread until
//              the Mutex is "Unlocked".
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/3/2004
// ---------------------------------------------------------------------------
bool osMutex::lock()
{
    return _pImplementation->lock();
}

// ---------------------------------------------------------------------------
// Name:        osMutex::unlock
// Description: Unlocks the Mutex. (Release its ownership).
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/3/2004
// ---------------------------------------------------------------------------
bool osMutex::unlock()
{
    return _pImplementation->unlock();
}

