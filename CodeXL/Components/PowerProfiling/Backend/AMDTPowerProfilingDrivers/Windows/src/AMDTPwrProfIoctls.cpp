//=====================================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc. All rights reserved.
//
/// \file $File: $
/// \version $Revision: $
/// \brief
//
//=====================================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=====================================================================
#include <ntifs.h>
#include <ntstrsafe.h>
#include <winerror.h>
#include <AMDTPwrProfDriver.h>
#include <AMDTPwrProfInternal.h>
#include <AMDTPwrProfIoctls.h>
#include <AMDTHelpers.h>
#include <AMDTSmu7Interface.h>
#include <AMDTSmu8Interface.h>
#include <AMDTCommonConfig.h>
#include <AMDTHwAccessInterface.h>
#include <AMDTSharedObjPath.h>
#include <AMDTCounterAccessInterface.h>
#include <WinDriverUtils\Include\Atomic.hpp>

extern PPWRPROF_DEV_EXTENSION gpPwrDevExt;
CoreData* g_pCoreCfg = NULL;

// Get the hardcoded version number of the driver
#pragma code_seg()

uint8* pSharedBuffer = NULL;
MDL* mdlPtr = NULL;
HANDLE sharedHandle = 0;
PVOID sharedBaseAddr = NULL;

// DeleteSharedMap:Free a shared memory area between application and driver mode for sharing pause info.
NTSTATUS DeleteSharedMap()
{
    NTSTATUS status = STATUS_SUCCESS;

    if (NULL != pSharedBuffer)
    {
        if (NULL != mdlPtr)
        {
            MmUnlockPages(mdlPtr);
            IoFreeMdl(mdlPtr);
            mdlPtr = NULL;
        }

        if (NULL != sharedBaseAddr)
        {
            status = ZwUnmapViewOfSection(NtCurrentProcess(), sharedBaseAddr);
            sharedBaseAddr = NULL;
        }

        if (0 != sharedHandle)
        {
            ZwClose(sharedHandle);
            sharedHandle = 0;
        }
    }

    return status;
}

// CreateSharedBuffer: Open a shared memory area between application and driver mode for sharing pause state
NTSTATUS CreateSharedBuffer(uint32 envVariable)
{
    // Convert the constant string to the UNICODE_STRING.
    UNICODE_STRING usSharedMemoryObjName;

    DRVPRINT("calling env 0x%x", envVariable);
    RtlInitUnicodeString(&usSharedMemoryObjName, PWRPROF_SHARED_OBJ_BASE);

    OBJECT_ATTRIBUTES objAttrs;

    // Initialize the object attributes structure.
    InitializeObjectAttributes(&objAttrs, &usSharedMemoryObjName,
                               (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE), (HANDLE)NULL,
                               (PSECURITY_DESCRIPTOR)NULL);
    // No error code returned according to DDK doc.

    HANDLE sectionHandle;
    NTSTATUS status = ZwOpenSection(&sectionHandle, (SECTION_MAP_READ | SECTION_MAP_WRITE), &objAttrs);

    if (STATUS_SUCCESS == status)
    {
        DRVPRINT("ZwOpenSection success");
        sharedHandle = sectionHandle;

        PVOID pBaseAddress = NULL;
        SIZE_T viewSize = 0;
        status = ZwMapViewOfSection(sectionHandle,          // section handle
                                    NtCurrentProcess(),     // current process
                                    &pBaseAddress,          // virtual based address
                                    0,                      // Zero bits
                                    PWRPROF_SHARED_BUFFER_SIZE,
                                    0,                      // optional
                                    &viewSize,              // How much to map
                                    ViewShare,              // Inherit disposition
                                    0,                      // ALlocation Type
                                    PAGE_READWRITE);        // protection

        if (STATUS_SUCCESS == status)
        {
            DRVPRINT("ZwMapViewOfSection success commit size 0x%x", viewSize);
            //save the base address to free it later
            sharedBaseAddr = pBaseAddress;
            //Allocate the length of ViewSize in bytes
            PMDL pMdl = IoAllocateMdl(pBaseAddress, static_cast<ULONG>(viewSize), FALSE, TRUE, NULL);

            if (NULL != pMdl)
            {
                DRVPRINT("pMdl success");

                __try
                {
                    MmProbeAndLockPages(pMdl, KernelMode, IoModifyAccess);
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                    status = STATUS_NO_MEMORY;
                }

                if (STATUS_SUCCESS == status)
                {
                    DRVPRINT("pSharedBuffer getting");
                    pSharedBuffer = static_cast<uint8*>(MmGetSystemAddressForMdlSafe(pMdl, HighPagePriority));

                    if (NULL != pSharedBuffer)
                    {
                        if (0x0 == envVariable)
                        {
                            DRVPRINT("pSharedBuffer success RtlSecureZeroMemory...");
                            RtlSecureZeroMemory(pSharedBuffer, PWRPROF_SHARED_BUFFER_SIZE);
                        }

                        mdlPtr = pMdl;
                    }
                    else
                    {
                        MmUnlockPages(pMdl);
                        IoFreeMdl(pMdl);
                        ZwUnmapViewOfSection(NtCurrentProcess(), sharedBaseAddr);
                        ZwClose(sharedHandle);
                        status = STATUS_NO_MEMORY;
                    }
                }
                else
                {
                    IoFreeMdl(pMdl);
                    ZwUnmapViewOfSection(NtCurrentProcess(), sharedBaseAddr);
                    ZwClose(sharedHandle);
                }
            }
            else
            {
                ZwUnmapViewOfSection(NtCurrentProcess(), sharedBaseAddr);
                ZwClose(sharedHandle);
                status = STATUS_NO_MEMORY;
            }
        }
        else
        {
            ZwClose(sharedHandle);
        }
    }

    return status;
}

