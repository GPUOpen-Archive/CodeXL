#include "..\Include\StackTraceDispatcher.hpp"
#include <WinDriverUtils\Include\Dpc.hpp>

#define MIN_DEPTH ((sizeof(Object) - FIELD_OFFSET(Object, m_callers) - 1) / sizeof(ULONG_PTR) + 1)

#ifdef DBG
    #define CALLERS_GUARD_SIZE 1
#else
    #define CALLERS_GUARD_SIZE 0
#endif


StackTraceDispatcher::StackTraceDispatcher() : m_pfnUserStackBackTraceCompleteCallback(NULL),
    m_pCoreKernelCallers(NULL),
    m_stackWalkersCount(0UL),
    m_maxDepth(MIN_DEPTH)
{
}


StackTraceDispatcher::~StackTraceDispatcher()
{
    Clear();
}


bool StackTraceDispatcher::Initialize(PfnUserStackBackTraceCompleteCallback pfnUserStackBackTraceCompleteCallback, ULONG maxDepth)
{
    Clear();

    m_pfnUserStackBackTraceCompleteCallback = NULL;
    m_maxDepth = (MIN_DEPTH < maxDepth) ? maxDepth : MIN_DEPTH;

    bool ret = false;

    if (NULL != pfnUserStackBackTraceCompleteCallback)
    {
        ULONG coresCount = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);

        m_pCoreKernelCallers = AllocateCoreObjects(coresCount);

        if (NULL != m_pCoreKernelCallers)
        {
            Object* pObject = AllocateObject();

            if (NULL != pObject)
            {
                ret = true;

                Stack<Object> objects;
                objects.Push(*pObject);

                for (ULONG i = 1UL, count = (coresCount * 3); i < count; ++i)
                {
                    pObject = AllocateObject();

                    if (NULL == pObject)
                    {
                        // The minimum number of objects is twice the number of cores.
                        if (i < (coresCount * 2))
                        {
                            //
                            // Free the allocated objects.
                            //

                            Stack<Object>::Iterator it = objects.Flush();

                            while (it.IsValid())
                            {
                                Object* pObject = &*it;
                                ++it;

                                FreeObject(pObject);
                            }

                            ret = false;
                        }

                        break;
                    }

                    objects.Push(*pObject);
                }

                // Insert the allocated objects into the pool.
                if (!objects.IsEmpty())
                {
                    uint count = objects.GetSize();
                    Stack<Object>::Iterator first = objects.Flush(), last = NULL;
                    m_objectsPool.PushStack(first, last, count);
                }

                if (ret)
                {
                    StackWalker::InitializeKernelModulesInfo(m_kernelModules);
                    StackWalker::SetGlobalCounter(&m_stackWalkersCount);
                    m_pfnUserStackBackTraceCompleteCallback = pfnUserStackBackTraceCompleteCallback;
                }
            }

            if (!ret)
            {
                FreeCoreObjects(m_pCoreKernelCallers);
                m_pCoreKernelCallers = NULL;
            }
        }
    }

    return ret;
}


bool StackTraceDispatcher::IsInitialized() const
{
    return NULL != m_pfnUserStackBackTraceCompleteCallback;
}


StackTraceDispatcher::CoreObject* StackTraceDispatcher::AllocateCoreObjects(ULONG count)
{
    SIZE_T bytesCount = sizeof(CoreObject) * (count + 1UL);
    CoreObject* pObjects = static_cast<CoreObject*>(ExAllocatePoolWithTag(NonPagedPool, bytesCount, ALLOC_POOL_TAG));

    if (NULL != pObjects)
    {
        bytesCount = (m_maxDepth + CALLERS_GUARD_SIZE) * sizeof(ULONG_PTR);

        // Tag the anchor.
        pObjects[count].m_pCallers = NULL;

        for (ULONG i = 0UL; i < count; ++i)
        {
            pObjects[i].m_pCallers = static_cast<ULONG_PTR*>(ExAllocatePoolWithTag(NonPagedPool, bytesCount, ALLOC_POOL_TAG));

            if (NULL == pObjects[i].m_pCallers)
            {
                FreeCoreObjects(pObjects);
                pObjects = NULL;

                break;
            }

            pObjects[i].m_time = 0ULL;
            pObjects[i].m_depth = 0UL;
        }
    }

    return pObjects;
}


