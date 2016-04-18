//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PPPollingThread.h
///
//==================================================================================

#ifndef PPPollingThread_h__
#define PPPollingThread_h__
//std
#include <condition_variable>

//Infra
#include <AMDTOSWrappers/Include/osThread.h>

//Local
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>
#include <AMDTPowerProfilingMidTier/include/IPowerProfilerBackendAdapter.h>
#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>

// Forward declarations.
enum AppLaunchStatus;

class PPPollingThread :
    public osThread
{
public:

    PPPollingThread(unsigned pollingInterval, PPSamplesDataHandler cb, void* pDataCbParams, PPFatalErrorHandler cbErr, void* pErrorCbParams,
                    IPowerProfilerBackendAdapter* pBeAdapter, amdtProfileDbAdapter* pDataAdapter);
    ~PPPollingThread();

    virtual int entryPoint();
    unsigned GetCurrentQuantizedTime() const { return m_quantizedTime; }

    // Reports the status of the session to be started.
    // Currently PPR_COMMUNICATION_FAILURE will be the default value which means that
    // the polling thread hasn't tried to start the session. It is guaranteed that,
    // after an attempt is made by the polling thread to start the session,
    // the returned value will be different than PPR_COMMUNICATION_FAILURE.
    PPResult GetSessionStartedStatus() const { return m_sessionStartedStatus; }

    // Indicates the status of the application launch.
    // This is only relevant for remote sessions.
    AppLaunchStatus GetApplicationLaunchStatus() const { return m_targetAppLaunchStatus; }
    void requestExit() { m_IsStopped = true; }

protected:
    virtual void beforeTermination() override;
private:
    PPPollingThread& operator=(const PPPollingThread&);
    const unsigned m_pollingInterval;
    unsigned m_quantizedTime;
    PPSamplesDataHandler m_dataCb;
    void* m_pDataCbParams;
    PPFatalErrorHandler m_errorCb;
    void* m_pErrorCbParams;
    IPowerProfilerBackendAdapter* m_pBeAdapter;

    // Adapter to gain access to the DB.
    amdtProfileDbAdapter* m_pDataAdapter;

    // Holds the result from the backend adapter.
    PPResult m_sessionStartedStatus;

    // Holds the result from the backend adapter.
    AppLaunchStatus m_targetAppLaunchStatus;
    bool m_IsStopped = false;
    bool m_profilingErr;
    PPResult m_profResult;
};

#endif // PPPollingThread_h__
