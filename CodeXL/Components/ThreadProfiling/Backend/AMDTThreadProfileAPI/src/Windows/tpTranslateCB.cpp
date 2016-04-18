//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpTranslateCB.cpp
///
//==================================================================================

// System headers
#include <windows.h>
#include <Ole2.h> // FIXME 
#include <stdio.h>
#include <strsafe.h>
#include <wmistr.h>
#include <evntrace.h>
#include <evntcons.h>
#include <tdh.h>

#include <wchar.h>
#include <in6addr.h> // FIXME not reqd

// Project headers
#include <tpInternalDataTypes.h>
#include <tpTranslate.h>
#include <tpTranslateCB.h>

#pragma comment(lib, "tdh.lib") // FIXME - should this be in tpInternalDataTypes.h

//
//  Globals
//

static bool             g_userMode;
static bool             g_verbose = false;      // print while processing ETL records
static gtUInt32         g_timerResolution;
static gtUInt32         g_pointerSize;
static FILE*            g_debugLogFP = stdout;
static wchar_t          g_pEtlFile[TP_MAX_ETL_PATH_LEN];

// FIXME - Do i need this
static char             g_eventTypeStr[TP_ETW_EVENT_TYPE_MAX][32];

tpTranslate* g_pTranslate;
bool g_handleProcessEvent = false;
bool g_handleCSwitchEvent = false;

PTRACE_EVENT_INFO       g_pTraceEventInfo = NULL;
PBYTE                   g_pPropertyBuffer = NULL;

//
//  Event Trace Callbacks
//

ULONG WINAPI EtwProcessBufferCB(PEVENT_TRACE_LOGFILE pBuffer)
{
    UNREFERENCED_PARAMETER(pBuffer);

    return 0;
} // EtwProcessBufferCB

void WINAPI EtwProcessEventCB(PEVENT_TRACE pEventTrace)
{
    if (g_verbose)
    {
        fprintf(g_debugLogFP, "Time: %llu\n", pEventTrace->Header.TimeStamp.QuadPart);
        fprintf(g_debugLogFP, "Processor number: %u\n", pEventTrace->BufferContext.ProcessorNumber);

        if (pEventTrace->MofData)
        {
            fprintf(g_debugLogFP, "has MOF data\n");
        }
    }

    return;
} // EtwProcessEventCB

void WINAPI EtwProcessEventRecordCB(PEVENT_RECORD pEventRecord)
{
    HRESULT status = S_OK;
    PTRACE_EVENT_INFO pTraceEventInfo = NULL;
    LPWSTR pEventGuid = NULL;

    if (NULL == g_pTraceEventInfo)
    {
        g_pTraceEventInfo = (PTRACE_EVENT_INFO)malloc(TP_TRACE_EVENT_INFO_MAXSIZE);
    }

    pTraceEventInfo = g_pTraceEventInfo;

    if (NULL != pEventRecord)
    {
        // Skip event trace header. This is available in EVENT_TRACE_LOGFILE.LogfileHeader
        //
        // if (IsEqualGUID(pEventRecord->EventHeader.ProviderId, EventTraceGuid) &&
        //     pEventRecord->EventHeader.EventDescriptor.Opcode == EVENT_TRACE_TYPE_INFO)
        // {
        //     // just printf.. skipping
        //     fprintf(g_debugLogFP, "Log file header\n");
        // }

        status = GetTdhTraceEventInfo(pEventRecord, &pTraceEventInfo);

        // Check whether the event is defined by
        //      MOF class
        //      in an instrumentation manifest
        //      WPP template

        if (S_OK == status)
        {
            // MOF class
            if (DecodingSourceWbem == pTraceEventInfo->DecodingSource)
            {
                HRESULT hr = StringFromCLSID(pTraceEventInfo->EventGuid, &pEventGuid);

                if (SUCCEEDED(hr))
                {
                    // Process the Trace Event Record - MOF
                    ProcessTraceEventRecordMOF(pEventRecord, pTraceEventInfo);

                    // If verbose requested, print the text format of Trace Event Record
                    if (g_verbose)
                    {
                        PrintTraceEventRecordMOF(pEventRecord, pTraceEventInfo);
                    }

                    // free the GUID memory
                    if (g_verbose)
                    {
                        fwprintf(g_debugLogFP, L"\nEvent GUID: %s\n", pEventGuid);
                    }

                    CoTaskMemFree(pEventGuid);
                    pEventGuid = NULL;
                }
                else
                {
                    fprintf(stderr, "StringFromCLSID failed with 0x%x\n", hr);
                    status = hr;
                }
            }
            else
            {
                // we are not interested in others
                fprintf(stderr, "Non MOF\n");
            }
        }
    }
    else
    {
        // TODO: what if is pEvent is NULL
        fprintf(stderr, "pEvent is NULL\n");
    }

    return;
} // EtwProcessEventRecordCB


void WINAPI EtwProcessMetaEventRecordsCB(PEVENT_RECORD pEventRecord)
{
    HRESULT status = S_OK;
    PTRACE_EVENT_INFO pTraceEventInfo = NULL;
#ifdef DEBUG
    LPWSTR pEventGuid = NULL;
#endif

    pTraceEventInfo = g_pTraceEventInfo;

    if (NULL != pEventRecord && IsMetaEvent(pEventRecord))
    {
        status = GetTdhTraceEventInfo(pEventRecord, &pTraceEventInfo);

        if (S_OK == status)
        {
            // MOF class
            if (DecodingSourceWbem == pTraceEventInfo->DecodingSource)
            {
#ifdef DEBUG
                HRESULT hr = StringFromCLSID(pTraceEventInfo->EventGuid, &pEventGuid);

                if (SUCCEEDED(hr))
#endif // DEBUG
                {
                    // Process the Trace Event Record - MOF
                    ProcessMetaTraceEventRecordsMOF(pEventRecord, pTraceEventInfo);
#ifdef DEBUG

                    // If verbose requested, print the text format of Trace Event Record
                    if (g_verbose)
                    {
                        PrintTraceEventRecordMOF(pEventRecord, pTraceEventInfo);
                    }

                    CoTaskMemFree(pEventGuid);
                    pEventGuid = NULL;
#endif // DEBUG
                }

#ifdef DEBUG
                else
                {
                    fprintf(stderr, "StringFromCLSID failed with 0x%x\n", hr);
                    status = hr;
                }

#endif // DEBUG
            }
            else
            {
                // we are not interested in others
                fprintf(stderr, "Non MOF\n");
            }
        }
    }
    else
    {
        // TODO: what if is pEvent is NULL
        //fprintf(stderr, "pEvent is NULL\n");
    }

    return;
} // EtwProcessMetaEventRecordsCB


void WINAPI EtwProcessCSwitchEventRecordsCB(PEVENT_RECORD pEventRecord)
{
    HRESULT status = S_OK;
    PTRACE_EVENT_INFO pTraceEventInfo = NULL;
#ifdef DEBUG
    LPWSTR pEventGuid = NULL;
#endif

    pTraceEventInfo = g_pTraceEventInfo;

    if (NULL != pEventRecord && IsCSwitchEvent(pEventRecord))
    {
        status = GetTdhTraceEventInfo(pEventRecord, &pTraceEventInfo);

        if (S_OK == status)
        {
            // MOF class
            if (DecodingSourceWbem == pTraceEventInfo->DecodingSource)
            {
#ifdef DEBUG
                HRESULT hr = StringFromCLSID(pTraceEventInfo->EventGuid, &pEventGuid);

                if (SUCCEEDED(hr))
#endif // DEBUG
                {
                    // Process the Trace Event Record - MOF
                    ProcessCSwitchTraceEventRecordsMOF(pEventRecord, pTraceEventInfo);
#ifdef DEBUG

                    // If verbose requested, print the text format of Trace Event Record
                    if (g_verbose)
                    {
                        PrintTraceEventRecordMOF(pEventRecord, pTraceEventInfo);
                    }

                    CoTaskMemFree(pEventGuid);
                    pEventGuid = NULL;
#endif // DEBUG
                }

#ifdef DEBUG
                else
                {
                    fprintf(stderr, "StringFromCLSID failed with 0x%x\n", hr);
                    status = hr;
                }

#endif // DEBUG
            }
            else
            {
                // we are not interested in others
                fprintf(stderr, "Non MOF\n");
            }
        }
    }
    else
    {
        // TODO: what if is pEvent is NULL
        //fprintf(stderr, "pEvent is NULL\n");
    }

    return;
} // EtwProcessCSwitchEventRecordsCB


AMDTResult SettranslateHandle(AMDTThreadProfileDataHandle dataHhandle,
                              PTRACE_EVENT_INFO pTraceEventInfo,
                              PBYTE pPropertyBuffer)
{
    g_pTranslate = static_cast<tpTranslate*>(dataHhandle);
    g_pTraceEventInfo = pTraceEventInfo;
    g_pPropertyBuffer = pPropertyBuffer;

    // FIXME: this is not supposed to be done here
    if (g_verbose)
    {
        PrintInitialize();
    }

    return AMDT_STATUS_OK;
}


//
//  Helper functions
//


AMDTResult GetTdhTraceEventInfo(PEVENT_RECORD pEventRecord,
                                PTRACE_EVENT_INFO* ppTraceEventInfo)
{
    AMDTResult status = ERROR_INVALID_PARAMETER;
    PTRACE_EVENT_INFO pTraceEventInfo = NULL;
    DWORD bufSize = 0;

    if ((NULL != pEventRecord) && (NULL != ppTraceEventInfo))
    {
        //pTraceEventInfo = *ppTraceEventInfo;

        // Process the Data
        status = TdhGetEventInformation(pEventRecord, 0, NULL, pTraceEventInfo, &bufSize);

        if (ERROR_INSUFFICIENT_BUFFER == status)
        {
            pTraceEventInfo = *ppTraceEventInfo;

            if (bufSize > TP_TRACE_EVENT_INFO_MAXSIZE)
            {
                pTraceEventInfo = (TRACE_EVENT_INFO*)realloc(pTraceEventInfo, bufSize);
            }

            if (pTraceEventInfo != NULL)
            {
                // Retrieve the event metadata.
                status = TdhGetEventInformation(pEventRecord, 0, NULL, pTraceEventInfo, &bufSize);
            }
            else
            {
                status = ERROR_OUTOFMEMORY;
            }
        }

        if (ERROR_SUCCESS == status)
        {
            *ppTraceEventInfo = pTraceEventInfo;
        }
        else if (ERROR_OUTOFMEMORY == status)
        {
            fprintf(stderr, "Failed to allocate memory for event info (size=%lu).\n", bufSize);
        }
        else
        {
            fprintf(stderr, "TdhGetEventInformation failed with 0x%x.\n", status);
        }
    }

    return status;
}


