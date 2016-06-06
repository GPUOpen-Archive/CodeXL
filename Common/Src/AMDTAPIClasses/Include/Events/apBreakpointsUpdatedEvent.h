//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apBreakpointsUpdatedEvent.h
///
//==================================================================================

//------------------------------ apBreakpointsUpdatedEvent.h ------------------------------

#ifndef __APBREAKPOINTSUPDATEDEVENT_H
#define __APBREAKPOINTSUPDATEDEVENT_H

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apBreakpointsUpdatedEvent : public apEvent
// General Description: Sent when a breakpoints is updated
// Author:  AMD Developer Tools Team
// Creation Date:       7/9/2011
// ----------------------------------------------------------------------------------
class AP_API apBreakpointsUpdatedEvent : public apEvent
{

public:
    apBreakpointsUpdatedEvent(int updatedBreakpointIndex);
    virtual ~apBreakpointsUpdatedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Accessors:
    int updatedBreakpointIndex() const {return _updatedBreakpointIndex;}

protected:

    // The index of the updated breakpoint, or -1 if all breakpoints are updated:
    int _updatedBreakpointIndex;

};


#endif //__APBREAKPOINTSUPDATEDEVENT_H

