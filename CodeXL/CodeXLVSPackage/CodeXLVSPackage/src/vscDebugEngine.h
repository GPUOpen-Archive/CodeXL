//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDebugEngine.h
///
//==================================================================================

//------------------------------ vspDebugEngine.h ------------------------------

#ifndef __VSPDEBUGENGINE_H
#define __VSPDEBUGENGINE_H

// Core interfaces:
#include <Include\Public\CoreInterfaces\IVscDebugEngineOwner.h>

// inc/vspDebugEngine.h : Declaration of the vspCDebugEngine


#include "resource.h"       // main symbols

#include "CodeXLVSPackageCore.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
    #error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>

// Local:
#include <src/vspEventObserver.h>

using namespace ATL;

// Forward declarations:
class vspCDebugThread;
class vspCDebugModule;

// vspCDebugEngine

class ATL_NO_VTABLE vspCDebugEngine :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<vspCDebugEngine, &CLSID_vspDebugEngine>,
    public IDebugEngine2,
    public IDebugEngineLaunch2,
    public IDebugProgramNode2,
    public IDebugProgram2
{
public:
    vspCDebugEngine();
    virtual ~vspCDebugEngine();

    static void setOwner(IVscDebugEngineOwner* pOwner);

    DECLARE_REGISTRY_RESOURCEID(IDR_VSPDEBUGENGINE)


    BEGIN_COM_MAP(vspCDebugEngine)
    COM_INTERFACE_ENTRY(IDebugEngine2)
    COM_INTERFACE_ENTRY(IDebugEngineLaunch2)
    COM_INTERFACE_ENTRY(IDebugProgramNode2)
    COM_INTERFACE_ENTRY(IDebugProgram2)
    END_COM_MAP()

    ////////////////////////////////////////////////////////////
    // IDebugEngine2
    STDMETHOD(EnumPrograms)(IEnumDebugPrograms2** ppEnum);
    STDMETHOD(Attach)(
        IDebugProgram2** rgpPrograms,
        IDebugProgramNode2** rgpProgramNodes,
        DWORD celtPrograms,
        IDebugEventCallback2* pCallback,
        ATTACH_REASON dwReason);
    STDMETHOD(CreatePendingBreakpoint)(
        IDebugBreakpointRequest2* pBPRequest,
        IDebugPendingBreakpoint2** ppPendingBP);
    STDMETHOD(SetException)(EXCEPTION_INFO* pException);
    STDMETHOD(RemoveSetException)(EXCEPTION_INFO* pException);
    STDMETHOD(RemoveAllSetExceptions)(REFGUID guidType);
    STDMETHOD(GetEngineId)(GUID* pguidEngine);
    STDMETHOD(DestroyProgram)(IDebugProgram2* pProgram);
    STDMETHOD(ContinueFromSynchronousEvent)(IDebugEvent2* pEvent);
    STDMETHOD(SetLocale)(WORD wLangID);
    STDMETHOD(SetRegistryRoot)(LPCOLESTR pszRegistryRoot);
    STDMETHOD(SetMetric)(LPCOLESTR pszMetric, VARIANT varValue);
    STDMETHOD(CauseBreak)(void);

    ////////////////////////////////////////////////////////////
    // IDebugEngineLaunch2
    STDMETHOD(LaunchSuspended)(
        LPCOLESTR pszMachine,
        IDebugPort2* pPort,
        LPCOLESTR pszExe,
        LPCOLESTR pszArgs,
        LPCOLESTR pszDir,
        BSTR bstrEnv,
        LPCOLESTR pszOptions,
        LAUNCH_FLAGS dwLaunchFlags,
        DWORD hStdInput,
        DWORD hStdOutput,
        DWORD hStdError,
        IDebugEventCallback2* pCallback,
        IDebugProcess2** ppDebugProcess);
    STDMETHOD(ResumeProcess)(IDebugProcess2* pProcess);
    STDMETHOD(CanTerminateProcess)(IDebugProcess2* pProcess);
    STDMETHOD(TerminateProcess)(IDebugProcess2* pProcess);

    ////////////////////////////////////////////////////////////
    // IDebugProgramNode2
    // STDMETHOD(CreatePendingBreakpoint)(IDebugBreakpointRequest2 *pBPRequest, IDebugPendingBreakpoint2** ppPendingBP); already defined for IDebugEngine2
    STDMETHOD(GetProgramName)(BSTR* pbstrProgramName);
    STDMETHOD(GetHostName)(DWORD dwHostNameType, BSTR* pbstrHostName);
    STDMETHOD(GetHostPid)(AD_PROCESS_ID* pHostProcessId);
    STDMETHOD(GetHostMachineName_V7)(BSTR* pbstrHostMachineName);
    STDMETHOD(Attach_V7)(
        IDebugProgram2* pMDMProgram,
        IDebugEventCallback2* pCallback,
        DWORD dwReason);
    STDMETHOD(GetEngineInfo)(BSTR* pbstrEngine, GUID* pguidEngine);
    STDMETHOD(DetachDebugger_V7)(void);

    ////////////////////////////////////////////////////////////
    // IDebugProgram2
    STDMETHOD(EnumThreads)(IEnumDebugThreads2** ppEnum);
    STDMETHOD(GetName)(BSTR* pbstrName);
    STDMETHOD(GetProcess)(IDebugProcess2** ppProcess);
    STDMETHOD(Terminate)(void);
    STDMETHOD(Attach)(IDebugEventCallback2* pCallback);
    STDMETHOD(Detach)(void);
    STDMETHOD(GetProgramId)(GUID* pguidProgramId);
    STDMETHOD(GetDebugProperty)(IDebugProperty2** ppProperty);
    STDMETHOD(Execute)(void);
    STDMETHOD(Continue)(IDebugThread2* pThread);
    STDMETHOD(Step)(IDebugThread2* pThread, STEPKIND sk, STEPUNIT step);
    // STDMETHOD(CauseBreak)(void); already defined for IDebugEngine2
    // STDMETHOD(GetEngineInfo)(BSTR* pbstrEngine, GUID* pguidEngine); already defined for IDebugProgramNode2
    STDMETHOD(EnumCodeContexts)(
        IDebugDocumentPosition2* pDocPos,
        IEnumDebugCodeContexts2** ppEnum);
    STDMETHOD(GetMemoryBytes)(IDebugMemoryBytes2** ppMemoryBytes);
    STDMETHOD(GetDisassemblyStream)(
        DISASSEMBLY_STREAM_SCOPE dwScope,
        IDebugCodeContext2* pCodeContext,
        IDebugDisassemblyStream2** ppDisassemblyStream);
    STDMETHOD(EnumModules)(IEnumDebugModules2** ppEnum);
    STDMETHOD(GetENCUpdate)(IDebugENCUpdate** ppUpdate);
    STDMETHOD(EnumCodePaths)(
        LPCOLESTR pszHint,
        IDebugCodeContext2* pStart,
        IDebugStackFrame2* pFrame,
        BOOL fSource,
        IEnumCodePaths2** ppEnum,
        IDebugCodeContext2** ppSafety);
    STDMETHOD(WriteDump)(DUMPTYPE DumpType, LPCOLESTR pszCrashDumpUrl);
    STDMETHOD(CanDetach)(void);

    // Accessors:
    const gtVector<vspCDebugThread*>&  debugProcessThreads() const {return _debugProcessThreads;}

    void onProcessTermination();
    void onThreadCreated(osThreadId threadId);
    void onKernelDebuggingWavefrontCreated(unsigned int wavefrontIndex);
    void onThreadDestroyed(osThreadId threadId);
    void onKernelDebuggingEnded();
    vspCDebugThread* getThread(osThreadId threadId);

    void onModuleLoaded(const osFilePath& moduleFilePath, osInstructionPointer moduleLoadAddress, bool areDebugSymbolsLoaded);
    void onModuleUnloaded(const osFilePath& moduleFilePath);
    vspCDebugModule* getModule(const osFilePath& moduleFilePath);

    void setIsCurrentlyKernelDebugging(bool isCurrentlyKernelDebugging) {_isCurrentlyKernelDebugging = isCurrentlyKernelDebugging;};

    // Are there currently debug engine allocated:
    static bool areThereDebugEngines() {return (_static_amountOfAllocatedEngines > 0);};

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

    void checkEditAndContinue();

private:
    static IVscDebugEngineOwner* _pOwner;
    apDebugProjectSettings _processCreationData;
    IDebugProcess2* _piDebuggedProcess;
    IDebugPortNotify2* _piDebugPortNotify;
    GUID _programGUID;
    gtVector<vspCDebugThread*> _debugProcessThreads;
    gtVector<vspCDebugModule*> _debugProcessModules;
    vspEventObserver _eventObserver;
    bool _isCurrentlyKernelDebugging;

    // Contain true iff theres more then one debug engine instance.
    // The flag is used for log notification:
    static int _static_amountOfAllocatedEngines;
};

HRESULT vscRegisterDebugEngine();
HRESULT vscUnregisterDebugEngine();

OBJECT_ENTRY_AUTO(__uuidof(vspDebugEngine), vspCDebugEngine)

#endif //__VSPDEBUGENGINE_H

