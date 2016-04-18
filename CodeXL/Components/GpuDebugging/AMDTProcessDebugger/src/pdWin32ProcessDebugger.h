//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWin32ProcessDebugger.h
///
//==================================================================================

//------------------------------ pdWin32ProcessDebugger.h ------------------------------

#ifndef __PDWIN32PROCESSDEBUGGER
#define __PDWIN32PROCESSDEBUGGER

// Forward decelerations:
class osDirectory;
class osFilePath;
class apDebuggedProcessCreatedEvent;

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Infra
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osMutex.h>
#include <AMDTOSWrappers/Include/osSynchronizationObject.h>
#include <AMDTOSWrappers/Include/osWin32DebugSymbolsManager.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osProcessSharedFile.h>
#include <AMDTAPIClasses/Include/apLoadedModule.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>

// Local:
#include <src/pdDebuggedProcessThreadData.h>
#include <src/pdWindowsLoadedModulesManager.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>


// ----------------------------------------------------------------------------------
// Class Name:           PD_API pdWin32ProcessDebugger : public pdProcessDebugger
// General Description:
//   Win32 implementation of pdProcessDebugger
//
// Author:               Yaki Tebeka
// Creation Date:        9/11/2003
// ----------------------------------------------------------------------------------
class pdWin32ProcessDebugger : public pdProcessDebugger
{
public:
    // Functions that are used by the application thread:
    // -------------------------------------------------
    pdWin32ProcessDebugger();
    virtual ~pdWin32ProcessDebugger();

    // Overrides pdProcessDebugger:
    virtual bool initializeDebugger(const apDebugProjectSettings& processCreationData) override;
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

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& event, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"Win32ProcessDebugger"; };
    virtual void onEventRegistration(apEvent& eve, bool& vetoEvent);

    virtual bool getDebuggedThreadCallStack(osThreadId threadId, osCallStack& callStack, bool hideSpyDLLsFunctions = true);
    virtual void fillCallsStackDebugInfo(osCallStack& callStack, bool hideSpyDLLsFunctions = true);
    virtual void fillThreadCreatedEvent(apThreadCreatedEvent& event);
    virtual bool canGetCallStacks();
    virtual bool canMakeThreadExecuteFunction(const osThreadId& threadId);
    virtual bool makeThreadExecuteFunction(const osThreadId& threadId, osProcedureAddress64 funcAddress);
    virtual FunctionExecutionMode functionExecutionMode() const;
    virtual void afterAPICallIssued();

private:
    friend class pdWin32DebuggerThread;

    // Functions that are used by the application thread:
    // -------------------------------------------------
    void initialize();
    void onDebuggedProcessCreationEvent(const apDebuggedProcessCreatedEvent& event);
    void onDebuggedProcessTerminationEvent();
    void onDebuggedProcessCreationFailureEvent();
    void onThreadCreatedEvent();
    void onThreadTerminatedEvent();
    void onExceptionEvent(const apExceptionEvent& event);
    bool addInstallDirToCurrentProcessPath();
    bool setDllDirectoryToSpiesDirectory();
    bool resetDllDirectory();
    bool setDllDirectory(const osFilePath& dllDirectory, gtString* o_pOriginalValue = NULL);
    bool updateThreadsData();
    bool makeThreadExecuteFunctionDirectly(const osThreadId& threadId, osProcedureAddress64 funcAddress);
    bool makeThreadExecuteFunctionInBreak(const osThreadId& threadId, osProcedureAddress64 funcAddress);
    bool waitForThreadFunctionExecution();

    // Functions that are used by the debugger thread:
    // -----------------------------------------------
    bool createDebuggedProcess();

    bool waitForDebugEvent();
    bool handleDebuggedProcessEvent(const DEBUG_EVENT& debugEvent, unsigned long& win32ContinueStatus);
    bool handleProcessCreation(const DEBUG_EVENT& debugEvent);
    bool handleProcessExit(const DEBUG_EVENT& debugEvent);
    bool handleThreadCreation(const DEBUG_EVENT& debugEvent);
    void handleThreadCreation(DWORD threadId, HANDLE threadHandle, osInstructionPointer threadStartAddress, bool isMainThread = false);
    void outputHandlingThreadCreationDebugLogMessage(osInstructionPointer threadStartAddress, bool isMainThread);
    bool handleThreadExit(const DEBUG_EVENT& debugEvent);
    void handleThreadExit(DWORD threadId, long exitCode);
    bool handleModuleLoad(const osFilePath& modulePath, osInstructionPointer moduleBaseAddress);
    bool handleDLLLoad(const DEBUG_EVENT& debugEvent);
    bool handleDLLUnload(const DEBUG_EVENT& debugEvent);
    bool handleException(const DEBUG_EVENT& debugEvent, unsigned long& win32ContinueStatus);
    bool handleBreakPoint(const DEBUG_EVENT& debugEvent, unsigned long& win32ContinueStatus);
    bool handleDebugString(const DEBUG_EVENT& debugEvent);
    bool handleWin32DebuggerError(const DEBUG_EVENT& debugEvent);

    bool readPointerFromDebuggedProcessMemory(void* pointerAddress, void** pPointerValue) const;
    bool readStringFromDebuggedProcessMemory(void* stringAddress, unsigned long stringSize,
                                             bool isUnicodeString, gtString& readString) const;

    bool injectMainThreadEntryPointBreakPoint();
    bool restoreMainThreadEntryPointOpCode();
    bool setDebuggedProcessDLLDirectory(const osFilePath& dllDirectoryPath);

    bool getDebuggedProcessDLLName(osInstructionPointer dllBaseAddressInDPAS,
                                   LPVOID pPtrLoadedDLLPathInDPAS, bool isUnicodeStringPath,
                                   gtString& dllName) const;
    bool deviceNameToFileName(const gtString& deviceName, gtString& fileName) const;
    bool setDebuggedProcessEnvVariables();
    bool addDebuggedProcessWorkDirToPath();
    bool removeDebuggedProcessEnvVariables();
    bool removeThreadFromThreadsList(DWORD threadId);

    // Functions that are used by both threads:
    // ----------------------------------------
    bool suspendDebuggedProcess(osThreadId triggeringThreadId);
    bool suspendDebuggedProcessThread(osThreadHandle threadHandle);
    bool resumeDebuggedProcessThread(osThreadHandle threadHandle);
    bool storeThreadExecutionContext(osThreadId threadId);
    bool restoreThreadExecutionContext(osThreadId threadId);
    HANDLE threadIdToThreadHandle(DWORD threadId);
    bool isSpyAPIThread(osThreadId threadId) const;