// ProcessTraceEventRecordMOF
//
// Main function that processes all the event trace records
//
AMDTResult ProcessTraceEventRecordMOF(PEVENT_RECORD pEventRecord, PTRACE_EVENT_INFO pTraceEventInfo)
{
    AMDTResult hr = AMDT_STATUS_OK;
    ThreadProfileEventProcess processRec;
    ThreadProfileEventThread threadRec;
    ThreadProfileEventCSwitch csRec;
    ThreadProfileEventStack stackRec;

    if ((NULL != pEventRecord) && (NULL != pTraceEventInfo))
    {
        ThreadProfileEventInfo traceEventInfo;
        GetTraceEventInfo(pEventRecord, pTraceEventInfo, traceEventInfo);

        // If the event contains event-specific data use TDH to extract
        // the event data. To extract the data, the event
        // must be defined by a MOF class or an instrumentation manifest.

        // CSWitch
        switch (traceEventInfo.m_eventType)
        {
            case TP_EVENT_TYPE_UNKNOWN:
            case TP_EVENT_TYPE_INFO:
                break;

            case TP_EVENT_TYPE_PROCESS_START:
                if (g_handleProcessEvent)
                {
                    processRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                    processRec.m_processorId = traceEventInfo.m_processorId;

                    ProcessProcessRecord(pEventRecord, pTraceEventInfo, processRec);

                    // TODO: shouldn't we check the return code from ProcessProcessRecord
                    g_pTranslate->AddProcessStartEvent(processRec);
                }

                break;

            case TP_EVENT_TYPE_PROCESS_STOP:
                if (g_handleProcessEvent)
                {
                    processRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                    processRec.m_processorId = traceEventInfo.m_processorId;

                    ProcessProcessRecord(pEventRecord, pTraceEventInfo, processRec);

                    // TODO: shouldn't we check the return code from ProcessProcessRecord
                    g_pTranslate->AddProcessStopEvent(processRec);
                }

                break;

            case TP_EVENT_TYPE_PROCESS_DEFUNCT:
            case TP_EVENT_TYPE_IMAGE_LOAD:
            case TP_EVENT_TYPE_IMAGE_UNLOAD:
                break;

            case TP_EVENT_TYPE_THREAD_START:
                if (g_handleProcessEvent)
                {
                    threadRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                    threadRec.m_processorId = traceEventInfo.m_processorId;

                    ProcessThreadRecord(pEventRecord, pTraceEventInfo, threadRec);

                    // TODO: shouldn't we check the return code from ProcessThreadRecord
                    g_pTranslate->AddThreadStartEvent(threadRec);
                }

                break;

            case TP_EVENT_TYPE_THREAD_STOP:
                if (g_handleProcessEvent)
                {
                    threadRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                    threadRec.m_processorId = traceEventInfo.m_processorId;

                    ProcessThreadRecord(pEventRecord, pTraceEventInfo, threadRec);

                    // TODO: shouldn't we check the return code from ProcessThreadRecord
                    g_pTranslate->AddThreadStopEvent(threadRec);
                }

                break;

            case TP_EVENT_TYPE_THREAD_READY:
                // FIXME what to do with this event?
                break;

            case TP_EVENT_TYPE_CSWITCH:
                if (g_handleCSwitchEvent)
                {
                    csRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                    csRec.m_processorId = traceEventInfo.m_processorId;

                    ProcessCSwitchRecord(pEventRecord, pTraceEventInfo, csRec);

                    // TODO: shouldn't we check the return code from ProcessCSwitchRecord
                    g_pTranslate->AddCSwitchEvent(csRec);
                }

                break;

            case TP_EVENT_TYPE_STACK:
                if (g_handleCSwitchEvent)
                {
                    stackRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                    stackRec.m_processorId = traceEventInfo.m_processorId;

                    ProcessStackRecord(pEventRecord, pTraceEventInfo, stackRec);

                    // TODO: shouldn't we check the return code from ProcessStackRecord
                    g_pTranslate->AddStackEvent(stackRec);
                }

                break;
        } // switch (eventType)

        if (g_verbose)
        {
            fprintf(g_debugLogFP, "\n");
        }
    }

    return hr;
} // ProcessTraceEventRecordMOF


// ProcessMetaTraceEventRecordsMOF
//
// Main function that processes teh following event trace records - info/image/process/threads
//
AMDTResult ProcessMetaTraceEventRecordsMOF(PEVENT_RECORD pEventRecord, PTRACE_EVENT_INFO pTraceEventInfo)
{
    AMDTResult hr = AMDT_STATUS_OK;
    ThreadProfileEventGeneric genericRec;
    ThreadProfileEventProcess processRec;
    ThreadProfileEventThread threadRec;
    ThreadProfileEventImage imageRec;

    if ((NULL != pEventRecord) && (NULL != pTraceEventInfo))
    {
        ThreadProfileEventInfo traceEventInfo;
        GetTraceEventInfo(pEventRecord, pTraceEventInfo, traceEventInfo);

        // If the event contains event-specific data use TDH to extract
        // the event data. To extract the data, the event
        // must be defined by a MOF class or an instrumentation manifest.

        // CSWitch
        switch (traceEventInfo.m_eventType)
        {
            case TP_EVENT_TYPE_UNKNOWN:
                break;

            case TP_EVENT_TYPE_INFO:
                genericRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                genericRec.m_processorId = traceEventInfo.m_processorId;

                // TODO check return code
                ProcessInfoRecord(pEventRecord, pTraceEventInfo, genericRec);

                g_pTranslate->AddInfoEvent(genericRec);
                break;

            case TP_EVENT_TYPE_PROCESS_START:
                processRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                processRec.m_processorId = traceEventInfo.m_processorId;

                // TODO check return code
                ProcessProcessRecord(pEventRecord, pTraceEventInfo, processRec);

                g_pTranslate->AddProcessStartEvent(processRec);
                break;

            case TP_EVENT_TYPE_PROCESS_STOP:
                processRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                processRec.m_processorId = traceEventInfo.m_processorId;

                // TODO check retval
                ProcessProcessRecord(pEventRecord, pTraceEventInfo, processRec);

                g_pTranslate->AddProcessStopEvent(processRec);
                break;

            case TP_EVENT_TYPE_PROCESS_DEFUNCT:
                break;

            case TP_EVENT_TYPE_IMAGE_LOAD:
                imageRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                imageRec.m_processorId = traceEventInfo.m_processorId;

                // TODO check retval
                ProcessImageRecord(pEventRecord, pTraceEventInfo, imageRec);

                g_pTranslate->AddImageLoadEvent(imageRec);
                break;

            case TP_EVENT_TYPE_IMAGE_UNLOAD:
                break;

            case TP_EVENT_TYPE_THREAD_START:
                threadRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                threadRec.m_processorId = traceEventInfo.m_processorId;

                // TODO check return code
                ProcessThreadRecord(pEventRecord, pTraceEventInfo, threadRec);

                g_pTranslate->AddThreadStartEvent(threadRec);
                break;

            case TP_EVENT_TYPE_THREAD_STOP:
                threadRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                threadRec.m_processorId = traceEventInfo.m_processorId;

                // TODO check return code
                ProcessThreadRecord(pEventRecord, pTraceEventInfo, threadRec);

                g_pTranslate->AddThreadStopEvent(threadRec);
                break;

            case TP_EVENT_TYPE_THREAD_READY:
                // FIXME what to do with this event?
                break;

            case TP_EVENT_TYPE_CSWITCH:
                break;

            case TP_EVENT_TYPE_STACK:
                break;
        } // switch (eventType)
    }

    return hr;
} // ProcessMetaTraceEventRecordsMOF


// ProcessCSwitchTraceEventRecordsMOF
//
// Main function that processes the following event trace records - cswitch and cstack
//
AMDTResult ProcessCSwitchTraceEventRecordsMOF(PEVENT_RECORD pEventRecord, PTRACE_EVENT_INFO pTraceEventInfo)
{
    AMDTResult hr = AMDT_STATUS_OK;
    ThreadProfileEventCSwitch csRec;
    ThreadProfileEventStack stackRec;

    if ((NULL != pEventRecord) && (NULL != pTraceEventInfo))
    {
        ThreadProfileEventInfo traceEventInfo;
        GetTraceEventInfo(pEventRecord, pTraceEventInfo, traceEventInfo);

        // If the event contains event-specific data use TDH to extract
        // the event data. To extract the data, the event
        // must be defined by a MOF class or an instrumentation manifest.

        // CSWitch
        switch (traceEventInfo.m_eventType)
        {
            case TP_EVENT_TYPE_CSWITCH:
                csRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                csRec.m_processorId = traceEventInfo.m_processorId;

                // TODO check return code
                ProcessCSwitchRecord(pEventRecord, pTraceEventInfo, csRec);

                g_pTranslate->AddCSwitchEvent(csRec);
                break;

            case TP_EVENT_TYPE_STACK:
                stackRec.m_timeStamp = traceEventInfo.m_timeStamp.QuadPart;
                stackRec.m_processorId = traceEventInfo.m_processorId;

                // TODO check return code
                ProcessStackRecord(pEventRecord, pTraceEventInfo, stackRec);

                g_pTranslate->AddStackEvent(stackRec);
                break;

            default:
                break;
        } // switch (eventType)
    }

    return hr;
} // ProcessCSwitchTraceEventRecordsMOF


// GetTraceEventType
//
void GetTraceEventType(GUID guid, UCHAR opcode, ThreadProfileEvent& event, ThreadProfileEventType& eventType)
{
    event = TP_EVENT_UNKNOWN;
    eventType = TP_EVENT_TYPE_UNKNOWN;

    //    TP_EVENT_TYPE_PROCESS_START
    //    TP_EVENT_TYPE_PROCESS_STOP
    //    TP_EVENT_TYPE_IMAGE_LOAD
    //    TP_EVENT_TYPE_IMAGE_UNLOAD
    //    TP_EVENT_TYPE_THREAD_START
    //    TP_EVENT_TYPE_THREAD_STOP
    //    TP_EVENT_TYPE_CSWITCH
    //    TP_EVENT_TYPE_STACK
    //strcpy(gEventTypeStr[10], "Load");
    //strcpy(gEventTypeStr[32], "Stack");
    //strcpy(gEventTypeStr[36], "CSwitch");
    //strcpy(gEventTypeStr[39], "Defunt");
    //strcpy(gEventTypeStr[50], "ReadyThread");

    if (IsEqualGUID(guid, EventTraceGuid) && (EVENT_TRACE_TYPE_INFO == opcode))
    {
        event = TP_EVENT_INFO;
        eventType = TP_EVENT_TYPE_INFO;
    }
    else if (IsEqualGUID(guid, g_imageGuid))
    {
        event = TP_EVENT_IMAGE;

        if ((EVENT_TRACE_TYPE_LOAD == opcode) || (EVENT_TRACE_TYPE_DC_START == opcode) || EVENT_TRACE_TYPE_DC_END == opcode)
        {
            eventType = TP_EVENT_TYPE_IMAGE_LOAD;
        }
        else if (EVENT_TRACE_TYPE_END == opcode)
        {
            eventType = TP_EVENT_TYPE_IMAGE_UNLOAD;
        }
    }
    else if (IsEqualGUID(guid, g_processGuid))
    {
        event = TP_EVENT_PROCESS;

        if ((EVENT_TRACE_TYPE_START == opcode) || (EVENT_TRACE_TYPE_DC_START == opcode) || (EVENT_TRACE_TYPE_DC_END == opcode))
        {
            eventType = TP_EVENT_TYPE_PROCESS_START;
        }
        else if (EVENT_TRACE_TYPE_END == opcode)
        {
            eventType = TP_EVENT_TYPE_PROCESS_STOP;
        }
        else if (39 == opcode)
        {
            eventType = TP_EVENT_TYPE_PROCESS_DEFUNCT;
        }
    }
    else if (IsEqualGUID(guid, g_threadGuid))
    {
        event = TP_EVENT_THREAD;

        if ((EVENT_TRACE_TYPE_START == opcode) || (EVENT_TRACE_TYPE_DC_START == opcode) || EVENT_TRACE_TYPE_DC_END == opcode)
        {
            eventType = TP_EVENT_TYPE_THREAD_START;
        }
        else if (EVENT_TRACE_TYPE_END == opcode)
        {
            eventType = TP_EVENT_TYPE_THREAD_STOP;
        }
        else if (36 == opcode)
        {
            eventType = TP_EVENT_TYPE_CSWITCH;
        }
        else if (50 == opcode)
        {
            eventType = TP_EVENT_TYPE_THREAD_READY;
        }
    }
    else if (IsEqualGUID(guid, g_stackWalkGuid))
    {
        event = TP_EVENT_STACK;

        if (32 == opcode)
        {
            eventType = TP_EVENT_TYPE_STACK;
        }
    }

    return;
} // GetTraceEventType


// GetTraceEventSring
//
char* GetTraceEventSring(ThreadProfileEvent event)
{
    switch (event)
    {
        case TP_EVENT_INFO:
            return "Info";

        case TP_EVENT_IMAGE:
            return "Image";

        case TP_EVENT_PROCESS:
            return "Process";

        case TP_EVENT_THREAD:
            return "Thread";

        case TP_EVENT_CSWITCH:
            return "CSwitch";

        default:
            break;
    }

    return "Unknown";
} // GetTraceEventSring

// GetTraceEventTypeSring
//
char* GetTraceEventTypeSring(ThreadProfileEventType event)
{
    switch (event)
    {
        case TP_EVENT_TYPE_INFO:
            return "Info";

        case TP_EVENT_TYPE_IMAGE_LOAD:
            return "Image Load";

        case TP_EVENT_TYPE_IMAGE_UNLOAD:
            return "Image UnLoad";

        case TP_EVENT_TYPE_PROCESS_START:
            return "Process Start";

        case TP_EVENT_TYPE_PROCESS_STOP:
            return "Process Stop";

        case TP_EVENT_TYPE_PROCESS_DEFUNCT:
            return "Process Defunct";

        case TP_EVENT_TYPE_THREAD_START:
            return "Thread Start";

        case TP_EVENT_TYPE_THREAD_STOP:
            return "Thread Stop";

        case TP_EVENT_TYPE_THREAD_READY:
            return "Thread Ready";

        case TP_EVENT_TYPE_CSWITCH:
            return "CSwitch";

        case TP_EVENT_TYPE_STACK:
            return "Stack";

        default:
            break;
    }

    return "Unknown";
}

