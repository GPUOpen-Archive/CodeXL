//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This worker thread is responsible for receiving mrequests frome the client.
//==============================================================================

#include <AMDTOSWrappers/Include/osSystemError.h>
#include "ClientRequestThread.h"
#include "../Common/NamedEvent.h"
#include "../Common/Logger.h"
#include "../Common/NamedSemaphore.h"
#include "../Common/OSWrappers.h"
#include "../Common/HTTPRequest.h"
#include "../Common/CommandTimingManager.h"
#include "../Common/SharedGlobal.h"
#include "ProcessTracker.h"
#include "ShutdownThread.h"
#include "../Common/Logger.h"
#include "PluginResponseThread.h"
#include "RenderStallThread.h"
#include "Commands.h"
#include "WebServer.h"

// Critical section is now handled by common code. This is set up when
// the web server is set up so it should be valid before any calls
// are made to it.
//static osCriticalSection* s_output_criticalSection = NULL;

/// Loops forever waiting for client connections. On connection starts a thread to handling the http transaction
/// \param server_socket Socket to wait on.
void ClientRequestThread::WaitForClientRequests(NetSocket* server_socket)
{
    NetSocket* client_socket;
    sockaddr_in client_address;
    static unsigned int handle = 0;

    socklen_t client_address_len = sizeof(sockaddr_in);

    NamedEvent shutdownEvent;
    bool opened = shutdownEvent.Open("GPS_SHUTDOWN_SERVER");

    if (opened == false)
    {
        Log(logERROR, "Failed to open shutdown event (error %d). Will not wait for client connections.\n", osGetLastSystemError());
        return;
    }

    // start another thread to listen for responses coming back from launched apps
    osThread* pluginThread = ForkAndWaitForPluginResponses();

    if (pluginThread == NULL)
    {
        Log(logERROR, "Failed to create shared memory for plugin. Will not wait for client connections.\n");
        return;
    }

    // start another thread to detect for stalls
    osThread* renderStallThread = ForkAndWaitForRenderStalls();

    if (renderStallThread == NULL)
    {
        Log(logERROR, "Failed to create renderStallThread.\n");
        return;
    }

    osThread* shutdownThread = ForkAndWaitForShutdownResponse(pluginThread, renderStallThread);

    // while the shutdown event is not signaled (passing in 0 as a wait time causes the function to return WAIT_TIMEOUT immediately),
    // accept client connections and handle the requests
    while (false == shutdownEvent.IsSignaled())
    {
        client_socket = server_socket->Accept((struct sockaddr*) & client_address, &client_address_len);

        if (client_socket == NULL)
        {
            OutputScreenError("Error in accept()");
            continue;
        }

        // This where a message gets handled from the client
        HandleHTTPRequest(client_socket, client_address.sin_addr, ++handle);
    }

    // shutdown the shared memory
    smClose("PLUGINS_TO_GPS");

    LogConsole(logMESSAGE, "ClientRequestThread loop terminating\n");

    // when the shutdown event is signaled, the loop is exited
    // and we have to wait for the plugin thread and the shutdown thread to exit before returning to the main server
    // First, wait for threads to finish. Deleting the thread will automatically terminate the thread
    while (shutdownThread->isAlive())
    {
        ;
    }

    delete shutdownThread;

    while (pluginThread->isAlive())
    {
        ;
    }

    delete pluginThread;

    while (renderStallThread->isAlive())
    {
        ;
    }

    delete renderStallThread;

    LogConsole(logMESSAGE, "ClientRequestThread terminating\n");

    shutdownEvent.Close();
}


//--------------------------------------------------------------
/// Starts a new thread which listens for plugins' responses
/// Uses common code in the osWrappers.
/// The execute method starts the thread running and eventually
/// calls the entryPoint() method
/// \return osThread object pointer to the new thread
//--------------------------------------------------------------
osThread* ClientRequestThread::ForkAndWaitForPluginResponses()
{
    // Create the shared memory and semaphore in the main thread before the
    // Plugin Thread is set up. This avoids a race condition where the plugin
    // thread could be in the middle of initialization when it gets terminated
    // by an early shutdown and still holds the lock for shared memory

    // create shared memory that is "big enough"
    if (smCreate("PLUGINS_TO_GPS", 1, gs_SHARED_MEMORY_SIZE) == false)
    {
        Log(logERROR, "Failed to create PLUGINS_TO_GPS shared memory.\n");
        return NULL;
    }

    // Create the thread which listens to plugin responses and start it running.
    // Create the thread which listens for a shutdown signal and start it running.
    // A pointer to the plugin response thread is passed into the shutdown thread
    // so that it can close down the plugin thread on shutdown
    gtString str;
    str.fromASCIIString("PluginResponseThread");
    osThread* pluginResponseThread = new PluginResponseThread(str);
    pluginResponseThread->execute();

    return pluginResponseThread;
}

