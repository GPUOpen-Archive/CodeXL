//=============================================================
// (c) 2013 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \version $Revision: $
/// \brief  Power profile control APIs
//
//=============================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=============================================================
#include <ntifs.h>
#include <ntddk.h>
#include <AMDTPwrProfDriver.h>
#include <AMDTPwrProfInternal.h>
#include <AMDTPwrProfioctls.h>
#include <AMDTHelpers.h>
#include <AMDTSmu7Interface.h>
#include <AMDTSmu8Interface.h>
#include <AMDTCounterAccessInterface.h>
#include <AMDTCommonConfig.h>
#include <WinDriverUtils\Include\NtKernelExt.h>

//static ClientData g_clientList[MAX_CLIENT_COUNT];
extern uint8* pSharedBuffer;

/// A global pointer so that the ISR callback can access data
PPWRPROF_DEV_EXTENSION gpPwrDevExt;
static bool g_powerOn = false;

//DriverEntry: PwrProf driver entry point. Called when it's first loaded.
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,
                                IN PUNICODE_STRING RegistryPath)
{
    NTSTATUS ret = STATUS_SUCCESS;
    PDEVICE_OBJECT pDeviceObject = NULL;
    PPWRPROF_DEV_EXTENSION pDevExt = NULL;
    UNICODE_STRING uniNtNameString;
    UNICODE_STRING uniWin32NameString;
    ClientData* pClientData = NULL;
    KAFFINITY tempAff;
    int i;

    //Suppressed warning
    (void)RegistryPath;

    if (!PcoreIsLoaded())
    {
        DRVPRINT("PCore is not loaded");
        ret = STATUS_UNSUCCESSFUL;
    }

    if (STATUS_SUCCESS == ret)
    {
        // Create counted string version of the device name.
        RtlInitUnicodeString(&uniNtNameString, PWRPROF_NT_DEVICE_NAME);

        // Create the device object
        // No standard device characteristics, not an exclusive device
        ret = IoCreateDevice(pDriverObject, sizeof(PWRPROF_DEV_EXTENSION),
                             &uniNtNameString, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObject);

        if (!NT_SUCCESS(ret))
        {
            DRVPRINT("Couldn't create the device object");
            ret = STATUS_UNSUCCESSFUL;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        // Fill in dispatch points.
        pDriverObject->MajorFunction[IRP_MJ_CREATE] = PwrProfCreate;
        pDriverObject->MajorFunction[IRP_MJ_CLOSE] = PwrProfClose;
        pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PwrProfDispatch;
        pDriverObject->MajorFunction[IRP_MJ_CLEANUP] = PwrProfCleanup;
        pDriverObject->DriverUnload = PwrProfUnload;

        // Create counted string version of our Win32 device name.
        RtlInitUnicodeString(&uniWin32NameString, PWRPROF_DOS_DEVICE_NAME);

        // Create a link from our device name to a name in the Win32 namespace.
        ret = IoCreateSymbolicLink(&uniWin32NameString, &uniNtNameString);

        if (STATUS_SUCCESS != ret)
        {
            DRVPRINT("Couldn't create the symbolic link");
            IoDeleteDevice(pDriverObject->DeviceObject);
            ret = STATUS_UNSUCCESSFUL;
        }
    }

    if (STATUS_SUCCESS == ret)
    {
        pDevExt = (PPWRPROF_DEV_EXTENSION)pDeviceObject->DeviceExtension;
        pDevExt->pDeviceObject = pDeviceObject; // Save off device object

        // Store location of DevExt so ISR callback routine can access it.
        gpPwrDevExt = pDevExt;

        //We're doing this because KeNumberProcessors is depreciated for Vista+
        // and KeQueryActiveProcessorCount is only valid for Vista Sp1+
        pDevExt->coreCount = 0;
        tempAff = KeQueryActiveProcessors();

        //Check each bit of the affinity
        do
        {
            pDevExt->coreCount++;
        }
        while (tempAff >>= 1);

        //fill in the count of each resource type
        pDevExt->maxResources = 0;

        for (i = 0; i < MAX_RESOURCE_TYPE; i++)
        {
            PcoreGetResourceCount((PCORERESOURCETYPES)i, &(pDevExt->resourceCounts[i]));

            if (pDevExt->maxResources < pDevExt->resourceCounts[i])
            {
                pDevExt->maxResources = pDevExt->resourceCounts[i];
            }

            if (EVENT_CTR == i)
            {
                pDevExt->maxConfigs = 8 * pDevExt->resourceCounts[i];
            }
        }

        // Reset and get the client list
        pClientData = (ClientData*)ExAllocatePoolWithTag(NonPagedPool, sizeof(ClientData) * MAX_CLIENT_COUNT, 'CLD');

        if (NULL == pClientData)
        {
            DRVPRINT(" failed memory allocation for pClientData");
            ret = STATUS_NO_MEMORY;
        }

        if (STATUS_SUCCESS == ret)
        {
            memset(pClientData, 0, sizeof(ClientData)* MAX_CLIENT_COUNT);

            pDevExt->m_pClient = pClientData;

            //Initialize clients
            for (i = 0; i < MAX_CLIENT_COUNT; i++)
            {
                pDevExt->m_pClient[i].m_clientId = i;
            }

            pDevExt->CanUnload = TRUE;
        }
    }

    return ret;
}

//PwrProfUnload: Called when the PwrProf driver is unloaded
VOID PwrProfUnload(IN PDRIVER_OBJECT pDriverObject)
{
    UNICODE_STRING uniWin32NameString;

    DRVPRINT("PWRPROF: Unloading!!");

    RtlInitUnicodeString(&uniWin32NameString, PWRPROF_DOS_DEVICE_NAME);

    // Delete the link from our device name to a name in the Win32 namespace.
    IoDeleteSymbolicLink(&uniWin32NameString);

    // Finally delete our device object
    IoDeleteDevice(pDriverObject->DeviceObject);
}

//PwrProfDispatch: This routine is the dispatch handler for the driver. It is responsible
// for processing the IRPs.
NTSTATUS PwrProfDispatch(IN PDEVICE_OBJECT pDO, IN PIRP pIrp)
{
    NTSTATUS ret = STATUS_NOT_IMPLEMENTED;
    PIO_STACK_LOCATION      pIrpStack;
    PPWRPROF_DEV_EXTENSION  pDevExt;

    //  Initialize the irp info field.
    //  This is used to return the number of bytes transferred.
    pIrp->IoStatus.Information = 0;

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pDevExt = (PPWRPROF_DEV_EXTENSION)pDO->DeviceExtension;

    // Dispatch based on MajorFunction code and IoControlCode.
    switch (pIrpStack->MajorFunction)
    {

        case IRP_MJ_DEVICE_CONTROL:
        {
            //  Dispatch on IOCTL
            switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode)
            {
                case IOCTL_GET_VERSION:
                    ret = IoctlGetVersionHandler(pDevExt, pIrp, pIrpStack);
                    break;

                case IOCTL_REGISTER_CLIENT:
                    ret = IoctlRegisterClient(pDevExt, pIrp, pIrpStack);
                    break;

                case IOCTL_ADD_PROF_CONFIGS:
                    ret = IoctlAddProfConfigsHandler(pDevExt, pIrp, pIrpStack);
                    break;

                case IOCTL_START_PROFILER:
                    ret = IoctlStartProfilerHandler(pDevExt, pIrp, pIrpStack);
                    break;

                case IOCTL_PAUSE_PROFILER:
                    ret = IoctlPauseProfilerHandler(pDevExt, pIrp, pIrpStack);
                    break;

                case IOCTL_RESUME_PROFILER:
                    ret = IoctlResumeProfilerHandler(pDevExt, pIrp, pIrpStack);
                    break;

                case IOCTL_GET_FILE_HEADER_BUFFER:
                    ret = IoctlGetFileHeaderBufferHandler(pDevExt, pIrp, pIrpStack);
                    break;

                case IOCTL_STOP_PROFILER:
                    ret = IoctlStopProfilerHandler(pDevExt, pIrp, pIrpStack);
                    break;

                case IOCTL_UNREGISTER_CLIENT:
                    ret = IoctlUnegisterClient(pDevExt, pIrp, pIrpStack);
                    break;

                case IOCTL_ACCESS_PCI_DEVICE:
                    ret = IoctlAccessPciDevice(pDevExt, pIrp, pIrpStack);
                    break;

                case IOCTL_ACCESS_MSR:
                    ret = IoctlAccessMSR(pDevExt, pIrp, pIrpStack);
                    break;

                case IOCTL_ACCESS_MMIO:
                    ret = IoctlAccessMMIO(pDevExt, pIrp, pIrpStack);
                    break;

                case IOCTL_SET_EVENT:
                    ret = IoctlSetEvent(pDevExt, pIrp, pIrpStack);
                    break;

                default:
                    // Unrecognized IOCTL request
                    ret = STATUS_NOT_SUPPORTED;
                    break;
            }

            break;
        }

        default:
            // we should only be getting IOCTLs here because, so any other
            // Major Function types are not supported by this driver
            ret = STATUS_NOT_IMPLEMENTED;
            break;
    }

    // We're done with I/O request.  Record the status of the I/O action.
    pIrp->IoStatus.Status = ret;

    // Don't boost priority when returning since this took little time.
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return ret;
} //PwrProfDispatch


