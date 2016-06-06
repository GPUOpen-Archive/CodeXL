//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMDIViewActivatedEvent.h
///
//==================================================================================

//------------------------------ apMDIViewActivatedEvent.h ------------------------------

#ifndef __APMDIVIEWACTIVATEDEVENT_H
#define __APMDIVIEWACTIVATEDEVENT_H

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:           apMDIViewActivatedEvent : public apEvent
// General Description:  This class is used for MDI view open process. The event is
//                       registered whenever somewhere in the application there is a
//                       need for display of MDI view object
//                       This event should not be used in direct, but should be inherited
//                       and used for specific objects display
// Author:  AMD Developer Tools Team
// Creation Date:        25/8/2013
// ----------------------------------------------------------------------------------
class AP_API apMDIViewActivatedEvent : public apEvent
{
public:
    apMDIViewActivatedEvent(const osFilePath& filePath);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Self functions:
    const osFilePath& filePath() const { return m_filePath; };

protected:

    // Do not allow the use of the default constructor:
    apMDIViewActivatedEvent();

protected:

    // The displayed file path:
    osFilePath m_filePath;
};

#endif //__APMDIVIEWACTIVATEDEVENT_H

