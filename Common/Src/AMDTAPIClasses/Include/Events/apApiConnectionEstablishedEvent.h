//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apApiConnectionEstablishedEvent.h
///
//==================================================================================

//------------------------------ apApiConnectionEstablishedEvent.h ------------------------------

#ifndef __APAPICONNECTIONESTABLISHEDEVENT
#define __APAPICONNECTIONESTABLISHEDEVENT

// Local:
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apApiConnectionEstablishedEvent
// General Description:
//   Is thrown when the API connection (with the spy that resides in the debugged
//   application) is established.
// Author:  AMD Developer Tools Team
// Creation Date:        4/10/2004
// ----------------------------------------------------------------------------------
class AP_API apApiConnectionEstablishedEvent : public apEvent
{
public:
    apApiConnectionEstablishedEvent(apAPIConnectionType APIConnectionType = AP_SPIES_UTILITIES_API_CONNECTION);
    virtual ~apApiConnectionEstablishedEvent();
    apAPIConnectionType establishedConnectionType() const { return _APIConnectionType; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    // The type of the API connection that was established:
    apAPIConnectionType _APIConnectionType;
};


#endif  // __APAPICONNECTIONESTABLISHEDEVENT
