//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdGDBOutputReader.h
///
//==================================================================================

//------------------------------ pdGDBOutputReader.h ------------------------------

#ifndef __PDGDBOUTPUTREADER
#define __PDGDBOUTPUTREADER

// Forward decelerations:
class osPipeSocket;
class pdGDBDriver;
class apExpression;

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osModuleArchitecture.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// Local:
#include <src/pdGDBCommandInfo.h>
#include <src/pdGDBDataStructs.h>


// ----------------------------------------------------------------------------------
// Class Name:           pdGDBOutputReader
// General Description:
//   Reads GDB output and acts accordingly.
//
// Author:               Yaki Tebeka
// Creation Date:        25/12/2006
// ----------------------------------------------------------------------------------
class pdGDBOutputReader
{
public:
    pdGDBOutputReader();
    virtual ~pdGDBOutputReader();

    void initialize();

    bool readGDBOutput(osPipeSocket& gdbCommunicationPipe, pdGDBDriver& GDBDriver, pdGDBCommandId executedGDBCommandId,
                       bool& wasDebuggedProcessSuspended, bool& wasDebuggedProcessTerminated, const pdGDBData** ppGDBOutputData = NULL);

    void getGDBOutput(gtAutoPtr<pdGDBData>& aptrGDBData) { aptrGDBData = _aptrGDBOutputData; };

    void waitForInternalDebuggedProcessInterrupt() { _isWaitingForInternalDebuggedProcessInterrupt = true; };
    void kernelDebuggingAboutToStart() {_isKernelDebuggingAboutToStart = true;};
    void kernelDebuggingJustFinished() {_isKernelDebuggingJustFinished = true;};

    // Helper function for handleGetLibraryAtAddressOutput()
    void setInstructionAddressToFind(osInstructionPointer findAddress) {_findAddress = findAddress;};

    // Pointer width field accessor
    void setModuleArchitecture(osModuleArchitecture arch) {_debuggedExecutableArchitecture = arch;};

