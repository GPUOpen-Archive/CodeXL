//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suITechnologyMonitor.h
///
//==================================================================================

//------------------------------ suITechnologyMonitor.h ------------------------------

#ifndef __SUITECHNOLOGYMONITOR_H
#define __SUITECHNOLOGYMONITOR_H

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>

// Local:
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>
#include <AMDTServerUtilities/Include/suContextMonitor.h>

// ----------------------------------------------------------------------------------
// Class Name:          SU_API suITechnologyMonitor
// General Description: A base class for technology monitors (e.g. gsOpenGLMonitor),
//                      Used to pass events to and between them.
// Author:              Uri Shomroni
// Creation Date:       10/12/2009
// ----------------------------------------------------------------------------------
class SU_API suITechnologyMonitor
{
public:
    suITechnologyMonitor();
    virtual ~suITechnologyMonitor();

    // Events notification functions:
    virtual void onDebuggedProcessTerminationAlert() = 0;
    virtual void beforeDebuggedProcessSuspended() = 0;
    virtual void afterDebuggedProcessResumed() = 0;
    virtual void beforeBreakpointException(bool isInOpenGLBeginEndBlock) = 0;
    virtual void afterBreakpointException(bool isInOpenGLBeginEndBlock) = 0;
    virtual void onDebuggedProcessExecutionModeChanged(apExecutionMode newExecutionMode) = 0;
    virtual void onStartHTMLLogFileRecording();
    virtual void onStopHTMLLogFileRecording();
    virtual void onBeforeKernelDebugging();
    virtual void onGenericBreakpointSet(apGenericBreakpointType breakpointType, bool isOn);

    // Get the context monitor for a requested id:
    int amountOfContexts() const;
    const suContextMonitor* contextMonitor(int contextId) const;
    suContextMonitor* contextMonitor(int contextId);

    // Log files recording:
    bool getHTMLLogFilePath(int contextId, bool& isLogFileExist, const osFilePath*& logFilesPath);

    void setShouldInitializePerformanceCounters(bool shouldInitCounters) {_shouldInitializePerformanceCounters = shouldInitCounters;}
    bool shouldInitializePerformanceCounters() const {return _shouldInitializePerformanceCounters;}

protected:
    // A vector containing contexts monitors:
    gtVector<suContextMonitor*> _contextsMonitors;

    // Contain true iff performance counters should be monitored:
    bool _shouldInitializePerformanceCounters;

};

#endif //__SUITECHNOLOGYMONITOR_H

