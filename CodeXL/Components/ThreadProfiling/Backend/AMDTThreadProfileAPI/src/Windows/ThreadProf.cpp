#define INITGUID  // Include this #define to use SystemTraceControlGuid/EventTraceGuid  in Evntrace.h.

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <strsafe.h>
#include <wmistr.h>
#include <evntrace.h>
#include <evntcons.h>
#include <tdh.h>
#include <wchar.h>
#include <in6addr.h> // FIXME not reqd
#include <string>
#include <list>

#pragma comment(lib, "tdh.lib")
#pragma comment(lib, "ws2_32.lib")  // For ntohs function // FIXME once in6 stuff, this is not reqd

#define MAX_NAME    256

//  Macros
// Flags
#define CXL_TP_CSWITCH        1         // EVENT_TRACE_FLAG_CSWITCH
#define CXL_TP_IMAGE_LOAD     (1 << 1)  // EVENT_TRACE_FLAG_IMAGE_LOAD
#define CXL_TP_PROCESS        (1 << 2)  // EVENT_TRACE_FLAG_PROCESS
#define CXL_TP_PROFILE        (1 << 3)  // EVENT_TRACE_FLAG_PROFILE (perfinfo / sampledprofile)
#define CXL_TP_SYSTEMCALL     (1 << 4)  // EVENT_TRACE_FLAG_SYSTEMCALL
#define CXL_TP_THREAD         (1 << 5)  // EVENT_TRACE_FLAG_THREAD
#define CXL_TP_MEM_PFAULT     (1 << 6)  // EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS
#define CXL_TP_MEM_HFAULT     (1 << 7)  // EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS

// globals
// GUIDs for NT kernel logger events https://msdn.microsoft.com/en-us/library/windows/desktop/aa364085(v=vs.85).aspx
// https://famellee.wordpress.com/2012/08/17/a-fight-with-etw-and-nt-kernel-logger/
// Type - https://msdn.microsoft.com/en-us/library/windows/desktop/dd765164(v=vs.85).aspx
//  Trace Info - https://msdn.microsoft.com/en-us/library/windows/desktop/dd392329(v=vs.85).aspx
// StackWalk_Event class  https://msdn.microsoft.com/en-us/library/windows/desktop/dd392323(v=vs.85).aspx
//
// Note: All the kernel events use MOF to publish the format of the event data.
// MOF approach to consume trace data -- https://msdn.microsoft.com/en-us/library/windows/desktop/aa364114(v=vs.85).aspx
// Trace Data Helper(TDH) approach - https://msdn.microsoft.com/en-us/library/windows/desktop/ee441328(v=vs.85).aspx
//              https://msdn.microsoft.com/en-us/library/windows/desktop/aa364115(v=vs.85).aspx
//

static const GUID ImageGuid     = { 0x2cb15d1d, 0x5fc1, 0x11d2, { 0xab, 0xe1, 0x00, 0xa0, 0xc9, 0x11, 0xf5, 0x18 } };
static const GUID ProcessGuid   = { 0x3d6fa8d0, 0xfe05, 0x11d0, { 0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c } };
static const GUID ThreadGuid    = { 0x3d6fa8d1, 0xfe05, 0x11d0, { 0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c } };
static const GUID StackWalkGuid = { 0xdef2fe46, 0x7bd6, 0x4b80, { 0xbd, 0x94, 0xf5, 0x7f, 0xe2, 0x0d, 0x0c, 0xe3 } };
static const GUID PerfInfoGuid  = { 0xce1dbfb4, 0x137e, 0x4da6, { 0x87, 0xb0, 0x3f, 0x59, 0xaa, 0x10, 0x2c, 0xbc } };

DWORD TPGetProperty(PEVENT_RECORD pEvent,
                    DWORD descriptorCount,
                    PPROPERTY_DATA_DESCRIPTOR pDataDescriptor,
                    PDWORD pPropertySize,
                    PBYTE*  ppData);

DWORD TPGetPropertyData(PEVENT_RECORD pEvent,
                        PTRACE_EVENT_INFO pInfo,
                        USHORT i,
                        PROPERTY_DATA_DESCRIPTOR* pDataDescriptor,
                        PDWORD PropertySize,
                        PBYTE* ppData,
                        PUSHORT pArraySize);

int TPPrintProperties(PEVENT_RECORD pEvent,
                      PTRACE_EVENT_INFO pInfo,
                      USHORT i,
                      LPWSTR pStructureName,
                      USHORT StructIndex);

DWORD TPGetMapInfo(PEVENT_RECORD pEvent,
                   LPWSTR pMapName,
                   DWORD DecodingSource,
                   PEVENT_MAP_INFO& pMapInfo);

DWORD TPFormatAndPrintData(PEVENT_RECORD pEvent,
                           USHORT InType,
                           USHORT OutType,
                           PBYTE pData,
                           DWORD DataSize,
                           PEVENT_MAP_INFO pMapInfo);

void TPPrintMapString(PEVENT_MAP_INFO pMapInfo, PBYTE pData);

void TPRemoveTrailingSpace(PEVENT_MAP_INFO pMapInfo);

// forward declration
struct ThreadProfileEventCSwitch;

int TPProcessCSwitchRecord(PEVENT_RECORD pEventRecord,
                           PTRACE_EVENT_INFO pTraceEventInfo,
                           ThreadProfileEventCSwitch& csRec);

int TPProcessCSwitchRecord__(PEVENT_RECORD pEvent,
                             PTRACE_EVENT_INFO pInfo,
                             USHORT i,
                             LPWSTR pStructureName,
                             USHORT StructIndex,
                             ThreadProfileEventCSwitch& csRec);

// Event types
enum ThreadProfileEvent
{
    TP_EVENT_UNKNOWN  = -1,
    TP_EVENT_INFO = 0,
    TP_EVENT_PROCESS = 1,
    TP_EVENT_IMAGE = 2,
    TP_EVENT_THREAD = 3,
    TP_EVENT_CSWITCH = 4,
    TP_EVENT_STACK = 5
};

enum ThreadProfileEventType
{
    TP_EVENT_TYPE_UNKNOWN = -1,
    TP_EVENT_TYPE_INFO,
    TP_EVENT_TYPE_PROCESS_START,
    TP_EVENT_TYPE_PROCESS_STOP,
    TP_EVENT_TYPE_PROCESS_DEFUNCT,
    TP_EVENT_TYPE_IMAGE_LOAD,
    TP_EVENT_TYPE_IMAGE_UNLOAD,
    TP_EVENT_TYPE_THREAD_START,
    TP_EVENT_TYPE_THREAD_STOP,
    TP_EVENT_TYPE_THREAD_READY,
    TP_EVENT_TYPE_CSWITCH,
    TP_EVENT_TYPE_STACK,
};

