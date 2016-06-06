//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apRenderContextCreatedEvent.h
///
//==================================================================================

//------------------------------ apRenderContextCreatedEvent.h ------------------------------

#ifndef __APRENDERCONTEXTCREATEDEVENT_H
#define __APRENDERCONTEXTCREATEDEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apRenderContextCreatedEvent
// General Description:
//   Is thrown when an OpenGL render context is created.
// Author:  AMD Developer Tools Team
// Creation Date:        25/12/2009
// ----------------------------------------------------------------------------------
class AP_API apRenderContextCreatedEvent : public apEvent
{
public:
    apRenderContextCreatedEvent(osThreadId triggeringThreadId, int contextId);
    virtual ~apRenderContextCreatedEvent();

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
    friend class osTransferableObjectCreator<apRenderContextCreatedEvent>;
    apRenderContextCreatedEvent();

private:
    // The OpenGL server context id:
    int _contextId;
};


#endif //__APRENDERCONTEXTCREATEDEVENT_H

