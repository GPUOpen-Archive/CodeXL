//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Defines the hooked functionality of the LoadLibrary API
///         functions.
//==============================================================================

#include <windows.h>

#include <Interceptor.h>
#include "../Common/misc.h"
#include "../Common/logger.h"
#include "../Common/SharedGlobal.h"
#include "../Common/PerfStudioServer_Version.h"

#include "WrapperInfo.h"
#include "LoadLibrary.h"

// By default, due to volume of messages and usefuleness of them, we disable LogTrace logging of this module
// Comment out the following lines to enable TRACE logging.
#undef   LogTrace
#define  LogTrace //

static bool s_bLoadingWrapper = false; ///< Used to flag when a wrapper is loading

// using namespace std;

typedef BOOL (WINAPI*  FreeLibrary_type)(HMODULE hLibModule); ///< Function pointer typedef
static FreeLibrary_type Real_FreeLibrary = FreeLibrary; ///< Store the real function pointer

typedef HMODULE(WINAPI*  LoadLibraryA_type)(LPCSTR lpLibFileName); ///< Function pointer typedef
static LoadLibraryA_type Real_LoadLibraryA = LoadLibraryA; ///< Store the real function pointer

typedef HMODULE(WINAPI*  LoadLibraryExA_type)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags); ///< Function pointer typedef
static LoadLibraryExA_type Real_LoadLibraryExA = LoadLibraryExA; ///< Store the real function pointer

typedef HMODULE(WINAPI*  LoadLibraryW_type)(LPCWSTR lpLibFileName); ///< Function pointer typedef
static LoadLibraryW_type Real_LoadLibraryW = LoadLibraryW; ///< Store the real function pointer

typedef HMODULE(WINAPI*  LoadLibraryExW_type)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags); ///< Function pointer typedef
static LoadLibraryExW_type Real_LoadLibraryExW = LoadLibraryExW; ///< Store the real function pointer

/// Mine entry point for intercepted function
/// \param hLibModule Library module
/// \return True if success false if fail.
static BOOL WINAPI Mine_FreeLibrary(HMODULE hLibModule)
{
    LogTrace(traceENTER, "hLibModule = 0x%p", hLibModule);

    /*
       char modulename[MAX_PATH];
       GetModuleFileNameA( hLibModule, modulename, MAX_PATH );
       Log(logMESSAGE, "hLibModule = 0x%p %s\n", hLibModule, modulename);
    */
    BOOL b = Real_FreeLibrary(hLibModule);

    if (s_bLoadingWrapper == false && hLibModule != NULL)
    {
        s_bLoadingWrapper = true;
        CheckWrapperOnFreeLibrary();
        s_bLoadingWrapper = false;
    }

    LogTrace(traceEXIT, "returned %d", b);
    return b;
}

/// Pause application to allow debugger to attach
/// This functionality only happens once - on the first call to LoadLibrary.
static void CheckForDebuggerAttach(void)
{
    static bool alreadyChecked = false;  // only pause application once.
    static const char fmtString[] = "The application has been paused to allow the Visual Studio debugger to be attached to the process.\n\nProcess ID = %d \n\nProcess Name: %s\n\nPress OK to continue";

    if (SG_GET_BOOL(OptionBreak) && !alreadyChecked)         // did we run PerfServer with --break option
    {
        char message[sizeof(fmtString) + 4 + MAX_PATH];    // Add 4 extra bytes for process ID - allows for up to 6 digits of process ID in total

        char modulename[MAX_PATH];
        GetModuleFileNameA(NULL, modulename, MAX_PATH);

        alreadyChecked = true;
        sprintf_s(message, sizeof(message), fmtString, GetCurrentProcessId(), modulename);
        MessageBoxStop(message);
    }
}

/// Reference counter indicating recursion depth of LoadLibrary calls. LoadLibrary can be used
/// recursively (LoadLibrary can spawn other LoadLibrary calls) and we're only interested in the
/// first call to LoadLibrary (reference count == 1).
static RefTrackerCounter s_dwInsideLoadLibrary;

/// Check the wrappers depending on whether PerfStudio is using DLL replacement or not.
/// If using DLL replacement, PerfStudio's versions of the API dll's (D3D12.dll, dxgi.dll etc)
/// will have been loaded by the OS. The UpdateHooks() method needs to be called on these
/// replaced Dll's to do some initialization that can't be done from DllMain, such as loading
/// in the system dll of the same name and getting the addresses of the real function
/// pointers.
/// If not using dll replacement, CheckWrapperOnLoadLibrary() is called, which loads the
/// appropriate PerfStudio server plugin, based on which system dll's have been loaded (ie
/// if D3D12.dll has been loaded, then load DX12Server.dll and call its UpdateHooks method
static void CheckWrappers()
{
    if (SG_GET_BOOL(OptionDllReplacement) == false)
    {
        CheckWrapperOnLoadLibrary();
    }
    else
    {
        UpdateHooksOnLoadLibrary();
    }
}

