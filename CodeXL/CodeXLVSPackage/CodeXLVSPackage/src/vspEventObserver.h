//------------------------------ vspEventObserver.h ------------------------------

#ifndef __VSPEVENTOBSERVER_H
#define __VSPEVENTOBSERVER_H

// Forward declarations:
class apBreakpointHitEvent;
class acProgressDlg;
class vspCDebugEngine;

// Core interfaces:
#include <Include/Public/CoreInterfaces/IVscEventObserverOwner.h>

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:           vspEventObserver : public apIEventsObserver
// General Description: An (apEvent) event observer, responsible of translating apEvent-s
//                      to events that can be consumed by Visual studio.
// Author:               Uri Shomroni
// Creation Date:        16/9/2010
// ----------------------------------------------------------------------------------
class vspEventObserver : public apIEventsObserver
{
public:
    vspEventObserver(vspCDebugEngine& debugEngine, IDebugEventCallback2* piDebugEventCallback);
    virtual ~vspEventObserver();

    // Sets the owner object in terms of interaction with VS-specific code.
    static void setOwner(IVscEventObserverOwner* pOwner);

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"VSPackageEventObserver"; };

    void setDebugEventCallbackInterface(IDebugEventCallback2* piDebugEventCallback);
    IDebugEventCallback2* debugEventCallbackInterface() const {return _piDebugEventCallback;};

private:
    // Do not allow use of my default constructor:
    vspEventObserver() = delete;

    void onBreakpointHitEvent(const apBreakpointHitEvent& breakpointEve, IDebugThread2* pTriggeringThread);

    /// Handle Exception event in the client application
    /// \Param exceptionEvent the event with the call stack information
    void OnExceptionEvent(const apExceptionEvent& exceptionEvent);

    void showDeferredCommandDialog(const char* message);
    void hideDeferredCommandDialog();

private:
    vspCDebugEngine& _debugEngine;
    IDebugEventCallback2* _piDebugEventCallback;
    acProgressDlg* m_pWaitingForDeferredCommandDlg;

    static gtVector<vspEventObserver*> ms_registeredObservers;

    static IVscEventObserverOwner* _pOwner;
};

#endif //__VSPEVENTOBSERVER_H

