//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osThread.cpp
///
//=====================================================================

//------------------------------ osThread.cpp ------------------------------

// Win32:
#define WIN32_LEAN_AND_MEAN 1
// The following definition is needed to use GetThreadId, see its documentation in msdn.com
#define _WIN32_WINNT 0x0502

#include <Windows.h>
#include <process.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osTimeInterval.h>

// Static member initializations:
gtASCIIString osThread::ms_threadNamingPrefix = "AMDT";


// ---------------------------------------------------------------------------
// Name:        osThread::osThread
// Description: Constructor.
// Author:      AMD Developer Tools Team
// Date:        8/2/2004
// ---------------------------------------------------------------------------
osThread::osThread(const gtString& threadName, bool syncTermination, bool isJoinable)
    : _threadName(threadName)
    , _threadId(OS_NO_THREAD_ID)
    , _threadHandle(NULL)
    , _wasThreadTerminated(false)
    , m_syncTermination(syncTermination)
    , m_isJoinable(true) // All threads are joinable on Windows
{
    (void)isJoinable; // unused argument on Windows
}


// ---------------------------------------------------------------------------
// Name:        osThread::~osThread
// Description: Destructor - Terminate the wrapped OS thread.
// Author:      AMD Developer Tools Team
// Date:        8/2/2004
// ---------------------------------------------------------------------------
osThread::~osThread()
{
    terminate();
}


