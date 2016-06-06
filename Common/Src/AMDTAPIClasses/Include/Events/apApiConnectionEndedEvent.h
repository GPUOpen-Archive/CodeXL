//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apApiConnectionEndedEvent.h
///
//==================================================================================

//------------------------------ apApiConnectionEndedEvent.h ------------------------------

#ifndef __APAPICONNECTIONENDEDEVENT_H
#define __APAPICONNECTIONENDEDEVENT_H


// Local:
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apApiConnectionEndedEvent
// General Description:
//   Is thrown when the API connection (with the spy that resides in the debugged
//   application) is stopped.
// Author:  AMD Developer Tools Team
// Creation Date:       13/01/2010
// ----------------------------------------------------------------------------------
class AP_API apApiConnectionEndedEvent : public apEvent
{
public:
    apApiConnectionEndedEvent(apAPIConnectionType apiConnectionType = AP_SPIES_UTILITIES_API_CONNECTION);
    virtual ~apApiConnectionEndedEvent();
    apAPIConnectionType connectionType() const { return _apiConnectionType;};

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    // The type of the API connection that has ended:
    apAPIConnectionType _apiConnectionType;
};

#endif //__APAPICONNECTIONENDEDEVENT_H

