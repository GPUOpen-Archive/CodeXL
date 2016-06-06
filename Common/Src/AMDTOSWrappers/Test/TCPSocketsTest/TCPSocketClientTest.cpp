//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ TCPSocketClientTest.cpp ------------------------------

// C++:
#include <iostream>
using namespace std;

// Infra:
#include <GRCryptographicLibrary/crDefaultKeys.h>

// OSWrappers:
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osChannelEncryptor.h>
#include <AMDTOSWrappers/Include/osTCPSocketClient.h>

// The socket client that we are about to test:
osTCPSocketClient socketClient;

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
    cout << "creating the socket client ..... ";
    bool retVal = socketClient.open();
    // Output the OK / Fail message:
    outputSuccessMassage(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        testConnectSocket
// Description: Tests the connection attempt of the client socket.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool testConnectSocket()
{
    cout << "Connecting to server ";
    cout << hostName.asCharArray();
    cout << " port ";
    cout << portNumber;
    cout << "..... ";
    osPortAddress portAddress(hostName, portNumber);
    bool retVal = socketClient.connect(portAddress);
    // Output the OK / Fail message:
    outputSuccessMassage(retVal);
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
    bool retVal = false;

    if (verbose)
    {
        cout << "Writing into the socket ..... ";
    }

    // The message to be written:
    char messageBuf[250] = "This is the message that the client sent";
    unsigned int messageSize = strlen(messageBuf) + 1;

    // Write the message into the buffer:
    retVal = socketClient.write((const gtByte*)messageBuf, messageSize);

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
    gtString clientMsg("This is the encrypted message that the client sent");

    channelEncryptor << clientMsg;

    if (verbose)
    {
        cout << "Wrote data" << endl;
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

    // TO_DO: This is horribly hard coded:
    unsigned long messageSize = 41;

    // Read from the socket:
    retVal = socketClient.read((gtByte*)messageBuf, messageSize);

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
// Name:        testSocketClose
// Description: Tests the socket termination
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool testSocketClose()
{
    cout << "Closing the socket ..... ";
    bool retVal = socketClient.close();
    // Output the OK / Fail message:
    outputSuccessMassage(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        main
// Description: Main test function.
//              Creates a TCP socket client that calls a TCP socket server that
//              is bound to a port 2004. It also sends few messages to the server.
// Arguments:   int argc
//              char* argv[]
// Return Val:  int
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    cout << "\n------ TCPSocketClientTest ------ \n";

    // Create the socket:
    bool rc = testCreateSocket();

    if (rc)
    {
        // Bind the socket to a port:
        rc = testConnectSocket();
    }

    if (rc)
    {
        // Write and read to / from the socket:
        for (int i = 0; rc && i < 100; i++)
        {
            rc = rc && testWriteToSocket(false);
            rc = rc && testReadFromSocket(false);
        }

        cout << "Testing encrypted channel operations ..... ";
        cout.flush();

        // Get the default socket encryption key:
        const crBlowfishEncryptionKey* pDefaultSocketEncryptionKey = crGetSocketsDefaultEncriptionKey();

        if (pDefaultSocketEncryptionKey != NULL)
        {
            // Create a channel encryptor:
            osChannelEncryptor channelEncryptor(socketClient, *pDefaultSocketEncryptionKey);

            // Restart the channel encryptor encryption stream:
            channelEncryptor.restartEncryptionStream();

            for (int i = 0; i < 100, ; i++)
            {
                // Read an encrypted message:
                testWriteToEncryptedSocket(channelEncryptor, false);
                testReadFromEncryptedSocket(channelEncryptor, false);
            }
        }

        cout << "Ended\n";
    }

    // Close the socket:
    testSocketClose();

    cout << "------ Test ended ------ \n\n";

    return 0;
}

