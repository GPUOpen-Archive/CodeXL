//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apComputeContextDeletedEvent.h
///
//==================================================================================

//------------------------------ apComputeContextDeletedEvent.h ------------------------------

#ifndef __APCOMPUTECONTEXTDELETEDEVENT_H
#define __APCOMPUTECONTEXTDELETEDEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apComputeContextDeletedEvent
// General Description:
//   Is thrown when an OpenCL compute context is deleted.
// Author:  AMD Developer Tools Team
// Creation Date:        17/1/2010
// ----------------------------------------------------------------------------------
class AP_API apComputeContextDeletedEvent : public apEvent
{
public:
    apComputeContextDeletedEvent(osThreadId triggeringThreadId, int contextId);
    virtual ~apComputeContextDeletedEvent();

    int contextId() const { return _contextId; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    // Only the transferable object creator should be able to call my default constructor:
    friend class osTransferableObjectCreator<apComputeContextDeletedEvent>;
    apComputeContextDeletedEvent();

private:
    // The OpenGL server context id:
    int _contextId;
};


#endif //__APCOMPUTECONTEXTDELETEDEVENT_H

