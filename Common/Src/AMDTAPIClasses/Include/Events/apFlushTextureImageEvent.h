//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apFlushTextureImageEvent.h
///
//==================================================================================

//------------------------------ apFlushTextureImageEvent.h ------------------------------

#ifndef __APFLUSHTEXTUREIMAGEEVENT_H
#define __APFLUSHTEXTUREIMAGEEVENT_H

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apFlushTextureImageEvent : public apEvent
// General Description: Handle the flush texture image event
// Author:  AMD Developer Tools Team
// Creation Date:        28/2/2011
// ----------------------------------------------------------------------------------
class AP_API apFlushTextureImageEvent : public apEvent
{
public:
    apFlushTextureImageEvent();
    virtual ~apFlushTextureImageEvent();


    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Must be implemented by sub-classes:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;
};

#endif //__APFLUSHTEXTUREIMAGEEVENT_H
