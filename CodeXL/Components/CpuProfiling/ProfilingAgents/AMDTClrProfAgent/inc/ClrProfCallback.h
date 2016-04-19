//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ClrProfCallback.h
///
//==================================================================================

#ifndef _CLRPROFCALLBACK_H_
#define _CLRPROFCALLBACK_H_

#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTProfilingAgentsData/inc/Windows/CelWriter.h>

// gtUInt64 is AssemblyID
typedef gtSet<gtUInt64> NgenAssemblyIDSet;


enum ENUMSYMLEVEL
{
    evNoneLevel     = 0,
    evUserLevel     = 0x1,
    evSystemLevel   = 0x2,
    evALL           = 0x3
};

class ClrProfCallBack : public ICorProfilerCallback2
{
public:
    ClrProfCallBack();
    ~ClrProfCallBack();

    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, void** ppInterface);

    // ICorProfilerCallback
    STDMETHOD(Initialize)(IUnknown* pICorProfilerInfoUnk);
    STDMETHOD(Shutdown)(void);
    STDMETHOD(AppDomainCreationStarted)(AppDomainID appDomainId);
    STDMETHOD(AppDomainCreationFinished)(AppDomainID appDomainId, HRESULT hrStatus);
    STDMETHOD(AppDomainShutdownStarted)(AppDomainID appDomainId);
    STDMETHOD(AppDomainShutdownFinished)(AppDomainID appDomainId, HRESULT hrStatus);
    STDMETHOD(AssemblyLoadStarted)(AssemblyID assemblyId);
    STDMETHOD(AssemblyLoadFinished)(AssemblyID assemblyId, HRESULT hrStatus);
    STDMETHOD(AssemblyUnloadStarted)(AssemblyID assemblyId);
    STDMETHOD(AssemblyUnloadFinished)(AssemblyID assemblyId, HRESULT hrStatus);
    STDMETHOD(ModuleLoadStarted)(ModuleID moduleId);
    STDMETHOD(ModuleLoadFinished)(ModuleID moduleId, HRESULT hrStatus);
    STDMETHOD(ModuleUnloadStarted)(ModuleID moduleId);
    STDMETHOD(ModuleUnloadFinished)(ModuleID moduleId, HRESULT hrStatus);
    STDMETHOD(ModuleAttachedToAssembly)(ModuleID moduleId, AssemblyID assemblyId);
    STDMETHOD(ClassLoadStarted)(ClassID classId);
    STDMETHOD(ClassLoadFinished)(ClassID classId, HRESULT hrStatus);
    STDMETHOD(ClassUnloadStarted)(ClassID classId);
    STDMETHOD(ClassUnloadFinished)(ClassID classId, HRESULT hrStatus);
    STDMETHOD(FunctionUnloadStarted)(FunctionID functionId);
    STDMETHOD(JITCompilationStarted)(FunctionID functionId, BOOL fIsSafeToBlock);
    STDMETHOD(JITCompilationFinished)(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock);
    STDMETHOD(JITCachedFunctionSearchStarted)(FunctionID FunctionID, BOOL* pbUseCachedFunction);
    STDMETHOD(JITCachedFunctionSearchFinished)(FunctionID functionId, COR_PRF_JIT_CACHE result);
    STDMETHOD(JITFunctionPitched)(FunctionID functionId);
    STDMETHOD(JITInlining)(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline);
    STDMETHOD(ThreadCreated)(ThreadID threadId);
    STDMETHOD(ThreadDestroyed)(ThreadID threadId);
    STDMETHOD(ThreadAssignedToOSThread)(ThreadID managedThreadId, DWORD osThreadID);
    STDMETHOD(RemotingClientInvocationStarted)(void);
    STDMETHOD(RemotingClientSendingMessage)(GUID* pCookie, BOOL fIsAsync);
    STDMETHOD(RemotingClientReceivingReply)(GUID* pCookie, BOOL fIsAsync);
    STDMETHOD(RemotingClientInvocationFinished)(void);
    STDMETHOD(RemotingServerReceivingMessage)(GUID* pCookie, BOOL fIsAsync);
    STDMETHOD(RemotingServerInvocationStarted)(void);
    STDMETHOD(RemotingServerInvocationReturned)(void);
    STDMETHOD(RemotingServerSendingReply)(GUID* pCookie, BOOL fIsAsync);
    STDMETHOD(UnmanagedToManagedTransition)(FunctionID functionId, COR_PRF_TRANSITION_REASON reason);
    STDMETHOD(ManagedToUnmanagedTransition)(FunctionID functionId, COR_PRF_TRANSITION_REASON reason);
    STDMETHOD(RuntimeSuspendStarted)(COR_PRF_SUSPEND_REASON suspendReason);
    STDMETHOD(RuntimeSuspendFinished)(void);
    STDMETHOD(RuntimeSuspendAborted)(void);
    STDMETHOD(RuntimeResumeStarted)(void);
    STDMETHOD(RuntimeResumeFinished)(void);
    STDMETHOD(RuntimeThreadSuspended)(ThreadID threadId);
    STDMETHOD(RuntimeThreadResumed)(ThreadID threadId);
    STDMETHOD(MovedReferences)(ULONG cMovedObjectIDRanges,
                               ObjectID oldObjectIDRangeStart[],
                               ObjectID newObjectIDRangeStart[],
                               ULONG cObjectIDRangeLength[]);
    STDMETHOD(ObjectAllocated)(ObjectID objectId, ClassID classId);
    STDMETHOD(ObjectsAllocatedByClass)(ULONG cClassCount, ClassID classIds[], ULONG cObjects[]);
    STDMETHOD(ObjectReferences)(ObjectID objectId, ClassID classId, ULONG cObjectRefs, ObjectID objectRefIds[]);
    STDMETHOD(RootReferences)(ULONG cRootRefs, ObjectID rootRefIds[]);
    STDMETHOD(ExceptionThrown)(ObjectID thrownObjectId);
    STDMETHOD(ExceptionSearchFunctionEnter)(FunctionID functionId);
    STDMETHOD(ExceptionSearchFunctionLeave)(void);
    STDMETHOD(ExceptionSearchFilterEnter)(FunctionID functionId);
    STDMETHOD(ExceptionSearchFilterLeave)(void);
    STDMETHOD(ExceptionSearchCatcherFound)(FunctionID functionId);
    STDMETHOD(ExceptionOSHandlerEnter)(FunctionID functionId);
    STDMETHOD(ExceptionOSHandlerLeave)(FunctionID functionId);
    STDMETHOD(ExceptionUnwindFunctionEnter)(FunctionID functionId);
    STDMETHOD(ExceptionUnwindFunctionLeave)(void);
    STDMETHOD(ExceptionUnwindFinallyEnter)(FunctionID functionId);
    STDMETHOD(ExceptionUnwindFinallyLeave)(void);
    STDMETHOD(ExceptionCatcherEnter)(FunctionID functionId, ObjectID objectId);
    STDMETHOD(ExceptionCatcherLeave)(void);
    STDMETHOD(COMClassicVTableCreated)(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable, ULONG cSlots);
    STDMETHOD(COMClassicVTableDestroyed)(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable);
    STDMETHOD(ExceptionCLRCatcherFound)(void);
    STDMETHOD(ExceptionCLRCatcherExecute)(void);

    // Method for ICorProfilerCallback2
    STDMETHOD(ThreadNameChanged)(ThreadID threadId, ULONG cchName, WCHAR name[]);

    STDMETHOD(GarbageCollectionStarted)(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason);
    STDMETHOD(SurvivingReferences)(
        /* [in] */ ULONG cSurvivingObjectIDRanges,
        /* [size_is][in] */ ObjectID objectIDRangeStart[],
        /* [size_is][in] */ ULONG cObjectIDRangeLength[]);

    STDMETHOD(GarbageCollectionFinished)(void);

    STDMETHOD(FinalizeableObjectQueued)(
        /* [in] */ DWORD finalizerFlags,
        /* [in] */ ObjectID objectID);

    STDMETHOD(RootReferences2)(
        /* [in] */ ULONG cRootRefs,
        /* [size_is][in] */ ObjectID rootRefIds[],
        /* [size_is][in] */ COR_PRF_GC_ROOT_KIND rootKinds[],
        /* [size_is][in] */ COR_PRF_GC_ROOT_FLAGS rootFlags[],
        /* [size_is][in] */ UINT_PTR rootIds[]);

    STDMETHOD(HandleCreated)(
        /* [in] */ GCHandleID handleId,
        /* [in] */ ObjectID initialObjectId);

    STDMETHOD(HandleDestroyed)(
        /* [in] */ GCHandleID handleId);

    // End of ICorProfilerCallback interface

