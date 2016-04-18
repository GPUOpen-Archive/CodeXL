//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suITechnologyMonitor.cpp
///
//==================================================================================

//------------------------------ suITechnologyMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>

// Local:
#include <AMDTServerUtilities/Include/suITechnologyMonitor.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suTechnologyMonitorsManager.h>


// ---------------------------------------------------------------------------
// Name:        suITechnologyMonitor::suITechnologyMonitor
// Description: Constructor
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
suITechnologyMonitor::suITechnologyMonitor(): _shouldInitializePerformanceCounters(true)
{
    // Register to the Technology Monitors manager:
    suTechnologyMonitorsManager::instance().registerTechnologyMonitor(this);

    // Add the default context to the list of context:
    suContextMonitor& nonContextMonitor = suContextMonitor::defaultContextInstance();
    _contextsMonitors.push_back(&nonContextMonitor);

    // Check the environment variables that contain information for performance counters monitoring:
    gtString perfCountersEnvVarValue;
    bool rcPerfCounters = osGetCurrentProcessEnvVariableValue(OS_STR_envVar_initializePerformanceCounters, perfCountersEnvVarValue);
    GT_IF_WITH_ASSERT(rcPerfCounters)
    {
        // If this environment variable is set to TRUE
        if (perfCountersEnvVarValue == OS_STR_envVar_valueTrue)
        {
            setShouldInitializePerformanceCounters(true);
        }
        else
        {
            // If this variable is set at all, we expect it to be TRUE or FALSE:
            setShouldInitializePerformanceCounters(false);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        suITechnologyMonitor::~suITechnologyMonitor
// Description: Destructor
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
suITechnologyMonitor::~suITechnologyMonitor()
{
    // Unregister from the Technology Monitors manager:
    suTechnologyMonitorsManager::instance().unregisterTechnologyMonitor(this);

    // Delete the context monitors:
    // Do not clear the first one, cause it is shared by all technology monitors:
    int noOfRenderContexts = (int)_contextsMonitors.size();

    for (int i = 1; i < noOfRenderContexts; i++)
    {
        delete _contextsMonitors[i];
        _contextsMonitors[i] = NULL;
    }

    _contextsMonitors.clear();
}


// ---------------------------------------------------------------------------
// Name:        suITechnologyMonitor::amountOfContexts
// Description: Returns the amount of contexts created by the debugged program
//              for this technology monitor
// Author:      Yaki Tebeka
// Date:        12/5/2004
// ---------------------------------------------------------------------------
int suITechnologyMonitor::amountOfContexts() const
{
    return (int)_contextsMonitors.size();
}


// ---------------------------------------------------------------------------
// Name:        suITechnologyMonitor::contextMonitor
// Description: Inputs a render context id and returns its context monitor.
//              (Or null, if a context of that id does not exist)
// Author:      Sigal Algranaty
// Date:        22/3/2010
// ---------------------------------------------------------------------------
const suContextMonitor* suITechnologyMonitor::contextMonitor(int contextId) const
{
    const suContextMonitor* retVal = NULL;

    // Index range test:
    int contextsAmount = amountOfContexts();

    if ((0 <= contextId) && (contextId < contextsAmount))
    {
        retVal = (const suContextMonitor*)_contextsMonitors[contextId];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suITechnologyMonitor::contextMonitor
// Description: Inputs a render context id and returns its context monitor.
//              (Or null, if a context of that id does not exist)
// Author:      Sigal Algranaty
// Date:        22/3/2010
// ---------------------------------------------------------------------------
suContextMonitor* suITechnologyMonitor::contextMonitor(int contextId)
{
    suContextMonitor* retVal = NULL;

    // Index range test:
    int contextsAmount = amountOfContexts();

    if ((0 <= contextId) && (contextId < contextsAmount))
    {
        retVal = (suContextMonitor*)_contextsMonitors[contextId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suITechnologyMonitor::onStartHTMLLogFileRecording
// Description: Handles start recoding into the HTML log files event.
// Author:      Yaki Tebeka
// Date:        18/8/2004
// ---------------------------------------------------------------------------
void suITechnologyMonitor::onStartHTMLLogFileRecording()
{
    bool rcStartHTMLRecording = true;

    // Iterate over the existing contexts:
    int amountOfContexts = (int)_contextsMonitors.size();

    for (int i = 0; i < amountOfContexts; i++)
    {
        // Start the current context log file recording:
        suCallsHistoryLogger* pCallsHistoryLogger = _contextsMonitors[i]->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
        {
            bool rc = pCallsHistoryLogger->startHTMLLogFileRecording();
            rcStartHTMLRecording = rcStartHTMLRecording && rc;
        }
    }

    GT_ASSERT_EX(rcStartHTMLRecording, L"Log files recording start has failed");
}


// ---------------------------------------------------------------------------
// Name:        suITechnologyMonitor::onStopHTMLLogFileRecording
// Description: Handles stop recoding into the HTML log files event.
// Author:      Yaki Tebeka
// Date:        18/8/2004
// ---------------------------------------------------------------------------
void suITechnologyMonitor::onStopHTMLLogFileRecording()
{
    // Iterate over the existing contexts:
    int amountOfContexts = (int)_contextsMonitors.size();

    for (int i = 0; i < amountOfContexts; i++)
    {
        // Stop the current context log file recording:
        suCallsHistoryLogger* pCallsHistoryLogger = _contextsMonitors[i]->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
        {
            pCallsHistoryLogger->stopHTMLLogFileRecording();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        suITechnologyMonitor::getHTMLLogFilePath
// Description: Get a context log files path (HTML)
// Arguments:   apContextID contextId
//            gtString& logFilesPath
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/3/2010
// ---------------------------------------------------------------------------
bool suITechnologyMonitor::getHTMLLogFilePath(int contextId, bool& isLogFileExist, osFilePath& logFilesPath)
{
    bool retVal = false;

    // Get the appropriate context monitor:
    const suContextMonitor* pContextMonitor = contextMonitor(contextId);

    if (pContextMonitor)
    {
        // Get its monitored functions calls logger:
        const suCallsHistoryLogger* pCallsLogger = pContextMonitor->callsHistoryLogger();
        GT_IF_WITH_ASSERT(pCallsLogger != NULL)
        {
            isLogFileExist = pCallsLogger->getHTMLLogFilePath(logFilesPath);
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suITechnologyMonitor::onBeforeKernelDebugging
// Description: By default do nothing
// Author:      Sigal Algranaty
// Date:        5/4/2011
// ---------------------------------------------------------------------------
void suITechnologyMonitor::onBeforeKernelDebugging()
{
}


// ---------------------------------------------------------------------------
// Name:        suITechnologyMonitor::onGenericBreakpointSet
// Description: By default do nothing
// Arguments:    apGenericBreakpointType breakpointType
//              bool isOn
// Author:      Sigal Algranaty
// Date:        7/7/2011
// ---------------------------------------------------------------------------
void suITechnologyMonitor::onGenericBreakpointSet(apGenericBreakpointType breakpointType, bool isOn)
{
    (void)(breakpointType); // unused
    (void)(isOn); // unused

}

