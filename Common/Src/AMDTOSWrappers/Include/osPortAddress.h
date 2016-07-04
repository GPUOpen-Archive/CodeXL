//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPortAddress.h
///
//=====================================================================

//------------------------------ osPortAddress.h ------------------------------

#ifndef __OSPORTADDRESS
#define __OSPORTADDRESS

// Forward declarations:
struct sockaddr_in;

// GRBaseTools:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           osPortAddress
// General Description:
//   Represents the address of a port.
//   The port can reside on the local machine, or on the network.
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/12/2003
// ----------------------------------------------------------------------------------
class OS_API osPortAddress
{
public:
    osPortAddress(const gtString& hostName, unsigned short remotePortNumber);
    osPortAddress(const gtASCIIString& hostName, unsigned short remotePortNumber);
    osPortAddress(unsigned short localPortNumber = 0, bool useHostname = true);
    osPortAddress(const sockaddr_in& internetSocketAddress);

    void setAsRemotePortAddress(const gtString& hostName, unsigned short remotePortNumber);
    void setAsLocalPortAddress(unsigned short localPortNumber, bool useHostname = true);
    bool setFromSocaddr(const sockaddr_in& internetSocketAddress);

    const gtString& hostName() const { return _hostName; };
    unsigned short portNumber() const { return _portNumber; };
    bool operator==(const osPortAddress& other) const;
    bool operator!=(const osPortAddress& other) const { return !(operator==(other)); };
    bool isOnSameSubNet(const osPortAddress& other) const;

    bool asSockaddr(sockaddr_in& internetSocketAddress, bool blockingDNSQuery) const;

    bool fromString(const gtString& addressAsString);
    void toString(gtString& addressAsString) const;

private:
    bool asULongAddress(unsigned long& ipAddress) const;

private:
    // The name of the host in which the port resides:
    gtString _hostName;

    // The port number:
    unsigned short _portNumber = 0;
};

bool OS_API osRemotePortAddressFromString(const gtString& i_portAddressAsStr, osPortAddress& o_portAddress);

#endif  // __OSPORTADDRESS
