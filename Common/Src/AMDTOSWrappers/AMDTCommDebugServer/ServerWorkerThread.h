#pragma once

#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>

class osTCPSocketServerConnectionHandler;

class ServerWorkerThread : public osThread
{
public:
    ServerWorkerThread(osTCPSocketServerConnectionHandler* pClientCallHandler);
    ~ServerWorkerThread();

protected:
    virtual int entryPoint();
    virtual void beforeTermination();

private:
    osTCPSocketServerConnectionHandler* m_pClientCallHandler;
    bool m_bContinueRunning;
    static osCriticalSection m_lockWriteToConsole;
};
