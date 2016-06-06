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
    InitializeSRWLock(&m_rwlock);
}

osReadWriteLock::~osReadWriteLock()
{
}

void osReadWriteLock::lockRead()
{
    AcquireSRWLockShared(&m_rwlock);
}

bool osReadWriteLock::tryLockRead()
{
    return FALSE != TryAcquireSRWLockShared(&m_rwlock);
}

void osReadWriteLock::unlockRead()
{
    ReleaseSRWLockShared(&m_rwlock);
}

void osReadWriteLock::lockWrite()
{
    AcquireSRWLockExclusive(&m_rwlock);
}

bool osReadWriteLock::tryLockWrite()
{
    return FALSE != TryAcquireSRWLockExclusive(&m_rwlock);
}

void osReadWriteLock::unlockWrite()
{
    ReleaseSRWLockExclusive(&m_rwlock);
}
