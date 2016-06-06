//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMonitoredObjectsTreeEvent.h
///
//==================================================================================

//------------------------------ apMonitoredObjectsTreeEvent.h -------------------

#ifndef __APMONITOREDOBJECTSTREEEVENT_H
#define __APMONITOREDOBJECTSTREEEVENT_H

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:           apMonitoredObjectsTreeSelectedEvent
// General Description:  Is used to notify monitored objects selection change
// Author:  AMD Developer Tools Team
// Creation Date:        12/10/2010
// ----------------------------------------------------------------------------------
class AP_API apMonitoredObjectsTreeSelectedEvent : public apEvent
{
public:
    apMonitoredObjectsTreeSelectedEvent(const void* pTreeItemData);
    ~apMonitoredObjectsTreeSelectedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    const void* selectedItemData() const {return _pTreeItemData;};

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    const void* _pTreeItemData;

};

// ----------------------------------------------------------------------------------
// Class Name:           apMonitoredObjectsTreeActivatedEvent
// General Description:  Is used to notify monitored objects activation
// Author:  AMD Developer Tools Team
// Creation Date:        12/10/2010
// ----------------------------------------------------------------------------------
class AP_API apMonitoredObjectsTreeActivatedEvent : public apEvent
{
public:
    apMonitoredObjectsTreeActivatedEvent(const void* pTreeItemData);
    ~apMonitoredObjectsTreeActivatedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    const void* selectedItemData() const {return _pTreeItemData;};

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    const void* _pTreeItemData;

};

#endif //__APMONITOREDOBJECTSTREEEVENT_H

