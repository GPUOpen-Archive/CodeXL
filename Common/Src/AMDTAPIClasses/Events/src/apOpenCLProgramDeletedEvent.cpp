//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLProgramDeletedEvent.cpp
///
//==================================================================================

//------------------------------ apOpenCLProgramDeletedEvent.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramDeletedEvent.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramDeletedEvent::apOpenCLProgramDeletedEvent
// Description: Constructor
// Arguments:   triggeringThreadId - The id of the thread that deleted the render context.
//              contextId - The OpenCL Server context id.
//              programId - The OpenCL program id.
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
apOpenCLProgramDeletedEvent::apOpenCLProgramDeletedEvent(osThreadId triggeringThreadId, int contextID, int programIndex, const osFilePath& programSourceFilePath)
    : apEvent(triggeringThreadId), _contextID(contextID), _programIndex(programIndex), _programSourceFilePath(programSourceFilePath)
{
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramDeletedEvent::~apOpenCLProgramDeletedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
apOpenCLProgramDeletedEvent::~apOpenCLProgramDeletedEvent()
{
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramDeletedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apOpenCLProgramDeletedEvent::type() const
{
    return OS_TOBJ_ID_CL_PROGRAM_DELETED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramDeletedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
bool apOpenCLProgramDeletedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the OpenCL Server context id:
    ipcChannel << (gtInt32)_contextID;

    // Write the OpenCL Server program id:
    ipcChannel << (gtInt32)_programIndex;

    // Write the program file path:
    _programSourceFilePath.writeSelfIntoChannel(ipcChannel);

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramDeletedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
bool apOpenCLProgramDeletedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the OpenCL Server context id:
    gtInt32 contextIDAsInt32 = 0;
    ipcChannel >> contextIDAsInt32;
    _contextID = (int)contextIDAsInt32;

    // Read the OpenCL program index:
    gtInt32 programIndexAsInt32 = 0;
    ipcChannel >> programIndexAsInt32;
    _programIndex = (int)programIndexAsInt32;

    // Read the program file path:
    _programSourceFilePath.readSelfFromChannel(ipcChannel);

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramDeletedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
apEvent::EventType apOpenCLProgramDeletedEvent::eventType() const
{
    return apEvent::AP_OPENCL_PROGRAM_DELETED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramDeletedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
apEvent* apOpenCLProgramDeletedEvent::clone() const
{
    osThreadId threadId = triggeringThreadId();
    apOpenCLProgramDeletedEvent* pEventCopy = new apOpenCLProgramDeletedEvent(threadId, _contextID, _programIndex, _programSourceFilePath);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLProgramDeletedEvent::apOpenCLProgramDeletedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        1/5/2011
// ---------------------------------------------------------------------------
apOpenCLProgramDeletedEvent::apOpenCLProgramDeletedEvent()
    : _contextID(0), _programIndex(0)
{
}

