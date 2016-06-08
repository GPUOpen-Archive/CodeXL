//------------------------------ osCommunicationDebugThread.h ------------------------------

#ifndef __OS_COMMUNICATION_DEBUG_THREAD
#define __OS_COMMUNICATION_DEBUG_THREAD

#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osTCPSocketClient.h>

//forward declaration
class osCommunicationDebugManager;


// ----------------------------------------------------------------------------------
// Class Name:           osCommunicationDebugThread
// General Description:
//   checks if there is data in the communication debug queue and sends it to a debug server.
//
// Author:               Doron Ofek
// Creation Date:        Dec-20, 2015
// ----------------------------------------------------------------------------------
class osCommunicationDebugThread : public osThread
{
public:
    osCommunicationDebugThread();
    virtual void beforeTermination();
    void requestExit() { m_isContinueRunning = false; }

protected:
    virtual int entryPoint();
    void popAndLogToDebugDestination();

    enum CommunicationDebugDestination
    {
        COMM_DEBUG_DISABLED,
        COMM_DEBUG_FILE,
        COMM_DEBUG_SERVER
    };

    bool m_isContinueRunning;
    gtString m_commDebugDestination;
    gtString m_previousCommDebugDestination;
    CommunicationDebugDestination m_commDebugDestinationType;
    osFile m_debugLogFile;
    osTCPSocketClient m_tcpSocket;

};

#endif  // __OS_COMMUNICATION_DEBUG_THREAD