/// Compare 2 character strings. Local version used here because lstrcmp is called
/// from LoadLibrary, and pulls in Kernel32.dll. If this dll hasn't been loaded
/// before the LoadLibrary functions have been hooked, it will lead to the same
/// recursion problems seen with MessageBox and conctl32.dll
/// \param str1 Input string
/// \param str2 Input string
/// \return 0 if strings are the same, non-zero if different.
static int my_lstrcmp(LPCSTR str1, LPCSTR str2)
{
    // Return -1 if either pointer is unsafe
    int result = -1;

    if ((str1 != nullptr) && (str2 != nullptr))
    {
        while (*str1 == *str2++)
        {
            if (*str1++ == 0)
            {
                return 0;
            }
        }

        result = (*(const unsigned char*)str1 - *(const unsigned char*)(str2 - 1));
    }

    return result;
}

/// Compare 2 wide character strings. Local version used here because lstrcmpW is called
/// from LoadLibrary, and pulls in Kernel32.dll. If this dll hasn't been loaded
/// before the LoadLibrary functions have been hooked, it will lead to the same
/// recursion problems seen with MessageBox and conctl32.dll
/// \param str1 Input string
/// \param str2 Input string
// \return 0 if strings are the same, non-zero if different.
static int my_lstrcmpW(LPCWSTR str1, LPCWSTR str2)
{
    // Return -1 if either pointer is unsafe
    int result = -1;

    if ((str1 != nullptr) && (str2 != nullptr))
    {
        while (*str1 == *str2++)
        {
            if (*str1++ == 0)
            {
                return 0;
            }
        }

        result = (*str1 - *(str2 - 1));
    }

    return result;
}

/// Mine entry point for intercepted function
/// \param lpLibFileName Library file name
/// \return Loaded module.
static HMODULE WINAPI Mine_LoadLibraryA(LPCSTR lpLibFileName)
{
    RefTracker rf(&s_dwInsideLoadLibrary);

    if (s_dwInsideLoadLibrary == 1)
    {
        LogTrace(traceENTER, "lpLibFilename = %s", lpLibFileName);
    }

    HMODULE res = Real_LoadLibraryA(lpLibFileName);
    DWORD realError = GetLastError();

    if (my_lstrcmp(lpLibFileName, "comctl32.dll") != 0)
    {
        // This function uses MessageBox - which will load comctl32.dll if necessary
        // To avoid recursion problems, only call it when a different DLL is being loaded.
        CheckForDebuggerAttach();
    }

    if (s_dwInsideLoadLibrary == 1)
    {
        CheckWrappers();
        LogTrace(traceEXIT, "returned 0x%p", res);
    }

    SetLastError(realError);
    return res;
}

/// Mine entry point for intercepted function
/// \param lpLibFileName Library file name
/// \param hFile File handle
/// \param dwFlags load flags
/// \return Loaded module.
static HMODULE WINAPI Mine_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    RefTracker rf(&s_dwInsideLoadLibrary);

    if (s_dwInsideLoadLibrary == 1)
    {
        LogTrace(traceENTER, "lpLibFilename = %s, hFile = %p, dwFlags = %d", lpLibFileName, hFile, dwFlags);
    }

    HMODULE res = Real_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
    DWORD realError = GetLastError();

    if (my_lstrcmp(lpLibFileName, "comctl32.dll") != 0)
    {
        // This function uses MessageBox - which will load comctl32.dll if necessary
        // To avoid recursion problems, only call it when a different DLL is being loaded.
        CheckForDebuggerAttach();
    }

    if (s_dwInsideLoadLibrary == 1)
    {
        CheckWrappers();
        LogTrace(traceEXIT, "returned 0x%p", res);
    }

    SetLastError(realError);
    return res;
}

/// Mine entry point for intercepted function
/// \param lpLibFileName Library file name
/// \return Loaded module.
static HMODULE WINAPI Mine_LoadLibraryW(LPCWSTR lpLibFileName)
{
    RefTracker rf(&s_dwInsideLoadLibrary);

    if (s_dwInsideLoadLibrary == 1)
    {
        LogTrace(traceENTER, "lpLibFilename = %S", lpLibFileName);
    }

    HMODULE res = Real_LoadLibraryW(lpLibFileName);
    DWORD realError = GetLastError();

    if (my_lstrcmpW(lpLibFileName, L"comctl32.dll") != 0)
    {
        // This function uses MessageBox - which will load comctl32.dll if necessary
        // To avoid recursion problems, only call it when a different DLL is being loaded.
        CheckForDebuggerAttach();
    }

    if (s_dwInsideLoadLibrary == 1)
    {
        CheckWrappers();
        LogTrace(traceEXIT, "returned 0x%p", res);
    }

    SetLastError(realError);
    return res;
}

