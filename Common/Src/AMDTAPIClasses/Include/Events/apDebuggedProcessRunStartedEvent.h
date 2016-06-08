//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessRunStartedEvent.h
///
//==================================================================================

//------------------------------ apDebuggedProcessRunStartedEvent.h ------------------------------

#ifndef __APDEBUGGEDPROCESSRUNSTARTEDEVENT
#define __APDEBUGGEDPROCESSRUNSTARTEDEVENT

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osTime.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apDebuggedProcessRunStartedEvent : public apEvent
// General Description:
//   Is triggered when the debugged process main thread reach its entry point.
//   This appends after:
//   a. The process was created.
//   b. The static linked modules (dlls) were loaded.
//
//  *** NOTICE ***
//  This event is NOT ALWAYS thrown. Example: When debugging .NET applications,
//  the .NET runtime changes the main thread context before it reaches its entry point,
//  causing it never to reach its entry point!
//  (See Case 450)
//
// Author:  AMD Developer Tools Team
// Creation Date:        22/6/2004
// ----------------------------------------------------------------------------------
class AP_API apDebuggedProcessRunStartedEvent : public apEvent
{
public:
    apDebuggedProcessRunStartedEvent(osProcessId processId, const osTime& processRunStartedTime);
    ~apDebuggedProcessRunStartedEvent();

    osProcessId processId() const { return _processId; };
    const osTime& processRunStartedTime() const { return _processRunStartedTime; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    friend class osTransferableObjectCreator<apDebuggedProcessRunStartedEvent>;

    // Do not allow the use of the default constructor:
    apDebuggedProcessRunStartedEvent();

private:
    // The OS id of the process that started running:
    osProcessId _processId;

    // The time in which the process run started:
    osTime _processRunStartedTime;
};


#endif  // __APDEBUGGEDPROCESSRUNSTARTEDEVENT
