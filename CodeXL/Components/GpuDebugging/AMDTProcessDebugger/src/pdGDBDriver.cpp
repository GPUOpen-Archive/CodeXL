//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdGDBDriver.cpp
///
//==================================================================================

//------------------------------ pdGDBDriver.cpp ------------------------------

/*
 *   GDB Usage:
 *   =========
 *
 *  a. We choose to use the GDB MI interface (Machine Interface), which is
 *     aimed for machine front ends of GDB. It given a consistent input and
 *     output syntax that can be safely parsed by machines.
 *     For more details about the GDB MI interface, see the "Debugging with GDB"
 *     book, chapter 24: The GDB/MI Interface.
 *     (http://sources.redhat.com/gdb/current/onlinedocs/gdb_25.html)
 *     Notice that GDB MI not fully implemented. Whenever a GDB MI function is
 *     missing, we can use the "regular" GDB commands, which are also active in
 *     MI mode.
 *
 *  b. Synchronous and Asynchronous GDB commands
 *     GDB offers synchronous and asynchronous GDB commands.
 *     - Synchronous commands return immediately (example: -stack-list-frames).
 *     - Asynchronous commands does not return immediately (example: -exec-run).
 *     After executing a synchronous command, we ask pdGDBOutputReader to read the
 *     command result immediately. After executing an asynchronos command, we use
 *     a thread (pdGDBListenerThread) that waits for GDB outputs. When this thread
 *     parses a GDB output that tells us that the debugged process was suspended,
 *     the thread is suspended from listening to GDB outputs.
 *
 */

// Standard C:
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// POSIX:
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osPipeSocketClient.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apInfrastructureFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <src/pdStringConstants.h>
#include <src/pdGDBDriver.h>
#include <src/pdGDBOutputReader.h>
#include <src/pdGDBProcessWaiterThread.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>

// stl
#include <thread>


// Contains the SHELL environment variable name:
static const gtString s_shellEnvVariableName = L"SHELL";

// Contains the bash shell path:
static const gtString s_bashShellPathEnvVariableName = L"/bin/bash";