AMDTResult GetTraceEventInfo(PEVENT_RECORD pEventRecord, PTRACE_EVENT_INFO pTraceEventInfo, ThreadProfileEventInfo& info)
{
    AMDTResult hr = AMDT_ERROR_FAIL;

    if ((NULL != pEventRecord) && (NULL != pTraceEventInfo))
    {
        info.m_timeStamp = pEventRecord->EventHeader.TimeStamp;

        GetTraceEventType(pEventRecord->EventHeader.ProviderId,                // GUID
                          pEventRecord->EventHeader.EventDescriptor.Opcode,    // opcode
                          info.m_event,
                          info.m_eventType);

        // Time stamp for when the event occurred.
        info.m_timeStamp = pEventRecord->EventHeader.TimeStamp;

        info.m_processId = pEventRecord->EventHeader.ProcessId;
        info.m_threadId = pEventRecord->EventHeader.ThreadId;
        info.m_procTime = pEventRecord->EventHeader.ProcessorTime;
        info.m_kernelTime = pEventRecord->EventHeader.KernelTime;
        info.m_userTime = pEventRecord->EventHeader.UserTime;
        // TODO: add pEventRecord->EventHeader.Size, pEventRecord->EventHeader.Flags, pEventRecord->EventHeader.EventProperty

        // TODO: other fields in pEventRecord->EventHeader.EventDescriptor
        // - Id, Channel, Version, Level, Opcode, Task, Keyword
        info.m_version = pEventRecord->EventHeader.EventDescriptor.Version;
        info.m_opcode = pEventRecord->EventHeader.EventDescriptor.Opcode;
        info.m_flags = 0; // FIXME

        // Need to get the PointerSize for each event to cover the case where you are
        // consuming events from multiple log files that could have been generated on
        // different architectures. Otherwise, you could have accessed the pointer
        // size when you opened the trace above (see pHeader->PointerSize).
        if (EVENT_HEADER_FLAG_32_BIT_HEADER == (pEventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_32_BIT_HEADER))
        {
            info.m_pointerSize = 4;
            g_pointerSize = 4;
        }
        else
        {
            info.m_pointerSize = 8;
            g_pointerSize = 8;
        }

        // processor id
        if ((pEventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_PROCESSOR_INDEX) != 0)
        {
            info.m_processorId = pEventRecord->BufferContext.ProcessorIndex;
        }
        else
        {
            info.m_processorId = pEventRecord->BufferContext.ProcessorNumber;
        }


        hr = AMDT_STATUS_OK;
    }

    return hr;
}


AMDTResult IsMetaEvent(PEVENT_RECORD pEventRecord)
{
    GUID guid = pEventRecord->EventHeader.ProviderId;
    UCHAR opcode = pEventRecord->EventHeader.EventDescriptor.Opcode;

    if ((IsEqualGUID(guid, g_threadGuid) && (36 == opcode))
        || (IsEqualGUID(guid, g_stackWalkGuid)))
    {
        return false;
    }

    return true;
}

AMDTResult IsCSwitchEvent(PEVENT_RECORD pEventRecord)
{
    GUID guid = pEventRecord->EventHeader.ProviderId;
    UCHAR opcode = pEventRecord->EventHeader.EventDescriptor.Opcode;

    if ((IsEqualGUID(guid, g_threadGuid) && (36 == opcode))
        || (IsEqualGUID(guid, g_stackWalkGuid)))
    {
        return true;
    }

    return false;
}

int ProcessInfoRecord(PEVENT_RECORD pEventRecord,
                      PTRACE_EVENT_INFO pTraceEventInfo,
                      ThreadProfileEventGeneric& genericRec)
{
    int status = AMDT_ERROR_FAIL;

    // Print the event data for all the top-level properties. Metadata for all the
    // top-level properties come before structure member properties in the
    // property information array. If the EVENT_HEADER_FLAG_STRING_ONLY flag is set,
    // the event data is a null-terminated string, so just print it.
    if (EVENT_HEADER_FLAG_STRING_ONLY == (pEventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_STRING_ONLY))
    {
        if (g_verbose)
        {
            fprintf(g_debugLogFP, " %ws ", (LPWSTR)pEventRecord->UserData);
        }
    }
    else
    {
        if (g_verbose)
        {
            fprintf(g_debugLogFP, "TopLevelPropertyCount : %d ", pTraceEventInfo->TopLevelPropertyCount);
        }

        // !!! DO NOT PROCESS aLL THE FIELDS !!
        //for (USHORT i = 0; i < pTraceEventInfo->TopLevelPropertyCount; i++)
        //{
        //    status = ProcessInfoRecordData(pEventRecord, pTraceEventInfo, i, NULL, 0, processRec);
        //}

        //  process only the important fields
        // NumberOfProcessors
        USHORT propertyIdx[] = { 3 };

        for (USHORT i = 0; i < (sizeof(propertyIdx) / sizeof(USHORT)); i++)
        {
            status = ProcessInfoRecordData(pEventRecord, pTraceEventInfo, propertyIdx[i], NULL, 0, genericRec);
        }
    }

    //fprintf(stderr, "Process record done.. %d\n", status);
    return status;
} // ProcessInfoRecord


// ProcessInfoRecordData
//
AMDTResult ProcessInfoRecordData(PEVENT_RECORD pEvent,
                                 PTRACE_EVENT_INFO pInfo,
                                 USHORT i,
                                 LPWSTR pStructureName,
                                 USHORT StructIndex,
                                 ThreadProfileEventGeneric& genericRec)
{
    AMDTResult retVal = AMDT_ERROR_FAIL;
    DWORD status = ERROR_SUCCESS;
    USHORT ArraySize = 0;

    PROPERTY_DATA_DESCRIPTOR DataDescriptor;
    PROPERTY_DATA_DESCRIPTOR DataDescriptors[2];
    ULONG DescriptorsCount = 0;
    DWORD PropertySize = 0;
    PBYTE pData = NULL;

    // Get the property data
    status = GetTdhPropertyData(pEvent, pInfo, i, &DataDescriptor, &PropertySize, &pData, &ArraySize);

    if (ERROR_SUCCESS == status)
    {
        // For CSWitch ArraySize == 1
        for (USHORT k = 0; k < ArraySize; k++)
        {
            LPWSTR propertyName = (LPWSTR)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);

            if (g_verbose)
            {
                fwprintf(stdout, L" %s ", propertyName);
            }

            ZeroMemory(&DataDescriptors, sizeof(DataDescriptors));

            // To retrieve a member of a structure, you need to specify an array of descriptors.
            // The first descriptor in the array identifies the name of the structure and the second
            // descriptor defines the member of the structure whose data you want to retrieve.

            if (pStructureName)
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)pStructureName;
                DataDescriptors[0].ArrayIndex = StructIndex;
                DataDescriptors[1].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[1].ArrayIndex = k;
                DescriptorsCount = 2;
            }
            else
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[0].ArrayIndex = k;
                DescriptorsCount = 1;
            }

            status = GetTdhProperty(pEvent, DescriptorsCount, &DataDescriptors[0], &PropertySize, &pData);

            if (ERROR_SUCCESS != status)
            {
                fwprintf(stdout, L"TPGetProperty failed with %lu\n", status);
                break;
            }

            // use the propertyname and the InType to fillin the data
            USHORT inType = pInfo->EventPropertyInfoArray[i].nonStructType.InType;

            switch (i)
            {
                case 3:
                    genericRec.m_numberOfProcessors = GetUInt32(inType, pData);
                    break;

                default:
                    break;
            }
        }
    }

    if (ERROR_SUCCESS != status)
    {
        fwprintf(stdout, L"TPGetPropertyData failed\n");
    }

    //if (NULL != pData)
    //{
    //    free(pData);
    //    pData = NULL;
    //}

    retVal = (ERROR_SUCCESS == status) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
    return retVal;
} // ProcessInfoRecordData


int ProcessProcessRecord(PEVENT_RECORD pEventRecord,
                         PTRACE_EVENT_INFO pTraceEventInfo,
                         ThreadProfileEventProcess& processRec)
{
    int status = AMDT_ERROR_FAIL;

    // Print the event data for all the top-level properties. Metadata for all the
    // top-level properties come before structure member properties in the
    // property information array. If the EVENT_HEADER_FLAG_STRING_ONLY flag is set,
    // the event data is a null-terminated string, so just print it.
    if (EVENT_HEADER_FLAG_STRING_ONLY == (pEventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_STRING_ONLY))
    {
        if (g_verbose)
        {
            fprintf(g_debugLogFP, " %ws ", (LPWSTR)pEventRecord->UserData);
        }
    }
    else
    {
        if (g_verbose)
        {
            fprintf(g_debugLogFP, "TopLevelPropertyCount : %d ", pTraceEventInfo->TopLevelPropertyCount);
        }

        // !!! DO NOT PROCESS aLL THE FIELDS !!
        //for (USHORT i = 0; i < pTraceEventInfo->TopLevelPropertyCount; i++)
        //{
        //    status = ProcessProcessRecordData(pEventRecord, pTraceEventInfo, i, NULL, 0, processRec);
        //}

        //  process only the important fields
        // processId, parentId, existStatus, imageFileName, commandLine
        USHORT propertyIdx[] = { 1, 2, 4, 7, 8 };

        for (USHORT i = 0; i < (sizeof(propertyIdx) / sizeof(USHORT)); i++)
        {
            status = ProcessProcessRecordData(pEventRecord, pTraceEventInfo, propertyIdx[i], NULL, 0, processRec);
        }
    }

    //fprintf(stderr, "Process record done.. %d\n", status);
    return status;
} // ProcessProcessRecord


// ProcessProcessRecordData
//
AMDTResult ProcessProcessRecordData(PEVENT_RECORD pEvent,
                                    PTRACE_EVENT_INFO pInfo,
                                    USHORT i,
                                    LPWSTR pStructureName,
                                    USHORT StructIndex,
                                    ThreadProfileEventProcess& processRec)
{
    AMDTResult retVal = AMDT_ERROR_FAIL;
    DWORD status = ERROR_SUCCESS;
    USHORT ArraySize = 0;

    PROPERTY_DATA_DESCRIPTOR DataDescriptor;
    PROPERTY_DATA_DESCRIPTOR DataDescriptors[2];
    ULONG DescriptorsCount = 0;
    DWORD PropertySize = 0;
    PBYTE pData = NULL;
    size_t stringLen = 0;
    LPSTR imageFileName = NULL;
    LPWSTR cmdline = NULL;

    // Get the property data
    status = GetTdhPropertyData(pEvent, pInfo, i, &DataDescriptor, &PropertySize, &pData, &ArraySize);

    if (ERROR_SUCCESS == status)
    {
        // For CSWitch ArraySize == 1
        for (USHORT k = 0; k < ArraySize; k++)
        {
            LPWSTR propertyName = (LPWSTR)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);

            if (g_verbose)
            {
                fwprintf(stdout, L" %s ", propertyName);
            }

            ZeroMemory(&DataDescriptors, sizeof(DataDescriptors));

            // To retrieve a member of a structure, you need to specify an array of descriptors.
            // The first descriptor in the array identifies the name of the structure and the second
            // descriptor defines the member of the structure whose data you want to retrieve.

            if (pStructureName)
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)pStructureName;
                DataDescriptors[0].ArrayIndex = StructIndex;
                DataDescriptors[1].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[1].ArrayIndex = k;
                DescriptorsCount = 2;
            }
            else
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[0].ArrayIndex = k;
                DescriptorsCount = 1;
            }

            status = GetTdhProperty(pEvent, DescriptorsCount, &DataDescriptors[0], &PropertySize, &pData);

            if (ERROR_SUCCESS != status)
            {
                fwprintf(stdout, L"TPGetProperty failed with %lu\n", status);
                break;
            }

            // use the propertyname and the InType to fillin the data
            USHORT inType = pInfo->EventPropertyInfoArray[i].nonStructType.InType;

            switch (i)
            {
                case 0:
                    processRec.m_uniqueProcessKey = GetPointer(inType, pData);
                    break;

                case 1:
                    processRec.m_processId = GetUInt32(inType, pData);
                    break;

                case 2:
                    processRec.m_parentId = GetUInt32(inType, pData);
                    break;

                case 3:
                    processRec.m_sessionId = GetUInt32(inType, pData);
                    break;

                case 4:
                    processRec.m_exitStatus = GetInt32(inType, pData);
                    processRec.m_isActive = (processRec.m_exitStatus == STILL_ACTIVE) ? true : false;
                    break;

                case 5:
                    processRec.m_directoryTableBase = GetPointer(inType, pData);
                    break;

                case 6:
                    // FIXME
                    processRec.m_userSID = NULL;
                    break;

                case 7:
                    GetAnsiString(inType, pData, stringLen, imageFileName);

                    if (NULL != imageFileName)
                    {
                        processRec.m_imageFileName.fromASCIIString(imageFileName, stringLen);
                    }

                    break;

                case 8:
                    GetUnicodeString(inType, pData, stringLen, cmdline);

                    if (NULL != cmdline)
                    {
                        processRec.m_commandLine = cmdline;
                    }

                    break;
            }
        }
    }

    if (ERROR_SUCCESS != status)
    {
        fwprintf(stdout, L"TPGetPropertyData failed\n");
    }

    //if (NULL != pData)
    //{
    //    free(pData);
    //    pData = NULL;
    //}

    retVal = (ERROR_SUCCESS == status) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
    return retVal;
} // ProcessProcessRecordData


