//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osNetworkAdapter.cpp
///
//=====================================================================

//------------------------------ osNetworkAdapter.cpp ------------------------------

// Standard C:
#include <limits.h>
#include <string.h>

// POSIX:
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

// Local:
#include <AMDTOSWrappers/Include/osNetworkAdapter.h>

// MAC OS X
#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    #include <net/if_dl.h>
    #include <sys/sysctl.h>
    #include <netdb.h>
#endif // Mac


// ---------------------------------------------------------------------------
// Name:        osFillNetworkAdapterDetails
// Description: Inputs a network adapter name and fills its details.
// Arguments: adapterName - The input network adapter name.
//            adapterDetails - Will get the network adapter details.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/12/2007
// ---------------------------------------------------------------------------
bool osFillNetworkAdapterDetails(const gtASCIIString& adapterName, osNetworkAdapter& adapterDetails)
{
    bool retVal = false;

    // Fill the adapter name:
    adapterDetails._name.fromASCIIString(adapterName.asCharArray());

    unsigned char* pMACAddressStr = NULL;
    const char* pTCPIPAddressStr = NULL;
    bool rcMAC = false;

#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
    // Create a dummy socket on which we can call ioctls to get the network adapter details:
    int socketFD = ::socket(AF_INET, SOCK_DGRAM, 0);
    GT_IF_WITH_ASSERT(socketFD != -1)
    {
        // Initialize an interface request structure that will be used to query the network
        // adapter parameters:
        struct ifreq hardwareInterfaceRequest;
        ::memset(&hardwareInterfaceRequest, 0, sizeof(struct ifreq));
        ::strcpy(hardwareInterfaceRequest.ifr_name, adapterName.asCharArray());

        // Perform the network adapeter query:
        int rc1 = ::ioctl(socketFD, SIOCGIFHWADDR, &hardwareInterfaceRequest);
        GT_IF_WITH_ASSERT(0 <= rc1)
        {
            // Get the network adapter MAC address:
            pMACAddressStr = (unsigned char*)hardwareInterfaceRequest.ifr_ifru.ifru_hwaddr.sa_data;
        }

        struct ifreq softwareInterfaceRequest;
        ::memset(&softwareInterfaceRequest, 0, sizeof(struct ifreq));
        ::strcpy(softwareInterfaceRequest.ifr_name, adapterName.asCharArray());

        // Perform the network adapeter query:
        int rc2 = ::ioctl(socketFD, SIOCGIFADDR, &softwareInterfaceRequest);

        if (0 <= rc2)
        {
            in_addr& ipAddress = (((sockaddr_in&)(softwareInterfaceRequest.ifr_ifru.ifru_addr)).sin_addr);
            pTCPIPAddressStr = (const char*)inet_ntoa(ipAddress);
        }

        // Close the dummy socket:
        ::close(socketFD);
    }

#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    int mib[6], len;

    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_LINK;
    mib[4] = NET_RT_IFLIST;
    mib[5] = if_nametoindex(adapterName.asCharArray());

    if (sysctl(mib, 6, NULL, (size_t*)&len, NULL, 0) >= 0)
    {
        char* macbuf = (char*) malloc(len);

        if (sysctl(mib, 6, macbuf, (size_t*)&len, NULL, 0) >= 0)
        {
            struct if_msghdr* ifm = (struct if_msghdr*)macbuf;
            struct sockaddr_dl* sdl = (struct sockaddr_dl*)(ifm + 1);
            pMACAddressStr = (unsigned char*)LLADDR(sdl);
        }

        free(macbuf);
    }

    // Create a dummy socket on which we can call ioctl to get the network adapter details:
    int socketFD = ::socket(AF_INET, SOCK_DGRAM, 0);
    GT_IF_WITH_ASSERT(socketFD != -1)
    {
        struct ifreq softwareInterfaceRequest;
        ::memset(&softwareInterfaceRequest, 0, sizeof(struct ifreq));
        ::strcpy(softwareInterfaceRequest.ifr_name, adapterName.asCharArray());

        // Perform the network adapeter query:
        int rc1 = ::ioctl(socketFD, SIOCGIFADDR, &softwareInterfaceRequest);

        if (0 <= rc1)
        {
            in_addr& ipAddress = (((sockaddr_in&)(softwareInterfaceRequest.ifr_ifru.ifru_addr)).sin_addr);
            pTCPIPAddressStr = (const char*)inet_ntoa(ipAddress);
        }
    }

    // Close the dummy socket:
    ::close(socketFD);
#else
#error Unknown Linux Variant!
#endif // AMDT_LINUX_VARIANT

    GT_IF_WITH_ASSERT(pMACAddressStr != NULL)
    {
        // If the adapter's MAC address is valid:
        if ((pMACAddressStr[0] != 0) || (pMACAddressStr[1] != 0) ||
            (pMACAddressStr[2] != 0) || (pMACAddressStr[3] != 0) ||
            (pMACAddressStr[4] != 0) || (pMACAddressStr[5] != 0))
        {
            // Fill the adapter MAC address string:
            gtASCIIString macAddressAscii;
            macAddressAscii.appendFormattedString("%02X-%02X-%02X-%02X-%02X-%02X", pMACAddressStr[0], pMACAddressStr[1], pMACAddressStr[2], pMACAddressStr[3], pMACAddressStr[4], pMACAddressStr[5]);
            adapterDetails._MACAddress.fromASCIIString(macAddressAscii.asCharArray());
        }
        else
        {
            // Mark that the MAC address is invalid:
            adapterDetails._MACAddress.makeEmpty();
        }

        rcMAC = true;
    }

    if (pTCPIPAddressStr != NULL)
    {
        adapterDetails._TCPIPAddress.fromASCIIString(pTCPIPAddressStr);
    }

    // An adapter can have no TCP / IP address, so don't consider a missing IP address to be a fault:
    retVal = rcMAC;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetCurrentMachineNetworkAdapters
// Description:
//   Retrieves a list of network adapters installed on the local machine.
//   The caller should release the network adapters vector memory.
// Arguments: networkAdapters - Will get the network adapters list.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/12/2007
// Implementation notes:
// - From if_nameindex man page:
//   The  if_nameindex()  function shall return an array of if_nameindex structures, one structure per interface. The
//   end of the array is indicated by a structure with an if_index field of zero and an if_name field of NULL.
//   Applications should call if_freenameindex() to release the memory that may  be  dynamically  allocated  by  this
//   function, after they have finished using it.
// ---------------------------------------------------------------------------
bool osGetCurrentMachineNetworkAdapters(gtPtrVector<osNetworkAdapter*>& networkAdapters)
{
    bool retVal = false;

    // Clear the output vector:
    networkAdapters.clear();

    // Get a list of network adapters:
    struct if_nameindex* pNetworkAdaptersOSVector = ::if_nameindex();
    GT_IF_WITH_ASSERT(pNetworkAdaptersOSVector != NULL)
    {
        retVal = true;

        // Iterate the network adapters:
        struct if_nameindex* pCurrentOSAdapter = pNetworkAdaptersOSVector;

        while (pCurrentOSAdapter->if_index != 0)
        {
            // Create a network adapter wrapper class:
            osNetworkAdapter* pCurrentNetworkAdapter = new osNetworkAdapter;
            GT_IF_WITH_ASSERT(pCurrentNetworkAdapter)
            {
                // Fill the current adapter details:
                gtASCIIString adapterName = pCurrentOSAdapter->if_name;
                bool rc1 = osFillNetworkAdapterDetails(adapterName, *pCurrentNetworkAdapter);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // Add the current adapter to the output adapters list:
                    networkAdapters.push_back(pCurrentNetworkAdapter);
                }
            }

            // Advance to the next network adapter:
            pCurrentOSAdapter++;
        }

        // Clean up OS memory:
        ::if_freenameindex(pNetworkAdaptersOSVector);
    }

    return retVal;
}


