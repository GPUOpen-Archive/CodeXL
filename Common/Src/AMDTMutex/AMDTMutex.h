//=============================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \brief  Mutex class
//=============================================================

#ifndef _AMDT_MUTEX_H_
#define _AMDT_MUTEX_H_

#include <string>
#define DEBUG_DEADLOCK 0

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#elif defined(_LINUX) || defined(LINUX)
    #include <sys/types.h>
    #include <pthread.h>
#endif

/// A wrapper class to simplify using the OS's CRITICAL_SECTION
class AMDTMutex
{
private:
    std::string m_strName; ///< name of the mutex
#ifdef _WIN32
    CRITICAL_SECTION m_cs; ///< The underlying critical section object
#elif defined(_LINUX) || defined(LINUX)
    pthread_mutex_t m_mtx;           ///< Mutex
    pthread_mutexattr_t m_mutexattr;   ///< Mutex attribute variable
#endif

public:
    /// Constructor: Initializes the critical section so that it can be used
    AMDTMutex();

    /// Constructor
    /// \param[in] strName name string
    AMDTMutex(const char* strName);

    /// Destructor: deletes the critical section
    ~AMDTMutex();

    /// Locks the critical section
    void Lock();

    /// Unlocks the critical section
    void Unlock();
};

/// Helper for Mutex class which Enters a critical section in the constructor and Leaves that section in the destructor
class AMDTScopeLock
{
private:
    AMDTMutex* m_pMtx; ///< Underlying mutex class

public:

    /// enters a critical section
    /// \param[in] m pointer to the mutex to use
    AMDTScopeLock(AMDTMutex* m)
    {
        m_pMtx = m;
        m_pMtx->Lock();
    }

    /// Enters a critical section
    /// \param[in] m reference to the mutex to use
    AMDTScopeLock(AMDTMutex& m)
    {
        m_pMtx = &m;
        m_pMtx->Lock();
    }

    /// Leaves the critical section
    ~AMDTScopeLock()
    {
        m_pMtx->Unlock();
    }
};

#endif // _AMDT_MUTEX_H_
