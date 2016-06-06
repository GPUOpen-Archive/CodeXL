//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apRenderContextDeletedEvent.h
///
//==================================================================================

//------------------------------ apRenderContextDeletedEvent.h ------------------------------

#ifndef __APRENDERCONTEXTDELETEDEVENT_H
#define __APRENDERCONTEXTDELETEDEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apRenderContextDeletedEvent
// General Description:
//   Is thrown when an OpenGL render context is deleted.
// Author:  AMD Developer Tools Team
// Creation Date:        25/12/2009
// ----------------------------------------------------------------------------------
class AP_API apRenderContextDeletedEvent : public apEvent
{
public:
    apRenderContextDeletedEvent(osThreadId triggeringThreadId, int contextId);
    virtual ~apRenderContextDeletedEvent();

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
    friend class osTransferableObjectCreator<apRenderContextDeletedEvent>;
    apRenderContextDeletedEvent();

private:
    // The OpenGL server context id:
    int _contextId;
};


#endif //__APRENDERCONTEXTDELETEDEVENT_H

