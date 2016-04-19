//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file hsHSAMonitor.cpp
///
//==================================================================================


// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTServerUtilities/Include/suTechnologyMonitorsManager.h>

// Local:
#include <src/hsDebuggingManager.h>
#include <src/hsHSAMonitor.h>
#include <src/hsStringConstants.h>

// Static member initializations:
hsHSAMonitor* hsHSAMonitor::ms_pMySingleInstance = nullptr;

hsHSAMonitor& hsHSAMonitor::instance()
{
    if (nullptr == ms_pMySingleInstance)
    {
        ms_pMySingleInstance = new hsHSAMonitor;
    }

    return *ms_pMySingleInstance;
}

void hsHSAMonitor::onDebuggedProcessTerminationAlert()
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogHSAMonitorTerminationAlertStart, OS_DEBUG_LOG_DEBUG);

    hsDebuggingManager& theHSDebuggingManager = hsDebuggingManager::instance();

    theHSDebuggingManager.StopKernelRun();
    theHSDebuggingManager.UninitializeInterception();

    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogHSAMonitorTerminationAlertEnd, OS_DEBUG_LOG_DEBUG);
}

void hsHSAMonitor::beforeDebuggedProcessSuspended()
{

}

void hsHSAMonitor::afterDebuggedProcessResumed()
{

}

void hsHSAMonitor::beforeBreakpointException(bool isInOpenGLBeginEndBlock)
{
    GT_UNREFERENCED_PARAMETER(isInOpenGLBeginEndBlock);
}

void hsHSAMonitor::afterBreakpointException(bool isInOpenGLBeginEndBlock)
{
    GT_UNREFERENCED_PARAMETER(isInOpenGLBeginEndBlock);
}

void hsHSAMonitor::onDebuggedProcessExecutionModeChanged(apExecutionMode newExecutionMode)
{
    GT_UNREFERENCED_PARAMETER(newExecutionMode);
}

hsHSAMonitor::hsHSAMonitor()
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogInitHSAMonitor, OS_DEBUG_LOG_EXTENSIVE);
}

hsHSAMonitor::~hsHSAMonitor()
{

}


