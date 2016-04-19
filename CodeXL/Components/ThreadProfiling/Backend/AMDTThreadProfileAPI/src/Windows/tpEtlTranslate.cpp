//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpEtlTranslate.cpp
///
//==================================================================================

// System headers
#include <windows.h>
#include <strsafe.h>
#include <wmistr.h>
#include <evntrace.h>
#include <evntcons.h>
#include <tdh.h>

// Project headers
#include <tpInternalDataTypes.h>
#include <tpEtlTranslate.h>

#pragma comment(lib, "tdh.lib") // FIXME - should this be in tpInternalDataTypes.h


//
// Public Member functions
//


AMDTResult tpEtlTranslate::OpenThreadProfileData(bool isPassOne)
{
    AMDTResult retVal = AMDT_ERROR_INTERNAL;

    // Open the trace session.
    TRACE_LOGFILE_HEADER* pLogHeader;

    if ((TRACEHANDLE)INVALID_HANDLE_VALUE == m_hTrace)
    {
        memset(&m_traceLogFile, 0, sizeof(EVENT_TRACE_LOGFILE));
        m_traceLogFile.LoggerName = NULL;
        m_traceLogFile.LogFileName = m_logFile;
        m_traceLogFile.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD;

        PEVENT_RECORD_CALLBACK callbackFn = (isPassOne)
                                            ? (PEVENT_RECORD_CALLBACK)EtwProcessMetaEventRecordsCB
                                            : (PEVENT_RECORD_CALLBACK)EtwProcessCSwitchEventRecordsCB;

        m_traceLogFile.EventRecordCallback = callbackFn;
        //traceLogFile.BufferCallback = EtwProcessBufferCB;
        //traceLogFile.EventCallback = EtwProcessEventCB;

        m_hTrace = OpenTrace(&m_traceLogFile);
    }

    if ((TRACEHANDLE)INVALID_HANDLE_VALUE != m_hTrace)
    {
        retVal = AMDT_STATUS_OK;
        m_isPassOne = isPassOne;

        pLogHeader = &m_traceLogFile.LogfileHeader;

        // check the log header filled by OpenTrace
        m_userMode = ((pLogHeader->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) == EVENT_TRACE_PRIVATE_LOGGER_MODE)
                     ? true : false;

        if (pLogHeader->TimerResolution > 0)
        {
            m_timerResolution = pLogHeader->TimerResolution / 10000;
        }

        //fprintf(m_debugLogFP, "userMode : %d timerResolution : %lu # Lost Events : %d \n",
        //    m_userMode, m_timerResolution, pLogHeader->EventsLost);

        //          !! FIXME !!
        // Use pLogHeader to access all fields prior to LoggerName.
        // Adjust pHeader based on the pointer size to access
        // all fields after LogFileName. This is required only if
        // you are consuming events on an architecture that is
        // different from architecture used to write the events.
        //
        if (pLogHeader->PointerSize != sizeof(PVOID))
        {
            pLogHeader = (PTRACE_LOGFILE_HEADER)((PUCHAR)pLogHeader +
                                                 2 * (pLogHeader->PointerSize - sizeof(PVOID)));
        }

        //fprintf(m_debugLogFP, "# Lost Buffers : %d \n", pLogHeader->BuffersLost);
    }
    else
    {
        wprintf(L"OpenTrace failed with %lu\n", GetLastError());
    }

    return retVal;
} // OpenThreadProfileData


AMDTResult tpEtlTranslate::ProcessThreadProfileData()
{
    AMDTResult retVal = AMDT_STATUS_OK;
    ULONG status = ERROR_SUCCESS;

    if (m_isPassOne)
    {
        // pass the tpTranslate object to translateCB layer
        AMDTThreadProfileDataHandle pHandle = static_cast<AMDTThreadProfileDataHandle>(this);
        retVal = SettranslateHandle(pHandle, this->m_pTraceEventInfo, this->m_pPropertyBuffer);

        status = ProcessTrace(&m_hTrace, 1, 0, 0);

        if ((ERROR_SUCCESS == status) || (ERROR_CANCELLED == status))
        {
            m_isPassOneCompleted = true;
        }
        else
        {
            retVal = AMDT_ERROR_INTERNAL;
            wprintf(L"PASS-1 ProcessTrace failed with %lu\n", status);
        }

        //wprintf(L"Pass1 - ProcessTrace done with %lu\n", status);
    }
    else
    {
        if (m_isPassOneCompleted && !IsallCSRecordsProcessed())
        {
            // Pass 2 - process cswitch records
            AMDTThreadProfileDataHandle pHandle = static_cast<AMDTThreadProfileDataHandle>(this);
            retVal = SettranslateHandle(pHandle, this->m_pTraceEventInfo, this->m_pPropertyBuffer);

            status = ProcessTrace(&m_hTrace, 1, 0, 0);

            if ((ERROR_SUCCESS == status) || (ERROR_CANCELLED == status))
            {
                m_allCSRecordsProcessed = (ERROR_SUCCESS == status) ? true : false;
            }
            else
            {
                retVal = AMDT_ERROR_INTERNAL;
                wprintf(L"PASS-2 ProcessTrace failed with %lu\n", status);
            }

            // wprintf(L"Pass2 - ProcessTrace done with %lu\n", status);
        }
        else
        {
            wprintf(L"Trying to perform Pass2 without doing Pass1..\n");
            retVal = ERROR_INTERNAL_ERROR;
        }
    }

    // FIXME:
    retVal = CloseThreadProfileData();

    return retVal;
}


AMDTResult tpEtlTranslate::CloseThreadProfileData()
{
    AMDTResult retVal;
    ULONG status = ERROR_SUCCESS;

    if ((TRACEHANDLE)INVALID_HANDLE_VALUE != m_hTrace)
    {
        status = CloseTrace(m_hTrace);

        if (ERROR_SUCCESS == status)
        {
            m_hTrace = (TRACEHANDLE)INVALID_HANDLE_VALUE;
        }
    }

    retVal = (ERROR_SUCCESS == status) ? AMDT_STATUS_OK : AMDT_ERROR_INTERNAL;
    return retVal;
} // CloseThreadProfileData