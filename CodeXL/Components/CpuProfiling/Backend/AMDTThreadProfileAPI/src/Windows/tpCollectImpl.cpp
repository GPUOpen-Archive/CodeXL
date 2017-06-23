//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpCollectImpl.cpp
///
//==================================================================================

//FIXME - also defined in tpInternalDataTypes.h
#define INITGUID  // Include this #define to use SystemTraceControlGuid/EventTraceGuid  in Evntrace.h.

// System headers
#include <windows.h>
#include <strsafe.h>  // ??
#include <wmistr.h>   // ??
#include <evntrace.h>
#include <evntcons.h>
#include <tdh.h>
#include <wchar.h>
#include <string>
#include <list>

// Project headers
#include <AMDTThreadProfileApi.h>
#include <tpInternalDataTypes.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <tpCollectImpl.h>

#pragma comment(lib, "tdh.lib")
// #pragma comment(lib, "ws2_32.lib")  // For ntohs function - unused

//
// Macros
//


//
// Globals
//

// GUIDs for NT kernel logger events
//  https://msdn.microsoft.com/en-us/library/windows/desktop/aa364085(v=vs.85).aspx
//  https://famellee.wordpress.com/2012/08/17/a-fight-with-etw-and-nt-kernel-logger/
//
// Type - https://msdn.microsoft.com/en-us/library/windows/desktop/dd765164(v=vs.85).aspx
// Trace Info - https://msdn.microsoft.com/en-us/library/windows/desktop/dd392329(v=vs.85).aspx
// StackWalk_Event class  https://msdn.microsoft.com/en-us/library/windows/desktop/dd392323(v=vs.85).aspx
//
// Note: All the kernel events use MOF to publish the format of the event data.
// MOF approach to consume trace data
//      https://msdn.microsoft.com/en-us/library/windows/desktop/aa364114(v=vs.85).aspx
// Trace Data Helper(TDH) approach
//      https://msdn.microsoft.com/en-us/library/windows/desktop/ee441328(v=vs.85).aspx
//      https://msdn.microsoft.com/en-us/library/windows/desktop/aa364115(v=vs.85).aspx
//

//static const GUID g_imageGuid = { 0x2cb15d1d, 0x5fc1, 0x11d2, { 0xab, 0xe1, 0x00, 0xa0, 0xc9, 0x11, 0xf5, 0x18 } };
//static const GUID g_processGuid = { 0x3d6fa8d0, 0xfe05, 0x11d0, { 0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c } };
//static const GUID g_threadGuid = { 0x3d6fa8d1, 0xfe05, 0x11d0, { 0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c } };
//static const GUID g_stackWalkGuid = { 0xdef2fe46, 0x7bd6, 0x4b80, { 0xbd, 0x94, 0xf5, 0x7f, 0xe2, 0x0d, 0x0c, 0xe3 } };
//static const GUID g_perfInfoGuid = { 0xce1dbfb4, 0x137e, 0x4da6, { 0x87, 0xb0, 0x3f, 0x59, 0xaa, 0x10, 0x2c, 0xbc } };

//const wchar_t *g_pLogFilePath = L"c:\\temp\\test2.etl";

//
// member functions
//

gtUInt32 tpCollectImpl::tpGetEventTraceFlags(gtUInt32 flags)
{
    UNREFERENCED_PARAMETER(flags);
    unsigned int traceFlags = 0;

    // By default enable the following flags
    //  EVENT_TRACE_FLAG_IMAGE_LOAD
    //  EVENT_TRACE_FLAG_PROCESS
    //  EVENT_TRACE_FLAG_THREAD
    //  EVENT_TRACE_FLAG_CSWITCH

    traceFlags =  EVENT_TRACE_FLAG_IMAGE_LOAD
                  | EVENT_TRACE_FLAG_PROCESS
                  | EVENT_TRACE_FLAG_THREAD
                  | EVENT_TRACE_FLAG_CSWITCH;

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
    // FIXME: this is not correct..
    // This will lead to recursion
    if (AMDT_THREAD_PROFILE_STATE_STARTED == m_tpState)
    {
        tpStopThreadProfile();
    }

    if (NULL != m_pSessionProperties)
    {
        free(m_pSessionProperties);
        m_pSessionProperties = NULL;
    }

    m_callstack = false;
    memset(m_logFilePath, 0, TP_MAX_ETL_PATH_LEN * sizeof(wchar_t));
    m_sessionHandle = 0;
    m_tpState = AMDT_THREAD_PROFILE_STATE_UNINITIALIZED;

    return AMDT_STATUS_OK;
} // tpClearTPSessionData