// ---------------------------------------------------------------------------
// Name:        osThread::threadEntryPoint
//
// Description: Serves as the Win32 thread entry point.
//              Calls osThread::entryPoint() of the osThread object that represents
//              the Win32 thread, and returns its return value.
//
// Arguments:   pParam - A pointer to the osThread object that represents
//                       the Win32 thread.
//
// Return Val:  int - The value returned by osThread::entryPoint();
//
// Author:      AMD Developer Tools Team
// Date:        8/2/2004
// ---------------------------------------------------------------------------
unsigned int __stdcall osThread::threadEntryPoint(void* pParam)
{
    int threadRetVal = -1;

    // Log the fact that the thread run started into the debug log:
    osThreadId currentThreadId = osGetCurrentThreadId();
    osThread* pThreadWrapper = (osThread*)pParam;
    osThread::debugLogThreadRunStarted(currentThreadId, pThreadWrapper->_threadName);

    if (!pThreadWrapper->_threadName.isEmpty())
    {
        // Name the thread in Visual Studio's thread's list:
        gtASCIIString threadNameASCII = pThreadWrapper->_threadName.asASCIICharArray();
        threadNameASCII.prepend(" - ").prepend(ms_threadNamingPrefix);
        osNameThreadInDebugger(currentThreadId, threadNameASCII);
    }

    // Get the osThread object that represents the OS thread:
    GT_IF_WITH_ASSERT(pThreadWrapper != NULL)
    {
        // Call osThread::entryPoint() of the osThread object that represents the Win32 thread:
        threadRetVal = pThreadWrapper->entryPoint();

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

    // Clean C runtime resources consumed by the thread:
    CloseHandle(pThreadWrapper->_threadHandle);
    pThreadWrapper->_threadHandle = NULL;
    pThreadWrapper->_threadId = OS_NO_THREAD_ID;
    _endthreadex(threadRetVal);

    return threadRetVal;
}


// ---------------------------------------------------------------------------
// Name:        osThread::execute
// Description: Starts executing this thread.
//              The thread execution start function will be the entryPoint()
//              function. Sub classes must implement the entryPoint() function.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/2/2004
// ---------------------------------------------------------------------------
bool osThread::execute()
{
    bool retVal = false;

    // Clear class members:
    _wasThreadTerminated = false;

    // Verify that the thread is not already running:
    if (_threadHandle == NULL)
    {
        // Create the thread in suspended mode:
        unsigned int threadId = 0;
        uintptr_t threadHandle = 0;
        threadHandle = _beginthreadex(NULL,             // Security attributes (same as the main thread  security attributes).
                                      0,                // Stack size (same as the main thread stack size).
                                      threadEntryPoint, // The thread entry point.
                                      this,             // Parameter passes to the thread's entry point.
                                      CREATE_SUSPENDED, // Create the thread in a suspended mode.
                                      &threadId);           // Will get the thread ID.

        if (threadHandle != 0)
        {
            // Store the created thread id and handle:
            _threadHandle = osThreadHandle(threadHandle);
            _threadId = threadId;

            // Log the thread creation into the debug log file:
            debugLogCreatedThread(_threadHandle, _threadName);

            // Start the thread execution:
            retVal = resumeExecution();
        }
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
// Date:        8/2/2004
// ---------------------------------------------------------------------------
bool osThread::breakExecution()
{
    bool retVal = false;

    // Suspend the thread execution:
    if (SuspendThread(_threadHandle) != -1)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osThread::resumeExecution
// Description:
//   Increments the thread suspend count.
//   If the suspend count reach 0 - The thread run is resumed.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/2/2004
// ---------------------------------------------------------------------------
bool osThread::resumeExecution()
{
    bool retVal = false;

    // Resume the Win32 thread execution:
    DWORD threadPreviousSuspendCount = ResumeThread(_threadHandle);

    if (threadPreviousSuspendCount != -1)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osThread::terminate
// Description: Terminates the OS thread.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/2/2004
// Implementation notes:
//   We use TerminateThread() to terminate the thread run.
//   The thread itself can call _endthreadex() to terminate its run.
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

        // Terminate the thread:
        if (TerminateThread(_threadHandle, 0xFFFFFFFFUL) != 0)
        {
            _threadId = OS_NO_THREAD_ID;
            _threadHandle = NULL;

            retVal = true;
        }

        // if sync terminate wait until thread is not alive:
        if (m_syncTermination)
        {
            while (isAlive());
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
// Date:        27/10/2004
// ---------------------------------------------------------------------------
bool osThread::isAlive() const
{
    bool retVal = false;

    // If the thread was not terminated:
    if (!_wasThreadTerminated && (_threadHandle != NULL))
    {
        // Check if the thread finished its execution:
        DWORD threadExitCode;

        if (GetExitCodeThread(_threadHandle, &threadExitCode) != NULL)
        {
            // If the thread is still active:
            if (threadExitCode == STILL_ACTIVE)
            {
                retVal = true;
            }
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
// Date:        17/5/2004
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
// Date:        2/3/2004
// ---------------------------------------------------------------------------
void osThread::beforeTermination()
{
}


// ---------------------------------------------------------------------------
// Name:        osThread::debugLogCreatedThread
// Description: Log the thread creation into the debug log file.
// Arguments: createdThreadHandle - The created thread handle.
// Author:      AMD Developer Tools Team
// Date:        5/5/2009
// ---------------------------------------------------------------------------
void osThread::debugLogCreatedThread(osThreadHandle createdThreadHandle, const gtString& threadName)
{
    // If we are in "debug" log severity:
    if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
    {
        // Will get the created thread id (if available):
        osThreadId createdThreadId = OS_NO_THREAD_ID;

        // Defining the GetThreadId function type.
        typedef DWORD (__stdcall * PFN_GET_THREAD_ID)(HANDLE);

        // Get a pointer to the GetThreadId function (defined in KERNEL32.DLL):
        PFN_GET_THREAD_ID pGetThreadId = (PFN_GET_THREAD_ID)GetProcAddress(GetModuleHandle(L"KERNEL32.DLL"), "GetThreadId");

        // If the function exists in KERNEL32.DLL:
        // (The function documentation specifies that it exists on Windows Vista and Windows Server 2003)
        if (pGetThreadId != NULL)
        {
            // Get the created thread id:
            createdThreadId = pGetThreadId(createdThreadHandle);
        }

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
//              was reached before the thread ended, or if the thread is not joinable
// Arguments: maxTimeToWait - The length of the timeout
// Author:      AMD Developer Tools Team
// Date:        1/27/2016
// ---------------------------------------------------------------------------
bool osThread::waitForThreadEnd(const osTimeInterval& maxTimeToWait)
{
    bool isThreadEndedInTime = false;

    if (m_isJoinable)
    {
        double timeoutInMilliSeconds = 0.0;
        maxTimeToWait.getAsMilliSeconds(timeoutInMilliSeconds);
        DWORD rc = WaitForSingleObject(_threadHandle, static_cast<unsigned int>(timeoutInMilliSeconds));

        if (WAIT_OBJECT_0 == rc)
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
    /*  // Uri, 14/12/08: This is the "right" method of doing this check, as it ignores any
        // pseudo-handles and such oddities. However, the "GetThreadId" function is only
        // supported on windows Vista and up.

        // Get the IDs of both threads and compare them:
        osThreadId threadId1 = ::GetThreadId(handle1);
        osThreadId threadId2 = ::GetThreadId(handle2);

        bool retVal = (threadId1 == threadId2);
        */
    bool retVal = (handle1 == handle2);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetCurrentThreadId
// Description: Returns the calling thread's id.
// Author:      AMD Developer Tools Team
// Date:        28/11/2006
// ---------------------------------------------------------------------------
osThreadId osGetCurrentThreadId()
{
    // Get the current thread OS id:
    DWORD retVal = ::GetCurrentThreadId();
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetUniqueCurrentThreadId
// Description:
//   Returns the calling thread's id. No difference from osGetCurrentThreadId
//   on Windows.
// Author:      AMD Developer Tools Team
// Date:        18/09/2015
// ---------------------------------------------------------------------------
osThreadId osGetUniqueCurrentThreadId()
{
    osThreadId retVal = osGetCurrentThreadId();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetCurrentThreadHandle
// Description: Returns a handle to the current thread
// Author:      AMD Developer Tools Team
// Date:        8/12/2008
// ---------------------------------------------------------------------------
osThreadHandle osGetCurrentThreadHandle()
{
    HANDLE retVal = ::GetCurrentThread();
    HANDLE realHandle = OS_NO_THREAD_HANDLE;

    // In Windows, the result of GetCurrentThread is a pseudohandle, thus we have to
    // duplicate it to get a real handle:
    BOOL dupSucceeded = ::DuplicateHandle(::GetCurrentProcess(),    // Source process
                                          retVal,                   // Source Handle
                                          ::GetCurrentProcess(),    // Target process
                                          &realHandle,              // Target Handle
                                          0,                        // Ignored because of DUPLICATE_SAME_ACCESS
                                          TRUE,                     // Can this handle be inherited?
                                          DUPLICATE_SAME_ACCESS);   // Copy the access rights from the original handle

    // If we failed, we return the pseudohandle, which suffices in most cases.
    if (dupSucceeded != FALSE)
    {
        retVal = realHandle;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetThreadStartTime
// Description: Retrieve the start time for a thread with the given Id.
// Arguments:   threadId - The input thread id.
//              startTime - The output start time of the thread.
// Author:      AMD Developer Tools Team
// Date:        3/31/2016
// ---------------------------------------------------------------------------
bool osGetThreadStartTime(osThreadId threadId, osTime& startTime)
{
    bool gotStartTime = false;
    startTime = 0;

    // Get the thread handle from id.
    HANDLE threadHandle = OpenThread(THREAD_ALL_ACCESS, TRUE, threadId);

    if (threadHandle != NULL)
    {
        FILETIME createTime = {};
        FILETIME dummyTime = {};
        BOOL bGotTimes = GetThreadTimes(threadHandle, &createTime, &dummyTime, &dummyTime, &dummyTime);
        CloseHandle(threadHandle);

        if (bGotTimes == TRUE)
        {
            // Convert the createTime to osTime and return.
            gtInt64* createTimeSecondsSinceEpoch = reinterpret_cast<gtInt64*>(&createTime);
            startTime = *createTimeSecondsSinceEpoch;

            gotStartTime = true;
        }
    }

    return gotStartTime;
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
            // We will wait 5 times, giving our CPU time slice to threads that have the
            // same priority as this thread, keeping this thread in a "ready to run" mode:
            if (waitedIntervalsCount < 5)
            {
                ::Sleep(0);
            }
            else
            {
                // The rest of the wait intervals will throw this thread off the CPU
                // for at least 1 msec:
                ::Sleep(1);

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
            // We will wait 5 times, giving our CPU time slice to threads that have the
            // same priority as this thread, keeping this thread in a "ready to run" mode:
            if (waitedIntervalsCount < 5)
            {
                ::Sleep(0);
            }
            else
            {
                // The rest of the wait intervals will throw this thread off the CPU
                // for at least 1 msec:
                ::Sleep(1);

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
    ::Sleep(miliseconds);
}


// ---------------------------------------------------------------------------
// Name:        osNameThreadInDebugger
// Description:
//  Names a thread so that debuggers (like Visual Studio) will be able to display it's
//  name in the threads list.
// Arguments:
//  threadId - The thread's id.
//  threadName - The thread's name.
// Author:      AMD Developer Tools Team
// Date:        25/11/2010
// Implementation notes:
//  This code was copied from few articles discussing this issue (google for "SetThreadName").
//  Among these articles are:
//  - Setting a Thread Name (Unmanaged) - http://msdn.microsoft.com/en-us/library/xcb2z8hs(VS.71).aspx
//  - Name your threads in the VC debugger thread list - http://www.codeproject.com/KB/threads/Name_threads_in_debugger.aspx
// ---------------------------------------------------------------------------
void osNameThreadInDebugger(osThreadId threadId, const gtASCIIString& threadName)
{
    // A struct that will contain the thread's name:
    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType;       // Must be 0x1000
        LPCSTR szName;      // Pointer to the thread's name (in user addr space)
        DWORD dwThreadID;   // The thread's id (-1 - use the caller thread's id)
        DWORD dwFlags;      // reserved for future use, must be zero
    } THREADNAME_INFO;

    // Create a structure instance containing the thread's id and name:
    THREADNAME_INFO threadNameInfo;
    {
        threadNameInfo.dwType = 0x1000;
        threadNameInfo.szName = threadName.asCharArray();
        threadNameInfo.dwThreadID = threadId;
        threadNameInfo.dwFlags = 0;
    }

    // Visual Studio's debugger gets the information about a thread's name via the exception OS_THREAD_NAMING_EXCEPTION_CODE (0x406D1388).
    // So, we trigger this exception with the appropriate argument:
    __try
    {
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        {
            RaiseException(OS_THREAD_NAMING_EXCEPTION_CODE, 0, sizeof(threadNameInfo) / sizeof(DWORD), (DWORD*)&threadNameInfo);
        }
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        {
            RaiseException(OS_THREAD_NAMING_EXCEPTION_CODE, 0, sizeof(threadNameInfo) / sizeof(ULONG_PTR), (ULONG_PTR*)&threadNameInfo);
        }
#else
        {
#error unknown address space
        }
#endif
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
    }
}


osProcessThreadsEnumerator::osProcessThreadsEnumerator() : m_pEnumHandler(INVALID_HANDLE_VALUE)
{
    m_te32.dwSize = sizeof(m_te32);
}

osProcessThreadsEnumerator::~osProcessThreadsEnumerator()
{
    deinitialize();
}

bool osProcessThreadsEnumerator::initialize(osProcessId pid)
{
    m_pEnumHandler = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);
    bool ret = false;

    if (INVALID_HANDLE_VALUE != m_pEnumHandler)
    {
        if (Thread32First(m_pEnumHandler, &m_te32) != FALSE)
        {
            ret = true;

            while (m_te32.th32OwnerProcessID != pid)
            {
                if (Thread32Next(m_pEnumHandler, &m_te32) == FALSE)
                {
                    ret = false;
                    break;
                }
            }
        }
        else
        {
            deinitialize();
        }
    }

    return ret;
}

void osProcessThreadsEnumerator::deinitialize()
{
    if (INVALID_HANDLE_VALUE != m_pEnumHandler)
    {
        CloseHandle(m_pEnumHandler);
        m_pEnumHandler = INVALID_HANDLE_VALUE;
    }
}

bool osProcessThreadsEnumerator::next(gtUInt32& threadId)
{
    threadId = m_te32.th32ThreadID;

    const DWORD pid = m_te32.th32OwnerProcessID;

    do
    {
        if (Thread32Next(m_pEnumHandler, &m_te32) == FALSE)
        {
            m_te32.th32ThreadID = OS_NO_THREAD_ID;
            break;
        }
    }
    while (pid != m_te32.th32OwnerProcessID);

    return OS_NO_THREAD_ID != threadId;
}


