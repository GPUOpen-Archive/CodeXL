//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apModuleLoadedEvent.h
///
//==================================================================================

//------------------------------ apModuleLoadedEvent.h ------------------------------

#ifndef __APMODULELOADEDEVENT_H
#define __APMODULELOADEDEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apModuleLoadedEvent
// General Description:
//   Represents the event of module (DLL / shared library / etc) loaded into the
//   debugged process address space.
//
// Author:  AMD Developer Tools Team
// Creation Date:        30/3/2004
// ----------------------------------------------------------------------------------
class AP_API apModuleLoadedEvent : public apEvent
{
public:
    apModuleLoadedEvent(osThreadId triggeringThreadId, const gtString& modulePath, osInstructionPointer loadAddress);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Self functions:
    const gtString& modulePath() const { return _modulePath; };
    osInstructionPointer moduleLoadAddress() const { return _moduleLoadAddress; };
    bool areDebugSymbolsLoaded() const { return _areDebugSymbolsLoaded; };

    void setDebugSymbolsLoadingStatus(bool areSymbolsLoaded) { _areDebugSymbolsLoaded = areSymbolsLoaded; };

private:
    friend class osTransferableObjectCreator<apModuleLoadedEvent>;

    // Do not allow the use of the default constructor:
    apModuleLoadedEvent();

private:
    // The loaded module path:
    gtString _modulePath;

    // The loaded address of the module in debugged process address space:
    // ("base address" in Win32 terminology).
    osInstructionPointer _moduleLoadAddress;

    // Contains true iff the module's debug symbols were loaded:
    bool _areDebugSymbolsLoaded;
};


#endif //__APMODULELOADEDEVENT_H

