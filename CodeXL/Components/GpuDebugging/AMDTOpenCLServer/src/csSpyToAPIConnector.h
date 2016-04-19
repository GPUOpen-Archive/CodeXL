//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csSpyToAPIConnector.h
///
//==================================================================================

//------------------------------ csSpyToAPIConnector.h ------------------------------

#ifndef __CSSPYTOAPICONNECTOR
#define __CSSPYTOAPICONNECTOR

// Forward decelerations:
class osSocket;
class csAPICallsHandlingThread;
class osPortAddress;

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// ----------------------------------------------------------------------------------
// Class Name:          csSpyToAPIConnector
// General Description:
//   Manages the socket connection between this OpenCL Spy dll and its API (that resides
//   inside the g-Debugger application).
// Author:               Sigal Algranaty
// Creation Date:        15/11/2009
// ----------------------------------------------------------------------------------
class csSpyToAPIConnector
{
public:
    static csSpyToAPIConnector& instance();
    bool initialize(const osPortAddress& portAddress);
    bool initialize(const gtString& sharedMemoryObjectName);
    bool terminate();

    csAPICallsHandlingThread* apiCallsHandlingThread() { return _pAPICallsHandlingThread; };

    // Notice: Use this access with care !!!!
    // (In most cases - the api socket is accessed by the _APICallsHandlingThread)
    osSocket* apiSocket() { return  _pSocketClient; };

private:
    // Only my instance() method should create my single instance:
    csSpyToAPIConnector();
    virtual ~csSpyToAPIConnector();

    void handleAPIInitializationCalls();
    bool createAndRunTheAPIHandlingThread();

private:
    // The single instance of this class:
    static csSpyToAPIConnector* _pMySingleInstance;

    // The socket client:
    osSocket* _pSocketClient;

    // The thread that will handle incoming API calls:
    csAPICallsHandlingThread* _pAPICallsHandlingThread;
};


#endif  // __CSSPYTOAPICONNECTOR
