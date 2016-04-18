//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afIBreakpointManager.h
///
//==================================================================================

#ifndef __AFIBREAKPOINTMANAGER_H
#define __AFIBREAKPOINTMANAGER_H

// Forward declarations:
class apBreakPoint;
class osFilePath;
class apKernelSourceBreakpointsUpdatedEvent;

// Infra:
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          AF_API afIBreakpointManager
// General Description: An interface that should be implemented by plugins to allow the
//                      afPluginConnectionManager to control breakpoints
// Author:              Uri Shomroni
// Creation Date:       3/5/2012
// ----------------------------------------------------------------------------------
class AF_API afIBreakpointManager
{
public:
    afIBreakpointManager() {};
    virtual ~afIBreakpointManager() {};

public:
    virtual int numberOfBreakpoints() = 0;

    // The value is mutable but cannot be deleted:
    virtual apBreakPoint* getBreakpoint(int breakpointIndex) = 0;

    // The breakpoint type / parameters are supported by this manager. Note that no two managers may answer "true" for the same breakpoint:
    virtual bool isBreakpointSupported(const apBreakPoint& breakpoint) = 0;

    // Should only be called if the breakpoint is supported:
    virtual bool setBreakpoint(const apBreakPoint& breakpoint) = 0;     // Should generate a breakpoint update event
    virtual bool removeBreakpoint(const apBreakPoint& breakpoint) = 0;  // Should generate a breakpoint update event

    // If the file type is unknown, this functions must return OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES
    virtual osTransferableObjectType breakpointTypeFromSourcePath(const osFilePath& sourceFilePath) = 0;

    // This should check:
    // 1. is the breakpoint supported by this manager
    // 2. is the breakpoint of a type that has a source file as its context
    // 3. is the source file currently matching the breakpoint (i.e. is the breakpoint contained in the file)
    virtual bool doesBreakpointMatchFile(const apBreakPoint& breakpoint, const osFilePath& sourceFilePath) = 0;

    virtual bool bindProgramToBreakpoints(int contextId, int programIndex, bool unbind) { (void)(contextId); (void)(programIndex); (void)(unbind); return true;}

    // Is called when a kernel source code is updated (should update the breakpoints locations):
    virtual bool onKernelSourceCodeUpdate(const apKernelSourceBreakpointsUpdatedEvent& eve) = 0;


};


#endif //__AFIBREAKPOINTMANAGER_H