void StackTraceDispatcher::FreeCoreObjects(CoreObject* pObjects) const
{
    for (CoreObject* pObject = pObjects; NULL != pObject->m_pCallers; ++pObject)
    {
        ExFreePoolWithTag(pObject->m_pCallers, ALLOC_POOL_TAG);
    }

    ExFreePoolWithTag(pObjects, ALLOC_POOL_TAG);
}


StackTraceDispatcher::Object* StackTraceDispatcher::AllocateObject() const
{
    return static_cast<Object*>(ExAllocatePoolWithTag(NonPagedPool, GetObjectSize(), ALLOC_POOL_TAG));
}


void StackTraceDispatcher::FreeObject(Object* pObject) const
{
    ExFreePoolWithTag(pObject, ALLOC_POOL_TAG);
}


void StackTraceDispatcher::Clear()
{
    if (NULL != m_pCoreKernelCallers)
    {
        FreeCoreObjects(m_pCoreKernelCallers);
        m_pCoreKernelCallers = NULL;
    }

    AtomicStack<Object>::Iterator it;

    while ((it = m_objectsPool.Flush()).IsValid())
    {
        do
        {
            Object* pObject = &*it;
            ++it;

            FreeObject(pObject);
        }
        while (it.IsValid());
    }
}


SIZE_T StackTraceDispatcher::GetObjectSize() const
{
    SIZE_T bytesCount = FIELD_OFFSET(Object, m_callers) + m_maxDepth * sizeof(ULONG_PTR);
    return ALIGN_UP_BY(bytesCount, PAGE_SIZE);
}


void StackTraceDispatcher::GetStackPotentialValuesBuffers(Object& object, ULONG callersCount, BOOLEAN is64Bit,
                                                          ULONG& maxCount, ULONG32*& pValues, USHORT*& pOffsets) const
{
    SIZE_T objectBytesCount = GetObjectSize();

#ifdef _AMD64_
    const ULONG VALUE_SIZE = is64Bit ? sizeof(ULONG64) : sizeof(ULONG32);
#else
    UNREFERENCED_PARAMETER(is64Bit);
    ASSERT(FALSE == is64Bit);
    const ULONG VALUE_SIZE = sizeof(ULONG32);
#endif

    SIZE_T pos = FIELD_OFFSET(Object, m_callers) + (callersCount * VALUE_SIZE);
    ASSERT(objectBytesCount >= pos);

    ULONG maxAvailableCount = static_cast<ULONG>((objectBytesCount - pos) / (sizeof(ULONG32) + sizeof(USHORT)));

    if (maxAvailableCount < maxCount)
    {
        maxCount = maxAvailableCount;
    }

    pValues = reinterpret_cast<ULONG32*>(reinterpret_cast<UCHAR*>(&object) + pos);
    ASSERT(((ULONG_PTR)pValues) == ((ULONG_PTR)ALIGN_UP(pValues, ULONG32)));

    pOffsets = reinterpret_cast<USHORT*>(&pValues[maxAvailableCount]);
}


StackWalker* StackTraceDispatcher::AcquireStackWalker(HANDLE processId, ULONG clientId, ULONG maxDepth,
                                                      const CSS_CodeRange pRanges[], ULONG countRanges)
{
    StackWalker* pStackWalker = NULL;

    ScopedLock lock(m_stackWalkersMutex);

    ULONG validCount = m_stackWalkersCount;
    ULONG firstInvalidIndex;

    if (0UL != validCount)
    {
        firstInvalidIndex = MAX_STACK_WALKERS;

        for (ULONG i = 0UL; i < MAX_STACK_WALKERS; ++i)
        {
            StackWalker& currentStackWalker = m_stackWalkers[i];

            if (currentStackWalker.GetProcessId() == processId)
            {
                if (0L < currentStackWalker.AddRef())
                {
                    // If the PID has not been miraculously changed.
                    if (currentStackWalker.GetProcessId() == processId)
                    {
                        pStackWalker = &currentStackWalker;
                        break;
                    }
                    else
                    {
                        currentStackWalker.Release();
                    }
                }
            }

            if (currentStackWalker.IsValid())
            {
                validCount--;

                if (0UL == validCount)
                {
                    if (i < firstInvalidIndex)
                    {
                        firstInvalidIndex = i + 1UL;
                    }

                    break;
                }
            }
            else if (!currentStackWalker.IsInitialized())
            {
                firstInvalidIndex = i;
            }
        }
    }
    else
    {
        firstInvalidIndex = 0UL;
    }

    if (NULL == pStackWalker)
    {
        pStackWalker = NULL;

        for (ULONG i = firstInvalidIndex; i < MAX_STACK_WALKERS; ++i)
        {
            StackWalker& currentStackWalker = m_stackWalkers[i];

            if (!currentStackWalker.IsInitialized())
            {
                if (currentStackWalker.Initialize(processId, pRanges, countRanges))
                {
                    pStackWalker = &currentStackWalker;
                    pStackWalker->RegisterClient(clientId, maxDepth);
                    break;
                }
            }
        }
    }
    else
    {
        pStackWalker->RegisterClient(clientId, maxDepth);
    }

    return pStackWalker;
}


