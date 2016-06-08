//=============================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \brief  Mutex class
//=============================================================

#include "AMDTMutex.h"

#ifdef _WIN32

AMDTMutex::AMDTMutex()
{
    m_strName = "Unnamed mutex";
    InitializeCriticalSection(&m_cs);
}

/// Constructor
/// \param[in] strName name string
AMDTMutex::AMDTMutex(const char* strName)
{
    m_strName = std::string(strName);
    InitializeCriticalSection(&m_cs);
}

/// Destructor: deletes the critical section
AMDTMutex::~AMDTMutex()
{
    DeleteCriticalSection(&m_cs);
}

/// Locks the critical section
void AMDTMutex::Lock()
{
    EnterCriticalSection(&m_cs);
}

/// Unlocks the critical section
void AMDTMutex::Unlock()
{
    LeaveCriticalSection(&m_cs);
}

#elif defined(_LINUX) || defined(LINUX)

/// Constructor: Initializes the critical section so that it can be used
AMDTMutex::AMDTMutex()
{
    m_strName = "Unnamed mutex";

    pthread_mutexattr_init(&m_mutexattr);
    // Set the mutex as a recursive mutex
    pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);

    // create the mutex with the attributes set
    pthread_mutex_init(&m_mtx, &m_mutexattr);

    //After initializing the mutex, the thread attribute can be destroyed
    pthread_mutexattr_destroy(&m_mutexattr);
}

/// Constructor
/// \param[in] strName name string
AMDTMutex::AMDTMutex(const char* strName)
{
    m_strName = std::string(strName);

    pthread_mutexattr_init(&m_mutexattr);
    // Set the mutex as a recursive mutex
    pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);

    // create the mutex with the attributes set
    pthread_mutex_init(&m_mtx, &m_mutexattr);

    //After initializing the mutex, the thread attribute can be destroyed
    pthread_mutexattr_destroy(&m_mutexattr);
}

/// Destructor: deletes the critical section
AMDTMutex::~AMDTMutex()
{
    // Destroy / close the mutex
    pthread_mutex_destroy(&m_mtx);
}

/// Locks the critical section
void AMDTMutex::Lock()
{
    // Acquire the mutex to access the shared resource
    pthread_mutex_lock(&m_mtx);
}

/// Unlocks the critical section
void AMDTMutex::Unlock()
{
    // Release the mutex  and release the access to shared resource
    pthread_mutex_unlock(&m_mtx);
}

#endif
