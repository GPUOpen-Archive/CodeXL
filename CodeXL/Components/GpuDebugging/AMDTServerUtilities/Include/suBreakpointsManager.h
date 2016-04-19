//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suBreakpointsManager.h
///
//==================================================================================

//------------------------------ suBreakpointsManager.h ------------------------------

#ifndef __SUBREAKPOINTSMANAGER_H
#define __SUBREAKPOINTSMANAGER_H

// Infra:
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apBreakReason.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apDetectedErrorParameters.h>

// Local:
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:   suBreakpointsManager
// General Description: Handles breakpoint functionality both for OpenCL and OpenGL monitors
// Author:               Sigal Algranaty
// Creation Date:        24/11/2009
// ----------------------------------------------------------------------------------
class SU_API suBreakpointsManager
{
public:
    static suBreakpointsManager& instance();
    virtual ~suBreakpointsManager();

    // Debugged process execution:
    void breakDebuggedProcessExecution();
    bool resumeDebuggedProcessRun();
    void waitOnDebuggedProcessSuspension();
    bool isBreakDebuggedProcessExecutionFlagOn() {return _breakDebuggedProcessExecution;};

    // "Step" commands:
    void setBreakpointAtNextMonitoredFunctionCall();
    void setBreakpointAtNextDrawFunctionCall();

    // Break on next frame functions:
    void setBreakAtTheNextFrame(bool breakAtTheNextFrame) { _breakAtTheNextFrame = breakAtTheNextFrame; };
    bool breakAtTheNextFrame() const { return _breakAtTheNextFrame; };

    // Break in current monitored function accessors:
    void setBreakInMonitoredFunctionCall(bool breakInFunction, bool& replacedWithStepOver);
    bool breakInMonitoredFunctionCall() const {return _breakInMonitoredFunction;};

    // Clear the break flags:
    void clearAllStepFlags()
    {
        _breakDebuggedProcessExecution = false; _breakAtTheNextMonitoredFunctionCall = false;
        _breakAtTheNextDrawFunctionCall = false; _breakAtTheNextFrame = false; _breakInMonitoredFunction = false;
    };

    // Generic breakpoints:
    bool breakOnGenericBreakpoint(apGenericBreakpointType breakpointType) const { return _shouldBreakOnGeneric[breakpointType]; };

    // HSA kernels:
    bool getHSABreakpointsForKernel(const gtString& hsaKernelName, gtVector<gtUInt64>& bpLines) const;
    bool addHSABreakpointForKernel(const gtString& hsaKernelName, const gtUInt64 bpLine);
    bool removeHSABreakpointFromKernel(const gtString& hsaKernelName, const gtUInt64 bpLine);

    // Breakpoints:
    bool setBreakpointAtMonitoredFunction(apMonitoredFunctionId functionId);
    bool clearBreakpointAtMonitoredFunction(apMonitoredFunctionId functionId);
    bool setKernelSourceCodeBreakpoint(oaCLProgramHandle programHandle, int lineNumber);
    bool clearKernelSourceCodeBreakpoint(oaCLProgramHandle programHandle, int lineNumber);
    bool setKernelFunctionNameBreakpoint(const gtString& kernelFuncName);
    bool clearKernelFunctionNameBreakpoint(const gtString& kernelFuncName);
    bool setGenericBreakpoint(apGenericBreakpointType breakpointType, bool isOn);
    void clearAllBreakPoints();
    int amountOfMonitoredFunctionBreakPoints() const;
    bool shouldBreakAt(apMonitoredFunctionId functionId);
    void getBreakpointsInProgram(oaCLProgramHandle programHandle, gtVector<int>& breakpointLineNumbers);
    void setKernelSourceBreakpointsDirty(bool areDirty) {_kernelSourceBreakpointsDirty = areDirty;};
    bool areKernelSourceBreakpointsDirty() {return _kernelSourceBreakpointsDirty;};
    bool shouldBreakAtKernelFunction(const gtString& kernelFuncName);

    // Break reason:
    apBreakReason breakReason() const {return _breakReason;};
    void setBreakReason(apBreakReason reason);

