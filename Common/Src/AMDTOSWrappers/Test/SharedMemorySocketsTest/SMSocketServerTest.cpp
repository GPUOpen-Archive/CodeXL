//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ SMSocketServerTest.cpp ------------------------------

// C++:
#include <iostream>
using namespace std;

// OSWrappers:
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osSharedMemorySocketServer.h>

// The name of the shared memory object name:
#define SM_SHARED_MEM_OBJ_NAME "SM_SERVER_TEST"

// The socket server that we are about to test:
static osSharedMemorySocketServer stat_socketServer(SM_SHARED_MEM_OBJ_NAME, 2000);


// ---------------------------------------------------------------------------
// Name:        outputSuccessMassage
// Description: Output the OK / Failed message according to the input rc value.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
void outputSuccessMassage(bool rc)
{
    if (rc)
    {
        cout << "OK\n";
    }
    else
    {
        cout << "Failed !!!\n";
    }
}


// ---------------------------------------------------------------------------
// Name:        testCreateSocket
// Description: Tests the socket creation.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool testCreateSocket()
{
    cout << "Creating the socket server ..... ";
    bool retVal = stat_socketServer.open();
    // Output the OK / Fail message:
    outputSuccessMassage(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        testWaitForClientConnection
// Description: Tests the socket wait for client connection functionality.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool testWaitForClientConnection()
{
    cout << "Wait for client connection:";
    bool retVal = stat_socketServer.waitForClientConnection();
    // Output the OK / Fail message:
    outputSuccessMassage(retVal);
    return retVal;
}


bool testReadFromSocket(bool verbose)
{
    if (verbose)
    {
        cout << "Reading from the socket ..... ";
    }

    // The buffer into which we will read:
    char messageBuf[250];

    // TO_DO: Horribly hard coded:
    int messageSize = 41;

    // Read from the socket:
    bool retVal = stat_socketServer.read((gtByte*)messageBuf, messageSize);

    if (verbose)
    {
        // Output the OK / Fail message:
        outputSuccessMassage(retVal);
    }

    if (verbose && retVal)
    {
        cout << "The read data is: \n";
        cout << "> ";
        cout << (char*)messageBuf << endl;
        cout << endl;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        testConnectSocket
// Description: Tests writing into the socket.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool testWriteToSocket(bool verbose)
{
    if (verbose)
    {
        cout << "Writing into the socket ..... ";
    }

    // The message to be written:
    char messageBuf[250] = "This is the message that the server sent";
    unsigned int messageSize = strlen(messageBuf) + 1;

    // Write the message into the buffer:
    bool retVal = stat_socketServer.write((const gtByte*)messageBuf, messageSize);

    if (verbose)
    {
        // Output the OK / Fail message:
        outputSuccessMassage(retVal);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        testSocketClose
// Description: Tests the socket termination.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool testSocketClose()
{
    cout << "Closing the socket ..... ";
    bool retVal = stat_socketServer.close();
    // Output the OK / Fail message:
    outputSuccessMassage(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        main
// Description: Main test function.
//              Creates a SM socket server that listens to a port 2004,
//              and accepts a SM client call and messages.
// Arguments:   int argc
//              char* argv[]
// Return Val:  int
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    cout << "\n------ SMSocketServerTest ------ \n";

    // Create the socket:
    bool rc = testCreateSocket();

    if (rc)
    {
        // Bind the socket to a port:
        rc = testWaitForClientConnection();

        if (rc)
        {
            osStopWatch stopWatch;
            stopWatch.start();

            // Read from the socket:
            for (int i = 0; i < 10000; i++)
            {
                testWriteToSocket(false);
                testReadFromSocket(false);
            }

            double timeIntevalSec = stopWatch.getTimeInterval();

            cout << "The read and write operations took: ";
            cout << timeIntevalSec;
            cout << " seconds.\n\n";

        }
    }

    // Close the socket:
    testSocketClose();

    cout << "------ Test ended ------ \n";

    return 0;
}


