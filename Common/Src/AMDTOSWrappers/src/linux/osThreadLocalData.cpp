//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osThreadLocalData.cpp
///
//=====================================================================

//------------------------------ osThreadLocalData.cpp ------------------------------

// POSIX:
#include <pthread.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osThreadLocalData.h>


// ---------------------------------------------------------------------------
// Name:        osAllocateThreadsLocalData
// Description:
//   Allocated threads local data handle. This handle can be accessed from all
//   threads, but each thread will get its own data copy.
//
// Arguments: hThreadLocalData - A handle to the allocated threads local data.
//
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        23/11/2006
// ---------------------------------------------------------------------------
bool osAllocateThreadsLocalData(osTheadLocalDataHandle& hThreadLocalData)
{
    bool retVal = false;

    // Allocate per threads data key:
    int rc = ::pthread_key_create(&hThreadLocalData, NULL);
    GT_IF_WITH_ASSERT(rc == 0)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFreeThreadsLocalData
// Description: Frees thread's local data, allocated by osAllocateThreadsLocalData.
// Arguments: pThreadLocalData - The threads local data handle.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        23/11/2006
// ---------------------------------------------------------------------------
bool osFreeThreadsLocalData(osTheadLocalDataHandle& hThreadLocalData)
{
    bool retVal = false;

    // Free the per threads data key:
    int rc = ::pthread_key_delete(hThreadLocalData);
    GT_IF_WITH_ASSERT(rc == 0)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSetCurrentThreadLocalData
// Description: Sets the current thread's local data associated with hThreadLocalData.
// Arguments: hThreadLocalData - Handle to threads local data, allocated by osAllocateThreadLocalData.
//            pData - The data to be set. This data will be the calling thread's data
//                    associated with hThreadLocalData.
//
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        23/11/2006
// ---------------------------------------------------------------------------
bool osSetCurrentThreadLocalData(const osTheadLocalDataHandle& hThreadLocalData, void* pData)
{
    bool retVal = false;

    // Set the calling thread's local data, associated with hThreadLocalData:
    int rc = ::pthread_setspecific(hThreadLocalData, pData);
    GT_IF_WITH_ASSERT(rc == 0)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetCurrentThreadLocalData
// Description: Retrieves the current thread's local data associated with hThreadLocalData.
// Arguments: hThreadLocalData - Handle to a threads local data, allocated by osAllocateThreadLocalData.
// Return Val: void* - Will get the calling thread's local data, associated with
//                    hThreadLocalData, or NULL if an error occurred.
//
// Author:      AMD Developer Tools Team
// Date:        23/11/2006
// ---------------------------------------------------------------------------
void* osGetCurrentThreadLocalData(const osTheadLocalDataHandle& hThreadLocalData)
{
    // Get the calling thread's local data, associated with hThreadLocalData:
    void* retVal = ::pthread_getspecific(hThreadLocalData);
    return retVal;
}

