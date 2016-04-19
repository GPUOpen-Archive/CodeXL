//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Mutex class
//==============================================================================

#ifndef GPS_MYMUTEX_H
#define GPS_MYMUTEX_H

#if defined _WIN32
#include <windows.h>
#include <process.h>

/// A wrapper class to simplify using the OS's CRITICAL_SECTION
class mutex
{
private:

    /// The underlying critical section object
    CRITICAL_SECTION m_cs;

public:

    /// Constructor
    ///
    /// Intitializes the critical section so that it can be used
    mutex()
    {
        InitializeCriticalSection(&m_cs);
    }

    /// Destructor
    ///
    /// Deletes the critical section
    ~mutex()
    {
        DeleteCriticalSection(&m_cs);
    }

    /// Locks the critical section
    void Lock()
    {
        EnterCriticalSection(&m_cs);
    }

    /// Unlocks the critical section
    void Unlock()
    {
        LeaveCriticalSection(&m_cs);
    }
};

#elif defined _LINUX
#include <pthread.h>

class mutex
{
private:

    /// The underlying mutex object
    pthread_mutex_t m_mutex;

public:

    /// Constructor
    ///
    /// Initializes the mutex so that it can be used
    mutex()
    {
        // Create a mutex attributes object that creates a recursive mutex
        // (A recursive mutex enables locking the mutex several times by the same thread,
        // maintaining a lock count). This is equivalent to the way Windows implements
        // its critical sections.
        pthread_mutexattr_t  mutexAttributes;
        pthread_mutexattr_init(&mutexAttributes);
        pthread_mutexattr_settype(&mutexAttributes, PTHREAD_MUTEX_RECURSIVE);

        pthread_mutex_init(&m_mutex, &mutexAttributes);
    }

    /// Destructor
    ///
    /// Deletes the mutex
    ~mutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    /// Locks the mutex
    void Lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    /// Unlocks the mutex
    void Unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }
};
#else
#error Unknown OS
#endif

/// Helper for Mutex class which Enters a critical section in the constructor and Leaves that section in the destructor
class ScopeLock
{
private:

    /// Underlying mutex class
    mutex* m_pMtx;

public:

    /// enters a critical section
    /// \param m pointer to the mutex to use
    ScopeLock(mutex* m)
    {
        m_pMtx = m;
        m_pMtx->Lock();
    }

    /// enters a critical section
    /// \param m reference to the mutex to use
    ScopeLock(mutex& m)
    {
        m_pMtx = &m;
        m_pMtx->Lock();
    }

    /// Leaves the critical section
    ~ScopeLock()
    {
        m_pMtx->Unlock();
    }
};

#endif // GPS_MYMUTEX_H
