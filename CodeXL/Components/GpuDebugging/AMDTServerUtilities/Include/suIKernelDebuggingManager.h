//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suIKernelDebuggingManager.h
///
//==================================================================================
#ifndef __SUIKERNELDEBUGGINGMANAGER_H
#define __SUIKERNELDEBUGGINGMANAGER_H

// Forward declarations:
class osCriticalSectionDelayedLocker;

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTAPIClasses/Include/apKernelDebuggingCommand.h>
#include <AMDTAPIClasses/Include/Events/apKernelDebuggingFailedEvent.h>

// Local:
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>

// Quickly enable or disable the use of the hw-based debugger for all projects:
// #define CS_USE_HD_HSA_HW_BASED_DEBUGGER 1

/// -----------------------------------------------------------------------------------------------
/// \class suIKernelDebuggingManager
/// \brief Description: Interface for a Debugging manager
/// -----------------------------------------------------------------------------------------------
class SU_API suIKernelDebuggingManager
{
public:
    /// The type of debugging session
    enum KernelDebuggingSessionReason
    {
        STEP_IN_COMMAND,
        KERNEL_FUNCTION_NAME_BREAKPOINT,
        KERNEL_SOURCE_CODE_BREAKPOINT
    };

    /// The kernel debugging manager identifier:
    enum KernelDebuggingManagerType
    {
        CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER,
        HD_HARDWARE_KERNEL_DEBUGGER,
    };

public:
    suIKernelDebuggingManager();
    virtual ~suIKernelDebuggingManager();

    /// Initializes the kernel debugging manager
    virtual bool initialize() = 0;
    /// Terminates the kernel debugging manager
    virtual bool terminate() = 0;

    // General kernel debugging API functions:

    /// Returns whether the selected debugging manager is currently active/enabled:
    virtual bool isAMDKernelDebuggingEnabled();

    /// Returns true if the build flags are supported by the current kernel debugger:
    virtual bool programBuildFlagsSupported(const char* buildFlags, gtString& failureReason);

    /// Returns true if the current system configuration requires the legacy flag:
    virtual bool shouldAppendLegacyBuildFlags(const gtASCIIString& currentFlags, int amdOCLRuntimeVer) const;

    /// Identify which manager is this, (to tell whether it requires function replacement, etc.)
    virtual KernelDebuggingManagerType kernelDebuggerType() = 0;

    // Functions used in debugging kernels:
    /// Returns a handle to the currently debugged kernel, or OA_CL_NULL_HANDLE if non exists:
    virtual oaCLKernelHandle currentlyDebuggedKernel() = 0;
    /// The geometry of the currently debugged kernel dispatch:
    virtual bool getGlobalWorkGeometry(int& o_workDimensions, int o_globalWorkSize[3], int o_localWorkSize[3], int o_globalWorkOffset[3]) = 0;
    /// Sets the value of the debugging command we will return via the callback function for the debugger by changing a field in the session object:
    virtual void setNextKernelDebuggingCommand(apKernelDebuggingCommand command) = 0;
    /// Sets the work item used for stepping. As long as this work item is not valid, we will continue stepping:
    virtual bool setSteppingWorkItem(const int coordinate[3]) = 0;
    /// Returns true if we have an execution mask and the indicated item is valid in it:
    virtual bool isWorkItemValid(const int coordinate[3]) = 0;
    /// Sets the coordinate parameter to be the first work item that is valid (Lowest Z, Lowest Y, Lowest X). If no work items are valid, returns -1:
    virtual bool getFirstValidWorkItem(int wavefrontIndex /*= -1*/, int o_coordinate[3]) = 0;
    /// Lets the callback function know it can release the kernel debugging API to execute the next command:
    virtual void releaseCallbackToNextCommand() = 0;