bool tpCollectImpl::tpGetUndocAPI()
{
    bool retVal = false;

#if 0
    typedef unsigned long(__stdcall * PFN_CreateMergedETLFile)(unsigned short const * * const, unsigned long, unsigned short const*);

#ifndef _WIN64
    LPCSTR FNNAME_CreateMergeETLFile = "? CreateMergedETLFile@@YGKQAPBGKPBG@Z";
#else
    LPCSTR FNNAME_CreateMergeETLFile = "? CreateMergedETLFile@@YAKQEAPEBGKPEBG@Z";
#endif

    HMODULE hPerfCtrl = LoadLibrary(TEXT("C:\Program Files (x86)\Windows Kits\8.1\Windows Performance Toolkit\perfctrl.dll"));

    PFN_CreateMergedETLFile pfn = (PFN_CreateMergedETLFile)GetProcAddress(hPerfCtrl, FNNAME_CreateMergeETLFile);

    if (NULL != pfn)
    {
        LPCWSTR pszPathTraceNT = m_logFilePath;
        wchar_t pszPathTraceNTMerged[256];

        (*pfn)((unsigned short const **)&pszPathTraceNT, 1, (unsigned short const*)pszPathTraceNTMerged);
    }

    FreeLibrary(hPerfCtrl);
#endif //0

    return retVal;
} // tpGetUndocAPI


//
//  Public Memeber functions
//

AMDTResult tpCollectImpl::tpInitialize()
{
    memset(m_logFilePath, 0, TP_MAX_ETL_PATH_LEN * sizeof(wchar_t));

    return AMDT_STATUS_OK;
}


AMDTResult tpCollectImpl::tpSetThreadProfileConfiguration(AMDTUInt32 flags, const char* pFilePath)
{
    AMDTResult retVal = AMDT_STATUS_OK;
    ULONG bufferSize = 0;
    EVENT_TRACE_PROPERTIES* pSessionProperties = NULL;

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

        size_t logFilePathLength = (wcslen(m_logFilePath) * 2) + 2;
        size_t loggerSessionLength = (wcslen(KERNEL_LOGGER_NAME) * 2) + 2;

        // Allocate memory for the session properties. The memory must
        // be large enough to include the log file name and session name,
        // which get appended to the end of the session properties structure.
        bufferSize = sizeof(EVENT_TRACE_PROPERTIES) + logFilePathLength + loggerSessionLength;

        if (NULL == m_pSessionProperties)
        {
            pSessionProperties = (EVENT_TRACE_PROPERTIES*)malloc(bufferSize);

            if (NULL != pSessionProperties)
            {
                // Set the session properties. You only append the log file name
                // to the properties structure; the StartTrace function appends
                // the session name for you.
                // Firt session name then logfile name; sessios name is copied by StartTrace() call

                ZeroMemory(pSessionProperties, bufferSize);
                pSessionProperties->Wnode.BufferSize = bufferSize;
                pSessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
                pSessionProperties->Wnode.ClientContext = 1; //QPC clock resolution

                // FIXME: do we really need to specify Guid
                // pSessionProperties->Wnode.Guid = ThreadGuid; //  SystemTraceControlGuid;

                pSessionProperties->EnableFlags = tpGetEventTraceFlags(m_flags);
                pSessionProperties->LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;
                pSessionProperties->MaximumFileSize = 512;  // 512 MB
                // pSessionProperties->FlushTimer = 1; // ??
                pSessionProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + loggerSessionLength;
                pSessionProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

                StringCbCopyW((LPWSTR)((char*)pSessionProperties + pSessionProperties->LogFileNameOffset),
                              logFilePathLength,
                              m_logFilePath);

                m_pSessionProperties = pSessionProperties;
                m_callstack          = tpIsCstackEnabled(m_flags);
                m_tpState            = AMDT_THREAD_PROFILE_STATE_INITIALIZED;
            }
            else
            {
                retVal = AMDT_ERROR_OUTOFMEMORY;
            }
        }
        else
        {
            retVal = AMDT_ERROR_PROFILE_ALREADY_CONFIGURED;
        }
    }

    if (AMDT_STATUS_OK == retVal)
    {
        m_tpState = AMDT_THREAD_PROFILE_STATE_INITIALIZED;
    }

    return retVal;
} // tpSetThreadProfileConfiguration


