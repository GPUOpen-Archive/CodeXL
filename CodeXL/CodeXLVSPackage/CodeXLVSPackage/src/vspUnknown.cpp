//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspUnknown.cpp
///
//==================================================================================

//------------------------------ vspUnknown.cpp ------------------------------

#include "stdafx.h"

// C++:
#include <cassert>

// Local:
#include <src/vspUnknown.h>


// ---------------------------------------------------------------------------
// Name:        vspCUnknown::vspCUnknown
// Description: Constructor
// Author:      Uri Shomroni
// Date:        15/9/2010
// ---------------------------------------------------------------------------
vspCUnknown::vspCUnknown()
    : _referenceCount(1)
{
}

// ---------------------------------------------------------------------------
// Name:        vspCUnknown::~vspCUnknown
// Description: Destructor
// Author:      Uri Shomroni
// Date:        15/9/2010
// ---------------------------------------------------------------------------
vspCUnknown::~vspCUnknown()
{
    // Make sure we weren't deleted in a C++ way:
    assert(_referenceCount == 0);
}

// ---------------------------------------------------------------------------
// Name:        vspCUnknown::addRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
ULONG vspCUnknown::addRef(void)
{
    _referenceCount++;
    return _referenceCount;
}

// ---------------------------------------------------------------------------
// Name:        vspCUnknown::release
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        8/9/2010
// ---------------------------------------------------------------------------
ULONG vspCUnknown::release(void)
{
    if (_referenceCount > 0)
    {
        _referenceCount--;
    }

    ULONG retVal = _referenceCount;

    if (_referenceCount == 0)
    {
        delete this;
    }

    return retVal;
}

