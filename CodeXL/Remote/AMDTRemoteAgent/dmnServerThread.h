//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnServerThread.h
///
//==================================================================================

#ifndef __dmnServerThread_h
#define __dmnServerThread_h

#include <AMDTRemoteAgent/dmnServerThreadConfig.h>
#include <AMDTRemoteAgent/IThreadEventObserver.h>

#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>

class dmnServerThread :
    public osThread
{
public:
    dmnServerThread(dmnServerThreadConfig* pConfig);
    ~dmnServerThread(void);
    void registerToThreadCreationEvent(IThreadEventObserver* pObserver);
protected:
    virtual int entryPoint() override;
private:
    dmnServerThreadConfig* m_pConfig;
    int m_sessionThreadsCount;
    gtList<IThreadEventObserver*> m_threadCreationObservers;
    osTCPSocketServer m_tcpServer;

    // No copy.
    dmnServerThread(const dmnServerThread& other);
    const dmnServerThread& operator=(const dmnServerThread& other);
};

#endif // __dmnServerThread_h