// ---------------------------------------------------------------------------
// Name:        pdBorkenPipeSignalHandler
// Description: Is called when a broken pipe signal is received by this process.
// Arguments: signalNumber - The received signal id.
// Author:      Yaki Tebeka
// Date:        15/10/2007
// Implementation notes:
//   We ignore the signal, since most broken pipe signals are received when
//   the debugged process is dying. The process debugger will detect that and
//   terminate the pdGDBDriver class instance.
// ---------------------------------------------------------------------------
void pdBorkenPipeSignalHandler(int signalNumber)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(signalNumber == SIGPIPE)
    {
        // Write the information into the log file:
        OS_OUTPUT_DEBUG_LOG(PD_STR_brokenPipeSignalReceived, OS_DEBUG_LOG_ERROR);

        //////////////////////////////////////////////////////////////////////////
        // Uri, 5/3/09: We do not throw the exception event, as it causes the GDB
        // driver to think it came from the debugged app (which in turn causes it
        // to try and get a crash calls stack and so forth eventually making
        // CodeXL hang. This code is left commented out in case we decide we do
        // need to report it to CodeXL somehow.
        //////////////////////////////////////////////////////////////////////////
        // Throw an exception event, notifying that a pipe was broken:
        // apExceptionEvent brokenPipeEvent(0, OS_SIGPIPE_SIGNAL, NULL, false);
        // apEventsHandler::instance().registerPendingDebugEvent(brokenPipeEvent);
    }
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::pdGDBDriver
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        17/12/2006
// ---------------------------------------------------------------------------
pdGDBDriver::pdGDBDriver()
    : _wasInitialized(false), _gdbProcessId(0), m_pGDBProcessWaiterThread(NULL), _pGDBCommunicationPipe(NULL), _pGDBProcessConsolePipe(nullptr), _pGDBProcessConsolePipeClient(nullptr),
      _pGDBListenerThread(NULL), _pDebuggedAppOutputReaderThread(NULL)
{
    // Initialize the GDB communication pipe names:
    clearGDBCommunicationPipeNames();

    // Register pdBorkenPipeSignalHandler as broken pipe signal handler:
    registerBrokenPipeSignalHandler();
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::~pdGDBDriver
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        17/12/2006
// ---------------------------------------------------------------------------
pdGDBDriver::~pdGDBDriver()
{
    OS_OUTPUT_DEBUG_LOG(L"Before deleting pdGDBProcessWaiterThread", OS_DEBUG_LOG_DEBUG);
    delete m_pGDBProcessWaiterThread;
    m_pGDBProcessWaiterThread = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting pdGDBProcessWaiterThread", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting osPipeSocket (_pGDBCommunicationPipe)", OS_DEBUG_LOG_DEBUG);
    delete _pGDBCommunicationPipe;
    _pGDBCommunicationPipe = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting osPipeSocket (_pGDBCommunicationPipe)", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting pdGDBListenerThread", OS_DEBUG_LOG_DEBUG);
    delete _pGDBListenerThread;
    _pGDBListenerThread = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting pdGDBListenerThread", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before deleting pdLinuxDebuggedApplicationOutputReaderThread", OS_DEBUG_LOG_DEBUG);
    delete _pDebuggedAppOutputReaderThread;
    _pDebuggedAppOutputReaderThread = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting pdLinuxDebuggedApplicationOutputReaderThread", OS_DEBUG_LOG_DEBUG);

}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::initialize
// Description: Initializes the connection with GDB - the GNU debugger.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        17/12/2006
// ---------------------------------------------------------------------------
bool pdGDBDriver::initialize(const gtString& gdbExecutable)
{
    bool retVal = false;

    // Sanity check - this class should be terminated by now:
    if (_wasInitialized)
    {
        GT_ASSERT(false);
        terminate();
    }

    // Initialize the GDB output reader:
    _gdbOutputReader.initialize();

    // If the gdb listener thread was not created yet - create it:
    if (_pGDBListenerThread == NULL)
    {
        _pGDBListenerThread = new pdGDBListenerThread;

    }

    // Delete old listening output thread and be ready for the new one:
    if (NULL != _pDebuggedAppOutputReaderThread)
    {
        delete _pDebuggedAppOutputReaderThread;
        _pDebuggedAppOutputReaderThread = NULL;
    }

    // If the debugged application output reader thread was not created yet:
    if (_pDebuggedAppOutputReaderThread == NULL)
    {
        // Create it:
        _pDebuggedAppOutputReaderThread = new pdLinuxDebuggedApplicationOutputReaderThread;

    }

    // Set GDB to use the bash shell:
    bool rc1 = setGDBUsedShell();
    GT_IF_WITH_ASSERT(rc1)
    {
        // Create pipes that will be used to communicate with GDB:
        bool rc2 = createGDBCommunicationPipes();
        GT_IF_WITH_ASSERT(rc2)
        {
            // Fork this process into 2 processes (child and parent):
            pid_t childProcessId = ::fork();

            // If this is the child process:
            if (childProcessId == 0)
            {
                // Redirect the child process stdin and stdout into the communication pipes:
                bool rc2 = redirectGDBStdinAndStdoutToPipes();

                if (rc2)
                {
                    // Replace the child process by a process running GDB:
                    char* const* pGDBCommadLineCommand = (char* const*)getGDBExecutionCommandLineArguments(gdbExecutable);
                    ::execvp(gdbExecutable.asASCIICharArray(), pGDBCommadLineCommand);

                    // If we reached here, it means that we didn't manage to execute
                    // the gdb process:
                    GT_ASSERT_EX(false, PD_STR_failedToLaunchGDB);

                    // Output an error string (that will reach the parent process
                    // through the pipe):
                    ssize_t result = ::write(1, PD_STR_failedToLaunchGDBASCII, strlen(PD_STR_failedToLaunchGDBASCII));
                    (void)(result); // unused
                    ::fflush(stdout);

                    // Output an error to the standard error (usually the console):
                    perror(PD_STR_failedToLaunchGDBASCII);
                }
                else
                {
                    ssize_t result = ::write(_gdbProcessStdout[1], PD_STR_failedToLaunchGDBASCII, strlen(PD_STR_failedToLaunchGDBASCII));
                    (void)(result); // unused
                    ::fflush(NULL);
                }

                // Exit the child process:
                ::kill(::getpid(), SIGKILL);
                ::exit(-1);
            }
            else if (childProcessId > 0)
            {
                // We are in the parent process:

                // Store GDB's process id:
                _gdbProcessId = childProcessId;

                // Launch a thread to wait on this pid:
                delete m_pGDBProcessWaiterThread;
                m_pGDBProcessWaiterThread = new pdGDBProcessWaiterThread(_gdbProcessId);

                m_pGDBProcessWaiterThread->execute();

                // Restore the content of the $SHELL environment variable:
                restoreShellEnvVariableValue();

                // Wait a short time, then check that the process exists (i.e. that the ::exit(-1) above was not reached):
                osSleep(200);

                if (0 == ::kill(childProcessId, 0))
                {
                    // Wrap GDB's communication pipe and store the wrapper into _pGDBCommunicationPipe:
                    bool rc1 = wrapGDBCommunicationPipes();
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Initialize GDB:
                        bool rc2 = initializeGDB();
                        GT_IF_WITH_ASSERT(rc2)
                        {
                            retVal = true;
                        }
                    }
                }
            }
            else // childProcessId < 0
            {
                GT_ASSERT(childProcessId >= 0);
            }
        }
    }

    if (!retVal)
    {
        // Trigger an infrastructure failure event:
        apInfrastructureFailureEvent infraFailureEvent(apInfrastructureFailureEvent::FAILED_TO_INITIALIZE_GDB);
        apEventsHandler::instance().registerPendingDebugEvent(infraFailureEvent);
    }

    _wasInitialized = retVal;
    m_createdProcessThread = false;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::terminate
