//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnServerThread.cpp
///
//==================================================================================

#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#include <AMDTRemoteAgent/dmnServerThread.h>
#include <AMDTRemoteAgent/dmnSessionThread.h>
#include <AMDTRemoteAgent/dmnUtils.h>
#include <AMDTRemoteAgent/Public Include/dmnStringConstants.h>

#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>


#define DMN_GENERIC_ERROR_MESSAGE L"Please check that the given port is not blocked by a firewall, and not\nbeing used by another process (including another instance of CodeXLRemoteAgent)."
#define DMN_BEFORE_EXIT_ERROR_MESSAGE L"CodeXL Remote Agent will now exit."
const int LOOP_SLEEP_INTERVAL_MS = 50;

// Outputs CodeXL Daemon's extracted settings to the user (console).
static void OutputDaemonSettingsToConsole(bool isSuccessful, const gtString& ipString)
{
    std::wstring rdTimeout;
    std::wstring wtTimeout;

    const dmnConfigManager* configManager = dmnConfigManager::Instance();
    GT_IF_WITH_ASSERT(configManager != NULL)
    {
        bool isOk = configManager->TimeoutToString(configManager->GetReadTimeout(), rdTimeout);
        GT_ASSERT_EX(isOk, L"DMN: RD TIMEOUT CONVERSION TO STRING.");

        isOk = configManager->TimeoutToString(configManager->GetWriteTimeout(), wtTimeout);
        GT_ASSERT_EX(isOk, L"DMN: WT TIMEOUT CONVERSION TO STRING.");

        // Extract the CodeXL version.
        osProductVersion cxlProductVersion;
        osGetApplicationVersion(cxlProductVersion);
        gtString cxlVersionAsString = cxlProductVersion.toString();

        std::wcout << DMN_STR_HEADER_LINE << std::endl;
        std::wcout << DMN_STR_SETTINGS_EXTRACTED_OK << std::endl;
        std::wcout << DMN_STR_SEPARATOR << std::endl;
        std::wcout << DMN_STR_AGENT_VERSION << cxlVersionAsString.asCharArray() << std::endl;
        std::wcout << DMN_STR_AGENT_READ_TIMEOUT << rdTimeout << std::endl;
        std::wcout << DMN_STR_AGENT_WRITE_TIMEOUT << wtTimeout << std::endl;
        std::wcout << DMN_STR_AGENT_PORT << ipString.asCharArray() << std::endl;
        std::wcout << DMN_STR_END_LINE << std::endl << std::endl;

        // Update the user regarding the agent's address.
        if (isSuccessful)
        {
            std::wcout << DMN_STR_AGENT_LISTENING_ON << L"<" <<
                  ipString.asCharArray() << L">" << std::endl << std::endl;
        }
        else
        {
            std::wcout << DMN_STR_ERR_CONNECTION << std::endl;
        }

        std::wcout << DMN_STR_PRESS_ANY_KEY << std::endl << std::endl;

        // On Windows, change the console title.
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        std::wstringstream conTitle;
        conTitle << DMN_STR_AGENT_LISTENING_ON << ipString.asCharArray();
        SetConsoleTitle(conTitle.str().c_str());
#endif
    }
}

// Outputs CodeXL Daemon's extracted settings to the log.
static void OutputDaemonSettingsToLog()
{
    std::wstring rdTimeout;
    std::wstring wtTimeout;

    const dmnConfigManager* configManager = dmnConfigManager::Instance();
    GT_IF_WITH_ASSERT(configManager != NULL)
    {
        bool isOk = configManager->TimeoutToString(configManager->GetReadTimeout(), rdTimeout);
        GT_ASSERT_EX(isOk, L"DMN: RD TIMEOUT CONVERSION TO STRING.");

        isOk = configManager->TimeoutToString(configManager->GetWriteTimeout(), wtTimeout);
        GT_ASSERT_EX(isOk, L"DMN: WT TIMEOUT CONVERSION TO STRING.");

        std::wstringstream stream;
        stream << L"CodeXL Remote Agent settings: Port Number:'" << configManager->GetPortNumber() << L"', " <<
               L"RD Timeout: '" << rdTimeout << L"', WT Timeout: '" << wtTimeout << L"'";

        dmnUtils::LogMessage(stream.str(), OS_DEBUG_LOG_DEBUG);
    }
}


// This routine prints the error message to std::cout and to the log,
// and then terminates the agent.
static void ExitOnCriticalNetworkError(const gtString& agentAddress)
{
    std::wcout << L"CodeXL Remote Agent failed to listen on " << agentAddress.asCharArray() << std::endl;
    std::wcout << DMN_GENERIC_ERROR_MESSAGE << std::endl << std::endl;
    std::wcout << DMN_BEFORE_EXIT_ERROR_MESSAGE << std::endl << std::endl;

    dmnUtils::LogMessage(L"DMN: Fatal network error occurred. Aborting.", OS_DEBUG_LOG_ERROR);

    // Terminate.
    exit(-1);
}

dmnServerThread::dmnServerThread(dmnServerThreadConfig* pConfig) : osThread(L"DaemonServerThread"), m_pConfig(pConfig),
    m_sessionThreadsCount(0), m_threadCreationObservers(), m_tcpServer()
{
    GT_ASSERT(m_pConfig != NULL);
}


dmnServerThread::~dmnServerThread(void)
{
    bool isOk = m_tcpServer.close();

    if (!isOk)
    {
        dmnUtils::LogMessage(L"DMN Server Thread: Unable to close tcp server.", OS_DEBUG_LOG_ERROR);
    }

    if (m_pConfig != NULL)
    {
        delete m_pConfig;
    }

    dmnSessionThread::KillDependantProcesses();
}