//--------------------------------------------------------------
/// Starts a new thread which detects if the server rendering has stalled.
/// Uses common code in the osWrappers.
/// The execute method starts the thread running and eventually
/// calls the entryPoint() method
/// \return osThread object pointer to the new thread
//--------------------------------------------------------------
osThread* ClientRequestThread::ForkAndWaitForRenderStalls()
{
    gtString str;
    str.fromASCIIString("RenderStallThread");
    osThread* renderStallThread = new RenderStallThread(str);
    renderStallThread->execute();

    return renderStallThread;
}

//--------------------------------------------------------------
/// Starts a new thread which listens for the shutdown signal
/// Uses common code in the osWrappers.
/// The execute method starts the thread running and eventually
/// calls the entryPoint() method
/// \param pluginThread pointer to the plugin thread, so that it can be terminated on shutdown
/// \param renderStallThread pointer to the render stall thread, so that it can be terminated on shutdown
/// \return osThread object pointer to the new thread
//--------------------------------------------------------------
osThread* ClientRequestThread::ForkAndWaitForShutdownResponse(osThread* pluginThread, osThread* renderStallThread)
{
    // The main code in this function.
    gtString str;
    str.fromASCIIString("ShutdownThread");
    ShutdownThread* shutdownThread = new ShutdownThread(str, pluginThread, renderStallThread);
    shutdownThread->execute();

    return shutdownThread;
}

//--------------------------------------------------------------
/// Processes incoming HTTP requests
/// \param client_socket pointer to the client socket
/// \param client_ip socket address information
/// \param handle identifier for this request
//--------------------------------------------------------------
void ClientRequestThread::HandleHTTPRequest(NetSocket* client_socket, SockAddrIn& client_ip, unsigned int handle)
{
    HTTPRequestHeader* pRequestHeader = new HTTPRequestHeader();
    pRequestHeader->SetClientHandle(handle);
    ProcessTracker::Instance()->AddSocketToMap(handle, client_socket);
    pRequestHeader->SetClientIP(client_ip);

    string strError;
    // Now read the header.
    HTTP_REQUEST_RESULT result = pRequestHeader->ReadWebRequest(strError, client_socket);

    // Check for socket error
    if (result == HTTP_SOCKET_ERROR)
    {
        string strFullError("Error SocketReadHeader.\n");

        strFullError  += strError;
        OutputScreenError(strFullError.c_str());

        client_socket->close();
        return;
    }

    // Handle parse failure
    if (result == HTTP_PARSE_ERROR || result == HTTP_POST_DATA_ERROR)
    {
        // handle bad header!
        CommunicationID badRequest = CreateRequest(pRequestHeader, true);
        SendHTTPErrorResponse(badRequest, 400, client_socket);     // 400 - bad request
        RemoveRequest(badRequest);
        return;
    }

#ifdef DEBUG_COMMS_PERFORMANCE

    if (client_socket != 0)
    {
        if (AllowCommand(pRequestHeader->GetUrl()) == true)
        {
            // Create a new timing object
            CommandTiming* pTiming = new CommandTiming();
            LARGE_INTEGER nPerformanceCount;
            QueryPerformanceCounter(&nPerformanceCount);
            // Set the start time
            pTiming->SetWebServerRoundTripStart(nPerformanceCount);
            // Assign it the request ID so we can track it in the map
            pTiming->SetRequestID(client_socket);
            pTiming->SetURL(pRequestHeader->GetUrl());
            // Add the object to the map
            CommandTimingManager::Instance()->AddTimingToPendingList(pTiming);
        }
    }

#endif

    // log line
    //PsAssert(s_output_criticalSection != NULL);
    //s_output_criticalSection->enter();
    static DWORD dwCnt = 0;
    LogConsole(logMESSAGE, "%3u: %s  - %s\n", dwCnt, inet_ntoa(pRequestHeader->GetClientIP()), pRequestHeader->GetUrl());
    dwCnt++;
    //s_output_criticalSection->leave();

    if (strstr(pRequestHeader->GetMethod(), "GET"))
    {
        ProcessGetMethod(pRequestHeader, client_socket, GetServerStalledState());
    }
    else if (strstr(pRequestHeader->GetMethod(), "POST"))
    {
        ProcessPostMethod(pRequestHeader, client_socket, GetServerStalledState());
    }
    else
    {
        CommunicationID badRequest = CreateRequest(pRequestHeader, true);
        SendHTTPErrorResponse(badRequest, 501, client_socket);     // 501 not implemented
        RemoveRequest(badRequest);
    }

    return;
}





