//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apKernelDebuggingFailedEvent.cpp
///
//==================================================================================

//------------------------------ apKernelDebuggingFailedEvent.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTAPIClasses/Include/Events/apKernelDebuggingFailedEvent.h>

// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingFailedEvent::apKernelDebuggingFailedEvent
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        9/3/2011
// ---------------------------------------------------------------------------
apKernelDebuggingFailedEvent::apKernelDebuggingFailedEvent(apKernelDebuggingFailureReason failureReason, cl_int openCLError, osThreadId triggeringThreadID, const gtString& failInformation)
    : apEvent(triggeringThreadID), _failureReason(failureReason), _openCLError(openCLError), m_failInformation(failInformation)
{
}

// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingFailedEvent::~apKernelDebuggingFailedEvent
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        9/3/2011
// ---------------------------------------------------------------------------
apKernelDebuggingFailedEvent::~apKernelDebuggingFailedEvent()
{

}

// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingFailedEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        9/3/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apKernelDebuggingFailedEvent::type() const
{
    return OS_TOBJ_ID_KERNEL_DEBUGGING_FAILED_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingFailedEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        9/3/2011
// ---------------------------------------------------------------------------
bool apKernelDebuggingFailedEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    // Write the failure details:
    ipcChannel << (gtInt32)_failureReason;
    ipcChannel << (gtInt32)_openCLError;
    ipcChannel << m_failInformation;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingFailedEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        9/3/2011
// ---------------------------------------------------------------------------
bool apKernelDebuggingFailedEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    // Read the failure details:
    gtInt32 failureReasonAsInt32 = -1;
    ipcChannel >> failureReasonAsInt32;
    _failureReason = (apKernelDebuggingFailureReason)failureReasonAsInt32;
    gtInt32 openCLErrorAsInt32 = -1;
    ipcChannel >> openCLErrorAsInt32;
    _openCLError = (cl_int)openCLErrorAsInt32;

    ipcChannel >> m_failInformation;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingFailedEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        9/3/2011
// ---------------------------------------------------------------------------
apEvent::EventType apKernelDebuggingFailedEvent::eventType() const
{
    return apEvent::AP_KERNEL_DEBUGGING_FAILED_EVENT;
}


// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingFailedEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller's responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        9/3/2011
// ---------------------------------------------------------------------------
apEvent* apKernelDebuggingFailedEvent::clone() const
{
    apKernelDebuggingFailedEvent* pClone = new apKernelDebuggingFailedEvent(_failureReason, _openCLError, triggeringThreadId(), m_failInformation);

    return pClone;
}

// ---------------------------------------------------------------------------
// Name:        apKernelDebuggingFailedEvent::getKernelDebuggingFailureString
// Description: Sets errorStr to a value that describes why kernel debugging failed.
// Author:  AMD Developer Tools Team
// Date:        19/9/2011
// ---------------------------------------------------------------------------
void apKernelDebuggingFailedEvent::getKernelDebuggingFailureString(gtString& errorStr) const
{
    errorStr = AP_STR_KernelDebuggingFailed;

    switch (_failureReason)
    {
        case apKernelDebuggingFailedEvent::AP_KERNEL_UNKNOWN_FAILURE:
            // No known reason:
            break;

        case apKernelDebuggingFailedEvent::AP_KERNEL_NOT_SUPPORTED:
        {
            // The kernel is not supported:
            errorStr.append(AP_STR_KernelDebuggingFailedReasonNotSupported);

            if (m_failInformation.isEmpty())
            {
                // If the specific feature is not mentioned, add the generic reason string:
                errorStr.append(AP_STR_KernelDebuggingFailedReasonNotSupportedGeneral);
            }
        }
        break;

        case apKernelDebuggingFailedEvent::AP_KERNEL_ARGS_NOT_SUPPORTED:
            // Arguments not supported:
            errorStr.append(AP_STR_KernelDebuggingFailedReasonArgsNotSupported);
            break;

        case apKernelDebuggingFailedEvent::AP_KERNEL_UNSUPPORTED_PLATFORM:
            // The platform is not supported:
            errorStr.append(AP_STR_KernelDebuggingFailedReasonUnsupportedPlatform);
            break;

        case apKernelDebuggingFailedEvent::AP_KERNEL_NON_GPU_DEVICE:
            // The device is not supported:
            errorStr.append(AP_STR_KernelDebuggingFailedReasonNonGPUDevice);
            break;

        case apKernelDebuggingFailedEvent::AP_KERNEL_NOT_DEBUGGABLE:
            // The kernel is not debuggable:
            errorStr.append(AP_STR_KernelDebuggingFailedReasonKernelNotDebuggable);
            break;

        case apKernelDebuggingFailedEvent::AP_KERNEL_QUEUE_NOT_INTERCEPTED:
            // The command queue was not intercepted correctly:
            errorStr.append(AP_STR_KernelDebuggingFailedReasonQueueNotIntercepted);
            break;

        case apKernelDebuggingFailedEvent::AP_KERNEL_ENQUEUE_ERROR:
            // The kernel enqueueing function had an error:
            errorStr.append(AP_STR_KernelDebuggingFailedReasonOpenCLError);
            break;

        case apKernelDebuggingFailedEvent::AP_KERNEL_DEBUG_FAILURE:
            // An error was encountered:
            errorStr.append(AP_STR_KernelDebuggingFailedReasonDebugError);
            break;

        default:
            GT_ASSERT(false);
            break;
    }

    // If we had any additional information, add it here:
    if (!m_failInformation.isEmpty())
    {
        errorStr.append(L"\n");
        errorStr.append(m_failInformation);
    }
}

