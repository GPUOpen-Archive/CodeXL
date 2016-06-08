//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apInfrastructureFailureEvent.h
///
//==================================================================================

//------------------------------ apInfrastructureFailureEvent.h ------------------------------

#ifndef __APINFRASTRUCTUREFAILUREEVENT_H
#define __APINFRASTRUCTUREFAILUREEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apInfrastructureFailureEvent
// General Description:
//   Is triggered when the infrastructure when a an infrastructure failure happens.
//   This enables the infrastructure notify the application about important failures.
//
// Author:  AMD Developer Tools Team
// Creation Date:        5/8/2008
// ----------------------------------------------------------------------------------
class AP_API apInfrastructureFailureEvent : public apEvent
{
public:
    enum FailureReason
    {
        FAILED_TO_INITIALIZE_GDB    // The gdb debugger initialization failed.
    };

public:
    apInfrastructureFailureEvent(FailureReason failureReason) { _failureReason = failureReason; };
    FailureReason failureReason() const { return _failureReason; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    friend class osTransferableObjectCreator<apInfrastructureFailureEvent>;

    // Do not allow the use of my default constructor:
    apInfrastructureFailureEvent();

private:
    // The infrastructure failure reason:
    FailureReason _failureReason;
};


#endif //__APINFRASTRUCTUREFAILUREEVENT_H