int ProcessImageRecord(PEVENT_RECORD pEventRecord,
                       PTRACE_EVENT_INFO pTraceEventInfo,
                       ThreadProfileEventImage& imageRec)
{
    int status = AMDT_ERROR_FAIL;

    // Print the event data for all the top-level properties. Metadata for all the
    // top-level properties come before structure member properties in the
    // property information array. If the EVENT_HEADER_FLAG_STRING_ONLY flag is set,
    // the event data is a null-terminated string, so just print it.
    if (EVENT_HEADER_FLAG_STRING_ONLY == (pEventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_STRING_ONLY))
    {
        if (g_verbose)
        {
            fprintf(g_debugLogFP, " %ws ", (LPWSTR)pEventRecord->UserData);
        }
    }
    else
    {
        if (g_verbose)
        {
            fprintf(g_debugLogFP, "TopLevelPropertyCount : %d ", pTraceEventInfo->TopLevelPropertyCount);
        }

        //  process only the important fields
        // imageBase, imageSize, processId, imageCheckSum, defaultBase, imageFileName
        USHORT propertyIdx[] = { 0, 1, 2, 3, 6, 11 };

        for (USHORT i = 0; i < (sizeof(propertyIdx) / sizeof(USHORT)); i++)
        {
            status = ProcessImageRecordData(pEventRecord, pTraceEventInfo, propertyIdx[i], NULL, 0, imageRec);
        }
    }

    //fprintf(stderr, "Process record done.. %d\n", status);
    return status;
} // ProcessImageRecord


// ProcessImageRecordData
//
AMDTResult ProcessImageRecordData(PEVENT_RECORD pEvent,
                                  PTRACE_EVENT_INFO pInfo,
                                  USHORT i,
                                  LPWSTR pStructureName,
                                  USHORT StructIndex,
                                  ThreadProfileEventImage& imageRec)
{
    AMDTResult retVal = AMDT_ERROR_FAIL;
    DWORD status = ERROR_SUCCESS;
    USHORT ArraySize = 0;

    PROPERTY_DATA_DESCRIPTOR DataDescriptor;
    PROPERTY_DATA_DESCRIPTOR DataDescriptors[2];
    ULONG DescriptorsCount = 0;
    DWORD PropertySize = 0;
    PBYTE pData = NULL;
    size_t stringLen = 0;
    LPWSTR imageFileName = NULL;

    // Get the property data
    status = GetTdhPropertyData(pEvent, pInfo, i, &DataDescriptor, &PropertySize, &pData, &ArraySize);

    if (ERROR_SUCCESS == status)
    {
        // For CSWitch ArraySize == 1
        for (USHORT k = 0; k < ArraySize; k++)
        {
            LPWSTR propertyName = (LPWSTR)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);

            if (g_verbose)
            {
                fwprintf(stdout, L" %s ", propertyName);
            }

            ZeroMemory(&DataDescriptors, sizeof(DataDescriptors));

            // To retrieve a member of a structure, you need to specify an array of descriptors.
            // The first descriptor in the array identifies the name of the structure and the second
            // descriptor defines the member of the structure whose data you want to retrieve.

            if (pStructureName)
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)pStructureName;
                DataDescriptors[0].ArrayIndex = StructIndex;
                DataDescriptors[1].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[1].ArrayIndex = k;
                DescriptorsCount = 2;
            }
            else
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[0].ArrayIndex = k;
                DescriptorsCount = 1;
            }

            status = GetTdhProperty(pEvent, DescriptorsCount, &DataDescriptors[0], &PropertySize, &pData);

            if (ERROR_SUCCESS != status)
            {
                fwprintf(stdout, L"TPGetProperty failed with %lu\n", status);
                break;
            }

            // use the propertyname and the InType to fillin the data
            USHORT inType = pInfo->EventPropertyInfoArray[i].nonStructType.InType;

            switch (i)
            {
                case 0:
                    imageRec.m_imageBase = GetPointer(inType, pData);
                    break;

                case 1:
                    imageRec.m_imageSize = GetPointer(inType, pData);
                    break;

                case 2:
                    imageRec.m_processId = GetUInt32(inType, pData);
                    break;

                case 3:
                    imageRec.m_imageCheckSum = GetUInt32(inType, pData);
                    break;

                case 4:
                    imageRec.m_timeDateStamp = GetInt32(inType, pData);
                    break;

                case 6:
                    imageRec.m_defaultBase = GetPointer(inType, pData);
                    break;

                case 11:
                    GetUnicodeString(inType, pData, stringLen, imageFileName);

                    if (NULL != imageFileName)
                    {
                        imageRec.m_fileName = imageFileName;
                    }

                    break;

                default:
                    break;
            }
        }
    }

    if (ERROR_SUCCESS != status)
    {
        fwprintf(stdout, L"TPGetPropertyData failed\n");
    }

    //if (NULL != pData)
    //{
    //    free(pData);
    //    pData = NULL;
    //}

    retVal = (ERROR_SUCCESS == status) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
    return retVal;
} // ProcessProcessRecordData


// ProcessThreadRecord
//
int ProcessThreadRecord(PEVENT_RECORD pEventRecord,
                        PTRACE_EVENT_INFO pTraceEventInfo,
                        ThreadProfileEventThread& threadRec)
{
    int status = AMDT_ERROR_FAIL;

    // Print the event data for all the top-level properties. Metadata for all the
    // top-level properties come before structure member properties in the
    // property information array. If the EVENT_HEADER_FLAG_STRING_ONLY flag is set,
    // the event data is a null-terminated string, so just print it.
    if (EVENT_HEADER_FLAG_STRING_ONLY == (pEventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_STRING_ONLY))
    {
        if (g_verbose)
        {
            fprintf(g_debugLogFP, " %ws ", (LPWSTR)pEventRecord->UserData);
        }
    }
    else
    {
        if (g_verbose)
        {
            fprintf(g_debugLogFP, "TopLevelPropertyCount : %d ", pTraceEventInfo->TopLevelPropertyCount);
        }

        // !!! DO NOT PROCESS aLL THE FIELDS !!
        //for (USHORT i = 0; i < pTraceEventInfo->TopLevelPropertyCount; i++)
        //{
        //    status = ProcessThreadRecordData(pEventRecord, pTraceEventInfo, i, NULL, 0, threadRec);
        //}

        // pid, tid, affinity
        USHORT propertyIdx[] = { 0, 1, 6 };

        for (USHORT i = 0; i < (sizeof(propertyIdx) / sizeof(USHORT)); i++)
        {
            status = ProcessThreadRecordData(pEventRecord, pTraceEventInfo, propertyIdx[i], NULL, 0, threadRec);
        }
    }

    //fprintf(stderr, "Thread record done.. %d\n", status);
    return status;
} // ProcessThreadRecord


// ProcessThreadRecordData
//
AMDTResult ProcessThreadRecordData(PEVENT_RECORD pEvent,
                                   PTRACE_EVENT_INFO pInfo,
                                   USHORT i,
                                   LPWSTR pStructureName,
                                   USHORT StructIndex,
                                   ThreadProfileEventThread& threadRec)
{
    AMDTResult retVal = AMDT_ERROR_FAIL;
    DWORD status = ERROR_SUCCESS;
    USHORT ArraySize = 0;

    PROPERTY_DATA_DESCRIPTOR DataDescriptor;
    PROPERTY_DATA_DESCRIPTOR DataDescriptors[2];
    ULONG DescriptorsCount = 0;
    DWORD PropertySize = 0;
    PBYTE pData = NULL;

    // Get the property data
    status = GetTdhPropertyData(pEvent, pInfo, i, &DataDescriptor, &PropertySize, &pData, &ArraySize);

    if (ERROR_SUCCESS == status)
    {
        // For CSWitch ArraySize == 1
        for (USHORT k = 0; k < ArraySize; k++)
        {
            LPWSTR propertyName = (LPWSTR)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);

            if (g_verbose)
            {
                fwprintf(stdout, L" %s ", propertyName);
            }

            ZeroMemory(&DataDescriptors, sizeof(DataDescriptors));

            // To retrieve a member of a structure, you need to specify an array of descriptors.
            // The first descriptor in the array identifies the name of the structure and the second
            // descriptor defines the member of the structure whose data you want to retrieve.

            if (pStructureName)
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)pStructureName;
                DataDescriptors[0].ArrayIndex = StructIndex;
                DataDescriptors[1].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[1].ArrayIndex = k;
                DescriptorsCount = 2;
            }
            else
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[0].ArrayIndex = k;
                DescriptorsCount = 1;
            }

            status = GetTdhProperty(pEvent, DescriptorsCount, &DataDescriptors[0], &PropertySize, &pData);

            if (ERROR_SUCCESS != status)
            {
                fwprintf(stdout, L"TPGetProperty failed with %lu\n", status);
                break;
            }

            // use the propertyname and the InType to fillin the data
            USHORT inType = pInfo->EventPropertyInfoArray[i].nonStructType.InType;

            switch (i)
            {
                case 0:
                    threadRec.m_processId = GetUInt32(inType, pData);
                    break;

                case 1:
                    threadRec.m_threadId = GetUInt32(inType, pData);
                    break;

                case 2:
                    threadRec.m_stackBase = GetPointer(inType, pData);
                    break;

                case 3:
                    threadRec.m_stackLimit = GetPointer(inType, pData);
                    break;

                case 4:
                    threadRec.m_userStackBase = GetPointer(inType, pData);
                    break;

                case 5:
                    threadRec.m_userStackLimit = GetPointer(inType, pData);
                    break;

                case 6:
                    threadRec.m_affinity = GetPointer(inType, pData);
                    break;

                case 7:
                    threadRec.m_win32StartAddr = GetPointer(inType, pData);
                    break;

                case 8:
                    threadRec.m_tebBase = GetPointer(inType, pData);
                    break;

                case 9:
                    threadRec.m_subProcessTag = GetUInt32(inType, pData);
                    break;

                case 10:
                    threadRec.m_basePriority = GetUInt8(inType, pData);
                    break;

                case 11:
                    threadRec.m_pagePriority = GetUInt8(inType, pData);
                    break;

                case 12:
                    threadRec.m_ioPriority = GetUInt8(inType, pData);
                    break;

                case 13:
                    threadRec.m_threadFlags = GetUInt8(inType, pData);
                    break;
            }
        }
    }

    if (ERROR_SUCCESS != status)
    {
        fwprintf(stdout, L"TPGetPropertyData failed\n");
    }

    //if (NULL != pData)
    //{
    //    free(pData);
    //    pData = NULL;
    //}

    retVal = (ERROR_SUCCESS == status) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
    return retVal;
} // ProcessThreadRecordData


