//=============================================================
// (c) 2012 Advanced Micro Devices, Inc.
//
/// \author franksw
/// \version $Revision: $
/// \brief contains all the ioctl handlers from the dispatch routine
//
//=============================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=============================================================

#include "..\inc\CpuProfDevice.hpp"
#include "..\inc\CpuProfIoControl.h"
#include "..\inc\CpuProfSystemCallbacks.hpp"
#include <ntstrsafe.h>
#include <winerror.h>


static const char* IoControlCodeToString(ULONG code)
{
    const char* pName = "IOCTL";

#ifdef DBG

    switch (code)
    {
        case IOCTL_GET_VERSION:             pName = "IOCTL_GET_VERSION"; break;

        case IOCTL_SET_TIMER_PROPERTIES:    pName = "IOCTL_SET_TIMER_PROPERTIES"; break;

        case IOCTL_GET_TIMER_PROPERTIES:    pName = "IOCTL_GET_TIMER_PROPERTIES"; break;

        case IOCTL_ADD_EVENT_PROPERTIES:    pName = "IOCTL_ADD_EVENT_PROPERTIES"; break;

        case IOCTL_GET_EVENT_PROPERTIES:    pName = "IOCTL_GET_EVENT_PROPERTIES"; break;

        case IOCTL_START_PROFILER:          pName = "IOCTL_START_PROFILER"; break;

        case IOCTL_STOP_PROFILER:           pName = "IOCTL_STOP_PROFILER"; break;

        case IOCTL_PAUSE_PROFILER:          pName = "IOCTL_PAUSE_PROFILER"; break;

        case IOCTL_RESUME_PROFILER:         pName = "IOCTL_RESUME_PROFILER"; break;

        case IOCTL_REGISTER_CLIENT:         pName = "IOCTL_REGISTER_CLIENT"; break;

        case IOCTL_UNREGISTER_CLIENT:       pName = "IOCTL_UNREGISTER_CLIENT"; break;

        case IOCTL_SET_OUTPUT_FILE:         pName = "IOCTL_SET_OUTPUT_FILE"; break;

        case IOCTL_GET_OUTPUT_FILE:         pName = "IOCTL_GET_OUTPUT_FILE"; break;

        case IOCTL_GET_RECORD_COUNT:        pName = "IOCTL_GET_RECORD_COUNT"; break;

        case IOCTL_GET_PROFILER_PROPERTIES: pName = "IOCTL_GET_PROFILER_PROPERTIES"; break;

        case IOCTL_SET_CSS_PROPERTIES:      pName = "IOCTL_SET_CSS_PROPERTIES"; break;

        case IOCTL_GET_CSS_PROPERTIES:      pName = "IOCTL_GET_CSS_PROPERTIES"; break;

        case IOCTL_SET_IBS_PROPERTIES:      pName = "IOCTL_SET_IBS_PROPERTIES"; break;

        case IOCTL_GET_IBS_PROPERTIES:      pName = "IOCTL_GET_IBS_PROPERTIES"; break;

        case IOCTL_SET_PID_PROPERTIES:      pName = "IOCTL_SET_PID_PROPERTIES"; break;

        case IOCTL_GET_PID_PROPERTIES:      pName = "IOCTL_GET_PID_PROPERTIES"; break;

        case IOCTL_GET_EVENT_COUNT:         pName = "IOCTL_GET_EVENT_COUNT"; break;

        case IOCTL_CLEAR_PROFILER:          pName = "IOCTL_CLEAR_PROFILER"; break;

        case IOCTL_GET_OVERHEAD:            pName = "IOCTL_GET_OVERHEAD"; break;

        case IOCTL_GET_AVAILABILITY:        pName = "IOCTL_GET_AVAILABILITY"; break;
    }

#else
    UNREFERENCED_PARAMETER(code);
#endif

    return pName;
}


