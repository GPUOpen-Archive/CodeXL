//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osApplication.cpp
///
//=====================================================================

//------------------------------ osApplication.cpp ------------------------------

// Local:
#include <AMDTOSWrappers/Include/osWin32Functions.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTBaseTools/Include/gtAssert.h>

#define INITGUID
#include <ShObjIdl.h>
#include <devpkey.h>
#include <propkey.h>
#include <propvarutil.h>
#pragma comment(lib, "propsys")
#pragma comment(lib, "version")

#if !defined(E_PROP_ID_UNSUPPORTED)
    //
    // MessageId: E_PROP_ID_UNSUPPORTED
    //
    // MessageText:
    //
    // The specified property ID is not supported for the specified property set.%0
    //
    #define E_PROP_ID_UNSUPPORTED            ((HRESULT)0x80070490L)
#endif

#ifdef _DEBUG
    #define AMDTWINCOMPONENTEXTUTILS_FILE_NAME  L"AMDTWinComponentExtUtils-d"
#else
    #define AMDTWINCOMPONENTEXTUTILS_FILE_NAME  L"AMDTWinComponentExtUtils"
#endif // DEBUG

DEFINE_PROPERTYKEY(PKEY_AppUserModel_HostEnvironment, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 14);
DEFINE_PROPERTYKEY(PKEY_AppUserModel_PackageInstallPath, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 15);
DEFINE_PROPERTYKEY(PKEY_AppUserModel_PackageFamilyName, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 17);
DEFINE_PROPERTYKEY(PKEY_AppUserModel_ParentID, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 19);
DEFINE_PROPERTYKEY(PKEY_AppUserModel_PackageFullName, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 21);

DEFINE_PROPERTYKEY(PKEY_Tile_SmallLogoPath, 0x86D40B4D, 0x9069, 0x443C, 0x81, 0x9A, 0x2A, 0x54, 0x09, 0x0D, 0xCC, 0xEC, 2);
DEFINE_PROPERTYKEY(PKEY_Tile_Background, 0x86D40B4D, 0x9069, 0x443C, 0x81, 0x9A, 0x2A, 0x54, 0x09, 0x0D, 0xCC, 0xEC, 4);
DEFINE_PROPERTYKEY(PKEY_Tile_LongDisplayName, 0x86D40B4D, 0x9069, 0x443C, 0x81, 0x9A, 0x2A, 0x54, 0x09, 0x0D, 0xCC, 0xEC, 11);

class DECLSPEC_UUID("DBCE7E40-7345-439D-B12C-114A11819A09") MrtResourceManager;
class DECLSPEC_UUID("660B90C8-73A9-4B58-8CAE-355B7F55341B") StartMenuCacheAndAppResolver;

enum START_MENU_APP_ITEMS_FLAGS
{
    SMAIF_DEFAULT = 0,
    SMAIF_EXTENDED = 1,
    SMAIF_USAGEINFO = 2
};

enum TILE_THEME_SELECTOR
{
    TILE_THEME_DEFAULT = 0,
    TILE_THEME_HIGH_CONTRAST_BLACK = 1,
    TILE_THEME_HIGH_CONTRAST_WHITE = 2,
    TILE_THEME_END = 3
};

enum RESOURCE_LAYOUT_DIRECTION
{
    RES_LAYOUTDIR_LTR = 0,
    RES_LAYOUTDIR_RTL = 1,
    RES_LAYOUTDIR_TTBLTR = 2,
    RES_LAYOUTDIR_TTBRTL = 3
};

enum RESOURCE_SCALE
{
    RES_SCALE_100_PERCENT = 0,
    RES_SCALE_140_PERCENT = 1,
    RES_SCALE_180_PERCENT = 2,
    RES_SCALE_80_PERCENT = 3
};

enum RESOURCE_CONTRAST
{
    RES_CONTRAST_STANDARD = 0,
    RES_CONTRAST_HIGH = 1,
    RES_CONTRAST_BLACK = 2,
    RES_CONTRAST_WHITE = 3
};

enum APPX_PACKAGE_ARCHITECTURE
{
    APPX_PACKAGE_ARCHITECTURE_X86 = 0,
    APPX_PACKAGE_ARCHITECTURE_ARM = 5,
    APPX_PACKAGE_ARCHITECTURE_X64 = 9,
    APPX_PACKAGE_ARCHITECTURE_NEUTRAL = 11
};


MIDL_INTERFACE("5da89bf4-3773-46be-b650-7e744863b7e8")
IAppxManifestApplication : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetStringValue(
        /* [string][in] */ __RPC__in_string LPCWSTR name,
        /* [retval][string][out] */ __RPC__deref_out_opt_string LPWSTR * value) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetAppUserModelId(
        /* [retval][string][out] */ __RPC__deref_out_opt_string LPWSTR * appUserModelId) = 0;
};

