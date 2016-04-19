//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  a named event class. The event is
///         global systemwide, so can be accessed across threads and processes
///         by its name. Each process will need its own copy of the event
//==============================================================================

#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>

#if defined (_WIN32)
    #include <Windows.h>
#elif defined (_LINUX)
    #include <boost/interprocess/sync/named_recursive_mutex.hpp>
    #include <boost/interprocess/sync/named_condition.hpp>
    #include <boost/thread/locks.hpp>
    #include "WinDefs.h"

    using namespace boost::interprocess;
#endif

#include "NamedEvent.h"
#include "SharedMemory.h"
#include "defines.h"
#include "misc.h"

/// Base Implementation abstract data type
class NamedEventImpl
{
public:

    /// Constructor
    NamedEventImpl() {};

    /// Destructor
    virtual ~NamedEventImpl() {};

    //--------------------------------------------------------------------------
    /// Create a system-wide event
    /// \param eventName the name this event will be known by
    /// \param signaled if true, the event will be created in the signaled state
    ///
    /// \return true if Create succeeded, false if error
    //--------------------------------------------------------------------------
    virtual bool  Create(const char* eventName, bool signaled) = 0;

    //--------------------------------------------------------------------------
    /// Open a previously created system-wide event
    /// \param eventname the name this event is be known by (the name it was
    /// given when created)
    /// \param inherit if true, processes created by this process will inherit
    /// the event handle
    ///
    /// \return true if Open succeeded, false if event doesn't exist
    //--------------------------------------------------------------------------
    virtual bool  Open(const char* eventname, bool inherit) = 0;

    //--------------------------------------------------------------------------
    /// Wait for a signal/event to happen. This function won't return until it
    /// receives a signal
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    virtual bool  Wait() = 0;

    //--------------------------------------------------------------------------
    /// Set the event to the signaled state. Any other threads/processes waiting
    /// for the signal can now proceed.
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    virtual bool  Signal() = 0;

    //--------------------------------------------------------------------------
    /// Is the event in the Signaled state. Does not block or wait but returns
    /// immediately
    ///
    /// \return true if currently signaled, false if not
    //--------------------------------------------------------------------------
    virtual bool  IsSignaled() = 0;

    //--------------------------------------------------------------------------
    /// Reset the event to non-signaled
    //--------------------------------------------------------------------------
    virtual void  Reset() = 0;

    //--------------------------------------------------------------------------
    /// Close the event
    //--------------------------------------------------------------------------
    virtual void  Close()
    {
    }

private:
    /// copy constructor made private; Cannon make copies of this object
    NamedEventImpl(const NamedEventImpl& rhs)
    {
        PS_UNREFERENCED_PARAMETER(rhs);
    }

    /// assignment operator made private; Cannon make copies of this object
    NamedEventImpl& operator= (const NamedEventImpl& rhs)
    {
        PS_UNREFERENCED_PARAMETER(rhs);
        return *this;
    }
};

/// Windows-specific implementation
#if defined (_WIN32)
class NamedEventWindows : public NamedEventImpl
{
public:
    /// default constructor
    NamedEventWindows()
        : m_hEvent(NULL)
    {
    }

    /// destructor
    ~NamedEventWindows()
    {
        Close();
    }

    //--------------------------------------------------------------------------
    /// Create a system-wide event
    /// \param eventName the name this event will be known by
    /// \param signaled if true, the event will be created in the signaled state
    ///
    /// \return true if Create succeeded, false if error
    //--------------------------------------------------------------------------
    virtual bool  Create(const char* eventName, bool signaled)
    {
        m_hEvent = CreateEvent(NULL, TRUE, signaled, eventName);

        if (m_hEvent == NULL)
        {
            return false;
        }

#ifdef DEBUG_PRINT
        printf("** Creating event %s (%d)\n", eventName, m_hEvent);
#endif
        return true;
    }

    //--------------------------------------------------------------------------
    /// Open a previously created system-wide event
    /// \param eventName the name this event is be known by (the name it was
    /// given when created)
    /// \param inherit if true, processes created by this process will inherit
    /// the event handle
    ///
    /// \return true if Open succeeded, false if event doesn't exist
    //--------------------------------------------------------------------------
    virtual bool  Open(const char* eventName, bool inherit)
    {
        m_hEvent = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, inherit, eventName);

