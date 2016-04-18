//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSpyToAPIConnector.h
///
//==================================================================================

//------------------------------ suSpyToAPIConnector.h ------------------------------

#ifndef __SUSPYTOAPICONNECTOR_H
#define __SUSPYTOAPICONNECTOR_H

// Forward decelerations:
class osSocket;
class suAPICallsHandlingThread;
class osPortAddress;

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>


// ----------------------------------------------------------------------------------
// Class Name:           suSpyToAPIConnector
// General Description:
//   Manages the socket connection between the spies and the GRAPIFunctions library.
//
// Author:               Yaki Tebeka
// Creation Date:        15/2/2004
// ----------------------------------------------------------------------------------
class suSpyToAPIConnector
{
public:
    static suSpyToAPIConnector& instance();
    bool initialize(const osPortAddress& portAddress);
    bool initialize(const gtString& sharedMemoryObjectName);
    bool terminate();

    bool setEventForwardingSocket(const osPortAddress& portAddress);
    bool setEventForwardingPipe(const gtString& eventsPipeName);

    suAPICallsHandlingThread* apiCallsHandlingThread() { return _pAPICallsHandlingThread; };

    // Notice: Use this access with care !!!!
    // (In most cases - the api socket is accessed by the _APICallsHandlingThread)
    osSocket* apiSocket() { return _pSocketClient; };

    osSocket* eventForwardingSocket() { return  _pEventForwardingSocket; };

private:
    // Only my instance() method should create my single instance:
    suSpyToAPIConnector();
    virtual ~suSpyToAPIConnector();

    void handleAPIInitializationCalls();
    bool createAndRunTheAPIHandlingThread();

private:
    // The single instance of this class:
    static suSpyToAPIConnector* _pMySingleInstance;

    // The socket client:
    osSocket* _pSocketClient;

    // The events forwarding socket:
    osSocket* _pEventForwardingSocket;

    // The thread that will handle incoming API calls:
    suAPICallsHandlingThread* _pAPICallsHandlingThread;
};


#endif //__SUSPYTOAPICONNECTOR_H

