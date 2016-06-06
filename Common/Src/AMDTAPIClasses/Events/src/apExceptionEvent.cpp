//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apExceptionEvent.cpp
///
//==================================================================================

//------------------------------ apExceptionEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Windows:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define WIN32_LEAN_AND_MEAN 1
    #include <Windows.h>
#endif

// Local:
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>


// ---------------------------------------------------------------------------
// Name:        apExceptionEvent::apExceptionEvent
// Description: Constructor
// Arguments:   triggeringThreadId - The thread that triggered the exception.
//              exceptionReason - The reason for the exception throw.
//                                (See apExceptionEvent::ExceptionReason enum).
//              exceptionAddress - The address in which the exception occur.
//              isSecondChange - is this is a second chance exception.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apExceptionEvent::apExceptionEvent(osThreadId triggeringThreadId, osExceptionReason exceptionReason,
                                   osInstructionPointer exceptionAddress, bool isSecondChance)
    : apEvent(triggeringThreadId), _exceptionReason(exceptionReason),
      _exceptionAddress(exceptionAddress), _isSecondChance(isSecondChance)
{
    // If the exception reason is out of range:
    if ((_exceptionReason < 0) || (OS_AMOUNT_OF_EXCEPTION_REASONS <= _exceptionReason))
    {
        _exceptionReason = OS_UNKNOWN_EXCEPTION_REASON;
        GT_ASSERT(0);
    }
}

// ---------------------------------------------------------------------------
// Name:        apExceptionEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apExceptionEvent::type() const
{
    return OS_TOBJ_ID_EXCEPTION_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apExceptionEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apExceptionEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the exception reason:
    ipcChannel << (gtInt32)_exceptionReason;

    // Write the exception address:
    ipcChannel << (gtUInt64)_exceptionAddress;

    // Write the second chance status:
    ipcChannel << _isSecondChance;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apExceptionEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apExceptionEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the exception reason:
    gtInt32 exceptionReasonAsInt32 = OS_AMOUNT_OF_EXCEPTION_REASONS;
    ipcChannel >> exceptionReasonAsInt32;
    _exceptionReason = (osExceptionReason)exceptionReasonAsInt32;

    // Read the exception address:
    gtUInt64 exceptionAddressAsUInt64 = 0;
    ipcChannel >> exceptionAddressAsUInt64;
    _exceptionAddress = (osInstructionPointer)exceptionAddressAsUInt64;

    // Read the second chance status:
    ipcChannel >> _isSecondChance;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apExceptionEvent::type
// Description: Returns my event type.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent::EventType apExceptionEvent::eventType() const
{
    return apEvent::AP_EXCEPTION;
}


// ---------------------------------------------------------------------------
// Name:        apExceptionEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
apEvent* apExceptionEvent::clone() const
{
    osThreadId threadId = triggeringThreadId();
    apExceptionEvent* pEventCopy = new apExceptionEvent(threadId, _exceptionReason, _exceptionAddress, _isSecondChance);
    return pEventCopy;
}

// ---------------------------------------------------------------------------
// Name:        apExceptionEvent::isFatalLinuxSignal
// Description: Returns true if the represented exception represents a Linux
//              signal that usually causes the debugged process termination.
// Author:  AMD Developer Tools Team
// Date:        16/12/2008
// ---------------------------------------------------------------------------
bool apExceptionEvent::isFatalLinuxSignal() const
{
    // TO_DO: LNX: Instead of guessing if a signal is fatal, we need to give the user
    //        control on which signals to break / continue, etc
    //        (similar to the control that gdb has)

    bool retVal = false;

    // We currently consider the below signals as fatal signals:
    if ((_exceptionReason == OS_SIGSEGV_SIGNAL) || (_exceptionReason == OS_EXC_BAD_ACCESS_SIGNAL) ||
        (_exceptionReason == OS_SIGPIPE_SIGNAL) || (_exceptionReason == OS_SIGABRT_SIGNAL) ||
        (_exceptionReason == OS_SIGTERM_SIGNAL) || (_exceptionReason == OS_SIGKILL_SIGNAL))
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apExceptionEvent::apExceptionEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apExceptionEvent::apExceptionEvent():
    _exceptionReason(OS_UNKNOWN_EXCEPTION_REASON), _exceptionAddress(NULL), _isSecondChance(false)
{

}


