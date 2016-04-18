//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTThreadProfileAPI.cpp
///
//==================================================================================

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// System headers
#include <wchar.h>

// Project headers
#include <AMDTThreadProfileApi.h>
#include <tpInternalDataTypes.h>
#include <tpCollect.h>
#include <tpTranslate.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <tpEtlTranslate.h>

    #pragma comment(lib, "tdh.lib")
    // #pragma comment(lib, "ws2_32.lib")  // For ntohs function - unused

    static void tpInitializeGlobals();
    static void tpClearGlobals();

#else
    #include <tpPerfTranslate.h>

    #pragma GCC diagnostic ignored "-Wwrite-strings"

    static void __attribute__((constructor)) tpInitializeGlobals(void);
    static void __attribute__((destructor)) tpClearGlobals(void);
#endif

//
// Globals
//

tpCollect g_tpCollect;
char* g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_MAXIMUMWAITREASON + 1];

//
// Helper functions
//

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

#ifdef _MANAGED
    #pragma managed(push, off)
#endif

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
    GT_UNREFERENCED_PARAMETER(hModule);
    GT_UNREFERENCED_PARAMETER(lpReserved);

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            tpInitializeGlobals();
            g_tpCollect.tpInitialize();
            break;
        }

        case DLL_PROCESS_DETACH:
        {
            tpClearGlobals();
            g_tpCollect.tpClear();
            break;
        }
    }

    return TRUE;
} // DllMain

#ifdef _MANAGED
    #pragma managed(pop)
#endif

#endif // WINDOWS

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    static void tpInitializeGlobals()
#else
    static void __attribute__((constructor)) tpInitializeGlobals(void)
#endif
{
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_EXECUTIVE] = TP_STR_ThreadWaitReasonExecutive;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_FREEPAGE] = TP_STR_ThreadWaitReasonFreepage;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_PAGEIN] = TP_STR_ThreadWaitReasonPageIn;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_POOLALLOCATION] = TP_STR_ThreadWaitReasonPoolAllocation;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_DELAYEXECUTION] = TP_STR_ThreadWaitReasonDelayExecution;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_SUSPENDED] = TP_STR_ThreadWaitReasonSuspended;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_USERREQUEST] = TP_STR_ThreadWaitReasonUserRequest;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WREXECUTIVE] = TP_STR_ThreadWaitReasonWrExecutive;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRFREEPAGE] = TP_STR_ThreadWaitReasonWrFreePage;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRPAGEIN] = TP_STR_ThreadWaitReasonWrPageIn;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRPOOLALLOCATION] = TP_STR_ThreadWaitReasonWrPoolAllocation;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRDELAYEXECUTION] = TP_STR_ThreadWaitReasonWrDelayExecution;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRSUSPENDED] = TP_STR_ThreadWaitReasonWrSuspended;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRUSERREQUEST] = TP_STR_ThreadWaitReasonWrUserRequest;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRSPARE0] = TP_STR_ThreadWaitReasonWrSpare0;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRQUEUE] = TP_STR_ThreadWaitReasonWrQueue;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRLPCRECEIVE] = TP_STR_ThreadWaitReasonWrLPCReceive;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRLPCREPLY] = TP_STR_ThreadWaitReasonWrLPCReply;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRVIRTUALMEMORY] = TP_STR_ThreadWaitReasonWrVirtualMemory;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRPAGEOUT] = TP_STR_ThreadWaitReasonWrPageOut;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRRENDEZVOUS] = TP_STR_ThreadWaitReasonWrRendezvous;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRKEYEDEVENT] = TP_STR_ThreadWaitReasonWrKeyedEvent;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRTERMINATED] = TP_STR_ThreadWaitReasonWrTerminated;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRPROCESSINSWAP] = TP_STR_ThreadWaitReasonWrProcessInSwap;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRCPURATECONTROL] = TP_STR_ThreadWaitReasonWrCpuRateControl;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRCALLOUTSTACK] = TP_STR_ThreadWaitReasonWrCalloutStack;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRKERNEL] = TP_STR_ThreadWaitReasonWrKernel;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRRESOURCE] = TP_STR_ThreadWaitReasonWrResource;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRPUSHLOCK] = TP_STR_ThreadWaitReasonWrPushLock;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRMUTEX] = TP_STR_ThreadWaitReasonWrMutex;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRQUANTUMEND] = TP_STR_ThreadWaitReasonWrQuantumEnd;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRDISPATCHINT] = TP_STR_ThreadWaitReasonWrDispatchInt;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRPREEMPTED] = TP_STR_ThreadWaitReasonWrPreempted;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRYIELDEXECUTION] = TP_STR_ThreadWaitReasonWrYieldExecution;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRFASTMUTEX] = TP_STR_ThreadWaitReasonWrFastMutex;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRGUARDEDMUTEX] = TP_STR_ThreadWaitReasonWrGuardedMutex;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRRUNDOWN] = TP_STR_ThreadWaitReasonWrRunDown;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRALERTBYTHREADID] = TP_STR_ThreadWaitReasonWrAlertByThreadid;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_WRDEFERREDPREEMPT] = TP_STR_ThreadWaitReasonWrDeferredPreempt;
    g_ppTpWaitReason[AMDT_THREAD_WAIT_REASON_MAXIMUMWAITREASON] = TP_STR_ThreadWaitReasonmaximumwaitreason;

    return;
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    static void tpClearGlobals()
#else
    static void __attribute__((destructor)) tpClearGlobals(void)
