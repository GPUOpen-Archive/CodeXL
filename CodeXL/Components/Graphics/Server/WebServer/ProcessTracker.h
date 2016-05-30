//==============================================================================
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief File contains a class that tracks which processes were injected into
///        and which plugins are in each process.
//==============================================================================

#ifndef GPS_PROCESS_TRACKER_H_
#define GPS_PROCESS_TRACKER_H_

#include <vector>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include "RequestInFlight.h"

#ifdef _LINUX
    #include "../Common/Linux/Interceptor.h"
    #define DLL_EXTENSION "so" ///< Extension definition
#else
    #define DLL_EXTENSION "dll" ///< Extension definition
#endif

class osThread;
class HTTPRequestHeader;

/// List of process information
typedef std::vector<ProcessInfo> ProcessInfoList;

/// File contains a class that tracks which processes were injected into
/// and which plugins are in each process.
class ProcessTracker : public TSingleton< ProcessTracker >
{
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<ProcessTracker>;

public:

    /// Generates XML that describes the injected processes and which
    /// plugins were injected.
    /// \return string containing the XML
    gtASCIIString GetProcessesXML();

    /// Tries to pass the request to any of the processes being tracked.
    /// \param pRequestHeader The request header.
    /// \param requestID The request ID.
    /// \param pClientSocket the socket used for the request
    /// \param renderLoopStalled Render loop stalled state
    /// \return false if the request is not targeted to one of the processes; true otherwise.
    bool HandleRequest(HTTPRequestHeader* pRequestHeader, CommunicationID requestID, NetSocket* pClientSocket, bool renderLoopStalled);

    /// Generates an XML command tree for each of the injected processes.
    /// \return the XML command tree
    std::string GetCommandTree();

    /// Gets the list of processes which have microDLL injected
    /// \return a list of processes which have microDLL inside
    ProcessInfoList GetListOfInjectedProcesses();

    /// Updates the list of processes which have MicroDLL loaded
    void UpdateListOfInjectedProcesses();

    /// Kills also attached processes
    void KillAllProcesses();

    /// Gets the name of the app that was injected into
    std::string GetInjectedAppName()
    {
        return m_injectedAppName;
    }

    /// Sets the name of the injected app
    void SetInjectedAppName(const char* name)
    {
        m_injectedAppName = name;
    }

    /// Gets the working directory of the injected app
    std::string GetInjectedAppDir()
    {
        return m_injectedAppDir;
    }

    /// Sets the working directory of the injected app
    void SetInjectedAppDir(const char* dir)
    {
        m_injectedAppDir = dir;
    }

    /// Gets the arguments supplied to the injected app
    std::string GetInjectedAppArgs()
    {
        return m_injectedAppArgs;
    }

    /// Sets the arguments supplied to the injected app
    void SetInjectedAppArgs(const char* args)
    {
        m_injectedAppArgs = args;
    }

#ifdef _LINUX
    /// Sets the plugin name required by the injected app
    /// (either 'GL' or 'GLES')
    void SetInjectedAppPluginName(const char* pluginName)
    {
        m_injectedAppPluginName = pluginName;
    }
#endif

    /// Create the shared memory needed to communicate between the
    /// web server and the plugin
    /// \return true if successful, false if error
    bool CreateSharedMemory();

    /// Writes information about the allowed plugins into shared memory
    /// and launches the application specified in s_strInjectedAppName
    /// \return true if the app is successfully launched; false otherwise
    bool WritePluginsToSharedMemoryAndLaunchApp();

    /// Wait for data coming in from the plugin on the streaming socket and forward
    /// it to the client.
    /// \param clientSocket the socket that the streaming data from the plugin
    ///        should be sent to
    /// \param transferBuffer the buffer where the incoming stream data is written
    ///        to
    void WaitForPluginStream(NetSocket* clientSocket, char* transferBuffer);

    /// Has the attached app been terminated
    /// \return true if it has been terminated, false if it is still running
    bool AppTerminated();

