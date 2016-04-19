//=============================================================
// (c) 2012 Advanced Micro Devices, Inc.
//
/// \author franksw
/// \version $Revision: $
/// \brief  Necessary internal functionality
/// This file contains the interface routines user applications
///                 indirectly call to do profiling. It interfaces with the
///                 pcore driver to do the low level hardware interactions
//
//=============================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=============================================================

#include "..\inc\CpuProfDevice.hpp"
#include "..\inc\CpuProfInternal.h"
#include "..\inc\UserAccess\CpuProfDriver.h"
#include <WinDriverStackWalker\Include\StackWalker.hpp>
#include "..\inc\CpuProfIoControl.h"

// CpuProf driver entry point.  Called when it's first loaded.
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS status = STATUS_UNSUCCESSFUL;

    if (PcoreIsLoaded())
    {
        UNICODE_STRING uniNtNameString;
        // Create counted string version of our device name.
        RtlInitUnicodeString(&uniNtNameString, NT_DEVICE_NAME);

        // Create the device object.
        PDEVICE_OBJECT pDeviceObject = NULL;
        status = IoCreateDevice(pDriverObject,
                                sizeof(CpuProf::Device),
                                &uniNtNameString,
                                FILE_DEVICE_UNKNOWN,
                                0,                     // No standard device characteristics
                                FALSE,                 // This isn't an exclusive device
                                &pDeviceObject);

        if (STATUS_SUCCESS == status)
        {
            // Fill in dispatch points.
            pDriverObject->MajorFunction[IRP_MJ_CREATE] = CpuProfCreate;
            pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CpuProfClose;
            pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = CpuProfDispatch;
            pDriverObject->MajorFunction[IRP_MJ_CLEANUP] = CpuProfCleanup;
            pDriverObject->DriverUnload = CpuProfUnload;

            UNICODE_STRING uniWin32NameString;
            // Create counted string version of our Win32 device name.
            RtlInitUnicodeString(&uniWin32NameString, DOS_DEVICE_NAME);

            // Create a link from our device name to a name in the Win32 namespace.
            status = IoCreateSymbolicLink(&uniWin32NameString, &uniNtNameString);

            if (STATUS_SUCCESS == status)
            {
                status = InitializeNtKernelExt(KX_INIT_FUNCTIONS & ~KX_INIT_THREAD_AFFINITY);

                if (STATUS_SUCCESS == status)
                {
                    ULONG coresCount = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);

                    status = CpuProf::Device::Create(pDeviceObject, coresCount);
                }

                if (STATUS_SUCCESS != status)
                {
                    IoDeleteSymbolicLink(&uniWin32NameString);
                    IoDeleteDevice(pDriverObject->DeviceObject);
                    PrintError("Failed to create CpuProf device!");
                }
            }
            else
            {
                IoDeleteDevice(pDriverObject->DeviceObject);
                PrintError("Failed to create the symbolic link!");
                status = STATUS_UNSUCCESSFUL;
            }
        }
        else
        {
            PrintError("Failed to create the device object!");
            status = STATUS_UNSUCCESSFUL;
        }
    }
    else
    {
        PrintError("Pcore is not loaded!");
    }


    if (STATUS_SUCCESS == status)
    {
        PrintInfo("Driver loaded successfully (v%d.%d.%d)!", CPUPROF_MAJOR_VERSION, CPUPROF_MINOR_VERSION, CPUPROF_BUILD_VERSION);
    }
    else
    {
        PrintError("Failed to load driver (v%d.%d.%d)!", CPUPROF_MAJOR_VERSION, CPUPROF_MINOR_VERSION, CPUPROF_BUILD_VERSION);
    }

    return status;
}


// Called when the CpuProf driver is unloaded
VOID CpuProfUnload(IN PDRIVER_OBJECT pDriverObject)
{
    PrintInfo("Unloading!");

    CpuProf::Device::Destroy();

    UNICODE_STRING uniWin32NameString;
    RtlInitUnicodeString(&uniWin32NameString, DOS_DEVICE_NAME);

    // Delete the link from our device name to a name in the Win32 namespace.
    IoDeleteSymbolicLink(&uniWin32NameString);

    // Finally delete our device object
    IoDeleteDevice(pDriverObject->DeviceObject);
}


