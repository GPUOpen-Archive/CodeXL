//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTCPSocketServer.cpp
///
//=====================================================================

//------------------------------ osTCPSocketServer.cpp ------------------------------

// Local:
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>


// ---------------------------------------------------------------------------
// Name:        osCanAddressBeUsedByTCPSocketServer
// Description: Returns true iff testedAddress can be used by this computer to
//              accept TCP connections
// Author:      AMD Developer Tools Team
// Date:        17/5/2010
// ---------------------------------------------------------------------------
bool osCanAddressBeUsedByTCPSocketServer(const osPortAddress& testedAddress)
{
    bool retVal = false;

    static osTCPSocketServer testingSocketServer;

    // Clean any connections previously used:
    if (testingSocketServer.isOpen())
    {
        testingSocketServer.close();
    }

    bool rc = testingSocketServer.open();

    if (rc)
    {
        // Bind the server to the input port:
        rc = testingSocketServer.bind(testedAddress);

        if (rc)
        {
            // Listen to the port - allow 1 unhanded pending connections.
            bool rcListen = testingSocketServer.listen(1);

            if (rcListen)
            {
                retVal = true;
            }
        }

        testingSocketServer.close();
    }

    return retVal;
}

