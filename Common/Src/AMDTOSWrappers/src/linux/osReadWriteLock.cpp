//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osReadWriteLock.cpp
///
//=====================================================================
#include <osReadWriteLock.h>

osReadWriteLock::osReadWriteLock()
{
    pthread_rwlock_init(&m_rwlock, NULL);
}

osReadWriteLock::~osReadWriteLock()
{
    pthread_rwlock_destroy(&m_rwlock);
}

void osReadWriteLock::lockRead()
{
    pthread_rwlock_rdlock(&m_rwlock);
}

bool osReadWriteLock::tryLockRead()
{
    return 0 == pthread_rwlock_tryrdlock(&m_rwlock);
}

void osReadWriteLock::unlockRead()
{
    pthread_rwlock_unlock(&m_rwlock);
}

void osReadWriteLock::lockWrite()
{
    pthread_rwlock_wrlock(&m_rwlock);
}

bool osReadWriteLock::tryLockWrite()
{
    return 0 == pthread_rwlock_trywrlock(&m_rwlock);
}

void osReadWriteLock::unlockWrite()
{
    pthread_rwlock_unlock(&m_rwlock);
}
