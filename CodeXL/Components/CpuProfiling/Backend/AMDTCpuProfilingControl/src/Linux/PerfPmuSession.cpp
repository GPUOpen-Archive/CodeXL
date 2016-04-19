//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfPmuSession.cpp
///
//==================================================================================

#include <sstream>
#include "PerfPmuSession.h"
#include <AMDTOSWrappers/Include/osDebugLog.h>

PerfPmuSession::PerfPmuSession(PerfProfiler* profiler, PerfConfig* profConfig, PerfPmuTarget* tgt) :
    m_pProfiler(NULL), m_pProfileConfig(NULL), m_pPmuTarget(NULL), m_state(PERF_PMU_SESSION_STATE_UN_INTIALIZED)
{
    // unused
    (void)(profiler);
    (void)(profConfig);
    (void)(tgt);
}

HRESULT PerfPmuSession::initialize(PerfProfiler* profiler, PerfConfig* profConfig, PerfPmuTarget* pmuTarget)
{
    HRESULT ret;

    ret = setProfiler(profiler);

    if (S_OK != ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"setProfiler failed, ret(%d)", ret);
        return E_INVALIDARG;
    }

    ret = setProfileConfig(profConfig);

    if (S_OK != ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"setProfileConfig failed, ret(%d)", ret);
        return E_INVALIDARG;
    }

    ret = setPMUTarget(pmuTarget);

    if (S_OK != ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"setPMUTarget failed, ret(%d)", ret);
        return E_INVALIDARG;
    }

    return ret;
}

HRESULT PerfPmuSession::setProfiler(PerfProfiler* profiler)
{
    if (! profiler)
    {
        return E_INVALIDARG;
    }

    m_pProfiler = profiler;
    // TODO: Check for valid PerfProfiler
    // get the state from the profiler and check it is initialized and ready...

    return init();
}

HRESULT PerfPmuSession::setProfileConfig(PerfConfig* config)
{
    if (! config)
    {
        return E_INVALIDARG;
    }

    m_pProfileConfig = config;
    // TODO: Check for valid PerfConfig
    // get the state from the profileconfig and check it is initialized

    return init();
}

HRESULT PerfPmuSession::setPMUTarget(PerfPmuTarget* pmuTarget)
{
    if (! pmuTarget)
    {
        return E_INVALIDARG;
    }

    m_pPmuTarget = pmuTarget;
    // TODO: Check for valid PerfPmuTarget
    // get the state from the profileconfig and check it is initialized

    return init();
}


HRESULT PerfPmuSession::init()
{
    HRESULT ret;

    // If its already in READY state, return
    if (PERF_PMU_SESSION_STATE_READY == m_state)
    {
        return S_OK; // error ?
    }

    // If all the components are available, initialize them
    if (m_pProfiler && m_pProfileConfig && m_pPmuTarget)
    {
        // set the output file
        // FIXME: check for valid value in string
        m_pProfiler->setOutputFile(m_outputFile);

        ret = m_pProfiler->initialize(m_pProfileConfig, m_pPmuTarget);

        if (S_OK != ret)
        {
            std::stringstream sstr;
            sstr << m_pProfiler->getErrStr();
            m_errStr = sstr.str();
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%hs", m_errStr.c_str());
            return ret;
        }

        // set the state to READY

        // post condition  - TODO: Check the profiler state
        m_state = PERF_PMU_SESSION_STATE_READY;
    }

    return S_OK;
}


HRESULT PerfPmuSession::startProfile(bool enable)
{
    if (PERF_PMU_SESSION_STATE_READY != m_state)
    {
        return E_UNEXPECTED;
    }

    HRESULT ret = m_pProfiler->startProfile(enable);

    if (S_OK == ret)
    {
        m_state = (enable) ? PERF_PMU_SESSION_STATE_ACTIVE : PERF_PMU_SESSION_STATE_INACTIVE;
    }
    else
    {

        std::stringstream sstr;
        sstr << m_pProfiler->getErrStr();
        m_errStr = sstr.str();
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"%hs", m_errStr.c_str());

        m_state = PERF_PMU_SESSION_STATE_ERROR;
    }

    return ret;
}

HRESULT PerfPmuSession::stopProfile()
{
    // should be Active or Paused state
    if ((PERF_PMU_SESSION_STATE_ACTIVE != m_state) &&
        (PERF_PMU_SESSION_STATE_INACTIVE != m_state) &&
        (PERF_PMU_SESSION_STATE_PAUSED != m_state))
    {
        return E_UNEXPECTED;
    }

    HRESULT ret = m_pProfiler->stopProfile();
    m_state = (S_OK == ret) ? PERF_PMU_SESSION_STATE_INACTIVE : PERF_PMU_SESSION_STATE_ERROR;

    return ret;
}


HRESULT PerfPmuSession::enableProfile()
{
    // Can either be ins INACTIVE or PAUSED state
    if ((PERF_PMU_SESSION_STATE_INACTIVE != m_state) && (PERF_PMU_SESSION_STATE_PAUSED != m_state))
    {
        return E_UNEXPECTED;
    }

    HRESULT ret = m_pProfiler->enableProfile();
    m_state = (S_OK == ret) ? PERF_PMU_SESSION_STATE_ACTIVE : PERF_PMU_SESSION_STATE_ERROR;
    return ret;
}


HRESULT PerfPmuSession::disableProfile()
{
    if (PERF_PMU_SESSION_STATE_ACTIVE != m_state)
    {
        return E_UNEXPECTED;
    }

    HRESULT ret = m_pProfiler->disableProfile();
    m_state = (S_OK == ret) ? PERF_PMU_SESSION_STATE_PAUSED : PERF_PMU_SESSION_STATE_ERROR;
    return ret;
}

HRESULT PerfPmuSession::readPMUCounters(PerfEventCountDataList** countData)
{
    if (NULL == countData)
    {
        return E_INVALIDARG;
    }

    if ((PERF_PMU_SESSION_STATE_ACTIVE != m_state)
        && (PERF_PMU_SESSION_STATE_INACTIVE != m_state)
        && (PERF_PMU_SESSION_STATE_PAUSED != m_state))
    {
        return E_UNEXPECTED;
    }

    HRESULT ret = m_pProfiler->readCounters(countData);

    if (ret != S_OK)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error in PerfPmuSession::readPMUCounters, ret(%d)", ret);
    }

    return ret;
}

HRESULT PerfPmuSession::clear()
{
    // If the session is active or paused, don't allow to clear it.
    if ((PERF_PMU_SESSION_STATE_ACTIVE == m_state) || (PERF_PMU_SESSION_STATE_PAUSED == m_state))
    {
        return E_UNEXPECTED;
    }

    m_state = PERF_PMU_SESSION_STATE_UN_INTIALIZED;

    m_pProfiler = NULL;
    m_pProfileConfig = NULL;
    m_pPmuTarget = NULL;

    m_outputFile.clear();

    return S_OK;
}
