//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdLinuxProcessDebugger.h
///
//==================================================================================

//------------------------------ pdLinuxProcessDebugger.h ------------------------------

#ifndef __PDLINUXPROCESSDEBUGGER
#define __PDLINUXPROCESSDEBUGGER

// Forward decelerations:
struct pdGDBThreadDataList;
class osCallStack;
class apExceptionEvent;
class apThreadCreatedEvent;
class pdDebuggedProcessWatcherThread;
class pdLauncherProcessWatcherThread;

// POSIX:
#include <sys/types.h>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osCondition.h>
#include <AMDTOSWrappers/Include/osModuleArchitecture.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>

// Local:
#include <src/pdGDBDataStructs.h>
#include <src/pdGDBDriver.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>

// Local:
#include <AMDTProcessDebugger/Include/ProcessDebuggerDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           pdLinuxProcessDebugger : public pdProcessDebugger
// General Description:
//   Linux implementation of pdProcessDebugger.
//
// Author:               Yaki Tebeka
// Creation Date:        20/12/2006
// ----------------------------------------------------------------------------------
class PD_API pdLinuxProcessDebugger : public pdProcessDebugger
{
public:
    pdLinuxProcessDebugger();
    virtual ~pdLinuxProcessDebugger();

    // Overrides pdProcessDebugger:
    virtual bool initializeDebugger(const apDebugProjectSettings& processCreationData) override;
    virtual bool launchDebuggedProcess();
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

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"LinuxProcessDebugger"; };
    virtual void onEventRegistration(apEvent& eve, bool& vetoEvent);

    virtual bool getDebuggedThreadCallStack(osThreadId threadId, osCallStack& callStack, bool hideSpyDLLsFunctions = true);
    virtual void fillCallsStackDebugInfo(osCallStack& callStack, bool hideSpyDLLsFunctions = true);
    virtual void fillThreadCreatedEvent(apThreadCreatedEvent& event);
    virtual bool canGetCallStacks();
    virtual bool canMakeThreadExecuteFunction(const osThreadId& threadId);
    virtual bool makeThreadExecuteFunction(const osThreadId& threadId, osProcedureAddress64 funcAddress);
    virtual void afterAPICallIssued();

    ///////////////////////////////////////////////////////////////////////////////////
    /// \brief Check if host debugging available
    ///
    /// \return true - availabel, false - not implemented yet
    ///
    /// \author Vadim Entov
    /// \date  07/09/2015
    virtual bool canGetHostVariables() const override;

    ///////////////////////////////////////////////////////////////////////////////////
    /// \brief Get locals variables list for specified frame and thhread
    ///
    /// \param[in]  threadId a id of requested thread
    /// \param[in]  callStackFrameIndex a index of call stack requested frame
    /// \param[out] o_variables a refernce to vector of local variables name
    ///
    /// \return true - successfull call, false - failed call
    ///
    /// \author Vadim Entov
    /// \date   07/09/2015
    virtual bool getHostLocals(osThreadId threadId, int callStackFrameIndex, int evaluationDepth, bool onlyNames, gtVector<apExpression>& o_locals) override;

    ///////////////////////////////////////////////////////////////////////////////////
    /// \brief Get local variable value
    ///
    /// \param[in]  threadId a id of requested thread
    /// \param[in]  callStackFrameIndex a index of call stack requested frame
    /// \param[in]  varaibleName a name of requested local variable
    /// \param[out] o_varValue a local varaible text presentation value
    /// \param[out] o_variable a loval variable text hexdecimal presentation (if available)
    ///
    /// \return true - successfull call, false - failed call
    ///
    /// \author Vadim Entov
    /// \date   07/09/2015
    virtual bool getHostExpressionValue(osThreadId threadId, int callStackFrameIndex, const gtString& expressionText, int evaluationDepth, apExpression& o_exp) override;

    ///////////////////////////////////////////////////////////////////////////////////
    /// \brief Check if host debugging feature available
    ///
    /// \return true - successfull call, false - failed call
    ///
    /// \author Vadim Entov
    /// \date   16/09/2015
    virtual bool canPerformHostDebugging() const override;

    ///////////////////////////////////////////////////////////////////////////////////
    /// \brief Get type of breakpoint for specific thread.
    ///     Check under which kind of breakpoint requested thread was stopped.
    ///
    /// \param[in]  threadId a id of requested thread
    ///
    /// \return true - successfull call, false - failed call
    ///
    /// \author Vadim Entov
    /// \date   16/09/2015
    virtual bool isAtAPIOrKernelBreakpoint(osThreadId threadId) const override;

    virtual apBreakReason hostBreakReason() const;

    ///////////////////////////////////////////////////////////////////////////////////
    /// \brief Set breakpoint on specified cpp code line
    ///
    /// \param[in]  fileName a breakpoint target file name
    /// \param[in]  lineNumber a breakpoint target line number
    ///
    /// \return true - successfull call, false - failed call
    ///
    /// \author Vadim Entov
    /// \date   16/09/2015
    virtual bool setHostSourceBreakpoint(const osFilePath& fileName, int lineNumber) override;

    ///////////////////////////////////////////////////////////////////////////////////
    /// \brief Set breakpoint on specific cpp code function
    ///
    /// \param[in]  funcName a breakpoint target function name
    ///
    /// \return true - successfull call, false - failed call
    ///
    /// \author Vadim Entov
    /// \date   16/09/2015
    virtual bool setHostFunctionBreakpoint(const gtString& funcName) override;

    ///////////////////////////////////////////////////////////////////////////////////
    /// \brief Do host "step over/step into" debugging action
    ///
    /// \param[in]  threadId a id of requested thread
    /// \param[in]  stepType a type of step action
    /// \return true - successfull call, false - failed call
    ///
    /// \author Vadim Entov
    /// \date   16/09/2015
    virtual bool performHostStep(osThreadId threadId, StepType stepType) override;

    ///////////////////////////////////////////////////////////////////////////////////
    /// \brief Delete host breakpoint
    ///
    /// \param[in]  fileName a breakpoint full file name
    /// \param[in]  lineNumber a breakpoint line number
    /// \return true - successfull call, false - failed call
    ///
    /// \author Vadim Entov
    /// \date   25/01/2016
    virtual bool deleteHostSourceBreakpoint(const osFilePath& fileName, int lineNumber) override;

    ///////////////////////////////////////////////////////////////////////
    /// \brief Suspend process.
    ///
    /// \return true - success, false - fail
    /// \author Vadim Entov
    /// \date 09/02/2016
    bool suspendHostDebuggedProcess() override;

