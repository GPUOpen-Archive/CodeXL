//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file rdEventsHandlingThread.h
///
//==================================================================================

//------------------------------ rdEventsHandlingThread.h ------------------------------

#ifndef __RDEVENTSHANDLINGTHREAD_H
#define __RDEVENTSHANDLINGTHREAD_H

// Infra:
#include <AMDTOSWrappers/Include/osThread.h>


// ----------------------------------------------------------------------------------
// Class Name:          rdEventsHandlingThread : public osThread
// General Description: A thread used to simulate the main application thread in the
//                      remote debugging server.
// Author:              Uri Shomroni
// Creation Date:       17/8/2009
// Implementation Notes: As the main thread of this server is the thread listening to
//                      process debugger commands, it does not have idle time. This
//                      thread gets the events notifications and handles the debug
//                      events.
//                      We do not do this directly in the debugger thread (ie make the
//                      pending event notification function handle debug events
//                      directly, as the process terminated event destroys that thread
//                      (causing it not to report the termination to other parts of the
//                      application).
// ----------------------------------------------------------------------------------
class rdEventsHandlingThread : public osThread
{
public:
    rdEventsHandlingThread(const gtString& threadName);
    ~rdEventsHandlingThread();

    // Overrides osThread:
    virtual int entryPoint();
    virtual void beforeTermination();
};

#endif //__RDEVENTSHANDLINGTHREAD_H

