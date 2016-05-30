//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdGDBOutputReader.cpp
///
//==================================================================================

//------------------------------ pdGDBOutputReader.cpp ------------------------------

// Standard C:
#include <unistd.h>

// std
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <future>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtASCIIStringTokenizer.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osPipeSocket.h>
#include <AMDTOSWrappers/Include/osSystemError.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessOutputStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunResumedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunStartedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apGDBErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apGDBOutputStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleLoadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleUnloadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOutputDebugStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>


// Local:
#include <src/pdStringConstants.h>
#include <src/pdGDBCommandInfo.h>
#include <src/pdGDBDriver.h>
#include <src/pdGDBOutputReader.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>

// The length of the gdb printouts aid buffer:
#define PD_GDB_PRINTOUTS_BUFF_LENGTH 4098

// GDB strings:
static const gtASCIIString s_gdbPromtStr = "(gdb)";
static const gtASCIIString s_gdbOperationSuccessStr = "^done";
static const gtASCIIString s_debuggedProcessStoppedMsg = "*stopped";
static const gtASCIIString s_runningThreadMsg = "*running,thread-id=\"";
static const gtASCIIString s_stoppedThreads = "stopped-threads=[\"";
static const gtASCIIString s_debuggedProcessRunningMsg = "*running";
static const gtASCIIString s_exitedGDBStr = "exited";
static const gtASCIIString s_newThreadMsg1 = "~\"[New Thread ";
static const gtASCIIString s_newThreadMsg2 = "=thread-created";
static const gtASCIIString s_newThreadMsg = "~\"[New Thread ";
static const gtASCIIString s_exitingThread = "=thread-exited,id=\"";
static const gtASCIIString s_switchingToThreadMsg = "~\"[Switching to Thread";
static const gtASCIIString s_switchingToProcessMsg = "~\"[Switching to process";
static const gtASCIIString s_switchingToProcessAndThread = " thread ";

