//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdDebuggedProcessThreadData.h
///
//==================================================================================

//------------------------------ pdDebuggedProcessThreadData.h ------------------------------

#ifndef __PDDEBUGGEDPROCESSTHREADDATA_H
#define __PDDEBUGGEDPROCESSTHREADDATA_H

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>


// ----------------------------------------------------------------------------------
// Struct Name:          pdDebuggedProcessThreadData
// General Description:
//   Holds data related to a debugged process thread.
//
// Author:               Yaki Tebeka
// Creation Date:        20/4/2006
// ----------------------------------------------------------------------------------
struct pdDebuggedProcessThreadData
{
    // The thread id (system wide):
    osThreadId _threadId;

    // The thread handle (not always shared among processes):
    osThreadHandle _threadHandle;

    // The thread start address:
    osInstructionPointer _threadStartAddress;

    // Contains true iff this is a thread that was created by a driver:
    // (We should not suspend / terminate / etc driver threads)
    bool _isDriverThread;

public:
    pdDebuggedProcessThreadData() : _threadId(OS_NO_THREAD_ID), _threadHandle(0), _threadStartAddress(NULL), _isDriverThread(false) {};
    pdDebuggedProcessThreadData(osThreadId threadId, osThreadHandle threadHandle, osInstructionPointer threadStartAddress, bool isDriverThread = false) : _threadId(threadId), _threadHandle(threadHandle), _threadStartAddress(threadStartAddress), _isDriverThread(isDriverThread) {};
};


#endif //__PDDEBUGGEDPROCESSTHREADDATA_H