StackWalker* StackTraceDispatcher::AcquireStackWalker(HANDLE processId, ULONG clientId, ULONG maxDepth)
{
    return AcquireStackWalker(processId, clientId, maxDepth, NULL, 0UL);
}


StackWalker* StackTraceDispatcher::FindStackWalker(HANDLE processId, ULONG clientId)
{
    StackWalker* pStackWalker = FindStackWalker(processId);

    if (NULL != pStackWalker && !pStackWalker->IsClientRegistered(clientId))
    {
        pStackWalker->Release();
        pStackWalker = NULL;
    }

    return pStackWalker;
}


StackWalker* StackTraceDispatcher::FindStackWalker(HANDLE processId)
{
    StackWalker* pStackWalker = NULL;

    ULONG validCount = m_stackWalkersCount;

    if (0UL != validCount)
    {
        for (ULONG i = 0UL; i < MAX_STACK_WALKERS; ++i)
        {
            StackWalker& currentStackWalker = m_stackWalkers[i];

            if (currentStackWalker.GetProcessId() == processId)
            {
                if (0L < currentStackWalker.AddRef())
                {
                    // If the PID has not been miraculously changed.
                    if (currentStackWalker.GetProcessId() == processId)
                    {
                        pStackWalker = &currentStackWalker;
                    }
                    else
                    {
                        currentStackWalker.Release();
                    }
                }

                break;
            }

            if (currentStackWalker.IsValid())
            {
                validCount--;

                if (0UL == validCount)
                {
                    break;
                }
            }
        }
    }

    return pStackWalker;
}


void StackTraceDispatcher::UnregisterClient(ULONG clientId)
{
    SetCaptureStackPotentialValues(clientId, false);

    for (ULONG i = 0UL; i < MAX_STACK_WALKERS; ++i)
    {
        StackWalker& stackWalker = m_stackWalkers[i];

        if (stackWalker.UnregisterClient(clientId))
        {
            stackWalker.Release();
        }
    }
}


void StackTraceDispatcher::SetCaptureStackPotentialValues(ULONG clientId, bool enable)
{
    ASSERT(MAX_CLIENTS_COUNT > clientId);

    if (enable)
    {
        m_clientsPotential.Set(static_cast<uint>(clientId));
    }
    else
    {
        m_clientsPotential.Clear(static_cast<uint>(clientId));
    }
}


bool StackTraceDispatcher::IsCaptureStackPotentialValuesEnabled(ULONG clientId) const
{
    ASSERT(MAX_CLIENTS_COUNT > clientId);
    return m_clientsPotential.Test(static_cast<uint>(clientId));
}


bool StackTraceDispatcher::IsCaptureStackPotentialValuesEnabled(const StackWalker& stackWalker) const
{
    // Check that the registered clients mask intersects with the potential values collection mask.
    return (0UL != (stackWalker.GetRegisteredClientsMask() & static_cast<ULONG>(m_clientsPotential.GetValue())));
}


