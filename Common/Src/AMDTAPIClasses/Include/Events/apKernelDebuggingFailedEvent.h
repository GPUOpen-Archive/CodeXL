//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apKernelDebuggingFailedEvent.h
///
//==================================================================================

//------------------------------ apKernelDebuggingFailedEvent.h ------------------------------

#ifndef __APKERNELDEBUGGINGFAILEDEVENT_H
#define __APKERNELDEBUGGINGFAILEDEVENT_H

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apAfterKernelDebuggingEvent : public apEvent
// General Description: An event invoked by the spy after when it attempts to debug a
//                      kernel but fails.
// Author:  AMD Developer Tools Team
// Creation Date:       9/3/2011
// ----------------------------------------------------------------------------------
class AP_API apKernelDebuggingFailedEvent : public apEvent
{
public:
    enum apKernelDebuggingFailureReason
    {
        AP_KERNEL_NOT_SUPPORTED,
        AP_KERNEL_ARGS_NOT_SUPPORTED,
        AP_KERNEL_UNSUPPORTED_PLATFORM,
        AP_KERNEL_NON_GPU_DEVICE,
        AP_KERNEL_NOT_DEBUGGABLE,
        AP_KERNEL_QUEUE_NOT_INTERCEPTED,
        AP_KERNEL_ENQUEUE_ERROR,
        AP_KERNEL_DEBUG_FAILURE,
        AP_KERNEL_UNKNOWN_FAILURE
    };

public:
    apKernelDebuggingFailedEvent(apKernelDebuggingFailureReason failureReason = AP_KERNEL_UNKNOWN_FAILURE, cl_int openCLError = CL_SUCCESS, osThreadId triggeringThreadID = OS_NO_THREAD_ID, const gtString& failInformation = L"");
    ~apKernelDebuggingFailedEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    apKernelDebuggingFailureReason failureReason() const {return _failureReason;};
    cl_int openCLError() const {return _openCLError;};
    const gtString& failInformation() const { return m_failInformation; };

    void getKernelDebuggingFailureString(gtString& errorStr) const;

private:
    apKernelDebuggingFailureReason _failureReason;
    cl_int _openCLError;
    gtString m_failInformation;
};

#endif //__APKERNELDEBUGGINGFAILEDEVENT_H

