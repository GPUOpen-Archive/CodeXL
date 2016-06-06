//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLQueueCreatedEvent.h
///
//==================================================================================

//------------------------------ apOpenCLQueueCreatedEvent.h ------------------------------

#ifndef __apOpenCLQueueCreatedEvent_H
#define __apOpenCLQueueCreatedEvent_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apOpenCLQueueCreatedEvent
// General Description:
//   Is thrown when an OpenCL queue is deleted.
// Author:  AMD Developer Tools Team
// Creation Date:        2/3/2010
// ----------------------------------------------------------------------------------
class AP_API apOpenCLQueueCreatedEvent : public apEvent
{
public:
    apOpenCLQueueCreatedEvent(osThreadId triggeringThreadId, int contextID, int queueID);
    virtual ~apOpenCLQueueCreatedEvent();

    int contextID() const {return _contextID;};
    int queueID() const {return _queueID;};

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    // Only the transferable object creator should be able to call my default constructor:
    friend class osTransferableObjectCreator<apOpenCLQueueCreatedEvent>;
    apOpenCLQueueCreatedEvent();

private:

    // The OpenCL context id:
    int _contextID;

    // The OpenCL queue id:
    int _queueID;
};


#endif //__apOpenCLQueueCreatedEvent_H

