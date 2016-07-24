//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apBeforeKernelDebuggingEvent.cpp
///
//==================================================================================

//------------------------------ apBeforeKernelDebuggingEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apBeforeKernelDebuggingEvent.h>

// ---------------------------------------------------------------------------
// Name:        apBeforeKernelDebuggingEvent::apBeforeKernelDebuggingEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
apBeforeKernelDebuggingEvent::apBeforeKernelDebuggingEvent(apKernelDebuggingType debuggingType, osThreadId triggeringThreadID, unsigned int workDimension, const gtSize_t* globalWorkOffset, const gtSize_t* globalWorkSize, const gtSize_t* localWorkSize)
    : apEvent(triggeringThreadID), m_debuggingType(debuggingType)
{
    bool isXValid = (workDimension > 0);
    bool isYValid = (workDimension > 1);
    bool isZValid = (workDimension > 2);

    _globalWorkSize[0] = isXValid ? (int)globalWorkSize[0] : -1;
    _globalWorkSize[1] = isYValid ? (int)globalWorkSize[1] : -1;
    _globalWorkSize[2] = isZValid ? (int)globalWorkSize[2] : -1;

    if (globalWorkOffset != NULL)
    {
        _globalWorkOffset[0] = isXValid ? (int)globalWorkOffset[0] : -1;
        _globalWorkOffset[1] = isYValid ? (int)globalWorkOffset[1] : -1;
        _globalWorkOffset[2] = isZValid ? (int)globalWorkOffset[2] : -1;
    }
    else // globalWorkOffset == NULL
    {
        _globalWorkOffset[0] = isXValid ? 0 : -1;
        _globalWorkOffset[1] = isYValid ? 0 : -1;
        _globalWorkOffset[2] = isZValid ? 0 : -1;
    }

    // If we don't have the local work size, use the global work size in its stead:
    if (localWorkSize != NULL)
    {
        _localWorkSize[0] = isXValid ? (int)localWorkSize[0] : -1;
        _localWorkSize[1] = isYValid ? (int)localWorkSize[1] : -1;
        _localWorkSize[2] = isZValid ? (int)localWorkSize[2] : -1;
    }
    else // localWorkSize == NULL
    {
        _localWorkSize[0] = _globalWorkSize[0];
        _localWorkSize[1] = _globalWorkSize[1];
        _localWorkSize[2] = _globalWorkSize[2];
    }
}

