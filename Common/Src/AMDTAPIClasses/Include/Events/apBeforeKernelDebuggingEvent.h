//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apBeforeKernelDebuggingEvent.h
///
//==================================================================================

//------------------------------ apBeforeKernelDebuggingEvent.h ------------------------------

#ifndef __APBEFOREKERNELDEBUGGINGEVENT_H
#define __APBEFOREKERNELDEBUGGINGEVENT_H

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apBeforeKernelDebuggingEvent : public apEvent
// General Description: An event invoked by the spy before it enters kernel debugging
//                      mode. This event should be followed by triggering a breakpoint.
// Author:  AMD Developer Tools Team
// Creation Date:       28/10/2010
// ----------------------------------------------------------------------------------
class AP_API apBeforeKernelDebuggingEvent : public apEvent
{
public:
    enum apKernelDebuggingType
    {
        AP_OPENCL_SOFTWARE_KERNEL_DEBUGGING,
        AP_HSA_HARDWARE_KERNEL_DEBUGGING,
    };

public:
    apBeforeKernelDebuggingEvent(apKernelDebuggingType debuggingType = AP_OPENCL_SOFTWARE_KERNEL_DEBUGGING, osThreadId triggeringThreadID = OS_NO_THREAD_ID, unsigned int workDimension = 0, const gtSize_t* globalWorkOffset = NULL, const gtSize_t* globalWorkSize = NULL, const gtSize_t* localWorkSize = NULL);
    ~apBeforeKernelDebuggingEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    apKernelDebuggingType debuggingType() const { return m_debuggingType; };

    gtSize_t globalWorkOffsetX() const {return _globalWorkOffset[0];};
    gtSize_t globalWorkOffsetY() const {return _globalWorkOffset[1];};
    gtSize_t globalWorkOffsetZ() const {return _globalWorkOffset[2];};
    gtSize_t globalWorkSizeX() const {return _globalWorkSize[0];};
    gtSize_t globalWorkSizeY() const {return _globalWorkSize[1];};
    gtSize_t globalWorkSizeZ() const {return _globalWorkSize[2];};
    gtSize_t localWorkSizeX() const {return _localWorkSize[0];};
    gtSize_t localWorkSizeY() const {return _localWorkSize[1];};
    gtSize_t localWorkSizeZ() const {return _localWorkSize[2];};

private:
    apKernelDebuggingType m_debuggingType;
    gtSize_t _globalWorkOffset[3];
    gtSize_t _globalWorkSize[3];
    gtSize_t _localWorkSize[3];
};

#endif //__APBEFOREKERNELDEBUGGINGEVENT_H

