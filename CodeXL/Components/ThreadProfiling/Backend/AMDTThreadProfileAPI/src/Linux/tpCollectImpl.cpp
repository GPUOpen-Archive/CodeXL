//=====================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \file $File: $
/// \version $Revision: $
/// \brief Thread Profile API.
//
//=====================================================================

// System headers
#include <wchar.h>
#include <string>
#include <list>

// Project headers
#include <AMDTThreadProfileApi.h>
#include <tpInternalDataTypes.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <tpCollectImpl.h>

#include <AMDTCpuProfilingControl/inc/CpuProfileControl.h>


//
// member functions
//


// FIXME: UNUSED or NOT REQUIRED
gtUInt32 tpCollectImpl::tpGetEventTraceFlags(gtUInt32 flags)
{
    GT_UNREFERENCED_PARAMETER(flags);
    unsigned int traceFlags = 0;

    return traceFlags;
} // tpGetEventTraceFlags


bool tpCollectImpl::tpIsValidFlags(gtUInt32 flags)
{
    bool rv = true;

    rv = (flags < AMDT_TP_EVENT_TRACE_MAX) ? true : false;

    return rv;
} // tpIsValidFlags


AMDTResult tpCollectImpl::tpClear()
{
    if (AMDT_THREAD_PROFILE_STATE_STARTED == m_tpState)
    {
        tpStopThreadProfile();
    }

    m_callstack = false;
    memset(m_logFilePath, 0, TP_MAX_ETL_PATH_LEN);
    m_tpState = AMDT_THREAD_PROFILE_STATE_UNINITIALIZED;

    return AMDT_STATUS_OK;
} // tpClearTPSessionData


//
//  Public Memeber functions
//

AMDTResult tpCollectImpl::tpInitialize()
{
    memset(m_logFilePath, 0, TP_MAX_ETL_PATH_LEN);

    return AMDT_STATUS_OK;
}


AMDTResult tpCollectImpl::tpSetThreadProfileConfiguration(AMDTUInt32 flags, const char* pFilePath)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    if (NULL == pFilePath || !tpIsValidFlags(flags))
    {
        retVal = AMDT_ERROR_INVALIDARG;
    }

    if (tpIsProfileInitialized())
    {
        retVal = AMDT_ERROR_PROFILE_ALREADY_CONFIGURED;
    }

    if (tpIsProfileRunning())
    {
        retVal = AMDT_ERROR_PROFILE_ALREADY_STARTED;
    }

    if (AMDT_STATUS_OK == retVal)
    {
        m_flags = flags;
        mbstowcs(m_logFilePath, pFilePath, TP_MAX_ETL_PATH_LEN - 1);

        retVal = fnEnableProfiling();

        if (S_OK == retVal)
        {
            gtString file;
            file.fromASCIIString(pFilePath);

            retVal = fnSetProfileOutputFile(file.asCharArray());

            if (S_OK == retVal)
            {
                bool isCSS = tpIsCstackEnabled(flags);

                retVal = fnSetThreadProfileConfiguration(isCSS);
            }
        }
    }

    if (S_OK == retVal)
    {
        m_tpState = AMDT_THREAD_PROFILE_STATE_INITIALIZED;
    }

    retVal = (S_OK == retVal) ? AMDT_STATUS_OK : AMDT_ERROR_INTERNAL;

    return retVal;
} // tpSetThreadProfileConfiguration


AMDTResult tpCollectImpl::tpStartThreadProfile()
{
    AMDTResult retVal = AMDT_ERROR_PROFILE_NOT_CONFIGURED;

    if (tpIsProfileInitialized())
    {
        retVal = AMDT_ERROR_PROFILE_ALREADY_STARTED;

        if (! tpIsProfileRunning())
        {
            //bool isCollectCStack = m_callstack;

            retVal = fnStartProfiling(false, NULL, NULL);

            if (S_OK == retVal)
            {
                m_tpState = AMDT_THREAD_PROFILE_STATE_STARTED;
            }

            retVal = (S_OK == retVal) ? AMDT_STATUS_OK : AMDT_ERROR_INTERNAL;
        }
    }


    return retVal;
} // tpStartThreadProfile


AMDTResult tpCollectImpl::tpStopThreadProfile()
{
    AMDTResult retVal = AMDT_ERROR_PROFILE_NOT_STARTED;

    if (tpIsProfileRunning())
    {
        retVal = fnStopProfiling();

        if (S_OK == retVal)
        {
            m_tpState = AMDT_THREAD_PROFILE_STATE_STOPPED;
        }

        retVal = tpClear();
    }

    return retVal;
} // tpStopThreadProfile
