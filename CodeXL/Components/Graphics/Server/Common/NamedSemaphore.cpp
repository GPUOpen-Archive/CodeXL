//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  an implementation of a named semaphore class. The
///         semaphore is global systemwide, so can be accessed across threads
///         and processes by its name. Each process will need its own copy of
///         the semaphore
//==============================================================================

#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osSystemError.h>

//#define DEBUG_PRINT 1
static const int MAX_SEM_COUNT = 100; ///< Max semaphore count

#if defined (_WIN32)
    #include <Windows.h>
#elif defined (_LINUX)
    #include <fcntl.h>       // For O_* constants
    #include <sys/stat.h>    // For mode constants
    #include <semaphore.h>
    #include <boost/interprocess/sync/named_recursive_mutex.hpp>
    #include <boost/interprocess/sync/named_mutex.hpp>
    #include <boost/interprocess/sync/named_condition.hpp>
    #include <boost/thread/locks.hpp>

    using namespace boost::interprocess;
#endif // _LINUX

#include "NamedSemaphore.h"
#include "SharedMemory.h"
#include "defines.h"

/// Base Implementation abstract data type
class NamedSemaphoreImpl
{
public:

    /// Constructor
    NamedSemaphoreImpl() {};

    /// Destructor
    virtual ~NamedSemaphoreImpl() {};

    /// Create a new semaphore
    /// \param semaphoreName Input name
    /// \return True if success, false if fail.
    virtual bool  Create(const char* semaphoreName) = 0;

    /// Open a semaphore
    /// \param semaphoreName Input name
    /// \return True if success, false if fail.
    virtual bool  Open(const char* semaphoreName) = 0;

    /// Wait
    /// \return True if success, false if fail.
    virtual bool  Wait() = 0;

    /// Signal
    /// \return True if success, false if fail.
    virtual bool  Signal() = 0;

    /// Close
    virtual void  Close()
    {
    }

private:

    /// copy constructor made private; Cannon make copies of this object
    NamedSemaphoreImpl(const NamedSemaphoreImpl& rhs)
    {
        PS_UNREFERENCED_PARAMETER(rhs);
    }

    /// assignment operator made private; Cannon make copies of this object
    NamedSemaphoreImpl& operator= (const NamedSemaphoreImpl& rhs)
    {
        PS_UNREFERENCED_PARAMETER(rhs);
        return *this;
    }
};

/// Windows-specific implementation
#if defined (_WIN32)
class NamedSemaphoreWindows : public NamedSemaphoreImpl
{
public:
    /// default constructor
    NamedSemaphoreWindows()
        : m_hSemaphore(NULL)
    {
    }

    /// destructor
    virtual ~NamedSemaphoreWindows()
    {
        Close();
    }

    //--------------------------------------------------------------------------
    /// Create a system-wide semaphore
    /// \param semaphoreName the name this semaphore will be known by
    ///
    /// \return true if Create succeeded, false if error
    //--------------------------------------------------------------------------
    virtual bool  Create(const char* semaphoreName)
    {
        m_hSemaphore = CreateSemaphore(NULL, 0, MAX_SEM_COUNT, semaphoreName);

        if (m_hSemaphore == NULL)
        {
            return false;
        }

#ifdef DEBUG_PRINT
        printf("** Creating semaphore %s (%d)\n", semaphoreName, m_hSemaphore);
#endif
        return true;
    }

    //--------------------------------------------------------------------------
    /// Open a previously created system-wide semaphore
    /// \param semaphoreName the name this semaphore is be known by (the name it
    /// was given when created)
    ///
    /// \return true if Open succeeded, false if semaphore doesn't exist
    //--------------------------------------------------------------------------
    virtual bool  Open(const char* semaphoreName)
    {
        m_hSemaphore = OpenSemaphore(SEMAPHORE_MODIFY_STATE, false, semaphoreName);

        if (m_hSemaphore == NULL)
        {
            return false;
        }

#ifdef DEBUG_PRINT
        printf("** Opening semaphore %s (%d)\n", semaphoreName, m_hSemaphore);
#endif
        return true;
    }

    //--------------------------------------------------------------------------
    /// Wait for a semaphore to be signaled. This function won't return until it
    /// receives a signal
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    virtual bool  Wait()
    {
#ifdef DEBUG_PRINT
        printf("** Waiting ... (%d)\n", m_hSemaphore);
#endif

        if (WAIT_OBJECT_0 != WaitForSingleObject(m_hSemaphore, INFINITE))
        {
            return false;
        }

#ifdef DEBUG_PRINT
        printf("** Wait done (%d)\n", m_hSemaphore);
#endif
        return true;
    }

