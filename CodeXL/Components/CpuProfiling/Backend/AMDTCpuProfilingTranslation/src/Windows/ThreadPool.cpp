//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ThreadPool.cpp
/// \brief Windows Thread Pool implementation.
///
//==================================================================================

#include "ThreadPool.h"
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osThread.h>

Semaphore::Semaphore() : m_sem(0)
{
}

Semaphore::~Semaphore()
{
    CloseHandle(m_sem);
}

// Create a semaphore
bool Semaphore::Open(int initCount, int maxCount)
{
    m_sem = CreateSemaphore(NULL,       // default security attributes
                            initCount,  // initial count
                            maxCount,   // maxThreads,  // maximum count
                            NULL);      // unnamed semaphore

    if (NULL == m_sem)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"CreateSemaphore error: %0x", GetLastError());
        return false;
    }

    return true;
}

bool Semaphore::Wait()
{
    return TimedWait(INFINITE);
}

bool Semaphore::TimedWait(unsigned int millSec)
{
    ULONG  ret;
    ret = WaitForSingleObject(m_sem, millSec);

    if (WAIT_OBJECT_0 != ret)
    {
        return false;
    }

    return true;
}

long Semaphore::Post(int count)
{
    long prevCount = 0;
    ReleaseSemaphore(m_sem, count, &prevCount);

    return prevCount;
}


WorkQueue::WorkQueue(unsigned size) : m_maxQueueSize(size),
    m_queueSize(size),
    m_topIndex(0),
    m_bottomIndex(0),
    m_incompleteWork(0),
    m_status(0),
    m_workHandlesQueue(NULL),
    m_waitingForCompletion(0)
{
    m_condCrit = CreateEvent(NULL, FALSE, FALSE, NULL);
}

WorkQueue::~WorkQueue()
{
    if (NULL != m_workHandlesQueue)
    {
        delete [] m_workHandlesQueue;
    }
}


// Initialize the WorkQueue
bool WorkQueue::Init()
{
    // If the queue size is zero return error..
    if (! m_queueSize)
    {
        return false;
    }

    m_workHandlesQueue = new ThreadPoolWork[m_queueSize];

    if (NULL == m_workHandlesQueue)
    {
        return false;
    }

    if (! m_availableWork.Open(0, m_queueSize))
    {
        // Error
        return false;
    }

    if (! m_availableThreads.Open(m_queueSize, m_queueSize))
    {
        // Error
        return false;
    }

    return true;
}


//
//  Assigns the work to workqueue
//
bool WorkQueue::AssignWork(ThreadPoolWork* work)
{
    if (! work)
    {
        return false;
    }

    SetWorkStatusAssign();

    // Wait on availableThreads - INFINITE time
    if (! m_availableThreads.Wait())
    {
        OS_OUTPUT_DEBUG_LOG(L"Error in Semaphore Wait", OS_DEBUG_LOG_ERROR);
        return false;
    }

    // Acquire lock; put the work in workQueue; notify availablework
    m_mutexSync.enter();
    EnqueueWork(work);
    m_mutexSync.leave();

    // Signal availableWork
    m_availableWork.Post();

    return true;
}


//
//  Fetches thw work from the workqueue
//
bool WorkQueue::FetchWork(ThreadPoolWork* work)
{
    if (! work)
    {
        return false;
    }

    // Wait on availableWork - INFINITE time
    if (! m_availableWork.Wait())
    {
        OS_OUTPUT_DEBUG_LOG(L"Error in Semaphore Wait", OS_DEBUG_LOG_ERROR);
        return false;
    }

    // Acquire m_mutexSync and enqueue the work
    m_mutexSync.enter();
    DequeueWork(work);
    m_mutexSync.leave();

    // Signal availableThreads
    m_availableThreads.Post();

    return TRUE;
}


void WorkQueue::SetWorkStatusComplete()
{
    _InterlockedDecrement(&m_incompleteWork);

    // FIXME: No need to acquire the lock while wake up the condition variable ?
    // If the workqueue is waiting on the condition variable.. wake him up
    if (0 != _InterlockedExchangeAdd(&m_waitingForCompletion, 0) && 0 == _InterlockedExchangeAdd(&m_incompleteWork, 0))
    {
        SetEvent(m_condCrit);
    }

    return;
}


