//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apInfrastructureEndsBeingBusyEvent.h
///
//==================================================================================

//------------------------------ apInfrastructureEndsBeingBusyEvent.h ------------------------------

#ifndef __APINFRASTRUCTUREENDSBEINGBUSYEVENT_H
#define __APINFRASTRUCTUREENDSBEINGBUSYEVENT_H

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apInfrastructureEndsBeingBusyEvent
// General Description:
//   Is triggered when the infrastructure ends being busy.
//   (See apInfrastructureStartsBeingBusyEvent for more details).
//
// Author:  AMD Developer Tools Team
// Creation Date:        8/4/2004
// ----------------------------------------------------------------------------------
class AP_API apInfrastructureEndsBeingBusyEvent : public apEvent
{
public:
    apInfrastructureEndsBeingBusyEvent() {};

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;
};

#endif //__APINFRASTRUCTUREENDSBEINGBUSYEVENT_H
