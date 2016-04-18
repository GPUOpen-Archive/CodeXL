//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afPluginConnectionManager.h
///
//==================================================================================

#ifndef __AFPLUGINCONNECTIONMANAGER_H
#define __AFPLUGINCONNECTIONMANAGER_H

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afIBreakpointManager.h>
#include <AMDTApplicationFramework/Include/afIRunModeManager.h>


// ----------------------------------------------------------------------------------
// Class Name:          AF_API afPluginConnectionManager
// General Description: Registers plugin implementations to allow queries for different information
//                      from within the application framework
// Author:              Uri Shomroni
// Creation Date:       3/5/2012
// ----------------------------------------------------------------------------------
class AF_API afPluginConnectionManager
{
public:
    static afPluginConnectionManager& instance();
    virtual ~afPluginConnectionManager();

    // Breakpoints:
    void registerBreakpointManager(afIBreakpointManager* piManager);
    void unregisterBreakpointManager(afIBreakpointManager* piManager);
    bool areBreakpointsSet();
    void getSetBreakpoints(gtVector<apBreakPoint*>& setBreakpoints);
    bool setBreakpoint(const apBreakPoint& breakpoint);
    bool removeBreakpoint(const apBreakPoint& breakpoint);

    osTransferableObjectType breakpointTypeFromSourcePath(const osFilePath& sourceFilePath);
    bool doesBreakpointMatchFile(const apBreakPoint& breakpoint, const osFilePath& sourceFilePath);
    virtual bool bindProgramToBreakpoints(int contextId, int programIndex, bool unbind);

    // Run modes:
    void registerRunModeManager(afIRunModeManager* piManager);
    void unregisterRunModeManager(afIRunModeManager* piManager);
    afRunModes getCurrentRunModeMask();
    bool stopCurrentRun(bool stopAndExit = false);
    void getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce);
    bool onKernelSourceCodeUpdate(const apKernelSourceBreakpointsUpdatedEvent& eve);

private:
    friend class afSingletonsDelete;

private:
    // Only the instance method can create this class:
    afPluginConnectionManager();

private:
    static afPluginConnectionManager* m_spMySingleInstance;

private:
    gtVector<afIBreakpointManager*> m_breakpointManagers;
    gtVector<afIRunModeManager*> m_runModeManagers;
};



#endif //__AFPLUGINCONNECTIONMANAGER_H

