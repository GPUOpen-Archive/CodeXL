//------------------------------ TCPSocketClientTest.cpp ------------------------------

// include Google Test header
#include <gtest/gtest.h>

// OSWrappers:
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTCPSocketClient.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>
#include <AMDTOSWrappers/Include/osNetworkAdapter.h>


// The used IP address and port number:
const gtString hostName = L"127.0.0.1";
unsigned short portNumber = 2015;

const gtString msg1FromClient = L"Client says: I'm looking for someone.";
const gtString msg1FromServer = L"Server says: Looking? Found someone, you have, I would say, hmmm?";

const gtString msg2FromClient = L"Client says: They tried and failed?";
const gtString msg2FromServer = L"Server says: They tried and died!";

const gtString msg3FromClient = L"Client says: A hospital? what is it?";
const gtString msg3FromServer = L"Server says: It's a big building with patients, but that's not important right now.";

// ---------------------------------------------------------------------------
// Test the open() method
// ---------------------------------------------------------------------------
TEST(TCPSocketTest, Open)
{
    osTCPSocketClient socketClient;
    bool retVal = socketClient.open();
    EXPECT_EQ(true, retVal);
}


// ---------------------------------------------------------------------------
// A thread that creates a server socket, waits for a client to connect and then
// read a string from the client and write a string to the client.
// This is used in the test case 'ReadAndWriteStrings' below.
// ---------------------------------------------------------------------------
class TCPSocketServerThread : public osThread
{
public:
    TCPSocketServerThread(const gtString& threadName, bool syncTermination = false)
        : osThread(threadName, syncTermination) {}
protected:
    int entryPoint()
    {
        osTCPSocketServerConnectionHandler clientCallHandler;
        osPortAddress serverAddress(hostName, portNumber);
        osTCPSocketServer socketServer;
        bool retVal = socketServer.open();
        retVal = socketServer.bind(serverAddress);
        EXPECT_EQ(true, retVal);

        retVal = socketServer.listen(1);
        EXPECT_EQ(true, retVal);

        retVal = socketServer.accept(clientCallHandler);
        EXPECT_EQ(true, retVal);

        // Exchange 1st pair of messages
        retVal = exchangeMessagesWithClient(clientCallHandler, msg1FromClient, msg1FromServer);

        // Exchange 2nd pair of messages
        if (true == retVal)
        {
            retVal = exchangeMessagesWithClient(clientCallHandler, msg2FromClient, msg2FromServer);
        }

        // Exchange 3rd pair of messages
        if (true == retVal)
        {
            retVal = exchangeMessagesWithClient(clientCallHandler, msg3FromClient, msg3FromServer);
        }

        retVal = socketServer.close();
        EXPECT_EQ(true, retVal);

        return 0;
    }

    bool exchangeMessagesWithClient(osTCPSocketServerConnectionHandler& connectedServerSocket,
                                    const gtString& expectedClientMsg,
                                    const gtString& serverMsg)
    {
        gtString fromClient;
        bool retVal = connectedServerSocket.readString(fromClient);
        EXPECT_EQ(true, retVal);
        EXPECT_TRUE(fromClient == expectedClientMsg);

        if (true == retVal)
        {
            retVal = connectedServerSocket.writeString(serverMsg);
            EXPECT_EQ(true, retVal);
        }

        return retVal;
    }
};

// ---------------------------------------------------------------------------
// Run a thread that creates a server socket. In the main thread create a client
// socket and test its connection to the server socket, writing and reading strings
// across the connection.
// ---------------------------------------------------------------------------
TEST(TCPSocketTest, ReadAndWriteStrings)
{
    osTCPSocketClient socketClient;
    osPortAddress serverAddress(hostName, portNumber);
    bool retVal = socketClient.open();
    EXPECT_EQ(true, retVal);

    gtString threadName(L"TcpServerThread");
    TCPSocketServerThread serverThread(threadName);
    serverThread.execute();

    retVal = socketClient.connect(serverAddress);
    EXPECT_EQ(true, retVal);

    osSleep(150);

    // Msg1
    retVal = socketClient.writeString(msg1FromClient);
    EXPECT_EQ(true, retVal);

    gtString fromServer;
    retVal = socketClient.readString(fromServer);
    EXPECT_TRUE(fromServer == msg1FromServer);

    osSleep(150);

    // Msg2
    retVal = socketClient.writeString(msg2FromClient);
    EXPECT_EQ(true, retVal);

    retVal = socketClient.readString(fromServer);
    EXPECT_TRUE(fromServer == msg2FromServer);

    osSleep(150);

    // Msg1
    retVal = socketClient.writeString(msg3FromClient);
    EXPECT_EQ(true, retVal);

    retVal = socketClient.readString(fromServer);
    EXPECT_TRUE(fromServer == msg3FromServer);

    osSleep(150);

    retVal = socketClient.close();
    EXPECT_EQ(true, retVal);
}


