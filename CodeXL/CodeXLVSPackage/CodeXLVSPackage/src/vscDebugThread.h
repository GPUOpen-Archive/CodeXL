//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDebugThread.h
///
//==================================================================================

//------------------------------ vspDebugThread.h ------------------------------

#ifndef __VSPDEBUGTHREAD_H
#define __VSPDEBUGTHREAD_H

// Forward declarations:
class vspCDebugEngine;

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <src/vspUnknown.h>

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugThread : public IDebugThread2
// General Description: A class that implements IDebugThread2, representing a thread
//                      in the debugged process
// Author:               Uri Shomroni
// Creation Date:        12/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugThread : public IDebugThread2, vspCUnknown
{
public:
    vspCDebugThread(vspCDebugEngine* pDebugEngine, DWORD threadId, bool m_isKernelDebuggingThread, bool isMainThread);
    virtual ~vspCDebugThread();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugThread2 methods
    STDMETHOD(EnumFrameInfo)(
        FRAMEINFO_FLAGS dwFieldSpec,
        UINT nRadix,
        IEnumDebugFrameInfo2** ppEnum);
    STDMETHOD(GetName)(BSTR* pbstrName);
    STDMETHOD(SetThreadName)(LPCOLESTR pszName);
    STDMETHOD(GetProgram)(IDebugProgram2** ppProgram);
    STDMETHOD(CanSetNextStatement)(
        IDebugStackFrame2* pStackFrame,
        IDebugCodeContext2* pCodeContext);
    STDMETHOD(SetNextStatement)(
        IDebugStackFrame2* pStackFrame,
        IDebugCodeContext2* pCodeContext);
    STDMETHOD(GetThreadId)(DWORD* pdwThreadId);
    STDMETHOD(Suspend)(DWORD* pdwSuspendCount);
    STDMETHOD(Resume)(DWORD* pdwSuspendCount);
    STDMETHOD(GetThreadProperties)(
        THREADPROPERTY_FIELDS dwFields,
        THREADPROPERTIES* ptp);
    STDMETHOD(GetLogicalThread)(
        IDebugStackFrame2* pStackFrame,
        IDebugLogicalThread2** ppLogicalThread);

    DWORD threadId() const {return _threadId;};
    void setThreadState(DWORD threadState) {_threadState = threadState;};
    bool isKernelDebugging() const {return m_isKernelDebuggingThread;};

private:
    // Do not allow use of my default constructor:
    vspCDebugThread();

private:
    vspCDebugEngine* _pDebugEngine;
    DWORD _threadId;
    bool m_isKernelDebuggingThread;
    bool _isMainThread;
    unsigned int _threadIndex;
    DWORD _threadState;
};

// ----------------------------------------------------------------------------------
// Class Name:          vspCEnumDebugThreads : public IEnumDebugThreads2
// General Description: Implements IEnumDebugThreads2, Enumerating the currently existing threads
// Author:               Uri Shomroni
// Creation Date:        13/9/2010
// ----------------------------------------------------------------------------------
class vspCEnumDebugThreads : public IEnumDebugThreads2, vspCUnknown
{
public:
    vspCEnumDebugThreads(const gtVector<vspCDebugThread*>& currentThreads);
    virtual ~vspCEnumDebugThreads();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IEnumDebugThreads2 methods
    STDMETHOD(Next)(
        ULONG celt,
        IDebugThread2** rgelt,
        ULONG* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumDebugThreads2** ppEnum);
    STDMETHOD(GetCount)(ULONG* pcelt);

private:
    // Do not allow use of my default constructor:
    vspCEnumDebugThreads();

private:
    // The enumerated threads:
    gtVector<vspCDebugThread*> _enumThreads;


    unsigned int _currentPosition;
};

#endif //__VSPDEBUGTHREAD_H

