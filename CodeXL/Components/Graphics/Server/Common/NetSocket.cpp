//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Cross-platform network socket code
//==============================================================================

#include <AMDTOSWrappers/Include/osSystemError.h>
#include <AMDTOSWrappers/Include/osTime.h>

#ifdef _LINUX
    #include <unistd.h>
#endif

#include "NetSocket.h"
#include "Logger.h"
#include "misc.h"

/// Timeout for select operation, in milliseconds
static const int SELECT_TIMEOUT = 5000;

/// Total number of live sockets in the process
int NetSocket::m_TotalLiveSockets = 0;

#if defined _WIN32
//-----------------------------------------------------------------------------
/// Windows needs this funky WSAStartup thing...
//-----------------------------------------------------------------------------
static bool InitWSA()
{
    // Commenting out the following to fix the Civ 5 connection issue.
    // init the winsock libraries
    WSADATA wd;
    int nError = WSAStartup(MAKEWORD(1, 1), &wd);

    if (nError != 0)
    {
        switch (nError)
        {
            case WSASYSNOTREADY:
                Log(logERROR, "InitCommunication - WSAStartup network subsystem not ready for communication\n");
                break;

            case WSAVERNOTSUPPORTED:
                Log(logERROR, "InitCommunication - WSAStartup Version 1.1 is not supported\n");
                break;

            case WSAEINPROGRESS:
                Log(logERROR, "InitCommunication - WSAStartup is blocked by another socket operation\n");
                break;

            case WSAEPROCLIM:
                Log(logERROR, "InitCommunication - WSAStartup is limited by too many other tasks using sockets\n");
                break;

            case WSAEFAULT:
                Log(logERROR, "InitCommunication - WSAStartup received invalid pointer\n");
                break;
        }

        return false;
    }

    return true;
}
#endif

//-----------------------------------------------------------------------------
/// Generates a socket (factory)
/// \return New socket pointer
//-----------------------------------------------------------------------------
NetSocket* NetSocket::Create()
{
#if defined _WIN32

    if (m_TotalLiveSockets == 0)
    {
        if (!InitWSA())
        {
            return NULL;
        }
    }

#endif

    NetSocket* s = new NetSocket;

    if (s->Init() != true)
    {
        delete s;
        s = 0;
    }

    return s;
}

//-----------------------------------------------------------------------------
/// Creates a socket from duplication info
/// Called from the plugin so it can communicate directly with the client
/// when streaming
/// \param pDup Socket duplication info
/// \return Pointer to socket copy
//-----------------------------------------------------------------------------
NetSocket* NetSocket::CreateFromDuplicate(NetSocket::DuplicationInfo* pDup)
{
#if defined _WIN32

    if (m_TotalLiveSockets == 0)
    {
        if (!InitWSA())
        {
            return NULL;
        }
    }

#endif

#if defined _WIN32
    /// On Windows, we use WSADuplicateSocket to construct WSAPROTOCOL_INFO structure
    /// in shared memory and pass it to the client process
    SocketType s = WSASocket(AF_INET, SOCK_STREAM, 0, pDup , 0, WSA_FLAG_OVERLAPPED);

    /// Failed?
    if (s != SOCKET_ERROR)
    {
        /// No... good. Construct a NetSocket around the OS socket
        return new NetSocket(s);
    }

#else
    PS_UNREFERENCED_PARAMETER(pDup);
#endif

    /// Failed
    return NULL;
}

//-----------------------------------------------------------------------------
/// Returns error information
/// \return Error code
//-----------------------------------------------------------------------------
int NetSocket::LastError()
{
#if defined _WIN32
    int e = WSAGetLastError();

    WSASetLastError(0);

    return e;
#else
    return osGetLastSystemError();
#endif
}

//-----------------------------------------------------------------------------
/// Construct around a (potentially zero) OS socket. Increment global live
/// socket counter.
/// \param s Input socket
//-----------------------------------------------------------------------------
NetSocket::NetSocket(NetSocket::SocketType s)
    : m_socket(s),
      m_TotalBytesSent(0),
      m_TotalBytesReceived(0)
{
    ++m_TotalLiveSockets;
}

//-----------------------------------------------------------------------------
/// Destructor. Decrements live socket counter.
//-----------------------------------------------------------------------------
NetSocket::~NetSocket()
{
    --m_TotalLiveSockets;

#if defined _WIN32

    /// Cleanup WSA on Windows.
    if (m_TotalLiveSockets == 0)
    {
        WSACleanup();
    }

#endif
}

//-----------------------------------------------------------------------------
/// Initializes a new OS socket.
/// \return True if success false if failure.
//-----------------------------------------------------------------------------
bool NetSocket::Init()
{
    /// We could probably use more arguments here -- IPv6 anyone?
    m_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_socket == SOCKET_ERROR)
    {
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Accepts connections on a listening socket
/// \param addr Input socket
/// \param addrlen Length
/// \return   New socket
//-----------------------------------------------------------------------------
NetSocket* NetSocket::Accept(struct sockaddr* addr, socklen_t* addrlen)
{
    /// Use BSD ::accept.
    NetSocket::SocketType s = ::accept(m_socket, addr, addrlen);

    if (s != SOCKET_ERROR)
    {
        /// On success, wrap the resulting OS socket.
        return new NetSocket(s);
    }

    return NULL;
}

//-----------------------------------------------------------------------------
/// Binds to a local address. Thin wrapper around BSD ::bind
/// \param port input port number
/// \return True if success, false if fail.
//-----------------------------------------------------------------------------
bool NetSocket::Bind(u_short port)
{
    bool retVal = false;

#ifdef _LINUX
    // inform the kernel that this socket is to be reused. This removes the
    // "socket still in use" error which lasts for a minute or so after the
    // last socket was closed.
    int yes = 1;

    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        return false;
    }

#endif

    sockaddr_in si;
    int socketStructLength = sizeof(sockaddr_in);
    memset(&si, 0, socketStructLength);

    si.sin_family = AF_INET;
    si.sin_port = htons(port);
    si.sin_addr.s_addr = htonl(INADDR_ANY);

    int bindVal = ::bind(m_socket, (struct sockaddr*)&si, socketStructLength);

    if (bindVal != SOCKET_ERROR)
    {
        retVal = true;
    }

    return retVal;
}

