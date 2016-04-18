//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RcuScheduler.cpp
///
//==================================================================================

#include "RcuScheduler.h"
#include <AMDTOSWrappers/Include/osAtomic.h>

#define DATA_THREAD_CAP 64

static PSLIST_ENTRY PushListSList(PSLIST_HEADER pListHead, PSLIST_ENTRY pList, PSLIST_ENTRY pListEnd, ULONG count)
{
#if (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE)
    pListEnd->Next = reinterpret_cast<PSLIST_ENTRY>(pListHead->Region);
    pListHead->Region = reinterpret_cast<ULONGLONG>(pList);
    pListHead->HeaderX64.Depth += count;
#else
    pListEnd->Next = pListHead->Next.Next;
    pListHead->Next.Next = pList;
    pListHead->Depth += (WORD)count;
#endif
    return pListEnd->Next;
}

RcuScheduler::RcuScheduler(RcuHandlerAbstractFactory& handlerFactory, unsigned int dataSize, unsigned int threadsCount) :
    m_readLock(0),
    m_updateLock(0),
    m_maxConcurrentBuffers(0),
    m_threadsCount(threadsCount),
    m_pThreads(NULL)
{
    InitializeSListHead(&m_emptyDataStackHeader);
    InitializeSListHead(&m_rawDataStackHeader);
    InitializeSListHead(&m_copiedDataStackHeader);

    SYSTEM_INFO sysInfo = { 0 };
    GetNativeSystemInfo(&sysInfo);

    if (0U == m_threadsCount)
    {
        if (1U < sysInfo.dwNumberOfProcessors)
        {
            // Do not utilize all threads as it makes the system non-responsive.
            m_threadsCount = sysInfo.dwNumberOfProcessors - 1U;
        }
        else
        {
            m_threadsCount = 1U;
        }
    }

    // Pre-allocate the space for the WorkerThread objects so that their RcuScheduler, RcuHandler and RcuData constructor
    // parameters can be passed by reference and avoid creating a thread if the pre-allocations for it fail
    m_pThreads = reinterpret_cast<WorkerThread*>(new gtUByte[m_threadsCount * sizeof(WorkerThread)]);

    if (NULL != m_pThreads)
    {
        for (unsigned int i = 0U; i < m_threadsCount; ++i)
        {
            gtUByte* pBuffer = new gtUByte[dataSize];

            if (NULL == pBuffer)
            {
                m_threadsCount = i;
                break;
            }

            RcuHandler* pHandler = handlerFactory.Create();

            if (NULL == pHandler)
            {
                delete [] pBuffer;
                m_threadsCount = i;
                break;
            }

            // Use placement new to allocate the WorkerThread in the buffer that was pre-allocated above
            new(&m_pThreads[i]) WorkerThread(*this, *pHandler, *reinterpret_cast<RcuData*>(pBuffer), i);
        }
    }
    else
    {
        m_threadsCount = 0U;
    }

    if (0U != m_threadsCount)
    {
        // Currently the CpuProf driver uses PAGE_SIZE buffers.
        gtUByte* pBuffer = new gtUByte[dataSize];

        if (NULL != pBuffer)
        {
            RcuData* pDataBegin;
            RcuData* pDataEnd;

            unsigned int countBuffers = 1U;
            pDataEnd = pDataBegin = reinterpret_cast<RcuData*>(pBuffer);

            for (unsigned int cap = DATA_THREAD_CAP * m_threadsCount; countBuffers < cap; ++countBuffers)
            {
                pBuffer = new gtUByte[dataSize];

                if (NULL == pBuffer)
                {
                    break;
                }

                RcuData* pData = reinterpret_cast<RcuData*>(pBuffer);
                pDataEnd->Next = pData;
                pDataEnd = pData;
            }

            m_maxConcurrentBuffers = static_cast<gtUInt16>(countBuffers);

            pDataEnd->Next = NULL;
            PushListSList(&m_emptyDataStackHeader, pDataBegin, pDataEnd, countBuffers);
        }
    }
}


RcuScheduler::~RcuScheduler()
{
    RcuData* pData = static_cast<RcuData*>(InterlockedFlushSList(&m_emptyDataStackHeader));

    while (NULL != pData)
    {
        gtUByte* pBuffer = reinterpret_cast<gtUByte*>(pData);
        pData = static_cast<RcuData*>(pData->Next);

        delete [] pBuffer;
    }

    if (NULL != m_pThreads)
    {
        for (unsigned int i = 0U; i < m_threadsCount; ++i)
        {
            m_pThreads[i].~WorkerThread();
        }

        gtUByte* pBuffer = reinterpret_cast<gtUByte*>(m_pThreads);
        delete [] pBuffer;
    }
}


unsigned int RcuScheduler::GetBufferSize(unsigned int dataSize)
{
    return dataSize - offsetof(RcuData, m_buffer);
}


