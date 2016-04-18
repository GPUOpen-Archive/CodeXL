//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileControl.cpp
///
//==================================================================================

#if (defined (_WIN32) || defined (_WIN64))

    #define CODEXL_OSWRAPPERS_AVBL      1
    #define CODEXL_LIBCPUPERFEVENT_AVBL 1

    #include "Windows/CpuProfileControl_Win.h"

#else

    #include "Linux/CpuProfileControl_Lin.h"

#endif

#include <CpuProfileControl.h>
#include <AMDTCpuProfilingTranslation/inc/CpuProfileDataTranslation.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osCpuid.h>


//state of the driver
ProfileState g_profilingState[MAX_CLIENT_COUNT];

//Driver Client id, index is id, value is the process id
osProcessId g_clientId[MAX_CLIENT_COUNT];
gtUInt32 INVALID_CLIENT = (gtUInt32)(-1);
//last error string, overwritten with every error
gtString g_errorString[MAX_CLIENT_COUNT];
gtString g_invalidClientErr;

//profile configuration info
unsigned long g_profileType[MAX_CLIENT_COUNT];


// Baskar:
// Currently LibCpuPerfEvent is available only for Windows; This should be in Linux
// too in near future. Till then i am making this code as Windows specific.
#ifdef CODEXL_LIBCPUPERFEVENT_AVBL
    EventsFile* gp_eventsFile = NULL;
#endif // CODEXL_LIBCPUPERFEVENT_AVBL

//Defines and constants
unsigned int g_maxCore = 0;
gtUInt64 g_SystemCoreMask = 0;

gtUInt64* g_pSystemCoreMask = NULL;
gtUInt32 g_SystemCoreMaskCount = 0;
gtUInt32 g_SystemCoreMaskSize = 0;

unsigned int g_maxEventConfigs = 0;


//
// Helper Functions
//

gtUInt32 helpGetClientId()
{
    gtUInt32 clientId = INVALID_CLIENT;

    osCpuid cpuid;

    if (!cpuid.hasHypervisor() || cpuid.isSupportedHypervisor())
    {
        osProcessId pid = osGetCurrentProcessId();

        for (gtUInt32 i = 0; i < MAX_CLIENT_COUNT; i++)
        {
            if (g_clientId[i] == pid)
            {
                clientId = i;
                break;
            }
        }
    }

    return clientId;
}


HRESULT validateCoreMaskArray(
    /*in*/ gtUInt32 clientId,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize)
{
    if (NULL == pCpuCoreMask)
    {
        g_errorString[clientId] = L"pCpuCoreMask must be non NULL";
        return E_INVALIDARG;
    }

    if (0 == cpuCoreMaskSize)
    {
        g_errorString[clientId] = L"cpuCoreMaskSize must be greater than 0";
        return E_INVALIDARG;
    }

    if (NULL == g_pSystemCoreMask)
    {
        // Mohit: should we just return S_OK if g_pSystemCoreMask is NULL?
        g_errorString[clientId] = L"Unable to get the active core count";
        return E_UNEXPECTED;
    }

    // Compare pCpuCoreMask with g_pSystemCoreMask
    for (unsigned int i = 0; i < cpuCoreMaskSize; i++)
    {
        if (pCpuCoreMask[i] > g_pSystemCoreMask[i])
        {
            wchar_t num[19]; //16 bytes + 0x + \0
            wchar_t buffer[75];

            swprintf(num, 19, L"0x%llx", pCpuCoreMask[i]);
            swprintf(buffer, 75,
                     L"The cpuCoreMask (%s) exceeded the maximum mask of 0x%llx",
                     num, g_pSystemCoreMask[i]);
            g_errorString[clientId] = buffer;
            return E_INVALIDARG;
        }
    }

    return S_OK;
}


//
// APIs
//

HRESULT fnEnableProfiling()
{
    //If this process has already called fnEnableProfiling
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    gtUInt32 clientId = helpGetClientId();

    if (INVALID_CLIENT != clientId)
    {
        return S_FALSE;
    }

#endif

    HRESULT res = S_OK;
    res = CpuPerfEnableProfiling();

    return res;
}


HRESULT fnReleaseProfiling()
{
    HRESULT hr = S_OK;
    gtUInt32 clientId = helpGetClientId();

    if ((INVALID_CLIENT == clientId) ||
        (ProfilingUnavailable == g_profilingState[clientId]))
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return S_FALSE;
    }

    hr = CpuPerfReleaseProfiling();

    return hr;
}


