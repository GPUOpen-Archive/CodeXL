//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMonitoredFunctionBreakPoint.cpp
///
//==================================================================================

//------------------------------ apMonitoredFunctionBreakPoint.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>


// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionBreakPoint::compareToOther
// Description: Returns true iff otherBreakpoint is an identical breakpoint.
// Author:  AMD Developer Tools Team
// Date:        1/11/2010
// ---------------------------------------------------------------------------
bool apMonitoredFunctionBreakPoint::compareToOther(const apBreakPoint& otherBreakpoint) const
{
    bool retVal = false;

    // If the other breakpoint is a monitored function breakpoint:
    if (otherBreakpoint.type() == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
    {
        // Downcast it:
        const apMonitoredFunctionBreakPoint& otherMonitoredFuncBreakpoint = (const apMonitoredFunctionBreakPoint&)otherBreakpoint;

        // The only thing to compare is the breakpoint function id:
        if (otherMonitoredFuncBreakpoint.monitoredFunctionId() == monitoredFunctionId())
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionBreakPoint::type
// Description: Returns my transferable object type
// Author:  AMD Developer Tools Team
// Date:        13/5/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apMonitoredFunctionBreakPoint::type() const
{
    return OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT;
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionBreakPoint::writeSelfIntoChannel
// Description: Writes my content into a channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/5/2004
// ---------------------------------------------------------------------------
bool apMonitoredFunctionBreakPoint::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    bool rc1 = apBreakPoint::writeSelfIntoChannel(ipcChannel);
    ipcChannel << (gtInt32)_monitoredFunctionId;

    retVal = rc1;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionBreakPoint::readSelfFromChannel
// Description: Reads my content from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/5/2004
// ---------------------------------------------------------------------------
bool apMonitoredFunctionBreakPoint::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    bool rc1 = apBreakPoint::readSelfFromChannel(ipcChannel);
    gtInt32 monitoredFunctionIdAsInt32 = 0;
    ipcChannel >> monitoredFunctionIdAsInt32;
    _monitoredFunctionId = (apMonitoredFunctionId)monitoredFunctionIdAsInt32;

    retVal = rc1;

    return retVal;
}