// This will basically wait for the work to be completed
HRESULT WorkQueue::WaitForWorkCompletion(int milliSec)
{
    HRESULT ret = S_OK;

    m_mutexCri.enter();
    _InterlockedIncrement(&m_waitingForCompletion);

    if (IsWorkPending())
    {
        // Release the lock acquired the caller..
        m_mutexCri.leave();

        DWORD dwMilliseconds = (0 != milliSec) ? static_cast<DWORD>(milliSec) : INFINITE;
        // Now wait for either event to become signaled
        DWORD result = WaitForSingleObject(m_condCrit, dwMilliseconds);

        // now acquire the mutex
        m_mutexCri.enter();

        if (WAIT_OBJECT_0 == result)
        {
            ret = S_OK;
        }
        else if (WAIT_TIMEOUT == result)
        {
            ret = E_TIMEOUT;
        }
        else
        {
            ret = E_FAIL;
        }
    }

    // TODO: Thread Cancellation to be handled here
    _InterlockedDecrement(&m_waitingForCompletion);
    m_mutexCri.leave();

    return ret;
}


void WorkQueue::EnqueueWork(ThreadPoolWork* work)
{
    if (! work)
    {
        return;
    }

    m_workHandlesQueue[m_topIndex] = *work;

    if (m_queueSize != 1)
    {
        if ((++m_topIndex) >= m_queueSize)
        {
            m_topIndex = 0;
        }
    }

    return;
}


void WorkQueue::DequeueWork(ThreadPoolWork* workArg)
{
    if (! workArg)
    {
        return;
    }

    *workArg = m_workHandlesQueue[m_bottomIndex];

    if (m_queueSize > 1)
    {
        if ((++m_bottomIndex) >= m_queueSize)
        {
            m_bottomIndex = 0;
        }
    }

    return;
}


//
// Class ThreadPool
//

ThreadPool::ThreadPool(unsigned nbrThreads) : m_nbrThreads(nbrThreads),
    m_status(THREAD_POOL_START),
    m_activeThreads(0),
    m_threadList(NULL),
    m_workQueue(NULL)
{
    if (0U == m_nbrThreads)
    {
        m_nbrThreads = 1;
    }
}


ThreadPool::~ThreadPool()
{
    // m_workQueue gets deleted in DestroyThreadPool()
}


//
//  Initializes the thread pool:
//      - creates all the worker threads
//      - the paramList.size() should match the number of threads to be created in the thread pool
//
bool ThreadPool::InitThreadPool(THREAD_WORK_ROUTINE threadWorkFunction, ThreadParamList& paramList)
{
    unsigned int nbrParams = paramList.size();

    if (m_nbrThreads != nbrParams)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"insufficent parameters for threads in threadpool; maxThreads(%u), nbrParams(%u)",
                                   m_nbrThreads, nbrParams);
        return false;
    }

    // Clear the threadlist
    if (NULL != m_threadList)
    {
        delete [] m_threadList;
    }

    m_threadList = new HANDLE[m_nbrThreads];

    if (NULL == m_threadList)
    {
        return false;
    }

    // Create the workqueue and initialize it
    m_workQueue = new WorkQueue(m_nbrThreads);

    if (NULL == m_workQueue)
    {
        return false;
    }

    if (! m_workQueue->Init())
    {
        return false;
    }

    ThreadParamList::iterator iter;
    iter = paramList.begin();

    for (gtUInt32 i = 0; i < m_nbrThreads; i++)
    {
        WorkThreadParam* tParam = new WorkThreadParam;

        if (NULL == tParam)
        {
            return false;
        }

        // Set the thread work function
        tParam->threadWorkFunction = threadWorkFunction;

        // Set the thread param from the paramList
        if (iter != paramList.end())
        {
            tParam->threadWorkParam = (void*)(*iter);
            iter++;
        }
        else
        {
            tParam->threadWorkParam = NULL;
        }

        tParam->tPool = this;

        HANDLE threadHandle = CreateThread(NULL,    // default security attributes
                                           0,       // default stack size
                                           (LPTHREAD_START_ROUTINE) &ThreadPool::ThreadExecute,
                                           tParam,  // thread start function argument
                                           0,       // default creation flags
                                           NULL);   // thread ID

        if (NULL == threadHandle)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Thread creation failed with error(%0x)", GetLastError());
            return false;
        }

        // Add it to the threadlist
        m_threadList[m_activeThreads++] = threadHandle;
    }

    // Post conditions
    GT_ASSERT(m_activeThreads == m_nbrThreads);

    // TP status
    this->m_status = THREAD_POOL_INIT;
    return true;
}


