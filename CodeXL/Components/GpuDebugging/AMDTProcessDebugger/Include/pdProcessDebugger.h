//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdProcessDebugger.h
///
//==================================================================================

//------------------------------ pdProcessDebugger.h ------------------------------

#ifndef __PDPROCESSDEBUGGER
#define __PDPROCESSDEBUGGER

// Forward declarations:
class apDebugProjectSettings;
class osCallStack;
class osFilePath;
class osPortAddress;
class apIEventsObserver;
class apThreadCreatedEvent;

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/apBreakReason.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// Local:
#include <AMDTProcessDebugger/Include/ProcessDebuggerDLLBuild.h>

// Definitions for in break function execution:
#define PD_EXECUTION_IN_BREAK_DUMMY_ADDRESS (osProcedureAddress64)0x1

// ----------------------------------------------------------------------------------
// Class Name:           PD_API pdProcessDebugger
// General Description:
//   The main class of the GRProcessDebugger.dll.
//   Represents a debugger that launches and debugs a remote application.
//   Enables:
//   - Launch a process for debugging.
//   - Terminate, suspend, etc the debugged process.
//   - Listens to the debugged process events (dll loaded / exception / etc).
//   - etc
//
// Author:               Yaki Tebeka
// Creation Date:        9/11/2003
// ----------------------------------------------------------------------------------
class PD_API pdProcessDebugger : public apIEventsObserver
{
public:
    enum FunctionExecutionMode
    {
        PD_DIRECT_EXECUTION_MODE,   // Direct function execution (using debugger facilities
        // to change the instruction pointer to the function address).
        PD_EXECUTION_IN_BREAK_MODE, // Execution in break (using a call from
        // suBreakpointsManager::handleFunctionExecutionDuringBreak).
        PD_NO_FUNCTION_EXECUTION,   // Direct Function execution is not allowed.
    };

    enum StepType
    {
        PD_STEP_IN,
        PD_STEP_OVER,
        PD_STEP_OUT,
    };

public:
    pdProcessDebugger();
    virtual ~pdProcessDebugger();

    static pdProcessDebugger& instance();
    static void registerInstance(pdProcessDebugger* aptrProcessDebuggerInstance);

    // Must be implemented by Sub-Classes:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Do host debugger (gdb, VS, etc..) initialization prerequestics
    ///
    /// \param processCreationData a data needed for the process debugger creation and initailization
    /// \return true - success, false - failed
    /// \author Vadim Entov
    /// \date 21/01/2016
    virtual bool initializeDebugger(const apDebugProjectSettings& processCreationData) = 0;
    virtual bool launchDebuggedProcess() = 0;
    virtual bool doesSupportTwoStepLaunching() const;
    virtual bool launchDebuggedProcessInSuspendedMode();
    virtual bool continueDebuggedProcessFromSuspendedCreation();
    virtual bool debuggedProcessExists() const = 0;
    virtual const apDebugProjectSettings* debuggedProcessCreationData() const = 0;
    virtual const apDebugProjectSettings* serverSideDebuggedProcessCreationData() const;
    virtual bool terminateDebuggedProcess() = 0;
    virtual bool isDebugging64BitApplication(bool& is64Bit) const = 0;

    virtual int amountOfDebuggedProcessThreads() const = 0;
    virtual bool getBreakpointTriggeringThreadIndex(int& index) const = 0;
    virtual bool getThreadId(int threadIndex, osThreadId& threadId) const = 0;
    virtual void setSpiesAPIThreadId(osThreadId spiesAPIThreadId) = 0;
    virtual int spiesAPIThreadIndex() const = 0;
    virtual osThreadId mainThreadId() const = 0;
    virtual osThreadId spiesAPIThreadId() const = 0;
    virtual osProcessId debuggedProcessId() const = 0;
    virtual bool isSpiesAPIThreadRunning() const = 0;

    virtual bool suspendDebuggedProcess() = 0;
    virtual bool resumeDebuggedProcess() = 0;
    virtual bool isDebuggedProcssSuspended() = 0;

    virtual bool suspendDebuggedProcessThread(osThreadId threadId) = 0;
    virtual bool resumeDebuggedProcessThread(osThreadId threadId) = 0;

    virtual bool getDebuggedThreadCallStack(osThreadId threadId, osCallStack& callStack, bool hideSpyDLLsFunctions = true) = 0;
    virtual void fillCallsStackDebugInfo(osCallStack& callStack, bool hideSpyDLLsFunctions = true) = 0;
    virtual void fillThreadCreatedEvent(apThreadCreatedEvent& event) = 0;
    virtual bool canGetCallStacks() = 0;
    virtual bool canMakeThreadExecuteFunction(const osThreadId& threadId) = 0;
    virtual bool makeThreadExecuteFunction(const osThreadId& threadId, osProcedureAddress64 funcAddress) = 0;
    virtual FunctionExecutionMode functionExecutionMode() const;
    virtual void afterAPICallIssued() = 0;
    virtual void setLocalLogFileDirectory(const osFilePath& localLogFilePath);
    virtual void remoteToLocalFilePath(osFilePath& io_filePath, bool useCache) const;

    virtual const wchar_t* eventObserverName() const { return L"pdProcessDebugger"; };

    virtual bool prepareProcessToTerminate();

    // Host debugging:
    virtual bool canGetHostVariables() const;
    virtual bool getHostLocals(osThreadId threadId, int callStackFrameIndex, gtVector<gtString>& o_variables);
    virtual bool getHostVariableValue(osThreadId threadId, int callStackFrameIndex, const gtString& variableName, gtString& o_varValue, gtString& o_varValueHex, gtString& o_varType);
    virtual bool canPerformHostDebugging() const;
    virtual bool isAtAPIOrKernelBreakpoint(osThreadId threadId) const;
    virtual apBreakReason hostBreakReason() const;
    virtual bool getHostBreakpointLocation(osFilePath& bpFile, int& bpLine) const;
    virtual bool setHostSourceBreakpoint(const osFilePath& fileName, int lineNumber);
    virtual bool deleteHostSourceBreakpoint(const osFilePath& fileName, int lineNumber);
    virtual bool setHostFunctionBreakpoint(const gtString& funcName);
    virtual bool performHostStep(osThreadId threadId, StepType stepType);
    virtual bool suspendHostDebuggedProcess();

private:
    friend class pdSingletonsDelete;

private:
    // The single instance of this class implementation:
    static pdProcessDebugger* _pMySingleInstance;
};

#endif  // __PDPROCESSDEBUGGER
