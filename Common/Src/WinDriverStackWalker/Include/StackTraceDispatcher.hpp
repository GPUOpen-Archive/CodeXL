#ifndef _STACKTRACEDISPATCHER_HPP_
#define _STACKTRACEDISPATCHER_HPP_
#pragma once

#include "StackWalker.hpp"

#pragma warning(push)
#pragma warning(disable:4201) // nameless struct/union

typedef void (*PfnUserStackBackTraceCompleteCallback)(ULONG clientId,
                                                      const ULONG_PTR pCallers[], ULONG callersCount,
                                                      const ULONG32 pValues[], const USHORT pOffsets[], ULONG stackValuesCount,
                                                      ULONG_PTR stackPtr, ULONG_PTR framePtr,
                                                      BOOLEAN is64Bit,
                                                      HANDLE processId, HANDLE threadId,
                                                      ULONG64 startTime, ULONG64 endTime);

#define INVALID_STACK_TRACE_ID ((ULONG64)-1)

class StackTraceDispatcher
{
private:
    enum
    {
        MAX_CLIENTS_COUNT = AtomicMask<>::CAPACITY,
        MAX_STACK_WALKERS = 128
    };

    PfnUserStackBackTraceCompleteCallback m_pfnUserStackBackTraceCompleteCallback;

    AtomicCounter<ULONG64> m_nextId;

    struct Object
    {
        union
        {
            struct
            {
                union
                {
                    StackEntry m_linkField;

                    KAPC m_apc;
                    KDPC m_dpc;
                };

                ULONG64 m_startTime;
            };

            ULONG_PTR m_callers[1];
        };
    };

    struct CoreObject
    {
        ULONG64 m_time;
        ULONG m_depth;
        ULONG_PTR* m_pCallers;
    };

    AtomicStack<Object> m_objectsPool;

    CoreObject* m_pCoreKernelCallers;

    ModuleInfoList m_kernelModules;

    StackWalker m_stackWalkers[MAX_STACK_WALKERS];

    GuardedMutex m_stackWalkersMutex;

    // The count of valid (active) stack walkers.
    AtomicCounter<ULONG> m_stackWalkersCount;

    ULONG m_maxDepth;

    AtomicMask<> m_clientsPotential;


    CoreObject* AllocateCoreObjects(ULONG count);
    void FreeCoreObjects(CoreObject* pObjects) const;
    Object* AllocateObject() const;
    void FreeObject(Object* pObject) const;
    void Clear();

    SIZE_T GetObjectSize() const;
    void GetStackPotentialValuesBuffers(Object& object, ULONG callersCount, BOOLEAN is64Bit,
                                        ULONG& maxCount, ULONG32*& pValues, USHORT*& pOffsets) const;

    void CompleteUserStackBackTrace(const StackWalker& stackWalker,
                                    const ULONG_PTR pCallers[], ULONG callersCount,
                                    const ULONG32 pValues[], const USHORT pOffsets[], ULONG stackValuesCount,
                                    const KTRAP_FRAME& trapFrame,
                                    BOOLEAN is64Bit,
                                    HANDLE processId, HANDLE threadId,
                                    ULONG64 startTime);

public:
    StackTraceDispatcher();
    ~StackTraceDispatcher();

    bool IsInitialized() const;
    bool Initialize(PfnUserStackBackTraceCompleteCallback pfnUserStackBackTraceCompleteCallback, ULONG maxDepth);


    StackWalker* AcquireStackWalker(HANDLE processId, ULONG clientId, ULONG maxDepth, const CSS_CodeRange pRanges[], ULONG countRanges);
    StackWalker* AcquireStackWalker(HANDLE processId, ULONG clientId, ULONG maxDepth);
    StackWalker* FindStackWalker(HANDLE processId, ULONG clientId);
    StackWalker* FindStackWalker(HANDLE processId);
    void UnregisterClient(ULONG clientId);


    ULONG CaptureKernelStackBackTrace(ULONG clientId, ULONG64 time, const KTRAP_FRAME& trapFrame, ULONG_PTR*& pCallers, ULONG maxDepth);
    bool EnqueueUserStackBackTrace(ULONG clientId, ULONG64 time);

    void SetCaptureStackPotentialValues(ULONG clientId, bool enable);
    bool IsCaptureStackPotentialValuesEnabled(ULONG clientId) const;
    bool IsCaptureStackPotentialValuesEnabled(const StackWalker& stackWalker) const;

    static ULONG GetMaxProcesses() { return MAX_STACK_WALKERS; }
    static bool IsUserStackWalkActive(PKTHREAD pThread);

private:
    static VOID StackTraceApc(KAPC *Apc, PKNORMAL_ROUTINE *NormalRoutine,
                              PVOID *NormalContext, PVOID *SystemArgument1, PVOID *SystemArgument2);
    static VOID StackTraceDpc(KDPC *Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2);


    void CaptureUserStackBackTrace(Object& object, PKTHREAD pThread, StackWalker& stackWalker, bool walk);
};

#pragma warning(pop)

#endif // _STACKTRACEDISPATCHER_HPP_
