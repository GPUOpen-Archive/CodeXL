//=====================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \file $File: $
/// \version $Revision: $
/// \brief Data Translate layer for Thread Profiler
//
//=====================================================================

#ifndef _TPCOLLECTIMPL_H_
#define _TPCOLLECTIMPL_H_

// OS Wrappers headers
#include <AMDTOSWrappers/Include/osProcess.h>

// Project Headers
#include <tpInternalDataTypes.h>
#include <AMDTThreadProfileDataTypes.h>

//
// tpCollectImpl
//

class tpCollectImpl
{
public:
    tpCollectImpl() : m_tpState(AMDT_THREAD_PROFILE_STATE_UNINITIALIZED),
        m_flags(0),
        m_callstack(false)
    {};

    ~tpCollectImpl()
    {
        tpClear();
    }

    AMDTResult tpInitialize();
    AMDTResult tpSetThreadProfileConfiguration(AMDTUInt32 flags, const char* pFilePath);
    AMDTResult tpStartThreadProfile();
    AMDTResult tpStopThreadProfile();
    AMDTResult tpClear();

private:
    AMDTThreadProfileState      m_tpState;
    AMDTUInt32                  m_flags;
    bool                        m_callstack;

    wchar_t                     m_logFilePath[TP_MAX_ETL_PATH_LEN];

private:
    bool tpIsProfileInitialized()
    {
        return (AMDT_THREAD_PROFILE_STATE_INITIALIZED == m_tpState) ? true : false;
    }

    bool tpIsProfileRunning()
    {
        return (AMDT_THREAD_PROFILE_STATE_STARTED == m_tpState) ? true : false;
    }

    gtUInt32 tpGetEventTraceFlags(gtUInt32 flags);

    bool tpIsValidFlags(gtUInt32 flags);

    bool tpIsCstackEnabled(gtUInt32 flags)
    {
        return ((flags & AMDT_TP_EVENT_TRACE_CALLSTACK) == AMDT_TP_EVENT_TRACE_CALLSTACK) ? true : false;
    }
};


#endif //_TPCOLLECTIMPL_H_
