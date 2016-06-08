//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file appEventObserver.h
///
//==================================================================================

#ifndef __APPEVENTOBSERVER_H
#define __APPEVENTOBSERVER_H


// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>

// Forward declaration:
class afGlobalVariableChangedEvent;
class gdApplicationCommands;
class apMemoryAllocationFailureEvent;

// ----------------------------------------------------------------------------------
// Class Name:          appEventObserver : public apIEventsObserver
// General Description: An (apEvent) event observer, responsible of handling events
//                      that are relevant for the application framework execution
// Author:               Sigal Algranaty
// Creation Date:        21/7/2011
// ----------------------------------------------------------------------------------
class appEventObserver : public apIEventsObserver
{
public:
    appEventObserver();
    virtual ~appEventObserver();

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"APPEventObserver"; };

protected:

    /// Is called when an MDI created event is called:
    void OnMDIViewEvent(const apMDIViewCreateEvent& mdiEvent);

    void OnGlobalVariableChanged(const afGlobalVariableChangedEvent& variableChangedEvent);

    /// Handle memory allocation event in the client application
    /// \Param memoryAllocationFailedEvent the event with the call stack information
    void OnMemoryAllocationFailedEvent(apMemoryAllocationFailureEvent& memoryAllocationFailedEvent);

    // TODO: document this!
    void OnExceptionEvent(const apExceptionEvent& exceptionEvent);
};

#endif //__APPEVENTOBSERVER_H