struct TPThreadMetaData
{
    UINT32      m_threadId;
};

struct ThreadProfileEventInfo
{
    ThreadProfileEvent      m_event;
    ThreadProfileEventType  m_eventType;

    UCHAR                   m_version;
    UCHAR                   m_opcode;

    LARGE_INTEGER           m_timeStamp;
    UINT32                  m_flags;
    UINT32                  m_processorId;
    UINT32                  m_processId;
    UINT32                  m_threadId;
    UINT64                  m_procTime;
    UINT32                  m_kernelTime;
    UINT32                  m_userTime;
};

// Process_TypeGroup1
struct ThreadProfileEventProcess
{
    ThreadProfileEventInfo  m_eventInfo;

    UINT32 UniqueProcessKey;
    UINT32 ProcessId;
    UINT32 ParentId;
    UINT32 SessionId;
    INT32  ExitStatus;
    UINT32 DirectoryTableBase;
    void*  UserSID;             // FIXME
    std::string ImageFileName;
    std::string CommandLine;
};

struct ThreadProfileEventImage
{
    ThreadProfileEventInfo  m_eventInfo;
};

struct ThreadProfileEventThread
{
    ThreadProfileEventInfo  m_eventInfo;
};

struct ThreadProfileEventCSwitch
{
    ThreadProfileEventInfo  m_eventInfo;

    UINT32 m_newThreadId;
    UINT32 m_oldThreadId;
    INT8 m_newThreadPriority;
    INT8 m_oldThreadPriority;
    UINT8 m_previousCState;
    INT8 m_spareByte;
    INT8 m_oldThreadWaitReason;
    INT8 m_oldThreadWaitMode;
    INT8 m_oldThreadState;
    INT8 m_oldThreadWaitIdealProcessor;
    UINT32 m_newThreadWaitTime;
    UINT32 m_reserved;
};


std::list<ThreadProfileEventProcess> g_eventProcessList;
std::list<ThreadProfileEventImage> g_eventImageList;
std::list<ThreadProfileEventThread> g_eventThreadList;


// Globals
static TRACEHANDLE g_SessionHandle = 0;
static EVENT_TRACE_PROPERTIES* g_pSessionProperties = NULL;
const wchar_t* g_pLogFilePath = L"c:\\temp\\test2.etl";

bool g_UserMode;
unsigned long g_TimerResolution = 0;
unsigned int g_PointerSize = 0;

unsigned int g_EventFlags[] =
{
    EVENT_TRACE_FLAG_CSWITCH,
    EVENT_TRACE_FLAG_IMAGE_LOAD,
    EVENT_TRACE_FLAG_PROCESS,
    EVENT_TRACE_FLAG_PROFILE,
    EVENT_TRACE_FLAG_SYSTEMCALL,
    EVENT_TRACE_FLAG_THREAD,
    EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS,
    EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS
};

size_t g_maxSupportedEvents = 8;

ULONG GetEventTraceFlags(unsigned long enableFlags)
{
    unsigned int traceFlags = 0;
    unsigned long mask = enableFlags;
    unsigned long index;

    unsigned int maxIndex = sizeof(g_EventFlags) / sizeof(unsigned int);

    while (_BitScanForward(&index, mask))
    {
        if (index < maxIndex)
        {
            traceFlags |= g_EventFlags[index];
        }

        // TODO: report invalid flags

        mask &= ~(1 << index);
    }

#if 0

    if ((enableFlags & CXL_TP_CSWITCH) == CXL_TP_CSWITCH)
    {
        traceFlags |= EVENT_TRACE_FLAG_CSWITCH;
    }

    if ((enableFlags & CXL_TP_IMAGE_LOAD) == CXL_TP_IMAGE_LOAD)
    {
        traceFlags |= EVENT_TRACE_FLAG_IMAGE_LOAD;
    }

    if ((enableFlags & CXL_TP_PROCESS) == CXL_TP_PROCESS)
    {
        traceFlags |= EVENT_TRACE_FLAG_PROCESS;
    }

    if ((enableFlags & CXL_TP_THREAD) == CXL_TP_THREAD)
    {
        traceFlags |= EVENT_TRACE_FLAG_THREAD;
    }

    if ((enableFlags & CXL_TP_PROFILE) == CXL_TP_PROFILE)
    {
        traceFlags |= EVENT_TRACE_FLAG_PROFILE;
    }

    if ((enableFlags & CXL_TP_SYSTEMCALL) == CXL_TP_SYSTEMCALL)
    {
        traceFlags |= EVENT_TRACE_FLAG_SYSTEMCALL;
    }

    if ((enableFlags & CXL_TP_MEM_PFAULT) == CXL_TP_MEM_PFAULT)
    {
        traceFlags |= EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS;
    }

#endif //0

    return traceFlags;
}

#if 0
static bool GetUndocAPI()
{
    bool retVal = false;

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
        LPCWSTR pszPathTraceNT = g_pLogFilePath;
        wchar_t pszPathTraceNTMerged[256];

        (*pfn)((unsigned short const **)&pszPathTraceNT, 1, (unsigned short const*)pszPathTraceNTMerged);
    }

    FreeLibrary(hPerfCtrl);

    return retVal;
}
#endif //0

int fnSetThreadProfileConfiguration(UINT32 enableFlags, wchar_t* pLogFilePath)
{
    int ret = S_OK;
    ULONG status = ERROR_SUCCESS;
    ULONG BufferSize = 0;
    ULONG result = 0;
    size_t logFilePathLength = (wcslen(pLogFilePath) * 2) + 2;
    size_t loggerSessionLength = (wcslen(KERNEL_LOGGER_NAME) * 2) + 2;

    // TODO get the client id
    // and access the globals according to the client id
    // error checking
    //   client is registered
    //   profile is in progress

    // Allocate memory for the session properties. The memory must
    // be large enough to include the log file name and session name,
    // which get appended to the end of the session properties structure.

    BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + logFilePathLength + loggerSessionLength;

    if (NULL == g_pSessionProperties)
    {
        g_pSessionProperties = (EVENT_TRACE_PROPERTIES*)malloc(BufferSize);

        if (NULL != g_pSessionProperties)
        {
            // Set the session properties. You only append the log file name
            // to the properties structure; the StartTrace function appends
            // the session name for you.
            // Firt session name then logfile name; sessios name is copied by StartTrace() call

            ZeroMemory(g_pSessionProperties, BufferSize);
            g_pSessionProperties->Wnode.BufferSize = BufferSize;
            g_pSessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
            g_pSessionProperties->Wnode.ClientContext = 1; //QPC clock resolution
            // FIXME: do we really need to specify Guid
            // g_pSessionProperties->Wnode.Guid = ThreadGuid; //  SystemTraceControlGuid;
            g_pSessionProperties->EnableFlags = GetEventTraceFlags(enableFlags); // EVENT_TRACE_FLAG_CSWITCH;
            g_pSessionProperties->LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL; //  EVENT_TRACE_REAL_TIME_MODE; //  EVENT_TRACE_FILE_MODE_CIRCULAR;
            g_pSessionProperties->MaximumFileSize = 512;  // 512 MB
            // g_pSessionProperties->FlushTimer = 1; // ??
            g_pSessionProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + loggerSessionLength;
            g_pSessionProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

            StringCbCopyW((LPWSTR)((char*)g_pSessionProperties + g_pSessionProperties->LogFileNameOffset), logFilePathLength, pLogFilePath);
        }
        else
        {
            wprintf(L"Unable to allocate %d bytes for properties structure.\n", BufferSize);
            ret = E_FAIL;
        }
    }
    else
    {
        wprintf(L"some other session is in progress ...\n");
        ret = E_FAIL;
    }

    return ret;
}

