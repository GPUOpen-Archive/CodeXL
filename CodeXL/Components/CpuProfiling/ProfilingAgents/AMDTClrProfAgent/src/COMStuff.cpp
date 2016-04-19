//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file COMStuff.cpp
///
//==================================================================================

#include <ClrProfCallBack.h>
#include <ClrProfAgent.h>

//=============================================================

HRESULT RegisterClassBase(const char* szID, const char* szDesc, const char* szProgID, const char* szIndepProgID, char* szOutCLSID);

HRESULT UnregisterClassBase(const char* szID, const char* szProgID, const char* szIndepProgID);

BOOL SetRegValue(char* szKeyName, char* szKeyword, char* szValue);

BOOL DeleteKey(const char* szKey, const char* szSubkey);

BOOL SetKeyAndValue(const char* szKey, const char* szSubkey, const char* szValue);

//=============================================================
#define ARRAY_SIZE( s ) (sizeof( s ) / sizeof( s[0] ))
#define MAX_LENGTH 256

#define REG_PROGRAM     "CodeXL"
#define REG_COMPONENT   "CLRProf"

#define COM_METHOD( TYPE ) TYPE STDMETHODCALLTYPE

HINSTANCE g_hInst;        // instance handle to this piece of code

//==========================================================

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    GT_UNREFERENCED_PARAMETER(lpReserved);

    // save off the instance handle for later use
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hInstance);
            g_hInst = hInstance;
            break;
    }

    return TRUE;
}

//================================================================

class CClassFactory : public IClassFactory
{
public:
    CClassFactory() { m_refCount = 1; }

    COM_METHOD(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_refCount);
    }
    COM_METHOD(ULONG) Release()
    {
        return InterlockedDecrement(&m_refCount);
    }

    COM_METHOD(HRESULT) QueryInterface(REFIID riid, void** ppInterface);

    // IClassFactory methods
    COM_METHOD(HRESULT) LockServer(BOOL fLock) { GT_UNREFERENCED_PARAMETER(fLock); return S_OK; }
    COM_METHOD(HRESULT) CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppInterface);

private:

    long m_refCount;
};

CClassFactory g_ProfilerClassFactory;

//================================================================

STDAPI DllUnregisterServer()
{
    const char* rcIndProgID = REG_PROGRAM "." REG_COMPONENT;
    char rcProgID[MAX_LENGTH];  // rcIndProgID.iVersion

    // format the ProgID value.
    sprintf(rcProgID, "%s.%d", rcIndProgID, 1);

    UnregisterClassBase("{" CLSID_AMDTClrProfAgent "}", rcProgID, rcIndProgID);

    return S_OK;
}

//================================================================
STDAPI DllRegisterServer()
{
    HRESULT hr = S_OK;
    char  szModule[OS_MAX_PATH];

    DllUnregisterServer();
    GetModuleFileNameA(g_hInst, szModule, ARRAY_SIZE(szModule));

    const char* rcIndProgID = REG_PROGRAM "." REG_COMPONENT;
    char rcProgID[MAX_LENGTH];          // rcIndProgID.iVersion
    char rcInproc[MAX_LENGTH + 2];      // CLSID\\InprocServer32
    char rcCLSID[MAX_LENGTH];           // CLSID\\szID


    // format the ProgID value.
    sprintf(rcProgID, "%s.%d", rcIndProgID, 1);

    // do the initial portion.
    hr = RegisterClassBase("{" CLSID_AMDTClrProfAgent "}",
                           "CodeXL CLR Profiler", rcProgID, rcIndProgID, rcCLSID);

    if (SUCCEEDED(hr))
    {
        // set the server path.
        SetKeyAndValue(rcCLSID, "InprocServer32", szModule);

        // add the threading model information.
        sprintf(rcInproc, "%s\\%s", rcCLSID, "InprocServer32");
        SetRegValue(rcInproc, "ThreadingModel", "Both");
    }
    else
    {
        DllUnregisterServer();
    }

    return hr;
}

//================================================================
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (rclsid == __uuidof(AMDTClrProfAgent))
    {
        hr = g_ProfilerClassFactory.QueryInterface(riid, ppv);
    }

    return hr;
}

//===========================================================

HRESULT CClassFactory::QueryInterface(REFIID riid, void** ppInterface)
{
    if (riid == IID_IUnknown)
    {
        *ppInterface = static_cast<IUnknown*>(this);
    }
    else if (riid == IID_IClassFactory)
    {
        *ppInterface = static_cast<IClassFactory*>(this);
    }
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppInterface)->AddRef();

    return S_OK;
}

