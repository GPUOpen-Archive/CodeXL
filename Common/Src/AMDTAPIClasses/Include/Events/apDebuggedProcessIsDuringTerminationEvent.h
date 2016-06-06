//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDebuggedProcessIsDuringTerminationEvent.h
///
//==================================================================================

//------------------------------ apDebuggedProcessIsDuringTerminationEvent.h ------------------------------

#ifndef __APDEBUGGEDPROCESSISDURINGTERMINATIONEVENT_H
#define __APDEBUGGEDPROCESSISDURINGTERMINATIONEVENT_H

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apDebuggedProcessIsDuringTerminationEvent : public apEvent
// General Description: An event thrown when the debugged process is during process termination
// Author:  AMD Developer Tools Team
// Creation Date:       19/7/2010
// ----------------------------------------------------------------------------------
class AP_API apDebuggedProcessIsDuringTerminationEvent : public apEvent
{
public:
    apDebuggedProcessIsDuringTerminationEvent();
    ~apDebuggedProcessIsDuringTerminationEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    friend class osTransferableObjectCreator<apDebuggedProcessIsDuringTerminationEvent>;
};


#endif //__APDEBUGGEDPROCESSISDURINGTERMINATIONEVENT_H