// This routine is the dispatch handler for the driver. It is responsible for processing the IRPs.
NTSTATUS CpuProfDispatch(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    // Initialize the IRP info field.
    // This is used to return the number of bytes transferred.
    pIrp->IoStatus.Information = 0;

    IO_STACK_LOCATION* pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    CpuProf::Device* pDevExt = static_cast<CpuProf::Device*>(pDeviceObject->DeviceExtension);

    NTSTATUS status;

    // Dispatch based on MajorFunction code and IoControlCode.
    if (IRP_MJ_DEVICE_CONTROL == pIrpStack->MajorFunction && CpuProf::Device::GetInstance() == pDevExt)
    {
        // Dispatch on IOCTL
        switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode)
        {
            case IOCTL_GET_VERSION:
                status = IoctlGetVersionHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_SET_TIMER_PROPERTIES:
                status = IoctlSetTimerPropertiesHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_GET_TIMER_PROPERTIES:
                status = IoctlGetTimerPropertiesHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_ADD_EVENT_PROPERTIES:
                status = IoctlSetEventPropertiesHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_GET_EVENT_PROPERTIES:
                status = IoctlGetEventPropertiesHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_START_PROFILER:
                status = IoctlStartProfilerHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_STOP_PROFILER:
                status = IoctlStopProfilerHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_PAUSE_PROFILER:
                status = IoctlPauseProfilerHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_RESUME_PROFILER:
                status = IoctlResumeProfilerHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_REGISTER_CLIENT:
                status = IoctlRegisterClient(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_UNREGISTER_CLIENT:
                status = IoctlUnegisterClient(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_SET_OUTPUT_FILE:
                status = IoctlSetOutputFileHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_GET_OUTPUT_FILE:
                status = IoctlGetOutputFileHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_GET_RECORD_COUNT:
                status = IoctlGetRecordCountHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_GET_PROFILER_PROPERTIES:
                status = IoctlGetProfilerPropertiesHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_SET_CSS_PROPERTIES:
                status = IoclSetCSSPropertiesHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_GET_CSS_PROPERTIES:
                status = IoclGetCSSPropertiesHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_SET_IBS_PROPERTIES:
                status = IoctlSetIbsPropertiesHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_GET_IBS_PROPERTIES:
                status = IoctlGetIbsPropertiesHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_SET_PID_PROPERTIES:
                status = IoctlSetPIDPropertiesHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_GET_PID_PROPERTIES:
                status = IoctlGetPIDPropertiesHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_GET_EVENT_COUNT:
                status = IoctlGetEventCountHandler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_CLEAR_PROFILER:
                status = IoctlClearProfiler(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_GET_OVERHEAD:
                status = IoctlGetOverhead(pDevExt, pIrp, pIrpStack);
                break;

            case IOCTL_GET_AVAILABILITY:
                status = IoctlGetAvailability(pDevExt, pIrp, pIrpStack);
                break;

            default:
                // Unrecognized IOCTL request
                status = STATUS_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        // Any other Major Function types are not supported by this driver.
        status = STATUS_NOT_IMPLEMENTED;
    }


    // We are done with I/O request.  Record the status of the I/O action.
    pIrp->IoStatus.Status = status;

    // Don't boost priority when returning since this took little time.
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return status;
} //CpuProfDispatch


//Unused device creation routine
NTSTATUS CpuProfCreate(IN PDEVICE_OBJECT pDeviceObject, IN PIRP Irp)
{
    PrintInfo("Driver Opened Successfully!");

    UNREFERENCED_PARAMETER(pDeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


//Unused device close routine
NTSTATUS CpuProfClose(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    PrintInfo("Driver Closed Successfully!");

    UNREFERENCED_PARAMETER(pDeviceObject);

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


//Called when all handles to the driver file object are closed (like the user
// application that opened the driver crashes).  It will ensure that any
// associated clients are unregistered (and therefore stop any applicable
// profiles).
NTSTATUS CpuProfCleanup(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    PrintInfo("Cleanup called.");

    CpuProf::Device* pDevExt = static_cast<CpuProf::Device*>(pDeviceObject->DeviceExtension);
    IO_STACK_LOCATION* pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

    pDevExt->ReleaseClients(pIrpStack->FileObject);

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}