#define PRINT_IOCTL_ENTRY(pIrpStack) \
    PrintInfo("Processing %s (Function: 0x%03X)...", IoControlCodeToString(pIrpStack->Parameters.DeviceIoControl.IoControlCode), \
              ((pIrpStack->Parameters.DeviceIoControl.IoControlCode >> 2) & 0xFFFUL))

static inline void InitializeRequest(IRP* pIrp)
{
    pIrp->IoStatus.Information = 0UL;
}

#define INIT_IOCTL_REQUEST(pIrp, pIrpStack) \
    PRINT_IOCTL_ENTRY(pIrpStack); InitializeRequest(pIrp)


template <typename TOut>
static inline void FinalizeSuccessfulRequest(IRP* pIrp, const TOut* pOut)
{
    UNREFERENCED_PARAMETER(pOut);
    pIrp->IoStatus.Information = sizeof(TOut);
}


template <typename TIn>
static inline NTSTATUS GetInputBuffer(IRP* pIrp, TIn const*& pIn)
{
    NTSTATUS status;
    const IO_STACK_LOCATION* pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

    if (sizeof(TIn) == pIrpStack->Parameters.DeviceIoControl.InputBufferLength)
    {
        pIn = static_cast<const TIn*>(pIrp->AssociatedIrp.SystemBuffer);
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_INFO_LENGTH_MISMATCH;
    }

    return status;
}

template <typename TOut>
static inline NTSTATUS GetOutputBuffer(IRP* pIrp, TOut*& pOut)
{
    NTSTATUS status;
    IO_STACK_LOCATION* pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

    if (sizeof(TOut) <= pIrpStack->Parameters.DeviceIoControl.OutputBufferLength)
    {
        pOut = static_cast<TOut*>(pIrp->AssociatedIrp.SystemBuffer);
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_BUFFER_TOO_SMALL;
    }

    return status;
}

template <typename TIn, typename TOut>
static inline NTSTATUS GetInputOutputBuffer(IRP* pIrp, TIn const*& pIn, TOut*& pOut)
{
    NTSTATUS status = GetInputBuffer(pIrp, pIn);

    if (STATUS_SUCCESS == status)
    {
        status = GetOutputBuffer(pIrp, pOut);
    }

    return status;
}

template <typename TInOut>
static inline NTSTATUS GetInputOutputBuffer(IRP* pIrp, TInOut*& pInOut)
{
    const TInOut* pIn;
    return GetInputOutputBuffer(pIrp, pIn, pInOut);
}

static ULONG CountBitsSet64(ULONG64 val)
{
    ULONG count = 0UL;

    while (0ULL != val)
    {
        // Clear the least significant bit set.
        val &= val - 1ULL;
        count++;
    }

    return count;
}


static bool IsCoreMaskValid(const ULONG64* pCpuMask, ULONG cpuMaskCoresCount)
{
    bool ret = true;

    if (0UL != cpuMaskCoresCount)
    {
        ULONG coresCount = CpuProf::GetCoresCount();

        ULONG maskLastIndex = (cpuMaskCoresCount - 1UL) / 64;

        // Check if the first and last addresses of the array are valid.
        if (TRUE == MmIsAddressValid((PVOID)&pCpuMask[0]) &&
            TRUE == MmIsAddressValid((PVOID)&pCpuMask[maskLastIndex]))
        {
            ULONG maskedCoresCount = 0UL;

            for (ULONG i = 0; i <= maskLastIndex; ++i)
            {
                maskedCoresCount += CountBitsSet64(pCpuMask[i]);
            }

            // Be sure there is at least 1 core configured and not too many.
            ret = (0UL != maskedCoresCount && maskedCoresCount <= coresCount);
        }
        else
        {
            ret = false;
        }
    }

    return ret;
}