//
// Once all the work is complete, destroy the thread pool
//
HRESULT ThreadPool::DestroyThreadPool(int milliSec)
{
    HRESULT res = S_OK;

    if (milliSec < 0)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"maxPollSecs is negative(%d)", milliSec);
        return E_INVALIDARG;
    }

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"maxPollSecs(%d)", milliSec);

    res = m_workQueue->WaitForWorkCompletion(milliSec);
    // return value can be S_OK, or E_TIMEOUT or E_FAIL

    // TODO: check, if threadpool needs to be cancelled, if so, exit all the threads;
    // else, keep waiting for the work to be processed.
    if (E_TIMEOUT == res)
    {
        return res;
    }

    // If the work completed or if there is any failure, destroy the thread pool..
    if (S_OK != res)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"failed with error(%d)", res);
    }

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"res(%d)", res);

    // Push a stop work to tell the threads to terminate
    for (unsigned i = 0, sz = size(); i < sz; i++)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Assigning stop work for thread(%u)", i);
        AssignWork(NULL);
    }

    // Now, wait for the threads to terminate
    OS_OUTPUT_DEBUG_LOG(L"Wait for threads to terminate", OS_DEBUG_LOG_DEBUG);
    AllThreadsJoin();

    // delete the m_workQueue
    OS_OUTPUT_DEBUG_LOG(L"Delete m_workQueue", OS_DEBUG_LOG_DEBUG);
    delete m_workQueue;

    return S_OK;
}


bool ThreadPool::AllThreadsJoin()
{
    if (NULL != m_threadList && 0U != m_activeThreads)
    {
        WaitForMultipleObjects(m_activeThreads, m_threadList, TRUE, INFINITE);

        for (unsigned i = 0U; i < m_activeThreads; i++)
        {
            CloseHandle(m_threadList[i]);
        }

        m_activeThreads = 0U;
    }

    return true;
}


HRESULT ThreadPool::WaitForWorkCompletion(int milliSec)
{
    HRESULT res = S_OK;

    if (milliSec < 0)
    {
        return E_INVALIDARG;
    }

    // return value can be S_OK, or E_TIMEOUT or E_FAIL
    res = m_workQueue->WaitForWorkCompletion(milliSec);

    return res;
}


bool ThreadPool::AssignWork(PrdWorkUnit* work)
{
    ThreadPoolWork workTP;

    if (NULL != work)
    {
        workTP.flag = TP_WORK_ACTUAL;
        workTP.work = *work;
    }
    else
    {
        workTP.flag = TP_WORK_STOP;
    }

    return (m_workQueue->AssignWork(&workTP));
}


bool ThreadPool::FetchWork(PrdWorkUnit* workArg)
{
    if (NULL == workArg)
    {
        return false;
    }

    ThreadPoolWork workTP;
    m_workQueue->FetchWork(&workTP);

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Thread(%d) got the work from workqueue", GetCurrentThreadId());
#endif

    if (TP_WORK_STOP == workTP.flag)
    {
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Thread (%d) Received dummy work", GetCurrentThreadId());
#endif
        return false;
    }

    *workArg = workTP.work;
    return true;
}


//
//  ThreadPool's thread start routine
//      - All the worker threads will start with this routine
//      - the parameter passed to this thread start routine is "WorkThreadParam"
//          It has
//              threadWorkFunction  :-  thread work function;
//              threadWorkParam     :-  thread param for the "threadWorkFunction"
//              tPool               :-  thread pool object itself
//
DWORD WINAPI ThreadPool::ThreadExecute(LPVOID param)
{
    osNameThreadInDebugger(osGetCurrentThreadId(), "PoolThread");

    PrdWorkUnit work;

    if (NULL == param)
    {
        OS_OUTPUT_DEBUG_LOG(L"Invalid arg for worker thread", OS_DEBUG_LOG_ERROR);
        return (DWORD)E_INVALIDARG;
    }

    WorkThreadParam* wParam = (WorkThreadParam*)param;
    ThreadPool* tp = (ThreadPool*)wParam->tPool;

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Thread (%d) Created", GetCurrentThreadId());

    // the worker cannot be assigned empty work..
    // ThreadPool::fetchwork() will return false on DUMMY work;
    // DUMMY work is used to request the thread to terminate.
    while (tp->FetchWork(&work))
    {
        wParam->threadWorkFunction(wParam->threadWorkParam, (void*)&work);

        tp->m_workQueue->SetWorkStatusComplete();
    }

    // This is to cleanup the Dummy work..
    // Is there a way to verify that we have received dummy work ?
    if (tp->m_workQueue->IsWorkPending())
    {
        tp->m_workQueue->SetWorkStatusComplete();
    }

    // If still some work is pending, emit a warning message
    if (tp->m_workQueue->IsWorkPending())
    {
        OS_OUTPUT_DEBUG_LOG(L"Warning: Still some work is pending in the workqueue or work-in-progress in some other thread...\n",
                            OS_DEBUG_LOG_DEBUG);
    }

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Return from the thread (%d)", GetCurrentThreadId());
    return 0;
}
