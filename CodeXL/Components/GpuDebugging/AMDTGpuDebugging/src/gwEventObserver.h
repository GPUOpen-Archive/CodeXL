//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwEventObserver.h
///
//==================================================================================

//------------------------------ gwEventObserver.h ------------------------------

#ifndef __GWEVENTOBSERVER_H
#define __GWEVENTOBSERVER_H


// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdFrameworkConnectionManager.h>
#include <AMDTApplicationComponents/Include/acSourceCodeView.h>

// Forward declaration:
class apAddWatchEvent;
class afGlobalVariableChangedEvent;
class afApplicationCommands;
class gdApplicationCommands;

// ----------------------------------------------------------------------------------
// Class Name:           gwEventObserver : public apIEventsObserver
// General Description: An (apEvent) event observer, responsible of translating apEvent-s
//                      to events that can be consumed by Visual studio.
// Author:               Sigal Algranaty
// Creation Date:        21/7/2011
// ----------------------------------------------------------------------------------
class gwEventObserver : public apIEventsObserver, acISourceViewToolTip
{
public:
    gwEventObserver();
    virtual ~gwEventObserver();

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"GWEventObserver"; };

    // Overrides acISourceViewToolTip:
    virtual QString Tooltip(QString& highlightedString);

protected:

    bool openBreakpointSourceCode(osThreadId threadId);
    void onAddWatch(const apAddWatchEvent& addWatchEvent);

    // Layout management:
    void updateLayout();

protected:

    void updateApplicationTitle();
    void updateWIToolbar(bool rebuildThreadValues, bool rebuildWIValues);

protected:

    gtString _lastLayoutSaved;

    gdFrameworkConnectionManager m_frameworkConnectionManager;

    // Get the application commands instance:
    afApplicationCommands* _pApplicationCommands;
    gdApplicationCommands* _pGDApplicationCommands;
};

#endif //__GWEVENTOBSERVER_H

