//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdDebugProcess.h
///
//==================================================================================

//------------------------------ vsdDebugProcess.h ------------------------------

#ifndef __VSDDEBUGPROCESS_H
#define __VSDDEBUGPROCESS_H

// Visual Studio:
#include <msdbg.h>

// Local:
#include <CodeXLVSPackageDebugger/Include/vsdPackageDLLBuild.h>
#include <CodeXLVSPackageDebugger/Include/vsdUnknown.h>

// ----------------------------------------------------------------------------------
// Class Name:          vsdCDebugProcess : public IDebugProcess2, vsdCUnknown
// General Description: Wraps an IDebugProcess2 interface, providing a process abstraction
// Author:              Uri Shomroni
// Creation Date:       17/1/2012
// ----------------------------------------------------------------------------------
class VSD_API vsdCDebugProcess : public IDebugProcess3, vsdCUnknown
{
public:
    vsdCDebugProcess(IDebugProcess2& riUnderlyingProcess, IDebugPort2& riControllingPort);
    virtual ~vsdCDebugProcess();

    void setControlledEnumPrograms(IEnumDebugPrograms2* piControlledEnumPrograms);
    STDMETHOD(UnderlyingProcessEnumPrograms)(IEnumDebugPrograms2** ppEnum);

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugProcess2 methods
    STDMETHOD(GetInfo)(PROCESS_INFO_FIELDS Fields, PROCESS_INFO* pProcessInfo);
    STDMETHOD(EnumPrograms)(IEnumDebugPrograms2** ppEnum);
    STDMETHOD(GetName)(GETNAME_TYPE gnType, BSTR* pbstrName);
    STDMETHOD(GetServer)(IDebugCoreServer2** ppServer);
    STDMETHOD(Terminate)(void);
    STDMETHOD(Attach)(IDebugEventCallback2* pCallback, GUID* rgguidSpecificEngines, DWORD celtSpecificEngines, HRESULT* rghrEngineAttach);
    STDMETHOD(CanDetach)(void);
    STDMETHOD(Detach)(void);
    STDMETHOD(GetPhysicalProcessId)(AD_PROCESS_ID* pProcessId);
    STDMETHOD(GetProcessId)(GUID* pguidProcessId);
    STDMETHOD(GetAttachedSessionName)(BSTR* pbstrSessionName);
    STDMETHOD(EnumThreads)(IEnumDebugThreads2** ppEnum);
    STDMETHOD(CauseBreak)(void);
    STDMETHOD(GetPort)(IDebugPort2** ppPort);

    ////////////////////////////////////////////////////////////
    // IDebugProcess3 methods
    STDMETHOD(Execute)(IDebugThread2* pThread);
    STDMETHOD(Continue)(IDebugThread2* pThread);
    STDMETHOD(Step)(IDebugThread2* pThread, STEPKIND sk, STEPUNIT step);
    STDMETHOD(GetDebugReason)(DEBUG_REASON* pReason);
    STDMETHOD(SetHostingProcessLanguage)(REFGUID guidLang);
    STDMETHOD(GetHostingProcessLanguage)(GUID* pguidLang);
    STDMETHOD(DisableENC)(EncUnavailableReason reason);
    STDMETHOD(GetENCAvailableState)(EncUnavailableReason* preason);
    STDMETHOD(GetEngineFilter)(GUID_ARRAY* pEngineArray);

private:
    IDebugProcess2* m_piUnderlyingProcess;
    IDebugProcess3* m_piUnderlyingProcess3;
    IDebugPort2& m_riControllingPort;
    IEnumDebugPrograms2* m_piControlledEnumPrograms;
};


// ----------------------------------------------------------------------------------
// Class Name:          VSD_API vsdCEnumDebugPrograms : public IEnumDebugPrograms2, vsdCUnknown
// General Description: Wraps IEnumDebugPrograms2, enumerating IDebugProgram2-s
// Author:              Uri Shomroni
// Creation Date:       18/1/2012
// ----------------------------------------------------------------------------------
class VSD_API vsdCEnumDebugPrograms : public IEnumDebugPrograms2, vsdCUnknown
{
public:
    vsdCEnumDebugPrograms::vsdCEnumDebugPrograms(IEnumDebugPrograms2& riUnderlyingEnumPrograms);
    virtual vsdCEnumDebugPrograms::~vsdCEnumDebugPrograms();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IEnumDebugPrograms2 methods
    STDMETHOD(Next)(ULONG celt, IDebugProgram2** rgelt, ULONG* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumDebugPrograms2** ppEnum);
    STDMETHOD(GetCount)(ULONG* pcelt);

private:
    IEnumDebugPrograms2* m_piUnderlyingEnumPrograms;
};

