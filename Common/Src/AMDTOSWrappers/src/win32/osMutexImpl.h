//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osMutexImpl.h
///
//=====================================================================

//------------------------------ osMutexImpl.h ------------------------------

#ifndef __OSMUTEXIMPL
#define __OSMUTEXIMPL

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

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
    bool unlock();

private:
    // The Win32 Mutex handle:
    osMutexHandle _mutexHandle;
};


#endif  // __OSMUTEXIMPL
