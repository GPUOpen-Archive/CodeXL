//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaAPIToSpyConnector.h
///
//==================================================================================

//------------------------------ gaAPIToSpyConnector.h ------------------------------

#ifndef __GAAPITOSPYCONNECTOR
#define __GAAPITOSPYCONNECTOR

// Forward decelerations:
class osPortAddress;
class osSocket;
class osTCPSocketServer;
class apApiConnectionEndedEvent;
class apApiConnectionEstablishedEvent;
class apExceptionEvent;
class gaIncomingSpyEventsListenerThread;
class gaSpyConnectionWaiterThread;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>


// ----------------------------------------------------------------------------------
// Class Name:           gaAPIToSpyConnector : public apIEventsObserver
// General Description:
//   Manages the socket connection between the this API module and the SpiesUtilities
//   module running within the debugged process.
//
// Author:               Yaki Tebeka
// Creation Date:        15/2/2004
// ----------------------------------------------------------------------------------
class gaAPIToSpyConnector : public apIEventsObserver
{
public:
    static gaAPIToSpyConnector& instance();

    bool initialize(const osPortAddress& spyAPIPortAddress, const osPortAddress& incomingEventsPortAddress);
    bool initialize(const gtString& spyAPIPipeName, const gtString& incomingEventsPipeName);
    bool terminate();

    bool isAPIConnectionActive(apAPIConnectionType apiType) const;
    osSocket* spiesAPIConnectingSocket() { return _pSpiesAPIConnectingSocket; };

    void onPendingDebugEvent(const apEvent& pendingEvent);
    void beforeForcedDebuggedProcessTermination();

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"APIToSpyConnector"; };

private:
    // Events handling:
    void onProcessTerminatedEvent();
    void onExceptionEvent(const apExceptionEvent& eve);
    void onAPIConnectionEstablishedEvent(const apApiConnectionEstablishedEvent& eve);
    void onAPIConnectionEndedEvent(const apApiConnectionEndedEvent& eve);

    void clearIsAPIConnectionActiveArray();

    // Only my instance() method should create my single instance:
    gaAPIToSpyConnector();

    // The singletons deleter should be able to delete me:
    friend class gaSingletonsDelete;
    virtual ~gaAPIToSpyConnector();

private:
    // The single instance of this class:
    static gaAPIToSpyConnector* _pMySingleInstance;

    // The socket connection to the spies API:
    osSocket* _pSpiesAPIConnectingSocket;

    // A thread that waits for the spy connection:
    gaSpyConnectionWaiterThread* _pSpiesAPIConnectionWaiterThread;

    // A thread that listens to incoming spy:
    gaIncomingSpyEventsListenerThread* _pIncomingSpyEventsListenerThread;

    // Logs which API connections are active:
    bool _isAPIConnectionActive[AP_AMOUNT_OF_API_CONNECTION_TYPES];

    // Contains true iff we are during a debugged process second chance exception handling:
    bool _isDuringSecondChanceExceptionHandling;

    // TCP / IP connection members:
    // ---------------------------
    // A socket that listens to a given port (used only in TCP / IP connection):
    osTCPSocketServer* _pPortListener;
};


#endif  // __GAAPITOSPYCONNECTOR
