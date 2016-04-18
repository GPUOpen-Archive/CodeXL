//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suTechnologyMonitorsManager.cpp
///
//==================================================================================

//------------------------------ suTechnologyMonitorsManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/Events/apTechnologyMonitorFailureEvent.h>

// Local:
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suITechnologyMonitor.h>
#include <AMDTServerUtilities/Include/suTechnologyMonitorsManager.h>

// Static members initializations:
suTechnologyMonitorsManager* suTechnologyMonitorsManager::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::suTechnologyMonitorsManager
// Description: Constructor
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
suTechnologyMonitorsManager::suTechnologyMonitorsManager()
{

}

// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::~suTechnologyMonitorsManager
// Description: Destructor
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
suTechnologyMonitorsManager::~suTechnologyMonitorsManager()
{

}

// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::instance
// Description: Returns my single instance
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
suTechnologyMonitorsManager& suTechnologyMonitorsManager::instance()
{
    // If the instance was not yet created, create it now:
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new suTechnologyMonitorsManager;

    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::registerTechnologyMonitor
// Description: Registers a technology monitor with this manager
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
void suTechnologyMonitorsManager::registerTechnologyMonitor(suITechnologyMonitor* pMonitor)
{
    // Avoid any other operations while we are registering:
    _technologyMonitorsAccessCS.enter();

    // Make sure this monitor was not registered yet:
    int amountOfMonitors = (int)_technologyMonitors.size();
    bool newMonitor = true;

    for (int i = 0; i < amountOfMonitors; i++)
    {
        if (_technologyMonitors[i] == pMonitor)
        {
            newMonitor = false;
            break;
        }
    }

    // If this is a valid monitor, add it to the vector:
    GT_IF_WITH_ASSERT(newMonitor && (pMonitor != NULL))
    {
        _technologyMonitors.push_back(pMonitor);
    }

    _technologyMonitorsAccessCS.leave();
}

// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::unregisterTechnologyMonitor
// Description: Unregisters a technology monitor from this manager
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
void suTechnologyMonitorsManager::unregisterTechnologyMonitor(suITechnologyMonitor* pMonitor)
{
    // Avoid any other operations while we are unregistering:
    _technologyMonitorsAccessCS.enter();

    // Find the monitor:
    int amountOfMonitors = (int)_technologyMonitors.size();
    bool foundMonitor = false;

    for (int i = 0; i < amountOfMonitors; i++)
    {
        if (_technologyMonitors[i] == pMonitor)
        {
            foundMonitor = true;
        }

        // Copy each item after the one we remove into the slot behind it (we exclude the last index,
        // which we are going to pop):
        if (foundMonitor && (i < amountOfMonitors - 1))
        {
            _technologyMonitors[i] = _technologyMonitors[i + 1];
        }
    }

    // Delete the last monitor. If the unregistered monitor was last, this will remove it.
    // If it wasn't this will remove a monitor that was duplicated in the loop.
    _technologyMonitors.pop_back();

    _technologyMonitorsAccessCS.leave();
}

// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::notifyMonitorsOnProcessTerminationAlert
// Description: Notifies the technology monitors we are about to terminate
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
void suTechnologyMonitorsManager::notifyMonitorsOnProcessTerminationAlert()
{
    // Avoid any other operations while we are notifying:
    _technologyMonitorsAccessCS.enter();

    int amountOfMonitors = (int)_technologyMonitors.size();

    for (int i = 0; i < amountOfMonitors; i++)
    {
        // Make sure this monitor is valid:
        suITechnologyMonitor* pCurrentMonitor = _technologyMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // Notify it:
            pCurrentMonitor->onDebuggedProcessTerminationAlert();
        }
    }

    _technologyMonitorsAccessCS.leave();
}

// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::notifyMonitorsBeforeDebuggedProcessSuspended
// Description: Notifies the technology monitors we are about to be suspended
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
void suTechnologyMonitorsManager::notifyMonitorsBeforeDebuggedProcessSuspended()
{
    // Avoid any other operations while we are notifying:
    _technologyMonitorsAccessCS.enter();

    int amountOfMonitors = (int)_technologyMonitors.size();

    for (int i = 0; i < amountOfMonitors; i++)
    {
        // Make sure this monitor is valid:
        suITechnologyMonitor* pCurrentMonitor = _technologyMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // Notify it:
            pCurrentMonitor->beforeDebuggedProcessSuspended();
        }
    }

    _technologyMonitorsAccessCS.leave();
}

// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::notifyMonitorsAfterDebuggedProcessResumed
// Description: Notifies the technology monitors we just resumed from suspension
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
void suTechnologyMonitorsManager::notifyMonitorsAfterDebuggedProcessResumed()
{
    // Avoid any other operations while we are notifying:
    _technologyMonitorsAccessCS.enter();

    int amountOfMonitors = (int)_technologyMonitors.size();

    for (int i = 0; i < amountOfMonitors; i++)
    {
        // Make sure this monitor is valid:
        suITechnologyMonitor* pCurrentMonitor = _technologyMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // Notify it:
            pCurrentMonitor->afterDebuggedProcessResumed();
        }
    }

    _technologyMonitorsAccessCS.leave();
}

// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::notifyMonitorsBeforeBreakpointException
// Description: Notifies the technology monitors this thread is about to raise
//              a breakpoint exception
// Author:      Uri Shomroni
// Date:        13/12/2009
// ---------------------------------------------------------------------------
void suTechnologyMonitorsManager::notifyMonitorsBeforeBreakpointException(bool isInOpenGLBeginEndBlock)
{
    // Avoid any other operations while we are notifying:
    _technologyMonitorsAccessCS.enter();

    int amountOfMonitors = (int)_technologyMonitors.size();

    for (int i = 0; i < amountOfMonitors; i++)
    {
        // Make sure this monitor is valid:
        suITechnologyMonitor* pCurrentMonitor = _technologyMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // Notify it:
            pCurrentMonitor->beforeBreakpointException(isInOpenGLBeginEndBlock);
        }
    }

    _technologyMonitorsAccessCS.leave();
}

// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::notifyMonitorsAfterBreakpointException
// Description: Notifies the technology monitors this thread just continued after
//              raising a breakpoint exception
// Author:      Uri Shomroni
// Date:        13/12/2009
// ---------------------------------------------------------------------------
void suTechnologyMonitorsManager::notifyMonitorsAfterBreakpointException(bool isInOpenGLBeginEndBlock)
{
    // Avoid any other operations while we are notifying:
    _technologyMonitorsAccessCS.enter();

    int amountOfMonitors = (int)_technologyMonitors.size();

    for (int i = 0; i < amountOfMonitors; i++)
    {
        // Make sure this monitor is valid:
        suITechnologyMonitor* pCurrentMonitor = _technologyMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // Notify it:
            pCurrentMonitor->afterBreakpointException(isInOpenGLBeginEndBlock);
        }
    }

    _technologyMonitorsAccessCS.leave();
}

// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::notifyMonitorsOnDebuggedProcessExecutionModeChanged
// Description: Notifies the technology monitors when the debugged process execution mode changes.
// Author:      Uri Shomroni
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void suTechnologyMonitorsManager::notifyMonitorsOnDebuggedProcessExecutionModeChanged(apExecutionMode executionMode)
{
    // Avoid any other operations while we are notifying:
    _technologyMonitorsAccessCS.enter();

    int amountOfMonitors = (int)_technologyMonitors.size();

    for (int i = 0; i < amountOfMonitors; i++)
    {
        // Make sure this monitor is valid:
        suITechnologyMonitor* pCurrentMonitor = _technologyMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // Notify it:
            pCurrentMonitor->onDebuggedProcessExecutionModeChanged(executionMode);
        }
    }

    _technologyMonitorsAccessCS.leave();
}


// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::notifyMonitorsStartMonitoredFunctionCallsLogFileRecording
// Description: Notifies the technology monitors when the monitored function calls log file recording is on
// Author:      Sigal Algranaty
// Date:        29/12/2009
// ---------------------------------------------------------------------------
void suTechnologyMonitorsManager::notifyMonitorsStartMonitoredFunctionCallsLogFileRecording()
{
    // Avoid any other operations while we are notifying:
    _technologyMonitorsAccessCS.enter();

    int amountOfMonitors = (int)_technologyMonitors.size();

    for (int i = 0; i < amountOfMonitors; i++)
    {
        // Make sure this monitor is valid:
        suITechnologyMonitor* pCurrentMonitor = _technologyMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // Notify it:
            pCurrentMonitor->onStartHTMLLogFileRecording();
        }
    }

    _technologyMonitorsAccessCS.leave();
}

// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::notifyMonitorsStopMonitoredFunctionCallsLogFileRecording
// Description: Notifies the technology monitors when the monitored function calls log file recording is off
// Author:      Sigal Algranaty
// Date:        29/12/2009
// ---------------------------------------------------------------------------
void suTechnologyMonitorsManager::notifyMonitorsStopMonitoredFunctionCallsLogFileRecording()
{
    // Avoid any other operations while we are notifying:
    _technologyMonitorsAccessCS.enter();

    int amountOfMonitors = (int)_technologyMonitors.size();

    for (int i = 0; i < amountOfMonitors; i++)
    {
        // Make sure this monitor is valid:
        suITechnologyMonitor* pCurrentMonitor = _technologyMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // Notify it:
            pCurrentMonitor->onStopHTMLLogFileRecording();
        }
    }

    _technologyMonitorsAccessCS.leave();
}

// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::notifyMonitorsBeforeKernelDebugging
// Description: Notifies the technology monitors before a kernel debugging session starts
// Author:      Sigal Algranaty
// Date:        29/12/2009
// ---------------------------------------------------------------------------
void suTechnologyMonitorsManager::notifyMonitorsBeforeKernelDebugging()
{
    // Avoid any other operations while we are notifying:
    _technologyMonitorsAccessCS.enter();

    int amountOfMonitors = (int)_technologyMonitors.size();

    for (int i = 0; i < amountOfMonitors; i++)
    {
        // Make sure this monitor is valid:
        suITechnologyMonitor* pCurrentMonitor = _technologyMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // Notify it:
            pCurrentMonitor->onBeforeKernelDebugging();
        }
    }

    _technologyMonitorsAccessCS.leave();
}


// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::notifyMonitorsGenericBreakpointSet
// Description: Notify the technology monitors that a generic breakpoint is set / unset
// Arguments:   apGenericBreakpointType breakpointType
//              bool isOn
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        7/7/2011
// ---------------------------------------------------------------------------
void suTechnologyMonitorsManager::notifyMonitorsGenericBreakpointSet(apGenericBreakpointType breakpointType, bool isOn)
{
    // Avoid any other operations while we are notifying:
    _technologyMonitorsAccessCS.enter();

    int amountOfMonitors = (int)_technologyMonitors.size();

    for (int i = 0; i < amountOfMonitors; i++)
    {
        // Make sure this monitor is valid:
        suITechnologyMonitor* pCurrentMonitor = _technologyMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentMonitor != NULL)
        {
            // Notify it:
            pCurrentMonitor->onGenericBreakpointSet(breakpointType, isOn);
        }
    }

    _technologyMonitorsAccessCS.leave();
}

// ---------------------------------------------------------------------------
// Name:        suTechnologyMonitorsManager::reportFailedSystemModuleLoad
// Description: Reports a technology monitor failure to load the appropriate system module
// Author:      Uri Shomroni
// Date:        16/2/2014
// ---------------------------------------------------------------------------
void suTechnologyMonitorsManager::reportFailedSystemModuleLoad(const gtString& errMsg)
{
    // Forward the event:
    osThreadId currentThreadId = osGetCurrentThreadId();
    apTechnologyMonitorFailureEvent techMonitorFailedEvent(errMsg, currentThreadId);
    bool rcEve = suForwardEventToClient(techMonitorFailedEvent);
    GT_ASSERT(rcEve);
}


