//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMemoryAllocationFailureEvent.h
///
//==================================================================================

//------------------------------ apMemoryAllocationFailureEvent.h ------------------------------

#ifndef __APMEMORYALLOCATIONFAILUREEVENT_H
#define __APMEMORYALLOCATIONFAILUREEVENT_H

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTOSWrappers/Include/osCallStack.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apMemoryAllocationFailureEvent
// General Description: This event is sent when debugged application fails to allocate memory
//                      it sends the call stack of the failed allocation place
// Author:  AMD Developer Tools Team
// Creation Date:       1/5/2015
// ----------------------------------------------------------------------------------
class AP_API apMemoryAllocationFailureEvent : public apEvent
{
public:
    apMemoryAllocationFailureEvent(osCallStack callStack);
    apMemoryAllocationFailureEvent();
    ~apMemoryAllocationFailureEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    osCallStack& callStack() { return m_callStack; }

private:
    /// Call stack where the allocation happened.
    osCallStack m_callStack;
};

#endif //__APMEMORYALLOCATIONFAILUREEVENT_H

