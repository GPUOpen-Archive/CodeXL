//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCondition.h
///
//=====================================================================

//------------------------------ osCondition.h ------------------------------

#ifndef __OSCONDITION
#define __OSCONDITION

// POSIX:
#include <pthread.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           osCondition
// General Description:
//    A condition on which threads can wait.
//
// Author:      AMD Developer Tools Team
// Creation Date:        15/1/2007
// ----------------------------------------------------------------------------------
class OS_API osCondition
{
public:
    osCondition();
    virtual ~osCondition();

    bool lockCondition();
    bool unlockCondition();
    bool isConditionLocked() const { return _isConditionLocked; };

    bool waitForCondition();

    bool signalSingleThread();
    bool signalAllThreads();

private:
    // Is the condition locked:
    bool _isConditionLocked;

    // A mutex that controls the access to _isConditionLocked:
    pthread_mutex_t _conditionMutex;

    // The OS condition object:
    pthread_cond_t _conditionObj;
};


#endif  // __OSCONDITION
