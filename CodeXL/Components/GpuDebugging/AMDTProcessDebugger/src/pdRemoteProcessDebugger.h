//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdRemoteProcessDebugger.h
///
//==================================================================================

//------------------------------ pdRemoteProcessDebugger.h ------------------------------

#ifndef __PDREMOTEPROCESSDEBUGGER_H
#define __PDREMOTEPROCESSDEBUGGER_H

// Forward decelerations:
class gtString;
class osChannel;
class CXLDaemonClient;
class pdRemoteProcessDebuggerEventsListenerThread;
class pdRemoteProcessDebuggerDebuggingServerWatcherThread;

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>


// Local:
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>

// ----------------------------------------------------------------------------------
// Class Name:          pdRemoteProcessDebugger : public pdProcessDebugger
// General Description: This process debugger acts as a front end to a remote process,
//                      launching a remote process debugger (via GRRemoteDebuggingServer),
//                      opening a pipe to it, and passing commands to it.
// Author:              Uri Shomroni
// Creation Date:       10/8/2009
// ----------------------------------------------------------------------------------
class pdRemoteProcessDebugger : public pdProcessDebugger
{
public:
    pdRemoteProcessDebugger();
    virtual ~pdRemoteProcessDebugger();

    // Overrides pdProcessDebugger:
    virtual bool initializeDebugger(const apDebugProjectSettings& processCreationData) override;
    virtual bool launchDebuggedProcess();
    virtual bool doesSupportTwoStepLaunching() const;
    virtual bool launchDebuggedProcessInSuspendedMode();
    virtual bool continueDebuggedProcessFromSuspendedCreation();
    virtual bool debuggedProcessExists() const;
    virtual const apDebugProjectSettings* debuggedProcessCreationData() const;
    virtual const apDebugProjectSettings* serverSideDebuggedProcessCreationData() const;
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
    virtual void fillThreadCreatedEvent(apThreadCreatedEvent& eve);
    virtual bool canGetCallStacks();
    virtual bool canMakeThreadExecuteFunction(const osThreadId& threadId);
    virtual bool makeThreadExecuteFunction(const osThreadId& threadId, osProcedureAddress64 funcAddress);
    virtual FunctionExecutionMode functionExecutionMode() const;
    virtual void afterAPICallIssued();
    virtual void setLocalLogFileDirectory(const osFilePath& localLogFilePath);
    virtual void remoteToLocalFilePath(osFilePath& io_filePath, bool useCache) const;

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual void onEventRegistration(apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"RemoteProcessDebugger"; };

    // Remote debugger only functions:
    bool isRemoteDebuggingServerAlive(bool checkLocal, bool checkRemote);

    // Host debugging
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
    enum remoteDebuggingServerConnectionMethod
    {
        PD_REMOTE_DEBUGGING_SERVER_NOT_CONNECTED,
        PD_REMOTE_DEBUGGING_SHARED_MEMORY_OBJECT,
        PD_REMOTE_DEBUGGING_TCP_IP_CONNECTION,
    };

    void StopEventListener();
    // General aid functions:
    void passDebugEventToRemoteDebuggingServer(const apEvent& eve);

    // Functions used for debugging on the local machine using the
    // remote debugging server and shared memory objects:
    gtString createSharedMemoryObjectPipeServer();
    gtString createEventsSharedMemoryObjectPipeServer();
    bool launchRemoteDebuggingServerOnLocalMachine(const gtString& sharedMemObj, const gtString& eventsSharedMemObj);
    bool launchRemoteDebuggingServerOnRemoteMachine(const osPortAddress& daemonConnectionPort, gtUInt16 remoteDebuggingConnectionPortNumber, gtUInt16 remoteDebuggingEventsPortNumber);
    void closeSharedMemoryObjectConnections();
    void closeTCPIPConnections();
    void cleanupDaemonConnection();

    // Close all open connections and reset the process debugger:
    void initialize();

private:
    // A channel that enables communicating with the remote debugging server:
    osChannel* _pRemoteDebuggingServerAPIChannel;

    // A channel that enables communicating with the remote debugging server's event handler:
    osChannel* _pRemoteDebuggingEventsAPIChannel;

    // The events listener thread:
    pdRemoteProcessDebuggerEventsListenerThread* _pEventsListenerThread;

    // The server watcher thread:
    pdRemoteProcessDebuggerDebuggingServerWatcherThread* _pServerWatcherThread;

    // A copy of the debugged process creation data as accepted:
    const apDebugProjectSettings* _pDebuggedProcessCreationData;

    // The modified process creation data, as recieved from the remote process debugger:
    apDebugProjectSettings* _pRemoteDebuggedProcessCreationData;

    // The currently used connection method:
    remoteDebuggingServerConnectionMethod _connectionMethod;

    // The Remote daemon client we are using:
    CXLDaemonClient* m_pDaemonClient;
    osPortAddress m_daemonConnectionPort;

    // The path to save local files in:
    const osFilePath* m_pLocalLogFilePath;

    // Cache of remote to local file paths:
    // (The left hand operand is gtString since osFilePath is non-comparable)
    gtMap<gtString, osFilePath> m_remoteToLocalFilePathCache;

    // Cache of often-queried values:
    bool _debuggedProcessExists;
    bool _debuggedProcessSuspended;
    bool _isDebugging64BitApplication;
    bool m_isSpiesAPIThreadRunning;
};

#endif //__PDREMOTEPROCESSDEBUGGER_H