ULONG StackTraceDispatcher::CaptureKernelStackBackTrace(ULONG clientId, ULONG64 time,
                                                        const KTRAP_FRAME& trapFrame, ULONG_PTR*& pCallers, ULONG maxDepth)
{
    ULONG count = 0UL;

    ASSERT(IS_KERNEL_MODE(trapFrame.SegCs));

    ULONG core = KeGetCurrentProcessorNumberEx(NULL);
    pCallers = m_pCoreKernelCallers[core].m_pCallers;

    if (time != m_pCoreKernelCallers[core].m_time)
    {
        HANDLE processId = PsGetCurrentProcessId();
        StackWalker* pStackWalker = FindStackWalker(processId, clientId);

        if (NULL != pStackWalker)
        {
            if (m_maxDepth < maxDepth)
            {
                maxDepth = m_maxDepth;
            }

            BOOLEAN is64Bit;
            count = pStackWalker->CaptureStackBackTrace(trapFrame, pCallers, maxDepth, is64Bit, KeGetCurrentIrql());

#ifdef _AMD64_
            ASSERT(TRUE == is64Bit);
#else
            ASSERT(FALSE == is64Bit);
#endif

            pStackWalker->Release();
        }

        m_pCoreKernelCallers[core].m_time = time;
        m_pCoreKernelCallers[core].m_depth = count;
    }
    else
    {
        count = m_pCoreKernelCallers[core].m_depth;
    }

    return count;
}


bool StackTraceDispatcher::EnqueueUserStackBackTrace(ULONG clientId, ULONG64 time)
{
    PKTHREAD pThread = KeGetCurrentThread();
    bool ret = FALSE == KxSetStackTraceApcInserted(pThread);

    if (!ret)
    {
        HANDLE processId = PsGetCurrentProcessId();
        StackWalker* pStackWalker = FindStackWalker(processId, clientId);

        if (NULL != pStackWalker)
        {
            Object* pObject = m_objectsPool.Pop();

            if (NULL != pObject)
            {
                pObject->m_startTime = time;

                KeInitializeApc(&pObject->m_apc, pThread, OriginalApcEnvironment,
                                StackTraceApc, NULL, reinterpret_cast<PKNORMAL_ROUTINE>(StackTraceApc), KernelMode, this);

                if (KeGetCurrentIrql() > DISPATCH_LEVEL)
                {
                    ret = FALSE != KxTryToInsertQueueNormalKernelApcOnCurrentThread(&pObject->m_apc, pStackWalker, NULL);

                    // If we failed to enqueue the APC, then try to enqueue a DPC.
                    // We check for the thread's lock, as we are about to set its affinity.
                    if (!ret)
                    {
                        KeInitializeDpc(&pObject->m_dpc, StackTraceDpc, this);

                        if (SetTargetCoreDpc(pObject->m_dpc, KxGetThreadNextProcessor(pThread)))
                        {
                            ret = FALSE != KeInsertQueueDpc(&pObject->m_dpc, pStackWalker, pThread);
                        }
                    }
                }
                else
                {
                    ret = FALSE != KeInsertQueueApc(&pObject->m_apc, pStackWalker, NULL, LOW_PRIORITY);
                }
            }

            if (!ret)
            {
                pStackWalker->Release();

                if (NULL != pObject)
                {
                    m_objectsPool.Push(*pObject);
                }

                KxClearStackTraceApcInserted(pThread);
            }
        }
        else
        {
            ret = false;
            KxClearStackTraceApcInserted(pThread);
        }
    }

    return ret;
}


bool StackTraceDispatcher::IsUserStackWalkActive(PKTHREAD pThread)
{
    return FALSE != KxIsUserStackWalkActive(pThread);
}


void StackTraceDispatcher::CompleteUserStackBackTrace(const StackWalker& stackWalker,
                                                      const ULONG_PTR pCallers[], ULONG callersCount,
                                                      const ULONG32 pValues[], const USHORT pOffsets[], ULONG stackValuesCount,
                                                      const KTRAP_FRAME& trapFrame,
                                                      BOOLEAN is64Bit,
                                                      HANDLE processId, HANDLE threadId,
                                                      ULONG64 startTime)
{
    if (NULL != m_pfnUserStackBackTraceCompleteCallback)
    {
        ULONG_PTR stackPtr = KxGetStackPointerFromTrapFrame(&trapFrame);
#ifdef _X86_
        ULONG_PTR framePtr = trapFrame.Ebp;
#else
        ULONG_PTR framePtr = trapFrame.Rbp;
#endif

        ScopedIrqlProtector irql(HIGH_LEVEL);

        LARGE_INTEGER endTime = KeQueryPerformanceCounter(NULL);

        for (MaskIterator itClientId = stackWalker.GetRegisteredClientsMask(); !itClientId.IsEmpty(); ++itClientId)
        {
            m_pfnUserStackBackTraceCompleteCallback(*itClientId,
                                                    pCallers, callersCount,
                                                    pValues, pOffsets, stackValuesCount,
                                                    stackPtr, framePtr,
                                                    is64Bit,
                                                    processId, threadId,
                                                    startTime, endTime.QuadPart);
        }
    }
}


