#pragma once

#include <list>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>

class ServerWorkerThread;

class ServerListenThread : public osThread
{
public:
    ServerListenThread();
    virtual ~ServerListenThread();
    bool init(int portNum);

protected:
    virtual int entryPoint();
    virtual void beforeTermination();

private:
    osTCPSocketServer m_serverSocket;
    std::list<ServerWorkerThread*> m_spawnedThreads;
    bool m_bContinueRunning;
};
