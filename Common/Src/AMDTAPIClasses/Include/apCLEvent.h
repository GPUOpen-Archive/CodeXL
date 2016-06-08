//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLEvent.h
///
//==================================================================================

//------------------------------ apCLEvent.h ------------------------------

#ifndef __APCLEVENT_H
#define __APCLEVENT_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAllocatedObject.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLEvent : public apAllocatedObject
// General Description: Represents a OpenCL event, a handle to a command in a command
//                      queue.
//
//
// Author:  AMD Developer Tools Team
// Creation Date:       21/1/2010
// ----------------------------------------------------------------------------------
class AP_API apCLEvent : public apAllocatedObject
{
public:
    apCLEvent(oaCLEventHandle eventHandle, oaCLCommandQueueHandle queueHandle, bool retainedBySpy);
    virtual ~apCLEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLEventHandle eventHandle() const {return _eventHandle;};
    oaCLCommandQueueHandle controllingQueueHandle() const {return _controllingQueueHandle;};
    cl_command_type commandType() const {return _commandType;};
    void setCommandType(cl_command_type cmdType) {_commandType = cmdType;};
    unsigned int referenceCount() const {return _referenceCount;};
    unsigned int displayReferenceCount() const;
    void setReferenceCount(unsigned int refCount) {_referenceCount = refCount;};
    bool isRetainedBySpy() const {return _isRetainedBySpy;};
    void setRetainedBySpy(bool retained) {_isRetainedBySpy = retained;};

    // cl_gremedy_object_naming:
    const gtString& eventName() const {return _eventName;};
    void setEventName(const gtString& name) {_eventName = name;};

private:
    // Disallow use of my default constructor:
    apCLEvent();

private:
    // The event handle:
    oaCLEventHandle _eventHandle;

    // The queue holding this event:
    oaCLCommandQueueHandle _controllingQueueHandle;

    // The command type:
    cl_command_type _commandType;

    // The reference count:
    unsigned int _referenceCount;

    // Did we add one to this event's reference count to retain it?
    bool _isRetainedBySpy;

    // cl_gremedy_object_naming:
    gtString _eventName;
};

#endif //__APCLEVENT_H