int fnStartThreadProfile()
{
    ULONG status = ERROR_SUCCESS;

    // Create the trace session.
    status = StartTrace((PTRACEHANDLE)&g_SessionHandle, KERNEL_LOGGER_NAME, g_pSessionProperties);

    if (ERROR_SUCCESS != status)
    {
        if (ERROR_ALREADY_EXISTS == status)
        {
            wprintf(L"The NT Kernel Logger session is already in use.\n");
            // TODO: Use ControlTrace() to update
            // result = ControlTrace(0, KERNEL_LOGGER_NAME, pSessionProperties, EVENT_TRACE_CONTROL_UPDATE);
        }
        else
        {
            wprintf(L"EnableTrace() failed with %lu\n", status);
        }
    }
    else
    {

        // StartTrace succeeded.

        // Events in "NT Kernel Logger",
        // - EVENT_TRACE_FLAG_THREAD
        //      - Supported Event Type
        //        1   - EVENT_TRACE_TYPE_START Start thread event. The Thread_V2_TypeGroup1 MOF class defines the event data for this event
        //        2   - EVENT_TRACE_TYPE_END End thread event. The Thread_V2_TypeGroup1 MOF class defines the event data for this event
        //       36  - Context switch event. The CSwitch MOF class defines the event data for this event
        //       50  - Ready thread event. The ReadyThread MOF class defines the event data for this event
        //
        // - EVENT_TRACE_FLAG_IMAGE_LOAD
        //      - Supported Event Type
        //        10   - EVENT_TRACE_TYPE_LOAD Image load event. Generated when a DLL or executable file is loaded. The provider
        //               generates only one event for the first time a given DLL is loaded. The Image_Load MOF class defines the event data for this event
        //        2    - EVENT_TRACE_TYPE_END End thread event. Image unload event. Generated when a DLL or executable file is unloaded.
        //               The provider generates only one event for the last time a given DLL is unloaded.
        //               The Image_Load MOF class defines the event data for this event

        // EVENT_TRACE_FLAG_CSWITCH
        //  Enables the following *Thread* event type -     CSWITCH
        //  FIXME: Do i need to enable EVENT_TRACE_FLAG_THREAD or just enabling EVENT_TRACE_FLAG_CSWITCH is enough?
        //

        // StackWalk_Event class will have the stack data
        //  https://msdn.microsoft.com/en-us/library/windows/desktop/dd392323(v=vs.85).aspx
        //CLASSIC_EVENT_ID traceCSForClasses[] = { { PerfInfoGuid, 46, { 0 } },
        //                                         { ThreadGuid, 36, { 0 } },
        //                                         { ThreadGuid, 50, { 0 } } };

        CLASSIC_EVENT_ID traceCallStackForClasses[] =
        {
            // { ThreadGuid, 1, { 0 } },   // Start Thread Event
            // { ThreadGuid, 2, { 0 } },   // End Thread Event
            { ThreadGuid, 36, { 0 } },  // Context Switch Event
            // { ThreadGuid, 50, { 0 } }   // Ready Thread Event
        };

        status = TraceSetInformation(g_SessionHandle, TraceStackTracingInfo, traceCallStackForClasses, sizeof(traceCallStackForClasses));
        wprintf(L"TraceSetInformation() status(%lu)\n", status);
    }

    return status;
}

int fnStopThreadProfile()
{
    ULONG status = ERROR_SUCCESS;

    if (g_SessionHandle)
    {
        // TODO:
        // To disable stack tracing, call this function with InformationClass set to TraceStackTracingInfo and InformationLength set to 0
        // TODO: disable only if stack tracing is enabled
        status = TraceSetInformation(g_SessionHandle, TraceStackTracingInfo, NULL, 0);
        wprintf(L"Disable stack tracing - TraceSetInformation() status(%lu)\n", status);

        if (NULL != g_pSessionProperties)
        {
            status = ControlTrace(g_SessionHandle, KERNEL_LOGGER_NAME, g_pSessionProperties, EVENT_TRACE_CONTROL_STOP);

            if (ERROR_SUCCESS == status)
            {
                free(g_pSessionProperties);
                g_pSessionProperties = NULL;
            }
            else
            {
                // FIXME: what to do if it fails
                wprintf(L"ControlTrace(stop) failed with %lu\n", status);
            }
        }

        g_SessionHandle = 0;

    }

    if (NULL != g_pSessionProperties)
    {
        free(g_pSessionProperties);
        g_pSessionProperties = NULL;
    }

    return status;
}


ULONG WINAPI EtwProcessBufferCB(PEVENT_TRACE_LOGFILE pBuffer)
{
    UNREFERENCED_PARAMETER(pBuffer);

    return 0;
}

void WINAPI EtwProcessEventCB(PEVENT_TRACE pEventTrace)
{

    fprintf(stderr, "Time: %llu\n", pEventTrace->Header.TimeStamp.QuadPart);
    fprintf(stderr, "Processor number: %u\n", pEventTrace->BufferContext.ProcessorNumber);

    if (pEventTrace->MofData)
    {
        fprintf(stderr, "has MOF data\n");
    }

    return;
}