/// Mine entry point for intercepted function
/// \param lpLibFileName Library file name
/// \param hFile File handle
/// \param dwFlags load flags
/// \return Loaded module.
static HMODULE WINAPI Mine_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    RefTracker rf(&s_dwInsideLoadLibrary);

    if (s_dwInsideLoadLibrary == 1)
    {
        LogTrace(traceENTER, "lpLibFilename = %S, hFile = %p, dwFlags = %d", lpLibFileName, hFile, dwFlags);
    }

    HMODULE res = Real_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
    DWORD realError = GetLastError();

    if (my_lstrcmpW(lpLibFileName, L"comctl32.dll") != 0)
    {
        // This function uses MessageBox - which will load comctl32.dll if necessary
        // To avoid recursion problems, only call it when a different DLL is being loaded.
        CheckForDebuggerAttach();
    }

    if (s_dwInsideLoadLibrary == 1)
    {
        CheckWrappers();
        LogTrace(traceEXIT, "returned 0x%p", res);
    }

    SetLastError(realError);
    return res;
}

/// Hook the load library functions
void HookLoadLibrary()
{
    LogTrace(traceENTER, "");

    AMDT::BeginHook();

    LONG error = AMDT::HookAPICall(&(PVOID&)Real_LoadLibraryA, Mine_LoadLibraryA);
    PsAssert(error == NO_ERROR);

    error = AMDT::HookAPICall(&(PVOID&)Real_LoadLibraryExA, Mine_LoadLibraryExA);
    PsAssert(error == NO_ERROR);

    error = AMDT::HookAPICall(&(PVOID&)Real_LoadLibraryW, Mine_LoadLibraryW);
    PsAssert(error == NO_ERROR);

    error = AMDT::HookAPICall(&(PVOID&)Real_LoadLibraryExW, Mine_LoadLibraryExW);
    PsAssert(error == NO_ERROR);

    error = AMDT::HookAPICall(&(PVOID&)Real_FreeLibrary, Mine_FreeLibrary);
    PsAssert(error == NO_ERROR);

    if (AMDT::EndHook() != NO_ERROR)
    {
        Log(logERROR, "HookLoadLibrary() failed\n");
    }

    // Load the "dxgi.dll" at the beginning as we missed the dxgi loading sometimes.
    // TODO: Fix this: This function is called from DllMain(), and according to the
    // Microsoft documentation, LoadLibrary() should not be called from DllMain()
    LoadLibraryA("dxgi.dll");

    LogTrace(traceEXIT, "");
    return;
}

/// Unhook the load library functions
void UnhookLoadLibrary()
{
    LogTrace(traceENTER, "");

    AMDT::BeginHook();

    LONG error = AMDT::UnhookAPICall(&(PVOID&)Real_LoadLibraryA, Mine_LoadLibraryA);
    PsAssert(error == NO_ERROR);

    error = AMDT::UnhookAPICall(&(PVOID&)Real_LoadLibraryExA, Mine_LoadLibraryExA);
    PsAssert(error == NO_ERROR);

    error = AMDT::UnhookAPICall(&(PVOID&)Real_LoadLibraryW, Mine_LoadLibraryW);
    PsAssert(error == NO_ERROR);

    error = AMDT::UnhookAPICall(&(PVOID&)Real_LoadLibraryExW, Mine_LoadLibraryExW);
    PsAssert(error == NO_ERROR);

    error = AMDT::UnhookAPICall(&(PVOID&)Real_FreeLibrary, Mine_FreeLibrary);
    PsAssert(error == NO_ERROR);

    if (AMDT::EndHook() != NO_ERROR)
    {
        Log(logERROR, "UnhookLoadLibrary() failed\n");
    }

    // Restore Real functions to original values in case they aren't restored correctly by the unhook call
    Real_LoadLibraryA = LoadLibraryA;
    Real_LoadLibraryExA = LoadLibraryExA;
    Real_LoadLibraryW = LoadLibraryW;
    Real_LoadLibraryExW = LoadLibraryExW;
    Real_FreeLibrary = FreeLibrary;

    LogTrace(traceEXIT, "");
    return;
}

