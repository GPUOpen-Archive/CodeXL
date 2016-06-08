//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ SMClientTest.cpp ------------------------------

// C++:
#include <iostream>
using namespace std;

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// OSWrappers:
#include <AMDTOSWrappers/Include/osSharedMemorySocketClient.h>

// The name of the shared memory object name:
#define SM_SHARED_MEM_OBJ_NAME "SM_SERVER_TEST"


// The socket client that we are about to test:
static osSharedMemorySocketClient stat_socketClient(SM_SHARED_MEM_OBJ_NAME);


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
    cout << "creating the socket client ..... ";
    bool retVal = stat_socketClient.open();

    // Output the OK / Fail message:
    outputSuccessMassage(retVal);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        testWriteToSocket
// Description: Tests writing into the socket.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool testWriteToSocket(bool verbose)
{
    bool retVal = false;

    if (verbose)
    {
        cout << "Writing into the socket ..... ";
    }

    // The message to be written:
    char messageBuf[250] = "This is the message that the client sent";
    unsigned int messageSize = strlen(messageBuf) + 1;

    // Write the message into the buffer:
    retVal = stat_socketClient.write((const gtByte*)messageBuf, messageSize);

    if (verbose)
    {
        // Output the OK / Fail message:
        outputSuccessMassage(retVal);
    }

    return retVal;
}

bool testReadFromSocket(bool verbose)
{
    bool retVal = false;

    if (verbose)
    {
        cout << "Reading from the socket ..... ";
    }

    // The buffer into which we will read:
    char messageBuf[250];

    // TO_DO: Horribly hard coded:
    int messageSize = 41;

    // Read from the socket:
    retVal = stat_socketClient.read((gtByte*)messageBuf, messageSize);

    if (verbose)
    {
        // Output the OK / Fail message:
        outputSuccessMassage(retVal);
    }

    if (retVal && verbose)
    {
        cout << "The read data is: \n";
        cout << "> ";
        cout << (char*)messageBuf << endl;
        cout << endl;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        testSocketClose
// Description: Tests the socket termination
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool testSocketClose()
{
    cout << "Closing the socket ..... ";
    bool retVal = stat_socketClient.close();

    // Output the OK / Fail message:
    outputSuccessMassage(retVal);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        main
// Description: Main test function.
//              Creates a SM socket client that calls a SM socket server that
//              is bound to a port 2004. It also sends few messages to the server.
// Arguments:   int argc
//              char* argv[]
// Return Val:  int
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    cout << "\n------ SMstat_socketClientTest ------ \n";

    // Create the socket:
    bool rc = testCreateSocket();

    if (rc)
    {
        // Write and read to / from the socket:
        for (int i = 0; i < 10000; i++)
        {
            testReadFromSocket(false);
            testWriteToSocket(false);
        }
    }

    // Close the socket:
    testSocketClose();

    cout << "------ Test ended ------ \n";

    return 0;
}