        if (m_hEvent == NULL)
        {
            return false;
        }

#ifdef DEBUG_PRINT
        printf("** Opening event %s (%d)\n", eventName, m_hEvent);
#endif
        return true;
    }

    //--------------------------------------------------------------------------
    /// Wait for a signal/event to happen. This function won't return until it
    /// receives a signal
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    virtual bool  Wait()
    {
#ifdef DEBUG_PRINT
        printf("** Waiting ... (%d)\n", m_hEvent);
#endif

        if (WAIT_OBJECT_0 != WaitForSingleObject(m_hEvent, INFINITE))
        {
            return false;
        }

#ifdef DEBUG_PRINT
        printf("** Wait done (%d)\n", m_hEvent);
#endif
        return true;
    }

    //--------------------------------------------------------------------------
    /// Set the event to the signaled state. Any other threads/processes waiting
    /// for the signal can now proceed.
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    virtual bool  Signal()
    {
#ifdef DEBUG_PRINT
        printf("** Signal (%d)\n", m_hEvent);
#endif

        if (SetEvent(m_hEvent) == FALSE)
        {
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Is the event in the Signaled state. Does not block or wait but returns
    /// immediately
    ///
    /// \return true if currently signaled, false if not
    //--------------------------------------------------------------------------
    virtual bool  IsSignaled()
    {
        DWORD dwObject = WaitForSingleObject(m_hEvent, 0);

        if (WAIT_OBJECT_0 == dwObject)
        {
            return true;
        }

        if (dwObject == WAIT_FAILED)
        {
            Log(logERROR, "Failed to wait on an event (Error %d).\n", GetLastError());
        }

        return false;
    }

    //--------------------------------------------------------------------------
    /// reset the event to non-signaled
    //--------------------------------------------------------------------------
    virtual void  Reset()
    {
#ifdef DEBUG_PRINT
        printf("** Reset (%d)\n", m_hEvent);
#endif
        ResetEvent(m_hEvent);
    }

    //--------------------------------------------------------------------------
    /// close the event
    //--------------------------------------------------------------------------
    virtual void  Close()
    {
#ifdef DEBUG_PRINT
        printf("** Close (%d)\n", m_hEvent);
#endif

        if (m_hEvent != NULL)
        {
            CloseHandle(m_hEvent);
            m_hEvent = NULL;
        }
    }

private:
    void*    m_hEvent;   ///< Windows pointer to event handle
};
#endif   // _WIN32

#if defined (_LINUX)
// Boost implementation. Uses a condition variable and a mutex
// The condition variable is used by the OS to detect signals
// the signal state, which is used by this implementation is in
// shared memory, so can be accessed by all threads and processes

static const int BUFFER_SIZE = 16;  ///< size of shared memory buffer, in bytes

// Because of the way that boost implements condition variables and mutexes
// (with shared memory), 32 and 64-bit versions need to be unique, since the
// shared memory consists of a data structure whose size is dependent on
// the bitsize.
#if defined X64
    static const char* EXT = "_x64";
#else
    static const char* EXT = "_x86";
#endif

class NamedEventBoost : public NamedEventImpl
{
public:
    /// default constructor
    NamedEventBoost()
        : m_mutex(NULL)
        , m_condition(NULL)
        , m_owner(false)
    {
        m_signalState = new SharedMemory();
    }

    /// destructor
    ~NamedEventBoost()
    {
        m_signalState->Close();
        delete m_condition;
        delete m_mutex;
        delete m_signalState;
    }

    //--------------------------------------------------------------------------
    /// Create a system-wide event
    /// \param eventName the name this event will be known by
    /// \param signaled if true, the event will be created in the signaled state
    ///
    /// \return true if Create succeeded, false if error
    //--------------------------------------------------------------------------
    virtual bool  Create(const char* eventName, bool signaled)
    {
#ifdef DEBUG_PRINT
        printf("NamedEvent: Create\n");
#endif
        char strTemp[PS_MAX_PATH];

        sprintf_s(m_mutexName, PS_MAX_PATH, "%s_mutex%s", eventName, EXT);

        if (m_mutex == NULL)
        {
            try
            {
                m_mutex = new named_mutex(open_only, m_mutexName);
            }
            catch (interprocess_exception&)
            {
                m_mutex = new named_mutex(open_or_create, m_mutexName);
                m_owner = true;
            }
        }

        sprintf_s(m_conditionName, PS_MAX_PATH, "%s_condition%s", eventName, EXT);

        if (m_condition == NULL)
        {
            m_condition = new named_condition(open_or_create, m_conditionName);
        }

        sprintf_s(strTemp, PS_MAX_PATH, "%s_memory", eventName);
        SharedMemory::MemStatus status = m_signalState->OpenOrCreate(BUFFER_SIZE, strTemp);

        if (SharedMemory::SUCCESS != status && SharedMemory::SUCCESS_ALREADY_CREATED != status)
        {
            return false;
        }

        if (signaled)
        {
            Signal();
        }
        else
        {
            Reset();
        }

#ifdef DEBUG_PRINT
        printf("NamedEvent: Create done\n");
#endif
        return true;
    }

    //--------------------------------------------------------------------------
    /// Open a previously created system-wide event
    /// \param eventName the name this event is be known by (the name it was
    /// given when created)
    /// \param inherit if true, processes created by this process will inherit
    /// the event (currently unused)
    ///
    /// \return true if Open succeeded, false if event doesn't exist
    //--------------------------------------------------------------------------
    virtual bool  Open(const char* eventName, bool inherit)
    {
        PS_UNREFERENCED_PARAMETER(inherit);
#ifdef DEBUG_PRINT
        printf("NamedEvent: Open\n");
#endif
        char strTemp[PS_MAX_PATH];

        sprintf_s(strTemp, PS_MAX_PATH, "%s_mutex%s", eventName, EXT);

        if (m_mutex == NULL)
        {
            try
            {
#ifdef DEBUG_PRINT
                printf("*** try to open_only mutex\n");
#endif
                m_mutex = new named_mutex(open_only, strTemp);
            }
            catch (interprocess_exception&)
            {
#ifdef DEBUG_PRINT
                printf("*** mutex open_only exception caught\n");
#endif
                return false;
            }
        }

        sprintf_s(strTemp, PS_MAX_PATH, "%s_condition%s", eventName, EXT);

        if (m_condition == NULL)
        {
            try
            {
#ifdef DEBUG_PRINT
                printf("*** try to open_only condition variable %s\n", strTemp);
#endif
                m_condition = new named_condition(open_only, strTemp);
            }
            catch (interprocess_exception&)
            {
#ifdef DEBUG_PRINT
                printf("*** condition var open_only exception caught\n");
#endif
                return false;
            }
        }

        sprintf_s(strTemp, PS_MAX_PATH, "%s_memory", eventName);

        if (SharedMemory::SUCCESS != m_signalState->Open(strTemp))
        {
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Wait for a signal/event to happen. This function won't return until it
    /// receives a signal
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    virtual bool  Wait()
    {
#ifdef DEBUG_PRINT
        printf("NamedEvent: Wait\n");
#endif
        boost::interprocess::scoped_lock<named_mutex> lock(*m_mutex);
        bool* state = (bool*)m_signalState->Get();

        while (! *state)
        {
#ifdef DEBUG_PRINT
            printf("NamedEvent: waiting\n");
#endif
            m_condition->wait(lock);
            state = (bool*)m_signalState->Get();
        }

#ifdef DEBUG_PRINT
        printf("NamedEvent: Wait done\n");
#endif
        return true;
    }

    //--------------------------------------------------------------------------
    /// Set the event to the signaled state. Any other threads/processes waiting
    /// for the signal can now proceed.
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    virtual bool  Signal()
    {
#ifdef DEBUG_PRINT
        printf("NamedEvent: Signal\n");
#endif
        boost::interprocess::scoped_lock<named_mutex> lock(*m_mutex);
        bool* state = (bool*)m_signalState->Get();
        *state = true;
        m_condition->notify_all();
        return true;
    }

    //--------------------------------------------------------------------------
    /// Is the event in the Signaled state. Does not block or wait but returns
    /// immediately
    ///
    /// \return true if currently signaled, false if not
    //--------------------------------------------------------------------------
    virtual bool  IsSignaled()
    {
#ifdef DEBUG_PRINT
        printf("NamedEvent: IsSignaled\n");
#endif
        boost::interprocess::scoped_lock<named_mutex> lock(*m_mutex);
        bool* state = (bool*)m_signalState->Get();
        return *state;
    }

    //--------------------------------------------------------------------------
    /// reset the event to non-signaled
    //--------------------------------------------------------------------------
    virtual void  Reset()
    {
#ifdef DEBUG_PRINT
        printf("NamedEvent: Reset\n");
#endif
        boost::interprocess::scoped_lock<named_mutex> lock(*m_mutex);
        bool* state = (bool*)m_signalState->Get();
        *state = false;
    }

    //--------------------------------------------------------------------------
    /// close the event
    //--------------------------------------------------------------------------
    virtual void  Close()
    {
        if (m_owner)
        {
            boost::interprocess::named_mutex::remove(m_mutexName);
            boost::interprocess::named_condition::remove(m_conditionName);
        }

        m_signalState->Close();
    }

private:
    named_mutex*     m_mutex;        ///< The mutex object used to protect the condition variable
    named_condition* m_condition;    ///< The condition variable used by the OS to detect signals
    SharedMemory*    m_signalState;  ///< The state of the signal. Used by this implementation to set or reset the signal state.
    bool             m_owner;        ///< Did this object create the event

    char             m_mutexName[PS_MAX_PATH];
    char             m_conditionName[PS_MAX_PATH];
};

// POSIX implementation
// Documentation would suggest using a semaphore
//  Win32 function         Linux threads                      Linux processes
//  CreateMutex         -> pthreads_mutex_init                semget / semctl
//  OpenMutex           -> n/a                                semget
//  WaitForSingleObject -> pThreads_mutex_lock/..tex_trylock  semop
//  ReleaseMutex        -> pthreads_mutex_unlock              semop
//  CloseHandle         -> pthreads_mutex_destroy             senctl

// There's also boost::named_condition
// Poco.NamedEvent (though these are auto-resetting)
/*
class NamedEventLinux : public NamedEventImpl
{
public:
   NamedEventLinux()
   {
   }

   ~NamedEventLinux()
   {
   }

   bool  Create(const char* eventName)
   {
      eventName;
      return true;
   }

   bool  Open(const char* eventName)
   {
      eventName;
      return true;
   }

   bool  Wait()
   {
      return true;
   }

   bool  Signal()
   {
      return true;
   }

   void  Reset()
   {
   }
};
*/
#endif   // _LINUX

/// Main Implementation methods.
/// default constructor
/// Pick an implementation based on platform
NamedEvent::NamedEvent()
{
#if defined (_WIN32)
    m_pImpl = new NamedEventWindows();
#else
    m_pImpl = new NamedEventBoost();
#endif
}

/// destructor
NamedEvent::~NamedEvent()
{
    delete m_pImpl;
}

//--------------------------------------------------------------------------
/// Create a system-wide event
/// \param eventName the name this event will be known by
/// \param signaled if true, the event will be created in the signaled state
///
/// \return true if Create succeeded, false if error
//--------------------------------------------------------------------------
bool  NamedEvent::Create(const char* eventName, bool signaled)
{
    return m_pImpl->Create(eventName, signaled);
}

//--------------------------------------------------------------------------
/// Open a previously created system-wide event
/// \param eventName the name this event is be known by (the name it was
/// given when created)
/// \param inherit if true, processes created by this process will inherit
/// the event
///
/// \return true if Open succeeded, false if event doesn't exist
//--------------------------------------------------------------------------
bool  NamedEvent::Open(const char* eventName, bool inherit)
{
    return m_pImpl->Open(eventName, inherit);
}

//--------------------------------------------------------------------------
/// Wait for a signal/event to happen. This function won't return until it
/// receives a signal
///
/// \return true if successful, false if error
//--------------------------------------------------------------------------
bool  NamedEvent::Wait()
{
    return m_pImpl->Wait();
}

//--------------------------------------------------------------------------
/// Set the event to the signaled state. Any other threads/processes waiting
/// for the signal can now proceed.
///
/// \return true if successful, false if error
//--------------------------------------------------------------------------
bool  NamedEvent::Signal()
{
    return m_pImpl->Signal();
}

//--------------------------------------------------------------------------
/// Is the event in the Signaled state. Does not block or wait but returns
/// immediately
///
/// \return true if currently signaled, false if not
//--------------------------------------------------------------------------
bool  NamedEvent::IsSignaled()
{
    return m_pImpl->IsSignaled();
}

//--------------------------------------------------------------------------
/// reset the event to non-signaled
//--------------------------------------------------------------------------
void  NamedEvent::Reset()
{
    m_pImpl->Reset();
}

//--------------------------------------------------------------------------
/// close the event
//--------------------------------------------------------------------------
void  NamedEvent::Close()
{
    m_pImpl->Close();
}
