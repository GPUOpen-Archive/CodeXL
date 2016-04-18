#include "..\inc\CpuProfDevice.hpp"
#include "..\inc\CpuProfSystemCallbacks.hpp"
#include <WinDriverUtils\Include\Cpuid.h>

namespace CpuProf
{

static void RegisterCallbacks();
static void UnregisterCallbacks();

Device* Device::s_pInstance = NULL;
ULONG Device::s_coresCount = 0UL;


NTSTATUS Device::Create(DEVICE_OBJECT* pDeviceObject, ULONG coresCount)
{
    NTSTATUS status;

    if (NULL == s_pInstance)
    {
        status = STATUS_SUCCESS;

        // This must come first, as we use this value from other locations.
        s_coresCount = coresCount;

        s_pInstance = reinterpret_cast<Device*>(pDeviceObject->DeviceExtension);
        ::new(s_pInstance) Device(pDeviceObject);

        for (ULONG id = 0; id < MAX_CLIENT_COUNT; ++id)
        {
            if (!s_pInstance->GetClient(id)->GetCssConfiguration().IsValid())
            {
                status = STATUS_NO_MEMORY;
                break;
            }
        }


        if (STATUS_SUCCESS == status)
        {
            // Setting up default Shared Object for the global, per-client shared pause.
            status = s_pInstance->CreateUserKernelSharedMap();

            if (STATUS_SUCCESS == status)
            {
                if (!s_pInstance->m_stackTraceDispatcher.Initialize(Client::UserStackBackTraceCompleteCallback, MAX_CSS_VALUES))
                {
                    status = STATUS_NO_MEMORY;
                    PrintError("Failed to initialize the Stack-trace dispatcher.");
                }
            }
            else
            {
                PrintError("Failed to create the shared map.");
            }
        }

        if (STATUS_SUCCESS != status)
        {
            Destroy();
        }
    }
    else
    {
        PrintError("Device already exists!");
        status = STATUS_INVALID_PARAMETER;
    }

    return status;
}


void Device::Destroy()
{
    if (NULL != s_pInstance)
    {
        s_pInstance->~Device();
        s_pInstance = NULL;
    }
    else
    {
        PrintWarning("Device does not exist!");
    }
}


Device::Device(DEVICE_OBJECT* pDeviceObject) : m_pDeviceObject(pDeviceObject),
    m_clientsCount(0UL),
    m_maxResources(0UL),
    m_hasIbsBrnTrgt(false),
    m_isIbsOpCntExt(false),
    m_pSharedObjSystemAddress(NULL),
    m_pSharedObjMdl(NULL),
    m_pSharedObjBaseAddress(NULL),
    m_sharedObjSectionHandle(NULL)
{
    // Fill in the count of each resource type.
    for (int type = 0; type < MAX_RESOURCE_TYPE; type++)
    {
        m_resourceCounts[type] = 0UL;
        PcoreGetResourceCount(static_cast<PCORERESOURCETYPES>(type), &m_resourceCounts[type]);

        if (m_maxResources < m_resourceCounts[type])
        {
            m_maxResources = m_resourceCounts[type];
        }
    }

    int aCPUInfo[NUM_CPUID_OFFSETS] = { -1 };

    // Check whether CPUID is available.
    __cpuid(aCPUInfo, CPUID_FnVendorIdentification);

    if (aCPUInfo[EAX_OFFSET] > 0)
    {
        // Read the CPUID extended feature bits.
        __cpuid(aCPUInfo, CPUID_FnAmdExtendedFeatures);

        // If IBS sampling is available.
        if (0 != (aCPUInfo[ECX_OFFSET] & CPUID_FnAmdExtendedFeatures_ECX_IBS))
        {
            //Instruction Based Sampling Identifiers
            __cpuid(aCPUInfo, CPUID_FnIbsIdentifiers);

            // This bit (BrnTrgt: branch target address reporting supported) will be 1 for GH and beyond
            m_hasIbsBrnTrgt = ((aCPUInfo[EAX_OFFSET] & CPUID_FnIbsIdentifiers_EAX_BrnTrgt) != 0);

            // This bit (OpCntExt: branch target address reporting supported) will be 1 for Llano and beyond
            m_isIbsOpCntExt = ((aCPUInfo[EAX_OFFSET] & CPUID_FnIbsIdentifiers_EAX_OpCntExt) != 0);
        }
    }
}


Device::~Device()
{
    DeleteUserKernelSharedMap();
}


// Open a shared memory area between application and driver mode for sharing pause state
NTSTATUS Device::CreateUserKernelSharedMap()
{
    // Convert the constant string to the UNICODE_STRING.
    UNICODE_STRING usSharedMemoryObjName;
    RtlInitUnicodeString(&usSharedMemoryObjName, CPU_PROF_SHARED_OBJ_BASE);

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
        m_sharedObjSectionHandle = sectionHandle;

        PVOID pBaseAddress = NULL;
        SIZE_T viewSize = 0;
        status = ZwMapViewOfSection(sectionHandle,          // section handle
                                    NtCurrentProcess(),     // current process
                                    &pBaseAddress,          // virtual based address
                                    0,                      // Zero bits
                                    CPU_PROF_SHARED_MEM_SIZE,
                                    0,                      // optional
                                    &viewSize,              // How much to map
                                    ViewShare,              // Inherit disposition
                                    0,                      // ALlocation Type
                                    PAGE_READWRITE);        // protection

        if (STATUS_SUCCESS == status)
        {
            //save the base address to free it later
            m_pSharedObjBaseAddress = pBaseAddress;
            //Allocate the length of ViewSize in bytes
            PMDL pMdl = IoAllocateMdl(pBaseAddress, static_cast<ULONG>(viewSize), FALSE, TRUE, NULL);

            if (NULL != pMdl)
            {
                __try
                {
                    MmProbeAndLockPages(pMdl, KernelMode, IoReadAccess);
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                    status = STATUS_NO_MEMORY;
                }


                if (STATUS_SUCCESS == status)
                {
                    m_pSharedObjSystemAddress = static_cast<SHARED_CLIENT*>(MmGetSystemAddressForMdlSafe(pMdl, HighPagePriority));

                    if (NULL != m_pSharedObjSystemAddress)
                    {
                        m_pSharedObjMdl = pMdl;

                        // Set the initial state as not paused.
                        for (int i = 0; i < MAX_CLIENT_COUNT; ++i)
                        {
                            m_pSharedObjSystemAddress[i].paused = FALSE;
                        }
                    }
                    else
                    {
                        MmUnlockPages(pMdl);
                        IoFreeMdl(pMdl);
                        ZwUnmapViewOfSection(NtCurrentProcess(), m_pSharedObjBaseAddress);
                        ZwClose(m_sharedObjSectionHandle);
                        status = STATUS_NO_MEMORY;
                    }
                }
                else
                {
                    IoFreeMdl(pMdl);
                    ZwUnmapViewOfSection(NtCurrentProcess(), m_pSharedObjBaseAddress);
                    ZwClose(m_sharedObjSectionHandle);
                }
            }
            else
            {
                ZwUnmapViewOfSection(NtCurrentProcess(), m_pSharedObjBaseAddress);
                ZwClose(m_sharedObjSectionHandle);
                status = STATUS_NO_MEMORY;
            }
        }
        else
        {
            ZwClose(m_sharedObjSectionHandle);
        }
    }

