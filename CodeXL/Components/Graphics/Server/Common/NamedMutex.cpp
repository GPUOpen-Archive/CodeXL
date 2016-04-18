//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of a named mutex class. The mutex is
///         global systemwide, so can be accessed across threads and processes
///         by its name. Each process will need its own copy of the mutex
//==============================================================================

#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>

#if defined (_WIN32)
    #include <Windows.h>
#elif defined (_LINUX)
    #include <fcntl.h>       // For O_* constants
    #include <sys/stat.h>    // For mode constants
    #include <semaphore.h>
    #include <boost/interprocess/sync/named_recursive_mutex.hpp>

    using namespace boost::interprocess;
#endif   // _LINUX

#include "NamedMutex.h"
#include "misc.h"

/// Base Implementation abstract data type
class NamedMutexImpl
{
public:

    /// Constructor
    NamedMutexImpl() {};

    /// Destructor
    virtual ~NamedMutexImpl() {};

    /// Open an existing systemwide global mutex if it exists already, or create
    /// a new one if it doesn't exist
    /// \param mutexName the name this mutex will be known by
    /// \param initialOwner if true, the calling process will own this mutex, causing
    /// it to be started in the locked state
    /// \param global if true, the mutex is shared between 32 and 64 bit versions. If
    /// false, the 32 and 64 bit versions will have independent mutexes
    /// \return true if OpenOrCreate succeeded, false if error
    virtual bool  OpenOrCreate(const char* mutexName, bool initialOwner, bool global) = 0;

    /// Open a previously created systemwide global mutex
    /// \param mutexName the name this mutex is be known by
    /// \param inherit if true, processes created by this process will inherit
    /// the mutex
    /// \param global if true, the mutex is shared between 32 and 64 bit versions. If
    /// false, the 32 and 64 bit versions will have independent mutexes
    /// \return true if Open succeeded, false if mutex doesn't exist
    virtual bool  Open(const char* mutexName, bool inherit, bool global) = 0;

    /// Attempt to lock the mutex
    /// \return true if the Lock succeeded, false if not
    virtual bool  Lock() = 0;

    /// Unlock a previously locked mutex
    virtual void  Unlock() = 0;

    /// Close the mutex
    virtual void  Close() = 0;

private:

    /// copy constructor made private; Cannon make copies of this object
    NamedMutexImpl(const NamedMutexImpl& rhs)
    {
        PS_UNREFERENCED_PARAMETER(rhs);
    }

    /// assignment operator made private; Cannon make copies of this object
    NamedMutexImpl& operator= (const NamedMutexImpl& rhs)
    {
        PS_UNREFERENCED_PARAMETER(rhs);
        return *this;
    }
};

/// Windows-specific implementation
#if defined (_WIN32)
class NamedMutexWindows : public NamedMutexImpl
{
public:
    /// default constructor
    NamedMutexWindows()
        : m_hMutex(NULL)
    {
    }

    /// destructor
    virtual ~NamedMutexWindows()
    {
        Close();
    }