HRESULT GetTraceEventInfo(PEVENT_RECORD pEventRecord, PTRACE_EVENT_INFO* ppTraceEventInfo)
{
    HRESULT status = ERROR_INVALID_PARAMETER;
    PTRACE_EVENT_INFO pTraceEventInfo = NULL;
    DWORD bufSize = 0;

    if ((NULL != pEventRecord) && (NULL != ppTraceEventInfo))
    {
        pTraceEventInfo = *ppTraceEventInfo;

        // Process the Data
        status = TdhGetEventInformation(pEventRecord, 0, NULL, pTraceEventInfo, &bufSize);

        if (ERROR_INSUFFICIENT_BUFFER == status)
        {
            pTraceEventInfo = (TRACE_EVENT_INFO*)realloc(pTraceEventInfo, bufSize);

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
            wprintf(L"Failed to allocate memory for event info (size=%lu).\n", bufSize);
        }
        else
        {
            wprintf(L"TdhGetEventInformation failed with 0x%x.\n", status);
        }
    }

    return status;
}

void GetTPEvent(GUID guid, UCHAR opcode, ThreadProfileEvent& event, ThreadProfileEventType& eventType)
{
    event = TP_EVENT_UNKNOWN;
    eventType = TP_EVENT_TYPE_UNKNOWN;

    //TP_EVENT_TYPE_PROCESS_START
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
    else if (IsEqualGUID(guid, ImageGuid))
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
    else if (IsEqualGUID(guid, ProcessGuid))
    {
        event = TP_EVENT_PROCESS;

        if ((EVENT_TRACE_TYPE_START == opcode) || (EVENT_TRACE_TYPE_DC_START == opcode) || EVENT_TRACE_TYPE_DC_END == opcode)
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
    else if (IsEqualGUID(guid, ThreadGuid))
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
    else if (IsEqualGUID(guid, StackWalkGuid))
    {
        event = TP_EVENT_STACK;

        if (32 == opcode)
        {
            eventType = TP_EVENT_TYPE_STACK;
        }
    }

    return;
}


char* getTPEventSring(ThreadProfileEvent event)
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
            return "Unknown";
    }

    return "Unknown";
}

char* getTPEventTypeSring(ThreadProfileEventType event)
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
            return "Unknown";
    }

    return "Unknown";
}

int GetTPEventInfo(PEVENT_RECORD pEventRecord, PTRACE_EVENT_INFO pTraceEventInfo, ThreadProfileEventInfo& info)
{
    HRESULT hr = S_OK;

    if ((NULL != pEventRecord) && (NULL != pTraceEventInfo))
    {
        info.m_timeStamp = pEventRecord->EventHeader.TimeStamp;

        GetTPEvent(pEventRecord->EventHeader.ProviderId,                // GUID
                   pEventRecord->EventHeader.EventDescriptor.Opcode,    // opcode
                   info.m_event,
                   info.m_eventType);


        info.m_processId = pEventRecord->EventHeader.ProcessId;
        info.m_threadId = pEventRecord->EventHeader.ThreadId;
        info.m_procTime = pEventRecord->EventHeader.ProcessorTime;
        info.m_kernelTime = pEventRecord->EventHeader.KernelTime;
        info.m_userTime = pEventRecord->EventHeader.UserTime;

        info.m_version = pEventRecord->EventHeader.EventDescriptor.Version;
        info.m_opcode = pEventRecord->EventHeader.EventDescriptor.Opcode;
        info.m_flags = 0; // FIXME

        //fprintf(stdout, " %u 0x%x 0x%x ", pEventRecord->EventHeader.Size,
        //    pEventRecord->EventHeader.Flags, pEventRecord->EventHeader.EventProperty);

        // processor id
        if ((pEventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_PROCESSOR_INDEX) != 0)
        {
            info.m_processorId = pEventRecord->BufferContext.ProcessorIndex;
        }
        else
        {
            info.m_processorId = pEventRecord->BufferContext.ProcessorNumber;
        }

        // If the event contains event-specific data use TDH to extract
        // the event data. For this example, to extract the data, the event
        // must be defined by a MOF class or an instrumentation manifest.

        // Need to get the PointerSize for each event to cover the case where you are
        // consuming events from multiple log files that could have been generated on
        // different architectures. Otherwise, you could have accessed the pointer
        // size when you opened the trace above (see pHeader->PointerSize).

        hr = S_OK;
    }

    return hr;
}

#define ETW_EVENT_TYPE_MAX      64

char gEventTypeStr[ETW_EVENT_TYPE_MAX][32] = { '\0' };

void PrintInitialize()
{
    strcpy(gEventTypeStr[EVENT_TRACE_TYPE_INFO], "Info");           // 0
    strcpy(gEventTypeStr[EVENT_TRACE_TYPE_START], "Start");         // 1
    strcpy(gEventTypeStr[EVENT_TRACE_TYPE_END], "End");             // 2 (end / unload)
    strcpy(gEventTypeStr[EVENT_TRACE_TYPE_DC_START], "DCStart");    // 3
    strcpy(gEventTypeStr[EVENT_TRACE_TYPE_DC_END], "DCEnd");        // 4
    strcpy(gEventTypeStr[EVENT_TRACE_TYPE_EXTENSION], "Extension");        // 5

    strcpy(gEventTypeStr[10], "Load");
    strcpy(gEventTypeStr[32], "Stack");
    strcpy(gEventTypeStr[36], "CSwitch");
    strcpy(gEventTypeStr[39], "Defunt");
    strcpy(gEventTypeStr[50], "ReadyThread");


    // print the header string
    fprintf(stdout, "TimeStamp  Event  EventType  Size  Flags  EventProperty ");
    fprintf(stdout, "ProcessID  ThreadID  ProcessorTime  KernelTime  UserTime  activityId ");
    fprintf(stdout, "EventDescriptor { Id Version Channel Level Opcode Task Keyword }");
    fprintf(stdout, "ExtendedDataCount  UserDataLength \n");

    return;
}

int GetEventString(GUID guid, char* pEventString)
{
    if ((NULL != pEventString))
    {
        if (IsEqualGUID(guid, EventTraceGuid))
            //&& pEventRecord->EventHeader.EventDescriptor.Opcode == EVENT_TRACE_TYPE_INFO)
        {
            // FIXME: do nothing now
            strcpy(pEventString, "Info");
        }
        else if (IsEqualGUID(guid, ImageGuid))
        {
            strcpy(pEventString, "Image");
        }
        else if (IsEqualGUID(guid, ProcessGuid))
        {
            strcpy(pEventString, "Process");
        }
        else if (IsEqualGUID(guid, ThreadGuid))
        {
            strcpy(pEventString, "Thread");
        }
        else if (IsEqualGUID(guid, StackWalkGuid))
        {
            strcpy(pEventString, "Stack");
        }
        else
        {
            strcpy(pEventString, "Unknown");
        }
    }

    return S_OK;
}

