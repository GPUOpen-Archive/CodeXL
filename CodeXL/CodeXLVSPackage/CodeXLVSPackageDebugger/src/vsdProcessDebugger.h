//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdProcessDebugger.h
///
//==================================================================================

//------------------------------ vsdProcessDebugger.h ------------------------------

#ifndef __VSDPROCESSDEBUGGER_H
#define __VSDPROCESSDEBUGGER_H

// Forward declarations:
class vsdCDebugEventCallback;

// Visual Studio:
#include <msdbg.h>

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>

// Local
#include <CodeXLVSPackageDebugger/Include/vsdPackageDLLBuild.h>

#define VSD_EXPRESSION_EVAL_TIMEOUT_MS 10000

// ----------------------------------------------------------------------------------
// Class Name:           VSD_API vsdProcessDebugger : public pdProcessDebugger
// General Description: Implements the pdProcessDebugger API over the native debug engine
// Author:               Uri Shomroni
// Creation Date:        25/12/2011
// ----------------------------------------------------------------------------------
class VSD_API vsdProcessDebugger : public pdProcessDebugger
{
    // Forward declarations:
private:
    class vsdCDebugThread;

public:
    vsdProcessDebugger(IDebugEngine2& riNativeDebugEngine, IDebugProgramProvider2& riNativeProgramProvider);
    virtual ~vsdProcessDebugger();
    static vsdProcessDebugger& vsInstance();

    void initialize();

    void setDebugPort(IDebugPort2* piDebugPort);
    IDebugPort2* getWrappedDebugPort() {return m_piDebugPort;};
    void addDebugThread(IDebugThread2* piDebugThread);
    bool getDebugThreadStartAddress(osThreadId threadId, osInstructionPointer& threadStartAddr) const;
    void removeDebugThread(IDebugThread2* piDebugThread);

    void continueFromSynchronousEvent(IDebugEvent2* piEvent);

    void setDllDirectory(const gtString& newDir, gtString& oldDir);

    // Process control:
    bool suspendProcessThreads();
    bool internalHaltProcess();
    bool internalResumeProcess();
    osProcessHandle debuggedProcessHandle();

    // Events received from vsdCDebugEventCallback:
    void handleProgramCreated(IDebugProgram2* piProgram);
    void handleProgramDestroyed(IDebugProgram2* piProgram);
    void handleEntryPointEvent(IDebugThread2* piThread);
    void handleExceptionEvent(IDebugEvent2* piEvent, IDebugExceptionEvent2* piExceptionEvent, IDebugThread2* piThread, bool& continueException);
    void handleBreakpointEvent(IDebugEvent2* piEvent, IDebugBreakpointEvent2* piBreakpointEvent, IDebugThread2* piThread, bool& continueBreakpoint);
    void handleStepCompleteEvent(IDebugEvent2* piEvent, IDebugThread2* piThread, bool& continueStep);
    void reportHostStep(osThreadId threadId, osInstructionPointer bpAddress);

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const {return L"VS Process Debugger";};
    virtual void onEventRegistration(apEvent& eve, bool& vetoEvent);

    // Overrides pdProcessDebugger:
    virtual bool initializeDebugger(const apDebugProjectSettings& processCreationData);
    virtual bool launchDebuggedProcess();
    virtual bool doesSupportTwoStepLaunching() const;
    virtual bool launchDebuggedProcessInSuspendedMode();
    virtual bool continueDebuggedProcessFromSuspendedCreation();
    virtual bool debuggedProcessExists() const;
    virtual const apDebugProjectSettings* debuggedProcessCreationData() const;
    virtual bool terminateDebuggedProcess();
    virtual bool isDebugging64BitApplication(bool& is64Bit) const;

    virtual int amountOfDebuggedProcessThreads() const;
    virtual bool getBreakpointTriggeringThreadIndex(int& index) const;
    virtual bool getThreadId(int threadIndex, osThreadId& threadId) const;
    virtual void setSpiesAPIThreadId(osThreadId spiesAPIThreadId);
    virtual int spiesAPIThreadIndex() const;
    virtual osThreadId mainThreadId() const;
    virtual osThreadId spiesAPIThreadId() const;
    virtual osProcessId debuggedProcessId() const;
    virtual bool isSpiesAPIThreadRunning() const;

