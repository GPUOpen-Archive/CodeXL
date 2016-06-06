//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessCreatedEvent.h
///
//==================================================================================

//------------------------------ apDebuggedProcessCreatedEvent.h ------------------------------

#ifndef __APDEBUGGEDPROCESSCREATEDEVENT
#define __APDEBUGGEDPROCESSCREATEDEVENT

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apDebuggedProcessCreatedEvent
// General Description:
//   Represents the event of the debugged process creation.
//   (The debugged process was launched by the debugger).
// Author:  AMD Developer Tools Team
// Creation Date:        30/3/2004
// ----------------------------------------------------------------------------------
class AP_API apDebuggedProcessCreatedEvent : public apEvent
{
public:
    apDebuggedProcessCreatedEvent(const apDebugProjectSettings& processCreationData, const osTime& processCreationTime, osInstructionPointer loadedAddress);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Self functions:
    const apDebugProjectSettings& processCreationData() const { return _processCreationData; };
    const osTime& processCreationTime() const { return _processCreationTime; };
    osInstructionPointer loadedAddress() const { return _loadedAddress; };
    bool areDebugSymbolsLoaded() const { return _areDebugSymbolsLoaded; };

    void setDebugSymbolsLoadingStatus(bool areSymbolsLoaded) { _areDebugSymbolsLoaded = areSymbolsLoaded; };

private:
    friend class osTransferableObjectCreator<apDebuggedProcessCreatedEvent>;

    // Do not allow the use of the default constructor:
    apDebuggedProcessCreatedEvent();

private:
    // The process creation data:
    apDebugProjectSettings _processCreationData;

    // The process creation time:
    osTime _processCreationTime;

    // The loaded address of the process exe module (in debugged process address space):
    // This address is sometimes called "base address" in Win32 terminology.
    osInstructionPointer _loadedAddress;

    // Contains true iff the dlls debug symbols were loaded successfully:
    bool _areDebugSymbolsLoaded;
};


#endif  // __APDEBUGGEDPROCESSCREATEDEVENT