HRESULT fnGetDriverVersion(
    unsigned int* pMajor,
    unsigned int* pMinor,
    unsigned int* pBuild)

{
    gtUInt32 clientId = helpGetClientId();

    if ((INVALID_CLIENT == clientId) ||
        (ProfilingUnavailable == g_profilingState[clientId]))
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (NULL == pMajor)
    {
        g_errorString[clientId] = L"pMajor was NULL";
        return E_INVALIDARG;
    }

    if (NULL == pMinor)
    {
        g_errorString[clientId] = L"pMinor was NULL";
        return E_INVALIDARG;
    }

    if (NULL == pBuild)
    {
        g_errorString[clientId] = L"pBuild was NULL";
        return E_INVALIDARG;
    }

    HRESULT hr = E_ACCESSDENIED;
    hr = CpuPerfGetDriverVersion(pMajor, pMinor, pBuild);
    return hr;
}


HRESULT fnMakeProfileEvent(
    /*in*/ unsigned int eventSelect,
    /*in*/ unsigned int unitMask,
    /*in*/ bool edgeDetect,
    /*in*/ bool usrEvents,
    /*in*/ bool osEvents,
    /*in*/ bool guestOnlyEvents,
    /*in*/ bool hostOnlyEvents,
    /*in*/ bool countingEvent,
    /*out*/ gtUInt64* pPerformanceEvent)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == pPerformanceEvent)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pPerformanceEvent was NULL";
        }

        return E_INVALIDARG;
    }

    if ((guestOnlyEvents) && (hostOnlyEvents))
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"guestOnlyEvents and hostOnlyEvents are exclusive";
        }

        return E_INVALIDARG;
    }

    if (guestOnlyEvents)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"guestOnlyEvents is reserved for future versions";
        }

        return E_INVALIDARG;
    }

    if (hostOnlyEvents)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"hostOnlyEvents is reserved for future versions";
        }

        return E_INVALIDARG;
    }

#ifdef CODEXL_LIBCPUPERFEVENT_AVBL

    //is the event and unit mask valid?
    if (NULL == gp_eventsFile)
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"The event file was not available");

        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = buffer;
        }

        return E_FAIL;
    }

    if (!gp_eventsFile->ValidateEvent(eventSelect, unitMask))
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"The event (0x%x) and unit mask (0x%x) were not validated",
                 eventSelect, unitMask);

        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = buffer;
        }

        return E_INVALIDARG;
    }

    CpuEvent eventData;

    if (!gp_eventsFile->FindEventByValue(eventSelect, eventData))
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"The event (0x%x) information could not be found",
                 eventSelect);

        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = buffer;
        }

        return E_INVALIDARG;
    }

    //Don't allow events which are not valid on this cpu
    osCpuid cpuid;
    int model;
    // GetCpuFamily (&model);
    model = cpuid.getModel();

    if (eventData.m_minValidModel > model)
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"The event (0x%x) was not valid on this cpu version",
                 eventSelect);

        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = buffer;
        }

        return E_INVALIDARG;
    }

#endif // CODEXL_LIBCPUPERFEVENT_AVBL

    HRESULT hr = S_OK;
    hr = CpuPerfMakeProfileEvent(eventSelect,
                                 unitMask,
                                 edgeDetect,
                                 usrEvents,
                                 osEvents,
                                 guestOnlyEvents,
                                 hostOnlyEvents,
                                 countingEvent,
                                 pPerformanceEvent);

    return hr;
}


//This will need to be updated once we have a processor with more than 4 event
//  counters
HRESULT fnGetEventCounters(
    /*out*/ unsigned int* pEventCounterCount,
    /*in*/  PmcType type)
{
    if (NULL == pEventCounterCount)
    {
        gtUInt32 clientId = helpGetClientId();

        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pEventCounterCount was NULL";
        }

        return E_INVALIDARG;
    }

#ifdef CODEXL_OSWRAPPERS_AVBL
    osCpuid cpuid;

    switch (type)
    {
        case PMC_CORE:
            if (cpuid.getFamily() < FAMILY_OR)
            {
                *pEventCounterCount = 4;
            }
            else
            {
                if (cpuid.getFamily() == FAMILY_KB)
                {
                    *pEventCounterCount = 4;
                }
                else
                {
                    *pEventCounterCount = 6;
                }
            }

            break;

        case PMC_NORTHBRIDGE:
            if (cpuid.getFamily() < FAMILY_OR)
            {
                return E_INVALIDARG;
            }
            else
            {
                *pEventCounterCount = 4;
            }

        case PMC_L2I:
            if (cpuid.getFamily() < FAMILY_KB)
            {
                return E_INVALIDARG;
            }
            else
            {
                *pEventCounterCount = 4;
            }

        default:
            return  E_INVALIDARG;
    }

