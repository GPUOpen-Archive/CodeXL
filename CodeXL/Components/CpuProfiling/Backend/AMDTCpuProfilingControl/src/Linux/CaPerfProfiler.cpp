//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CaPerfProfiler.cpp
///
//==================================================================================

// standard headers
#include <unistd.h>
#include <signal.h>

// project headers
#include "CaPerfProfiler.h"
#include <AMDTOSWrappers/Include/osDebugLog.h>


// signal handler for the SIGUSR1 to be used by PERF sample reader thread
#ifdef CAPERF_USES_SIGUSR1
static void sigusr1_handler(int sig)
{
    (void)(sig); // unused
    return;
}
#endif // CAPERF_USES_SIGUSR1


//Thread start routine for reading PERF samples
void* sampleReaderThreadProc(void* pObj)
{

    // Baskar: This is for fixing BUG353686: CXL collects no samples if profile duration is less than 4 secs.
    // This PERF Sample Reader Thread will do a ppoll() waiting on the PERF fds. Once the profile stops,
    // the CaPerfProfiler::stopProfile() will send SIGUSR1 to the sample reader thread, to notify that
    // the profiling is done.
    // For doing this, the thread should set a signal handler for SIGUSR1. This signal handler is for
    // the CXL process. Not sure, how the CXL sets the signal handler or whether this signal SIGUSR1
    // is used by other stuff
    //
#ifdef CAPERF_USES_SIGUSR1
    struct sigaction sigact;
    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_handler = sigusr1_handler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;

    int ret = sigaction(SIGUSR1, &sigact, NULL);

    if (0 != ret)
    {
        OS_OUTPUT_DEBUG_LOG(L"Setting signal handler for SIGUSR1 failed...", OS_DEBUG_LOG_ERROR);
    }

#endif // CAPERF_USES_SIGUSR1

    if (NULL != pObj)
    {
        CaPerfProfiler* pProfiler = (CaPerfProfiler*) pObj;

        // This is a blocking routin. It polls on the fds returned by the
        // PERF syscall. When the profiling is stopped, this will return.
        pProfiler->readSampleBuffers();
    }

    pthread_exit(NULL);
    return NULL;
}


//This creates the thread to read PERF samples
HRESULT CaPerfProfiler::createSampleReaderThread()
{
    pthread_t threadId;

    int ret = pthread_create(&threadId,
                             NULL,
                             sampleReaderThreadProc,
                             (void*)this);

    if (0 != ret)
    {
        return E_FAIL;
    }

    m_threadId = threadId;
    return S_OK;
}


//This stops the thread created to read PERF samples
HRESULT CaPerfProfiler::stopSampleReaderThread()
{
    // stop the sample reader thread and wait for it to get terminated
    if (0 != m_threadId)
    {
#ifdef CAPERF_USES_SIGUSR1
        // pthread_cancel(m_threadId);
        // Send SIGUSR1 to notify the sample-reader-thread, that we are done with sampling
        pthread_kill(m_threadId, SIGUSR1);
#endif // CAPERF_USES_SIGUSR1

        // wait for the thread to terminate
        pthread_join(m_threadId, NULL);
        m_threadId = 0;
    }

    return S_OK;
}

#define PERF_EVENT_PARANOID "/proc/sys/kernel/perf_event_paranoid"

HRESULT CaPerfProfiler::initialize(PerfConfig*  config, PerfPmuTarget* tgt)
{
    HRESULT ret = S_OK;

    // process config and construct CAProfileCfgPERF, CAEventPERF, etc
    // process tgt and construct PERF specific tgt information..
    // open the event counters; by default all will be disabled;

    // TODO: Check for the availability of PERF kernel subsystem

    m_errStr = "\nReasons:\n";

    //-----------------------------------------------------------
    // Check for existence of the file /proc/sys/kernel/perf_event_paranoid.
    if ((access(PERF_EVENT_PARANOID, F_OK)) < 0)
    {
        m_errStr = m_errStr + "PERF Not Available (Missing " + PERF_EVENT_PARANOID + " )";
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%hs", m_errStr.c_str());
        return E_NOTAVAILABLE;
    }

    // intialize profile config
    if (config)
    {
        ret = setProfileConfig(config);

        if (S_OK != ret)
        {
            return ret;
        }
    }

    // intialzie PMU target
    if (tgt)
    {
        ret = setPMUTarget(tgt);

        if (S_OK != ret)
        {
            return ret;
        }
    }

    return ret;
}


HRESULT CaPerfProfiler::setProfileConfig(PerfConfig*  config)
{
    HRESULT ret = S_OK;

    if (! config)
    {
        return E_INVALIDARG;
    }

    // The profile state should be in PERF_PROFILER_STATE_NOT_INITIALIZED
    if (PERF_PROFILER_STATE_NOT_INITIALIZED != m_state)
    {
        return E_UNEXPECTED;
    }

    m_pProfileConfig = config;

    // set the profiler status..
    m_state = PERF_PROFILER_STATE_INITIALIZED;

    return ret;
}


HRESULT CaPerfProfiler::setPMUTarget(PerfPmuTarget* tgt)
{
    HRESULT ret = S_OK;

    if (! tgt)
    {
        return E_INVALIDARG;
    }

    if ((PERF_PROFILER_STATE_INITIALIZED != m_state)
        || (NULL == m_pProfileConfig))
    {
        return E_UNEXPECTED;
    }

    m_pTarget = tgt;

    // Process the profile events in PerfConfig and create PERF specific
    //  internal data structures..
    m_pPerfCfg = new CaPerfConfig(*m_pProfileConfig, *m_pTarget);

    // FIXME: set the output file in profile config
    m_pPerfCfg->setOutputFile(getOutputFile());

    ret = m_pPerfCfg->initialize();

    if (S_OK != ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error while initializing CaPerfConfig, ret(%d)", ret);
        return E_INVALIDDATA;
    }

    // Now the profiler is ready to profile.
    m_state = PERF_PROFILER_STATE_READY;
    return S_OK;
}