    bool flushGDBPrompt();
    void resetGDBPrompt() { _wasGDBPrompt = false; }
    bool readGDBOutputLine(gtASCIIString& gdbOutputLine);

private:
    bool readGDBOutput(gtASCIIString& gdbOutputString);
    bool readSynchronousCommandGDBOutput(gtASCIIString& gdbOutputString);
    bool readAsynchronousCommandGDBOutput(gtASCIIString& gdbOutputString);
    bool parseGDBOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);
    bool parseGeneralGDBOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);
    bool parseGeneralGDBOutputLine(const gtASCIIString& gdbOutputLine);

    bool handleGetThreadsInfoOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);
    bool handleThreadsInfoLineOutput(const gtASCIIString& gdbOutputLine, pdGDBThreadDataList& outputThreadsList);
    bool handleGetThreadsInfoViaMachineInterfaceOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);
    bool handleThreadsInfoViaMachineInterfaceLineOutput(const gtASCIIString& gdbOutputLine, pdGDBThreadDataList& outputThreadsList);
    osThreadId getThreadIdInDebuggedApplicationAddressSpace(int gdbThreadId) const;
    bool handleGetThreadInfoOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);
    bool handleGDBResultOutput(const gtASCIIString& gdbOutputLine);
    bool handleGetCurrThreadCallStackOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);
    bool handleGetExecutablePidOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);
    bool handleGetSymbolAtAddressOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);
    bool handleGetDebugInfoAtAddressOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);
    bool handleGetDebugHumanInfoAtAddressOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);
    bool handleGetLibraryAtAddressOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);
    bool handleAbortDebuggedProcessOutput(const gtASCIIString& gdbOutputString);
    bool handleWaitingForDebuggedProcessOutput(const gtASCIIString& gdbOutputString);
    bool addFrameDataToCallStack(const gtASCIIString& frameDataString, osCallStack& callStack);
    void markSpyFrames(osCallStackFrame& callStackFrame);
    bool handleGDBConsoleOutput(const gtASCIIString& gdbOutputLine);
    bool handleDebuggedProcessOutput(const gtASCIIString& gdbOutputLine);
    bool handleGDBInternalOutput(const gtASCIIString& gdbOutputLine);
    bool handleAsynchronousOutput(const gtASCIIString& gdbOutputLine);
    bool handleStatusAsynchronousOutput(const gtASCIIString& gdbOutputLine);
    bool handleUnknownGDBOutput(const gtASCIIString& gdbOutputLine);
    bool handleGetVariableTypeGDBOutput(const gtASCIIString& gdbOutputLine, const pdGDBData** ppGDBOutputData);
    bool handleHostSteps(const gtASCIIString& gdbOutputLine, const pdGDBData** ppGDBOutputData);

    bool getStopReasonString(const gtASCIIString& gdbOutputLine, gtASCIIString& stopReasonString);
    bool isAnyThreadStoppedInSpy();
    bool isCurrentThreadStoppedInsideSpy(bool& wasStoppedInSpy);
    void handleNewThreadMessage(const gtASCIIString& gdbOutputLine);
    void handleExitThreadMessage(const gtASCIIString& gdbOutputLine);
    void handleSwitchingToThreadMessage(const gtASCIIString& gdbOutputLine);
    void handleSwitchingToProcessMessage(const gtASCIIString& gdbOutputLine);
    void handleSwitchingToProcessAndThreadMessage(const gtASCIIString& gdbOutputLine);
    void handleSignalOutput(const gtASCIIString& gdbOutputLine, bool& wasBreakpointHit);
    bool handleBreakpointHit(bool isGDBBreakpoint = false, bool isStep = false);
    void handleException(int excetionReason);
    bool handleSharedModuleLoadedMessage(const gtASCIIString& gdbOutputLine);
    bool handleSharedModuleUnloadedMessage(const gtASCIIString& gdbOutputLine);
    void outputParsingGDBOutputLogMessage(pdGDBCommandId executedGDBCommandId, const gtASCIIString& gdbOutputString);
    void outputEndedParsingGDBOutputLogMessage(const gtASCIIString& gdbOutputString);
    void outputGeneralLineLogMessage(const gtASCIIString& gdbOutputLine);
    void outputThreadLineLogMessage(const gtASCIIString& gdbOutputLine);
    void outputCallStackLogMessage(const gtASCIIString& gdbOutputString);
    void outputCallStackLineLogMessage(const gtASCIIString& gdbOutputString);

    bool isExecutingSynchronousCommand() const;
    void initMembers();

    osThreadId threadIdFromGDBId(int threadGDBId);

    /////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Process GDB's output on suspend process command
    ///
    /// \param[in] gdbOutputString a GDB answer
    ///
    /// \return true - command success/false - GDB return error
    bool handleSuspendProcess(const gtASCIIString& gdbOutputString);

    /////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Process GDB's output on resume process command
    ///
    /// \param[in] gdbOutputString a GDB answer
    ///
    /// \return true - command success/false - GDB return error
    bool handleResumeProcess(const gtASCIIString& gdbOutputString);

    /////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Process GDB's output on switch to thread command
    ///
    /// \param[in] gdbOutputString a GDB answer
    ///
    /// \return true - command success/false - GDB return error
    bool handleSwitchToThread(const gtASCIIString& gdbOutputString);

    /////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Process GDB's output on switch to frame command
    ///
    /// \param[in] gdbOutputString a GDB answer
    ///
    /// \return true - command success/false - GDB return error
    bool handleSwitchToFrame(const gtASCIIString& gdbOutputString);

    /////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Process GDB's output on get frame locals variables
    ///
    /// \param[in]  gdbOutputString a GDB answer
    /// \param[out] ppGDBOutputData a pointer to
    ///
    /// \return true - command success/false - GDB return error
    bool handleGetLocalsFrame(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);

    /////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Process GDB's output on get variable value
    ///
    /// \param[in]  gdbOutputString a GDB answer
    /// \param[out] ppGDBOutputData a pointer to
    ///
    /// \return true - command success/false - GDB return error
    bool handleGetVariable(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);

    /////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Process GDB's output on set breakpoint
    ///
    /// \param[in]  gdbOutputString a GDB answer
    /// \param[out] ppGDBOutputData a pointer to
    ///
    /// \return true - command success/false - GDB return error
    bool handleSetBreakpointOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData);

    /////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Process GDB's output on "step in" command
    ///
    /// \param[in]  gdbOutputString a GDB answer
    /// \param[out] ppGDBOutputData a pointer to
    ///
    /// \return true - command success/false - GDB return error
    bool handleStepIn(const gtASCIIString& gdbOutputString);

    /////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Process GDB's output on "step over" command
    ///
    /// \param[in]  gdbOutputString a GDB answer
    /// \param[out] ppGDBOutputData a pointer to
    ///
    /// \return true - command success/false - GDB return error
    bool handleStepOver(const gtASCIIString& gdbOutputString);

    /////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Process GDB's output on "step out" command
    ///
    /// \param[in]  gdbOutputString a GDB answer
    /// \param[out] ppGDBOutputData a pointer to
    ///
    /// \return true - command success/false - GDB return error
    bool handleStepOut(const gtASCIIString& gdbOutputString);

    //////////////////////////////////////////////////////////////////
    /// \brief Parsing "*stopped..." gdb output line and find a stopped thread id
    ///
    /// \param gdbOutputLine a line readed from gdb output pipe
    ///
    /// \return stoped thread Id, 0 - in case all threads stopped, -1 in case error
    ///
    /// \author Vadim Entov
    /// \date 14/1/2016
    int GetStoppedThreadGDBId(const gtASCIIString& gdbOutputLine);

    /////////////////////////////////////////////////////////////////
    /// \brief Parse "*running, thread-id="... " message
    ///
    /// \param gdbOutputLine a parsing gdb output string
    /// \return threadId or -1
    ///
    /// \author Vadim Entov
    /// \date 18/01/2016
    int GetRunningThreadGDBId(const gtASCIIString& gdbOutputLine);

    /////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Parse nested gdb values
    ///
    /// \param[in]  current value string
    ///
    /// \return vector of found childs
    std::vector<apExpression> parseVariableValueChilds(const std::string& value_str);

    void parseChildValue(const std::string& str, size_t & start_position, apExpression& parent);

    bool findValueName(const std::string& str, size_t & start_position);

