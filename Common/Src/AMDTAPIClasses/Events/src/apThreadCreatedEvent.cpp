//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apThreadCreatedEvent.cpp
///
//==================================================================================

//------------------------------ apThreadCreatedEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>


// ---------------------------------------------------------------------------
// Name:        apThreadCreatedEvent::apThreadCreatedEvent
// Description: Constructor
// Arguments:   threadOSId - The thread Operating system id.
//              lwpOSId - The Light Weight Process Operating system id (Linux).
//              threadCreationTime - The thread creation time.
//              threadStartAddress - The thread start address (in debugged process
//                                   virtual address space)
// Author:  AMD Developer Tools Team
// Date:        8/5/2005
// ---------------------------------------------------------------------------
apThreadCreatedEvent::apThreadCreatedEvent(const osThreadId& threadOSId,
                                           const osThreadId& lwpOSId,
                                           const osTime& threadCreationTime,
                                           osInstructionPointer threadStartAddress)
    : _threadOSId(threadOSId), _lwpOSId(lwpOSId), _threadCreationTime(threadCreationTime),
      _threadStartAddress(threadStartAddress), _startFunctionSourceCodeFileLineNum(-1)
{
}


// ---------------------------------------------------------------------------
// Name:        apThreadCreatedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apThreadCreatedEvent::type() const
{
    return OS_TOBJ_ID_THREAD_CREATED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apThreadCreatedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apThreadCreatedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the thread OS id into the channel:
    ipcChannel << (gtUInt64)_threadOSId;

    // Write the LWP OS id into the channel:
    ipcChannel << (gtUInt64)_lwpOSId;

    // Write the thread creation time into the channel:
    ipcChannel << _threadCreationTime;

    // Write the thread start address into the channel:
    ipcChannel << (gtUInt64)_threadStartAddress;

    // The path of the module in which this address resides:
    retVal = retVal && _threadStartModuleFilePath.writeSelfIntoChannel(ipcChannel);

    // Write the name of the function on which the start address points into the channel:
    ipcChannel << _threadStartFunctionName;

    // Write the details of the source code into the channel:
    retVal = retVal && _startFunctionSourceCodeFile.writeSelfIntoChannel(ipcChannel);

    // Write the start function source function line number into the channel:
    ipcChannel << (gtInt32)_startFunctionSourceCodeFileLineNum;

    // Call my parent class's version of this function:
    retVal = apEvent::writeSelfIntoChannel(ipcChannel) && retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apThreadCreatedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apThreadCreatedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the thread OS id from the channel:
    gtUInt64 threadOSIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    ipcChannel >> threadOSIdAsUInt64;
    _threadOSId = (osThreadId)threadOSIdAsUInt64;

    // Read the LWP OS id from the channel:
    gtUInt64 lwpOSIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    ipcChannel >> lwpOSIdAsUInt64;
    _lwpOSId = (osThreadId)lwpOSIdAsUInt64;

    // Read the thread creation time from the channel:
    ipcChannel >> _threadCreationTime;

    // Read the thread start address from the channel:
    gtUInt64 threadStartAddressAsUInt64 = 0;
    ipcChannel >> threadStartAddressAsUInt64;
    _threadStartAddress = (osInstructionPointer)threadStartAddressAsUInt64;

    // Read the path of the module from the channel:
    retVal = retVal && _threadStartModuleFilePath.readSelfFromChannel(ipcChannel);

    // Read the name of the function from the channel:
    ipcChannel >> _threadStartFunctionName;

    // Write the details of the source code into the channel:
    retVal = retVal && _startFunctionSourceCodeFile.readSelfFromChannel(ipcChannel);

    // Write the start function source function line number into the channel:
    gtUInt32 functionFileLineAsUInt32 = 0;
    ipcChannel >> functionFileLineAsUInt32;
    _startFunctionSourceCodeFileLineNum = (int)functionFileLineAsUInt32;

    // Call my parent class's version of this function:
    retVal = apEvent::readSelfFromChannel(ipcChannel) && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apThreadCreatedEvent::type
// Description: Returns my event type.
// Author:  AMD Developer Tools Team
// Date:        8/5/2005
// ---------------------------------------------------------------------------
apEvent::EventType apThreadCreatedEvent::eventType() const
{
    return apEvent::AP_THREAD_CREATED;
}


// ---------------------------------------------------------------------------
// Name:        apThreadCreatedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        8/5/2005
// ---------------------------------------------------------------------------
apEvent* apThreadCreatedEvent::clone() const
{
    apThreadCreatedEvent* retVal = new apThreadCreatedEvent(_threadOSId, _lwpOSId, _threadCreationTime,
                                                            _threadStartAddress);

    if (retVal)
    {
        retVal->setThreadStartModuleFilePath(_threadStartModuleFilePath);
        retVal->setThreadStartFunctionName(_threadStartFunctionName);
        retVal->setThreadStartSourceCodeDetails(_startFunctionSourceCodeFile, _startFunctionSourceCodeFileLineNum);
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apThreadCreatedEvent::apThreadCreatedEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apThreadCreatedEvent::apThreadCreatedEvent():
    _threadOSId(OS_NO_THREAD_ID), _lwpOSId(OS_NO_THREAD_ID), _threadStartAddress(NULL), _startFunctionSourceCodeFileLineNum(-1)
{

}

