//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSpyBreakpointImplementation.cpp
///
//==================================================================================

//------------------------------ suSpyBreakpointImplementation.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apBeforeDebuggedProcessRunResumedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunResumedEvent.h>

// Local:
#include <src/suSpyToAPIConnector.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suSpyBreakpointImplementation.h>


// Static members initializations:
suSpyBreakpointImplementation* suSpyBreakpointImplementation::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        suSpyBreakpointImplementation::suSpyBreakpointImplementation
// Description: Constructor
// Author:      Uri Shomroni
// Date:        1/11/2009
// ---------------------------------------------------------------------------
suSpyBreakpointImplementation::suSpyBreakpointImplementation():
    _isRunSuspended(false), _threadToExecuteFunction(OS_NO_THREAD_ID),
    _functionToExecute(NULL), _executedFunctionRetVal(false), _isCurrentlyExecutingFunction(false)
{

}

// ---------------------------------------------------------------------------
// Name:        suSpyBreakpointImplementation::~suSpyBreakpointImplementation
// Description: Destructor
// Author:      Uri Shomroni
// Date:        1/11/2009
// ---------------------------------------------------------------------------
suSpyBreakpointImplementation::~suSpyBreakpointImplementation()
{
    // Release any threads which might be locked by us:
    bool rcResume = resumeAllThreads();

    GT_ASSERT(rcResume);
}

// ---------------------------------------------------------------------------
// Name:        suSpyBreakpointImplementation::instance
// Description: Creates (if needed) and returns my single instance
// Author:      Uri Shomroni
// Date:        1/11/2009
// ---------------------------------------------------------------------------
suSpyBreakpointImplementation& suSpyBreakpointImplementation::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new suSpyBreakpointImplementation;

    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        suSpyBreakpointImplementation::breakpointImplementation
// Description: Runs a loop until we are told to release the threads. Effectively
//              emulates a "break" state, but enables us to call functions as well.
// Author:      Uri Shomroni
// Date:        1/11/2009
// ---------------------------------------------------------------------------
void suSpyBreakpointImplementation::breakpointImplementation()
{
    // Only stop if we need to:
    if (_isRunSuspended)
    {
        // Don't break if this is the API thread, or if we don't know what is
        // the API thread:
        osThreadId thisThreadId = osGetCurrentThreadId();
        osThreadId spiesAPIThreadId = suSpiesAPIThreadId();

        if ((spiesAPIThreadId != OS_NO_THREAD_ID) && (thisThreadId != spiesAPIThreadId))
        {
            // Loop until we are released:
            while (_isRunSuspended)
            {
                if (_runSuspensionCondition.isConditionLocked())
                {
                    // Make this thread wait:
                    _runSuspensionCondition.waitForCondition();
                }

                // If we were released to execute a function:
                if (thisThreadId == _threadToExecuteFunction)
                {
                    // Stop all the other threads:
                    _runSuspensionCondition.lockCondition();

                    // Sanity check:
                    GT_IF_WITH_ASSERT(_functionToExecute != NULL)
                    {
                        // cast the function pointer to the right type
                        bool(* functionToExecute)() = (bool(*)())_functionToExecute;
                        _executedFunctionRetVal = functionToExecute();
                    }

                    // Reset the parameters:
                    _threadToExecuteFunction = OS_NO_THREAD_ID;
                    _functionToExecute = NULL;

                    // Let the API thread know we're done:
                    _isCurrentlyExecutingFunction = false;
                }
            }
        }

    }
}

// ---------------------------------------------------------------------------
// Name:        suSpyBreakpointImplementation::breakAllThreads
// Description: Implements a break on all threads, called when the "breakpoint"
//              is "hit".
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        1/11/2009
// ---------------------------------------------------------------------------
bool suSpyBreakpointImplementation::breakAllThreads()
{
    bool retVal = true;

    // Lock the condition and raise the flag.
    // NOTE: This order is important, so a thread that just happens to be inside
    //       the breakpointImplementation function won't run ahead.
    retVal = _runSuspensionCondition.lockCondition();
    _isRunSuspended = true;

    // We raise the events AFTER the actual locking, to avoid timing issues on
    // the GUI side:
    osSocket* pEventsSocket = suSpyToAPIConnector::instance().eventForwardingSocket();
    GT_IF_WITH_ASSERT(pEventsSocket != NULL)
    {
        osThreadId currentThreadID = osGetCurrentThreadId();
        // TO_DO: Uri, 2/11/09: Get the real breakpoint address:
        apBreakpointHitEvent breakpointHitEvent(currentThreadID, NULL);
        apDebuggedProcessRunSuspendedEvent processSuspendedEvent(currentThreadID);
        *pEventsSocket << (const osTransferableObject&)breakpointHitEvent;
        *pEventsSocket << (const osTransferableObject&)processSuspendedEvent;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suSpyBreakpointImplementation::resumeAllThreads
// Description: Releases all threads, to resume the normal process run.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        1/11/2009
// ---------------------------------------------------------------------------
bool suSpyBreakpointImplementation::resumeAllThreads()
{
    bool retVal = true;

    // We raise the events BEFORE the actual unlocking, to avoid timing issues
    // on the GUI side:
    osSocket* pEventsSocket = suSpyToAPIConnector::instance().eventForwardingSocket();
    GT_IF_WITH_ASSERT(pEventsSocket != NULL)
    {
        apBeforeDebuggedProcessRunResumedEvent beforeResumedEvent;
        apDebuggedProcessRunResumedEvent processResumedEvent;
        *pEventsSocket << (const osTransferableObject&)beforeResumedEvent;
        *pEventsSocket << (const osTransferableObject&)processResumedEvent;
    }

    // Lower the flag and release the threads.
    // NOTE: This order is important, so the locked threads won't start spinning
    //       immediately:
    _isRunSuspended = false;
    retVal = _runSuspensionCondition.unlockCondition();
    retVal = _runSuspensionCondition.signalAllThreads() && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suSpyBreakpointImplementation::makeThreadExecuteFunction
// Description: Makes one thread execute a specific function once. This is done
//              by releasing all the threads, but only one of them would answer
//              the call. The function must be of type bool (*func)().
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        1/11/2009
// ---------------------------------------------------------------------------
bool suSpyBreakpointImplementation::makeThreadExecuteFunction(osThreadId threadID, osProcedureAddress functionAddress)
{
    bool retVal = false;

    // Make sure the thread we want to execute on is legal:
    // TO_DO: implement
    GT_IF_WITH_ASSERT(true)
    {
        // Set the parameters, then release the threads.
        // NOTE: This order is important, to avoid threads spinning redundantly and
        //       to avoid access issues.
        _isCurrentlyExecutingFunction = true;
        _functionToExecute = functionAddress;
        _executedFunctionRetVal = false;
        _threadToExecuteFunction = threadID;
        _runSuspensionCondition.unlockCondition();
        _runSuspensionCondition.signalAllThreads();

        // Wait for the function to return:
        bool rcTimeout = osWaitForFlagToTurnOff(_isCurrentlyExecutingFunction, ULONG_MAX);

        // Make sure the function exited and return the value:
        retVal = (_executedFunctionRetVal && rcTimeout);
    }

    return retVal;
}