//IoctlGetVersionHandler: Power profiling driver version
//Verion data composed in 64 bits in following manner
//0:31 = power profile driver version
//32:48 = Pcore build version
//48:56 = pcore minor version
//56:64 = pcore major version
NTSTATUS IoctlGetVersionHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                IN PIRP pIrp,
                                IN PIO_STACK_LOCATION pIrpStack)
{
    NTSTATUS ret = STATUS_SUCCESS;
    PULONG64 pulVersion = NULL;
    ULONG pcoreMajor = 0;
    ULONG pcoreMinor = 0;
    ULONG pcoreBuild = 0;

    DRVPRINT("IoctlGetVersionHandler called");

    //Supress warning
    (void)pDevExt;

    pIrp->IoStatus.Information  = 0;

    if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG64))
    {
        ret = STATUS_BUFFER_TOO_SMALL;
    }

    if (STATUS_SUCCESS == ret)
    {
        pulVersion = (PULONG64)pIrp->AssociatedIrp.SystemBuffer;

        if (S_OK != PcoreVersion(&pcoreMajor, &pcoreMinor, &pcoreBuild))
        {
            ret = STATUS_DEVICE_NOT_READY;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        *pulVersion = DRIVER_VERSION
                      | ((ULONG64)pcoreMajor << 56)
                      | ((ULONG64)pcoreMinor << 48)
                      | ((ULONG64)pcoreBuild << 32)
                      | ((ULONG64)PWRPROF_MAJOR_VERSION << 28)
                      | ((ULONG64)PWRPROF_MINOR_VERSION << 24)
                      | ((ULONG64)PWRPROF_BUILD_VERSION << 20);

        pIrp->IoStatus.Information = sizeof(ULONG64);
    }

    return ret;
}


//IoctlRegisterClient: Register a profile client with the AMDTPwrProf personality driver
NTSTATUS IoctlRegisterClient(IN PPWRPROF_DEV_EXTENSION pDevExt,
                             IN PIRP pIrp,
                             IN PIO_STACK_LOCATION pIrpStack)
{
    NTSTATUS ret = STATUS_SUCCESS;
    ULONG regId = 0;
    PULONG pulRegId = NULL;

    pIrp->IoStatus.Information  = 0;

    //  Verifying parameters
    if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG))
    {
        ret = STATUS_BUFFER_TOO_SMALL;
    }

    if (STATUS_SUCCESS == ret)
    {
        pulRegId = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

        //check each possible client for availability
        for (regId = 0; regId < MAX_CLIENT_COUNT; regId++)
        {
            // if the client is available (0 == the validClient field),
            // immediately grab the client by putting 1 into validClient
            if (SYNCH_AVAILABLE == InterlockedCompareExchange((LONG*) & (pDevExt->m_pClient[regId].m_validClient),
                                                              CLIENT_REGISTERED, SYNCH_AVAILABLE))
            {
                //If the pcore registration failed
                if (S_OK != PcoreRegister(&(pDevExt->m_pClient[regId].m_osClientCfg.m_pcoreReg), NULL))
                {
                    //return the client to an available state
                    InterlockedExchange((LONG*) & (pDevExt->m_pClient[regId].m_validClient),
                                        SYNCH_AVAILABLE);

                    ret = STATUS_ACCESS_DENIED;
                    break;
                }

                pDevExt->m_pClient[regId].m_osClientCfg.m_userFileObj = pIrpStack->FileObject;
                *pulRegId = regId;
                break;
            }
        }
    }

    // If no clients were available, return an error
    if ((STATUS_SUCCESS == ret) && (MAX_CLIENT_COUNT == regId))
    {
        ret = STATUS_ACCESS_DENIED;
    }

    pIrp->IoStatus.Information = sizeof(ULONG);

    return ret;
}

//SetOutputFile : Setup output file  for storing raw data samples
bool SetOutputFile(ClientData* pClient, const wchar_t* pRawFilePath, ULONG prdLength)
{
    (void)prdLength;
    (void)pRawFilePath;
    (void)pClient;

    ASSERT(NULL != pRawFilePath && 0UL != prdLength && NULL != pClient);

    bool ret = false;
    return ret;
}

