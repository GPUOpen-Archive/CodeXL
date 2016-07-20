//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscBreakpointsManager.h
///
//==================================================================================

//------------------------------ vscBreakpointsManager.h ------------------------------

#ifndef __VSPBREAKPOINTSMANAGER_H
#define __VSPBREAKPOINTSMANAGER_H

// Forward declarations:
class gtString;
class osFilePath;
class apBreakPoint;
class apHostSourceCodeBreakpoint;
class apKernelFunctionNameBreakpoint;
class apKernelSourceBreakpointsUpdatedEvent;
class apKernelSourceCodeBreakpoint;
class apMonitoredFunctionBreakPoint;
class apGenericBreakpoint;
class vspCDebugEngine;

// Core interfaces:
#include <Include/Public/CoreInterfaces/IVscBreakpointsManagerOwner.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>

// Local:
#include <src/vscDebugBreakpoint.h>

// ----------------------------------------------------------------------------------
// Class Name:           vscBreakpointsManager
// General Description: Manages all translations between apBreakpoint and its subclasses
//                      and the parallel Visual Studio interfaces
// Author:               Uri Shomroni
// Creation Date:        28/9/2010
// ----------------------------------------------------------------------------------
class vscBreakpointsManager
{
public:
    vscBreakpointsManager();
    ~vscBreakpointsManager();
    void setDebugEngine(vspCDebugEngine* pDebugEngine);
    void setDebugEventCallback(IDebugEventCallback2* piDebugEventCallback);
    void setDebuggerInterface(IVsDebugger* piDebugger);
    void releaseInterfaces();

    static vscBreakpointsManager& instance();

    // Sets the owner object in terms of interaction with VS-specific code.
    static void setOwner(IVscBreakpointsManagerOwner* pOwner);

    bool setBreakpoint(const apBreakPoint& breakpoint);
    bool removeBreakpoint(const apBreakPoint& breakpoint);

    vspCDebugBreakpoint* respondToBreakpointRequest(IDebugBreakpointRequest2* piRequest);
    HRESULT bindBreakpoint(vspCDebugBreakpoint& breakpoint, bool performBind);
    void rebindBreakpointsInKernelSourceFile(const osFilePath& kernelSourceFilePath);
    void onKernelSourceCodeBreakpointsUpdated(const apKernelSourceBreakpointsUpdatedEvent& eve);
    void deleteBreakpoint(vspCDebugBreakpoint& breakpoint);
    void setBreakpointEnableStatus(vspCDebugBreakpoint& breakpoint);

    vspCDebugBreakpoint* getMonitoredFunctionBreakpoint(apMonitoredFunctionId funcId);
    vspCDebugBreakpoint* getGenericBreakpoint(apGenericBreakpointType breakpointType);
    vspCDebugBreakpoint* getKernelFunctionNameBreakpoint(const gtString& kernelFuncName);
    vspCDebugBreakpoint* getKernelSourceCodeBreakpoint(oaCLProgramHandle programHandle, int lineNumber);
    vspCDebugBreakpoint* getHostSourceBreakpoint(const osFilePath& sourcePath, int lineNumber);

    void onProcessTerminate();

private:
    // VS API functions:
    bool requestMonitoredFunctionBreakpoint(const apMonitoredFunctionBreakPoint& breakpoint);
    bool removeMonitoredFunctionBreakpoint(const apMonitoredFunctionBreakPoint& breakpoint);

    bool requestKernelSourceCodeBreakpoint(const apKernelSourceCodeBreakpoint& breakpoint);
    bool removeKernelSourceCodeBreakpoint(const apKernelSourceCodeBreakpoint& breakpoint);

    bool requestHostSourceCodeBreakpoint(const apHostSourceCodeBreakpoint& breakpoint);
    bool removeHostSourceCodeBreakpoint(const apHostSourceCodeBreakpoint& breakpoint);

    bool requestKernelFunctionNameBreakpoint(const apKernelFunctionNameBreakpoint& breakpoint);
    bool removeKernelFunctionNameBreakpoint(const apKernelFunctionNameBreakpoint& breakpoint);

    bool requestGenericBreakpoint(const apGenericBreakpoint& breakpoint);
    bool removeGenericBreakpoint(const apGenericBreakpoint& breakpoint);

    // GR API functions:
    bool addAPIBreakpointOnNamedFunction(const gtString& functionName, bool performBind, BP_ERROR_TYPE& errorType, gtString& errorMessage, apMonitoredFunctionId& funcId, gtString& breakpointKernelFunctionName, apGenericBreakpointType& breakpointType, bool enabled);
    bool addAPIBreakpointInOpenCLProgram(oaCLProgramHandle& programHandle, int lineNumber, bool performBind, BP_ERROR_TYPE& errorType, gtString& errorMessage, bool enabled);
    bool addAPIBreakpointInHostSource(const osFilePath& sourcePath, int lineNumber, bool performBind, BP_ERROR_TYPE& errorType, gtString& errorMessage, bool enabled);

    // Aid functions:
    void onBreakpointBindAttempt(vspCDebugBreakpoint& breakpoint, bool bindSuccess, bool bindAttempted, const BP_ERROR_TYPE& errorType, const gtString& errorMessage);
    void setBreakpointType(vspCDebugBreakpoint& breakpoint, BP_LOCATION_TYPE breakpointLocationKind, bool isCLCodeBreakpoint, oaCLProgramHandle openCLProgramHandle, const osFilePath& sourcePath, int lineNumber, apMonitoredFunctionId breakpointFuncId, const gtString& breakpointKernelFunctionName, apGenericBreakpointType breakpointType, bool bindSuccess);
    void removeBreakpointFromVectors(IDebugBreakpointRequest2* piBreakpointRequest);
    void unbindAndRemoveAllBreakpointsAndEngine();

private:
    friend class vspSingletonsDelete;

private:

    static IVscBreakpointsManagerOwner* m_pOwner;

    static vscBreakpointsManager* _pMySingleInstance;
    IVsDebugger* _piDebugger;
    vspCDebugEngine* _pDebugEngine;
    IDebugEventCallback2* _piDebugEventCallback;
    gtVector<vspCDebugBreakpoint*> _breakpoints;

    // Keep an multiple views of the same data.
    // The data is a duplication of the _breakpoints vector.
    // We sacrifice memory efficiency for runtime performance.
    gtMap<apMonitoredFunctionId, vspCDebugBreakpoint*> _breakpointByFuncId;
    gtMap<gtString, vspCDebugBreakpoint*> _breakpointByKernelFuncName;
    gtMap<apGenericBreakpointType, vspCDebugBreakpoint*> _breakpointByGenericType;
};

#endif //__VSPBREAKPOINTSMANAGER_H