NTSTATUS IoctlGetVersionHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);
    UNREFERENCED_PARAMETER(pDevExt);

    ULONG64* o_pVersion;
    NTSTATUS status = GetOutputBuffer(pIrp, o_pVersion);

    if (STATUS_SUCCESS == status)
    {
        ULONG pcoreMajor = 0UL, pcoreMinor = 0UL, pcoreBuild = 0UL;

        if (S_OK == PcoreVersion(&pcoreMajor, &pcoreMinor, &pcoreBuild))
        {
            *o_pVersion = DRIVER_VERSION |
                          (static_cast<ULONG64>(pcoreMajor) << 56) |
                          (static_cast<ULONG64>(pcoreMinor) << 48) |
                          (static_cast<ULONG64>(pcoreBuild) << 32);

            FinalizeSuccessfulRequest(pIrp, o_pVersion);
        }
        else
        {
            status = STATUS_DEVICE_NOT_READY;
        }
    }

    return status;
}


/// Register a profile client with the CpuProf personality driver
NTSTATUS IoctlRegisterClient(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    ULONG* o_pClientId;
    NTSTATUS status = GetOutputBuffer(pIrp, o_pClientId);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->AcquireClient(pIrpStack->FileObject);

        if (NULL != pClient)
        {
            *o_pClientId = pClient->GetId();

            FinalizeSuccessfulRequest(pIrp, o_pClientId);
        }
        else
        {
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


/// Unregister a profile client with the CpuProf personality driver
NTSTATUS IoctlUnegisterClient(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    const ULONG* i_pClientId;
    NTSTATUS status = GetInputBuffer(pIrp, i_pClientId);

    if (STATUS_SUCCESS == status)
    {
        if (pDevExt->ReleaseClient(*i_pClientId))
        {
            FinalizeSuccessfulRequest(pIrp, i_pClientId);
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", *i_pClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Allow the user to configure the APIC timer for a profile for the specified
// profile client
NTSTATUS IoctlSetTimerPropertiesHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    TIMER_PROPERTIES* io_pTimerProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pTimerProps);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(io_pTimerProps->ulClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            io_pTimerProps->ulStatus = static_cast<ULONG>(PROF_ERROR);
            FinalizeSuccessfulRequest(pIrp, io_pTimerProps);

            if (IsCoreMaskValid(reinterpret_cast<ULONG64*>(io_pTimerProps->ullCpuMask.QuadPart), io_pTimerProps->ulCoreMaskCount))
            {
                status = pClient->SetTbpConfiguration(*io_pTimerProps);

                // Update the status for the user client.
                if (STATUS_SUCCESS == status)
                {
                    io_pTimerProps->ulStatus = PROF_SUCCESS;
                }
            }
            else
            {
                PrintError("Core mask array validation failed (cores count %u)!", CpuProf::GetCoresCount());
                status = STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", io_pTimerProps->ulClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Allows the user to retrieve the timer configuration for the specified profile client.
NTSTATUS IoctlGetTimerPropertiesHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    TIMER_PROPERTIES* io_pTimerProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pTimerProps);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(io_pTimerProps->ulClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            status = pClient->GetTbpConfiguration(*io_pTimerProps);

            // Update the status for the user client.
            io_pTimerProps->ulStatus = PROF_SUCCESS;

            FinalizeSuccessfulRequest(pIrp, io_pTimerProps);
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", io_pTimerProps->ulClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Add an event configuration to the list to be profiled for the specified profile client.
NTSTATUS IoctlSetEventPropertiesHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    EVENT_PROPERTIES* io_pEventProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pEventProps);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(io_pEventProps->ulClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            io_pEventProps->ulStatus = static_cast<ULONG>(PROF_ERROR);
            FinalizeSuccessfulRequest(pIrp, io_pEventProps);

            if (IsCoreMaskValid(reinterpret_cast<ULONG64*>(io_pEventProps->ullCpuMask.QuadPart), io_pEventProps->ulCoreMaskCount))
            {
                status = pClient->AddEbpConfiguration(*io_pEventProps);

                // Update the status for the user client.
                if (STATUS_SUCCESS == status)
                {
                    io_pEventProps->ulStatus = PROF_SUCCESS;
                }
            }
            else
            {
                PrintError("Core mask array validation failed (cores count %u)!", CpuProf::GetCoresCount());
                status = STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", io_pEventProps->ulClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Retrieve the event settings configured through the call(s) to IOCTL_ADD_EVENT_PROPERTIES.
NTSTATUS IoctlGetEventPropertiesHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    EVENT_PROPERTIES* io_pEventProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pEventProps);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(io_pEventProps->ulClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            ULONG countEvents = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength / sizeof(EVENT_PROPERTIES);

            status = pClient->GetEbpConfigurations(io_pEventProps, countEvents);

            // Update the status for the user client.
            io_pEventProps->ulStatus = PROF_SUCCESS;

            FinalizeSuccessfulRequest(pIrp, io_pEventProps);
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", io_pEventProps->ulClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Start a profile on the specified profile client
NTSTATUS IoctlStartProfilerHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    PROFILER_PROPERTIES* io_pProfProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pProfProps);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(io_pProfProps->ulClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            status = pClient->StartProfiling(reinterpret_cast<HANDLE>(io_pProfProps->hAbort.QuadPart), pIrp->RequestorMode);

            // Update the status for the user client.
            io_pProfProps->ulStatus = PROF_SUCCESS;

            FinalizeSuccessfulRequest(pIrp, io_pProfProps);
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", io_pProfProps->ulClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Stop a profile on the specified profile client.
NTSTATUS IoctlStopProfilerHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    const ULONG* i_pClientId;
    ULONG* o_pStatus;
    NTSTATUS status = GetInputOutputBuffer(pIrp, i_pClientId, o_pStatus);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(*i_pClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            status = pClient->StopProfiling();

            // Retrieve and reset the abort reason.
            *o_pStatus = pClient->GetLastErrorCode();
            pClient->SetLastErrorCode(PROF_SUCCESS);

            FinalizeSuccessfulRequest(pIrp, o_pStatus);
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", *i_pClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Pause any profiling that is going on.
// This should only be done for counting profiles as it will take much longer for the sampling profiles.
NTSTATUS IoctlPauseProfilerHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    const ULONG* i_pClientId;
    ULONG* o_pProfilingState;
    NTSTATUS status = GetInputOutputBuffer(pIrp, i_pClientId, o_pProfilingState);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(*i_pClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            status = pClient->PauseProfiling();

            *o_pProfilingState = pClient->GetState();

            FinalizeSuccessfulRequest(pIrp, o_pProfilingState);
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", *i_pClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Resume any profiling configurations.
NTSTATUS IoctlResumeProfilerHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    const ULONG* i_pClientId;
    ULONG* o_pProfilingState;
    NTSTATUS status = GetInputOutputBuffer(pIrp, i_pClientId, o_pProfilingState);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(*i_pClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            status = pClient->ResumeProfiling();

            *o_pProfilingState = pClient->GetState();

            FinalizeSuccessfulRequest(pIrp, o_pProfilingState);
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", *i_pClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


//  Gets the profile handler properties.
NTSTATUS IoctlGetProfilerPropertiesHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    PROFILER_PROPERTIES* io_pProfProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pProfProps);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(io_pProfProps->ulClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            io_pProfProps->ulProfilerState = pClient->GetState();

            // Update the status for the user client.
            io_pProfProps->ulStatus = PROF_SUCCESS;

            FinalizeSuccessfulRequest(pIrp, io_pProfProps);
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", io_pProfProps->ulClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


NTSTATUS IoctlSetOutputFileHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    OUTPUT_FILE_DESCRIPTOR* io_pFileDescriptor;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pFileDescriptor);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(io_pFileDescriptor->ulClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            io_pFileDescriptor->ulStatus = static_cast<ULONG>(PROF_ERROR);
            FinalizeSuccessfulRequest(pIrp, io_pFileDescriptor);

            //  Simple check to make sure user sent down valid address and a size
            if (TRUE == MmIsAddressValid(reinterpret_cast<PVOID>(io_pFileDescriptor->uliPathName.QuadPart))       &&
                0UL < io_pFileDescriptor->ulPathSize   && io_pFileDescriptor->ulPathSize   < _MAX_PATH            &&
                TRUE == MmIsAddressValid(reinterpret_cast<PVOID>(io_pFileDescriptor->uliTempTiPathName.QuadPart)) &&
                0UL < io_pFileDescriptor->ulTempTiSize && io_pFileDescriptor->ulTempTiSize < _MAX_PATH)
            {
                // Not currently profiling.
                if (!pClient->IsStarted())
                {
                    // Output file is not set yet.
                    if (!pClient->IsOutputFileSet())
                    {
                        PCWSTR pPrdFilePath = reinterpret_cast<PCWSTR>(io_pFileDescriptor->uliPathName.QuadPart);
                        PCWSTR pTiFilePath = reinterpret_cast<PCWSTR>(io_pFileDescriptor->uliTempTiPathName.QuadPart);
                        size_t prdLength = 0, tiLength = 0;

                        __try
                        {
                            RtlStringCchLengthW(pPrdFilePath, _MAX_PATH, &prdLength);
                            RtlStringCchLengthW(pTiFilePath,  _MAX_PATH, &tiLength);
                        }
                        __except (EXCEPTION_EXECUTE_HANDLER)
                        {
                        }

                        if (io_pFileDescriptor->ulPathSize   == static_cast<ULONG>(prdLength) &&
                            io_pFileDescriptor->ulTempTiSize == static_cast<ULONG>(tiLength))
                        {
                            if (pClient->SetOutputFile(pPrdFilePath, static_cast<ULONG>(prdLength),
                                                       pTiFilePath,  static_cast<ULONG>(tiLength)))
                            {
                                // Update the status for the user client.
                                io_pFileDescriptor->ulStatus = PROF_SUCCESS;
                            }
                            else
                            {
                                status = STATUS_FILE_INVALID;
                            }
                        }
                        else
                        {
                            PrintError("Provided string size doesn't match actual string length!");
                            status = STATUS_INVALID_PARAMETER;
                        }
                    }
                    else
                    {
                        status = STATUS_ALREADY_COMMITTED;
                    }
                }
                else
                {
                    status = STATUS_DEVICE_BUSY;
                }
            }
            else
            {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", io_pFileDescriptor->ulClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Return the output file name previously set by the call to set an output file.
NTSTATUS IoctlGetOutputFileHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    OUTPUT_FILE_DESCRIPTOR* io_pFileDescriptor;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pFileDescriptor);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(io_pFileDescriptor->ulClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            if (!pClient->GetOutputFile(reinterpret_cast<wchar_t*>(io_pFileDescriptor->uliPathName.QuadPart),
                                        io_pFileDescriptor->ulPathSize,
                                        reinterpret_cast<wchar_t*>(io_pFileDescriptor->uliTempTiPathName.QuadPart),
                                        io_pFileDescriptor->ulTempTiSize))
            {
                // Output file was not previously set.
                if (!pClient->IsOutputFileSet())
                {
                    status = STATUS_NO_SUCH_FILE;
                }
                else
                {
                    status = STATUS_INVALID_USER_BUFFER;
                }
            }

            // Update the status for the user client.
            io_pFileDescriptor->ulStatus = PROF_SUCCESS;

            FinalizeSuccessfulRequest(pIrp, io_pFileDescriptor);
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", io_pFileDescriptor->ulClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


NTSTATUS IoctlGetRecordCountHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    const ULONG* i_pClientId;
    ULONG* o_pSamplesCount;
    NTSTATUS status = GetInputOutputBuffer(pIrp, i_pClientId, o_pSamplesCount);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(*i_pClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            *o_pSamplesCount = pClient->GetSamplesCount();

            FinalizeSuccessfulRequest(pIrp, o_pSamplesCount);
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", *i_pClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Allow the user to configure callstack sampling for a target application.
NTSTATUS IoclSetCSSPropertiesHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    CSS_PROPERTIES* io_pCssProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pCssProps);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(io_pCssProps->ulClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            status = pClient->SetCssConfiguration(*io_pCssProps);

            // Update the status for the user client.
            io_pCssProps->ulStatus = PROF_SUCCESS;

            FinalizeSuccessfulRequest(pIrp, io_pCssProps);
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", io_pCssProps->ulClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


NTSTATUS IoclGetCSSPropertiesHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    CSS_PROPERTIES* io_pCssProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pCssProps);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(io_pCssProps->ulClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            status = pClient->GetCssConfiguration(*io_pCssProps);

            // Update the status for the user client.
            io_pCssProps->ulStatus = PROF_SUCCESS;

            FinalizeSuccessfulRequest(pIrp, io_pCssProps);
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", io_pCssProps->ulClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Configure IBS profiling
NTSTATUS IoctlSetIbsPropertiesHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    IBS_PROPERTIES* io_pIbsProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pIbsProps);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(io_pIbsProps->ulClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            io_pIbsProps->ulStatus = static_cast<ULONG>(PROF_ERROR);
            FinalizeSuccessfulRequest(pIrp, io_pIbsProps);

            if (IsCoreMaskValid(reinterpret_cast<ULONG64*>(io_pIbsProps->ullCpuMask.QuadPart), io_pIbsProps->ulCoreMaskCount))
            {
                status = pClient->SetIbsConfiguration(*io_pIbsProps);

                // Update the status for the user client.
                if (STATUS_SUCCESS == status)
                {
                    io_pIbsProps->ulStatus = PROF_SUCCESS;
                }
            }
            else
            {
                PrintError("Core mask array validation failed (cores count %u)!", CpuProf::GetCoresCount());
                status = STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", io_pIbsProps->ulClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Get the IBS configuration for the profile
NTSTATUS IoctlGetIbsPropertiesHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    IBS_PROPERTIES* io_pIbsProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pIbsProps);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(io_pIbsProps->ulClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            status = pClient->GetIbsConfiguration(*io_pIbsProps);

            // Update the status for the user client.
            io_pIbsProps->ulStatus = PROF_SUCCESS;

            FinalizeSuccessfulRequest(pIrp, io_pIbsProps);
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", io_pIbsProps->ulClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Configure PID filtering for target applications.
NTSTATUS IoctlSetPIDPropertiesHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    PID_PROPERTIES* io_pPidProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pPidProps);

    if (STATUS_SUCCESS == status)
    {
        // Since the array is 0-terminated, we check that it is not an empty array.
        if (0ULL != io_pPidProps->ullPidArray[0])
        {
            CpuProf::Client* pClient = pDevExt->GetClient(io_pPidProps->ulClientId);

            if (NULL != pClient && pClient->IsValid())
            {
                io_pPidProps->ulStatus = static_cast<ULONG>(PROF_ERROR);
                FinalizeSuccessfulRequest(pIrp, io_pPidProps);

                // If not currently profiling.
                if (!pClient->IsStarted())
                {
                    // If PID filter is not set yet.
                    if (pClient->IsSystemWide())
                    {
                        // Output file should be already set.
                        if (pClient->IsOutputFileSet())
                        {
                            if (pClient->InitializeAttachedProcesses(io_pPidProps->ullPidArray))
                            {
                                pClient->SetAutoAttachToChildProcesses(FALSE != io_pPidProps->bAddChildrenToFilter);

                                // Update the status for the user client.
                                io_pPidProps->ulStatus = PROF_SUCCESS;
                            }
                            else
                            {
                                status = STATUS_ALREADY_COMMITTED;
                            }
                        }
                        else
                        {
                            status = STATUS_NO_SUCH_FILE;
                        }
                    }
                    else
                    {
                        status = STATUS_ALREADY_COMMITTED;
                    }
                }
                else
                {
                    status = STATUS_DEVICE_BUSY;
                }
            }
            else
            {
                PrintError("IOCTL request with invliad client ID (%u).", io_pPidProps->ulClientId);
                status = STATUS_ACCESS_DENIED;
            }
        }
        else
        {
            status = STATUS_INVALID_PARAMETER;
        }
    }

    return status;
}


// Get the previously configured PID filtering
NTSTATUS IoctlGetPIDPropertiesHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    PID_PROPERTIES* io_pPidProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pPidProps);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(io_pPidProps->ulClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            // If PID filter is already set.
            if (!pClient->IsSystemWide())
            {
                // Initialize the structure to zero.
                RtlZeroMemory(io_pPidProps, sizeof(PID_PROPERTIES));

                ULONG count = 0UL;
                const HANDLE* pPidList = pClient->GetAttachedProcesses(count);

                for (ULONG i = 0UL; i < count; ++i)
                {
                    io_pPidProps->ullPidArray[i] = reinterpret_cast<ULONG64>(pPidList[i]);
                }

                io_pPidProps->bAddChildrenToFilter = pClient->IsAutoAttachToChildProcessesEnabled();

                // Update the status for the user client.
                io_pPidProps->ulStatus = PROF_SUCCESS;

                FinalizeSuccessfulRequest(pIrp, io_pPidProps);
            }
            else
            {
                status = STATUS_DEVICE_NOT_READY;
            }
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", io_pPidProps->ulClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Get the current value of the counting event configuration during a profile
NTSTATUS IoctlGetEventCountHandler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    COUNT_PROPERTIES* io_pCountProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, io_pCountProps);

    if (STATUS_SUCCESS == status)
    {
        if (io_pCountProps->ulCounterIndex < pDevExt->GetResourceCount(EVENT_CTR) ||
            io_pCountProps->ulCounterIndex < pDevExt->GetResourceCount(NB_CTR) ||
            io_pCountProps->ulCounterIndex < pDevExt->GetResourceCount(L2I_CTR))
        {
            if (io_pCountProps->ullCore < CpuProf::GetCoresCount())
            {
                CpuProf::Client* pClient = pDevExt->GetClient(io_pCountProps->ulClientId);

                if (NULL != pClient && pClient->IsValid())
                {
                    status = pClient->ReadCountingEvent(static_cast<ULONG>(io_pCountProps->ullCore),
                                                        io_pCountProps->ulCounterIndex,
                                                        io_pCountProps->ullEventCfg,
                                                        io_pCountProps->ullEventCount);

                    // Update the status for the user client.
                    if (STATUS_SUCCESS == status)
                    {
                        io_pCountProps->ulStatus = PROF_SUCCESS;
                    }
                    else
                    {
                        io_pCountProps->ulStatus = static_cast<ULONG>(PROF_ERROR);
                    }

                    FinalizeSuccessfulRequest(pIrp, io_pCountProps);
                }
                else
                {
                    PrintError("IOCTL request with invliad client ID (%u).", io_pCountProps->ulClientId);
                    status = STATUS_ACCESS_DENIED;
                }
            }
            else
            {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return status;
}


// Clear all previous settings for the profile
NTSTATUS IoctlClearProfiler(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    const ULONG* i_pClientId;
    ULONG* o_pStatus;
    NTSTATUS status = GetInputOutputBuffer(pIrp, i_pClientId, o_pStatus);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(*i_pClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            // If not currently profiling.
            if (!pClient->IsStarted())
            {
                pClient->Clear();

                *o_pStatus = PROF_SUCCESS;
                FinalizeSuccessfulRequest(pIrp, o_pStatus);
            }
            else
            {
                status = STATUS_DEVICE_BUSY;
            }
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", *i_pClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


// Get the overhead spent during the last profile (in approximate cycle count duration).
NTSTATUS IoctlGetOverhead(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);

    const ULONG* i_pClientId;
    OVERHEAD_PROPERTIES* o_pOverheadProps;
    NTSTATUS status = GetInputOutputBuffer(pIrp, i_pClientId, o_pOverheadProps);

    if (STATUS_SUCCESS == status)
    {
        CpuProf::Client* pClient = pDevExt->GetClient(*i_pClientId);

        if (NULL != pClient && pClient->IsValid())
        {
            if (!pClient->IsStarted())
            {
                o_pOverheadProps->reaperOverhead = pClient->GetPrdWriter().GetReaperOverhead();
                pClient->GetPrdWriter().ClearReaperOverhead();

                o_pOverheadProps->pcoreSchedulingOverhead = pClient->GetSamplingOverhead();
                pClient->ClearSamplingOverhead();

                o_pOverheadProps->pcoreInterruptOverhead = 0;

                //accumulate the cycle data from each core
                for (ULONG core = 0UL, coresCount = CpuProf::Device::GetCoresCount(); core < coresCount; ++core)
                {
                    // Note that this also resets Pcore's overhead counter.
                    o_pOverheadProps->pcoreSchedulingOverhead += PcoreTestGetSchedulerOverhead(core);
                    o_pOverheadProps->pcoreInterruptOverhead += PcoreTestGetInterruptOverhead(core);
                }

                // Update the status for the user client.
                o_pOverheadProps->ulStatus = PROF_SUCCESS;

                FinalizeSuccessfulRequest(pIrp, o_pOverheadProps);
            }
            else
            {
                status = STATUS_DEVICE_BUSY;
            }
        }
        else
        {
            PrintError("IOCTL request with invliad client ID (%u).", *i_pClientId);
            status = STATUS_ACCESS_DENIED;
        }
    }

    return status;
}


NTSTATUS IoctlGetAvailability(IN CpuProf::Device* pDevExt, IN PIRP pIrp, IN PIO_STACK_LOCATION pIrpStack)
{
    INIT_IOCTL_REQUEST(pIrp, pIrpStack);
    UNREFERENCED_PARAMETER(pDevExt);

    RESOURCE_AVAILABILITY* o_pAvailability;
    NTSTATUS status = GetOutputBuffer(pIrp, o_pAvailability);

    if (STATUS_SUCCESS == status)
    {
        ULONG mask;

        mask = 0UL;
        PcoreGetResourceAvailability(EVENT_CTR, 0UL, &mask);
        o_pAvailability->pmcAvailable = mask;

        mask = 0UL;
        PcoreGetResourceAvailability(NB_CTR, 0UL, &mask);
        o_pAvailability->nbAvailable = mask;

        mask = 0UL;
        PcoreGetResourceAvailability(L2I_CTR, 0UL, &mask);
        o_pAvailability->l2iAvailable = mask;

        // Since we allocate the events across all cores, this provides a system-wide availability.
        for (ULONG core = 1UL, coresCount = CpuProf::Device::GetCoresCount(); core < coresCount; ++core)
        {
            if (S_OK == PcoreGetResourceAvailability(EVENT_CTR, core, &mask))
            {
                o_pAvailability->pmcAvailable &= mask;
            }

            if (S_OK == PcoreGetResourceAvailability(NB_CTR, core, &mask))
            {
                o_pAvailability->nbAvailable &= mask;
            }

            if (S_OK == PcoreGetResourceAvailability(L2I_CTR, core, &mask))
            {
                o_pAvailability->l2iAvailable &= mask;
            }
        }

        FinalizeSuccessfulRequest(pIrp, o_pAvailability);
    }

    return status;
}
