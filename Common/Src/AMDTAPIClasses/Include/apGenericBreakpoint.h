//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGenericBreakpoint.h
///
//==================================================================================

//------------------------------ apGenericBreakpoint.h ------------------------------

#ifndef __APGENERICBREAKPOINT_H
#define __APGENERICBREAKPOINT_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apBreakPoint.h>
#include <AMDTAPIClasses/Include/apBreakReason.h>


// ----------------------------------------------------------------------------------
// Class Name:          apGenericBreakpointType
// General Description: This enumeration holds the types of generic breakpoints that
//                      can be set in CodeXL
// Author:  AMD Developer Tools Team
// Creation Date:        7/7/2011
// ----------------------------------------------------------------------------------
enum apGenericBreakpointType
{
    // Check-boxes
    AP_BREAK_TYPE_UNKNOWN = -1,
    AP_BREAK_ON_GL_ERROR = 0,
    AP_BREAK_ON_CL_ERROR = 1,
    AP_BREAK_ON_DETECTED_ERROR = 2,
    AP_BREAK_ON_REDUNDANT_STATE_CHANGE = 3,
    AP_BREAK_ON_DEPRECATED_FUNCTION = 4,
    AP_BREAK_ON_MEMORY_LEAK = 5,
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    AP_BREAK_ON_SOFTWARE_FALLBACK = 6,
    AP_LAST_BREAK_TYPE = AP_BREAK_ON_SOFTWARE_FALLBACK,
#elif (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    AP_BREAK_ON_DEBUG_OUTPUT = 6,
    AP_LAST_BREAK_TYPE = AP_BREAK_ON_DEBUG_OUTPUT,
#else
    AP_LAST_BREAK_TYPE = AP_BREAK_ON_MEMORY_LEAK,
#endif
    AP_AMOUNT_OF_GENERIC_BREAKPOINT_TYPES = AP_LAST_BREAK_TYPE + 1
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGenericBreakpoint
// General Description:
//   Represent a generic breakpoint - a breakpoint that is triggered once an event
//   occurs in the debugged process application
// Author:  AMD Developer Tools Team
// Creation Date:        6/7/2011
// ----------------------------------------------------------------------------------
class AP_API apGenericBreakpoint : public apBreakPoint
{
public:

    apGenericBreakpoint(apGenericBreakpointType breakType);
    apGenericBreakpoint();
    virtual ~apGenericBreakpoint() {};

    // Accessors:
    apGenericBreakpointType breakpointType() const {return _breakType;}
    void setBreakpointType(apGenericBreakpointType  breakType) {_breakType = breakType;}

    // As / From string:
    static bool breakpointTypeToString(apGenericBreakpointType breakType, gtString& breakpointName);
    static bool breakpointTypeFromString(const gtString& breakpointTypeAsStr, apGenericBreakpointType& breakType);

    // From break reason:
    static apGenericBreakpointType breakpointTypeFromBreakReason(apBreakReason breakReason);

    // Overrides apBreakPoint:
    virtual bool compareToOther(const apBreakPoint& otherBreakpoint) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

protected:

    // The generic breakpoint type:
    apGenericBreakpointType _breakType;
};

#endif //__APKERNELFUNCTIONNAMEBREAKPOINT_H

