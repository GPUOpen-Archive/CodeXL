//------------------------------ ServerWorkerThread.cpp ------------------------------

#include <iostream>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include "ServerWorkerThread.h"

osCriticalSection ServerWorkerThread::m_lockWriteToConsole;

ServerWorkerThread::ServerWorkerThread(osTCPSocketServerConnectionHandler* pClientCallHandler)
    : osThread(L"CommDebugServerWorkerThread", true)
    , m_bContinueRunning(true)
    , m_pClientCallHandler(pClientCallHandler)
{
    GT_ASSERT(pClientCallHandler != nullptr);
}

ServerWorkerThread::~ServerWorkerThread()
{
    if (m_pClientCallHandler != nullptr)
    {
        delete m_pClientCallHandler;
        m_pClientCallHandler = nullptr;
    }
}


int ServerWorkerThread::entryPoint()
{
    bool retVal = false;
    gtString peerAddressString;

    if (m_pClientCallHandler != nullptr)
    {
        osPortAddress peerAddress;
        retVal = m_pClientCallHandler->getPeerHostAddress(peerAddress);

        if (true == retVal)
        {
            osCriticalSectionLocker guard(m_lockWriteToConsole);
            peerAddress.toString(peerAddressString);
            std::wcout << L"Client " << peerAddressString.asCharArray() << L" connected" << std::endl;
        }

        while (m_bContinueRunning)
        {
            gtString debugString;

            // Wait for the client to send something
            retVal = m_pClientCallHandler->readString(debugString);

            if (m_pClientCallHandler->isOpen())
            {
                if (true == retVal && debugString.length() > 0)
                {
                    // Print the debug string in a thread-safe manner
                    osCriticalSectionLocker guard(m_lockWriteToConsole);
                    std::wcout << peerAddressString.asCharArray() << L"," << debugString.asCharArray() << std::endl;
                }
            }
            else
            {
                m_bContinueRunning = false;
                std::wcout << L"Client " << peerAddressString.asCharArray() << L" disconnected" << std::endl;
            }
        }

        retVal = m_pClientCallHandler->close();
    }

    return 0;
}

void ServerWorkerThread::beforeTermination()
{
    m_bContinueRunning = false;

    if (m_pClientCallHandler != nullptr)
    {
        m_pClientCallHandler->close();
    }
}