    //--------------------------------------------------------------------------
    /// Set the semaphore to the signaled state. Any other threads/processes
    /// waiting for the signal can now proceed.
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    virtual bool  Signal()
    {
#ifdef DEBUG_PRINT
        printf("** Signal (%d)\n", m_hSemaphore);
#endif
        // increase the semaphore by 1; Get the previous count to lCount
        long lCount = 0;

        if (ReleaseSemaphore(m_hSemaphore, 1, &lCount) == FALSE)
        {
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// close the semaphore
    //--------------------------------------------------------------------------
    virtual void  Close()
    {
#ifdef DEBUG_PRINT
        printf("** Close (%d)\n", m_hSemaphore);
#endif

        if (m_hSemaphore != NULL)
        {
            CloseHandle(m_hSemaphore);
            m_hSemaphore = NULL;
        }
    }

private:
    void*    m_hSemaphore;   ///< Windows pointer to semaphore handle
};

#endif   // _WIN32

#if defined (_LINUX)

static const int BUFFER_SIZE = 16;  ///< size of shared memory buffer

// Because of the way that Linux implements condition variables and mutexes
// (with shared memory), 32 and 64-bit versions need to be unique, since the
// shared memory consists of a data structure whose size is dependent on
// the bitsize.
#if defined X64
    static const char* EXT = "_x64";
#else
    static const char* EXT = "_x86";
#endif

/// Linux (POSIX) implementation
/// Uses a POSIX semaphore to emulate a mutex, since semaphores are system-wide, and
/// provide the naming required to identify individual mutexes
/// see http://linux.die.net/man/7/sem_overview

static const int s_Mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

class NamedSemaphorePosix : public NamedSemaphoreImpl
{
public:
    /// default constructor
    NamedSemaphorePosix()
        : m_semaphore(NULL)
        , m_owner(false)
    {
    }

    /// destructor
    virtual ~NamedSemaphorePosix()
    {
#ifdef DEBUG_PRINT
        LogConsole(logMESSAGE, "~NamedSemaphore %p (%s)\n", m_semaphore, m_semaphoreName);
#endif
        Close();
    }

    //--------------------------------------------------------------------------
    /// Create a system-wide semaphore
    /// \param semaphoreName the name this semaphore will be known by
    ///
    /// \return true if Create succeeded, false if error
    //--------------------------------------------------------------------------
    virtual bool  Create(const char* semaphoreName)
    {
#ifdef DEBUG_PRINT
        LogConsole(logMESSAGE, "NamedSemaphore: Create\n");
#endif

        if (m_semaphore == NULL)
        {
            char name[PS_MAX_PATH];
            sprintf_s(name, PS_MAX_PATH, "/%s%s", semaphoreName, EXT);

            // try to create the semaphore first
            m_semaphore = sem_open(name, O_CREAT | O_EXCL, s_Mode, 0);
            m_owner = true;

            if (m_semaphore == SEM_FAILED)
            {
#ifdef DEBUG_PRINT
                LogConsole(logMESSAGE, "Creating semaphore failed\n");
#endif
                return false;
            }

            strcpy(m_semaphoreName, name);

#ifdef DEBUG_PRINT
            LogConsole(logMESSAGE, "Created a new semaphore %p (%s)\n", m_semaphore, m_semaphoreName);
#endif

        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Open a previously created system-wide semaphore
    /// \param semaphoreName the name this semaphore is be known by (the name it
    /// was given when created)
    ///
    /// \return true if Open succeeded, false if semaphore doesn't exist
    //--------------------------------------------------------------------------
    virtual bool  Open(const char* semaphoreName)
    {
        if (m_semaphore == NULL)
        {
            char name[PS_MAX_PATH];
            sprintf_s(name, PS_MAX_PATH, "/%s%s", semaphoreName, EXT);

            // open the semaphore
            m_semaphore = sem_open(name, 0);

            if (m_semaphore != SEM_FAILED)
            {
                strcpy(m_semaphoreName, name);
#ifdef DEBUG_PRINT
                LogConsole(logMESSAGE, "NamedSemaphore: Opened OK %p (%s)\n", m_semaphore, m_semaphoreName);
#endif
                return true;
            }
            else
            {
#ifdef DEBUG_PRINT
                LogConsole(logMESSAGE, "Failed to open semaphore %s. Errno is %d\n", name, osGetLastSystemError());
#endif
                return false;
            }
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Wait for a semaphore to be signaled. This function won't return until it
    /// receives a signal
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    virtual bool  Wait()
    {
#ifdef DEBUG_PRINT
        LogConsole(logMESSAGE, "Semaphore:Wait %p (%s)\n", m_semaphore, m_semaphoreName);
#endif

        if (m_semaphore == NULL)
        {
#ifdef DEBUG_PRINT
            LogConsole(logERROR, "semaphore is NULL\n");
#endif
            return false;
        }

        if (sem_wait(m_semaphore) == 0)
        {
            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------
    /// Set the semaphore to the signaled state. Any other threads/processes
    /// waiting for the signal can now proceed.
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    virtual bool  Signal()
    {
#ifdef DEBUG_PRINT
        LogConsole(logMESSAGE, "semaphore:Signal %p (%s)\n", m_semaphore, m_semaphoreName);
#endif

        if (sem_post(m_semaphore) == 0)
        {
            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------
    /// close the semaphore
    //--------------------------------------------------------------------------
    virtual void  Close()
    {
#ifdef DEBUG_PRINT
        printf("semaphore:Close %p (%s)\n", m_semaphore, m_semaphoreName);
#endif
        sem_close(m_semaphore);
        m_semaphore = NULL;

        if (m_owner)
        {
            sem_unlink(m_semaphoreName);
            m_owner = false;
        }
    }

private:
    sem_t* m_semaphore;                                    ///< pointer to mutex objecti
    char   m_semaphoreName[PS_MAX_PATH];                   ///< name of mutex
    bool   m_owner;                                        ///< did this object create the mutex
};

// Boost-specific implementation. Uses a similar implementation to the
// NamedEvent class but uses an integer condition variable rather than
// a bool
class NamedSemaphoreBoost : public NamedSemaphoreImpl
{
public:
    /// default constructor
    NamedSemaphoreBoost()
        : m_mutex(NULL)
        , m_condition(NULL)
        , m_owner(false)
    {
        m_signalState = new SharedMemory();
    }

    /// destructor
    virtual ~NamedSemaphoreBoost()
    {
        Close();
        delete m_signalState;
    }

    //--------------------------------------------------------------------------
    /// Create a system-wide semaphore
    /// \param semaphoreName the name this semaphore will be known by
    ///
    /// \return true if Create succeeded, false if error
    //--------------------------------------------------------------------------
    virtual bool  Create(const char* semaphoreName)
    {
#ifdef DEBUG_PRINT
        printf("NamedSemaphore: Create\n");
#endif
        char strTemp[PS_MAX_PATH];

        sprintf_s(m_mutexName, PS_MAX_PATH, "%s_mutex%s", semaphoreName, EXT);

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

        sprintf_s(m_conditionName, PS_MAX_PATH, "%s_condition%s", semaphoreName, EXT);

        if (m_condition == NULL)
        {
            m_condition = new named_condition(open_or_create, m_conditionName);
        }

        sprintf_s(strTemp, PS_MAX_PATH, "%s_memory", semaphoreName);
        m_signalState->OpenOrCreate(BUFFER_SIZE, strTemp);
        return true;
    }

    //--------------------------------------------------------------------------
    /// Open a previously created system-wide semaphore
    /// \param semaphoreName the name this semaphore is be known by (the name it
    /// was given when created)
    ///
    /// \return true if Open succeeded, false if semaphore doesn't exist
    //--------------------------------------------------------------------------
    virtual bool  Open(const char* semaphoreName)
    {
#ifdef DEBUG_PRINT
        printf("NamedSemaphore: Open\n");
#endif
        char strTemp[PS_MAX_PATH];

        sprintf_s(strTemp, PS_MAX_PATH, "%s_mutex%s", semaphoreName, EXT);

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

        sprintf_s(strTemp, PS_MAX_PATH, "%s_condition%s", semaphoreName, EXT);

        if (m_condition == NULL)
        {
            try
            {
#ifdef DEBUG_PRINT
                printf("*** try to open_only condition variable %s\n", semaphoreName);
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

        sprintf_s(strTemp, PS_MAX_PATH, "%s_memory", semaphoreName);

        if (SharedMemory::SUCCESS != m_signalState->Open(strTemp))
        {
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Wait for a semaphore to be signaled. This function won't return until it
    /// receives a signal
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    virtual bool  Wait()
    {
#ifdef DEBUG_PRINT
        printf("NamedSemaphore: Wait\n");
#endif
        boost::interprocess::scoped_lock<named_mutex> lock(*m_mutex);
        unsigned int* state = (unsigned int*)m_signalState->Get();

        // count is OK, so no need to wait
        if ((*state) > 0)
        {
            (*state)--;
            return true;
        }

        // if count is 0, nothing signaled so wait
        else if ((*state) == 0)
        {
#ifdef DEBUG_PRINT
            printf("NamedSemaphore: waiting\n");
#endif
            m_condition->wait(lock);
            state = (unsigned int*)m_signalState->Get();
            (*state)--;
        }

#ifdef DEBUG_PRINT
        printf("NamedSemaphore: Wait done\n");
#endif
        return true;
    }

    //--------------------------------------------------------------------------
    /// Set the semaphore to the signaled state. Any other threads/processes
    /// waiting for the signal can now proceed.
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    virtual bool  Signal()
    {
#ifdef DEBUG_PRINT
        printf("NamedSemaphore: Signal\n");
#endif
        boost::interprocess::scoped_lock<named_mutex> lock(*m_mutex);
        unsigned int* state = (unsigned int*)m_signalState->Get();
        (*state)++;
        m_condition->notify_one();
        return true;
    }

    //--------------------------------------------------------------------------
    /// close the semaphore
    //--------------------------------------------------------------------------
    virtual void  Close()
    {
        if (m_owner)
        {
            boost::interprocess::named_mutex::remove(m_mutexName);
            boost::interprocess::named_condition::remove(m_conditionName);
        }

        m_signalState->Close();
        delete m_condition;
        delete m_mutex;

        m_condition = NULL;
        m_mutex = NULL;
    }
private:
    named_mutex*     m_mutex;        ///< The mutex object used to protect the condition variable
    named_condition* m_condition;    ///< The condition variable used by the OS to detect signals
    SharedMemory*    m_signalState;  ///< The state of the signal. Used by this implementation to set or reset the signal state.
    bool             m_owner;        ///< Does this object own the semaphore

    char             m_mutexName[PS_MAX_PATH];
    char             m_conditionName[PS_MAX_PATH];
};

#endif // _LINUX

/// Main Implementation methods.
/// default constructor
/// Pick an implementation based on platform
NamedSemaphore::NamedSemaphore()
{
#if defined (_WIN32)
    m_pImpl = new NamedSemaphoreWindows();
#else
    m_pImpl = new NamedSemaphoreBoost();
#endif
}

/// destructor
NamedSemaphore::~NamedSemaphore()
{
    delete m_pImpl;
}

//--------------------------------------------------------------------------
/// Create a system-wide semaphore
/// \param semaphoreName the name this semaphore will be known by
///
/// \return true if Create succeeded, false if error
//--------------------------------------------------------------------------
bool  NamedSemaphore::Create(const char* semaphoreName)
{
    return m_pImpl->Create(semaphoreName);
}

//--------------------------------------------------------------------------
/// Open a previously created system-wide semaphore
/// \param semaphoreName the name this semaphore is be known by (the name it
/// was given when created)
///
/// \return true if Open succeeded, false if the semaphore doesn't exist
//--------------------------------------------------------------------------
bool  NamedSemaphore::Open(const char* semaphoreName)
{
    return m_pImpl->Open(semaphoreName);
}

//--------------------------------------------------------------------------
/// Wait for a semaphore to be signaled. This function won't return until it
/// receives a signal
///
/// \return true if successful, false if error
//--------------------------------------------------------------------------
bool  NamedSemaphore::Wait()
{
    return m_pImpl->Wait();
}

//--------------------------------------------------------------------------
/// Signal the semaphore. Adds 1 to its count. Any other threads/processes
/// waiting for the signal can now proceed if the count is greater than 0.
/// The task of waking waiting threads and allowing them to proceed is
/// the responsibility of the OS
///
/// \return true if successful, false if error
//--------------------------------------------------------------------------
bool  NamedSemaphore::Signal()
{
    return m_pImpl->Signal();
}

//--------------------------------------------------------------------------
/// close the semaphore
//--------------------------------------------------------------------------
void  NamedSemaphore::Close()
{
    m_pImpl->Close();
}
