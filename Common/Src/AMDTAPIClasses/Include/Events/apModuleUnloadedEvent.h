//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apModuleUnloadedEvent.h
///
//==================================================================================

//------------------------------ apModuleUnloadedEvent.h ------------------------------

#ifndef __APMODULEUNLOADEDEVENT_H
#define __APMODULEUNLOADEDEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apModuleUnloadedEvent
// General Description:
//   Represents the event of module (DLL / shared library / etc) unloaded from the
//   debugged process address space.
//
// Author:  AMD Developer Tools Team
// Creation Date:        30/3/2004
// ----------------------------------------------------------------------------------
class AP_API apModuleUnloadedEvent : public apEvent
{
public:
    apModuleUnloadedEvent(osThreadId triggeringThreadId, const gtString& modulePath);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Self functions:
    const gtString& modulePath() const { return _modulePath; };

private:
    friend class osTransferableObjectCreator<apModuleUnloadedEvent>;

    // Do not allow the use of the default constructor:
    apModuleUnloadedEvent();

private:
    // The unloaded module path:
    gtString _modulePath;
};


#endif //__APMODULEUNLOADEDEVENT_H