    //--------------------------------------------------------------------------
    /// Open an existing systemwide global mutex if it exists already, or create
    /// a new one if it doesn't exist
    /// \param mutexName the name this mutex will be known by
    /// \param initialOwner if true, the calling process will own this mutex, causing
    /// it to be started in the locked state
    /// \param global if true, the mutex is shared between 32 and 64 bit versions. If
    /// false, the 32 and 64 bit versions will have independent mutexes
    ///
    /// \return true if OpenOrCreate succeeded, false if error
    //--------------------------------------------------------------------------
    virtual bool  OpenOrCreate(const char* mutexName, bool initialOwner, bool global)
    {
        PS_UNREFERENCED_PARAMETER(global);

        if (m_hMutex == NULL)
        {
            // CreateMutex will create the mutex if it can't be opened
            m_hMutex = CreateMutex(NULL,                       // security attributes
                                   initialOwner,               // initial ownership?
                                   mutexName);                 // name

#ifdef DEBUG_PRINT
            printf("*** %d:%dCreating Mutex\n", osGetCurrentProcessId(), osGetCurrentThreadId());
#endif

            if (m_hMutex == NULL)
            {
                return false;
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Open a previously created systemwide global mutex
    /// \param mutexName the name this mutex is be known by
    /// \param inherit if true, processes created by this process will inherit
    /// the mutex
    /// \param global if true, the mutex is shared between 32 and 64 bit versions. If
    /// false, the 32 and 64 bit versions will have independent mutexes
    ///
    /// \return true if Open succeeded, false if mutex doesn't exist
    //--------------------------------------------------------------------------
    virtual bool  Open(const char* mutexName, bool inherit, bool global)
    {
        PS_UNREFERENCED_PARAMETER(global);
        m_hMutex = OpenMutexA(MUTEX_ALL_ACCESS,  // security attributes
                              inherit,           // inherit handle?
                              mutexName);        // name

        if (m_hMutex == NULL)
        {
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Attempt to lock the mutex
    ///
    /// \return true if the Lock succeeded, false if not
    //--------------------------------------------------------------------------
    virtual bool  Lock()
    {
        if (WAIT_OBJECT_0 != WaitForSingleObject(m_hMutex, INFINITE))
        {
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Unlock a previously locked mutex
    //--------------------------------------------------------------------------
    virtual void  Unlock()
    {
        ReleaseMutex(m_hMutex);
    }

    //--------------------------------------------------------------------------
    /// Close the mutex
    //--------------------------------------------------------------------------
    virtual void  Close()
    {
        if (m_hMutex != NULL)
        {
            CloseHandle(m_hMutex);
            m_hMutex = NULL;
        }
    }

private:
    HANDLE m_hMutex;     ///< Windows handle to mutex
};

#endif   // _WIN32

#if defined (_LINUX)
/// Linux (POSIX) implementation
/// Uses a POSIX semaphore to emulate a mutex, since semaphores are system-wide and
/// provide the naming required to identify individual mutexes
/// see http://linux.die.net/man/7/sem_overview

// Because of the way that Linux implements condition variables and mutexes
// (with shared memory), 32 and 64-bit versions need to be unique, since the
// shared memory consists of a data structure whose size is dependent on
// the bitsize.
#if defined X64
    static const char* EXT = "_x64";
#else
    static const char* EXT = "_x86";
#endif

static const int s_Mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

class NamedMutexPosix : public NamedMutexImpl
{
public:
    /// default constructor
    NamedMutexPosix()
        : m_mutex(NULL)
        , m_threadID(0)
        , m_lockCount(0)
        , m_owner(false)
    {
    }

    /// destructor
    virtual ~NamedMutexPosix()
    {
        Close();
    }

#ifdef DEBUG_PRINT
    // method to log messages to console. Can't use logger since this uses a mutex and would lead to recursion here
    void LogMessage(int messageType, const char* fmt, ...)
    {
        char* pMessageType = "Message";

        if (messageType == logERROR)
        {
            pMessageType = "Error";
        }

        char message[1024];
        va_list arg_ptr;
        va_start(arg_ptr, fmt);
        vsnprintf(message, 1024, fmt, arg_ptr);
        va_end(arg_ptr);

        printf("%s: PID: %11d TID: %12ld : %s", pMessageType, osGetCurrentProcessId(), osGetCurrentThreadId(), message);
    }
#endif

    //--------------------------------------------------------------------------
    /// Open an existing systemwide global mutex if it exists already, or create
    /// a new one if it doesn't exist
    /// \param mutexName the name this mutex will be known by
    /// \param initialOwner if true, the calling process will own this mutex, causing
    /// it to be started in the locked state
    /// \param global if true, the mutex is shared between 32 and 64 bit versions. If
    /// false, the 32 and 64 bit versions will have independent mutexes
    ///
    /// \return true if OpenOrCreate succeeded, false if error
    //--------------------------------------------------------------------------
    virtual bool  OpenOrCreate(const char* mutexName, bool initialOwner, bool global)
    {
#ifdef DEBUG_PRINT
        LogMessage(logMESSAGE, "NamedMutex:OpenOrCreate checking mutex\n");
#endif

        if (m_mutex == NULL)
        {
            char name[PS_MAX_PATH];

            if (global)
            {
                sprintf_s(name, PS_MAX_PATH, "/%s", mutexName);
            }
            else
            {
                sprintf_s(name, PS_MAX_PATH, "/%s%s", mutexName, EXT);
            }

            // try to create the mutex first
            m_mutex = sem_open(name, O_CREAT | O_EXCL, s_Mode, 1);
            m_owner = true;

            if (m_mutex == SEM_FAILED)
            {
                // create failed. Mutex already exists, so just open it
                m_mutex = sem_open(name, 0);
                m_owner = false;

                if (m_mutex == SEM_FAILED)
                {
#ifdef DEBUG_PRINT
                    LogMessage(logERROR, "Opening existing mutex failed\n");
#endif
                    return false;
                }

#ifdef DEBUG_PRINT
                LogMessage(logMESSAGE, "Opened an existing mutex %s (%s)\n", m_mutexName, mutexName);
#endif
            }

            strcpy(m_mutexName, name);
#ifdef DEBUG_PRINT
            LogMessage(logMESSAGE, "Created a new mutex %s\n", m_mutexName);
#endif

        }

        if (initialOwner)
        {
#ifdef DEBUG_PRINT
            LogMessage(logMESSAGE, "Initial owner - trying lock\n");
#endif
            return Lock();
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Open a previously created systemwide global mutex
    /// \param mutexName the name this mutex is be known by
    /// \param inherit if true, processes created by this process will inherit
    /// the mutex
    /// \param global if true, the mutex is shared between 32 and 64 bit versions. If
    /// false, the 32 and 64 bit versions will have independent mutexes
    ///
    /// \return true if Open succeeded, false if mutex doesn't exist
    //--------------------------------------------------------------------------
    virtual bool  Open(const char* mutexName, bool inherit, bool global)
    {
        PS_UNREFERENCED_PARAMETER(inherit);

        if (m_mutex == NULL)
        {
            char name[PS_MAX_PATH];

            if (global)
            {
                sprintf_s(name, PS_MAX_PATH, "/%s", mutexName);
            }
            else
            {
                sprintf_s(name, PS_MAX_PATH, "/%s%s", mutexName, EXT);
            }

            // open the mutex
            m_mutex = sem_open(name, 0);

            if (m_mutex != SEM_FAILED)
            {
                strcpy(m_mutexName, name);
#ifdef DEBUG_PRINT
                LogMessage(logMESSAGE, "NamedMutex:Open(%s) OK\n", m_mutexName);
#endif
                return true;
            }

#ifdef DEBUG_PRINT
            LogMessage(logMESSAGE, "NamedMutex:Open failed (%s)\n", m_mutexName);
#endif
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Attempt to lock the mutex
    ///
    /// \return true if the Lock succeeded, false if not
    //--------------------------------------------------------------------------
    virtual bool  Lock()
    {
#ifdef DEBUG_PRINT
        LogMessage(logMESSAGE, "NamedMutex:Lock - waiting..%s\n", m_mutexName);
#endif

        // if this thread currently has the lock, increment the lock count
        // lock count is initialized in constructor and when unlocking
        osThreadId threadID = osGetCurrentThreadId();

        if (threadID == m_threadID)
        {
            m_lockCount++;
#ifdef DEBUG_PRINT
            LogMessage(logMESSAGE, "thread already owns lock: ignoring. Lock count now %d\n", m_lockCount);
#endif
            return true;
        }

        if (sem_wait(m_mutex) == 0)      // lock the mutex and check for error
        {
#ifdef DEBUG_PRINT
            int value;
            sem_getvalue(m_mutex, &value);
            LogMessage(logMESSAGE, "NamedMutex: Lock acquired. Count is %d\n", value);
#endif
            m_threadID = threadID;
            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------
    /// Unlock a previously locked mutex
    //--------------------------------------------------------------------------
    virtual void  Unlock()
    {
#ifdef DEBUG_PRINT
        LogMessage(logMESSAGE, "NamedMutex:Unlock\n");
#endif
        osThreadId threadID = osGetCurrentThreadId();

        if (threadID == m_threadID)
        {
            m_lockCount--;
        }

        if (m_lockCount < 0)
        {
#ifdef DEBUG_PRINT
            LogMessage(logMESSAGE, "Unlocking\n");
#endif
            m_lockCount = 0;
            m_threadID = 0;
            sem_post(m_mutex);            // unlock the mutex
        }
    }

    //--------------------------------------------------------------------------
    /// Close the mutex
    //--------------------------------------------------------------------------
    virtual void  Close()
    {
#ifdef DEBUG_PRINT
        LogMessage(logMESSAGE, "NamedMutex:Close() %s\n", m_mutexName);
#endif
        sem_close(m_mutex);
        m_mutex = NULL;
        m_lockCount = 0;
        m_threadID = 0;

        if (m_owner)
        {
            sem_unlink(m_mutexName);
            m_owner = false;
        }
    }

private:
    sem_t*       m_mutex;                                  ///< pointer to mutex object
    osThreadId   m_threadID;                               ///< ID of the thread which currently owns the lock
    long         m_lockCount;                              ///< number of times this thread has called lock (non-recursive mutex)
    char         m_mutexName[PS_MAX_PATH];                 ///< name of mutex
    bool         m_owner;                                  ///< did this object create the mutex
};

/// Boost implementation
//  http://www.boost.org/doc/libs/1_48_0/doc/html/interprocess/synchronization_mechanisms.html
class NamedMutexBoost : public NamedMutexImpl
{
public:
    /// default constructor
    NamedMutexBoost()
        : m_mutex(NULL)
        , m_owner(false)
    {
    }

    /// destructor
    virtual ~NamedMutexBoost()
    {
        Close();
        delete m_mutex;
    }

    //--------------------------------------------------------------------------
    /// Open an existing systemwide global mutex if it exists already, or create
    /// a new one if it doesn't exist.
    /// For some reason, 32 and 64 bit mutexes don't play nice, so append the
    /// architecture type to the name to make 64 and 32 bit mutex names unique
    /// \param mutexName the name this mutex will be known by
    /// \param initialOwner if true, the calling process will own this mutex, causing
    /// it to be started in the locked state
    /// \param global if true, the mutex is shared between 32 and 64 bit versions. If
    /// false, the 32 and 64 bit versions will have independent mutexes
    ///
    /// \return true if OpenOrCreate succeeded, false if error
    //--------------------------------------------------------------------------
    virtual bool  OpenOrCreate(const char* mutexName, bool initialOwner, bool global)
    {
#ifdef DEBUG_PRINT
        printf("*** %d:%d Checking mutex\n", osGetCurrentProcessId(), osGetCurrentThreadId());
#endif

        if (m_mutex == NULL)
        {
#ifdef DEBUG_PRINT
            printf("*** >>>>>>>>>>>>> Creating a new mutex <<<<<<<<<<<<<\n");
#endif
            char name[PS_MAX_PATH];

            if (global)
            {
                strcpy(name, mutexName);
            }
            else
            {
                sprintf_s(name, PS_MAX_PATH, "%s%s", mutexName, EXT);
            }

            try
            {
                m_mutex = new named_recursive_mutex(open_only, name);
                m_owner = false;
            }
            catch (interprocess_exception&)
            {
                m_mutex = new named_recursive_mutex(open_or_create, name);
                m_owner = true;
            }

            strcpy(m_mutexName, name);
        }

        if (initialOwner)
        {
            return Lock();
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Open a previously created systemwide global mutex.
    /// For some reason, 32 and 64 bit mutexes don't play nice, so append the
    /// architecture type to the name to make 64 and 32 bit mutex names unique
    /// \param mutexName the name this mutex is be known by
    /// \param inherit if true, processes created by this process will inherit
    /// the mutex (currently unused)
    /// \param global if true, the mutex is shared between 32 and 64 bit versions. If
    /// false, the 32 and 64 bit versions will have independent mutexes
    ///
    /// \return true if Open succeeded, false if mutex doesn't exist
    //--------------------------------------------------------------------------
    virtual bool  Open(const char* mutexName, bool inherit, bool global)
    {
        PS_UNREFERENCED_PARAMETER(inherit);

        if (m_mutex == NULL)
        {
            try
            {
#ifdef DEBUG_PRINT
                printf("*** try to open_only mutex\n");
#endif

                char name[PS_MAX_PATH];

                if (global)
                {
                    strcpy(name, mutexName);
                }
                else
                {
                    sprintf_s(name, PS_MAX_PATH, "%s%s", mutexName, EXT);
                }

                m_mutex = new named_recursive_mutex(open_only, name);
                strcpy(m_mutexName, name);
                return true;
            }
            catch (interprocess_exception&)
            {
#ifdef DEBUG_PRINT
                printf("*** open_only exception caught\n");
#endif
                return false;
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Attempt to lock the mutex
    ///
    /// \return true if the Lock succeeded, false if not
    //--------------------------------------------------------------------------
    virtual bool  Lock()
    {
#ifdef DEBUG_PRINT
        printf("*** %d:%dTrying to lock mutex\n", osGetCurrentProcessId(), osGetCurrentThreadId());
#endif
        m_mutex->lock();
#ifdef DEBUG_PRINT
        printf("*** %d:%d Mutex locked\n", osGetCurrentProcessId(), osGetCurrentThreadId());
#endif
        return true;
    }

    //--------------------------------------------------------------------------
    /// Unlock a previously locked mutex
    //--------------------------------------------------------------------------
    virtual void  Unlock()
    {
        m_mutex->unlock();
    }

    //--------------------------------------------------------------------------
    /// Close the mutex
    /// In this implementation, the mutex is removed from the system if this
    /// object is the owner of the mutex ie this object was the one that created
    /// it. If any other processes or threads which use this mutex can continue
    /// to use it safely. A call to Open() will fail, since the mutex is no
    /// longer in the system, and a call to OpenOrCreate will create a new mutex
    //--------------------------------------------------------------------------
    virtual void  Close()
    {
        if (m_owner)
        {
            boost::interprocess::named_recursive_mutex::remove(m_mutexName);
            m_owner = false;
        }
    }

private:
    boost::interprocess::named_recursive_mutex* m_mutex;  ///< pointer to mutex objecti
    char m_mutexName[PS_MAX_PATH];                        ///< name of mutex
    bool m_owner;                                         ///< did this object create the mutex
};
#endif   // _LINUX

/// Main Implementation methods.
/// default constructor
/// Pick an implementation based on platform
NamedMutex::NamedMutex()
{
#if defined (_WIN32)
    m_pImpl = new NamedMutexWindows();
#else
    m_pImpl = new NamedMutexBoost();
#endif
}

/// destructor
NamedMutex::~NamedMutex()
{
    delete m_pImpl;
}

//--------------------------------------------------------------------------
/// Open an existing systemwide global mutex if it exists already, or create
/// a new one if it doesn't exist
/// \param mutexName the name this mutex will be known by
/// \param initialOwner if true, the calling process will own this mutex, causing
/// it to be started in the locked state
/// \param global if true, the mutex is shared between 32 and 64 bit versions. If
/// false, the 32 and 64 bit versions will have independent mutexes
///
/// \return true if OpenOrCreate succeeded, false if error
//--------------------------------------------------------------------------
bool   NamedMutex::OpenOrCreate(const char* mutexName, bool initialOwner, bool global)
{
    return m_pImpl->OpenOrCreate(mutexName, initialOwner, global);
}

//--------------------------------------------------------------------------
/// Open a previously created systemwide global mutex
/// \param mutexName the name this mutex is be known by
/// \param inherit if true, processes created by this process will inherit
/// the mutex
/// \param global if true, the mutex is shared between 32 and 64 bit versions. If
/// false, the 32 and 64 bit versions will have independent mutexes
///
/// \return true if Open succeeded, false if mutex doesn't exist
//--------------------------------------------------------------------------
bool   NamedMutex::Open(const char* mutexName, bool inherit, bool global)
{
    return m_pImpl->Open(mutexName, inherit, global);
}

//--------------------------------------------------------------------------
/// Attempt to lock the mutex
///
/// \return true if the Lock succeeded, false if not
//--------------------------------------------------------------------------
bool  NamedMutex::Lock()
{
    return m_pImpl->Lock();
}

//--------------------------------------------------------------------------
/// Unlock a previously locked mutex
//--------------------------------------------------------------------------
void  NamedMutex::Unlock()
{
    m_pImpl->Unlock();
}

//--------------------------------------------------------------------------
/// Close the mutex
//--------------------------------------------------------------------------
void  NamedMutex::Close()
{
    m_pImpl->Close();
}
