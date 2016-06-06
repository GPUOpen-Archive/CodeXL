//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apAllocatedObject.cpp
///
//==================================================================================

//------------------------------ apAllocatedObject.cpp ------------------------------

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local
#include <AMDTAPIClasses/Include/apAllocatedObject.h>

// ---------------------------------------------------------------------------
// Name:        apAllocatedObject::apAllocatedObject
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        20/10/2008
// ---------------------------------------------------------------------------
apAllocatedObject::apAllocatedObject(): _allocatedObjectId(-1)
{

}

// ---------------------------------------------------------------------------
// Name:        apAllocatedObject::~apAllocatedObject
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        20/10/2008
// ---------------------------------------------------------------------------
apAllocatedObject::~apAllocatedObject()
{

}


// ---------------------------------------------------------------------------
// Name:        apAllocatedObject::setAllocatedObjectId
// Description: Sets the object's allocated object ID, but only if it has not
//              been set before
// Author:  AMD Developer Tools Team
// Date:        20/10/2008
// ---------------------------------------------------------------------------
bool apAllocatedObject::setAllocatedObjectId(int allocObjId, bool allowOverwrite)
{
    bool retVal = false;

    // Make sure we are not changing the ID of an existing object.
    GT_IF_WITH_ASSERT(allowOverwrite || (_allocatedObjectId == -1))
    {
        _allocatedObjectId = allocObjId;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apAllocatedObject::writeSelfIntoChannel
// Description:
// Arguments: osChannel& ipcChannel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/10/2008
// ---------------------------------------------------------------------------
bool apAllocatedObject::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    ipcChannel << (gtInt32)_allocatedObjectId;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apAllocatedObject::readSelfFromChannel
// Description:
// Arguments: osChannel& ipcChannel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/10/2008
// ---------------------------------------------------------------------------
bool apAllocatedObject::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 allocObjId = -1;

    ipcChannel >> allocObjId;

    setAllocatedObjectId((int)allocObjId);

    return true;
}
