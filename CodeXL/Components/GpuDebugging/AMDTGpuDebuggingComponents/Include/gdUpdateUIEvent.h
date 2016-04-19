//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdUpdateUIEvent.h
///
//==================================================================================

//------------------------------ gdUpdateUIEvent.h ------------------------------

#ifndef __GDUPDATEUIEVENT
#define __GDUPDATEUIEVENT

// Infra:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdUpdateUIEvent
// General Description:  Is triggered when an application update UI should happen
// Author:               Sigal Algranaty
// Creation Date:        10/1/11
// ----------------------------------------------------------------------------------
class GD_API gdUpdateUIEvent : public apEvent
{
public:

    gdUpdateUIEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

};


#endif  // __GDUPDATEUIEVENT