int GetEventTypeString(int eventType, char* pEventString)
{
    HRESULT ret = E_FAIL;

    if (NULL != pEventString)
    {
        if (eventType <= ETW_EVENT_TYPE_MAX)
        {
            strcpy(pEventString, gEventTypeStr[eventType]);
            ret = S_OK;
        }
    }

    return ret;
}


HRESULT TPProcessTraceEventRecordMOF(PEVENT_RECORD pEventRecord, PTRACE_EVENT_INFO pTraceEventInfo)
{
    HRESULT hr = S_OK;
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
        int eventType = pEventRecord->EventHeader.EventDescriptor.Opcode;

        GetEventString(pEventRecord->EventHeader.ProviderId, eventString);
        GetEventTypeString(eventType, eventTypeString);

        // Time stamp for when the event occurred.
        ft.dwHighDateTime = pEventRecord->EventHeader.TimeStamp.HighPart;
        ft.dwLowDateTime = pEventRecord->EventHeader.TimeStamp.LowPart;

        FileTimeToSystemTime(&ft, &st);
        SystemTimeToTzSpecificLocalTime(NULL, &st, &stLocal);

        TimeStamp = pEventRecord->EventHeader.TimeStamp.QuadPart;
        Nanoseconds = (TimeStamp % 10000000) * 100;

        //fprintf(stdout, "%02d/%02d/%02d %02d:%02d:%02d.%I64u ",
        //    stLocal.wMonth, stLocal.wDay, stLocal.wYear, stLocal.wHour, stLocal.wMinute, stLocal.wSecond, Nanoseconds);
        fprintf(stdout, "%02d:%02d:%02d.%I64u ", stLocal.wHour, stLocal.wMinute, stLocal.wSecond, Nanoseconds);

        fprintf(stdout, " %s %s ", eventString, eventTypeString);

        fprintf(stdout, " %u 0x%x 0x%x ", pEventRecord->EventHeader.Size,
                pEventRecord->EventHeader.Flags, pEventRecord->EventHeader.EventProperty);
        fprintf(stdout, " %d %d ", pEventRecord->EventHeader.ProcessId, pEventRecord->EventHeader.ThreadId);
        fprintf(stdout, " %lu %lu %lu ", pEventRecord->EventHeader.ProcessorTime, pEventRecord->EventHeader.KernelTime,
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
            g_PointerSize = 4;
        }
        else
        {
            g_PointerSize = 8;
        }

        // CSWitch
        if (eventType == 36)
        {
            ThreadProfileEventCSwitch csRec;
            TPProcessCSwitchRecord(pEventRecord, pTraceEventInfo, csRec);
        }

        fprintf(stdout, "\n");
    }

    return hr;
}


HRESULT PrintTraceEventRecordMOF(PEVENT_RECORD pEventRecord, PTRACE_EVENT_INFO pTraceEventInfo)
{
    HRESULT hr = S_OK;
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
        fprintf(stdout, "%02d:%02d:%02d.%I64u ", stLocal.wHour, stLocal.wMinute, stLocal.wSecond, Nanoseconds);

        fprintf(stdout, " %s %s ", eventString, eventTypeString);

        fprintf(stdout, " %u 0x%x 0x%x ", pEventRecord->EventHeader.Size,
                pEventRecord->EventHeader.Flags, pEventRecord->EventHeader.EventProperty);
        fprintf(stdout, " %d %d ", pEventRecord->EventHeader.ProcessId, pEventRecord->EventHeader.ThreadId);
        fprintf(stdout, " %lu %lu %lu ", pEventRecord->EventHeader.ProcessorTime, pEventRecord->EventHeader.KernelTime,
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
            g_PointerSize = 4;
        }
        else
        {
            g_PointerSize = 8;
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
                int status = TPPrintProperties(pEventRecord, pTraceEventInfo, i, NULL, 0);
                fwprintf(stdout, L"\n");

                if (ERROR_SUCCESS != status)
                {
                    // FIXME
                    // fwprintf(stderr, L"Printing top level properties failed.\n");
                }
            }
        }


        fprintf(stdout, "\n");
    }

#if 0
    LPWSTR pEventGuid = NULL; // TODO: do ew need to pass it back
    hr = StringFromCLSID(pTraceEventInfo->EventGuid, &pEventGuid);

    if (S_OK == hr)
    {
        wprintf(L"\nEvent GUID: %s\n", pEventGuid);
        CoTaskMemFree(pEventGuid);
        pEventGuid = NULL;

        wprintf(L"Event version: %d\n", pEventRecord->EventHeader.EventDescriptor.Version);
        wprintf(L"Event type: %d\n", pEventRecord->EventHeader.EventDescriptor.Opcode);
        wprintf(L"Timestamp: %lu\n", pEventRecord->EventHeader.TimeStamp.QuadPart);
        wprintf(L"Flags: %d\n", pEventRecord->EventHeader.Flags);
        wprintf(L"Process Id: %d\n", pEventRecord->EventHeader.ProcessId);
        wprintf(L"Thread Id: %d\n", pEventRecord->EventHeader.ThreadId);
        wprintf(L"Event Property: %d\n", pEventRecord->EventHeader.EventProperty);
        wprintf(L"Extended Data Count : %d\n", pEventRecord->ExtendedDataCount);
        wprintf(L"User Data Count : %d\n", pEventRecord->UserDataLength);
    }
    else
    {
        wprintf(L"StringFromCLSID failed with 0x%x\n", hr);
    }

#endif

    return hr;
}

VOID WINAPI EtwProcessEventRecordCB(PEVENT_RECORD pEventRecord)
{
    HRESULT status = ERROR_SUCCESS;
    PTRACE_EVENT_INFO pTraceEventInfo = NULL;
    LPWSTR pEventGuid = NULL;
    PBYTE pUserData = NULL;
    PBYTE pEndOfUserData = NULL;
    DWORD pointerSize = 0;
    //ULONGLONG TimeStamp = 0;
    //ULONGLONG Nanoseconds = 0;
    //SYSTEMTIME st;
    //SYSTEMTIME stLocal;
    //FILETIME ft;

    // TODO: what if is pEvent is NULL
    if (NULL == pEventRecord)
    {
        fprintf(stderr, "pEvent is NULL\n");
        return;
    }

    // Skip event trace header. This is available in EVENT_TRACE_LOGFILE.LogfileHeader
    //if (IsEqualGUID(pEventRecord->EventHeader.ProviderId, EventTraceGuid) &&
    //    pEventRecord->EventHeader.EventDescriptor.Opcode == EVENT_TRACE_TYPE_INFO)
    //{
    //    // just printf.. skipping
    //    fprintf(stderr, "Log file header\n");
    //}
    else
    {
        status = GetTraceEventInfo(pEventRecord, &pTraceEventInfo);

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

                if (FAILED(hr))
                {
                    wprintf(L"StringFromCLSID failed with 0x%x\n", hr);
                    status = hr;
                    goto error_exit;
                }


                // PrintTraceEventRecordMOF(pEventRecord, pTraceEventInfo);
                TPProcessTraceEventRecordMOF(pEventRecord, pTraceEventInfo);

                // free the GUID memory
                //wprintf(L"\nEvent GUID: %s\n", pEventGuid);
                CoTaskMemFree(pEventGuid);
                pEventGuid = NULL;
            }
            else
            {
                // we are not interested in others
                fprintf(stderr, "Non MOF\n");
            }
        }
    }

