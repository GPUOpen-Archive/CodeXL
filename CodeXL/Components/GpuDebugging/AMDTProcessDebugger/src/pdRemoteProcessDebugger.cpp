//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdRemoteProcessDebugger.cpp
///
//==================================================================================

//------------------------------ pdRemoteProcessDebugger.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osPipeSocketServer.h>
#include <AMDTOSWrappers/Include/osTCPSocketClient.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>
#include <AMDTAPIClasses/Include/apExpression.h>
#include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreationFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>

// The remote client is not used for the 64-bit Windows version of the process debugger:
#if !((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE))
    #include <AMDTRemoteClient/Include/CXLDaemonClient.h>
#endif

// Local:
#include <src/pdStringConstants.h>
#include <AMDTProcessDebugger/Include/pdRemoteProcessDebuggerCommandId.h>
#include <src/pdRemoteProcessDebugger.h>
#include <src/pdRemoteProcessDebuggerEventsListenerThread.h>
#include <src/pdRemoteProcessDebuggerDebuggingServerWatcherThread.h>
#include <src/pdRemoteProcessDebuggerTCPIPConnectionWaiterThread.h>

// Remote Debugging Server:
#include <AMDTRemoteDebuggingServer/Include/rdStringConstants.h>

// TCP/IP timeout:
#define PD_REMOTE_DEBUGGING_SERVER_TCP_IP_TIMEOUT OS_CHANNEL_INFINIT_TIME_OUT

