//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdProcessDebugger.cpp
///
//==================================================================================

//------------------------------ pdProcessDebugger.cpp ------------------------------

//  Notification mechanism overview:
//  -------------------------------
//  The debugger has a "debugger thread" that monitors the debugged process.
//  When a debug process event occures, the debugger thread catch it and inserts it
//  into a "pending events queue" - a queue that contains events that are waiting to
//  be handled by the main application thread.
//  The main application thread should call handlePendingDebugEvent() to handle the
//  events that are waiting in the "pending events queue". The event handling includes
//  triggering the specific event observer.
//
//  Notification mechanism main functions:
//  -------------------------------------
//  - registerPendingEventNotificationCallback()
//      Registered a function that will be called asynchronously by the debugger thread
//      whenever a debugged process is placed in the "pending events queue".
//      This function usually registers a synchronous execution (Windows message / timer /
//      other) that will call handlePendingDebugEvent() by the main application thread.
//
//  - handlePendingDebugEvent()
//      Checks for events that reside in the "pending events queue", and handles them.
//      (Triggers observers, etc).
//      Notice: This function should be called by the main application thread !
//
//  - registerDebuggedProcessObserver()
//      Registered observers that listen to debugged process events. Whenever a debugged
//      process event is handled by handlePendingDebugEvent(), it will trigger this
//      observer.


// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>

// Local:
#include <src/pdStringConstants.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>

// Static member initializations:
pdProcessDebugger* pdProcessDebugger::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::pdProcessDebugger
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        30/12/2003
// ---------------------------------------------------------------------------
pdProcessDebugger::pdProcessDebugger()
{
    // Register to recieve debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_PROCESS_DEBUGGER_EVENTS_HANDLING_PRIORITY);

    // Also register for event registration notifications:
    apEventsHandler::instance().registerEventsRegistrationObserver(*this);
}


// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::~pdProcessDebugger
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        30/12/2003
// ---------------------------------------------------------------------------
pdProcessDebugger::~pdProcessDebugger()
{
    // Unregister from recieving debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);

    // Also unregister for event registration notifications:
    apEventsHandler::instance().unregisterEventsRegistrationObserver(*this);
}


// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::instance
// Description: Returns the single instance of this class.
// Author:      Yaki Tebeka
// Date:        19/5/2004
// ---------------------------------------------------------------------------
pdProcessDebugger& pdProcessDebugger::instance()
{
    GT_ASSERT(_pMySingleInstance);
    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::registerInstance
// Description:
//   Registers a single instance of this class.
//   If another instance of this class was already registered, it will be
//   replaced by the new instance. The other process debugger is not deleted,
//   as process debugger creation and deletion is handled in pdProcessDebuggersManager
// Arguments:   pProcessDebuggerInstance - Auto ptr to the single instance to
//                                         be registered.
// Author:      Yaki Tebeka
// Date:        19/5/2004
// ---------------------------------------------------------------------------
void pdProcessDebugger::registerInstance(pdProcessDebugger* pProcessDebuggerInstance)
{
    // Make the input process debugger the current process debugger:
    _pMySingleInstance = pProcessDebuggerInstance;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::doesSupportTwoStepLaunching
// Description: Should be overridden to return true iff the process debugger
//              implements the launchDebuggedProcessInSuspendedMode and
//              continueDebuggedProcessFromSuspendedCreation functions
// Author:      Uri Shomroni
// Date:        19/9/2010
// ---------------------------------------------------------------------------
bool pdProcessDebugger::doesSupportTwoStepLaunching() const
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::launchDebuggedProcessInSuspendedMode
// Description: To be overridden by process debuggers that return true to
//              doesSupportTwoStepLaunching. Creates the debugged process but does
//              not start its run. When this function returns, debuggedProcessId()
//              should have a valid value if implemented.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        19/9/2010
// ---------------------------------------------------------------------------
bool pdProcessDebugger::launchDebuggedProcessInSuspendedMode()
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::continueDebuggedProcessFromSuspendedCreation
// Description: Completes the debugged process launching after
//              launchDebuggedProcessInSuspendedMode, by starting the debugged
//              process run.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        19/9/2010
// ---------------------------------------------------------------------------
bool pdProcessDebugger::continueDebuggedProcessFromSuspendedCreation()
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::functionExecutionMode
// Description: Returns the function execution mode used by this process debugger.
//              This is the direct exection modes for most process debuggers.
// Author:      Uri Shomroni
// Date:        12/2/2010
// ---------------------------------------------------------------------------
pdProcessDebugger::FunctionExecutionMode pdProcessDebugger::functionExecutionMode() const
{
    return PD_DIRECT_EXECUTION_MODE;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::setLocalLogFileDirectory
// Description: Sets the output path to be used by remoteToLocalFilePath.
//              Since most process debuggers do not require that functionality,
//              This function can also be ignored
// Author:      Uri Shomroni
// Date:        23/10/2013
// ---------------------------------------------------------------------------
void pdProcessDebugger::setLocalLogFileDirectory(const osFilePath& localLogFilePath)
{
    // Unused parameter:
    (void)(localLogFilePath);
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::remoteToLocalFilePath
// Description: Takes a remote file path, and does all operations needed
//              to add it to the local machine. For most process debuggers,
//              Since they are running in a local context, this means making
//              no operation at all.
// Author:      Uri Shomroni
// Date:        30/9/2013
// ---------------------------------------------------------------------------
void pdProcessDebugger::remoteToLocalFilePath(osFilePath& io_filePath, bool useCache) const
{
    // Unused parameter:
    (void)(io_filePath);
    (void)(useCache);
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::serverSideDebuggedProcessCreationData
// Description: Returns the server-side version of the debugged process creation
//              data. This is the same as the input data for most debuggers.
// Author:      Uri Shomroni
// Date:        29/8/2013
// ---------------------------------------------------------------------------
const apDebugProjectSettings* pdProcessDebugger::serverSideDebuggedProcessCreationData() const
{
    return debuggedProcessCreationData();
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::canGetHostVariables
// Description: Returns true iff the specific implementation of pdProcessDebugger
//              supports getting host variables and locals lists.
//              Default is false.
// Author:      Uri Shomroni
// Date:        31/8/2015
// ---------------------------------------------------------------------------
bool pdProcessDebugger::canGetHostVariables() const
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::getHostLocals
// Description: Get the list of local variables for the selected
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        31/8/2015
// ---------------------------------------------------------------------------
bool pdProcessDebugger::getHostLocals(osThreadId threadId, int callStackFrameIndex, gtVector<gtString>& o_variables)
{
    GT_UNREFERENCED_PARAMETER(threadId);
    GT_UNREFERENCED_PARAMETER(callStackFrameIndex);
    GT_UNREFERENCED_PARAMETER(o_variables);

    // This implementation should never be called. Each implementer where
    // canGetHostVariables() == true should also override this function.
    GT_ASSERT(false);
    return false;
}


bool pdProcessDebugger::deleteHostSourceBreakpoint(const osFilePath& fileName, int lineNumber)
{
    GT_UNREFERENCED_PARAMETER(fileName);
    GT_UNREFERENCED_PARAMETER(lineNumber);

    return false;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::getHostVariableValue
// Description: Get the value for the specific variable.
//              If the expression cannot be parsed or is not accessible,
//              the output variables should be set to error values.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        31/8/2015
// ---------------------------------------------------------------------------
bool pdProcessDebugger::getHostVariableValue(osThreadId threadId, int callStackFrameIndex, const gtString& variableName, gtString& o_varValue, gtString& o_varValueHex, gtString& o_varType)
{
    GT_UNREFERENCED_PARAMETER(threadId);
    GT_UNREFERENCED_PARAMETER(callStackFrameIndex);
    GT_UNREFERENCED_PARAMETER(variableName);
    GT_UNREFERENCED_PARAMETER(o_varValue);
    GT_UNREFERENCED_PARAMETER(o_varValueHex);
    GT_UNREFERENCED_PARAMETER(o_varType);

    // This implementation should never be called. Each implementer where
    // canGetHostVariables() == true should also override this function.
    GT_ASSERT(false);
    return false;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::canPerformHostDebugging
// Description: Returns true iff the specific implementation of pdProcessDebugger
//              supports setting host breakpoints and host code stepping.
//              Default is false.
// Author:      Uri Shomroni
// Date:        31/8/2015
// ---------------------------------------------------------------------------
bool pdProcessDebugger::canPerformHostDebugging() const
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::isAtAPIOrKernelBreakpoint
// Description: Returns true iff the selected thread is a server thread that
//              triggered a spy breakpoint. Host breaks and all other threads
//              should return false.
//              If called with OS_NO_THREAD_ID, should return true iff the current
//              suspension is due to a spy breakpoint (i.e. if the value is true
//              for at least one thread).
// Author:      Uri Shomroni
// Date:        31/8/2015
// ---------------------------------------------------------------------------
bool pdProcessDebugger::isAtAPIOrKernelBreakpoint(osThreadId threadId) const
{
    GT_UNREFERENCED_PARAMETER(threadId);

    // Each implementer where canPerformHostDebugging() == true should also override this function.
    return !canPerformHostDebugging();
    // Should be (OS_NO_THREAD_ID != threadId) ? !isBreakpointTriggeringThreadId(threadId) : wasSpyBreakpointEncountered().
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::isAtAPIOrKernelBreakpoint
// Description: If currently at a host breakpoint, returns the break reason.
// Author:      Uri Shomroni
// Date:        2/2/2016
// ---------------------------------------------------------------------------
apBreakReason pdProcessDebugger::hostBreakReason() const
{
    return AP_FOREIGN_BREAK_HIT;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::isAtAPIOrKernelBreakpoint
// Description: If currently at a host source breakpoint, returns the location.
// Author:      Uri Shomroni
// Date:        24/3/2016
// ---------------------------------------------------------------------------
bool pdProcessDebugger::getHostBreakpointLocation(osFilePath& bpFile, int& bpLine) const
{
    GT_UNREFERENCED_PARAMETER(bpFile);
    GT_UNREFERENCED_PARAMETER(bpLine);
    return false;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::setHostSourceBreakpoint
// Description: Sets a source code breakpoint (or pending breakpoint) at the
//              specified source location. The breakpoint will automatically
//              bind and be hit when possible (no events should be registered
//              for the binding).
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        31/8/2015
// ---------------------------------------------------------------------------
bool pdProcessDebugger::setHostSourceBreakpoint(const osFilePath& fileName, int lineNumber)
{
    GT_UNREFERENCED_PARAMETER(fileName);
    GT_UNREFERENCED_PARAMETER(lineNumber);

    // This implementation should never be called. Each implementer where
    // canPerformHostDebugging() == true should also override this function.
    GT_ASSERT(false);
    return false;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::setHostFunctionBreakpoint
// Description: Sets a function breakpoint (or pending breakpoint) at the
//              specified function symbol. The name must not be a monitored
//              function name (those are handled by gaPersistentDataManager as
//              spy breakpoints). The breakpoint will automatically bind and be
//              hit when possible (no events should be registeredfor the binding).
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        31/8/2015
// ---------------------------------------------------------------------------
bool pdProcessDebugger::setHostFunctionBreakpoint(const gtString& funcName)
{
    GT_UNREFERENCED_PARAMETER(funcName);

    // This implementation should never be called. Each implementer where
    // canPerformHostDebugging() == true should also override this function.
    GT_ASSERT(false);
    // GT_ASSERT(apMonitoredFunctionsAmount == apMonitoredFunctionsManager::instance().monitoredFunctionId(funcName.asCharArray()));
    return false;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::performHostStep
// Description: Performs a host step. Operation includes the "resume" operations
//              This function should fail if called for a thread where
//              isAtAPIOrKernelBreakpoint() == true.
//              On failure, the process is NOT resumed.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        31/8/2015
// ---------------------------------------------------------------------------
bool pdProcessDebugger::performHostStep(osThreadId threadId, StepType stepType)
{
    GT_UNREFERENCED_PARAMETER(threadId);
    GT_UNREFERENCED_PARAMETER(stepType);

    // This implementation should never be called. Each implementer where
    // canPerformHostDebugging() == true should also override this function.
    GT_ASSERT(false);
    return false;
}

///////////////////////////////////////////////////////////////////////
/// \brief Prepare current debugged process to terminate.
///
/// \return true - success, false - fail
/// \author Vadim Entov
/// \date 01/02/2016
bool pdProcessDebugger::prepareProcessToTerminate()
{
    return true;
}

///////////////////////////////////////////////////////////////////////
/// \brief Suspend process.
///
/// \return true - success, false - fail
/// \author Vadim Entov
/// \date 09/02/2016
bool pdProcessDebugger::suspendHostDebuggedProcess()
{
    return false;
}