private:
    void initialize();
    bool launchGDB(const apDebugProjectSettings& processCreationData);
    bool setDebuggedProcessEnvVariables();
    bool processEnvVariableToString(const osEnvironmentVariable& envVar, gtString& envVarAsString);
    bool processPreloadEnvVariableValue(gtString& ldPreloadValue);
    bool processLibraryPathEnvVariableValue(gtString& ldLibraryPathValue);
    bool processFrameworkPathEnvVariableValue(gtString& dyldFrameworkPathValue);
    bool setDebuggedProcessCommandLineArguments();
    bool setDebuggedProcessWorkingDirectory();
    bool setDebuggedProcessArchitecture();
    bool afterGDBProcessSuspensionActions();
    void logCreatedThread(osThreadId OSThreadId);
    bool updateDebuggedProcessThreadsData();
    bool updateCurrentThreadCallStack();
    bool updateThreadCallStack(osThreadId threadId, pdGDBCallStack*& pCallStackData);
    bool updateThreadCallStackDuringInternalContinue(osThreadId threadId, pdGDBCallStack*& pCallStackData);
    void clearCallStacksMap();
    bool suspendDebuggedProcessThreads();
    bool getFunctionNameAtAddress(osProcedureAddress address, gtString& functionName);

    void onDebuggedProcessCreationEvent();
    void onDebuggedProcessTerminationEvent();
    void onProcessRunStartedEvent(const apEvent& event);
    void onProcessRunSuspendedEvent(const apEvent& event);
    void onProcessRunResumedEvent();
    void onBreakpointHitEvent(const apEvent& event);
    void onExceptionEventRegistration(const apExceptionEvent& eve);
    void onProcessRunSuspendedEventRegistration(apEvent& eve);
    void onThreadCreatedEventRegistration(apThreadCreatedEvent& eve, bool& vetoEvent);

    bool logDebuggedProcessPid();
    bool getAPIThreadGDBThreadID(gtString& apiThreadGDBId);
    bool getActiveThreadId(osThreadId& activeThreadId);
    osThreadId getOSThreadIdByGDBIndex(int gdbThreadId);
    int getGDBThreadId(const osThreadId& threadId);
    bool isDriverThread(const osThreadId& threadId);
    bool switchGDBActiveThread(osThreadId threadId, bool bSync = true);
    void outputHiddenSpyCallStack(const osCallStack& inputStack, osCallStack& outputStack);

    bool waitForDebuggedProcessSuspensionCondition();
    bool releaseDebuggedProcessSuspensionCondition();

    ///////////////////////////////////////////////////////////////////////////////////
    /// \brief Suspend debugged process in case process in running state
    ///
    /// \param[out]  suspendedBefore a process already suspended
    /// \param[in] onlyRelevantThrds: if false juset suspend all threads and don't check if driver and spy threads runs
    /// \return true - successfull call, false - failed call
    ///
    /// \author Vadim Entov
    /// \date   24/09/2015
    bool trySuspendProcess(bool& suspendedBefore, bool onlyRelevantThrds = true);

    ///////////////////////////////////////////////////////////////////////////////////
    /// \brief Resume suspended process
    ///
    /// \return true - successfull call, false - failed call
    ///
    /// \author Vadim Entov
    /// \date   24/09/2015
    bool tryResumeProcess();

    ///////////////////////////////////////////////////////////////////////////////////
    /// \brief Convert thread index to gdb thread id
    ///
    /// \param[in]  threadId a id of requested thread
    /// \param[out] reference to result value
    ///
    /// \return true - successfull call, false - failed call
    ///
    /// \author Vadim Entov
    /// \date  09/012/2015
    bool getThreadGDBId(osThreadId threadId, int& gdbId) const;

    bool getThreadIndexFromId(osThreadId threadId, int& threadIndex) const;

    ///////////////////////////////////////////////////////////////////////////////////
    /// \brief Resume spy thread only
    ///
    /// \return true - successfull call, false - failed call
    ///
    /// \author Vadim Entov
    /// \date  19/01/2016
    bool ReleaseSpyThread();

    ///////////////////////////////////////////////////////////////////////
    /// \brief Prepare current debugged process to terminate.
    ///
    /// \return true - success, false - fail
    /// \author Vadim Entov
    /// \date 01/02/2016
    bool prepareProcessToTerminate();

    ///////////////////////////////////////////////////////////////////////
    /// \brief Fill collection of gdb thred indexes for future resuming.
    ///     Spy thread and all opencl driver threads will resumed on host
    ///     breakpoint or host break.
    ///
    /// \return true - success, false - fail
    /// \author
    /// \date 30/03/2016
    bool fillThrdsRealizeCollection();

