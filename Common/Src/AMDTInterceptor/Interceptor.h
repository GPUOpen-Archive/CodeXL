//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Interceptor Interface function declarations
//==============================================================================

#ifndef _INTERCEPTOR_H
#define _INTERCEPTOR_H

#include <Windows.h>

namespace AMDT
{
// ---------------------------------------------------------------------------
/// Initialize anything before beginning API Hooking
/// \return 0 if successful, non-zero for error
// ---------------------------------------------------------------------------
LONG BeginHook();

// ---------------------------------------------------------------------------
/// Perform any cleanup after API Hooking
/// \return 0 if successful, non-zero for error
// ---------------------------------------------------------------------------
LONG EndHook();

// ---------------------------------------------------------------------------
/// Hook an API call.
/// \param ppRealFn The real function pointer
/// \param pMineFn The Hooked function pointer
/// \return 0 if successful, non-zero for error
// ---------------------------------------------------------------------------
LONG WINAPI HookAPICall(_Inout_ PVOID* ppRealFn, _In_ PVOID pMineFn);

// ---------------------------------------------------------------------------
/// Unhook an API call.
/// \param ppRealFn The real function pointer
/// \param pMineFn The Hooked function pointer
/// \return 0 if successful, non-zero for error
// ---------------------------------------------------------------------------
LONG WINAPI UnhookAPICall(_Inout_ PVOID* ppRealFn, _In_ PVOID pMineFn);

// ---------------------------------------------------------------------------
/// Create a process and inject a dll into it (non-unicode). For more
/// information see the MSDN documentation for CreateProcess
/// \param lpApplicationName The name of the module to be executed
/// \param lpCommandLine The command line to be executed
/// \param lpProcessAttributes A pointer to a SECURITY_ATTRIBUTES structure
///        that determines whether the returned handle to the new process object
///        can be inherited by child processes.
/// \param lpThreadAttributes A pointer to a SECURITY_ATTRIBUTES structure
///        that determines whether the returned handle to the new thread object
///        can be inherited by child processes.
/// \param bInheritHandles If this parameter TRUE, each inheritable handle in
///        the calling process is inherited by the new process.If the parameter
///        is FALSE, the handles are not inherited
/// \param dwCreationFlags The flags that control the priority class and the
///        creation of the process
/// \param lpEnvironment A pointer to the environment block for the new process
///        If this parameter is NULL, the new process uses the environment of the
///        calling process
/// \param lpCurrentDirectory The full path to the current directory for the
///        process
/// \param lpStartupInfo A pointer to a STARTUPINFO or STARTUPINFOEX structure
/// \param lpProcessInformation A pointer to a PROCESS_INFORMATION structure that
///        receives identification information about the new process
/// \param lpDllName Full path and name of the dll to inject into the new
///        process
/// \return non-zero if function succeeds, 0 if error
// ---------------------------------------------------------------------------
BOOL WINAPI CreateProcessAndInjectDllA(_In_opt_ LPCSTR lpApplicationName,
                                       _Inout_opt_ LPSTR lpCommandLine,
                                       _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                       _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                       _In_ BOOL bInheritHandles,
                                       _In_ DWORD dwCreationFlags,
                                       _In_opt_ LPVOID lpEnvironment,
                                       _In_opt_ LPCSTR lpCurrentDirectory,
                                       _In_ LPSTARTUPINFOA lpStartupInfo,
                                       _Out_ LPPROCESS_INFORMATION lpProcessInformation,
                                       _In_ LPCSTR lpDllName);

// ---------------------------------------------------------------------------
/// Create a process and inject a dll into it (unicode). For more information
/// see the MSDN documentation for CreateProcess
/// \param lpApplicationName The name of the module to be executed
/// \param lpCommandLine The command line to be executed
/// \param lpProcessAttributes A pointer to a SECURITY_ATTRIBUTES structure
///        that determines whether the returned handle to the new process object
///        can be inherited by child processes.
/// \param lpThreadAttributes A pointer to a SECURITY_ATTRIBUTES structure
///        that determines whether the returned handle to the new thread object
///        can be inherited by child processes.
/// \param bInheritHandles If this parameter TRUE, each inheritable handle in
///        the calling process is inherited by the new process.If the parameter
///        is FALSE, the handles are not inherited
/// \param dwCreationFlags The flags that control the priority class and the
///        creation of the process
/// \param lpEnvironment A pointer to the environment block for the new process
///        If this parameter is NULL, the new process uses the environment of the
///        calling process
/// \param lpCurrentDirectory The full path to the current directory for the
///        process
/// \param lpStartupInfo A pointer to a STARTUPINFO or STARTUPINFOEX structure
/// \param lpProcessInformation A pointer to a PROCESS_INFORMATION structure that
///        receives identification information about the new process
/// \param lpDllName Full path and name of the dll to inject into the new
///        process
/// \return non-zero if function succeeds, 0 if error
// ---------------------------------------------------------------------------
BOOL WINAPI CreateProcessAndInjectDllW(_In_opt_ LPCWSTR lpApplicationName,
                                       _Inout_opt_  LPWSTR lpCommandLine,
                                       _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                       _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                       _In_ BOOL bInheritHandles,
                                       _In_ DWORD dwCreationFlags,
                                       _In_opt_ LPVOID lpEnvironment,
                                       _In_opt_ LPCWSTR lpCurrentDirectory,
                                       _In_ LPSTARTUPINFOW lpStartupInfo,
                                       _Out_ LPPROCESS_INFORMATION lpProcessInformation,
                                       _In_ LPCSTR lpDllName);

#ifdef UNICODE
    #define CreateProcessAndInjectDll     CreateProcessAndInjectDllW
#else
    #define CreateProcessAndInjectDll      CreateProcessAndInjectDllA
#endif // !UNICODE

// ---------------------------------------------------------------------------
/// Call at the beginning of the injected dll DllMain() function.
/// If the injected dll is called from rundll32.exe, exit immediately and don't
/// call the rest of DllMain. This is a secondary process used to inject the
/// target application and once done injecting needs to exit as soon as possible.
/// \param reason_for_call The reason that DllMain is called. This is the same
///        parameter passed into DllMain
/// \return true if the rest of DllMain is to be executed, false if early exit
///         is needed
// ---------------------------------------------------------------------------
bool InitHookDLL(DWORD reason_for_call);

}

#endif // def _INTERCEPTOR_H