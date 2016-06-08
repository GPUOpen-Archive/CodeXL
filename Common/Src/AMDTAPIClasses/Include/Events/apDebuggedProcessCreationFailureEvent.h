//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessCreationFailureEvent.h
///
//==================================================================================

//------------------------------ apDebuggedProcessCreationFailureEvent.h ------------------------------

#ifndef __APDEBUGGEDPROCESSCREATIONFAILUREEVENT
#define __APDEBUGGEDPROCESSCREATIONFAILUREEVENT

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra:
#include <AMDTOSWrappers/Include/osTime.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apDebuggedProcessCreationFailureEvent
// General Description:
//   Represents the event of the process creation failure.
// Author:  AMD Developer Tools Team
// Creation Date:        11/2/2010
// ----------------------------------------------------------------------------------
class AP_API apDebuggedProcessCreationFailureEvent : public apEvent
{
public:
    enum ProcessCreationFailureReason
    {
        COULD_NOT_CREATE_PROCESS,       // The system could not remotely create the process
        AUTOMATIC_CONFIGURATION_FAILED, // The automatic configuration failed
        REMOTE_HANDSHAKE_MISMATCH       // The handshake process between the client and the remote agent has failed
    };

public:
    apDebuggedProcessCreationFailureEvent(ProcessCreationFailureReason processCreationFailureReason, const gtString& createdProcessCommandLine, const gtString& createdProcessWorkDir, const gtString& processCreationError);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Self functions:
    const osTime& processCreationTime() const {return _processCreationTime;};
    const gtString& createdProcessCommandLine() const {return _createdProcessCommandLine;};
    const gtString& createdProcessWorkDir() const {return _createdProcessWorkDir;};
    const gtString& processCreationError() const {return _processCreationError;};
    ProcessCreationFailureReason processCreationFailureReason() const {return _failureReason;};

private:
    friend class osTransferableObjectCreator<apDebuggedProcessCreationFailureEvent>;

    // Do not allow the use of the default constructor:
    apDebuggedProcessCreationFailureEvent();

private:
    // The reason for the failure:
    ProcessCreationFailureReason _failureReason;

    // The process creation time:
    osTime _processCreationTime;

    // Process command line:
    gtString _createdProcessCommandLine;

    // Process working directory:
    gtString _createdProcessWorkDir;

    // The error string:
    gtString _processCreationError;
};


#endif  // __APDEBUGGEDPROCESSCREATIONFAILUREEVENT
