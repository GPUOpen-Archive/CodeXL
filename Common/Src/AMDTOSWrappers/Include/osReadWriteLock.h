//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osReadWriteLock.h
///
//=====================================================================

//------------------------------ osReadWriteLock.h ------------------------------

#ifndef __OSREADWRITELOCK
#define __OSREADWRITELOCK

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#if AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
    #include <pthread.h>
#endif


class OS_API osReadWriteLock
{
public:
    osReadWriteLock();
    ~osReadWriteLock();

    void lockRead();
    bool tryLockRead();
    void unlockRead();

    void lockWrite();
    bool tryLockWrite();
    void unlockWrite();

private:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    SRWLOCK m_rwlock;
#else
    pthread_rwlock_t m_rwlock;
#endif
};


#endif  // __OSREADWRITELOCK
