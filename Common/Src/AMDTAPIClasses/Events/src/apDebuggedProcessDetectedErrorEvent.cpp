//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessDetectedErrorEvent.cpp
///
//==================================================================================

//------------------------------ apDebuggedProcessDetectedErrorEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessDetectedErrorEvent.h>


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessDetectedErrorEvent::apDebuggedProcessDetectedErrorEvent
// Description: Constructor
// Arguments:   errorCode - The error code.
//              detectedErrorParameters - The detected error parameters.
// Author:  AMD Developer Tools Team
// Date:        21/3/2005
// ---------------------------------------------------------------------------
apDebuggedProcessDetectedErrorEvent::apDebuggedProcessDetectedErrorEvent(osThreadId triggeringThreadId,
        const apDetectedErrorParameters& detectedErrorParameters,
        bool wasGeneratedByBreak)
    : apEvent(triggeringThreadId), _detectedErrorParameters(detectedErrorParameters), _wasGeneratedByBreak(wasGeneratedByBreak)
{
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessDetectedErrorEvent::~apDebuggedProcessDetectedErrorEvent
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        21/3/2005
// ---------------------------------------------------------------------------
apDebuggedProcessDetectedErrorEvent::~apDebuggedProcessDetectedErrorEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessDetectedErrorEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apDebuggedProcessDetectedErrorEvent::type() const
{
    return OS_TOBJ_ID_DETECTED_ERROR_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessDetectedErrorEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessDetectedErrorEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    // Write the detected error parameters into the channel:
    retVal = _detectedErrorParameters.writeSelfIntoChannel(ipcChannel);

    ipcChannel << _wasGeneratedByBreak;

    // Call my parent class's version of this function:
    retVal = apEvent::writeSelfIntoChannel(ipcChannel) && retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessDetectedErrorEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apDebuggedProcessDetectedErrorEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the detected error parameters from channel:
    retVal = _detectedErrorParameters.readSelfFromChannel(ipcChannel);

    ipcChannel >> _wasGeneratedByBreak;

    // Call my parent class's version of this function:
    retVal = apEvent::readSelfFromChannel(ipcChannel) && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessDetectedErrorEvent::type
// Description: Returns my event type.
// Author:  AMD Developer Tools Team
// Date:        21/3/2005
// ---------------------------------------------------------------------------
apEvent::EventType apDebuggedProcessDetectedErrorEvent::eventType() const
{
    return apEvent::AP_DETECTED_ERROR_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessDetectedErrorEvent::clone
// Description: Returns a new copy of self. It is the callers responsibility
//              to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        21/3/2005
// ---------------------------------------------------------------------------
apEvent* apDebuggedProcessDetectedErrorEvent::clone() const
{
    osThreadId threadId = triggeringThreadId();
    return new apDebuggedProcessDetectedErrorEvent(threadId, _detectedErrorParameters, _wasGeneratedByBreak);
}

// ---------------------------------------------------------------------------
// Name:        apDebuggedProcessDetectedErrorEvent::apDebuggedProcessDetectedErrorEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apDebuggedProcessDetectedErrorEvent::apDebuggedProcessDetectedErrorEvent():
    _wasGeneratedByBreak(true)
{

}
