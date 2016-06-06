//------------------------------ osCommunicationDebugThread.cpp ------------------------------

#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCommunicationDebugThread.h>
#include <AMDTOSWrappers/Include/osCommunicationDebugManager.h>
#include <AMDTOSWrappers/Include/osDoubleBufferQueue.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>


// ---------------------------------------------------------------------------
// Name:        osCommunicationDebugThread::osCommunicationDebugThread
// Description: Default constructor.
// Author:      Doron Ofek
// Date:        Dec-20, 2015
// ---------------------------------------------------------------------------
osCommunicationDebugThread::osCommunicationDebugThread()
    : osThread(L"CommunicationDebugThread", true, true)
    , m_isContinueRunning(true)
    , m_commDebugDestinationType(COMM_DEBUG_DISABLED)
{
    m_debugLogFile.excludeFromCommunicationDebug(true);
    m_tcpSocket.excludeFromCommunicationDebug(true);
    m_tcpSocket.setSingleThreadAccess(false);
}

// ---------------------------------------------------------------------------
// Name:        osCommunicationDebugThread::entryPoint
// Description: Thread main function
// Author:      Doron Ofek
// Date:        Dec-20, 2015
// ---------------------------------------------------------------------------
int osCommunicationDebugThread::entryPoint()
{
    const gtString envVarCommunicationDebugDestination(L"AMDT_COMM_DEBUG_DESTINATION");
    osCommunicationDebugManager*& pManager = osCommunicationDebugManager::m_spCommunicationDebugManager;

    GT_ASSERT(pManager != nullptr);

    // The thread's main loop:
    // Continuously check the environment variable that enables/disables the
    // dump of debug data to the debug file/server
    while (m_isContinueRunning)
    {
        // Check the env variable
        bool isGetEnvVarOk = osGetCurrentProcessEnvVariableValue(envVarCommunicationDebugDestination, m_commDebugDestination);

        if (isGetEnvVarOk)
        {
            if (m_commDebugDestination.isEmpty())
            {
                if (pManager != nullptr)
                {
                    pManager->m_isCommunicationDebugEnabled = false;
                }

                m_commDebugDestinationType = COMM_DEBUG_DISABLED;
            }
            else
            {
                if (m_previousCommDebugDestination != m_commDebugDestination)
                {
                    // Need to setup the destination
                    wchar_t firstChar = m_commDebugDestination.asCharArray()[0];

                    if (firstChar >= L'0' && firstChar <= L'9')
                    {
                        // Destination starts with a digit so it is an IP address
                        osPortAddress serverAddress;
                        bool isValidAddress = serverAddress.fromString(m_commDebugDestination);

                        if (isValidAddress)
                        {
                            bool retVal = m_tcpSocket.open();

                            if (true == retVal)
                            {
                                // Attempt to connect to the server
                                retVal = m_tcpSocket.connect(serverAddress);
                            }

                            if (true == retVal)
                            {
                                m_commDebugDestinationType = COMM_DEBUG_SERVER;
                                pManager->m_isCommunicationDebugEnabled = true;
                            }
                        }
                        else
                        {
                            m_commDebugDestinationType = COMM_DEBUG_DISABLED;
                            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Failed to create an ip address from communication debug destination: %s", m_commDebugDestination.asCharArray());
                        }
                    }
                    else
                    {
                        // Destination is a file
                        m_commDebugDestinationType = COMM_DEBUG_FILE;
                        osFilePath logFilePath(m_commDebugDestination);
                        bool isFileOpenedOK = m_debugLogFile.open(logFilePath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

                        if (isFileOpenedOK)
                        {
                            pManager->m_isCommunicationDebugEnabled = true;
                        }
                        else
                        {
                            m_commDebugDestinationType = COMM_DEBUG_DISABLED;
                            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Failed to open communication debug log file: %s", m_commDebugDestination.asCharArray());
                        }
                    }

                    m_previousCommDebugDestination = m_commDebugDestination;
                }
            }
        }
        else
        {
            if (pManager != nullptr)
            {
                pManager->m_isCommunicationDebugEnabled = false;
            }

            m_commDebugDestinationType = COMM_DEBUG_DISABLED;
        }

        popAndLogToDebugDestination();


        // Put this thread to sleep for a fixed duration of time
        osSleep(100);
    }

    // Try to log any last items remaining in the queue before the thread terminates
    popAndLogToDebugDestination();

    return 0;

}

// ---------------------------------------------------------------------------
// Name:        osCommunicationDebugThread::popAndLogToDebugDestination
// Description: Pop debug items from the communication debug queue and log them
//              to the debug destination (network server or file)
// Author:      Doron Ofek
// Date:        Dec-23, 2015
// ---------------------------------------------------------------------------
void osCommunicationDebugThread::popAndLogToDebugDestination()
{
    osCommunicationDebugManager*& pManager = osCommunicationDebugManager::m_spCommunicationDebugManager;

    // If debug is enabled
    if (pManager != nullptr && pManager->m_isCommunicationDebugEnabled)
    {
        // Pop and log all debug strings that are in the consumer queue
        if (pManager != nullptr && pManager->m_pDebugQ != nullptr)
        {
            osDoubleBufferQueue<gtString>::popper qPopper(*(pManager->m_pDebugQ));
            bool isFoundItemsToPop = false;

            while (m_isContinueRunning && !qPopper.isConsumerBufferEmpty())
            {
                isFoundItemsToPop = true;
                // Get a reference to the debug string at the front of the consumer queue
                const gtString& str = qPopper.front();

                if (COMM_DEBUG_FILE == m_commDebugDestinationType)
                {
                    // Log the debug string to the debug file
                    m_debugLogFile.writeString(str);
                }
                else if (COMM_DEBUG_SERVER == m_commDebugDestinationType)
                {
                    m_tcpSocket.writeString(str);
                }

                // Remove the debug string from the consumer queue
                qPopper.pop();
            }

            if (isFoundItemsToPop && COMM_DEBUG_FILE == m_commDebugDestinationType)
            {
                m_debugLogFile.flush();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        osCommunicationDebugThread::beforeTermination
// Description: Called before the thread is terminated to handle cleanup
// Author:      Doron Ofek
// Date:        Dec-23, 2015
// ---------------------------------------------------------------------------
void osCommunicationDebugThread::beforeTermination()
{
    m_isContinueRunning = false;
    m_tcpSocket.close();
}