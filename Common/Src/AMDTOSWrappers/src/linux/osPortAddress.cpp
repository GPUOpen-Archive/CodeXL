//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPortAddress.cpp
///
//=====================================================================

//------------------------------ osPortAddress.cpp ------------------------------

// Standard C:
#include <string.h>

// POSIX:
#include <netdb.h>
#include <arpa/inet.h>

// GRBaseTools:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osDNSQueryThread.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osTCPSocket.h>

// The maximal host name size (in chars):
#define MAX_HOSTNAME_SIZE 250


// ---------------------------------------------------------------------------
// Name:        osPortAddress::osPortAddress
// Description: Constructor - Represents a remote machine port.
// Arguments:   hostName - The host on which the port resides.
//              remotePortNumber - The port number on the local machine.
// Author:      AMD Developer Tools Team
// Date:        3/1/2004
// ---------------------------------------------------------------------------
osPortAddress::osPortAddress(const gtString& hostName, unsigned short remotePortNumber)
{
    setAsRemotePortAddress(hostName, remotePortNumber);
}

// ---------------------------------------------------------------------------
// Name:        osPortAddress::osPortAddress
// Description: Constructor - Represents a remote machine port. (uses ASCII string)
// Arguments:   hostName - The host on which the port resides.
//              remotePortNumber - The port number on the local machine.
// Author:      AMD Developer Tools Team
// Date:        13/9/2010
// ---------------------------------------------------------------------------
osPortAddress::osPortAddress(const gtASCIIString& hostName, unsigned short remotePortNumber)
{
    // Convert the host name to a unicode string:
    gtString hostNameUnicode;
    hostNameUnicode.fromASCIIString(hostName.asCharArray());
    setAsRemotePortAddress(hostNameUnicode, remotePortNumber);
}


// ---------------------------------------------------------------------------
// Name:        osPortAddress::osPortAddress
// Description: Constructor - Represents a local machine port.
// Arguments:   localPortNumber - The port number on the local machine
// Author:      AMD Developer Tools Team
// Date:        3/1/2004
// ---------------------------------------------------------------------------
osPortAddress::osPortAddress(unsigned short localPortNumber, bool useHostname)
{
    setAsLocalPortAddress(localPortNumber, useHostname);
}