#else
    // FIXME: just a temp workaround
    *pEventCounterCount = 4;
    (void)(type); // unused
#endif // CODEXL_OSWRAPPERS_AVBL

    return S_OK;
}


//This function retrieves the availability of event counters per core.
HRESULT fnGetEventCounterAvailability(
    /*in*/ gtUInt64 performanceEvent,
    /*out*/ unsigned int* pAvailabilityMask,
    /*in*/  PmcType type)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == pAvailabilityMask)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pAvailabilityMask was NULL";
        }

        return E_INVALIDARG;
    }

    HRESULT hr = CpuPerfGetEventCounterAvailability(performanceEvent,
                                                    pAvailabilityMask,
                                                    type);

    return hr;
}


HRESULT fnGetProfilerState(
    /*out*/ ProfileState* pProfileState)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == pProfileState)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pProfileState was NULL";
        }

        return E_INVALIDARG;
    }

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        *pProfileState = ProfilingUnavailable;
        return S_OK;
    }

    HRESULT hr = S_OK;
    hr = CpuPerfGetProfilerState(pProfileState);
    return hr;
}


HRESULT fnStartProfiling(
    /*in*/ bool startPaused,
    /*in*/ const wchar_t* pauseKey,
    /*out*/ ProfileState* pProfileState)
{
    gtUInt32 clientId = helpGetClientId();

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (NULL != pProfileState)
    {
        *pProfileState = g_profilingState[clientId];
    }

    if (ProfilingStopped != g_profilingState[clientId])
    {
        g_errorString[clientId] = L"The profiler was not ready to start profiling";
        return E_ACCESSDENIED;
    }

    //verify that at least one configuration has been set
    if (0 == g_profileType[clientId])
    {
        g_errorString[clientId] = L"The profiler was not configured. For Custom Profile, at least one event should be present in the Monitored Events list.";
        return E_PENDING;
    }

    HRESULT hr = S_OK;
    hr = CpuPerfStartProfiling(startPaused,
                               pauseKey,
                               pProfileState);

    return hr;
}


/** This will provide the profile control key for the pauseKey.  The profile
    control key is used for /ref fnPauseProfiling and /ref fnResumeProfiling

    \ingroup profiling
    @param[in] pauseKey If you want to pause a profile from another
    application, provide the same string as was provided to \ref
    fnStartProfiling to control the profile.  The pause key used by the
    CodeAnalyst Gui and CaProfile profiles is \ref CPU_PROFILE_PAUSE_KEY.
    \return The profile control key
    \retval NULL There was no active profile with the pause key
*/
CaProfileCtrlKey fnGetProfileControlKey(const wchar_t* pauseKey)
{
    return CpuPerfGetProfileControlKey(pauseKey);
}


HRESULT fnPauseProfiling(CaProfileCtrlKey profileKey)
{
    return CpuPerfPauseProfiling(profileKey);
}


HRESULT fnResumeProfiling(CaProfileCtrlKey profileKey)
{
    return CpuPerfResumeProfiling(profileKey);
}


HRESULT fnStopProfiling()
{
    gtUInt32 clientId = helpGetClientId();

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"The profiler was not ready to stop";
        return E_ACCESSDENIED;
    }

    HRESULT hr = CpuPerfStopCPUUtilMonitoring(clientId);

    if ((HRESULT)E_ACCESSDENIED == hr)
    {
        return hr;
    }

    hr = CpuPerfStopSamplingProfile(clientId);

    return hr;
}


HRESULT fnGetLastProfileError(
    /*in*/ unsigned int size,
    /*out*/ wchar_t* pErrorString)
{
    if (NULL == pErrorString)
    {
        return (E_INVALIDARG) ;
    }

    if (size <= 1)
    {
        //Size 1 is just enough space for the \0 character
        return S_FALSE;
    }

    gtUInt32 clientId = helpGetClientId();

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    errno_t err ;

    if (INVALID_CLIENT == clientId)
    {
        err = wcsncpy_s(pErrorString,       // Destination string
                        size,                       // Size of the destination string
                        g_invalidClientErr.asCharArray(), // Source string
                        _TRUNCATE) ;                // Truncate to fit destination
    }
    else
    {
        err = wcsncpy_s(pErrorString,   // Destination string
                        size,               // Size of the destination string
                        g_errorString[clientId].asCharArray(), // Source string
                        _TRUNCATE) ;            // Truncate to fit destination
    }

    if (err == STRUNCATE)
    {
        // STRUNCATE requires us to include errno.h
        return (S_FALSE) ;
    }

#else
    // Linux
    const wchar_t* pWchar = (INVALID_CLIENT == clientId) ? g_invalidClientErr.asCharArray() : g_errorString[clientId].asCharArray();

    if (size < (wcslen(pWchar) + 1))
    {
        return S_FALSE;
    }

    wcscpy(pErrorString, pWchar);
#endif

    return S_OK;
}


