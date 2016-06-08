//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPortAddress.cpp
///
//=====================================================================

//------------------------------ osPortAddress.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osPortAddress.h>



// ---------------------------------------------------------------------------
// Name:        osRemotePortAddressFromString
// Description: Converts a string of the format "hostname:portnumber" to an osPortAddress
// Arguments:   i_portAddressAsStr - input string
//              o_portAddress - output port address
// Author:      AMD Developer Tools Team
// Date:        8/7/2013
// ---------------------------------------------------------------------------
bool osRemotePortAddressFromString(const gtString& i_portAddressAsStr, osPortAddress& o_portAddress)
{
    bool retVal = false;

    // Make sure the port address is of the form "hostname:port":
    if (1 == i_portAddressAsStr.count(':'))
    {
        // Get the colon position and the last character index:
        int colonPos = i_portAddressAsStr.find(':');
        int lastChar = i_portAddressAsStr.length() - 1;

        // Sanity check:
        if ((0 < colonPos) && (colonPos < lastChar))
        {
            // Get the port number substring:
            gtString portNumberStr;
            i_portAddressAsStr.getSubString(colonPos + 1, lastChar, portNumberStr);

            // Make sure it is a number:
            int portNumberAsInt = -1;
            bool rcInt = portNumberStr.toIntNumber(portNumberAsInt);

            if (rcInt)
            {
                // Make sure it is in the correct range to be an unsigned short:
                if ((-1 < portNumberAsInt) && (0xffff >= portNumberAsInt))
                {
                    // Get the hostname substring:
                    gtString hostName;
                    i_portAddressAsStr.getSubString(0, colonPos - 1, hostName);

                    // Sanity check:
                    if (!hostName.isEmpty())
                    {
                        // Set the success value:
                        retVal = true;

                        // Set the output value:
                        o_portAddress.setAsRemotePortAddress(hostName, (unsigned short)portNumberAsInt);
                    }
                }
            }
        }
    }

    return retVal;
}