// ProcessCSwitchRecord
//
int ProcessCSwitchRecord(PEVENT_RECORD pEventRecord,
                         PTRACE_EVENT_INFO pTraceEventInfo,
                         ThreadProfileEventCSwitch& csRec)
{
    int status = AMDT_ERROR_FAIL;

    // Print the event data for all the top-level properties. Metadata for all the
    // top-level properties come before structure member properties in the
    // property information array. If the EVENT_HEADER_FLAG_STRING_ONLY flag is set,
    // the event data is a null-terminated string, so just print it.
    if (EVENT_HEADER_FLAG_STRING_ONLY == (pEventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_STRING_ONLY))
    {
        if (g_verbose)
        {
            fprintf(g_debugLogFP, " %ws ", (LPWSTR)pEventRecord->UserData);
        }
    }
    else
    {
        if (g_verbose)
        {
            fprintf(g_debugLogFP, "TopLevelPropertyCount : %d ", pTraceEventInfo->TopLevelPropertyCount);
        }

        // !!! DO NOT PROCESS aLL THE FIELDS !!
        //for (USHORT i = 0; i < pTraceEventInfo->TopLevelPropertyCount; i++)
        //{
        //    status = ProcessCSwitchRecordData(pEventRecord, pTraceEventInfo, i, NULL, 0, csRec);
        //}

        // new tid, old tid, oldThreadWaitReason, oldThreadWaitMode, oldThreadState
        USHORT propertyIdx[] = { 0, 1, 6, 7, 8 };

        for (USHORT i = 0; i < (sizeof(propertyIdx) / sizeof(USHORT)); i++)
        {
            status = ProcessCSwitchRecordData(pEventRecord, pTraceEventInfo, propertyIdx[i], NULL, 0, csRec);
        }
    }

    //fprintf(stderr, "CSwitch record done.. %d\n", status);

    return status;
} // ProcessCSwitchRecord

// ProcessCSwitchRecordData
//
AMDTResult ProcessCSwitchRecordData(PEVENT_RECORD pEvent,
                                    PTRACE_EVENT_INFO pInfo,
                                    USHORT i,
                                    LPWSTR pStructureName,
                                    USHORT StructIndex,
                                    ThreadProfileEventCSwitch& csRec)
{
    AMDTResult retVal = AMDT_ERROR_FAIL;
    DWORD status = ERROR_SUCCESS;
    USHORT ArraySize = 0;

    PROPERTY_DATA_DESCRIPTOR DataDescriptor;
    PROPERTY_DATA_DESCRIPTOR DataDescriptors[2];
    ULONG DescriptorsCount = 0;
    DWORD PropertySize = 0;
    PBYTE pData = NULL;

    // Get the property data
    status = GetTdhPropertyData(pEvent, pInfo, i, &DataDescriptor, &PropertySize, &pData, &ArraySize);

    if (ERROR_SUCCESS == status)
    {
        // For CSWitch ArraySize == 1
        for (USHORT k = 0; k < ArraySize; k++)
        {
            LPWSTR propertyName = (LPWSTR)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);

            if (g_verbose)
            {
                fwprintf(stdout, L" %s ", propertyName);
            }

            ZeroMemory(&DataDescriptors, sizeof(DataDescriptors));

            // To retrieve a member of a structure, you need to specify an array of descriptors.
            // The first descriptor in the array identifies the name of the structure and the second
            // descriptor defines the member of the structure whose data you want to retrieve.

            if (pStructureName)
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)pStructureName;
                DataDescriptors[0].ArrayIndex = StructIndex;
                DataDescriptors[1].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[1].ArrayIndex = k;
                DescriptorsCount = 2;
            }
            else
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[0].ArrayIndex = k;
                DescriptorsCount = 1;
            }

            status = GetTdhProperty(pEvent, DescriptorsCount, &DataDescriptors[0], &PropertySize, &pData);

            if (ERROR_SUCCESS != status)
            {
                fwprintf(stdout, L"TPGetProperty failed with %lu\n", status);
                break;
            }

            // use the propertyname and the InType to fillin the data
            USHORT inType = pInfo->EventPropertyInfoArray[i].nonStructType.InType;

            switch (i)
            {
                case 0:
                    // lstrcmpW(LPCWSTR(propertyName), L"OldThreadId")
                    csRec.m_newThreadId = GetUInt32(inType, pData);
                    break;

                case 1:
                    csRec.m_oldThreadId = GetUInt32(inType, pData);
                    break;

                case 2:
                    csRec.m_newThreadPriority = GetInt8(inType, pData);
                    break;

                case 3:
                    csRec.m_oldThreadPriority = GetInt8(inType, pData);
                    break;

                case 4:
                    csRec.m_previousCState = GetUInt8(inType, pData);
                    break;

                case 5:
                    csRec.m_spareByte = GetInt8(inType, pData);
                    break;

                case 6:
                    csRec.m_oldThreadWaitReason = GetInt8(inType, pData);
                    break;

                case 7:
                    csRec.m_oldThreadWaitMode = GetInt8(inType, pData);
                    break;

                case 8:
                    csRec.m_oldThreadState = GetInt8(inType, pData);
                    break;

                case 9:
                    csRec.m_oldThreadWaitIdealProcessor = GetInt8(inType, pData);
                    break;

                case 10:
                    csRec.m_newThreadWaitTime = GetUInt32(inType, pData);
                    break;

                case 11:
                    csRec.m_reserved = GetUInt32(inType, pData);
                    break;
            }
        }
    }

    if (ERROR_SUCCESS != status)
    {
        fwprintf(stdout, L"TPGetPropertyData failed\n");
    }

    //if (NULL != pData)
    //{
    //    free(pData);
    //    pData = NULL;
    //}

    retVal = (ERROR_SUCCESS == status) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
    return retVal;
} // ProcessCSwitchRecordData


int ProcessStackRecord(PEVENT_RECORD pEventRecord,
                       PTRACE_EVENT_INFO pTraceEventInfo,
                       ThreadProfileEventStack& stackRec)
{
    UNREFERENCED_PARAMETER(stackRec);

    int status = AMDT_ERROR_FAIL;

    // Print the event data for all the top-level properties. Metadata for all the
    // top-level properties come before structure member properties in the
    // property information array. If the EVENT_HEADER_FLAG_STRING_ONLY flag is set,
    // the event data is a null-terminated string, so just print it.
    if (EVENT_HEADER_FLAG_STRING_ONLY == (pEventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_STRING_ONLY))
    {
        if (g_verbose)
        {
            fprintf(g_debugLogFP, " %ws ", (LPWSTR)pEventRecord->UserData);
        }
    }
    else
    {
        if (g_verbose)
        {
            fprintf(g_debugLogFP, "TopLevelPropertyCount : %d ", pTraceEventInfo->TopLevelPropertyCount);
        }

        stackRec.m_nbrFrames = 0;
        status = AMDT_STATUS_OK;

        for (USHORT i = 0; i < pTraceEventInfo->TopLevelPropertyCount && (AMDT_STATUS_OK == status); i++)
        {
            status = ProcessStackRecordData(pEventRecord, pTraceEventInfo, i, NULL, 0, stackRec);
        }
    }

    //fprintf(stderr, "stack record done.. %d\n", status);

    return status;
} // ProcessStackRecord


// ProcessStackRecordData
//
AMDTResult ProcessStackRecordData(PEVENT_RECORD pEvent,
                                  PTRACE_EVENT_INFO pInfo,
                                  USHORT i,
                                  LPWSTR pStructureName,
                                  USHORT StructIndex,
                                  ThreadProfileEventStack& stackRec)
{
    AMDTResult retVal = AMDT_ERROR_FAIL;
    DWORD status = ERROR_SUCCESS;
    USHORT ArraySize = 0;

    PROPERTY_DATA_DESCRIPTOR DataDescriptor;
    PROPERTY_DATA_DESCRIPTOR DataDescriptors[2];
    ULONG DescriptorsCount = 0;
    DWORD PropertySize = 0;
    PBYTE pData = NULL;

    // Get the property data
    status = GetTdhPropertyData(pEvent, pInfo, i, &DataDescriptor, &PropertySize, &pData, &ArraySize);

    if (ERROR_SUCCESS == status)
    {
        // For CSWitch ArraySize == 1
        for (USHORT k = 0; k < ArraySize; k++)
        {
            LPWSTR propertyName = (LPWSTR)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);

            if (g_verbose)
            {
                fwprintf(stdout, L" %s ", propertyName);
            }

            ZeroMemory(&DataDescriptors, sizeof(DataDescriptors));

            // To retrieve a member of a structure, you need to specify an array of descriptors.
            // The first descriptor in the array identifies the name of the structure and the second
            // descriptor defines the member of the structure whose data you want to retrieve.

            if (pStructureName)
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)pStructureName;
                DataDescriptors[0].ArrayIndex = StructIndex;
                DataDescriptors[1].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[1].ArrayIndex = k;
                DescriptorsCount = 2;
            }
            else
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[0].ArrayIndex = k;
                DescriptorsCount = 1;
            }

            status = GetTdhProperty(pEvent, DescriptorsCount, &DataDescriptors[0], &PropertySize, &pData);

            if (ERROR_SUCCESS != status)
            {
                //fwprintf(stdout, L"TPGetProperty failed with %lu\n", status);
                break;
            }

            // use the propertyname and the InType to fillin the data
            USHORT inType = pInfo->EventPropertyInfoArray[i].nonStructType.InType;

            switch (i)
            {
                case 0:
                    stackRec.m_eventTimeStamp = GetUInt64(inType, pData);
                    break;

                case 1:
                    stackRec.m_stackProcess = GetUInt32(inType, pData);
                    break;

                case 2:
                    stackRec.m_stackThread = GetUInt32(inType, pData);
                    break;

                default:
                    stackRec.m_nbrFrames++;
                    stackRec.m_stacks[i - 3] = GetPointer(inType, pData);
                    break;
            }
        }
    }

    //if (ERROR_SUCCESS != status)
    //{
    //    fwprintf(stdout, L"TPGetPropertyData failed\n");
    //}

    //if (NULL != pData)
    //{
    //    free(pData);
    //    pData = NULL;
    //}

    retVal = (ERROR_SUCCESS == status) ? AMDT_STATUS_OK : AMDT_ERROR_FAIL;
    return retVal;
} // ProcessStackRecordData


//
// Print routines
//


//
// Warpper on top of TDH interface
//
DWORD GetTdhProperty(PEVENT_RECORD pEvent,
                     DWORD descriptorCount,
                     PPROPERTY_DATA_DESCRIPTOR pDataDescriptor,
                     PDWORD pPropertySize,
                     PBYTE*  ppData)
{
    DWORD status = ERROR_SUCCESS;

    // FIXME..
    if (NULL == pDataDescriptor)
    {
        return ERROR_INVALID_PARAMETER;
    }

    // DWORD Count = 0;  // Expects the count to be defined by a UINT16 or UINT32

    status = TdhGetPropertySize(pEvent, 0, NULL, descriptorCount, pDataDescriptor, pPropertySize);

    if (ERROR_SUCCESS == status)
    {
        if (NULL != ppData)
        {
            // *ppData = (PBYTE)malloc(*pPropertySize);
            if ((*pPropertySize) > TP_EVENT_PROPERTY_BUFFER_MAXSIZE)
            {
                g_pPropertyBuffer = (PBYTE)realloc(g_pPropertyBuffer, *pPropertySize);
                // TODO: preserve the latest size and use it for future comparision
            }

            *ppData = g_pPropertyBuffer;
        }

        status = TdhGetProperty(pEvent, 0, NULL, descriptorCount, pDataDescriptor, *pPropertySize, *ppData);
        // TODO: check return code

        //*pArraySize = (USHORT)Count;
    }

    return status;
} // GetTdhProperty

// Get the size of the array. For MOF-based events, the size is specified in the declaration or using
// the MAX qualifier. For manifest-based events, the property can specify the size of the array
// using the count attribute. The count attribue can specify the size directly or specify the name
// of another property in the event data that contains the size.
DWORD GetTdhPropertyData(PEVENT_RECORD pEvent,
                         PTRACE_EVENT_INFO pInfo,
                         USHORT i,
                         PROPERTY_DATA_DESCRIPTOR* pDataDescriptor,
                         PDWORD pPropertySize,
                         PBYTE* ppData,
                         PUSHORT pArraySize)
{
    DWORD status = ERROR_SUCCESS;

    if (NULL == pDataDescriptor)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ((pInfo->EventPropertyInfoArray[i].Flags & PropertyParamCount) == PropertyParamCount)
    {
        //DWORD Count = 0;  // Expects the count to be defined by a UINT16 or UINT32
        DWORD j = pInfo->EventPropertyInfoArray[i].countPropertyIndex;

        ZeroMemory(pDataDescriptor, sizeof(PROPERTY_DATA_DESCRIPTOR));

        pDataDescriptor->PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[j].NameOffset);
        pDataDescriptor->ArrayIndex = ULONG_MAX;

        //status = TdhGetPropertySize(pEvent, 0, NULL, 1, pDataDescriptor, &PropertySize);
        //// TODO: check return code
        //status = TdhGetProperty(pEvent, 0, NULL, 1, pDataDescriptor, PropertySize, (PBYTE)&Count);
        //// TODO: check return code

        PBYTE pData;
        status = GetTdhProperty(pEvent, 1, pDataDescriptor, pPropertySize, &pData);
        // TODO: check return code

        if (ERROR_SUCCESS == status)
        {
            if (NULL != ppData)
            {
                *ppData = pData;
            }

            *pArraySize = *((USHORT*)pData);
        }
    }
    else
    {
        *pArraySize = pInfo->EventPropertyInfoArray[i].count;
    }

    return status;
}

