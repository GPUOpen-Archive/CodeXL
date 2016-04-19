//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afGlobalVariableChangedEvent.h
///
//==================================================================================

#ifndef __GDCodeXLGLOBALVARIABLECHANGEDEVENT
#define __GDCodeXLGLOBALVARIABLECHANGEDEVENT

// Infra:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           afGlobalVariableChangedEvent
// General Description:
//  Is triggered when an application global variable changes its value.
// Author:               Yaki Tebeka
// Creation Date:        24/5/2004
// ----------------------------------------------------------------------------------
class AF_API afGlobalVariableChangedEvent : public apEvent
{
public:
    enum GlobalVariableId
    {
        CHOSEN_CONTEXT_ID,
        CHOSEN_THREAD_INDEX,
        CURRENT_PROJECT
    };

    afGlobalVariableChangedEvent(GlobalVariableId variableId);
    GlobalVariableId changedVariableId() const { return _changedVariableId; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

private:
    // Do not allow the use of my default constructor:
    afGlobalVariableChangedEvent();

private:
    // Contains the id of the global variable that was changed.
    GlobalVariableId _changedVariableId;
};


#endif  // __GDCodeXLGLOBALVARIABLECHANGEDEVENT
