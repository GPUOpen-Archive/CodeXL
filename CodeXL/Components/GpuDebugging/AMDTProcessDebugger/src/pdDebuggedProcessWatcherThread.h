//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdDebuggedProcessWatcherThread.h
///
//==================================================================================

//------------------------------ pdDebuggedProcessWatcherThread.h ------------------------------

#ifndef __PDDEBUGGEDPROCESSWATCHERTHREAD_H
#define __PDDEBUGGEDPROCESSWATCHERTHREAD_H

// Forward declarations:
class pdGDBDriver;

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osThread.h>

// ----------------------------------------------------------------------------------
// Class Name:           pdDebuggedProcessWatcherThread : public osThread
// General Description: A thread that watches the debugged process and waits for it
//                      to exit (Some debugged applications do not report exiting to
//                      gdb properly)
// Author:               Uri Shomroni
// Creation Date:        2/6/2009
// ----------------------------------------------------------------------------------
class pdDebuggedProcessWatcherThread : public osThread
{
public:
    pdDebuggedProcessWatcherThread(osProcessId debuggedProcessPID, pdGDBDriver& rGDBDriver);
    ~pdDebuggedProcessWatcherThread();

    // Overrides osThread
    virtual int entryPoint();
    virtual void beforeTermination();

private:
    // Disallow use of my default constructor:
    pdDebuggedProcessWatcherThread();

private:
    osProcessId _debuggedProcessPID;
    pdGDBDriver& _gdbDriver;
};

#endif //__PDDEBUGGEDPROCESSWATCHERTHREAD_H