    // Aid functions:
    /// Gets the calls stack of the kernel currently being debugged:
    virtual bool getCurrentKernelCallStack(const int coordinate[3], osCallStack& o_kernelStack) = 0;
    /// Returns the line number for the current program counter:
    virtual int  getCurrentKernelDebugLineNumber(const int coordinate[3]) = 0;
    /// Returns true iff the named variable exists in the current scope:
    virtual bool doesVariableExistInCurrentScope(const gtString& variableName, const int coordinate[3]) = 0;
    /// Gets and formats the variable value for a given work item:
    virtual bool getVariableValueString(const gtString& variableName, const int coordinate[3], gtString& o_valueString, gtString& o_valueStringHex, gtString& o_variableType) = 0;
    /// Gets the direct children (members) of the selected variable:
    virtual bool getVariableMembers(const gtString& variableName, const int coordinate[3], gtVector<gtString>& o_memberNames) = 0;
    /// Gets a list of variables available in the current PC:
    virtual bool getAvailableVariables(const int coordinate[3], gtVector<gtString>& variableNames, bool getLeaves, int stackFrameDepth) = 0;
    /// Gets the execution mask if it is available:
    virtual bool getExecutionMask(bool*& o_executionMask, int& o_maskSize) = 0;
    /// Gets the number of active wavefronts:
    virtual bool getAmountOfActiveWavefronts(int& amountOfWavefronts) = 0;
    /// Gets active wavefront IDs:
    virtual bool getActiveWavefrontID(int wavefrontIndex, int& wavefrontId) = 0;
    /// Gets the wavefront if it is available:
    virtual bool getWavefrontIndex(const int coordinate[3], int& o_wavefrontIndex) = 0;
    /// Exports the variable values to a file:
    virtual bool exportVariableValuesToFile(const gtString& variableName, bool& o_variableTypeSupported, osFilePath& o_variableDataFilePath) = 0;
    /// Checks if the breakpoint requested is currently resolved. If it is, returns the line where it was resolved:
    virtual bool getKernelSourceBreakpointResolution(oaCLProgramHandle programHandle, int requestedLineNumber, int& o_resolvedLineNumber) = 0;
    /// Set if kernel debugging is enabled at all:
    virtual bool setKernelDebuggingEnableState(bool kernelDebuggingState);

    /// How should kernel debugging managers handle multiple kernel dispatches:
    static bool setMultipleKernelDebugDispatchMode(apMultipleKernelDebuggingDispatchMode mode);
    /// Handling access to the dispatch mechanism:
    static bool beforeCheckingKernelDebuggingDisptach(osCriticalSectionDelayedLocker& dispatchLock);
    /// Entering and exiting kernel dispatches:
    static void setDisptachInFlight(bool inFlight);
    /// Static method which causes synchronization by using a static critical section shared by debugging managers:
    static void synchronizeWithKernelDebuggingCallback();
    // Called when kernel debugging fails (before the started event was sent:
    static void reportKernelDebuggingFailure(cl_int openCLError, apKernelDebuggingFailedEvent::apKernelDebuggingFailureReason failureReason, gtString& errorString);
    // static parse variable name if variableName is an expression of type "varName[123]" or "*varName":
    static void extractDerefernceDataFromExpresssion(const gtString& variableName, gtString& variableBaseName, gtString& dereferencedVariableMember, bool& shouldDereference, int& variableIndex);
    // In OpenCL C, the floatn, intn, and other vector types have aliases
    static bool matchOpenCLMemberAliases(const gtString& variableName, gtString& variableNameWithAlias);
    // Creates a raw data file path from a variable name
    static void filePathFromVariableName(const gtString& variableName, osFilePath& rawDataFilePath);
    // Does a string represent a pseudo-variable?
    static bool isPseudoVariable(const gtString& variableName);
    // Evaluate pseudo-variables:
    static bool getPseudoVariableValueString(const gtString& variableName, const int coordinate[3], const int globalWorkGeometry[10], gtString& o_valueString, gtString& o_valueStringHex, gtString& o_variableType);
    // Get pseudo-variables members:
    static bool getPseudoVariableMembers(const gtString& variableName, const int globalWorkGeometry[10], gtVector<gtString>& o_memberNames);

protected:
    /// Holds initialization state of this kernel debugging manager
    bool m_isInitialized;
    /// Prevent the kernel debugging manager(s) to have two kernel callbacks at the same time:
    static osCriticalSection m_sCriticalSection;
    /// Prevent the kernel debugging manager(s) from enqueueing two kernels for debug at the same time:
    static osCriticalSection m_sDispatchCriticalSection;
    /// Is a debugging dispatch ongoing?
    static bool m_sDispatchInFlight;
    /// How should attempts to dispatch multiple kernels at the same time be handled?
    static apMultipleKernelDebuggingDispatchMode m_sMultipleKernelDebugDispatchMode;
    /// flag to show if kernel debugging is enabled at all
    bool m_isKernelDebuggingEnabled;
};

// Exposed pseudo variable names:
#define SU_PSEUDO_VAR_DISPATCH_DETAILS_NAME L"Kernel dispatch details"

#endif //__SUIKERNELDEBUGGINGMANAGER_H