#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    bool loadProcessDebuggerESLauncher();
    bool launchApplicationWithiPhoneSimulator(const osFilePath& executablePath, const apDebugProjectSettings& processCreationData);

    osProcedureAddress _pESLauncherHelperFunction;
#endif

private:
    // An aid class that drives GDB:
    pdGDBDriver _gdbDriver;

    // Holds the debugged process creation data:
    apDebugProjectSettings _debuggedProcessCreationData;

    // Contains the debugged process pid:
    pid_t _debuggedProcessPid;

    // The debugged process threads data:
    pdGDBThreadDataList* _pDebuggedProcessThreadsData;

    // The id of the spies API thread:
    osThreadId _spiesAPIThreadId;

    // Contains true iff the API thread has started running:
    bool _isAPIThreadRunning;

    // Holds the current thread's call stack:
    gtMap<osThreadId, pdGDBCallStack*> _threadCallStacks;

    // A condition that enables waiting until the debugged process is suspended:
    osCondition _waitForDebuggedProcessSuspensionCondition;

    // Contains true iff a debugged process exists:
    // (It was launched and not terminated yet)
    bool _debuggedProcessExists;

    // Contains true iff the debugged process run is suspended:
    // (The debugged process hit a  spy breakpoint / etc)
    bool _isDebuggedProcssSuspended;

    // Set on process stopped by SIGINT signal or host debug breakpoint
    // If flag is true the process "physically stopped" under GDB
    bool _isDebuggedProcessGDBStopped;

    // Contains true iff the debugged process is a 64-bit process:
    bool _isDebugging64BitApplication;

    // Contains true iff the debugged process run was continued
    // internally by this class to enable the Spy API thread to run.
    // (To the outside world, the debugged process seems suspended)
    bool _isDuringInternalContinue;

    // Contains true iff the gdb run's internal continue is suspended,
    // which we do if we need to call a synchronus function after the
    // internal resume already started (eg getting debug information,
    // resuming threads on Mac before the "external" resume).
    bool _isDuringGDBSynchronusCommandExecution;

    // Contains true iff we are during debugged process function execution:
    // (For more details, see makeThreadExecuteFunction)
    bool _isDuringFunctionExecution;

    // Flags related to the various steps in kernel debugging::
    bool _isDuringKernelDebugging;
    bool m_isDuringHSAKernelDebugging;

    // Will get true iff we are during a debugged process suspension that was caused
    // by a fatal signal received by the debugged process:
    bool _isDuringFatalSignalSuspension;

    /// Process suspended by host breakpoint hit
    bool _isUnderHostBreakpoint;

    bool _canMakeExecuteFunction;

    // Maps debugged process address to the name of the function that resides
    // in this address:
    gtMap<osProcedureAddress, gtString> _functionNameAtAddress;

    // A thread to watch launcher applications (ie iPhone Simulator)
    pdLauncherProcessWatcherThread* _pLauncherProcessWatcherThread;

    // Which type of architecture we are using:
    osModuleArchitecture _debuggedExecutableArchitecture;

    // Watcher thread for iPhone applications (is used when GDB does not tell us that the process was terminated):
    pdDebuggedProcessWatcherThread* _pWatcherThread;

    /// Thread Id of stopped under breakpoint thread:
    osThreadId m_triggeringThreadId;

    /// Break reason for host suspensions:
    apBreakReason m_hostBreakReason;

    /// Last step kind issued:
    apBreakReason m_lastStepKind;

    enum class gdb_state
    {
        gdb_initialized_state,
        gdb_not_initialized_state
    };

    gdb_state _currentGDBState;

    std::map<std::pair<osFilePath, int>, int>   m_mapBpFileNameToIndex;    ///< gdb breakpoints map. Key - pair of file name\line number, value - gdb bp index

    std::set<int>                               m_GDBIdsOfThreadsToRelease;      ///< Collection of opencl driver threads and additional spy thread gdb id which will
    ///< be resumed on host breakpoint or host break

    enum { SUSPEND_PROCESS_WAITING_TIMEOUT = 200 };
};


#endif  // __PDLINUXPROCESSDEBUGGER
