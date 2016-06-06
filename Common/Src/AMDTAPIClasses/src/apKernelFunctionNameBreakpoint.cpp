//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apKernelFunctionNameBreakpoint.cpp
///
//==================================================================================

//------------------------------ apKernelFunctionNameBreakpoint.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apKernelFunctionNameBreakpoint.h>

// ---------------------------------------------------------------------------
// Name:        apKernelFunctionNameBreakpoint::compareToOther
// Description: Returns true iff otherBreakpoint is an identical breakpoint.
// Author:  AMD Developer Tools Team
// Date:        23/2/2011
// ---------------------------------------------------------------------------
bool apKernelFunctionNameBreakpoint::compareToOther(const apBreakPoint& otherBreakpoint) const
{
    bool retVal = false;

    // If the other breakpoint is a monitored function breakpoint:
    if (otherBreakpoint.type() == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
    {
        // Downcast it:
        const apKernelFunctionNameBreakpoint& otherKernelFuncNameBreakpoint = (const apKernelFunctionNameBreakpoint&)otherBreakpoint;

        // The only thing to compare is the kernel function name:
        if (otherKernelFuncNameBreakpoint.kernelFunctionName() == kernelFunctionName())
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apKernelFunctionNameBreakpoint::type
// Description: Returns my transferable object type
// Author:  AMD Developer Tools Team
// Date:        23/2/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apKernelFunctionNameBreakpoint::type() const
{
    return OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT;
}


// ---------------------------------------------------------------------------
// Name:        apKernelFunctionNameBreakpoint::writeSelfIntoChannel
// Description: Writes my content into a channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        23/2/2011
// ---------------------------------------------------------------------------
bool apKernelFunctionNameBreakpoint::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    bool rc1 = apBreakPoint::writeSelfIntoChannel(ipcChannel);
    ipcChannel << _kernelFunctionName;

    retVal = rc1;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apKernelFunctionNameBreakpoint::readSelfFromChannel
// Description: Reads my content from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        23/2/2011
// ---------------------------------------------------------------------------
bool apKernelFunctionNameBreakpoint::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    bool rc1 = apBreakPoint::readSelfFromChannel(ipcChannel);
    ipcChannel >> _kernelFunctionName;

    retVal = rc1;

    return retVal;
}