// PwrCoreInitialize: DPC per core to initialize profile config
static VOID PwrCoreInitialize(PKDPC pDpc, PVOID context, PVOID synchEvent, PVOID arg2)
{
    (void)pDpc;
    (void)context;
    (void) arg2;
    InitializeGenericCounterAccess((uint32)KeGetCurrentProcessorNumberEx(NULL));
    static_cast<EventNotifier*>(synchEvent)->Notify();
}

// PwrSetTargetCoreDpc: Set DPC
bool PwrSetTargetCoreDpc(KDPC& dpc, uint32 core)
{
    PROCESSOR_NUMBER procNumber;

    bool succeeded = (STATUS_SUCCESS == KeGetProcessorNumberFromIndex(static_cast<ULONG>(core), &procNumber) &&
                      STATUS_SUCCESS == KeSetTargetProcessorDpcEx(&dpc, &procNumber));

    if (!succeeded)
    {
        DRVPRINT("Invalid target core number!");
    }

    return succeeded;
}

// PwrExecuteDpc: Execute DPC
bool PwrExecuteDpc(uint core, PKDEFERRED_ROUTINE routine, void* context)
{
    bool succeeded = false;
    KDPC dpc;

    KeInitializeDpc(&dpc, routine, context);

    if (PwrSetTargetCoreDpc(dpc, core))
    {
        KeSetImportanceDpc(&dpc, HighImportance);

        EventNotifier synchEvent;

        if (TRUE == KeInsertQueueDpc(&dpc, &synchEvent, NULL))
        {
            synchEvent.Wait();

            succeeded = true;
        }
        else
        {
            DRVPRINT("Failed to enqueue DPC!");
        }
    }

    return succeeded;
}