// Both MOF-based events and manifest-based events can specify name/value maps. The
// map values can be integer values or bit values. If the property specifies a value
// map, get the map.
DWORD GetTdhMapInfo(PEVENT_RECORD pEvent, LPWSTR pMapName, DWORD DecodingSource, PEVENT_MAP_INFO& pMapInfo)
{
    DWORD status = ERROR_SUCCESS;
    DWORD MapSize = 0;

    // Retrieve the required buffer size for the map info.

    status = TdhGetEventMapInformation(pEvent, pMapName, pMapInfo, &MapSize);

    if (ERROR_INSUFFICIENT_BUFFER == status)
    {
        pMapInfo = (PEVENT_MAP_INFO)malloc(MapSize);

        if (pMapInfo == NULL)
        {
            wprintf(L"Failed to allocate memory for map info (size=%lu).\n", MapSize);
            status = ERROR_OUTOFMEMORY;
            goto cleanup;
        }

        // Retrieve the map info.
        status = TdhGetEventMapInformation(pEvent, pMapName, pMapInfo, &MapSize);
    }

    if (ERROR_SUCCESS == status)
    {
        if (DecodingSourceXMLFile == DecodingSource)
        {
            RemoveTrailingSpace(pMapInfo);
        }
    }
    else
    {
        if (ERROR_NOT_FOUND == status)
        {
            status = ERROR_SUCCESS; // This case is okay.
        }
        else
        {
            wprintf(L"TdhGetEventMapInformation failed with 0x%x.\n", status);
        }
    }

cleanup:

    return status;
}


DWORD GetUnicodeString(USHORT InType, PBYTE pData, size_t& StringLength, LPWSTR& pString)
{
    DWORD status = ERROR_NOT_FOUND;

    if (TDH_INTYPE_UNICODESTRING == InType)
    {
        StringLength = wcslen((LPWSTR)pData);

        if (g_verbose)
        {
            wprintf(L"%.*s ", StringLength, (LPWSTR)pData);
        }

        pString = (LPWSTR)pData;

        status = ERROR_SUCCESS;
    }

    return status;
}

DWORD GetAnsiString(USHORT InType, PBYTE pData, size_t& StringLength, LPSTR& pString)
{
    DWORD status = ERROR_NOT_FOUND;

    if (TDH_INTYPE_ANSISTRING == InType)
    {
        StringLength = strlen((LPSTR)pData);
        pString = (LPSTR)pData;

        if (g_verbose)
        {
            wprintf(L"%.*S ", StringLength, (LPSTR)pData);
        }

        status = ERROR_SUCCESS;
    }

    return status;
}

INT8 GetInt8(USHORT InType, PBYTE pData)
{
    INT8 data = 0x7F;

    if (TDH_INTYPE_INT8 == InType)
    {
        if (g_verbose)
        {
            wprintf(L"%hd ", *(PCHAR)pData);
        }

        data = *(PCHAR)pData;
    }

    return data;
}

UINT8 GetUInt8(USHORT InType, PBYTE pData)
{
    UINT8 data = 0xFF;

    if (TDH_INTYPE_UINT8 == InType)
    {
        if (g_verbose)
        {
            wprintf(L"%hu ", *(PUCHAR)pData);
        }

        data = *(PUCHAR)pData;
    }

    return data;
}

LONG GetInt32(USHORT InType, PBYTE pData)
{
    LONG data = 0xFFFFFFFF;

    if (TDH_INTYPE_INT32 == InType)
    {
        if (g_verbose)
        {
            wprintf(L"%d ", *(PLONG)pData);
        }

        data = *(PLONG)pData;
    }

    return data;
}

ULONG GetUInt32(USHORT InType, PBYTE pData)
{
    ULONG data = 0xFFFFFFFFUL;

    if (TDH_INTYPE_UINT32 == InType)
    {
        if (g_verbose)
        {
            wprintf(L"%u ", *(PULONG)pData);
        }

        data = *(PULONG)pData;
    }

    return data;
}

ULONGLONG GetUInt64(USHORT InType, PBYTE pData)
{
    ULONGLONG data = 0xFFFFFFFFFFFFFFFFULL;

    if (TDH_INTYPE_UINT64 == InType)
    {
        if (g_verbose)
        {
            wprintf(L"%I64u ", *(PULONGLONG)pData);
        }

        data = *(PULONGLONG)pData;
    }

    return data;
}

ULONGLONG GetPointer(USHORT InType, PBYTE pData)
{
    ULONGLONG ptrData = 0;

    if (TDH_INTYPE_POINTER == InType)
    {
        if (4 == g_pointerSize)
        {
            if (g_verbose)
            {
                wprintf(L"0x%ulx\n", *(PULONG)pData);
            }

            ptrData = *(PULONG)pData;
        }
        else
        {
            if (g_verbose)
            {
                wprintf(L"0x%llu\n", *(PULONGLONG)pData);
            }

            ptrData = *(PULONGLONG)pData;
        }
    }

    return ptrData;
}

ULONG GetPUlong(USHORT InType, PBYTE pData)
{
    ULONG ptrData = 0;

    // FIXME: should i use g_pointersize ??
    // if ((TDH_INTYPE_POINTER == InType) && (4 == g_pointerSize))
    if (TDH_INTYPE_POINTER == InType)
    {
        if (g_verbose)
        {
            wprintf(L"0x%x\n", *(PULONG)pData);
        }

        ptrData = *(PULONG)pData;
    }

    return ptrData;
}

ULONGLONG GetPUlonglong(USHORT InType, PBYTE pData)
{
    ULONGLONG ptrData = 0;

    // FIXME: should i use g_pointerSize
    // if ((TDH_INTYPE_POINTER == InType) && (8 == g_pointerSize))
    if (TDH_INTYPE_POINTER == InType)
    {
        if (g_verbose)
        {
            wprintf(L"0x%llx\n", *(PULONGLONG)pData);
        }

        ptrData = *(PULONGLONG)pData;
    }

    return ptrData;
}


DWORD GetWbemsid(USHORT InType, PBYTE pData, DWORD dataSize)
{
    UNREFERENCED_PARAMETER(dataSize);

    DWORD status = ERROR_SUCCESS;

    if (TDH_INTYPE_WBEMSID == InType)
    {
        WCHAR UserName[TP_MAX_NAME_LEN];
        WCHAR DomainName[TP_MAX_NAME_LEN];
        DWORD cchUserSize = TP_MAX_NAME_LEN;
        DWORD cchDomainSize = TP_MAX_NAME_LEN;
        SID_NAME_USE eNameUse;

        if ((PULONG)pData > 0)
        {
            // A WBEM SID is actually a TOKEN_USER structure followed
            // by the SID. The size of the TOKEN_USER structure differs
            // depending on whether the events were generated on a 32-bit
            // or 64-bit architecture. Also the structure is aligned
            // on an 8-byte boundary, so its size is 8 bytes on a
            // 32-bit computer and 16 bytes on a 64-bit computer.
            // Doubling the pointer size handles both cases.

            pData += g_pointerSize * 2;

            if (!LookupAccountSid(NULL, (PSID)pData, UserName, &cchUserSize, DomainName, &cchDomainSize, &eNameUse))
            {
                if (ERROR_NONE_MAPPED == status)
                {
                    wprintf(L"Unable to locate account for the specified SID\n");
                    status = ERROR_SUCCESS;
                }
                else
                {
                    wprintf(L"LookupAccountSid failed with %lu\n", status = GetLastError());
                }
            }
            else
            {
                if (g_verbose)
                {
                    wprintf(L"%s\\%s\n", DomainName, UserName);
                }
            }
        }
    }

    return status;
}

//
//  Print routines
//

AMDTResult PrintTraceEventRecordMOF(PEVENT_RECORD pEventRecord, PTRACE_EVENT_INFO pTraceEventInfo)
{
    AMDTResult hr = AMDT_STATUS_OK;
    char eventString[64] = { '\0' };
    char eventTypeString[64] = { '\0' };
    char activityIdString[64] = { '\0' };
    ULONGLONG TimeStamp = 0;
    ULONGLONG Nanoseconds = 0;
    SYSTEMTIME st;
    SYSTEMTIME stLocal;
    FILETIME ft;

    //fprintf(stdout, "TimeStamp  Event  EventType  Size  HeaderType  Flags  EventProperty");
    //fprintf(stdout, "ProcessID  ThreadID  ProcessorTime  KernelTime  UserTime  activityId");
    //fprintf(stdout, "ExtendedDataCount  UserDataLength\n");

    if ((NULL != pEventRecord) && (NULL != pTraceEventInfo))
    {
        GetEventString(pEventRecord->EventHeader.ProviderId, eventString);
        GetEventTypeString(pEventRecord->EventHeader.EventDescriptor.Opcode, eventTypeString);

        // Time stamp for when the event occurred.
        ft.dwHighDateTime = pEventRecord->EventHeader.TimeStamp.HighPart;
        ft.dwLowDateTime = pEventRecord->EventHeader.TimeStamp.LowPart;

        FileTimeToSystemTime(&ft, &st);
        SystemTimeToTzSpecificLocalTime(NULL, &st, &stLocal);

        TimeStamp = pEventRecord->EventHeader.TimeStamp.QuadPart;
        Nanoseconds = (TimeStamp % 10000000) * 100;

        //fprintf(stdout, "%02d/%02d/%02d %02d:%02d:%02d.%I64u ",
        //    stLocal.wMonth, stLocal.wDay, stLocal.wYear, stLocal.wHour, stLocal.wMinute, stLocal.wSecond, Nanoseconds);
        fprintf(stdout, "%I64u %02d:%02d:%02d.%I64u ",
                pEventRecord->EventHeader.TimeStamp.QuadPart,
                stLocal.wHour, stLocal.wMinute, stLocal.wSecond, Nanoseconds);

        fprintf(stdout, " %s %s ", eventString, eventTypeString);

        fprintf(stdout, " %u 0x%x 0x%x ", pEventRecord->EventHeader.Size,
                pEventRecord->EventHeader.Flags, pEventRecord->EventHeader.EventProperty);
        fprintf(stdout, " %d %d ", pEventRecord->EventHeader.ProcessId, pEventRecord->EventHeader.ThreadId);
        fprintf(stdout, " %llu %lu %lu ", pEventRecord->EventHeader.ProcessorTime, pEventRecord->EventHeader.KernelTime,
                pEventRecord->EventHeader.UserTime);

        GetEventString(pEventRecord->EventHeader.ActivityId, activityIdString);
        fprintf(stdout, " %s ", activityIdString);

        // Event Descriptor
        fprintf(stdout, " %u %u %u %u %u %u %llu ", pEventRecord->EventHeader.EventDescriptor.Id,
                pEventRecord->EventHeader.EventDescriptor.Version, pEventRecord->EventHeader.EventDescriptor.Channel,
                pEventRecord->EventHeader.EventDescriptor.Level, pEventRecord->EventHeader.EventDescriptor.Opcode,
                pEventRecord->EventHeader.EventDescriptor.Task, pEventRecord->EventHeader.EventDescriptor.Keyword);

        fprintf(stdout, " %d %d", pEventRecord->ExtendedDataCount, pEventRecord->UserDataLength);

        // If the event contains event-specific data use TDH to extract
        // the event data. For this example, to extract the data, the event
        // must be defined by a MOF class or an instrumentation manifest.

        // Need to get the PointerSize for each event to cover the case where you are
        // consuming events from multiple log files that could have been generated on
        // different architectures. Otherwise, you could have accessed the pointer
        // size when you opened the trace above (see pHeader->PointerSize).

        if (EVENT_HEADER_FLAG_32_BIT_HEADER == (pEventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_32_BIT_HEADER))
        {
            g_pointerSize = 4;
        }
        else
        {
            g_pointerSize = 8;
        }

        // Print the event data for all the top-level properties. Metadata for all the
        // top-level properties come before structure member properties in the
        // property information array. If the EVENT_HEADER_FLAG_STRING_ONLY flag is set,
        // the event data is a null-terminated string, so just print it.
        if (EVENT_HEADER_FLAG_STRING_ONLY == (pEventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_STRING_ONLY))
        {
            fwprintf(stdout, L" %s ", (LPWSTR)pEventRecord->UserData);
        }
        else
        {
            fwprintf(stdout, L"TopLevelPropertyCount : %d ", pTraceEventInfo->TopLevelPropertyCount);

            for (USHORT i = 0; i < pTraceEventInfo->TopLevelPropertyCount; i++)
            {
                int status = PrintEventTraceData(pEventRecord, pTraceEventInfo, i, NULL, 0);
                fwprintf(stdout, L"\n");

                if (ERROR_SUCCESS != status)
                {
                    // FIXME
                    // fwprintf(m_debugLogFP, L"Printing top level properties failed.\n");
                }
            }
        }

        fprintf(stdout, "\n");
    }

    return hr;
}