HRESULT fnSetCountingEvent(
    /*in*/ unsigned int core,
    /*in*/ unsigned int eventCounterIndex,
    /*in*/ EventConfiguration performanceEvent)
{
    gtUInt32 clientId = helpGetClientId();

    if ((INVALID_CLIENT == clientId))
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (core >= g_maxCore)
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"The core (%d) was not valid, the max is %d",
                 core, g_maxCore - 1);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    HRESULT hr = CpuPerfSetCountingEvent(core,
                                         eventCounterIndex,
                                         performanceEvent);

    return hr;
}


HRESULT fnGetEventCount(
    /*in*/ unsigned int core,
    /*in*/ unsigned int eventCounterIndex,
    /*out*/ gtUInt64* pEventCount)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == pEventCount)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pEventCount was NULL";
        }

        return E_INVALIDARG;
    }

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (core >= g_maxCore)
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"The core (%d) was not valid, the max is %d",
                 core, g_maxCore - 1);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    HRESULT hr = CpuPerfGetEventCount(core,
                                      eventCounterIndex,
                                      pEventCount);
    return hr;
}


HRESULT fnSetCountingConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ gtUInt64 cpuCoreMask)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == pPerformanceEvents)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pPerformanceEvent was NULL";
        }

        return E_INVALIDARG;
    }

    if (0 == count)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"count was 0";
        }

        return E_INVALIDARG;
    }

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (g_maxEventConfigs < count)
    {
        wchar_t buffer[75];
        swprintf(buffer, 75, L"The maximum number of EventConfigurations is %d",
                 g_maxEventConfigs);
        g_errorString[clientId] = buffer;
    }

    bool profileAllCores = false;

    if ((gtUInt64) - 1 == cpuCoreMask)
    {
        profileAllCores = true;
    }

    if (0 == cpuCoreMask)
    {
        g_errorString[clientId] = L"cpuCoreMask must be greater than 0";
        return E_INVALIDARG;
    }

    if ((false == profileAllCores) && (cpuCoreMask > g_SystemCoreMask))
    {
        wchar_t buffer[75];
        swprintf(buffer, 75, L"The cpumask (0x%llx) exceeded the maximum mask of 0x%llx",
                 cpuCoreMask, g_SystemCoreMask);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    HRESULT hr = CpuPerfSetCountingConfiguration(pPerformanceEvents,
                                                 count,
                                                 cpuCoreMask,
                                                 profileAllCores);
    return hr;
}  //fnSetCountingConfiguration


HRESULT fnSetCountingConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == pPerformanceEvents)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pPerformanceEvent was NULL";
        }

        return E_INVALIDARG;
    }

    if (0 == count)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"count was 0";
        }

        return E_INVALIDARG;
    }

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (g_maxEventConfigs < count)
    {
        wchar_t buffer[75];
        swprintf(buffer, 75, L"The maximum number of EventConfigurations is %d",
                 g_maxEventConfigs);
        g_errorString[clientId] = buffer;
    }

    if ((0 == cpuCoreMaskSize) && (NULL != pCpuCoreMask))
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pCpuCoreMask was not NULL";
        }

        return E_INVALIDARG;
    }

    if ((0 != cpuCoreMaskSize) && (NULL == pCpuCoreMask))
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"cpuCoreMaskSize was not 0";
        }

        return E_INVALIDARG;
    }

    bool profileAllCores = false;

    // if cpuCoreMaskSize is 0, profile all the cores;
    // Set EVENT_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
    if ((NULL == pCpuCoreMask) && (0 == cpuCoreMaskSize))
    {
        profileAllCores = true;
    }

    HRESULT hr = S_OK;

    if (false == profileAllCores)
    {
        hr = validateCoreMaskArray(clientId,
                                   pCpuCoreMask,
                                   cpuCoreMaskSize);

        if (S_OK != hr)
        {
            return hr;
        }
    }

    hr = CpuPerfSetCountingConfiguration(clientId,
                                         pPerformanceEvents,
                                         count,
                                         pCpuCoreMask,
                                         cpuCoreMaskSize,
                                         profileAllCores);

    return hr;
}  // fnSetCountingConfiguration