MIDL_INTERFACE("9eb8a55a-f04b-4d0d-808d-686185d4847a")
IAppxManifestApplicationsEnumerator : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetCurrent(
        /* [retval][out] */ __RPC__deref_out_opt IAppxManifestApplication** application) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(
        /* [retval][out] */ __RPC__out BOOL * hasCurrent) = 0;

    virtual HRESULT STDMETHODCALLTYPE MoveNext(
        /* [retval][out] */ __RPC__out BOOL * hasNext) = 0;
};

MIDL_INTERFACE("283ce2d7-7153-4a91-9649-7a0f7240945f")
IAppxManifestPackageId : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetName(
        /* [retval][string][out] */ __RPC__deref_out_opt_string LPWSTR * name) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetArchitecture(
        /* [retval][out] */ __RPC__out APPX_PACKAGE_ARCHITECTURE * architecture) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetPublisher(
        /* [retval][string][out] */ __RPC__deref_out_opt_string LPWSTR * publisher) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetVersion(
        /* [retval][out] */ __RPC__out UINT64 * packageVersion) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetResourceId(
        /* [retval][string][out] */ __RPC__deref_out_opt_string LPWSTR * resourceId) = 0;

    virtual HRESULT STDMETHODCALLTYPE ComparePublisher(
        /* [string][in] */ __RPC__in_string LPCWSTR other,
        /* [retval][out] */ __RPC__out BOOL * isSame) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetPackageFullName(
        /* [retval][string][out] */ __RPC__deref_out_opt_string LPWSTR * packageFullName) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetPackageFamilyName(
        /* [retval][string][out] */ __RPC__deref_out_opt_string LPWSTR * packageFamilyName) = 0;
};

MIDL_INTERFACE("4e1bd148-55a0-4480-a3d1-15544710637c")
IAppxManifestReader : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetPackageId(
        /* [retval][out] */ __RPC__deref_out_opt IAppxManifestPackageId** packageId) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetProperties(
        /* [retval][out] */ __RPC__deref_out_opt class IAppxManifestProperties** packageProperties) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetPackageDependencies(
        /* [retval][out] */ __RPC__deref_out_opt class IAppxManifestPackageDependenciesEnumerator** dependencies) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetCapabilities(
        /* [retval][out] */ __RPC__out enum APPX_CAPABILITIES * capabilities) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetResources(
        /* [retval][out] */ __RPC__deref_out_opt class IAppxManifestResourcesEnumerator** resources) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetDeviceCapabilities(
        /* [retval][out] */ __RPC__deref_out_opt class IAppxManifestDeviceCapabilitiesEnumerator** deviceCapabilities) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetPrerequisite(
        /* [string][in] */ __RPC__in_string LPCWSTR name,
        /* [retval][out] */ __RPC__out UINT64 * value) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetApplications(
        /* [retval][out] */ __RPC__deref_out_opt IAppxManifestApplicationsEnumerator** applications) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetStream(
        /* [retval][out] */ __RPC__deref_out_opt IStream** manifestStream) = 0;
};

class DECLSPEC_UUID("5842a140-ff9f-4166-8f5c-62f5b7b0c781") AppxFactory;
MIDL_INTERFACE("beb94909-e451-438b-b5a7-d79e767b75d8")
IAppxFactory : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE CreatePackageWriter(
        /* [in] */ __RPC__in_opt IStream * outputStream,
        /* [in] */ __RPC__in struct APPX_PACKAGE_SETTINGS * settings,
        /* [retval][out] */ __RPC__deref_out_opt class IAppxPackageWriter** packageWriter) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreatePackageReader(
        /* [in] */ __RPC__in_opt IStream * inputStream,
        /* [retval][out] */ __RPC__deref_out_opt class IAppxPackageReader** packageReader) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateManifestReader(
        /* [in] */ __RPC__in_opt IStream * inputStream,
        /* [retval][out] */ __RPC__deref_out_opt IAppxManifestReader** manifestReader) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateBlockMapReader(
        /* [in] */ __RPC__in_opt IStream * inputStream,
        /* [retval][out] */ __RPC__deref_out_opt class IAppxBlockMapReader** blockMapReader) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateValidatedBlockMapReader(
        /* [in] */ __RPC__in_opt IStream * blockMapStream,
        /* [in] */ __RPC__in LPCWSTR signatureFileName,
        /* [retval][out] */ __RPC__deref_out_opt class IAppxBlockMapReader** blockMapReader) = 0;
};


