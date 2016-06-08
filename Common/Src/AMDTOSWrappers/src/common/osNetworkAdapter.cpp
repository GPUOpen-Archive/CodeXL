//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osNetworkAdapter.cpp
///
//=====================================================================

//------------------------------ osNetworkAdapter.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osNetworkAdapter.h>

// ---------------------------------------------------------------------------
// Name:        osGetCurrentMachineIPAddresses
// Description: Returns the local machine's IP addresses from all adapters:
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/5/2010
// ---------------------------------------------------------------------------
bool osGetCurrentMachineIPAddresses(gtVector<gtString>& ipAddresses, bool includeLocalLoopback)
{
    bool retVal = false;

    // Get this machine's network adapters:
    gtPtrVector<osNetworkAdapter*> currentMachineNetworkAdapters;
    bool rcAdap = osGetCurrentMachineNetworkAdapters(currentMachineNetworkAdapters);
    GT_IF_WITH_ASSERT(rcAdap)
    {
        // We consider this a success, since a machine might not have any addresses at all:
        retVal = true;
        int numberOfAdapters = (int)currentMachineNetworkAdapters.size();

        for (int i = 0; i < numberOfAdapters; i++)
        {
            // Sanity check:
            const osNetworkAdapter* pCurrentAdapter = currentMachineNetworkAdapters[i];
            GT_IF_WITH_ASSERT(pCurrentAdapter != NULL)
            {
                // Get the current adapter's address:
                const gtString& currentAdapterAddress = pCurrentAdapter->_TCPIPAddress;

                if (!currentAdapterAddress.isEmpty())
                {
                    // If we do not want the local loopback, make sure the address is not it:
                    static const gtString localLoopbackIPAddressAsString = L"127.0.0.1";

                    if (includeLocalLoopback || (localLoopbackIPAddressAsString != currentAdapterAddress))
                    {
                        // Add the address to the output vector:
                        ipAddresses.push_back(currentAdapterAddress);
                    }
                }
            }
        }
    }

    return retVal;
}