HRESULT fnGetCountingEventCount(
    unsigned int core,
    unsigned int* pCount)
{
    if ((core >= g_maxCore) || (NULL == pCount))
    {
        return E_INVALIDARG;
    }

    gtUInt32 clientId = helpGetClientId();

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    *pCount = 0;

    HRESULT hr = CpuPerfGetCountingEventCount(core, pCount);
    return hr;
}


HRESULT fnGetAllEventCounts(
    /*in*/ unsigned int core,
    /*in*/ unsigned int size,
    /*out*/ gtUInt64* pCounts)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == pCounts)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pCounts was NULL";
        }

        return E_INVALIDARG;
    }

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (core >= g_maxCore)
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"The core (%d) was not valid, the max is %d",
                 core, g_maxCore - 1);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    unsigned int countEventsOnCore;

    if ((S_OK != fnGetCountingEventCount(core, &countEventsOnCore))
        || (0 == countEventsOnCore))
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"No counting events were configured for core %d",
                 core);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    if (countEventsOnCore > size)
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"The buffer should contain at least %d elements",
                 countEventsOnCore);
        g_errorString[clientId] = buffer;
        return E_OUTOFMEMORY;
    }

    //handle case where they set the events individually
    if ((Profiling != g_profilingState[clientId]) &&
        (ProfilingPaused != g_profilingState[clientId]))
    {
        g_errorString[clientId] = L"The profiler was not currently profiling";
        return E_PENDING;
    }

    HRESULT hr = CpuPerfGetAllEventCounts(core,
                                          size,
                                          pCounts);
    return hr;
}


HRESULT fnSetProfileOutputFile(
    /*in*/ const wchar_t* pFileName)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == pFileName)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pFileName was NULL";
        }

        return E_INVALIDARG;
    }

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    HRESULT hr = CpuPerfSetProfileOutputFile(pFileName);
    return hr;
}


// TODO: Yet to be implemented
HRESULT fnSetProfileOutputDirectory(
    /*in*/ const wchar_t* pDirectoryName)
{
    (void)(pDirectoryName); // unused
    // TODO: Yet to be implemented
    return E_FAIL;
}


HRESULT fnGetCurrentTimeMark(CPA_TIME* pTimeMark)
{
    if (NULL == pTimeMark)
    {
        gtUInt32 clientId = helpGetClientId();

        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pTimeMark was NULL";
        }

        return E_INVALIDARG;
    }

    return CpuPerfGetCurrentTimeMark(pTimeMark);
}


#if 0
HRESULT fnGetSampleCount(gtUInt32* pCount)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == pCount)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pCount was NULL";
        }

        return E_INVALIDARG;
    }

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (((Profiling != g_profilingState[clientId]) &&
         (ProfilingPaused != g_profilingState[clientId])))
    {
        g_errorString[clientId] = L"The profiler was not currently profiling";
        return E_ACCESSDENIED;
    }

    HRESULT hr = S_OK;
    hr = CpuPerfGetSampleCount(pCount);
    return hr;
}
#endif // 0


HRESULT fnSetTimerConfiguration(
    /*in*/ gtUInt64 cpuCoreMask,
    /*in/out*/ unsigned int* puSPeriod)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == puSPeriod)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"puSPeriod was NULL";
        }

        return E_INVALIDARG;
    }

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    // if cpuCoreMask is set to -1, profile all the cores;
    // Set TIMER_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
    bool profileAllCores = false;

    if ((gtUInt64) - 1 == cpuCoreMask)
    {
        profileAllCores = true;
    }

    if (0 == cpuCoreMask)
    {
        g_errorString[clientId] = L"cpuCoreMask must be greater than 0";
        return E_INVALIDARG;
    }

    if ((false == profileAllCores) && (cpuCoreMask > g_SystemCoreMask))
    {
        wchar_t num[19]; //16 bytes + 0x + \0
        wchar_t buffer[75];

        swprintf(num, 19, L"0x%llx", cpuCoreMask);
        swprintf(buffer, 75,
                 L"The cpuCoreMask (%s) exceeded the maximum mask of 0x%llx",
                 num, g_SystemCoreMask);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    hr = CpuPerfSetTimerConfiguration(cpuCoreMask,
                                      puSPeriod,
                                      profileAllCores);
    return hr;
}


