//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTransferableObject.cpp
///
//=====================================================================

//------------------------------ osTransferableObject.cpp ------------------------------

// Standard C:
#include <limits.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osRawMemoryStream.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTOSWrappers/Include/osTransferableObjectCreatorsManager.h>


// ---------------------------------------------------------------------------
// Name:        osTransferableObject::~osTransferableObject
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        31/1/2004
// ---------------------------------------------------------------------------
osTransferableObject::~osTransferableObject()
{
}


// ---------------------------------------------------------------------------
// Name:        osTransferableObject::isParameterObject
// Description: Returns true iff this is a sub-class of apParameter.
//              (Ugly, but works ...)
// Author:      AMD Developer Tools Team
// Date:        5/5/2004
// ---------------------------------------------------------------------------
bool osTransferableObject::isParameterObject() const
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        isEventObject
// Description: Returns true iff this is a sub-class of apEvent.
//              (Ugly, but works ...)
// Author:      AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
bool osTransferableObject::isEventObject() const
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        isCLEnqueuedCommandObject
// Description: Returns true iff this is a sub-class of apCLEnqueuedCommand
//              (Ugly, but works ...)
// Author:      AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool osTransferableObject::isCLEnqueuedCommandObject() const
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        osTransferableObject::clone
// Description: Return a copy of this transferable object.
// Author:      AMD Developer Tools Team
// Date:        3/10/2004
// Implementation notes:
//   a. Create a new transferable object (of the specific sub-class type).
//   b. Dump the current object content into a raw memory stream.
//   c. Make the new object read itself from this stream.
// ---------------------------------------------------------------------------
osTransferableObject* osTransferableObject::clone() const
{
    osTransferableObject* retVal = NULL;

    // a. Create a new transferable object (of the specific sub-class type).
    osTransferableObjectType thisObjectType = type();
    gtAutoPtr<osTransferableObject> aptrTransferableObj;
    osTransferableObjectCreatorsManager& objectsCreatorMgr = osTransferableObjectCreatorsManager::instance();
    bool rc = objectsCreatorMgr.createObject(thisObjectType, aptrTransferableObj);

    if (rc)
    {
        // b. Dump the current object content into a raw memory stream.
        osRawMemoryStream rawMemoryStr(1000);
        rc = this->writeSelfIntoChannel(rawMemoryStr);

        if (rc)
        {
            // c. Make the new object read itself from this stream.
            rc = aptrTransferableObj->readSelfFromChannel(rawMemoryStr);

            if (rc)
            {
                // We will return the created and initialized object:
                retVal = aptrTransferableObj.releasePointedObjectOwnership();
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a transferable object into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        10/4/2004
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, const osTransferableObject& transferableObj)
{
    // Write the object Type:
    osTransferableObjectType objectType = transferableObj.type();
    ipcChannel << (gtUInt32)objectType;

    // Write the object itself:
    bool rc = transferableObj.writeSelfIntoChannel(ipcChannel);
    GT_ASSERT(rc);

    // Return the IPC channel:
    return ipcChannel;
}


// ---------------------------------------------------------------------------
// Name:        operator>>
// Description:
//   Reads (and creates) a transferable object from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        10/4/2004
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, gtAutoPtr<osTransferableObject>& aptrTransferableObj)
{
    // Read the transferable object Type:
    gtUInt32 objectTypeAsUInt32 = OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES;
    ipcChannel >> objectTypeAsUInt32;

    // Create a transferable object (of the read type):
    osTransferableObjectCreatorsManager& objectsCreatorMgr = osTransferableObjectCreatorsManager::instance();
    bool rc = objectsCreatorMgr.createObject((osTransferableObjectType)objectTypeAsUInt32, aptrTransferableObj);

    if (rc)
    {
        // Read the transferable object content from the IPC channel:
        rc = aptrTransferableObj->readSelfFromChannel(ipcChannel);
    }
    else // !rc
    {
        gtString errMsg;
        errMsg.appendFormattedString(L"Failed to create transferable object of type %u", objectTypeAsUInt32);
        GT_ASSERT_EX(rc, errMsg.asCharArray());
    }

    // Sanity test:
    GT_ASSERT(rc);

    // Return the IPC channel:
    return ipcChannel;
}