error_exit:
    return;
}

int fnProcessThreadProfileData(wchar_t* logFile)
{
    ULONG status = ERROR_SUCCESS;

    // Open the trace session.
    EVENT_TRACE_LOGFILE traceLogFile;
    TRACE_LOGFILE_HEADER* pLogHeader;
    TRACEHANDLE hTrace = 0;


    memset(&traceLogFile, 0, sizeof(EVENT_TRACE_LOGFILE));
    traceLogFile.LoggerName = NULL;
    traceLogFile.LogFileName = logFile;
    traceLogFile.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD;

    traceLogFile.EventRecordCallback = EtwProcessEventRecordCB;

    //traceLogFile.BufferCallback = EtwProcessBufferCB;
    //traceLogFile.EventCallback = EtwProcessEventCB;

    pLogHeader = &traceLogFile.LogfileHeader;

    hTrace = OpenTrace(&traceLogFile);

    if ((TRACEHANDLE)INVALID_HANDLE_VALUE == hTrace)
    {
        wprintf(L"OpenTrace failed with %lu\n", GetLastError());
        goto cleanup;
    }

    // check the log header filled by OpenTrace
    g_UserMode = ((pLogHeader->LogFileMode & EVENT_TRACE_PRIVATE_LOGGER_MODE) == EVENT_TRACE_PRIVATE_LOGGER_MODE)
                 ? true : false;

    if (pLogHeader->TimerResolution > 0)
    {
        g_TimerResolution = pLogHeader->TimerResolution / 10000;
    }

    fprintf(stderr, "userMode : %d timerResolution : %lu # Lost Events : %d \n",
            g_UserMode, g_TimerResolution, pLogHeader->EventsLost);

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

    fprintf(stderr, "# Lost Buffers : %d \n", pLogHeader->BuffersLost);

    status = ProcessTrace(&hTrace, 1, 0, 0);

    if (status != ERROR_SUCCESS && status != ERROR_CANCELLED)
    {
        wprintf(L"ProcessTrace failed with %lu\n", status);
        goto cleanup;
    }



cleanup:

    if ((TRACEHANDLE)INVALID_HANDLE_VALUE != hTrace)
    {
        status = CloseTrace(hTrace);
    }

    return status;
}

void Cleanup()
{
    fnStopThreadProfile();
}



void main(int argc, char* argv[])
{
    ULONG status = ERROR_SUCCESS;
    bool isCollectRun = true;
    bool isReportRun = true;

    if (argc == 2)
    {
        if (!strcmp(argv[1], "-c"))
        {
            isReportRun = false;
        }
        else if (!strcmp(argv[1], "-r"))
        {
            isCollectRun = false;
        }
    }

    if (isCollectRun)
    {
        // FIXME: do we need to do this?
        // Get the undocumented API
        // GetUndocAPI();

        // unsigned long flags = CXL_TP_CSWITCH | CXL_TP_IMAGE_LOAD | CXL_TP_PROCESS | CXL_TP_THREAD;
        unsigned long flags = CXL_TP_PROCESS | CXL_TP_CSWITCH | CXL_TP_IMAGE_LOAD;

        // Create the thread profile configuration
        status = fnSetThreadProfileConfiguration(flags, const_cast<wchar_t*>(g_pLogFilePath));

        if (ERROR_SUCCESS != status)
        {
            wprintf(L"fnSetThreadProfileConfiguration is failed %d.\n", status);
            goto error_exit;
        }

        // start the thread profile
        status = fnStartThreadProfile();

        if (ERROR_SUCCESS != status)
        {
            wprintf(L"fnStartThreadProfile is failed %d.\n", status);
            goto error_exit;
        }

        wprintf(L"Press any key to end trace session ");
        _getch();

        // stop the thread profile
        status = fnStopThreadProfile();

        if (ERROR_SUCCESS != status)
        {
            wprintf(L"fnStopThreadProfile is failed %d.\n", status);
            goto error_exit;
        }
    }

    if (isReportRun)
    {
        // Process the file
        PrintInitialize();

        fnProcessThreadProfileData(const_cast<wchar_t*>(g_pLogFilePath));
    }

error_exit:
    Cleanup();
}


DWORD TPGetProperty(PEVENT_RECORD pEvent,
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

    DWORD Count = 0;  // Expects the count to be defined by a UINT16 or UINT32

    status = TdhGetPropertySize(pEvent, 0, NULL, descriptorCount, pDataDescriptor, pPropertySize);
    // TODO: check return code

    if (NULL != ppData)
    {
        *ppData = (PBYTE)malloc(*pPropertySize);
    }

    status = TdhGetProperty(pEvent, 0, NULL, descriptorCount, pDataDescriptor, *pPropertySize, *ppData);
    // TODO: check return code

    //*pArraySize = (USHORT)Count;

    return status;
}

// Get the size of the array. For MOF-based events, the size is specified in the declaration or using
// the MAX qualifier. For manifest-based events, the property can specify the size of the array
// using the count attribute. The count attribue can specify the size directly or specify the name
// of another property in the event data that contains the size.

DWORD TPGetPropertyData(PEVENT_RECORD pEvent,
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
        DWORD Count = 0;  // Expects the count to be defined by a UINT16 or UINT32
        DWORD j = pInfo->EventPropertyInfoArray[i].countPropertyIndex;

        ZeroMemory(pDataDescriptor, sizeof(PROPERTY_DATA_DESCRIPTOR));

        pDataDescriptor->PropertyName = (ULONGLONG)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[j].NameOffset);
        pDataDescriptor->ArrayIndex = ULONG_MAX;

        //status = TdhGetPropertySize(pEvent, 0, NULL, 1, pDataDescriptor, &PropertySize);
        //// TODO: check return code
        //status = TdhGetProperty(pEvent, 0, NULL, 1, pDataDescriptor, PropertySize, (PBYTE)&Count);
        //// TODO: check return code

        PBYTE pData;
        TPGetProperty(pEvent, 1, pDataDescriptor, pPropertySize, &pData);
        // TODO: check return code

        if (NULL != ppData)
        {
            *ppData = pData;
        }

        *pArraySize = *((USHORT*)pData);
    }
    else
    {
        *pArraySize = pInfo->EventPropertyInfoArray[i].count;
    }

    return status;
}


