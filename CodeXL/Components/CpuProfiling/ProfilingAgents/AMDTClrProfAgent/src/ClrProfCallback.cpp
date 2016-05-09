//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ClrProfCallback.cpp
///
//==================================================================================

#include <direct.h>
#include <Mscoree.h>
#include <psapi.h>

#include <ProfilingAgents/Utils/ExecutableReader.h>
#include <ClrProfCallBack.h>
#include <AMDTProfilingAgentsData/inc/JncWriter.h>
#include <AMDTProfilingAgentsData/inc/Windows/PjsWriter.h>
#include <BasicHelper.h>

//uses windows system time, adjusted to use the same data structure as Linux
static gtUInt64 GetSystemTime()
{
    LARGE_INTEGER systime;
    QueryPerformanceCounter(&systime);
    return systime.QuadPart;
}

ClrProfCallBack::ClrProfCallBack()
{
    //  InitializeCriticalSectionAndSpinCount( &m_criticalSection, 10000 );
    InitializeCriticalSection(&m_criticalSection);

    m_pICorProfilerInfo = NULL;
    m_pCelWriter = NULL;

    m_EnumSymLevel = evUserLevel;

    m_NgenAssemblySet.clear();

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    m_b64bit = true;
#else
    m_b64bit = false;
#endif

#ifdef _CXLDEBUG
    m_pDebugOutFile = NULL;
#endif

}

ClrProfCallBack::~ClrProfCallBack()
{
    DeleteCriticalSection(&m_criticalSection);
    m_NgenAssemblySet.clear();
}

//====================================================
ULONG ClrProfCallBack::AddRef()
{
    return S_OK;
}

ULONG ClrProfCallBack::Release()
{
    return S_OK;
}


HRESULT ClrProfCallBack::QueryInterface(REFIID riid, void** ppInterface)
{
    if (riid == IID_IUnknown)
    {
        *ppInterface = static_cast<IUnknown*>(this);
    }
    else if (riid == IID_ICorProfilerCallback)
    {
        *ppInterface = static_cast<ICorProfilerCallback*>(this);
    }
    else if (riid == IID_ICorProfilerCallback2)
    {
        *ppInterface = static_cast<ICorProfilerCallback2*>(this);
    }
    else
    {
        *ppInterface = NULL;

        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppInterface)->AddRef();

    return S_OK;

} // ProfilerCallbackBase::QueryInterface


//====================================================

HRESULT ClrProfCallBack::Initialize(IUnknown* pICorProfilerInfoUnk)
{
    //
    ScopedLock cSynch(m_criticalSection);

#if AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE
    GetInititializationParameters();
    /*
    if (!GetInititializationParameters() )
        __asm int 3
    */
#else
    GetInititializationParameters();
#endif

    // Get the ICorProfilerInfo interface we need, and stuff it away in
    // a member variable.
    HRESULT hr = pICorProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo, (LPVOID*)&m_pICorProfilerInfo);

    if (FAILED(hr))
    {
        return E_INVALIDARG;
    }

    // Indicate which events we're interested in.
    // See GetInititializationParameters
    m_pICorProfilerInfo->SetEventMask(m_dwEventMask);

    DWORD retLength;
    GetCORVersion(m_CorVersion, OS_MAX_PATH, &retLength);

#ifdef _CXLDEBUG
    // Open the file that we'll write out output to
    m_pDebugOutFile = _wfopen(m_szDebugOutfileName, L"wt");
    ProfilerPrintf("Initialize\n");
    ProfilerPrintf("CLR Version %ls\n", m_CorVersion);
#endif

    return S_OK;
}

HRESULT ClrProfCallBack::Shutdown()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("Shutdown\n");
    fclose(m_pDebugOutFile);
    m_pDebugOutFile = NULL;
#endif

    if (m_pCelWriter)
    {
        delete m_pCelWriter;
        m_pCelWriter = NULL;
    }

    return S_OK;
}


HRESULT ClrProfCallBack::AppDomainCreationStarted(AppDomainID appDomainId)
{
    GT_UNREFERENCED_PARAMETER(appDomainId);

#ifdef _CXLDEBUG
    ProfilerPrintf("AppDomainCreationStarted\n");

#endif

    return E_NOTIMPL;
}


HRESULT ClrProfCallBack::AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus)
{
    GT_UNREFERENCED_PARAMETER(hrStatus);

    ScopedLock cSynch(m_criticalSection);

    // Now it's OK to grab the name
    wchar_t wszDomain[512];
    ULONG cchDomain = sizeof(wszDomain) / sizeof(wszDomain[0]);

    m_pICorProfilerInfo->GetAppDomainInfo(appDomainId, cchDomain, &cchDomain,
                                          wszDomain, 0);
    gtUInt64 systime = GetSystemTime();
    m_pCelWriter->WriteAppDomainCreationFinished(appDomainId, systime,
                                                 cchDomain + 1, wszDomain);
#ifdef _CXLDEBUG
    ProfilerPrintf("AppDomainCreationFinished: %ls\n", wszDomain);
#endif

    return S_OK;
}