AMDTResult tpCollectImpl::tpStartThreadProfile()
{
    AMDTResult retVal = AMDT_STATUS_OK;
    ULONG status;

    if (!tpIsProfileInitialized())
    {
        retVal = AMDT_ERROR_PROFILE_NOT_CONFIGURED;
    }

    if (tpIsProfileRunning())
    {
        retVal = AMDT_ERROR_PROFILE_ALREADY_STARTED;
    }

    PTRACEHANDLE pSessionHandle = &m_sessionHandle;
    PEVENT_TRACE_PROPERTIES pSessionProperties = m_pSessionProperties;
    bool isCollectCStack = m_callstack;

    if (NULL == pSessionProperties)
    {
        retVal = AMDT_ERROR_INTERNAL;
    }

    if (AMDT_STATUS_OK == retVal)
    {
        // Create the trace session.
        status = StartTrace(pSessionHandle, KERNEL_LOGGER_NAME, pSessionProperties);

        if (ERROR_SUCCESS == status)
        {
            // StartTrace succeeded.

            // Events in "NT Kernel Logger",
            // - EVENT_TRACE_FLAG_THREAD
            //      - Supported Event Type
            //        1   - EVENT_TRACE_TYPE_START Start thread event. The Thread_V2_TypeGroup1 MOF class defines the
            //              event data for this event
            //        2   - EVENT_TRACE_TYPE_END End thread event. The Thread_V2_TypeGroup1 MOF class defines the
            //              event data for this event
            //       36   - Context switch event. The CSwitch MOF class defines the event data for this event
            //       50   - Ready thread event. The ReadyThread MOF class defines the event data for this event
            //
            // - EVENT_TRACE_FLAG_IMAGE_LOAD
            //      - Supported Event Type
            //        10   - EVENT_TRACE_TYPE_LOAD Image load event. Generated when a DLL or executable file is loaded.
            //               The provider generates only one event for the first time a given DLL is loaded.
            //               The Image_Load MOF class defines the event data for this event
            //        2    - EVENT_TRACE_TYPE_END End thread event. Image unload event. Generated when a DLL or executable
            //               file is unloaded. The provider generates only one event for the last time a given DLL is unloaded.
            //               The Image_Load MOF class defines the event data for this event
            //
            // - EVENT_TRACE_FLAG_CSWITCH
            //      - Enables the following *Thread* event type - CSWITCH
            //        FIXME: Do i need to enable EVENT_TRACE_FLAG_THREAD or just enabling EVENT_TRACE_FLAG_CSWITCH is enough?
            //

            // StackWalk_Event class will have the stack data
            //  https://msdn.microsoft.com/en-us/library/windows/desktop/dd392323(v=vs.85).aspx
            //  CLASSIC_EVENT_ID traceCSForClasses[] = { { PerfInfoGuid, 46, { 0 } },
            //                                         { ThreadGuid, 36, { 0 } },
            //                                         { ThreadGuid, 50, { 0 } } };

            if (isCollectCStack)
            {
                CLASSIC_EVENT_ID traceCallStackForClasses[] =
                {
                    //{ g_threadGuid, 1, { 0 } },   // Start Thread Event
                    //{ g_threadGuid, 2, { 0 } },   // End Thread Event
                    { g_threadGuid, 36, { 0 } },  // Context Switch Event
                    //{ g_threadGuid, 50, { 0 } }   // Ready Thread Event
                };

                status = TraceSetInformation(*pSessionHandle,
                                             TraceStackTracingInfo,
                                             traceCallStackForClasses,
                                             sizeof(traceCallStackForClasses));

                if (ERROR_SUCCESS != status)
                {
                    retVal = AMDT_ERROR_INTERNAL;
                    fprintf(stderr, "TraceSetInformation() status(%lu)\n", status);
                }
            }
        }
        else if (ERROR_ALREADY_EXISTS == status)
        {
            retVal = AMDT_ERROR_PROFILE_SESSION_EXISTS;

            fprintf(stderr, "The NT Kernel Logger session is already in use.\n");

            // TODO: Use ControlTrace() to update
            status = ControlTraceW(0, KERNEL_LOGGER_NAME, pSessionProperties, EVENT_TRACE_CONTROL_UPDATE);
            fprintf(stderr, "ControlTraceW status(%lu).\n", status);

            if (ERROR_SUCCESS == status)
            {
                retVal = AMDT_STATUS_OK;
            }
            else
            {
                // FIXME: just for testing purpose
                status = ControlTrace(m_sessionHandle,
                                      KERNEL_LOGGER_NAME,
                                      pSessionProperties,
                                      EVENT_TRACE_CONTROL_STOP);
            }
        }
        else
        {
            retVal = AMDT_ERROR_INTERNAL;
            fprintf(stderr, "StartTrace() failed with %lu\n", status);
        }
    }

    if (AMDT_STATUS_OK == retVal)
    {
        m_tpState = AMDT_THREAD_PROFILE_STATE_STARTED;
    }

    return retVal;
} // tpStartThreadProfile