// Incomplete.. should replace TPFormatAndPrintData

//TDH_INTYPE_UNICODESTRING(1)
//TDH_INTYPE_ANSISTRING(2)
//TDH_INTYPE_INT8(3)
//TDH_INTYPE_UINT8(4)
//TDH_INTYPE_INT32(7)
//TDH_INTYPE_UINT32(8)
//TDH_INTYPE_UINT64(10)
//TDH_INTYPE_POINTER(16)
//TDH_INTYPE_WBEMSID(310)

DWORD TPGetUnicodeString(USHORT InType, PBYTE pData, size_t& StringLength, LPWSTR& pString)
{
    DWORD status = ERROR_NOT_FOUND;

    if (TDH_INTYPE_UNICODESTRING == InType)
    {
        StringLength = wcslen((LPWSTR)pData);

        wprintf(L"%.*s ", StringLength, (LPWSTR)pData);
        pString = (LPWSTR)pData;

        status = ERROR_SUCCESS;
    }

    return status;
}

DWORD TPGetAnsiString(USHORT InType, PBYTE pData, size_t& StringLength, LPSTR& pString)
{
    DWORD status = ERROR_NOT_FOUND;

    if (TDH_INTYPE_ANSISTRING == InType)
    {
        StringLength = strlen((LPSTR)pData);
        pString = (LPSTR)pData;

        wprintf(L"%.*S ", StringLength, (LPSTR)pData);
        status = ERROR_SUCCESS;
    }

    return status;
}

INT8 TPGetInt8(USHORT InType, PBYTE pData)
{
    INT8 data = 0x7F;

    if (TDH_INTYPE_INT8 == InType)
    {
        wprintf(L"%hd ", *(PCHAR)pData);
        data = *(PCHAR)pData;
    }

    return data;
}

UINT8 TPGetUInt8(USHORT InType, PBYTE pData)
{
    UINT8 data = 0xFF;

    if (TDH_INTYPE_UINT8 == InType)
    {
        wprintf(L"%hu ", *(PCHAR)pData);
        data = *(PCHAR)pData;
    }

    return data;
}

LONG TPGetInt32(USHORT InType, PBYTE pData)
{
    LONG data = 0xFFFFFFFF;

    if (TDH_INTYPE_INT32 == InType)
    {
        wprintf(L"%d ", *(PSHORT)pData);
        data = *(PSHORT)pData;
    }

    return data;
}

ULONG TPGetUInt32(USHORT InType, PBYTE pData)
{
    ULONG data = 0xFFFFFFFFUL;

    if (TDH_INTYPE_UINT32 == InType)
    {
        wprintf(L"%u ", *(PUSHORT)pData);
        data = *(PUSHORT)pData;
    }

    return data;
}

ULONGLONG TPGetUInt64(USHORT InType, PBYTE pData)
{
    ULONGLONG data = 0xFFFFFFFFFFFFFFFFULL;

    if (TDH_INTYPE_UINT32 == InType)
    {
        wprintf(L"%I64u ", *(PULONGLONG)pData);
        data = *(PULONGLONG)pData;
    }

    return data;
}

ULONG TPGetPUlong(USHORT InType, PBYTE pData)
{
    ULONG ptrData = 0;

    if ((TDH_INTYPE_POINTER == InType) && (4 == g_PointerSize))
    {
        wprintf(L"0x%x\n", *(PULONG)pData);
        ptrData = *(PULONG)pData;
    }

    return ptrData;
}

ULONGLONG TPGetPUlonglong(USHORT InType, PBYTE pData)
{
    ULONGLONG ptrData = 0;

    if ((TDH_INTYPE_POINTER == InType) && (8 == g_PointerSize))
    {
        wprintf(L"0x%x\n", *(PULONGLONG)pData);
        ptrData = *(PULONGLONG)pData;
    }

    return ptrData;
}


DWORD TPGetWbemsid(USHORT InType, PBYTE pData, DWORD DataSize)
{
    DWORD status = ERROR_SUCCESS;

    if (TDH_INTYPE_WBEMSID == InType)
    {
        WCHAR UserName[MAX_NAME];
        WCHAR DomainName[MAX_NAME];
        DWORD cchUserSize = MAX_NAME;
        DWORD cchDomainSize = MAX_NAME;
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

            pData += g_PointerSize * 2;

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
                wprintf(L"%s\\%s\n", DomainName, UserName);
            }
        }
    }

    return status;
}


// Both MOF-based events and manifest-based events can specify name/value maps. The
// map values can be integer values or bit values. If the property specifies a value
// map, get the map.

DWORD TPGetMapInfo(PEVENT_RECORD pEvent, LPWSTR pMapName, DWORD DecodingSource, PEVENT_MAP_INFO& pMapInfo)
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
            TPRemoveTrailingSpace(pMapInfo);
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