private:
    int RecursiveMakeDir(wchar_t* dir);
    bool GetInititializationParameters();
    bool GetMethodNameFromFunctionId(FunctionID, LPWSTR wszClass, LPWSTR wszFunction);
    //HRESULT GetFunctionProperties(FunctionID functionID, BOOL *isStatic, ULONG *argCount,
    //                              WCHAR *returnTypeStr, WCHAR *functionParameters, WCHAR *functionName);
    bool GetClassNameFromClassId(ClassID classID, LPWSTR wszClass, ModuleID* pModuleId = NULL);

    void QueryNgenSymbols(wchar_t* pNgenFileName);
    HRESULT CheckNgenSymbols(ModuleID moduleId, AssemblyID assemblyId);

#ifdef _CXLDEBUG
    int ProfilerPrintf(char* pszFormat, ...);
    void ProfilerPrintfCodeBytes(LPCBYTE startAddr, ULONG cSize);
#endif


    wchar_t m_szCELfileName[OS_MAX_PATH];
    wchar_t m_CLRJITDIR[OS_MAX_PATH];
    wchar_t m_CorVersion[OS_MAX_PATH];

    DWORD m_dwEventMask;
    ICorProfilerInfo* m_pICorProfilerInfo;
    bool m_Initialized;
    CelWriter* m_pCelWriter;

    CRITICAL_SECTION m_criticalSection;
    DWORD   m_EnumSymLevel;
    bool    m_b64bit;
    wchar_t m_WindowsDir[OS_MAX_PATH];
    NgenAssemblyIDSet m_NgenAssemblySet;

#ifdef _CXLDEBUG
    wchar_t m_szDebugOutfileName[OS_MAX_PATH];
    FILE* m_pDebugOutFile;
#endif
};

#endif // _CLRPROFCALLBACK_H_
