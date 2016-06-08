//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCriticalSectionImpl.h
///
//=====================================================================

//------------------------------ osCriticalSectionImpl.h ------------------------------

#ifndef __OSCRITICALSECTIONIMPL
#define __OSCRITICALSECTIONIMPL

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// ----------------------------------------------------------------------------------
// Class Name:           osCriticalSectionImpl
// General Description: Win32 Implementation of osCriticalSection.
// Author:      AMD Developer Tools Team
// Creation Date:        19/5/2005
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
    // The Win32 Critical section object:
    CRITICAL_SECTION _win32CriticalSectionObject;
};


#endif  // __OSCRITICALSECTIONIMPL
