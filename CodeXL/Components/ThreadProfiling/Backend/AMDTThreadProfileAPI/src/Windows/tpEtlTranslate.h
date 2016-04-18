//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpEtlTranslate.h
///
//==================================================================================

#ifndef _TPETLTRANSLATE_H_
#define _TPETLTRANSLATE_H_

#define INITGUID  // Include this #define to use SystemTraceControlGuid/EventTraceGuid  in Evntrace.h.

// System Headers
#include <Windows.h>
#include <evntrace.h>
#include <evntcons.h>
#include <tdh.h>

// Project Headers
#include <tpInternalDataTypes.h>
#include <AMDTThreadProfileDataTypes.h>
#include <tpTranslateCB.h>
#include <tpTranslate.h>


//
// Data Structures
//


class tpEtlTranslate : public tpTranslate
{

public:
    tpEtlTranslate()
    {
        m_isPassOne = true;
        m_hTrace = (TRACEHANDLE)INVALID_HANDLE_VALUE;
        m_pTraceEventInfo = (PTRACE_EVENT_INFO)malloc(TP_TRACE_EVENT_INFO_MAXSIZE);
        m_pPropertyBuffer = (PBYTE)malloc(TP_EVENT_PROPERTY_BUFFER_MAXSIZE);
    };

    ~tpEtlTranslate()
    {
        CloseThreadProfileData();

        if (NULL != m_pTraceEventInfo)
        {
            free(m_pTraceEventInfo);
            m_pTraceEventInfo = NULL;
        }

        if (NULL != m_pPropertyBuffer)
        {
            free(m_pPropertyBuffer);
            m_pPropertyBuffer = NULL;
        }
    }

    AMDTResult OpenThreadProfileData(bool isPassOne);
    AMDTResult ProcessThreadProfileData();
    AMDTResult CloseThreadProfileData();

private:
    bool                    m_isPassOne;
    EVENT_TRACE_LOGFILE     m_traceLogFile;
    TRACEHANDLE             m_hTrace;

    PTRACE_EVENT_INFO       m_pTraceEventInfo;
    PBYTE                   m_pPropertyBuffer;
};

#endif // _TPETLTRANSLATE_H_