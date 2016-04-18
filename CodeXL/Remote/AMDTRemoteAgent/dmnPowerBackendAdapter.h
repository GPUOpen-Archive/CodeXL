//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnPowerBackendAdapter.h
///
//==================================================================================

#ifndef __dmnPowerBackendAdapter_h
#define __dmnPowerBackendAdapter_h

// Local.
class dmnSessionThread;
#include <AMDTRemoteAgent/IAppWatcherObserver.h>
#include <AMDTRemoteAgent/dmnAppWatcherThread.h>

// Infra.
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osTCPSocketServerConnectionHandler.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Power profiling backend.
#include <AMDTPowerProfileAPI/inc/AMDTPowerProfileApi.h>
#include <AMDTPowerProfileAPI/inc/AMDTPowerProfileDataTypes.h>

class dmnPowerBackendAdapter : IAppWatcherObserver
{
public:
    dmnPowerBackendAdapter(dmnSessionThread* pOwner, osTCPSocketServerConnectionHandler* pConnHandler);
    ~dmnPowerBackendAdapter();

    bool handlePowerSessionInitRequest();
    bool handlePowerSessionConfigRequest();
    bool handleSetSamplingIntervalMsRequest();
    bool handleGetSystemTopologyRequest();
    bool handleGetSMinSamplingIntervalMsRequest();
    bool handleGetSCurrentSamplingIntervalMsRequest();
    bool handleEnableCounterRequest();
    bool handleDisableCounterRequest();
    bool handleIsCounterEnabledRequest();
    bool handleStartPowerProfilingRequest();
    bool handleStopPowerProfilingRequest();
    bool handlePausePowerProfilingRequest();
    bool handleResumePowerProfilingRequest();
    bool handleClosePowerProfilingSessionRequest();
    bool handleReadAllEnabledCountersRequest();
    bool handleGetDeviceCountersRequest();

    // IAppWatcherObserver implementations.
    virtual void onAppTerminated() override;

private:

    gtUInt32 StopPowerProfiling();

    dmnSessionThread* m_pSessionThread;

    // The connection handler.
    osTCPSocketServerConnectionHandler* m_pConnHandler;

    // The process ID for the application in focus (if any).
    osProcessId m_procId;

    // The thread that will signal us if the target app stopped running.
    dmnAppWatcherThread* m_pAppWatcherThread;

    // This flag will be set to true when it is safe to terminate this session.
    // "Safe" means that the client has already been notified about the session's
    // expected termination.
    bool m_isTerminationSafe;

    // stop profile status
    bool     m_isStopProfile;

};
#endif // __dmnPowerBackendAdapter_h



