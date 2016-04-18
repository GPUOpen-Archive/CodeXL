//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suAPICallsHandlingThread.h
///
//==================================================================================

//------------------------------ suAPICallsHandlingThread.h ------------------------------

#ifndef __SUAPICALLSHANDLINGTHREAD_H
#define __SUAPICALLSHANDLINGTHREAD_H

// Forward declarations:
class osTCPSocketClient;
class osThread;

// Infra:
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osThread.h>


// ----------------------------------------------------------------------------------
// Class Name:           suAPICallsHandlingThread : public osThread
// General Description:
//   A thread that handles incoming API calls.
//   We need a separate thread to perform this task, since we would like to have the
//   API active even when the main debugged process thread is not running (Example: The
//   API should remain active after the debugged process main thread hit a breakpoint).
//
// Author:               Yaki Tebeka
// Creation Date:        15/2/2004
// ----------------------------------------------------------------------------------
class suAPICallsHandlingThread : public osThread
{
public:
    suAPICallsHandlingThread(osSocket& socketClient);
    virtual ~suAPICallsHandlingThread();

    bool isListeningToAPICalls() const;

    // Events:
    void afterSpiesUtilitiesAPIInitializationEnded();
    bool mainThreadStarsInitializingAPIConnection();
    void mainThreadEndedInitializingAPIConnection();

protected:
    // Overrides osThread:
    virtual int entryPoint();

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    suAPICallsHandlingThread() = delete;
    suAPICallsHandlingThread(const suAPICallsHandlingThread&) = delete;
    suAPICallsHandlingThread& operator=(const suAPICallsHandlingThread&) = delete;

private:
    // The socket that connects this spy dll to its API:
    osSocket& _socketClient;

    // Contains true iff the API thread is listening to API calls:
    bool _isListeningToAPICalls;

    // Will get false when the spies utilities API initialization is done:
    bool _shouldWaitForSpiesUtilitiesAPIInitialization;

    // A critical section that synchronizes the API initialization:
    // (Either the API thread or the main thread should initialize a given API connection
    //  initialization. This critical section object makes sure that only one of the threads does
    //  the work).
    osCriticalSection _APIConnectionInitializationCS;
};


#endif //__SUAPICALLSHANDLINGTHREAD_H

