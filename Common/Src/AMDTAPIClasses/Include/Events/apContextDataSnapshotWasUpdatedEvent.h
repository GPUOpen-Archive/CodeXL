//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apContextDataSnapshotWasUpdatedEvent.h
///
//==================================================================================

//------------------------------ apContextDataSnapshotWasUpdatedEvent.h ------------------------------

#ifndef __APCONTEXTDATASNAPSHOTWASUPDATEDEVENT_H
#define __APCONTEXTDATASNAPSHOTWASUPDATEDEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apContextDataSnapshotWasUpdatedEvent : public apEvent
// General Description: An event thrown when the debugged process is during process termination
// Author:  AMD Developer Tools Team
// Creation Date:       19/7/2010
// ----------------------------------------------------------------------------------
class AP_API apContextDataSnapshotWasUpdatedEvent : public apEvent
{
public:
    apContextDataSnapshotWasUpdatedEvent(apContextID updatedContextID, bool isContextBeingDeleted);
    ~apContextDataSnapshotWasUpdatedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    bool isContextBeingDeleted() const {return _isContextBeingDeleted;};
    apContextID updatedContextID() const {return _updatedContextID;};

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    friend class osTransferableObjectCreator<apContextDataSnapshotWasUpdatedEvent>;

    // The updated context ID:
    apContextID _updatedContextID;

    // Is context about to be deleted:
    bool _isContextBeingDeleted;

    // Do not allow the use of my default constructor:
    apContextDataSnapshotWasUpdatedEvent() {};
};

#endif //__APCONTEXTDATASNAPSHOTWASUPDATEDEVENT_H

//------------------------------ apContextDataSnapshotWasUpdatedEvent.h ------------------------------

#ifndef __APCONTEXTDATASNAPSHOTWASUPDATEDEVENT_H
#define __APCONTEXTDATASNAPSHOTWASUPDATEDEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apContextDataSnapshotWasUpdatedEvent : public apEvent
// General Description: An event thrown when the debugged process is during process termination
// Author:  AMD Developer Tools Team
// Creation Date:       19/7/2010
// ----------------------------------------------------------------------------------
class AP_API apContextDataSnapshotWasUpdatedEvent : public apEvent
{
public:
    apContextDataSnapshotWasUpdatedEvent(apContextID updatedContextID, bool isContextBeingDeleted);
    ~apContextDataSnapshotWasUpdatedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    bool isContextBeingDeleted() const {return _isContextBeingDeleted;};
    apContextID updatedContextID() const {return _updatedContextID;};

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    friend class osTransferableObjectCreator<apContextDataSnapshotWasUpdatedEvent>;

    // The updated context ID:
    apContextID _updatedContextID;

    // Is context about to be deleted:
    bool _isContextBeingDeleted;

    // Do not allow the use of my default constructor:
    apContextDataSnapshotWasUpdatedEvent() {};
};

#endif //__APCONTEXTDATASNAPSHOTWASUPDATEDEVENT_H