//PwrProfCreate: Unused device creation routine
NTSTATUS PwrProfCreate(IN PDEVICE_OBJECT pDeviceObject, IN PIRP Irp)
{
    DRVPRINT("PWRPROF: Driver Opened Successfully!\n");

    //Supressed warning
    (void)pDeviceObject;

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

//PwrProfClose: Unused device close routine
NTSTATUS PwrProfClose(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    DRVPRINT("Driver Closed Successfully!!");

    //Suppressed warning
    (void)pDeviceObject;
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

//PwrProfCleanup: Called when all handles to the driver file object are closed (like the user
//application that opened the driver crashes).  It will ensure that any
//associated clients are unregistered (and therefore stop any applicable
//profiles).
NTSTATUS PwrProfCleanup(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    PPWRPROF_DEV_EXTENSION pDevExt = (PPWRPROF_DEV_EXTENSION)pDeviceObject->DeviceExtension;
    PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);
    PFILE_OBJECT pFileOut = pStack->FileObject;
    ULONG regId;

    DRVPRINT("Cleanup called");

    // For each client
    for (regId = 0; regId < MAX_CLIENT_COUNT; regId++)
    {
        if (pDevExt->m_pClient[regId].m_osClientCfg.m_userFileObj == pFileOut)
        {
            DRVPRINT("cleanup unregistering client %d", regId);
            HelpUnregisterClient(&(pDevExt->m_pClient[regId]));
        }
    }

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

//SampleDataCallback: This function is called back from pcore's sample interrupt.
VOID NTAPI SampleDataCallback(PCORE_DATA* pData)
{
    //The callback argument informs us of which client set the configuration on pcore

    CoreData* pCoreCfg = (CoreData*)pData->callbackArgument;

    if (true == g_powerOn)
    {
        // Fill context data
        pCoreCfg->m_coreId = pData->core;
        pCoreCfg->m_contextData.m_processId = (uint32) pData->contextId.m_processId;
        pCoreCfg->m_contextData.m_threadId = (uint32) pData->contextId.m_threadId;
        pCoreCfg->m_contextData.m_timeStamp = pData->timeStamp;
        pCoreCfg->m_contextData.m_ip =  KxGetInstructionPointerFromTrapFrame(pData->pTrapFrame);

        WriteSampleData(pCoreCfg);

        // Trigger event to client to inform the data availability
        if (1 == pCoreCfg->m_sampleId)
        {
            bool signal = false;

            if (PROFILE_TYPE_PROCESS_PROFILING == pCoreCfg->m_profileType)
            {
                if (pCoreCfg->m_samplingInterval < 100)
                {
                    signal = !(pCoreCfg->m_pCoreBuffer->m_recCnt % (pCoreCfg->m_samplingInterval));
                }
                else
                {
                    signal = !(pCoreCfg->m_pCoreBuffer->m_recCnt % 100);
                }
            }
            else
            {
                if (pCoreCfg->m_samplingInterval < 100)
                {
                    uint32 interval = 100 / pCoreCfg->m_samplingInterval;
                    signal = !(pCoreCfg->m_pCoreBuffer->m_recCnt % interval);
                }
                else
                {
                    signal = true;
                }
            }

            if (true == signal)
            {
                // DRVPRINT("offset %d recCnt %d", pCoreCfg->m_pCoreBuffer->m_currentOffset,
                //                            pCoreCfg->m_pCoreBuffer->m_recCnt);

                if (nullptr != gpPwrDevExt->m_userSpaceEvent)
                {
                    KeSetEvent((PKEVENT)gpPwrDevExt->m_userSpaceEvent, 0, FALSE);
                }
            }
        }
    }
}

// PwrProfControlMonitoringCb: This callback will be registered with pcore to receive following events
// 1. Start monitoring when profile is started
// 2. Stop monitoring when profile session is ended successfully
// 3. Stop/Start monitoring due to suspend and resume becuse of system hibernate
void NTAPI PwrProfControlMonitoringCb(PVOID pData, bool isActivated)
{
    CoreData* pCoreCfg = (CoreData*)pData;

    if ((NULL != pCoreCfg) && (1 == pCoreCfg->m_sampleId))
    {
        // SMU configuration is available only on master core
        // Check if SMU needs to be activated or deactivated
        ConfigureSmu(pCoreCfg->m_smuCfg, isActivated);
    }

    if (true == isActivated)
    {
        g_powerOn = true;
        // Initialize non-smu counters
        InitializeGenericCounterAccess((uint32)KeGetCurrentProcessorNumberEx(NULL));

        if (PROFILE_TYPE_TIMELINE != pCoreCfg->m_profileType)
        {
            ConfigureSourceProfiling(pCoreCfg);
        }
    }
    else
    {
        CloseGenericCounterAccess();

        if (PROFILE_TYPE_TIMELINE != pCoreCfg->m_profileType)
        {
            CloseSourceProfiling(pCoreCfg);
        }

        g_powerOn = false;
    }
}

//AllocateAndInitDataBuffers:
int32 AllocateAndInitDataBuffers(ProfileConfig* pSrcCfg,
                                 CoreData** ppCoreCfg)
{
    int32 ret = STATUS_SUCCESS;
    uint32 cnt = 0;
    CoreData* pCore = NULL;
    uint8* pBuffer = NULL;
    uint32 offset = PWRPROF_SHARED_METADATA_SIZE;

    if (NULL == pSrcCfg)
    {
        ret = STATUS_NO_MEMORY;
        DRVPRINT("pSrcCfg memory access failed");
    }

    // Allocate only for configured cores
    pCore = (CoreData*)GetMemoryPoolBuffer(pSrcCfg->m_samplingSpec.m_maskCnt * sizeof(CoreData), true);

    if (NULL == pCore)
    {
        ret = STATUS_NO_MEMORY;
        DRVPRINT("pCore memory allocation failed");
    }

    if (STATUS_SUCCESS == ret)
    {
        for (cnt = 0; cnt < pSrcCfg->m_samplingSpec.m_maskCnt; cnt++)
        {
            CoreData* pCfg = pCore + cnt;

            if (NULL == pCfg)
            {
                ret = STATUS_NO_MEMORY;
                DRVPRINT("pCfg memory allocation failed cnt %d", cnt);
                break;
            }

            if (0 == cnt)
            {
                //Allocate for Smu config on first/master core
                pCfg->m_smuCfg = (SmuList*) GetMemoryPoolBuffer(sizeof(SmuList), true);

                if (NULL == pCfg->m_smuCfg)
                {
                    ret = STATUS_NO_MEMORY;
                    DRVPRINT("pCfg->m_smuCfg memory allocation failed");
                    break;
                }
            }
            else
            {
                pCfg->m_smuCfg = NULL;
            }

            pBuffer = pSharedBuffer + offset;

            if (NULL == pBuffer)
            {
                ret = STATUS_NO_MEMORY;
                DRVPRINT("m_pCoreBuffer memory allocation failed for core %d", cnt);
                break;
            }

            memset(pBuffer, 0, sizeof(PageBuffer) + PWRPROF_PERCORE_BUFFER_SIZE);
            offset = offset + sizeof(PageBuffer);

            pCfg->m_pCoreBuffer = (PageBuffer*)pBuffer;
            pCfg->m_pCoreBuffer->m_maxValidOffset = 0;

            pBuffer = pSharedBuffer + offset;

            if (NULL == pBuffer)
            {
                ret = STATUS_NO_MEMORY;
                DRVPRINT("m_pBuffer memory allocation failed for core %d", cnt);
                break;
            }

            offset = offset + PWRPROF_PERCORE_BUFFER_SIZE;

            pCfg->m_pCoreBuffer->m_pBuffer = pBuffer;

            if (NULL == pCfg->m_pCoreBuffer->m_pBuffer)
            {
                ret = STATUS_NO_MEMORY;
                DRVPRINT("pCfg->m_pCoreBuffer->m_pBuffer memory allocation failed");
                break;
            }

            // Allocate PCORE config per core
            pCfg->m_pOsData = (OsCoreCfgData*)GetMemoryPoolBuffer(sizeof(OsCoreCfgData), true);

            if (NULL == pCfg->m_pOsData)
            {
                DRVPRINT("pCfg->m_pOsData memory allocation failed");
                ret = STATUS_NO_MEMORY;
                break;
            }
        }

        *ppCoreCfg = pCore;
    }

    return ret;
}

