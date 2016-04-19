//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdGDBListenerThread.cpp
///
//==================================================================================

//------------------------------ pdGDBListenerThread.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/Events/apGDBListenerThreadWasSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>

// Local:
#include <src/pdStringConstants.h>
#include <src/pdGDBOutputReader.h>
#include <src/pdGDBListenerThread.h>
#include <src/pdGDBDriver.h>


// ---------------------------------------------------------------------------
// Name:        pdGDBListenerThread::pdGDBListenerThread
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        21/12/2006
// ---------------------------------------------------------------------------
pdGDBListenerThread::pdGDBListenerThread()
    : osThread(L"pdGDBListenerThread"), _pGDBCommunicationPipe(NULL), _executedGDBCommandId(PD_GDB_NULL_CMD), _pGDBDriver(NULL), _pGDBOutputReader(NULL),
      _shouldExitListenerThread(false)
{
    // Lock the condition object:
    _shouldListenToPipeCondition.lockCondition();

    // Create and execute the OS thread:
    bool rc2 = osThread::execute();
    GT_ASSERT(rc2);
}


// ---------------------------------------------------------------------------
// Name:        pdGDBListenerThread::pdGDBListenerThread
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        21/12/2006
// ---------------------------------------------------------------------------
pdGDBListenerThread::~pdGDBListenerThread()
{
    // Exit the listener thread:
    exitListenerThread();
}