    // Breakpoint triggering context id:
    const apContextID& breakpointTriggeringContextId() const {return _breakpointTriggeringContextId;};

    // Detected Errors:
    void reportDetectedError(apContextID triggeringContextId, apErrorCode errorCode, const gtString& errorDescription, apMonitoredFunctionId associatedFuncId);
    const apDetectedErrorParameters& getDetectedErrorPrameters() const {return _detectedErrorParameters;};

    // Trigger breakpoints:
    void beforeTriggeringBreakpoint() {_breakpointAccessCS.enter();};
    void afterTriggeringBreakpoint() {_breakpointAccessCS.leave();};
    void testAndTriggerBeforeFuncExecutionBreakpoints(apMonitoredFunctionId  monitoredFunctionId, apContextID contextId, bool isInOpenGLBeginEndBlock = false);
    void testAndTriggerProfileModeBreakpoints(apMonitoredFunctionId  monitoredFunctionId, apContextID contextId);
    void triggerBreakpointException(apContextID contextId, apMonitoredFunctionId funcId, bool isInOpenGLBeginEndBlock = false);

    // Function to be executed:
    void setFunctionToBeExecutedDuringBreak(osProcedureAddress funcAddress) {_functionToBeExecutedDuringBreak = funcAddress;};
    void handleFunctionExecutionDuringBreak();

private:

    suBreakpointsManager();
    void testCommonFunctionExecutionBreakpoint(apMonitoredFunctionId monitoredFunctionId, bool isInOpenGLBeginEndBlock);

    // An array that holds monitored functions breakpoints.
    // (There is a breakpoint at monitored function iff _breakpointAtMonitoredFunction[i] == true)
    bool _breakpointAtMonitoredFunction[apMonitoredFunctionsAmount];

    // A list of source breakpoints for each OpenCL program:
    gtMap<oaCLProgramHandle, gtList<int> > _kernelSourceBreakpoints;
    bool _kernelSourceBreakpointsDirty;

    // A list of source breakpoints for HSA kernels:
    gtMap<gtString, gtVector<gtUInt64> > m_hsaKernelSourceCodeBreakpoints;

    // A vector of kernel function names to break on:
    gtVector<gtString> _kernelFunctionNameBreakpoints;

    // Contain true iff a generic breakpoint of type i is on:
    bool _shouldBreakOnGeneric[AP_AMOUNT_OF_GENERIC_BREAKPOINT_TYPES];

    // Contains true iff breakDebuggedProcessExecution() was called and we didn't
    // break the debugged process execution yet:
    bool _breakDebuggedProcessExecution;

    // Contains true iff we should throw a breakpoint exception at the next call
    // to a monitored function.
    bool _breakAtTheNextMonitoredFunctionCall;

    // Contains true iff we should throw a breakpoint exception at the next draw function call.
    bool _breakAtTheNextDrawFunctionCall;

    // Contains true iff we should throw a breakpoint exception at the next frame terminator.
    bool _breakAtTheNextFrame;

    // Contains true iff we are at a function that allows stepping into it, and we should stop at the
    // first line inside it:
    bool _breakInMonitoredFunction;

    // Manages access to the below members and the triggerBreakpointException function:
    osCriticalSection _breakpointAccessCS;

    // Contains the debugged process break reason:
    apBreakReason _breakReason;

    // The context that triggered the breakpoint:
    apContextID _breakpointTriggeringContextId;

    // The function that triggered the breakpoint:
    apMonitoredFunctionId _breakpointTriggeringFunctionId;

    // The ID of the render context which is about to be deleted:
    apContextID _aboutToBeDeletedContextID;

    // Detected error parameters:
    apDetectedErrorParameters _detectedErrorParameters;

    // Used for function execution where makeThreadExecute function causes crashes (win7 64-bit):
    osProcedureAddress _functionToBeExecutedDuringBreak;

    // My single instance:
    static suBreakpointsManager* _pMySingleInstance;
};


#endif //__SUBREAKPOINTSMANAGER_H