//===========================================================
HRESULT CClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppInstance)
{
    GT_UNREFERENCED_PARAMETER(riid);

    // aggregation is not supported by these objects
    if (pUnkOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    ClrProfCallBack* pProfCallback = new ClrProfCallBack;

    *ppInstance = (void*)pProfCallback;

    return S_OK;
}

//===========================================================
HRESULT RegisterClassBase(const char* szID, const char* szDesc, const char* szProgID, const char* szIndepProgID, char* szOutCLSID)
{
    char szUserBase[MAX_LENGTH];

    strcpy(szOutCLSID, "Software\\Classes\\CLSID\\");
    strcat(szOutCLSID, szID);

    // create ProgID keys.
    strcpy(szUserBase, "Software\\Classes\\");
    strcat(szUserBase, szProgID);
    SetKeyAndValue(szUserBase, NULL, szDesc);
    SetKeyAndValue(szUserBase, "CLSID", szID);

    // create VersionIndependentProgID keys.
    strcpy(szUserBase, "Software\\Classes\\");
    strcat(szUserBase, szIndepProgID);
    SetKeyAndValue(szUserBase, NULL, szDesc);
    SetKeyAndValue(szUserBase, "CurVer", szProgID);
    SetKeyAndValue(szUserBase, "CLSID", szID);

    // create entries under CLSID.
    SetKeyAndValue(szOutCLSID, NULL, szDesc);
    SetKeyAndValue(szOutCLSID, "ProgID", szProgID);
    SetKeyAndValue(szOutCLSID, "VersionIndependentProgID", szIndepProgID);
    SetKeyAndValue(szOutCLSID, "NotInsertable", NULL);

    return S_OK;
}

//===========================================================
HRESULT UnregisterClassBase(const char* szID, const char* szProgID, const char* szIndepProgID)
{
    char szCLSID[MAX_LENGTH];
    char szUserBase[MAX_LENGTH];

    strcpy(szCLSID, "Software\\Classes\\CLSID\\");
    strcat(szCLSID, szID);

    HKEY hk = NULL;
    REGSAM  samDesired = KEY_QUERY_VALUE;
    BOOL isSys64;

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    isSys64 = TRUE;
    samDesired |= KEY_WOW64_32KEY;
#else
    isSys64 = FALSE;
    IsWow64Process(GetCurrentProcess(), &isSys64);
    samDesired |= KEY_WOW64_64KEY;
#endif

    if (FALSE != isSys64 && RegOpenKeyExA(HKEY_LOCAL_MACHINE, szCLSID, 0, samDesired, &hk) == ERROR_SUCCESS)
    {
        RegCloseKey(hk);
    }
    else
    {
        // delete the prog ID settings.
        strcpy(szUserBase, "Software\\Classes\\");
        strcat(szUserBase, szProgID);
        RegDeleteTreeA(HKEY_LOCAL_MACHINE, szUserBase);

        // delete the version independent prog ID settings.
        strcpy(szUserBase, "Software\\Classes\\");
        strcat(szUserBase, szIndepProgID);
        RegDeleteTreeA(HKEY_LOCAL_MACHINE, szUserBase);
    }

    // delete the class ID settings.
    RegDeleteTreeA(HKEY_LOCAL_MACHINE, szCLSID);

    return S_OK;
}

//===========================================================

BOOL DeleteKey(const char* szKey, const char* szSubkey)
{
    char rcKey[MAX_LENGTH]; // buffer for the full key name.


    // init the key with the base key name.
    strcpy(rcKey, szKey);

    // append the subkey name (if there is one).
    if (szSubkey != NULL)
    {
        strcat(rcKey, "\\");
        strcat(rcKey, szSubkey);
    }

    // delete the registration key.
    RegDeleteKeyA(HKEY_LOCAL_MACHINE, rcKey);

    return TRUE;
}

//===========================================================

BOOL SetKeyAndValue(const char* szKey, const char* szSubkey, const char* szValue)
{
    HKEY hKey;              // handle to the new reg key.
    char rcKey[MAX_LENGTH]; // buffer for the full key name.

    // init the key with the base key name.
    strcpy(rcKey, szKey);

    // append the subkey name (if there is one).
    if (szSubkey != NULL)
    {
        strcat(rcKey, "\\");
        strcat(rcKey, szSubkey);
    }

    // create the registration key.
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                        rcKey,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        NULL) == ERROR_SUCCESS)
    {
        // set the value (if there is one).
        if (szValue != NULL)
        {
            RegSetValueExA(hKey, NULL, 0, REG_SZ, (BYTE*)szValue,
                           (DWORD)(((strlen(szValue) + 1) * sizeof(char))));
        }

        RegCloseKey(hKey);

        return TRUE;
    }

    return FALSE;
}

BOOL SetRegValue(char* szKeyName, char* szKeyword, char* szValue)
{
    HKEY hKey; // handle to the new reg key.

    // create the registration key.
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                        szKeyName, 0,  NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL, &hKey,
                        NULL) == ERROR_SUCCESS)
    {
        // set the value (if there is one).
        if (szValue != NULL)
        {
            RegSetValueExA(hKey, szKeyword, 0, REG_SZ,
                           (BYTE*)szValue,
                           (DWORD)((strlen(szValue) + 1) * sizeof(char)));
        }

        RegCloseKey(hKey);

        return TRUE;
    }

    return FALSE;
}