//-----------------------------------------------------------------------------
/// Listens for incoming connections
/// \return True if success, false if fail.
//-----------------------------------------------------------------------------
bool NetSocket::Listen()
{
    int listen = ::listen(m_socket, SOMAXCONN);

    if (listen != SOCKET_ERROR)
    {
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
/// Closes the socket AND DELETES THE
/// \return True if success, false if fail.
//-----------------------------------------------------------------------------
bool NetSocket::close()
{
#if defined (_WIN32)
    int r = ::closesocket(m_socket);
#elif defined (_LINUX)
    int r = ::close(m_socket);
#endif

    delete this;

    if (r != SOCKET_ERROR)
    {
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
/// Select a socket. Check its status before trying to read or write
/// \param checkForRead true if checking read status, false to check write status
/// \return true if OK, false if no sockets available or socket error
//-----------------------------------------------------------------------------
bool NetSocket::Select(bool checkForRead)
{
#if defined (_WIN32)
    int highestFD = 0;
    fd_set fdEnabledSocketDescriptorsSet  = { 0 };
#elif defined (_LINUX)
    fd_set fdEnabledSocketDescriptorsSet;
    int highestFD = m_socket + 1;
#endif

    // A set of sockets that we will enable read/write on:
    FD_ZERO(&fdEnabledSocketDescriptorsSet);

    // Add this socket to the above set:
#pragma warning( push, 3 )  // this macro generates level 4 warnings. Suppress these
    FD_SET(m_socket, &fdEnabledSocketDescriptorsSet);
#pragma warning (pop)

    // Convert the timeout to a timeval structure:
    TIMEVAL timeOutAsTimeVal;
    osTimeValFromMilliseconds(SELECT_TIMEOUT, timeOutAsTimeVal);

    // Enable reading/writing from our socket, depending on checkForRead flag
    int rc;

    if (checkForRead == true)
    {
        rc = ::select(highestFD, &fdEnabledSocketDescriptorsSet, NULL, NULL, &timeOutAsTimeVal);
    }
    else
    {
        rc = ::select(highestFD, NULL, &fdEnabledSocketDescriptorsSet, NULL, &timeOutAsTimeVal);
    }

    // tidy up
    FD_ZERO(&fdEnabledSocketDescriptorsSet);

    // Return status if socket is ready for read/write
    if (rc > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------------
/// Receive data
/// \param buf Input data buffer
/// \param len Input length of buffer
/// \param readDataSize Size of data to read
/// \return True if success, false if fail.
//-----------------------------------------------------------------------------
bool NetSocket::Receive(void* buf, gtSize_t len, gtSize_t& readDataSize)
{
    // Get the socket status before trying to read from it.
    readDataSize = 0;
    bool success = Select(true);

    if (success == false)
    {
        return false;
    }

    int r;

    r = ::recv(m_socket,
               reinterpret_cast<char*>(buf),
               (int)len,
               0);

    if (r != SOCKET_ERROR)
    {
        m_TotalBytesReceived += r;
        readDataSize = r;
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
/// Send data
/// \param buf Data buffer to send
/// \param len Length of the data to send
/// \return True if success, false if fail.
//-----------------------------------------------------------------------------
bool NetSocket::Send(const void* buf, int len)
{
    // Get the socket status before trying to read from it.
    bool success = Select(false);

    if (success == false)
    {
        return false;
    }

    bool  retVal = false;

#if defined _WIN32
    int s;
    s = ::send(m_socket,
               reinterpret_cast<const char*>(buf),
               len,
               0);

#elif defined _LINUX

    ssize_t s;
    s = ::write(m_socket, buf, len);
#endif

    if (s != SOCKET_ERROR)
    {
        m_TotalBytesSent += s;
        retVal = true;
    }

    return retVal;
}

//-----------------------------------------------------------------------------
/// Connect to an open socket.
/// \param  portAddress port address
/// \return True if success, false if fail.
//-----------------------------------------------------------------------------
bool NetSocket::Connect(osPortAddress& portAddress)
{
    bool retVal = false;

    osSocketDescriptor socketDescriptor = m_socket;

    if (socketDescriptor != -1)
    {
        sockaddr_in internetSocketAddress;
        bool rc1 = portAddress.asSockaddr(internetSocketAddress, true);

        if (rc1)
        {
            int rc2 = ::connect(socketDescriptor, (struct sockaddr*)&internetSocketAddress, sizeof(sockaddr_in));

            if (rc2 == 0)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}

//-----------------------------------------------------------------------------
/// Construct information required to duplicate the socket into another
/// process.
/// \param pid Process ID
/// \param pDst Socket info
/// \return The duplicate socket
//-----------------------------------------------------------------------------
int NetSocket::DuplicateToPID(unsigned long pid, void* pDst)
{
#if defined _WIN32
    return WSADuplicateSocket(m_socket, pid, (LPWSAPROTOCOL_INFO)pDst);
#else
    PS_UNREFERENCED_PARAMETER(pid);
    PS_UNREFERENCED_PARAMETER(pDst);
    return 0;
#endif
}
