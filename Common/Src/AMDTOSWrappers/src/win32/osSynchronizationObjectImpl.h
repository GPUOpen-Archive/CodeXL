//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSynchronizationObjectImpl.h
///
//=====================================================================

//------------------------------ osSynchronizationObjectImpl.h ------------------------------

#ifndef __OSSYNCHRONIZATIONOBJECTIMPL
#define __OSSYNCHRONIZATIONOBJECTIMPL

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>


// ----------------------------------------------------------------------------------
// Class Name:           osSynchronizationObjectImpl
// General Description: Win32 Implementation of a synchronization object.
// Author:      AMD Developer Tools Team
// Creation Date:        29/3/2004
// ----------------------------------------------------------------------------------
class osSynchronizationObjectImpl
{
public:
    osSynchronizationObjectImpl();
    virtual ~osSynchronizationObjectImpl();
    bool lock();
    bool unlock();

private:
    // The event object (synchronization object) handle:
    HANDLE _eventObjHandle;
};


#endif  // __OSSYNCHRONIZATIONOBJECTIMPL