// Maximal amount of GDB string printouts:
#define PD_MAX_GDB_STRING_PRINTOUTS 500
// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::pdGDBOutputReader
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        21/12/2006
// ---------------------------------------------------------------------------
pdGDBOutputReader::pdGDBOutputReader()
    : _executedGDBCommandId(PD_GDB_NULL_CMD),
      _executedGDBCommandRequiresFlush(false),
      _pGDBDriver(NULL),
      _pGDBCommunicationPipe(NULL),
      _wasDebuggedProcessSuspended(false),
      m_didDebuggedProcessReceiveFatalSignal(false),
      _wasDebuggedProcessTerminated(false),
      _wasDebuggedProcessCreated(false),
      _debuggedProcessCurrentThreadGDBId(-1),
      _debuggedProcessCurrentThreadId(OS_NO_THREAD_ID),
      _isWaitingForInternalDebuggedProcessInterrupt(false),
      _isKernelDebuggingAboutToStart(false),
      _isKernelDebuggingJustFinished(false),
      _currentThreadNumber(-1),
      _processId(0),
      _findAddress(NULL),
      _debuggedExecutableArchitecture(OS_UNKNOWN_ARCHITECTURE),
      _amountOfGDBStringPrintouts(0)
{
    initMembers();
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::~pdGDBOutputReader
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        21/12/2006
// ---------------------------------------------------------------------------
pdGDBOutputReader::~pdGDBOutputReader()
{
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::readGDBOutput
// Description: Does this class work - reads the current GDB output and
//              acts accordingly.
// Arguments: gdbCommunicationPipe - The GDB communication pipe.
//            pGDBDriver - A GDB driver that can be used to drive GDB and perform additional actions.
//            executedGDBCommandId - The last executed GDB command.
//            ppGDBOutputData - Will get GDB output data (if relevant to
//                              the executed GDB command).
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/12/2006
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::readGDBOutput(osPipeSocket& gdbCommunicationPipe, pdGDBDriver& GDBDriver, pdGDBCommandId executedGDBCommandId,
                                      bool& wasDebuggedProcessSuspended, bool& wasDebuggedProcessTerminated, const pdGDBData** ppGDBOutputData)
{
    bool retVal = false;

    // Prevent the GUI main thread and the pdGDBListenerThread from accessing this function at the same time:
    osCriticalSectionLocker gdbPipeAccessLocker(m_gdbPipeAccessCS);

    // Initialize this class members:
    initMembers();

    // Clear the output data pointer (if any):
    if (ppGDBOutputData != NULL)
    {
        *ppGDBOutputData = NULL;
    }

    // Log the executed GDB command:
    _executedGDBCommandId = executedGDBCommandId;
    _executedGDBCommandRequiresFlush = pdDoesCommandRequireFlush(_executedGDBCommandId);

    // Log the GDB driver:
    _pGDBDriver = &GDBDriver;

    // Log GDB's communication pipe:
    _pGDBCommunicationPipe = &gdbCommunicationPipe;

    // Read the gdb output:
    gtASCIIString gdbOutput;
    bool rc3 = readGDBOutput(gdbOutput);
    GT_IF_WITH_ASSERT(rc3)
    {
        // Parse the command results:
        bool rc4 = parseGDBOutput(gdbOutput, ppGDBOutputData);
        GT_IF_WITH_ASSERT(rc4)
        {
            retVal = true;
        }
    }

    // If the reading or parsing failed, assume no change was made:
    wasDebuggedProcessSuspended = retVal && _wasDebuggedProcessSuspended;
    wasDebuggedProcessTerminated = retVal && _wasDebuggedProcessTerminated;

    // Register all the events for this command at once. This is done to allow event registration
    // observers to use the GDB driver without:
    // 1. Causing feedback loops
    // 2. Corrupting this class's members (see BUG 356150)
    gtPtrVector<apEvent*> eventsToRegister = m_eventsToRegister;

    // Clear the events:
    m_eventsToRegister.clear();

    int numberOfEvents = (int)eventsToRegister.size();

    for (int i = 0; numberOfEvents > i; i++)
    {
        // Register the event:
        apEventsHandler::instance().registerPendingDebugEvent(*(eventsToRegister[i]));
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::readGDBOutput
// Description: Reads the current GDB output string.
// Arguments:   gdbOutputString - Will get GDB's current output string.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/12/2006
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::readGDBOutput(gtASCIIString& gdbOutputString)
{
    bool retVal = false;

    // If we are executing a synchronous command:
    bool isSyncCommand = isExecutingSynchronousCommand();

    if (isSyncCommand)
    {
        retVal = readSynchronousCommandGDBOutput(gdbOutputString);
    }
    else
    {
        retVal = readAsynchronousCommandGDBOutput(gdbOutputString);
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::readSynchronousCommandGDBOutput
// Description: Reads GDB output string resulting from the run of a synchronous GDB command.
// Arguments:   gdbOutputString - Will GDB's output string.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/12/2008
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::readSynchronousCommandGDBOutput(gtASCIIString& gdbOutputString)
{
    bool retVal = false;
    gdbOutputString.makeEmpty();

    bool goOn = true;

    if (_wasGDBPrompt)
    {
        //_wasGDBPrompt = false;
        return true;
    }

    while (goOn)
    {
        // Read printouts chunk from gdb's output stream:
        static char buff[PD_GDB_PRINTOUTS_BUFF_LENGTH];
        gtSize_t bytesRead = 0;
        bool rc1 = _pGDBCommunicationPipe->readAvailableData(buff, PD_GDB_PRINTOUTS_BUFF_LENGTH, bytesRead);

        if (!rc1)
        {
            GT_ASSERT(rc1);
            goOn = false;
        }
        else
        {
            // Add the read buffer into the output string:
            buff[bytesRead] = 0;
            gdbOutputString += buff;

            // If under debug log severity:
            if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
            {
                gtString dbgMsg;
                dbgMsg.fromUtf8String(buff);
                dbgMsg.prepend(L"GDB Read sync line row data: ");
                OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
            }

            // If we finished reading all currently available gdb printouts:
            if (bytesRead < PD_GDB_PRINTOUTS_BUFF_LENGTH)
            {
                // Verify that we got the gdb prompt that ends all synchronous commands:
                bool gotGDBPrompt = (gdbOutputString.find(s_gdbPromtStr) != -1);

                if (gotGDBPrompt)
                {
                    _wasGDBPrompt = true;

                    if (_executedGDBCommandRequiresFlush)
                    {
                        // Uri, 28/12/09 - We add a "\n" to gdb on certain commands that execute slow. This causes the output to be:
                        // ^done, XXXXXXXX
                        //  XXXXXX
                        //  ...
                        // (gdb)
                        // &"\n"
                        // ^done
                        // (gdb)
                        // and would sometimes not be read entirely by the command itself (as this loop might stop on the first (gdb) ).
                        // The output would then flow over to the next command, causing all sorts of trouble (see Geo Paradigm emails from Nov. - Dec. 2009).
                        // If we get only one gdb prompt with a &"\n" (echo of the flush string), we wait for another one (in fact, we wait for as many as needed):
                        static const gtASCIIString flushCommandOutput = "&\"\\n\"";
                        int numberOfFlushEchoes = gdbOutputString.count(flushCommandOutput);
                        int numberOfGDBPrompts = gdbOutputString.count(s_gdbPromtStr);

                        if (((numberOfFlushEchoes + 1) == numberOfGDBPrompts) && (numberOfFlushEchoes > 0))
                        {
                            retVal = true;
                            goOn = false;
                        }
                    }
                    else // !_executedGDBCommandRequiresFlush
                    {
                        retVal = true;
                        goOn = false;
                    }
                }
                else // !gotGDBPrompt
                {
                    // If we got the "failed to launch gdb" string:
                    bool failedToLaunchGDB = (gdbOutputString.find(PD_STR_failedToLaunchGDBASCII) != -1);

                    if (failedToLaunchGDB)
                    {
                        retVal = true;
                        goOn = false;
                    }
                    else // failedToLaunchGDB
                    {
                        // Wait 1 millisecond to release the CPU and let gdb output the rest of its output:
                        osSleep(1);

                        // Go on for another read loop:
                        goOn = true;
                    }

                    //                    handleGDBConsoleOutput(gdbOutputString);
                }
            }
        }
    }

    GetStoppedThreadGDBId(gdbOutputString);
    GetRunningThreadGDBId(gdbOutputString);

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::readAsynchronousCommandGDBOutput
// Description: Reads the next GDB asynchronous output. Use this function for reading
//              GDB outputs of GDB asynchronous commands.
// Arguments:   gdbOutputString - Will get GDB's current output line.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/12/2008
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::readAsynchronousCommandGDBOutput(gtASCIIString& gdbOutputString)
{
    bool retVal = false;

    // Read the next GDB output line:
    bool rc1 = readGDBOutputLine(gdbOutputString);

    // If under debug log severity:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        gtString dbgMsg;
        dbgMsg.fromUtf8String(gdbOutputString.asCharArray());
        dbgMsg.prepend(L"GDB Read async line row data: ");
        OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    if (gdbOutputString.find(s_gdbPromtStr) != -1)
    {
        _wasGDBPrompt = true;
    }

    if (rc1)
    {
        retVal = true;

        // If the debugged process run was suspended or terminated:
        if (gdbOutputString.find(s_debuggedProcessStoppedMsg) != -1)
        {
            // If we didn't read the "(gdb)" prompt yet:
            if (gdbOutputString.find(s_gdbPromtStr) == -1)
            {
                OS_OUTPUT_DEBUG_LOG(PD_STR_waitingForGDBCommandPrompt, OS_DEBUG_LOG_DEBUG);

                // The "(gdb)" prompt is waiting for us in the pipe - read it:
                gtASCIIString commandPromptString;

                if (gdbOutputString.find("SIGINT"))
                {
                    GetStoppedThreadGDBId(gdbOutputString);
                }
            }
        }

        if (gdbOutputString.find("=thread-exited,id=\"") != -1)
        {
            handleExitThreadMessage(gdbOutputString);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::readGDBOutputLine
// Description: Reads the next GDB output line.
// Arguments:   gdbOutputLine - Will get GDB's current output line.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/12/2008
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::readGDBOutputLine(gtASCIIString& gdbOutputLine)
{
    bool retVal = false;
    gdbOutputLine.makeEmpty();

    bool goOn = true;

    while (goOn)
    {
        // A null terminated buffer that will receive the read chars:
        static char buff[2];
        buff[1] = 0;

        // Read one char from GDB's current output:
        gtSize_t bytesRead = 0;
        bool rc1 = _pGDBCommunicationPipe->readAvailableData(buff, 1, bytesRead);

        if (!rc1)
        {
            GT_ASSERT(rc1);
            goOn = false;
        }
        else
        {
            // If we finished reading an entire GDB output line (terminated by a new line):
            if (buff[0] == '\n')
            {
                retVal = true;
                goOn = false;
            }
            else
            {
                // Add the read char into the output string:
                gdbOutputLine += buff;

                // Go on for another read loop:
                goOn = true;
            }
        }
    }

    // Check if some thread exited or stopped
    GetStoppedThreadGDBId(gdbOutputLine);

    // Check if some thread created or start running
    GetRunningThreadGDBId(gdbOutputLine);

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::parseGDBOutput
// Description:
//   Parses an input GDB output string. This string may contain multiple lines.
//
// Arguments: gdbOutputString - The output string to be parsed.
//            ppGDBOutputData - Will get GDB output data (if relevant to
//                              the executed GDB command).
// Return Val:  bool - false - iff a GDB error / internal error occurred.
// Author:      Yaki Tebeka
// Date:        26/12/2006
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::parseGDBOutput(const gtASCIIString& gdbOutputString,
                                       const pdGDBData** ppGDBOutputData)
{
    bool retVal = false;

    // Output debug log message:
    outputParsingGDBOutputLogMessage(_executedGDBCommandId, gdbOutputString);

    switch (_executedGDBCommandId)
    {
        case PD_SET_ENV_VARIABLE_CMD:
        {
            retVal = gdbOutputString.find("^done") != -1;
        }
        break;

        case PD_GDB_SET_BREAKPOINT_CMD:
        {
            retVal = handleSetBreakpointOutput(gdbOutputString, ppGDBOutputData);
        }
        break;

        case PD_CONTINUE_THREAD_CMD:
        {
            retVal = parseGeneralGDBOutput(gdbOutputString, ppGDBOutputData);
        }
        break;

        case PD_GET_THREADS_INFO_CMD:
        {
            // Executing the "get threads info" command:
            retVal = handleGetThreadsInfoOutput(gdbOutputString, ppGDBOutputData);
        }
        break;

        case PD_GET_THREADS_INFO_VIA_MI_CMD:
        {
            // Executing the "get threads info" command:
            retVal = handleGetThreadsInfoViaMachineInterfaceOutput(gdbOutputString, ppGDBOutputData);
        }
        break;

        case PD_GET_THREAD_INFO_CMD:
        {
            // Executing the "get thread info" command:
            retVal = handleGetThreadInfoOutput(gdbOutputString, ppGDBOutputData);
        }
        break;

        case PD_GET_CUR_THREAD_CALL_STACK_CMD:
        {
            // Executing the "get current thread call stack" command:
            retVal = handleGetCurrThreadCallStackOutput(gdbOutputString, ppGDBOutputData);
        }
        break;

        case PD_GET_EXECUTABLE_PID_CMD:
        {
            // Executing the "get debugged process pid" command:
            retVal = handleGetExecutablePidOutput(gdbOutputString, ppGDBOutputData);
        }
        break;

        case PD_GET_SYMBOL_AT_ADDRESS:
        {
            // Executing the "get symbol name at address" command:
            retVal = handleGetSymbolAtAddressOutput(gdbOutputString, ppGDBOutputData);
        }
        break;

        case PD_GET_DEBUG_INFO_AT_ADDRESS:
        {
            bool retValHuman = handleGetDebugHumanInfoAtAddressOutput(gdbOutputString, ppGDBOutputData);
            bool retValMachine = handleGetDebugInfoAtAddressOutput(gdbOutputString, ppGDBOutputData);
            retVal = retValHuman || retValMachine;
        }
        break;

        case PD_GET_LIBRARY_AT_ADDRESS:
        {
            retVal = handleGetLibraryAtAddressOutput(gdbOutputString, ppGDBOutputData);
        }
        break;

        case PD_GDB_ABORT_DEBUGGED_PROCESS_CMD:
        {
            // Executing the "kill" command:
            retVal = handleAbortDebuggedProcessOutput(gdbOutputString);
        }
        break;

        case PD_GDB_WAIT_FOR_PROCESS_CMD:
        {
            retVal = handleWaitingForDebuggedProcessOutput(gdbOutputString);
        }
        break;

        case PD_GDB_SUSPEND_DEBUGGED_PROCESS_CMD:
        {
            retVal = handleSuspendProcess(gdbOutputString);
        }
        break;

        case PD_GDB_RESUME_DEBUGGED_PROCESS_CMD:
        {
            retVal = handleResumeProcess(gdbOutputString);
        }
        break;

        case PD_SET_ACTIVE_FRAME_CMD:
        {
            retVal = handleSwitchToFrame(gdbOutputString);
        }
        break;

        case PD_GET_LOCALS_INFO_CMD:
        {
            retVal = handleGetLocalsFrame(gdbOutputString, ppGDBOutputData);
        }
        break;

        case PD_GET_LOCAL_VARIABLE_CMD:
        {
            retVal = handleGetVariable(gdbOutputString, ppGDBOutputData);
        }
        break;

        case PD_GET_VARIABLE_TYPE_CMD:
        {
            retVal = handleGetVariableTypeGDBOutput(gdbOutputString, ppGDBOutputData);
        }
        break;

        case PD_GDB_STEP_INTO_CMD:
        case PD_GDB_STEP_OVER_CMD:
        case PD_GDB_STEP_OUT_CMD:
        case PD_GDB_UNTIL_CMD:
        {
            retVal = handleHostSteps(gdbOutputString, ppGDBOutputData);
        }
        break;

        default:
        {
            // Executing any other command:
            retVal = parseGeneralGDBOutput(gdbOutputString, ppGDBOutputData);
        }
        break;
    }

    // Output debug log message:
    outputEndedParsingGDBOutputLogMessage(gdbOutputString);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::parseGeneralGDBOutput
// Description:
//   Parses a "general" GDB output string.
//   A "general" GDB output is a GDB output that is the result of
//   ruuning a GDB command that doe not require specific output parsing.
//   This string may contain multiple lines.
//
// Arguments: gdbOutputString - The output string to be parsed.
//            ppGDBOutputData - Will get GDB output data (if relevant to
//                              the executed GDB command).
// Return Val:  bool - false - iff a GDB error / internal error occurred.
// Author:      Yaki Tebeka
// Date:        26/12/2006
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::parseGeneralGDBOutput(const gtASCIIString& gdbOutputString,
                                              const pdGDBData** ppGDBOutputData)
{
    (void)(ppGDBOutputData); // unused
    bool retVal = true;

    // Parse the output line by line:
    gtASCIIString currentOutputLine;
    int currPos = 0;
    int gdbPrintoutsLen = gdbOutputString.length();

    while (currPos < gdbPrintoutsLen)
    {
        currentOutputLine.makeEmpty();

        // Search for the current line delimiter:
        int lineDelimiterPos = gdbOutputString.find('\n', currPos);

        // If we found a line delimiter:
        if (lineDelimiterPos != -1)
        {
            gdbOutputString.getSubString(currPos,  lineDelimiterPos, currentOutputLine);
            currPos = lineDelimiterPos + 1;
        }
        else
        {
            gdbOutputString.getSubString(currPos,  gdbPrintoutsLen - 1, currentOutputLine);
            currPos = gdbPrintoutsLen;
        }

        // Parse the current GDB output line:
        bool rc = parseGeneralGDBOutputLine(currentOutputLine);
        retVal = retVal && rc;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::parseGeneralGDBOutputLine
// Description:
//   Parses an input GDB output string. This string may contain multiple lines.
// Arguments: gdbOutputLine - A GDB single output line to be parsed.
// Return Val:  bool - false - iff the printout contains a gdb reported error.
// Author:      Yaki Tebeka
// Date:        19/12/2006
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::parseGeneralGDBOutputLine(const gtASCIIString& gdbOutputLine)
{
    bool retVal = true;

    // If under debug log severity, output debug printout:
    outputGeneralLineLogMessage(gdbOutputLine);

    char outputType = gdbOutputLine[0];

    switch (outputType)
    {
        case '^':
        {
            retVal = handleGDBResultOutput(gdbOutputLine);
        }
        break;

        case '~':
        {
            // This is a GDB console output:
            retVal = handleGDBConsoleOutput(gdbOutputLine);
        }
        break;

        case '@':
        {
            // This is a debugged process output:
            retVal = handleDebuggedProcessOutput(gdbOutputLine);
        }
        break;

        case '&':
        {
            // This is a GDB internal output:
            retVal = handleGDBInternalOutput(gdbOutputLine);
        }
        break;

        case '*':
        {
            // This is "execution asynchronous output" notification:
            retVal = handleAsynchronousOutput(gdbOutputLine);
        }
        break;

        case '=':
        {
            // This is "status asynchronous output" notification:
            retVal = handleStatusAsynchronousOutput(gdbOutputLine);
        }
        break;

        case '(':
        {
            // If we got the GDB prompt:
            if (gdbOutputLine.startsWith(s_gdbPromtStr))
            {
                // Nothing to be done.
            }
            else
            {
                // Unknown output type:
                retVal = handleUnknownGDBOutput(gdbOutputLine);
            }
        }
        break;

        default:
        {
            // The current GDB line does not contain any prefix. This usually happens when
            // the debugged process writes outputs.
            retVal = handleDebuggedProcessOutput(gdbOutputLine);
        }
        break;
    }

    return retVal;
}




// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleGetThreadsInfoOutput
// Description:
//   Parses and handles a GDB output string that is the result of
//   an "info threads" GDB command.
// Arguments: gdbOutputLine - The GDB output line.
// Return Val:  bool - false - iff an internal / a GDB error occurred.
// Author:      Yaki Tebeka
// Date:        11/1/2007
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleGetThreadsInfoOutput(const gtASCIIString& gdbOutputString,
                                                   const pdGDBData** ppGDBOutputData)
{
    bool retVal = false;

    // Create a data structure that will hold the threads list:
    pdGDBThreadDataList* pThreadsList = new pdGDBThreadDataList;
    GT_IF_WITH_ASSERT(pThreadsList != NULL)
    {
        retVal = true;

        // Parse the output line by line:
        gtASCIIString currentOutputLine;
        int currPos = 0;
        int gdbPrintoutsLen = gdbOutputString.length();

        while (currPos < gdbPrintoutsLen)
        {
            currentOutputLine.makeEmpty();

            // Search for the current line delimiter:
            int lineDelimiterPos = gdbOutputString.find('\n', currPos);

            // If we found a line delimiter:
            if (lineDelimiterPos != -1)
            {
                gdbOutputString.getSubString(currPos, lineDelimiterPos, currentOutputLine);
                currPos = lineDelimiterPos + 1;
            }
            else
            {
                gdbOutputString.getSubString(currPos, gdbPrintoutsLen - 1, currentOutputLine);
                currPos = gdbPrintoutsLen;
            }

            // Parse the current GDB output line:
            bool rc3 = handleThreadsInfoLineOutput(currentOutputLine, *pThreadsList);
            retVal = retVal && rc3;
        }

        if (retVal || pThreadsList->_threadsDataList.size())
        {
            if (ppGDBOutputData != NULL)
            {
                // Output the threads list:
                *ppGDBOutputData = pThreadsList;
                retVal = true;
            }
        }
        else
        {
            // Failure cleanup:
            delete pThreadsList;
        }

        _currentThreadNumber = -1;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleThreadsInfoLineOutput
//   Parses and handles a single GDB output line that is the result of
//   an "info threads" GDB command.
// Arguments: gdbOutputLine - The GDB output line.
//            outputThreadsList - Threads list to be filled up.
// Return Val:  bool - false - iff an internal / a GDB error occurred.
// Author:      Yaki Tebeka
// Date:        11/1/2007
//
// Implementation notes:
//
//   On Linux, An example result of an "info threads" GDB command may be:
//  &"info threads\n"
//  ~"\n"
//  ~"  2 Thread 1084229952 (LWP 3379)  "
//  ~"* 1 Thread 46912520689776 (LWP 3378)  "
//  ^done,frame={addr="0x00000030e89097e6",func="pthread_cond_wait@@GLIBC_2.3.2",args=[],from="/lib64/libpthread.so.0"},frame={addr="0x00000030e74c3086",func="poll",args=[],from="/lib64/libc.so.6"}
//  (gdb)
//
//  On Mac OS X, An example result of an "info threads" GDB command may be:
//  ~"  2 process 1671 thread 0x2403  "
//  ~"* 1 process 1671 local thread 0x2f33  "
// ^done,frame={addr="0x900a2eee",fp="0xb0122ea4",func="__semwait_signal",args=[]},frame={addr="0x900eb672",fp="0xb00a0d04",func="select$DARWIN_EXTSN"
//  ,args=[]},frame={addr="0x00000000",fp="0xbffff074",func="??",args=[]}
// (gdb)
//  Furthermore:
//  - As of XCode tools 3.1.2 (gdb v 6.3.50 / apple version gdb-962), the format is indeed more machine readable, and is only a single line:
// ^done,threadno="4",target_tid="process 10583 thread 0x5207",frame={addr="0x93ce83ae",fp="0xb01a4ea4",func="__semwait_signal",args=[]},threadno="3",target_tid="process 10583 thread 0x4d07",frame={addr="0x93ce11c6",fp="0xb0122e74",func="mach_msg_trap",args=[]},threadno="2" ...
// (gdb)
//  - As of XCode tools 3.2 (comes with Snow Leopard), gdb version 6.3.50-20050815 (Apple version gdb-1344), the format was changed to:
// ^done,threadno="4",target_tid="port# 0x2a03",frame={addr="0x9766a876",fp="0xb0184d20",func="select$DARWIN_EXTSN",args=[]},threadno="3",target_tid="port# 0x1903",frame={addr="0x976711a2",fp="0xb0102f70",func="__workq_kernreturn",args=[]},threadno="2",target_tid="port# 0x1503",frame={addr="0x9767210a",fp="0xb0080d60",func="kevent",args=[]},threadno="1",target_tid="port# 0x903",frame={addr="0x976ac972",fp="0xbffff160",func="__kill",args=[]}
// (gdb)
//  - As of XCode tools 3.2.3 (Comes with iPhone SDK 4.0), gdb version 6.3.50-20050815 (Apple version gdb-1469), the above formatted
//    output no longer contains thread ids. As a result, we need to use the machine interface command -thread-list-ids, see handleThreadsInfoViaMachineInterfaceLineOutput below.
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleThreadsInfoLineOutput(const gtASCIIString& gdbOutputLine,
                                                    pdGDBThreadDataList& outputThreadsList)
{
    bool retVal = true;
    bool threadDataFound = false;

    // Linux / old mac format
    static gtASCIIString stat_threadString1("Thread");
    static gtASCIIString stat_threadString2("thread");
    static gtASCIIString stat_threadString3("port#");
    static gtASCIIString stat_threadString4("process");
    static gtASCIIString stat_localString("local");
    static gtASCIIString stat_starString("*");
    static gtASCIIString stat_consultOutputStringPrefix("~");
    static gtASCIIString stat_executedCommandEcho;

#if (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    // New Mac format
    static gtASCIIString stat_threadNumberString("threadno");
    static gtASCIIString stat_threadTargetThreadIDString("target_tid");
    static gtASCIIString stat_threadProcessID("process");
#endif

    // Get the executed command echo:
    if (stat_executedCommandEcho.isEmpty())
    {
        const pdGDBCommandInfo* pCommandInfo = pdGetGDBCommandInfo(PD_GET_THREADS_INFO_CMD);
        GT_IF_WITH_ASSERT(pCommandInfo != NULL)
        {
            stat_executedCommandEcho = pCommandInfo->_commandExecutionString;
        }
    }

    // If under debug log severity, output debug log message:
    outputThreadLineLogMessage(gdbOutputLine);

    // Will get the current line's thread data:
    pdGDBThreadData currThreadData;

    // Look for "Thread" or "thread" or port#.
    // NOTICE: Look for a space after the Thread word, otherwise, a line like this -
    // "~    () from /lib64/libpthread.so.0\n"
    // (which could be produced by gdb broken line)
    // is threated as thread line:
    gtASCIIString tmp = stat_threadString1;
    tmp.append(" ");
    int pos1 = gdbOutputLine.find(tmp);

    if (pos1 == -1)
    {
        tmp = stat_threadString2;
        tmp.append(" ");
        pos1 = gdbOutputLine.find(tmp);

        if (pos1 == -1)
        {
            tmp = stat_threadString3;
            tmp.append(" ");
            pos1 = gdbOutputLine.find(tmp);

            if (pos1 == -1)
            {
                tmp = stat_threadString4;
                tmp.append(" ");

                pos1 = gdbOutputLine.find(tmp);
            }
        }
    }

    // If this is a thread related line:
    if (pos1 != -1)
    {
        // If this is not just an echo of the executed GDB command:
        if (gdbOutputLine.find(stat_executedCommandEcho) == -1)
        {
#if (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
            // Check if this is the new format:
            pos1 = gdbOutputLine.find(stat_threadNumberString);

            if (pos1 != -1)
            {
                // If we found at least one "threadno=".." ", this is the new format, which has all the threads in one line
                GT_ASSERT(outputThreadsList._threadsDataList.size() == 0);

                retVal = true;
                threadDataFound = false;
                gtASCIIString currentToken1;
                gtASCIIStringTokenizer strTokenizer1(gdbOutputLine, ",");
                pdGDBThreadData* pCurrentThreadData = NULL;

                while (strTokenizer1.getNextToken(currentToken1))
                {
                    if (currentToken1.startsWith(stat_threadNumberString))
                    {
                        // Store the previous thread in our list and create a new "thread data":
                        if (threadDataFound)
                        {
                            GT_IF_WITH_ASSERT(pCurrentThreadData != NULL)
                            {
                                outputThreadsList._threadsDataList.push_back(*pCurrentThreadData);
                                delete pCurrentThreadData;
                                pCurrentThreadData = new pdGDBThreadData;
                            }
                            else
                            {
                                // We found data, but no thread to match it...
                                retVal = false;
                            }
                        }
                        else
                        {
                            // We only allow this to happen during the first run
                            GT_IF_WITH_ASSERT(pCurrentThreadData == NULL)
                            {
                                pCurrentThreadData = new pdGDBThreadData;
                            }
                            else
                            {
                                // There is a thread with data we didn't find...
                                retVal = false;
                            }
                        }

                        // Mark that we don't yet have this thread's data:
                        threadDataFound = false;

                        // Make sure we have a thread data to work with:
                        GT_IF_WITH_ASSERT(pCurrentThreadData != NULL)
                        {
                            // Get the thread number (gdb index):
                            GT_IF_WITH_ASSERT(currentToken1.count('\"') == 2)
                            {
                                gtASCIIString threadIndexAsString = currentToken1;
                                int firstQuotePosition = threadIndexAsString.find('\"');
                                int secondQuotePosition = threadIndexAsString.find('\"', (firstQuotePosition + 1));
                                threadIndexAsString.truncate((firstQuotePosition + 1), (secondQuotePosition - 1));

                                unsigned long threadIndex = 0;
                                bool rcInd = threadIndexAsString.toUnsignedLongNumber(threadIndex);

                                GT_IF_WITH_ASSERT(rcInd)
                                {
                                    pCurrentThreadData->_gdbThreadId = threadIndex;

                                    if ((_currentThreadNumber > 0) && (threadIndex == (unsigned int)_currentThreadNumber))
                                    {
                                        pCurrentThreadData->_isGDBsActiveThread = true;
                                    }
                                }
                            }
                        }
                    }
                    else if (currentToken1.startsWith(stat_threadTargetThreadIDString))
                    {
                        // Make sure we have exactly two quotation marks:
                        GT_IF_WITH_ASSERT(currentToken1.count('\"') == 2)
                        {
                            bool isLocal = false;

                            gtASCIIString targetThreadIdInfo = currentToken1;
                            int firstQuotePosition = targetThreadIdInfo.find('\"');
                            int secondQuotePosition = targetThreadIdInfo.find('\"', (firstQuotePosition + 1));
                            targetThreadIdInfo.truncate((firstQuotePosition + 1), (secondQuotePosition - 1));

                            gtASCIIString currentToken2;
                            gtASCIIStringTokenizer strTokenizer2(targetThreadIdInfo, " ");

                            while (strTokenizer2.getNextToken(currentToken2))
                            {
                                if (currentToken2.startsWith(stat_threadProcessID))
                                {
                                    // skip this token and the next one, which is the process id
                                    strTokenizer2.getNextToken(currentToken2);
                                }
                                else if (currentToken2.startsWith(stat_localString))
                                {
                                    // This is a local thread
                                    isLocal = true;
                                }
                                else if (currentToken2.startsWith(stat_threadString2) || currentToken2.startsWith(stat_threadString3))
                                {
                                    // Threads ids marked with "port#" are Mach thread ids:
                                    /*
                                    if (currentToken2.startsWith(stat_threadString3) == true)
                                    {
                                        isLocal = true;
                                    }
                                    */

                                    // This is the thread number, make sure we got only one:
                                    GT_IF_WITH_ASSERT(!threadDataFound)
                                    {
                                        // Skip to the next token, the actual thread Id:
                                        strTokenizer2.getNextToken(currentToken2);
                                        unsigned long threadOSId = 0;
                                        bool rcNum = currentToken2.toUnsignedLongNumber(threadOSId);
                                        GT_IF_WITH_ASSERT(rcNum)
                                        {
                                            GT_IF_WITH_ASSERT(pCurrentThreadData != NULL)
                                            {
                                                if (isLocal)
                                                {
                                                    // Get the thread id in the debugged application address space:
                                                    osThreadId applicationThreadId = getThreadIdInDebuggedApplicationAddressSpace(pCurrentThreadData->_gdbThreadId);
                                                    GT_ASSERT(applicationThreadId != OS_NO_THREAD_ID);
                                                    pCurrentThreadData->_OSThreadId = applicationThreadId;
                                                }
                                                else
                                                {
                                                    pCurrentThreadData->_OSThreadId = (osThreadId)threadOSId;
                                                }
                                            }

                                            threadDataFound = true;
                                        }
                                    }
                                }
                                else
                                {
                                    // Unexpected token!
                                    GT_ASSERT_EX(false, L"Unexpected token inside target_tid");
                                }
                            }
                        }
                    }
                    else
                    {
                        // All other tokens (namely the thread frame and its subtokens) are ignored
                    }
                }

                // Store the last thread in our list:
                if (threadDataFound)
                {
                    GT_IF_WITH_ASSERT(pCurrentThreadData != NULL)
                    {
                        outputThreadsList._threadsDataList.push_back(*pCurrentThreadData);
                    }
                    else
                    {
                        // We found data, but no thread to match it...
                        retVal = false;
                    }
                }
                else
                {
                    // There is a thread with data we didn't find, or we didn't find any threads at all...
                    retVal = false;
                }

                // free up memory
                delete pCurrentThreadData;
                pCurrentThreadData = NULL;

                // Reset the "found thread data" flag, so we don't push back the same thread twice by accident:
                threadDataFound = false;
            }
            else
#endif  // Mac only
            {
                // This is the Linux / old Mac format(s), thus this line describes only one thread.
                // Parse the string token by token:
                int tokenIndex = 0;
                int threadOSIdTokenIndex = -1;
                int localTokenIndex = -1;
                gtASCIIString currToken;
                gtASCIIStringTokenizer strTokenizer(gdbOutputLine, " ");

                while (strTokenizer.getNextToken(currToken))
                {
                    // If the current token contains "*":
                    if (currToken.find(stat_starString) != -1)
                    {
                        // "*" marks GDB's active thread:
                        currThreadData._isGDBsActiveThread = true;
                    }
                    else if (currToken.startsWith(stat_consultOutputStringPrefix))
                    {
                        // "~" prefixes are ignored.
                    }
                    else
                    {
                        if (tokenIndex == 0)
                        {
                            // Read the thread's GDB id:
                            int threadGDBId = 0;
                            bool rc1 = currToken.toIntNumber(threadGDBId);
                            GT_IF_WITH_ASSERT(rc1)
                            {
                                currThreadData._gdbThreadId = threadGDBId;
                                threadDataFound = true;
                            }
                            else
                            {
                                retVal = false;
                            }
                        }
                        else if (tokenIndex == threadOSIdTokenIndex)
                        {
                            // Read the thread's OS id:
                            unsigned long threadOSId = 0;
                            bool rc2 = currToken.toUnsignedLongNumber(threadOSId);
                            GT_IF_WITH_ASSERT(rc2)
                            {
                                // If this is a "local" thread id:
                                if (localTokenIndex == (tokenIndex - 2))
                                {
                                    // Get the thread id in the debugged application address space:
                                    osThreadId applicationThreadId = getThreadIdInDebuggedApplicationAddressSpace(currThreadData._gdbThreadId);
                                    GT_ASSERT(applicationThreadId == OS_NO_THREAD_ID);
                                    currThreadData._OSThreadId = applicationThreadId;
                                }
                                else
                                {
                                    currThreadData._OSThreadId = threadOSId;
                                }

                                // We are done parsing this line:
                                break;
                            }
                            else
                            {
                                retVal = false;
                            }
                        }
                        else if ((currToken == stat_threadString1) || (currToken == stat_threadString2) || (currToken == stat_threadString3) || (currToken == stat_threadString4))
                        {
                            // If the current token is "Thread" or "thread", the next token is the OS thread id:
                            threadOSIdTokenIndex = tokenIndex + 1;
                        }
                        else if (currToken == stat_localString)
                        {
                            // This is part of a "local thread" string that tells us that the thread id is given in local GDB address space:
                            localTokenIndex = tokenIndex;
                        }

                        // Next token index:
                        tokenIndex++;
                    }
                }
            }
        }
    }

    if (retVal && threadDataFound)
    {
        // Output the current line's thread data:
        outputThreadsList._threadsDataList.push_front(currThreadData);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleGetThreadsInfoViaMachineInterfaceOutput
// Description:
//   Parses and handles a GDB output string that is the result of
//   an "-thread-list-ids" GDB command.
// Arguments: gdbOutputLine - The GDB output line.
// Return Val:  bool - false - iff an internal / a GDB error occurred.
// Author:      Uri Shomorni
// Date:        18/7/2010
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleGetThreadsInfoViaMachineInterfaceOutput(const gtASCIIString& gdbOutputString,
                                                                      const pdGDBData** ppGDBOutputData)
{
    bool retVal = false;

    // Create a data structure that will hold the threads list:
    pdGDBThreadDataList* pThreadsList = new pdGDBThreadDataList;
    GT_IF_WITH_ASSERT(pThreadsList != NULL)
    {
        retVal = true;

        // Parse the output line by line:
        gtASCIIString currentOutputLine;
        int currPos = 0;
        int gdbPrintoutsLen = gdbOutputString.length();

        while (currPos < gdbPrintoutsLen)
        {
            currentOutputLine.makeEmpty();

            // Search for the current line delimiter:
            int lineDelimiterPos = gdbOutputString.find('\n', currPos);

            // If we found a line delimiter:
            if (lineDelimiterPos != -1)
            {
                gdbOutputString.getSubString(currPos, lineDelimiterPos, currentOutputLine);
                currPos = lineDelimiterPos + 1;
            }
            else
            {
                gdbOutputString.getSubString(currPos, gdbPrintoutsLen - 1, currentOutputLine);
                currPos = gdbPrintoutsLen;
            }

            // Parse the current GDB output line:
            bool rc3 = handleThreadsInfoViaMachineInterfaceLineOutput(currentOutputLine, *pThreadsList);
            retVal = retVal && rc3;
        }

        if (retVal)
        {
            if (ppGDBOutputData != NULL)
            {
                // Output the threads list:
                *ppGDBOutputData = pThreadsList;
            }
        }
        else
        {
            // Failure cleanup:
            delete pThreadsList;
        }

        _currentThreadNumber = -1;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleThreadsInfoViaMachineInterfaceLineOutput
//   Parses and handles a single GDB output line that is the result of
//   an "-thread-list-ids" GDB command.
// Arguments: gdbOutputLine - The GDB output line.
//            outputThreadsList - Threads list to be filled up.
// Return Val:  bool - false - iff an internal / a GDB error occurred.
// Author:      Uri Shomroni
// Date:        15/7/2010
//
// Implementation notes:
//
//  On Mac OS X, An example result of an "info threads" GDB command may be:
//  ^done,thread-ids={thread-id="3",thread-id="2",thread-id="1"},number-of-threads="3",threads=[thread={thread-id="3",state="WAITING",mach-port-number="0x1f03",pthread-id="0x1005c1000",unique-id="0x644c"},thread={thread-id="2", ...}],time={wallclock="0.00106",user="0.00041",system="0.00064",start="1279211248.950793",end="1279211248.951850"}
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleThreadsInfoViaMachineInterfaceLineOutput(const gtASCIIString& gdbOutputLine,
        pdGDBThreadDataList& outputThreadsList)
{
    bool retVal = false;

    static const gtASCIIString stat_threadsDataStart("threads=[");
    static const gtASCIIString stat_threadsDataEnd("]");
    static const int stat_threadsDataPrefixLength = stat_threadsDataStart.length();
    static const int stat_threadsDataSuffixLength = stat_threadsDataEnd.length();
    static const gtASCIIString stat_threadDataStart("thread={");
    static const gtASCIIString stat_threadDataEnd("}");
    static const int stat_threadDataPrefixLength = stat_threadDataStart.length();
    static const int stat_threadDataSuffixLength = stat_threadDataEnd.length();
    static const gtASCIIString start_threadIndexTokenStart("thread-id=\"");
    static const gtASCIIString start_threadMachPortNumberTokenStart("mach-port-number=\"");
    static const gtASCIIString start_threadLocalTokenStart("local");

    // If under debug log severity, output debug log message:
    outputThreadLineLogMessage(gdbOutputLine);

    // Find the threads data delimiters:
    int threadsDataStart = gdbOutputLine.find(stat_threadsDataStart);
    int threadsDataEnd = gdbOutputLine.find(stat_threadsDataEnd, threadsDataStart);

    if ((threadsDataStart != -1) || (threadsDataEnd != -1))
    {
        GT_IF_WITH_ASSERT((threadsDataStart > -1) && (threadsDataEnd > threadsDataStart))
        {
            // Get only the threads data itself:
            gtASCIIString threadsData;
            gdbOutputLine.getSubString(threadsDataStart + stat_threadsDataPrefixLength, threadsDataEnd - stat_threadsDataSuffixLength, threadsData);
            GT_IF_WITH_ASSERT(!threadsData.isEmpty())
            {
                GT_ASSERT(outputThreadsList._threadsDataList.size() == 0);
                retVal = true;

                // Find the first thread's Data:
                int currentThreadDataStart = threadsData.find(stat_threadDataStart);
                int currentThreadDataEnd = threadsData.find(stat_threadDataEnd, currentThreadDataStart);
                pdGDBThreadData* pCurrentThreadData = NULL;

                // While we get more threads' data:
                while ((currentThreadDataStart > -1) && (currentThreadDataEnd > currentThreadDataStart))
                {
                    // Get the current thread's data:
                    bool threadDataFound = false;
                    gtASCIIString currentThreadData;
                    threadsData.getSubString(currentThreadDataStart + stat_threadDataPrefixLength, currentThreadDataEnd - stat_threadDataSuffixLength, currentThreadData);
                    GT_IF_WITH_ASSERT(!currentThreadData.isEmpty())
                    {
                        // Tokenize by commas, each field will be shown as name="value":
                        gtASCIIString threadDataToken;
                        gtASCIIStringTokenizer threadDataStrTokenizer(currentThreadData, ",");
                        pCurrentThreadData = new pdGDBThreadData;
                        bool isThreadNumberLocal = false;

                        while (threadDataStrTokenizer.getNextToken(threadDataToken))
                        {
                            // See which field is this:
                            if (threadDataToken.startsWith(start_threadIndexTokenStart))
                            {
                                // This is the (gdb) thread index, labelled "thread-id":
                                gtASCIIString threadIndexAsString;
                                GT_ASSERT(threadDataToken.count('\"') == 2);
                                int firstQuote = threadDataToken.find('\"');
                                int lastQuote = threadDataToken.reverseFind('\"');
                                threadDataToken.getSubString(firstQuote + 1, lastQuote - 1, threadIndexAsString);

                                // If the index is a number:
                                unsigned long threadIndex = 0;
                                bool rcInd = threadIndexAsString.toUnsignedLongNumber(threadIndex);
                                GT_IF_WITH_ASSERT(rcInd)
                                {
                                    // Store it:
                                    pCurrentThreadData->_gdbThreadId = threadIndex;

                                    if ((_currentThreadNumber > 0) && (threadIndex == (unsigned int)_currentThreadNumber))
                                    {
                                        pCurrentThreadData->_isGDBsActiveThread = true;
                                    }
                                }
                            }
                            else if (threadDataToken.startsWith(start_threadMachPortNumberTokenStart))
                            {
                                // This is the thread OS id (mach port #), labelled "mach-port-number":
                                gtASCIIString machPortNumberAsString;
                                GT_ASSERT(threadDataToken.count('\"') == 2);
                                int firstQuote = threadDataToken.find('\"');
                                int lastQuote = threadDataToken.reverseFind('\"');
                                threadDataToken.getSubString(firstQuote + 1, lastQuote - 1, machPortNumberAsString);

                                // If the port number is a number:
                                unsigned long machPortNumberAsULong = 0;
                                bool rcNum = machPortNumberAsString.toUnsignedLongNumber(machPortNumberAsULong);
                                GT_IF_WITH_ASSERT(rcNum)
                                {
                                    // Store it:
                                    osThreadId machPortNumber = (osThreadId)machPortNumberAsULong;
                                    GT_ASSERT(machPortNumber != OS_NO_THREAD_ID);
                                    pCurrentThreadData->_OSThreadId = machPortNumber;
                                    threadDataFound = true;

                                    // Avoid getting the thread ID twice:
                                    isThreadNumberLocal = false;
                                }
                            }
                            else if (threadDataToken.startsWith(start_threadLocalTokenStart))
                            {
                                // This is a local thread id, note that we got SOME thread id, but we need to get the mach port from gdb:
                                if (!threadDataFound)
                                {
                                    isThreadNumberLocal = true;
                                }
                            }
                            else
                            {
                                // Ignore other fields, they are irrelevant to us.
                            }
                        }

                        // If we only found a local id, get the global one from gdb:
                        if (isThreadNumberLocal)
                        {
                            // Get the thread id in the debugged application address space:
                            osThreadId applicationThreadId = getThreadIdInDebuggedApplicationAddressSpace(pCurrentThreadData->_gdbThreadId);
                            GT_IF_WITH_ASSERT(applicationThreadId != OS_NO_THREAD_ID)
                            {
                                pCurrentThreadData->_OSThreadId = applicationThreadId;
                                threadDataFound = true;
                            }
                        }

                        // Threads with no ids do not interest us:
                        if (threadDataFound)
                        {
                            outputThreadsList._threadsDataList.push_back(*pCurrentThreadData);
                        }

                        // Free up memory
                        delete pCurrentThreadData;
                        pCurrentThreadData = NULL;
                    }

                    // Get the next thread's data:
                    currentThreadDataStart = threadsData.find(stat_threadDataStart, currentThreadDataEnd);
                    currentThreadDataEnd = threadsData.find(stat_threadDataEnd, currentThreadDataStart);

                    // We take >= here for the case where both values are -1 (i.e. no threads data)
                    GT_ASSERT(currentThreadDataEnd >= currentThreadDataStart);
                }
            }
        }
    }
    else // ((threadsDataStart == -1) && (threadsDataEnd == -1))
    {
        // This is the gdb prompt or a command echo:
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::getThreadIdInDebuggedApplicationAddressSpace
// Description: Inputs a GDB thread id and returns the thread OS id in the
//              debugged application address space, or OS_NO_THREAD_ID in case of failure.
// Author:      Yaki Tebeka
// Date:        23/12/2008
// Implementation notes:
//   Under Mac OS X, GDB sometimes outputs thread ids as ~"2 process 1671 local thread 0x2f33" which
//   means that GDB's output thread id is the Mach threads id under the GDB address space. However, we
//   need the Mach thread id under the debugged process address space. This function enables getting
//   the thread id under the debugged process address space.
// ---------------------------------------------------------------------------
osThreadId pdGDBOutputReader::getThreadIdInDebuggedApplicationAddressSpace(int gdbThreadId) const
{
    osThreadId retVal = OS_NO_THREAD_ID;

    // This functionality is relevant to Mac OS X only:
    //#if AMDT_BUILD_TARGET == AMDT_MAC_OS_X_LINUX_VARIANT
    {
        gtASCIIString commandArguments;
        commandArguments.appendFormattedString("%d", gdbThreadId);
        const pdGDBData* pGDBOutputData = NULL;
        bool rc1 = _pGDBDriver->executeGDBCommand(PD_GET_THREAD_INFO_CMD, commandArguments, &pGDBOutputData);
        GT_IF_WITH_ASSERT(rc1 && (pGDBOutputData != NULL))
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(pGDBOutputData->type() == pdGDBData::PD_GDB_THREAD_DATA)
            {
                // Downcast the result to thread data:
                const pdGDBThreadData* pThreadData = (const pdGDBThreadData*)pGDBOutputData;

                // Output the thread's OS id given in the debugged process address space:
                retVal = pThreadData->_OSThreadId;
            }
        }
    }
    //#else
    //    {
    //       // This functionality should not be reached under none-Mac OS X platforms!
    //      GT_ASSERT(false);
    // }
    //#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleGetThreadInfoOutput
// Description:
//   Parses and handles a GDB output string that is the result of an "info thread" GDB command.
// Arguments: gdbOutputString - The GDB output for the "info thread" command.
//            ppGDBOutputData - Will get the queried thread information.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        24/12/2008
// Implementation notes:
//   The "info thread" command has the following output format:
//   ~"Thread 0x813 (local 0x2f3b) has current state \"WAITING\"\n"
//   ~"Thread 0x813 has a suspend count of 0.\n"</p>
//
//  As of XCode tools 3.2 (comes with Snow Leopard), gdb version 6.3.50-20050815 (Apple version gdb-1344), the format was changed to:
//  ~"Thread 2 has current state \"WAITING\"\n"
//  ~"\tMach port #0x1503 (gdb port #0x4833)\n"
//  ~"\tframe 0: "
//  ~"\tpthread ID: 0xb0081000\n"
//  ~"\tsystem-wide unique thread id: 0x2af5\n"
//  ~"\ttotal user time: 0x29fe0\n"
//  ~"\ttotal system time: 0x4ca90\n"
//  ~"\tscaled cpu usage percentage: 0\n"
//  ~"\tscheduling policy in effect: 0x1\n"
//  ~"\trun state: 0x3 (WAITING)\n"
//  ~"\tflags: 0x1 (SWAPPED)\n"
//  ~"\tnumber of seconds that thread has slept: 0\n"
//  ~"\tcurrent priority: 31\n"
//  ~"\tmax priority: 63\n"
//  ~"\tsuspend count: 0.\n"
//  ^done,frame={addr="0x976711a2",fp="0xb0080f70",func="__workq_kernreturn",args=[]}
//  (gdb)
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleGetThreadInfoOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData)
{
    bool retVal = false;

    static gtASCIIString stat_threadOSIdPrefix1 = "~\"Thread";
    static gtASCIIString stat_threadOSIdPrefix2 = "Mach port #";
    static int stat_threadOSIdPrefix1Length = stat_threadOSIdPrefix1.length();
    static int stat_threadOSIdPrefix2Length = stat_threadOSIdPrefix2.length();

    int threadIdPrefixLength = 0;
    gtASCIIString threadIdSuffix;

    // If the current line contains the thread's OS id in format 2:
    int pos1 = gdbOutputString.find(stat_threadOSIdPrefix2);

    GetStoppedThreadGDBId(gdbOutputString);

    if (pos1 != -1)
    {
        threadIdPrefixLength = stat_threadOSIdPrefix2Length;
        threadIdSuffix = " ";
    }
    else
    {
        // If the current line contains the thread's OS id in format 1:
        pos1 = gdbOutputString.find(stat_threadOSIdPrefix1);

        if (pos1 != -1)
        {
            threadIdPrefixLength = stat_threadOSIdPrefix1Length;
            threadIdSuffix = " ";
        }
    }

    // If we found the thread id's perfix:
    if (pos1 != -1)
    {
        // Calculate the place where the thread's pthread id begin:
        int pos2 = pos1 + threadIdPrefixLength + 1;

        // Search for the suffic that is located after the thread's OS id:
        int pos3 = gdbOutputString.find(threadIdSuffix, pos2);
        GT_IF_WITH_ASSERT(pos3 != -1)
        {
            // Get the thread's OS id as a string:
            gtASCIIString threadOSIdString;
            gdbOutputString.getSubString(pos2, pos3 - 1, threadOSIdString);

            // Translate it to an unsigned long:
            unsigned long threadOSId = 0;
            bool rc1 = threadOSIdString.toUnsignedLongNumber(threadOSId);
            GT_IF_WITH_ASSERT(rc1)
            {
                // Create an output structure:
                pdGDBThreadData* pOutputStruct = new pdGDBThreadData;
                GT_IF_WITH_ASSERT(pOutputStruct != NULL)
                {
                    // Output the queried thread id:
                    pOutputStruct->_OSThreadId = threadOSId;

                    if (ppGDBOutputData != NULL)
                    {
                        *ppGDBOutputData = pOutputStruct;
                    }

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleGetCurrThreadCallStackOutput
// Description:
//   Parses and handles a GDB output string that is the result of
//   an "get current thread call stack" GDB command.
// Arguments: gdbOutputLine - The GDB output line.
// Return Val:  bool - false - iff an internal / a GDB error occurred.
// Author:      Yaki Tebeka
// Date:        14/3/2007
// Implementation notes:
//  The call stack data format is similar to the below example:
//  stack=[frame={level="0",addr="0x00000030e74c3086",func="poll",from="/lib64/libc.so.6"},
//             frame={level="1",addr="0x00002aaaab801e99",func="wxapp_poll_func",file="../src/gtk/app.cpp",line="332"}]
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleGetCurrThreadCallStackOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData)
{
    bool retVal = false;

    static gtASCIIString gdbStackPerfix = "stack=";
    static gtASCIIString gdbFramePerfix = "frame=";
    static int gdbFramePerfixSize = gdbFramePerfix.length();

    // If under debug log severity, output debug printout:
    outputCallStackLogMessage(gdbOutputString);

    GetStoppedThreadGDBId(gdbOutputString);

    // Sanity check - verify that the input contains the stack prefix:
    int stackDataLoc = gdbOutputString.find(gdbStackPerfix) ;

    if (stackDataLoc != -1)
    {
        // Allocate the output struct:
        pdGDBCallStack* pReadCallStack = new pdGDBCallStack;
        GT_IF_WITH_ASSERT(pReadCallStack)
        {
            retVal = true;

            // Look for the first frame data:
            int searchStartPos = stackDataLoc + gdbStackPerfix.length();
            int currFramePos = gdbOutputString.find(gdbFramePerfix, searchStartPos);

            // While there are frames that we didn't parse:
            while (currFramePos != -1)
            {
                // Look for the frame opening bracket:
                int openingBracketPos = gdbOutputString.find('{', currFramePos);
                GT_IF_WITH_ASSERT(openingBracketPos != -1)
                {
                    // Look for the frame closing bracket:
                    int closingBracketPos = gdbOutputString.find('}', openingBracketPos);
                    GT_IF_WITH_ASSERT(closingBracketPos != -1)
                    {
                        // Get the frame data string:
                        gtASCIIString frameDataString;
                        gdbOutputString.getSubString(openingBracketPos + 1, closingBracketPos - 1, frameDataString);

                        // Add the current frame data to the call stack:
                        bool rc1 = addFrameDataToCallStack(frameDataString, pReadCallStack->_callStack);
                        GT_ASSERT(rc1);
                    }
                }

                // Look for the next frame data:
                currFramePos = gdbOutputString.find(gdbFramePerfix, currFramePos + gdbFramePerfixSize);
            }

            // If all went well - output the call stack:
            if (retVal && (ppGDBOutputData != NULL))
            {
                *ppGDBOutputData = pReadCallStack;
            }
            else
            {
                // Failure clean up:
                delete pReadCallStack;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleGetExecutablePidOutput
// Description:
//   Parses and handles a GDB output string that is the result of
//   a "get executable pid" GDB command.
// Arguments: gdbOutputLine - The GDB output line.
//            ppGDBOutputData - Will get the executable pid.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/8/2007
// Implementation notes:
//  The output of the GDB "info proc" command format is similar to the below example:
//  &"info proc\n"
//  ~"process 592\n"
//  ~"cmdline = '/home/yaki/work/driveo/debug/bin/CodeXL-bin'\n"
//  ~"cwd = '/home/yaki/work/driveo/debug/bin/examples/teapot'\n"
//  ~"exe = '/home/yaki/work/driveo/debug/bin/CodeXL-bin'\n"
//  ^done
//  (gdb)
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleGetExecutablePidOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData)
{
    bool retVal = false;

    static gtASCIIString gdbProcessIdPerfix = "process ";
    static int gdbProcessIdPerfixLen = gdbProcessIdPerfix.length();

    // Find the process id prefix:
    int pos1 = gdbOutputString.find(gdbProcessIdPerfix) ;

    if (pos1 != -1)
    {
        // Read the process id:
        int pos2 = pos1 + gdbProcessIdPerfixLen;
        long processId = 0;
        const char* pProcessIdStr = gdbOutputString.asCharArray() + pos2;
        int rc1 = sscanf(pProcessIdStr, "%ld", &processId);
        GT_IF_WITH_ASSERT(rc1 == 1)
        {
            // Allocate the output struct:
            pdGDBProcessId* pProcessIdStruct = new pdGDBProcessId;
            GT_IF_WITH_ASSERT(pProcessIdStruct != NULL)
            {
                // Output the process id:
                pProcessIdStruct->_processPid = processId;

                if (ppGDBOutputData != NULL)
                {
                    *ppGDBOutputData = pProcessIdStruct;
                }

                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleGetSymbolAtAddressOutput
// Description:
//   Parses and handles a GDB output string that is the result of
//   a "get symbol name at address" GDB command.
//
// Arguments: gdbOutputLine - The GDB output line.
//            ppGDBOutputData - Will get the name of the symbol that resides under the input address.
//
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/1/2008
// Implementation notes:
//  The output of the GDB "info symbol" command format is similar to the below example:
//  info symbol 0x2aaaaab1c146
//  &"info symbol 0x2aaaaab1c146\n"
//  ~"gaUpdateCurrentThreadTextureRawDataStub() in section .text\n"
// ^done
// (gdb)
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleGetSymbolAtAddressOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData)
{
    bool retVal = false;

    static gtASCIIString stat_startSymbolPrefix = "\"";
    static int stat_startSymbolPrefixLen = stat_startSymbolPrefix.length();
    static gtASCIIString stat_endSymbolSuffix = "in section";

    // Find the symbol suffix:
    int pos3 = gdbOutputString.find(stat_endSymbolSuffix);
    GT_IF_WITH_ASSERT(pos3 != -1)
    {
        // Find the start symbol prefix:
        int pos1 = gdbOutputString.reverseFind(stat_startSymbolPrefix, pos3);
        GT_IF_WITH_ASSERT(pos1 != -1)
        {
            // Calculate the first symbol char position:
            int pos2 = pos1 + stat_startSymbolPrefixLen;

            // Calculate the last symbol char position:
            int pos4 = pos3 - 2;

            // The symbol is sometimes prefixed with few spaces - if so, the symbol start position
            // should reside after these spaces:
            int pos5 = gdbOutputString.reverseFind(" ", pos4);

            if (pos2 <= pos5)
            {
                pos2 = pos5 + 1;
            }

            // Sanity check:
            GT_IF_WITH_ASSERT(pos2 < pos4)
            {
                // Get the symbol name:
                gtASCIIString symbolName;
                gdbOutputString.getSubString(pos2, pos4, symbolName);

                // Create a structure that will contain the output symbol name:
                pdGDBStringData* pStringOutputStruct = new pdGDBStringData(symbolName);
                GT_IF_WITH_ASSERT(pStringOutputStruct != NULL)
                {
                    if (ppGDBOutputData != NULL)
                    {
                        *ppGDBOutputData = pStringOutputStruct;
                    }

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleGetDebugInfoAtAddressOutput
// Description: Handles the output of the "info line" commands human format. The commands we
//              issue are solely of the form "info line *0xfeedface", which gives the debug info of the address.
//              The answer is human readable with the following format:
//              ~"Line 2027 of \"Examples/AMDTTeaPot/AMDTTeaPotLib/src/AMDTTeapotOCLSmokeSystem.cpp\" starts at address 0x41d384 <AMDTTeapotOCLSmokeSystem::compute(AMDTTeapotRenderState const&, Mat4 const&, float, float)+2344> and ends at 0x41d3f6 <AMDTTeapotOCLSmokeSystem::compute(AMDTTeapotRenderState const&, Mat4 const&, float, float)+2458>.\n"
//              since it is implementation-dependant, we instead use the uniform
//              output, which is supplied by running gdb with the --fullname switch.
// Return Val: bool  - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        12/11/2012
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleGetDebugHumanInfoAtAddressOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData)
{
    bool retVal = false;

    static const gtASCIIString debugInformationReadableLineStart = "~\"Line ";
    static const gtASCIIString debugInformationReadableNumberEnd = " of \\\"";
    static const gtASCIIString debugInformationReadableSourceEnd = "\\\"";

    // Find the different section of the string:
    int readableLineStart = gdbOutputString.find(debugInformationReadableLineStart);

    if (readableLineStart > -1)
    {
        int readableLineNumberEnd = gdbOutputString.find(debugInformationReadableNumberEnd, readableLineStart);
        GT_IF_WITH_ASSERT(readableLineNumberEnd > readableLineStart)
        {
            int readableSourceEnd = gdbOutputString.find(debugInformationReadableSourceEnd, readableLineNumberEnd + debugInformationReadableNumberEnd.length());
            GT_IF_WITH_ASSERT(readableSourceEnd > readableLineNumberEnd)
            {
                // parse the different sections:
                gtASCIIString filePathAsString, lineNumberAsString;
                unsigned int lineNumber = 0;

                gdbOutputString.getSubString((readableLineStart + debugInformationReadableLineStart.length()), (readableLineNumberEnd - 1), lineNumberAsString);
                gdbOutputString.getSubString((readableLineNumberEnd + debugInformationReadableNumberEnd.length()), (readableSourceEnd - 1), filePathAsString);
                bool rcNum = lineNumberAsString.toUnsignedIntNumber(lineNumber);

                GT_IF_WITH_ASSERT(rcNum)
                {
                    gtString filePathAsUnicodeString;
                    filePathAsUnicodeString.fromASCIIString(filePathAsString.asCharArray());
                    osFilePath filePath(filePathAsUnicodeString);

                    if (ppGDBOutputData != NULL)
                    {
                        pdGDBSourceCodeData* pSourceCodeData = new pdGDBSourceCodeData(filePath, lineNumber);

                        if (ppGDBOutputData != NULL)
                        {
                            *ppGDBOutputData = pSourceCodeData;
                        }

                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleGetDebugInfoAtAddressOutput
// Description: Handles the output of the "info line" commands machine format. The commands we
//              issue are solely of the form "info line *0xfeedface", which gives
//              the debug info of the address. The answer is human readable, but
//              since it is implementation-dependant, we instead use the uniform
//              output, which is supplied by running gdb with the --fullname switch.
//              this output begins with two \032 characters, followed by a colon
//              separated list of: full source code file path, line number(1-based),
//              character number (0-based from the start of the file), and further
//              information which is irrelevant to us and doesn't appear in all
//              implementations
//              ~"\032\032/home/jdoe/myFolder/myProject/src/mySource.cpp:42:1337\n"
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        17/2/2009
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleGetDebugInfoAtAddressOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData)
{
    bool retVal = false;

    static const gtASCIIString debugInformationReadableLineStart = "~\"\\032\\032";
    static const gtASCIIString debugInformationReadableLineEnd = "\\n\"";

    int readableLineStart = gdbOutputString.find(debugInformationReadableLineStart);

    if (readableLineStart > -1)
    {
        int readableLineEnd = gdbOutputString.find(debugInformationReadableLineEnd, readableLineStart);
        GT_IF_WITH_ASSERT(readableLineEnd > readableLineStart)
        {
            gtASCIIString readableLine;
            gdbOutputString.getSubString((readableLineStart + debugInformationReadableLineStart.length()), (readableLineEnd + 1), readableLine);
            GT_IF_WITH_ASSERT(readableLine.count(':') >= 2)
            {
                int firstColonPosition = readableLine.find(':');
                int secondColonPosition = readableLine.find(':', (firstColonPosition + 1));

                if (secondColonPosition == -1)
                {
                    secondColonPosition = readableLine.length();
                }

                GT_IF_WITH_ASSERT(secondColonPosition > (firstColonPosition + 1))
                {
                    gtASCIIString filePathAsString, lineNumberAsString;
                    unsigned int lineNumber = 0;
                    readableLine.getSubString(0, (firstColonPosition - 1), filePathAsString);
                    readableLine.getSubString((firstColonPosition + 1), (secondColonPosition - 1), lineNumberAsString);
                    bool rcNum = lineNumberAsString.toUnsignedIntNumber(lineNumber);
                    GT_IF_WITH_ASSERT(rcNum)
                    {
                        gtString filePathAsUnicodeString;
                        filePathAsUnicodeString.fromASCIIString(filePathAsString.asCharArray());
                        osFilePath filePath(filePathAsUnicodeString);
                        GT_IF_WITH_ASSERT(filePath.isRegularFile())
                        {
                            if (ppGDBOutputData != NULL)
                            {
                                pdGDBSourceCodeData* pSourceCodeData = new pdGDBSourceCodeData(filePath, lineNumber);
                                GT_IF_WITH_ASSERT(pSourceCodeData != NULL)
                                {
                                    if (ppGDBOutputData != NULL)
                                    {
                                        // If data was created by human format release it since it might not be with full path:
                                        if (NULL != *ppGDBOutputData)
                                        {
                                            delete(*ppGDBOutputData);
                                        }

                                        *ppGDBOutputData = pSourceCodeData;
                                    }

                                    retVal = true;
                                }
                            }
                            else
                            {
                                retVal = true;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        // We didn't find the line we looked for, this probably means the address has no debug information:
        if ((ppGDBOutputData != NULL) && (*ppGDBOutputData == NULL))
        {
            pdGDBSourceCodeData* pSourceCodeData = new pdGDBSourceCodeData(osFilePath(), 0);


            *ppGDBOutputData = pSourceCodeData;
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleGetLibraryAtAddressOutput
// Description: Handles the output from the "info sharedlibrary 0xfeedface" function
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        22/2/2009
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleGetLibraryAtAddressOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData)
{
    bool retVal = false;

    pdGDBLibraryData* pLibraryInfo = new pdGDBLibraryData();

    // The "info sharedlibrary" has a different implementation on Mac, so we use it otherwise.
    static const gtASCIIString macOutputIdentifier = ",shlib-info=";

    if (gdbOutputString.find(macOutputIdentifier) >= 0)
    {
        // On Mac, the output is machine-readable, starts with ^done, and continues with the shared library
        // information as displayed when reading the output from the command with no address specified.
        // this output starts with 'shlib-info=[num="42",name="myLib.dylib",kind=...' and has somewhere in
        // the rest of the output a 'path="/myFolder/bin/myLib.dylib"' field.

        // Make sure we only got one shared library as a result:
        GT_IF_WITH_ASSERT(gdbOutputString.count(macOutputIdentifier) == 1)
        {
            // We need to make sure that the field we read is named "path" (or if that fails, "name") and not something containing
            // these strings, so we search for them with a delimiter before:
            static const gtASCIIString macLibraryNameStart1 = ",path=\"";
            static const gtASCIIString macLibraryNameStart2 = "[path=\"";
            static const gtASCIIString macLibraryNameStart3 = ",name=\"";
            static const gtASCIIString macLibraryNameStart4 = "[name=\"";
            int nameFieldStart = gdbOutputString.find(macLibraryNameStart1);

            if (nameFieldStart < 0)
            {
                nameFieldStart = gdbOutputString.find(macLibraryNameStart2);

                if (nameFieldStart < 0)
                {
                    nameFieldStart = gdbOutputString.find(macLibraryNameStart3);

                    if (nameFieldStart < 0)
                    {
                        nameFieldStart = gdbOutputString.find(macLibraryNameStart4);

                        if (nameFieldStart < 0)
                        {
                            GT_ASSERT_EX(false, L"Could not find file name in 'info sharedlibrary' output");
                        }
                        else
                        {
                            nameFieldStart += macLibraryNameStart4.length();
                        }
                    }
                    else
                    {
                        nameFieldStart += macLibraryNameStart3.length();
                    }
                }
                else
                {
                    nameFieldStart += macLibraryNameStart2.length();
                }
            }
            else
            {
                nameFieldStart += macLibraryNameStart1.length();
            }

            if (nameFieldStart > 0)
            {
                int nameFieldEnd = gdbOutputString.find('\"', nameFieldStart);
                GT_IF_WITH_ASSERT(nameFieldEnd > nameFieldStart + 1)
                {
                    nameFieldEnd--;
                    gtASCIIString filePathAsString;
                    gdbOutputString.getSubString(nameFieldStart, nameFieldEnd, filePathAsString);
                    gtString filePathAsUnicodeString;
                    filePathAsUnicodeString.fromASCIIString(filePathAsString.asCharArray());
                    pLibraryInfo->_libraryFilePath.setFullPathFromString(filePathAsUnicodeString);
                    retVal = true;
                }
            }
        }
    }
    else //gdbOutputString.find(macOutputIdentifier) < 0
    {
        // This is a linux-style output.
        // Linux ignores the address we sent it (which is why we hold it as a member) and outputs
        // a table with all the shared libraries in the following format:
        // From             To              Syms Read   Shared Object Library
        // 0xfacefeed       0xfeedface      Yes         ./myLib.so
        // 0x00b00f00       0x00f00b00      No          /usr/lib/myOtherLib.so
        // This output is also human readable, so we need to parse out the ~" at the beginning of lines
        // and the \n" at the end.

        // Sanity check:
        GT_IF_WITH_ASSERT(_findAddress != (osInstructionPointer)NULL)
        {
            // Iterate the libraries until we find one that has the right address range:
            gtASCIIStringTokenizer stringTokenizer1(gdbOutputString, "\n\r");
            gtASCIIString currentToken1;

            while (stringTokenizer1.getNextToken(currentToken1))
            {
                static const gtASCIIString outputLineStart = "~\"";

                // Ignore the command echo and the prompt:
                if (currentToken1.startsWith(outputLineStart))
                {
                    // Make sure the line ends as we expect it:
                    static const gtASCIIString outputLineEnd = "\\n\"";
                    int expectedLineEnd = (currentToken1.length() - outputLineEnd.length());
                    GT_IF_WITH_ASSERT(currentToken1.reverseFind(outputLineEnd) == expectedLineEnd)
                    {
                        // Ignore "not found" messages:
                        static const gtASCIIString notFoundMessage1 = "No shared libraries matched"; // Ubuntu 10 message

                        if (gdbOutputString.find(notFoundMessage1) != -1)
                        {
                            continue;
                        }

                        currentToken1.truncate(outputLineStart.length(), (expectedLineEnd - 1));
                        gtASCIIStringTokenizer stringTokenizer2(currentToken1, " ");
                        gtASCIIString currentToken2;

                        // Each line should have four strings:
                        bool rcToken = stringTokenizer2.getNextToken(currentToken2);
                        GT_IF_WITH_ASSERT(rcToken)
                        {
                            // If the first one is "From:" this is the header line, ignore it:
                            static const gtASCIIString firstHeader = "From";

                            if (currentToken2.compareNoCase(firstHeader) != 0)
                            {
                                // The first token is the start address:
                                unsigned long long addressAsULongLong = 0;
                                bool rcNum = currentToken2.toUnsignedLongLongNumber(addressAsULongLong);
                                GT_IF_WITH_ASSERT(rcNum)
                                {
                                    osInstructionPointer startAddress = (osInstructionPointer)addressAsULongLong;
                                    rcToken = stringTokenizer2.getNextToken(currentToken2);
                                    GT_IF_WITH_ASSERT(rcToken)
                                    {
                                        // The second token is the end address:
                                        addressAsULongLong = 0;
                                        rcNum = currentToken2.toUnsignedLongLongNumber(addressAsULongLong);
                                        GT_IF_WITH_ASSERT(rcNum)
                                        {
                                            osInstructionPointer endAddress = (osInstructionPointer)addressAsULongLong;

                                            // See if this library contains our address:
                                            if ((endAddress > _findAddress) && (startAddress < _findAddress))
                                            {
                                                rcToken = stringTokenizer2.getNextToken(currentToken2);
                                                GT_IF_WITH_ASSERT(rcToken)
                                                {
                                                    rcToken = stringTokenizer2.getNextToken(currentToken2);

                                                    // The third token is the debug symbols loaded status, ignore it
                                                    GT_IF_WITH_ASSERT(rcToken)
                                                    {
                                                        gtString filePathAsUnicodeString;
                                                        filePathAsUnicodeString.fromASCIIString(currentToken2.asCharArray());
                                                        pLibraryInfo->_libraryFilePath.setFullPathFromString(filePathAsUnicodeString);
                                                        retVal = true;
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (retVal)
    {
        if (ppGDBOutputData != NULL)
        {
            *ppGDBOutputData = pLibraryInfo;
        }
        else
        {
            // cleanup
            delete pLibraryInfo;
        }
    }
    else
    {
        // cleanup
        delete pLibraryInfo;

        // We failed to find anything, but it doesn't mean we really failed:
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleAbortDebuggedProcessOutput
// Description:
//   Parses and handles a GDB output string that is the result of
//   an "abort debugged process" GDB command.
//
// Arguments: gdbOutputLine - The GDB output line.
//
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        27/1/2009
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleAbortDebuggedProcessOutput(const gtASCIIString& gdbOutputString)
{
    bool retVal = false;

    // If the debugged process termination was successful:
    // (we don't search for "^done", since on Mac gdb only outputs "done")
    int pos1 = gdbOutputString.find("done");
    GT_IF_WITH_ASSERT(pos1 != -1)
    {
        retVal = true;
    }

    std::stringstream ss(gdbOutputString.asCharArray());
    std::string token;

    while (std::getline(ss, token, '\n'))
    {
        if (std::string::npos != token.find("=thread-exited,id=\""))
        {
            handleExitThreadMessage(gtASCIIString(token.c_str()));
        }
    }

    GT_IF_WITH_ASSERT(nullptr != _pGDBDriver)
    {
        if (_pGDBDriver->IsAllThreadsStopped())
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleWaitingForDebuggedProcessOutput
// Description: Handle the output from a "attach -waitfor" command
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        11/5/2009
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleWaitingForDebuggedProcessOutput(const gtASCIIString& gdbOutputString)
{
    bool retVal = false;

    // We are expecting a string of format "Attaching to process ####"
    static const gtASCIIString echoMessage = "attach -waitfor";
    static const gtASCIIString waitingMessageStart = "Waiting for process ";
    static const gtASCIIString attachingMessageStart = "~\"Attaching to process ";

    if (gdbOutputString.startsWith(attachingMessageStart))
    {
        gtASCIIString processIdAsString = gdbOutputString;
        processIdAsString.truncate(attachingMessageStart.length(), -1);
        processIdAsString.removeTrailing('\"');
        processIdAsString.removeTrailing('n');
        processIdAsString.removeTrailing('\\');
        processIdAsString.removeTrailing('.');
        int processId = -1;
        bool rcNum = processIdAsString.toIntNumber(processId);
        GT_IF_WITH_ASSERT(rcNum)
        {
            // Mark that the debugged process is terminated:
            _wasDebuggedProcessTerminated = false;
            _wasDebuggedProcessSuspended = false;
            m_didDebuggedProcessReceiveFatalSignal = false;
            _debuggedProcessCurrentThreadGDBId = -1;
            _debuggedProcessCurrentThreadId = OS_NO_THREAD_ID;

            // Trigger a process run started event:
            osTime processRunStartedTime;
            processRunStartedTime.setFromCurrentTime();

            apDebuggedProcessRunStartedEvent* pProcessRunStartedEvent = new apDebuggedProcessRunStartedEvent(processId, processRunStartedTime);
            m_eventsToRegister.push_back(pProcessRunStartedEvent);
            _wasDebuggedProcessCreated = true;

            // When attaching to a process, it is suspended by GDB. Resume it:
            gtASCIIString commandArg;
            _pGDBDriver->executeGDBCommand(PD_CONTINUE_CMD, commandArg);

            retVal = true;
        }
    }
    else if (gdbOutputString.startsWith(s_gdbPromtStr))
    {
        // The attachment was cancelled, mark that the process doesn't exist:
        _wasDebuggedProcessTerminated = true;
        _wasDebuggedProcessCreated = false;
        _wasDebuggedProcessSuspended = false;
        m_didDebuggedProcessReceiveFatalSignal = false;
        _processId = 0;
        retVal = true;
    }
    else if ((gdbOutputString.startsWith(waitingMessageStart)) || (gdbOutputString.find(echoMessage) > -1))
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::addFrameDataToCallStack
// Description:
//   Parses an input string that contains call stack's frame data
//   and adds the frame's data into a given call stack.
// Arguments: frameDataString - A string containing the frame's data.
//                   callStack - A call stack into which the frame's data will be added.
// Return Val:  bool - true / false - success / failure.
// Author:      Yaki Tebeka
// Date:        15/3/2007
// Implementation notes:
//  The frame data format is similar to the below examples:
//  level="0",addr="0x00000030e74c3086",func="poll",from="/lib64/libc.so.6"
//  level="1",addr="0x00002aaaab820647",func="wxEventLoop::Run",file="../src/gtk/evtloop.cpp",line="76"
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::addFrameDataToCallStack(const gtASCIIString& frameDataString, osCallStack& callStack)
{
    bool retVal = false;

    static const gtASCIIString gdbAddressStr = "addr";
    static const gtASCIIString gdbFuncStr = "func";
    static const gtASCIIString gdbFileStr = "file";
    static const gtASCIIString gdbFullFileStr = "fullname";
    static const gtASCIIString gdbLineStr = "line";
    static const gtASCIIString gdbModuleStr = "from";

    // If under debug log severity, output debug printout:
    outputCallStackLineLogMessage(frameDataString);

    // Will get the call stack frame:
    osCallStackFrame readCallStackFrame;

    // Parse the string by (name="value") pairs:
    gtASCIIString curPairString;
    gtASCIIStringTokenizer strTokenizer(frameDataString, ",");

    while (strTokenizer.getNextToken(curPairString))
    {
        retVal = true;

        // Look for the = sign:
        int equalSignPos = curPairString.find('=');

        if (equalSignPos != -1)
        {
            // Get the pair name:
            gtASCIIString name;
            curPairString.getSubString(0, equalSignPos - 1, name);

            // Get the pair value:
            gtASCIIString value;
            curPairString.getSubString(equalSignPos + 2, curPairString.length() - 2, value);

            // Act according to the pair name:
            if (name == gdbAddressStr)
            {
                // value is an instruction couter address value:
                osInstructionPointer address = NULL;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                int rc1 = 0;

                if (_debuggedExecutableArchitecture == OS_I386_ARCHITECTURE)
                {
                    osProcedureAddress addressAsProcAddress = NULL;
                    rc1 = sscanf(value.asCharArray(), "%p", &addressAsProcAddress);
                    address = (osInstructionPointer)addressAsProcAddress;
                }
                else if (_debuggedExecutableArchitecture == OS_X86_64_ARCHITECTURE)
                {
                    gtASCIIString valueToRead = value;
                    valueToRead.toLowerCase();

                    if (valueToRead.startsWith("0x"))
                    {
                        valueToRead.truncate(2, -1);
                        rc1 = sscanf(value.asCharArray(), "%llx", &address);
                    }
                    else
                    {
                        rc1 = sscanf(value.asCharArray(), "%llu", &address);
                    }
                }
                else
                {
                    // Unsupported or unknown architecture, we should not get here!
                    GT_ASSERT(false);
                }

#else
                int rc1 = sscanf(value.asCharArray(), "%p", &address);
#endif
                GT_IF_WITH_ASSERT(rc1 == 1)
                {
                    // We move the pointer one instruction back so it points to the current instruction
                    address = (osInstructionPointer)((gtUInt64)address - 1);
                    readCallStackFrame.setInstructionCounterAddress(address);
                }
            }
            else if (name == gdbFuncStr)
            {
                // value is a function name:
                gtString functionNameAsUnicodeString;
                functionNameAsUnicodeString.fromASCIIString(value.asCharArray());
                readCallStackFrame.setFunctionName(functionNameAsUnicodeString);
            }
            else if ((name == gdbFileStr) || (name == gdbFullFileStr))
            {
                // value is a file name:
                gtString filePathAsUnicodeString;
                filePathAsUnicodeString.fromASCIIString(value.asCharArray());
                osFilePath moduleFilePath(filePathAsUnicodeString);
                readCallStackFrame.setSourceCodeFilePath(moduleFilePath);
            }
            else if (name == gdbLineStr)
            {
                // value is a line number:
                int lineNumber = 0;
                bool rc2 = value.toIntNumber(lineNumber);
                GT_IF_WITH_ASSERT(rc2)
                {
                    readCallStackFrame.setSourceCodeFileLineNumber(lineNumber);
                }
            }
            else if (name == gdbModuleStr)
            {
                // value is a module file path:
                gtString filePathAsUnicodeString;
                filePathAsUnicodeString.fromASCIIString(value.asCharArray());
                osFilePath moduleFilePath(filePathAsUnicodeString);
                readCallStackFrame.setModuleFilePath(moduleFilePath);
            }
        }
    }

    if (retVal)
    {
        // Check if the frame is a spy frame and mark it if needed:
        markSpyFrames(readCallStackFrame);

        // Add the frame to the call stack:
        callStack.addStackFrame(readCallStackFrame);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::markSpyFrames
// Description:
//   Inputs a call stack frame. Checks if it is an OpenGL spy frame,
//   and marks it as such if needed.
// Arguments: callStackFrame - The input call stack frame.
// Author:      Yaki Tebeka
// Date:        26/12/2006
// ---------------------------------------------------------------------------
void pdGDBOutputReader::markSpyFrames(osCallStackFrame& callStackFrame)
{
    static gtVector<gtString> spySourceFileNames;

    // Initialize the vector on the first time this is called:
    if (spySourceFileNames.empty())
    {
        // TO_DO: A better approach would be to check the module name or part of a full
        //              source file name path, but currently we don't get them out of GDB.
        //              So, meanwhile we check the following file names:
        spySourceFileNames.push_back(L"gsOpenGLWrappers.cpp");
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
        spySourceFileNames.push_back(L"gsGLXWrappers.cpp");
#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
        spySourceFileNames.push_back(L"gsCGLWrappers.cpp");
        spySourceFileNames.push_back(L"gsEAGLWrappers.mm");
#else
#error unknown Linux variant
#endif
        spySourceFileNames.push_back(L"gsOpenGLExtensionsWrappers.cpp");
        spySourceFileNames.push_back(L"gsOpenGLMonitor.cpp");
        spySourceFileNames.push_back(L"gsOpenGLModuleInitializer.cpp");
        spySourceFileNames.push_back(L"gsOpenGLSpyInitFuncs.cpp");

        spySourceFileNames.push_back(L"csOpenCLWrappers.cpp");
        spySourceFileNames.push_back(L"csOpenGLIntegrationWrappers.cpp");
        spySourceFileNames.push_back(L"csOpenCLMonitor.cpp");
        spySourceFileNames.push_back(L"csOpenCLModuleInitializer.cpp");
        spySourceFileNames.push_back(L"csSingletonsDelete.cpp");
        spySourceFileNames.push_back(L"csOpenCLServerInitialization.cpp");
    }

    static const gtString openGLSpyModuleName = OS_GREMEDY_OPENGL_SERVER_MODULE_NAME;
    static const gtString openGLSpyESModuleName = OS_OPENGL_ES_COMMON_DLL_NAME;
    static const gtString openCLSpyModuleName = OS_GREMEDY_OPENCL_SERVER_MODULE_NAME;

    // Will get true iff the input frame is a spy frame:
    bool isSpyFrame = false;

    // Get the module file path:
    const gtString& modulePath = callStackFrame.moduleFilePath().asString();
    bool isSpyFileName = (modulePath.find(openGLSpyModuleName) != -1) || (modulePath.find(openGLSpyESModuleName) != -1) || (modulePath.find(openCLSpyModuleName) != -1);
    static const gtString linuxSystemPathPrefix = L"/usr/lib";

    if (isSpyFileName && (!modulePath.startsWith(linuxSystemPathPrefix)))
    {
        isSpyFrame = true;
    }
    else
    {
        // Get the source code file path:
        const gtString& frameSourcePath = callStackFrame.sourceCodeFilePath().asString();

        // Check if it contains spy files:
        for (const gtString& fn : spySourceFileNames)
        {
            if (-1 != frameSourcePath.find(fn))
            {
                isSpyFrame = true;
                break;
            }
        }
    }

    // If this is a spy function frame - mark it as such:
    if (isSpyFrame)
    {
        callStackFrame.markAsSpyFunction();
    }
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleGDBResultOutput
// Description:
//   Parses and handles a GDB output line that contains GDB result.
// Arguments: gdbOutputLine - The GDB output line.
// Return Val:  bool - false - iff an internal / a GDB error occurred.
// Author:      Yaki Tebeka
// Date:        26/12/2006
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleGDBResultOutput(const gtASCIIString& gdbOutputLine)
{
    bool retVal = true;

    static gtASCIIString targetIsRunningMsg = "^running";
    static gtASCIIString errorMsg = "^error";
    static gtASCIIString connectedToRemoteTargetMsg = "^connected";
    static const gtASCIIString badThreadMsg = "^done,bad_thread=";
    static const gtASCIIString exitedMsg = "exited";

    gtString gdbOutputLineAsUnicodeString;
    gdbOutputLineAsUnicodeString.fromASCIIString(gdbOutputLine.asCharArray());

    bool sendTermination = false;

    if (gdbOutputLine.startsWith(s_gdbOperationSuccessStr))
    {
        const pdGDBCommandInfo* pCommandInfo = pdGetGDBCommandInfo(_executedGDBCommandId);
        GT_IF_WITH_ASSERT(pCommandInfo != NULL)
        {
            // If we got this during an asynchronus command, this might mean the debugged process exited:
            if (pCommandInfo->_commandType == PD_GDB_ASYNCHRONOUS_CMD)
            {
                // If the application exited:
                if (gdbOutputLine.find(s_exitedGDBStr) > -1)
                {
                    // Report it (see the end of the function):
                    sendTermination = true;
                }
            }
            else
            {
                // An synchronous operation was successful, do nothing for now.
                if (gdbOutputLine.startsWith(badThreadMsg))
                {
                    // This means that the command "succeeded" but did not have the desired result, so we consider it failed:
                    retVal = false;
                }
            }
        }

        // Clear the last logged GDB command:
        _executedGDBCommandId = PD_GDB_NULL_CMD;
    }
    else if (gdbOutputLine.startsWith(targetIsRunningMsg))
    {
        // The debugged process is running:

        // Raise an appropriate event, but don't notify this more than once (Apple's gdb V. 1344 - 6.3.50 shows "switch to process" for every break)
        bool debuggedProcessExists = pdProcessDebugger::instance().debuggedProcessExists();

        if (!(_wasDebuggedProcessCreated && debuggedProcessExists))
        {
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
            {
                _wasDebuggedProcessCreated = true;
                // The process run started:
                osTime processRunStartedTime;
                processRunStartedTime.setFromCurrentTime();

                apDebuggedProcessRunStartedEvent* pProcessRunStartedEvent = new apDebuggedProcessRunStartedEvent(_processId, processRunStartedTime);
                m_eventsToRegister.push_back(pProcessRunStartedEvent);
            }
#endif
        }
        else
        {
            // Some thread was resumed
            GT_IF_WITH_ASSERT(_pGDBDriver != NULL)
            {
                if (_pGDBDriver->IsAllThreadsRunning())
                {
                    // The process run was resumed:
                    apDebuggedProcessRunResumedEvent* pProcessRunResumedEvent = new apDebuggedProcessRunResumedEvent;
                    m_eventsToRegister.push_back(pProcessRunResumedEvent);
                }
            }
        }
    }
    else if (gdbOutputLine.startsWith(errorMsg))
    {
        // A GDB operation failed:
        retVal = false;
        int messageStartPosition = gdbOutputLine.find("msg=\"");
        int messageEndPosition = gdbOutputLine.reverseFind('"');
        GT_IF_WITH_ASSERT_EX((messageEndPosition - messageStartPosition > 5) && (messageStartPosition >= 0),
                             gdbOutputLineAsUnicodeString.asCharArray())
        {
            retVal = true;

            gtString errorMsg;
            gdbOutputLineAsUnicodeString.getSubString((messageStartPosition + 5), (messageEndPosition - 1), errorMsg);
            apGDBErrorEvent* pErrorEvent = new apGDBErrorEvent(errorMsg);
            m_eventsToRegister.push_back(pErrorEvent);


            // If the error is "exited during function execution", also send a termination event:
            if (PD_EXECUTE_FUNCTION_CMD == _executedGDBCommandId)
            {
                if (-1 != gdbOutputLine.find(exitedMsg))
                {
                    sendTermination = true;
                }
            }
            else if (PD_GDB_RUN_DEBUGGED_PROCESS_CMD == _executedGDBCommandId)
            {
                if (-1 != gdbOutputLine.find(exitedMsg))
                {
                    sendTermination = true;
                }
            }
        }
    }
    else if (gdbOutputLine.startsWith(connectedToRemoteTargetMsg))
    {
        // We don't support remote debugging yet ...
        GT_ASSERT_EX(false, gdbOutputLineAsUnicodeString.asCharArray());
        retVal = false;
    }
    else
    {
        // Unknown GDB result output:
        GT_ASSERT_EX(false, gdbOutputLineAsUnicodeString.asCharArray());
        retVal = false;
    }

    // Handle process exits:
    if (sendTermination)
    {
        // Mark that the process terminated:
        _wasDebuggedProcessTerminated = true;
        _wasDebuggedProcessCreated = false;

        // Trigger a process terminated event:
        apDebuggedProcessTerminatedEvent* pDebuggedProcessTerminationEvent = new apDebuggedProcessTerminatedEvent(0);
        m_eventsToRegister.push_back(pDebuggedProcessTerminationEvent);

        // Mark that the debugged process is not suspended (it is terminated):
        _wasDebuggedProcessSuspended = false;
        m_didDebuggedProcessReceiveFatalSignal = false;

        // Mark that there is no current debugged process thread:
        _debuggedProcessCurrentThreadGDBId = -1;
        _debuggedProcessCurrentThreadId = OS_NO_THREAD_ID;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleGDBConsoleOutput
// Description:
//   Parses and handles a GDB output line that contains GDB console output.
// Arguments: gdbOutputLine - The GDB output line.
// Return Val:  bool - false - iff an internal / a GDB error occurred.
// Author:      Yaki Tebeka
// Date:        26/12/2006
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleGDBConsoleOutput(const gtASCIIString& gdbOutputLine)
{
    bool retVal = true;

    if (gdbOutputLine.startsWith(s_newThreadMsg))
    {
        // A new debugged process thread was created:
        handleNewThreadMessage(gdbOutputLine);
    }
    else if (gdbOutputLine.find(s_switchingToThreadMsg) != -1)
    {
        // Handle the "[Switching to Thread..." message:
        handleSwitchingToThreadMessage(gdbOutputLine);
    }
    else if (gdbOutputLine.startsWith(s_debuggedProcessStoppedMsg))
    {
        bool wasBreakpointHit = false;
        handleSignalOutput(gdbOutputLine, wasBreakpointHit);
    }
    else if (gdbOutputLine.find(s_switchingToProcessMsg) != -1)
    {
        bool doesProcessExist = pdProcessDebugger::instance().debuggedProcessExists();

        if ((gdbOutputLine.find(s_switchingToProcessAndThread) != -1) && doesProcessExist)
        {
            // Handle the "[Switching to process ... {local} thread ..." message:
            handleSwitchingToProcessAndThreadMessage(gdbOutputLine);
        }
        else
        {
            // Handle the "[Switching to process..." message:
            handleSwitchingToProcessMessage(gdbOutputLine);
        }
    }
    else
    {
        // The "set environment" command echos a string when setting environment
        // variables to null. We would like to ignore this string.
        if (_executedGDBCommandId != PD_SET_ENV_VARIABLE_CMD)
        {
            // Increment the amount of GDB string printouts:
            _amountOfGDBStringPrintouts++;

            if (_amountOfGDBStringPrintouts == PD_MAX_GDB_STRING_PRINTOUTS)
            {
                // Tell the user that we reached the maximal debug printouts:
                static gtString maxReportsMsg;
                maxReportsMsg.makeEmpty();
                maxReportsMsg.appendFormattedString(PD_STR_GDBStringMaxPrintoutsReached, PD_MAX_GDB_STRING_PRINTOUTS);
                apGDBOutputStringEvent* pOutputEvent = new apGDBOutputStringEvent(maxReportsMsg);
                m_eventsToRegister.push_back(pOutputEvent);
            }
            else if (_amountOfGDBStringPrintouts < PD_MAX_GDB_STRING_PRINTOUTS)
            {
                // Report the GDB console output as a debugged process output string:
                gtString gdbOutputLineAsUnicodeString;
                gdbOutputLineAsUnicodeString.fromASCIIString(gdbOutputLine.asCharArray());
                apGDBOutputStringEvent* pOutputEvent = new apGDBOutputStringEvent(gdbOutputLineAsUnicodeString);
                m_eventsToRegister.push_back(pOutputEvent);
            }

            // Otherwise ignore the printouts:
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleDebuggedProcessOutput
// Description:
//   Parses and handles a GDB output line that contains a debugged
//   process output.
// Arguments: gdbOutputLine - The GDB output line.
// Return Val:  bool - false - iff an internal / a GDB error occurred.
// Author:      Yaki Tebeka
// Date:        26/12/2006
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleDebuggedProcessOutput(const gtASCIIString& gdbOutputLine)
{
    bool retVal = true;

    // If we got a message that tells us that gdb launch failed:
    bool failedToLaunchGDB = (gdbOutputLine.find(PD_STR_failedToLaunchGDBASCII) != -1);

    if (failedToLaunchGDB)
    {
        retVal = false;
    }
    else
    {
        gtString gdbOutputLineAsUnicodeString;
        gdbOutputLineAsUnicodeString.fromASCIIString(gdbOutputLine.asCharArray());
        static const gtString debugStringPrefix(OS_STR_DebugStringOutputPrefix);

        if (gdbOutputLineAsUnicodeString.startsWith(debugStringPrefix))
        {
            // This is a debug string generated by osOutputDebugString.
            // Get the debug string itself:
            int prefixLength = debugStringPrefix.length();
            gtString debugMessage = gdbOutputLineAsUnicodeString;
            debugMessage.truncate(prefixLength, -1);

            // Make a debug string event:
            apOutputDebugStringEvent* pDebugStringEve = new apOutputDebugStringEvent(OS_NO_THREAD_ID, debugMessage);
            m_eventsToRegister.push_back(pDebugStringEve);
        }
        else
        {
            // Trigger a "debugged process output string" event:
            apDebuggedProcessOutputStringEvent* pOutputStringEve = new apDebuggedProcessOutputStringEvent(gdbOutputLineAsUnicodeString);
            m_eventsToRegister.push_back(pOutputStringEve);

        }

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleGDBInternalOutput
// Description:
//   Parses and handles a GDB internal output.
// Arguments: gdbOutputLine - The GDB output line.
// Return Val:  bool - false - iff an internal / a GDB error occurred.
// Author:      Yaki Tebeka
// Date:        26/12/2006
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleGDBInternalOutput(const gtASCIIString& gdbOutputLine)
{
    bool retVal = true;

    // If under debug log severity:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        // Output the messages to the debug log:
        gtString gdbOutputLineAsUnicodeString;
        gdbOutputLineAsUnicodeString.fromASCIIString(gdbOutputLine.asCharArray());
        OS_OUTPUT_DEBUG_LOG(gdbOutputLineAsUnicodeString.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleAsynchronousOutput
// Description:
//   Parses and handles a GDB output line that contains a GDB "exec-async-output"
//   outputs.
// Arguments: gdbOutputLine - The GDB output line.
// Return Val:  bool - false - iff an internal / a GDB error occurred.
// Author:      Yaki Tebeka
// Date:        26/12/2006
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleAsynchronousOutput(const gtASCIIString& gdbOutputLine)
{
    bool retVal = false;

    static const gtASCIIString breakpointHitGDBStr = "breakpoint-hit";
    static const gtASCIIString breakpointNextGDBStr = "end-stepping-range";
    static const gtASCIIString breakpointFinishGDBStr = "function-finished";
    static const gtASCIIString signalReceivedGDBStr = "signal-received";
    static const gtASCIIString runningThreadGDBStr = "running";

    // If the debugged process is running:
    if (gdbOutputLine.startsWith(s_debuggedProcessRunningMsg))
    {
        // Sigal 29/9/2009
        // On OpenSuse OS, gdb version 6.8.50.20081120 : An example result of an "-exec-continue" GDB command may be:
        // *running, thread id="all"
        // We skip this line, since gdb already told us that the debugged process run was resumed using the "^running" output.
        GetRunningThreadGDBId(gdbOutputLine);
        retVal = true;
    }
    // If the debugged process stopped running:
    else if (gdbOutputLine.startsWith(s_debuggedProcessStoppedMsg))
    {
        // Mark that the debugged process run was suspended:
        //_wasDebuggedProcessSuspended = true;
        m_didDebuggedProcessReceiveFatalSignal = false;

        // Will get true if we caught a signal:
        bool wasSignalCaught = false;
        bool wasBreakpointHit = false;

        // The following mechanism is only needed in Mac, where the "info threads" output
        // does not supply the information of which thread is current:
#if (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
        // If there is a thread number specified, store it:
        static gtASCIIString threadNumberGDBStr = "thread-id=\"";

        // Clear the current thread number:
        _currentThreadNumber = -1;

        int threadNumberLocation = gdbOutputLine.find(threadNumberGDBStr);

        if (threadNumberLocation != -1)
        {
            // Get the thread number:
            gtASCIIString threadNumberAsString;
            threadNumberLocation += threadNumberGDBStr.length();
            int threadNumberEnd = gdbOutputLine.find('\"', threadNumberLocation);
            GT_IF_WITH_ASSERT(threadNumberEnd > threadNumberLocation)
            {
                // We are on the quotation mark, move one char back:
                threadNumberEnd--;
                gdbOutputLine.getSubString(threadNumberLocation, threadNumberEnd, threadNumberAsString);

                int threadNumber = -1;
                bool rcNum = threadNumberAsString.toIntNumber(threadNumber);

                GT_IF_WITH_ASSERT(rcNum)
                {
                    _currentThreadNumber = threadNumber;
                }
            }
        }

#endif

        // Get the string that describes the stop reason:
        gtASCIIString stopReason;
        bool rc1 = getStopReasonString(gdbOutputLine, stopReason);
        bool wasHostBreakPoint = false;

        if (!rc1)
        {
            /// Gdb not always put stop reason of the "*stopped" message for steps
            if (gdbOutputLine.find("frame=") != -1)
            {
                flushGDBPrompt();
                rc1 = true;
            }
        }

        if (rc1)
        {
            // If a signal was received:
            wasSignalCaught = stopReason.startsWith(signalReceivedGDBStr);
            bool isBreakpoint = stopReason.startsWith(breakpointHitGDBStr);
            bool isStep = stopReason.startsWith(breakpointNextGDBStr) || stopReason.startsWith(breakpointFinishGDBStr);

            if ((isBreakpoint && gdbOutputLine.find("disp=\"del\"") != -1))
            {
                isBreakpoint = false;
                isStep = true;
            }

            if (wasSignalCaught)
            {
                // Handle the signal:
                handleSignalOutput(gdbOutputLine, wasBreakpointHit);
            }
            else if (isBreakpoint || isStep)
            {
                bool isGDBBreakpoint = (-1 != gdbOutputLine.find("bkptno") && (gdbOutputLine.find("disp=\"del\"") == -1));

                if (isStep || isGDBBreakpoint)
                {
                    // Host breakpoint
                    int pos1 = gdbOutputLine.find("thread-id=\"");

                    if (-1 != pos1)
                    {
                        int length = ::strlen("thread-id=\"");
                        int pos2 = gdbOutputLine.find("\"", pos1 + length);

                        GT_IF_WITH_ASSERT(-1 != pos2)
                        {
                            gtASCIIString strThreadId;
                            gdbOutputLine.getSubString(pos1 + length, pos2 - 1, strThreadId);

                            int threadGDBId = 0;

                            GT_IF_WITH_ASSERT(strThreadId.toIntNumber(threadGDBId))
                            {
                                _pGDBDriver->OnThreadGDBStopped(threadGDBId);

                                _debuggedProcessCurrentThreadGDBId = threadGDBId;
                                _debuggedProcessCurrentThreadId = threadIdFromGDBId(_debuggedProcessCurrentThreadGDBId);

                                wasBreakpointHit = handleBreakpointHit(isGDBBreakpoint, isStep);
                                _wasDebuggedProcessSuspended = true;
                                wasHostBreakPoint = true;
                            }
                        }
                    }
                }
                else
                {
                    // The debugged process hit a breakpoint (usually set by GDB):
                    // If this breakpoint came from the spy, report it to the application. Otherwise,
                    // treat it as a non-fatal signal (SIGTRAP is non-fatal):
                    bool wasStoppedInSpy = false;
                    bool rcLoc = isCurrentThreadStoppedInsideSpy(wasStoppedInSpy);
                    GT_ASSERT(rcLoc);

                    // Some applications (l3com case 7852, OpenSceneGraph) have a thread that sits in pthread_cond_wait, which is the same as the
                    // breakpoint triggering thread.
                    // This sometimes confuses gdb making it think that a different thread triggered the signal, causing spy breakpoints to be considered
                    // as foreign breaks, and making the app become stuck. So, to be on the safe side, before deciding a SIGTRAP is a foreign break, we
                    // check all threads to make sure none of them is inside the spy.
                    if (!wasStoppedInSpy)
                    {
                        wasStoppedInSpy = isAnyThreadStoppedInSpy();
                    }

                    wasBreakpointHit = wasStoppedInSpy;

                    if (wasBreakpointHit)
                    {
                        wasBreakpointHit = handleBreakpointHit();
                    }
                }
            }
            else if (stopReason.startsWith(s_exitedGDBStr))
            {
                // The debugged process exited:
                _wasDebuggedProcessTerminated = true;
                _wasDebuggedProcessCreated = false;
            }
        }
        else // !rc1
        {
            if (_isWaitingForInternalDebuggedProcessInterrupt)
            {
                // Always consider a stop during a "call" that is not a SIGINT signal to be an exit / crash. This happens if the application exits normally
                // during a break (e.g. closing its window) and then using the call command finalizes the exit command. Not identifying this causes a hang in the GUI:
                _wasDebuggedProcessTerminated = true;
                _wasDebuggedProcessCreated = false;
            }
        }


        // If the debugged process was terminated:
        if (_wasDebuggedProcessTerminated)
        {
            // Trigger a process terminated event:
            apDebuggedProcessTerminatedEvent* pDebuggedProcessTerminationEvent = new apDebuggedProcessTerminatedEvent(0);
            m_eventsToRegister.push_back(pDebuggedProcessTerminationEvent);

            // Mark that the debugged process is not suspended (it is terminated):
            _wasDebuggedProcessSuspended = false;
            m_didDebuggedProcessReceiveFatalSignal = false;

            // Mark that there is no current debugged process thread:
            _debuggedProcessCurrentThreadGDBId = -1;
            _debuggedProcessCurrentThreadId = OS_NO_THREAD_ID;
        }
        else
        {
            // If we caught a signal that is not a breakpoint:
            if (wasSignalCaught && !wasBreakpointHit && !m_didDebuggedProcessReceiveFatalSignal)
            {
                // We will resume the debugged process run shortly to make the debugged process handle the _non-fatal_ signal (see pdLinuxProcessDebugger::onExceptionEvent).
                // Therefore, we don't want the outside world to know that the debugged process was suspended.
            }
            else
            {
                // The debugged process should be suspended, as it got a breakpoint or a fatal signal.
                // Trigger also a debugged process suspended event:
                apDebuggedProcessRunSuspendedEvent* pRunSuspendedEvent = new apDebuggedProcessRunSuspendedEvent(_debuggedProcessCurrentThreadId, wasHostBreakPoint);
                m_eventsToRegister.push_back(pRunSuspendedEvent);
            }
        }

        retVal = true;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleStatusAsynchronousOutput
// Description: Handles GDB "status asynchronous output"
// Arguments: gdbOutputLine - The GDB output string.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        31/12/2008
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleStatusAsynchronousOutput(const gtASCIIString& gdbOutputLine)
{
    bool retVal = false;

    static gtASCIIString s_SharedLibAddedMsg = "=shlibs-added";
    static gtASCIIString s_SharedLibRemovedMsg = "=shlibs-removed";
    static gtASCIIString s_StartThreadMsg = "=thread-created,id=\"";
    static gtASCIIString s_ExitThreadMsg = "=thread-exited,id=\"";
    static gtASCIIString s_ThreadGroupStarted = "=thread-group-started";

    // If a shared module was loaded into the debugged process address space::
    if (gdbOutputLine.find(s_StartThreadMsg) != -1)
    {
        handleNewThreadMessage(gdbOutputLine);
        retVal = true;
    }
    else if (gdbOutputLine.find(s_ExitThreadMsg) != -1)
    {
        handleExitThreadMessage(gdbOutputLine);
        retVal = true;
    }
    else if (gdbOutputLine.find(s_SharedLibRemovedMsg) != -1)
    {
        // If a shared module was removed from the debugged process address space:
        retVal = handleSharedModuleUnloadedMessage(gdbOutputLine);
    }
    else if (gdbOutputLine.find(s_ThreadGroupStarted) != -1)
    {
        GT_IF_WITH_ASSERT(gdbOutputLine.find("pid") != -1)
        {
            int pos1 = gdbOutputLine.find("pid=\"");
            GT_IF_WITH_ASSERT(-1 != pos1)
            {
                pos1 += (int)strlen("pid=\"");
                int pos2 = gdbOutputLine.find("\"", pos1);

                GT_IF_WITH_ASSERT(1 < pos2)
                {
                    gtASCIIString processId;
                    pos2--;

                    GT_IF_WITH_ASSERT(pos1 < pos2)
                    {
                        gdbOutputLine.getSubString(pos1, pos2, processId);

                        GT_IF_WITH_ASSERT(processId.toIntNumber(_processId))
                        {
                            retVal = true;
                        }
                        else
                        {
                            _processId = 0;
                        }
                    }
                }
            }
        }
    }
    else
    {
        // Unknown asynchronous output - do nothing:
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleUnknownGDBOutput
// Description:
//   Parses and handles a GDB output line that contains an GDB output that
//   this class does not recognize.
// Arguments: gdbOutputLine - The GDB output line.
// Return Val:  bool - false - iff an internal / a GDB error occured.
// Author:      Yaki Tebeka
// Date:        26/12/2006
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleUnknownGDBOutput(const gtASCIIString& gdbOutputLine)
{
    bool retVal = true;

    // Report this to the development team:
    gtString gdbOutputLineAsUnicodeString;
    gdbOutputLineAsUnicodeString.fromASCIIString(gdbOutputLine.asCharArray());
    GT_ASSERT_EX(false, gdbOutputLineAsUnicodeString.asCharArray());

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::getStopReasonString
// Description:
//   Inputs a GDB output line that contains a GDB "*stopped" notification
//   and returns the sub string that describes the stop reason.
// Arguments: gdbOutputLine - The input GDB output line.
//            stopReasonString - Will get the sub string that describes the stop reason.
// Return Val:  bool - Sucess / Failure.
// Author:      Yaki Tebeka
// Date:        26/12/2006
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::getStopReasonString(const gtASCIIString& gdbOutputLine,
                                            gtASCIIString& stopReasonString)
{
    bool retVal = false;

    static gtASCIIString stopReasonGDBStr = "reason=\"";

    // Get the stop reason:
    int pos1 = gdbOutputLine.find(stopReasonGDBStr);
    GT_IF_WITH_ASSERT(pos1 != -1)
    {
        int pos2 = pos1 + stopReasonGDBStr.length();
        int pos3 = gdbOutputLine.find('\"', pos2);
        GT_IF_WITH_ASSERT(pos3 != -1)
        {
            gdbOutputLine.getSubString(pos2, (pos3 - 1), stopReasonString);
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::isAnyThreadStoppedInSpy
// Description: Returns true iff any thread is stopped in the spy
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/8/2010
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::isAnyThreadStoppedInSpy()
{
    bool retVal = false;

    // Get the debugged process threads data:
    static const gtASCIIString emptyStr;
    const pdGDBData* pGDBOutputData = NULL;

    // On Mac, starting with Xcode 3.2.3, the "info threads" command does not give us all the data we need, so we use the machine interface "-thread-list-ids" instead.
    // On Linux, the machine interface function is not implementer on all platforms, so we cannot use it as we might not get the data.
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
    bool rc1 = _pGDBDriver->executeGDBCommand(PD_GET_THREADS_INFO_CMD, emptyStr, &pGDBOutputData);
#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    bool rc1 = _pGDBDriver->executeGDBCommand(PD_GET_THREADS_INFO_VIA_MI_CMD, emptyStr, &pGDBOutputData);
#else
#error Unknown Linux Variant!
#endif
    GT_IF_WITH_ASSERT(rc1 && (pGDBOutputData != NULL))
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(pGDBOutputData->type() == pdGDBData::PD_GDB_THREAD_DATA_LIST)
        {
            // Store the threads data;
            pdGDBThreadDataList* pDebuggedProcessThreadsData = (pdGDBThreadDataList*)pGDBOutputData;

            const gtList<pdGDBThreadData>& debuggedProcessThreadsDataList = pDebuggedProcessThreadsData->_threadsDataList;

            // Iterate all the threads:
            gtList<pdGDBThreadData>::const_iterator iter = debuggedProcessThreadsDataList.begin();
            gtList<pdGDBThreadData>::const_iterator endIter = debuggedProcessThreadsDataList.end();

            while (iter != endIter)
            {
                // Set the current thread:
                int currentThreadGDBIndex = (*iter)._gdbThreadId;
                GT_IF_WITH_ASSERT(currentThreadGDBIndex > -1)
                {
                    gtASCIIString currentThreadIndexAsString;
                    currentThreadIndexAsString.appendFormattedString("%d", currentThreadGDBIndex);
                    bool rcThd = _pGDBDriver->executeGDBCommand(PD_SET_ACTIVE_THREAD_CMD, currentThreadIndexAsString);
                    GT_IF_WITH_ASSERT(rcThd)
                    {
                        bool threadWasSuspended = true;
                        bool threadRunning = _pGDBDriver->IsThreadRunning(currentThreadGDBIndex);
                        bool rcStopThrd = true;

                        if (threadRunning)
                        {
                            _pGDBDriver->SuspendThread(currentThreadGDBIndex);
                            threadWasSuspended = false;
                        }

                        GT_IF_WITH_ASSERT(rcStopThrd)
                        {
                            // Check if this thread is in the spy:
                            bool isInSpy = false;
                            bool rcSpy = isCurrentThreadStoppedInsideSpy(isInSpy);
                            GT_IF_WITH_ASSERT(rcSpy)
                            {
                                // We found a thread inside the spy:
                                if (isInSpy)
                                {
                                    // Stop looking:
                                    retVal = true;
                                }
                            }

                            if (!threadWasSuspended)
                            {
                                bool rcRes = _pGDBDriver->ResumeThread(currentThreadGDBIndex);

                                GT_ASSERT(rcRes);
                            }

                            if (retVal)
                            {
                                break;
                            }

                        }
                    }
                }

                iter++;
            }
        }
    }

    // Clean up the threads data:
    delete pGDBOutputData;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::isCurrentThreadStoppedInsideSpy
// Description: If the gdb current thread is stopped inside the spy, sets wasStoppedInSpy to true.
// Arguments: gdbOutputLine - the output line
//            wasStoppedInSpy - will be filled with true iff the breakpoint came from the spy utilities module
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        18/7/2010
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::isCurrentThreadStoppedInsideSpy(bool& wasStoppedInSpy)
{
    bool retVal = false;

    // Get the break call stack:
    osCallStack breakCallStack;
    static const gtASCIIString emptyStr;
    const pdGDBData* pGDBOutputData = NULL;
    bool rcStack = _pGDBDriver->executeGDBCommand(PD_GET_CUR_THREAD_CALL_STACK_CMD, emptyStr, &pGDBOutputData);
    GT_IF_WITH_ASSERT(rcStack && (pGDBOutputData != NULL))
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(pGDBOutputData->type() == pdGDBData::PD_GDB_CALL_STACK_DATA)
        {
            // Get the call stack data:
            const pdGDBCallStack* pCallStackData = (const pdGDBCallStack*)pGDBOutputData;

            // Get its top frame:
            breakCallStack = pCallStackData->_callStack;
        }
    }
    delete pGDBOutputData;

    // Verify the stack isn't empty:
    int amountOfBreakStackFrames = breakCallStack.amountOfStackFrames();
    GT_IF_WITH_ASSERT(amountOfBreakStackFrames > 0)
    {
        // Go down the stack, looking for spy frames (we cannot assume the topmost or 2nd topmost frame is the one that called ::kill(),
        // as different unix version have a different stack depth to each one:
        retVal = true;

        // Until proven otherwise, this break came from another module, so it is a foreign break:
        wasStoppedInSpy = false;

        // Iterate the stack frames
        static const gtString stat_spiesUtilitiesModuleName(OS_SPY_UTILS_FILE_PREFIX);
        static const gtString stat_spiesBreakpointFunctionName(OS_SPIES_BREAKPOINT_FUNCTION_NAME);

        for (int i = 0; i < amountOfBreakStackFrames; i++)
        {
            // Get the current frame:
            const osCallStackFrame* pCurrentStackFrame = breakCallStack.stackFrame(i);

            if (pCurrentStackFrame != NULL)
            {
                // We consider the breakpoint to be triggered by the spy if:
                // 1. The stack has a spy frame (i.e. a frame that the addFrameDataToCallStack function recognized as such
                // 2. The stack has a frame from GRSpiesUtilities
                // 3. The stack has the function suBreakpointsManager::triggerBreakpointException (this is needed for release builds, where debug info may not be available)
                gtString modulePathLowerCase = pCurrentStackFrame->moduleFilePath().asString();
                int spyUtilitiesFileNameInModulePath = modulePathLowerCase.toLowerCase().find(stat_spiesUtilitiesModuleName);

                if (pCurrentStackFrame->isSpyFunction() || (spyUtilitiesFileNameInModulePath > -1) || (pCurrentStackFrame->functionName() == stat_spiesBreakpointFunctionName))
                {
                    // If this is the spy utilities module:
                    wasStoppedInSpy = true;
                    break;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleSignalOutput
// Description:
//   Inputs a GDB output line that contains a GDB "signal-received" notification
//   and returns the appropriate event type.
// Arguments: gdbOutputLine - The input GDB output line.
//                   wasBreakpointHit - Will get true iff the signal represents a breakpoint hit.
//
// Author:      Yaki Tebeka
// Date:        3/1/2007
// ---------------------------------------------------------------------------
void pdGDBOutputReader::handleSignalOutput(const gtASCIIString& gdbOutputLine, bool& wasBreakpointHit)
{
    static gtASCIIString signalNameGDBStr = "signal-name=\"";
    static int signalNameGDBStrLen = signalNameGDBStr.length();

    static gtASCIIString sigThreadStopped = "0"; /// < In case stopped specific thread in non-stop GDB mode
    static gtASCIIString sigHupGDBStr = "SIGHUP";
    static gtASCIIString sigIntGDBStr = "SIGINT";
    static gtASCIIString sigQuitGDBStr = "SIGQUIT";
    static gtASCIIString sigIllGDBStr = "SIGILL";
    static gtASCIIString sigTrapGDBStr = "SIGTRAP";
    static gtASCIIString sigAbrtGDBStr = "SIGABRT";
    static gtASCIIString sigBugGDBStr = "SIGBUS";
    static gtASCIIString sigFpeGDBStr = "SIGFPE";
    static gtASCIIString sigKillGDBStr = "SIGKILL";
    static gtASCIIString sigSegvGDBStr = "SIGSEGV";
    static gtASCIIString sigPipeGDBStr = "SIGPIPE";
    static gtASCIIString sigAlrmGDBStr = "SIGALRM";
    static gtASCIIString sigTermGDBStr = "SIGTERM";
    static gtASCIIString sigUsr1GDBStr = "SIGUSR1";
    static gtASCIIString sigUsr2GDBStr = "SIGUSR2";
    static gtASCIIString sigStopGDBStr = "SIGSTOP";
    static gtASCIIString sigStpGDBStr = "SIGTSTP";
    static gtASCIIString sigTTInGDBStr = "SIGTTIN";
    static gtASCIIString sigTTOuGDBStr = "SIGTTOU";

    static gtASCIIString strSIGEMT_SIGNAL = "SIGEMT";
    static gtASCIIString strSIGSYS_SIGNAL = "SIGSYS";
    static gtASCIIString strSIGURG_SIGNAL = "SIGURG";
    static gtASCIIString strSIGSTOP_SIGNAL = "SIGSTOP";
    static gtASCIIString strSIGTSTP_SIGNAL = "SIGTSTP";
    static gtASCIIString strSIGCONT_SIGNAL = "SIGCONT";
    static gtASCIIString strSIGCHLD_SIGNAL = "SIGCHLD";
    static gtASCIIString strSIGTTIN_SIGNAL = "SIGTTIN";
    static gtASCIIString strSIGTTOU_SIGNAL = "SIGTTOU";
    static gtASCIIString strSIGIO_SIGNAL = "SIGIO";
    static gtASCIIString strSIGXCPU_SIGNAL = "SIGXCPU";
    static gtASCIIString strSIGXFSZ_SIGNAL = "SIGXFSZ";
    static gtASCIIString strSIGVTALRM_SIGNAL = "SIGVTALRM";
    static gtASCIIString strSIGPROF_SIGNAL = "SIGPROF";
    static gtASCIIString strSIGWINCH_SIGNAL = "SIGWINCH";
    static gtASCIIString strSIGLOST_SIGNAL = "SIGLOST";
    static gtASCIIString strSIGPWR_SIGNAL = "SIGPWR";
    static gtASCIIString strSIGPOLL_SIGNAL = "SIGPOLL";
    static gtASCIIString strSIGWIND_SIGNAL = "SIGWIND";
    static gtASCIIString strSIGPHONE_SIGNAL = "SIGPHONE";
    static gtASCIIString strSIGWAITING_SIGNAL = "SIGWAITING";
    static gtASCIIString strSIGLWP_SIGNAL = "SIGLWP";
    static gtASCIIString strSIGDANGER_SIGNAL = "SIGDANGER";
    static gtASCIIString strSIGGRANT_SIGNAL = "SIGGRANT";
    static gtASCIIString strSIGRETRACT_SIGNAL = "SIGRETRACT";
    static gtASCIIString strSIGMSG_SIGNAL = "SIGMSG";
    static gtASCIIString strSIGSOUND_SIGNAL = "SIGSOUND";
    static gtASCIIString strSIGSAK_SIGNAL = "SIGSAK";
    static gtASCIIString strSIGPRIO_SIGNAL = "SIGPRIO";
    static gtASCIIString strSIGCANCEL_SIGNAL = "SIGCANCEL";
    static gtASCIIString strEXC_BAD_ACCESS_SIGNAL = "EXC_BAD_ACCESS";
    static gtASCIIString strEXC_BAD_INSTRUCTION_SIGNAL = "EXC_BAD_INSTRUCTION";
    static gtASCIIString strEXC_ARITHMETIC_SIGNAL = "EXC_ARITHMETIC";
    static gtASCIIString strEXC_EMULATION_SIGNAL = "EXC_EMULATION";
    static gtASCIIString strEXC_SOFTWARE_SIGNAL = "EXC_SOFTWARE";
    static gtASCIIString strEXC_BREAKPOINT_SIGNAL = "EXC_BREAKPOINT";
    static gtASCIIString strSIG32_SIGNAL = "SIG32";
    static gtASCIIString strSIG33_SIGNAL = "SIG33";
    static gtASCIIString strSIG34_SIGNAL = "SIG34";
    static gtASCIIString strSIG35_SIGNAL = "SIG35";
    static gtASCIIString strSIG36_SIGNAL = "SIG36";
    static gtASCIIString strSIG37_SIGNAL = "SIG37";
    static gtASCIIString strSIG38_SIGNAL = "SIG38";
    static gtASCIIString strSIG39_SIGNAL = "SIG39";
    static gtASCIIString strSIG40_SIGNAL = "SIG40";
    static gtASCIIString strSIG41_SIGNAL = "SIG41";
    static gtASCIIString strSIG42_SIGNAL = "SIG42";
    static gtASCIIString strSIG43_SIGNAL = "SIG43";
    static gtASCIIString strSIG44_SIGNAL = "SIG44";
    static gtASCIIString strSIG45_SIGNAL = "SIG45";
    static gtASCIIString strSIG46_SIGNAL = "SIG46";
    static gtASCIIString strSIG47_SIGNAL = "SIG47";
    static gtASCIIString strSIG48_SIGNAL = "SIG48";
    static gtASCIIString strSIG49_SIGNAL = "SIG49";
    static gtASCIIString strSIG50_SIGNAL = "SIG50";
    static gtASCIIString strSIG51_SIGNAL = "SIG51";
    static gtASCIIString strSIG52_SIGNAL = "SIG52";
    static gtASCIIString strSIG53_SIGNAL = "SIG53";
    static gtASCIIString strSIG54_SIGNAL = "SIG54";
    static gtASCIIString strSIG55_SIGNAL = "SIG55";
    static gtASCIIString strSIG56_SIGNAL = "SIG56";
    static gtASCIIString strSIG57_SIGNAL = "SIG57";
    static gtASCIIString strSIG58_SIGNAL = "SIG58";
    static gtASCIIString strSIG59_SIGNAL = "SIG59";
    static gtASCIIString strSIG60_SIGNAL = "SIG60";
    static gtASCIIString strSIG61_SIGNAL = "SIG61";
    static gtASCIIString strSIG62_SIGNAL = "SIG62";
    static gtASCIIString strSIG63_SIGNAL = "SIG63";
    static gtASCIIString strSIG64_SIGNAL = "SIG64";
    static gtASCIIString strSIG65_SIGNAL = "SIG65";
    static gtASCIIString strSIG66_SIGNAL = "SIG66";
    static gtASCIIString strSIG67_SIGNAL = "SIG67";
    static gtASCIIString strSIG68_SIGNAL = "SIG68";
    static gtASCIIString strSIG69_SIGNAL = "SIG69";
    static gtASCIIString strSIG70_SIGNAL = "SIG70";
    static gtASCIIString strSIG71_SIGNAL = "SIG71";
    static gtASCIIString strSIG72_SIGNAL = "SIG72";
    static gtASCIIString strSIG73_SIGNAL = "SIG73";
    static gtASCIIString strSIG74_SIGNAL = "SIG74";
    static gtASCIIString strSIG75_SIGNAL = "SIG75";
    static gtASCIIString strSIG76_SIGNAL = "SIG76";
    static gtASCIIString strSIG77_SIGNAL = "SIG77";
    static gtASCIIString strSIG78_SIGNAL = "SIG78";
    static gtASCIIString strSIG79_SIGNAL = "SIG79";
    static gtASCIIString strSIG80_SIGNAL = "SIG80";
    static gtASCIIString strSIG81_SIGNAL = "SIG81";
    static gtASCIIString strSIG82_SIGNAL = "SIG82";
    static gtASCIIString strSIG83_SIGNAL = "SIG83";
    static gtASCIIString strSIG84_SIGNAL = "SIG84";
    static gtASCIIString strSIG85_SIGNAL = "SIG85";
    static gtASCIIString strSIG86_SIGNAL = "SIG86";
    static gtASCIIString strSIG87_SIGNAL = "SIG87";
    static gtASCIIString strSIG88_SIGNAL = "SIG88";
    static gtASCIIString strSIG89_SIGNAL = "SIG89";
    static gtASCIIString strSIG90_SIGNAL = "SIG90";
    static gtASCIIString strSIG91_SIGNAL = "SIG91";
    static gtASCIIString strSIG92_SIGNAL = "SIG92";
    static gtASCIIString strSIG93_SIGNAL = "SIG93";
    static gtASCIIString strSIG94_SIGNAL = "SIG94";
    static gtASCIIString strSIG95_SIGNAL = "SIG95";
    static gtASCIIString strSIG96_SIGNAL = "SIG96";
    static gtASCIIString strSIG97_SIGNAL = "SIG97";
    static gtASCIIString strSIG98_SIGNAL = "SIG98";
    static gtASCIIString strSIG99_SIGNAL = "SIG99";
    static gtASCIIString strSIG100_SIGNAL = "SIG100";
    static gtASCIIString strSIG101_SIGNAL = "SIG101";
    static gtASCIIString strSIG102_SIGNAL = "SIG102";
    static gtASCIIString strSIG103_SIGNAL = "SIG103";
    static gtASCIIString strSIG104_SIGNAL = "SIG104";
    static gtASCIIString strSIG105_SIGNAL = "SIG105";
    static gtASCIIString strSIG106_SIGNAL = "SIG106";
    static gtASCIIString strSIG107_SIGNAL = "SIG107";
    static gtASCIIString strSIG108_SIGNAL = "SIG108";
    static gtASCIIString strSIG109_SIGNAL = "SIG109";
    static gtASCIIString strSIG110_SIGNAL = "SIG110";
    static gtASCIIString strSIG111_SIGNAL = "SIG111";
    static gtASCIIString strSIG112_SIGNAL = "SIG112";
    static gtASCIIString strSIG113_SIGNAL = "SIG113";
    static gtASCIIString strSIG114_SIGNAL = "SIG114";
    static gtASCIIString strSIG115_SIGNAL = "SIG115";
    static gtASCIIString strSIG116_SIGNAL = "SIG116";
    static gtASCIIString strSIG117_SIGNAL = "SIG117";
    static gtASCIIString strSIG118_SIGNAL = "SIG118";
    static gtASCIIString strSIG119_SIGNAL = "SIG119";
    static gtASCIIString strSIG120_SIGNAL = "SIG120";
    static gtASCIIString strSIG121_SIGNAL = "SIG121";
    static gtASCIIString strSIG122_SIGNAL = "SIG122";
    static gtASCIIString strSIG123_SIGNAL = "SIG123";
    static gtASCIIString strSIG124_SIGNAL = "SIG124";
    static gtASCIIString strSIG125_SIGNAL = "SIG125";
    static gtASCIIString strSIG126_SIGNAL = "SIG126";
    static gtASCIIString strSIG127_SIGNAL = "SIG127";

    static gtASCIIString strSIGINFO_SIGNAL = "SIGINFO";

    if (nullptr != _pGDBCommunicationPipe && nullptr != _pGDBDriver)
    {
        std::string cmd = "-exec-interrupt --all\n";
        _wasGDBPrompt = false;
        _pGDBCommunicationPipe->write(cmd.c_str(), cmd.length());

        do
        {
            gtASCIIString output;
            readGDBOutputLine(output);

            if (output.find(s_gdbPromtStr) != -1)
            {
                _wasGDBPrompt = true;
            }

            GetStoppedThreadGDBId(output);
        }
        while (!_pGDBDriver->IsAllThreadsStopped() || !_wasGDBPrompt);

        _wasGDBPrompt = false;
    }

    // Will get true iff the signal represents a breakpoint hit:
    wasBreakpointHit = false;

    // Will get the exception reason, if exists:
    int exceptionReason = -1;

    // If the debugged process got a signal:
    int pos1 = gdbOutputLine.find(signalNameGDBStr);

    if (pos1 != -1)
    {
        int pos2 = pos1 + signalNameGDBStrLen;
        int pos3 = gdbOutputLine.find("\"", pos2);
        GT_IF_WITH_ASSERT(pos3 != -1)
        {
            // Get the signal name:
            gtASCIIString signalNameStr;
            gdbOutputLine.getSubString(pos2, pos3 - 1, signalNameStr);

            // If we got a breakpoint hit:
            if (signalNameStr == sigTrapGDBStr)
            {
                // If this breakpoint came from the spy, report it to the application. Otherwise,
                // treat it as a non-fatal signal (SIGTRAP is non-fatal):
                bool wasStoppedInSpy = false;
                bool rcLoc = isCurrentThreadStoppedInsideSpy(wasStoppedInSpy);
                GT_ASSERT(rcLoc);

                // Some applications (l3com case 7852, OpenSceneGraph) have a thread that sits in pthread_cond_wait, which is the same as the
                // breakpoint triggering thread.
                // This sometimes confuses gdb making it think that a different thread triggered the signal, causing spy breakpoints to be considered
                // as foreign breaks, and making the app become stuck. So, to be on the safe side, before deciding a SIGTRAP is a foreign break, we
                // check all threads to make sure none of them is inside the spy.
                if (!wasStoppedInSpy)
                {
                    wasStoppedInSpy = isAnyThreadStoppedInSpy();
                }

                wasBreakpointHit = wasStoppedInSpy;

                if (wasBreakpointHit)
                {
                    int triggeringThreadsId = GetStoppedThreadGDBId(gdbOutputLine);
                    /*
                                        if (nullptr != _pGDBCommunicationPipe && nullptr != _pGDBDriver)
                                        {
                                            std::string cmd = "-exec-interrupt --all\n";
                                            _pGDBCommunicationPipe->write(cmd.c_str(), cmd.length());

                                            do
                                            {
                                                gtASCIIString output;
                                                readGDBOutputLine(output);

                                                GetStoppedThreadId(output);
                                            } while (!_pGDBDriver->IsAllThreadsStopped());
                                        }
                                        */

                    _debuggedProcessCurrentThreadGDBId = triggeringThreadsId;
                    _debuggedProcessCurrentThreadId = threadIdFromGDBId(_debuggedProcessCurrentThreadGDBId);
                    wasBreakpointHit = handleBreakpointHit();
                }
                else // !wasBreakpointHit
                {
                    // This is a foreign breakpoint, so just treat it as a normal SIGTRAP signal:
                    exceptionReason = OS_SIGTRAP_SIGNAL;
                }
            }
            else if (signalNameStr == sigStopGDBStr)
            {
                // Stop signal, a foreign breakpoint:
                exceptionReason = OS_SIGSTOP_SIGNAL;
            }
            else if (signalNameStr == sigTTInGDBStr)
            {
                // Background read attempted from control terminal signal, a foreign breakpoint:
                exceptionReason = OS_SIGTTIN_SIGNAL;
            }
            else if (signalNameStr == sigTTOuGDBStr)
            {
                // Background write attempted from control terminal signal, a foreign breakpoint:
                exceptionReason = OS_SIGTTOU_SIGNAL;
            }
            else if (signalNameStr == sigStpGDBStr)
            {
                // Keyboard stop signal, a foreign breakpoint:
                exceptionReason = OS_SIGTSTP_SIGNAL;
            }
            else if (signalNameStr == sigHupGDBStr)
            {
                // Hangup detected on controlling terminal or death of controlling process:
                exceptionReason = OS_SIGHUP_SIGNAL;
            }
            else if (signalNameStr == sigQuitGDBStr)
                {
                    // Quit from keyboard:
                    exceptionReason = OS_SIGQUIT_SIGNAL;
                }
                else if (signalNameStr == sigIllGDBStr)
                {
                    // Illegal Instruction:
                    exceptionReason = OS_SIGILL_SIGNAL;
                }
                else if (signalNameStr == sigAbrtGDBStr)
                {
                    // Abort signal from abort(3):
                    exceptionReason = OS_SIGABRT_SIGNAL;
                }
                else if (signalNameStr == sigBugGDBStr)
                {
                    // Bus error:
                    exceptionReason = OS_SIGBUS_SIGNAL;
                }
                else if (signalNameStr == sigFpeGDBStr)
                {
                    // Floating point exception:
                    exceptionReason = OS_SIGFPE_SIGNAL;
                }
                else if (signalNameStr == sigKillGDBStr)
                {
                    // Kill signal:
                    exceptionReason = OS_SIGKILL_SIGNAL;
                }
                else if (signalNameStr == sigSegvGDBStr)
                {
                    // Invalid memory reference:
                    exceptionReason = OS_SIGSEGV_SIGNAL;
                }
                else if (signalNameStr == sigPipeGDBStr)
                {
                    // Broken pipe: write to pipe with no readers:
                    exceptionReason = OS_SIGPIPE_SIGNAL;
                }
                else if (signalNameStr == sigAlrmGDBStr)
                {
                    // Timer signal from alarm(2):
                    exceptionReason = OS_SIGALRM_SIGNAL;
                }
                else if (signalNameStr == sigTermGDBStr)
                {
                    // Termination signal:
                    exceptionReason = OS_SIGTERM_SIGNAL;
                }
                else if (signalNameStr == sigUsr1GDBStr)
                {
                    // User-defined signal 1:
                    exceptionReason = OS_SIGUSR1_SIGNAL;
                }
                else if (signalNameStr == sigUsr2GDBStr)
                {
                    // User-defined signal 2:
                    exceptionReason = OS_SIGUSR2_SIGNAL;
                }
                else if (signalNameStr == sigUsr2GDBStr)
                {
                    // User-defined signal 2:
                    exceptionReason = OS_SIGUSR2_SIGNAL;
                }
                else if (signalNameStr == strSIGEMT_SIGNAL)
                {
                    // Emulation trap
                    exceptionReason = OS_SIGEMT_SIGNAL;
                }
                else if (signalNameStr == strSIGSYS_SIGNAL)
                {
                    // Bad system call
                    exceptionReason = OS_SIGSYS_SIGNAL;
                }
                else if (signalNameStr == strSIGURG_SIGNAL)
                {
                    // Urgent I/O condition
                    exceptionReason = OS_SIGURG_SIGNAL;
                }
                else if (signalNameStr == strSIGSTOP_SIGNAL)
                {
                    // Stopped (signal)
                    exceptionReason = OS_SIGSTOP_SIGNAL;
                }
                else if (signalNameStr == strSIGTSTP_SIGNAL)
                {
                    // Stopped (user)
                    exceptionReason = OS_SIGTSTP_SIGNAL;
                }
                else if (signalNameStr == strSIGCONT_SIGNAL)
                {
                    // Continued
                    exceptionReason = OS_SIGCONT_SIGNAL;
                }
                else if (signalNameStr == strSIGCHLD_SIGNAL)
                {
                    // Child status changed
                    exceptionReason = OS_SIGCHLD_SIGNAL;
                }
                else if (signalNameStr == strSIGTTIN_SIGNAL)
                {
                    // Stopped (tty input)
                    exceptionReason = OS_SIGTTIN_SIGNAL;
                }
                else if (signalNameStr == strSIGTTOU_SIGNAL)
                {
                    // Stopped (tty output)
                    exceptionReason = OS_SIGTTOU_SIGNAL;
                }
                else if (signalNameStr == strSIGIO_SIGNAL)
                {
                    // I/O possible
                    exceptionReason = OS_SIGIO_SIGNAL;
                }
                else if (signalNameStr == strSIGXCPU_SIGNAL)
                {
                    // CPU time limit exceeded
                    exceptionReason = OS_SIGXCPU_SIGNAL;
                }
                else if (signalNameStr == strSIGXFSZ_SIGNAL)
                {
                    // File size limit exceeded
                    exceptionReason = OS_SIGXFSZ_SIGNAL;
                }
                else if (signalNameStr == strSIGVTALRM_SIGNAL)
                {
                    // Virtual timer expired
                    exceptionReason = OS_SIGVTALRM_SIGNAL;
                }
                else if (signalNameStr == strSIGPROF_SIGNAL)
                {
                    // Profiling timer expired
                    exceptionReason = OS_SIGPROF_SIGNAL;
                }
                else if (signalNameStr == strSIGWINCH_SIGNAL)
                {
                    // Window size changed
                    exceptionReason = OS_SIGWINCH_SIGNAL;
                }
                else if (signalNameStr == strSIGLOST_SIGNAL)
                {
                    // Resource lost
                    exceptionReason = OS_SIGLOST_SIGNAL;
                }
                else if (signalNameStr == strSIGPWR_SIGNAL)
                {
                    // Power fail/restart
                    exceptionReason = OS_SIGPWR_SIGNAL;
                }
                else if (signalNameStr == strSIGPOLL_SIGNAL)
                {
                    // Pollable event occurred
                    exceptionReason = OS_SIGPOLL_SIGNAL;
                }
                else if (signalNameStr == strSIGWIND_SIGNAL)
                {
                    // SIGWIND
                    exceptionReason = OS_SIGWIND_SIGNAL;
                }
                else if (signalNameStr == strSIGPHONE_SIGNAL)
                {
                    // SIGPHONE
                    exceptionReason = OS_SIGPHONE_SIGNAL;
                }
                else if (signalNameStr == strSIGWAITING_SIGNAL)
                {
                    // Process's LWPs are blocked
                    exceptionReason = OS_SIGWAITING_SIGNAL;
                }
                else if (signalNameStr == strSIGLWP_SIGNAL)
                {
                    // Signal LWP
                    exceptionReason = OS_SIGLWP_SIGNAL;
                }
                else if (signalNameStr == strSIGDANGER_SIGNAL)
                {
                    // Swap space dangerously low
                    exceptionReason = OS_SIGDANGER_SIGNAL;
                }
                else if (signalNameStr == strSIGGRANT_SIGNAL)
                {
                    // Monitor mode granted
                    exceptionReason = OS_SIGGRANT_SIGNAL;
                }
                else if (signalNameStr == strSIGRETRACT_SIGNAL)
                {
                    // Need to relinquish monitor mode
                    exceptionReason = OS_SIGRETRACT_SIGNAL;
                }
                else if (signalNameStr == strSIGMSG_SIGNAL)
                {
                    // Monitor mode data available
                    exceptionReason = OS_SIGMSG_SIGNAL;
                }
                else if (signalNameStr == strSIGSOUND_SIGNAL)
                {
                    // Sound completed
                    exceptionReason = OS_SIGSOUND_SIGNAL;
                }
                else if (signalNameStr == strSIGSAK_SIGNAL)
                {
                    // Secure attention
                    exceptionReason = OS_SIGSAK_SIGNAL;
                }
                else if (signalNameStr == strSIGPRIO_SIGNAL)
                {
                    // SIGPRIO
                    exceptionReason = OS_SIGPRIO_SIGNAL;
                }
                else if (signalNameStr == strSIGCANCEL_SIGNAL)
                {
                    // LWP internal signal
                    exceptionReason = OS_SIGCANCEL_SIGNAL;
                }
                else if (signalNameStr == strEXC_BAD_ACCESS_SIGNAL)
                {
                    // Could not access memory
                    exceptionReason = OS_EXC_BAD_ACCESS_SIGNAL;
                }
                else if (signalNameStr == strEXC_BAD_INSTRUCTION_SIGNAL)
                {
                    // Illegal instruction/operand
                    exceptionReason = OS_EXC_BAD_INSTRUCTION_SIGNAL;
                }
                else if (signalNameStr == strEXC_ARITHMETIC_SIGNAL)
                {
                    // Arithmetic exception
                    exceptionReason = OS_EXC_ARITHMETIC_SIGNAL;
                }
                else if (signalNameStr == strEXC_EMULATION_SIGNAL)
                {
                    // Emulation instruction
                    exceptionReason = OS_EXC_EMULATION_SIGNAL;
                }
                else if (signalNameStr == strEXC_SOFTWARE_SIGNAL)
                {
                    // Software generated exception
                    exceptionReason = OS_EXC_SOFTWARE_SIGNAL;
                }
                else if (signalNameStr == strEXC_BREAKPOINT_SIGNAL)
                {
                    // Breakpoint
                    exceptionReason = OS_EXC_BREAKPOINT_SIGNAL;
                }
                else if (signalNameStr == strSIG32_SIGNAL)
                {
                    // Real-time event 32
                    exceptionReason = OS_SIG32_SIGNAL;
                }
                else if (signalNameStr == strSIG33_SIGNAL)
                {
                    // Real-time event 33
                    exceptionReason = OS_SIG33_SIGNAL;
                }
                else if (signalNameStr == strSIG34_SIGNAL)
                {
                    // Real-time event 34
                    exceptionReason = OS_SIG34_SIGNAL;
                }
                else if (signalNameStr == strSIG35_SIGNAL)
                {
                    // Real-time event 35
                    exceptionReason = OS_SIG35_SIGNAL;
                }
                else if (signalNameStr == strSIG36_SIGNAL)
                {
                    // Real-time event 36
                    exceptionReason = OS_SIG36_SIGNAL;
                }
                else if (signalNameStr == strSIG37_SIGNAL)
                {
                    // Real-time event 37
                    exceptionReason = OS_SIG37_SIGNAL;
                }
                else if (signalNameStr == strSIG38_SIGNAL)
                {
                    // Real-time event 38
                    exceptionReason = OS_SIG38_SIGNAL;
                }
                else if (signalNameStr == strSIG39_SIGNAL)
                {
                    // Real-time event 39
                    exceptionReason = OS_SIG39_SIGNAL;
                }
                else if (signalNameStr == strSIG40_SIGNAL)
                {
                    // Real-time event 40
                    exceptionReason = OS_SIG40_SIGNAL;
                }
                else if (signalNameStr == strSIG41_SIGNAL)
                {
                    // Real-time event 41
                    exceptionReason = OS_SIG41_SIGNAL;
                }
                else if (signalNameStr == strSIG42_SIGNAL)
                {
                    // Real-time event 42
                    exceptionReason = OS_SIG42_SIGNAL;
                }
                else if (signalNameStr == strSIG43_SIGNAL)
                {
                    // Real-time event 43
                    exceptionReason = OS_SIG43_SIGNAL;
                }
                else if (signalNameStr == strSIG44_SIGNAL)
                {
                    // Real-time event 44
                    exceptionReason = OS_SIG44_SIGNAL;
                }
                else if (signalNameStr == strSIG45_SIGNAL)
                {
                    // Real-time event 45
                    exceptionReason = OS_SIG45_SIGNAL;
                }
                else if (signalNameStr == strSIG46_SIGNAL)
                {
                    // Real-time event 46
                    exceptionReason = OS_SIG46_SIGNAL;
                }
                else if (signalNameStr == strSIG47_SIGNAL)
                {
                    // Real-time event 47
                    exceptionReason = OS_SIG47_SIGNAL;
                }
                else if (signalNameStr == strSIG48_SIGNAL)
                {
                    // Real-time event 48
                    exceptionReason = OS_SIG48_SIGNAL;
                }
                else if (signalNameStr == strSIG49_SIGNAL)
                {
                    // Real-time event 49
                    exceptionReason = OS_SIG49_SIGNAL;
                }
                else if (signalNameStr == strSIG50_SIGNAL)
                {
                    // Real-time event 50
                    exceptionReason = OS_SIG50_SIGNAL;
                }
                else if (signalNameStr == strSIG51_SIGNAL)
                {
                    // Real-time event 51
                    exceptionReason = OS_SIG51_SIGNAL;
                }
                else if (signalNameStr == strSIG52_SIGNAL)
                {
                    // Real-time event 52
                    exceptionReason = OS_SIG52_SIGNAL;
                }
                else if (signalNameStr == strSIG53_SIGNAL)
                {
                    // Real-time event 53
                    exceptionReason = OS_SIG53_SIGNAL;
                }
                else if (signalNameStr == strSIG54_SIGNAL)
                {
                    // Real-time event 54
                    exceptionReason = OS_SIG54_SIGNAL;
                }
                else if (signalNameStr == strSIG55_SIGNAL)
                {
                    // Real-time event 55
                    exceptionReason = OS_SIG55_SIGNAL;
                }
                else if (signalNameStr == strSIG56_SIGNAL)
                {
                    // Real-time event 56
                    exceptionReason = OS_SIG56_SIGNAL;
                }
                else if (signalNameStr == strSIG57_SIGNAL)
                {
                    // Real-time event 57
                    exceptionReason = OS_SIG57_SIGNAL;
                }
                else if (signalNameStr == strSIG58_SIGNAL)
                {
                    // Real-time event 58
                    exceptionReason = OS_SIG58_SIGNAL;
                }
                else if (signalNameStr == strSIG59_SIGNAL)
                {
                    // Real-time event 59
                    exceptionReason = OS_SIG59_SIGNAL;
                }
                else if (signalNameStr == strSIG60_SIGNAL)
                {
                    // Real-time event 60
                    exceptionReason = OS_SIG60_SIGNAL;
                }
                else if (signalNameStr == strSIG61_SIGNAL)
                {
                    // Real-time event 61
                    exceptionReason = OS_SIG61_SIGNAL;
                }
                else if (signalNameStr == strSIG62_SIGNAL)
                {
                    // Real-time event 62
                    exceptionReason = OS_SIG62_SIGNAL;
                }
                else if (signalNameStr == strSIG63_SIGNAL)
                {
                    // Real-time event 63
                    exceptionReason = OS_SIG63_SIGNAL;
                }
                else if (signalNameStr == strSIG64_SIGNAL)
                {
                    // Real-time event 64
                    exceptionReason = OS_SIG64_SIGNAL;
                }
                else if (signalNameStr == strSIG65_SIGNAL)
                {
                    // Real-time event 65
                    exceptionReason = OS_SIG65_SIGNAL;
                }
                else if (signalNameStr == strSIG66_SIGNAL)
                {
                    // Real-time event 66
                    exceptionReason = OS_SIG66_SIGNAL;
                }
                else if (signalNameStr == strSIG67_SIGNAL)
                {
                    // Real-time event 67
                    exceptionReason = OS_SIG67_SIGNAL;
                }
                else if (signalNameStr == strSIG68_SIGNAL)
                {
                    // Real-time event 68
                    exceptionReason = OS_SIG68_SIGNAL;
                }
                else if (signalNameStr == strSIG69_SIGNAL)
                {
                    // Real-time event 69
                    exceptionReason = OS_SIG69_SIGNAL;
                }
                else if (signalNameStr == strSIG70_SIGNAL)
                {
                    // Real-time event 70
                    exceptionReason = OS_SIG70_SIGNAL;
                }
                else if (signalNameStr == strSIG71_SIGNAL)
                {
                    // Real-time event 71
                    exceptionReason = OS_SIG71_SIGNAL;
                }
                else if (signalNameStr == strSIG72_SIGNAL)
                {
                    // Real-time event 72
                    exceptionReason = OS_SIG72_SIGNAL;
                }
                else if (signalNameStr == strSIG73_SIGNAL)
                {
                    // Real-time event 73
                    exceptionReason = OS_SIG73_SIGNAL;
                }
                else if (signalNameStr == strSIG74_SIGNAL)
                {
                    // Real-time event 74
                    exceptionReason = OS_SIG74_SIGNAL;
                }
                else if (signalNameStr == strSIG75_SIGNAL)
                {
                    // Real-time event 75
                    exceptionReason = OS_SIG75_SIGNAL;
                }
                else if (signalNameStr == strSIG76_SIGNAL)
                {
                    // Real-time event 76
                    exceptionReason = OS_SIG76_SIGNAL;
                }
                else if (signalNameStr == strSIG77_SIGNAL)
                {
                    // Real-time event 77
                    exceptionReason = OS_SIG77_SIGNAL;
                }
                else if (signalNameStr == strSIG78_SIGNAL)
                {
                    // Real-time event 78
                    exceptionReason = OS_SIG78_SIGNAL;
                }
                else if (signalNameStr == strSIG79_SIGNAL)
                {
                    // Real-time event 79
                    exceptionReason = OS_SIG79_SIGNAL;
                }
                else if (signalNameStr == strSIG80_SIGNAL)
                {
                    // Real-time event 80
                    exceptionReason = OS_SIG80_SIGNAL;
                }
                else if (signalNameStr == strSIG81_SIGNAL)
                {
                    // Real-time event 81
                    exceptionReason = OS_SIG81_SIGNAL;
                }
                else if (signalNameStr == strSIG82_SIGNAL)
                {
                    // Real-time event 82
                    exceptionReason = OS_SIG82_SIGNAL;
                }
                else if (signalNameStr == strSIG83_SIGNAL)
                {
                    // Real-time event 83
                    exceptionReason = OS_SIG83_SIGNAL;
                }
                else if (signalNameStr == strSIG84_SIGNAL)
                {
                    // Real-time event 84
                    exceptionReason = OS_SIG84_SIGNAL;
                }
                else if (signalNameStr == strSIG85_SIGNAL)
                {
                    // Real-time event 85
                    exceptionReason = OS_SIG85_SIGNAL;
                }
                else if (signalNameStr == strSIG86_SIGNAL)
                {
                    // Real-time event 86
                    exceptionReason = OS_SIG86_SIGNAL;
                }
                else if (signalNameStr == strSIG87_SIGNAL)
                {
                    // Real-time event 87
                    exceptionReason = OS_SIG87_SIGNAL;
                }
                else if (signalNameStr == strSIG88_SIGNAL)
                {
                    // Real-time event 88
                    exceptionReason = OS_SIG88_SIGNAL;
                }
                else if (signalNameStr == strSIG89_SIGNAL)
                {
                    // Real-time event 89
                    exceptionReason = OS_SIG89_SIGNAL;
                }
                else if (signalNameStr == strSIG90_SIGNAL)
                {
                    // Real-time event 90
                    exceptionReason = OS_SIG90_SIGNAL;
                }
                else if (signalNameStr == strSIG91_SIGNAL)
                {
                    // Real-time event 91
                    exceptionReason = OS_SIG91_SIGNAL;
                }
                else if (signalNameStr == strSIG92_SIGNAL)
                {
                    // Real-time event 92
                    exceptionReason = OS_SIG92_SIGNAL;
                }
                else if (signalNameStr == strSIG93_SIGNAL)
                {
                    // Real-time event 93
                    exceptionReason = OS_SIG93_SIGNAL;
                }
                else if (signalNameStr == strSIG94_SIGNAL)
                {
                    // Real-time event 94
                    exceptionReason = OS_SIG94_SIGNAL;
                }
                else if (signalNameStr == strSIG95_SIGNAL)
                {
                    // Real-time event 95
                    exceptionReason = OS_SIG95_SIGNAL;
                }
                else if (signalNameStr == strSIG96_SIGNAL)
                {
                    // Real-time event 96
                    exceptionReason = OS_SIG96_SIGNAL;
                }
                else if (signalNameStr == strSIG97_SIGNAL)
                {
                    // Real-time event 97
                    exceptionReason = OS_SIG97_SIGNAL;
                }
                else if (signalNameStr == strSIG98_SIGNAL)
                {
                    // Real-time event 98
                    exceptionReason = OS_SIG98_SIGNAL;
                }
                else if (signalNameStr == strSIG99_SIGNAL)
                {
                    // Real-time event 99
                    exceptionReason = OS_SIG99_SIGNAL;
                }
                else if (signalNameStr == strSIG100_SIGNAL)
                {
                    // Real-time event 100
                    exceptionReason = OS_SIG100_SIGNAL;
                }
                else if (signalNameStr == strSIG101_SIGNAL)
                {
                    // Real-time event 101
                    exceptionReason = OS_SIG101_SIGNAL;
                }
                else if (signalNameStr == strSIG102_SIGNAL)
                {
                    // Real-time event 102
                    exceptionReason = OS_SIG102_SIGNAL;
                }
                else if (signalNameStr == strSIG103_SIGNAL)
                {
                    // Real-time event 103
                    exceptionReason = OS_SIG103_SIGNAL;
                }
                else if (signalNameStr == strSIG104_SIGNAL)
                {
                    // Real-time event 104
                    exceptionReason = OS_SIG104_SIGNAL;
                }
                else if (signalNameStr == strSIG105_SIGNAL)
                {
                    // Real-time event 105
                    exceptionReason = OS_SIG105_SIGNAL;
                }
                else if (signalNameStr == strSIG106_SIGNAL)
                {
                    // Real-time event 106
                    exceptionReason = OS_SIG106_SIGNAL;
                }
                else if (signalNameStr == strSIG107_SIGNAL)
                {
                    // Real-time event 107
                    exceptionReason = OS_SIG107_SIGNAL;
                }
                else if (signalNameStr == strSIG108_SIGNAL)
                {
                    // Real-time event 108
                    exceptionReason = OS_SIG108_SIGNAL;
                }
                else if (signalNameStr == strSIG109_SIGNAL)
                {
                    // Real-time event 109
                    exceptionReason = OS_SIG109_SIGNAL;
                }
                else if (signalNameStr == strSIG110_SIGNAL)
                {
                    // Real-time event 110
                    exceptionReason = OS_SIG110_SIGNAL;
                }
                else if (signalNameStr == strSIG111_SIGNAL)
                {
                    // Real-time event 111
                    exceptionReason = OS_SIG111_SIGNAL;
                }
                else if (signalNameStr == strSIG112_SIGNAL)
                {
                    // Real-time event 112
                    exceptionReason = OS_SIG112_SIGNAL;
                }
                else if (signalNameStr == strSIG113_SIGNAL)
                {
                    // Real-time event 113
                    exceptionReason = OS_SIG113_SIGNAL;
                }
                else if (signalNameStr == strSIG114_SIGNAL)
                {
                    // Real-time event 114
                    exceptionReason = OS_SIG114_SIGNAL;
                }
                else if (signalNameStr == strSIG115_SIGNAL)
                {
                    // Real-time event 115
                    exceptionReason = OS_SIG115_SIGNAL;
                }
                else if (signalNameStr == strSIG116_SIGNAL)
                {
                    // Real-time event 116
                    exceptionReason = OS_SIG116_SIGNAL;
                }
                else if (signalNameStr == strSIG117_SIGNAL)
                {
                    // Real-time event 117
                    exceptionReason = OS_SIG117_SIGNAL;
                }
                else if (signalNameStr == strSIG118_SIGNAL)
                {
                    // Real-time event 118
                    exceptionReason = OS_SIG118_SIGNAL;
                }
                else if (signalNameStr == strSIG119_SIGNAL)
                {
                    // Real-time event 119
                    exceptionReason = OS_SIG119_SIGNAL;
                }
                else if (signalNameStr == strSIG120_SIGNAL)
                {
                    // Real-time event 120
                    exceptionReason = OS_SIG120_SIGNAL;
                }
                else if (signalNameStr == strSIG121_SIGNAL)
                {
                    // Real-time event 121
                    exceptionReason = OS_SIG121_SIGNAL;
                }
                else if (signalNameStr == strSIG122_SIGNAL)
                {
                    // Real-time event 122
                    exceptionReason = OS_SIG122_SIGNAL;
                }
                else if (signalNameStr == strSIG123_SIGNAL)
                {
                    // Real-time event 123
                    exceptionReason = OS_SIG123_SIGNAL;
                }
                else if (signalNameStr == strSIG124_SIGNAL)
                {
                    // Real-time event 124
                    exceptionReason = OS_SIG124_SIGNAL;
                }
                else if (signalNameStr == strSIG125_SIGNAL)
                {
                    // Real-time event 125
                    exceptionReason = OS_SIG125_SIGNAL;
                }
                else if (signalNameStr == strSIG126_SIGNAL)
                {
                    // Real-time event 126
                    exceptionReason = OS_SIG126_SIGNAL;
                }
                else if (signalNameStr == strSIG127_SIGNAL)
                {
                    // Real-time event 127
                    exceptionReason = OS_SIG127_SIGNAL;
                }
                else if (signalNameStr == strSIGINFO_SIGNAL)
                {
                    // Information request
                    exceptionReason = OS_SIGINFO_SIGNAL;
                }
                else if ((signalNameStr == sigThreadStopped) || (signalNameStr == sigIntGDBStr))
                {
                    int threadId = GetStoppedThreadGDBId(gdbOutputLine);

                    if (threadId == -1)
                    {
                        _wasDebuggedProcessSuspended = _pGDBDriver->IsAllThreadsStopped();
                    }
                    else
                    {
                        _wasDebuggedProcessSuspended = true;
                    }

                    exceptionReason = OS_STANDALONE_THREAD_STOPPED;
                }
                else
                {
                    // An unknown signal was received by the debugged process:
                    exceptionReason = OS_UNKNOWN_EXCEPTION_REASON;

                    // Assert:
                    gtString errorMsg; errorMsg.fromASCIIString(signalNameStr.asCharArray());
                    errorMsg.prepend(L": ").prepend(PD_STR_unknownSignalReceived);
                    GT_ASSERT_EX(false, errorMsg.asCharArray());
                }

            // If the debugged process encountered an exception:
            if (exceptionReason != -1)
            {
                if (OS_STANDALONE_THREAD_STOPPED != exceptionReason)
                {
                    handleException(exceptionReason);
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////
/// \brief Parsing "*stopped..." gdb output line and find a stopped thread id
///
/// \param gdbOutputLine a line readed from gdb output pipe
///
/// \return stoped thread Id, 0 - in case all threads stopped, -1 in case error
///
/// \author Vadim Entov
/// \date 14/1/2016
int pdGDBOutputReader::GetStoppedThreadGDBId(const gtASCIIString& gdbOutputLine)
{
    int result = -1;

    GT_IF_WITH_ASSERT(_pGDBDriver)
    {
        int pos1 = gdbOutputLine.find(s_stoppedThreads);

        while (-1 != pos1)
        {
            int length = s_stoppedThreads.length();
            int pos2 = gdbOutputLine.find("\"]", pos1 + length);

            GT_IF_WITH_ASSERT(-1 != pos2)
            {
                gtASCIIString strThreadId;
                gdbOutputLine.getSubString(pos1 + length, pos2 - 1, strThreadId);

                int threadGDBId = 0;

                GT_IF_WITH_ASSERT(strThreadId.toIntNumber(threadGDBId))
                {
                    _pGDBDriver->OnThreadGDBStopped(threadGDBId);
                    result = threadGDBId;
                }
            }

            pos1 = gdbOutputLine.find(s_stoppedThreads, pos2);
        }

        pos1 = gdbOutputLine.find(s_exitingThread);

        while (-1 != pos1)
        {
            int length = s_exitingThread.length();
            int pos2 = gdbOutputLine.find("\"", pos1 + length);

            GT_IF_WITH_ASSERT(-1 != pos2)
            {
                gtASCIIString strThreadId;
                gdbOutputLine.getSubString(pos1 + length, pos2 - 1, strThreadId);

                int threadGDBId = 0;

                GT_IF_WITH_ASSERT(strThreadId.toIntNumber(threadGDBId))
                {
                    _pGDBDriver->OnThreadExit(threadGDBId);
                    result = threadGDBId;
                }
            }

            pos1 = gdbOutputLine.find(s_exitingThread, pos2);
        }

        pos1 = gdbOutputLine.find("bkptno");

        while (pos1 != -1)
        {
            // Host breakpoint
            pos1 = gdbOutputLine.find("thread-id=\"", pos1);

            if (-1 != pos1)
            {
                int length = ::strlen("thread-id=\"");
                int pos2 = gdbOutputLine.find("\"", pos1 + length);

                GT_IF_WITH_ASSERT(-1 != pos2)
                {
                    gtASCIIString strThreadId;
                    gdbOutputLine.getSubString(pos1 + length, pos2 - 1, strThreadId);

                    int threadGDBId = 0;

                    GT_IF_WITH_ASSERT(strThreadId.toIntNumber(threadGDBId))
                    {
                        _pGDBDriver->OnThreadGDBStopped(threadGDBId);
                    }
                }

                pos1 = gdbOutputLine.find("bkptno", pos2);
            }
        }
    }

    return result;
}

/////////////////////////////////////////////////////////////////
/// \brief Parse "*running, thread-id="... " message
///
/// \param gdbOutputLine a parsing gdb output string
/// \return threadId or -1
///
/// \author Vadim Entov
/// \date 18/01/2016
int pdGDBOutputReader::GetRunningThreadGDBId(const gtASCIIString& gdbOutputLine)
{
    int result = -1;

    GT_IF_WITH_ASSERT(_pGDBDriver)
    {
        int pos1 = gdbOutputLine.find(s_runningThreadMsg);

        while (-1 != pos1)
        {
            int length = s_runningThreadMsg.length();
            int pos2 = gdbOutputLine.find("\"", pos1 + length);

            GT_IF_WITH_ASSERT(-1 != pos2)
            {
                gtASCIIString strThreadId;
                gdbOutputLine.getSubString(pos1 + length, pos2 - 1, strThreadId);

                int threadGDBId = 0;

                GT_IF_WITH_ASSERT(strThreadId.toIntNumber(threadGDBId))
                {
                    _pGDBDriver->OnThreadGDBResumed(threadGDBId);
                    result = threadGDBId;
                }
            }

            pos1 = gdbOutputLine.find(s_runningThreadMsg, pos2);
        }

        pos1 = gdbOutputLine.find("bkptno");

        while (pos1 != -1)
        {
            // Host breakpoint
            pos1 = gdbOutputLine.find("thread-id=\"", pos1);

            if (-1 != pos1)
            {
                int length = ::strlen("thread-id=\"");
                int pos2 = gdbOutputLine.find("\"", pos1 + length);

                GT_IF_WITH_ASSERT(-1 != pos2)
                {
                    gtASCIIString strThreadId;
                    gdbOutputLine.getSubString(pos1 + length, pos2 - 1, strThreadId);

                    int threadGDBId = 0;

                    GT_IF_WITH_ASSERT(strThreadId.toIntNumber(threadGDBId))
                    {
                        _pGDBDriver->OnThreadGDBStopped(threadGDBId);
                    }
                }

                pos1 = gdbOutputLine.find("bkptno", pos2);
            }
        }

    }

    return result;
}

////////////////////////////////////////////////////////////////////
/// \brief Parsing "thread exited" message
/// \param gdbOutputLine - GDB's output line.
///
/// \author Vadim Entov
/// \date 01/02/2016
void pdGDBOutputReader::handleExitThreadMessage(const gtASCIIString& gdbOutputLine)
{
    static gtASCIIString s_StartStr = "=thread-exited,id=\"";

    // Get the OS LWP id
    unsigned long gdbThreadId = 0;
    int pos1 = gdbOutputLine.find(s_StartStr);

    if (pos1 != -1 && nullptr != _pGDBDriver)
    {
        int pos2 = pos1 + s_StartStr.length();
        int pos3 = gdbOutputLine.find("\"", pos2);

        if (pos3 != -1)
        {
            gtASCIIString idString;
            gdbOutputLine.getSubString(pos2, pos3, idString);
            bool rc1 = idString.toUnsignedLongNumber(gdbThreadId);
            GT_ASSERT(rc1);
            _pGDBDriver->OnThreadExit(gdbThreadId);

            if (0 == _pGDBDriver->GetRunnungThreadsCount())
            {
                _wasDebuggedProcessCreated = false;
                _wasDebuggedProcessSuspended = false;
                _wasDebuggedProcessTerminated = true;
                m_didDebuggedProcessReceiveFatalSignal = false;
                _debuggedProcessCurrentThreadGDBId = -1;
                _debuggedProcessCurrentThreadId = OS_NO_THREAD_ID;
                _processId = 0;

                // Trigger a process terminated event:
                apDebuggedProcessTerminatedEvent* pDebuggedProcessTerminationEvent = new apDebuggedProcessTerminatedEvent(0);
                m_eventsToRegister.push_back(pDebuggedProcessTerminationEvent);
            }
        }
    }
}

bool pdGDBOutputReader::handleHostSteps(const gtASCIIString& gdbOutputLine, const pdGDBData** ppGDBOutputData)
{
    GT_UNREFERENCED_PARAMETER(ppGDBOutputData);

    bool retVal = true;

    if (gdbOutputLine.find("*running") != -1)
    {
        if (gdbOutputLine.find("*stopped") == -1)
        {
            GetRunningThreadGDBId(gdbOutputLine);
        }
        else
        {
            gtASCIIString stoppedStr = "";
            gdbOutputLine.getSubString(gdbOutputLine.find("*stopped"), gdbOutputLine.length() - 1, stoppedStr);
            retVal = handleAsynchronousOutput(gdbOutputLine);
        }
    }
    else if (gdbOutputLine.find("*stopped") != -1)
    {
        gtASCIIString stoppedStr = "";
        gdbOutputLine.getSubString(gdbOutputLine.find("*stopped"), gdbOutputLine.length() - 1, stoppedStr);
        retVal = handleAsynchronousOutput(gdbOutputLine);
    }
    else if (gdbOutputLine.find("^error") != -1)
    {
        /// Step Out from main. gdb can't proceed "-exec-finish" from main frame and
        /// print error: not meaningful in the outermost frame
        /// Application have to catch this issue and just resume continue.

        GT_IF_WITH_ASSERT(ppGDBOutputData)
        {
            *ppGDBOutputData = new pdGDBHostStepErrorInfoIndex(gdbOutputLine);

            if (gdbOutputLine.find("not meaningful in the outermost frame") == -1)
            {
                retVal = false;
            }
        }
        else
        {
            retVal = false;
        }
    }

    return retVal;
}

bool pdGDBOutputReader::handleGetVariableTypeGDBOutput(const gtASCIIString& gdbOutputLine, const pdGDBData** ppGDBOutputData)
{
    bool result = false;

    const gtASCIIString startTypeAnswer = "type = ";

    int posStart = gdbOutputLine.find(startTypeAnswer);

    if (-1 != posStart)
    {
        posStart += startTypeAnswer.length();
        int posEnd = gdbOutputLine.find("\n", posStart);

        GT_IF_WITH_ASSERT(-1 != posEnd)
        {
            gtASCIIString variableType = "";

            posEnd -= 4;
            gdbOutputLine.getSubString(posStart, posEnd, variableType);

            GT_IF_WITH_ASSERT(nullptr != ppGDBOutputData)
            {
                gtString vType;
                vType.fromASCIIString(variableType.asCharArray());

                *ppGDBOutputData = new pdGDBFVariableType(vType);
                result = true;
            }
        }
    }
    else
    {
        if (gdbOutputLine.find("^error") != -1)
        {
            GT_IF_WITH_ASSERT(gdbOutputLine.find("No symbol") != -1 && gdbOutputLine.find("in current context") != -1)
            {
                // Legal situation. Symbol type not found in release build
                *ppGDBOutputData = new pdGDBFVariableType(L"No symbol in current context");
                result = true;
            }
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleNewThreadMessage
// Description: Is called when GDB outputs the ""[New Thread..."
//              message.
// Arguments: gdbOutputLine - GDB's output line.
// Author:      Yaki Tebeka
// Date:        8/7/2007
// ---------------------------------------------------------------------------
void pdGDBOutputReader::handleNewThreadMessage(const gtASCIIString& gdbOutputLine)
{
#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    {
        // Uri, 8/3/09: The Mac version of gdb does not currently report thread created
        // events at all, so we do not need to translate these output strings to debugged
        // process events. We leave this #ifdef-ed out, since if the Mac gdb would at some
        // point report these events, they would probably not be in the same form (LWP
        // does not exist in Mac, for one)
        OS_OUTPUT_DEBUG_LOG(L"Ignoring thread creation output", OS_DEBUG_LOG_DEBUG);
    }
#else
    {
        static gtASCIIString s_gdbfindLwpIdStr = "(LWP ";
        static gtASCIIString s_gdbfindThreadIdStr1 = "[New Thread ";
        static gtASCIIString s_gdbfindThreadIdStr2 = "=thread-created,id=\"";

        // Get the OS LWP id
        osThreadId lwpOSId = OS_NO_THREAD_ID;
        int pos1 = gdbOutputLine.find(s_gdbfindLwpIdStr);

        if (pos1 != -1)
        {
            int pos2 = pos1 + s_gdbfindLwpIdStr.length();
            int pos3 = gdbOutputLine.find(')', pos2);

            if (pos3 != -1)
            {
                gtASCIIString lwpIdString;
                gdbOutputLine.getSubString(pos2, pos3, lwpIdString);
                bool rc1 = lwpIdString.toUnsignedLongNumber(lwpOSId);
                GT_ASSERT(rc1);
            }
        }

        // Get the OS thread id
        osThreadId threadOSId = OS_NO_THREAD_ID;
        int pos11 = gdbOutputLine.find(s_gdbfindThreadIdStr1);


        if (pos11 != -1)
        {
            int pos12 = pos11 + s_gdbfindThreadIdStr1.length();
            int pos13 = gdbOutputLine.find(" (", pos12);

            if (pos13 != -1)
            {
                gtASCIIString threadIdString;
                gdbOutputLine.getSubString(pos12, pos13, threadIdString);
                bool rc1 = threadIdString.toUnsignedLongNumber(threadOSId);
                GT_ASSERT(rc1);
            }
        }
        else
        {
            pos11 = gdbOutputLine.find(s_gdbfindThreadIdStr2);

            if (pos11 != -1)
            {
                int pos12 = pos11 + s_gdbfindThreadIdStr2.length();
                int pos13 = gdbOutputLine.find("\"", pos12);

                if (pos13 != -1)
                {
                    gtASCIIString threadIdString;
                    gdbOutputLine.getSubString(pos12, pos13 - 1, threadIdString);
                    int threadGDBId = 0;

                    GT_IF_WITH_ASSERT(threadIdString.toIntNumber(threadGDBId))
                    {
                        GT_IF_WITH_ASSERT(_pGDBDriver)
                        {
                            _pGDBDriver->OnThreadCreated(threadGDBId);
                        }
                    }
                }
            }

            return;
        }

        // Get the OS thread start address:
        osInstructionPointer threadStartAddress = NULL;

        int pos4 = s_newThreadMsg.length();
        int pos5 = gdbOutputLine.find(' ', pos4);

        if (pos5 != -1)
        {
            gtASCIIString threadStartAddressStr;
            gdbOutputLine.getSubString(pos4, pos5, threadStartAddressStr);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            int rc1 = 0;

            if (_debuggedExecutableArchitecture == OS_I386_ARCHITECTURE)
            {
                rc1 = ::sscanf(threadStartAddressStr.asCharArray(), "%p", &threadStartAddress);
            }
            else if (_debuggedExecutableArchitecture == OS_X86_64_ARCHITECTURE)
            {
                rc1 = ::sscanf(threadStartAddressStr.asCharArray(), "0x%llx", &threadStartAddress);
            }
            else
            {
                // Unsupported or unknown architecture, we should not get here!
                GT_ASSERT(false);
            }

#else
            int rc1 = ::sscanf(threadStartAddressStr.asCharArray(), "%p", &threadStartAddress);
#endif
            GT_ASSERT(rc1 == 1);
        }

        // Get the current time:
        osTime threadCreationTime;
        threadCreationTime.setFromCurrentTime();

        // Raise an appropriate event:
        apThreadCreatedEvent* pThreadCreatedEvent = new apThreadCreatedEvent(threadOSId, lwpOSId, threadCreationTime, threadStartAddress);
        m_eventsToRegister.push_back(pThreadCreatedEvent);
    }
#endif
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleSwitchingToThreadMessage
// Description: Is called when GDB outputs the "[Switching to Thread..."
//              message.
// Arguments: gdbOutputLine - GDB's output line.
// Author:      Yaki Tebeka
// Date:        8/7/2007
// ---------------------------------------------------------------------------
void pdGDBOutputReader::handleSwitchingToThreadMessage(const gtASCIIString& gdbOutputLine)
{
    static int sizeOfSwitchingThreadMgs = s_switchingToThreadMsg.length();

    // Look for the position of the "[Switching to Thread..." message:
    int switchingThreadMsgPos = gdbOutputLine.find(s_switchingToThreadMsg);
    GT_IF_WITH_ASSERT(switchingThreadMsgPos != -1)
    {
        // Look for the space that ends the thread id:
        int pos1 = gdbOutputLine.find(' ', switchingThreadMsgPos + sizeOfSwitchingThreadMgs + 1);
        GT_IF_WITH_ASSERT(pos1 != -1)
        {
            // Get the thread id string:
            gtASCIIString threadIdAsString;
            gdbOutputLine.getSubString(switchingThreadMsgPos + sizeOfSwitchingThreadMgs + 1, pos1 - 1, threadIdAsString);

            // Translate it to osThreadId:
            int threadGDBId = 0;
            bool rc1 = threadIdAsString.toIntNumber(threadGDBId);
            GT_IF_WITH_ASSERT(rc1)
            {
                _debuggedProcessCurrentThreadGDBId = threadGDBId;
                _debuggedProcessCurrentThreadId = threadIdFromGDBId(_debuggedProcessCurrentThreadGDBId);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleSwitchingToProcessMessage
// Description: Is called when GDB outputs the "[Switching to process..."
//              message.
// Arguments: gdbOutputLine - GDB's output line.
// Author:      Yaki Tebeka
// Date:        17/12/2008
// Implementation notes:
// On Leopard and Linux, the message format is: ~[Switching to process 15109 local thread 0x2e03]\n
// On Snow Leopard, the message format is: ~[Switching to process]\n
// ---------------------------------------------------------------------------
void pdGDBOutputReader::handleSwitchingToProcessMessage(const gtASCIIString& gdbOutputLine)
{
    static int sizeOfSwitchingToProcessMgs = s_switchingToProcessMsg.length();

    // Look for the position of the "[Switching to process..." message:
    int switchingToProcessMsgPos = gdbOutputLine.find(s_switchingToProcessMsg);
    GT_IF_WITH_ASSERT(switchingToProcessMsgPos != -1)
    {
        // Look for the space that ends the process id:
        int pos1 = gdbOutputLine.find(' ', switchingToProcessMsgPos + sizeOfSwitchingToProcessMgs + 1);

        if (pos1 == -1)
        {
            // On Snow leopard, we need to look for the ] that ends the process id:
            pos1 = gdbOutputLine.find(']', switchingToProcessMsgPos + sizeOfSwitchingToProcessMgs + 1);
        }

        GT_IF_WITH_ASSERT(pos1 != -1)
        {
            // Get the process id string:
            gtASCIIString processIdAsString;
            gdbOutputLine.getSubString(switchingToProcessMsgPos + sizeOfSwitchingToProcessMgs + 1, pos1 - 1, processIdAsString);

            // Translate it to osProcessId:
            long debuggedProcessId = 0;
            bool rc1 = processIdAsString.toLongNumber(debuggedProcessId);
            GT_IF_WITH_ASSERT(rc1)
            {
                // If under debug log severity:
                if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
                {
                    // Output a debug string:
                    gtString dbgMsg = PD_STR_debuggedProcessPID;
                    dbgMsg.appendFormattedString(L"%d", debuggedProcessId);
                    OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
                }

                // Don't notify this more than once (Apple's gdb V. 1344 - 6.3.50 shows "switch to process" for every break)
                if (!_wasDebuggedProcessCreated)
                {
                    _wasDebuggedProcessCreated = true;

                    // Notify that the debugged process run started:
                    osTime processRunStartedTime;
                    processRunStartedTime.setFromCurrentTime();
                    apDebuggedProcessRunStartedEvent* pProcessRunStartedEvent = new apDebuggedProcessRunStartedEvent(debuggedProcessId, processRunStartedTime);
                    m_eventsToRegister.push_back(pProcessRunStartedEvent);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleSwitchingToProcessAndThreadMessage
// Description: Handle the output of thread changing in Mac (which also reports
//              the process name)
// Author:      Uri Shomroni
// Date:        25/2/2009
// ---------------------------------------------------------------------------
void pdGDBOutputReader::handleSwitchingToProcessAndThreadMessage(const gtASCIIString& gdbOutputLine)
{
    static const gtASCIIString processIndicator = "process";
    static const gtASCIIString threadIndicator = "thread";
    static const gtASCIIString localIndicator = "local";
    unsigned long processId = 0;
    int threadGDBId = 0;
    bool isLocal = false;

    gtASCIIStringTokenizer strTokenizer(gdbOutputLine, " ");
    gtASCIIString currentToken;

    while (strTokenizer.getNextToken(currentToken))
    {
        if (currentToken.compareNoCase(processIndicator) == 0)
        {
            // This is the process ID:
            bool rcToken = strTokenizer.getNextToken(currentToken);
            GT_IF_WITH_ASSERT(rcToken)
            {
                bool rcNum = currentToken.toUnsignedLongNumber(processId);

                if (!rcNum)
                {
                    GT_ASSERT(rcNum);
                    processId = 0;
                }
            }
        }
        else if (currentToken.compareNoCase(threadIndicator) == 0)
        {
            bool rcToken = strTokenizer.getNextToken(currentToken);
            GT_IF_WITH_ASSERT(rcToken)
            {
                bool rcNum = currentToken.toIntNumber(threadGDBId);

                if (!rcNum)
                {
                    GT_ASSERT(rcNum);
                    threadGDBId = -1;
                }
            }
        }
        else if (currentToken.compareNoCase(localIndicator) == 0)
        {
            bool rcToken = strTokenizer.getNextToken(currentToken);
            GT_IF_WITH_ASSERT(rcToken)
            {
                // Verify the next token is the "thread", then skip it:
                GT_IF_WITH_ASSERT(currentToken.compareNoCase(threadIndicator) == 0)
                {
                    rcToken = strTokenizer.getNextToken(currentToken);
                    GT_IF_WITH_ASSERT(rcToken)
                    {
                        threadGDBId = -1;
                        isLocal = true;
                        bool rcNum = currentToken.toIntNumber(threadGDBId);

                        if (!rcNum)
                        {
                            GT_ASSERT(rcNum);
                            threadGDBId = -1;
                        }
                    }
                }
            }
        }
    }

    GT_IF_WITH_ASSERT((processId != 0) && (threadGDBId != 0))
    {
        // If under debug log severity:
        if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
        {
            // Output a debug string:
            gtString dbgMsg = PD_STR_debuggedProcessPID;
            dbgMsg.appendFormattedString(L"%d", processId);
            dbgMsg.appendFormattedString(PD_STR_debuggedProcessCurrentTID, threadGDBId);
            OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }

        // We have no way to handle local thread ids, so we only register if the thread
        // is not local (this event is pretty much ignored anyway)
        if (!isLocal)
        {
            _debuggedProcessCurrentThreadGDBId = threadGDBId;
            _debuggedProcessCurrentThreadId = threadIdFromGDBId(_debuggedProcessCurrentThreadGDBId);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleBreakpointHit
// Description: Is called when the debugged process hits a breakpoint
// Author:      Yaki Tebeka
// Date:        27/6/2007
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleBreakpointHit(bool isGDBBreakpoint, bool isStep)
{
    // Will get true iff we want to consider this breakpoint as a real breakpoint:
    bool retVal = true;
    bool sendEvent = true;

    // Ignore the breakpoints used to synchronize the beginning and end of kernel debugging:
    apBreakReason breakReason = AP_FOREIGN_BREAK_HIT;

    if (_isKernelDebuggingAboutToStart)
    {
        retVal = false;
        _isKernelDebuggingAboutToStart = false;
        breakReason = AP_BEFORE_KERNEL_DEBUGGING_HIT;
    }
    else if (_isKernelDebuggingJustFinished)
    {
        retVal = false;
        _isKernelDebuggingJustFinished = false;
        breakReason = AP_AFTER_KERNEL_DEBUGGING_HIT;
    }
    else if (isGDBBreakpoint)
    {
        breakReason = AP_HOST_BREAKPOINT_HIT;
    }
    else if (isStep)
    {
        // TO_DO:
        // breakReason = m_lastStepKind;
        breakReason = AP_STEP_OVER_BREAKPOINT_HIT;
    }

    // If this is a breakpoint we want to report:
    if (sendEvent)
    {
        // Trigger a breakpoint hit event:
        apBreakpointHitEvent* pBreakpointHitEvent = new apBreakpointHitEvent(_debuggedProcessCurrentThreadId, NULL);

        if (breakReason != AP_FOREIGN_BREAK_HIT)
        {
            pBreakpointHitEvent->setBreakReason(breakReason);
        }

        m_eventsToRegister.push_back(pBreakpointHitEvent);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleException
// Description: Is called when the debugged process gets an exception (=signal)
// Arguments: excetionReason - The exception reason in osExceptionReason values.
// Author:      Yaki Tebeka
// Date:        27/6/2007
// ---------------------------------------------------------------------------
void pdGDBOutputReader::handleException(int excetionReason)
{
    // Trigger a "first chance" exception event:
    apExceptionEvent* pExceptionEvent = new apExceptionEvent(_debuggedProcessCurrentThreadId, (osExceptionReason)excetionReason, NULL, false);
    m_didDebuggedProcessReceiveFatalSignal = pExceptionEvent->isFatalLinuxSignal();
    m_eventsToRegister.push_back(pExceptionEvent);

    // If this is a crash signal, note that the debugged process is suspended before crashing:
    if (m_didDebuggedProcessReceiveFatalSignal)
    {
        _wasDebuggedProcessSuspended = true;
    }
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleSharedModuleLoadedMessage
// Description: Is called when a module is loaded into the debugged process address space.
// Arguments: gdbOutputLine - GDB's output line.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/1/2009
// Implementation notes:
//  Below are example module loaded GDB output:
//   =shlibs-added,shlib-info=[num="3",name="Carbon",kind="F",dyld-addr="0x940fd000",reason="dyld",requested-state="Y",state="Y",path="/System/Library/Frameworks/Carbon.framework/Versions/A/Carbon",description="/System/Library/Frameworks/Carbon.framework/Versions/A/Carbon",loaded_addr="0x940fd000",slide="-0x6bf03000",prefix=""]
//   =shlibs-added,shlib-info=[num="5",name="libfreeimage-3.9.3.dylib",kind="-",dyld-addr="0x61e000",reason="dyld",requested-state="Y",state="Y",path="/Users/team/work/driveo/debug/bin/libfreeimage-3.9.3.dylib",description="/Users/team/work/driveo/debug/bin/libfreeimage-3.9.3.dylib",loaded_addr="0x61e000",slide="0x61e000",prefix=""]
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleSharedModuleLoadedMessage(const gtASCIIString& gdbOutputLine)
{
    bool retVal = false;

    static gtASCIIString s_pathPrefix = "path=\"";
    static int s_pathPrefixLength = s_pathPrefix.length();
    static gtASCIIString s_loadedAddressPrefix = "loaded_addr=\"";
    static int s_loadedAddressPrefixLength = s_loadedAddressPrefix.length();
    static gtASCIIString s_dataSuffix = "\"";

    // Will get the loaded module path and load address:
    gtASCIIString loadedModulePath;
    osInstructionPointer moduleLoadAddress = 0;

    // Get the position in which the "path=" appears:
    int pos1 = gdbOutputLine.find(s_pathPrefix);
    GT_IF_WITH_ASSERT(pos1 != -1)
    {
        // Look for the " that ends the path:
        int pos2 = pos1 + s_pathPrefixLength;
        int pos3 = gdbOutputLine.find(s_dataSuffix, pos2);
        GT_IF_WITH_ASSERT(pos3 != -1)
        {
            // Get the loaded module path:
            gdbOutputLine.getSubString(pos2, pos3 - 1, loadedModulePath);
        }
    }

    // Get the position in which the "loaded_addr=" appears:
    int pos4 = gdbOutputLine.find(s_loadedAddressPrefix);
    GT_IF_WITH_ASSERT(pos4 != -1)
    {
        // Look for the " that ends the loaded address:
        int pos5 = pos4 + s_loadedAddressPrefixLength;
        int pos6 = gdbOutputLine.find(s_dataSuffix, pos5);

        if ((pos6 != -1) && (pos5 < pos6))
        {
            // Get the module loaded address as string:
            gtASCIIString moduleLoadedAddressAsStr;
            gdbOutputLine.getSubString(pos5, pos6 - 1, moduleLoadedAddressAsStr);

            // Translate it to a void*:
            unsigned long long moduleLoadedAddressAsULongLong = 0;
            bool rc1 = moduleLoadedAddressAsStr.toUnsignedLongLongNumber(moduleLoadedAddressAsULongLong);
            GT_IF_WITH_ASSERT(rc1)
            {
                moduleLoadAddress = (osInstructionPointer)moduleLoadedAddressAsULongLong;
            }
        }
    }

    // Trigger a "module loaded" event:
    gtString modulePathAsUnicodeString;
    modulePathAsUnicodeString.fromASCIIString(loadedModulePath.asCharArray());
    apModuleLoadedEvent* pModuleLoadedEvent = new apModuleLoadedEvent(0, modulePathAsUnicodeString, moduleLoadAddress);
    m_eventsToRegister.push_back(pModuleLoadedEvent);

    // Check success status:
    if (!loadedModulePath.isEmpty())
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::handleSharedModuleUnloadedMessage
// Description: Is called when a module is unloaded from the debugged process address space.
// Arguments: gdbOutputLine - GDB's output line.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/1/2009
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::handleSharedModuleUnloadedMessage(const gtASCIIString& gdbOutputLine)
{
    bool retVal = false;

    static gtASCIIString s_pathPrefix = "path=\"";
    static int s_pathPrefixLength = s_pathPrefix.length();
    static gtASCIIString s_dataSuffix = "\"";

    // Will get the unloaded module path and load address:
    gtASCIIString unloadedModulePath;

    // Get the position in which the "path=" appears:
    int pos1 = gdbOutputLine.find(s_pathPrefix);
    GT_IF_WITH_ASSERT(pos1 != -1)
    {
        // Look for the " that ends the path:
        int pos2 = pos1 + s_pathPrefixLength;
        int pos3 = gdbOutputLine.find(s_dataSuffix, pos2);
        GT_IF_WITH_ASSERT(pos3 != -1)
        {
            // Get the unloaded module path:
            gdbOutputLine.getSubString(pos2, pos3, unloadedModulePath);
        }
    }

    // Trigger a "module unloaded" event:
    gtString modulePathAsUnicodeString;
    modulePathAsUnicodeString.fromASCIIString(unloadedModulePath.asCharArray());
    apModuleUnloadedEvent* pModuleUnloadedEvent = new apModuleUnloadedEvent(0, modulePathAsUnicodeString);
    m_eventsToRegister.push_back(pModuleUnloadedEvent);

    // Check success status:
    if (!unloadedModulePath.isEmpty())
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::outputParsingGDBOutputLogMessage
// Description: If under debug log severity, output a "parsing GDB output..."
//              debug log message.
// Arguments: executedGDBCommandId - The executed GDB command id.
//            gdbOutputString - GDB's output string.
// Author:      Yaki Tebeka
// Date:        17/4/2007
// ---------------------------------------------------------------------------
void pdGDBOutputReader::outputParsingGDBOutputLogMessage(pdGDBCommandId executedGDBCommandId, const gtASCIIString& gdbOutputString)
{
    // If under debug log severity:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        // Will contain the debug message:
        gtString debugMsg;

        // Get the executed command information:
        const pdGDBCommandInfo* pCommandInfo = pdGetGDBCommandInfo(executedGDBCommandId);
        GT_IF_WITH_ASSERT(pCommandInfo)
        {
            // Add GDB's command string to the debug message:
            debugMsg.fromASCIIString(pCommandInfo->_commandExecutionString);
        }
        debugMsg.prepend(PD_STR_executedGDBCommand);

        // Add the parsed test to the debug message:
        debugMsg += PD_STR_parsingGDBOutput;
        gtString gdbOutputAsUnicodeString;
        gdbOutputAsUnicodeString.fromASCIIString(gdbOutputString.asCharArray());
        debugMsg += gdbOutputAsUnicodeString;

        // Output the debug message:
        OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::outputEndedParsingGDBOutputLogMessage
// Description: If under debug log severity, output a "Finished parsing GDB output..."
//              debug log message.
// Arguments: gdbOutputString - GDB's output string.
// Author:      Yaki Tebeka
// Date:        18/12/2008
// ---------------------------------------------------------------------------
void pdGDBOutputReader::outputEndedParsingGDBOutputLogMessage(const gtASCIIString& gdbOutputString)
{
    // If under debug log severity:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        // Add the parsed test to the debug message:
        gtString debugMsg;
        debugMsg.fromASCIIString(gdbOutputString.asCharArray());
        debugMsg.prepend(PD_STR_endedParsingGDBOutput);

        // Output the debug message:
        OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::outputGeneralLineLogMessage
// Description: If under debug log severity, output a "parsing general GDB output line ..."
//              debug log message.
// Arguments: gdbOutputLine - The parsed output line.
// Author:      Yaki Tebeka
// Date:        17/4/2007
// ---------------------------------------------------------------------------
void pdGDBOutputReader::outputGeneralLineLogMessage(const gtASCIIString& gdbOutputLine)
{
    // If under debug log severity:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        gtString debugMsg;
        debugMsg.fromASCIIString(gdbOutputLine.asCharArray());
        debugMsg.prepend(PD_STR_parsingGDBOutputLine);
        OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::outputThreadLineLogMessage
// Description: If under debug log severity, output a "Parsing thread line ..."
//              debug log message.
// Arguments: gdbOutputString - The read GDB output output line.
// Author:      Yaki Tebeka
// Date:        17/4/2007
// ---------------------------------------------------------------------------
void pdGDBOutputReader::outputThreadLineLogMessage(const gtASCIIString& gdbOutputLine)
{
    // If under debug log severity:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        gtString debugMsg;
        debugMsg.fromASCIIString(gdbOutputLine.asCharArray());
        debugMsg.prepend(PD_STR_parsingThreadLine);
        OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::outputCallStackLogMessage
// Description: If under debug log severity, output a "Parsing call stack ..."
//              debug log message.
// Arguments: gdbOutputString - The read GDB output output line.
// Author:      Yaki Tebeka
// Date:        17/4/2007
// ---------------------------------------------------------------------------
void pdGDBOutputReader::outputCallStackLogMessage(const gtASCIIString& gdbOutputString)
{
    // If under debug log severity:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        gtString debugMsg;
        debugMsg.fromASCIIString(gdbOutputString.asCharArray());
        debugMsg.prepend(PD_STR_parsingCallStack);
        OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::outputCallStackLineLogMessage
// Description: If under debug log severity, output a "Parsing call stack line ..."
//              debug log message.
// Arguments: gdbOutputString - The read GDB output output line.
// Author:      Yaki Tebeka
// Date:        17/4/2007
// ---------------------------------------------------------------------------
void pdGDBOutputReader::outputCallStackLineLogMessage(const gtASCIIString& gdbOutputString)
{
    // If under debug log severity:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        gtString debugMsg;
        debugMsg.fromASCIIString(gdbOutputString.asCharArray());
        debugMsg.prepend(PD_STR_parsingCallStackLine);
        OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::isExecutingSynchronousCommand
// Description:
//  Returns true iff we are reading the output of a GDB synchronous command.
// Author:      Yaki Tebeka
// Date:        29/8/2007
// ---------------------------------------------------------------------------
bool pdGDBOutputReader::isExecutingSynchronousCommand() const
{
    bool retVal = false;

    // Get the executed command information:
    const pdGDBCommandInfo* pCommandInfo = pdGetGDBCommandInfo(_executedGDBCommandId);
    GT_IF_WITH_ASSERT(pCommandInfo)
    {
        if (pCommandInfo->_commandType == PD_GDB_SYNCHRONOUS_CMD)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::initMembers
// Description: Initialize this class internal members.
// Author:      Yaki Tebeka
// Date:        8/7/2007
// ---------------------------------------------------------------------------
void pdGDBOutputReader::initMembers()
{
    _executedGDBCommandId = PD_GDB_NULL_CMD;
    _executedGDBCommandRequiresFlush = false;
    _pGDBCommunicationPipe = NULL;
    _gdbErrorString.makeEmpty();
    _wasDebuggedProcessSuspended = false;
    m_didDebuggedProcessReceiveFatalSignal = false;
    _wasDebuggedProcessTerminated = false;
    //_wasGDBPrompt = false;
    _debuggedProcessCurrentThreadGDBId = -1;
    _debuggedProcessCurrentThreadId = OS_NO_THREAD_ID;
    m_eventsToRegister.deleteElementsAndClear();
}

// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::threadIdFromGDBId
// Description: Translates a thread's GDB index to an OS thread ID
// Author:      Uri Shomroni
// Date:        29/3/2016
// ---------------------------------------------------------------------------
osThreadId pdGDBOutputReader::threadIdFromGDBId(int threadGDBId)
{
    osThreadId retVal = OS_NO_THREAD_ID;

    if (0 < threadGDBId)
    {
        // Get the debugged process threads data:
        static const gtASCIIString emptyStr;
        const pdGDBData* pGDBOutputData = NULL;

        // On Mac, starting with Xcode 3.2.3, the "info threads" command does not give us all the data we need, so we use the machine interface "-thread-list-ids" instead.
        // On Linux, the machine interface function is not implementer on all platforms, so we cannot use it as we might not get the data.
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
        bool rc1 = _pGDBDriver->executeGDBCommand(PD_GET_THREADS_INFO_CMD, emptyStr, &pGDBOutputData);
#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
        bool rc1 = _pGDBDriver->executeGDBCommand(PD_GET_THREADS_INFO_VIA_MI_CMD, emptyStr, &pGDBOutputData);
#else
#error Unknown Linux Variant!
#endif
        GT_IF_WITH_ASSERT(rc1 && (pGDBOutputData != NULL))
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(pGDBOutputData->type() == pdGDBData::PD_GDB_THREAD_DATA_LIST)
            {
                // Store the threads data;
                pdGDBThreadDataList* pDebuggedProcessThreadsData = (pdGDBThreadDataList*)pGDBOutputData;

                const gtList<pdGDBThreadData>& debuggedProcessThreadsDataList = pDebuggedProcessThreadsData->_threadsDataList;

                for (const auto& t : debuggedProcessThreadsDataList)
                {
                    if (t._gdbThreadId == threadGDBId)
                    {
                        retVal = t._OSThreadId;
                        break;
                    }
                }
            }
        }

        // Clean up the threads data:
        delete pGDBOutputData;

        return retVal;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBOutputReader::initialize
// Description: Initializes the GDB output reader
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        13/12/2010
// ---------------------------------------------------------------------------
void pdGDBOutputReader::initialize()
{
    _amountOfGDBStringPrintouts = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Process GDB's output on suspend process command
///
/// \param[in] gdbOutputString a GDB answer
///
/// \return true - command success/false - GDB return error
bool pdGDBOutputReader::handleSuspendProcess(const gtASCIIString& gdbOutputString)
{
    std::vector<std::string> tokens;
    std::stringstream ss(gdbOutputString.asCharArray());
    std::string item;

    while (std::getline(ss, item, '\n'))
    {
        tokens.push_back(item);
    }

    for (std::string& it : tokens)
    {
        if (it.find("signal-name=\"0\"") != std::string::npos)
        {
            std::string::size_type pos = it.find("stopped-threads");

            if (std::string::npos != pos)
            {
                std::string::size_type pos2 = it.find("\"]", pos + strlen("stopped-threads=[\"") + 1);

                if (std::string::npos != pos2)
                {
                    std::string::iterator iterStart = it.begin();
                    iterStart += pos;
                    iterStart += strlen("stopped-threads=[\"");

                    std::string::iterator iterEnd = it.begin();
                    iterEnd += pos2;

                    std::string threadNum(iterStart, iterEnd);

                    int threadGDBId = std::atoi(threadNum.c_str());

                    if (_pGDBDriver)
                    {
                        _pGDBDriver->OnThreadGDBStopped(threadGDBId);
                        _wasDebuggedProcessSuspended = _pGDBDriver->IsAllThreadsStopped();
                    }
                }
            }
        }
    }

    return true;
};

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Process GDB's output on resume process command
///
/// \param[in] gdbOutputString a GDB answer
///
/// \return true - command success/false - GDB return error
bool pdGDBOutputReader::handleResumeProcess(const gtASCIIString& gdbOutputString)
{
    GT_UNREFERENCED_PARAMETER(gdbOutputString);
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Process GDB's output on switch to thread command
///
/// \param[in] gdbOutputString a GDB answer
///
/// \return true - command success/false - GDB return error
bool pdGDBOutputReader::handleSwitchToThread(const gtASCIIString& gdbOutputString)
{
    GT_UNREFERENCED_PARAMETER(gdbOutputString);
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Process GDB's output on switch to frame command
///
/// \param[in] gdbOutputString a GDB answer
///
/// \return true - command success/false - GDB return error
bool pdGDBOutputReader::handleSwitchToFrame(const gtASCIIString& gdbOutputString)
{
    return gdbOutputString.find("^error") == -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Process GDB's output on get frame locals variables
///
/// \param[in]  gdbOutputString a GDB answer
/// \param[out] ppGDBOutputData a pointer to
///
/// \return true - command success/false - GDB return error
bool pdGDBOutputReader::handleGetLocalsFrame(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData)
{
    bool result = false;
    gtList<std::pair<gtString, gtString> > variablesList;

    GT_IF_WITH_ASSERT(nullptr != ppGDBOutputData)
    {
        std::string gdbString = std::string(gdbOutputString.asCharArray());
        std::string::size_type pos_start = gdbString.find("[");
        std::string::size_type pos_end = gdbString.rfind("]");

        if (pos_start != std::string::npos && pos_end != std::string::npos)
        {
            size_t open_bracket_count = 0;
            size_t close_bracket_count = 0;
            std::string::size_type open_bracket_count_pos = std::string::npos;
            std::string::size_type close_bracket_count_pos = std::string::npos;

            for (std::string::size_type it = pos_start; it < pos_end; it++)
            {
                if ('{' == gdbString[it])
                {
                    open_bracket_count++;

                    if (open_bracket_count == 1)
                    {
                        open_bracket_count_pos = it;
                    }
                }
                else
                {
                    if ('}' == gdbString[it])
                    {
                        close_bracket_count++;
                        close_bracket_count_pos = it;
                    }
                }

                if (open_bracket_count == close_bracket_count && open_bracket_count != 0)
                {
                    std::string token(std::begin(gdbString) + open_bracket_count_pos, std::begin(gdbString) + close_bracket_count_pos);

                    std::string::size_type name_begin = token.find("name=\"");
                    std::string::size_type value_begin = token.find("value=\"");

                    if (std::string::npos != name_begin && std::string::npos != value_begin)
                    {
                        name_begin += strlen("name=\"");
                        value_begin += strlen("value=\"");
                        std::string::size_type name_end = token.find("\"", name_begin);
                        std::string::size_type value_end = token.rfind("\"");

                        if (std::string::npos != name_end && std::string::npos != value_end)
                        {
                            std::string name(std::begin(token) + name_begin, std::begin(token) + name_end);
                            std::string value(std::begin(token) + value_begin, std::begin(token) + value_end);

                            gtString gtVariableName = L"";
                            gtString gtVariableValue = L"";
                            gtVariableValue.fromASCIIString(value.c_str());
                            gtVariableName.fromASCIIString(name.c_str());

                            variablesList.insert(variablesList.begin(), std::make_pair(gtVariableName, gtVariableValue));
                        }
                    }

                    open_bracket_count = 0;
                    close_bracket_count = 0;
                    open_bracket_count_pos = std::string::npos;
                    close_bracket_count_pos = std::string::npos;
                }
            }
        }

        *ppGDBOutputData = new pdGDBFrameLocalsData(variablesList);

        GT_IF_WITH_ASSERT(nullptr != *ppGDBOutputData)
        {
            result = true;
        }
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Process GDB's output on get variable value
///
/// \param[in]  gdbOutputString a GDB answer
/// \param[out] ppGDBOutputData a pointer to
///
/// \return true - command success/false - GDB return error
bool pdGDBOutputReader::handleGetVariable(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData)
{
    bool result = false;

    GT_UNREFERENCED_PARAMETER(gdbOutputString);
    GT_UNREFERENCED_PARAMETER(ppGDBOutputData);

    gtString gtValue = L"";

    GT_IF_WITH_ASSERT(nullptr != ppGDBOutputData)
    {
        std::string gdbString = std::string(gdbOutputString.asCharArray());
        std::string::size_type pos_start = gdbString.find("^done,value=\"");

        if (0 == pos_start)
        {
            std::string::size_type pos_end = gdbString.rfind("\"");

            if (std::string::npos != pos_end)
            {
                std::string value(std::begin(gdbString) + strlen("^done,value=\""), std::begin(gdbString) + pos_end);
                gtValue.fromASCIIString(value.c_str());
            }
        }

        *ppGDBOutputData = new pdGDBFrameLocalVariableValue(L"", gtValue);

        GT_IF_WITH_ASSERT(nullptr != *ppGDBOutputData)
        {
            result = true;
        }
    }

    return result;
};

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Process GDB's output on set breakpoint
///
/// \param[in]  gdbOutputString a GDB answer
/// \param[out] ppGDBOutputData a pointer to
///
/// \return true - command success/false - GDB return error
bool pdGDBOutputReader::handleSetBreakpointOutput(const gtASCIIString& gdbOutputString, const pdGDBData** ppGDBOutputData)
{
    bool result = gdbOutputString.find("^error") < 0;

    if (result)
    {
        const gtASCIIString bpNumberStart = "bkpt={number=\"";

        int pos1 = gdbOutputString.find(bpNumberStart);
        GT_IF_WITH_ASSERT(pos1 != -1)
        {
            int pos2 = pos1 + bpNumberStart.length();
            int pos3 = gdbOutputString.find('\"', pos2);
            GT_IF_WITH_ASSERT(pos3 != -1)
            {
                gtASCIIString numberString = "";
                gdbOutputString.getSubString(pos2, (pos3 - 1), numberString);

                int index = -1;

                GT_IF_WITH_ASSERT(numberString.toIntNumber(index))
                {
                    *ppGDBOutputData = new pdGDBBreakpointIndex(index);
                    result = true;
                }
            }
        }

    }

    return result;
};

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Process GDB's output on "step in" command
///
/// \param[in]  gdbOutputString a GDB answer
/// \param[out] ppGDBOutputData a pointer to
///
/// \return true - command success/false - GDB return error
bool pdGDBOutputReader::handleStepIn(const gtASCIIString& gdbOutputString)
{
    return gdbOutputString.find("^error") < 0;
};

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Process GDB's output on "step over" command
///
/// \param[in]  gdbOutputString a GDB answer
/// \param[out] ppGDBOutputData a pointer to
///
/// \return true - command success/false - GDB return error
bool pdGDBOutputReader::handleStepOver(const gtASCIIString& gdbOutputString)
{
    return gdbOutputString.find("^error") < 0;
};

/////////////////////////////////////////////////////////////////////////////////////////
/// \brief Process GDB's output on "step out" command
///
/// \param[in]  gdbOutputString a GDB answer
///
/// \return true - command success/false - GDB return error
bool pdGDBOutputReader::handleStepOut(const gtASCIIString& gdbOutputString)
{
    return gdbOutputString.find("^error") < 0;
};

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Flush GDB prompt
///
/// \return true - success, false - fail
/// \author Vadim Entov
/// \date 10/02/2016
bool pdGDBOutputReader::flushGDBPrompt()
{
    bool result = false;

    gtASCIIString gdbOutput;

    if (!_wasGDBPrompt)
    {
        readSynchronousCommandGDBOutput(gdbOutput);

        result = (gdbOutput.find("(gdb)") != -1);
    }
    else
    {
        result = true;
    }

    return result;
}


