//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLProgramCreatedEvent.cpp
///
//==================================================================================

//------------------------------ apOpenCLProgramCreatedEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramCreatedEvent.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramCreatedEvent::apOpenCLProgramCreatedEvent
// Description: Constructor
// Arguments:   triggeringThreadId - The id of the thread that deleted the render context.
//              contextId - The OpenCL Server context id.
//              programIndex - The OpenCL program id.
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
apOpenCLProgramCreatedEvent::apOpenCLProgramCreatedEvent(osThreadId triggeringThreadId, int contextID, int programIndex, const osFilePath& programSourceFilePath)
    : apEvent(triggeringThreadId), _contextID(contextID), _programIndex(programIndex), _programSourceFilePath(programSourceFilePath)
{
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramCreatedEvent::~apOpenCLProgramCreatedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
apOpenCLProgramCreatedEvent::~apOpenCLProgramCreatedEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramCreatedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apOpenCLProgramCreatedEvent::type() const
{
    return OS_TOBJ_ID_CL_PROGRAM_CREATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramCreatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
bool apOpenCLProgramCreatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the OpenCL Server context id:
    ipcChannel << (gtInt32)_contextID;

    // Write the OpenCL Server program index:
    ipcChannel << (gtInt32)_programIndex;

    // Write the program source file path:
    bool retVal = _programSourceFilePath.writeSelfIntoChannel(ipcChannel);

    // Call my parent class's version of this function:
    retVal = apEvent::writeSelfIntoChannel(ipcChannel) && retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramCreatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
bool apOpenCLProgramCreatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the OpenCL Server context id:
    gtInt32 contextIDAsInt32 = 0;
    ipcChannel >> contextIDAsInt32;
    _contextID = (int)contextIDAsInt32;

    // Read the OpenCL program index:
    gtInt32 programIndexAsInt32 = 0;
    ipcChannel >> programIndexAsInt32;
    _programIndex = (int)programIndexAsInt32;

    // Read the program source file path
    bool retVal = _programSourceFilePath.readSelfFromChannel(ipcChannel);

    // Call my parent class's version of this function:
    retVal = apEvent::readSelfFromChannel(ipcChannel) && retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramCreatedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
apEvent::EventType apOpenCLProgramCreatedEvent::eventType() const
{
    return apEvent::AP_OPENCL_PROGRAM_CREATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramCreatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
apEvent* apOpenCLProgramCreatedEvent::clone() const
{
    osThreadId threadId = triggeringThreadId();
    apOpenCLProgramCreatedEvent* pEventCopy = new apOpenCLProgramCreatedEvent(threadId, _contextID, _programIndex, _programSourceFilePath);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramCreatedEvent::apOpenCLProgramCreatedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
apOpenCLProgramCreatedEvent::apOpenCLProgramCreatedEvent()
    : _contextID(0), _programIndex(0)
{
}

