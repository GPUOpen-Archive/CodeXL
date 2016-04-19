//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  declaration of cross-platform network socket code
//==============================================================================

#ifndef GPS_NETSOCKET_INCLUDED
#define GPS_NETSOCKET_INCLUDED

#if defined _WIN32
    #include <Winsock2.h>
    typedef int socklen_t;
#else
    #include <stdio.h>
    #include <netdb.h>
    #include <arpa/inet.h>
#endif

#include <AMDTOSWrappers/Include/osTCPSocketServer.h>

//-----------------------------------------------------------------------------
/// Class to represent a network socket
/// Deals with OS specifics
/// NetSocket derives from osTCPSocketServer so that OS Specifics are
/// abstracted away in the base classes. It could have contained an
/// osTCPSocketServer, but this class needs access to the protected socket
/// descriptor methods.
//-----------------------------------------------------------------------------
class NetSocket
{
public:
#if defined _WIN32
    /// Structure that is passed in shared memory to allow target process
    /// to construct a socket from the parent
    typedef WSAPROTOCOL_INFO   DuplicationInfo;
#else
    /// Structure that is passed in shared memory to allow target process
    /// to construct a socket from the parent
    typedef int                DuplicationInfo;
#endif

    static NetSocket* Create();
    static NetSocket* CreateFromDuplicate(DuplicationInfo* pDup);
    static int LastError();

    /// Close the socket and delete the class instance
    bool close();

    /// Bind. Equivalent to BSD ::bind
    bool Bind(u_short port);

    /// Listen. Equivalent to BSD ::listen
    bool Listen();

    /// Accept. Equivalent to BSD ::accept. Returns new socket,
    NetSocket* Accept(struct sockaddr* addr, socklen_t* addrlen);

    /// Receive from target.
    bool Receive(void* buf, gtSize_t len, gtSize_t& readDataSize);

    /// Send data to target
    bool Send(const void* buf, int len);

    /// Connect to an open socket
    bool Connect(osPortAddress& portAddress);

    /// Builds data structure suitable for sending to target process
    /// pid such that it can inherit the socket
    int DuplicateToPID(unsigned long pid, void* pDst);

    ~NetSocket();

protected:

    /// Socket type
    typedef osSocketDescriptor       SocketType;

    /// Protected constructor and destructor. Allow construction
    /// from existing socket
    NetSocket(osSocketDescriptor s = osSocketDescriptor(0));

    bool Init();

    /// call select on a socket
    bool Select(bool checkForRead = true);

    /// This is the actual OS-level socket
    SocketType m_socket;

    /// Statistics capture live sockets
    static int m_TotalLiveSockets;

    /// Statistics total bytes sent
    size_t     m_TotalBytesSent;

    /// Statistics total bytes received
    size_t     m_TotalBytesReceived;
};

#endif // GPS_NETSOCKET_INCLUDED
