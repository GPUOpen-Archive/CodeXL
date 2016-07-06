//------------------------------ ServerListenThread.cpp ------------------------------
#include <iostream>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osNetworkAdapter.h>
#include "ServerListenThread.h"
#include "ServerWorkerThread.h"

ServerListenThread::ServerListenThread()
    : osThread(L"CommDebugServerListenThread", true)
    , m_bContinueRunning(true)
{
    m_serverSocket.excludeFromCommunicationDebug(true);
}

bool ServerListenThread::init(int portNum)
{
    const bool useHostName = false;
    osPortAddress serverAddress(static_cast<unsigned short>(portNum), useHostName);

    // Open the server socket
    bool retVal = m_serverSocket.open();

    GT_IF_WITH_ASSERT(true == retVal)
    {
        // Bind the socket to a local IP address
        retVal = m_serverSocket.bind(serverAddress);
    }

    GT_IF_WITH_ASSERT(true == retVal)
    {
        // Put the socket into listening mode
        retVal = m_serverSocket.listen(0);

        gtString serverAddressString;
        serverAddress.toString(serverAddressString);

        std::wcout << L"Server listening on " << serverAddressString.asCharArray() << std::endl;
    }

    return retVal;
}

int ServerListenThread::entryPoint()
{
    bool retVal = false;
    osTCPSocketServerConnectionHandler* pClientCallHandler = nullptr;


    while (m_bContinueRunning)
    {
        pClientCallHandler = new osTCPSocketServerConnectionHandler;
        pClientCallHandler->excludeFromCommunicationDebug(true);

        // wait for new client connections
        retVal = m_serverSocket.accept(*pClientCallHandler);

        if (true == retVal)
        {
            ServerWorkerThread* pWorkerThread = new ServerWorkerThread(pClientCallHandler);
            m_spawnedThreads.push_back(pWorkerThread);
            pWorkerThread->execute();
        }

    }

    retVal = m_serverSocket.close();

    return 0;
}

void ServerListenThread::beforeTermination()
{
    m_bContinueRunning = false;
    m_serverSocket.close();

    // close all spawned threads
    for (auto it : m_spawnedThreads)
    {
        it->terminate();
    }
}

ServerListenThread::~ServerListenThread()
{
    // free all allocated thread objects
    for (auto it : m_spawnedThreads)
    {
        ServerWorkerThread* pThread = it;
        delete pThread;
    }
    m_spawnedThreads.clear();
}