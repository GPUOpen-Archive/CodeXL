//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains the functions to detour LoadLibrary.
//==============================================================================

#include <string>
#include <windows.h>
#include <tchar.h>
#include <sstream>

#include "Interceptor.h"
#include "PluginInfo.h"
#include "Fileutils.h"
#include "Logger.h"

using namespace GPULogger;

static bool s_bLoadLibraryAttached = false;


typedef BOOL (WINAPI*  FreeLibrary_type)(HMODULE hLibModule);
static FreeLibrary_type Real_FreeLibrary = FreeLibrary;

typedef HMODULE(WINAPI*  LoadLibraryA_type)(LPCSTR lpLibFileName);
static LoadLibraryA_type Real_LoadLibraryA = LoadLibraryA;

typedef HMODULE(WINAPI*  LoadLibraryExA_type)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
static LoadLibraryExA_type Real_LoadLibraryExA = LoadLibraryExA;

typedef HMODULE(WINAPI*  LoadLibraryW_type)(LPCWSTR lpLibFileName);
static LoadLibraryW_type Real_LoadLibraryW = LoadLibraryW;

typedef HMODULE(WINAPI*  LoadLibraryExW_type)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
static LoadLibraryExW_type Real_LoadLibraryExW = LoadLibraryExW;

static bool s_bD3D11CreateDeviceAttached = false;
static bool s_bD3DX11CompileShaderAttached = false;

static BOOL WINAPI Mine_FreeLibrary(HMODULE hLibModule)
{
    Log(traceMESSAGE, "Detoured FreeLibrary called, HOMDULE: %p", hLibModule);
    BOOL retVal = Real_FreeLibrary(hLibModule);
    DWORD realError = GetLastError();

    CheckOnFreeLibrary();

    SetLastError(realError);
    return retVal;
}

static HMODULE WINAPI Mine_LoadLibraryA(LPCSTR lpLibFileName)
{
    HMODULE res = Real_LoadLibraryA(lpLibFileName);
    Log(traceMESSAGE, "Detoured LoadLibraryA called: %s, HOMDULE: %p\n", lpLibFileName, res);
    DWORD realError = GetLastError();

#ifdef _DEBUG

    if (lstrcmpA(lpLibFileName, "comctl32.dll") != 0)
    {
        // This function uses MessageBox - which will load comctl32.dll if necessary
        // To avoid recursion problems, only call it when a different DLL is being loaded.
        FileUtils::CheckForDebuggerAttach();
    }

#endif

    CheckOnLoadLibrary();

    SetLastError(realError);
    return res;
}

static HMODULE WINAPI Mine_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    HMODULE res = Real_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
    Log(traceMESSAGE, "Detoured LoadLibraryExA called: %s, HOMDULE: %p\n", lpLibFileName, res);
    DWORD realError = GetLastError();

#ifdef _DEBUG

    if (lstrcmpA(lpLibFileName, "comctl32.dll") != 0)
    {
        // This function uses MessageBox - which will load comctl32.dll if necessary
        // To avoid recursion problems, only call it when a differenct DLL is being loaded.
        FileUtils::CheckForDebuggerAttach();
    }

#endif

    CheckOnLoadLibrary();

    SetLastError(realError);
    return res;
}

static HMODULE WINAPI Mine_LoadLibraryW(LPCWSTR lpLibFileName)
{
    HMODULE res = Real_LoadLibraryW(lpLibFileName);
    LogW(traceMESSAGE, L"Detoured LoadLibraryW called: %s, HOMDULE: %p\n", lpLibFileName, res);
    DWORD realError = GetLastError();

#ifdef _DEBUG

    if (lstrcmpW(lpLibFileName, L"comctl32.dll") != 0)
    {
        // This function uses MessageBox - which will load comctl32.dll if necessary
        // To avoid recursion problems, only call it when a differenct DLL is being loaded.
        FileUtils::CheckForDebuggerAttach();
    }

#endif

    CheckOnLoadLibrary();

    SetLastError(realError);
    return res;
}

static HMODULE WINAPI Mine_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    HMODULE res = Real_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
    LogW(traceMESSAGE, L"Detoured LoadLibraryExW called: %s, HOMDULE: %p\n", lpLibFileName, res);
    DWORD realError = GetLastError();

#ifdef _DEBUG

    if (lstrcmpW(lpLibFileName, L"comctl32.dll") != 0)
    {
        // This function uses MessageBox - which will load comctl32.dll if necessary
        // To avoid recursion problems, only call it when a differenct DLL is being loaded.
        FileUtils::CheckForDebuggerAttach();
    }

#endif

    CheckOnLoadLibrary();

    SetLastError(realError);
    return res;
}

bool DetoursAttachLoadLibrary()
{
    if (s_bLoadLibraryAttached)
    {
        return true;
    }

    LONG error = AMDT::BeginHook();

    if (NO_ERROR == error)
    {
        error |= AMDT::HookAPICall(&(PVOID&)Real_LoadLibraryA, Mine_LoadLibraryA);
        error |= AMDT::HookAPICall(&(PVOID&)Real_LoadLibraryExA, Mine_LoadLibraryExA);
        error |= AMDT::HookAPICall(&(PVOID&)Real_LoadLibraryW, Mine_LoadLibraryW);
        error |= AMDT::HookAPICall(&(PVOID&)Real_LoadLibraryExW, Mine_LoadLibraryExW);
        error |= AMDT::HookAPICall(&(PVOID&)Real_FreeLibrary, Mine_FreeLibrary);
        error |= AMDT::EndHook();
    }

    s_bLoadLibraryAttached = true;

    if (NO_ERROR != error)
    {
        return false;
    }

    return true;
}

bool DetoursDetachLoadLibrary()
{
    if (!s_bLoadLibraryAttached)
    {
        return true;
    }

    LONG error = AMDT::BeginHook();

    if (NO_ERROR == error)
    {
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_LoadLibraryA, Mine_LoadLibraryA);
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_LoadLibraryExA, Mine_LoadLibraryExA);
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_LoadLibraryW, Mine_LoadLibraryW);
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_LoadLibraryExW, Mine_LoadLibraryExW);
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_FreeLibrary, Mine_FreeLibrary);
        error |= AMDT::EndHook();
    }

    if (NO_ERROR != error)
    {
        return false;
    }

    /* Restore detoured functions to original values to work around a detours bug */
    Real_LoadLibraryA = LoadLibraryA;
    Real_LoadLibraryExA = LoadLibraryExA;
    Real_LoadLibraryW = LoadLibraryW;
    Real_LoadLibraryExW = LoadLibraryExW;
    Real_FreeLibrary = FreeLibrary;

    s_bLoadLibraryAttached = false;

    return true;
}
