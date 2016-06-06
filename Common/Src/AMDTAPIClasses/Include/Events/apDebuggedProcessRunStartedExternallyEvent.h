//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessRunStartedExternallyEvent.h
///
//==================================================================================

//------------------------------ apDebuggedProcessRunStartedExternallyEvent.h ------------------------------

#ifndef __APDEBUGGEDPROCESSRUNSTARTEDEXTERNALLYEVENT_H
#define __APDEBUGGEDPROCESSRUNSTARTEDEXTERNALLYEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apDebuggedProcessRunStartedExternallyEvent : public apEvent
// General Description: An event thrown when the debugged process is started by a party
//                      other than CodeXL (Such as Visual Studio in the CodeXL visual
//                      studio integration).
//                      this event should NEVER be thrown when CodeXL is run separately
// Author:  AMD Developer Tools Team
// Creation Date:       19/4/2009
// ----------------------------------------------------------------------------------
class AP_API apDebuggedProcessRunStartedExternallyEvent : public apEvent
{
public:
    apDebuggedProcessRunStartedExternallyEvent(const apDebugProjectSettings& creationData);
    ~apDebuggedProcessRunStartedExternallyEvent();

    const apDebugProjectSettings& getProcessCreationData() const {return m_processCreationData;};

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    friend class osTransferableObjectCreator<apDebuggedProcessRunStartedExternallyEvent>;

    // Disallow use of my default constructor:
    apDebuggedProcessRunStartedExternallyEvent();

private:
    apDebugProjectSettings m_processCreationData;
};

#endif //__APDEBUGGEDPROCESSRUNSTARTEDEXTERNALLYEVENT_H

