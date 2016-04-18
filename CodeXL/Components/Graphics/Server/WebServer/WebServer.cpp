//==============================================================================
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The web server listens for messages from the client
//==============================================================================

#include <AMDTOSWrappers/Include/osSystemError.h>
#include <AMDTOSWrappers/Include/osThread.h>

// from Common
#include "../Common/Logger.h"
#include "../Common/NamedSemaphore.h"
#include "../Common/OSWrappers.h"
#include "../Common/HTTPRequest.h"
#include "../Common/CommandTimingManager.h"
#include "../Common/SharedGlobal.h"

#include "Commands.h"
#include "ProcessTracker.h"
#include "WebServer.h"
#include "ShutdownThread.h"
#include "ClientRequestThread.h"

static osCriticalSection* s_output_criticalSection = NULL; ///< Critical section to manage how messages are written to the screen from separate threads

//--------------------------------------------------------------
// global vars
//--------------------------------------------------------------
//--------------------------------------------------------------
// g_shutdownEvent is used to tell the listening thread in the
// server to close, which will cause the main process to close
//--------------------------------------------------------------
NamedEvent g_shutdownEvent;
bool g_bAppSpecifiedAtCmdLine = false;

static DWORD s_dwPort = 80; ///< Default port number.

/// Debug function used to filter out specific commands.
/// \param strCmd Command string
/// \return True if allowed, false if not
bool AllowCommand(char* strCmd)
{
    PS_UNREFERENCED_PARAMETER(strCmd);

    return true;

    // Leave commented out while developing
    //if ( strstr ( strCmd, "IndexBufferInfo.xml" ) != NULL )
    //{
    //   return true;
    //}
    //if ( strstr ( strCmd, "BreakPoint" ) != NULL )
    //{
    //   return true;
    //}
    //return false;
}

//--------------------------------------------------------------
/// Binds to ip address and port
/// \param s Input socket
/// \return Bound socket address or 0 for failure to bind
//--------------------------------------------------------------
static NetSocket* WebServerBind(NetSocket* s)
{
    if (s->Bind((u_short)s_dwPort) == false)
    {
        if (s_dwPort == 80)
        {
            MessageBoxError("Can't listen on port 80, this port is probably being used by another application.\n\nCommon apps that use this port are:\n 1 ) Skype\n 2 ) a local webserver\n\nPlease use the –port=<arg> command-line option to use an alternate port.\nRemember that you'll have to use include the new port number when connecting to the server.\n");
        }
        else
        {
            MessageBoxError(FormatText("Port %u is already in use by another application.", s_dwPort));
        }

        s->close();
        return (0);
    }

    return s;
}

//--------------------------------------------------------------
/// Listen for connection requests
/// \param server_socket Socket to listen to.
/// \return True if we can listen on eth socket, false if we cannot listen.
//--------------------------------------------------------------
static bool WebServerListen(NetSocket* server_socket)
{
    if (server_socket->Listen() == false)
    {
        server_socket->close();
        return false;
    }

    return true;
}

//--------------------------------------------------------------
/// ForkAndWaitForClientRequests()
/// Creates a new thread and calls WaitForClientConnections
/// \param server_socket Input socket.
/// \return osThread object pointer to the new thread
//--------------------------------------------------------------
osThread* ForkAndWaitForClientRequests(NetSocket* server_socket)
{
    osThread* pClientRequestThread = NULL;
    gtString str;
    str.fromASCIIString("ClientRequestThread");
    pClientRequestThread = new ClientRequestThread(str, server_socket);
    pClientRequestThread->execute();

    return pClientRequestThread;
}

//--------------------------------------------------------------
///  Initializes webserver
/// \param port Port to listen on.
/// \return Socket to listen to.
//--------------------------------------------------------------
NetSocket* WebServerSetup(int port)
{
    NetSocket* server_socket;

    s_dwPort = port;

#ifdef DEBUG_COMMS_PERFORMANCE
    // set the performance counter frequency
    unsigned __int64 nPerfCountFreq ;
    QueryPerformanceFrequency((LARGE_INTEGER*)&nPerfCountFreq);
    CommandTimingManager::Instance()->SetPerformanceCounterFrequency(1.0 / (double)nPerfCountFreq);
#endif

    s_output_criticalSection = new osCriticalSection();

    NetSocket* s = NetSocket::Create();

    if (s == NULL)
    {
        LogConsole(logERROR, "Error creating sock()\n");
        return NULL;
    }

    // init the webserver
    server_socket = WebServerBind(s);

    if (server_socket)
    {
        if (WebServerListen(server_socket))
        {
            return server_socket;
        }
        else
        {
            LogConsole(logERROR, "can't listen\n");
        }
    }
    else
    {
        LogConsole(logERROR, "Error binding socket\n");
    }

    return NULL;
}


//--------------------------------------------------------------
/// Cleans up the webserver
/// \param server_socket Socket to close.
//--------------------------------------------------------------
void WebServerCleanup(NetSocket* server_socket)
{
    if (server_socket != NULL)
    {
        server_socket->close();
    }

    delete s_output_criticalSection;
    s_output_criticalSection = NULL;

    return;
}

/// Get the webserver's port
/// \return the port number.
DWORD GetWebServerPort()
{
    return s_dwPort;
}

////////////////////////////////////////////////////////////////////////
/// Writes an error to the console. It clears the error before exiting.
/// \param errmsg The message to display.
////////////////////////////////////////////////////////////////////////
void ClientRequestThread::OutputScreenError(const char* errmsg)
{
    PsAssert(s_output_criticalSection != NULL);
    s_output_criticalSection->enter();
    LogConsole(logERROR, "%s - %i\n", errmsg, NetSocket::LastError());
    s_output_criticalSection->leave();
}


////////////////////////////////////////////////////////////////////////
/// Writes a message to the console
/// \param msg The message to display.
////////////////////////////////////////////////////////////////////////
void ClientRequestThread::OutputScreenMessage(const char* msg)
{
    PsAssert(s_output_criticalSection != NULL);
    s_output_criticalSection->enter();
    LogConsole(logMESSAGE, "%s\n", msg);
    s_output_criticalSection->leave();
}

