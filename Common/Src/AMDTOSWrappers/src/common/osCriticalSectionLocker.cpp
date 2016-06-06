//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCriticalSectionLocker.cpp
///
//=====================================================================

//------------------------------ osCriticalSectionLocker.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>


// ---------------------------------------------------------------------------
// Name:        osCriticalSectionLocker::osCriticalSectionLocker
// Description: Constructor - "enters" the critical section.
// Arguments:   criticalSectionObj - The critical section to be locked.
// Author:      AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
osCriticalSectionLocker::osCriticalSectionLocker(osCriticalSection& criticalSectionObj)
    : _criticalSection(criticalSectionObj), _wasCriticalSectionLeft(false)
{
    // "Enter" the critical section:
    _criticalSection.enter();
}


// ---------------------------------------------------------------------------
// Name:        osCriticalSectionLocker::~osCriticalSectionLocker
// Description: Destructor - Unlocks the critical section.
// Author:      AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
osCriticalSectionLocker::~osCriticalSectionLocker()
{
    // If the critical section was not left by this class - leave it now:
    leaveCriticalSection();
}


// ---------------------------------------------------------------------------
// Name:        osCriticalSectionLocker::leaveCriticalSection
// Description: Leave the critical section manually.
// Author:      AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
void osCriticalSectionLocker::leaveCriticalSection()
{
    // If the critical section was not left by this class - leave it now:
    if (!_wasCriticalSectionLeft)
    {
        _criticalSection.leave();
        _wasCriticalSectionLeft = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        osCriticalSectionLocker::osCriticalSectionDelayedLocker
// Description: Constructor.
// Author:      AMD Developer Tools Team
// Date:        5/7/2015
// ---------------------------------------------------------------------------
osCriticalSectionDelayedLocker::osCriticalSectionDelayedLocker()
    : m_pCriticalSection(nullptr)
{

}

// ---------------------------------------------------------------------------
// Name:        osCriticalSectionLocker::~osCriticalSectionDelayedLocker
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        5/7/2015
// ---------------------------------------------------------------------------
osCriticalSectionDelayedLocker::~osCriticalSectionDelayedLocker()
{
    leaveCriticalSection();
}

// ---------------------------------------------------------------------------
// Name:        osCriticalSectionLocker::attachToCriticalSection
// Description: Enter the critical section.
// Author:      AMD Developer Tools Team
// Date:        5/7/2015
// ---------------------------------------------------------------------------
bool osCriticalSectionDelayedLocker::attachToCriticalSection(osCriticalSection& criticalSectionObj)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr == m_pCriticalSection)
    {
        // For thread safety of this object, first lock the object, and then set it as member:
        criticalSectionObj.enter();

        m_pCriticalSection = &criticalSectionObj;

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osCriticalSectionLocker::leaveCriticalSection
// Description: Leave the critical section manually.
// Author:      AMD Developer Tools Team
// Date:        5/7/2015
// ---------------------------------------------------------------------------
void osCriticalSectionDelayedLocker::leaveCriticalSection()
{
    if (nullptr != m_pCriticalSection)
    {
        // For thread safety of this object, first clear the member, then release the critical section:
        osCriticalSection& criticalSectionObj = *m_pCriticalSection;

        m_pCriticalSection = nullptr;

        criticalSectionObj.leave();
    }
}