DWORD TPFormatAndPrintData(PEVENT_RECORD pEvent,
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
                wprintf(L"%hu ", ntohs(*(PUSHORT)pData));
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
                    TPPrintMapString(pMapInfo, pData);
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
                wprintf(L"0x%x ", *(PULONGLONG)pData);
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
            // wprintf(L"%I64f\n", *(DOUBLE*)pData);
            wprintf(L"%I64f ", *(DOUBLE*)pData);

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
                fwprintf(stdout, L"IPv6Address !!\n");
            }
            else
            {
                for (DWORD i = 0; i < DataSize; i++)
                {
                    wprintf(L"%.2x", pData[i]);
                }

                // wprintf(L"\n");
                wprintf(L" ");
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
            if (4 == g_PointerSize)
            {
                // wprintf(L"0x%x\n", *(PULONG)pData);
                wprintf(L"0x%x ", *(PULONG)pData);
            }
            else
            {
                // wprintf(L"0x%x\n", *(PULONGLONG)pData);
                wprintf(L"0x%x ", *(PULONGLONG)pData);
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
            WCHAR UserName[MAX_NAME];
            WCHAR DomainName[MAX_NAME];
            DWORD cchUserSize = MAX_NAME;
            DWORD cchDomainSize = MAX_NAME;
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
            wprintf(L"0x%x ", (PULONG)pData);
            break;
        }

        case TDH_INTYPE_HEXINT64:
        {
            // wprintf(L"0x%x\n", (PULONGLONG)pData);
            wprintf(L"0x%x ", (PULONGLONG)pData);
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
            WCHAR UserName[MAX_NAME];
            WCHAR DomainName[MAX_NAME];
            DWORD cchUserSize = MAX_NAME;
            DWORD cchDomainSize = MAX_NAME;
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

                pData += g_PointerSize * 2;

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

void TPRemoveTrailingSpace(PEVENT_MAP_INFO pMapInfo)
{
    SIZE_T ByteLength = 0;

    for (DWORD i = 0; i < pMapInfo->EntryCount; i++)
    {
        ByteLength = (wcslen((LPWSTR)((PBYTE)pMapInfo + pMapInfo->MapEntryArray[i].OutputOffset)) - 1) * 2;
        *((LPWSTR)((PBYTE)pMapInfo + (pMapInfo->MapEntryArray[i].OutputOffset + ByteLength))) = L'\0';
    }
}

int TPProcessCSwitchRecord(PEVENT_RECORD pEventRecord,
                           PTRACE_EVENT_INFO pTraceEventInfo,
                           ThreadProfileEventCSwitch& csRec)
{
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

        ThreadProfileEventCSwitch csRec;

        for (USHORT i = 0; i < pTraceEventInfo->TopLevelPropertyCount; i++)
        {
            //int status = TPPrintProperties(pEventRecord, pTraceEventInfo, i, NULL, 0);
            int status = TPProcessCSwitchRecord__(pEventRecord, pTraceEventInfo, i, NULL, 0, csRec);
            // TODO: cosume csRec here
        }

        fwprintf(stdout, L"TopLevelPropertyCount : %d ", pTraceEventInfo->TopLevelPropertyCount);
    }

    return S_OK;
} // TPProcessCSwitchRecord

int TPProcessCSwitchRecord__(PEVENT_RECORD pEvent,
                             PTRACE_EVENT_INFO pInfo,
                             USHORT i,
                             LPWSTR pStructureName,
                             USHORT StructIndex,
                             ThreadProfileEventCSwitch& csRec)
{
    DWORD status = ERROR_SUCCESS;
    DWORD LastMember = 0;  // Last member of a structure
    USHORT ArraySize = 0;

    PROPERTY_DATA_DESCRIPTOR DataDescriptor;
    PROPERTY_DATA_DESCRIPTOR DataDescriptors[2];
    ULONG DescriptorsCount = 0;
    DWORD PropertySize = 0;
    PBYTE pData = NULL;

    // Get the property data
    status = TPGetPropertyData(pEvent, pInfo, i, &DataDescriptor, &PropertySize, &pData, &ArraySize);

    if (ERROR_SUCCESS == status)
    {
        // For CSWitch ArraySize == 1
        for (USHORT k = 0; k < ArraySize; k++)
        {
            LPWSTR propertyName = (LPWSTR)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset);

            fwprintf(stdout, L"*struct* %s* ", propertyName);

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

            status = TPGetProperty(pEvent, DescriptorsCount, &DataDescriptors[0], &PropertySize, &pData);

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
                    csRec.m_newThreadId = TPGetUInt32(inType, pData);
                    break;

                case 1:
                    csRec.m_oldThreadId = TPGetUInt32(inType, pData);
                    break;

                case 2:
                    csRec.m_newThreadPriority = TPGetInt8(inType, pData);
                    break;

                case 3:
                    csRec.m_oldThreadPriority = TPGetInt8(inType, pData);
                    break;

                case 4:
                    csRec.m_previousCState = TPGetUInt8(inType, pData);
                    break;

                case 5:
                    csRec.m_spareByte = TPGetInt8(inType, pData);
                    break;

                case 6:
                    csRec.m_oldThreadWaitReason = TPGetInt8(inType, pData);
                    break;

                case 7:
                    csRec.m_oldThreadWaitMode = TPGetInt8(inType, pData);
                    break;

                case 8:
                    csRec.m_oldThreadState = TPGetInt8(inType, pData);
                    break;

                case 9:
                    csRec.m_oldThreadWaitIdealProcessor = TPGetInt8(inType, pData);
                    break;

                case 10:
                    csRec.m_newThreadWaitTime = TPGetUInt32(inType, pData);
                    break;

                case 11:
                    csRec.m_reserved = TPGetUInt32(inType, pData);
                    break;
            }
        }
    }

    if (ERROR_SUCCESS != status)
    {
        fwprintf(stdout, L"TPGetPropertyData failed\n");
    }

    if (NULL != pData)
    {
        free(pData);
        pData = NULL;
    }

    return status;
} // TPProcessCSwitchRecord__


int TPPrintProperties(PEVENT_RECORD pEvent,
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
    status = TPGetPropertyData(pEvent, pInfo, i, &DataDescriptor, &PropertySize, &pData, &ArraySize);

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
                status = TPPrintProperties(pEvent, pInfo, j, (LPWSTR)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].NameOffset), k);

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

            status = TPGetProperty(pEvent, DescriptorsCount, &DataDescriptors[0], &PropertySize, &pData);

            if (ERROR_SUCCESS != status)
            {
                fwprintf(stdout, L"TPGetProperty failed with %lu\n", status);
                goto error_exit;
            }

            // TODO: get name/value mapping, if the property specifies a value map
            status = TPGetMapInfo(pEvent,
                                  (PWCHAR)((PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[i].nonStructType.MapNameOffset),
                                  pInfo->DecodingSource,
                                  pMapInfo);

            if (ERROR_SUCCESS != status)
            {
                fwprintf(stdout, L"TPGetMapInfo failed\n");
                goto error_exit;
            }

            status = TPFormatAndPrintData(pEvent,
                                          pInfo->EventPropertyInfoArray[i].nonStructType.InType,
                                          pInfo->EventPropertyInfoArray[i].nonStructType.OutType,
                                          pData,
                                          PropertySize,
                                          pMapInfo);

            if (ERROR_SUCCESS != status)
            {
                fwprintf(stdout, L"TPFormatAndPrintData failed\n");
                goto error_exit;
            }

            if (NULL != pData)
            {
                free(pData);
                pData = NULL;
            }

            if (pMapInfo)
            {
                free(pMapInfo);
                pMapInfo = NULL;
            }
        }
    }

error_exit:

    if (NULL != pData)
    {
        free(pData);
        pData = NULL;
    }

    if (pMapInfo)
    {
        free(pMapInfo);
        pMapInfo = NULL;
    }

    return status;
}


void TPPrintMapString(PEVENT_MAP_INFO pMapInfo, PBYTE pData)
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