// Description: Terminates the connection with GDB - the GNU debugger.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        17/12/2006
// ---------------------------------------------------------------------------
bool pdGDBDriver::terminate()
{
    bool retVal = true;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // If this is an ES (iPhone) project
    const apDebugProjectSettings* pProcessCreationData = pdProcessDebugger::instance().debuggedProcessCreationData();

    if (pProcessCreationData != NULL)
    {
        if (apDoesProjectTypeSupportOpenGLES(pProcessCreationData->projectType()))
        {
            // Suspend GDB if it is waiting for something:
            if (_gdbProcessId != 0)
            {
                ::kill(_gdbProcessId, SIGINT);
                osSleep(100);
            }
        }
    }

#endif

    // Close GDBs communication pipes:
    bool rc1 = closeGDBCommunicationPipes();
    GT_ASSERT(rc1);

    bool rc2 = true;

    if (_gdbProcessId != 0)
    {
        // Force gdb to exit:
        int rcKill = ::kill(_gdbProcessId, SIGKILL);
        GT_IF_WITH_ASSERT(rcKill == 0)
        {
            // We need to wait for gdb process. Otherwise, it will remain
            // zombie, trying to tell us that it died.
            int gdbProcessStatus = 0;
            ::waitpid(_gdbProcessId, &gdbProcessStatus, 0);
        }
        else
        {
            rc2 = false;
        }
    }

    // Mark that the GDB process does not exist:
    _gdbProcessId = 0;
    delete m_pGDBProcessWaiterThread;
    m_pGDBProcessWaiterThread = NULL;

    // Mark that we are not initialized anymore:
    _wasInitialized = false;

    retVal = rc1 && rc2;

    m_processExistingThreads.clear();
    m_processStoppedThreads.clear();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::onDebuggedProcessRunResumed
// Description: Is called when the debugged process run is resumed.
// Author:      Yaki Tebeka
// Date:        19/12/2006
// ---------------------------------------------------------------------------
bool pdGDBDriver::onDebuggedProcessRunResumed()
{
    bool retVal = false;

    // Start listening to debugged process events:
    bool rc1 = _pGDBListenerThread->startListening(*_pGDBCommunicationPipe, *this, _gdbOutputReader, PD_CONTINUE_CMD);
    GT_IF_WITH_ASSERT(rc1)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::waitForInternalDebuggedProcessInterrupt
// Description: Waits for an internally generated interrupt (SIGINT), sent to
//              the debugged process. This interrupt will suspend the debugged
//              process, but we don't want to report it to the outside world.
// Author:      Yaki Tebeka
// Date:        24/12/2007
// ---------------------------------------------------------------------------
void pdGDBDriver::waitForInternalDebuggedProcessInterrupt()
{
    _gdbOutputReader.waitForInternalDebuggedProcessInterrupt();
}

// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::flushCommandOutput
// Description: Sends a "return" character to GDB, causing it to flush out any
//              output from the last command (this is only needed in the iPhone
//              when gdb does not normally report when the debugged process
//              exits.
// Author:      Uri Shomroni
// Date:        2/6/2009
// ---------------------------------------------------------------------------
void pdGDBDriver::flushCommandOutput()
{
    // Sanity check:
    if (_pGDBCommunicationPipe != NULL)
    {
        gtString flushString = '\n';
        _pGDBCommunicationPipe->write((const gtByte*)(flushString.asCharArray()), flushString.length());
    }
}

// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::getOutputReaderThreadFileName
// Description: Returns the output reader thread's file name
// Author:      Uri Shomroni
// Date:        7/6/2009
// ---------------------------------------------------------------------------
void pdGDBDriver::getOutputReaderThreadFileName(gtString& fileName) const
{
    GT_IF_WITH_ASSERT(_pDebuggedAppOutputReaderThread != NULL)
    {
        fileName = _pDebuggedAppOutputReaderThread->getPipeFilePath();
    }
}

void pdGDBDriver::getProcessConsolePipeName(gtString& fileName)
{
    if (nullptr != _pGDBProcessConsolePipe)
    {
        _pGDBProcessConsolePipe->getServerFilePath(fileName);
    }
}

// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::executeGDBCommand
// Description: Executes a GDB command.
// Arguments: gdbCommandId - The id of the command to be executed.
//            commandArgs - The command arguments (in GDB MI2 format).
//            ppGDBOutputData - Will get GDB output data (if relevant to
//                              the executed GDB command).
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/12/2006
// ---------------------------------------------------------------------------
bool pdGDBDriver::executeGDBCommand(pdGDBCommandId gdbCommandId,
                                    const gtASCIIString& commandArgs,
                                    const pdGDBData** ppGDBOutputData)
{
    bool retVal = false;

    if (ppGDBOutputData != NULL)
    {
        *ppGDBOutputData = NULL;
    }

    // Get the command information:
    const pdGDBCommandInfo* pCommandInfo = pdGetGDBCommandInfo(gdbCommandId);

    if (pCommandInfo)
    {
        // Build the GDB command string:
        gtASCIIString commandString;
        bool rc1 = buildGDBCommandString(gdbCommandId, commandArgs, commandString);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Send the command string to gdb:
            _gdbOutputReader.resetGDBPrompt();

            bool rc2 = writeToGDBInput(commandString);
            GT_IF_WITH_ASSERT(rc2)
            {
                // If this is a synchronous command:
                if (pCommandInfo->_commandType == PD_GDB_SYNCHRONOUS_CMD)
                {
                    if (pdDoesCommandRequireFlush(pCommandInfo->_commandId))
                    {
                        flushCommandOutput();
                    }

                    // Read gdb's outputs immediately:
                    bool ignoredS = false;
                    bool ignoredT = false;
                    retVal = _gdbOutputReader.readGDBOutput(*_pGDBCommunicationPipe, *this, gdbCommandId, ignoredS, ignoredT, ppGDBOutputData);
                }
                else if (pCommandInfo->_commandType == PD_GDB_ASYNCHRONOUS_CMD)
                {
                    // This is an asynchronous command:

                    // Read the "^running" message:
                    bool ignoredS = false;
                    bool ignoredT = false;
                    bool rc1 = _gdbOutputReader.readGDBOutput(*_pGDBCommunicationPipe, *this, gdbCommandId, ignoredS, ignoredT, ppGDBOutputData);

                    if (rc1)
                    {
                        // If the process still exists (e.g. we did not hit "continue" or "kill" on an exception):
                        if ((PD_CONTINUE_CMD != gdbCommandId) || pdProcessDebugger::instance().debuggedProcessExists())
                        {
                            // Ask the GDB listener thread to listen to GDB outputs
                            // (asynchronous of this thread)

                            if (PD_CONTINUE_CMD == gdbCommandId)
                            {
                                while (!IsAllThreadsRunning())
                                {
                                    _gdbOutputReader.readGDBOutput(*_pGDBCommunicationPipe, *this, gdbCommandId, ignoredS, ignoredT, ppGDBOutputData);
                                }
                            }

                            retVal = _pGDBListenerThread->startListening(*_pGDBCommunicationPipe, *this, _gdbOutputReader, gdbCommandId);
                        }
                        else
                        {
                            retVal = true;
                        }
                    }
                }
                else if (pCommandInfo->_commandType == PD_GDB_ASYNCHRONOUS_NO_ANSWER_CMD)
                {
                    retVal = _pGDBListenerThread->startListening(*_pGDBCommunicationPipe, *this, _gdbOutputReader, gdbCommandId);
                }
                else
                {
                    // Unknown command type:
                    GT_ASSERT(false);
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::getGDBExecutionCommandLineArguments
// Description: Returns the command line command that starts GDB run.
// Author:      Yaki Tebeka
// Date:        18/12/2006
// ---------------------------------------------------------------------------
const char** pdGDBDriver::getGDBExecutionCommandLineArguments(const gtString& gdbExecutable) const
{
    // The command line arguments:
    static gtString arg0;                               // gdb application.
    static const char* arg1 = "--fullname";             // Output the full file name and line  number
    // each time a stack frame is displayed.
    static const char* arg2 = "--nx";                   // Do not execute gdb initialization files.
    static const char* arg3 = "--interpreter=mi2";      // We will use the GDB MI2 interface
    // (see "The used GDB interface" comment at the top of this file).

    arg0 = gdbExecutable;

    static const char** stat_pGDBExecutionCommandLineArgs = NULL;

    stat_pGDBExecutionCommandLineArgs  = (const char**)(::malloc(5 * sizeof(char*)));
    GT_IF_WITH_ASSERT(stat_pGDBExecutionCommandLineArgs != NULL)
    {
        stat_pGDBExecutionCommandLineArgs[0] = arg0.asASCIICharArray();
        stat_pGDBExecutionCommandLineArgs[1] = arg1;
        stat_pGDBExecutionCommandLineArgs[2] = arg2;
        stat_pGDBExecutionCommandLineArgs[3] = arg3;
        stat_pGDBExecutionCommandLineArgs[4] = 0;
    }

    return stat_pGDBExecutionCommandLineArgs;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::setGDBUsedShell
// Description:
//   Set GDBs used shell to be a bash shell.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        6/9/2007
// Implementation notes:
//  - When gdb launches an executable, it first creates a shell and then executes
//    the executable in it. The shell is created using the $SHELL environment
//    variable.
//  - Forcing gdb to always use bash shell eliminates all kinds of problems. Example,
//    when using tcsh, the user can setenv LD_LIBRARY_PATH in the .cshrc file. This will
//    override our LD_LIBRARY_PATH settings and will cause failure in loading the
//    spy required libraries. This problem does not exist when using bash.
//  - We are setting this process $SHELL environment variable. This process environment
//    block will be copied to child process environment block, including gdb, which
//    will be a child process of this process.
// ---------------------------------------------------------------------------
bool pdGDBDriver::setGDBUsedShell()
{
    bool retVal = false;

    // Log the current value of the $SHELL environment variable:
    bool rc1 = osGetCurrentProcessEnvVariableValue(s_shellEnvVariableName, _storedShellEnvVariableValue);

    if (!rc1)
    {
        _storedShellEnvVariableValue.makeEmpty();
    }

    // Will contains the "SHELL = /bin/bash":
    osEnvironmentVariable shellEnvVariable;
    shellEnvVariable._name = s_shellEnvVariableName;
    shellEnvVariable._value = s_bashShellPathEnvVariableName;

    // Set the shell environment environment of this process:
    // (This will be copies later into gdb's environment block)
    bool rc2 = osSetCurrentProcessEnvVariable(shellEnvVariable);
    GT_IF_WITH_ASSERT(rc2)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::restoreShellEnvVariableValue
// Description:
//   Restores the content of the $SHELL environment variable, that was changed
//   by setGDBUsedShell().
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/9/2007
// ---------------------------------------------------------------------------
bool pdGDBDriver::restoreShellEnvVariableValue()
{
    bool retVal = false;

    if (!_storedShellEnvVariableValue.isEmpty())
    {
        // Will contains the "SHELL = <_storedShellEnvVariableValue>":
        osEnvironmentVariable shellEnvVariable;
        shellEnvVariable._name = s_shellEnvVariableName;
        shellEnvVariable._value = _storedShellEnvVariableValue;

        // Set the shell environment environment of this process:
        // (This will be copies later into gdb's environment block)
        bool rc2 = osSetCurrentProcessEnvVariable(shellEnvVariable);
        GT_IF_WITH_ASSERT(rc2)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::createGDBCommunicationPipes
// Description:
//   Creates pipes that will be used to communicate with GDB.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        17/12/2006
// ---------------------------------------------------------------------------
bool pdGDBDriver::createGDBCommunicationPipes()
{
    bool retVal = false;

    bool stdinPipeCreated = false;
    bool stdoutPipeCreated = false;
    bool stdinProcessConsoleCreated = false;

    // Clear the GDB communication pipe names:
    clearGDBCommunicationPipeNames();

    // Create the gdb input pipe:
    int rc1 = ::pipe(_gdbProcessStdin);
    GT_IF_WITH_ASSERT(rc1 == 0)
    {
        // Make the pipe remain open across execve calls:
        int rc2 = ::fcntl(_gdbProcessStdin[0], F_SETFD, FD_CLOEXEC);
        int rc3 = ::fcntl(_gdbProcessStdin[1], F_SETFD, FD_CLOEXEC);

        GT_IF_WITH_ASSERT((rc2 == 0) && (rc3 == 0))
        {
            stdinPipeCreated = true;
        }
    }

    // Create the gdb output pipe:
    int rc4 = ::pipe(_gdbProcessStdout);
    GT_IF_WITH_ASSERT(rc4 == 0)
    {
        // Make the pipe remain open across execve calls:
        int rc5 = ::fcntl(_gdbProcessStdout[0], F_SETFD, FD_CLOEXEC);
        int rc6 = ::fcntl(_gdbProcessStdout[1], F_SETFD, FD_CLOEXEC);

        GT_IF_WITH_ASSERT((rc5 == 0) && (rc6 == 0))
        {
            stdoutPipeCreated = true;
        }
    }

    // Create the gdb output pipe:
    int rc7 = ::pipe(_gdbProcessConsoleStdIn);
    GT_IF_WITH_ASSERT(rc7 == 0)
    {
        // Make the pipe remain open across execve calls:
        int rc8 = ::fcntl(_gdbProcessConsoleStdIn[0], F_SETFD, FD_CLOEXEC);
        int rc9 = ::fcntl(_gdbProcessConsoleStdIn[1], F_SETFD, FD_CLOEXEC);

        GT_IF_WITH_ASSERT((rc8 == 0) && (rc9 == 0))
        {
            stdinProcessConsoleCreated = true;
        }
    }


    retVal = stdinPipeCreated && stdoutPipeCreated && stdinProcessConsoleCreated;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::wrapGDBCommunicationPipes
// Description:
//   Creates an osPipeSocketClient to wrap GDB's communication pipes
//   and stored it in _pGDBCommunicationPipe.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/2/2008
// ---------------------------------------------------------------------------
bool pdGDBDriver::wrapGDBCommunicationPipes()
{
    bool retVal = false;

    // Delete old the sockets wrapper (if exists):
    delete _pGDBCommunicationPipe;
    _pGDBCommunicationPipe = NULL;

    // Create a class that will wrap GDB'c communication pipes:
    _pGDBCommunicationPipe = new osPipeSocketClient(_gdbProcessStdout[0], _gdbProcessStdin[1], L"GDB Connection Socket");
    GT_IF_WITH_ASSERT(NULL != _pGDBCommunicationPipe)
    {
        // since we have a thread that waits for gdb a-synchronous commands outputs,
        // We cannot assume a fixed read timeout for gdb's outputs:
        _pGDBCommunicationPipe->setReadOperationTimeOut(OS_CHANNEL_INFINIT_TIME_OUT);

        retVal = true;
    }

    if (retVal)
    {
        delete _pGDBProcessConsolePipe;
        _pGDBProcessConsolePipe = nullptr;

        delete _pGDBProcessConsolePipeClient;
        _pGDBProcessConsolePipeClient = nullptr;

        // Create a class that will wrap GDB'c communication pipes:

        gtString pipeName = L"Console_input";

        // Get the current time and date as strings:
        osTime curretTime;
        curretTime.setFromCurrentTime();

        gtString dateAsString;
        curretTime.dateAsString(dateAsString, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);

        gtString timeAsString;
        curretTime.timeAsString(timeAsString, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);

        // Append the date and time to the file name:
        pipeName += L"-";
        pipeName += dateAsString;
        pipeName += L"-";
        pipeName += timeAsString;

        _pGDBProcessConsolePipe = new osPipeSocketServer(pipeName);
        GT_IF_WITH_ASSERT(NULL != _pGDBProcessConsolePipe)
        {
            _pGDBProcessConsolePipe->open();
            auto thrd = std::thread([this]() { static_cast<osPipeSocketServer*>(_pGDBProcessConsolePipe)->waitForClientConnection(); });

            _pGDBProcessConsolePipeClient = new osPipeSocketClient(pipeName, L"Console input pipe");
            _pGDBProcessConsolePipeClient->open();

            thrd.join();
            // since we have a thread that waits for gdb a-synchronous commands outputs,
            // We cannot assume a fixed read timeout for gdb's outputs:
            _pGDBProcessConsolePipe->setReadOperationTimeOut(OS_CHANNEL_INFINIT_TIME_OUT);

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::closeGDBCommunicationPipes
// Description: Closes GDB communication pipes.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/9/2007
// ---------------------------------------------------------------------------
bool pdGDBDriver::closeGDBCommunicationPipes()
{
    bool retVal = false;

    // Close our ends of GDB's communication pipes:
    bool rc1 = true;

    auto deleteFunc = [](osPipeSocket*& forClose)
    {
        if (nullptr != forClose)
        {
            bool rc1 = forClose->close();
            GT_ASSERT(rc1);

            delete forClose;
            forClose = nullptr;
        }
    };

    deleteFunc(_pGDBCommunicationPipe);
    deleteFunc(_pGDBProcessConsolePipe);
    deleteFunc(_pGDBProcessConsolePipeClient);

    // Clear the GDB communication pipe names:
    clearGDBCommunicationPipeNames();

    retVal = rc1;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::redirectGDBStdinAndStdoutToPipes
// Description:
//   Redirects this process stdin and stdout streams into the
//   pipes, created by createGDBCommunicationPipess().
//
//   This process will later on be replaced by the GDB process,
//   leaving the GDB process with the redirections we made.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        17/12/2006
//
// Implementation notes:
//
//  In POSIX, the following file descriptors exist for every program:
//  0 - stdin.
//  1 - stdout
//  2 - stderr
//
//  To redirect stdin/out into out pipe:
//  - The close(x) call wipes out the contents of file descriptor table entry x,
//    which is the table entry for stdin / stdout.
//  - The dup2() call copies the contents of the its input file descriptor
//    table entry to the stdin / stdout entry.
// ---------------------------------------------------------------------------
bool pdGDBDriver::redirectGDBStdinAndStdoutToPipes()
{
    bool retVal = false;

    bool stdinRedirected = false;
    bool stdoutRedirected = false;

    // Print a diagnotic message:
    if (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity())
    {
        gtString logMsg;
        logMsg.appendFormattedString(L"Redirecting gdb stdin and stdout pipes:\nSTDIN: %d -> 0\nSTDOUT: %d -> 1", _gdbProcessStdin[0], _gdbProcessStdout[1]);

        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
    }

    GT_IF_WITH_ASSERT(0 <= _gdbProcessStdin[0])
    {
        GT_IF_WITH_ASSERT((0 != _gdbProcessStdin[0]) && (1 != _gdbProcessStdin[0]))
        {
            // Redirect the stdin of this process into _gdbProcessStdin[0]:
            int rc1 = ::dup2(_gdbProcessStdin[0], 0);
            GT_IF_WITH_ASSERT(rc1 != -1)
            {
                // Close the duplicated file descriptor:
                int rc2 = ::close(_gdbProcessStdin[0]);
                GT_ASSERT(rc2 == 0);
                _gdbProcessStdin[0] = -1;

                stdinRedirected = true;
            }
        }
    }

    GT_IF_WITH_ASSERT(0 <= _gdbProcessStdout[1])
    {
        GT_IF_WITH_ASSERT((0 != _gdbProcessStdout[1]) && (1 != _gdbProcessStdout[1]))
        {
            // Redirect the stdout of this process into _gdbProcessStdout[1]:
            int rc3 = ::dup2(_gdbProcessStdout[1], 1);
            GT_IF_WITH_ASSERT(rc3 != -1)
            {
                // Close the duplicated file descriptor:
                int rc4 = ::close(_gdbProcessStdout[1]);
                GT_ASSERT(rc4 == 0);
                _gdbProcessStdout[1] = -1;

                stdoutRedirected = true;
            }
        }
    }

    retVal = stdinRedirected && stdoutRedirected;
    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::initializeGDB
// Description:
//  Sends gdb commands that initialize the communication with gdb
//  to a communication that enables us controlling it using pipes.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/12/2006
// ---------------------------------------------------------------------------
bool pdGDBDriver::initializeGDB()
{
    bool retVal = false;

    // Read gdb's outputs immediately:
    pdGDBOutputReader gdbOutputReader;
    bool ignoredS = false;
    bool ignoredT = false;
    bool rc1 = gdbOutputReader.readGDBOutput(*_pGDBCommunicationPipe, *this, PD_GDB_NULL_CMD, ignoredS, ignoredT);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Send GDB initialization commands:
        bool rc2 = sendGDBInitializationCommands();
        GT_IF_WITH_ASSERT(rc2)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::sendGDBInitializationCommands
// Description: Sends GDB commands that initialize GDB behavior the way
//              we want it to behave.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/8/2007
// ---------------------------------------------------------------------------
bool pdGDBDriver::sendGDBInitializationCommands()
{
    bool retVal = false;

    // Set GDB's "confirmation mode" to "off" (from now on, GDB will not ask yes/no questions):
    gtASCIIString commandArgs1 = "confirm off";
    bool rc1 = executeGDBCommand(PD_SET_GDB_VARIABLE_CMD, commandArgs1);

    if (!rc1)
    {
        _gdbOutputReader.resetGDBPrompt();
        _gdbOutputReader.flushGDBPrompt();
    }

    // Set GDB to non-stop mode. Only current thread will stopped on breakpoint hit. Other thread will suspends
    // from the linux process debugger class instance.
    commandArgs1 = "target-async 1";
    rc1 = executeGDBCommand(PD_SET_GDB_VARIABLE_CMD, commandArgs1);
    GT_ASSERT(rc1);

    commandArgs1 = "non-stop on";
    rc1 = executeGDBCommand(PD_SET_GDB_VARIABLE_CMD, commandArgs1);
    GT_ASSERT(rc1);

    commandArgs1 = "pagination off";
    rc1 = executeGDBCommand(PD_SET_GDB_VARIABLE_CMD, commandArgs1);
    GT_ASSERT(rc1);

    commandArgs1 = "print null-stop";
    rc1 = executeGDBCommand(PD_SET_GDB_VARIABLE_CMD, commandArgs1);
    GT_ASSERT(rc1);

    // On Mac OS X only:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    {
        // Yaki - 16/12/2008:
        // Start the debugged program directly (instead of starting a shell that will run the debugged program).
        // (If we are using a shell, the debugged process usually crashes on invocation).
        gtString commandArgs3 = "start-with-shell 0";
        bool rc3 = executeGDBCommand(PD_SET_GDB_VARIABLE_CMD, commandArgs3);
        GT_ASSERT(rc3)
    }
#endif // Mac OS X only

    if (rc1)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::buildGDBCommandString
// Description: Builds a GDB command string according to GDB MI2 interface syntax.
// Arguments: gdbCommand - The gdb command.
//            commandArgs - The gdb command arguments.
//            commandString - Will get the output command string.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/12/2006
// ---------------------------------------------------------------------------
bool pdGDBDriver::buildGDBCommandString(pdGDBCommandId gdbCommandId, const gtASCIIString& commandArgs,
                                        gtASCIIString& commandString)
{
    bool retVal = false;
    commandString.makeEmpty();

    // Get the command information:
    const pdGDBCommandInfo* pCommandInfo = pdGetGDBCommandInfo(gdbCommandId);
    GT_IF_WITH_ASSERT(pCommandInfo)
    {
        // Get the command arguments as GDB expects them:
        gtASCIIString commandArgumentsInGDBStyle;
        bool rc1 = buildGDBCommandArguments(*pCommandInfo, commandArgs, commandArgumentsInGDBStyle);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Build the command string:
            commandString = pCommandInfo->_commandExecutionString;
            commandString += ' ';
            commandString += commandArgumentsInGDBStyle;
            commandString += '\n';

            retVal = true;
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::buildGDBCommandArguments
// Description: Inputs GDB command arguments and adjust them to the way GDB
//              expects to receive them.
// Arguments: commandInfo - The executed command information.
//            commandArgs - The command arguments.
//            commandArgumentsInGDBStyle - The command arguments as GDB expects them.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        25/1/2009
// ---------------------------------------------------------------------------
bool pdGDBDriver::buildGDBCommandArguments(const pdGDBCommandInfo& commandInfo, const gtASCIIString& commandArgs, gtASCIIString& commandArgumentsInGDBStyle)
{
    bool retVal = true;
    commandArgumentsInGDBStyle.makeEmpty();

    // Will get true iff we applied a special command arguments handling:
    bool wereCommandArgsHandled = false;

    // Build the appropriate command arguments:
    if (commandInfo._commandId == PD_SET_WORK_DIR_CMD)
    {
        // On Mac OS X only:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        {
            // If the path does not have commas - add them:
            // (This solves a problem in which path's with double spaces are not treated correctly under Mac OS X)
            if (commandArgs.length() > 0 && commandArgs[0] != '\"')
            {
                commandArgumentsInGDBStyle = '\"';
                commandArgumentsInGDBStyle += commandArgs;
                commandArgumentsInGDBStyle += '\"';

                wereCommandArgsHandled = true;
            }
        }
#endif
    }

    // If we didn't apply a special command arguments handling:
    if (!wereCommandArgsHandled)
    {
        commandArgumentsInGDBStyle = commandArgs;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::writeToGDBInput
// Description: Writes a string into GDB's input pipe.
// Arguments: gdbInputString - A string that will be sent to GDB's input pipe.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/12/2006
// ---------------------------------------------------------------------------
bool pdGDBDriver::writeToGDBInput(const gtASCIIString& gdbInputString)
{
    bool retVal = false;

    // If under debug log severity, output debug printout:
    outputWritingToGDBLogMessage(gdbInputString);

    // Sanity check:
    int inputStringLength = gdbInputString.length();
    GT_IF_WITH_ASSERT(0 < inputStringLength)
    {
        // Write the input string into the GDB process stdin stream:
        bool rc1 = _pGDBCommunicationPipe->write(gdbInputString.asCharArray(), inputStringLength);
        GT_IF_WITH_ASSERT(rc1)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::outputWritingToGDBLogMessage
// Description: If in debug log severity, writes a "Writing to GDB" log message.
// Arguments: gdbInputString - A string that will be sent to GDB's input pipe.
// Author:      Yaki Tebeka
// Date:        17/4/2007
// ---------------------------------------------------------------------------
void pdGDBDriver::outputWritingToGDBLogMessage(const gtASCIIString& gdbInputString)
{
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        gtString debugMsg;
        debugMsg.fromASCIIString(gdbInputString.asCharArray());
        debugMsg.prepend(L"Writing to GDB: ");
        OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}


// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::clearGDBCommunicationPipeNames
// Description: Clears the GDB communication pipe names.
// Author:      Yaki Tebeka
// Date:        16/9/2007
// ---------------------------------------------------------------------------
void pdGDBDriver::clearGDBCommunicationPipeNames()
{
    _gdbProcessStdin[0] = -1;
    _gdbProcessStdin[1] = -1;

    _gdbProcessStdout[0] = -1;
    _gdbProcessStdout[1] = -1;

    _gdbProcessConsoleStdIn[0] = -1;
    _gdbProcessConsoleStdIn[1] = -1;
}



// ---------------------------------------------------------------------------
// Name:        pdGDBDriver::registerBrokenPipeSignalHandler
// Description: Register pdBorkenPipeSignalHandler as broken pipe signal handler.
// Author:      Yaki Tebeka
// Date:        15/10/2007
// ---------------------------------------------------------------------------
void pdGDBDriver::registerBrokenPipeSignalHandler()
{
    // A structure that will ask the OS to call pdBorkenPipeSignalHandler
    // whenever a SIGPIPE signal is received:
    struct sigaction newSignalAction;
    newSignalAction.sa_handler = pdBorkenPipeSignalHandler;
    sigemptyset(&newSignalAction.sa_mask);
    newSignalAction.sa_flags = 0;

    // Register the structure as the new SIGPIPE signal action:
    int rc1 = ::sigaction(SIGPIPE, &newSignalAction, NULL);
    GT_ASSERT(rc1 == 0);
}

///////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread created callback.
///
/// \param threadGDBId a gdb id of new created thread
/// \author Vadim Entov
/// \date 23/12/2015
void pdGDBDriver::OnThreadCreated(int threadGDBId)
{
    osCriticalSectionLocker lock(m_threadsInfoCS);

    auto existingIt = m_processExistingThreads.find(threadGDBId);
    auto stoppedIt = m_processStoppedThreads.find(threadGDBId);
    GT_IF_WITH_ASSERT((m_processExistingThreads.end() == existingIt) && (m_processStoppedThreads.end() == stoppedIt))
    {
        m_processExistingThreads.insert(threadGDBId);
        m_createdProcessThread = true;
    }
};

///////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread exiting callback.
///
/// \param threadGDBId a gdb id of exited thread
/// \author Vadim Entov
/// \date 23/12/2015
void pdGDBDriver::OnThreadExit(int threadGDBId)
{
    osCriticalSectionLocker lock(m_threadsInfoCS);

    auto existingIt = m_processExistingThreads.find(threadGDBId);
    auto stoppedIt = m_processStoppedThreads.find(threadGDBId);
    GT_IF_WITH_ASSERT(m_processExistingThreads.end() != existingIt)
    {
        m_processExistingThreads.erase(existingIt);

        if (m_processStoppedThreads.end() != stoppedIt)
        {
            m_processStoppedThreads.erase(stoppedIt);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread stopped callback.
///
/// \param threadGDBId a gdb id of stopped thread
/// \author Vadim Entov
/// \date 23/12/2015
void pdGDBDriver::OnThreadGDBStopped(int threadGDBId)
{
    osCriticalSectionLocker lock(m_threadsInfoCS);

    gtString debugMsg;
    debugMsg.appendFormattedString(L"Stopped thread, gdbId: %d", threadGDBId);
    OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

    auto existingIt = m_processExistingThreads.find(threadGDBId);
    auto stoppedIt = m_processStoppedThreads.find(threadGDBId);

    if ((m_processExistingThreads.end() != existingIt) && (m_processStoppedThreads.end() == stoppedIt))
    {
        m_processStoppedThreads.insert(threadGDBId);
    }
};

///////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread resumed callback.
///
/// \param threadGDBId a gdb id of resumed thread
/// \author Vadim Entov
/// \date 23/12/2015
void pdGDBDriver::OnThreadGDBResumed(int threadGDBId)
{
    osCriticalSectionLocker lock(m_threadsInfoCS);

    auto existingIt = m_processExistingThreads.find(threadGDBId);
    auto stoppedIt = m_processStoppedThreads.find(threadGDBId);

    if (m_processExistingThreads.end() == existingIt)
    {
        m_processExistingThreads.insert(threadGDBId);
    }

    if (m_processStoppedThreads.end() != stoppedIt)
    {
        m_processStoppedThreads.erase(stoppedIt);
    }

    gtString debugMsg;
    debugMsg.appendFormattedString(L"Resume thread gdbId: %d Stopped threads count: %d", threadGDBId, (int)m_processStoppedThreads.size());
    OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
};

///////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread resumed callback.
///
/// \param spyThreadGDBId a spy thread index or -1. In case spy thread
///     index is equal -1 check ALL threads for stopped state
/// \return true in case all debugged process threads stopped
/// \author Vadim Entov
/// \date 23/12/2015
bool pdGDBDriver::IsAllThreadsStopped(std::set<int>* pUnneededThreads)
{
    osCriticalSectionLocker lock(m_threadsInfoCS);

    bool result = (m_processExistingThreads == m_processStoppedThreads);

    if (!result && (nullptr != pUnneededThreads))
    {
        if (m_processExistingThreads.size() - m_processStoppedThreads.size() == pUnneededThreads->size())

            for (auto id : *pUnneededThreads)
            {
                if (m_processStoppedThreads.find(id) == m_processStoppedThreads.end())
                {
                    result = true;
                }
                else
                {
                    result = false;
                    break;
                }
            }
    }

    if (m_processExistingThreads.size() == 0)
    {
        result = m_createdProcessThread;
    }

    return result;
};

///////////////////////////////////////////////////////////////////////////////////////
/// \brief Check specified thread state
///
/// \param threadGDBId a requested thread GDB id
/// \return true in case thread in running state and vice versa
/// \author Vadim Entov
/// \date 19/01/2015
bool pdGDBDriver::IsThreadRunning(int threadGDBId)
{
    osCriticalSectionLocker lock(m_threadsInfoCS);

    return (m_processStoppedThreads.find(threadGDBId) == m_processStoppedThreads.end());
}


///////////////////////////////////////////////////////////////////////////////////////
/// \brief Check host process threads running state
///
/// \return true in case all debugged process threads resumed
/// \author Vadim Entov
/// \date 18/01/2015
bool pdGDBDriver::IsAllThreadsRunning()
{
    osCriticalSectionLocker lock(m_threadsInfoCS);

    bool result = m_processStoppedThreads.empty();

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////
/// \brief Get count of currently running threads
///
/// \return Running threads count
/// \author Vadim Entov
/// \date 28/01/2015
int pdGDBDriver::GetRunnungThreadsCount()
{
    osCriticalSectionLocker lock(m_threadsInfoCS);

    return m_processExistingThreads.size() - m_processStoppedThreads.size();
}

///////////////////////////////////////////////////////////////////////////////////////
/// \brief Get count of currently existing threads
///
/// \return Running threads count
/// \author Vadim Entov
/// \date 11/02/2016
int pdGDBDriver::GetExistingThreadsCount()
{
    osCriticalSectionLocker lock(m_threadsInfoCS);

    return m_processExistingThreads.size();
}


//////////////////////////////////////////////////////////////////////////////////////
/// \brief External starting GDB listener thread
///
/// \return true - success, false - fail
/// \author Vadim Entov
/// \date 03/02/2016
bool pdGDBDriver::StartBackgoundGDBListen()
{
    bool result = false;

    GT_IF_WITH_ASSERT(nullptr != _pGDBListenerThread)
    {
        result = _pGDBListenerThread->startListening(*_pGDBCommunicationPipe, *this, _gdbOutputReader, PD_CONTINUE_CMD);
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Get currently host process stopped threads
///
/// \return Set of currently host process stopped threads
/// \author Vadim Entov
/// \date 03/02/2016
const gtSet<int>& pdGDBDriver::GetStoppedThreads()
{
    osCriticalSectionLocker lock(m_threadsInfoCS);

    return m_processStoppedThreads;
}

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Flush GDB prompt
///
/// \return true - success, false - fail
/// \author Vadim Entov
/// \date 10/02/2016
bool pdGDBDriver::FlushPrompt()
{
    bool result = false;

    result = _gdbOutputReader.flushGDBPrompt();

    return result;
}

/////////////////////////////////////////////////////////////////////////////////////
/// \brief Suspend specific thread
///
/// \param threadGDBId a gdb thread index
/// \return true - success, false - failed
/// \author Vadim Entov
/// \date 18/02/2016
bool pdGDBDriver::SuspendThread(int threadGDBId)
{
    bool retVal = false;

    if (IsThreadRunning(threadGDBId))
    {
        executeGDBCommand(PD_GDB_STOP_THREAD_SYNC, "");

        /// TODO: Bad solution. If host application got any additional signal or breakpoint
        ///       the signal will be lost.
        while (IsThreadRunning(threadGDBId))
        {
            gtASCIIString gdbOutputLine;
            _gdbOutputReader.readGDBOutputLine(gdbOutputLine);
        }

        retVal = true;
    }
    else
    {
        retVal = true;
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////
/// \brief Resume specific thread
///
/// \param threadGDBId a gdb thread index
/// \return true - success, false - failed
/// \author Vadim Entov
/// \date 18/02/2016
bool pdGDBDriver::ResumeThread(int threadGDBId)
{
    bool retVal = false;

    if (!IsThreadRunning(threadGDBId))
    {
        bool rcRes = executeGDBCommand(PD_CONTINUE_THREAD_CMD, "");

        GT_ASSERT(rcRes);

        while (!IsThreadRunning(threadGDBId))
        {
            gtASCIIString gdbOutputLine;
            _gdbOutputReader.readGDBOutputLine(gdbOutputLine);
        }

        retVal = true;
    }
    else
    {
        retVal = true;
    }

    return retVal;
}


