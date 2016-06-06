//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apKernelWorkItemChangedEvent.cpp
///
//==================================================================================

//------------------------------ apKernelWorkItemChangedEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apKernelWorkItemChangedEvent.h>

// ---------------------------------------------------------------------------
// Name:        apKernelWorkItemChangedEvent::apKernelWorkItemChangedEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        13/3/2011
// ---------------------------------------------------------------------------
apKernelWorkItemChangedEvent::apKernelWorkItemChangedEvent(int coordinate, int workItemValue)
    : apEvent(), _coordinate(coordinate), _workItemValue(workItemValue)
{
}

// ---------------------------------------------------------------------------
// Name:        apKernelWorkItemChangedEvent::~apKernelWorkItemChangedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        13/3/2011
// ---------------------------------------------------------------------------
apKernelWorkItemChangedEvent::~apKernelWorkItemChangedEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apKernelWorkItemChangedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        13/3/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apKernelWorkItemChangedEvent::type() const
{
    return OS_TOBJ_ID_KERNEL_CURRENT_WORK_ITEM_CHANGED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apKernelWorkItemChangedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/3/2011
// ---------------------------------------------------------------------------
bool apKernelWorkItemChangedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    // Write the global work offset:
    ipcChannel << (gtUInt32)_coordinate;
    ipcChannel << (gtUInt32)_workItemValue;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apKernelWorkItemChangedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/3/2011
// ---------------------------------------------------------------------------
bool apKernelWorkItemChangedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    // Read the global work offset:
    gtUInt32 varInt32 = GT_UINT32_MAX;
    ipcChannel >> varInt32;
    _coordinate = (int)varInt32;
    ipcChannel >> varInt32;
    _workItemValue = (int)varInt32;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apKernelWorkItemChangedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        13/3/2011
// ---------------------------------------------------------------------------
apEvent::EventType apKernelWorkItemChangedEvent::eventType() const
{
    return apEvent::AP_KERNEL_CURRENT_WORK_ITEM_CHANGED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apKernelWorkItemChangedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        13/3/2011
// ---------------------------------------------------------------------------
apEvent* apKernelWorkItemChangedEvent::clone() const
{
    apKernelWorkItemChangedEvent* pClone = new apKernelWorkItemChangedEvent(_coordinate, _workItemValue);


    return pClone;
}