HRESULT CaPerfProfiler::startProfile(bool enable)
{
    HRESULT ret;

    // if not in ready state, return error
    if (PERF_PROFILER_STATE_READY != m_state)
    {
        return E_UNEXPECTED;
    }

    // TODO: Check for valid counting/sampling events..
    // if there is no event, then error out

    ret = m_pPerfCfg->startProfile(false);

    if (S_OK != ret)
    {
        m_errStr = m_pPerfCfg->getErrStr();
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error in startProfile, ret(%d)", ret);
        m_state = PERF_PROFILER_STATE_ERROR;
        return ret;
    }

    // Now CaPerfProfiler is in INACTIVE state
    m_state = PERF_PROFILER_STATE_INACTIVE;

    if (m_pPerfCfg->hasSampleEvents())
    {
        // In PERF we need to create a Sample Reader thread to read the samples and
        // write them into the profile output file
        ret = createSampleReaderThread();

        if (S_OK != ret)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error while creating sample reader thread. ret(%d)", ret);
            // TODO: cleanup the internal objects like CaPerfConfig
            m_state = PERF_PROFILER_STATE_ERROR;
            return E_FAIL;
        }
    }

    // Now enable the profile
    if (enable)
    {
        enableProfile();
    }

    return S_OK;
}

HRESULT CaPerfProfiler::stopProfile()
{
    HRESULT ret;

    // if not in active/paused state, return error
    if ((PERF_PROFILER_STATE_ACTIVE != m_state)
        && (PERF_PROFILER_STATE_INACTIVE != m_state)
        && (PERF_PROFILER_STATE_PAUSED != m_state))
    {
        return E_UNEXPECTED;
    }

    ret = m_pPerfCfg->stopProfile();

    if (S_OK != ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error in stopProfile, ret(%d)\n", ret);
        return ret;
    }

    // stop the sample reader thread and wait for it to get terminated
    if (m_pPerfCfg->hasSampleEvents())
    {
        stopSampleReaderThread();
    }

    // Baskar: we need to close the file-descriptor's opened for the sampling-events.
    // PMCs won't be released by PERF, till we close these fd's.
    //
    // At the API layer, though we cleanup everything in fnClearConfigurations(), since
    // this cleanup API gets called only when the user starts a new profile, the CodeXL
    // locks the PMC counters even after the profiling completes. Hence we need to
    // clear it here.

    clear();

    m_state = PERF_PROFILER_STATE_INACTIVE;
    return S_OK;
}

HRESULT CaPerfProfiler::enableProfile()
{
    HRESULT ret;

    // should be in inactive/paused state
    if ((PERF_PROFILER_STATE_INACTIVE != m_state)
        && (PERF_PROFILER_STATE_PAUSED != m_state))
    {
        return E_UNEXPECTED;
    }

    ret = m_pPerfCfg->enableProfile();

    if (S_OK != ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error in enableProfile, ret(%d)", ret);
        return ret;
    }

    m_state = PERF_PROFILER_STATE_ACTIVE;
    return S_OK;
}


HRESULT CaPerfProfiler::disableProfile()
{
    HRESULT ret;

    // if not in active state, return error
    if (PERF_PROFILER_STATE_ACTIVE != m_state)
    {
        return E_UNEXPECTED;
    }

    ret = m_pPerfCfg->disableProfile();

    if (S_OK != ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error in disableProfile, ret(%d)", ret);
        return ret;
    }

    m_state = PERF_PROFILER_STATE_PAUSED;
    return S_OK;
}

#if 0
HRESULT CaPerfProfiler::pauseProfile()
{
    return stopProfile();
}

HRESULT CaPerfProfiler::resumeProfile()
{
    HRESULT ret;

    // if not in inactive state, return error
    if (PERF_PROFILER_STATE_INACTIVE != m_state)
    {
        return E_UNEXPECTED;
    }

    ret = m_pPerfCfg->enableProfile();

    if (S_OK != ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error in resumeProfile, ret(%d)", ret);
        return ret;
    }

    m_state = PERF_PROFILER_STATE_ACTIVE;
    return ret;
}
#endif //0

HRESULT CaPerfProfiler::readCounters(PerfEventCountDataList** countData)
{
    HRESULT ret;

    // Either should in active/inactive/paused state, else return error
    if ((PERF_PROFILER_STATE_ACTIVE != m_state) &&
        (PERF_PROFILER_STATE_INACTIVE != m_state) &&
        (PERF_PROFILER_STATE_PAUSED != m_state))
    {
        return E_UNEXPECTED;
    }

    ret = m_pPerfCfg->readCounters(countData);

    if (S_OK != ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error in readCounters, ret(%d)", ret);
        return ret;
    }

    return S_OK;
}

int CaPerfProfiler::readSampleBuffers()
{
    int ret;

    ret = m_pPerfCfg->readSampleData();

    return ret;
}


void CaPerfProfiler::clear()
{
    PerfProfiler::clear();

    // delete CaPerfConfig
    if (m_pPerfCfg)
    {
        delete m_pPerfCfg;
        m_pPerfCfg = NULL;
    }
}