int RcuScheduler::Start(bool block)
{
    int recourdsCount = -1;

    if (NULL != m_pThreads && 0U != m_threadsCount && 0 != m_maxConcurrentBuffers)
    {
        // We must have at least one translator thread running.
        if (m_pThreads[0U].execute())
        {
            recourdsCount = 0;

            for (unsigned int i = 1U; i < m_threadsCount; ++i)
            {
                m_pThreads[i].execute();
            }

            if (block)
            {
                recourdsCount = WaitForCompletion();
            }
        }
    }

    return recourdsCount;
}


int RcuScheduler::WaitForCompletion(unsigned int milliseconds)
{
    int recourdsCount = 0;

    if (0U != m_threadsCount)
    {
        for (unsigned int i = 0U; i < m_threadsCount; ++i)
        {
            HANDLE hThread = m_pThreads[i].getHandle();

            if (NULL != hThread)
            {
                DWORD waitRet = WaitForSingleObject(hThread, milliseconds);

                if (WAIT_OBJECT_0 != waitRet)
                {
                    recourdsCount = -1;
                    break;
                }
            }

            recourdsCount += static_cast<int>(m_pThreads[i].m_readRecordsCount);
        }
    }

    return recourdsCount;
}


void RcuScheduler::WorkerThreadEntry(WorkerThread& thread)
{
    bool readCompleted = false;
    SLIST_ENTRY* pEntry;

    do
    {
        if (0 != QueryDepthSList(&m_emptyDataStackHeader))
        {
            if (0 == m_readLock && AtomicCompareAndSwap(m_readLock, 0, 1))
            {
                while (NULL != (pEntry = InterlockedPopEntrySList(&m_emptyDataStackHeader)))
                {
                    if (thread.m_handler.Read(*static_cast<RcuData*>(pEntry)))
                    {
                        InterlockedPushEntrySList(&m_rawDataStackHeader, pEntry);
                        thread.m_readRecordsCount++;
                    }
                    else
                    {
                        InterlockedPushEntrySList(&m_emptyDataStackHeader, pEntry);
                        readCompleted = true;
                        break;
                    }
                }

                // Release the lock.
                AtomicSwap(m_readLock, 0);
            }
            else
            {
                if (QueryDepthSList(&m_emptyDataStackHeader) == m_maxConcurrentBuffers)
                {
                    YieldProcessor();
                }
            }
        }

        if (NULL != (pEntry = InterlockedPopEntrySList(&m_rawDataStackHeader)))
        {
            RcuData* pRawData = static_cast<RcuData*>(pEntry);
            thread.m_handler.Copy(pRawData, thread.m_pUpdateData);
            InterlockedPushEntrySList(&m_copiedDataStackHeader, thread.m_pUpdateData);
            thread.m_pUpdateData = pRawData;
            thread.m_copyRecordsCount++;
        }

        TryUpdateAll(thread);
    }
    while (!readCompleted);


    if (QueryDepthSList(&m_emptyDataStackHeader) != m_maxConcurrentBuffers)
    {
        while (NULL != (pEntry = InterlockedPopEntrySList(&m_rawDataStackHeader)))
        {
            RcuData* pRawData = static_cast<RcuData*>(pEntry);
            thread.m_handler.Copy(pRawData, thread.m_pUpdateData);
            InterlockedPushEntrySList(&m_copiedDataStackHeader, thread.m_pUpdateData);
            thread.m_pUpdateData = pRawData;
            thread.m_copyRecordsCount++;

            TryUpdateAll(thread);
        }

        TryUpdateAll(thread);
    }
}


bool RcuScheduler::TryUpdateAll(WorkerThread& thread)
{
    bool acquiredLock = (0 != QueryDepthSList(&m_copiedDataStackHeader) && 0 == m_updateLock && AtomicCompareAndSwap(m_updateLock, 0, 1));

    if (acquiredLock)
    {
        SLIST_ENTRY* pEntry;

        // Try to update pending data (and release as many buffers as possible).
        while (NULL != (pEntry = InterlockedPopEntrySList(&m_copiedDataStackHeader)))
        {
            thread.m_handler.Update(*static_cast<RcuData*>(pEntry));
            InterlockedPushEntrySList(&m_emptyDataStackHeader, pEntry);
            thread.m_updateRecordsCount++;
        }

        // Release the lock.
        AtomicSwap(m_updateLock, 0);
    }

    return acquiredLock;
}


RcuScheduler::WorkerThread::WorkerThread(RcuScheduler& scheduler, RcuHandler& handler, RcuData& updateData, int id) :
    osThread((0 <= id) ? gtString(L"RCU Worker [").appendUnsignedIntNumber(id).append(L']') :
             gtString(L"RCU Worker")),
    m_scheduler(scheduler),
    m_handler(handler),
    m_pUpdateData(&updateData),
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    m_id(id),
#endif
    m_readRecordsCount(0U),
    m_copyRecordsCount(0U),
    m_updateRecordsCount(0U)
{
}


RcuScheduler::WorkerThread::~WorkerThread()
{
    delete &m_handler;

    if (NULL != m_pUpdateData)
    {
        delete [] m_pUpdateData;
    }
}


int RcuScheduler::WorkerThread::entryPoint()
{
    m_scheduler.WorkerThreadEntry(*this);
    return 0;
}