// The timeout for the communication channel with the remote agent (in milliseconds):
#define PD_REMOTE_DEBUGGING_SERVER_TCP_IP_CONNECTION_WAIT_TIMEOUT_MS 1500

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::pdRemoteProcessDebugger
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        11/8/2009
// ---------------------------------------------------------------------------
pdRemoteProcessDebugger::pdRemoteProcessDebugger()
    : _pRemoteDebuggingServerAPIChannel(nullptr), _pRemoteDebuggingEventsAPIChannel(nullptr), _pEventsListenerThread(nullptr),
      _pServerWatcherThread(nullptr), _pDebuggedProcessCreationData(nullptr), _pRemoteDebuggedProcessCreationData(nullptr),
      _connectionMethod(PD_REMOTE_DEBUGGING_SERVER_NOT_CONNECTED), m_pDaemonClient(nullptr), m_daemonConnectionPort(0), m_pLocalLogFilePath(nullptr),
      _debuggedProcessExists(false), _debuggedProcessSuspended(false), _isDebugging64BitApplication(false), m_isSpiesAPIThreadRunning(false)
{
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::~pdRemoteProcessDebugger
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        11/8/2009
// ---------------------------------------------------------------------------
pdRemoteProcessDebugger::~pdRemoteProcessDebugger()
{
    initialize();

    StopEventListener();


    if (_pServerWatcherThread != nullptr)
    {
        _pServerWatcherThread->stopMonitoringDebuggingServer();
        delete _pServerWatcherThread;
        _pServerWatcherThread = nullptr;
    }

    if (_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        delete _pRemoteDebuggingServerAPIChannel;
        _pRemoteDebuggingServerAPIChannel = nullptr;
    }

    if (_pRemoteDebuggingEventsAPIChannel != nullptr)
    {
        delete _pRemoteDebuggingEventsAPIChannel;
        _pRemoteDebuggingEventsAPIChannel = nullptr;
    }
}

void pdRemoteProcessDebugger::StopEventListener()
{
    if (_pEventsListenerThread != nullptr)
    {
        _pEventsListenerThread->setEventsChannel(nullptr);
        _pEventsListenerThread->stopListening();
        delete _pEventsListenerThread;
        _pEventsListenerThread = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////
/// \brief Do host debugger (gdb, VS, etc..) initialization prerequestics
///
/// \param processCreationData a data needed for the process debugger creation and initailization
/// \return true - success, false - failed
/// \author Vadim Entov
/// \date 21/01/2016
bool pdRemoteProcessDebugger::initializeDebugger(const apDebugProjectSettings& processCreationData)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger initializing debug session", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    bool connectionEstablished = false;
    m_isSpiesAPIThreadRunning = false;

    // We default to 32-bit apps:
    _isDebugging64BitApplication = false;

    // Is this truly a remote target, or is it "local" remote debugging - via a shared memory object?
    if (processCreationData.isRemoteTarget())
    {
        const gtString& remoteTargetHostname = processCreationData.remoteTargetName();
        GT_IF_WITH_ASSERT(!remoteTargetHostname.isEmpty())
        {
            // Get the daemon connection port:
            gtUInt16 daemonConnectionPortNumber = processCreationData.remoteTargetDaemonConnectionPort();
            osPortAddress daemonConnectionPort(remoteTargetHostname, daemonConnectionPortNumber);
            connectionEstablished = launchRemoteDebuggingServerOnRemoteMachine(daemonConnectionPort, processCreationData.remoteTargetConnectionPort(), processCreationData.remoteTargetEventsPort());

            if (_pServerWatcherThread == nullptr)
            {
                _pServerWatcherThread = new pdRemoteProcessDebuggerDebuggingServerWatcherThread();
                _pServerWatcherThread->execute();
            }
        }
    }
    else // !processCreationData.isRemoteTarget()
    {
        // Open a pipe socket server to connect with the remote debugging server:
        gtString sharedMemObjectName = createSharedMemoryObjectPipeServer();

        GT_IF_WITH_ASSERT(!sharedMemObjectName.isEmpty())
        {
            gtString eventsPipeSharedMemObjName = createEventsSharedMemoryObjectPipeServer();
            GT_IF_WITH_ASSERT(!eventsPipeSharedMemObjName.isEmpty())
            {
                connectionEstablished = launchRemoteDebuggingServerOnLocalMachine(sharedMemObjectName, eventsPipeSharedMemObjName);
            }
        }
    }

    GT_IF_WITH_ASSERT(connectionEstablished && (_pRemoteDebuggingServerAPIChannel != nullptr) && (_pRemoteDebuggingEventsAPIChannel != nullptr))
    {
        // Create and run the events listener thread, if it doesn't exist yet:
        if (_pEventsListenerThread == nullptr)
        {
            _pEventsListenerThread = new pdRemoteProcessDebuggerEventsListenerThread;
            _pEventsListenerThread->execute();
        }

        // Set it to listen to the pipe we created:
        _pEventsListenerThread->setEventsChannel(_pRemoteDebuggingEventsAPIChannel);

        // Start listening to events:
        _pEventsListenerThread->startListening();

        // Log the debugged process creation data:
        _pDebuggedProcessCreationData = new apDebugProjectSettings(processCreationData);

        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_INITIALIZE_PROCESS_DEBUGGER_CMD;
        bool rcCreationData = _pDebuggedProcessCreationData->writeSelfIntoChannel(*_pRemoteDebuggingServerAPIChannel);
        GT_ASSERT(rcCreationData);
        *_pRemoteDebuggingServerAPIChannel >> retVal;

        if (retVal)
        {
            _pRemoteDebuggedProcessCreationData = new apDebugProjectSettings(processCreationData);
            _pRemoteDebuggedProcessCreationData->readSelfFromChannel(*_pRemoteDebuggingServerAPIChannel);
        }
    }

    if (!retVal)
    {
        // Launching the debugged process failed, close the connections we opened
        initialize();
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended initializing debug session", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::launchDebuggedProcess
// Description: Launch a process for a debug session.
// Arguments: processCreationData - Contains the data of the process to be launched.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::launchDebuggedProcess()
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger launching debugged process", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT((nullptr != _pRemoteDebuggingServerAPIChannel) && (nullptr != _pRemoteDebuggingEventsAPIChannel))
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_LAUNCH_DEBUGGED_PROCESS_CMD;
        *_pRemoteDebuggingServerAPIChannel >> retVal;

        // Ask the remote debugging server if this is a 64-bit application:
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_IS_DEBUGGING_64_BIT_APPLICATION_CMD;
        *_pRemoteDebuggingServerAPIChannel >> retVal;

        if (retVal)
        {
            *_pRemoteDebuggingServerAPIChannel >> _isDebugging64BitApplication;
        }
    }

    if (!retVal)
    {
        // Launching the debugged process failed, close the connections we opened
        initialize();
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended launching debugged process", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::doesSupportTwoStepLaunching
// Description: Returns true iff the remote process debugger implements the
//              launchDebuggedProcessInSuspendedMode and
//              continueDebuggedProcessFromSuspendedCreation functions.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        11/4/2011
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::doesSupportTwoStepLaunching() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger support two step launch?", OS_DEBUG_LOG_EXTENSIVE);

    // TO_DO: when we support REAL remote debugging, this should be retrieved from the debugging server:
    return true;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::launchDebuggedProcessInSuspendedMode
// Description: Creates the debugged process but does not start its run. When
//              this function returns, debuggedProcessId() has a valid value.
// Arguments: const apDebugProjectSettings& processCreationData
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        11/4/2011
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::launchDebuggedProcessInSuspendedMode()
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger launching debugged process in suspended mode", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT((nullptr != _pRemoteDebuggingServerAPIChannel) && (nullptr != _pRemoteDebuggingEventsAPIChannel))
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_LAUNCH_DEBUGGED_PROCESS_IN_SUSPENDED_MODE_CMD;
        *_pRemoteDebuggingServerAPIChannel >> retVal;

        // Ask the remote debugging server if this is a 64-bit application:
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_IS_DEBUGGING_64_BIT_APPLICATION_CMD;
        *_pRemoteDebuggingServerAPIChannel >> retVal;

        if (retVal)
        {
            *_pRemoteDebuggingServerAPIChannel >> _isDebugging64BitApplication;
        }
    }

    if (!retVal)
    {
        // Launching the debugged process failed, close the connections we opened
        initialize();
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended launching debugged process in suspended mode", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::continueDebuggedProcessFromSuspendedCreation
// Description: Completes the debugged process launching after
//              launchDebuggedProcessInSuspendedMode, by starting the debugged
//              process run.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        11/4/2011
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::continueDebuggedProcessFromSuspendedCreation()
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger continue debugged process from suspended creation", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_CONTINUE_DEBUGGED_PROCESS_FROM_SUSPENDED_CREATION_CMD;
        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended continue debugged process from suspended creation", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::debuggedProcessExists
// Description: Returns true iff a debugged process exists.
// Author:      Yaki Tebeka
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::debuggedProcessExists() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger debugged process exists?", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    /*  // Uri, 18/8/09: This code causes slowdown as this function is queried very
        // often. It DOES work (and the other side is implementer in the remote
        // debugging server - but we use a caching mechanism instead.
        GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
        {
            *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_DEBUGGED_PROCESS_EXISTS_CMD;
            *_pRemoteDebuggingServerAPIChannel >> retVal;
        }
        */

    retVal = _debuggedProcessExists;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::debuggedProcessCreationData
// Description: Returns the data used for launching the debugged process.
// Author:      Yaki Tebeka
// Date:        11/8/2009
// ---------------------------------------------------------------------------
const apDebugProjectSettings* pdRemoteProcessDebugger::debuggedProcessCreationData() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get debugged process creation data", OS_DEBUG_LOG_EXTENSIVE);
    return _pDebuggedProcessCreationData;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::serverSideDebuggedProcessCreationData
// Description: Returns the process creation data, as processed by the remote debugger
// Author:      Uri Shomroni
// Date:        29/8/2013
// ---------------------------------------------------------------------------
const apDebugProjectSettings* pdRemoteProcessDebugger::serverSideDebuggedProcessCreationData() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get server-side debugged process creation data", OS_DEBUG_LOG_EXTENSIVE);
    return _pRemoteDebuggedProcessCreationData;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::terminateDebuggedProcess
// Description: Terminates the debugged process.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::terminateDebuggedProcess()
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger terminating debugged process", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_TERMINATE_DEBUGGED_PROCESS_CMD;
        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended terminating debugged process", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::isDebugging64BitApplication
// Description: Query whether the debugged application is a 64-bit application.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        21/9/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::isDebugging64BitApplication(bool& is64Bit) const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger is debugged process 64 bit?", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = true;

    is64Bit = _isDebugging64BitApplication;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::amountOfDebuggedProcessThreads
// Description: Returns the amount of debugged process threads.
// Author:      Yaki Tebeka
// Date:        11/8/2009
// ---------------------------------------------------------------------------
int pdRemoteProcessDebugger::amountOfDebuggedProcessThreads() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get amount of debugged process thread", OS_DEBUG_LOG_EXTENSIVE);
    int retVal = 0;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_AMOUNT_OF_DEBUGGED_PROCESS_THREADS_CMD;
        gtInt32 retValAsInt32 = 0;
        *_pRemoteDebuggingServerAPIChannel >> retValAsInt32;
        retVal = (int)retValAsInt32;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended getting amount of debugged process threads", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::getThreadId
// Description: Gets an OS thread Id from the internal thread index.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::getThreadId(int threadIndex, osThreadId& threadId) const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get thread id", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_GET_THREAD_ID_CMD;
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)threadIndex;
        *_pRemoteDebuggingServerAPIChannel >> retVal;

        if (retVal)
        {
            gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
            *_pRemoteDebuggingServerAPIChannel >> threadIdAsUInt64;
            threadId = (osThreadId)threadIdAsUInt64;
        }
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended getting thread id", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::setSpiesAPIThreadId
// Description: Sets the spy API thread Id
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
void pdRemoteProcessDebugger::setSpiesAPIThreadId(osThreadId spiesAPIThreadId)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger set server API thread id", OS_DEBUG_LOG_EXTENSIVE);
    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_SET_SPY_API_THREAD_ID_CMD;
        *_pRemoteDebuggingServerAPIChannel << (gtUInt64)spiesAPIThreadId;
    }
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended set server API thread id", OS_DEBUG_LOG_EXTENSIVE);
}


// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::spiesAPIThreadIndex
// Description: Gets the internal index of the spies API thread.
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
int pdRemoteProcessDebugger::spiesAPIThreadIndex() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get server API thread index", OS_DEBUG_LOG_EXTENSIVE);
    int retVal = -1;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_SPIES_API_THREAD_INDEX_CMD;

        gtInt32 threadIndexAsInt32 = -1;
        *_pRemoteDebuggingServerAPIChannel >> threadIndexAsInt32;
        retVal = (int)threadIndexAsInt32;
    }
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended getting server API thread index", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::mainThreadId
// Description: Gets the Thread Id of the debugged application's main thread.
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osThreadId pdRemoteProcessDebugger::mainThreadId() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get main thread id", OS_DEBUG_LOG_EXTENSIVE);
    osThreadId retVal = OS_NO_THREAD_ID;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_MAIN_THREAD_ID_CMD;

        gtUInt64 threadIdAsUInt64 = GT_UINT64_MAX;
        *_pRemoteDebuggingServerAPIChannel >> threadIdAsUInt64;
        retVal = (osThreadId)threadIdAsUInt64;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended getting main thread id", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::spiesAPIThreadId
// Description: Gets the thread Id of the spies API thread
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osThreadId pdRemoteProcessDebugger::spiesAPIThreadId() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get server API thread id", OS_DEBUG_LOG_EXTENSIVE);
    osThreadId retVal = OS_NO_THREAD_ID;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_SPIES_API_THREAD_ID_CMD;

        gtUInt64 threadIdAsUInt64 = GT_UINT64_MAX;
        *_pRemoteDebuggingServerAPIChannel >> threadIdAsUInt64;
        retVal = (osThreadId)threadIdAsUInt64;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended getting server API thread id", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::debuggedProcessId
