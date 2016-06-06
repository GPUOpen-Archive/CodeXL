//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPortAddress.cpp
///
//=====================================================================

//------------------------------ osPortAddress.cpp ------------------------------

// Win32:
#include "Winsock2.h"

// GRBaseTools:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
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
// Arguments: internetSocketAddress - The socket address and port number given
//                                    in internet style socket address (of type sockaddr_in)
// Author:      AMD Developer Tools Team
// Date:        4/10/2006
// ---------------------------------------------------------------------------
osPortAddress::osPortAddress(const sockaddr_in& internetSocketAddress)
{
    bool rc = setFromSocaddr(internetSocketAddress);
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
// Arguments: internetSocketAddress - The socket address and port number given as
//                                    internet style socket address (of type sockaddr_in).
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/10/2006
// ---------------------------------------------------------------------------
bool osPortAddress::setFromSocaddr(const sockaddr_in& internetSocketAddress)
{
    bool retVal = false;

    // Translate the socket address into a string containing internet protocol
    // dotted address string (example: 172.23.232.323):
    const char* pHostNameStr = inet_ntoa(internetSocketAddress.sin_addr);

    if (pHostNameStr != NULL)
    {
        // Copy the host name:
        _hostName.fromASCIIString(pHostNameStr);

        // Copy the port number:
        _portNumber = internetSocketAddress.sin_port;

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
// Name:        osPortAddress::getLocalMachineName
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
        // Check if me and other are on the same sub-network.
        // We actually check the first two [0,255] IP addresses components. This simple
        // test should work well on IP Address Classes B and C.
        // (For more details, search the net for "IP Address Classes").
        if ((mySocketAddress.sin_addr.S_un.S_un_b.s_b1 == otherSocketAddress.sin_addr.S_un.S_un_b.s_b1) &&
            (mySocketAddress.sin_addr.S_un.S_un_b.s_b2 == otherSocketAddress.sin_addr.S_un.S_un_b.s_b2))
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osPortAddress::asSockaddr
// Description: Translates my to a port address to internet style
//              socket address (of type sockaddr_in).
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        18/1/2004
// ---------------------------------------------------------------------------
bool osPortAddress::asSockaddr(sockaddr_in& internetSocketAddress, bool blockingDNSQuery) const
{
    bool retVal = false;

    // Use "Internetwork" address family (TCP, UDP, etc):
    internetSocketAddress.sin_family = AF_INET;

    // Convert the port number to TCP/IP network byte order representation
    // (which is "big-endian"):
    internetSocketAddress.sin_port = htons(_portNumber);

    if (internetSocketAddress.sin_port != 0)
    {
        // TO_DO: Delete me if all works well without me:
        // Set the host IP address to be any of the local machine IP addresses:
        // internetSocketAddress.sin_addr.S_un.S_addr = htonl( INADDR_ANY );

        unsigned long ipAddress = INADDR_NONE;
        bool rc = asULongAddress(ipAddress);

        // If the address is not in an "A.B.C.D" format, run a DNS query on it:
        if (!rc)
        {
            // Try to convert from host name:
            // (Example: machine13)

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
                    // Get the host first IP address (note that our thread stores it as 4 characters):
                    ipAddress = *((gtUInt32*)(pDNSQueryThread->_hostAddress));
                    rc = true;
                }
            }

            // Clean up:
            delete pDNSQueryThread;
            pDNSQueryThread = NULL;
        }

        if (rc)
        {
            // Set the host IP address to the remote host address:
            internetSocketAddress.sin_addr.s_addr = ipAddress;
            retVal = true;
        }
    }

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
                if ((portNumber > 0) && (portNumber < 0xFFFF))
                {
                    _portNumber = (unsigned short)portNumber;
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
    ipAddress = INADDR_NONE;

    // TO_DO: Unicode - I think that this code should be removed
#if 0
    // Get the host name as a string pointer:
    LPCTSTR lpHostName = _hostName.asCharArray();

    // Handle unicode strings:
#ifdef _UNICODE
    char hostName[MAX_HOSTNAME_SIZE] = { 0 };
    WideCharToMultiByte(CP_ACP, 0, lpHostName, -1, hostName, sizeof(hostName), NULL, NULL);
#else
    LPCTSTR hostName = lpHostName;
#endif
#endif

    // Try to convert form internet protocol dotted address string:
    // (Example: 172.23.232.323)
    ipAddress = inet_addr(_hostName.asASCIICharArray());

    if (ipAddress != INADDR_NONE)
    {
        retVal = true;
    }

    return retVal;
}