// ---------------------------------------------------------------------------
// Name:        osPortAddress::osPortAddress
// Description: Constructor
// Arguments: socketAddress - The socket address and port number as sockaddr_in.
// Author:      AMD Developer Tools Team
// Date:        4/10/2006
// ---------------------------------------------------------------------------
osPortAddress::osPortAddress(const sockaddr_in& socketAddress)
{
    bool rc = setFromSocaddr(socketAddress);
    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        osPortAddress::setAsRemotePortAddress
// Description: Set myself to be a remote port address.
// Arguments:   hostName - The host on which the port resides.
//              remotePortNumber - The port number on the local machine.
// Author:      AMD Developer Tools Team
// Date:        16/5/2004
// ---------------------------------------------------------------------------
void osPortAddress::setAsRemotePortAddress(const gtString& hostName, unsigned short remotePortNumber)
{
    _hostName = hostName;
    _portNumber = remotePortNumber;
}


// ---------------------------------------------------------------------------
// Name:        osPortAddress::setAsLocalPortAddress
// Description: Set myself to be a local machine port address.
// Arguments:   localPortNumber - The port number on the local machine.
// Author:      AMD Developer Tools Team
// Date:        16/5/2004
// ---------------------------------------------------------------------------
void osPortAddress::setAsLocalPortAddress(unsigned short localPortNumber, bool useHostname)
{
    if (!useHostname)
    {
        // Will receive true if we find a valid, non-hostname address.
        bool foundValid = false;

        gtVector<gtString> ipAddresses;
        bool rcAddrs = osTCPSocket::getIpAddresses(ipAddresses);
        GT_ASSERT(rcAddrs);

        if (rcAddrs && !ipAddresses.empty())
        {
            bool foundPreferred = false;
            static const gtString s_localhostName1 = L"127.0.0.1";
            static const gtString s_localhostName2 = L"localhost";
            static const gtString s_hostnameEnvVarName = L"HOSTNAME";
            gtString hostnameEnvVarValue;
            bool rcEnv = osGetCurrentProcessEnvVariableValue(s_hostnameEnvVarName, hostnameEnvVarValue);

            // Take the first address which is not the localhost. If HOSTNAME is set to something else,
            // prefer that one.
            int numberOfAddresses = (int)ipAddresses.size();

            for (int i = 0; (i < numberOfAddresses) && (!foundPreferred); i++)
            {
                // Ignore the localhost addresses:
                const gtString& currentAddr = ipAddresses[i];

                if ((currentAddr != s_localhostName1) && (currentAddr != s_localhostName2))
                {
                    // Is this an IPv4 Address?
                    static const gtString s_allowedIPv4Chars = L"0123456789.";

                    if ((3 == currentAddr.count('.')) && (currentAddr.onlyContainsCharacters(s_allowedIPv4Chars)))
                    {
                        // Select the first IPv4 address we find (but continue looking for the preferred address):
                        if (!foundValid)
                        {
                            _hostName = currentAddr;
                            foundValid = true;
                        }
                    }

                    // If we got a preferred address, compare to it:
                    if (rcEnv && (currentAddr == hostnameEnvVarValue))
                    {
                        // The preferred address is always considered valid (as long as it is not a localhost address):
                        foundValid = true;
                        foundPreferred = true;
                        _hostName = currentAddr;
                    }
                }
            }
        }

        // If we didn't find a valid IP address, fall back on the hostname function:
        useHostname = !foundValid;
    }

    if (useHostname)
    {
        // Set the local machine name as the host name:
        bool rc = osGetLocalMachineName(_hostName);
        GT_ASSERT(rc);
    }

    // Set the port number:
    _portNumber = localPortNumber;
}


// ---------------------------------------------------------------------------
// Name:        osPortAddress::setFromSocaddr
// Description: Set my address from a socket address given as a sockaddr_in structure.
// Arguments: socketAddress - The socket address and port number as sockaddr_in.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/10/2006
// ---------------------------------------------------------------------------
bool osPortAddress::setFromSocaddr(const sockaddr_in& socketAddress)
{
    bool retVal = false;

    // Translate the socket address into a string containing internet protocol
    // dotted address string (example: 172.23.232.323):
    const char* pHostNameStr = inet_ntoa(socketAddress.sin_addr);

    if (pHostNameStr != NULL)
    {
        // Copy the host name:
        _hostName.fromASCIIString(pHostNameStr);

        // Copy the port number:
        _portNumber = socketAddress.sin_port;

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osPortAddress::operator==
// Description: Comparison operator - returns true iff this port address represents
//              the same port address as other.
// Author:      AMD Developer Tools Team
// Date:        2/11/2004
// ---------------------------------------------------------------------------
bool osPortAddress::operator==(const osPortAddress& other) const
{
    // Check if the port numbers are identical:
    bool condition1 = (_portNumber == other._portNumber);

    // Check if they are both on the same host:
    bool condition2 = (_hostName == other._hostName);

    // Both ports represents the same port address iff the above conditions
    // were satisfied:
    bool retVal = condition1 && condition2;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osPortAddress::isOnSameSubNet
// Description: Checks if another port address resides in the same sub net as this port address.
// Arguments:   other - The other port address.
// Return Val:  bool - true iff the two port addresses reside on the same sub network.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool osPortAddress::isOnSameSubNet(const osPortAddress& other) const
{
    bool retVal = false;

    // Get my and other addresses as sockaddr_in structures:
    sockaddr_in mySocketAddress;
    sockaddr_in otherSocketAddress;
    bool rc1 = this->asSockaddr(mySocketAddress, false);
    bool rc2 = other.asSockaddr(otherSocketAddress, false);
    GT_IF_WITH_ASSERT(rc1 && rc2)
    {
        // Get both address as strings in standard numbers-and-dots notation:
        gtString mySockAddrAsString;
        mySockAddrAsString.fromASCIIString(inet_ntoa(mySocketAddress.sin_addr));
        gtString otherSockAddrAsString;
        otherSockAddrAsString.fromASCIIString(inet_ntoa(otherSocketAddress.sin_addr));

        // Read the string IP numbers into integers:
        int mySockAddrAsArray[4];
        int otherSockAddrAsArray[4];

        int rc3 = sscanf(mySockAddrAsString.asASCIICharArray(), "%d.%d.%d.%d", &(mySockAddrAsArray[0]), &(mySockAddrAsArray[1]), &(mySockAddrAsArray[2]), &(mySockAddrAsArray[3]));
        int rc4 = sscanf(otherSockAddrAsString.asASCIICharArray(), "%d.%d.%d.%d", &(otherSockAddrAsArray[0]), &(otherSockAddrAsArray[1]), &(otherSockAddrAsArray[2]), &(otherSockAddrAsArray[3]));
        GT_IF_WITH_ASSERT(rc3 && rc4)
        {
            // Check if me and other are on the same sub-network.
            // We actually check the first two [0,255] IP addresses components. This simple
            // test should work well on IP Address Classes B and C.
            // (For more details, search the net for "IP Address Classes").
            if ((mySockAddrAsArray[0] == otherSockAddrAsArray[0]) && (mySockAddrAsArray[1] == otherSockAddrAsArray[1]))
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osPortAddress::asSockaddr
// Description: Translates my to a port address of type sockaddr_in.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        18/1/2004
// ---------------------------------------------------------------------------
bool osPortAddress::asSockaddr(sockaddr_in& socketAddress, bool blockingDNSQuery) const
{
    bool retVal = false;

    // Create a thread to query the DNS for us:
    osDNSQueryThread* pDNSQueryThread = new osDNSQueryThread;

    pDNSQueryThread->_hostName = _hostName;

    if (blockingDNSQuery)
    {
        // Call the function on this thread, to block:
        pDNSQueryThread->entryPoint();
    }
    else // !blockingDNSQuery
    {
        // Execute the DNS query thread:
        pDNSQueryThread->execute();

        // Wait for the thread to finish, or for the timeout to pass
        osWaitForFlagToTurnOff(pDNSQueryThread->_isDNSQueryPending, OS_DNS_QUERY_THREAD_TIMEOUT);
    }

    if (!pDNSQueryThread->_isDNSQueryPending)
    {
        GT_IF_WITH_ASSERT((pDNSQueryThread->_hostAddressLength > 0) && (pDNSQueryThread->_hostAddress != NULL))
        {
            // Clear the struct. If we do not do this, we require elevation to make this operation on Mac OS X.
            // See http://lists.e-advies.nl/pipermail/openradius-list/2002-August/000215.html:
            ::memset(&socketAddress, 0, sizeof(socketAddress));

            // Copy the host name into the output internet socket address:
            ::memcpy(&socketAddress.sin_addr.s_addr, pDNSQueryThread->_hostAddress, pDNSQueryThread->_hostAddressLength);

            // Set the address type to "Internetwork" address family (TCP, UDP, etc):
            socketAddress.sin_family = PF_INET;

            // Translate the port number to network byte order and copy it into the output internet socket address:
            socketAddress.sin_port = htons(_portNumber);

            retVal = true;
        }
    }

    // Clean up:
    delete pDNSQueryThread;
    pDNSQueryThread = NULL;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osPortAddress::fromString
// Description: Acquires the osPortAddress from a string in the form "www.hostname.com:4242"
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/9/2008
// ---------------------------------------------------------------------------
bool osPortAddress::fromString(const gtString& addressAsString)
{
    bool retVal = false;

    if (addressAsString.count(':') == 1)
    {
        int colonLocation = addressAsString.find(':');

        if ((colonLocation > 0) && (colonLocation < addressAsString.length() - 1))
        {
            gtString portAsString;
            addressAsString.getSubString(colonLocation + 1, -1, portAsString);

            int portNumber;

            if (portAsString.toIntNumber(portNumber))
            {
                if ((portNumber > 0) && (portNumber < 65536))
                {
                    _portNumber = portNumber;
                    addressAsString.getSubString(0, colonLocation - 1, _hostName);
                    retVal = true;
                }
            }
        }
    }

    if (!retVal)
    {
        _hostName.makeEmpty();
        _portNumber = 0;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osPortAddress::toString
// Description: Sets addressAsString to hold a string representing the portaddress
//              in the format "www.hostname.com:4242"
// Author:      AMD Developer Tools Team
// Date:        4/9/2008
// ---------------------------------------------------------------------------
void osPortAddress::toString(gtString& addressAsString) const
{
    addressAsString.makeEmpty();

    if (!_hostName.isEmpty())
    {
        addressAsString.appendFormattedString(L"%ls:%u", _hostName.asCharArray(), _portNumber);
    }
}

// ---------------------------------------------------------------------------
// Name:        osPortAddress::getWin32IPAddress
// Description: Returns the port IP address in an unsigned long format.
//              (A 32-bit number in the network byte order used in TCP/IP networks).
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// Implementation Notes:
//  a. Try to convert from internet protocol dotted address string.
//  b. If a fails - try to convert from host name.
// ---------------------------------------------------------------------------
bool osPortAddress::asULongAddress(unsigned long& ipAddress) const
{
    bool retVal = false;
    ipAddress = 0;

    int periodCount = _hostName.count('.');

    // Try and convert from IP address string (eg 96.102.11.75)
    if (periodCount <= 3)
    {
        int currentPeriod = _hostName.find('.');
        int prevPeriod = 0;
        gtString classAAsString;
        gtString classBAsString;
        gtString classCAsString;
        gtString classDAsString;
        int classA = -2;
        int classB = -2;
        int classC = -2;
        int classD = -2;
        _hostName.getSubString(prevPeriod, currentPeriod - 1, classAAsString);
        bool successfulConversions = classAAsString.toIntNumber(classA);

        // Read as many classes as are in the string:
        if (periodCount > 0)
        {
            prevPeriod = currentPeriod + 1;
            currentPeriod = _hostName.find('.', currentPeriod + 1);
            _hostName.getSubString(prevPeriod, currentPeriod - 1, classBAsString);
            successfulConversions = successfulConversions && classBAsString.toIntNumber(classB);
        }

        if (periodCount > 1)
        {
            prevPeriod = currentPeriod + 1;
            currentPeriod = _hostName.find('.', currentPeriod + 1);
            _hostName.getSubString(prevPeriod, currentPeriod - 1, classCAsString);
            successfulConversions = successfulConversions && classCAsString.toIntNumber(classC);
        }

        if (periodCount > 2)
        {
            prevPeriod = currentPeriod + 1;
            currentPeriod = _hostName.find('.', currentPeriod + 1);
            _hostName.getSubString(prevPeriod, currentPeriod - 1, classDAsString);
            successfulConversions = successfulConversions && classDAsString.toIntNumber(classD);
        }

        // If we got less than one class in the string, the last one contains the information for all the other,
        // where the significant bits are the larger (lower letter) classes. EG if the string is 10.258 then
        // A = 10, B = 258 / 256^2 = 0, C = 258 mod 256^2 / 256 = 1, D = 258 mod 256 = 2 so 10.258 is 10.0.1.2.
        if (classB < 0)
        {
            classB = classA % (256 * 256 * 256);
            classA = classA / (256 * 256 * 256);
        }

        if (classC < 0)
        {
            classC = classB % (256 * 256);
            classB = classB / (256 * 256);
        }

        if (classD < 0)
        {
            classD = classC % 256;
            classC = classC / 256;
        }

        if (successfulConversions && (classA < 256) && (classB < 256) && (classC < 256) && (classD < 256) &&
            (classA >= 0) && (classB >= 0) && (classC >= 0) && (classD >= 0))
        {
            ipAddress = classA | (classB << 8) | (classC << 16) | (classD << 24);
            retVal = true;
        }
    }

    return retVal;
}