// Description: Returns the debugged process ID.
// Author:      Uri Shomroni
// Date:        16/9/2010
// ---------------------------------------------------------------------------
osProcessId pdRemoteProcessDebugger::debuggedProcessId() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get debugged process id", OS_DEBUG_LOG_EXTENSIVE);
    osProcessId retVal = 0;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_DEBUGGED_PROCESS_ID_CMD;

        gtUInt64 processIdAsUInt64 = GT_UINT64_MAX;
        *_pRemoteDebuggingServerAPIChannel >> processIdAsUInt64;
        retVal = (osThreadId)processIdAsUInt64;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended getting debugged process id", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::isSpiesAPIThreadRunning
// Description: Queries whether the spies API thread is running
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::isSpiesAPIThreadRunning() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get is server API thread running", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    // If the debugged process doesn't exist, it certainly can't be connected:
    if (_debuggedProcessExists)
    {
        retVal = m_isSpiesAPIThreadRunning;

        // If we did not yet detect the Spies API thread as running, query again:
        if (!m_isSpiesAPIThreadRunning)
        {
            OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get is server API thread running from remote debugging server", OS_DEBUG_LOG_EXTENSIVE);
            GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
            {
                *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_IS_SPY_API_THREAD_RUNNING_CMD;

                *_pRemoteDebuggingServerAPIChannel >> retVal;
            }

            // If there was a change:
            if (retVal)
            {
                // Update the cache member:
                ((pdRemoteProcessDebugger*)this)->m_isSpiesAPIThreadRunning = retVal;
            }

            OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended getting is server API thread running from remote debugging server", OS_DEBUG_LOG_EXTENSIVE);
        }
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended getting is server API thread running", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::suspendDebuggedProcess
// Description: Suspends the debugged process
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::suspendDebuggedProcess()
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger suspending debugged process", OS_DEBUG_LOG_EXTENSIVE);

    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_SUSPEND_DEBUGGED_PROCESS_CMD;

        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended suspending debugged process", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::resumeDebuggedProcess
// Description: Resumes the debugged process run
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::resumeDebuggedProcess()
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger resuming debugged process", OS_DEBUG_LOG_EXTENSIVE);

    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_RESUME_DEBUGGED_PROCESS_CMD;

        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended resuming debugged process", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::isDebuggedProcssSuspended
// Description: Queries whether the debugged process is suspended
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::isDebuggedProcssSuspended()
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger is debugged process suspended?", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    /*  // Uri, 18/8/09: This code causes slowdown as this function is queried very
        // often. It DOES work (and the other side is implementer in the remote
        // debugging server - but we use a caching mechanism instead.
        GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
        {
            *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_IS_DEBUGGED_PROCESS_SUSPENDED_CMD;

            *_pRemoteDebuggingServerAPIChannel >> retVal;
        }
        */

    retVal = _debuggedProcessSuspended;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::suspendDebuggedProcessThread
// Description: Suspends a debugged process thread
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::suspendDebuggedProcessThread(osThreadId threadId)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger suspending debugged process thread", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_SUSPEND_DEBUGGED_PROCESS_THREAD_CMD;
        *_pRemoteDebuggingServerAPIChannel << (gtUInt64)threadId;

        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended suspending debugged process thread", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::resumeDebuggedProcessThread
// Description: Resume a debugged process thread
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::resumeDebuggedProcessThread(osThreadId threadId)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger resuming debugged process thread", OS_DEBUG_LOG_EXTENSIVE);

    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_RESUME_DEBUGGED_PROCESS_THREAD_CMD;
        *_pRemoteDebuggingServerAPIChannel << (gtUInt64)threadId;

        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended resuming debugged process thread", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::getDebuggedThreadCallStack
// Description: Gets the debugged thread's current calls stack
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::getDebuggedThreadCallStack(osThreadId threadId, osCallStack& callStack, bool hideSpyDLLsFunctions)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get thread call stack", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_GET_DEBUGGED_THREAD_CALL_STACK_CMD;
        *_pRemoteDebuggingServerAPIChannel << (gtUInt64)threadId;
        *_pRemoteDebuggingServerAPIChannel << hideSpyDLLsFunctions;

        *_pRemoteDebuggingServerAPIChannel >> retVal;

        if (retVal)
        {
            callStack.readSelfFromChannel(*_pRemoteDebuggingServerAPIChannel);
        }
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended getting thread call stack", OS_DEBUG_LOG_EXTENSIVE);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::fillCallsStackDebugInfo
// Description: Fills a calls stack's debug info
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
void pdRemoteProcessDebugger::fillCallsStackDebugInfo(osCallStack& callStack, bool hideSpyDLLsFunctions)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger filling call stack debug info", OS_DEBUG_LOG_EXTENSIVE);
    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_FILL_CALL_STACK_DEBUG_INFO_CMD;
        bool rcStack = callStack.writeSelfIntoChannel(*_pRemoteDebuggingServerAPIChannel);
        *_pRemoteDebuggingServerAPIChannel << hideSpyDLLsFunctions;
        GT_ASSERT(rcStack);

        rcStack = callStack.readSelfFromChannel(*_pRemoteDebuggingServerAPIChannel);
        GT_ASSERT(rcStack);
    }
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended filling call stack debug info", OS_DEBUG_LOG_EXTENSIVE);
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::fillThreadCreatedEvent
// Description: Fills a thread created event
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
void pdRemoteProcessDebugger::fillThreadCreatedEvent(apThreadCreatedEvent& eve)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger filling thread created event", OS_DEBUG_LOG_EXTENSIVE);
    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_FILL_THERAD_CREATED_EVENT_CMD;
        bool rcEvent = eve.writeSelfIntoChannel(*_pRemoteDebuggingServerAPIChannel);
        GT_ASSERT(rcEvent);

        rcEvent = eve.readSelfFromChannel(*_pRemoteDebuggingServerAPIChannel);
        GT_ASSERT(rcEvent);
    }
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended filling thread created event", OS_DEBUG_LOG_EXTENSIVE);
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::canGetCallStacks
// Description: Query whether this process debugger can get debugged process
//              calls stacks by itself (without the API
// Author:      Uri Shomroni
// Date:        25/1/2010
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::canGetCallStacks()
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger can get call stacks?", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    // Any process debugger may be implemented on the debugging server side, so
    // we need to query the remote debugging server:
    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_CAN_GET_CALL_STACKS_CMD;

        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended querying can get call stacks", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::canMakeThreadExecuteFunction
// Description: Returns whether this process debugger implementation supports
//              the "make thread execute function" mechanism.
// Author:      Uri Shomroni
// Date:        2/11/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::canMakeThreadExecuteFunction(const osThreadId& threadId)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger can make thread execute function?", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    // Any process debugger may be implemented on the debugging server side, so
    // we need to query the remote debugging server:
    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_CAN_MAKE_THREAD_EXECUTE_FUNCTION_CMD;

        *_pRemoteDebuggingServerAPIChannel << (gtUInt64)threadId;

        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended querying can make thread execute function", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::makeThreadExecuteFunction
// Description: Makes a debugged process thread execute a given function
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::makeThreadExecuteFunction(const osThreadId& threadId, osProcedureAddress64 funcAddress)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger making thread execute function", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_MAKE_THREAD_EXECUTE_FUNCTION_CMD;
        *_pRemoteDebuggingServerAPIChannel << (gtUInt64)threadId;
        *_pRemoteDebuggingServerAPIChannel << (gtUInt64)funcAddress;

        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended making thread execute function", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::functionExecutionMode
// Description:
// Return Val:  pdProcessDebugger::FunctionExecutionMode
// Author:      Uri Shomroni
// Date:        12/2/2010
// ---------------------------------------------------------------------------
pdProcessDebugger::FunctionExecutionMode pdRemoteProcessDebugger::functionExecutionMode() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get function execution mode", OS_DEBUG_LOG_EXTENSIVE);
    FunctionExecutionMode retVal = pdProcessDebugger::functionExecutionMode();

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_FUNCTION_EXECUTION_MODE_CMD;

        gtUInt32 executionModeAsUInt32 = (gtUInt32)retVal;
        *_pRemoteDebuggingServerAPIChannel >> executionModeAsUInt32;
        retVal = (FunctionExecutionMode)executionModeAsUInt32;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended getting function execution mode", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::afterAPICallIssued
