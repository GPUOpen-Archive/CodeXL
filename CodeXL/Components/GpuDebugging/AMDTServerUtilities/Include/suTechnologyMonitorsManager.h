//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suTechnologyMonitorsManager.h
///
//==================================================================================

//------------------------------ suTechnologyMonitorsManager.h ------------------------------

#ifndef __SUTECHNOLOGYMONITORSMANAGER_H
#define __SUTECHNOLOGYMONITORSMANAGER_H

// Forward Declarations:
class suITechnologyMonitor;

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>

// Local:
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          suTechnologyMonitorsManager
// General Description: Holds a vector of all active subclasses of suITechnologyMonitor
//                      and passes events to them.
// Author:              Uri Shomroni
// Creation Date:       10/12/2009
// ----------------------------------------------------------------------------------
class SU_API suTechnologyMonitorsManager
{
public:
    suTechnologyMonitorsManager();
    ~suTechnologyMonitorsManager();

    static suTechnologyMonitorsManager& instance();

    // Registration:
    void registerTechnologyMonitor(suITechnologyMonitor* pMonitor);
    void unregisterTechnologyMonitor(suITechnologyMonitor* pMonitor);

    // Events:
    void notifyMonitorsOnProcessTerminationAlert();
    void notifyMonitorsBeforeDebuggedProcessSuspended();
    void notifyMonitorsAfterDebuggedProcessResumed();
    void notifyMonitorsBeforeBreakpointException(bool isInOpenGLBeginEndBlock);
    void notifyMonitorsAfterBreakpointException(bool isInOpenGLBeginEndBlock);
    void notifyMonitorsOnDebuggedProcessExecutionModeChanged(apExecutionMode executionMode);
    void notifyMonitorsStartMonitoredFunctionCallsLogFileRecording();
    void notifyMonitorsStopMonitoredFunctionCallsLogFileRecording();
    void notifyMonitorsBeforeKernelDebugging();
    void notifyMonitorsGenericBreakpointSet(apGenericBreakpointType breakpointType, bool isOn);

    // Technology monitor failures:
    static void reportFailedSystemModuleLoad(const gtString& errMsg);

private:
    friend class suSingletonsDelete;

private:
    // The registered technology monitors:
    gtVector<suITechnologyMonitor*> _technologyMonitors;
    osCriticalSection _technologyMonitorsAccessCS;

    // My single instance:
    static suTechnologyMonitorsManager* _pMySingleInstance;
};

#endif //__SUTECHNOLOGYMONITORSMANAGER_H