// Helper function.
void notifyObservers(gtList<IThreadEventObserver*>& observerList, osThread* pCreatedThread)
{
    //std::for_each(observerList.begin(), observerList.end(), [&pCreatedThread](IThreadEventObserver* pObserver)
    gtList<IThreadEventObserver*>::iterator iter;

    for (iter = observerList.begin(); iter != observerList.end(); iter++)
    {
        if ((*iter) != NULL)
        {
            (*iter)->onThreadCreation(pCreatedThread);
        }
    }
}

static void setTimeouts(const dmnServerThreadConfig* configObject, osTCPSocketServerConnectionHandler* connHandler)
{
    // Check if WT Timeout was set.
    if (configObject->isWriteTimeout())
    {
        // If so, forward it to our new connection.
        connHandler->setWriteOperationTimeOut(configObject->getWriteTimeout());
    }

    // Check if RD Timeout was set.
    if (configObject->isReadTimeout())
    {
        // If so, forward it to our new connection.
        connHandler->setReadOperationTimeOut(configObject->getReadTimeout());
    }
}


int dmnServerThread::entryPoint()
{
    if (m_pConfig == NULL)
    {
        std::wstringstream stream;
        stream << L"DMN Server Thread: Invalid configuration manager, aborting.";
        dmnUtils::LogMessage(stream.str(), OS_DEBUG_LOG_ERROR);
        std::wcout << L"Unable to extract configuration. Aborting." << std::endl;
        return -1;
    }

    // Define the daemon's port.
    const unsigned int dmnPortNumber =  m_pConfig->getDaemonPortNumber();
    osPortAddress portAddr(static_cast<unsigned short>(dmnPortNumber), false);

    if (m_pConfig->isUserForcedIp())
    {
        portAddr.setAsRemotePortAddress(m_pConfig->getUserForcedIpString(), static_cast<unsigned short>(dmnPortNumber));
    }

    // This holds the string representation of the ip:port to which the daemon thread is listening.
    gtString agentAddress;
    portAddr.toString(agentAddress);

    bool isOk = m_tcpServer.open();

    if (!isOk)
    {
        std::wstringstream stream;
        stream << L"DMN Server Thread: unable to open tcpServer on port " << m_pConfig->getDaemonPortNumber();
        dmnUtils::LogMessage(stream.str(), OS_DEBUG_LOG_ERROR);
        ExitOnCriticalNetworkError(agentAddress);
    }


    isOk = m_tcpServer.bind(portAddr);

    if (!isOk)
    {
        std::wstringstream stream;
        stream << L"DMN Server Thread: unable to bind to port " << m_pConfig->getDaemonPortNumber();
        dmnUtils::LogMessage(stream.str(), OS_DEBUG_LOG_ERROR);
        ExitOnCriticalNetworkError(agentAddress);
    }

    // Listen on the port for incoming connections.
    isOk = m_tcpServer.listen(m_pConfig->getBacklog());

    if (!isOk)
    {
        std::wstringstream stream;
        stream << L"DMN Server Thread: unable to listen to port " << m_pConfig->getDaemonPortNumber();
        dmnUtils::LogMessage(stream.str(), OS_DEBUG_LOG_ERROR);
        ExitOnCriticalNetworkError(agentAddress);
    }

    // Output the settings.
    OutputDaemonSettingsToConsole(isOk, agentAddress);
    OutputDaemonSettingsToLog();

    // Wait for incoming connections.
    for (; ;)
    {
        // Mitigate the busy waiting period.
        osSleep(LOOP_SLEEP_INTERVAL_MS);

        // Create a new connection object.
        osTCPSocketServerConnectionHandler* pConnHandler =
            new(std::nothrow) osTCPSocketServerConnectionHandler();

        // Set the timeouts.
        setTimeouts(m_pConfig, pConnHandler);

        // Wait for incoming connections.
        isOk = m_tcpServer.accept(*pConnHandler);

        if (!isOk)
        {
            std::wstringstream stream;
            stream << L"DMN Server Thread: unable to accept on port " << m_pConfig->getDaemonPortNumber();
            dmnUtils::LogMessage(stream.str(), OS_DEBUG_LOG_ERROR);
            ExitOnCriticalNetworkError(agentAddress);
        }
        else
        {
            dmnUtils::LogMessage(L"DMN Server Thread: received an incoming connection.", OS_DEBUG_LOG_DEBUG);
            osPortAddress peerAddr;
            isOk = pConnHandler->getPeerHostAddress(peerAddr);
            GT_ASSERT_EX(isOk, L"DMN Session thread: failed to extract peer's address while receiving connection.");
            std::wstringstream msgStream;
            msgStream << std::endl << L"*** Connection: Received a connection";

            if (isOk)
            {
                gtString peerAddrStr;
                peerAddr.toString(peerAddrStr);
                msgStream << L" from <" << peerAddrStr.asCharArray() << L"> ***" << std::endl;
            }
            else
            {
                msgStream << L" ***" << std::endl;
            }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            // Add an extra empty line on Windows.
            msgStream << std::endl;
#endif

            std::wcout << msgStream.str();
        }

        // Create a new Session Thread to handle the session.
        std::wstringstream thNameStream;
        thNameStream << "Session Thread " << (++m_sessionThreadsCount);
        dmnSessionThread* pSessionTh = new(std::nothrow) dmnSessionThread(pConnHandler, thNameStream.str().c_str());

        // Start session.
        isOk = pSessionTh->execute();
        GT_ASSERT(isOk);

        if (isOk)
        {
            // Notify whoever registered.
            notifyObservers(m_threadCreationObservers, pSessionTh);
        }
    }

    return 0;
}

void dmnServerThread::registerToThreadCreationEvent(IThreadEventObserver* pObserver)
{
    m_threadCreationObservers.push_back(pObserver);
}

