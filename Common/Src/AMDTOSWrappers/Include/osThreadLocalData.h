//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osThreadLocalData.h
///
//=====================================================================

//------------------------------ osThreadLocalData.h ------------------------------

#ifndef __OSTHREAD_LOCAL_DATA
#define __OSTHREAD_LOCAL_DATA

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

OS_API bool osAllocateThreadsLocalData(osTheadLocalDataHandle& hThreadLocalData);
OS_API bool osFreeThreadsLocalData(osTheadLocalDataHandle& hThreadLocalData);

OS_API bool osSetCurrentThreadLocalData(const osTheadLocalDataHandle& hThreadLocalData, void* pData);
OS_API void* osGetCurrentThreadLocalData(const osTheadLocalDataHandle& hThreadLocalData);

#endif  // __OSTHREAD_LOCAL_DATA
