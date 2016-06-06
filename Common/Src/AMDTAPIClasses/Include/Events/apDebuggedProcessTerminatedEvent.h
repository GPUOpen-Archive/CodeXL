//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessTerminatedEvent.h
///
//==================================================================================

//------------------------------ apDebuggedProcessTerminatedEvent.h ------------------------------

#ifndef __APDEBUGGEDPROCESSTERMINATEDEVENT
#define __APDEBUGGEDPROCESSTERMINATEDEVENT

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra:
#include <AMDTOSWrappers/Include/osTime.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apDebuggedProcessTerminatedEvent
// General Description:
//   Represents the event of the debugged process termination.
// Author:  AMD Developer Tools Team
// Creation Date:        30/3/2004
// ----------------------------------------------------------------------------------
class AP_API apDebuggedProcessTerminatedEvent : public apEvent
{
public:
    apDebuggedProcessTerminatedEvent(long exitCode);
    virtual ~apDebuggedProcessTerminatedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Self functions:
    long processExitCode() const { return _exitCode; };
    const osTime& processTerminationTime() const { return _processTerminationTime; };

private:
    friend class osTransferableObjectCreator<apDebuggedProcessTerminatedEvent>;

    // Do not allow the use of the default constructor:
    apDebuggedProcessTerminatedEvent();

private:
    // The process exit code:
    long _exitCode;

    // The process exit time:
    osTime _processTerminationTime;
};


#endif  // __APDEBUGGEDPROCESSTERMINATEDEVENT