AMDTResult tpCollectImpl::tpStopThreadProfile()
{
    AMDTResult retVal = AMDT_STATUS_OK;
    ULONG status = ERROR_SUCCESS;

    //if (!tpIsProfileInitialized())
    //{
    //    retVal = AMDT_ERROR_PROFILE_NOT_CONFIGURED;
    //}

    if (!tpIsProfileRunning())
    {
        retVal = AMDT_ERROR_PROFILE_NOT_STARTED;
    }

    PEVENT_TRACE_PROPERTIES pSessionProperties = m_pSessionProperties;
    bool isCollectCStack = m_callstack;

    if (NULL == pSessionProperties)
    {
        retVal = AMDT_ERROR_INTERNAL;
    }

    //fprintf(stderr, "tpStopThreadProfile .. retVal(%lx), state(%u)\n", retVal, m_tpState);

    if (AMDT_STATUS_OK == retVal)
    {
        // To disable stack tracing, call this function with InformationClass set to TraceStackTracingInfo
        // and InformationLength set to 0
        if (isCollectCStack)
        {
            status = TraceSetInformation(m_sessionHandle, TraceStackTracingInfo, NULL, 0);
            fprintf(stderr, "Disable stack tracing - TraceSetInformation() status(%lu)\n", status);
        }

        status = ControlTrace(m_sessionHandle,
                              KERNEL_LOGGER_NAME,
                              pSessionProperties,
                              EVENT_TRACE_CONTROL_STOP);

        //fprintf(stderr, "ControlTrace(stop) status(%lu)\n", status);

        if (ERROR_SUCCESS != status)
        {
            // FIXME: what to do if it fails
            fprintf(stderr, "ControlTrace(stop) failed with %lu\n", status);
        }

        // TODO: set this only is top succeeds
        m_tpState = AMDT_THREAD_PROFILE_STATE_UNINITIALIZED;
        status = tpClear();
    }

    return status;
} // tpStopThreadProfile
