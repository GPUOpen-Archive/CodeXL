#ifndef _STACKWALKER_HPP_
#define _STACKWALKER_HPP_
#pragma once

#include "ModuleInfo.hpp"
#include "UserAccess\CodeRange.h"
#include <WinDriverUtils\Include\Atomic.hpp>

class StackWalker
{
private:
    // Process ID
    AtomicCounter<HANDLE> m_processId;

    AtomicMask<> m_clients;
    AtomicCounter<LONG> m_refCount;

    AtomicCounter<ULONG> m_clientsMaxDepth;

    ModuleInfoList m_modules;

    static ModuleInfoList* s_pKernelModules;
    static AtomicCounter<ULONG>* s_pGlobalCount;


    bool RecoverFramePointer(ULONG32& framePtr, ULONG32 stackStart, ULONG32 stackEnd, ULONG32 stackCurrent, MODE mode, KIRQL irql);

    ULONG WalkFrameChain(ULONG32* pCallers, ULONG maxCount, MODE mode,
                         ULONG32 framePtr, ULONG32 stackCurrent, ULONG32 stackStart, ULONG32 stackEnd, KIRQL irql);

#ifdef _AMD64_
    ULONG VirtualStackUnwind(const KTRAP_FRAME* pTrapFrame, ULONG64* pCallers, ULONG maxCount, MODE mode,
                             ULONG64 stackStart, ULONG64 stackEnd, KIRQL irql);
#endif

    ULONG WalkStack32(ULONG32* pValues, USHORT* pOffsets, ULONG maxCount, MODE mode,
                      ULONG32 stackCurrent, ULONG32 stackStart, ULONG32 stackEnd, KIRQL irql);

#ifdef _AMD64_
    ULONG WalkStack64(ULONG32* pValues, USHORT* pOffsets, ULONG maxCount, MODE mode,
                      ULONG64 stackCurrent, ULONG64 stackStart, ULONG64 stackEnd, KIRQL irql);
#endif

    void Deinitialize();

public:
    enum { MAX_CLIENTS_COUNT = AtomicMask<>::CAPACITY };

    StackWalker();
    ~StackWalker();

    bool Initialize(HANDLE processId, const CSS_CodeRange pRanges[], ULONG countRanges);

    LONG AddRef();
    LONG Release();

    ULONG GetRegisteredClientsMaxDepth() const { return m_clientsMaxDepth; }

    bool RegisterClient(ULONG clientId, ULONG maxDepth);
    bool UnregisterClient(ULONG clientId);
    bool IsClientRegistered(ULONG clientId) const;
    ULONG GetRegisteredClientsMask() const { return m_clients.GetValue(); }

    HANDLE GetProcessId() const { return m_processId; }
    bool IsValid() const;
    bool IsInitialized() const;

    bool AddModuleInfo(ULONG_PTR imageLoadAddr, ULONG imageSize);
    ModuleInfo* LookupModuleInfo(ULONG_PTR addr, MODE mode);

    bool IsPotentialReturnAddress(ULONG_PTR addr, MODE mode, KIRQL irql);

    ULONG CaptureStackPotentialValues(const KTRAP_FRAME& trapFrame, ULONG32* pValues, USHORT* pOffsets, ULONG maxCount, KIRQL irql);
    ULONG CaptureStackBackTrace(const KTRAP_FRAME& trapFrame, ULONG_PTR* pCallers, ULONG maxCount, BOOLEAN& is64Bit, KIRQL irql);

    static bool IsAfterCallInst(ULONG_PTR addr);

    static ULONG InitializeKernelModulesInfo(ModuleInfoList& kernelModules);
    static void SetGlobalCounter(AtomicCounter<ULONG>* pCounter);
};

#endif // _STACKWALKER_HPP_
