//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCriticalSection.cpp
///
//=====================================================================

//------------------------------ osCriticalSection.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Per OS implementations of this class:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <win32/osCriticalSectionImpl.h>
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <linux/osCriticalSectionImpl.h>
#else
    #error Unknown build configuration!
#endif

// Local:
#include <AMDTOSWrappers/Include/osCriticalSection.h>


// ---------------------------------------------------------------------------
// Name:        osCriticalSection::osCriticalSection
// Description: Constructor - creates the critical section object.
// Author:      AMD Developer Tools Team
// Date:        19/5/2005
// ---------------------------------------------------------------------------
osCriticalSection::osCriticalSection()
{
    // Create our OS specific Critical section implementation:
    _pImplementation = new osCriticalSectionImpl;

}


// ---------------------------------------------------------------------------
// Name:        osCriticalSection::~osCriticalSection
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        19/5/2005
// ---------------------------------------------------------------------------
osCriticalSection::~osCriticalSection()
{
    delete _pImplementation;
    _pImplementation = NULL;
}


// ---------------------------------------------------------------------------
// Name:        osCriticalSection::enter
// Description: "Enter" (try to own) the critical section. If the critical section
//              is currently owned by another thread, this function will halt its
//              calling thread until the critical section is "Left".
// Author:      AMD Developer Tools Team
// Date:        19/5/2005
// ---------------------------------------------------------------------------
void osCriticalSection::enter()
{
    if (_pImplementation != NULL)
    {
        _pImplementation->enter();
    }
}


// ---------------------------------------------------------------------------
// Name:        osCriticalSection::tryEntering
// Description:
//   Tries to enter the critical section.
//   - If the critical section is already owned by another thread, this function will fail,
//     returning false.
//   - If the critical section is not owned by another thread, this function will "Enter"
//     the critical section, mark it as owned by the calling thread and return true.
// Author:      AMD Developer Tools Team
// Date:        31/8/2009
// ---------------------------------------------------------------------------
bool osCriticalSection::tryEntering()
{
    bool retVal = false;

    if (_pImplementation != NULL)
    {
        // Try to enter the critical section:
        retVal = _pImplementation->tryEntering();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCriticalSection::leave
// Description: "Leave" (release ownership) the critical section.
// Author:      AMD Developer Tools Team
// Date:        19/5/2005
// ---------------------------------------------------------------------------
void osCriticalSection::leave()
{
    if (_pImplementation != NULL)
    {
        _pImplementation->leave();
    }
}
