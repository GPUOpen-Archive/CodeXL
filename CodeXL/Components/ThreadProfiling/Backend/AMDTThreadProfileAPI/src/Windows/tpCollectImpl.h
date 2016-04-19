//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpCollectImpl.h
///
//==================================================================================

#ifndef _TPCOLLECTIMPL_H_
#define _TPCOLLECTIMPL_H_

#define INITGUID  // Include this #define to use SystemTraceControlGuid/EventTraceGuid  in Evntrace.h.

// System Headers
#include <Windows.h>
#include <evntrace.h>
#include <evntcons.h>
#include <tdh.h>

// OS Wrappers headers
#include <AMDTOSWrappers/Include/osProcess.h>

// Project Headers
#include <tpInternalDataTypes.h>
#include <AMDTThreadProfileDataTypes.h>

// #pragma comment(lib, "tdh.lib")
// #pragma comment(lib, "ws2_32.lib")  // For ntohs function - unused

//
// tpCollectImpl
//

class tpCollectImpl
{
public:
    tpCollectImpl() : m_tpState(AMDT_THREAD_PROFILE_STATE_UNINITIALIZED),
        m_flags(0),
        m_callstack(false),
        m_sessionHandle(0),
        m_pSessionProperties(NULL)
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

    TRACEHANDLE                 m_sessionHandle;
    PEVENT_TRACE_PROPERTIES     m_pSessionProperties;

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

    bool tpGetUndocAPI();   // currently UNUSED
};


#endif //_TPCOLLECTIMPL_H_