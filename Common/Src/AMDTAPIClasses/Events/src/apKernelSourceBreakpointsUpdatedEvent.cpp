//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apKernelSourceBreakpointsUpdatedEvent.cpp
///
//==================================================================================

//------------------------------ apKernelSourceBreakpointsUpdatedEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apKernelSourceBreakpointsUpdatedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apKernelSourceBreakpointsUpdatedEvent::apKernelSourceBreakpointsUpdatedEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        8/5/2011
// ---------------------------------------------------------------------------
apKernelSourceBreakpointsUpdatedEvent::apKernelSourceBreakpointsUpdatedEvent(osThreadId triggeringThreadID, oaCLProgramHandle debuggedProgramHandle)
    : apEvent(triggeringThreadID), _debuggedProgramHandle(debuggedProgramHandle)
{

}

// ---------------------------------------------------------------------------
// Name:        apKernelSourceBreakpointsUpdatedEvent::~apKernelSourceBreakpointsUpdatedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        8/5/2011
// ---------------------------------------------------------------------------
apKernelSourceBreakpointsUpdatedEvent::~apKernelSourceBreakpointsUpdatedEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apKernelSourceBreakpointsUpdatedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        8/5/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apKernelSourceBreakpointsUpdatedEvent::type() const
{
    return OS_TOBJ_ID_KERNEL_SOURCE_BREAKPOINTS_UPDATED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apKernelSourceBreakpointsUpdatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/5/2011
// ---------------------------------------------------------------------------
bool apKernelSourceBreakpointsUpdatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    // Write the program handle
    ipcChannel << (gtUInt64)_debuggedProgramHandle;

    // Write the breakpoint bindings:
    gtInt32 numberOfBindings = (gtInt32)_updatedBreakpoints.size();
    ipcChannel << numberOfBindings;

    for (gtInt32 i = 0; i < numberOfBindings; i++)
    {
        // Requested line:
        ipcChannel << (gtInt32)_updatedBreakpoints[i]._requestedLineNumber;

        // Bound line:
        ipcChannel << (gtInt32)_updatedBreakpoints[i]._boundLineNumber;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apKernelSourceBreakpointsUpdatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/5/2011
// ---------------------------------------------------------------------------
bool apKernelSourceBreakpointsUpdatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    // Read the program handle
    gtUInt64 debuggedProgramHandleAsUInt64 = OA_CL_NULL_HANDLE;
    ipcChannel >> debuggedProgramHandleAsUInt64;
    _debuggedProgramHandle = (oaCLProgramHandle)debuggedProgramHandleAsUInt64;

    // Read the breakpoint bindings:
    _updatedBreakpoints.clear();
    gtInt32 numberOfBindings = -1;
    ipcChannel >> numberOfBindings;

    for (gtInt32 i = 0; i < numberOfBindings; i++)
    {
        // Requested line:
        gtInt32 requestedLineAsInt32 = -1;
        ipcChannel >> requestedLineAsInt32;

        // Bound line:
        gtInt32 boundLineAsInt32 = -1;
        ipcChannel >> boundLineAsInt32;

        addBreakpointBinding((int)requestedLineAsInt32, (int)boundLineAsInt32);
    }

    return retVal;


}

// ---------------------------------------------------------------------------
// Name:        apKernelSourceBreakpointsUpdatedEvent::eventType
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        8/5/2011
// ---------------------------------------------------------------------------
apEvent::EventType apKernelSourceBreakpointsUpdatedEvent::eventType() const
{
    return apEvent::AP_KERNEL_SOURCE_BREAKPOINTS_UPDATED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apKernelSourceBreakpointsUpdatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        8/5/2011
// ---------------------------------------------------------------------------
apEvent* apKernelSourceBreakpointsUpdatedEvent::clone() const
{
    apKernelSourceBreakpointsUpdatedEvent* pClone = new apKernelSourceBreakpointsUpdatedEvent(triggeringThreadId(), _debuggedProgramHandle);


    int numberOfBindings = (int)_updatedBreakpoints.size();

    for (int i = 0; i < numberOfBindings; i++)
    {
        pClone->addBreakpointBinding(_updatedBreakpoints[i]._requestedLineNumber, _updatedBreakpoints[i]._boundLineNumber);
    }

    return pClone;
}

// ---------------------------------------------------------------------------
// Name:        apKernelSourceBreakpointsUpdatedEvent::addBreakpointBinding
// Description: Adds a breakpoint binding to our vector:
// Author:  AMD Developer Tools Team
// Date:        8/5/2011
// ---------------------------------------------------------------------------
void apKernelSourceBreakpointsUpdatedEvent::addBreakpointBinding(int requestedLineNumber, int boundLineNumber)
{
    apKernelSourceBreakpointBinding newBinding;
    newBinding._requestedLineNumber = requestedLineNumber;
    newBinding._boundLineNumber = boundLineNumber;

    _updatedBreakpoints.push_back(newBinding);
}

// ---------------------------------------------------------------------------
// Name:        apKernelSourceBreakpointsUpdatedEvent::getBreakpointBoundLineNumber
// Description: Looks through the breakpoints bound again at the event. If the
//              requested line number is matched, returns the new bound number.
//              otherwise, returns -1.
// Author:  AMD Developer Tools Team
// Date:        8/5/2011
// ---------------------------------------------------------------------------
int apKernelSourceBreakpointsUpdatedEvent::getBreakpointBoundLineNumber(int requestedLineNumber) const
{
    int retVal = -1;

    // Look for this binding in our list:
    int numberOfBindings = (int)_updatedBreakpoints.size();

    for (int i = 0; i < numberOfBindings; i++)
    {
        // If we found it:
        if (_updatedBreakpoints[i]._requestedLineNumber == requestedLineNumber)
        {
            // Make sure it's a valid value:
            int foundBoundLineNumber = _updatedBreakpoints[i]._boundLineNumber;

            if (foundBoundLineNumber > -1)
            {
                // Return the value:
                retVal = foundBoundLineNumber;
                break;
            }
        }
    }

    return retVal;
}