void StackTraceDispatcher::CaptureUserStackBackTrace(Object& object, PKTHREAD pThread, StackWalker& stackWalker, bool walk)
{
    PKTHREAD pCurrentThread = KeGetCurrentThread();

    KxLockUserStackWalk(pCurrentThread);

    KTRAP_FRAME* pTrapFrame = KxGetBaseTrapFrame();

    if (pCurrentThread == pThread && NULL != pTrapFrame)
    {
        KIRQL irql = KeGetCurrentIrql();
        ULONG64 startTime = object.m_startTime;
        ULONG callersCount = 0UL;

#ifdef _AMD64_
        BOOLEAN is64Bit = TRUE;
#else
        BOOLEAN is64Bit = FALSE;
#endif

        ULONG maxDepth = stackWalker.GetRegisteredClientsMaxDepth();

        if (maxDepth > m_maxDepth)
        {
            maxDepth = m_maxDepth;
        }

        if (walk)
        {
            callersCount = stackWalker.CaptureStackBackTrace(*pTrapFrame, object.m_callers, maxDepth, is64Bit, irql);
        }

        if (0UL == callersCount)
        {
            object.m_callers[0] = KxGetInstructionPointerFromTrapFrame(pTrapFrame);
            callersCount = 1UL;
        }

        ULONG32* pValues = NULL;
        USHORT* pOffsets = NULL;
        ULONG stackValuesCount = 0UL;

        // Collect potential values for all 32-bit stacks and only 64-bit stacks for which we failed to back-trace.
        if (IsCaptureStackPotentialValuesEnabled(stackWalker) && (FALSE == is64Bit || 1UL >= callersCount))
        {
            GetStackPotentialValuesBuffers(object, callersCount, is64Bit, maxDepth, pValues, pOffsets);

            if (0UL != maxDepth)
            {
                stackValuesCount = stackWalker.CaptureStackPotentialValues(*pTrapFrame, pValues, pOffsets, maxDepth, irql);
            }
        }

        CompleteUserStackBackTrace(stackWalker,
                                   object.m_callers, callersCount,
                                   pValues, pOffsets, stackValuesCount,
                                   *pTrapFrame,
                                   is64Bit,
                                   PsGetThreadProcessId(pThread), PsGetThreadId(pThread),
                                   startTime);
    }

    m_objectsPool.Push(object);
    KxClearStackTraceApcInserted(pThread);

    stackWalker.Release();

    KxUnlockUserStackWalk(pCurrentThread);
}


VOID StackTraceDispatcher::StackTraceApc(KAPC* Apc, PKNORMAL_ROUTINE* NormalRoutine,
                                         PVOID* NormalContext, PVOID* SystemArgument1, PVOID* SystemArgument2)
{
    PKTHREAD pThread = Apc->Thread;

    Object* pObject = CONTAINING_RECORD(Apc, Object, m_apc);
    StackTraceDispatcher* pDispatcher = static_cast<StackTraceDispatcher*>(*NormalContext);
    StackWalker* pStackWalker = static_cast<StackWalker*>(*SystemArgument1);

    UNREFERENCED_PARAMETER(SystemArgument2);

    pDispatcher->CaptureUserStackBackTrace(*pObject, pThread, *pStackWalker, true);

    *NormalRoutine = NULL;
}


VOID StackTraceDispatcher::StackTraceDpc(KDPC* Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
{
    Object* pObject = CONTAINING_RECORD(Dpc, Object, m_dpc);
    StackTraceDispatcher* pDispatcher = static_cast<StackTraceDispatcher*>(DeferredContext);
    StackWalker* pStackWalker = static_cast<StackWalker*>(SystemArgument1);
    PKTHREAD pThread = static_cast<PKTHREAD>(SystemArgument2);

    KeInitializeApc(&pObject->m_apc, pThread, OriginalApcEnvironment,
                    StackTraceApc, NULL, reinterpret_cast<PKNORMAL_ROUTINE>(StackTraceApc), KernelMode, pDispatcher);

    if (FALSE == KeInsertQueueApc(&pObject->m_apc, pStackWalker, NULL, LOW_PRIORITY))
    {
        pDispatcher->CaptureUserStackBackTrace(*pObject, pThread, *pStackWalker, false);
    }
}