HRESULT ClrProfCallBack::AppDomainShutdownStarted(AppDomainID appDomainId)
{
    ScopedLock cSynch(m_criticalSection);

    gtUInt64 systime = GetSystemTime();
    m_pCelWriter->WriteAppDomainShutdownStarted(appDomainId, systime);

#ifdef _CXLDEBUG
    ProfilerPrintf("AppDomainShutdownStarted\n");
#endif
    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus)
{
    GT_UNREFERENCED_PARAMETER(appDomainId);
    GT_UNREFERENCED_PARAMETER(hrStatus);

#ifdef _CXLDEBUG
    ProfilerPrintf("AppDomainShutdownFinished\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::AssemblyLoadStarted(AssemblyID assemblyId)
{
    GT_UNREFERENCED_PARAMETER(assemblyId);

#ifdef _CXLDEBUG
    // Assembly ID can't be used yet, so we can't grab the name...
    ProfilerPrintf("AssemblyLoadStarted\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus)
{
    GT_UNREFERENCED_PARAMETER(hrStatus);

    ScopedLock cSynch(m_criticalSection);

    // Now it's OK to grab the name
    wchar_t wszAssembly[512];
    AppDomainID appDomainId = 0;
    ULONG cchAssembly = sizeof(wszAssembly) / sizeof(wszAssembly[0]);

    m_pICorProfilerInfo->GetAssemblyInfo(assemblyId, cchAssembly,
                                         &cchAssembly, wszAssembly, &appDomainId, 0);

    gtUInt64 systime = GetSystemTime();
    m_pCelWriter->WriteAssemblyLoadFinished(assemblyId, appDomainId, systime,
                                            cchAssembly + 1, wszAssembly);

#ifdef _CXLDEBUG
    ProfilerPrintf("AssemblyLoadFinished (id=%d): %ls  Status: %08X\n", assemblyId, wszAssembly, hrStatus);
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::AssemblyUnloadStarted(AssemblyID assemblyId)
{
    ScopedLock cSynch(m_criticalSection);

    gtUInt64 systime = GetSystemTime();
    m_pCelWriter->WriteAssemblyUnloadStarted(assemblyId, systime);

#ifdef _CXLDEBUG
    wchar_t wszAssembly[512];
    ULONG cchAssembly = sizeof(wszAssembly) / sizeof(wszAssembly[0]);

    m_pICorProfilerInfo->GetAssemblyInfo(assemblyId, cchAssembly,
                                         &cchAssembly, wszAssembly, 0, 0);
    ProfilerPrintf("AssemblyUnloadStarted: %ls\n", wszAssembly);
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus)
{
    GT_UNREFERENCED_PARAMETER(assemblyId);
    GT_UNREFERENCED_PARAMETER(hrStatus);

#ifdef _CXLDEBUG
    ProfilerPrintf("AssemblyUnloadFinished\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ModuleLoadStarted(ModuleID moduleId)
{
    GT_UNREFERENCED_PARAMETER(moduleId);

#ifdef _CXLDEBUG
    // The moduleId can't be used for anything yet...
    ProfilerPrintf("ModuleLoadStarted\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)
{
    GT_UNREFERENCED_PARAMETER(hrStatus);

    ScopedLock cSynch(m_criticalSection);

    // Now the moduleId can be used to retrieve the module name
    ULONG cchModule;
    LPCBYTE t_loadAddress = 0;
    wchar_t t_moduleName[OS_MAX_PATH];

    m_pICorProfilerInfo->GetModuleInfo(moduleId,
                                       &t_loadAddress,
                                       OS_MAX_PATH,
                                       &cchModule,
                                       t_moduleName,
                                       NULL);

    gtUInt64 systime = GetSystemTime();
    m_pCelWriter->WriteModuleLoadFinished(moduleId, systime, cchModule + 1, t_moduleName);

#ifdef _CXLDEBUG
    ProfilerPrintf("ModuleLoadFinished: ID: %d, %ls\n", moduleId, t_moduleName);
#endif

    return S_OK;
}


HRESULT ClrProfCallBack::ModuleUnloadStarted(ModuleID moduleId)
{
    ScopedLock cSynch(m_criticalSection);

    gtUInt64 systime = GetSystemTime();
    m_pCelWriter->WriteModuleUnloadStarted(moduleId, systime);

#ifdef _CXLDEBUG
    // Now the moduleId can be used to retrieve the module name
    wchar_t wszModule[512];
    ULONG cchModule = sizeof(wszModule) / sizeof(wszModule[0]);
    m_pICorProfilerInfo->GetModuleInfo(moduleId, 0, cchModule, &cchModule, wszModule, 0);
    ProfilerPrintf("ModuleUnloadStarted: %ls\n", wszModule);
#endif

    return S_OK;
}

HRESULT ClrProfCallBack::ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus)
{
    GT_UNREFERENCED_PARAMETER(moduleId);
    GT_UNREFERENCED_PARAMETER(hrStatus);

#ifdef _CXLDEBUG
    ProfilerPrintf("ModuleUnloadFinished\n");
#endif
    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID assemblyId)
{
    ScopedLock cSynch(m_criticalSection);

#ifdef _CXLDEBUG
    wchar_t wszAssembly[512] = {'\0'};
    ULONG cchAssembly = sizeof(wszAssembly) / sizeof(wszAssembly[0]);

    m_pICorProfilerInfo->GetAssemblyInfo(assemblyId, cchAssembly,
                                         &cchAssembly, wszAssembly, 0, 0);
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    ProfilerPrintf("ModuleAttachedToAssembly: modID: %I64d, %ls\n", moduleId, wszAssembly);
#else
    ProfilerPrintf("ModuleAttachedToAssembly: modID: %u, assemblyId:%u %ls\n",
                   moduleId, assemblyId, wszAssembly);
#endif
#endif


    CheckNgenSymbols(moduleId, assemblyId);

    return S_OK;
}


HRESULT ClrProfCallBack::ClassLoadStarted(ClassID classId)
{
#ifdef _CXLDEBUG
    wchar_t wszClass[512];

    if (GetClassNameFromClassId(classId, wszClass))
    {
        ProfilerPrintf("ClassLoadStarted: %ls\n", wszClass);
    }
    else
    {
        ProfilerPrintf("ClassLoadStarted\n");
    }

#else
    GT_UNREFERENCED_PARAMETER(classId);
#endif

    return S_OK;
}
HRESULT ClrProfCallBack::ClassLoadFinished(ClassID classId, HRESULT hrStatus)
{
    GT_UNREFERENCED_PARAMETER(hrStatus);

    ScopedLock cSynch(m_criticalSection);

    wchar_t wszClass[512];
    ModuleID moduleId = 0;

    gtUInt64 systime = GetSystemTime();

    if (GetClassNameFromClassId(classId, wszClass, &moduleId))
    {
        m_pCelWriter->WriteClassLoadFinished(classId, moduleId, systime, lstrlenW(wszClass) + 1, wszClass);
    }

#ifdef _CXLDEBUG
    ProfilerPrintf("ClassLoadFinished\n");
#endif

    return S_OK;
}
HRESULT ClrProfCallBack::ClassUnloadStarted(ClassID classId)
{
    ScopedLock cSynch(m_criticalSection);

    gtUInt64 systime = GetSystemTime();
    m_pCelWriter->WriteClassUnloadStarted(classId, systime);

#ifdef _CXLDEBUG
    wchar_t wszClass[512];

    if (GetClassNameFromClassId(classId, wszClass))
    {
        ProfilerPrintf("ClassUnloadStarted: %ls\n", wszClass);
    }
    else
    {
        ProfilerPrintf("ClassUnloadStarted\n");
    }

#endif

    return S_OK;
}
HRESULT ClrProfCallBack::ClassUnloadFinished(ClassID classId, HRESULT hrStatus)
{
    GT_UNREFERENCED_PARAMETER(classId);
    GT_UNREFERENCED_PARAMETER(hrStatus);

#ifdef _CXLDEBUG
    ProfilerPrintf("ClassUnloadFinished\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::FunctionUnloadStarted(FunctionID functionId)
{
    ScopedLock cSynch(m_criticalSection);

    gtUInt64 systime = GetSystemTime();
    m_pCelWriter->WriteFunctionUnloadStarted(functionId, systime);

#ifdef _CXLDEBUG
    ProfilerPrintf("FunctionUnloadStarted\n");
#endif

    return S_OK;
}


HRESULT ClrProfCallBack::JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock)
{
#ifdef _CXLDEBUG
    wchar_t wszClass[512];
    wchar_t wszMethod[512];

    BOOL isStatic;
    ULONG argCount;
    WCHAR returnType[MAX_LENGTH];
    WCHAR functionName[MAX_LENGTH];
    WCHAR functionParameters[10 * MAX_LENGTH];
    WCHAR functionSignature[15 * MAX_LENGTH];

    returnType[0] = L'\0';
    functionName[0] = L'\0';
    functionParameters[0] = L'\0';
    functionSignature[0] = L'\0';

    // get the sig of the function and
    // use utilcode to get the parameters you want
    BasicHelper::GetFunctionProperties(m_pICorProfilerInfo,
                                       functionId,
                                       &isStatic,
                                       &argCount,
                                       returnType,
                                       (sizeof(returnType) / sizeof(WCHAR)),
                                       functionParameters,
                                       (sizeof(functionParameters) / sizeof(WCHAR)),
                                       functionName,
                                       (sizeof(functionName) / sizeof(WCHAR)));

    const size_t sigLen = (sizeof(functionSignature) / sizeof(WCHAR));

    _snwprintf(functionSignature, sigLen, L"%s%s (%s)",
               (isStatic ? L"static " : L""),
               returnType,
               functionParameters);

    functionSignature[sigLen - 1] = L'\0';

    ProfilerPrintf("JITCompilationStarted\n");
    ProfilerPrintf("funcName: %ls, signature: %ls\n", functionName, functionSignature);

    if (GetMethodNameFromFunctionId(functionId, wszClass, wszMethod))
    {
        ProfilerPrintf("JITCompilationStarted: %ls::%ls\n", wszClass, wszMethod);
    }
    else
    {
        ProfilerPrintf("JITCompilationStarted\n");
    }

#else

    GT_UNREFERENCED_PARAMETER(functionId);
    GT_UNREFERENCED_PARAMETER(fIsSafeToBlock);

#endif

    return E_NOTIMPL;
}


// This guy does the work of getting information about the function in question, like name, signature, etc...
HRESULT ClrProfCallBack::JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
    GT_UNREFERENCED_PARAMETER(hrStatus);
    GT_UNREFERENCED_PARAMETER(fIsSafeToBlock);

    ScopedLock cSynch(m_criticalSection);

#ifdef _CXLDEBUG
    ProfilerPrintf("JITCompilationFinished\n");
#endif

    HRESULT hr = E_FAIL;
    gtUInt64 systime = GetSystemTime();
    BOOL bIsStatic = FALSE;
    ULONG argCount = 0;
    wchar_t returnTypeStr[OS_MAX_PATH] = { 0 };
    wchar_t functionParameters[10 * OS_MAX_PATH] = { 0 };
    wchar_t functionName[OS_MAX_PATH] = L"UNKNOWNFUNC";
    wchar_t className[OS_MAX_PATH] = L"UNKNOWNCLASS";
    IMetaDataImport* pMDImport = NULL;
    // dummy flag to stop do-while loop after first iteration
    bool loopAgain = false;

    do
    {
        if (NULL == functionId)
        {
            break;
        }

        mdToken token;
        ClassID classID;
        ModuleID moduleId;

        // Get the classID
        hr = m_pICorProfilerInfo->GetFunctionInfo(functionId, &classID, &moduleId, NULL);

        if (!SUCCEEDED(hr))
        {
            break;
        }

        // Get the MetadataImport interface and the metadata token
        hr = m_pICorProfilerInfo->GetTokenAndMetaDataFromFunction(functionId, IID_IMetaDataImport, (IUnknown**)&pMDImport, &token);

        if (!SUCCEEDED(hr))
        {
            break;
        }

        hr = pMDImport->GetMethodProps(token, NULL, functionName, OS_MAX_PATH, 0, 0, NULL, NULL, NULL, NULL);

        if (!SUCCEEDED(hr))
        {
            break;
        }

        mdTypeDef classToken = NULL;

        hr = m_pICorProfilerInfo->GetClassIDInfo(classID, NULL, &classToken);

        if (!SUCCEEDED(hr))
        {
            break;
        }

        if (mdTypeDefNil == classToken)
        {
            break;
        }

        hr = pMDImport->GetTypeDefProps(classToken, className, MAX_LENGTH, NULL, NULL, NULL);

        DWORD methodAttr = 0;
        PCCOR_SIGNATURE sigBlob = NULL;

        hr = pMDImport->GetMethodProps((mdMethodDef) token, NULL, NULL, 0, NULL, &methodAttr, &sigBlob, NULL, NULL, NULL);

        if (!SUCCEEDED(hr))
        {
            break;
        }

        ULONG callConv;

        // Is the method static ?
        bIsStatic = (BOOL)((methodAttr & mdStatic) != 0);

        // Make sure we have a method signature.
        char buffer[2 * MAX_LENGTH];

        sigBlob += CorSigUncompressData(sigBlob, &callConv);

        if (callConv != IMAGE_CEE_CS_CALLCONV_FIELD)
        {
            static WCHAR* callConvNames[8] =
            {
                L"",
                L"unmanaged cdecl ",
                L"unmanaged stdcall ",
                L"unmanaged thiscall ",
                L"unmanaged fastcall ",
                L"vararg ",
                L"<error> "
                L"<error> "
            };
            buffer[0] = '\0';

            if ((callConv & 7) != 0)
            {
                sprintf(buffer, "%ws ", callConvNames[callConv & 7]);
            }

            // Grab the argument count
            sigBlob += CorSigUncompressData(sigBlob, &argCount);

            // Get the return type
            sigBlob = BasicHelper::ParseElementType(pMDImport, sigBlob, buffer);

            // if the return typ returned back empty, write void
            if (buffer[0] == '\0')
            {
                sprintf(buffer, "void");
            }

            swprintf(returnTypeStr, L"%S", buffer);

            // Get the parameters
            for (ULONG i = 0; (SUCCEEDED(hr) && (sigBlob != NULL) && (i < argCount)); i++)
            {
                buffer[0] = '\0';

                sigBlob = BasicHelper::ParseElementType(pMDImport, sigBlob, buffer);

                if (i == 0)
                {
                    swprintf(functionParameters, L"%S", buffer);
                }
                else if (sigBlob != NULL)
                {
                    swprintf(functionParameters, L"%s+%S", functionParameters, buffer);
                }
                else
                {
                    hr = E_FAIL;
                }
            }
        }
        else
        {
            // Get the return type
            buffer[0] = '\0';
            sigBlob = BasicHelper::ParseElementType(pMDImport, sigBlob, buffer);
            swprintf(returnTypeStr, L"%s %S", returnTypeStr, buffer);
        }

        // Now the moduleId can be used to retrieve the module name
        wchar_t wszModule[OS_MAX_PATH];
        ULONG cchModule = sizeof(wszModule) / sizeof(wszModule[0]);
        LPCBYTE moduleLoadAddr;
        m_pICorProfilerInfo->GetModuleInfo(moduleId, &moduleLoadAddr, cchModule, &cchModule, wszModule, 0);

        LPCBYTE startAddr = NULL;
        ULONG   codeSize = 0;

        JncWriter jncWriter;
        wchar_t jncFileName[FILENAME_MAX];

        if (S_OK == m_pICorProfilerInfo->GetCodeInfo(functionId, &startAddr, &codeSize))
        {
            swprintf(jncFileName, L"%ls\\JIT_code%llx.jnc", m_CLRJITDIR, reinterpret_cast<gtUInt64>(startAddr));

            m_pCelWriter->WriteJITCompilationFinished(moduleId, functionId, className,
                                                      functionName, jncFileName, systime, startAddr, codeSize);

            const gtUByte* pChar = startAddr;
            jncWriter.SetJNCFileName(jncFileName);
            jncWriter.SetJITStartAddr(reinterpret_cast<gtUInt64>(startAddr));

            LPCBYTE pMethodHeader = NULL;
            ULONG cbMethodSize = 0;

            jncWriter.SetJITFuncName(className, functionName);
            jncWriter.SetJITModuleName(wszModule);

            hr = m_pICorProfilerInfo->GetILFunctionBody(moduleId, token,
                                                        &pMethodHeader, &cbMethodSize);
#ifdef _CXLDEBUG
            ProfilerPrintf("IL Method Header and body\n");
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
            ProfilerPrintf("Module load address = %I64X, methodAddress = %I64X, offset= %I64X\n",
                           moduleLoadAddr, pMethodHeader, (pMethodHeader - moduleLoadAddr));
#else
            ProfilerPrintf("Module load address = %X, methodAddress = %X, offset= %X\n",
                           moduleLoadAddr, pMethodHeader, (pMethodHeader - moduleLoadAddr));
#endif
            ProfilerPrintfCodeBytes((LPCBYTE) pMethodHeader, cbMethodSize);
#endif

            if (SUCCEEDED(hr))
            {
                const gtUByte* pILCode = pMethodHeader;
                COR_DEBUG_IL_TO_NATIVE_MAP* pMap = NULL;
                pMap = new COR_DEBUG_IL_TO_NATIVE_MAP[cbMethodSize];
                ULONG32 mapEntryCount = 0;

                if (pMap)
                {
                    mapEntryCount = cbMethodSize;
                    hr = m_pICorProfilerInfo->GetILToNativeMapping(functionId, mapEntryCount, &mapEntryCount, pMap);

                    if (!SUCCEEDED(hr))
                    {
                        // failed, set map count to 0, JNCFile will not write offset mapping section.
                        mapEntryCount = 0;
#ifdef _CXLDEBUG
                        ProfilerPrintf("GetILToNativeMapping failed -- Error Code = 0x%x\n", hr);
#endif
                    }

#ifdef _CXLDEBUG
                    else
                    {
                        ProfilerPrintf("GetILToNativeMapping succeeded. cMap(%d). %S_%S\n",
                                       mapEntryCount, className, functionName);

                        for (unsigned int i = 0; i < mapEntryCount; i++)
                        {
                            ProfilerPrintf("%i: ILOffset:%d NativeOffset:[%d - %d]; \n",
                                           i, pMap[i].ilOffset, pMap[i].nativeStartOffset, pMap[i].nativeEndOffset);
                        }
                    }

#endif
                }

                jncWriter.WriteCLRJITNativeCode(pChar, codeSize,
                                                pILCode, cbMethodSize, (unsigned int)(pMethodHeader - moduleLoadAddr),
                                                (ULONG32*) pMap, mapEntryCount);

                if (pMap)
                {
                    delete [] pMap;
                    pMap = NULL;
                }
            }
            else
            {
                jncWriter.WriteJITNativeCode(pChar, codeSize);
            }


#ifdef _CXLDEBUG
            ProfilerPrintf("\tClass Name = %ls, FunctionName = %ls\n", className, functionName);

            ProfilerPrintfCodeBytes(startAddr, codeSize);
#endif
        }

        jncWriter.Close();
    }
    while (loopAgain);

    if (pMDImport)
    {
        pMDImport->Release();
        pMDImport = NULL;
    }

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::JITCachedFunctionSearchStarted(FunctionID functionId, BOOL* pbUseCachedFunction)
{
#ifdef _CXLDEBUG
    ScopedLock cSynch(m_criticalSection);

    wchar_t wszClass[512];
    wchar_t wszMethod[512];

    if (GetMethodNameFromFunctionId(functionId, wszClass, wszMethod))
    {
        ProfilerPrintf("JITCachedFunctionSearchStarted: %ls::%ls\n",
                       wszClass, wszMethod);
    }
    else
    {
        ProfilerPrintf("JITCachedFunctionSearchStarted\n");
    }

    // Force JIT'ting to occur
    *pbUseCachedFunction = FALSE;
#else
    GT_UNREFERENCED_PARAMETER(functionId);
    GT_UNREFERENCED_PARAMETER(pbUseCachedFunction);
#endif

    return S_OK;
}
HRESULT ClrProfCallBack::JITCachedFunctionSearchFinished(FunctionID functionId, COR_PRF_JIT_CACHE result)
{
    GT_UNREFERENCED_PARAMETER(functionId);
    GT_UNREFERENCED_PARAMETER(result);

#ifdef _CXLDEBUG
    ProfilerPrintf("JITCachedFunctionSearchFinished\n");
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::JITFunctionPitched(FunctionID functionId)
{
    GT_UNREFERENCED_PARAMETER(functionId);

#ifdef _CXLDEBUG
    ProfilerPrintf("JITFunctionPitched\n");
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::JITInlining(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline)
{
    GT_UNREFERENCED_PARAMETER(callerId);
    GT_UNREFERENCED_PARAMETER(calleeId);
    GT_UNREFERENCED_PARAMETER(pfShouldInline);

#ifdef _CXLDEBUG
    ProfilerPrintf("JITInlining\n");

    if (pfShouldInline == NULL)
    {
        return E_POINTER;
    }

#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ThreadCreated(ThreadID threadId)
{
    GT_UNREFERENCED_PARAMETER(threadId);

#ifdef _CXLDEBUG
    ProfilerPrintf("ThreadCreated\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ThreadDestroyed(ThreadID threadId)
{
    GT_UNREFERENCED_PARAMETER(threadId);

#ifdef _CXLDEBUG
    ProfilerPrintf("ThreadDestroyed\n");
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadID)
{
    GT_UNREFERENCED_PARAMETER(managedThreadId);
    GT_UNREFERENCED_PARAMETER(osThreadID);

#ifdef _CXLDEBUG
    ProfilerPrintf("ThreadAssignedToOSThread\n");
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::RemotingClientInvocationStarted()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("RemotingClientInvocationStarted\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::RemotingClientSendingMessage(GUID* pCookie, BOOL fIsAsync)
{
    GT_UNREFERENCED_PARAMETER(pCookie);
    GT_UNREFERENCED_PARAMETER(fIsAsync);

#ifdef _CXLDEBUG
    ProfilerPrintf("RemotingClientSendingMessage\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::RemotingClientReceivingReply(GUID* pCookie, BOOL fIsAsync)
{
    GT_UNREFERENCED_PARAMETER(pCookie);
    GT_UNREFERENCED_PARAMETER(fIsAsync);

#ifdef _CXLDEBUG
    ProfilerPrintf("RemotingClientReceivingReply\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::RemotingClientInvocationFinished()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("RemotingClientInvocationFinished\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync)
{
    GT_UNREFERENCED_PARAMETER(pCookie);
    GT_UNREFERENCED_PARAMETER(fIsAsync);

#ifdef _CXLDEBUG
    ProfilerPrintf("RemotingServerReceivingMessage\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::RemotingServerInvocationStarted()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("RemotingServerInvocationStarted\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::RemotingServerInvocationReturned()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("RemotingServerInvocationReturned\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::RemotingServerSendingReply(GUID* pCookie, BOOL fIsAsync)
{
    GT_UNREFERENCED_PARAMETER(pCookie);
    GT_UNREFERENCED_PARAMETER(fIsAsync);

#ifdef _CXLDEBUG
    ProfilerPrintf("RemotingServerSendingReply\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::UnmanagedToManagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
{
#ifdef _CXLDEBUG
    wchar_t wszClass[512];
    wchar_t wszMethod[512];

    if (GetMethodNameFromFunctionId(functionId, wszClass, wszMethod))
    {
        ProfilerPrintf("UnmanagedToManagedTransition: %ls::%ls\n", wszClass,
                       wszMethod);
    }
    else
    {
        ProfilerPrintf("UnmanagedToManagedTransition\n");
    }

#else
    GT_UNREFERENCED_PARAMETER(functionId);
    GT_UNREFERENCED_PARAMETER(reason);
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ManagedToUnmanagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
{
#ifdef _CXLDEBUG
    wchar_t wszClass[512];
    wchar_t wszMethod[512];

    if (GetMethodNameFromFunctionId(functionId, wszClass, wszMethod))
    {
        ProfilerPrintf("ManagedToUnmanagedTransition: %ls::%ls\n", wszClass,
                       wszMethod);
    }
    else
    {
        ProfilerPrintf("ManagedToUnmanagedTransition\n");
    }

#else
    GT_UNREFERENCED_PARAMETER(functionId);
    GT_UNREFERENCED_PARAMETER(reason);
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason)
{
#ifdef _CXLDEBUG
    char* pszSuspendReason = "???";

    switch (suspendReason)
    {
        case COR_PRF_SUSPEND_FOR_GC:
            pszSuspendReason = "SUSPEND_FOR_GC";
            break;

        case COR_PRF_SUSPEND_FOR_APPDOMAIN_SHUTDOWN:
            pszSuspendReason = "SUSPEND_FOR_APPDOMAIN_SHUTDOWN";
            break;

        case COR_PRF_SUSPEND_FOR_CODE_PITCHING:
            pszSuspendReason = "SUSPEND_FOR_CODE_PITCHING";
            break;

        case COR_PRF_SUSPEND_FOR_SHUTDOWN:
            pszSuspendReason = "SUSPEND_FOR_SHUTDOWN";
            break;

        case COR_PRF_SUSPEND_FOR_INPROC_DEBUGGER:
            pszSuspendReason = "SUSPEND_FOR_INPROC_DEBUGGER";
            break;

        case COR_PRF_SUSPEND_FOR_GC_PREP:
            pszSuspendReason = "SUSPEND_FOR_GC_PREP";
            break;
    }

    ProfilerPrintf("RuntimeSuspendStarted: %s\n", pszSuspendReason);
#else
    GT_UNREFERENCED_PARAMETER(suspendReason);
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::RuntimeSuspendFinished()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("RuntimeSuspendFinished\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::RuntimeSuspendAborted()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("RuntimeSuspendAborted\n");
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::RuntimeResumeStarted()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("RuntimeResumeStarted\n");
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::RuntimeResumeFinished()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("RuntimeResumeFinished\n");
#endif

    return E_NOTIMPL;
}


HRESULT ClrProfCallBack::RuntimeThreadSuspended(ThreadID threadId)
{
    GT_UNREFERENCED_PARAMETER(threadId);

#ifdef _CXLDEBUG
    ProfilerPrintf("RuntimeThreadSuspended\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::RuntimeThreadResumed(ThreadID threadId)
{
    GT_UNREFERENCED_PARAMETER(threadId);

#ifdef _CXLDEBUG
    ProfilerPrintf("RuntimeThreadResumed\n");
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::MovedReferences(ULONG cMovedObjectIDRanges,
                                         ObjectID oldObjectIDRangeStart[],
                                         ObjectID newObjectIDRangeStart[],
                                         ULONG cObjectIDRangeLength[])
{
    GT_UNREFERENCED_PARAMETER(cMovedObjectIDRanges);
    GT_UNREFERENCED_PARAMETER(oldObjectIDRangeStart);
    GT_UNREFERENCED_PARAMETER(newObjectIDRangeStart);
    GT_UNREFERENCED_PARAMETER(cObjectIDRangeLength);

#ifdef _CXLDEBUG
    ProfilerPrintf("MovedReferences\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ObjectAllocated(ObjectID objectId, ClassID classId)
{
    GT_UNREFERENCED_PARAMETER(objectId);
    GT_UNREFERENCED_PARAMETER(classId);

#ifdef _CXLDEBUG
    ProfilerPrintf("ObjectAllocated\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ObjectsAllocatedByClass(ULONG cClassCount, ClassID classIds[], ULONG cObjects[])
{
    GT_UNREFERENCED_PARAMETER(cClassCount);
    GT_UNREFERENCED_PARAMETER(classIds);
    GT_UNREFERENCED_PARAMETER(cObjects);

#ifdef _CXLDEBUG
    ProfilerPrintf("ObjectsAllocatedByClass\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ObjectReferences(ObjectID objectId, ClassID classId, ULONG cObjectRefs, ObjectID objectRefIds[])
{
    GT_UNREFERENCED_PARAMETER(objectId);
    GT_UNREFERENCED_PARAMETER(classId);
    GT_UNREFERENCED_PARAMETER(cObjectRefs);
    GT_UNREFERENCED_PARAMETER(objectRefIds);

#ifdef _CXLDEBUG
    wchar_t wszClass[512];

    if (GetClassNameFromClassId(classId, wszClass))
    {
        ProfilerPrintf("ObjectReferences: %ls refs: %u\n", wszClass, cObjectRefs);
    }
    else
    {
        ProfilerPrintf("ObjectReferences\n");
    }

#endif

    return S_OK;
}
HRESULT ClrProfCallBack::RootReferences(ULONG cRootRefs, ObjectID rootRefIds[])
{
    GT_UNREFERENCED_PARAMETER(cRootRefs);
    GT_UNREFERENCED_PARAMETER(rootRefIds);

#ifdef _CXLDEBUG
    ProfilerPrintf("RootReferences\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ExceptionThrown(ObjectID thrownObjectId)
{
#ifdef _CXLDEBUG
    ClassID classId;

    HRESULT hr = m_pICorProfilerInfo->GetClassFromObject(thrownObjectId, &classId);

    wchar_t wszClass[512];

    if (GetClassNameFromClassId(classId, wszClass))
    {
        ProfilerPrintf("ExceptionThrown: %ls\n", wszClass);
    }
    else
    {
        ProfilerPrintf("ExceptionThrown\n");
    }

#else
    GT_UNREFERENCED_PARAMETER(thrownObjectId);
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ExceptionSearchFunctionEnter(FunctionID functionId)
{
#ifdef _CXLDEBUG
    wchar_t wszClass[512];
    wchar_t wszMethod[512];

    GetMethodNameFromFunctionId(functionId, wszClass, wszMethod);

    ProfilerPrintf("ExceptionSearchFunctionEnter: %ls::%ls\n", wszClass, wszMethod);
#else
    GT_UNREFERENCED_PARAMETER(functionId);
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::ExceptionSearchFunctionLeave()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("ExceptionSearchFunctionLeave\n");
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::ExceptionSearchFilterEnter(FunctionID functionId)
{
#ifdef _CXLDEBUG
    wchar_t wszClass[512];
    wchar_t wszMethod[512];

    GetMethodNameFromFunctionId(functionId, wszClass, wszMethod);

    ProfilerPrintf("ExceptionSearchFilterEnter: %ls::%ls\n", wszClass, wszMethod);
#else
    GT_UNREFERENCED_PARAMETER(functionId);
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ExceptionSearchFilterLeave()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("ExceptionSearchFilterLeave\n");
#endif

    return E_NOTIMPL;
}


HRESULT ClrProfCallBack::ExceptionSearchCatcherFound(FunctionID functionId)
{
#ifdef _CXLDEBUG
    wchar_t wszClass[512];
    wchar_t wszMethod[512];

    GetMethodNameFromFunctionId(functionId, wszClass, wszMethod);

    ProfilerPrintf("ExceptionSearchCatcherFound: %ls::%ls\n", wszClass, wszMethod);
#else
    GT_UNREFERENCED_PARAMETER(functionId);
#endif

    return E_NOTIMPL;
}


HRESULT ClrProfCallBack::ExceptionOSHandlerEnter(FunctionID functionId)
{
#ifdef _CXLDEBUG
    wchar_t wszClass[512];
    wchar_t wszMethod[512];

    GetMethodNameFromFunctionId(functionId, wszClass, wszMethod);

    ProfilerPrintf("ExceptionOSHandlerEnter: %ls::%ls\n", wszClass, wszMethod);
#else
    GT_UNREFERENCED_PARAMETER(functionId);
#endif

    return E_NOTIMPL;
}


HRESULT ClrProfCallBack::ExceptionOSHandlerLeave(FunctionID functionId)
{
    GT_UNREFERENCED_PARAMETER(functionId);

#ifdef _CXLDEBUG
    ProfilerPrintf("ExceptionOSHandlerLeave\n");
#endif

    return E_NOTIMPL;
}


HRESULT ClrProfCallBack::ExceptionUnwindFunctionEnter(FunctionID functionId)
{
    GT_UNREFERENCED_PARAMETER(functionId);

#ifdef _CXLDEBUG
    wchar_t wszClass[512];
    wchar_t wszMethod[512];

    GetMethodNameFromFunctionId(functionId, wszClass, wszMethod);

    ProfilerPrintf("ExceptionUnwindFunctionEnter: %ls::%ls\n", wszClass, wszMethod);
#endif

    return E_NOTIMPL;
}


HRESULT ClrProfCallBack::ExceptionUnwindFunctionLeave()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("ExceptionUnwindFunctionLeave\n");
#endif

    return E_NOTIMPL;
}


HRESULT ClrProfCallBack::ExceptionUnwindFinallyEnter(FunctionID functionId)
{
    GT_UNREFERENCED_PARAMETER(functionId);

#ifdef _CXLDEBUG
    ProfilerPrintf("ExceptionUnwindFinallyEnter\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ExceptionUnwindFinallyLeave()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("ExceptionUnwindFinallyLeave\n");
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ExceptionCatcherEnter(FunctionID functionId, ObjectID objectId)
{
#ifdef _CXLDEBUG
    wchar_t wszClass[512];
    wchar_t wszMethod[512];

    GetMethodNameFromFunctionId(functionId, wszClass, wszMethod);

    ProfilerPrintf("ExceptionCatcherEnter: %ls::%ls\n", wszClass, wszMethod);
#else
    GT_UNREFERENCED_PARAMETER(functionId);
    GT_UNREFERENCED_PARAMETER(objectId);
#endif

    return E_NOTIMPL;
}
HRESULT ClrProfCallBack::ExceptionCatcherLeave()
{
#ifdef _CXLDEBUG
    ProfilerPrintf("ExceptionCatcherLeave\n");
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::COMClassicVTableCreated(ClassID wrappedClassId, REFGUID implementedIID,  VOID* pUnk, ULONG cSlots)
{
#ifdef _CXLDEBUG
    wchar_t wszClass[512];

    if (GetClassNameFromClassId(wrappedClassId, wszClass))
    {
        ProfilerPrintf(
            "COMClassicVTableCreated: %ls  vtable: %p  IID.Data1: %08X\n",
            wszClass, pUnk, implementedIID.Data1);
    }
    else
    {
        ProfilerPrintf("COMClassicVTableCreated\n");
    }

#else
    GT_UNREFERENCED_PARAMETER(wrappedClassId);
    GT_UNREFERENCED_PARAMETER(implementedIID);
    GT_UNREFERENCED_PARAMETER(pUnk);
    GT_UNREFERENCED_PARAMETER(cSlots);
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::COMClassicVTableDestroyed(ClassID wrappedClassId, REFGUID implementedIID, VOID* pUnk)
{
#ifdef _CXLDEBUG
    wchar_t wszClass[512];

    if (GetClassNameFromClassId(wrappedClassId, wszClass))
    {
        ProfilerPrintf("COMClassicVTableDestroyed: %ls  vtable: %p  IID.Data1: %08X\n", wszClass, pUnk, implementedIID.Data1);
    }
    else
    {
        ProfilerPrintf("COMClassicVTableDestroyed\n");
    }

#else
    GT_UNREFERENCED_PARAMETER(wrappedClassId);
    GT_UNREFERENCED_PARAMETER(implementedIID);
    GT_UNREFERENCED_PARAMETER(pUnk);
#endif

    return E_NOTIMPL;
}


HRESULT ClrProfCallBack::ExceptionCLRCatcherFound(void)
{
#ifdef _CXLDEBUG
    ProfilerPrintf("ExceptionCLRCatcherFound\n");
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::ExceptionCLRCatcherExecute(void)
{
#ifdef _CXLDEBUG
    ProfilerPrintf("ExceptionCLRCatcherExecute\n");
#endif

    return E_NOTIMPL;
}

HRESULT ClrProfCallBack::ThreadNameChanged(ThreadID threadId, ULONG cchName, WCHAR name[])
{
    GT_UNREFERENCED_PARAMETER(threadId);
    GT_UNREFERENCED_PARAMETER(cchName);
    GT_UNREFERENCED_PARAMETER(name);

    return S_OK;
}

HRESULT ClrProfCallBack::GarbageCollectionStarted(int cGenerations, BOOL generationCollected[ ], COR_PRF_GC_REASON reason)
{
    GT_UNREFERENCED_PARAMETER(cGenerations);
    GT_UNREFERENCED_PARAMETER(generationCollected);
    GT_UNREFERENCED_PARAMETER(reason);

    return S_OK;
}
HRESULT ClrProfCallBack::SurvivingReferences(
    /* [in] */ ULONG cSurvivingObjectIDRanges,
    /* [size_is][in] */ ObjectID objectIDRangeStart[  ],
    /* [size_is][in] */ ULONG cObjectIDRangeLength[  ])
{
    GT_UNREFERENCED_PARAMETER(cSurvivingObjectIDRanges);
    GT_UNREFERENCED_PARAMETER(objectIDRangeStart);
    GT_UNREFERENCED_PARAMETER(cObjectIDRangeLength);

    return S_OK;
}

HRESULT ClrProfCallBack::GarbageCollectionFinished(void)
{
    return S_OK;
}

HRESULT ClrProfCallBack::FinalizeableObjectQueued(
    /* [in] */ DWORD finalizerFlags,
    /* [in] */ ObjectID objectID)
{
    GT_UNREFERENCED_PARAMETER(finalizerFlags);
    GT_UNREFERENCED_PARAMETER(objectID);

    return S_OK;
}

HRESULT ClrProfCallBack::RootReferences2(
    /* [in] */ ULONG cRootRefs,
    /* [size_is][in] */ ObjectID rootRefIds[  ],
    /* [size_is][in] */ COR_PRF_GC_ROOT_KIND rootKinds[  ],
    /* [size_is][in] */ COR_PRF_GC_ROOT_FLAGS rootFlags[  ],
    /* [size_is][in] */ UINT_PTR rootIds[  ])
{
    GT_UNREFERENCED_PARAMETER(cRootRefs);
    GT_UNREFERENCED_PARAMETER(rootRefIds);
    GT_UNREFERENCED_PARAMETER(rootKinds);
    GT_UNREFERENCED_PARAMETER(rootFlags);
    GT_UNREFERENCED_PARAMETER(rootIds);

    return S_OK;
}

HRESULT ClrProfCallBack::HandleCreated(
    /* [in] */ GCHandleID handleId,
    /* [in] */ ObjectID initialObjectId)
{
    GT_UNREFERENCED_PARAMETER(handleId);
    GT_UNREFERENCED_PARAMETER(initialObjectId);

    return S_OK;
}

HRESULT ClrProfCallBack::HandleDestroyed(
    /* [in] */ GCHandleID handleId)
{
    GT_UNREFERENCED_PARAMETER(handleId);

    return S_OK;
}




//=============================================================================
// Helper functions

bool ClrProfCallBack::GetMethodNameFromFunctionId(FunctionID functionId, LPWSTR wszClass, LPWSTR wszMethod)
{
    mdToken dwToken;
    IMetaDataImport* pIMetaDataImport = 0;
    HRESULT hr = m_pICorProfilerInfo->GetTokenAndMetaDataFromFunction(functionId, IID_IMetaDataImport,
                                                                      (LPUNKNOWN*)&pIMetaDataImport, &dwToken);

    if (FAILED(hr)) { return false; }

    wchar_t _wszMethod[512];
    DWORD cchMethod = sizeof(_wszMethod) / sizeof(_wszMethod[0]);
    mdTypeDef mdClass;

    hr = pIMetaDataImport->GetMethodProps(dwToken, &mdClass, _wszMethod, cchMethod, &cchMethod, 0, 0, 0, 0, 0);

    if (FAILED(hr)) { return false; }

    lstrcpyW(wszMethod, _wszMethod);

    wchar_t wszTypeDef[512];
    DWORD cchTypeDef = sizeof(wszTypeDef) / sizeof(wszTypeDef[0]);

    if (mdClass == 0x02000000)
    {
        mdClass = 0x02000001;
    }

    hr = pIMetaDataImport->GetTypeDefProps(mdClass, wszTypeDef, cchTypeDef, &cchTypeDef, 0, 0);

    if (FAILED(hr)) { return false; }

    lstrcpyW(wszClass, wszTypeDef);

    pIMetaDataImport->Release();

    //
    // If we were ambitious, we'd save every FunctionID away in a map to avoid
    //  needing to hit the metatdata APIs every time.
    //
    return true;
}

bool ClrProfCallBack::GetClassNameFromClassId(ClassID classId, LPWSTR wszClass, ModuleID* pModuleId)
{
    ModuleID moduleId;
    mdTypeDef typeDef;

    wszClass[0] = 0;

    HRESULT hr = m_pICorProfilerInfo->GetClassIDInfo(classId, &moduleId, &typeDef);

    if (FAILED(hr))
    {
        return false;
    }

    if (typeDef == 0)   // ::GetClassIDInfo can fail, yet not set HRESULT
    {
        // __asm int 3
        return false;
    }

    IMetaDataImport* pIMetaDataImport = 0;
    hr = m_pICorProfilerInfo->GetModuleMetaData(moduleId, ofRead, IID_IMetaDataImport, (LPUNKNOWN*)&pIMetaDataImport);

    if (FAILED(hr))
    {
        return false;
    }

    if (!pIMetaDataImport)
    {
        return false;
    }

    wchar_t wszTypeDef[512];
    DWORD cchTypeDef = sizeof(wszTypeDef) / sizeof(wszTypeDef[0]);

    hr = pIMetaDataImport->GetTypeDefProps(typeDef, wszTypeDef, cchTypeDef, &cchTypeDef, 0, 0);

    if (FAILED(hr))
    {
        return false;
    }

    lstrcpyW(wszClass, wszTypeDef);

    // If we were ambitious, we'd save the ClassID away in a map to avoid
    //  needing to hit the metatdata APIs every time.

    pIMetaDataImport->Release();

    if (pModuleId)
    {
        *pModuleId = moduleId;
    }

    return true;
}


bool ClrProfCallBack::GetInititializationParameters()
{
    wchar_t szProfilerOptions[256];
    wchar_t tempDirName[FILENAME_MAX];
    HKEY temp_key;
    HRESULT hr;

    m_Initialized = false;

    DWORD tPid = GetCurrentProcessId();

    memset(tempDirName, 0, sizeof(tempDirName));
    GetTempPath(OS_MAX_PATH, tempDirName);

    swprintf(m_CLRJITDIR, L"%s%d", tempDirName, tPid);

    if (0 == RecursiveMakeDir(m_CLRJITDIR))
    {
        GetWindowsDirectory(m_WindowsDir, OS_MAX_PATH - 1);

        swprintf(m_szCELfileName, L"%s\\%d.cel", m_CLRJITDIR, tPid);
#ifdef _CXLDEBUG
        swprintf(m_szDebugOutfileName, L"%s\\%d.txt", m_CLRJITDIR, tPid);
#endif

        //@REM    COR_PRF_MONITOR_FUNCTION_UNLOADS  = 0x1,
        //@REM    COR_PRF_MONITOR_CLASS_LOADS       = 0x2,
        //@REM    COR_PRF_MONITOR_MODULE_LOADS      = 0x4,
        //@REM    COR_PRF_MONITOR_ASSEMBLY_LOADS    = 0x8,

        //@REM    COR_PRF_MONITOR_APPDOMAIN_LOADS   = 0x10,
        //@REM    COR_PRF_MONITOR_JIT_COMPILATION   = 0x20,
        //@REM    COR_PRF_MONITOR_EXCEPTIONS        = 0x40,
        //@REM    COR_PRF_MONITOR_GC                = 0x80,

        //@REM    COR_PRF_MONITOR_OBJECT_ALLOCATED  = 0x100,
        //@REM    COR_PRF_MONITOR_THREADS           = 0x200,
        //@REM    COR_PRF_MONITOR_REMOTING          = 0x400,
        //@REM    COR_PRF_MONITOR_CODE_TRANSITIONS  = 0x800,

        //@REM    COR_PRF_MONITOR_ENTERLEAVE        = 0x1000,
        //@REM    COR_PRF_MONITOR_CCW               = 0x2000,
        //@REM    COR_PRF_MONITOR_REMOTING_COOKIE   = 0x4000 | COR_PRF_MONITOR_REMOTING,
        //@REM    COR_PRF_MONITOR_REMOTING_ASYNC    = 0x8000 | COR_PRF_MONITOR_REMOTING,
        //@REM    COR_PRF_MONITOR_SUSPENDS          = 0x10000,
        //@REM    COR_PRF_MONITOR_CACHE_SEARCHES    = 0x20000,
        //@REM    COR_PRF_MONITOR_CLR_EXCEPTIONS    = 0x1000000,
        //@REM    COR_PRF_MONITOR_ALL               = 0x107ffff,
        //@REM    COR_PRF_ENABLE_REJIT              = 0x40000,
        //@REM    COR_PRF_ENABLE_INPROC_CXLDEBUGGING    = 0x80000,
        //@REM    COR_PRF_ENABLE_JIT_MAPS           = 0x100000,
        //@REM    COR_PRF_DISABLE_INLINING          = 0x200000,
        //@REM    COR_PRF_DISABLE_OPTIMIZATIONS     = 0x400000,
        //@REM    COR_PRF_ENABLE_OBJECT_ALLOCATED   = 0x800000,
        //@REM    COR_PRF_ALL                       = 0x1ffffff,

        // assign reasonable default values to the event mask
        m_dwEventMask = COR_PRF_MONITOR_CLASS_LOADS;
        m_dwEventMask += COR_PRF_MONITOR_MODULE_LOADS;
        m_dwEventMask += COR_PRF_MONITOR_ASSEMBLY_LOADS;
        m_dwEventMask += COR_PRF_MONITOR_FUNCTION_UNLOADS;
        m_dwEventMask += COR_PRF_MONITOR_APPDOMAIN_LOADS;
        m_dwEventMask += COR_PRF_MONITOR_JIT_COMPILATION;
        m_dwEventMask += COR_PRF_ENABLE_JIT_MAPS;

#ifdef _CXLDEBUG

        // This will be easy to change events for us during debug/testing
        if (GetEnvironmentVariable(L"CACLRPROF_MASK", szProfilerOptions, sizeof(szProfilerOptions)))
        {
            m_dwEventMask = wcstoul(szProfilerOptions, 0, 16);
        }

#endif

        m_EnumSymLevel = evUserLevel;

        if (GetEnvironmentVariable(L"CA_NGENSYMBOL_MASK", szProfilerOptions, sizeof(szProfilerOptions)))
        {
            m_EnumSymLevel = wcstoul(szProfilerOptions, 0, 16);
        }

        m_pCelWriter = new CelWriter;

        if (m_pCelWriter)
        {
            if (m_pCelWriter->Initialize(m_szCELfileName))
            {
                m_Initialized = true;
            }
        }
    }

    return m_Initialized;
}

int ClrProfCallBack::RecursiveMakeDir(wchar_t* dir)
{
    int ret = 0;
    wchar_t temp_dir[FILENAME_MAX];
    wchar_t* next_dir = dir;

    memset(temp_dir, 0, FILENAME_MAX);

    while ((next_dir = wcschr(next_dir, L'\\')) != NULL)
    {
        next_dir ++;
        wcsncpy(temp_dir, dir, (next_dir - dir));
        _wmkdir(temp_dir);
    }

    if (0 == ret)
    {
        ret = _wmkdir(dir);
    }

    return ret;
}

void ClrProfCallBack::QueryNgenSymbols(wchar_t* pNgenFileName)
{
#ifdef _CXLDEBUG
    ProfilerPrintf("native assembly: %ls\n", pNgenFileName);
#endif

    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(pNgenFileName))
    {
        return;
    }

    wchar_t shortFileName[_MAX_FNAME];
    wchar_t ext[_MAX_EXT];
    wchar_t wszClass[512];
    wchar_t wszMethod[512];
    char tSymbol[512];
    wchar_t tPJSFileName[OS_MAX_PATH];
    _wsplitpath(pNgenFileName, NULL, NULL, shortFileName, ext);
    swprintf(tPJSFileName, L"%s\\%s%s.pjs", m_CLRJITDIR, shortFileName, ext);
    PjsWriter tPJSfile(tPJSFileName);

    HMODULE hmodule = LoadLibrary(pNgenFileName);

    if (!hmodule)
    {
        return;
    }

    tPJSfile.WriteFileHeader(pNgenFileName, (gtVAddr) hmodule);

    ExecutableReader exeReader;

    if (exeReader.Open(pNgenFileName))
    {
        unsigned sectionCount = exeReader.GetSectionsCount();

        for (unsigned i = 0; i < sectionCount; i++)
        {
            if (0 != _stricmp(exeReader.GetSectionShortName(i), ".text"))
            {
                continue;
            }

            FunctionID lastFunId = 0;
            gtRVAddr secStart, secEnd, tmpAddr;
            exeReader.GetSectionRvaLimits(i, secStart, secEnd);
            tmpAddr = secStart;

            while (tmpAddr < secEnd)
            {
                LPCBYTE tAddress = (LPCBYTE)(hmodule) + tmpAddr;
                FunctionID tFuncId = 0;

                if (E_FAIL != m_pICorProfilerInfo->GetFunctionFromIP(tAddress, &tFuncId))
                {
                    if (lastFunId == tFuncId)
                    {
                        tmpAddr++;
                        continue;
                    }

                    lastFunId = tFuncId;

                    ClassID tempClass;
                    ModuleID tempMod;
                    mdToken tempToke;

                    m_pICorProfilerInfo->GetFunctionInfo(tFuncId, &tempClass, &tempMod, &tempToke);

                    if (0 == tempClass)
                    {
#ifdef _CXLDEBUG
                        ProfilerPrintf("Skipping unloaded function %I64d at IP=0x%I64x\n",
                                       tFuncId, tAddress);
#endif
                        tmpAddr++;
                        continue;
                    }

                    if (! GetMethodNameFromFunctionId(tFuncId, wszClass, wszMethod))
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
                        swprintf(wszMethod, L"UnknownFunction (ID %I64d)", (UINT_PTR) tFuncId);

#else
                        swprintf(wszMethod, 512, L"UnknownFunction (ID %d)", (UINT_PTR) tFuncId);
#endif
                    LPCBYTE startAddr = NULL;
                    ULONG   codeSize = 0;

                    if (S_OK == m_pICorProfilerInfo->GetCodeInfo(tFuncId, &startAddr, &codeSize))
                    {
                        if (tAddress < startAddr || tAddress > (startAddr + codeSize))
                        {
                            tmpAddr++;
                            continue;
                        }

                        sprintf(tSymbol, "%S", wszMethod);
                        tPJSfile.AddRecord((UINT_PTR) startAddr - (UINT_PTR) hmodule, codeSize, tSymbol);
#ifdef _CXLDEBUG
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
                        ProfilerPrintf("IP=0x%I64X, functionID = %I64d, name=%S::%S, start=0x%I64X size = %08x tmpAddr= %I64x\n",
                                       tAddress, tFuncId, wszClass, wszMethod, startAddr, codeSize, tmpAddr);
#else
                        ProfilerPrintf("IP=0x%X, functionID = %d, name=%S::%S, start=0x%X size = %08x\n",
                                       tAddress, tFuncId, wszClass, wszMethod, startAddr, codeSize);
#endif
#endif
                        tmpAddr += codeSize + 1;
                    }
                    else
                    {
                        tmpAddr++;
                    }
                }
                else
                {
                    tmpAddr++;
                }
            }
        }
    }

    FreeLibrary(hmodule);
}


HRESULT ClrProfCallBack::CheckNgenSymbols(ModuleID moduleId, AssemblyID assemblyId)
{
    // Check if we had get symbols for this Ngened assembly before.
    NgenAssemblyIDSet::iterator it = m_NgenAssemblySet.find((gtUInt64) assemblyId);

    if (it != m_NgenAssemblySet.end())
    {
        m_pCelWriter->WriteModuleAttachedToAssembly(moduleId, assemblyId, 0, NULL);
        return S_OK;
    }

    DWORD   cbNeeded;
    HANDLE  hProcess = NULL;
    HMODULE* phModArray = NULL;
    DWORD   modNum = 1024;
    wchar_t wszAssemblyName[512] = {'\0'};

    ULONG cchAssembly = sizeof(wszAssemblyName) / sizeof(wszAssemblyName[0]);

    m_pICorProfilerInfo->GetAssemblyInfo(assemblyId, cchAssembly,
                                         &cchAssembly, wszAssemblyName, 0, 0);

    // module attached to assembly
    // Module name: C:\Windows\assembly\GAC_32\mscorlib\2.0.0.0__b77a5c561934e089\mscorlib.dll
    // Assembly: mscorlib
    wchar_t wszModule[512];
    ULONG cchModule = sizeof(wszModule) / sizeof(wszModule[0]);
    m_pICorProfilerInfo->GetModuleInfo(moduleId, 0, cchModule, &cchModule, wszModule, 0);

    // allocate space for module info.
    // Since we don't know exact number of modules yet. Just allocate certain amount.
    // If it's not enough, we will reallocate again.

    phModArray = new HMODULE[modNum];

    hProcess = GetCurrentProcess();

    HRESULT hr = E_FAIL;

    if (NULL != hProcess && EnumProcessModules(hProcess, phModArray, sizeof(HMODULE) * modNum, &cbNeeded))
    {
        // get the module number in the process
        DWORD cModules = cbNeeded / sizeof(HMODULE);

        // determinte if we have enough space for the module handles
        if (cModules > modNum)
        {
            // if we don't have enough space, free the previous space
            delete [] phModArray;

            // re-allocate space for the module handles
            modNum = cModules;
            phModArray = new HMODULE[modNum];

            if (NULL == phModArray)
            {
                return E_FAIL;
            }
        }

        hr = S_OK;

        // enumerate process module
        if (EnumProcessModules(hProcess, phModArray, sizeof(HMODULE) * modNum, &cbNeeded))
        {
            cModules = cbNeeded / sizeof(HMODULE);

            wchar_t enumModName[OS_MAX_PATH];
            MODULEINFO modinfo;
            BOOL bFlag = FALSE;
            DWORD returncode = 0;

            // for each module in the process
            for (unsigned int j = 0; j < cModules; j++)
            {
                wchar_t searchStr[256];
                // get the module full name
                returncode = GetModuleFileNameEx(hProcess, phModArray[j], enumModName, sizeof(enumModName));

                // the enumerated module does not contain the assembly name
                if (!wcsstr(enumModName, (const wchar_t*) wszAssemblyName))
                {
                    continue;
                }

                if (m_b64bit)
                {
                    swprintf(searchStr, L"_64\\%s\\", wszAssemblyName);

                    if (!wcsstr(enumModName, searchStr))
                    {
                        continue;
                    }
                }
                else
                {
                    swprintf(searchStr, L"_32\\%s\\", wszAssemblyName);

                    if (!wcsstr(enumModName, searchStr))
                    {
                        continue;
                    }
                }

                swprintf(searchStr, L"\\%s.ni.", wszAssemblyName);

                if (!wcsstr(enumModName, searchStr))
                {
                    continue;
                }

                // now the enumModNam is the native assembly full path, wszMdoule is .NET binary file path;
                QueryNgenSymbols(enumModName);

                MODULEINFO modinfoInner;
                bFlag = GetModuleInformation(hProcess, phModArray[j], &modinfoInner, sizeof(MODULEINFO));

                if (!GetModuleInformation(hProcess, phModArray[j], &modinfoInner, sizeof(MODULEINFO)))
                {
                    modinfoInner.lpBaseOfDll = 0;
                }

                m_pCelWriter->WriteModuleAttachedToAssembly(moduleId, assemblyId,
                                                            (gtUInt64)modinfoInner.lpBaseOfDll, enumModName);

#ifdef _CXLDEBUG
                ProfilerPrintf("ModuleAttachedToAssembly: modID: %I64u, assemblyId:%I64u\n AssembyLoad:%I64x %ls\n",
                               (gtUInt64)moduleId, (gtUInt64)assemblyId, (gtUInt64)modinfoInner.lpBaseOfDll, enumModName);
#endif

                // store the assembly id to avoid duplicate symbol querying due to
                // multiple loads from same assembly (i.e. mscorlib).
                // -Lei 05/04/2010
                m_NgenAssemblySet.insert((gtUInt64) assemblyId);
                break;
            } // for each module
        }
    }

    if (NULL != phModArray)
    {
        delete [] phModArray;
    }

    return hr;
}


//**********************************************************
// The following section is used for debug only
//**********************************************************
#ifdef _CXLDEBUG

int ClrProfCallBack::ProfilerPrintf(char* pszFormat, ...)
{
    char szBuffer[1024];
    int retValue;
    va_list argptr;

    va_start(argptr, pszFormat);
    retValue = wvsprintfA(szBuffer, pszFormat, argptr);
    va_end(argptr);

    fputs(szBuffer, m_pDebugOutFile);
    fflush(m_pDebugOutFile);

    return retValue;
}


void ClrProfCallBack::ProfilerPrintfCodeBytes(LPCBYTE startAddr, ULONG cSize)
{
    fprintf(m_pDebugOutFile, "Code Bytes: at 0x%p, size (%0x)", startAddr, cSize);
    unsigned char* pChar = (unsigned char*) startAddr;

    for (unsigned int i = 0; i < cSize; i++)
    {
        if (i % 16 == 0)
        {
            fprintf(m_pDebugOutFile, "\n0x%p    ", startAddr + i);
        }

        fprintf(m_pDebugOutFile, " %02x", (unsigned char) *pChar);

        pChar++;
    }

    fprintf(m_pDebugOutFile, "\n");
    fflush(m_pDebugOutFile);
}


#endif // _CXLDEBUG
