//------------------------------ gaSMSocketSpyConnectionWaiterThread.cpp ------------------------------

Yaki 6 / 5 / 2005:
osSharedMemorySocket performance seems to be poor on dual core machines. Therefore, it was
replaced by osPipeSocket.

/*
// Infra:
#include <AMDTOSWrappers/osSharedMemorySocketServer.h>

// Local:
#include <inc/gaSMSocketSpyConnectionWaiterThread.h>


// ---------------------------------------------------------------------------
// Name:        gaSMSocketSpyConnectionWaiterThread::gaSMSocketSpyConnectionWaiterThread
// Description: Constructor.
// Arguments:   socketServer - The shared memory socket server.
// Author:      Yaki Tebeka
// Date:        25/8/2005
// ---------------------------------------------------------------------------
gaSMSocketSpyConnectionWaiterThread::gaSMSocketSpyConnectionWaiterThread(osSharedMemorySocketServer& socketServer)
: _socketServer(socketServer)
{
}


// ---------------------------------------------------------------------------
// Name:        gaSMSocketSpyConnectionWaiterThread::~gaSMSocketSpyConnectionWaiterThread
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        25/8/2005
// ---------------------------------------------------------------------------
gaSMSocketSpyConnectionWaiterThread::~gaSMSocketSpyConnectionWaiterThread()
{
}


// ---------------------------------------------------------------------------
// Name:        gaSMSocketSpyConnectionWaiterThread::entryPoint
// Description: The thread's entry point.
// Return Val:  int - 0   - Success
//                  -  -1 - Failure
// Author:      Yaki Tebeka
// Date:        25/8/2005
// ---------------------------------------------------------------------------
int gaSMSocketSpyConnectionWaiterThread::entryPoint()
{
    int retVal = -1;

    // Wait for an incoming client connection:
    bool rc = _socketServer.waitForClientConnection();
    if (rc)
    {
        // Mark that the API connection was initialized:
        _isAPIConnectionInitialized = true;

        // Trigger an "API Connection established" event:
        triggerAPIConnectionEstablishedEvent();

        // Set the thread return value:
        retVal = 0;
    }

    return retVal;
}

*/
