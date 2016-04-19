//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdGDBProcessWaiterThread.h
///
//==================================================================================

//------------------------------ pdGDBProcessWaiterThread.h ------------------------------

#ifndef __PDGDBPROCESSWAITERTHREAD_H
#define __PDGDBPROCESSWAITERTHREAD_H

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osThread.h>

// ----------------------------------------------------------------------------------
// Class Name:          pdGDBProcessWaiterThread : public osThread
// General Description: A thread that performs a waitpid operation on the gdb process, to avoid
//                      having said process become a zombie
// Author:              Uri Shomroni
// Creation Date:       3/4/2012
// ----------------------------------------------------------------------------------
class pdGDBProcessWaiterThread : public osThread
{
public:
    pdGDBProcessWaiterThread(osProcessId gdbProcessId);
    virtual ~pdGDBProcessWaiterThread();

    // Overrides osThread:
    virtual int entryPoint();
    virtual void beforeTermination();

private:
    osProcessId _gdbProcessId;
    bool _continueWaiting;
};

#endif //__PDGDBPROCESSWAITERTHREAD_H

