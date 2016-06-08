//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apThreadTerminatedEvent.h
///
//==================================================================================

//------------------------------ apThreadTerminatedEvent.h ------------------------------

#ifndef __APTHREADTERMINATEDEVENT
#define __APTHREADTERMINATEDEVENT

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra
#include <AMDTOSWrappers/Include/osTime.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apThreadTerminatedEvent
// General Description:
//   Is triggered when a thread is created within the debugged process.
// Author:  AMD Developer Tools Team
// Creation Date:        08/5/2005
// ----------------------------------------------------------------------------------
class AP_API apThreadTerminatedEvent : public apEvent
{
public:
    apThreadTerminatedEvent(const osThreadId& threadOSId, long threadExitCode,
                            const osTime& threadTerminationTime);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Self functions:
    const osThreadId& threadOSId() const { return _threadOSId; };
    long threadExitCode() const { return _threadExitCode; };
    const osTime& threadTerminationTime() const { return _threadTerminationTime; };

private:
    friend class osTransferableObjectCreator<apThreadTerminatedEvent>;

    // Do not allow the use of the default constructor:
    apThreadTerminatedEvent();

private:
    // The thread OS id:
    osThreadId _threadOSId;

    // The thread exit code:
    long _threadExitCode;

    // The thread exit time:
    osTime _threadTerminationTime;
};


#endif  // __APTHREADTERMINATEDEVENT
