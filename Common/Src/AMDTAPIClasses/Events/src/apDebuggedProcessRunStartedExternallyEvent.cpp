//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessRunStartedExternallyEvent.cpp
///
//==================================================================================

//------------------------------ apDebuggedProcessRunStartedExternallyEvent.cpp ------------------------------

#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunStartedExternallyEvent.h>


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedExternallyEvent::apDebuggedProcessRunStartedExternallyEvent
// Description: Constructor
// Arguments: apDebugProjectSettings creationData - the process creation data
// Author:  AMD Developer Tools Team
// Date:        19/4/2009
// ---------------------------------------------------------------------------
apDebuggedProcessRunStartedExternallyEvent::apDebuggedProcessRunStartedExternallyEvent(const apDebugProjectSettings& creationData):
    m_processCreationData(creationData)
{

}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedExternallyEvent::~apDebuggedProcessRunStartedExternallyEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        19/4/2009
// ---------------------------------------------------------------------------
apDebuggedProcessRunStartedExternallyEvent::~apDebuggedProcessRunStartedExternallyEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedExternallyEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apDebuggedProcessRunStartedExternallyEvent::type() const
{
    return OS_TOBJ_ID_DEBUGGED_PROCESS_RUN_STARTED_EXTERNALLY_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedExternallyEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessRunStartedExternallyEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the process creation data
    bool retVal = m_processCreationData.writeSelfIntoChannel(ipcChannel);

    // Call my parent class's version of this function:
    retVal = apEvent::writeSelfIntoChannel(ipcChannel) && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedExternallyEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessRunStartedExternallyEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the process creation data:
    bool retVal = m_processCreationData.readSelfFromChannel(ipcChannel);

    // Call my parent class's version of this function:
    retVal = apEvent::readSelfFromChannel(ipcChannel) && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedExternallyEvent::type
// Description:
// Return Val: EventType
// Author:  AMD Developer Tools Team
// Date:        19/4/2009
// ---------------------------------------------------------------------------
apEvent::EventType apDebuggedProcessRunStartedExternallyEvent::eventType() const
{
    return AP_DEBUGGED_PROCESS_RUN_STARTED_EXTERNALLY;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedExternallyEvent::clone
// Description: Makes a copy of this event, it is the caller's responsabilty to
//              release this copy.
// Author:  AMD Developer Tools Team
// Date:        19/4/2009
// ---------------------------------------------------------------------------
apEvent* apDebuggedProcessRunStartedExternallyEvent::clone() const
{
    apDebuggedProcessRunStartedExternallyEvent* pClone = new apDebuggedProcessRunStartedExternallyEvent(m_processCreationData);
    return pClone;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessRunStartedExternallyEvent::apDebuggedProcessRunStartedExternallyEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apDebuggedProcessRunStartedExternallyEvent::apDebuggedProcessRunStartedExternallyEvent()
{

}
