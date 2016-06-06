//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osNetworkAdapter.cpp
///
//=====================================================================

//------------------------------ osNetworkAdapter.cpp ------------------------------

#include <cstdlib>

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <Iphlpapi.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

// Local:
#include <AMDTOSWrappers/Include/osNetworkAdapter.h>


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
//  The adapter names we get are of the form: {B9601C54-FE71-4CD5-BD1B-375E67A51D3E}.
//  To get the adapter "human readable name", you can look in the registry under
//  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Network for the adapter name.
// ---------------------------------------------------------------------------
bool osGetCurrentMachineNetworkAdapters(gtPtrVector<osNetworkAdapter*>& networkAdapters)
{
    bool retVal = false;

    // Clear the output vector:
    networkAdapters.clear();

    // Load the IP Helper API library:
    HMODULE hIpHlpApi = ::LoadLibrary(L"IpHlpApi.dll");
    GT_IF_WITH_ASSERT(hIpHlpApi != NULL)
    {
        // Get the GetAdaptersInfo function pointer:
        typedef DWORD (WINAPI * GetAdaptersInfoPROC)(PIP_ADAPTER_INFO, PULONG);
        GetAdaptersInfoPROC pGetAdaptersInfo = (GetAdaptersInfoPROC)GetProcAddress(hIpHlpApi, "GetAdaptersInfo");

        if (pGetAdaptersInfo != NULL)
        {
            // Get the required size for the GetAdaptersInfo output buffer:
            ULONG bufferSize = 0;
            pGetAdaptersInfo(NULL, &bufferSize);
            GT_IF_WITH_ASSERT(0 < bufferSize)
            {
                // Allocate a buffer of the required size:
                BYTE* pBuffer = new BYTE[bufferSize];
                GT_IF_WITH_ASSERT(pBuffer != NULL)
                {
                    // Get the adapters information:
                    DWORD rc1 = pGetAdaptersInfo((PIP_ADAPTER_INFO)pBuffer, &bufferSize);
                    GT_IF_WITH_ASSERT(rc1 == ERROR_SUCCESS)
                    {
                        retVal = true;

                        // Iterate the available network adapters:
                        PIP_ADAPTER_INFO pCurrOSAdapter = (PIP_ADAPTER_INFO)pBuffer;

                        while (pCurrOSAdapter != NULL)
                        {
                            // Allocate a network adapter wrapper class:
                            osNetworkAdapter* pCurrAdapter = new osNetworkAdapter;
                            GT_IF_WITH_ASSERT(pCurrAdapter != NULL)
                            {
                                // Fill the adapter name:
                                pCurrAdapter->_name.fromASCIIString(pCurrOSAdapter->AdapterName);

                                // If the adapter's MAC address is valid:
                                if ((pCurrOSAdapter->Address[0] != 0) || (pCurrOSAdapter->Address[1] != 0) ||
                                    (pCurrOSAdapter->Address[2] != 0) || (pCurrOSAdapter->Address[3] != 0) ||
                                    (pCurrOSAdapter->Address[4] != 0) || (pCurrOSAdapter->Address[5] != 0))
                                {
                                    // Fill the adapter MAC address:
                                    pCurrAdapter->_MACAddress.appendFormattedString(L"%02X-%02X-%02X-%02X-%02X-%02X", pCurrOSAdapter->Address[0], pCurrOSAdapter->Address[1], pCurrOSAdapter->Address[2], pCurrOSAdapter->Address[3], pCurrOSAdapter->Address[4], pCurrOSAdapter->Address[5]);
                                }
                                else
                                {
                                    // Mark that the MAC address is invalid:
                                    pCurrAdapter->_MACAddress.makeEmpty();
                                }

                                if (pCurrOSAdapter->IpAddressList.IpAddress.String[0] != 0)
                                {
                                    const size_t nullTerminatedIpStringBuffLength = 16;
                                    char nullTerminatedIpAddress[nullTerminatedIpStringBuffLength];
                                    memset(nullTerminatedIpAddress, 0, nullTerminatedIpStringBuffLength);
                                    strcpy_s(nullTerminatedIpAddress, nullTerminatedIpStringBuffLength, pCurrOSAdapter->IpAddressList.IpAddress.String);
                                    pCurrAdapter->_TCPIPAddress.fromASCIIString(nullTerminatedIpAddress);

                                }

                                // Add the current adapter to the output adapters list:
                                networkAdapters.push_back(pCurrAdapter);
                            }

                            // Advance to the next network adapter:
                            pCurrOSAdapter = pCurrOSAdapter->Next;
                        }
                    }

                    // Release the allocated buffer:
                    delete[] pBuffer;
                }
            }
        }

        // Free the IP Helper API library:
        ::FreeLibrary(hIpHlpApi);
        hIpHlpApi = NULL;
    }

    return retVal;
}