// ---------------------------------------------------------------------------
// Name:        pdGDBListenerThread::startListening
// Description: Start listening to the GDB output pipe.
// Arguments: gdbCommunicationPipe - A pipe to which the gdb process stdout is redirected.
//            gdbOutputReader - A reader that reads GDB's output.
//            executedGDBCommandId - The last executed GDB command.
// Author:      Yaki Tebeka
// Date:        21/12/2006
//
// Implementation notes:
//   This function is called by the main application thread. It invokes the
//   GDB listener thread that after invocation, starts listening to the GDB output pipe.
// ---------------------------------------------------------------------------
bool pdGDBListenerThread::startListening(osPipeSocket& gdbCommunicationPipe, pdGDBDriver& GDBDriver, pdGDBOutputReader& gdbOutputReader,
                                         pdGDBCommandId executedGDBCommandId)
{
    bool retVal = false;

    // Store the GDB pipe file descriptor, GDB driver, GDB reader and the currently executed GDC command:
    _pGDBCommunicationPipe = &gdbCommunicationPipe;
    _pGDBDriver = &GDBDriver;
    _executedGDBCommandId = executedGDBCommandId;
    _pGDBOutputReader = &gdbOutputReader;

    _shouldExitListenerThread = false;

    // Unlock the condition:
    bool rc1 = _shouldListenToPipeCondition.unlockCondition();
    GT_IF_WITH_ASSERT(rc1)
    {
        // Signal the thread that the condition was unlocked:
        bool rc2 = _shouldListenToPipeCondition.signalSingleThread();
        GT_IF_WITH_ASSERT(rc2)
        {
            // Output debug message:
            OS_OUTPUT_DEBUG_LOG(PD_STR_startedListeningToGDBOutputs, OS_DEBUG_LOG_DEBUG);

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBListenerThread::exitListenerThread
// Description: Makes the GDB listener thread exit.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        3/2/2010
// ---------------------------------------------------------------------------
bool pdGDBListenerThread::exitListenerThread()
{
    bool retVal = false;

    // If the thread is running and reading gdb outputs:
    bool isConditionLocked = _shouldListenToPipeCondition.isConditionLocked();

    if (!isConditionLocked)
    {
        // Terminate the thread:
        retVal = terminate();
    }
    else
    {
        // The thread is waiting on the locked condition:

        // Ask the thread to exit:
        _shouldExitListenerThread = true;

        // Unlock the condition:
        bool rc1 = _shouldListenToPipeCondition.unlockCondition();
        GT_IF_WITH_ASSERT(rc1)
        {
            // Signal the thread that the condition was unlocked:
            bool rc2 = _shouldListenToPipeCondition.signalSingleThread();
            GT_IF_WITH_ASSERT(rc2)
            {
                retVal = true;
            }
        }
    }

    // Output a debug log message:
    if (retVal)
    {
        OS_OUTPUT_DEBUG_LOG(PD_STR_AskingTheListenerThreadToExit, OS_DEBUG_LOG_DEBUG);
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(PD_STR_AskingTheListenerThreadToExitFailed, OS_DEBUG_LOG_ERROR);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBListenerThread::entryPoint
// Description: The OS thread entry point.
// Author:      Yaki Tebeka
// Date:        22/12/2006
// ---------------------------------------------------------------------------
int pdGDBListenerThread::entryPoint()
{
    int retVal = 0;

    // An infinite loop:

    for (;;)
    {
        // Output debug log:
        OS_OUTPUT_DEBUG_LOG(PD_STR_listenerThreadStartedWaitingForCondition, OS_DEBUG_LOG_DEBUG);

        // While the debugged process is running:
        bool wasDebuggedProcessTerminated = false;
        bool wasDebuggedProcessSuspended = false;

        // Wait until the condition is unblocked:
        // (it is unblocked by startListening)

        if (_pGDBDriver)
        {
            if (_pGDBDriver->IsAllThreadsStopped())
            {
                _shouldListenToPipeCondition.waitForCondition();
                wasDebuggedProcessTerminated = (_pGDBDriver->GetExistingThreadsCount() == 0);
            }
        }
        else
        {
            _shouldListenToPipeCondition.waitForCondition();
        }

        // Output debug log:
        OS_OUTPUT_DEBUG_LOG(PD_STR_listenerThreadEndedWaitingForCondition, OS_DEBUG_LOG_DEBUG);

        // If we were asked to exit the thread:
        if (_shouldExitListenerThread)
        {
            break;
        }

        while (!wasDebuggedProcessSuspended && !wasDebuggedProcessTerminated)
        {
            // Output debug log:
            OS_OUTPUT_DEBUG_LOG(PD_STR_waitingForGDBOutputs, OS_DEBUG_LOG_DEBUG);

            // TO_DO: Yaki - here we should add a select with a loop that enables us to
            //               exit the waiting when the debugged process is terminated.

            // Read data from the GDB stdout pipe:
            readDataFromGDBStdoutPipe(wasDebuggedProcessTerminated, wasDebuggedProcessSuspended);

            wasDebuggedProcessSuspended = _pGDBDriver->IsAllThreadsStopped();

            // Output debug log message:
            osDebugLog& theDebugLogger = osDebugLog::instance();
            osDebugLogSeverity debugLogSevirity = theDebugLogger.loggedSeverity();

            if (OS_DEBUG_LOG_DEBUG <= debugLogSevirity)
            {
                gtString logMSG = PD_STR_endedParsingGDBOutputs;

                if (wasDebuggedProcessSuspended)
                {
                    logMSG += PD_STR_debuggedProcessWasSuspended;
                }
                else if (wasDebuggedProcessTerminated)
                {
                    logMSG += PD_STR_debuggedProcessWasTerminated;
                }
                else
                {
                    logMSG += PD_STR_debuggedProcessIsStillRunning;
                }

                OS_OUTPUT_DEBUG_LOG(logMSG.asCharArray(), OS_DEBUG_LOG_DEBUG);
            }
        }

        // Lock the condition:
        _shouldListenToPipeCondition.lockCondition();

        // Notify observers (mainly the pdLinuxProcessDebugger) that the gdb listener thread
        // was suspended:
        apGDBListenerThreadWasSuspendedEvent listenerThreadWasSuspendedEvent;
        apEventsHandler::instance().registerPendingDebugEvent(listenerThreadWasSuspendedEvent);
    }

    // Output a debug printout:
    OS_OUTPUT_DEBUG_LOG(PD_STR_exitingTheListenerThread, OS_DEBUG_LOG_DEBUG);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBListenerThread::beforeTermination
// Description: Is called before the OS thread is terminated.
// Author:      Yaki Tebeka
// Date:        22/12/2006
// ---------------------------------------------------------------------------
void pdGDBListenerThread::beforeTermination()
{
    // Nothing to be cleaned yet.
}


// ---------------------------------------------------------------------------
// Name:        pdGDBListenerThread::readDataFromGDBStdoutPipe
// Description:
//   Reads data from the GDB stdout, parses acts according to GDBs outputs.
// Arguments: wasDebuggedProcessTerminated - true iff the debugged process
//                                          was terminated.
//                    wasDebuggedProcessSuspended - true iff the debugged process
//                                          was suspended.
// Author:      Yaki Tebeka
// Date:        22/12/2006
// ---------------------------------------------------------------------------
void pdGDBListenerThread::readDataFromGDBStdoutPipe(bool& wasDebuggedProcessTerminated, bool& wasDebuggedProcessSuspended)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pGDBOutputReader != NULL && _pGDBDriver != NULL)
    {
        // Read gdb's outputs, parse it and act accordingly:
        bool rc1 = _pGDBOutputReader->readGDBOutput(*_pGDBCommunicationPipe, *_pGDBDriver, _executedGDBCommandId, wasDebuggedProcessSuspended, wasDebuggedProcessTerminated);

        wasDebuggedProcessSuspended =  _pGDBDriver->IsAllThreadsStopped();

        if (!rc1)
        {
            // We failed to parse gdb's output
            GT_ASSERT_EX(rc1, PD_STR_failedToParseGDBOutput);

            // Since we don't know what did the output mean, we assume that the debugged process continues to run:
            wasDebuggedProcessTerminated = false;
            wasDebuggedProcessSuspended = false;
        }
    }
}

