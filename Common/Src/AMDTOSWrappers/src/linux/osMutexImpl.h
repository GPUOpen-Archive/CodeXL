//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osMutexImpl.h ------------------------------

#ifndef __OSMUTEXIMPL
#define __OSMUTEXIMPL

// POSIX:
#include <pthread.h>

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>


// ----------------------------------------------------------------------------------
// Class Name:           osMutexImpl
// General Description: Win32 Implementation of osMutex.
// Author:      AMD Developer Tools Team
// Creation Date:        29/3/2004
// ----------------------------------------------------------------------------------
class osMutexImpl
{
public:
    osMutexImpl();
    virtual ~osMutexImpl();
    bool lock();
    bool tryLocking();
    bool unlock();

private:
    // The POSIX mutex:
    osMutexHandle _posixMutex;
};


#endif  // __OSMUTEXIMPL