    virtual bool suspendDebuggedProcess();
    virtual bool resumeDebuggedProcess();
    virtual bool isDebuggedProcssSuspended();

    virtual bool suspendDebuggedProcessThread(osThreadId threadId);
    virtual bool resumeDebuggedProcessThread(osThreadId threadId);

    virtual bool getDebuggedThreadCallStack(osThreadId threadId, osCallStack& callStack, bool hideSpyDLLsFunctions = true);
    virtual void fillCallsStackDebugInfo(osCallStack& callStack, bool hideSpyDLLsFunctions = true);
    virtual void fillThreadCreatedEvent(apThreadCreatedEvent& event);
    virtual bool canGetCallStacks();
    virtual bool canMakeThreadExecuteFunction(const osThreadId& threadId);
    virtual bool makeThreadExecuteFunction(const osThreadId& threadId, osProcedureAddress64 funcAddress);
    virtual FunctionExecutionMode functionExecutionMode() const;
    virtual void afterAPICallIssued();

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
    // Disallow use of my default constructor:
    vsdProcessDebugger() = delete;

    vsdCDebugThread* getThreadFromId(osThreadId threadId) const;
    int getThreadIndexFromId(osThreadId threadId) const;
    bool IsSpyModulePath(const gtString& modulePath) const;
    bool IsSpySourcePath(const gtString& sourcePath, bool& skipAnotherFrame) const;
    bool IsSpyFuncName(const gtString& funcName) const;
    bool IsDriverAddress(osInstructionPointer pc);
    bool IsDriverModuleName(const gtString& moduleNameLower) const;

private:
    class vsdCDebugThread
    {
    public:
        vsdCDebugThread(IDebugThread2& riThread);
        ~vsdCDebugThread();

        void setCanSuspendThread(bool canSuspend);
        bool suspendThread();
        bool resumeThread();

        osThreadId threadId();
        bool isDriverThread() const { return m_isDriverThread; };
        osInstructionPointer threadStartAddress() const { return m_threadStartAddress; };
        bool getCallStack(osCallStack& callStack, bool hideSpyDLLsFunctions);
        int skippedFrameCount(bool hideSpyDLLsFunctions);
        bool listLocalsForStackFrame(int callStackFrameIndex, gtVector<gtString>& o_variables);
        bool evaluateVariableInStackFrame(int callStackFrameIndex, const gtString& variableName, gtString& o_varValue, gtString& o_varValueHex, gtString& o_varType, bool evalHex);
        bool performStep(IDebugProcess3& riProcess3, IDebugProgram2& riProgram, StepType stepType);
        bool executeFunction(osProcedureAddress64 funcAddress, bool& waitingForExecutedFunctionFlag);
        bool markThreadAsSuspendedProcessEntryPoint();

    private:
        bool resumeThreadForExecution(bool& waitingForExecutedFunctionFlag);

    private:
        vsdCDebugThread() = delete;
        vsdCDebugThread(const vsdCDebugThread&) = delete;
        vsdCDebugThread(vsdCDebugThread&&) = delete;
        vsdCDebugThread& operator=(const vsdCDebugThread&) = delete;
        vsdCDebugThread& operator=(vsdCDebugThread&&) = delete;

        void detectThreadEntryPoint();

    private:
        bool m_isDriverThread;
        osInstructionPointer m_threadStartAddress;
        IDebugThread2* m_piDebugThread;
        HANDLE m_hThread;
        bool m_canSuspendThread;
        bool m_isSuspended;
    };

    class vsdCDebugBreakpointRequest : public IDebugBreakpointRequest2, public IDebugDocumentPosition2, public vsdCUnknown
    {
    public:
        vsdCDebugBreakpointRequest(IDebugProgram2& piProgram, const osFilePath& srcPath, int lineNum);

        ////////////////////////////////////////////////////////////
        // IUnknown methods
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

        ////////////////////////////////////////////////////////////
        // IDebugBreakpointRequest2 methods
        STDMETHOD(GetLocationType)(BP_LOCATION_TYPE* pBPLocationType);
        STDMETHOD(GetRequestInfo)(BPREQI_FIELDS dwFields, BP_REQUEST_INFO* pBPRequestInfo);

