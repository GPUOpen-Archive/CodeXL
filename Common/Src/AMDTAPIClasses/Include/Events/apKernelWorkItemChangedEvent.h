//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apKernelWorkItemChangedEvent.h
///
//==================================================================================

//------------------------------ apKernelWorkItemChangedEvent.h ------------------------------

#ifndef __APKERNELWORKITEMCHANGEDEVENT_H
#define __APKERNELWORKITEMCHANGEDEVENT_H

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apKernelWorkItemChangedEvent : public apEvent
// General Description: An event invoked by the persistent data manager. The event
//                      is notifying the application with the change of the current kernel
//                      work item.
// Author:  AMD Developer Tools Team
// Creation Date:       13/3/2011
// ----------------------------------------------------------------------------------
class AP_API apKernelWorkItemChangedEvent : public apEvent
{
public:
    apKernelWorkItemChangedEvent(int coordinate, int workItemValue);
    ~apKernelWorkItemChangedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    int corrdinate() const {return _coordinate;};
    int workItemValue() const {return _workItemValue;};

private:

    int _coordinate;
    int _workItemValue;

};

#endif //__APKERNELWORKITEMCHANGEDEVENT_H