private:
    // The executed GDB command:
    pdGDBCommandId _executedGDBCommandId;
    bool _executedGDBCommandRequiresFlush;

    // A GDB driver that can be used to drive GDB and perform additional actions:
    pdGDBDriver* _pGDBDriver;

    // GDB's communication pipe:
    osPipeSocket* _pGDBCommunicationPipe;
    osCriticalSection m_gdbPipeAccessCS;

    // Will contain GDB error strings:
    gtASCIIString _gdbErrorString;

    // Will contain true iff we read a GDBs output that told us that
    // the debugged process run was suspended:
    bool _wasDebuggedProcessSuspended;

    // Will contain true iff we read a GDBs output that told us that
    // the debugged process got a fatal signal output:
    bool m_didDebuggedProcessReceiveFatalSignal;

    // Will contain true iff we read a GDBs output that told us that
    // the debugged process run was terminated:
    bool _wasDebuggedProcessTerminated;

    // Will contain true iff we read a GDB output that told us that
    // the debugged process was created:
    bool _wasDebuggedProcessCreated;

    bool _wasGDBPrompt;

    // If the output requires events to be registered, make sure their registration does not interfere with the other members:
    gtPtrVector<apEvent*> m_eventsToRegister;

    // Contains GDB output data structures:
    gtAutoPtr<pdGDBData> _aptrGDBOutputData;

    // Contains the debugged process current thread id:
    int _debuggedProcessCurrentThreadGDBId;
    osThreadId _debuggedProcessCurrentThreadId;

    // Contains true iff we are waiting for an internal interrupt (SIGINT) to be
    // sent to the debugged process:
    bool _isWaitingForInternalDebuggedProcessInterrupt;

    // Flags related to the various steps in kernel debugging::
    bool _isKernelDebuggingAboutToStart;
    bool _isKernelDebuggingJustFinished;

    // Used to maintain the gdb thread number between the "*stopped" and "info threads" outputs on mac:
    int _currentThreadNumber;

    // Current process id
    int _processId;

    // Used for finding the correct line out of the "info sharedlibrary" output on Linux:
    osInstructionPointer _findAddress;

    // What architecture are we running under
    osModuleArchitecture _debuggedExecutableArchitecture;

    // Contain the amount of printed GDB strings:
    int _amountOfGDBStringPrintouts;
};

#endif  // __PDGDBOUTPUTREADER