HRESULT fnSetTimerConfiguration(
    /*in/out*/ unsigned int* puSPeriod,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == puSPeriod)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"puSPeriod was NULL";
        }

        return E_INVALIDARG;
    }

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if ((0 == cpuCoreMaskSize) && (NULL != pCpuCoreMask))
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pCpuCoreMask was not NULL";
        }

        return E_INVALIDARG;
    }

    if ((0 != cpuCoreMaskSize) && (NULL == pCpuCoreMask))
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"cpuCoreMaskSize was not 0";
        }

        return E_INVALIDARG;
    }

    bool profileAllCores = false;

    // if cpuCoreMaskSize is 0, profile all the cores;
    // Set TIMER_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
    if ((NULL == pCpuCoreMask) && (0 == cpuCoreMaskSize))
    {
        profileAllCores = true;
    }

    HRESULT hr = S_OK;

    if (false == profileAllCores)
    {
        hr = validateCoreMaskArray(clientId,
                                   pCpuCoreMask,
                                   cpuCoreMaskSize);

        if (S_OK != hr)
        {
            return hr;
        }
    }

    hr = CpuPerfSetTimerConfiguration(puSPeriod,
                                      pCpuCoreMask,
                                      cpuCoreMaskSize,
                                      profileAllCores);
    return hr;
}



#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
HRESULT fnSetThreadProfileConfiguration(bool isCSS)
{
    gtUInt32 clientId = helpGetClientId();

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    HRESULT hr = S_OK;

    hr = CpuPerfSetThreadProfileConfiguration(isCSS);

    return hr;
} // fnSetThreadProfileConfiguration
#endif


HRESULT fnSetEventConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ gtUInt64 cpuCoreMask)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == pPerformanceEvents)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pPerformanceEvents was NULL";
        }

        return E_INVALIDARG;
    }

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;

    }

    if (0 == count)
    {
        g_errorString[clientId] = L"count was 0";
        return E_INVALIDARG;
    }

    if (g_maxEventConfigs < count)
    {
        wchar_t buffer[75];
        swprintf(buffer, 75, L"The maximum number of EventConfigurations is %d",
                 g_maxEventConfigs);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    // if cpuCoreMask is set to -1, profile all the cores;
    // Set EVENT_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
    bool profileAllCores = false;

    if ((gtUInt64) - 1 == cpuCoreMask)
    {
        profileAllCores = true;
    }

    if (0 == cpuCoreMask)
    {
        g_errorString[clientId] = L"cpuCoreMask must be greater than 0";
        return E_INVALIDARG;
    }

    if ((false == profileAllCores) && (cpuCoreMask > g_SystemCoreMask))
    {
        wchar_t num[19]; //16 bytes + 0x + \0
        wchar_t buffer[75];
        swprintf(num, 19, L"0x%llx", cpuCoreMask);
        swprintf(buffer, 75,
                 L"The cpuCoreMask (%s) exceeded the maximum mask of 0x%llx",
                 num, g_SystemCoreMask);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    HRESULT hr = CpuPerfSetEventConfiguration(pPerformanceEvents,
                                              count,
                                              cpuCoreMask,
                                              profileAllCores);
    return hr;
}


HRESULT fnSetEventConfiguration(
    /*in*/ const EventConfiguration* pPerformanceEvents,
    /*in*/ unsigned int count,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == pPerformanceEvents)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pPerformanceEvents was NULL";
        }

        return E_INVALIDARG;
    }

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;

    }

    if (0 == count)
    {
        g_errorString[clientId] = L"count was 0";
        return E_INVALIDARG;
    }

    if (g_maxEventConfigs < count)
    {
        wchar_t buffer[75];
        swprintf(buffer, 75, L"The maximum number of EventConfigurations is %d",
                 g_maxEventConfigs);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    if ((0 == cpuCoreMaskSize) && (NULL != pCpuCoreMask))
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pCpuCoreMask was not NULL";
        }

        return E_INVALIDARG;
    }

    if ((0 != cpuCoreMaskSize) && (NULL == pCpuCoreMask))
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"cpuCoreMaskSize was not 0";
        }

        return E_INVALIDARG;
    }

    bool profileAllCores = false;

    // if cpuCoreMaskSize is 0, profile all the cores;
    // Set EVENT_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
    if ((NULL == pCpuCoreMask) && (0 == cpuCoreMaskSize))
    {
        profileAllCores = true;
    }

    HRESULT hr = S_OK;

    if (false == profileAllCores)
    {
        hr = validateCoreMaskArray(clientId,
                                   pCpuCoreMask,
                                   cpuCoreMaskSize);

        if (S_OK != hr)
        {
            return hr;
        }
    }

    hr = CpuPerfSetEventConfiguration(pPerformanceEvents,
                                      count,
                                      pCpuCoreMask,
                                      cpuCoreMaskSize,
                                      profileAllCores);
    return hr;
}


