//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suContextMonitor.cpp
///
//==================================================================================

//------------------------------ suContextMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTServerUtilities/Include/suContextMonitor.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>



// Static members initializations:
suContextMonitor* suContextMonitor::_pDefaultContextSingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        suContextMonitor::defaultContextInstance
// Description: Returns the single instance of the no context context monitor
// Author:      Sigal Algranaty
// Date:        21/3/2010
// ---------------------------------------------------------------------------
suContextMonitor& suContextMonitor::defaultContextInstance()
{
    // If the default context instance was not created yet:
    if (_pDefaultContextSingleInstance == NULL)
    {
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLogCreatingNullContext, OS_DEBUG_LOG_INFO);

        // Create it:
        apContextID defaultContextId;
        defaultContextId._contextId = 0;
        defaultContextId._contextType = AP_NULL_CONTEXT;
        _pDefaultContextSingleInstance = new suContextMonitor(defaultContextId);

        // Allocate the calls history logger:
        suNullContextCallsHistoryLogger* pNullContextHistoryLogger = new suNullContextCallsHistoryLogger;

        _pDefaultContextSingleInstance->_pCallsHistoryLogger = pNullContextHistoryLogger;

        // If log file recording is active, apply it now:
        bool areHTMLLogFilesActive = suAreHTMLLogFilesActive();

        if (areHTMLLogFilesActive)
        {
            pNullContextHistoryLogger->startHTMLLogFileRecording();
        }
    }

    return *_pDefaultContextSingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        suContextMonitor::suContextMonitor
// Description: Constructor.
// Author:      Sigal Algranaty
// Date:        21/3/2010
// ---------------------------------------------------------------------------
suContextMonitor::suContextMonitor(apContextID contextID)
    : _contextID(contextID), _pCallsHistoryLogger(NULL), _isDuringContextDataUpdate(false)
{
}

// ---------------------------------------------------------------------------
// Name:        suContextMonitor::~suContextMonitor
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        21/3/2010
// ---------------------------------------------------------------------------
suContextMonitor::~suContextMonitor()
{
    delete _pCallsHistoryLogger;
    _pCallsHistoryLogger = NULL;
}

// ---------------------------------------------------------------------------
// Name:        suContextMonitor::onFrameTerminatorCall
// Description: Is called when a frame terminator is called
// Author:      Sigal Algranaty
// Date:        21/3/2010
// ---------------------------------------------------------------------------
void suContextMonitor::onFrameTerminatorCall()
{
    // Some monitors are not relevant in Profile mode:
    apExecutionMode currentExecMode = suDebuggedProcessExecutionMode();

    if (currentExecMode != AP_PROFILING_MODE)
    {
        suCallsHistoryLogger* pCallsHistoryLogger = callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
        {
            pCallsHistoryLogger->onFrameTerminatorCall();
        }
        _callsStatisticsLogger.onFrameTerminatorCall();
    }
}


// ---------------------------------------------------------------------------
// Name:        suContextMonitor::onMonitoredFunctionCall
// Description: Is called when a monitored function call is callled.
// Author:      Yaki Tebeka
// Date:        23/3/2010
// ---------------------------------------------------------------------------
void suContextMonitor::onMonitoredFunctionCall()
{
}


// ---------------------------------------------------------------------------
// Name:        suContextMonitor::addFunctionCall
// Description: Logs a monitored function call into the context call history logger.
// Arguments:   apMonitoredFunctionId calledFunctionId - the logged function id.
//              argumentsAmount - The amount of function arguments.
//              va_list& pArgumentList - Function arguments in va_list style.
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/3/2010
// ---------------------------------------------------------------------------
void suContextMonitor::addFunctionCall(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus functionDeprecationStatus)
{
    // Add the function call to OpenCL calls logger:
    _pCallsHistoryLogger->addFunctionCall(calledFunctionId, argumentsAmount, pArgumentList, functionDeprecationStatus);

    // Get the function call enumerator, and save it.
    // The function call is added to the statistics logger only after the function call (since we need to know
    // the function redundancy status). For that we need to know the function enumerator:
    _callsStatisticsLogger.saveCurrentFunctionCallAttributes(calledFunctionId, argumentsAmount, pArgumentList, functionDeprecationStatus);


}

// ---------------------------------------------------------------------------
// Name:        suContextMonitor::onDebuggedProcessTerminationAlert
// Description: Is called before the debugged process is terminated.
// Author:      Yaki Tebeka
// Date:        25/1/2005
// ---------------------------------------------------------------------------
void suContextMonitor::onDebuggedProcessTerminationAlert()
{
    // Notify members about the coming termination event:
    _pCallsHistoryLogger->onDebuggedProcessTerminationAlert();
}


// ---------------------------------------------------------------------------
// Name:        suContextMonitor::updateContextDataSnapshot
// Description: This function should not be called.
// Author:      Yaki Tebeka
// Date:        23/3/2010
// ---------------------------------------------------------------------------
bool suContextMonitor::updateContextDataSnapshot(bool sendEvents)
{
    (void)(sendEvents); // unused
    GT_ASSERT(false);
    return false;
}


// ---------------------------------------------------------------------------
// Name:        suContextMonitor::beforeUpdatingContextDataSnapshot
// Description: Is called before a context data snapshot is updated.
//              Notifies relevant clients about this event.
// Author:      Sigal Algranaty
// Date:        22/3/2010
// ---------------------------------------------------------------------------
void suContextMonitor::beforeUpdatingContextDataSnapshot()
{
    // Mark that we are during context data update:
    _isDuringContextDataUpdate = true;
}


// ---------------------------------------------------------------------------
// Name:        suContextMonitor::afterUpdatingContextDataSnapshot
// Description: Is called after a context data snapshot is updated.
//              Notifies relevant clients about this event.
// Author:      Sigal Algranaty
// Date:        22/3/2010
// ---------------------------------------------------------------------------
void suContextMonitor::afterUpdatingContextDataSnapshot()
{
    // Mark that we are finished context data update:
    _isDuringContextDataUpdate = false;
}


// ---------------------------------------------------------------------------
// Name:        suContextMonitor::afterMonitoredFunctionExecutionActions
// Description: Is called after a monitored function is executed
// Arguments:   apMonitoredFunctionId calledFunctionId
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        22/3/2010
// ---------------------------------------------------------------------------
void suContextMonitor::afterMonitoredFunctionExecutionActions(apMonitoredFunctionId calledFunctionId)
{
    // Add the call to the calls statistics:
    _callsStatisticsLogger.addFunctionCall(calledFunctionId);
}

