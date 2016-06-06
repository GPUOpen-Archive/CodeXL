//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apBreakpointHitEvent.h
///
//==================================================================================

//------------------------------ apBreakpointHitEvent.h ------------------------------

#ifndef __APBREAKPOINTHITEVENT
#define __APBREAKPOINTHITEVENT

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apBreakReason.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apBreakpointHitEvent
// General Description:
//   Is triggered when the debugged process hits a breakpoint.
// Author:  AMD Developer Tools Team
// Creation Date:        30/3/2004
// ----------------------------------------------------------------------------------
class AP_API apBreakpointHitEvent : public apEvent
{
public:
    apBreakpointHitEvent(osThreadId triggeringThreadId, osInstructionPointer breakPointAddress);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Self functions:
    apBreakReason breakReason() const { return _breakReason; };
    const apFunctionCall* breakedOnFunctionCall() const { return _aptrBreakedOnFunctionCall.pointedObject(); };
    osInstructionPointer breakPointAddress() const { return _breakPointAddress; };
    GLenum openGLErrorCode() const { return _openGLErrorCode; };

    void setBreakReason(apBreakReason breakReason) { _breakReason = breakReason; };
    void setBreakedOnFunctionCall(gtAutoPtr<apFunctionCall>& aptrBreakedOnFunctionCall) { _aptrBreakedOnFunctionCall = aptrBreakedOnFunctionCall; };
    void setBreakPointAddress(osInstructionPointer breakAddress) { _breakPointAddress = breakAddress; };
    void setOpenGLErrorCode(GLenum errorCode) { _openGLErrorCode = errorCode; };

private:
    // Do not allow the use of the default constructor:
    apBreakpointHitEvent();

    friend class osTransferableObjectCreator<apBreakpointHitEvent>;

private:
    // The break reason:
    apBreakReason _breakReason;

    // The details of the function that triggered the breakpoint event:
    gtAutoPtr<apFunctionCall> _aptrBreakedOnFunctionCall;

    // The address in which the breakpoint occur
    // (In debugged process address space)
    osInstructionPointer _breakPointAddress;

    // OpenGL error code (If the breakpoint was caused by an OpenGL error):
    GLenum _openGLErrorCode;
};


#endif  // __APBREAKPOINTHITEVENT