//IoctlAddProfConfigsHandler: Set profile configurations
NTSTATUS IoctlAddProfConfigsHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                    IN PIRP pIrp,
                                    IN PIO_STACK_LOCATION pIrpStack)
{
    NTSTATUS ret = STATUS_SUCCESS;
    ProfileConfig* pSrcCfg = NULL;
    PPROF_CONFIGS pProfConfigs = NULL;
    ClientData* pClient = NULL;
    uint16 confCount = 0;
    uint64 coreMask = 0;
    uint16 coreId = 0;
    uint64 coreCounterMask = 0;

    DRVPRINT("IoctlAddProfConfigsHandler called");

    pIrp->IoStatus.Information  = 0;

    //  Verifying parameters
    if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(PROF_CONFIGS))
    {
        DRVPRINT("ERROR IoctlAddProfConfigsHandler, input wrong size");
        ret = STATUS_INFO_LENGTH_MISMATCH;
    }

    if (STATUS_SUCCESS == ret)
    {
        if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(PROF_CONFIGS))
        {
            DRVPRINT("ERROR IoctlAddProfConfigsHandler, output too small");
            ret = STATUS_BUFFER_TOO_SMALL;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pProfConfigs = (PPROF_CONFIGS)pIrp->AssociatedIrp.SystemBuffer;

        if (!HelpCheckClient(pDevExt, pProfConfigs->ulClientId))
        {
            DRVPRINT("ERROR IoctlAddProfConfigsHandler, helpCheckClient failed");
            ret = STATUS_ACCESS_DENIED;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        // Delete the memory pool in case it is not deleted in previous run
        ReleaseMemoryPool();

        // Create memory pool for this session
        if (false == CreateMemoryPool())
        {
            DRVPRINT("ERROR Session pool creation failed");
            ret = STATUS_NO_MEMORY;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        //Get client context
        pClient = & (pDevExt->m_pClient[pProfConfigs->ulClientId]);
        // Considering only one configuration as of now from client
        pSrcCfg = reinterpret_cast<ProfileConfig*>(pProfConfigs->uliProfileConfigs);

        //Simple check to make sure user sent down valid address and count
        if ((!MmIsAddressValid(reinterpret_cast<PVOID>(pProfConfigs->uliProfileConfigs)))
            || (0 == pProfConfigs->ulConfigCnt)
            || (0 == pSrcCfg->m_attrCnt))
        {
            pIrp->IoStatus.Information = sizeof(PROF_CONFIGS);
            ret = STATUS_INVALID_PARAMETER;
        }
    }

    // Create shared buffer
    if (STATUS_SUCCESS == ret)
    {
        ret = CreateSharedBuffer(pSrcCfg->m_fill);

        if (STATUS_SUCCESS != ret)
        {
            DRVPRINT("ERROR CreateSharedBuffer failed");
        }
    }

    // Allocate per core buffer
    if (STATUS_SUCCESS == ret)
    {
        ret = AllocateAndInitDataBuffers(pSrcCfg, &g_pCoreCfg);

        if (STATUS_SUCCESS != ret)
        {
            DRVPRINT("ERROR AllocateAndInitDataBuffers failed");
        }
    }

    // Allocate header buffer
    if (STATUS_SUCCESS == ret)
    {
        uint8* pBuffer = NULL;
        pClient->m_profileState |= STATE_TBP_SET;
        pClient->m_configCount = pSrcCfg->m_samplingSpec.m_maskCnt;
        pBuffer = (uint8*)GetMemoryPoolBuffer(HEADER_BUFFER_SIZE, true);

        if (NULL != pBuffer)
        {
            pClient->m_header.m_pBuffer = pBuffer;
        }
        else
        {
            DRVPRINT("Header bufffer memory allocation failed");
            ret = STATUS_NO_MEMORY;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        uint32 cfgIdx = 0;
        PwrInternalAddr internalCounter;
        memset(&internalCounter, 0, sizeof(PwrInternalAddr));

        if (NULL != pSharedBuffer)
        {
            memcpy(&internalCounter, &pSharedBuffer[PWR_INTERNAL_COUNTER_BASE], sizeof(PwrInternalAddr));
        }

        //Get core specific mask
        coreCounterMask = pSrcCfg->m_apuCounterMask & PWR_PERCORE_COUNTER_MASK;
        coreMask = pSrcCfg->m_samplingSpec.m_mask;

        // Iterate over all cores and check if mask is set for that core
        for (confCount = 0; confCount < gpPwrDevExt->coreCount; confCount++)
        {
            CoreData* pCoreCfg = NULL;
            uint64 phyCoreMask = 0;

            // Get next core id
            if (coreMask & 0x01)
            {
                coreMask = coreMask >> 1;
                pCoreCfg = g_pCoreCfg + cfgIdx;

                if (NULL == pCoreCfg)
                {
                    ret = STATUS_NO_MEMORY;
                    DRVPRINT("pCoreCfg memory access error");
                    break;
                }

                coreId++;
                cfgIdx++;
            }
            else
            {
                coreId++;
                coreMask =  coreMask >> 1;
                continue;
            }

            if (pSrcCfg->m_apuCounterMask & (1ULL << COUNTERID_CORE_ENERGY))
            {
                if (!HelpPwrIsSmtEnabled() || (0 == (confCount % 2)))
                {
                    phyCoreMask = (1ULL << COUNTERID_CORE_ENERGY);
                }
            }

            // Fill per core configuration
            if ((NULL != pCoreCfg) && (NULL != pCoreCfg->m_pOsData))
            {
                pCoreCfg->m_clientId = pProfConfigs->ulClientId;
                pCoreCfg->m_pCoreBuffer->m_recCnt = 0;
                pCoreCfg->m_coreId = coreId - 1;
                pCoreCfg->m_sampleId = cfgIdx;

                // PCORE configuration
                pCoreCfg->m_pOsData->m_pcoreCfg.m_coreId = coreId - 1;
                pCoreCfg->m_pOsData->m_pcoreCfg.m_type = APIC;
                pCoreCfg->m_pOsData->m_pcoreCfg.m_resourceId = 0;
                pCoreCfg->m_pOsData->m_pcoreCfg.m_cfg.msrControlValue = 0;
                pCoreCfg->m_pOsData->m_pcoreCfg.m_cfg.fnDataCallback = SampleDataCallback;
                pCoreCfg->m_pOsData->m_pcoreCfg.m_cfg.fnControlMonitoringCb = PwrProfControlMonitoringCb;

                if (PROFILE_TYPE_TIMELINE == (ProfileType)pSrcCfg->m_samplingSpec.m_profileType)
                {
                    //Need to multiply with 10 to make the sampling perion in ms
                    pCoreCfg->m_pOsData->m_pcoreCfg.m_cfg.count = pSrcCfg->m_samplingSpec.m_samplingPeriod * 10;
                }
                else
                {
                    // Sampling period is 1ms in case of source code profling
                    pCoreCfg->m_pOsData->m_pcoreCfg.m_cfg.count = 10;
                }

                pCoreCfg->m_samplingInterval = (uint32)pSrcCfg->m_samplingSpec.m_samplingPeriod;
            }
            else
            {
                ret = STATUS_ACCESS_DENIED;
                DRVPRINT("Invalid memory access pCoreCfg or m_pOsData");
                break;
            }

            if (0 == (cfgIdx - 1))
            {
                uint32 cnt = 0;
                bool isSmu = false;

                if (NULL == pCoreCfg->m_smuCfg)
                {
                    ret = STATUS_ACCESS_DENIED;
                    DRVPRINT("Invalid memory access pCoreCfg->m_smuCfg");
                    break;
                }

                // Set superset of the attribute mask for master core
                pCoreCfg->m_counterMask = pSrcCfg->m_apuCounterMask;
                memcpy(pCoreCfg->m_smuCfg, &pSrcCfg->m_activeList, sizeof(SmuList));

                // Check if any smu counters are configured
                for (cnt = 0; cnt < pSrcCfg->m_activeList.m_count; cnt++)
                {
                    if (0 != pSrcCfg->m_activeList.m_info[cnt].m_counterMask)
                    {
                        isSmu = true;
                        break;
                    }
                }

                if ((true == isSmu) && (false == FillSmuAccessData(&pSrcCfg->m_activeList, pCoreCfg->m_smuCfg)))
                {
                    pProfConfigs->ulStatus = PROF_ERROR_SMU_CONGIGURATION;
                    DRVPRINT("Error: PROF_ERROR_SMU_CONGIGURATION");
                    ret = STATUS_ACCESS_DENIED;
                    break;
                }
            }
            else
            {
                // Set only core specific attributes to other core config
                // Also add first 3 MUST attributes to the mask
                pCoreCfg->m_counterMask = coreCounterMask | phyCoreMask;
                pCoreCfg->m_smuCfg = NULL;
            }
            pCoreCfg->m_skipFirst = 0;

            if (PROFILE_TYPE_PROCESS_PROFILING != pCoreCfg->m_profileType)
            {
                pCoreCfg->m_skipFirst = 1;
            }

            PwrExecuteDpc(static_cast<uint32>(confCount), (PKDEFERRED_ROUTINE)PwrCoreInitialize, false);

            DRVPRINT("confCount %d mask 0x%x", confCount, pCoreCfg->m_counterMask);
            // Fill internal counters if any
            memcpy(&pCoreCfg->m_internalCounter, &internalCounter, sizeof(PwrInternalAddr));

            pCoreCfg->m_profileType = pSrcCfg->m_samplingSpec.m_profileType;

            // TODO: Need to remove this line when pcore is cleaned up
            pCoreCfg->m_pOsData->m_pcoreCfg.m_cfg.attributeMask = 0;

            // Calculate the expected length of the record
            GetRequiredBufferLength(pCoreCfg, &pCoreCfg->m_recLen);

            pCoreCfg->m_pOsData->m_pcoreCfg.m_cfg.callbackArgument = (PVOID)pCoreCfg;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        // Write the config to header buffer
        WriteHeader(pClient, pSrcCfg);
        // Return Code
        pProfConfigs->ulStatus = PROF_SUCCESS;
        pIrp->IoStatus.Information = sizeof(PROF_CONFIGS);
    }

    return ret;
}

//IoctlStartProfilerHandler: Start a profile on the specified profile client
NTSTATUS IoctlStartProfilerHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                   IN PIRP pIrp,
                                   IN PIO_STACK_LOCATION pIrpStack)
{
    NTSTATUS ret = STATUS_SUCCESS;
    PPROFILER_PROPERTIES pProfProp = NULL;
    ULONG ulCaStatus = PROF_SUCCESS;
    ClientData* pClient = NULL;

    DRVPRINT("IoctlStartProfilerHandler called");

    pIrp->IoStatus.Information = 0;

    //  Verifying parameters
    if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(PROFILER_PROPERTIES))
    {
        ret = STATUS_INFO_LENGTH_MISMATCH;
    }

    if (STATUS_SUCCESS == ret)
    {
        if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(PROFILER_PROPERTIES))
        {
            ret = STATUS_BUFFER_TOO_SMALL;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pProfProp = (PPROFILER_PROPERTIES)pIrp->AssociatedIrp.SystemBuffer;

        if (!HelpCheckClient(pDevExt, pProfProp->ulClientId))
        {
            ret = STATUS_ACCESS_DENIED;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pClient = & (pDevExt->m_pClient[pProfProp->ulClientId]);
        //Check for valid state
        ret = CheckIfValidOperation(pDevExt->resourceCounts,
                                    pClient->m_profileState, IOCTL_START_PROFILER);

        if (STATUS_SUCCESS != ret)
        {
            DRVPRINT("IoctlStartProfilerHandler invalid Operation");
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        //intialize profile values
        pClient->m_profileState |= STATE_PROFILING;
    }

    if (STATUS_SUCCESS == ret)
    {
        if (STATUS_SUCCESS != ret)
        {
            //revert the profile state if allocation is failed
            pClient->m_profileState &= (((uint32) - 1) ^ STATE_PROFILING);
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        //Update buffer header for start time
        ret = UpdateBufferHeader(pClient, 0);
    }

    if (STATUS_SUCCESS == ret)
    {
        //Add the configurations to pcore
        ret = HelpAddPcoreCfgs(pClient, g_pCoreCfg);

        if (STATUS_SUCCESS != ret)
        {
            HelpStopProfile(pClient);
        }
    }


    //Update user level status
    pProfProp->ulStatus = ulCaStatus;
    pIrp->IoStatus.Information = sizeof(PROFILER_PROPERTIES);

    return ret;
}


//IoctlPauseProfilerHandler: Pause any profiling that is going on.  This should only be done for counting
//profiles as it will take much longer for the sampling profiles
NTSTATUS IoctlPauseProfilerHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                   IN PIRP pIrp,
                                   IN PIO_STACK_LOCATION pIrpStack)
{
    NTSTATUS ret = STATUS_SUCCESS;
    PULONG pulRegId = NULL;
    PULONG pulState = NULL;
    ClientData* pClient = NULL;

    DRVPRINT("IoctlPauseProfilerHandler called");
    pIrp->IoStatus.Information  = 0;

    //  Verifying parameters
    if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(ULONG))
    {
        ret = STATUS_INFO_LENGTH_MISMATCH;
    }

    if (STATUS_SUCCESS == ret)
    {
        if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG))
        {
            ret = STATUS_BUFFER_TOO_SMALL;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pulRegId = (PULONG)pIrp->AssociatedIrp.SystemBuffer;
        pulState = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

        if (NULL == pulState)
        {
            ret = STATUS_INVALID_PARAMETER;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        if (!HelpCheckClient(pDevExt, *(uint32*)pulRegId))
        {
            ret = STATUS_ACCESS_DENIED;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pClient = & (pDevExt->m_pClient[*pulRegId]);
        //Check if current status of profiling is valid to enter to pause state
        ret = CheckIfValidOperation(pDevExt->resourceCounts,
                                    pClient->m_profileState,
                                    IOCTL_PAUSE_PROFILER);
    }

    if (STATUS_SUCCESS != ret)
    {
        DRVPRINT("IoctlPauseProfilerHandler invalid Operation");
    }

    if (STATUS_SUCCESS == ret)
    {
        //Check is profiling is already in paused state
        if (0 != (pClient->m_profileState & STATE_PAUSED))
        {
            //already in paused state
            DRVPRINT("IGNORING PAUSE, already paused!!!");
            *pulState = pClient->m_profileState;
            pIrp->IoStatus.Information = sizeof(ULONG);
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        HelpRemovePcoreCfgs(pClient);

        //Set the paused state
        pClient->m_profileState |= STATE_PAUSED;
        *pulState = pClient->m_profileState;

        pIrp->IoStatus.Information = sizeof(ULONG);
    }

    return ret;
}


//IoctlResumeProfilerHandler: Resume any profiling configurations.
NTSTATUS IoctlResumeProfilerHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                    IN PIRP pIrp,
                                    IN PIO_STACK_LOCATION pIrpStack)
{
    NTSTATUS ret = STATUS_SUCCESS;
    PULONG pulRegId = NULL;
    PULONG pulState = NULL;
    ClientData* pClient = NULL;

    DRVPRINT("IoctlResumeProfilerHandler called");
    pIrp->IoStatus.Information  = 0;

    //  Verifying parameters
    if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(ULONG))
    {
        ret = STATUS_INFO_LENGTH_MISMATCH;
    }

    if (STATUS_SUCCESS == ret)
    {
        if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG))
        {
            ret = STATUS_BUFFER_TOO_SMALL;
        }
    }


    if (STATUS_SUCCESS == ret)
    {
        pulRegId = (PULONG)pIrp->AssociatedIrp.SystemBuffer;
        pulState = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

        if (!HelpCheckClient(pDevExt, *(uint32*)pulRegId))
        {
            ret = STATUS_ACCESS_DENIED;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pClient = & (pDevExt->m_pClient[*pulRegId]);

        ret = CheckIfValidOperation(pDevExt->resourceCounts,
                                    pClient->m_profileState,
                                    IOCTL_RESUME_PROFILER);
    }

    if (STATUS_SUCCESS != ret)
    {
        DRVPRINT("IoctlResumeProfilerHandler invalid Operation");
    }

    if (STATUS_SUCCESS == ret)
    {
        //check if profile is already running
        if (0 == (pClient->m_profileState & STATE_PAUSED))
        {
            DRVPRINT("IGNORING RESUME, already resumed!!!");
            //Profile is already runnuning
            *pulState = pClient->m_profileState;
            pIrp->IoStatus.Information = sizeof(ULONG);
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        ret = HelpAddPcoreCfgs(pClient, g_pCoreCfg);
    }

    if (STATUS_SUCCESS == ret)
    {
        //clear the paused state
        pClient->m_profileState &= (((ULONG) - 1) ^ STATE_PAUSED);
    }

    *pulState = pClient->m_profileState;
    pIrp->IoStatus.Information = sizeof(ULONG);

    return ret;
}

//IoctlGetFileHeaderBufferHandler: Header buffer is separate from the actual raw data buffer
//This function will copy the header buffer to client buffer.
NTSTATUS IoctlGetFileHeaderBufferHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                         IN PIRP pIrp,
                                         IN PIO_STACK_LOCATION pIrpStack)
{
    NTSTATUS ret = STATUS_SUCCESS;
    PFILE_HEADER pFileHeader = NULL;
    ULONG ulCaStatus = PROF_SUCCESS;
    ClientData* pClient = NULL;

    DRVPRINT("IoctlGetFileHeaderBufferHandler called");

    pIrp->IoStatus.Information = 0;

    //  Verifying parameters
    if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(FILE_HEADER))
    {
        ret = STATUS_INFO_LENGTH_MISMATCH;
    }

    if (STATUS_SUCCESS == ret)
    {
        if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(FILE_HEADER))
        {
            ret = STATUS_BUFFER_TOO_SMALL;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pFileHeader = (PFILE_HEADER)pIrp->AssociatedIrp.SystemBuffer;

        if (!HelpCheckClient(pDevExt, pFileHeader->ulClientId))
        {
            ret = STATUS_ACCESS_DENIED;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pClient = & (pDevExt->m_pClient[pFileHeader->ulClientId]);

        //Check if the header buffer is ready and accessible
        ret = CheckIfValidOperation(pDevExt->resourceCounts,
                                    pClient->m_profileState,
                                    IOCTL_GET_FILE_HEADER_BUFFER);
    }

    if (STATUS_SUCCESS != ret)
    {
        DRVPRINT("IoctlGetFileHeaderBufferHandler invalid Operation");
    }

    if (STATUS_SUCCESS == ret)
    {
        //Check for the valid address
        if ((!MmIsAddressValid(reinterpret_cast<PVOID>(pFileHeader->uliBuffer))) || (0 != pFileHeader->ulBufferId))
        {
            DRVPRINT("IoctlGetFileHeaderBufferHandler invalid buffer id");
            ret = STATUS_INVALID_PARAMETER;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        RtlMoveMemory(reinterpret_cast<UCHAR*>(pFileHeader->uliBuffer),
                      pClient->m_header.m_pBuffer,
                      HEADER_BUFFER_SIZE);

        //Only one buffer is considered for header data
        pFileHeader->ulNoOfBuffer = 1;
    }

    //Update the status for the client
    pFileHeader->ulStatus = ulCaStatus;
    pIrp->IoStatus.Information = sizeof(FILE_HEADER);

    return ret;
}


//IoctlStopProfilerHandler: Stop a profile on the specified profile client
NTSTATUS IoctlStopProfilerHandler(IN PPWRPROF_DEV_EXTENSION pDevExt,
                                  IN PIRP pIrp,
                                  IN PIO_STACK_LOCATION pIrpStack)
{
    NTSTATUS ret = STATUS_SUCCESS;
    PULONG pulStatus = NULL;
    ClientData* pClient = NULL;
    PPROF_CONFIGS pProfConfigs = NULL;

    DRVPRINT("IoctlStopProfilerHandler called");
    pIrp->IoStatus.Information  = 0;

    //Verifying parameters
    if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(ULONG64))
    {
        ret = STATUS_INFO_LENGTH_MISMATCH;
    }

    if (STATUS_SUCCESS == ret)
    {
        if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG64))
        {
            ret = STATUS_BUFFER_TOO_SMALL;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        // Stop the event and release handle
        if (NULL != gpPwrDevExt->m_userSpaceEvent)
        {
            ObDereferenceObject(gpPwrDevExt->m_userSpaceEvent);
            gpPwrDevExt->m_userSpaceEvent = nullptr;
        }

        pProfConfigs = (PPROF_CONFIGS)pIrp->AssociatedIrp.SystemBuffer;
        pulStatus = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

        //Checking for valid Inputs and States
        if (!HelpCheckClient(pDevExt, pProfConfigs->ulClientId))
        {
            ret = STATUS_ACCESS_DENIED;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pClient = & (pDevExt->m_pClient[pProfConfigs->ulClientId]);
        ret = HelpStopProfile(pClient);
    }

    if (STATUS_SUCCESS == ret)
    {
        *pulStatus = PROF_SUCCESS;
        pIrp->IoStatus.Information = sizeof(ULONG);
    }

    return ret;
}


//IoctlUnegisterClient: Unregister a profile client with the PwrProf personality driver
NTSTATUS IoctlUnegisterClient(IN PPWRPROF_DEV_EXTENSION pDevExt,
                              IN PIRP pIrp,
                              IN PIO_STACK_LOCATION pIrpStack)
{
    NTSTATUS ret = STATUS_SUCCESS;
    PULONG pulRegId = NULL;
    ClientData* pClient;

    pIrp->IoStatus.Information  = 0;

    //  Verifying parameters
    if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(ULONG))
    {
        ret = STATUS_INFO_LENGTH_MISMATCH;
    }

    if (STATUS_SUCCESS == ret)
    {
        pulRegId = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

        if (!HelpCheckClient(pDevExt, *(uint32*)pulRegId))
        {
            ret = STATUS_ACCESS_DENIED;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pClient = & (pDevExt->m_pClient[*pulRegId]);
        //Unregister client now
        ret = HelpUnregisterClient(pClient);
    }

    pIrp->IoStatus.Information = sizeof(ULONG);

    return ret;
}

NTSTATUS IoctlAccessPciDevice(IN PPWRPROF_DEV_EXTENSION pDevExt,
                              IN PIRP pIrp,
                              IN PIO_STACK_LOCATION pIrpStack)
{
    NTSTATUS ret = STATUS_SUCCESS;
    PACCESS_PCI pPCI = NULL;

    (void)pDevExt;

    pIrp->IoStatus.Information = 0;

    //  Verifying parameters
    if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(ACCESS_PCI))
    {
        ret = STATUS_INFO_LENGTH_MISMATCH;
    }

    if (STATUS_SUCCESS == ret)
    {
        if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ACCESS_PCI))
        {
            ret = STATUS_BUFFER_TOO_SMALL;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pPCI = (PACCESS_PCI)pIrp->AssociatedIrp.SystemBuffer;

        //Check for the valid address
        if (NULL == pPCI)
        {
            ret = STATUS_INVALID_PARAMETER;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        ret = HelpAccessPciAddress(pPCI);

        if (false == ret)
        {
            ret = STATUS_ACCESS_DENIED;
        }
    }

    pIrp->IoStatus.Information = sizeof(ACCESS_PCI);

    return ret;
}

