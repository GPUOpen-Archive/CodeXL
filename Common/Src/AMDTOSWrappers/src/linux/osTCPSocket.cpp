//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTCPSocket.cpp
///
//=====================================================================

//------------------------------ osTCPSocket.cpp ------------------------------

// POSIX:
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osSystemError.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osTCPSocket.h>

// Read interval (we divide the read timeout into small interval of this length):
#define OS_TCP_SOCKET_READ_INTERVAL 1000

// Maximal amount of error printouts:
#define OS_TCP_SOCKET_MAX_ERROR_PRINTOUTS 10

// Static members initializations:
osSocketDescriptor osTCPSocket::NO_OS_SOCKET_DESCRIPTOR = -1;


// ---------------------------------------------------------------------------
// Name:        osTCPSocket::osTCPSocket
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        3/1/2004
// ---------------------------------------------------------------------------
osTCPSocket::osTCPSocket(const gtString& socketName)
    : osSocket(socketName), _errorsCount(0), _blockOnDNS(false), _socketDescriptor(NO_OS_SOCKET_DESCRIPTOR), _isOpen(false)
{
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocket::osTCPSocket
// Description: Constructor
// Arguments:   osSocketDescriptor - The OS socket descriptor.
// Author:      AMD Developer Tools Team
// Date:        3/1/2004
// --------------------------------------------------------------------------
osTCPSocket::osTCPSocket(unsigned int osSocketDescriptor, const gtString& socketName)
    : osSocket(socketName), _errorsCount(0), _blockOnDNS(false), _socketDescriptor(NO_OS_SOCKET_DESCRIPTOR), _isOpen(false)
{
    setOSDescriptor(osSocketDescriptor);
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocket::getPeerHostAddress
// Description:
//   Retrieves the peer host address. (The address of the host to which
//   this socket is connected).
//   Notice that this method can only be called after the socket
//   connection was established (connect / accept succeeded).
// Arguments: peerHostAddress - Will get the peer host address.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        27/4/2008
// ---------------------------------------------------------------------------
bool osTCPSocket::getPeerHostAddress(osPortAddress& peerHostAddress) const
{
    bool retVal = false;

    // Get the peer host address:
    struct sockaddr_in peerSocketAddress;
    socklen_t sockaddrLen = sizeof(peerSocketAddress);
    int rc1 = ::getpeername(_socketDescriptor, (sockaddr*)&peerSocketAddress, &sockaddrLen);
    GT_IF_WITH_ASSERT(rc1 == 0)
    {
        // Output the peer host address:
        bool rc2 = peerHostAddress.setFromSocaddr(peerSocketAddress);
        GT_IF_WITH_ASSERT(rc2)
        {
            retVal = true;
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        osTCPSocket::getCurrenttAddress
// Description:
//   Retrieves the local address.
// Arguments: peerHostAddress - Will get the current address.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        07/08/2013
// ---------------------------------------------------------------------------
bool osTCPSocket::getCurrentAddress(osPortAddress& peerHostAddress) const
{
    bool retVal = false;

    // Get the peer host address:
    struct sockaddr_in peerSocketAddress;
    socklen_t sockaddrLen = sizeof(peerSocketAddress);
    int rc1 = ::getsockname(_socketDescriptor, (sockaddr*)&peerSocketAddress, &sockaddrLen);
    GT_IF_WITH_ASSERT(rc1 == 0)
    {
        // Output the peer host address:
        bool rc2 = peerHostAddress.setFromSocaddr(peerSocketAddress);
        GT_IF_WITH_ASSERT(rc2)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocket::setOSDescriptor
// Description: Sets my OS descriptor.
// Author:      AMD Developer Tools Team
// Date:        18/1/2004
// ---------------------------------------------------------------------------
void osTCPSocket::setOSDescriptor(osSocketDescriptor socketDescriptor)
{
    // If I already have an associated OS socket - release it:
    if (_isOpen)
    {
        close();
    }

    // Set my OS socket descriptor:
    _socketDescriptor = socketDescriptor;

    // If we got a NULL socket descriptor:
    if (socketDescriptor == NO_OS_SOCKET_DESCRIPTOR)
    {
        _isOpen = false;
    }
    else
    {
        _isOpen = true;
    }
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocket::~osTCPSocket
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        3/1/2004
// ---------------------------------------------------------------------------
osTCPSocket::~osTCPSocket()
{
    // If the OS socket is open - close it:
    if (_isOpen)
    {
        close();
    }
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocket::channelType
// Description: Returns my channel type - TCP sockets are binary channels.
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
osChannel::osChannelType osTCPSocket::channelType() const
{
    return osChannel::OS_BINARY_CHANNEL;
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocket::open
// Description: Opens the socket end point.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/1/2004
// ---------------------------------------------------------------------------
bool osTCPSocket::open()
{
    bool retVal = false;

    // Verify that the socket was not already opened:
    GT_IF_WITH_ASSERT(!_isOpen)
    {
        // Use "Internetwork" address family (TCP, UDP, etc):
        int socketAddressFamily = PF_INET;

        // This is a TCP socket:
        int socketType = SOCK_STREAM;

        // Create the socket:
        _socketDescriptor = ::socket(socketAddressFamily, socketType, 0);
        GT_IF_WITH_ASSERT(_socketDescriptor != -1)
        {
            _isOpen = true;
            retVal = true;

            // Performance optimization: Avoiding Nagle algorithm to solve a severe
            // performance issue encountered in remote debugging sessions.
            int flag = 1;
            int result = setsockopt(_socketDescriptor,  /* socket affected */
                                    IPPROTO_TCP,        /* set option at TCP level */
                                    TCP_NODELAY,        /* name of option */
                                    (char*)&flag,       /* the cast is historical cruft */
                                    sizeof(int));       /* length of option value */

            GT_ASSERT(result >= 0);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osTCPSocket::open
// Description: Opens the socket end point and specifies the sizes for the send and receive buffers
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/20/2016
// ---------------------------------------------------------------------------
bool osTCPSocket::open(int sizeOfReceiveBuffer, int sizeOfSendBuffer)
{
    bool retVal = false;

    // Verify that the socket was not already opened:
    if (!_isOpen)
    {
        retVal = open();

        if (retVal)
        {
            //  Specify the total per-socket buffer space reserved for receives.
            int result = setsockopt(_socketDescriptor,            /* socket affected */
                                    SOL_SOCKET,                                      /* set option at TCP level */
                                    SO_RCVBUF,                                      /* name of option */
                                    (char*)&sizeOfReceiveBuffer,                                    /* the cast is historical cruft */
                                    sizeof(int));                                    /* length of option value */
            retVal = (result == 0);
        }

        if (retVal)
        {
            //  Specify the total per-socket buffer space reserved for receives.
            int result = setsockopt(_socketDescriptor,            /* socket affected */
                                    SOL_SOCKET,                                      /* set option at TCP level */
                                    SO_SNDBUF,                                      /* name of option */
                                    (char*)&sizeOfSendBuffer,                                    /* the cast is historical cruft */
                                    sizeof(int));                                    /* length of option value */
            retVal = (result == 0);
        }

        if (!retVal)
        {
            // Get the system's last recorded error:
            gtString systemErrorAsString;
            osGetLastSystemErrorAsString(systemErrorAsString);

            // Output the error to the log file:
            gtString errorMsg = L"Socket open error: ";
            errorMsg += systemErrorAsString;
            OS_OUTPUT_DEBUG_LOG(errorMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osTCPSocket::close
// Description: Close this socket
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/1/2004
// ---------------------------------------------------------------------------
bool osTCPSocket::close()
{
    bool retVal = false;

    // Sanity check:
    if (_socketDescriptor != NO_OS_SOCKET_DESCRIPTOR)
    {
        // Close the OS socket:
        int rc1 = ::close(_socketDescriptor);
        GT_IF_WITH_ASSERT(rc1 == 0)
        {
            _isOpen = false;
            _socketDescriptor = NO_OS_SOCKET_DESCRIPTOR;
            retVal = true;
        }
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"Warning: close() called on an uninitialized socket descriptor" , OS_DEBUG_LOG_DEBUG);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocket::isOpen
// Description: Returns true iff the socket is open.
// Author:      AMD Developer Tools Team
// Date:        31/1/2004
// ---------------------------------------------------------------------------
bool osTCPSocket::isOpen() const
{
    return _isOpen;
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocket::write
// Description: Write data into the socket.
// Arguments:   pDataBuffer - A buffer that contains the data to be written.
//              dataSize - The size of the data.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool osTCPSocket::writeImpl(const gtByte* pDataBuffer, gtSize_t dataSize)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(0 < dataSize)
    {
        // Calculate the write timeout:
        struct timeval writeTimeout;
        osTimeValFromMilliseconds(_writeOperationTimeOut, writeTimeout);

        // Build a file descriptors set that contains the socket file descriptor:
        fd_set fileDescriptorsSet;
        FD_ZERO(&fileDescriptorsSet);
        FD_SET(_socketDescriptor, &fileDescriptorsSet);

        // Build a pipe set of pipes that will be tested for errors:
        fd_set exceptionPipeSet;
        FD_ZERO(&exceptionPipeSet);
        FD_SET(_socketDescriptor, &exceptionPipeSet);

        // Wait until we can write data into the socket:
        int highestFD = _socketDescriptor + 1;
        int rc1 = ::select(highestFD, NULL, &fileDescriptorsSet, &exceptionPipeSet, &writeTimeout);

        if (rc1 < 0)
        {
            // An error occurred:
            verifyConnectionAfterError();
            GT_ASSERT_EX(false, OS_STR_writeError);
        }
        else if (rc1 == 0)
        {
            // The write timeout was reached:
            GT_ASSERT_EX(false, OS_STR_timeoutReached);
        }
        else
        {
            // If the socket had an exception (error):
            bool socketExceptionOccur = (FD_ISSET(_socketDescriptor, &exceptionPipeSet));

            if (socketExceptionOccur)
            {
                GT_ASSERT_EX(false, OS_STR_socketException);
            }
            else
            {
                // Write the data into the socket:
                ssize_t rc1 = ::write(_socketDescriptor, pDataBuffer, dataSize);

                // Verify that all the data was written:
                GT_IF_WITH_ASSERT(rc1 == (ssize_t)dataSize)
                {
                    retVal = true;
                }
                else
                {
                    verifyConnectionAfterError();
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocket::read
// Description: Reads data from the socket.
// Arguments:   pDataBuffer - Buffer to read the data into.
//              dataSize - The size of the data to be read.
//              amountOfDataRead - The amount of data read by this function call.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool osTCPSocket::readImpl(gtByte* pDataBuffer, gtSize_t dataSize)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(0 < dataSize)
    {
        gtSize_t readDataSize = 0;
        retVal = readDataFromSocket(pDataBuffer, dataSize, false, readDataSize);

        retVal = retVal && (dataSize == readDataSize);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocket::readAvailableData
// Description:
//   Reads and outputs the data that is currently available in the socket.
//   If no data is available in the pipe for reading, waits _readOperationTimeOut
//   milliseconds until data is available.
//   If more data than bufferSize is available, reads only bufferSize bytes of data.
//
// Arguments: pDataBuffer - A buffer that will receive the data.
//            bufferSize - The buffer size.
//            amountOfDataRead - The amount of data actually read.
//
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        10/2/2008
// ---------------------------------------------------------------------------
bool osTCPSocket::readAvailableDataImpl(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead)
{
    bool retVal = readDataFromSocket(pDataBuffer, bufferSize, true, amountOfDataRead);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocket::readDataFromSocket
// Description:
//   Reads data from the pipe.
//   readOnlyAvailableData - true - Causes this function to implement readAvailableData().
//                         - false - Causes this function to implement read().
//
// Author:      AMD Developer Tools Team
// Date:        10/2/2008
// ---------------------------------------------------------------------------
bool osTCPSocket::readDataFromSocket(gtByte* pDataBuffer, gtSize_t bufferSize, bool readOnlyAvailableData, gtSize_t& readDataSize)
{
    bool retVal = false;

    // Will get the amount of data read so far:
    readDataSize = 0;

    // Testing section - leave commented out!
    /*
    gtString foo1 = "Read starts: ";
    foo1.appendFormattedString(L"bytes to read: %d", dataSize);
    OS_OUTPUT_DEBUG_LOG(foo1.asCharArray(), OS_INFO_SEVERITY);
    */

    // If the socket does not have errors:
    if (_errorsCount == 0)
    {
        // Will hold the time left until the read operation timeout reaches:
        long timeLeftToTimeout = _readOperationTimeOut;

        // Will contain the current read interval timeout:
        struct timeval readIntervalTimeout;

        // Loop until we get all required data:
        bool goOn = true;

        while (goOn)
        {
            // Calculate current read interval timeout (we allow maximal interval of OS_TCP_SOCKET_READ_INTERVAL):
            if (OS_TCP_SOCKET_READ_INTERVAL < timeLeftToTimeout)
            {
                osTimeValFromMilliseconds(OS_TCP_SOCKET_READ_INTERVAL, readIntervalTimeout);
            }
            else
            {
                osTimeValFromMilliseconds(timeLeftToTimeout, readIntervalTimeout);
            }

            // Testing section - leave commented out!
            // OS_OUTPUT_DEBUG_LOG(L"Doing select", OS_INFO_SEVERITY);

            // Build a file descriptors set that contains the incoming socket:
            fd_set incomingSocketsSet;
            FD_ZERO(&incomingSocketsSet);
            FD_SET(_socketDescriptor, &incomingSocketsSet);

            // Build a file descriptors set of sockets that will be tested for errors:
            fd_set exceptionSocketSet;
            FD_ZERO(&exceptionSocketSet);
            FD_SET(_socketDescriptor, &exceptionSocketSet);

            // Wait until we have data to read:
            int highestPipe = _socketDescriptor + 1;
            int rc1 = ::select(highestPipe, &incomingSocketsSet, NULL, &exceptionSocketSet, &readIntervalTimeout);

            if (0 < rc1)
            {
                // If the pipe had an exception (error):
                bool pipeExceptionOccur = (FD_ISSET(_socketDescriptor, &exceptionSocketSet));

                if (pipeExceptionOccur)
                {
                    osSystemErrorCode errorCode = osGetLastSystemError();
                    verifyConnectionAfterError(errorCode);
                    // Build an appropriate error message:
                    gtString errMsg = OS_STR_pipeException;
                    errMsg += OS_STR_osReportedErrorIs;
                    gtString systemError;
                    osGetSystemErrorAsString(errorCode, systemError);
                    errMsg += systemError;
                    GT_ASSERT_EX(false, errMsg.asCharArray());

                    _errorsCount++;
                    goOn = false;
                }
                else
                {
                    // We have data available for reading:
                    // Read the available data:
                    size_t rc2 = ::read(_socketDescriptor, pDataBuffer, bufferSize - readDataSize);
                    readDataSize += rc2;
                    pDataBuffer += rc2;

                    // Testing section - leave commented out!
                    /*
                    gtString foo2 = "Read bytes: ";
                    foo2.appendFormattedString(L"%d", rc2);
                    OS_OUTPUT_DEBUG_LOG(foo2.asCharArray(), OS_INFO_SEVERITY);
                    */

                    // If we read all required data:
                    if ((size_t) - 1 == rc2)
                    {
                        // Socket error!
                        verifyConnectionAfterError();
                        GT_ASSERT(false);
                        goOn = false;
                    }
                    else if (0 == rc2)
                    {
                        // A return value of 0 means "end of file". Thus, the connection is closed:
                        _isOpen = false;
                        goOn = false;
                        retVal = true;
                    }
                    else if (bufferSize == readDataSize)
                    {
                        goOn = false;
                        retVal = true;

                        // Testing section - leave commented out!
                        // OS_OUTPUT_DEBUG_LOG(L"Read all required data", OS_INFO_SEVERITY);
                    }
                    else if (bufferSize < readDataSize)
                    {
                        // We read too much data (this code should not be reached):
                        GT_ASSERT_EX(false, OS_STR_readError);
                        _errorsCount++;
                        goOn = false;
                    }
                    else
                    {
                        // If we were asked to read only the first available data:
                        if (readOnlyAvailableData)
                        {
                            goOn = false;
                            retVal = true;
                        }
                        else
                        {
                            // Testing section - leave commented out!
                            // OS_OUTPUT_DEBUG_LOG(L"Still have data to read - doing another read iteration", OS_INFO_SEVERITY);
                        }
                    }
                }
            }
            else if (rc1 == 0)
            {
                // Timeout for current interval elapsed, but there is no data in the pipe for reading:

                // Calculate the time left for read operation timeout:
                timeLeftToTimeout -= OS_TCP_SOCKET_READ_INTERVAL;

                // If we reached the read operation timeout:
                if (timeLeftToTimeout <= 0)
                {
                    GT_ASSERT_EX(false, OS_STR_timeoutReached);
                    goOn = false;
                }
                else
                {
                    // If someone reduced the read operation timeout while we are waiting
                    // for data (see "implementation notes" at the header of this function):
                    if (_readOperationTimeOut < timeLeftToTimeout)
                    {
                        timeLeftToTimeout = _readOperationTimeOut;

                        // Testing section - leave commented out!
                        // OS_OUTPUT_DosTCPSocketEBUG_LOG(L"Reducing timeout", OS_INFO_SEVERITY);
                    }

                    // Do another wait interval ...
                }
            }
            else if (rc1 == -1)
            {
                // An error occurred:

                // If select() was interrupted by a signal:
                osSystemErrorCode lastSystemError = osGetLastSystemError();
                verifyConnectionAfterError(lastSystemError);

                if (lastSystemError == EINTR)
                {
                    // Continue waiting for the pipe:
                    goOn = true;
                }
                else
                {
                    // Build an appropriate error message:
                    gtString errMsg = OS_STR_readError;
                    errMsg += OS_STR_osReportedErrorIs;
                    gtString systemError;
                    osGetSystemErrorAsString(lastSystemError, systemError);
                    errMsg += systemError;
                    GT_ASSERT_EX(false, errMsg.asCharArray());

                    goOn = false;
                }
            }
        }
    }

    // Testing section - leave commented out!
    // OS_OUTPUT_DEBUG_LOG(L"Read ended", OS_INFO_SEVERITY);

    // If there was an error:
    if (!retVal)
    {
        // Mark that the pipe has an error:
        _errorsCount++;

        // Yaki - 4/11/2007:
        // We limit the amount of "Pipe read error" printouts, since some clients sent
        // us huge log files, containing millions of "Pipe read error" printouts.
        if (_errorsCount < OS_TCP_SOCKET_MAX_ERROR_PRINTOUTS)
        {
            gtString debugMessage;
            debugMessage.appendFormattedString(OS_STR_pipeReadError, _socketName.asCharArray());
            GT_ASSERT_EX(false, debugMessage.asCharArray());
        }
        else if (_errorsCount == OS_TCP_SOCKET_MAX_ERROR_PRINTOUTS)
        {
            gtString debugMessage;
            debugMessage.appendFormattedString(OS_STR_pipeReadErrorLastMessage, _socketName.asCharArray());
            GT_ASSERT_EX(false, debugMessage.asCharArray());
        }
        else
        {
            // Don't enable overflowing _errorsCount:
            _errorsCount = OS_TCP_SOCKET_MAX_ERROR_PRINTOUTS + 10;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocket::getIpAddresses
// Description:
//   Gets a list of ip addresses of the current machine.
//   Consult getifaddrs man page for more details about this technique.
// Author:      AMD Developer Tools Team
// Date:        15/9/2013
// ---------------------------------------------------------------------------
bool osTCPSocket::getIpAddresses(gtVector<gtString>& o_addresses)
{
    char host[NI_MAXHOST];
    ::memset(host, 0, NI_MAXHOST * sizeof(char));

    ifaddrs* ifaddr = NULL;
    int rcAd = ::getifaddrs(&ifaddr);
    bool retVal = (rcAd > -1);
    GT_IF_WITH_ASSERT_EX(retVal, L"::getifaddrs function failed.")
    {
        // If there are no addresses, we successfully return an empty vector:
        retVal = (NULL == ifaddr);

        // Now iterate through the returned linked-list.
        for (ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            // If the address is defined:
            if (ifa->ifa_addr != NULL)
            {
                // Get its family:
                int family = ifa->ifa_addr->sa_family;

                if (family == AF_INET || family == AF_INET6)
                {
                    int rcNm = getnameinfo(ifa->ifa_addr, ((family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

                    // Consider getting at least one name as a success:
                    retVal = (0 == rcNm) || retVal;

                    GT_IF_WITH_ASSERT_EX(0 == rcNm, L"getnameinfo function failed.")
                    {
                        gtString currentAddr;
                        currentAddr.fromASCIIString(host);
                        o_addresses.push_back(currentAddr);
                        retVal = true;
                    }
                    else
                    {
                        gtString logMsg;
                        logMsg.fromASCIIString(gai_strerror(rcNm)).prepend(L"getnameinfo function failed: ");
                        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
                    }
                }
            }
        }

        freeifaddrs(ifaddr);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osTCPSocket::setKeepAlive
// Description:
//   Turns on the TCP KeepAlive setting for this specific socket.
//   Important note: the arguments for this functions are ignored on Linux.
//   This is because on Linux the TCP KeepAlive parameters are user defined
//   parameters. If you wish to change your system's KeepAlive parameters, use:
//   echo <integer value> > /proc/sys/net/ipv4/tcp_keepalive_time
//   echo <integer value> > /proc/sys/net/ipv4/tcp_keepalive_intvl
//   echo <integer value> > /proc/sys/net/ipv4/tcp_keepalive_probes
// Arguments: timeoutMs  - ignored on Linux, see function description above.
//            intervalMs - ignored on Linux, see function description above.
// Author:      AMD Developer Tools Team
// Date:        6/2/2014
// ---------------------------------------------------------------------------
bool osTCPSocket::setKeepAlive(unsigned long timeoutMs, unsigned long intervalMs)
{
    // These two parameters are ignored on Linux, see function description above.
    (void)(timeoutMs);
    (void)(intervalMs);

    bool ret = true;
    int optval = 1;
    socklen_t  optlen = sizeof(optval);

    // Turn on TCP KeepAlive setting for this socket.
    if (setsockopt(_socketDescriptor, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0)
    {
        // Handle failure.
        ret = false;
        OS_OUTPUT_DEBUG_LOG(L"Failed to set TCP Keep Alive to this socket: setsockopt() failed.", OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        osTCPSocket::isConnectionAlive
// Description:
//   Checks if the current connection is not broken.
//   Note: Cureently not implemented on Linux.
// Arguments: isAlive  - a buffer to hold the result.
// Return value: true for success, false otherwise.
// Author:      AMD Developer Tools Team
// Date:        23/7/2014
// ---------------------------------------------------------------------------
bool osTCPSocket::isConnectionAlive(bool& isAlive)
{
    (void)isAlive;
    GT_ASSERT_EX(false, L"This function is currently not implemented on Linux.")
    return false;
}

// ---------------------------------------------------------------------------
// Name:        osTCPSocket::verifyConnectionAfterError
// Description: Check if the error code indicates that the socket disconnected
//              and set the isOpen flag accordingly.
//              This is especially important when using streaming operators that
//              in the absence of exceptions have no mechanism for indicating failure.
// Arguments:
// Return value: na
// Author:      Doron Ofek
// Date:        Dec-29, 2015
// ---------------------------------------------------------------------------
void osTCPSocket::verifyConnectionAfterError(osSystemErrorCode lastSystemError)
{
    switch (lastSystemError)
    {
        // Cases that do not indicate connection loss
        case EAGAIN:        // The socket is marked nonblocking and the receive operation would block, or a receive timeout had been set and

        // the timeout expired before data was received.POSIX.1 - 2001 allows either error to be returned for this case,
        // and does not require these constants to have the same value, so a portable application should check for both possibilities.
        //    case EWOULDBLOCK:   // Same as EAGAIN
        case EFAULT:        // The receive buffer pointer(s) point outside the process's address space.
        case EINTR:         // The receive was interrupted by delivery of a signal before any data were available; see signal(7).
        case EINVAL:        // Invalid argument passed.
        case ENOMEM:        // Could not allocate memory for recvmsg().
        {
            // nothing to do
            break;
        }

        // Cases that indicate connection loss
        case EBADF:         // The argument sockfd is an invalid descriptor.
        case ECONNREFUSED:  // A remote host refused to allow the network connection(typically because it is not running the requested service).
        case ENOTCONN:      // The socket is associated with a connection - oriented protocol and has not been connected(see connect(2) and accept(2)).
        case ENOTSOCK:      // The argument sockfd does not refer to a socket.
        {
            close();
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        osTCPSocket::verifyConnectionAfterError
// Description: Check if the error code indicates that the socket disconnected
//              and set the isOpen flag accordingly.
//              This is especially important when using streaming operators that
//              in the absence of exceptions have no mechanism for indicating failure.
// Arguments:
// Return value: na
// Author:      Doron Ofek
// Date:        Dec-29, 2015
// ---------------------------------------------------------------------------
void osTCPSocket::verifyConnectionAfterError()
{
    osSystemErrorCode lastSystemError = osGetLastSystemError();
    verifyConnectionAfterError(lastSystemError);
}