// ---------------------------------------------------------------------------
// Name:        apBeforeKernelDebuggingEvent::~apBeforeKernelDebuggingEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
apBeforeKernelDebuggingEvent::~apBeforeKernelDebuggingEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apBeforeKernelDebuggingEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apBeforeKernelDebuggingEvent::type() const
{
    return OS_TOBJ_ID_BEFORE_KERNEL_DEBUGGING_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apBeforeKernelDebuggingEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
bool apBeforeKernelDebuggingEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    // Write the debugging type:
    ipcChannel << (gtInt32)m_debuggingType;

    // Write the global work offset:
    ipcChannel << (gtUInt64)_globalWorkOffset[0];
    ipcChannel << (gtUInt64)_globalWorkOffset[1];
    ipcChannel << (gtUInt64)_globalWorkOffset[2];

    // Write the global work size:
    ipcChannel << (gtUInt64)_globalWorkSize[0];
    ipcChannel << (gtUInt64)_globalWorkSize[1];
    ipcChannel << (gtUInt64)_globalWorkSize[2];

    // Write the local work size:
    ipcChannel << (gtUInt64)_localWorkSize[0];
    ipcChannel << (gtUInt64)_localWorkSize[1];
    ipcChannel << (gtUInt64)_localWorkSize[2];

    //////////////////////////////////////////////////////////////////////////
    // Uri, 30/5/11:
    // We READ from the pipe here to synchronize the Server and Client.
    // The place where this event is written is immediately followed by
    // triggering a breakpoint exception. If we do not synchronize here, the
    // event registration will race with the breakpoint handling, sometimes
    // causing an unexpected event to reach the package event handlers, which
    // messes up the timing.
    //////////////////////////////////////////////////////////////////////////
    gtInt32 dummyValue = -1;
    ipcChannel >> dummyValue;
    GT_ASSERT(dummyValue == 0x881305);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apBeforeKernelDebuggingEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
bool apBeforeKernelDebuggingEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    // Read the debugging type:
    gtInt32 debuggingTypeAsInt32 = -1;
    ipcChannel >> debuggingTypeAsInt32;
    m_debuggingType = (apKernelDebuggingType)debuggingTypeAsInt32;

    // Read the global work offset:
    gtUInt64 globalWorkOffset0AsUInt64 = GT_UINT64_MAX;
    ipcChannel >> globalWorkOffset0AsUInt64;
    _globalWorkOffset[0] = (gtSize_t)globalWorkOffset0AsUInt64;
    gtUInt64 globalWorkOffset1AsUInt64 = GT_UINT64_MAX;
    ipcChannel >> globalWorkOffset1AsUInt64;
    _globalWorkOffset[1] = (gtSize_t)globalWorkOffset1AsUInt64;
    gtUInt64 globalWorkOffset2AsUInt64 = GT_UINT64_MAX;
    ipcChannel >> globalWorkOffset2AsUInt64;
    _globalWorkOffset[2] = (gtSize_t)globalWorkOffset2AsUInt64;

    // Read the global work size:
    gtUInt64 globalWorkSize0AsUInt64 = GT_UINT64_MAX;
    ipcChannel >> globalWorkSize0AsUInt64;
    _globalWorkSize[0] = (gtSize_t)globalWorkSize0AsUInt64;
    gtUInt64 globalWorkSize1AsUInt64 = GT_UINT64_MAX;
    ipcChannel >> globalWorkSize1AsUInt64;
    _globalWorkSize[1] = (gtSize_t)globalWorkSize1AsUInt64;
    gtUInt64 globalWorkSize2AsUInt64 = GT_UINT64_MAX;
    ipcChannel >> globalWorkSize2AsUInt64;
    _globalWorkSize[2] = (gtSize_t)globalWorkSize2AsUInt64;

    // Read the local work size:
    gtUInt64 localWorkSize0AsUInt64 = GT_UINT64_MAX;
    ipcChannel >> localWorkSize0AsUInt64;
    _localWorkSize[0] = (gtSize_t)localWorkSize0AsUInt64;
    gtUInt64 localWorkSize1AsUInt64 = GT_UINT64_MAX;
    ipcChannel >> localWorkSize1AsUInt64;
    _localWorkSize[1] = (gtSize_t)localWorkSize1AsUInt64;
    gtUInt64 localWorkSize2AsUInt64 = GT_UINT64_MAX;
    ipcChannel >> localWorkSize2AsUInt64;
    _localWorkSize[2] = (gtSize_t)localWorkSize2AsUInt64;

    //////////////////////////////////////////////////////////////////////////
    // Uri, 30/5/11:
    // We WRITE into the pipe here to synchronize the Server and Client.
    // The place where this event is written is immediately followed by
    // triggering a breakpoint exception. If we do not synchronize here, the
    // event registration will race with the breakpoint handling, sometimes
    // causing an unexpected event to reach the package event handlers, which
    // messes up the timing.
    //////////////////////////////////////////////////////////////////////////
    gtInt32 dummyValue = 0x881305;
    ipcChannel << dummyValue;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apBeforeKernelDebuggingEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
apEvent::EventType apBeforeKernelDebuggingEvent::eventType() const
{
    return apEvent::AP_BEFORE_KERNEL_DEBUGGING_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apBeforeKernelDebuggingEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
apEvent* apBeforeKernelDebuggingEvent::clone() const
{
    apBeforeKernelDebuggingEvent* pClone = new apBeforeKernelDebuggingEvent(m_debuggingType, triggeringThreadId(), 3, _globalWorkOffset, _globalWorkSize, _localWorkSize);

    return pClone;
}

