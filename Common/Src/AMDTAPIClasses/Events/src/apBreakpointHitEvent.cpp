//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apBreakpointHitEvent.cpp
///
//==================================================================================

//------------------------------ apBreakpointHitEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>


// ---------------------------------------------------------------------------
// Name:        apBreakpointHitEvent::apBreakpointHitEvent
// Description: Constructor
// Arguments:
//    triggeringThreadId - The id of the thread that triggered the event.
//    breakPointAddress - The address in which the break point
//                                  occurred (In debugged process address space)
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apBreakpointHitEvent::apBreakpointHitEvent(osThreadId triggeringThreadId, osInstructionPointer breakPointAddress)
    : apEvent(triggeringThreadId),
      _breakReason(AP_FOREIGN_BREAK_HIT),
      _breakPointAddress(breakPointAddress),
      _openGLErrorCode(GL_NO_ERROR)
{
}

// ---------------------------------------------------------------------------
// Name:        apBreakpointHitEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apBreakpointHitEvent::type() const
{
    return OS_TOBJ_ID_BREAKPOINT_HIT_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apBreakpointHitEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apBreakpointHitEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the break reason:
    ipcChannel << (gtInt32)_breakReason;

    bool isFunctionCallPresent = (_aptrBreakedOnFunctionCall.pointedObject() != NULL);

    ipcChannel << isFunctionCallPresent;

    if (isFunctionCallPresent)
    {
        // Write the broken-on function:
        retVal = _aptrBreakedOnFunctionCall->writeSelfIntoChannel(ipcChannel);
    }

    // Write the break address:
    ipcChannel << (gtUInt64)_breakPointAddress;

    // Write the error code:
    ipcChannel << (gtInt32)_openGLErrorCode;

    // Call my parent class's version of this function:
    retVal = apEvent::writeSelfIntoChannel(ipcChannel) && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apBreakpointHitEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apBreakpointHitEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the break reason:
    gtInt32 breakReasonAsInt32 = AP_FOREIGN_BREAK_HIT;
    ipcChannel >> breakReasonAsInt32;
    _breakReason = (apBreakReason)breakReasonAsInt32;

    bool isFunctionCallPresent = false;
    ipcChannel >> isFunctionCallPresent;

    if (isFunctionCallPresent)
    {
        // Read the broken-on function (and replace the existing item if needed):
        apFunctionCall* pFuncCall = new apFunctionCall;

        retVal = pFuncCall->readSelfFromChannel(ipcChannel);
        _aptrBreakedOnFunctionCall = pFuncCall;
    }
    else
    {
        _aptrBreakedOnFunctionCall = NULL;
        retVal = true;
    }

    // Read the break address:
    gtUInt64 breakPointAddressAsUInt64 = 0;
    ipcChannel >> breakPointAddressAsUInt64;
    _breakPointAddress = (osInstructionPointer)breakPointAddressAsUInt64;

    // Read the error code:
    gtInt32 openGLErrorCodeAsInt32 = GL_NO_ERROR;
    ipcChannel >> openGLErrorCodeAsInt32;
    _openGLErrorCode = (GLenum)openGLErrorCodeAsInt32;

    // Call my parent class's version of this function:
    retVal = apEvent::readSelfFromChannel(ipcChannel) && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apBreakpointHitEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent::EventType apBreakpointHitEvent::eventType() const
{
    return apEvent::AP_BREAKPOINT_HIT;
}


// ---------------------------------------------------------------------------
// Name:        apBreakpointHitEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent* apBreakpointHitEvent::clone() const
{
    // Get the triggering thread id:
    osThreadId threadId = triggeringThreadId();

    // Create a new breakpoint event:
    apBreakpointHitEvent* pEventCopy = new apBreakpointHitEvent(threadId, _breakPointAddress);

    if (pEventCopy)
    {
        // Copy this event values into it:
        pEventCopy->setBreakReason(_breakReason);
        pEventCopy->setOpenGLErrorCode(_openGLErrorCode);

        if (_aptrBreakedOnFunctionCall.pointedObject() != NULL)
        {
            gtAutoPtr<apFunctionCall> aptrFunctionCallCopy = (apFunctionCall*)(_aptrBreakedOnFunctionCall->clone());
            pEventCopy->setBreakedOnFunctionCall(aptrFunctionCallCopy);
        }
    }

    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apBreakpointHitEvent::apBreakpointHitEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apBreakpointHitEvent::apBreakpointHitEvent():
    _breakReason(AP_FOREIGN_BREAK_HIT), _breakPointAddress(NULL), _openGLErrorCode(GL_NO_ERROR)
{

}