#endif
{
    return;
}

//
//  APIs
//

// AMDTIsThreadProfileAvailable
//
AMDTResult AMDTIsThreadProfileAvailable(bool* pIsAvailable)
{
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;

    if (NULL != pIsAvailable)
    {
        // TODO:
        // On Windows,
        // - return true only if CodeXL is launched with admin privileges, otherwise return AMDT_ERROR_ACCESSDENIED
        // On Linux,
        // - return true only if the CodeXL is launched with root access, otherwise return AMDT_ERROR_ACCESSDENIED
        // - Also, find the oldest kernel version that supports sampling based on CONTEXT_SWITCH event.
        //   For kernel versions older than that version, return AMDT_ERROR_NOTAVAILABLE
        *pIsAvailable = true;
        retVal = AMDT_STATUS_OK;
    }

    return retVal;
} // AMDTIsThreadProfileAvailable


// AMDTSetThreadProfileConfiguration
//
AMDTResult AMDTSetThreadProfileConfiguration(AMDTUInt32 flags, const char* pFilePath)
{
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;

    if (NULL != pFilePath)
    {
        retVal = g_tpCollect.tpSetThreadProfileConfiguration(flags, pFilePath);
    }

    return retVal;
} // AMDTSetThreadProfileConfiguration


// AMDTStartThreadProfile
//
AMDTResult AMDTStartThreadProfile()
{
    AMDTResult retVal = AMDT_STATUS_OK;

    retVal = g_tpCollect.tpStartThreadProfile();

    return retVal;
} // AMDTStartThreadProfile


// AMDTStopThreadProfile
//
AMDTResult AMDTStopThreadProfile()
{
    AMDTResult retVal = AMDT_STATUS_OK;

    retVal = g_tpCollect.tpStopThreadProfile();

    return retVal;
} // AMDTStopThreadProfile


// AMDTOpenThreadProfile
//
AMDTResult AMDTOpenThreadProfile(const char* pPath,
                                 AMDTThreadProfileDataHandle* pReaderHandle)
{
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;

    if ((NULL != pReaderHandle) && (NULL != pPath))
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        tpTranslate* pTranslate = new tpEtlTranslate;
#else
        tpTranslate* pTranslate = new tpPerfTranslate;
#endif

        // TODO: if tpTranslate::Initialize() does not validate the path, do it here
        retVal = pTranslate->Initialize(pPath);

        if (AMDT_STATUS_OK == retVal)
        {
            AMDTThreadProfileDataHandle pHandle = static_cast<AMDTThreadProfileDataHandle>(pTranslate);

            *pReaderHandle = pHandle;
        }
    }

    return retVal;
} // AMDTOpenThreadProfile


