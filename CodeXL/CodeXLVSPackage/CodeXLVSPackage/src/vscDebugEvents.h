//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDebugEvents.h
///
//==================================================================================

//------------------------------ vspDebugEvents.h ------------------------------

#ifndef __VSPDEBUGEVENTS_H
#define __VSPDEBUGEVENTS_H

// Forward declaration:
class apExceptionEvent;
class vspCDebugBreakpoint;
class vspCDebugEngine;
class vspCDebugExpression;
class vspCDebugModule;
class vspCDebugProperty;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osExceptionReason.h>

// Local:
#include <src/vspUnknown.h>

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugEvent : public IDebugEvent2
// General Description: A base implementation for IDebugEvent2, to be derived into I*Event2
//                      implementations.
// Author:               Uri Shomroni
// Creation Date:        8/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugEvent : public IDebugEvent2, vspCUnknown
{
protected:
    vspCDebugEvent(REFIID eventInterfaceID);
    virtual ~vspCDebugEvent();

    // Must be implemented by subclasses:
    virtual DWORD eventAttributes() const = 0;

public:
    bool send(IDebugEventCallback2* piEventCallback, IDebugEngine2* piEngine, IDebugProgram2* piProgram, IDebugThread2* piThread);

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugEvent2 methods
    STDMETHOD(GetAttributes)(DWORD* pdwAttrib);

private:
    // Do not allow use of my default constructor:
    vspCDebugEvent();

private:
    IID _eventInterfaceID;
};


// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugEngineCreateEvent: public vspCDebugEvent, IDebugEngineCreateEvent2
// General Description: Implements IDebugEngineCreateEvent2
// Author:               Uri Shomroni
// Creation Date:        8/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugEngineCreateEvent: public vspCDebugEvent, IDebugEngineCreateEvent2
{
public:
    vspCDebugEngineCreateEvent(IDebugEngine2* piEngine);
    virtual ~vspCDebugEngineCreateEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugEngineCreateEvent2 methods
    STDMETHOD(GetEngine)(IDebugEngine2** ppEngine);

private:
    // Do not allow use of my default constructor:
    vspCDebugEngineCreateEvent();

private:
    IDebugEngine2* _piEngine;
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugProgramCreateEvent: public vspCDebugEvent, IDebugProgramCreateEvent2
// General Description: Implements IDebugProgramCreateEvent2
// Author:               Uri Shomroni
// Creation Date:        8/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugProgramCreateEvent: public vspCDebugEvent, IDebugProgramCreateEvent2
{
public:
    vspCDebugProgramCreateEvent();
    virtual ~vspCDebugProgramCreateEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugProgramCreateEvent2 methods
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugProgramDestroyEvent: public vspCDebugEvent, IDebugProgramDestroyEvent2
// General Description: Implements IDebugProgramDestroyEvent2
// Author:               Uri Shomroni
// Creation Date:        20/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugProgramDestroyEvent: public vspCDebugEvent, IDebugProgramDestroyEvent2
{
public:
    vspCDebugProgramDestroyEvent(int exitCode);
    virtual ~vspCDebugProgramDestroyEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugProgramDestroyEvent2 methods
    STDMETHOD(GetExitCode)(DWORD* pdwExit);

private:
    // Do not allow use of my default constructor:
    vspCDebugProgramDestroyEvent();

private:
    int _exitCode;
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugThreadCreateEvent: public vspCDebugEvent, IDebugThreadCreateEvent2
// General Description: Implements IDebugThreadCreateEvent2
// Author:               Uri Shomroni
// Creation Date:        12/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugThreadCreateEvent: public vspCDebugEvent, IDebugThreadCreateEvent2
{
public:
    vspCDebugThreadCreateEvent();
    virtual ~vspCDebugThreadCreateEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugThreadCreateEvent2 methods
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugThreadDestroyEvent: public vspCDebugEvent, IDebugThreadDestroyEvent2
// General Description: Implements IDebugThreadDestroyEvent2
// Author:               Uri Shomroni
// Creation Date:        21/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugThreadDestroyEvent: public vspCDebugEvent, IDebugThreadDestroyEvent2
{
public:
    vspCDebugThreadDestroyEvent(int exitCode);
    virtual ~vspCDebugThreadDestroyEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugThreadDestroyEvent2 methods
    STDMETHOD(GetExitCode)(DWORD* pdwExit);

private:
    // Do not allow use of my default constructor:
    vspCDebugThreadDestroyEvent();

private:
    int _exitCode;
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugModuleLoadEvent: public vspCDebugEvent, IDebugModuleLoadEvent2
// General Description: Implements IDebugModuleLoadEvent2
// Author:               Uri Shomroni
// Creation Date:        26/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugModuleLoadEvent: public vspCDebugEvent, IDebugModuleLoadEvent2
{
public:
    vspCDebugModuleLoadEvent(vspCDebugModule* pModule, bool wasLoaded);
    virtual ~vspCDebugModuleLoadEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugModuleLoadEvent2 methods
    STDMETHOD(GetModule)(IDebugModule2** pModule, BSTR* pbstrDebugMessage, BOOL* pbLoad);

private:
    // Do not allow use of my default constructor:
    vspCDebugModuleLoadEvent();

private:
    vspCDebugModule* _pModule;
    gtString _debugMessage;
    bool _wasLoaded;
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugLoadCompleteEvent: public vspCDebugEvent, IDebugLoadCompleteEvent2
// General Description: Implements IDebugLoadCompleteEvent2
// Author:               Uri Shomroni
// Creation Date:        12/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugLoadCompleteEvent: public vspCDebugEvent, IDebugLoadCompleteEvent2
{
public:
    vspCDebugLoadCompleteEvent();
    virtual ~vspCDebugLoadCompleteEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugLoadCompleteEvent2 methods
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugBreakEvent: public vspCDebugEvent, IDebugBreakEvent2
// General Description: Implements IDebugBreakEvent2
// Author:               Uri Shomroni
// Creation Date:        13/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugBreakEvent: public vspCDebugEvent, IDebugBreakEvent2
{
public:
    vspCDebugBreakEvent();
    virtual ~vspCDebugBreakEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugBreakEvent2 methods
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugStepCompleteEvent: public vspCDebugEvent, IDebugStepCompleteEvent2
// General Description: Implements IDebugStepCompleteEvent2
// Author:               Uri Shomroni
// Creation Date:        21/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugStepCompleteEvent: public vspCDebugEvent, IDebugStepCompleteEvent2
{
public:
    vspCDebugStepCompleteEvent();
    virtual ~vspCDebugStepCompleteEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugStepCompleteEvent2 methods
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugBreakpointBoundEvent: public vspCDebugEvent, IDebugBreakpointBoundEvent2
// General Description: Implements IDebugBreakpointBoundEvent2
// Author:               Uri Shomroni
// Creation Date:        4/10/2010
// ----------------------------------------------------------------------------------
class vspCDebugBreakpointBoundEvent: public vspCDebugEvent, IDebugBreakpointBoundEvent2
{
public:
    vspCDebugBreakpointBoundEvent(vspCDebugBreakpoint* pBreakpoint);
    virtual ~vspCDebugBreakpointBoundEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugBreakpointBoundEvent2 methods
    STDMETHOD(GetPendingBreakpoint)(IDebugPendingBreakpoint2** ppPendingBP);
    STDMETHOD(EnumBoundBreakpoints)(IEnumDebugBoundBreakpoints2** ppEnum);

private:
    // Do not allow use of my default constructor:
    vspCDebugBreakpointBoundEvent();

private:
    vspCDebugBreakpoint* _pBreakpoint;
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugBreakpointUnBoundEvent: public vspCDebugEvent, IDebugBreakpointUnboundEvent2
// General Description: Implements IDebugBreakpointUnboundEvent2
// Author:               Uri Shomroni
// Creation Date:        15/5/2011
// ----------------------------------------------------------------------------------
class vspCDebugBreakpointUnboundEvent: public vspCDebugEvent, IDebugBreakpointUnboundEvent2
{
public:
    vspCDebugBreakpointUnboundEvent(vspCDebugBreakpoint* pBreakpoint, BP_UNBOUND_REASON reason);
    virtual ~vspCDebugBreakpointUnboundEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugBreakpointUnboundEvent2 methods
    STDMETHOD(GetBreakpoint)(IDebugBoundBreakpoint2** ppBP);
    STDMETHOD(GetReason)(BP_UNBOUND_REASON* pdwUnboundReason);

private:
    // Do not allow use of my default constructor:
    vspCDebugBreakpointUnboundEvent();

private:
    vspCDebugBreakpoint* _pBreakpoint;
    BP_UNBOUND_REASON _reason;
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugBreakpointErrorEvent: public vspCDebugEvent, IDebugBreakpointErrorEvent2
// General Description: Implements IDebugBreakpointErrorEvent2
// Author:               Uri Shomroni
// Creation Date:        4/10/2010
// ----------------------------------------------------------------------------------
class vspCDebugBreakpointErrorEvent: public vspCDebugEvent, IDebugBreakpointErrorEvent2
{
public:
    vspCDebugBreakpointErrorEvent(vspCDebugBreakpoint* pBreakpoint);
    virtual ~vspCDebugBreakpointErrorEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugBreakpointErrorEvent2 methods
    STDMETHOD(GetErrorBreakpoint)(IDebugErrorBreakpoint2** ppErrorBP);

private:
    // Do not allow use of my default constructor:
    vspCDebugBreakpointErrorEvent();

private:
    vspCDebugBreakpoint* _pBreakpoint;
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugBreakpointEvent: public vspCDebugEvent, IDebugBreakpointEvent2
// General Description: Implements IDebugBreakpointEvent2
// Author:               Uri Shomroni
// Creation Date:        4/10/2010
// ----------------------------------------------------------------------------------
class vspCDebugBreakpointEvent: public vspCDebugEvent, IDebugBreakpointEvent2
{
public:
    vspCDebugBreakpointEvent(vspCDebugBreakpoint* pBreakpoint);
    virtual ~vspCDebugBreakpointEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugBreakpointEvent2 methods
    STDMETHOD(EnumBreakpoints)(IEnumDebugBoundBreakpoints2** ppEnum);

private:
    // Do not allow use of my default constructor:
    vspCDebugBreakpointEvent();

private:
    vspCDebugBreakpoint* _pBreakpoint;
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugExceptionEvent: public vspCDebugEvent, IDebugExceptionEvent2
// General Description: Implements IDebugExceptionEvent2
// Author:               Uri Shomroni
// Creation Date:        10/10/2010
// ----------------------------------------------------------------------------------
class vspCDebugExceptionEvent: public vspCDebugEvent, IDebugExceptionEvent2
{
public:
    vspCDebugExceptionEvent(vspCDebugEngine* pDebugEngine, const apExceptionEvent& exceptionEve);
    virtual ~vspCDebugExceptionEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugExceptionEvent2 methods
    STDMETHOD(GetException)(EXCEPTION_INFO* pExceptionInfo);
    STDMETHOD(GetExceptionDescription)(BSTR* pbstrDescription);
    STDMETHOD(CanPassToDebuggee)(void);
    STDMETHOD(PassToDebuggee)(BOOL fPass);

private:
    // Do not allow use of my default constructor:
    vspCDebugExceptionEvent();

private:
    vspCDebugEngine* _pDebugEngine;
    osExceptionReason _exceptionReason;
    bool _isSecondChanceException;
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugExpressionEvaluationCompleteEvent: public vspCDebugEvent, IDebugExpressionEvaluationCompleteEvent2
// General Description: Implements IDebugExpressionEvaluationCompleteEvent2
// Author:               Uri Shomroni
// Creation Date:        4/10/2010
// ----------------------------------------------------------------------------------
class vspCDebugExpressionEvaluationCompleteEvent: public vspCDebugEvent, IDebugExpressionEvaluationCompleteEvent2
{
public:
    vspCDebugExpressionEvaluationCompleteEvent(vspCDebugExpression* pExpression, vspCDebugProperty* pValueAsProperty);
    virtual ~vspCDebugExpressionEvaluationCompleteEvent();

    // Overrides vspCDebugEvent:
    virtual DWORD eventAttributes() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugExpressionEvaluationCompleteEvent2 methods
    STDMETHOD(GetExpression)(IDebugExpression2** ppExpr);
    STDMETHOD(GetResult)(IDebugProperty2** ppResult);

private:
    // Do not allow use of my default constructor:
    vspCDebugExpressionEvaluationCompleteEvent();

private:
    vspCDebugExpression* _pExpression;
    vspCDebugProperty* _pValueAsProperty;
};

#endif //__VSPDEBUGEVENTS_H