void PrintInitialize()
{
    strcpy(g_eventTypeStr[EVENT_TRACE_TYPE_INFO], "Info");           // 0
    strcpy(g_eventTypeStr[EVENT_TRACE_TYPE_START], "Start");         // 1
    strcpy(g_eventTypeStr[EVENT_TRACE_TYPE_END], "End");             // 2 (end / unload)
    strcpy(g_eventTypeStr[EVENT_TRACE_TYPE_DC_START], "DCStart");    // 3
    strcpy(g_eventTypeStr[EVENT_TRACE_TYPE_DC_END], "DCEnd");        // 4
    strcpy(g_eventTypeStr[EVENT_TRACE_TYPE_EXTENSION], "Extension");        // 5

    strcpy(g_eventTypeStr[10], "Load");
    strcpy(g_eventTypeStr[32], "Stack");
    strcpy(g_eventTypeStr[36], "CSwitch");
    strcpy(g_eventTypeStr[39], "Defunt");
    strcpy(g_eventTypeStr[50], "ReadyThread");


    // print the header string
    if (g_verbose)
    {
        fprintf(g_debugLogFP, "TimeStamp  Event  EventType  Size  Flags  EventProperty ");
        fprintf(g_debugLogFP, "ProcessID  ThreadID  ProcessorTime  KernelTime  UserTime  activityId ");
        fprintf(g_debugLogFP, "EventDescriptor { Id Version Channel Level Opcode Task Keyword }");
        fprintf(g_debugLogFP, "ExtendedDataCount  UserDataLength \n");
    }

    return;
}

AMDTResult GetEventString(GUID guid, char* pEventString)
{
    if ((NULL != pEventString))
    {
        if (IsEqualGUID(guid, EventTraceGuid))
            //&& pEventRecord->EventHeader.EventDescriptor.Opcode == EVENT_TRACE_TYPE_INFO)
        {
            // FIXME: do nothing now
            strcpy(pEventString, "Info");
        }
        else if (IsEqualGUID(guid, g_imageGuid))
        {
            strcpy(pEventString, "Image");
        }
        else if (IsEqualGUID(guid, g_processGuid))
        {
            strcpy(pEventString, "Process");
        }
        else if (IsEqualGUID(guid, g_threadGuid))
        {
            strcpy(pEventString, "Thread");
        }
        else if (IsEqualGUID(guid, g_stackWalkGuid))
        {
            strcpy(pEventString, "Stack");
        }
        else
        {
            strcpy(pEventString, "Unknown");
        }
    }

    return AMDT_STATUS_OK;
}

AMDTResult GetEventTypeString(int eventType, char* pEventString)
{
    AMDTResult ret = AMDT_ERROR_FAIL;

    if (NULL != pEventString)
    {
        if (eventType <= TP_ETW_EVENT_TYPE_MAX)
        {
            strcpy(pEventString, g_eventTypeStr[eventType]);
            ret = AMDT_STATUS_OK;
        }
    }

    return ret;
}

// PrintEventTraceData
//
int PrintEventTraceData(PEVENT_RECORD pEvent,
                        PTRACE_EVENT_INFO pInfo,
                        USHORT i,
                        LPWSTR pStructureName,
                        USHORT StructIndex)

{
    DWORD status = ERROR_SUCCESS;
    DWORD LastMember = 0;  // Last member of a structure
    USHORT ArraySize = 0;
    PEVENT_MAP_INFO pMapInfo = NULL;

    PROPERTY_DATA_DESCRIPTOR DataDescriptor;
    PROPERTY_DATA_DESCRIPTOR DataDescriptors[2];
    ULONG DescriptorsCount = 0;
    DWORD PropertySize = 0;
    PBYTE pData = NULL;

    // Get the property data
    status = GetTdhPropertyData(pEvent, pInfo, i, &DataDescriptor, &PropertySize, &pData, &ArraySize);

    fwprintf(stdout, L"ArraySize : %d", ArraySize);

    for (USHORT k = 0; k < ArraySize; k++)
    {
        LPWSTR fieldName = (LPWSTR)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);

        fwprintf(stdout, L"*struct* %s* ", fieldName);
        fwprintf(stdout, L"%*s%s: ", (pStructureName) ? 4 : 0, L"", (LPWSTR)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset));

        // If the property is a structure, print the members of the structure.

        if ((pInfo->EventPropertyInfoArray[i].Flags & PropertyStruct) == PropertyStruct)
        {
            wprintf(L"!!!!--!!!!\n");
            wprintf(L"\n");

            LastMember = pInfo->EventPropertyInfoArray[i].structType.StructStartIndex +
                         pInfo->EventPropertyInfoArray[i].structType.NumOfStructMembers;

            for (USHORT j = pInfo->EventPropertyInfoArray[i].structType.StructStartIndex; j < LastMember; j++)
            {
                status = PrintEventTraceData(pEvent, pInfo, j, (LPWSTR)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset), k);

                if (ERROR_SUCCESS != status)
                {
                    fwprintf(stdout, L"Printing the members of the structure failed.\n");
                    goto error_exit;
                }
            }
        }
        else
        {
            ZeroMemory(&DataDescriptors, sizeof(DataDescriptors));

            // To retrieve a member of a structure, you need to specify an array of descriptors.
            // The first descriptor in the array identifies the name of the structure and the second
            // descriptor defines the member of the structure whose data you want to retrieve.

            if (pStructureName)
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)pStructureName;
                DataDescriptors[0].ArrayIndex = StructIndex;
                DataDescriptors[1].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[1].ArrayIndex = k;
                DescriptorsCount = 2;
            }
            else
            {
                DataDescriptors[0].PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);
                DataDescriptors[0].ArrayIndex = k;
                DescriptorsCount = 1;
            }

            status = GetTdhProperty(pEvent, DescriptorsCount, &DataDescriptors[0], &PropertySize, &pData);

            if (ERROR_SUCCESS != status)
            {
                fwprintf(stdout, L"TPGetProperty failed with %lu\n", status);
                goto error_exit;
            }

            // TODO: get name/value mapping, if the property specifies a value map
            status = GetTdhMapInfo(pEvent,
                                   (PWCHAR)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].nonStructType.MapNameOffset),
                                   pInfo->DecodingSource,
                                   pMapInfo);

            if (ERROR_SUCCESS != status)
            {
                fwprintf(stdout, L"TPGetMapInfo failed\n");
                goto error_exit;
            }

            status = FormatAndPrintData(pEvent,
                                        pInfo->EventPropertyInfoArray[i].nonStructType.InType,
                                        pInfo->EventPropertyInfoArray[i].nonStructType.OutType,
                                        pData,
                                        PropertySize,
                                        pMapInfo);

            if (ERROR_SUCCESS != status)
            {
                fwprintf(stdout, L"FormatAndPrintData failed\n");
                goto error_exit;
            }

            //if (NULL != pData)
            //{
            //    free(pData);
            //    pData = NULL;
            //}

            if (pMapInfo)
            {
                free(pMapInfo);
                pMapInfo = NULL;
            }
        }
    }

error_exit:
    //if (NULL != pData)
    //{
    //    free(pData);
    //    pData = NULL;
    //}

    if (pMapInfo)
    {
        free(pMapInfo);
        pMapInfo = NULL;
    }

    return status;
} // PrintEventTraceData


