//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCriticalSectionImpl.cpp
///
//=====================================================================

//------------------------------ osCriticalSectionImpl.cpp ------------------------------

// Windows:
#define _WIN32_WINNT 0x403
#include <Windows.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <win32/osCriticalSectionImpl.h>


// ---------------------------------------------------------------------------
// Name:        osCriticalSectionImpl::osCriticalSectionImpl
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        19/5/2005
// ---------------------------------------------------------------------------
osCriticalSectionImpl::osCriticalSectionImpl()
{
    // Initialize the win32 critical section object:
    ::InitializeCriticalSection(&_win32CriticalSectionObject);
}


// ---------------------------------------------------------------------------
// Name:        osCriticalSectionImpl::osCriticalSectionImpl
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        19/5/2005
// ---------------------------------------------------------------------------
osCriticalSectionImpl::~osCriticalSectionImpl()
{
    // Releases all resources used by the win32 critical section object
    ::DeleteCriticalSection(&_win32CriticalSectionObject);
}


// ---------------------------------------------------------------------------
// Name:        osCriticalSectionImpl::enter
// Description: "Enter" the critical section and mark it as owned by the calling thread.
//              If the critical section is already owned - wait (halts the calling thread)
//              until it will be "Left" by its owning thread.
// Author:      AMD Developer Tools Team
// Date:        19/5/2005
// ---------------------------------------------------------------------------
void osCriticalSectionImpl::enter()
{
    // Enter (or wait for) the Win32 critical section:
    EnterCriticalSection(&_win32CriticalSectionObject);
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
    bool retVal = false;

    // Try to enter the critical section:
    BOOL rc1 = TryEnterCriticalSection(&_win32CriticalSectionObject);

    if (rc1 != 0)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCriticalSectionImpl::leave
// Description: "Leaves" (release ownership) the critical section. Other threads
//              can now "Enter" the critical section.
// Author:      AMD Developer Tools Team
// Date:        19/5/2005
// ---------------------------------------------------------------------------
void osCriticalSectionImpl::leave()
{
    // Leave (release ownership) the Win32 critical section:
    ::LeaveCriticalSection(&_win32CriticalSectionObject);
}