HRESULT fnGetIbsAvailable(
    /*out*/ bool* pIsIbsAvailable)
{
    gtUInt32 clientId = helpGetClientId();

    if (NULL == pIsIbsAvailable)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pIsIbsAvailable is NULL";
        }

        return E_INVALIDARG;
    }

    // *pIsIbsAvailable = IsCpuIbsOkay();
#ifdef CODEXL_OSWRAPPERS_AVBL
    osCpuid cpuid;
    *pIsIbsAvailable = cpuid.isIbsAvailable();
#endif // CODEXL_OSWRAPPERS_AVBL

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

    // Check if the linux kernel supports IBS profiling
    if ((access("/sys/devices/ibs_fetch/type", F_OK)) < 0)
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] =
                L"Support for IBS Profiling is not available "
                L"in this Linux kernel version. It is supported "
                L"from Linux kernel version 3.5 onwards.";
        }

        *pIsIbsAvailable = false;
    }
    else
    {
        *pIsIbsAvailable = true;
    }

#endif // AMDT_LINUX_OS

    return S_OK;
}


HRESULT fnSetIbsConfiguration(
    /*in*/ gtUInt64 cpuCoreMask,
    /*in*/ unsigned long fetchPeriod,
    /*in*/ unsigned long opPeriod,
    /*in*/ bool randomizeFetchSamples,
    /*in*/ bool useDispatchOps)
{
    gtUInt32 clientId = helpGetClientId();

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

#ifdef CODEXL_OSWRAPPERS_AVBL
    // bool extOps = false;
    osCpuid cpuid;

    // if (!IsCpuIbsOkay (NULL, &extOps))
    if (! cpuid.isIbsAvailable())
    {
        g_errorString[clientId] = L"The system cannot do Ibs profiling";
        return E_ACCESSDENIED;
    }

#endif // CODEXL_OSWRAPPERS_AVBL

    // if cpuCoreMaskSize is 0, profile all the cores;
    // Set IBS_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
    bool profileAllCores = false;

    if ((gtUInt64) - 1 == cpuCoreMask)
    {
        profileAllCores = true;
    }

    if (0 == cpuCoreMask)
    {
        g_errorString[clientId] = L"cpuCoreMask must be greater than 0";
        return E_INVALIDARG;
    }

    if ((false == profileAllCores) && (cpuCoreMask > g_SystemCoreMask))
    {
        wchar_t num[19];  //16 bytes + 0x + \0
        wchar_t buffer[75];
        swprintf(num, 19, L"0x%llx", cpuCoreMask);
        swprintf(buffer, 75, L"The cpuCoreMask (%s) exceeded the maximum mask of 0x%llx",
                 num, g_SystemCoreMask);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    hr = CpuPerfSetIbsConfiguration(cpuCoreMask,
                                    fetchPeriod,
                                    opPeriod,
                                    randomizeFetchSamples,
                                    useDispatchOps,
                                    profileAllCores);
    return hr;
}


HRESULT fnSetIbsConfiguration(
    /*in*/ unsigned long fetchPeriod,
    /*in*/ unsigned long opPeriod,
    /*in*/ bool randomizeFetchSamples,
    /*in*/ bool useDispatchOps,
    /*in*/ const gtUInt64* pCpuCoreMask,
    /*in*/ unsigned int cpuCoreMaskSize)
{
    gtUInt32 clientId = helpGetClientId();

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

#ifdef CODEXL_OSWRAPPERS_AVBL
    osCpuid cpuid;

    if (! cpuid.isIbsAvailable())
    {
        g_errorString[clientId] = L"The system cannot do Ibs profiling";
        return E_ACCESSDENIED;
    }

#endif // CODEXL_OSWRAPPERS_AVBL

    if ((0 == cpuCoreMaskSize) && (NULL != pCpuCoreMask))
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"pCpuCoreMask was not NULL";
        }

        return E_INVALIDARG;
    }

    if ((0 != cpuCoreMaskSize) && (NULL == pCpuCoreMask))
    {
        if (INVALID_CLIENT != clientId)
        {
            g_errorString[clientId] = L"cpuCoreMaskSize was not 0";
        }

        return E_INVALIDARG;
    }

    bool profileAllCores = false;

    // if cpuCoreMaskSize is 0, profile all the cores;
    // Set IBS_PROPERTIES::ulCoreMaskCount to 0, to profile all cores.
    if ((NULL == pCpuCoreMask) && (0 == cpuCoreMaskSize))
    {
        profileAllCores = true;
    }

    HRESULT hr = S_OK;

    if (false == profileAllCores)
    {
        hr = validateCoreMaskArray(clientId,
                                   pCpuCoreMask,
                                   cpuCoreMaskSize);

        if (S_OK != hr)
        {
            return hr;
        }
    }

    hr = CpuPerfSetIbsConfiguration(fetchPeriod,
                                    opPeriod,
                                    randomizeFetchSamples,
                                    useDispatchOps,
                                    pCpuCoreMask,
                                    cpuCoreMaskSize,
                                    profileAllCores);

    return hr;
}