        ////////////////////////////////////////////////////////////
        // IDebugDocumentPosition2 methods
        STDMETHOD(GetFileName)(BSTR* pbstrFileName);
        STDMETHOD(GetDocument)(IDebugDocument2** ppDoc);
        STDMETHOD(IsPositionInDocument)(IDebugDocument2* pDoc);
        STDMETHOD(GetRange)(TEXT_POSITION* pBegPosition, TEXT_POSITION* pEndPosition);

    private:
        vsdCDebugBreakpointRequest() = delete;
        // Can only be deleted by the Release() method:
        virtual ~vsdCDebugBreakpointRequest();

    private:
        BP_REQUEST_INFO m_bpRequest;
        osFilePath m_sourcePath;
        int m_lineNumber;
    };

    class vsdHostBreakpoint
    {
    public:
        vsdHostBreakpoint(IDebugPendingBreakpoint2& riPendingBreakpoint, const osFilePath& requestedPath, int requestedLineNumber);
        ~vsdHostBreakpoint();

    public:
        bool removeRealBreakpoint();
        const osFilePath& requestedPath() const { return m_requestedPath; };
        const int requestedLineNumber() const { return m_requestedLineNumber; };
        const gtString& requestedFunctionName() const { return m_requestedFunctionName; };

    private:
        vsdHostBreakpoint() = delete;
        vsdHostBreakpoint(const vsdHostBreakpoint&) = delete;
        vsdHostBreakpoint(vsdHostBreakpoint&&) = delete;
        vsdHostBreakpoint& operator=(const vsdHostBreakpoint&) = delete;
        vsdHostBreakpoint& operator=(vsdHostBreakpoint&&) = delete;

    private:
        IDebugPendingBreakpoint2* m_piPendingBreakpoint;
        osFilePath m_requestedPath;
        int m_requestedLineNumber;
        gtString m_requestedFunctionName;
    };

private:
    // Debugged Process Data:
    apDebugProjectSettings* m_pProcessCreationData;
    bool m_debuggedProcessExists;
    bool m_isDebuggedProcessSuspended;
    bool m_isDebuggedProcessAtFatalException;
    osProcessId m_processId;
    osProcessHandle m_hProcess;
    osThreadId m_mainThreadId;
    osThreadId m_spiesAPIThreadId;
    bool m_gotIs64Bit;
    bool m_is64Bit;
    bool m_suspendThreadOnEntryPoint;
    bool m_isKernelDebuggingAboutToStart;
    bool m_isDuringKernelDebugging;
    bool m_isKernelDebuggingJustFinished;

    // Breakpoint data:
    osThreadId m_bpThreadId;
    bool m_isAPIBreakpoint;
    apBreakReason m_hostBreakReason;
    osFilePath m_currentBreakpointRequestedFile;
    int m_currentBreakpointRequestedLine;
    apBreakReason m_lastStepKind;

    // Spy frame hiding:
    bool m_waitingForDeferredStep;

    // Helper data:
    bool m_isWaitingForProcessSuspension;
    bool m_waitingForExecutedFunction;
    IDebugEvent2* m_piEventToContinue;
    bool m_skipContinueEvent;
    bool m_issuingStepCommand;

    // Instances of the Debug engine interfaces:
    IDebugEngine2* m_piNativeDebugEngine;
    IDebugEngineLaunch2* m_piNativeDebugEngineLaunch;
    IDebugProgramProvider2* m_piNativeProgramProvider;

    // Instances of process information:
    IDebugPort2* m_piDebugPort;
    IDebugDefaultPort2* m_piDebugDefaultPort;
    IDebugProcess2* m_piProcess;
    IDebugProgram2* m_piAttachedProgram;
    IDebugProgram2* m_piProgram;

    // Instances of process information wrappers:
    osCriticalSection m_threadDataCS;
    gtPtrVector<vsdCDebugThread*> m_debuggedProcessThreads;
    vsdCDebugThread* m_debuggedProcessSuspensionThread1;
    vsdCDebugThread* m_debuggedProcessSuspensionThread2;
    gtPtrVector<vsdHostBreakpoint*> m_hostBreakpoints;

    // Interface implementations:
    vsdCDebugEventCallback* m_pDebugEventCallback;
};

#endif //__VSDPROCESSDEBUGGER_H