NTSTATUS IoctlAccessMSR(IN PPWRPROF_DEV_EXTENSION pDevExt,
                        IN PIRP pIrp,
                        IN PIO_STACK_LOCATION pIrpStack)
{
    NTSTATUS ret = STATUS_SUCCESS;
    PACCESS_MSR pMSR = NULL;

    DRVPRINT("IoctlAccessMSR called");
    (void)pDevExt;

    pIrp->IoStatus.Information = 0;

    //  Verifying parameters
    if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(ACCESS_MSR))
    {
        ret = STATUS_INFO_LENGTH_MISMATCH;
    }

    if (STATUS_SUCCESS == ret)
    {
        if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ACCESS_MSR))
        {
            ret = STATUS_BUFFER_TOO_SMALL;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pMSR = (PACCESS_MSR)pIrp->AssociatedIrp.SystemBuffer;

        //Check for the valid address
        if (NULL == pMSR)
        {
            ret = STATUS_INVALID_PARAMETER;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        ret = HelpAccessMSRAddress(pMSR);

        if (false == ret)
        {
            ret = STATUS_BUFFER_TOO_SMALL;
        }
    }

    pIrp->IoStatus.Information = sizeof(ACCESS_MSR);

    return ret;
}

// IoctlAccessMMIO: MMIO access for user space.
NTSTATUS IoctlAccessMMIO(IN PPWRPROF_DEV_EXTENSION pDevExt,
                         IN PIRP pIrp,
                         IN PIO_STACK_LOCATION pIrpStack)
{
    NTSTATUS ret = STATUS_SUCCESS;
    PACCESS_MMIO pMMIO = NULL;

    DRVPRINT("IoctlAccessMMIO called");
    (void)pDevExt;

    pIrp->IoStatus.Information = 0;

    //  Verifying parameters
    if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(ACCESS_MMIO))
    {
        ret = STATUS_INFO_LENGTH_MISMATCH;
    }

    if (STATUS_SUCCESS == ret)
    {
        if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ACCESS_MMIO))
        {
            ret = STATUS_BUFFER_TOO_SMALL;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pMMIO = (PACCESS_MMIO)pIrp->AssociatedIrp.SystemBuffer;

        //Check for the valid address
        if (NULL == pMMIO)
        {
            ret = STATUS_INVALID_PARAMETER;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        ret = AccessMMIO(pMMIO);
    }

    pIrp->IoStatus.Information = sizeof(ACCESS_MMIO);

    return ret;
}