// Description: Run after an API Call is issued
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
void pdRemoteProcessDebugger::afterAPICallIssued()
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger after API call issued", OS_DEBUG_LOG_EXTENSIVE);
    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_AFTER_API_CALL_ISSUED_CMD;
    }
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended after API call issued", OS_DEBUG_LOG_EXTENSIVE);
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebugger::setLocalLogFileDirectory
// Description: Sets the log file path for acquiring remote files.
//              Note that is is not passed to the remote debugger, as the path
//              is (by definition) on the local machine.
// Author:      Uri Shomroni
// Date:        23/10/2013
// ---------------------------------------------------------------------------
void pdRemoteProcessDebugger::setLocalLogFileDirectory(const osFilePath& localLogFilePath)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger setting local log file path", OS_DEBUG_LOG_EXTENSIVE);

    if (nullptr != m_pLocalLogFilePath)
    {
        delete m_pLocalLogFilePath;
        m_pLocalLogFilePath = nullptr;
    }

    m_pLocalLogFilePath = new osFilePath(localLogFilePath);
    GT_ASSERT(nullptr != m_pLocalLogFilePath);
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::remoteToLocalFilePath
// Description: Pulls in a file from a remote host. For Daemon connection,
//              It then copies over the file using the daemon.
// Author:      Uri Shomroni
// Date:        30/9/2013
// ---------------------------------------------------------------------------
void pdRemoteProcessDebugger::remoteToLocalFilePath(osFilePath& io_filePath, bool useCache) const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger remote to local file path", OS_DEBUG_LOG_EXTENSIVE);
    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_REMOTE_TO_LOCAL_FILE_PATH_CMD;
        *_pRemoteDebuggingServerAPIChannel << useCache;

        // Send the original file path:
        io_filePath.writeSelfIntoChannel(*_pRemoteDebuggingServerAPIChannel);

        // Get the (possibly modified) file path back:
        io_filePath.readSelfFromChannel(*_pRemoteDebuggingServerAPIChannel);
    }

    if (nullptr != m_pDaemonClient)
    {
        bool getFile = true;

        // Search for the file path in the cache, if requested:
        if (useCache)
        {
            gtMap<gtString, osFilePath>::const_iterator findIter = m_remoteToLocalFilePathCache.find(io_filePath.asString());
            gtMap<gtString, osFilePath>::const_iterator endIter = m_remoteToLocalFilePathCache.end();

            if (findIter != endIter)
            {
                // We have a cache hit!
                OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger remote to local file path cache hit", OS_DEBUG_LOG_EXTENSIVE);
                getFile = false;
                io_filePath = findIter->second;
            }
        }

        if (getFile)
        {
            OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger remote to local file path getting via daemon", OS_DEBUG_LOG_EXTENSIVE);
            GT_IF_WITH_ASSERT(m_pDaemonClient->IsInitialized(m_daemonConnectionPort))
            {
                // Construct the local file path:
                osFilePath localFilePath(osFilePath::OS_TEMP_DIRECTORY);

                if (nullptr != m_pLocalLogFilePath)
                {
                    localFilePath = *m_pLocalLogFilePath;
                    localFilePath.reinterpretAsDirectory();
                }
                else // nullptr == m_pLocalLogFilePath
                {
                    GT_IF_WITH_ASSERT(nullptr != _pDebuggedProcessCreationData)
                    {
                        localFilePath = _pDebuggedProcessCreationData->logFilesFolder().directoryPath();
                        localFilePath.reinterpretAsDirectory();
                    }
                }

                // Copy the file name and extension:
                localFilePath.setFromOtherPath(io_filePath, false, true, true);

                // Get the file:
                bool rcFl = m_pDaemonClient->GetRemoteFile(io_filePath.asString(), localFilePath.asString(), false, nullptr, nullptr);

                if (rcFl)
                {
                    // Add this to the cache (we add files to the cache even if we didn't request them there, since
                    // it's a fairly "cheap" operation):
                    ((pdRemoteProcessDebugger*)this)->m_remoteToLocalFilePathCache[io_filePath.asString()] = localFilePath;
                    io_filePath = localFilePath;
                }
            }
            OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger remote to local file path finished getting via daemon", OS_DEBUG_LOG_EXTENSIVE);
        }
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger remote to local file path ended", OS_DEBUG_LOG_EXTENSIVE);
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::onEvent
// Description: Called when a debugged process event occurs:
// Author:      Uri Shomroni
// Date:        12/8/2009
// ---------------------------------------------------------------------------
void pdRemoteProcessDebugger::onEvent(const apEvent& eve, bool& vetoEvent)
{
    // Unused parameters
    (void)vetoEvent;

    apEvent::EventType eveType = eve.eventType();

    switch (eveType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            // The debugged process was terminated, kill the debugging server:
            GT_IF_WITH_ASSERT(_pServerWatcherThread != nullptr)
            {
                _pServerWatcherThread->stopMonitoringDebuggingServer();
            }
        }
        break;

        default:
        {
            // Do nothing
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::onEventRegistration
// Description: Called when a debugged process event is about to be registered,
//              we use it to update our status and remove duplicate termination events
// Author:      Uri Shomroni
// Date:        18/8/2009
// ---------------------------------------------------------------------------
void pdRemoteProcessDebugger::onEventRegistration(apEvent& eve, bool& vetoEvent)
{
    apEvent::EventType eveType = eve.eventType();

    switch (eveType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            // Don't let this event get thrown twice:
            if (!_debuggedProcessExists)
            {
                vetoEvent = true;
            }
            else
            {
                // Note the debugged process doesn't exist:
                delete m_pLocalLogFilePath;
                m_pLocalLogFilePath = nullptr;
                m_remoteToLocalFilePathCache.clear();
                _debuggedProcessExists = false;
                _debuggedProcessSuspended = false;
                m_isSpiesAPIThreadRunning = false;

                if (nullptr != _pEventsListenerThread)
                {
                    _pEventsListenerThread->stopListening();
                }

                // If the daemon connection exists, we can close it now:
                cleanupDaemonConnection();
            }
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            if (nullptr != _pEventsListenerThread)
            {
                _pEventsListenerThread->stopListening();
            }

            // If the daemon connection exists, we can close it now:
            cleanupDaemonConnection();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            // Note the debugged process exists:
            m_remoteToLocalFilePathCache.clear();
            _debuggedProcessExists = true;
            _debuggedProcessSuspended = false;
            m_isSpiesAPIThreadRunning = false;
        }
        break;

        case apEvent::AP_BREAKPOINT_HIT:
        {
            _debuggedProcessSuspended = true;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            _debuggedProcessSuspended = true;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        {
            _debuggedProcessSuspended = false;
        }
        break;

        case apEvent::AP_BEFORE_KERNEL_DEBUGGING_EVENT:
        case apEvent::AP_AFTER_KERNEL_DEBUGGING_EVENT:
        {
            // These need to pass to the remote debugging server at registration time:
            passDebugEventToRemoteDebuggingServer(eve);
        }
        break;

        case apEvent::AP_THREAD_TERMINATED:
        {
            // If a thread stopped running, it may have been the API thread. Mark as not running it so that it is updated on the next query:
            m_isSpiesAPIThreadRunning = false;
        }
        break;

        default:
        {
            // Do nothing
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::isRemoteDebuggingServerAlive
// Description: Checks if the local / remote RDS exists and returns the answer.
//              The answer is whether AT LEAST ONE of the requested RDSes running.
// Author:      Uri Shomroni
// Date:        16/10/2013
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::isRemoteDebuggingServerAlive(bool checkLocal, bool checkRemote)
{
    bool retVal = false;
    bool rcLoc = false;
    bool rcRem = false;

    // Local RDS:
    if (checkLocal)
    {
        // If we have a local server, we have a server watcher thread:
        if (nullptr != _pServerWatcherThread)
        {
            // Is the thread currently monitoring anything?
            rcLoc = _pServerWatcherThread->isMonitoringRemoteDebuggingServer();
        }
    }

    // Remote RDS:
    if (checkRemote)
    {
        // Sanity check:
        if (nullptr != m_pDaemonClient)
        {
            // If we've connected:
            if (m_pDaemonClient->IsInitialized(m_daemonConnectionPort))
            {
                // Request the status:
                DaemonSessionStatus rdsStatus = dssSessionStatusCount;
                bool rcStat = m_pDaemonClient->GetSessionStatus(romDEBUG, rdsStatus);

                if (rcStat && (dssAlive == rdsStatus))
                {
                    // The remote target RDS is running:
                    rcRem = true;
                }
            }
        }
    }

    retVal = (checkLocal && rcLoc) || (checkRemote && rcRem);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::passDebugEventToRemoteDebuggingServer
// Description: Used to pass debug events to the remote debugging server, usually
//              these are events from the spy event listener thread that are
//              relevant to the process debugger.
// Author:      Uri Shomroni
// Date:        26/6/2011
// ---------------------------------------------------------------------------
void pdRemoteProcessDebugger::passDebugEventToRemoteDebuggingServer(const apEvent& eve)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger passing debugged process event to remote debugging server", OS_DEBUG_LOG_EXTENSIVE);
    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_HANDLE_DEBUG_EVENT;
        *_pRemoteDebuggingServerAPIChannel << (const osTransferableObject&)eve;

        bool retVal = false;
        *_pRemoteDebuggingServerAPIChannel >> retVal;
        GT_ASSERT(retVal);
    }
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger ended passing debugged process event to remote debugging server", OS_DEBUG_LOG_EXTENSIVE);
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::createSharedMemoryObjectPipeServer
// Description: Creates a shared memory object pipe server used for debugging commands, and
//              returns its file path as a string (or an empty string on failure)
// Author:      Uri Shomroni
// Date:        12/8/2009
// ---------------------------------------------------------------------------
gtString pdRemoteProcessDebugger::createSharedMemoryObjectPipeServer()
{
    gtString retVal;

    // Get the temp directory and the current time and date, to construct the shared memory object name:
    osFilePath tempDir(osFilePath::OS_TEMP_DIRECTORY);
    osTime now;
    now.setFromCurrentTime();
    gtString dateStr;
    gtString timeStr;
    now.dateAsString(dateStr, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);
    now.timeAsString(timeStr, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);

    // Create the shared object's full path:
    gtString uniqueSharedObjectName = tempDir.asString();
    uniqueSharedObjectName.append(osFilePath::osPathSeparator);
    uniqueSharedObjectName.append(PD_STR_remoteProcessDebuggerSharedMemoryObject);
    uniqueSharedObjectName.append('-');
    uniqueSharedObjectName.append(dateStr);
    uniqueSharedObjectName.append('-');
    uniqueSharedObjectName.append(timeStr);

    // Create the pipe server:
    osPipeSocketServer* pPipeSocketServer = new osPipeSocketServer(uniqueSharedObjectName);
    GT_IF_WITH_ASSERT(pPipeSocketServer != nullptr)
    {
        // Open it:
        bool rcPipeServer = pPipeSocketServer->open();
        GT_IF_WITH_ASSERT(rcPipeServer)
        {
            // Return the success values:
            _pRemoteDebuggingServerAPIChannel = pPipeSocketServer;
            _connectionMethod = PD_REMOTE_DEBUGGING_SHARED_MEMORY_OBJECT;
            retVal = uniqueSharedObjectName;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::createEventsSharedMemoryObjectPipeServer
// Description: Creates a shared memory object pipe server used for events, and
//              returns its file path as a string (or an empty string on failure)
// Author:      Uri Shomroni
// Date:        12/8/2009
// ---------------------------------------------------------------------------
gtString pdRemoteProcessDebugger::createEventsSharedMemoryObjectPipeServer()
{
    gtString retVal;

    // Get the temp directory and the current time and date, to construct the shared memory object name:
    osFilePath tempDir(osFilePath::OS_TEMP_DIRECTORY);
    osTime now;
    now.setFromCurrentTime();
    gtString dateStr;
    gtString timeStr;
    now.dateAsString(dateStr, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);
    now.timeAsString(timeStr, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);

    // Create the shared object's full path:
    gtString uniqueSharedObjectName = tempDir.asString();
    uniqueSharedObjectName.append(osFilePath::osPathSeparator);
    uniqueSharedObjectName.append(PD_STR_remoteProcessDebuggerEventsSharedMemoryObject);
    uniqueSharedObjectName.append('-');
    uniqueSharedObjectName.append(dateStr);
    uniqueSharedObjectName.append('-');
    uniqueSharedObjectName.append(timeStr);

    // Create the pipe server:
    osPipeSocketServer* pPipeSocketServer = new osPipeSocketServer(uniqueSharedObjectName);
    GT_IF_WITH_ASSERT(pPipeSocketServer != nullptr)
    {
        // Open it:
        bool rcPipeServer = pPipeSocketServer->open();
        GT_IF_WITH_ASSERT(rcPipeServer)
        {
            _pRemoteDebuggingEventsAPIChannel = pPipeSocketServer;
            retVal = uniqueSharedObjectName;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::launchRemoteDebuggingServerOnLocalMachine
// Description: Launches the remote debugging server on the local machine using the
//              given shared memory object names and starts a thread to watch it.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        12/8/2009
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::launchRemoteDebuggingServerOnLocalMachine(const gtString& sharedMemObj, const gtString& eventsSharedMemObj)
{
    // Unused parameters:
    (void)sharedMemObj;
    (void)eventsSharedMemObj;

    bool retVal = false;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Currently, the only supported local debugging server is the 64-bit debugging server:
    osFilePath remoteDebuggingServerExecutable;
    bool rcPath = true;
    bool isDllsDirSet = osGetCurrentApplicationDllsPath(remoteDebuggingServerExecutable);

    if (!isDllsDirSet)
    {
        rcPath = osGetCurrentApplicationPath(remoteDebuggingServerExecutable);
    }

    GT_IF_WITH_ASSERT(rcPath)
    {
        remoteDebuggingServerExecutable.setFileName(PD_STR_remoteDebuggingServer64ExecutableFileName);

        // The first command line argument must be the executable path:
        gtString commandLineArgs = remoteDebuggingServerExecutable.asString();
        commandLineArgs.append('\"').prepend('\"');
        gtString workDir;
        osDirectory executableDir;
        remoteDebuggingServerExecutable.getFileDirectory(executableDir);
        workDir = executableDir.directoryPath().asString();

        // Initialize the output structs:
        STARTUPINFO startupInfo = {0};
        PROCESS_INFORMATION processInfo = {0};

        // Set the environment variables used by the server process:
        osEnvironmentVariable debuggingMode, sharedMemObjName, eventsSharedMemObjName, debugLogSeverity;
        debuggingMode._name = RD_STR_DebuggingModeEnvVar;
        debuggingMode._value = RD_STR_DebuggingModeSharedMemoryObject;
        sharedMemObjName._name = RD_STR_SharedMemoryObjectNameEnvVar;
        sharedMemObjName._value = sharedMemObj;
        eventsSharedMemObjName._name = RD_STR_EventsSharedMemoryObjectNameEnvVar;
        eventsSharedMemObjName._value = eventsSharedMemObj;
        debugLogSeverity._name = RD_STR_DebugLogSeverityEnvVar;
        debugLogSeverity._value = osDebugLogSeverityToString(osDebugLog::instance().loggedSeverity());
        osSetCurrentProcessEnvVariable(debuggingMode);
        osSetCurrentProcessEnvVariable(sharedMemObjName);
        osSetCurrentProcessEnvVariable(eventsSharedMemObjName);
        osSetCurrentProcessEnvVariable(debugLogSeverity);

        // Create the server process:
        int rc =
            CreateProcess(nullptr,                                 // Specify the path in the Command line arguments
                          (LPWSTR)commandLineArgs.asCharArray(),    // Command line arguments
                          nullptr,                                 // Default process security attributes
                          nullptr,                                 // Default thread security attributes
                          FALSE,                                // Don't inherit handles
                          CREATE_NEW_CONSOLE,                   // Don't debug this process - if we try to, the command will fail.
                          // Also, this is a console app - run it without a visible console.
                          nullptr,                                 // Environment
                          (LPWSTR)workDir.asCharArray(),        // Work dir
                          &startupInfo,                         // Pointer to STARTUPINFO structure.
                          &processInfo);                        // Pointer to PROCESS_INFORMATION structure.

        retVal = (rc != 0);

        if (!retVal)
        {
            DWORD errCode = ::GetLastError();
            gtString rdsErr = L"Could not launch Remote Debugging Server. Error code:";
            rdsErr.appendFormattedString(L"%#x", errCode);

            GT_ASSERT_EX(retVal, rdsErr.asCharArray());
        }

        // Clear the environment variables:
        osRemoveCurrentProcessEnvVariable(RD_STR_DebuggingModeEnvVar);
        osRemoveCurrentProcessEnvVariable(RD_STR_SharedMemoryObjectNameEnvVar);
        osRemoveCurrentProcessEnvVariable(RD_STR_EventsSharedMemoryObjectNameEnvVar);
        osRemoveCurrentProcessEnvVariable(RD_STR_DebugLogSeverityEnvVar);

        GT_IF_WITH_ASSERT(retVal)
        {
            osPipeSocketServer* pSocketServer = (osPipeSocketServer*)_pRemoteDebuggingServerAPIChannel;
            GT_IF_WITH_ASSERT(pSocketServer != nullptr)
            {
                // Wait for a client connection, so we will be synchronized with the debugging server:
                pSocketServer->waitForClientConnection();
            }

            osPipeSocketServer* pEventsSocketServer = (osPipeSocketServer*)_pRemoteDebuggingEventsAPIChannel;
            GT_IF_WITH_ASSERT(pEventsSocketServer != nullptr)
            {
                // Wait for a client connection, so we will be synchronized with the debugging server:
                pEventsSocketServer->waitForClientConnection();
            }

            // Create the server watcher thread if needed:
            if (_pServerWatcherThread == nullptr)
            {
                _pServerWatcherThread = new pdRemoteProcessDebuggerDebuggingServerWatcherThread;
                _pServerWatcherThread->execute();
            }

            // Monitor the debugging server:
            _pServerWatcherThread->monitorLocalMachineDebuggingServer(processInfo.dwProcessId);
        }
    }

#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::launchRemoteDebuggingServerOnRemoteMachine
// Description: Connects to the CodeXL daemon on the remote machine, and establishes
//              two connections with the target machine
// Author:      Uri Shomroni
// Date:        7/8/2013
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebugger::launchRemoteDebuggingServerOnRemoteMachine(const osPortAddress& daemonConnectionPort, gtUInt16 remoteDebuggingConnectionPortNumber, gtUInt16 remoteDebuggingEventsPortNumber)
{
    bool retVal = false;

    // This flag indicates whether the handshake result is match (true) or mismatch (false).
    bool isHandshakeMatch = false;

    // This flag indicates whether the handshake execution succeeded.
    bool isHandshakeSucesss = false;

    // This var will hold the handshake error message (if any).
    gtString handshakeErrMsg;

#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE))
    // This should not be called in the 64-bit windows version of this project!
    GT_ASSERT(false);
#else // !((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE))

    // Clean up any previous connections:
    CXLDaemonClient::Close();

    bool rcDae = CXLDaemonClient::Init(daemonConnectionPort, LONG_MAX);
    CXLDaemonClient* pClient = CXLDaemonClient::GetInstance();
    GT_IF_WITH_ASSERT(nullptr != pClient)
    {
        GT_IF_WITH_ASSERT(rcDae)
        {
            osPortAddress daemonConnectionPortBuffer;
            rcDae = pClient->ConnectToDaemon(daemonConnectionPortBuffer);

            GT_IF_WITH_ASSERT(rcDae)
            {

                // First, do the handshake:
                rcDae = isHandshakeSucesss = pClient->PerformHandshake(isHandshakeMatch, handshakeErrMsg);

                GT_IF_WITH_ASSERT(rcDae)
                {
                    if (isHandshakeMatch)
                    {
                        // If successful, open two TCP servers for the RDS to connect to:
                        osTCPSocketServer tcpConnectionServer;
                        bool rcCon = tcpConnectionServer.open();
                        GT_IF_WITH_ASSERT(rcCon)
                        {
                            osPortAddress remoteDebuggingServerConnectionPort(remoteDebuggingConnectionPortNumber, false);
                            rcCon = tcpConnectionServer.bind(remoteDebuggingServerConnectionPort);
                            GT_IF_WITH_ASSERT(rcCon)
                            {
                                rcCon = tcpConnectionServer.listen(1);
                                GT_ASSERT(rcCon);
                            }
                        }
                        osTCPSocketServer tcpEventsServer;
                        bool rcEve = tcpEventsServer.open();
                        GT_IF_WITH_ASSERT(rcEve)
                        {
                            osPortAddress remoteDebuggingServerEventsPort(remoteDebuggingEventsPortNumber, false);
                            rcEve = tcpEventsServer.bind(remoteDebuggingServerEventsPort);
                            GT_IF_WITH_ASSERT(rcEve)
                            {
                                rcEve = tcpEventsServer.listen(1);
                                GT_ASSERT(rcEve);
                            }
                        }

                        // If both succeeded:
                        if (rcCon && rcEve)
                        {
                            // Wait for a connection on both channels:
                            osTCPSocketServerConnectionHandler* pTCPConnection = new osTCPSocketServerConnectionHandler;
                            pdRemoteProcessDebuggerTCPIPConnectionWaiterThread connectionWaiterThread(*this, tcpConnectionServer, *pTCPConnection);
                            connectionWaiterThread.execute();
                            osTCPSocketServerConnectionHandler* pTCPEventsConnection = new osTCPSocketServerConnectionHandler;
                            pdRemoteProcessDebuggerTCPIPConnectionWaiterThread eventConnectionWaiterThread(*this, tcpEventsServer, *pTCPEventsConnection);
                            eventConnectionWaiterThread.execute();

                            // Prepare the environment variables that the remote daemon should set for the RDS.
                            osEnvironmentVariable conTypeVar(RD_STR_DebuggingModeEnvVar, RD_STR_DebuggingModeTCPIPConnection);
                            osEnvironmentVariable conVar;
                            conVar._name = RD_STR_TCPIPConnectionPortEnvVar;
                            conVar._value = daemonConnectionPortBuffer.hostName();
                            conVar._value.appendFormattedString(L":%u", remoteDebuggingConnectionPortNumber);
                            osEnvironmentVariable eveVar;
                            eveVar._name = RD_STR_EventsTCPIPConnectionPortEnvVar;
                            eveVar._value = daemonConnectionPortBuffer.hostName();
                            eveVar._value.appendFormattedString(L":%u", remoteDebuggingEventsPortNumber);
                            osEnvironmentVariable logLvlVar;
                            logLvlVar._name = RD_STR_DebugLogSeverityEnvVar;
                            logLvlVar._value = osDebugLogSeverityToString(osDebugLog::instance().loggedSeverity());

                            std::vector<osEnvironmentVariable> envVars;
                            envVars.push_back(conTypeVar);
                            envVars.push_back(conVar);
                            envVars.push_back(eveVar);
                            envVars.push_back(logLvlVar);

                            rcDae = pClient->LaunchRDS(L"", envVars);
                            GT_IF_WITH_ASSERT(rcDae)
                            {
                                // Save the pointer to the remote client.
                                m_pDaemonClient = pClient;

                                // Wait until both connections are established:
                                rcCon = rcCon && rcEve && connectionWaiterThread.waitForConnection(PD_REMOTE_DEBUGGING_SERVER_TCP_IP_CONNECTION_WAIT_TIMEOUT_MS) && connectionWaiterThread.wasConnectionSuccessful();
                                rcEve = rcCon && rcEve && eventConnectionWaiterThread.waitForConnection(PD_REMOTE_DEBUGGING_SERVER_TCP_IP_CONNECTION_WAIT_TIMEOUT_MS) && eventConnectionWaiterThread.wasConnectionSuccessful();
                                GT_IF_WITH_ASSERT(rcCon && rcEve)
                                {
                                    _pRemoteDebuggingServerAPIChannel = pTCPConnection;
                                    _pRemoteDebuggingServerAPIChannel->setReadOperationTimeOut(PD_REMOTE_DEBUGGING_SERVER_TCP_IP_TIMEOUT);
                                    _pRemoteDebuggingServerAPIChannel->setWriteOperationTimeOut(PD_REMOTE_DEBUGGING_SERVER_TCP_IP_TIMEOUT);
                                    _pRemoteDebuggingEventsAPIChannel = pTCPEventsConnection;
                                    _pRemoteDebuggingEventsAPIChannel->setReadOperationTimeOut(PD_REMOTE_DEBUGGING_SERVER_TCP_IP_TIMEOUT);
                                    _pRemoteDebuggingEventsAPIChannel->setWriteOperationTimeOut(PD_REMOTE_DEBUGGING_SERVER_TCP_IP_TIMEOUT);
                                    _connectionMethod = PD_REMOTE_DEBUGGING_TCP_IP_CONNECTION;
                                    m_daemonConnectionPort = daemonConnectionPort;
                                    retVal = true;
                                }
                                else
                                {
                                    // Close and delete the connections:
                                    pTCPConnection->close();
                                    delete pTCPConnection;
                                    pTCPEventsConnection->close();
                                    delete pTCPEventsConnection;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE))

    // If we failed to start debugging, notify the system.
    if (!retVal)
    {
        GT_IF_WITH_ASSERT(nullptr != pClient)
        {
            // Make sure that our session is closed.
            bool isTerminationSuccess = pClient->TerminateWholeSession();
            GT_ASSERT_EX(isTerminationSuccess, L"Unable to terminate remote session after faililng to start a remote debugging sessison properly.");
        }

        if (isHandshakeSucesss && !isHandshakeMatch)
        {
            // It is a handshake failure.
            apDebuggedProcessCreationFailureEvent processCreationFailedEvent(apDebuggedProcessCreationFailureEvent::REMOTE_HANDSHAKE_MISMATCH,
                                                                             L"", L"", handshakeErrMsg);
            apEventsHandler::instance().registerPendingDebugEvent(processCreationFailedEvent);
        }
        else
        {
            // It is some other failure.
            apDebuggedProcessCreationFailureEvent processCreationFailedEvent(apDebuggedProcessCreationFailureEvent::COULD_NOT_CREATE_PROCESS,
                                                                             L"", L"", L"");
            apEventsHandler::instance().registerPendingDebugEvent(processCreationFailedEvent);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::closeSharedMemoryObjectConnections
// Description: Closes the shared memory object connections:
// Author:      Uri Shomroni
// Date:        12/8/2009
// ---------------------------------------------------------------------------
void pdRemoteProcessDebugger::closeSharedMemoryObjectConnections()
{
    osPipeSocketServer* pServerConnection = (osPipeSocketServer*)_pRemoteDebuggingServerAPIChannel;
    osPipeSocketServer* pEventsConnection = (osPipeSocketServer*)_pRemoteDebuggingEventsAPIChannel;

    // Close and destroy the server connection:
    if (pServerConnection != nullptr)
    {
        pServerConnection->close();
        delete _pRemoteDebuggingServerAPIChannel;
        _pRemoteDebuggingServerAPIChannel = nullptr;
    }

    // Close and destroy the events connection:
    if (pEventsConnection != nullptr)
    {
        pEventsConnection->close();
        delete _pRemoteDebuggingEventsAPIChannel;
        _pRemoteDebuggingEventsAPIChannel = nullptr;
    }

    // Mark that we have disconnected:
    _connectionMethod = PD_REMOTE_DEBUGGING_SERVER_NOT_CONNECTED;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::closeTCPIPConnections
// Description: Closes the TCP/IP connections
// Author:      Uri Shomroni
// Date:        29/8/2013
// ---------------------------------------------------------------------------
void pdRemoteProcessDebugger::closeTCPIPConnections()
{
    osTCPSocketServerConnectionHandler* pServerConnection = (osTCPSocketServerConnectionHandler*)_pRemoteDebuggingServerAPIChannel;
    osTCPSocketServerConnectionHandler* pEventsConnection = (osTCPSocketServerConnectionHandler*)_pRemoteDebuggingEventsAPIChannel;

    // Close and destroy the server connection:
    if (pServerConnection != nullptr)
    {
        pServerConnection->close();
        delete _pRemoteDebuggingServerAPIChannel;
        _pRemoteDebuggingServerAPIChannel = nullptr;
    }

    // Close and destroy the events connection:
    if (pEventsConnection != nullptr)
    {
        StopEventListener();
        pEventsConnection->close();
        delete _pRemoteDebuggingEventsAPIChannel;
        _pRemoteDebuggingEventsAPIChannel = nullptr;
    }

    // Close the daemon connection if it was open:
    cleanupDaemonConnection();

    // Mark that we have disconnected:
    _connectionMethod = PD_REMOTE_DEBUGGING_SERVER_NOT_CONNECTED;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::cleanupDaemonConnection
// Description: Cleans up the daemon connection
// Author:      Uri Shomroni
// Date:        7/10/2013
// ---------------------------------------------------------------------------
void pdRemoteProcessDebugger::cleanupDaemonConnection()
{
    // Clean up any remote daemon connection:
    if (m_pDaemonClient != nullptr)
    {
        // Close the connection:
        GT_ASSERT(m_pDaemonClient->IsInitialized(m_daemonConnectionPort));
        bool rcTerm = m_pDaemonClient->TerminateWholeSession();
        GT_ASSERT(rcTerm);

        // Do not delete the daemon client, as it is a singleton:
        // delete m_pDaemonClient;
        m_pDaemonClient = nullptr;

        // Close the daemon itself:
        CXLDaemonClient::Close();
    }
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebugger::initialize
// Description: Close all open connections and reset the process debugger
// Author:      Uri Shomroni
// Date:        12/8/2009
// ---------------------------------------------------------------------------
void pdRemoteProcessDebugger::initialize()
{
    switch (_connectionMethod)
    {
        case PD_REMOTE_DEBUGGING_SERVER_NOT_CONNECTED:
        {
            // We are not connected, do nothing:
        }
        break;

        case PD_REMOTE_DEBUGGING_SHARED_MEMORY_OBJECT:
        {
            // Close the shared memory object connection:
            closeSharedMemoryObjectConnections();
        }
        break;

        case PD_REMOTE_DEBUGGING_TCP_IP_CONNECTION:
        {
            // Close the shared memory object connection:
            closeTCPIPConnections();
        }
        break;

        default:
        {
            // Unknown connection method
            GT_ASSERT(false);
        }
        break;
    }

    delete _pDebuggedProcessCreationData;
    _pDebuggedProcessCreationData = nullptr;

    delete _pRemoteDebuggedProcessCreationData;
    _pRemoteDebuggedProcessCreationData = nullptr;

    // If the server watcher thread is active, kill the debugging server:
    GT_IF_WITH_ASSERT(_pServerWatcherThread != nullptr)
    {
        _pServerWatcherThread->stopMonitoringDebuggingServer();
    }

    delete m_pLocalLogFilePath;
    m_pLocalLogFilePath = nullptr;

    m_remoteToLocalFilePathCache.clear();

    _debuggedProcessExists = false;
    _debuggedProcessSuspended = false;
    _isDebugging64BitApplication = false;
    m_isSpiesAPIThreadRunning = false;

    _connectionMethod = PD_REMOTE_DEBUGGING_SERVER_NOT_CONNECTED;
}

bool pdRemoteProcessDebugger::canGetHostVariables() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger can get host variables function", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_REMOTE_CAN_GET_HOST_VARIABLES_CMD;

        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    if (retVal)
    {
        OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger can get host variables function exit with result true", OS_DEBUG_LOG_EXTENSIVE);
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger can get host variables function exit with result false", OS_DEBUG_LOG_EXTENSIVE);
    }

    return retVal;
}

bool pdRemoteProcessDebugger::getHostLocals(osThreadId threadId, int callStackFrameIndex, int evaluationDepth, bool onlyNames, gtVector<apExpression>& o_locals)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get host locals function", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_REMOTE_GET_HOST_LOCALS_CMD;
        *_pRemoteDebuggingServerAPIChannel << (gtUInt64)threadId;
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)callStackFrameIndex;
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)evaluationDepth;
        *_pRemoteDebuggingServerAPIChannel << onlyNames;

        *_pRemoteDebuggingServerAPIChannel >> retVal;

        if (retVal)
        {
            gtInt32 outputVectorSize = 0;
            *_pRemoteDebuggingServerAPIChannel >> outputVectorSize;

            o_locals.resize(outputVectorSize);

            for (gtInt32 i = 0; i < outputVectorSize; i++)
            {
                bool rcVar = o_locals[i].readSelfFromChannel(*_pRemoteDebuggingServerAPIChannel);
                GT_ASSERT(rcVar);
                retVal = retVal && rcVar;
            }
        }
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get host locals function exit", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

bool pdRemoteProcessDebugger::getHostExpressionValue(osThreadId threadId, int callStackFrameIndex, const gtString& expressionText, int evaluationDepth, apExpression& o_exp)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get host expression value function", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_REMOTE_GET_HOST_EXPRESSION_VALUE_CMD;
        *_pRemoteDebuggingServerAPIChannel << (gtUInt64)threadId;
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)callStackFrameIndex;
        *_pRemoteDebuggingServerAPIChannel << expressionText;
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)evaluationDepth;

        *_pRemoteDebuggingServerAPIChannel >> retVal;

        if (retVal)
        {
            retVal = o_exp.readSelfFromChannel(*_pRemoteDebuggingServerAPIChannel);
        }
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get host expression value exit", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

bool pdRemoteProcessDebugger::canPerformHostDebugging() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger can perform host debugging function", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_REMOTE_CAN_PERFORM_HOST_DEBUGGING;

        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    if (retVal)
    {
        OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger can perform host debugging function exit with retVal true", OS_DEBUG_LOG_EXTENSIVE);
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger can perform host debugging function exit with retVal false", OS_DEBUG_LOG_EXTENSIVE);
    }

    return retVal;
}

bool pdRemoteProcessDebugger::isAtAPIOrKernelBreakpoint(osThreadId threadId) const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger is API or kernel breakpoint function", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    if (_debuggedProcessExists)
    {
        if (((osSocket*)_pRemoteDebuggingServerAPIChannel)->isOpen())
        {
            GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
            {
                *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_REMOTE_IS_API_OR_KERNEL_BP_CMD;
                *_pRemoteDebuggingServerAPIChannel << (gtUInt64)threadId;

                *_pRemoteDebuggingServerAPIChannel >> retVal;
            }
        }
        else
        {
            apDebuggedProcessTerminatedEvent eve(0);

            apEventsHandler::instance().registerPendingDebugEvent(eve);
        }

        if (retVal)
        {
            OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger is API or kernel breakpoint function exit with retVal true", OS_DEBUG_LOG_EXTENSIVE);
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger is API or kernel breakpoint function exit with retVal false", OS_DEBUG_LOG_EXTENSIVE);
        }
    }

    return retVal;
}

apBreakReason pdRemoteProcessDebugger::hostBreakReason() const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger host breakpoint reason function", OS_DEBUG_LOG_EXTENSIVE);
    apBreakReason retVal = apBreakReason::AP_FOREIGN_BREAK_HIT;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_REMOTE_HOST_BREAK_REASON_CMD;

        gtInt32 breakReasonAsInt32 = -1;
        *_pRemoteDebuggingServerAPIChannel >> breakReasonAsInt32;
        retVal = (apBreakReason)breakReasonAsInt32;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger host breakpoint reason exit", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

bool pdRemoteProcessDebugger::getHostBreakpointLocation(osFilePath& bpFile, int& bpLine) const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger host breakpoint location function", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_REMOTE_HOST_BREAKPOINT_LOCATION_CMD;

        *_pRemoteDebuggingServerAPIChannel >> retVal;

        if (retVal)
        {
            bpFile.readSelfFromChannel(*_pRemoteDebuggingServerAPIChannel);

            gtInt32 lineNumAsInt32 = -1;
            *_pRemoteDebuggingServerAPIChannel >> lineNumAsInt32;
            bpLine = (int)lineNumAsInt32;
        }
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger host breakpoint location exit", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

bool pdRemoteProcessDebugger::setHostSourceBreakpoint(const osFilePath& fileName, int lineNumber)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger cset host source breakpoint function", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_REMOTE_SET_HOST_BP_CMD;
        fileName.writeSelfIntoChannel(*_pRemoteDebuggingServerAPIChannel);
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)lineNumber;

        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger cset host source breakpoint function exit", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

bool pdRemoteProcessDebugger::deleteHostSourceBreakpoint(const osFilePath& fileName, int lineNumber)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger cset host source breakpoint function", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_REMOTE_DELETE_HOST_BP_CMD;
        fileName.writeSelfIntoChannel(*_pRemoteDebuggingServerAPIChannel);
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)lineNumber;

        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger cset host source breakpoint function exit", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

bool pdRemoteProcessDebugger::setHostFunctionBreakpoint(const gtString& funcName)
{
    (void)funcName;
    return false;
}

bool pdRemoteProcessDebugger::performHostStep(osThreadId threadId, StepType stepType)
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger cset host source breakpoint function", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_REMOTE_PERFORM_HOST_STEP_CMD;
        *_pRemoteDebuggingServerAPIChannel << (gtUInt64)threadId;
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)stepType;

        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger cset host source breakpoint function exit", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

bool pdRemoteProcessDebugger::suspendHostDebuggedProcess()
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger suspend host debugged process function", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_REMOTE_SUSPEND_HOST_DEBUGGED_PROCESS;

        *_pRemoteDebuggingServerAPIChannel >> retVal;
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger suspend host debugged process function exit", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

bool pdRemoteProcessDebugger::getBreakpointTriggeringThreadIndex(int& index) const
{
    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get host BP triggering index function", OS_DEBUG_LOG_EXTENSIVE);
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRemoteDebuggingServerAPIChannel != nullptr)
    {
        *_pRemoteDebuggingServerAPIChannel << (gtInt32)PD_REMOTE_GET_BP_TRIGGERING_THREAD_INDEX_CMD;

        *_pRemoteDebuggingServerAPIChannel >> retVal;

        if (retVal)
        {
            gtInt32 indexAsInt32 = -1;
            *_pRemoteDebuggingServerAPIChannel >> indexAsInt32;
            index = (int)indexAsInt32;
        }
    }

    OS_OUTPUT_DEBUG_LOG(L"pdRemoteProcessDebugger get host BP triggering index function exit", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

