//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ThreadPool.h
/// \brief Windows Thread Pool implementation.
///
//==================================================================================

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <intrin.h>

//
//  Macros
//
// Thread pool status
#define  THREAD_POOL_START      0x1
#define  THREAD_POOL_INIT       0x2
#define  THREAD_POOL_FINI       0x4

// Work Type
#define TP_WORK_ACTUAL      0x1
#define TP_WORK_STOP        0x2

class Semaphore
{
public:
    Semaphore();
    ~Semaphore();

    // Create a semaphore
    bool Open(int initCount, int maxCount);

    bool Wait();

    // Returns
    //  'true' - if the the semaphore is signaled
    //  'false' - if times
    bool TimedWait(unsigned int millSec);

    long Post(int count = 1);

private:
    HANDLE  m_sem;
};

//
//  Typedefs
//
typedef gtList<void*> ThreadParamList;

// Thread Work Routine
//  It has 2 parameters
//      - threadParam (that is passed to the the thread start routine)
//      - workUnit
typedef int (*THREAD_WORK_ROUTINE)(LPVOID, LPVOID);


//
// PRD Work Unit
//
struct PrdWorkUnit
{
    gtUInt64 baseAddress;
    gtUInt64 totalBytes;
    gtUInt32 numberRecords;
};


struct ThreadPoolWork
{
    int             flag;
    PrdWorkUnit     work;
};


//
//  Class Interfaces
//

// Class WorkQueue
class WorkQueue
{
public:
    // Constructors
    WorkQueue(unsigned size = 1U);

    // Destructor
    ~WorkQueue();

    bool Init();

    bool AssignWork(ThreadPoolWork* work);
    bool FetchWork(ThreadPoolWork*  work);

    unsigned QueueItems() const { return m_queueSize; }

    void SetWorkStatusAssign() { _InterlockedIncrement(&m_incompleteWork); }

    void SetWorkStatusComplete();

    bool IsWorkPending() { return 0 != _InterlockedExchangeAdd(&m_incompleteWork, 0); }

    // This will basically wait for the work to be completed
    HRESULT WaitForWorkCompletion(int milliSec = 0);

private:
    unsigned m_maxQueueSize;
    unsigned m_queueSize;
    gtUInt32 m_topIndex;
    gtUInt32 m_bottomIndex;
    volatile long m_incompleteWork;
    gtUInt32 m_status;

    // work queue buffer
    ThreadPoolWork* m_workHandlesQueue;

    // Mutexes
    osCriticalSection m_mutexSync; // Mutex used to synchronize the work queue
    osCriticalSection m_mutexCri; // Mutex used with the condition variable

    // Semaphores
    Semaphore m_availableWork;
    Semaphore m_availableThreads;

    // Condition variable
    // used to check whether the workQueue has finished the work or not..
    HANDLE m_condCrit;
    volatile long m_waitingForCompletion;

    void EnqueueWork(ThreadPoolWork*  work);

    void DequeueWork(ThreadPoolWork* workArg);
};


// Class ThreadPool
//  Manages all the thread pool activities.
//
class ThreadPool
{
public:
    ThreadPool(unsigned maxThreads = 1U);
    ~ThreadPool();

    // Initialize the thread pool - creates all the worker threads
    // The paramList.size() should match the number of threads to be created in the thread pool
    bool InitThreadPool(THREAD_WORK_ROUTINE threadStartFunction, ThreadParamList& paramList);

    // Once all the work is complete, destroy the thread pool
    HRESULT DestroyThreadPool(int milliSec);

    // Wait for the work to complete...
    HRESULT WaitForWorkCompletion(int milliSec = 0);

    // Thread start routine
    static DWORD WINAPI ThreadExecute(LPVOID param);

    // Assign the work to workqueue
    bool AssignWork(PrdWorkUnit* work);

    // Fetch the work from qorkqueue
    bool FetchWork(PrdWorkUnit*  work);

    unsigned GetTotalSize() const { return m_nbrThreads; }
    unsigned size() const { return m_activeThreads; }

    bool AllThreadsJoin();

private:
    unsigned m_nbrThreads;
    gtUInt32 m_status;
    unsigned m_activeThreads;

    HANDLE* m_threadList;

    WorkQueue* m_workQueue;
};


// "ThreadExecute" is ThreadPool's thread start routine
// All the worker threads will start with this routine
// The parameter passed to this thread start routine is "WorkThreadParam"
//
// It has
//  threadWorkFunction  :-  thread work function;
//                          provided by the user through "InitThreadPool()"
//  threadWorkParam     :-  thread param for the "threadWorkFunction"
//                          provided by the user through "InitThreadPool()"
//  tPool               :-  thread pool object itself
//
struct WorkThreadParam
{
    THREAD_WORK_ROUTINE threadWorkFunction;
    void*                threadWorkParam;
    ThreadPool*          tPool;
};

#endif // _THREADPOOL_H_