private:
    // A thread that runs the current debugging session:
    pdWin32DebuggerThread* _pDebuggerThread;

    // Contains true iff a debugged process exists:
    // (It was launched and not terminated yet)
    bool _debuggedProcessExist;

    // Contains true iff the debugged process run is suspended:
    // (The debugged process hit a breakpoint / etc)
    bool _isDebuggedProcssSuspended;

    // Contains true iff the debugged process is a 64-bit process:
    bool _isDebugging64BitApplication;

    // Contains true iff we hit the breakpoint that is thrown somewhere after the
    // process creation:
    bool _wasProcessCreationBreakPointHit;

    // Contains true iff the main thread entry point breakpoint was hit.
    bool _wasMainThreadEntryPointBreakpointHit;

    // Contains true iff the debugged process threw a second change exception,
    // and we didn't kill it yet.
    bool _isDuringSecondChangeExceptionHandling;

    // Contains true iff we are during debugged process termination:
    bool _isDuringDebuggedProcessTermination;

    // Holds the process creation data:
    apDebugProjectSettings* _pProcessCreationData;

    // Holds the debugged process PROCESS_INFORMATION structure:
    PROCESS_INFORMATION* _pProcessInfo;

    // Holds a list of debugged process loaded modules:
    pdWindowsLoadedModulesManager _loadedModulesManager;

    // Index for the unknown loaded dlls:
    int _unknownDllIndex;

    // The debugged process threads data:
    gtList<pdDebuggedProcessThreadData> _debuggedProcessThreadsData;

    // A mutex that coordinates access to _debuggedProcessThreadsData:
    // (the list is accessed by both the application thread and the debugger thread)
    osMutex _threadsListAccessMutex;

    // Contains true iff we need to update the threads data:
    bool _needToUpdateThreadsData;

    // The spies API thread id:
    osThreadId _spiesAPIThreadId;

    // The main thread id:
    DWORD _mainThreadId;

    // The main thread entry point address:
    LPTHREAD_START_ROUTINE _mainThreadEntryPointAddress;

    // The main thread entry point Op code:
    BYTE  _mainThreadEntryPointOpCode;

    // Contains a stored thread context (and the thread id):
    osThreadId _idOfThreadWithStoredContext;
    CONTEXT _storedThreadContext;

    // Contains true iff we are waiting for a function executed by
    // makeThreadExecuteFunction() to end (and throw its end breakpoint exception).
    bool _waitingForExecutedFunction;

    // Contains true iff we are waiting for createDebuggedProcess to create the debugged
    // process via ::CreateProcess.
    bool _waitingForDebuggedProcessCreation;

    // Contains true iff we are during the wait at two-step process debugging:
    bool _waitingAtDebuggedProcessLaunch;

    // Contains true iff the executed function crashed:
    // (A crash happened in the debugged process while in makeThreadExecuteFunction())
    bool _executedFunctionCrashed;

    // Flags related to the various steps in kernel debugging::
    bool _isKernelDebuggingAboutToStart;
    bool _isDuringKernelDebugging;
    bool _isKernelDebuggingJustFinished;

    // Flag related to ignore of AMD_OCL_BUILD_OPTIONS content:
    bool _isIgnoringAMD_OCL_BUILD_OPTIONS;

    // A synchronization object that disables doing some operations when running
    // a debugged process executed function (See makeThreadExecuteFunction())
    osSynchronizationObject _executedFuncSyncObj;

    // Stores current process environment variables values:
    // (See setDebuggedProcessEnvVariables for more details)
    gtList<osEnvironmentVariable> _storedEnvironmentVariablesValues;

    // Contains true iff we are running Windows 7 and a 64-bit process debugger
    // (i.e. inside the remote debugging server):
    bool _isWindows7With64Bit;

    // The thread that triggered the breakpoint which we are currently suspended on:
    osThreadId _breakpointTriggeringThreadId;

    // Amount of output string printouts:
    int _amountOfOutputStringPrintouts;

    // redirection files
    osProcessSharedFile m_outputFile;
    osProcessSharedFile m_inputFile;
};


#endif  // __PDWIN32PROCESSDEBUGGER
