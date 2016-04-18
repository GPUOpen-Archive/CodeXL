#ifndef _CPUPROF_DEVICE_HPP_
#define _CPUPROF_DEVICE_HPP_
#pragma once

#include "CpuProfClient.hpp"
#include <WinDriverStackWalker\Include\StackTraceDispatcher.hpp>

namespace CpuProf {

/// \class CPUPROF_DEV_EXTENSION Holds all the driver state information and extends the device
class Device : ExplicitObject
{
private:
    // The driver device object.
    DEVICE_OBJECT* m_pDeviceObject;

    PrdDataBufferPool m_prdDataBuffersPool;

    // The array of clients, to allow for multiple Apps simultaneously profiling
    Client m_clients[MAX_CLIENT_COUNT];

    AtomicMask<MAX_CLIENT_COUNT> m_clientsMask;
    AtomicCounter<ULONG> m_clientsCount;

    // Obtained from \ref PcoreGetResourceCount
    ULONG m_resourceCounts[MAX_RESOURCE_TYPE];
    // The maximum number of resources of any one type
    ULONG m_maxResources;

    // Tracks whether the extended branch target feature of IBS_OP is available
    bool m_hasIbsBrnTrgt;
    // Tracks whether the extended extended count features of IBS_OP is available
    bool m_isIbsOpCntExt;

    // Pointer to the pause shared memory, an array of BOOLEAN, corresponding to index to aClients
    SHARED_CLIENT* m_pSharedObjSystemAddress;
    // Memory probed and locked page(s) containing the shared pause memory
    MDL* m_pSharedObjMdl;
    // Shared memory base address
    PVOID m_pSharedObjBaseAddress;
    // Shared memory section handle
    HANDLE m_sharedObjSectionHandle;

    StackTraceDispatcher m_stackTraceDispatcher;

public:
    Device(DEVICE_OBJECT* pDeviceObject);
    ~Device();

    Client* AcquireClient(PFILE_OBJECT pUserFileObject);
    bool ReleaseClient(ULONG clientId);
    ULONG ReleaseClients(PFILE_OBJECT pUserFileObject);

    StackTraceDispatcher& GetStackTraceDispatcher() { return m_stackTraceDispatcher; }

    PrdDataBufferPool& GetPrdDataBufferPool() { return m_prdDataBuffersPool; }

    ULONG GetRegisteredClientsMask() const { return m_clientsMask.GetValue(); }
    ULONG GetRegisteredClientsCount() const { return m_clientsCount; }
    ULONG GetResourceCount(PCORERESOURCETYPES type) const;
    ULONG GetMaxResourceCount() const { return m_maxResources; }

    bool HasIbsBrnTrgt() const { return m_hasIbsBrnTrgt; }
    bool IsIbsOpCntExt() const { return m_isIbsOpCntExt; }

    bool IsSamplingSuspended(ULONG clientId) const;

    const DEVICE_OBJECT* GetDeviceObject() const { return m_pDeviceObject; }
          DEVICE_OBJECT* GetDeviceObject()       { return m_pDeviceObject; }

    const Client* GetClient(ULONG clientId) const;
          Client* GetClient(ULONG clientId);

private:
    // Sets up the CodeAnalyst User-mode Kernel-mode Shared Object.
    NTSTATUS CreateUserKernelSharedMap();

    // Frees up the User-mode Kernel-mode Shared Object.
    NTSTATUS DeleteUserKernelSharedMap();

    static Device* s_pInstance;

    // number of CPUs on system
    static ULONG s_coresCount;

public:
    static Device* GetInstance() { return s_pInstance; }
    static ULONG GetCoresCount() { return s_coresCount; }

    static NTSTATUS Create(DEVICE_OBJECT* pDeviceObject, ULONG coresCount);
    static void Destroy();
};

inline ULONG GetCoresCount() { return Device::GetCoresCount(); }
inline Client* GetClient(ULONG clientId) { return Device::GetInstance()->GetClient(clientId); }

} // namespace CpuProf

#endif // _CPUPROF_DEVICE_HPP_
