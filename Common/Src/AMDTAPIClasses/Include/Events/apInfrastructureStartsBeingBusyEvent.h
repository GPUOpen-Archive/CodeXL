//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apInfrastructureStartsBeingBusyEvent.h
///
//==================================================================================

//------------------------------ apInfrastructureStartsBeingBusyEvent.h ------------------------------

#ifndef __APINFRASTRUCTURESTARTSBEINGBUSYEVENT_H
#define __APINFRASTRUCTURESTARTSBEINGBUSYEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apInfrastructureStartsBeingBusyEvent
// General Description:
//   Is triggered when the infrastructure starts being busy.
//   (Enabling clients to display a wait dialog / etc).
//
// Author:  AMD Developer Tools Team
// Creation Date:        8/4/2004
// ----------------------------------------------------------------------------------
class AP_API apInfrastructureStartsBeingBusyEvent : public apEvent
{
public:
    // Describes the reason for the infrastructure being busy:
    enum BusyReason
    {
        AP_UPDATING_DEBUGGED_PROCESS_DATA
    };

    apInfrastructureStartsBeingBusyEvent(BusyReason busyReason) : _busyReason(busyReason) {};
    BusyReason busyReason() const { return _busyReason; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    friend class osTransferableObjectCreator<apInfrastructureStartsBeingBusyEvent>;

    // Do not allow the use of my default constructor:
    apInfrastructureStartsBeingBusyEvent();

private:
    // The reason for the infrastructure being busy:
    BusyReason _busyReason;
};



#endif //__APINFRASTRUCTURESTARTSBEINGBUSYEVENT_H

