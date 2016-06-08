#ifndef _INTERCEPTOR_H
#define _INTERCEPTOR_H

#include <Windows.h>
#include <detours.h>

namespace AMDT
{
LONG BeginHook()
{
    LONG detoursError = DetourTransactionBegin();

    if (detoursError == NO_ERROR)
    {
        detoursError = DetourUpdateThread(GetCurrentThread());
    }

    return detoursError;
}

LONG EndHook()
{
    return DetourTransactionCommit();
}

// Hook an API call.
LONG WINAPI HookAPICall(_Inout_ PVOID* ppPointer, _In_ PVOID pDetour)
{
    return DetourAttach(ppPointer, pDetour);
}

// Unhook an API call.
LONG WINAPI UnhookAPICall(_Inout_ PVOID* ppPointer, _In_ PVOID pDetour)
{
    return DetourDetach(ppPointer, pDetour);
}

// Create a process and inject a dll into it
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
    return DetourCreateProcessWithDllExA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
                                         dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, lpDllName, NULL);
}

// Create a process and inject a dll into it
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
    return DetourCreateProcessWithDllExW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
                                         dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, lpDllName, NULL);
}

#ifdef UNICODE
    #define CreateProcessAndInjectDll     CreateProcessAndInjectDllW
#else
    #define CreateProcessAndInjectDll      CreateProcessAndInjectDllA
#endif // !UNICODE

// Call at the beginning of MicroDLL
// If the injected dll is called from a helper process, exit immediately and don't detour anything.
// In the case of a 32-bit launcher creating a 64-bit application, a 64-bit helper process
// is created. Rundll32.dll is used to call DetourFinishHelperProcess in MicroDLL, which has
// been exported from the detours library in this dll via the Microdll.def file as ordinal 1,
// and this is used to patch the application's import table to 'inject' the 64-bit microdll.
// The important thing to note here is that a 64-bit dll can only be injected into a 64-bit process
// via a separate 64-bit process.
// The same is true for a 64-bit launcher creating a 32-bit process.
// See the detours 3 help for more information.
bool InitHookDLL(DWORD reason_for_call)
{
    if (DetourIsHelperProcess())
    {
        return false;
    }

    if (reason_for_call == DLL_PROCESS_ATTACH)
    {
        DetourRestoreAfterWith();
    }

    return true;
}

}

#endif // def _INTERCEPTOR_H