// ----------------------------------------------------------------------------------
// Class Name:          vsdCSingleProgramEnumDebugPrograms : public IEnumDebugPrograms2
// General Description: Implements IEnumDebugPrograms2, Enumerating a single program.
// Author:              Uri Shomroni
// Creation Date:       25/1/2012
// ----------------------------------------------------------------------------------
class vsdCSingleProgramEnumDebugPrograms : public IEnumDebugPrograms2, vsdCUnknown
{
public:
    vsdCSingleProgramEnumDebugPrograms(IDebugProgram2* piProgram);
    virtual ~vsdCSingleProgramEnumDebugPrograms();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IEnumDebugPrograms2 methods
    STDMETHOD(Next)(
        ULONG celt,
        IDebugProgram2** rgelt,
        ULONG* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumDebugPrograms2** ppEnum);
    STDMETHOD(GetCount)(ULONG* pcelt);

private:
    // Do not allow use of my default constructor:
    vsdCSingleProgramEnumDebugPrograms();

private:
    // The enumerated breakpoint:
    IDebugProgram2* _piEnumProgram;

    bool _wasEnumeratorUsed;
};

// ----------------------------------------------------------------------------------
// Class Name:          VSD_API vsdCDebugProgram : public IDebugProgram2, vsdCUnknown
// General Description: Wraps an IDebugProgram2, representing a program
// Author:              Uri Shomroni
// Creation Date:       19/1/2012
// ----------------------------------------------------------------------------------
class VSD_API vsdCDebugProgram : public IDebugProgram2, vsdCUnknown
{
public:
    vsdCDebugProgram(IDebugProgram2& riUnderlyingProgram);
    virtual ~vsdCDebugProgram();

    bool isUnderlyingProgram(IDebugProgram2* piProgram) const { return (m_piUnderlyingProgram == piProgram); };

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugProgram2 methods
    STDMETHOD(EnumThreads)(IEnumDebugThreads2** ppEnum);
    STDMETHOD(GetName)(BSTR* pbstrName);
    STDMETHOD(GetProcess)(IDebugProcess2** ppProcess);
    STDMETHOD(Terminate)(void);
    STDMETHOD(Attach)(IDebugEventCallback2* pCallback);
    STDMETHOD(CanDetach)(void);
    STDMETHOD(Detach)(void);
    STDMETHOD(GetProgramId)(GUID* pguidProgramId);
    STDMETHOD(GetDebugProperty)(IDebugProperty2** ppProperty);
    STDMETHOD(Execute)(void);
    STDMETHOD(Continue)(IDebugThread2* pThread);
    STDMETHOD(Step)(IDebugThread2* pThread, STEPKIND sk, STEPUNIT step);
    STDMETHOD(CauseBreak)(void);
    STDMETHOD(GetEngineInfo)(BSTR* pbstrEngine, GUID* pguidEngine);
    STDMETHOD(EnumCodeContexts)(IDebugDocumentPosition2* pDocPos, IEnumDebugCodeContexts2** ppEnum);
    STDMETHOD(GetMemoryBytes)(IDebugMemoryBytes2** ppMemoryBytes);
    STDMETHOD(GetDisassemblyStream)(DISASSEMBLY_STREAM_SCOPE dwScope, IDebugCodeContext2* pCodeContext, IDebugDisassemblyStream2** ppDisassemblyStream);
    STDMETHOD(EnumModules)(IEnumDebugModules2** ppEnum);
    STDMETHOD(GetENCUpdate)(IDebugENCUpdate** ppUpdate);
    STDMETHOD(EnumCodePaths)(LPCOLESTR pszHint, IDebugCodeContext2* pStart, IDebugStackFrame2* pFrame, BOOL fSource, IEnumCodePaths2** ppEnum, IDebugCodeContext2** ppSafety);
    STDMETHOD(WriteDump)(DUMPTYPE DumpType, LPCOLESTR pszDumpUrl);

private:
    IDebugProgram2* m_piUnderlyingProgram;
};

// ----------------------------------------------------------------------------------
// Class Name:          VSD_API vsdCDebugProgramNode : public IDebugProgramNode2, vsdCUnknown
// General Description: Wraps an IDebugProgramNode2, representing a program node (attachment point)
// Author:              Uri Shomroni
// Creation Date:       26/5/2014
// ----------------------------------------------------------------------------------
class VSD_API vsdCDebugProgramNode : public IDebugProgramNode2, vsdCUnknown
{
public:
    vsdCDebugProgramNode(IDebugProgramNode2& riUnderlyingProgramNode);
    virtual ~vsdCDebugProgramNode();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugProgramNode2 methods
    STDMETHOD(GetProgramName)(BSTR* pbstrProgramName);
    STDMETHOD(GetHostName)(GETHOSTNAME_TYPE dwHostNameType, BSTR* pbstrHostName);
    STDMETHOD(GetHostPid)(AD_PROCESS_ID* pHostProcessId);
    STDMETHOD(GetHostMachineName_V7)(BSTR* pbstrHostMachineName);
    STDMETHOD(Attach_V7)(IDebugProgram2* pMDMProgram, IDebugEventCallback2* pCallback, DWORD dwReason);
    STDMETHOD(GetEngineInfo)(BSTR* pbstrEngine, GUID* pguidEngine);
    STDMETHOD(DetachDebugger_V7)(void);

private:
    IDebugProgramNode2* m_piUnderlyingProgramNode;
};


#endif //__VSDDEBUGPROCESS_H

