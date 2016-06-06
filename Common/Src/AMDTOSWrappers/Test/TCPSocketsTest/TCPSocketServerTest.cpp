//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ TCPSocketServerTest.cpp ------------------------------

// C++:
#include <iostream>
using namespace std;

// Infra:
#include <GRCryptographicLibrary/crDefaultKeys.h>

// OSWrappers:
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osChannelEncryptor.h>
#include <AMDTOSWrappers/Include/osTCPSocketServerConnectionHandler.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>

// The socket server that we are about to test:
osTCPSocketServer socketServer;

// The socket that will handle the client connection:
osTCPSocketServerConnectionHandler clientCallHandler;

// The used IP address and port number:
const gtString hostName = "192.168.1.103";
unsigned short portNumber = 2004;


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
    cout << "creating the socket server ..... ";
    bool retVal = socketServer.open();
    // Output the OK / Fail message:
    outputSuccessMassage(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        testBindSocket
// Description: Tests the socket bind to a port.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool testBindSocket()
{
    cout << "Binding the socket server";
    cout << hostName.asCharArray();
    cout << "to port ";
    cout << portNumber;
    cout << "..... ";
    osPortAddress portAddress(hostName, portNumber);
    bool retVal = socketServer.bind(portAddress);
    // Output the OK / Fail message:
    outputSuccessMassage(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        testListenToPort
// Description: Tests the socket listen to a port.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool testListenToPort()
{
    cout << "Listening to the port ..... ";
    bool retVal = socketServer.listen(1);
    // Output the OK / Fail message:
    outputSuccessMassage(retVal);
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

    // TO_DO: This is horribly hard coded:
    unsigned long messageSize = 41;

    // Read from the socket:
    retVal = clientCallHandler.read((gtByte*)messageBuf, messageSize);

    if (verbose)
    {
        // Output the OK / Fail message:
        outputSuccessMassage(retVal);

        if (retVal)
        {
            cout << "The read data is: \n";
            cout << "> ";
            cout << (char*)messageBuf << endl;

            cout << endl;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        testReadFromEncryptedSocket
// Description: Tests reading from an encrypted socket.
// Author:      AMD Developer Tools Team
// Date:        21/9/2009
// ---------------------------------------------------------------------------
bool testReadFromEncryptedSocket(osChannelEncryptor& channelEncryptor, bool verbose)
{
    bool retVal = false;

    if (verbose)
    {
        cout << "Reading from the encrypted socket ..... ";
    }

    // Read from the socket:
    gtString readData;
    channelEncryptor >> readData;

    if (verbose)
    {
        cout << "The read data is: \n";
        cout << readData.asCharArray();
        cout << endl;
        cout << endl;
    }

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
    char messageBuf[250] = "This is the message that the server sent";
    unsigned int messageSize = strlen(messageBuf) + 1;

    // Write the message into the buffer:
    retVal = clientCallHandler.write((const gtByte*)messageBuf, messageSize);

    if (verbose)
    {
        // Output the OK / Fail message:
        outputSuccessMassage(retVal);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        testWriteToEncryptedSocket
// Description: Tests writing into an encrypted socket.
// Author:      AMD Developer Tools Team
// Date:        21/9/2009
// ---------------------------------------------------------------------------
bool testWriteToEncryptedSocket(osChannelEncryptor& channelEncryptor, bool verbose)
{
    bool retVal = false;

    if (verbose)
    {
        cout << "Writing into the encrypted socket ..... ";
    }

    // The message to be written:
    gtString clientMsg("This is the encrypted message that the server sent");

    channelEncryptor << clientMsg;

    if (verbose)
    {
        cout << "Wrote data" << endl;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        testAccept
// Description: Tests the socket accept of an incoming socket call.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool testAcceptIncomingClientCall()
{
    cout << "Waiting for an incoming socket client call ..... ";
    bool retVal = socketServer.accept(clientCallHandler);
    cout << "Client call received.\n";
    cout << "Accepting the socket client call ..... ";
    // Output the OK / Fail message:
    outputSuccessMassage(retVal);

    if (retVal)
    {
        osStopWatch stopWatch;
        stopWatch.start();

        // Read from the socket:
        for (int i = 0; i < 100; i++)
        {
            testReadFromSocket(false);
            testWriteToSocket(false);
        }

        double timeIntevalSec = 0;
        stopWatch.getTimeInterval(timeIntevalSec);

        cout << "The read and write operations took: ";
        cout << timeIntevalSec;
        cout << " seconds.\n";
        cout << "Testing encrypted channel operations ..... ";
        cout.flush();

        // Get the default socket encryption key:
        const crBlowfishEncryptionKey* pDefaultSocketEncryptionKey = crGetSocketsDefaultEncriptionKey();

        if (pDefaultSocketEncryptionKey != NULL)
        {
            // Create a channel encryptor:
            osChannelEncryptor channelEncryptor(clientCallHandler, *pDefaultSocketEncryptionKey);

            // Restart the channel encryptor encryption stream:
            channelEncryptor.restartEncryptionStream();

            for (int i = 0; i < 100; i++)
            {
                // Read an encrypted message:
                testReadFromEncryptedSocket(channelEncryptor, false);
                testWriteToEncryptedSocket(channelEncryptor, false);
            }
        }

        cout << "Ended\n";

        // Close the client handler socket:
        clientCallHandler.close();
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
    bool retVal = socketServer.close();
    // Output the OK / Fail message:
    outputSuccessMassage(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        main
// Description: Main test function.
//              Creates a TCP socket server that listens to a port 2004,
//              and accepts a TCP client call and messages.
// Arguments:   int argc
//              char* argv[]
// Return Val:  int
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    cout << "\n------ TCPSocketServerTest ------ \n";

    // Create the socket:
    bool rc = testCreateSocket();

    if (rc)
    {
        // Bind the socket to a port:
        rc = testBindSocket();

        if (rc)
        {
            // Listen to the port:
            rc = testListenToPort();

            if (rc)
            {
                // Accept an incoming socket client call:
                testAcceptIncomingClientCall();
            }
        }
    }

    // Close the socket:
    testSocketClose();

    cout << "------ Test ended ------ \n\n";

    return 0;
}