// IoctlSetEvent: Set event to communicate with userspace
NTSTATUS IoctlSetEvent(IN PPWRPROF_DEV_EXTENSION pDevExt,
                       IN PIRP pIrp,
                       IN PIO_STACK_LOCATION pIrpStack)
{
    NTSTATUS ret = STATUS_SUCCESS;
    HANDLE eventHld = 0;
    uint64* pData = NULL;

    DRVPRINT("called");

    pIrp->IoStatus.Information = 0;

    //  Verifying parameters
    if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength != sizeof(uint64))
    {
        DRVPRINT("Input buffer length missmatch expected %d, actual %d", sizeof(uint64),
                 pIrpStack->Parameters.DeviceIoControl.InputBufferLength);
        ret = STATUS_INFO_LENGTH_MISMATCH;
    }

    if (STATUS_SUCCESS == ret)
    {
        if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(uint64))
        {
            DRVPRINT("Output buffer length missmatch expected %d, actual %d", sizeof(uint64),
                     pIrpStack->Parameters.DeviceIoControl.OutputBufferLength);
            ret = STATUS_BUFFER_TOO_SMALL;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pData = (uint64*)pIrp->AssociatedIrp.SystemBuffer;
        eventHld = (HANDLE) * pData;
        DRVPRINT("eventHld %p", *pData);
        ret = ObReferenceObjectByHandle(eventHld,
                                        SYNCHRONIZE,
                                        NULL,
                                        KernelMode,
                                        reinterpret_cast<PVOID*>(&pDevExt->m_userSpaceEvent),
                                        NULL
                                       );

        if (!NT_SUCCESS(ret))
        {
            switch (ret)
            {
                case STATUS_OBJECT_TYPE_MISMATCH:
                    DRVPRINT("ERROR: ObReferenceObjectByHandle failed: STATUS_OBJECT_TYPE_MISMATCH");
                    break;

                case STATUS_ACCESS_DENIED:
                    DRVPRINT("ERROR: ObReferenceObjectByHandle failed: STATUS_ACCESS_DENIED");
                    break;

                case STATUS_INVALID_HANDLE:
                    DRVPRINT("ERROR: ObReferenceObjectByHandle failed: STATUS_INVALID_HANDLE");
                    break;

                default:
                    DRVPRINT("ERROR: ObReferenceObjectByHandle unknown %d", ret);
                    break;
            }

        }

    }

    pIrp->IoStatus.Information = sizeof(HANDLE);

    return ret;
}


