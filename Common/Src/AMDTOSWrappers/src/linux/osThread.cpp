//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osThread.cpp
///
//=====================================================================

//------------------------------ osThread.cpp ------------------------------
#include <time.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// On Mac OS X:
#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    // Mach kernel:
    #include <mach/mach_init.h>
#endif

// POSIX:
#include <signal.h>
#include <pthread.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <dirent.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTimeInterval.h>

// ---------------------------------------------------------------------------
// Name:        osThread::osThread
// Description: Constructor.
// Author:      AMD Developer Tools Team
// Date:        9/11/2006
// ---------------------------------------------------------------------------
osThread::osThread(const gtString& threadName, bool syncTermination, bool isJoinable)
    : _threadName(threadName)
    , _threadId(OS_NO_THREAD_ID)
    , _threadHandle(OS_NO_THREAD_HANDLE)
    , _wasThreadTerminated(false)
    , m_syncTermination(syncTermination)
    , m_isJoinable(isJoinable)
{
}


// ---------------------------------------------------------------------------
// Name:        osThread::~osThread
// Description: Destructor - Terminate the wrapped OS thread.
// Author:      AMD Developer Tools Team
// Date:        9/11/2006
// ---------------------------------------------------------------------------
osThread::~osThread()
{
    terminate();
}


// ---------------------------------------------------------------------------
// Name:        osThread::threadEntryPoint
//
// Description: The POSIX thread entry point.
//              Calls osThread::entryPoint() of the osThread object that represents
//              the POSIX thread.
//
// Arguments:   pParam - A pointer to the osThread object that represents
//                       the OS thread (casted into void*).
//
// Author:      AMD Developer Tools Team
// Date:        9/11/2006
// ---------------------------------------------------------------------------
void* osThread::threadEntryPoint(void* pParam)
{
    // Log the fact that the thread run started into the debug log:
    osThreadId currentThreadId = osGetCurrentThreadId();
    osThread* pThreadWrapper = (osThread*)pParam;
    osThread::debugLogThreadRunStarted(currentThreadId, pThreadWrapper->_threadName);

    // Get the osThread object that represents the OS thread:
    GT_IF_WITH_ASSERT(pThreadWrapper != NULL)
    {
        // Call its osThread::entryPoint() function:
        pThreadWrapper->entryPoint();

        // Enter the thread termination critical section:
        osCriticalSectionLocker csLocker(pThreadWrapper->_threadTerminationCS);

        // If the thread was not already terminated:
        if (!pThreadWrapper->_wasThreadTerminated)
        {
            // Allow "before termination" clean ups of the thread wrapper class instance:
            pThreadWrapper->beforeTermination();

            // Mark that the thread was terminated:
            pThreadWrapper->_wasThreadTerminated = true;
        }

        // Leave the thread termination critical section:
        csLocker.leaveCriticalSection();
    }

    return NULL;
}