    return status;
}


// Free a shared memory area between application and driver mode for sharing pause info.
NTSTATUS Device::DeleteUserKernelSharedMap()
{
    NTSTATUS status;

    if (NULL != m_pSharedObjSystemAddress)
    {
        MmUnlockPages(m_pSharedObjMdl);
        IoFreeMdl(m_pSharedObjMdl);
        status = ZwUnmapViewOfSection(NtCurrentProcess(), m_pSharedObjBaseAddress);
        ZwClose(m_sharedObjSectionHandle);
    }
    else
    {
        status = STATUS_SUCCESS;
    }

    return status;
}


const Client* Device::GetClient(ULONG clientId) const
{
    return (clientId < MAX_CLIENT_COUNT) ? &m_clients[clientId] : NULL;
}


Client* Device::GetClient(ULONG clientId)
{
    return (clientId < MAX_CLIENT_COUNT) ? &m_clients[clientId] : NULL;
}


Client* Device::AcquireClient(PFILE_OBJECT pUserFileObject)
{
    Client* pClient = NULL;

    if (NULL != pUserFileObject)
    {
        for (ULONG id = 0; id < MAX_CLIENT_COUNT; ++id)
        {
            // If the regId is available.
            if (!m_clientsMask.Set(id))
            {
                if (m_clients[id].Register(pUserFileObject))
                {
                    pClient = &m_clients[id];

                    // If this is the first client.
                    if (0UL == m_clientsCount++)
                    {
                        RegisterCallbacks();
                    }

                    break;
                }

                m_clientsMask.Clear(id);
            }
        }
    }

    return pClient;
}


bool Device::ReleaseClient(ULONG clientId)
{
    Client* pClient = GetClient(clientId);

    bool ret = (NULL != pClient && pClient->IsValid());

    if (ret)
    {
        // Stop any profile.
        if (pClient->IsStarted())
        {
            pClient->StopProfiling();

            PrintInfo("Aborting the profile due to unexpected unregistration.");
            pClient->SetLastErrorCode(PROF_CRITICAL_ERROR);
        }
        else
        {
            pClient->Clear();
            pClient->SetLastErrorCode(PROF_SUCCESS);
        }

        pClient->Unregister();
        m_clientsMask.Clear(clientId);

        // If this is the last client.
        if (0UL == --m_clientsCount)
        {
            UnregisterCallbacks();
        }
    }

    return ret;
}


ULONG Device::ReleaseClients(PFILE_OBJECT pUserFileObject)
{
    ULONG count = 0UL;

    for (ULONG id = 0; id < MAX_CLIENT_COUNT; ++id)
    {
        if (m_clients[id].IsRegistered(pUserFileObject))
        {
            if (ReleaseClient(id))
            {
                count++;
            }
        }
    }

    return count;
}


ULONG Device::GetResourceCount(PCORERESOURCETYPES type) const
{
    ASSERT(static_cast<ULONG>(type) < static_cast<ULONG>(MAX_RESOURCE_TYPE));
    return m_resourceCounts[type];
}


bool Device::IsSamplingSuspended(ULONG clientId) const
{
    ASSERT(NULL != GetClient(clientId));
    return NULL != m_pSharedObjSystemAddress && TRUE == m_pSharedObjSystemAddress[clientId].paused;
}


static void RegisterCallbacks()
{
    // Add callbacks for task info notifications.
    PsSetCreateProcessNotifyRoutine(CreateProcessCallback, FALSE);
    PsSetLoadImageNotifyRoutine(LoadImageCallback);
    PsSetCreateThreadNotifyRoutine(CreateThreadCallback);
}


static void UnregisterCallbacks()
{
    // Remove Process notifications.
    PsSetCreateProcessNotifyRoutine(CreateProcessCallback, TRUE);
    PsRemoveLoadImageNotifyRoutine(LoadImageCallback);
    PsRemoveCreateThreadNotifyRoutine(CreateThreadCallback);
}

} // namespace CpuProf
