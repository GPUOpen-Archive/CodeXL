//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osCriticalSectionImpl.h ------------------------------

#ifndef __OSCRITICALSECTIONIMPL
#define __OSCRITICALSECTIONIMPL

// Local:
#include <linux/osMutexImpl.h>

// ----------------------------------------------------------------------------------
// Class Name:           osCriticalSectionImpl
// General Description:
//   Linux does not support critical section. Therefore, on Linux, we implement
//   critical section using mutex.
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/11/2006
// ----------------------------------------------------------------------------------
class osCriticalSectionImpl
{
public:
    osCriticalSectionImpl();
    virtual ~osCriticalSectionImpl();
    void enter();
    bool tryEntering();
    void leave();

private:
    // The mutex implementation:
    osMutexImpl _mutexImpl;
};


#endif  // __OSCRITICALSECTIONIMPL
