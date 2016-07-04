#include <cstdio>
#include <iostream>

#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osConsole.h>
#include "ServerListenThread.h"

int main(int argc, char* argv[])
{
    const int DEFAULT_PORT_NUMBER = 10000;
    int portNum = 0;
    std::wcout << L"AMDTCommDebugServer running, press any key to exit." << std::endl;

    if (argc > 1)
    {
        gtString portNumString;
        portNumString.fromASCIIString(argv[1], strlen(argv[1]));
        portNumString.toIntNumber(portNum);
    }
    else
    {
        portNum = DEFAULT_PORT_NUMBER;
    }

    ServerListenThread listenThread;
    listenThread.init(portNum);
    listenThread.execute();

    // Wait for user interaction on main thread.
    osWaitForKeyPress();

    listenThread.terminate();
}