//=============================================================
// (c) 2012 Advanced Micro Devices, Inc.
//
/// \author franksw
/// \version $Revision: #4 $
/// \brief Contains the functions associated with performing TaskInfo operations
//
//=============================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/Driver/Windows/CpuProf/src/CpuProfSystemCallbacks.cpp#4 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=============================================================

#include "..\inc\CpuProfDevice.hpp"
#include "..\inc\CpuProfSystemCallbacks.hpp"
#include <WinDriverStackWalker\Include\StackWalker.hpp>
#include <WinDriverUtils\Include\WorkItem.hpp>


namespace CpuProf
{


static void EnqueueWriteCreateProcessInfo(ULONG64 time, ULONG core, HANDLE hProcessId, BOOLEAN is32Bit, BOOLEAN bCreate);
static void EnqueueWriteCreateThreadInfo(ULONG64 time, ULONG core, HANDLE hProcessId, HANDLE hThreadId, BOOLEAN bCreate);
static void EnqueueWriteLoadImageInfo(TASK_INFO_RECORD tiRecord, ULONG pathBytes, FILE_OBJECT* pFileObject);


// Notify each valid client of the created process information
VOID CreateProcessCallback(IN HANDLE hParentId, IN HANDLE hProcessId, IN BOOLEAN bCreate)
{
    LARGE_INTEGER time = KeQueryPerformanceCounter(NULL);
    ULONG core = KeGetCurrentProcessorNumberEx(NULL);
    Device* pDevice = Device::GetInstance();
    BOOLEAN is32Bit = TRUE;
    ULONG clientsCount = 0UL;
    Client* pValidClient = NULL;

    StackWalker* pStackWalker = NULL;
    bool paramStackWalker = false;

    if (bCreate)
    {
#ifdef _AMD64_
        is32Bit = IoIs32bitProcess(NULL);
#endif

        // For each client.
        for (MaskIterator itClientId = pDevice->GetRegisteredClientsMask(); !itClientId.IsEmpty(); ++itClientId)
        {
            Client* pClient = GetClient(*itClientId);

            // If the client is valid.
            if (NULL != pClient && pClient->IsValid())
            {
                pClient->ProcessCreatedCallback(hParentId, hProcessId, core, pStackWalker, paramStackWalker);

                clientsCount++;
                pValidClient = pClient;
            }
        }
    }
    else
    {
        // For each client.
        for (MaskIterator itClientId = pDevice->GetRegisteredClientsMask(); !itClientId.IsEmpty(); ++itClientId)
        {
            Client* pClient = GetClient(*itClientId);

            // If the client is valid.
            if (NULL != pClient && pClient->IsValid())
            {
                pClient->ProcessDestroyedCallback(hProcessId, pStackWalker, paramStackWalker);

                clientsCount++;
                pValidClient = pClient;
            }
        }
    }

    if (1UL == clientsCount)
    {
        EnqueueWorkItem(pDevice->GetDeviceObject(),
                        DelayedWorkQueue,
                        Client::WriteCreateProcessInfo,
                        pValidClient, static_cast<ULONG64>(time.QuadPart), core, hProcessId, is32Bit, bCreate);
    }
    else if (0UL != clientsCount)
    {
        EnqueueWorkItem(pDevice->GetDeviceObject(),
                        DelayedWorkQueue,
                        EnqueueWriteCreateProcessInfo,
                        static_cast<ULONG64>(time.QuadPart), core, hProcessId, is32Bit, bCreate);
    }
}


static void EnqueueWriteCreateProcessInfo(ULONG64 time, ULONG core, HANDLE hProcessId, BOOLEAN is32Bit, BOOLEAN bCreate)
{
    Device* pDevice = Device::GetInstance();
    DEVICE_OBJECT* pDeviceObject = pDevice->GetDeviceObject();

    // For each client.
    for (MaskIterator itClientId = pDevice->GetRegisteredClientsMask(); !itClientId.IsEmpty(); ++itClientId)
    {
        Client* pClient = GetClient(*itClientId);

        // If the client is valid.
        if (NULL != pClient && pClient->IsValid())
        {
            EnqueueWorkItem(pDeviceObject,
                            DelayedWorkQueue,
                            Client::WriteCreateProcessInfo,
                            pClient, time, core, hProcessId, is32Bit, bCreate);
        }
    }
}


void Client::WriteCreateProcessInfo(Client* pClient, ULONG64 time, ULONG core, HANDLE hProcessId, BOOLEAN is32Bit, BOOLEAN bCreate)
{
    ASSERT(NULL != pClient);

    if (pClient->IsValid() && pClient->m_tiWriter.IsOpened())
    {
        TASK_INFO_RECORD tiRecord;
        RtlZeroMemory(&tiRecord, sizeof(tiRecord));

        tiRecord.m_RecordType = static_cast<UCHAR>(bCreate ? MONITOR_PROC_CREATE : MONITOR_PROC_DELETE);
        tiRecord.m_Core = static_cast<UCHAR>(core);
        tiRecord.m_ProcessHandle = reinterpret_cast<ULONG64>(hProcessId);

#ifdef _AMD64_

        if (!is32Bit)
        {
            tiRecord.m_Size = 64;
        }

#else
        UNREFERENCED_PARAMETER(is32Bit);
#endif

        tiRecord.m_TickStamp = time - pClient->m_startTime;

        if (!pClient->m_tiWriter.WriteRecord(tiRecord))
        {
            PrintError("Failed to write record to TI file.");
            pClient->SetLastErrorCode(PROF_FILE_WRITE_ERROR);
        }
    }
}


// Notify each valid client of the loaded image information
VOID LoadImageCallback(IN PUNICODE_STRING pFullImageName, IN HANDLE hProcessId, IN PIMAGE_INFO pImageInfo)
{
    LARGE_INTEGER time = KeQueryPerformanceCounter(NULL);
    Device* pDevice = Device::GetInstance();

    StackWalker* pStackWalker = pDevice->GetStackTraceDispatcher().FindStackWalker(hProcessId);

    if (NULL != pStackWalker)
    {
        pStackWalker->AddModuleInfo(reinterpret_cast<ULONG_PTR>(pImageInfo->ImageBase), static_cast<ULONG>(pImageInfo->ImageSize));
        pStackWalker->Release();
    }

    TASK_INFO_RECORD tiRecord;
    RtlZeroMemory(&tiRecord, sizeof(tiRecord));

    tiRecord.m_Core = static_cast<UCHAR>(KeGetCurrentProcessorNumberEx(NULL));
    tiRecord.m_ProcessHandle = reinterpret_cast<ULONG64>(hProcessId);
    tiRecord.m_RecordType = static_cast<UCHAR>(MONITOR_IMAGE_LOAD);
    tiRecord.m_StartAddress = reinterpret_cast<ULONG64>(pImageInfo->ImageBase);
    tiRecord.m_Size = static_cast<ULONG64>(pImageInfo->ImageSize);
    tiRecord.m_TickStamp = static_cast<ULONG64>(time.QuadPart);

    ULONG pathBufferLen = sizeof(tiRecord.m_PathName) - sizeof(WCHAR);

    if (pFullImageName->Length < pathBufferLen)
    {
        pathBufferLen = pFullImageName->Length;
    }

    // Module name in UNICODE format, may not have complete path information.
    RtlCopyMemory(tiRecord.m_PathName, pFullImageName->Buffer, pathBufferLen);

    //
    // We cannot call IoQueryFileDosDeviceName from within this function,
    // as it will result with a BSOD for Windows 7.
    // As quoted from the description of PsSetLoadImageNotifyRoutine in MSDN:
    //
    // "In Windows 7, Windows Server 2008 R2, and earlier versions of Windows,
    //  the operating system holds an internal system lock during calls to
    //  load-image notify routines for images loaded in user process address space (user space).
    //  To avoid deadlocks, load-image notify routines must not call system routines that map,
    //  allocate, query, free, or perform other operations on user-space virtual memory."
    //

    FILE_OBJECT* pFileObject = NULL;

    if (pImageInfo->ExtendedInfoPresent)
    {
        IMAGE_INFO_EX* pImageInfoEx = CONTAINING_RECORD(pImageInfo, IMAGE_INFO_EX, ImageInfo);

        if (NULL != pImageInfoEx->FileObject)
        {
            pFileObject = pImageInfoEx->FileObject;
            ObReferenceObject(pFileObject);
        }
    }

    if (!EnqueueWorkItem(pDevice->GetDeviceObject(),
                         DelayedWorkQueue,
                         EnqueueWriteLoadImageInfo,
                         tiRecord,
                         pathBufferLen,
                         pFileObject))
    {
        if (NULL != pFileObject)
        {
            ObDereferenceObject(pFileObject);
        }
    }
}


static void EnqueueWriteLoadImageInfo(TASK_INFO_RECORD tiRecord, ULONG pathBytes, FILE_OBJECT* pFileObject)
{
    Device* pDevice = Device::GetInstance();
    DEVICE_OBJECT* pDeviceObject = pDevice->GetDeviceObject();

    ULONG pathBufferLen = sizeof(tiRecord.m_PathName) - sizeof(WCHAR);
    HANDLE imageFileHandle = NULL;
    bool foundDosName = false;

    if (NULL != pFileObject)
    {
        POBJECT_NAME_INFORMATION pDosFileName;

        // If we managed to get the dos name.
        if (STATUS_SUCCESS == IoQueryFileDosDeviceName(pFileObject, &pDosFileName))
        {
            if (pDosFileName->Name.Length < pathBufferLen)
            {
                pathBufferLen = pDosFileName->Name.Length;
            }

            RtlCopyMemory(tiRecord.m_PathName, pDosFileName->Name.Buffer, pathBufferLen);
            ExFreePool(pDosFileName);
            foundDosName = true;
        }

        ObDereferenceObject(pFileObject);
    }

    if (!foundDosName)
    {
        UNICODE_STRING fullImageName;
        fullImageName.Length = static_cast<USHORT>(pathBytes);
        fullImageName.MaximumLength = static_cast<USHORT>(pathBufferLen);
        fullImageName.Buffer = tiRecord.m_PathName;

        OBJECT_ATTRIBUTES objAttrs;
        // No error code returned according to DDK doc
        InitializeObjectAttributes(&objAttrs, &fullImageName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

        IO_STATUS_BLOCK ioStatus;

        // Try to get a reference to the image file object.
        if (STATUS_SUCCESS == ZwOpenFile(&imageFileHandle, SYNCHRONIZE, &objAttrs, &ioStatus, FILE_SHARE_READ, FILE_NON_DIRECTORY_FILE))
        {
            PFILE_OBJECT pFileObject;

            if (STATUS_SUCCESS == ObReferenceObjectByHandle(imageFileHandle,
                                                            0,
                                                            NULL,
                                                            KernelMode,
                                                            reinterpret_cast<PVOID*>(&pFileObject),
                                                            NULL))
            {
                POBJECT_NAME_INFORMATION pDosFileName;

                // If we managed to get the dos name.
                if (STATUS_SUCCESS == IoQueryFileDosDeviceName(pFileObject, &pDosFileName))
                {
                    if (pDosFileName->Name.Length < pathBufferLen)
                    {
                        pathBufferLen = pDosFileName->Name.Length;
                    }

                    RtlCopyMemory(tiRecord.m_PathName, pDosFileName->Name.Buffer, pathBufferLen);
                    ExFreePool(pDosFileName);
                    foundDosName = true;
                }

                ObDereferenceObject(pFileObject);
            }

            if (!foundDosName)
            {
                UCHAR buffer[sizeof(FILE_FS_VOLUME_INFORMATION) + MAXIMUM_VOLUME_LABEL_LENGTH];
                FILE_FS_VOLUME_INFORMATION* pFileInfo = reinterpret_cast<FILE_FS_VOLUME_INFORMATION*>(buffer);

                if (STATUS_SUCCESS == ZwQueryVolumeInformationFile(imageFileHandle,
                                                                   &ioStatus,
                                                                   reinterpret_cast<PVOID>(pFileInfo),
                                                                   sizeof(buffer),
                                                                   FileFsVolumeInformation))
                {
                    ULONG requiredLen = (sizeof(wchar_t) + pFileInfo->VolumeLabelLength + pathBytes);

                    if (requiredLen <= pathBufferLen)
                    {
                        RtlMoveMemory(reinterpret_cast<UCHAR*>(tiRecord.m_PathName + 1) + pFileInfo->VolumeLabelLength, tiRecord.m_PathName, pathBytes);
                        RtlCopyMemory(tiRecord.m_PathName + 1, pFileInfo->VolumeLabel, pFileInfo->VolumeLabelLength);
                        tiRecord.m_PathName[0] = L'\\';
                        pathBufferLen = requiredLen;
                        foundDosName = true;
                    }
                }
            }

            ZwClose(imageFileHandle);
        }
    }

    *reinterpret_cast<wchar_t*>(reinterpret_cast<UCHAR*>(tiRecord.m_PathName) + pathBufferLen) = L'\0';

    // For each client.
    for (MaskIterator itClientId = pDevice->GetRegisteredClientsMask(); !itClientId.IsEmpty(); ++itClientId)
    {
        Client* pClient = GetClient(*itClientId);

        // If the client is valid.
        if (NULL != pClient && pClient->IsValid())
        {
            EnqueueWorkItem(pDeviceObject,
                            DelayedWorkQueue,
                            Client::WriteLoadImageInfo,
                            pClient, tiRecord);
        }
    }
}


void Client::WriteLoadImageInfo(Client* pClient, TASK_INFO_RECORD tiRecord)
{
    ASSERT(NULL != pClient);

    if (pClient->IsValid() && pClient->m_tiWriter.IsOpened())
    {
        tiRecord.m_TickStamp -= pClient->m_startTime;

        if (!pClient->m_tiWriter.WriteRecord(tiRecord))
        {
            PrintError("Failed to write record to TI file.");
            pClient->SetLastErrorCode(PROF_FILE_WRITE_ERROR);
        }
    }
}


// Notify each valid client of the thread information
VOID CreateThreadCallback(IN HANDLE hProcessId, IN HANDLE hThreadId, IN BOOLEAN bCreate)
{
    LARGE_INTEGER time = KeQueryPerformanceCounter(NULL);
    ULONG core = KeGetCurrentProcessorNumberEx(NULL);

    EnqueueWorkItem(Device::GetInstance()->GetDeviceObject(),
                    DelayedWorkQueue,
                    EnqueueWriteCreateThreadInfo,
                    static_cast<ULONG64>(time.QuadPart), core, hProcessId, hThreadId, bCreate);
}


static void EnqueueWriteCreateThreadInfo(ULONG64 time, ULONG core, HANDLE hProcessId, HANDLE hThreadId, BOOLEAN bCreate)
{
    Device* pDevice = Device::GetInstance();
    DEVICE_OBJECT* pDeviceObject = pDevice->GetDeviceObject();

    // For each client.
    for (MaskIterator itClientId = pDevice->GetRegisteredClientsMask(); !itClientId.IsEmpty(); ++itClientId)
    {
        Client* pClient = GetClient(*itClientId);

        // If the client is valid.
        if (NULL != pClient && pClient->IsValid())
        {
            EnqueueWorkItem(pDeviceObject,
                            DelayedWorkQueue,
                            Client::WriteCreateThreadInfo,
                            pClient, time, core, hProcessId, hThreadId, bCreate);
        }
    }
}


void Client::WriteCreateThreadInfo(Client* pClient, ULONG64 time, ULONG core, HANDLE hProcessId, HANDLE hThreadId, BOOLEAN bCreate)
{
    ASSERT(NULL != pClient);

    if (pClient->IsValid() && pClient->m_tiWriter.IsOpened())
    {
        TASK_INFO_RECORD tiRecord;
        RtlZeroMemory(&tiRecord, sizeof(tiRecord));

        tiRecord.m_RecordType = static_cast<UCHAR>(bCreate ? MONITOR_THREAD_CREATE : MONITOR_THREAD_DELETE);
        tiRecord.m_Core = static_cast<UCHAR>(core);
        tiRecord.m_ProcessHandle = reinterpret_cast<ULONG64>(hProcessId);
        tiRecord.m_StartAddress = reinterpret_cast<ULONG64>(hThreadId);
        tiRecord.m_TickStamp = time - pClient->m_startTime;

        if (!pClient->m_tiWriter.WriteRecord(tiRecord))
        {
            PrintError("Failed to write record to TI file.");
            pClient->SetLastErrorCode(PROF_FILE_WRITE_ERROR);
        }
    }
}

} // namespace CpuProf
