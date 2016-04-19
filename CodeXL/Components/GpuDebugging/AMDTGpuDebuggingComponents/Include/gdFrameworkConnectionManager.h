//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdFrameworkConnectionManager.h
///
//==================================================================================

//------------------------------ gdFrameworkConnectionManager.h ------------------------------

#ifndef __GDFRAMEWORKCONNECTIONMANAGER_H
#define __GDFRAMEWORKCONNECTIONMANAGER_H

// Infra:
#include <AMDTApplicationFramework/Include/afIBreakpointManager.h>
#include <AMDTApplicationFramework/Include/afIRunModeManager.h>

class apKernelSourceBreakpointsUpdatedEvent;

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

class GD_API gdFrameworkConnectionManager : public afIRunModeManager, public afIBreakpointManager
{
public:
    gdFrameworkConnectionManager();
    virtual ~gdFrameworkConnectionManager();

public:
    // Overrides afIRunModeManager:
    virtual afRunModes getCurrentRunModeMask();

    virtual bool canStopCurrentRun();
    virtual bool stopCurrentRun();

    virtual bool getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce);

    // Overrides afIBreakpointsManager:
    virtual int numberOfBreakpoints();
    virtual apBreakPoint* getBreakpoint(int breakpointIndex);
    virtual bool isBreakpointSupported(const apBreakPoint& breakpoint);
    virtual bool setBreakpoint(const apBreakPoint& breakpoint);     // Should generate a breakpoint update event
    virtual bool removeBreakpoint(const apBreakPoint& breakpoint);  // Should generate a breakpoint update event
    virtual osTransferableObjectType breakpointTypeFromSourcePath(const osFilePath& sourceFilePath);
    virtual bool doesBreakpointMatchFile(const apBreakPoint& breakpoint, const osFilePath& sourceFilePath);

    virtual bool bindProgramToBreakpoints(int contextId, int programIndex, bool unbind);
    virtual bool onKernelSourceCodeUpdate(const apKernelSourceBreakpointsUpdatedEvent& eve);

};

#endif //__GDFRAMEWORKCONNECTIONMANAGER_H

