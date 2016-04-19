//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdUnknown.cpp
///
//==================================================================================

//------------------------------ vsdUnknown.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <CodeXLVSPackageDebugger/Include/vsdUnknown.h>


// ---------------------------------------------------------------------------
// Name:        vsdCUnknown::vsdCUnknown
// Description: Constructor
// Author:      Uri Shomroni
// Date:        15/9/2010
// ---------------------------------------------------------------------------
vsdCUnknown::vsdCUnknown()
    : m_referenceCount(1)
{
}

// ---------------------------------------------------------------------------
// Name:        vsdCUnknown::~vsdCUnknown
// Description: Destructor
// Author:      Uri Shomroni
// Date:        15/9/2010
// ---------------------------------------------------------------------------
vsdCUnknown::~vsdCUnknown()
{
    // Make sure we weren't deleted in a C++ way:
    GT_ASSERT(m_referenceCount == 0);
}

// ---------------------------------------------------------------------------
// Name:        vsdCUnknown::addRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
ULONG vsdCUnknown::addRef(void)
{
    m_referenceCount++;
    return m_referenceCount;
}

// ---------------------------------------------------------------------------
// Name:        vsdCUnknown::release
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
ULONG vsdCUnknown::release(void)
{
    if (m_referenceCount > 0)
    {
        m_referenceCount--;
    }

    ULONG retVal = m_referenceCount;

    if (m_referenceCount == 0)
    {
        delete this;
    }

    return retVal;
}