HRESULT fnSetFilterProcesses(
    /*in*/ unsigned int* pProcessIds,
    /*in*/ unsigned int count,
    /*in*/ bool systemWideProfile,
    /*in*/ bool ignoreChildren)
{
    gtUInt32 clientId = helpGetClientId();

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (NULL == pProcessIds)
    {
        g_errorString[clientId] = L"pProcessIds is NULL";
        return E_INVALIDARG;
    }

    if (0 == count)
    {
        g_errorString[clientId] = L"count is 0";
        return E_INVALIDARG;
    }

    if (MAX_PID_COUNT < count)
    {
        wchar_t buffer[65];
        swprintf(buffer, 65, L"pProcessIds must have less than %d elements",
                 MAX_PID_COUNT);
        g_errorString[clientId] = buffer;
        return E_INVALIDARG;
    }

    HRESULT hr = CpuPerfSetFilterProcesses(pProcessIds,
                                           count,
                                           systemWideProfile,
                                           ignoreChildren);
    return hr;
}


HRESULT fnSetCallStackSampling(
    /*in*/ unsigned int* pProcessIds,
    /*in*/ unsigned int count,
    /*in*/ unsigned int unwindLevel,
    /*in*/ unsigned int samplePeriod,
    /*in*/ CpuProfileCssScope scope,
    /*in*/ bool captureVirtualStack)
{
    gtUInt32 clientId = helpGetClientId();

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    if (NULL == pProcessIds)
    {
        g_errorString[clientId] = L"pProcessIds was NULL";
        return E_INVALIDARG;
    }

    if (0 == count)
    {
        g_errorString[clientId] = L"count was 0";
        return E_INVALIDARG;
    }

    if (0 == unwindLevel)
    {
        g_errorString[clientId] = L"unwindLevel was 0";
        return E_INVALIDARG;
    }

    if (0 == samplePeriod)
    {
        g_errorString[clientId] = L"samplePeriod was 0";
        return E_INVALIDARG;
    }

    if (CP_CSS_SCOPE_ALL < static_cast<unsigned int>(scope))
    {
        g_errorString[clientId] = L"scope is invalid";
        return E_INVALIDARG;
    }

    if (unwindLevel > CP_CSS_MAX_UNWIND_DEPTH)
    {
        g_errorString[clientId] = L"unwindLevel must be less than ";
        g_errorString[clientId].appendUnsignedIntNumber(CP_CSS_MAX_UNWIND_DEPTH + 1);
        return E_INVALIDARG;
    }

    HRESULT hr = CpuPerfSetCallStackSampling(pProcessIds,
                                             count,
                                             unwindLevel,
                                             samplePeriod,
                                             scope,
                                             captureVirtualStack);
    return hr;
}


HRESULT fnClearConfigurations()
{
    HRESULT retVal = S_OK;
    gtUInt32 clientId = helpGetClientId();

    if (INVALID_CLIENT == clientId)
    {
        g_invalidClientErr = L"fnEnableProfiling was not called";
        return E_ACCESSDENIED;
    }

    retVal = CpuPerfClearConfigurations(clientId);
    return retVal;
}


HRESULT fnEnableCPUUtilization(unsigned int utilization_interval,
                               unsigned int monitorFlag)
{
    HRESULT hr = S_OK;

    hr = CpuPerfEnableCPUUtilization(utilization_interval,
                                     monitorFlag);
    return hr;
}


HRESULT fnAddCPUUtilizationProcessId(unsigned pid)
{
    HRESULT hr = S_OK;

    hr = CpuPerfAddCPUUtilizationProcessId(pid);

    return hr;
}


HRESULT fnWriteRunInfo(const wchar_t* pRIFilePath, const RunInfo* pRunInfo)
{
    if (NULL == pRIFilePath || NULL == pRunInfo)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    hr = fnWriteRIFile(pRIFilePath, pRunInfo);

    return hr;
}