// ---------------------------------------------------------------------------
// Name:        osThread::execute
// Description: Starts executing this thread.
//              The thread execution start function will be the entryPoint()
//              function. Sub classes must implement the entryPoint() function.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        9/11/2006
// Implementation notes:
//   On Mac OS X we mainly use pthreads. However, since pthread_t on Mac is a pointer to a struct,
//   we prefer using the Mach kernel thread id as our thread id. We store the thread's pthread id as
//   the thread handle.
// ---------------------------------------------------------------------------
bool osThread::execute()
{
    bool retVal = false;

    // Clear class members:
    _wasThreadTerminated = false;

    // Create a thread's attribute structure:
    pthread_attr_t threadAttributes;
    pthread_attr_init(&threadAttributes);

    int rc1 = 0;

    if (false == m_isJoinable)
    {
        // Make the thread a "detached" thread:
        // Yaki - 24/9/2009 - By default, Linux creates "joinable" thread. If the program does not wait for a joinable thread's returned value,
        // it creates quite a big OS resources leak (on Red Hat Enterprize 5 x86_64, we got a leak of ~10 MB per thread). Since we
        // currrently don't offer an API for waiting for or joining threads, we create all threads as "detached" thread. This saves us
        // from this memory leak that caused our license server's memory to inflate. See case 5810 for more details on this issue.
        rc1 = pthread_attr_setdetachstate(&threadAttributes, PTHREAD_CREATE_DETACHED);
    }

    GT_IF_WITH_ASSERT(rc1 == 0)
    {
        // Create the OS thread and store the thread's pthread id as the thread's handle:
        int rc2 = ::pthread_create(&_threadHandle, &threadAttributes, threadEntryPoint, (void*)this);
        GT_IF_WITH_ASSERT(rc2 == 0)
        {
            // On Mac OS X:
#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
            {
                // Get the thread's mach thread id and store it as the thread's id:
                // (See above implementation note)
                _threadId = pthread_mach_thread_np(_threadHandle);
            }
#else
            {
                _threadId = _threadHandle;
            }
#endif

            // Log the thread creation into the debug log file:
            debugLogCreatedThread(_threadId, _threadName);

            retVal = true;
        }

        // Clean up:
        int rc3 = pthread_attr_destroy(&threadAttributes);
        GT_ASSERT(rc3 == 0);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osThread::breakExecution
// Description: Suspends the execution of this thread.
//
//              USE THIS FUNCTION WITH CARE !!!!!
//              (Suspending a thread that owns a synchronization object (mutex /
//               critical section / etc) can lead to a deadlock !!!
//
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        9/11/2006
// ---------------------------------------------------------------------------
bool osThread::breakExecution()
{
    bool retVal = false;

    // Yaki 11/9/2008:
    // pthreads can be suspended using pthread_cond_wait or similar condition function from
    // within the thread itself. However, as far as we know, POSIX does not offer functions
    // that enable suspending and resuming a thread from within another thread.
    // (HP-UX and Solar offer pthread_suspend() and pthread_continue(), but Linux does not seem to offer
    //  these functions)
    GT_ASSERT(false);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osThread::resumeExecution
// Description:
//   Increments the thread suspend count.
//   If the suspend count reach 0 - The thread run is resumed.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        9/11/2006
// ---------------------------------------------------------------------------
bool osThread::resumeExecution()
{
    bool retVal = false;

    // Yaki 11/9/2008:
    // pthreads can be suspended using pthread_cond_wait or similar condition function from
    // within the thread itself. However, as far as we know, POSIX does not offer functions
    // that enable suspending and resuming a thread from within another thread.
    // (HP-UX and Solar offer pthread_suspend() and pthread_continue(), but Linux does not seem to offer
    //  these functions)
    GT_ASSERT(false);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osThread::terminate
// Description: Terminates the OS thread.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        9/11/2006
// ---------------------------------------------------------------------------
bool osThread::terminate()
{
    bool retVal = false;

    // Enter the thread termination critical section:
    osCriticalSectionLocker csLocker(_threadTerminationCS);

    // If the thread is not alive:
    bool isThreadAlive = isAlive();

    if (!isThreadAlive)
    {
        // Nothing to be done:
        retVal = true;
    }
    else
    {
        // Issue "before termination" clean ups:
        beforeTermination();

        // Try to terminate the OS thread:
        int rc = ::pthread_cancel(_threadHandle);

        // if sync terminate wait until thread is not alive:
        if (m_syncTermination)
        {
            while (isAlive());
        }

        // If we managed to terminate the thread, or the thread was
        // already terminated:
        GT_IF_WITH_ASSERT((rc == 0) || (rc == ESRCH))
        {
            retVal = true;
        }
    }

    // Mark that the thread was terminated:
    _wasThreadTerminated = true;

    // Leave the thread termination critical section:
    csLocker.leaveCriticalSection();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osThread::isAlive
// Description: Returns true iff the thread is alive (it was executed and its
//              run was not ended).
// Author:      AMD Developer Tools Team
// Date:        9/11/2006
// ---------------------------------------------------------------------------
bool osThread::isAlive() const
{
    bool retVal = false;

    // If the thread was not terminated:
    if (!_wasThreadTerminated)
    {
        // If this thread has no handle, it surely isn't running:
        if (_threadHandle != OS_NO_THREAD_HANDLE)
        {
            // We send a dummy ("0") signal to the thread. If the function fails, the thread
            // is dead. If not, it's alive.
            // This is probably the best way to test this, though it could report a false
            // positive if the thread ID was reused...
            int retCode = ::pthread_kill(_threadHandle, 0);
            retVal = (retCode == 0);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osThread::entryPoint
// Description:
//   The thread entry point.
//   Should be overridden by sub-classes that would like the thread to execute their code.
//
// Return Val:  int - The thread return value.
// Author:      AMD Developer Tools Team
// Date:        9/11/2006
// ---------------------------------------------------------------------------
int osThread::entryPoint()
{
    return 0;
}


// ---------------------------------------------------------------------------
// Name:        osThread::beforeTermination
// Description: Is called before the thread is terminated. Enables sub classes
//              to clean up before the thread exits. (Sub classes can override this method).
//              NOTICE: (Sigal 23/8/10) Do not perform 'delete this' in this function overrides,
//              since we use the class members after the call to 'beforeTermination'
//              in the thread entry point
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        9/11/2006
// ---------------------------------------------------------------------------
void osThread::beforeTermination()
{
}


// ---------------------------------------------------------------------------
// Name:        osThread::debugLogCreatedThread
// Description: Log the thread creation into the debug log file.
// Arguments: createdThreadId - The created thread id.
// Author:      AMD Developer Tools Team
// Date:        5/5/2009
// ---------------------------------------------------------------------------
void osThread::debugLogCreatedThread(osThreadId createdThreadId, const gtString& threadName)
{
    // If we are in "debug" log severity:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        // Translate it into a string:
        gtString createdThreadIdAsStr;
        osThreadIdAsString(createdThreadId, createdThreadIdAsStr);

        // Output the created thread id to the debug log file:
        gtString dbgStr;
        dbgStr.appendFormattedString(OS_STR_OSThreadWasCreated, threadName.asCharArray());

        if (createdThreadId != OS_NO_THREAD_ID)
        {
            dbgStr += createdThreadIdAsStr;
        }
        else
        {
            dbgStr += L"N/A";
        }

        OS_OUTPUT_DEBUG_LOG(dbgStr.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}


// ---------------------------------------------------------------------------
// Name:        osThread::debugLogThreadRunStarted
// Description: Log the fact that the thread run started into the debug log.
// Arguments: threadId - The id of the thread that just started running.
// Author:      AMD Developer Tools Team
// Date:        5/5/2009
// ---------------------------------------------------------------------------
void osThread::debugLogThreadRunStarted(osThreadId threadId, const gtString& threadName)
{
    // If we are in "debug" log severity:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        // Translate the thread id into a string:
        gtString threadIdAsStr;
        osThreadIdAsString(threadId, threadIdAsStr);

        // Output the thread id to the debug log file:
        gtString dbgStr;
        dbgStr.appendFormattedString(OS_STR_OSThreadRunStarted, threadName.asCharArray());
        dbgStr += threadIdAsStr;
        OS_OUTPUT_DEBUG_LOG(dbgStr.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}

// ---------------------------------------------------------------------------
// Name:        osThread::waitForThreadEnd
// Description: Wait for the thread to end or until the timeout expires.
//              Returns true if thread ended in time. Returns false if timeout
//              was reached before the thread ended
// Arguments: maxTimeToWait - The length of the timeout
// Author:      AMD Developer Tools Team
// Date:        1/27/2016
// ---------------------------------------------------------------------------
bool osThread::waitForThreadEnd(const osTimeInterval& maxTimeToWait)
{
    bool isThreadEndedInTime = false;

    if (m_isJoinable)
    {
        struct timespec tsTimeout;
        int rc = 0;

        if (clock_gettime(CLOCK_REALTIME, &tsTimeout) == -1)
        {
            /* Handle error */
        }

        gtUInt64 seconds, nanoseconds;
        maxTimeToWait.getAsWholeSecondsAndRemainder(seconds, nanoseconds);

        tsTimeout.tv_sec += seconds;
        tsTimeout.tv_nsec += nanoseconds;

        rc = pthread_timedjoin_np(_threadHandle, NULL, &tsTimeout);

        if (0 == rc)
        {
            isThreadEndedInTime = true;
        }
    }

    return isThreadEndedInTime;
}

// ---------------------------------------------------------------------------
// Name:        osAreThreadHandlesEquivalent
// Description: checks if two thread handles represent the same thread
// Author:      AMD Developer Tools Team
// Date:        8/12/2008
// ---------------------------------------------------------------------------
bool osAreThreadHandlesEquivalent(const osThreadHandle& handle1, const osThreadHandle& handle2)
{
    bool retVal = false;

    if ((handle1 == OS_NO_THREAD_HANDLE) || (handle2 == OS_NO_THREAD_HANDLE))
    {
        // If one of the values is NULL, return true iff the other is NULL, too
        retVal = ((handle1 == OS_NO_THREAD_HANDLE) && (handle2 == OS_NO_THREAD_HANDLE));
    }
    else
    {
        // Check if the thread handles are equivalent:
        retVal = (::pthread_equal(handle1, handle2) != 0);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetCurrentThreadId
// Description: Returns the OS id for the calling thread.
// Author:      AMD Developer Tools Team
// Date:        23/12/2008
// ---------------------------------------------------------------------------
osThreadId osGetCurrentThreadId()
{
    osThreadId retVal = OS_NO_THREAD_ID;

#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
    {
        // Get the current pthread id:
        retVal = ::pthread_self();
    }
#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    {
        // Get the current Mach kernel thread id:
        retVal = mach_thread_self();
    }
#else
#error Error Unknown linux variant!
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetUniqueCurrentThreadId
// Description: Returns the unique OS id for the calling thread.
// Author:      AMD Developer Tools Team
// Date:        18/09/2015
// ---------------------------------------------------------------------------
osThreadId osGetUniqueCurrentThreadId()
{
    osThreadId retVal = OS_NO_THREAD_ID;

#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
    {
        // Get the current thread id:
        retVal = syscall(SYS_gettid);
    }
#else
#error Error Unknown linux variant!
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetCurrentThreadHandle
// Description: Returns a handle to the current thread
// Author:      AMD Developer Tools Team
// Date:        28/11/2006
// ---------------------------------------------------------------------------
osThreadHandle osGetCurrentThreadHandle()
{
    osThreadHandle retVal = OS_NO_THREAD_HANDLE;

    // Get the current pthread thread id:
    retVal = ::pthread_self();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osThreadIdAsString
// Description: Inputs a thread id and outputs it as a string.
// Arguments: threadId - The input thread id.
//            threadIdAsString - The output thread id as a string.
// Author:      AMD Developer Tools Team
// Date:        5/5/2009
// ---------------------------------------------------------------------------
void osThreadIdAsString(osThreadId threadId, gtString& threadIdAsString)
{
    threadIdAsString.makeEmpty();
    threadIdAsString.appendFormattedString(L"%lu", (unsigned long)threadId);
}


// ---------------------------------------------------------------------------
// Name:        osWaitForFlagToTurnOff
// Description:
//  Suspends the calling thread until an input flag is turned off.
//  Performs an efficient wait (does not load the CPU)
//
// Arguments: flag - The flag to which we wait.
//            timeOutMsec - Wait timeout, measured in msec.
// Return Val: bool  - true iff the flag was cleared before we reached the
//                     timeout period.
// Author:      AMD Developer Tools Team
// Date:        20/11/2005
// ---------------------------------------------------------------------------
bool osWaitForFlagToTurnOff(bool& flag, unsigned long timeOutMsec)
{
    bool retVal = false;

    // Represents 1 millisecond and 50 milliseconds time intervals:
    static struct timeval stat_milisecondTimeInterval;
    static struct timeval stat_fiftyMilisecondTimeInterval;

    // If the time intervals were not calculated yet:
    static bool stat_timeoutsWereCalculated = false;

    if (!stat_timeoutsWereCalculated)
    {
        osTimeValFromMilliseconds(1, stat_milisecondTimeInterval);
        osTimeValFromMilliseconds(50, stat_fiftyMilisecondTimeInterval);

        stat_timeoutsWereCalculated = true;
    }

    // If the flag was not turned off yet:
    if (flag)
    {
        // Start measuring our wait time:
        osStopWatch stopWatch;
        stopWatch.start();

        // Counts our waiting time (msec):
        unsigned long waitedSoFarMSec = 0;

        // Counts the amount of wait intervals:
        int waitedIntervalsCount = 0;

        // While the flag is on and we didn't meet the timeout interval:
        while (flag && (waitedSoFarMSec < timeOutMsec))
        {
            // We will wait 5 times, giving our CPU small time slice to other threads:
            if (waitedIntervalsCount < 5)
            {
                int rc1 = ::select(0, NULL, NULL, NULL, &stat_milisecondTimeInterval);
                GT_ASSERT(rc1 == 0);
            }
            else
            {
                // The rest of the wait intervals will be longer:
                int rc1 = ::select(0, NULL, NULL, NULL, &stat_fiftyMilisecondTimeInterval);
                GT_ASSERT(rc1 == 0);

                // Verify that we will not overflow the waited interval count:
                waitedIntervalsCount = 6;
            }

            // Update the "waited so far" time count:
            double waitedSoFarSec = 0;
            stopWatch.getTimeInterval(waitedSoFarSec);
            waitedSoFarMSec = long(waitedSoFarSec * 1000);

            // Increment the intervals count:
            waitedIntervalsCount++;
        }
    }

    // Return true iff the function execution was ended:
    retVal = !flag;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osWaitForFlagToTurnOn
// Description:
//  Suspends the calling thread until an input flag is turned on.
//  Performs an efficient wait (does not load the CPU)
//
// Arguments: flag - The flag to which we wait.
//            timeOutMsec - Wait timeout, measured in msec.
// Return Val: bool  - true iff the flag was set before we reached the
//                     timeout period.
// Author:      AMD Developer Tools Team
// Date:        20/11/2005
// ---------------------------------------------------------------------------
bool osWaitForFlagToTurnOn(bool& flag, unsigned long timeOutMsec)
{
    bool retVal = false;

    // Represents 1 millisecond and 50 milliseconds time intervals:
    static struct timeval stat_milisecondTimeInterval;
    static struct timeval stat_fiftyMilisecondTimeInterval;

    // If the time intervals were not calculated yet:
    static bool stat_timeoutsWereCalculated = false;

    if (!stat_timeoutsWereCalculated)
    {
        osTimeValFromMilliseconds(1, stat_milisecondTimeInterval);
        osTimeValFromMilliseconds(50, stat_fiftyMilisecondTimeInterval);

        stat_timeoutsWereCalculated = true;
    }

    // If the flag was not turned on yet:
    if (!flag)
    {
        // Start measuring our wait time:
        osStopWatch stopWatch;
        stopWatch.start();

        // Counts our waiting time (msec):
        unsigned long waitedSoFarMSec = 0;

        // Counts the amount of wait intervals:
        int waitedIntervalsCount = 0;

        // While the flag is off and we didn't meet the timeout interval:
        while (!flag && (waitedSoFarMSec < timeOutMsec))
        {
            // We will wait 5 times, giving our CPU small time slice to other threads:
            if (waitedIntervalsCount < 5)
            {
                int rc1 = ::select(0, NULL, NULL, NULL, &stat_milisecondTimeInterval);
                GT_ASSERT(rc1 == 0);
            }
            else
            {
                // The rest of the wait intervals will be longer:
                int rc1 = ::select(0, NULL, NULL, NULL, &stat_fiftyMilisecondTimeInterval);
                GT_ASSERT(rc1 == 0);

                // Verify that we will not overflow the waited interval count:
                waitedIntervalsCount = 6;
            }

            // Update the "waited so far" time count:
            double waitedSoFarSec = 0;
            stopWatch.getTimeInterval(waitedSoFarSec);
            waitedSoFarMSec = long(waitedSoFarSec * 1000);

            // Increment the intervals count:
            waitedIntervalsCount++;
        }
    }

    // Return true iff the function execution was ended:
    retVal = flag;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osSleep
// Description: Suspends the execution of the current thread until the
//              time-out interval elapses.
// Arguments: miliseconds - Timeout interval, measured in milliseconds.
// Author:      AMD Developer Tools Team
// Date:        4/6/2008
// ---------------------------------------------------------------------------
void osSleep(unsigned long miliseconds)
{
    timeval milisecondTimeInterval;
    osTimeValFromMilliseconds(miliseconds, milisecondTimeInterval);

    // The rest of the wait intervals will be longer:
    int rc1 = ::select(0, NULL, NULL, NULL, &milisecondTimeInterval);
    GT_ASSERT(rc1 == 0);
}


osProcessThreadsEnumerator::osProcessThreadsEnumerator() : m_pEnumHandler(NULL)
{
}

osProcessThreadsEnumerator::~osProcessThreadsEnumerator()
{
    deinitialize();
}

bool osProcessThreadsEnumerator::initialize(osProcessId pid)
{
    char filename[OS_MAX_PATH];
    snprintf(filename, sizeof(filename), "/proc/%d/task", pid);

    m_pEnumHandler = opendir(filename);
    return (NULL != m_pEnumHandler);
}

void osProcessThreadsEnumerator::deinitialize()
{
    if (NULL != m_pEnumHandler)
    {
        closedir(reinterpret_cast<DIR*>(m_pEnumHandler));
        m_pEnumHandler = NULL;
    }
}

bool osProcessThreadsEnumerator::next(gtUInt32& threadId)
{
    struct dirent entry, *pNext;
    bool ret = false;

    while (0 == readdir_r(reinterpret_cast<DIR*>(m_pEnumHandler), &entry, &pNext) && NULL != pNext)
    {
        if (isdigit(*entry.d_name))
        {
            threadId = static_cast<gtUInt32>(strtoul(entry.d_name, NULL, 10));
            ret = true;
            break;
        }
    }

    return ret;
}