// AMDTProcessThreadProfileData
//
AMDTResult AMDTProcessThreadProfileData(AMDTThreadProfileDataHandle readerHandle)
{
    AMDTResult retVal = AMDT_ERROR_HANDLE;
    tpTranslate* pTranslate = static_cast<tpTranslate*>(readerHandle);

    if (NULL != pTranslate)
    {
        // first process all the process/image/thread start & end records
        retVal = pTranslate->OpenThreadProfileData(true);

        if (AMDT_STATUS_OK == retVal)
        {
            retVal = pTranslate->ProcessThreadProfileData();
        }
    }

    return retVal;
} // AMDTProcessThreadProfileData


// AMDTSetFilterProcesses
//
// This also adds the child processes of the given PIDs
//
AMDTResult AMDTSetFilterProcesses(AMDTThreadProfileDataHandle readerHandle,
                                  AMDTUInt32 count,
                                  AMDTProcessId* pProcessIds)
{
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;

    if ((NULL != pProcessIds) && (count > 0))
    {
        retVal = AMDT_ERROR_HANDLE;

        tpTranslate* pTranslate = static_cast<tpTranslate*>(readerHandle);

        if (NULL != pTranslate)
        {
            for (AMDTUInt32 i = 0; i < count; i++)
            {
                pTranslate->AddInterestingPid(pProcessIds[i]);
            }

            // disable processing CS records for all pid/tid
            pTranslate->DisableHandleAllPids();

            retVal = AMDT_STATUS_OK;
        }
    }

    return retVal;
} // AMDTSetFilterProcesses


// AMDTGetNumOfProcessors
//
AMDTResult AMDTGetNumOfProcessors(AMDTThreadProfileDataHandle readerHandle,
                                  AMDTUInt32* pNbrProcessors)
{
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;

    if (NULL != pNbrProcessors)
    {
        retVal = AMDT_ERROR_HANDLE;

        tpTranslate* pTranslate = static_cast<tpTranslate*>(readerHandle);

        if (NULL != pTranslate)
        {
            *pNbrProcessors = pTranslate->GetNumberOfProcessors();
            retVal = AMDT_STATUS_OK;
        }
    }

    return retVal;
} //AMDTGetNumOfProcessors


// AMDTGetProcessIds
//
AMDTResult AMDTGetProcessIds(AMDTThreadProfileDataHandle readerHandle,
                             AMDTUInt32* pNbrProcesses,
                             AMDTUInt32 size,
                             AMDTProcessId* pProcesses)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    if ((NULL == pNbrProcesses) && (NULL == pProcesses))
    {
        retVal = AMDT_ERROR_INVALIDARG;
    }

    tpTranslate* pTranslate = static_cast<tpTranslate*>(readerHandle);

    if (NULL == pTranslate)
    {
        retVal = AMDT_ERROR_HANDLE;
    }

    if (AMDT_STATUS_OK == retVal)
    {
        AMDTUInt32 nbrProcesses = 0;

        nbrProcesses = pTranslate->GetNumberOfProcesses();

        if (NULL != pNbrProcesses)
        {
            *pNbrProcesses = nbrProcesses;
        }

        if ((size > 0) && (NULL != pProcesses))
        {
            retVal = pTranslate->GetProcessIds(size, pProcesses);
        }
        else if (NULL == pNbrProcesses)
        {
            retVal = AMDT_ERROR_INVALIDARG;
        }
    }

    return retVal;
} // AMDTGetProcessIds


// AMDTGetProcessData
//
AMDTResult AMDTGetProcessData(AMDTThreadProfileDataHandle readerHandle,
                              AMDTProcessId pid,
                              AMDTProcessData* pProcessData)
{
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;

    if (NULL != pProcessData)
    {
        retVal = AMDT_ERROR_HANDLE;

        tpTranslate* pTranslate = static_cast<tpTranslate*>(readerHandle);

        if (NULL != pTranslate)
        {
            retVal = pTranslate->GetProcessData(pid, *pProcessData);
        }
    }

    return retVal;
} // AMDTGetProcessData


