//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscAppEventObserver.h
///
//==================================================================================

//------------------------------ vscAppEventObserver.h ------------------------------

#ifndef __VSPAPPEVENTOBSERVER_H
#define __VSPAPPEVENTOBSERVER_H


// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// Forward declaration:
class afGlobalVariableChangedEvent;
class gdApplicationCommands;


// ----------------------------------------------------------------------------------
// Class Name:          vscAppEventObserver : public apIEventsObserver
// General Description: An (apEvent) event observer, responsible of handling events
//                      that are relevant for the application framework execution
// Author:               Sigal Algranaty
// Creation Date:        21/7/2011
// ----------------------------------------------------------------------------------
class vscAppEventObserver : public apIEventsObserver
{
public:
    vscAppEventObserver();
    virtual ~vscAppEventObserver();

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"vscAppEventObserver"; };

protected:

};

#endif //__VSPAPPEVENTOBSERVER_H

