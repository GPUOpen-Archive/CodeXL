//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apUserWarningEvent.h
///
//==================================================================================

//------------------------------ apUserWarningEvent.h ------------------------------

#ifndef __APUSERWARNINGEVENT_H
#define __APUSERWARNINGEVENT_H


// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apUserWarningEvent : public apEvent
// General Description: The class is used when CodeXL needs to output a message for
//                      the user in the output window
// Author:  AMD Developer Tools Team
// Creation Date:       30/10/2011
// ----------------------------------------------------------------------------------
class AP_API apUserWarningEvent : public apEvent
{
public:

    apUserWarningEvent(const gtString& userWarningString);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Self functions:
    const gtString& userWarningString() const { return _userWarningString; };

private:

    friend class osTransferableObjectCreator<apUserWarningEvent>;

    // Do not allow the use of the default constructor:
    apUserWarningEvent();

private:
    // The outputted warning string:
    gtString _userWarningString;
};


#endif //__APUSERWARNINGEVENT_H

