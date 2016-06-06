//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  DLL Injection class implementation
//=============================================================================

#include <stdlib.h>
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <strsafe.h>

#include "dllInjector.h"

/*
References:
http://securityxploded.com/dll-injection-and-hooking.php
http://securityxploded.com/ntcreatethreadex.php
http://blog.opensecurityresearch.com/2013/01/windows-dll-injection-basics.html
http://resources.infosecinstitute.com/using-createremotethread-for-dll-injection-on-windows/
http://syprog.blogspot.com/2012/05/createremotethread-bypass-windows.html
http://www.kdsbest.com/?p=159
http://research.microsoft.com/pubs/68568/huntusenixnt99.pdf
https://www.joachim-bauch.de/tutorials/loading-a-dll-from-memory/
*/

// How to use the dll injector from Detours Express:
// 1. Download the Detours express package from Microsoft
//    It can be downloaded from here:
//    http ://research.microsoft.com/en-us/downloads/d36340fb-4d3c-4ddd-bf5b-1db25d03713d/default.aspx
//    or by searching for "Detours express 3.0 download".
// 2. Install the package to the hard drive. The detours files are included from
//    dllInjectorDetours.inl and assume that detours is installed to Common/Lib/Ext/Detours Express 3.0.
//    It can be installed elsewhere but the path for the included detours files will need modifying
//    in dllInjectorDetours.inl, including creatwth.cpp and modules.cpp.
// 3. Bug fix: In creatwth.cpp, Change the line:
//       DETOUR_EXE_RESTORE der;
//    to:
//       static DETOUR_EXE_RESTORE der;
//    This struct is quite large and can a cause stack overflow on some systems.
//    Also replace both occurrences of '#1' in the same file with 'InjectDLL'
// 4. Uncomment the #define USE_DETOURS line below and rebuild the solution.

//#define USE_DETOURS

// Implementation files
#include "dllInjectorLoadLibrary.inl"
#ifdef USE_DETOURS
    #include "dllInjectorDetours.inl"
#endif // USE_DETOURS

/// Public interface shared between implementations
DLLInjector::DLLInjector()
{
#ifdef USE_DETOURS
    m_pImpl = new DLLInjectorDetours();
#else
    m_pImpl = new DLLInjectorLoadLibrary();
#endif // USE_DETOURS
}

DLLInjector::~DLLInjector()
{
    delete m_pImpl;
}

bool DLLInjector::CalledFromRundll()
{
    return m_pImpl->CalledFromRundll();
}

void DLLInjector::Inject(LPSTR lpszCmdLine)
{
    return m_pImpl->Inject(lpszCmdLine);
}

BOOL WINAPI DLLInjector::CreateProcessAndInjectDllA(_In_opt_ LPCSTR lpApplicationName,
                                                    _Inout_opt_ LPSTR lpCommandLine,
                                                    _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                                    _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                                    _In_ BOOL bInheritHandles,
                                                    _In_ DWORD dwCreationFlags,
                                                    _In_opt_ LPVOID lpEnvironment,
                                                    _In_opt_ LPCSTR lpCurrentDirectory,
                                                    _In_ LPSTARTUPINFOA lpStartupInfo,
                                                    _Out_ LPPROCESS_INFORMATION lpProcessInformation,
                                                    _In_ LPCSTR lpDllName)
{
    return m_pImpl->CreateProcessAndInjectDllA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
                                               lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, lpDllName);
}

BOOL WINAPI DLLInjector::CreateProcessAndInjectDllW(_In_opt_ LPCWSTR lpApplicationName,
                                                    _Inout_opt_  LPWSTR lpCommandLine,
                                                    _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                                    _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                                    _In_ BOOL bInheritHandles,
                                                    _In_ DWORD dwCreationFlags,
                                                    _In_opt_ LPVOID lpEnvironment,
                                                    _In_opt_ LPCWSTR lpCurrentDirectory,
                                                    _In_ LPSTARTUPINFOW lpStartupInfo,
                                                    _Out_ LPPROCESS_INFORMATION lpProcessInformation,
                                                    _In_ LPCSTR lpDllName)
{
    return m_pImpl->CreateProcessAndInjectDllW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
                                               lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, lpDllName);

}

static DLLInjector s_dllInjector;

// ---------------------------------------------------------------------------
/// Inject a dll into a suspended process. This is an external function called
/// from rundll32.exe when process-hopping between 64 and 32 bit processes.
/// As such, the arguments to this function will be provided by rundll32
/// \param lpszCmdLine The command line arguments passed to this function. This
///        consists of a string containing the process ID of the application
///        to inject into and the full path of the dll to inject.
// ---------------------------------------------------------------------------
VOID CALLBACK InjectDLL(_In_ HWND /*hwnd*/, _In_ HINSTANCE /*hinst*/, _In_ LPSTR lpszCmdLine, _In_ INT /*nCmdShow*/)
{
    s_dllInjector.Inject(lpszCmdLine);
}

namespace AMDT
{
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
                                       _In_ LPCSTR lpDllName)
{
    return s_dllInjector.CreateProcessAndInjectDllA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
                                                    lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, lpDllName);
}

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
                                       _In_ LPCSTR lpDllName)
{
    return s_dllInjector.CreateProcessAndInjectDllW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
                                                    lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, lpDllName);
}

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
bool InitHookDLL(DWORD reason_for_call)
{
    UNREFERENCED_PARAMETER(reason_for_call);

    if (s_dllInjector.CalledFromRundll())
    {
        return false;
    }

    return true;
}

} // namespace AMDT