MIDL_INTERFACE("33F71155-C2E9-4FFE-9786-A32D98577CFF")
IStartMenuAppItems62 : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE EnumItems(START_MENU_APP_ITEMS_FLAGS, const GUID&, void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetItem(START_MENU_APP_ITEMS_FLAGS, LPCWSTR, const GUID&, void**) = 0;
};

MIDL_INTERFACE("02C5CCF3-805F-4654-A7B7-340A74335365")
IStartMenuAppItems63 : public IStartMenuAppItems62
{
};

MIDL_INTERFACE("130A2F65-2BE7-4309-9A58-A9052FF2B61C")
IMrtResourceManager : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Initialize() = 0;
    virtual HRESULT STDMETHODCALLTYPE InitializeForCurrentApplication() = 0;
    virtual HRESULT STDMETHODCALLTYPE InitializeForPackage(LPCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE InitializeForFile(LPCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMainResourceMap(const GUID&, void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetResourceMap(LPCWSTR, const GUID&, void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDefaultContext(const GUID&, void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetReference(const GUID&, void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsResourceReference(LPCWSTR, BOOL*) = 0;
};



MIDL_INTERFACE("E3C22B30-8502-4B2F-9133-559674587E51")
IResourceContext : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetLanguage(LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetHomeRegion(LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLayoutDirection(RESOURCE_LAYOUT_DIRECTION*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetTargetSize(USHORT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetScale(RESOURCE_SCALE*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetContrast(RESOURCE_CONTRAST*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAlternateForm(LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetQualifierValue(LPCWSTR, LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetLanguage(LPCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetHomeRegion(LPCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetLayoutDirection(RESOURCE_LAYOUT_DIRECTION) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetTargetSize(USHORT) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetScale(RESOURCE_SCALE) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetContrast(RESOURCE_CONTRAST) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetAlternateForm(LPCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetQualifierValue(LPCWSTR, LPCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE TrySetQualifierValue(LPCWSTR, LPCWSTR, HRESULT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE Reset() = 0;
    virtual HRESULT STDMETHODCALLTYPE ResetQualifierValue(LPCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE Clone(IResourceContext**) = 0;
    virtual HRESULT STDMETHODCALLTYPE OverrideToMatch(class IResourceCandidate*) = 0;
};

MIDL_INTERFACE("6E21E72B-B9B0-42AE-A686-983CF784EDCD")
IResourceMap : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetUri(LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSubtree(LPCWSTR*, IResourceMap**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetString(LPCWSTR, LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetStringForContext(IResourceContext*, LPCWSTR, LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFilePath(LPCWSTR, LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFilePathForContext(IResourceContext*, LPCWSTR, LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetNamedResourceCount(unsigned int*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetNamedResourceUri(unsigned int, LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetNamedResource(LPCWSTR, const GUID&, void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFullyQualifiedReference(LPCWSTR, LPCWSTR, LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFilePathByUri(IUri*, LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFilePathForContextByUri(IResourceContext*, IUri*, LPWSTR*) = 0;
};

#if (_MSC_VER < 1600)

typedef enum ACTIVATEOPTIONS
{
    AO_NONE = 0,
    AO_DESIGNMODE   = 0x1,
    AO_NOERRORUI    = 0x2,
    AO_NOSPLASHSCREEN   = 0x4
} ACTIVATEOPTIONS;

class DECLSPEC_UUID("45BA127D-10A8-46EA-8AB7-56EA9078943C") ApplicationActivationManager;

MIDL_INTERFACE("2e941141-7f97-4756-ba1d-9decde894a3d")
IApplicationActivationManager : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE ActivateApplication(
        /* [in] */ __RPC__in LPCWSTR appUserModelId,
        /* [unique][in] */ __RPC__in_opt LPCWSTR arguments,
        /* [in] */ ACTIVATEOPTIONS options,
        /* [out] */ __RPC__out DWORD * processId) = 0;

    virtual HRESULT STDMETHODCALLTYPE ActivateForFile(
        /* [in] */ __RPC__in LPCWSTR appUserModelId,
        /* [in] */ __RPC__in_opt IShellItemArray * itemArray,
        /* [unique][in] */ __RPC__in_opt LPCWSTR verb,
        /* [out] */ __RPC__out DWORD * processId) = 0;

    virtual HRESULT STDMETHODCALLTYPE ActivateForProtocol(
        /* [in] */ __RPC__in LPCWSTR appUserModelId,
        /* [in] */ __RPC__in_opt IShellItemArray * itemArray,
        /* [out] */ __RPC__out DWORD * processId) = 0;
};

#endif // (_MSC_VER < 1600)

static HRESULT IPropertyStore_GetString(IPropertyStore* pPropStore, const PROPERTYKEY& key, wchar_t** ppStr);
static const wchar_t* FindNonMrtSubstring(const wchar_t* pPath);
static APPX_PACKAGE_ARCHITECTURE ExtractPackageArchitecture(const wchar_t* pPackagePath);
static bool ExtractPackageApplicationExecutable(const wchar_t* pPackagePath, const wchar_t* pUserModelId, gtString& exeFullPath);

osExecutedApplicationType g_theExecutedApplicationType = OS_STANDALONE_APPLICATION_TYPE;

// ---------------------------------------------------------------------------
// Name:        osGetCurrentApplicationPath
// Description: Returns the current application path (exe full path).
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/6/2004
// Implementation notes:
//  We ask the OS for the command line that started this process. The application path
//  is the first command line argument in this command line.
// ---------------------------------------------------------------------------
bool osGetCurrentApplicationPath(osFilePath& applicationPath, bool convertToLower)
{
    bool retVal = false;

    // Get the current application module file name:
    wchar_t buff[MAX_PATH + 1];
    HMODULE  hApplicationModule = GetModuleHandle(NULL);
    DWORD rc = GetModuleFileName(hApplicationModule, buff, MAX_PATH + 1);

    if (rc != 0)
    {
        gtString applicationPathAsString = buff;

        if (convertToLower)
        {
            applicationPathAsString.toLowerCase();
        }

        applicationPath = applicationPathAsString;
        retVal = true;
    }

    // We didn't manage to use GetModuleFileNameExA - Use the command line instead:
    if (!retVal)
    {
        // Get the current process command line:
        gtString win32ProcessCommandLine = GetCommandLine();

        // Will get a "clean" process command line:
        gtString processCommandLine;

        // If the command line is surrounded by commas ('):
        if (win32ProcessCommandLine[0] == '\"')
        {
            // Remove the surrounding commas:
            int originalLength = win32ProcessCommandLine.length();
            win32ProcessCommandLine.getSubString(1, originalLength - 2, processCommandLine);
        }
        else
        {
            processCommandLine = win32ProcessCommandLine;
        }

        // Look for the first appearance of ".exe":
        int firstExeExtensionPosition = processCommandLine.find(L".exe");

        // If we found a space - the first command line argument is the application path:
        if (firstExeExtensionPosition != -1)
        {
            // Get the application path:
            gtString applicationPathString;
            processCommandLine.getSubString(0, firstExeExtensionPosition + 3, applicationPathString);

            // Output it:
            applicationPath = applicationPathString;
            retVal = true;
        }
    }

    return retVal;
}


bool osSupportWindowsStoreApps()
{
    osWindowsVersion winVer;
    return osGetWindowsVersion(winVer) && winVer >= OS_WIN_8;
}

bool osLaunchSuspendedWindowsStoreApp(const gtString& userModelId,
                                      const gtString& arguments,
                                      osProcessId& processId,
                                      osProcessHandle& processHandle,
                                      osFilePath& executablePath)
{
    bool retVal = false;

    if (osSupportWindowsStoreApps())
    {
        processId = 0;
        typedef LONG(NTAPI * PfnNtSuspendProcess)(IN HANDLE hProcess);
        static PfnNtSuspendProcess pfnNtSuspendProcess = NULL;

        if (NULL == pfnNtSuspendProcess)
        {
            pfnNtSuspendProcess = reinterpret_cast<PfnNtSuspendProcess>(
                                      GetProcAddress(GetModuleHandle(TEXT("ntdll")), "NtSuspendProcess"));
        }

        if (NULL != pfnNtSuspendProcess)
        {
            IApplicationActivationManager* pAppActivationManager;

            if (SUCCEEDED(CoCreateInstance(__uuidof(ApplicationActivationManager),
                                           NULL,
                                           CLSCTX_LOCAL_SERVER,
                                           __uuidof(IApplicationActivationManager),
                                           reinterpret_cast<void**>(&pAppActivationManager))))
            {
                if (SUCCEEDED(CoAllowSetForegroundWindow(pAppActivationManager, NULL)) &&
                    SUCCEEDED(pAppActivationManager->ActivateApplication(userModelId.asCharArray(),
                                                                         arguments.asCharArray(),
                                                                         AO_NONE,
                                                                         &processId)))
                {
                    processHandle = OpenProcess(SYNCHRONIZE |
                                                PROCESS_QUERY_INFORMATION |
                                                PROCESS_SET_INFORMATION |
                                                PROCESS_VM_READ |
                                                PROCESS_TERMINATE |
                                                PROCESS_SUSPEND_RESUME,
                                                FALSE,
                                                processId);

                    if (NULL != processHandle)
                    {
                        pfnNtSuspendProcess(processHandle);
                        retVal = true;

                        wchar_t bufFullPath[MAX_PATH];
                        DWORD szBuf = MAX_PATH;

                        if (QueryFullProcessImageNameW(processHandle, 0, bufFullPath, &szBuf))
                        {
                            executablePath.setFullPathFromString(bufFullPath);
                        }
                        else
                        {
                            executablePath.clear();
                        }
                    }
                }

                pAppActivationManager->Release();
            }
        }
    }

    return retVal;
}

bool osResumeSuspendedWindowsStoreApp(const osProcessHandle& processHandle, bool closeHandle)
{
    bool retVal = false;

    if (osSupportWindowsStoreApps())
    {
        typedef LONG(NTAPI * PfnNtResumeProcess)(IN HANDLE hProcess);
        static PfnNtResumeProcess pfnNtResumeProcess = NULL;

        if (NULL == pfnNtResumeProcess)
        {
            pfnNtResumeProcess = reinterpret_cast<PfnNtResumeProcess>(
                                     GetProcAddress(GetModuleHandle(TEXT("ntdll")), "NtResumeProcess"));
        }

        if (NULL != processHandle && NULL != pfnNtResumeProcess)
        {
            pfnNtResumeProcess(processHandle);
            retVal = true;
        }

        if (closeHandle)
        {
            CloseHandle(processHandle);
        }
    }

    return retVal;
}

bool osEnumerateInstalledWindowsStoreApps(gtList<WindowsStoreAppInfo>& storeApps)
{
    bool retVal = false;

    if (osSupportWindowsStoreApps())
    {
        typedef int(*GetAppInfo)(gtList<WindowsStoreAppInfo>&);
        static osModuleHandle moduleHandle = nullptr;
        static GetAppInfo fpGetAppInfo = nullptr;

        if (nullptr == moduleHandle)
        {
            static osModule winComponentExtUtilsModule;
            static bool loadedCompUtils = false;
            if (!loadedCompUtils)
            {
                osFilePath modulePath;
                osGetLoadedModulePath(nullptr, modulePath);
                modulePath.setFileName(AMDTWINCOMPONENTEXTUTILS_FILE_NAME);
                modulePath.setFileExtension(OS_MODULE_EXTENSION);
                loadedCompUtils = winComponentExtUtilsModule.loadModule(modulePath);
            }

            GT_IF_WITH_ASSERT(loadedCompUtils)
            {
                moduleHandle = winComponentExtUtilsModule.GetModuleHandle();
                GT_IF_WITH_ASSERT(moduleHandle != nullptr)
                {
                    fpGetAppInfo = (GetAppInfo)GetProcAddress(moduleHandle, "GetAppInfo");
                }
            }
        }

        GT_IF_WITH_ASSERT(fpGetAppInfo && 0 == fpGetAppInfo(storeApps))
        {
            retVal = true;
        }
    }

    return retVal;
}

bool osDetermineIsWindowsStoreApp64Bit(const gtString& userModelId)
{
    APPX_PACKAGE_ARCHITECTURE arch = APPX_PACKAGE_ARCHITECTURE_NEUTRAL;

    if (osSupportWindowsStoreApps())
    {
        IStartMenuAppItems62* pStartMenuApps;

        if (SUCCEEDED(CoCreateInstance(__uuidof(StartMenuCacheAndAppResolver),
                                       NULL,
                                       CLSCTX_INPROC_SERVER,
                                       __uuidof(IStartMenuAppItems62),
                                       reinterpret_cast<void**>(&pStartMenuApps)))
            ||
            SUCCEEDED(CoCreateInstance(__uuidof(StartMenuCacheAndAppResolver),
                                       NULL,
                                       CLSCTX_INPROC_SERVER,
                                       __uuidof(IStartMenuAppItems63),
                                       reinterpret_cast<void**>(&pStartMenuApps))))
        {
            IEnumObjects* pEnum;

            if (SUCCEEDED(pStartMenuApps->EnumItems(SMAIF_DEFAULT, __uuidof(IEnumObjects), reinterpret_cast<void**>(&pEnum))))
            {
                bool found = false;
                IPropertyStore* pPropertyStore;
                ULONG num;

                while (!found && S_OK == pEnum->Next(1UL, __uuidof(IPropertyStore), reinterpret_cast<void**>(&pPropertyStore), &num))
                {
                    wchar_t* pAppUserModelId = NULL;
                    IPropertyStore_GetString(pPropertyStore, PKEY_AppUserModel_ID, &pAppUserModelId);

                    if (NULL != pAppUserModelId)
                    {
                        if (0 == wcscmp(pAppUserModelId, userModelId.asCharArray()))
                        {
                            wchar_t* pPath = NULL;
                            IPropertyStore_GetString(pPropertyStore, PKEY_AppUserModel_PackageInstallPath, &pPath);

                            if (NULL != pPath)
                            {
                                arch = ExtractPackageArchitecture(pPath);
                                CoTaskMemFree(pPath);
                            }

                            found = true;
                        }

                        CoTaskMemFree(pAppUserModelId);
                    }

                    pPropertyStore->Release();
                }

                pEnum->Release();
            }

            pStartMenuApps->Release();
        }
    }

    if (APPX_PACKAGE_ARCHITECTURE_NEUTRAL == arch)
    {
#if defined(_M_X64)
        arch = APPX_PACKAGE_ARCHITECTURE_X64;
#else
        BOOL isWow64;
        arch = (IsWow64Process(GetCurrentProcess(), &isWow64) && isWow64) ? APPX_PACKAGE_ARCHITECTURE_X64 : APPX_PACKAGE_ARCHITECTURE_X86;
#endif
    }

    return APPX_PACKAGE_ARCHITECTURE_X64 == arch;
}

bool osGetWindowsStoreAppExecutable(const gtString& userModelId, gtString& exeFullPath)
{
    bool ret = false;

    if (osSupportWindowsStoreApps())
    {
        IStartMenuAppItems62* pStartMenuApps;

        if (SUCCEEDED(CoCreateInstance(__uuidof(StartMenuCacheAndAppResolver),
                                       NULL,
                                       CLSCTX_INPROC_SERVER,
                                       __uuidof(IStartMenuAppItems62),
                                       reinterpret_cast<void**>(&pStartMenuApps)))
            ||
            SUCCEEDED(CoCreateInstance(__uuidof(StartMenuCacheAndAppResolver),
                                       NULL,
                                       CLSCTX_INPROC_SERVER,
                                       __uuidof(IStartMenuAppItems63),
                                       reinterpret_cast<void**>(&pStartMenuApps))))
        {
            IEnumObjects* pEnum;

            if (SUCCEEDED(pStartMenuApps->EnumItems(SMAIF_DEFAULT, __uuidof(IEnumObjects), reinterpret_cast<void**>(&pEnum))))
            {
                bool found = false;
                IPropertyStore* pPropertyStore;
                ULONG num;

                while (!found && S_OK == pEnum->Next(1UL, __uuidof(IPropertyStore), reinterpret_cast<void**>(&pPropertyStore), &num))
                {
                    wchar_t* pAppUserModelId = NULL;
                    IPropertyStore_GetString(pPropertyStore, PKEY_AppUserModel_ID, &pAppUserModelId);

                    if (NULL != pAppUserModelId)
                    {
                        if (0 == wcscmp(pAppUserModelId, userModelId.asCharArray()))
                        {
                            wchar_t* pPath = NULL;
                            IPropertyStore_GetString(pPropertyStore, PKEY_AppUserModel_PackageInstallPath, &pPath);

                            if (NULL != pPath)
                            {
                                ret = ExtractPackageApplicationExecutable(pPath, userModelId.asCharArray(), exeFullPath);
                                CoTaskMemFree(pPath);
                            }

                            found = true;
                        }

                        CoTaskMemFree(pAppUserModelId);
                    }

                    pPropertyStore->Release();
                }

                pEnum->Release();
            }

            pStartMenuApps->Release();
        }
    }

    return ret;
}


static HRESULT IPropertyStore_GetString(IPropertyStore* pPropStore, const PROPERTYKEY& key, wchar_t** ppStr)
{
    PROPVARIANT propVar;
    HRESULT hr = pPropStore->GetValue(key, &propVar);

    if (SUCCEEDED(hr))
    {
        if (VT_EMPTY != propVar.vt)
        {
            hr = PropVariantToStringAlloc(propVar, ppStr);
        }
        else
        {
            hr = E_PROP_ID_UNSUPPORTED;
        }

        PropVariantClear(&propVar);
    }

    return hr;
}


static APPX_PACKAGE_ARCHITECTURE ExtractPackageArchitecture(const wchar_t* pPackagePath)
{
    APPX_PACKAGE_ARCHITECTURE arch = APPX_PACKAGE_ARCHITECTURE_NEUTRAL;

    size_t len = wcslen(pPackagePath);

    if (len <= (MAX_PATH - (16 + 1)))
    {
        wchar_t manifestPath[OS_MAX_PATH];
        memcpy(manifestPath, pPackagePath, sizeof(wchar_t) * len);
        manifestPath[len++] = L'\\';
        wcscpy(manifestPath + len, L"AppXManifest.xml");

        IStream* pInputStream;

        if (SUCCEEDED(SHCreateStreamOnFileEx(manifestPath,
                                             STGM_READ | STGM_SHARE_EXCLUSIVE,
                                             0,
                                             FALSE,
                                             NULL,
                                             &pInputStream)))
        {
            IAppxFactory* pFactory;

            if (SUCCEEDED(CoCreateInstance(__uuidof(AppxFactory),
                                           NULL,
                                           CLSCTX_INPROC_SERVER,
                                           __uuidof(IAppxFactory),
                                           reinterpret_cast<void**>(&pFactory))))
            {
                IAppxManifestReader* pManifestReader;

                if (SUCCEEDED(pFactory->CreateManifestReader(pInputStream, &pManifestReader)))
                {
                    IAppxManifestPackageId* pManifestPackageId;

                    if (SUCCEEDED(pManifestReader->GetPackageId(&pManifestPackageId)))
                    {
                        pManifestPackageId->GetArchitecture(&arch);
                        pManifestPackageId->Release();
                    }

                    pManifestReader->Release();
                }

                pFactory->Release();
            }

            pInputStream->Release();
        }
    }

    return arch;
}

static bool ExtractPackageApplicationExecutable(const wchar_t* pPackagePath, const wchar_t* pUserModelId, gtString& exeFullPath)
{
    bool ret = false;

    exeFullPath.makeEmpty();

    pUserModelId = wcsrchr(pUserModelId, L'!');

    if (NULL != pUserModelId && L'\0' != pUserModelId[0] && L'\0' != pUserModelId[1])
    {
        pUserModelId++;

        size_t len = wcslen(pPackagePath);

        if (len <= (MAX_PATH - (16 + 1)))
        {
            wchar_t manifestPath[OS_MAX_PATH];
            memcpy(manifestPath, pPackagePath, sizeof(wchar_t) * len);
            manifestPath[len++] = L'\\';
            wcscpy(manifestPath + len, L"AppXManifest.xml");

            IStream* pInputStream;

            if (SUCCEEDED(SHCreateStreamOnFileEx(manifestPath,
                                                 STGM_READ | STGM_SHARE_EXCLUSIVE,
                                                 0,
                                                 FALSE,
                                                 NULL,
                                                 &pInputStream)))
            {
                IAppxFactory* pFactory;

                if (SUCCEEDED(CoCreateInstance(__uuidof(AppxFactory),
                                               NULL,
                                               CLSCTX_INPROC_SERVER,
                                               __uuidof(IAppxFactory),
                                               reinterpret_cast<void**>(&pFactory))))
                {
                    IAppxManifestReader* pManifestReader;

                    if (SUCCEEDED(pFactory->CreateManifestReader(pInputStream, &pManifestReader)))
                    {
                        IAppxManifestPackageId* pManifestPackageId;

                        if (SUCCEEDED(pManifestReader->GetPackageId(&pManifestPackageId)))
                        {
                            IAppxManifestApplicationsEnumerator* pApplications = NULL;

                            if (SUCCEEDED(pManifestReader->GetApplications(&pApplications)))
                            {
                                BOOL hasCurrent = FALSE;
                                HRESULT hr = pApplications->GetHasCurrent(&hasCurrent);

                                while (!ret && SUCCEEDED(hr) && hasCurrent)
                                {
                                    IAppxManifestApplication* pApplication = NULL;

                                    if (SUCCEEDED(pApplications->GetCurrent(&pApplication)))
                                    {
                                        LPWSTR pApplicationId = NULL;
                                        pApplication->GetStringValue(L"Id", &pApplicationId);

                                        if (NULL != pApplicationId)
                                        {
                                            if (0 == wcscmp(pUserModelId, pApplicationId))
                                            {
                                                LPWSTR pApplicationExe = NULL;
                                                pApplication->GetStringValue(L"Executable", &pApplicationExe);

                                                if (NULL != pApplicationExe)
                                                {
                                                    exeFullPath = pPackagePath;
                                                    exeFullPath.append(L'\\');
                                                    exeFullPath.append(pApplicationExe);
                                                    CoTaskMemFree(pApplicationExe);
                                                }

                                                ret = true;
                                            }

                                            CoTaskMemFree(pApplicationId);
                                        }
                                    }

                                    pApplication->Release();
                                    pApplication = NULL;

                                    hr = pApplications->MoveNext(&hasCurrent);
                                }
                            }

                            pManifestPackageId->Release();
                        }

                        pManifestReader->Release();
                    }

                    pFactory->Release();
                }

                pInputStream->Release();
            }
        }
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        GetCurrentModuleHandle
// Description: Get a handle to the DLL from which this code is running
//              This is an internal helper function for osGetApplicationVersion()
//              so it is not exported from the OSWrappers DLL
// Author:      AMD Developer Tools Team
// Date:        Apr-13, 2016
// ---------------------------------------------------------------------------
HMODULE osGetCurrentModuleHandle()
{
    HMODULE hMod = NULL;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       reinterpret_cast<LPCWSTR>(&osGetCurrentModuleHandle),
                       &hMod);
    return hMod;
}

// ---------------------------------------------------------------------------
// Name:        osGetApplicationVersionFromFileInfo
// Description: Returns the application version from the file info
//              This is an internal helper function for osGetApplicationVersion()
//              so it is not exported from the OSWrappers DLL
// Author:      AMD Developer Tools Team
// Date:        Apr-13, 2016
// ---------------------------------------------------------------------------
bool osGetApplicationVersionFromFileInfo(const wchar_t* fileName, osProductVersion& applicationVersion)
{
    bool retVal = false;
    DWORD fileHandle = 0;

    DWORD infoLength = GetFileVersionInfoSize(fileName, &fileHandle);

    GT_IF_WITH_ASSERT(infoLength > 0)
    {
        char* pData = new char[infoLength];

        if (GetFileVersionInfo(fileName, fileHandle, infoLength, pData))
        {
            VS_FIXEDFILEINFO* pFileInfo = NULL;
            unsigned int bufLen = 0;

            if (VerQueryValue(pData, L"\\", (LPVOID*)&pFileInfo, (PUINT)&bufLen))
            {
                retVal = true;
                applicationVersion._majorVersion = HIWORD(pFileInfo->dwProductVersionMS);
                applicationVersion._minorVersion = LOWORD(pFileInfo->dwProductVersionMS);
                applicationVersion._patchNumber = HIWORD(pFileInfo->dwProductVersionLS);
                applicationVersion._revisionNumber = LOWORD(pFileInfo->dwProductVersionLS);
            }
        }

        delete[] pData;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetApplicationVersion
// Description: Returns the current application version. For CodeXL standalone
//              the version will be calculated in runtime. For VS extension,
//              the version will be taken from the macros.
// Author:      AMD Developer Tools Team
// Date:        29/6/2004
// ---------------------------------------------------------------------------
void osGetApplicationVersion(osProductVersion& applicationVersion)
{

    // The Windows implementation gets the product version from file info instead of macros.
    // This prevents rebuild of OSWrappers when build number is bumped up on Windows and shortens the Windows incremental build.

    applicationVersion._majorVersion = 0;
    applicationVersion._minorVersion = 0;
    applicationVersion._patchNumber = 0;
    applicationVersion._revisionNumber = 0;

    // Get current file name
    const int bufLen = 4096;
    wchar_t fileName[bufLen];
    DWORD rc = GetModuleFileName(NULL, fileName, bufLen);

    // This flag indicates whether the version was eventually extracted or not.
    bool isVersionExtracted = false;

    if (rc != 0)
    {
        // To be on the safe side.
        fileName[4095] = '\0';

        gtString fileNameStr(fileName);


        // Read the data directly from the executable file info unless we're running inside Visual Studio as
        // a VS extension
        if ((-1 == fileNameStr.find(L"devenv", 0)))
        {
            isVersionExtracted = osGetApplicationVersionFromFileInfo(fileName, applicationVersion);
        }
    }

    if (!isVersionExtracted)
    {
        // Read the data directly from the current DLL file info
        HMODULE hMod = osGetCurrentModuleHandle();
        rc = GetModuleFileName(hMod, fileName, bufLen);

        if (rc != 0)
        {
            isVersionExtracted = osGetApplicationVersionFromFileInfo(fileName, applicationVersion);
        }
    }

    if (!isVersionExtracted)
    {
        // Get the application version from the version macros.
        osGetApplicationVersionFromMacros(applicationVersion);
    }
}



// ---------------------------------------------------------------------------
// Name:        GetExecutedApplicationType
// Description: Returns the type of application we're running inside -
//              standalone or Visual Studio plug-in
// Author:      AMD Developer Tools Team
// Date:        Jul-26, 2015
// ---------------------------------------------------------------------------
OS_API osExecutedApplicationType GetExecutedApplicationType()
{
    return g_theExecutedApplicationType;
}

// ---------------------------------------------------------------------------
// Name:        SetExecutedApplicationType
// Description: By default the app type is standalone, so only the VS plug-in should call
//              this function to let interested code know we're running inside Visual Studio
// Author:      AMD Developer Tools Team
// Date:        Jul-26, 2015
// ---------------------------------------------------------------------------
OS_API void SetExecutedApplicationType(osExecutedApplicationType appType)
{
    g_theExecutedApplicationType = appType;
}