// AMDTGetThreadIds
//
AMDTResult AMDTGetThreadIds(AMDTThreadProfileDataHandle readerHandle,
                            AMDTProcessId pid,
                            AMDTUInt32* pNbrThreads,
                            AMDTUInt32 size,
                            AMDTThreadId* pThreads)
{
    AMDTResult retVal = AMDT_STATUS_OK;

    if ((NULL == pNbrThreads) && (NULL == pThreads))
    {
        retVal = AMDT_ERROR_INVALIDARG;
    }

    tpTranslate* pTranslate = static_cast<tpTranslate*>(readerHandle);

    if (NULL == pTranslate)
    {
        retVal = AMDT_ERROR_HANDLE;
    }

    AMDTUInt32 nbrThreads = 0;

    if (AMDT_STATUS_OK == retVal)
    {
        if (TP_ALL_PIDS == pid)
        {
            nbrThreads = pTranslate->GetNumberOfThreads();
        }
        else if (pTranslate->IsValidProcess(pid))
        {
            nbrThreads = pTranslate->GetNumberOfThreads(pid);
        }
        else
        {
            retVal = AMDT_ERROR_INVALIDARG;
        }
    }

    if (AMDT_STATUS_OK == retVal)
    {
        if (NULL != pNbrThreads)
        {
            *pNbrThreads = nbrThreads;
        }

        if ((size > 0) && (NULL != pThreads))
        {
            retVal = pTranslate->GetThreadIds(pid, nbrThreads, pThreads);
        }
        else if (NULL == pNbrThreads)
        {
            retVal = AMDT_ERROR_INVALIDARG;
        }
    }

    return retVal;
} // AMDTGetThreadIds


// AMDTGetThreadData
//
AMDTResult AMDTGetThreadData(AMDTThreadProfileDataHandle readerHandle,
                             AMDTThreadId tid,
                             AMDTThreadData* pThreadData)
{
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;

    if (NULL != pThreadData)
    {
        retVal = AMDT_ERROR_HANDLE;

        tpTranslate* pTranslate = static_cast<tpTranslate*>(readerHandle);

        if (NULL != pTranslate)
        {
            retVal = pTranslate->GetThreadData(tid, *pThreadData);
        }
    }

    return retVal;
} // AMDTGetThreadData


// AMDTGetThreadSampleData
//
AMDTResult AMDTGetThreadSampleData(AMDTThreadProfileDataHandle readerHandle,
                                   AMDTThreadId tid,
                                   AMDTUInt32* pNbrRecords,
                                   AMDTThreadSample** ppThreaSampledData)
{
    AMDTResult retVal = AMDT_ERROR_HANDLE;
    tpTranslate* pTranslate = static_cast<tpTranslate*>(readerHandle);

    if (NULL != pTranslate)
    {
        if (pTranslate->IsPassOneCompleted())
        {
            if (!pTranslate->IsallCSRecordsProcessed())
            {
                // Process the CSWITCH and CALLSTACK records
                retVal = pTranslate->OpenThreadProfileData(false);

                if (AMDT_STATUS_OK == retVal)
                {
                    retVal = pTranslate->ProcessThreadProfileData();
                }
            }

            AMDTUInt32 nbrRecords = 0;
            AMDTThreadSample* pThreadSampleData = NULL;
            retVal = pTranslate->GetThreadSampleData(tid, &nbrRecords, &pThreadSampleData);

            if (AMDT_STATUS_OK == retVal)
            {
                if (NULL != pNbrRecords)
                {
                    *pNbrRecords = nbrRecords;
                }

                if (NULL != ppThreaSampledData)
                {
                    *ppThreaSampledData = pThreadSampleData;
                }
            }
        }
        else
        {
            fprintf(stderr, "AMDTProcessThreadProfileData() was not called..\n");
        }
    }

    return retVal;
} // AMDTGetThreadSampleData


// AMDTCloseThreadProfile
//
AMDTResult AMDTCloseThreadProfile(AMDTThreadProfileDataHandle readerHandle)
{
    AMDTResult retVal = AMDT_ERROR_HANDLE;

    tpTranslate* pTranslate = static_cast<tpTranslate*>(readerHandle);

    if (NULL != pTranslate)
    {
        retVal = pTranslate->CloseThreadProfileData();
        delete pTranslate;
    }

    return retVal;
} // AMDTCloseThreadProfile


// AMDTSetSymbolSearchPath
AMDTResult AMDTSetSymbolSearchPath(AMDTThreadProfileDataHandle readerHandle,
                                   const char* pSearchPath,
                                   const char* pServerList,
                                   const char* pCachePath)
{
    AMDTResult retVal = AMDT_ERROR_HANDLE;

    tpTranslate* pTranslate = static_cast<tpTranslate*>(readerHandle);

    if (NULL != pTranslate)
    {
        retVal = pTranslate->SetSymbolSearchPath(pSearchPath,
                                                 pServerList,
                                                 pCachePath);
    }

    return retVal;
} // AMDTSetSymbolSearchPath


