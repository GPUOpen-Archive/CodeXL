//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This worker thread is responsible for receiving mrequests frome the client.
//==============================================================================

#ifndef CLIENT_REQUEST_THREAD_H_
#define CLIENT_REQUEST_THREAD_H_

#include <AMDTOSWrappers/Include/osThread.h>
#include "../Common/NamedSemaphore.h"
#include "../Common/NetSocket.h"
#include "../Common/HTTPRequest.h"
#include "Commands.h"

/// name of semaphore to indicate web server thread has terminated
#define CLIENT_THREAD_SEMAPHORE "CLIENT_THREAD_SEMAPHORE"

/// Shared memory size
static const unsigned long gs_SHARED_MEMORY_SIZE = 1000000;

/// Worker thread to service the client requests
class ClientRequestThread : public osThread
{
public:

    /// Constructor
    explicit ClientRequestThread(const gtString& threadName, NetSocket* serverSocket)
        : osThread(threadName)
        , m_serverSocket(serverSocket)
    {
    }

    /// Destructor
    ~ClientRequestThread()
    {
    }

protected:

    /// Entry point for worker thread
    virtual int entryPoint()
    {
        // open the client thread semaphore
        NamedSemaphore semaphore;
        semaphore.Open(CLIENT_THREAD_SEMAPHORE);

        WaitForClientRequests(m_serverSocket);

        // signal the semaphore to indicate that this thread is done
        semaphore.Signal();
        semaphore.Close();
        return 0;
    }

    void WaitForClientRequests(NetSocket* serverSocket);

    void HandleHTTPRequest(NetSocket* client_socket, SockAddrIn& client_ip, unsigned int handle);

    osThread* ForkAndWaitForPluginResponses();

    osThread* ForkAndWaitForRenderStalls();

    osThread* ForkAndWaitForShutdownResponse(osThread* pluginThread, osThread* renderStallThread);

    void OutputScreenError(const char* errmsg);

    void OutputScreenMessage(const char* msg);

private:

    /// Socket to send responses to.
    NetSocket* m_serverSocket;

};

#endif // CLIENT_REQUEST_THREAD_H_