    /// Add a socket corresponding to a handle to the map
    /// \param handle the handle of interest
    /// \param socket the socket corresponding to the handle
    void AddSocketToMap(CommunicationID handle, NetSocket* socket);

    /// Remove a socket from the map
    /// \param handle the handle whose socket to remove
    void RemoveSocketFromMap(CommunicationID handle);

    /// Get a socket corresponding to a handle in the map
    /// \param handle the handle of interest
    /// \return socket corresponding to the handle
    NetSocket* GetSocketFromHandle(CommunicationID handle);

    /// Used to send a server status message back to the requesting command.
    /// \param serverState The server state type to return
    /// \param pRequestHeader The requesting HTTP command
    /// \param client_socket The socket to communicate over
    void HandleServerStatusResponse(GRAPHICS_SERVER_STATE serverState, HTTPRequestHeader* pRequestHeader, NetSocket* client_socket);

protected:

    /// Prevents default instances of the ProcessTracker class.
    ProcessTracker()
        : m_injectedAppID(0)
        , m_streamSocket(NULL)
        , m_serverSocket(NULL)
        , m_streamThread(NULL)
    {
#ifdef _LINUX
        m_injectedAppPluginName = "GL";
#endif
    }

private:

    /// list of processes that have had MicroDLL loaded into them
    ProcessInfoList m_injectedProcessList;

    /// name of the injected app
    std::string m_injectedAppName;

    /// working directory of the injected app
    std::string m_injectedAppDir;

    /// arguments of the injected app
    std::string m_injectedAppArgs;

#ifdef _LINUX
    /// plugin name required by the injected app
    std::string m_injectedAppPluginName;
#endif

    /// process id of the injected app
    unsigned long m_injectedAppID;

    /// Launches the specied app in a new process with the specified working directory and arguments.
    /// \param strApp the application to launch
    /// \param strDir the working directory for the app
    /// \param strArgs the arguments to supply to the app
    /// \param binaryType Binary type of exe
    /// \return process information about the launched process
    PROCESS_INFORMATION LaunchAppInNewProcess(gtASCIIString strApp, gtASCIIString strDir, gtASCIIString strArgs, osModuleArchitecture binaryType);

    /// Injects the application specified by the Inject command and
    /// sends the results of the injection to the requester
    /// \param requestID the ID of the incoming request
    /// \param sCmd pointer to the string containing the request
    /// \param pClientSocket the socket used to send the response
    void DoInjectCommand(CommunicationID requestID, char** sCmd, NetSocket* pClientSocket);

    bool PassRequestToPlugin(const char* strDestination, HTTPRequestHeader* pRequestHeader, unsigned long pid, NetSocket* pClientSocket);

    ////////////////////////////////////////////////////////////////////////////////////////////
    /// Before launching a new process, this function will initialize the environment for the
    /// new process.
    /// \return true if the the environment for the new process was initialized fully.
    ////////////////////////////////////////////////////////////////////////////////////////////
    bool PrelaunchEnvironmentInitialization();

    bool CreateStreamSocket(HTTPRequestHeader* pRequestHeader, NetSocket* pClientSocket);

    ////////////////////////////////////////////////////////////////////////////////////////////
    /// Close down any streaming sockets created
    ////////////////////////////////////////////////////////////////////////////////////////////
    void CloseStreamSockets();

    ////////////////////////////////////////////////////////////////////////////////////////////
    /// Close down the stream thread, responsible for forwarding any streaming requests back
    /// to the client
    ////////////////////////////////////////////////////////////////////////////////////////////
    void CloseStreamThread();

    /// Socket used for streaming data directly to the web server.
    /// This replaces the duplicate socket which communicates directly
    /// between the plugin and client
    NetSocket*  m_streamSocket;      /// the socket used for actual streaming (after accept)
    NetSocket*  m_serverSocket;      /// the socket set up and bound to the port

    osThread*   m_streamThread;      /// The thread which reads input from the plugin and forwards to the web server

    /// map of request handle to socket
    std::map<CommunicationID, NetSocket*> m_handleToSocketMap;
};

#endif // GPS_PROCESS_TRACKER_H_