AMDTResult AMDTGetFunctionName(AMDTThreadProfileDataHandle readerHandle,
                               AMDTProcessId pid,
                               AMDTUInt64 pc,
                               char** ppFuncName)
{
    AMDTResult retVal = AMDT_ERROR_HANDLE;
    tpTranslate* pTranslate = static_cast<tpTranslate*>(readerHandle);

    if (NULL != pTranslate)
    {
        retVal = AMDT_ERROR_INVALIDARG;

        if (NULL != ppFuncName)
        {
            char* pName = NULL;
            retVal = pTranslate->GetFunctionName(pid, pc, &pName);
            *ppFuncName = pName;
        }
    }

    return retVal;
} // AMDTGetFunctionName


AMDTResult AMDTGetThreadStateString(AMDTThreadState threadState, char** ppString)
{
    char* pStr = NULL;
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;

    if (NULL != ppString)
    {
        retVal = AMDT_STATUS_OK;

        switch (threadState)
        {
            case AMDT_THREAD_STATE_INITIALIZED:
                pStr = TP_STR_ThreadStateInitialized;
                break;

            case AMDT_THREAD_STATE_READY:
                pStr = TP_STR_ThreadStateReady;
                break;

            case AMDT_THREAD_STATE_RUNNING:
                pStr = TP_STR_ThreadStateRunning;
                break;

            case AMDT_THREAD_STATE_STANDBY:
                pStr = TP_STR_ThreadStateStandby;
                break;

            case AMDT_THREAD_STATE_TERMINATED:
                pStr = TP_STR_ThreadStateTerminated;
                break;

            case AMDT_THREAD_STATE_WAITING:
                pStr = TP_STR_ThreadStateWaiting;
                break;

            case AMDT_THREAD_STATE_TRANSITION:
                pStr = TP_STR_ThreadStateTransition;
                break;

            case AMDT_THREAD_STATE_DEFERREDREADY:
                pStr = TP_STR_ThreadStateDeferredReady;
                break;

            case AMDT_THREAD_STATE_GATEWAIT:
                pStr = TP_STR_ThreadStateGatewait;
                break;

            case AMDT_THREAD_STATE_MAXIMUM:
            default:
                pStr = TP_STR_Unknown;
                break;
        }
    }

    if (AMDT_STATUS_OK == retVal)
    {
        *ppString = pStr;
    }

    return retVal;
} // AMDTGetThreadStateString


AMDTResult AMDTGetThreadWaitModeString(AMDTThreadWaitMode waitMode, char** ppString)
{
    char* pStr = NULL;
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;

    if (NULL != ppString)
    {
        retVal = AMDT_STATUS_OK;

        switch (waitMode)
        {
            case AMDT_THREAD_WAIT_MODE_KERNEL:
                pStr = TP_STR_ThreadWaitModeKernel;
                break;

            case AMDT_THREAD_WAIT_MODE_USER:
                pStr = TP_STR_ThreadWaitModeUser;
                break;

            default:
                pStr = TP_STR_Unknown;
                break;
        }
    }

    if (AMDT_STATUS_OK == retVal)
    {
        *ppString = pStr;
    }

    return retVal;
} // AMDTGetThreadWaitModeString


AMDTResult AMDTGetThreadWaitReasonString(AMDTThreadWaitReason waitReason, char** ppString)
{
    char* pStr = NULL;
    AMDTResult retVal = AMDT_ERROR_INVALIDARG;

    if ((NULL != ppString)
        && (waitReason >= AMDT_THREAD_WAIT_REASON_EXECUTIVE)
        && (waitReason < AMDT_THREAD_WAIT_REASON_MAXIMUMWAITREASON))
    {
        pStr = g_ppTpWaitReason[waitReason];

        retVal = AMDT_STATUS_OK;
    }

    if (AMDT_STATUS_OK == retVal)
    {
        *ppString = pStr;
    }

    return retVal;
} // AMDTGetThreadWaitReasonString
