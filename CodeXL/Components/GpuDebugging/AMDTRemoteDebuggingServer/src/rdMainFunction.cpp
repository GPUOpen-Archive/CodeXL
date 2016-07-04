//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file rdMainFunction.cpp
///
//==================================================================================

//------------------------------ rdMainFunction.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osPipeSocketClient.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osTCPSocketClient.h>
#include <AMDTOSWrappers/Include/osOutOfMemoryHandling.h>
#include <AMDTAPIClasses/Include/apiClassesInitFunc.h>

// Local:
#include <AMDTRemoteDebuggingServer/Include/rdStringConstants.h>
#include <src/rdDebuggerCommandExecutor.h>
#include <src/rdEventHandler.h>
#include <src/rdEventsHandlingThread.h>

// Constants for the connection:
// Uri, 29/8/13 - set an infinite timeout for now, as our controlling process (daemon / client)
// can kill us if something fails.
#define RD_TCP_IP_CONNECTION_DEFAULT_TIMEOUT OS_CHANNEL_INFINITE_TIME_OUT

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
// ---------------------------------------------------------------------------
// Name:        ConsoleClosingHandler
// Description: A handler routine that handles the various signals that a Windows console might receive from the OS.
//              Note that on Windows event handlers are being called in a LIFO order (last registered first called).
//              So, in case that another process had created us (which is always the case with RDS), this event
//              handler is to be called before our parent process' corresponding event handler.
//              Therefore, we return FALSE to allow our parent to handle the event if necessary.
//              What we do here is terminating our child processes (if any). The debugger will
//              be notified of the debugged application's exit, and the remote debugging session
//              will end in a clean manner (as our client will be notified about the session's end).
// Author:      Amit Ben-Moshe
// Date:        20/10/2013
// ---------------------------------------------------------------------------
BOOL WINAPI ConsoleClosingHandler(DWORD dwCtrlType)
{
    GT_UNREFERENCED_PARAMETER(dwCtrlType);

    osProcessId myProcId = osGetCurrentProcessId();

    if (myProcId > 0)
    {
        osTerminateChildren(myProcId);
    }

    // Return false, for parent processes (if any) to be able to handle these events.
    return FALSE;
}

#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS


// ---------------------------------------------------------------------------
// Name:        main
// Description: The Remote Debugging Server's main function.
// Return Val: 0 - Success
//            -1 - Failure
// Author:      Uri Shomroni
// Date:        10/8/2009
// ---------------------------------------------------------------------------
int main(int argc, char** argv)
{
    // Unused command line arguments:
    (void)(argc);
    (void)(argv);

    int retVal = -1;

    // First, register the out-of-memory event handler.
    std::set_new_handler(osDumpCallStackAndExit);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // If we are on Windows, set the console control event handler.
    ::SetConsoleCtrlHandler(ConsoleClosingHandler, TRUE);
#endif

    // Set the thread naming prefix:
    osThread::setThreadNamingPrefix("CodeXL Remote Debugging Server");

    // Initialize the debug log:
    osDebugLog& theDebugLog = osDebugLog::instance();
    theDebugLog.initialize(RD_STR_LogFileName, RD_STR_logFileProductDescription);

    // Set the log's severity:
    gtString debugLogSeverityAsString;
    osGetCurrentProcessEnvVariableValue(RD_STR_DebugLogSeverityEnvVar, debugLogSeverityAsString);
    osDebugLogSeverity debugLogSeverity = osStringToDebugLogSeverity(debugLogSeverityAsString.asCharArray());
    theDebugLog.setLoggedSeverity(debugLogSeverity);

    // Initialize the API classes:
    apiClassesInitFunc();

    // Get the environment variable string which tells us what is the way to connect to the process debugger:
    gtString debuggingModeString;
    bool rcEnv = osGetCurrentProcessEnvVariableValue(RD_STR_DebuggingModeEnvVar, debuggingModeString);
    GT_ASSERT(rcEnv);

    osChannel* pProcessDebuggerConnectionChannel = NULL;
    osChannel* pEventsHandlerConnectionChannel = NULL;

    // Create the pipe according to the type specified by the env. variable:
    if (debuggingModeString == RD_STR_DebuggingModeSharedMemoryObject)
    {
        // We are debugging through a shared memory object (on the local machine), get its path:
        gtString sharedMemoryObjectPath;
        rcEnv = osGetCurrentProcessEnvVariableValue(RD_STR_SharedMemoryObjectNameEnvVar, sharedMemoryObjectPath);
        gtString eventsSharedMemoryObjectPath;
        rcEnv = osGetCurrentProcessEnvVariableValue(RD_STR_EventsSharedMemoryObjectNameEnvVar, eventsSharedMemoryObjectPath) && rcEnv;

        GT_IF_WITH_ASSERT(rcEnv)
        {
            // Open the shared memory objects as channels to communicate with the process debugger:
            osPipeSocketClient* pSharedMemObjectPipeSocketClient = new osPipeSocketClient(sharedMemoryObjectPath, L"SharedMemorySocketClient");

            osPipeSocketClient* pEventsSharedMemObjectPipeSocketClient = new osPipeSocketClient(eventsSharedMemoryObjectPath, L"EventSharedMemorySocketClient");

            GT_IF_WITH_ASSERT((pSharedMemObjectPipeSocketClient != NULL) && (pEventsSharedMemObjectPipeSocketClient != NULL))
            {
                // Open the shared memory objects as pipes:
                bool rcOpen = pSharedMemObjectPipeSocketClient->open();
                rcOpen = pEventsSharedMemObjectPipeSocketClient->open() && rcOpen;

                // If we opened the pipe successfully
                GT_IF_WITH_ASSERT(rcOpen)
                {
                    // Set the pipes' timeouts to be infinite:
                    pSharedMemObjectPipeSocketClient->setReadOperationTimeOut(OS_CHANNEL_INFINITE_TIME_OUT);
                    pSharedMemObjectPipeSocketClient->setWriteOperationTimeOut(OS_CHANNEL_INFINITE_TIME_OUT);
                    pEventsSharedMemObjectPipeSocketClient->setReadOperationTimeOut(OS_CHANNEL_INFINITE_TIME_OUT);
                    pEventsSharedMemObjectPipeSocketClient->setWriteOperationTimeOut(OS_CHANNEL_INFINITE_TIME_OUT);

                    // Return it:
                    pProcessDebuggerConnectionChannel = pSharedMemObjectPipeSocketClient;
                    pEventsHandlerConnectionChannel = pEventsSharedMemObjectPipeSocketClient;
                }
            }
        }
    }
    else if (debuggingModeString == RD_STR_DebuggingModeTCPIPConnection)
    {
        // We are debugging through a TCP/IP connection (from a remote machine), get its ports:
        gtString tcpipConnectionPort;
        rcEnv = osGetCurrentProcessEnvVariableValue(RD_STR_TCPIPConnectionPortEnvVar, tcpipConnectionPort);
        gtString eventsTCPIPConnectionPort;
        rcEnv = osGetCurrentProcessEnvVariableValue(RD_STR_EventsTCPIPConnectionPortEnvVar, eventsTCPIPConnectionPort) && rcEnv;

        GT_IF_WITH_ASSERT(rcEnv)
        {
            // Make sure the values we got are valid strings:
            osPortAddress tcpipConnectionPortAddress;
            bool rcPort = osRemotePortAddressFromString(tcpipConnectionPort, tcpipConnectionPortAddress);

            osPortAddress eventsTCPIPConnectionPortAddress;
            rcPort = osRemotePortAddressFromString(eventsTCPIPConnectionPort, eventsTCPIPConnectionPortAddress) && rcPort;

            GT_IF_WITH_ASSERT(rcPort)
            {
                // Open the TCP/IP ports as channels to communicate with the process debugger:
                osTCPSocketClient* pTCPIPSocketClient = new osTCPSocketClient;

                osTCPSocketClient* pEventsTCPIPSocketClient = new osTCPSocketClient;

                GT_IF_WITH_ASSERT((NULL != pTCPIPSocketClient) && (NULL != pEventsTCPIPSocketClient))
                {
                    // Set the socket clients to force immediate resolution of DNSes:
                    pTCPIPSocketClient->setBlockOnDNS(true);
                    pEventsTCPIPSocketClient->setBlockOnDNS(true);

                    // Open the TCP connections:
                    bool rcOpen = pTCPIPSocketClient->open();
                    rcOpen = pEventsTCPIPSocketClient->open() && rcOpen;
                    GT_IF_WITH_ASSERT(rcOpen)
                    {
                        // Set the IP connection timeouts to be the default:
                        pTCPIPSocketClient->setReadOperationTimeOut(RD_TCP_IP_CONNECTION_DEFAULT_TIMEOUT);
                        pTCPIPSocketClient->setWriteOperationTimeOut(RD_TCP_IP_CONNECTION_DEFAULT_TIMEOUT);
                        pEventsTCPIPSocketClient->setReadOperationTimeOut(RD_TCP_IP_CONNECTION_DEFAULT_TIMEOUT);
                        pEventsTCPIPSocketClient->setWriteOperationTimeOut(RD_TCP_IP_CONNECTION_DEFAULT_TIMEOUT);

                        // Connect to the TCP ports:
                        bool rcConnect = pTCPIPSocketClient->connect(tcpipConnectionPortAddress);
                        rcConnect = pEventsTCPIPSocketClient->connect(eventsTCPIPConnectionPortAddress) && rcConnect;
                        GT_IF_WITH_ASSERT(rcConnect)
                        {
                            // Return it:
                            pProcessDebuggerConnectionChannel = pTCPIPSocketClient;
                            pEventsHandlerConnectionChannel = pEventsTCPIPSocketClient;
                        }
                    }
                }
            }
        }
    }
    else
    {
        GT_ASSERT_EX(false, L"Unknown remote debugging connection type");
    }

    // If we know what kind of channel we need and
    if ((pProcessDebuggerConnectionChannel != NULL) && (pEventsHandlerConnectionChannel != NULL))
    {
        // Create an events handler that sends events through the pipe:
        rdEventHandler eventsHandler(*pEventsHandlerConnectionChannel);

        // Create a thread to handle events (this is done to remove events handling from the debugger thread
        // so it won't delete itsself while processing them):
        rdEventsHandlingThread eventsHandlingThread(L"EventsHandlingThread");
        eventsHandlingThread.execute();

        // Create the command executor, which will run this server's read / execute loop:
        rdDebuggerCommandExecutor commandExecutor(*pProcessDebuggerConnectionChannel);
        commandExecutor.listenToDebuggingCommands();

        retVal = 0;
    }

    return retVal;
}