DWORD FormatAndPrintData(PEVENT_RECORD pEvent,
                         USHORT InType,
                         USHORT OutType,
                         PBYTE pData,
                         DWORD DataSize,
                         PEVENT_MAP_INFO pMapInfo)
{
    UNREFERENCED_PARAMETER(pEvent);

    DWORD status = ERROR_SUCCESS;

    wprintf(L"InType (%d) ", InType);

    switch (InType)
    {
        case TDH_INTYPE_UNICODESTRING:
        case TDH_INTYPE_COUNTEDSTRING:
        case TDH_INTYPE_REVERSEDCOUNTEDSTRING:
        case TDH_INTYPE_NONNULLTERMINATEDSTRING:
        {
            size_t StringLength = 0;

            if (TDH_INTYPE_COUNTEDSTRING == InType)
            {
                StringLength = *(PUSHORT)pData;
            }
            else if (TDH_INTYPE_REVERSEDCOUNTEDSTRING == InType)
            {
                StringLength = MAKEWORD(HIBYTE((PUSHORT)pData), LOBYTE((PUSHORT)pData));
            }
            else if (TDH_INTYPE_NONNULLTERMINATEDSTRING == InType)
            {
                StringLength = DataSize;
            }
            else
            {
                StringLength = wcslen((LPWSTR)pData);
            }

            // wprintf(L"%.*s\n", StringLength, (LPWSTR)pData);
            wprintf(L"%.*s ", StringLength, (LPWSTR)pData);
            break;
        }

        case TDH_INTYPE_ANSISTRING:
        case TDH_INTYPE_COUNTEDANSISTRING:
        case TDH_INTYPE_REVERSEDCOUNTEDANSISTRING:
        case TDH_INTYPE_NONNULLTERMINATEDANSISTRING:
        {
            size_t StringLength = 0;

            if (TDH_INTYPE_COUNTEDANSISTRING == InType)
            {
                StringLength = *(PUSHORT)pData;
            }
            else if (TDH_INTYPE_REVERSEDCOUNTEDANSISTRING == InType)
            {
                StringLength = MAKEWORD(HIBYTE((PUSHORT)pData), LOBYTE((PUSHORT)pData));
            }
            else if (TDH_INTYPE_NONNULLTERMINATEDANSISTRING == InType)
            {
                StringLength = DataSize;
            }
            else
            {
                StringLength = strlen((LPSTR)pData);
            }

            // wprintf(L"%.*S\n", StringLength, (LPSTR)pData);
            wprintf(L"%.*S ", StringLength, (LPSTR)pData);
            break;
        }

        case TDH_INTYPE_INT8:
        {
            // wprintf(L"%hd\n", *(PCHAR)pData);
            wprintf(L"%hd ", *(PCHAR)pData);
            break;
        }

        case TDH_INTYPE_UINT8:
        {
            if (TDH_OUTTYPE_HEXINT8 == OutType)
            {
                // wprintf(L"0x%x\n", *(PBYTE)pData);
                wprintf(L"0x%x ", *(PBYTE)pData);
            }
            else
            {
                // wprintf(L"%hu\n", *(PBYTE)pData);
                wprintf(L"%hu ", *(PBYTE)pData);
            }

            break;
        }

        case TDH_INTYPE_INT16:
        {
            // wprintf(L"%hd\n", *(PSHORT)pData);
            wprintf(L"%hd ", *(PSHORT)pData);
            break;
        }

        case TDH_INTYPE_UINT16:
        {
            if (TDH_OUTTYPE_HEXINT16 == OutType)
            {
                // wprintf(L"0x%x\n", *(PUSHORT)pData);
                wprintf(L"0x%x ", *(PUSHORT)pData);
            }
            else if (TDH_OUTTYPE_PORT == OutType)
            {
                // wprintf(L"%hu\n", ntohs(*(PUSHORT)pData));
                //wprintf(L"%hu ", ntohs(*(PUSHORT)pData));
            }
            else
            {
                // wprintf(L"%hu\n", *(PUSHORT)pData);
                wprintf(L"%hu ", *(PUSHORT)pData);
            }

            break;
        }

        case TDH_INTYPE_INT32:
        {
            if (TDH_OUTTYPE_HRESULT == OutType)
            {
                // wprintf(L"0x%x\n", *(PLONG)pData);
                wprintf(L"0x%x ", *(PLONG)pData);
            }
            else
            {
                // wprintf(L"%d\n", *(PLONG)pData);
                wprintf(L"%d ", *(PLONG)pData);
            }

            break;
        }

        case TDH_INTYPE_UINT32:
        {
            if (TDH_OUTTYPE_HRESULT == OutType ||
                TDH_OUTTYPE_WIN32ERROR == OutType ||
                TDH_OUTTYPE_NTSTATUS == OutType ||
                TDH_OUTTYPE_HEXINT32 == OutType)
            {
                // wprintf(L"0x%x\n", *(PULONG)pData);
                wprintf(L"0x%x ", *(PULONG)pData);
            }
            else if (TDH_OUTTYPE_IPV4 == OutType)
            {
                wprintf(L" IPV4 ");
                // wprintf(L"%d.%d.%d.%d\n", (*(PLONG)pData >> 0) & 0xff,
                wprintf(L"%d.%d.%d.%d ", (*(PLONG)pData >> 0) & 0xff,
                        (*(PLONG)pData >> 8) & 0xff,
                        (*(PLONG)pData >> 16) & 0xff,
                        (*(PLONG)pData >> 24) & 0xff);
            }
            else
            {
                if (pMapInfo)
                {
                    wprintf(L" **!!** ");
                    PrintMapString(pMapInfo, pData);
                }
                else
                {
                    // wprintf(L"%lu\n", *(PULONG)pData);
                    wprintf(L"%lu ", *(PULONG)pData);
                }
            }

            break;
        }

        case TDH_INTYPE_INT64:
        {
            // wprintf(L"%I64d\n", *(PLONGLONG)pData);
            wprintf(L"%I64d ", *(PLONGLONG)pData);

            break;
        }

        case TDH_INTYPE_UINT64:
        {
            if (TDH_OUTTYPE_HEXINT64 == OutType)
            {
                // wprintf(L"0x%x\n", *(PULONGLONG)pData);
                wprintf(L"0x%llx ", *(PULONGLONG)pData);
            }
            else
            {
                // wprintf(L"%I64u\n", *(PULONGLONG)pData);
                wprintf(L"%I64u ", *(PULONGLONG)pData);
            }

            break;
        }

        case TDH_INTYPE_FLOAT:
        {
            // wprintf(L"%f\n", *(PFLOAT)pData);
            wprintf(L"%f ", *(PFLOAT)pData);

            break;
        }

        case TDH_INTYPE_DOUBLE:
        {

            wprintf(L"%f ", *(double*)pData);

            break;
        }

        case TDH_INTYPE_BOOLEAN:
        {
            // wprintf(L"%s\n", (0 == (PBOOL)pData) ? L"false" : L"true");
            wprintf(L"%s ", (0 == (PBOOL)pData) ? L"false" : L"true");

            break;
        }

        case TDH_INTYPE_BINARY:
        {
            if (TDH_OUTTYPE_IPV6 == OutType)
            {
                fprintf(g_debugLogFP, "IPv6Address !!\n");
            }
            else
            {
                for (DWORD i = 0; i < DataSize; i++)
                {
                    fprintf(g_debugLogFP, "%.2x", pData[i]);
                }

                // wprintf(L"\n");
                fprintf(g_debugLogFP, " ");
            }

            break;
        }

        case TDH_INTYPE_GUID:
        {
            WCHAR szGuid[50];

            StringFromGUID2(*(GUID*)pData, szGuid, sizeof(szGuid) - 1);
            // wprintf(L"%s\n", szGuid);
            wprintf(L"%s ", szGuid);

            break;
        }

        case TDH_INTYPE_POINTER:
        case TDH_INTYPE_SIZET:
        {
            if (4 == g_pointerSize)
            {
                // wprintf(L"0x%x\n", *(PULONG)pData);
                wprintf(L"0x%x ", *(PULONG)pData);
            }
            else
            {
                // wprintf(L"0x%x\n", *(PULONGLONG)pData);
                wprintf(L"0x%llx ", *(PULONGLONG)pData);
            }

            break;
        }

        case TDH_INTYPE_FILETIME:
        {
            break;
        }

        case TDH_INTYPE_SYSTEMTIME:
        {
            break;
        }

        case TDH_INTYPE_SID:
        {
            WCHAR UserName[TP_MAX_NAME_LEN];
            WCHAR DomainName[TP_MAX_NAME_LEN];
            DWORD cchUserSize = TP_MAX_NAME_LEN;
            DWORD cchDomainSize = TP_MAX_NAME_LEN;
            SID_NAME_USE eNameUse;

            if (!LookupAccountSid(NULL, (PSID)pData, UserName, &cchUserSize, DomainName, &cchDomainSize, &eNameUse))
            {
                if (ERROR_NONE_MAPPED == status)
                {
                    wprintf(L"Unable to locate account for the specified SID\n");
                    status = ERROR_SUCCESS;
                }
                else
                {
                    wprintf(L"LookupAccountSid failed with %lu\n", status = GetLastError());
                }

                goto cleanup;
            }
            else
            {
                wprintf(L"%s\\%s\n", DomainName, UserName);
            }

            break;
        }

        case TDH_INTYPE_HEXINT32:
        {
            // wprintf(L"0x%x\n", (PULONG)pData);
            wprintf(L"0x%p ", (PULONG)pData);
            break;
        }

        case TDH_INTYPE_HEXINT64:
        {
            // wprintf(L"0x%x\n", (PULONGLONG)pData);
            wprintf(L"0x%llx ", (ULONGLONG)pData);
            break;
        }

        case TDH_INTYPE_UNICODECHAR:
        {
            // wprintf(L"%c\n", *(PWCHAR)pData);
            wprintf(L"%c ", *(PWCHAR)pData);
            break;
        }

        case TDH_INTYPE_ANSICHAR:
        {
            // wprintf(L"%C\n", *(PCHAR)pData);
            wprintf(L"%C ", *(PCHAR)pData);
            break;
        }

        case TDH_INTYPE_WBEMSID:
        {
            WCHAR UserName[TP_MAX_NAME_LEN];
            WCHAR DomainName[TP_MAX_NAME_LEN];
            DWORD cchUserSize = TP_MAX_NAME_LEN;
            DWORD cchDomainSize = TP_MAX_NAME_LEN;
            SID_NAME_USE eNameUse;

            if ((PULONG)pData > 0)
            {
                // A WBEM SID is actually a TOKEN_USER structure followed
                // by the SID. The size of the TOKEN_USER structure differs
                // depending on whether the events were generated on a 32-bit
                // or 64-bit architecture. Also the structure is aligned
                // on an 8-byte boundary, so its size is 8 bytes on a
                // 32-bit computer and 16 bytes on a 64-bit computer.
                // Doubling the pointer size handles both cases.

                pData += g_pointerSize * 2;

                if (!LookupAccountSid(NULL, (PSID)pData, UserName, &cchUserSize, DomainName, &cchDomainSize, &eNameUse))
                {
                    if (ERROR_NONE_MAPPED == status)
                    {
                        wprintf(L"Unable to locate account for the specified SID\n");
                        status = ERROR_SUCCESS;
                    }
                    else
                    {
                        wprintf(L"LookupAccountSid failed with %lu\n", status = GetLastError());
                    }

                    goto cleanup;
                }
                else
                {
                    wprintf(L"%s\\%s\n", DomainName, UserName);
                }
            }

            break;
        }

        default:
            status = ERROR_NOT_FOUND;
    }

cleanup:

    return status;
}

// The mapped string values defined in a manifest will contain a trailing space
// in the EVENT_MAP_ENTRY structure. Replace the trailing space with a null-
// terminating character, so that the bit mapped strings are correctly formatted.

void RemoveTrailingSpace(PEVENT_MAP_INFO pMapInfo)
{
    SIZE_T ByteLength = 0;

    for (DWORD i = 0; i < pMapInfo->EntryCount; i++)
    {
        ByteLength = (wcslen((LPWSTR)((PBYTE)pMapInfo + pMapInfo->MapEntryArray[i].OutputOffset)) - 1) * 2;
        *((LPWSTR)((PBYTE)pMapInfo + (pMapInfo->MapEntryArray[i].OutputOffset + ByteLength))) = L'\0';
    }
}

void PrintMapString(PEVENT_MAP_INFO pMapInfo, PBYTE pData)
{
    BOOL MatchFound = FALSE;

    if ((pMapInfo->Flag & EVENTMAP_INFO_FLAG_MANIFEST_VALUEMAP) == EVENTMAP_INFO_FLAG_MANIFEST_VALUEMAP ||
        ((pMapInfo->Flag & EVENTMAP_INFO_FLAG_WBEM_VALUEMAP) == EVENTMAP_INFO_FLAG_WBEM_VALUEMAP &&
         (pMapInfo->Flag & (~EVENTMAP_INFO_FLAG_WBEM_VALUEMAP)) != EVENTMAP_INFO_FLAG_WBEM_FLAG))
    {
        if ((pMapInfo->Flag & EVENTMAP_INFO_FLAG_WBEM_NO_MAP) == EVENTMAP_INFO_FLAG_WBEM_NO_MAP)
        {
            wprintf(L"%s\n", (LPWSTR)((PBYTE)pMapInfo + pMapInfo->MapEntryArray[*(PULONG)pData].OutputOffset));
        }
        else
        {
            for (DWORD i = 0; i < pMapInfo->EntryCount; i++)
            {
                if (pMapInfo->MapEntryArray[i].Value == *(PULONG)pData)
                {
                    // wprintf(L"%s\n", (LPWSTR)((PBYTE)pMapInfo + pMapInfo->MapEntryArray[i].OutputOffset));
                    wprintf(L"%s ", (LPWSTR)((PBYTE)pMapInfo + pMapInfo->MapEntryArray[i].OutputOffset));
                    MatchFound = TRUE;
                    break;
                }
            }

            if (FALSE == MatchFound)
            {
                // wprintf(L"%lu\n", *(PULONG)pData);
                wprintf(L"%lu ", *(PULONG)pData);
            }
        }
    }
    else if ((pMapInfo->Flag & EVENTMAP_INFO_FLAG_MANIFEST_BITMAP) == EVENTMAP_INFO_FLAG_MANIFEST_BITMAP ||
             (pMapInfo->Flag & EVENTMAP_INFO_FLAG_WBEM_BITMAP) == EVENTMAP_INFO_FLAG_WBEM_BITMAP ||
             ((pMapInfo->Flag & EVENTMAP_INFO_FLAG_WBEM_VALUEMAP) == EVENTMAP_INFO_FLAG_WBEM_VALUEMAP &&
              (pMapInfo->Flag & (~EVENTMAP_INFO_FLAG_WBEM_VALUEMAP)) == EVENTMAP_INFO_FLAG_WBEM_FLAG))
    {
        if ((pMapInfo->Flag & EVENTMAP_INFO_FLAG_WBEM_NO_MAP) == EVENTMAP_INFO_FLAG_WBEM_NO_MAP)
        {
            DWORD BitPosition = 0;

            for (DWORD i = 0; i < pMapInfo->EntryCount; i++)
            {
                if ((*(PULONG)pData & (BitPosition = (1 << i))) == BitPosition)
                {
                    wprintf(L"%s%s",
                            (MatchFound) ? L" | " : L"",
                            (LPWSTR)((PBYTE)pMapInfo + pMapInfo->MapEntryArray[i].OutputOffset));

                    MatchFound = TRUE;
                }
            }

        }
        else
        {
            for (DWORD i = 0; i < pMapInfo->EntryCount; i++)
            {
                if ((pMapInfo->MapEntryArray[i].Value & *(PULONG)pData) == pMapInfo->MapEntryArray[i].Value)
                {
                    wprintf(L"%s%s",
                            (MatchFound) ? L" | " : L"",
                            (LPWSTR)((PBYTE)pMapInfo + pMapInfo->MapEntryArray[i].OutputOffset));

                    MatchFound = TRUE;
                }
            }
        }

        if (MatchFound)
        {
            // Baskar
            // wprintf(L"\n");
        }
        else
        {
            // Baskar
            // wprintf(L"%lu\n", *(PULONG)pData);
            wprintf(L"%lu ", *(PULONG)pData);
        }
    }
}