//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGenericBreakpoint.cpp
///
//==================================================================================

//------------------------------ apGenericBreakpoint.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        apGenericBreakpoint::apGenericBreakpoint
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        6/7/2011
// ---------------------------------------------------------------------------
apGenericBreakpoint::apGenericBreakpoint()
    : _breakType(AP_BREAK_TYPE_UNKNOWN)
{

}

// ---------------------------------------------------------------------------
// Name:        apGenericBreakpoint::apGenericBreakpoint
// Description: Constructor
// Arguments:   apGenericBreakpointType breakType
// Author:  AMD Developer Tools Team
// Date:        6/7/2011
// ---------------------------------------------------------------------------
apGenericBreakpoint::apGenericBreakpoint(apGenericBreakpointType breakType)
    : _breakType(breakType)
{

}

// ---------------------------------------------------------------------------
// Name:        apGenericBreakpoint::compareToOther
// Description: Returns true iff otherBreakpoint is an identical breakpoint.
// Author:  AMD Developer Tools Team
// Date:        6/7/2011
// ---------------------------------------------------------------------------
bool apGenericBreakpoint::compareToOther(const apBreakPoint& otherBreakpoint) const
{
    bool retVal = false;

    // If the other breakpoint is a monitored function breakpoint:
    if (otherBreakpoint.type() == OS_TOBJ_ID_GENERIC_BREAKPOINT)
    {
        // Downcast it:
        const apGenericBreakpoint& otherGenericBreakpoint = (const apGenericBreakpoint&)otherBreakpoint;

        // The only thing to compare is the type:
        if (otherGenericBreakpoint.breakpointType() == breakpointType())
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGenericBreakpoint::type
// Description: Returns my transferable object type
// Author:  AMD Developer Tools Team
// Date:        6/7/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apGenericBreakpoint::type() const
{
    return OS_TOBJ_ID_GENERIC_BREAKPOINT;
}


// ---------------------------------------------------------------------------
// Name:        apGenericBreakpoint::writeSelfIntoChannel
// Description: Writes my content into a channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        6/7/2011
// ---------------------------------------------------------------------------
bool apGenericBreakpoint::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    ipcChannel << (gtInt32)_breakType;

    bool rc1 = apBreakPoint::writeSelfIntoChannel(ipcChannel);

    retVal = rc1;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGenericBreakpoint::readSelfFromChannel
// Description: Reads my content from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        6/7/2011
// ---------------------------------------------------------------------------
bool apGenericBreakpoint::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    gtInt32 valueAsInt32 = 0;
    ipcChannel >> valueAsInt32;
    _breakType = (apGenericBreakpointType)valueAsInt32;

    bool rc1 = apBreakPoint::readSelfFromChannel(ipcChannel);

    retVal = rc1;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGenericBreakpoint::breakpointTypeToString
// Description: Get the breakpoint type as string
// Arguments:   apGenericBreakpointType breakType
//              gtString& breakpointName
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        6/7/2011
// ---------------------------------------------------------------------------
bool apGenericBreakpoint::breakpointTypeToString(apGenericBreakpointType breakType, gtString& breakpointName)
{
    bool retVal = true;

    breakpointName.makeEmpty();

    switch (breakType)
    {

        case AP_BREAK_ON_GL_ERROR:
            breakpointName = AP_STR_GLErrorBreakpoint;
            break;

        case AP_BREAK_ON_CL_ERROR:
            breakpointName = AP_STR_CLErrorBreakpoint;
            break;

        case AP_BREAK_ON_DETECTED_ERROR:
            breakpointName = AP_STR_DetectedErrorBreakpoint;
            break;

        case AP_BREAK_ON_DEPRECATED_FUNCTION:
            breakpointName = AP_STR_DeprecatedBreakpoint;
            break;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case AP_BREAK_ON_SOFTWARE_FALLBACK:
            breakpointName = AP_STR_SoftwareFallbackBreakpoint;
            break;
#endif

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

        case AP_BREAK_ON_DEBUG_OUTPUT:
            breakpointName = AP_STR_DebugOutput;
            break;
#endif

        case AP_BREAK_ON_REDUNDANT_STATE_CHANGE:
            breakpointName = AP_STR_RedundantStateChangesBreakpoint;
            break;

        case AP_BREAK_ON_MEMORY_LEAK:
            breakpointName = AP_STR_MemoryLeakBreakpoint;
            break;

        default:
            breakpointName.makeEmpty();
            retVal = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGenericBreakpoint::breakpointTypeFromString
// Description: Get the breakpoint type from its string description
// Arguments:   breakpointTypeAsStr - the type as string
//              apGenericBreakpointType& breakType - output
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        10/7/2011
// ---------------------------------------------------------------------------
bool apGenericBreakpoint::breakpointTypeFromString(const gtString& breakpointTypeAsStr, apGenericBreakpointType& breakType)
{
    bool retVal = true;

    if (breakpointTypeAsStr == AP_STR_GLErrorBreakpoint)
    {
        breakType = AP_BREAK_ON_GL_ERROR;
    }
    else if (breakpointTypeAsStr == AP_STR_CLErrorBreakpoint)
    {
        breakType = AP_BREAK_ON_CL_ERROR;
    }
    else if (breakpointTypeAsStr == AP_STR_DetectedErrorBreakpoint)
    {
        breakType = AP_BREAK_ON_DETECTED_ERROR;
    }
    else if (breakpointTypeAsStr == AP_STR_DetectedErrorBreakpoint)
    {
        breakType = AP_BREAK_ON_DETECTED_ERROR;
    }
    else if (breakpointTypeAsStr == AP_STR_DeprecatedBreakpoint)
    {
        breakType = AP_BREAK_ON_DEPRECATED_FUNCTION;
    }

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    else if (breakpointTypeAsStr == AP_STR_SoftwareFallbackBreakpoint)
    {
        breakType = AP_BREAK_ON_SOFTWARE_FALLBACK;
    }

#endif
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    else if (breakpointTypeAsStr == AP_STR_DebugOutput)
    {
        breakType = AP_BREAK_ON_DEBUG_OUTPUT;
    }

#endif
    else if (breakpointTypeAsStr == AP_STR_RedundantStateChangesBreakpoint)
    {
        breakType = AP_BREAK_ON_REDUNDANT_STATE_CHANGE;
    }
    else if (breakpointTypeAsStr == AP_STR_MemoryLeakBreakpoint)
    {
        breakType = AP_BREAK_ON_MEMORY_LEAK;
    }

    else
    {
        retVal = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGenericBreakpoint::breakpointTypeFromBreakReason
// Description: Get the breakpoint type from the requested break reason
// Arguments:   apBreakReason breakReason
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        10/7/2011
// ---------------------------------------------------------------------------
apGenericBreakpointType apGenericBreakpoint::breakpointTypeFromBreakReason(apBreakReason breakReason)
{
    apGenericBreakpointType retVal = AP_BREAK_TYPE_UNKNOWN;

    switch (breakReason)
    {

        case AP_OPENGL_ERROR_BREAKPOINT_HIT:
            retVal = AP_BREAK_ON_GL_ERROR;
            break;

        case AP_OPENCL_ERROR_BREAKPOINT_HIT:
            retVal = AP_BREAK_ON_CL_ERROR;
            break;

        case AP_DETECTED_ERROR_BREAKPOINT_HIT:
            retVal = AP_BREAK_ON_DETECTED_ERROR;
            break;

        case AP_DEPRECATED_FUNCTION_BREAKPOINT_HIT:
            retVal = AP_BREAK_ON_DEPRECATED_FUNCTION;
            break;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case AP_SOFTWARE_FALLBACK_BREAKPOINT_HIT:
            retVal = AP_BREAK_ON_SOFTWARE_FALLBACK;
            break;
#endif

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

        case AP_GLDEBUG_OUTPUT_REPORT_BREAKPOINT_HIT:
            retVal = AP_BREAK_ON_DEBUG_OUTPUT;
            break;
#endif

        case AP_REDUNDANT_STATE_CHANGE_BREAKPOINT_HIT:
            retVal = AP_BREAK_ON_REDUNDANT_STATE_CHANGE;
            break;

        case AP_MEMORY_LEAK_BREAKPOINT_HIT:
            retVal = AP_BREAK_ON_MEMORY_LEAK;
            break;

        default:
            GT_ASSERT_EX(false, L"Should not get here");
            break;
    }

    return retVal;
}


