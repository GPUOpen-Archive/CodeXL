//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains the functions to detour CreateProcess.
//==============================================================================

#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include "..\Common\Windows\RefTracker.h"
#include "..\Common\Logger.h"
#include "DetourCreateProcess.h"
#include "Interceptor.h"
#include "DLLMain.h"

using std::string;
using std::wstring;
using std::vector;
using std::stringstream;
using std::wstringstream;
using namespace GPULogger;

static bool s_bCreateProcessAttached = false;

// Skip ocl compiler exes
static vector<string> s_vecSkipList;
static vector<wstring> s_vecSkipListW;

static bool IsInList(const string& str)
{
    string tmp = str;

    // remove tailing "
    while (tmp.length() > 0 && tmp[tmp.length() - 1] == '"')
    {
        tmp = tmp.substr(0, tmp.length() - 1);
    }

    for (vector<string>::iterator it = s_vecSkipList.begin(); it != s_vecSkipList.end(); it++)
    {
        size_t idx = tmp.find(*it);

        if (idx != string::npos)
        {
            if (idx + it->length() == tmp.length())
            {
                if (idx != 0)
                {
                    // handle situation like: myappas.exe if we skip as.exe
                    if (tmp[ idx - 1 ] == '\\' || tmp[ idx - 1 ] == '/')
                    {
                        return true;
                    }
                }
                else
                {
                    // token appear at end of the string
                    return true;
                }
            }
        }
    }

    return false;
}

static bool IsInListW(const wstring& str)
{
    wstring tmp = str;

    // remove tailing "
    while (tmp.length() > 0 && tmp[tmp.length() - 1] == L'"')
    {
        tmp = tmp.substr(0, tmp.length() - 1);
    }

    for (vector<wstring>::iterator it = s_vecSkipListW.begin(); it != s_vecSkipListW.end(); it++)
    {
        size_t idx = tmp.find(*it);

        if (idx != wstring::npos)
        {
            if (idx + it->length() == tmp.length())
            {
                if (idx != 0)
                {
                    // handle situation like: myappas.exe if we skip as.exe
                    if (tmp[ idx - 1 ] == L'\\' || tmp[ idx - 1 ] == L'/')
                    {
                        return true;
                    }
                }
                else
                {
                    // token appear at end of the string
                    return true;
                }
            }
        }
    }

    return false;
}

static bool CheckSkip(const string& str)
{
    stringstream ss;
    ss << str;
    string tok;

    while (!ss.eof())
    {
        ss >> tok;

        if (IsInList(tok))
        {
            return true;
        }
    }

    return false;
}

static bool CheckSkipW(const wstring& str)
{
    wstringstream ss;
    ss << str;
    wstring tok;

    while (!ss.eof())
    {
        ss >> tok;

        if (IsInListW(tok))
        {
            LogW(traceMESSAGE, L"Skip app: %s", str.c_str());
            return true;
        }
    }

    return false;
}


static RefTrackerCounter s_dwInsideWrapper;

typedef BOOL (WINAPI* CreateProcessA_type)(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);

CreateProcessA_type Real_CreateProcessA = CreateProcessA;

BOOL WINAPI Mine_CreateProcessA(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation)
{
    Log(traceMESSAGE, "Detoured CreateProcessA called\n");
    BOOL ret;

    std::string strApplicationName = (lpApplicationName != NULL) ? std::string(lpApplicationName) : "";
    std::string strCommandLine     = (lpCommandLine != NULL)     ? std::string(lpCommandLine)     : "";

    bool bSkip = CheckSkip(strApplicationName) ||
                 CheckSkip(strCommandLine);

    if (s_dwInsideWrapper == 0 && !bSkip)
    {
        RefTracker rf(&s_dwInsideWrapper);
        Log(traceMESSAGE, "Calling DetourCreateProcessWithDll(ASCII) %s %s", lpApplicationName, lpCommandLine);
        ret = AMDT::CreateProcessAndInjectDllA(lpApplicationName,
                                               lpCommandLine,
                                               lpProcessAttributes,
                                               lpThreadAttributes,
                                               bInheritHandles,
                                               dwCreationFlags,
                                               lpEnvironment,
                                               lpCurrentDirectory,
                                               lpStartupInfo,
                                               lpProcessInformation,
                                               g_strMicroDllPath.c_str());

        if (!ret)
        {
            // If we failed to detour create process, call real one so that app can continue
            Log(logERROR, "Failed to detour create process[ app name:%s, arg = %s ]", lpApplicationName, lpCommandLine);
            return Real_CreateProcessA(lpApplicationName,
                                       lpCommandLine,
                                       lpProcessAttributes,
                                       lpThreadAttributes,
                                       bInheritHandles,
                                       dwCreationFlags,
                                       lpEnvironment,
                                       lpCurrentDirectory,
                                       lpStartupInfo,
                                       lpProcessInformation);
        }
    }
    else
    {
        ret = Real_CreateProcessA(lpApplicationName,
                                  lpCommandLine,
                                  lpProcessAttributes,
                                  lpThreadAttributes,
                                  bInheritHandles,
                                  dwCreationFlags,
                                  lpEnvironment,
                                  lpCurrentDirectory,
                                  lpStartupInfo,
                                  lpProcessInformation);
    }

    return ret;
}

typedef BOOL (WINAPI* CreateProcessW_type)(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);

CreateProcessW_type Real_CreateProcessW = CreateProcessW;

BOOL WINAPI Mine_CreateProcessW(LPCWSTR lpApplicationName,
                                LPWSTR lpCommandLine,
                                LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                BOOL bInheritHandles,
                                DWORD dwCreationFlags,
                                LPVOID lpEnvironment,
                                LPCWSTR lpCurrentDirectory,
                                LPSTARTUPINFOW lpStartupInfo,
                                LPPROCESS_INFORMATION lpProcessInformation)
{
    Log(traceMESSAGE, "Detoured CreateProcessW called\n");
    BOOL ret;

    wstring strApplicationName = (lpApplicationName != NULL) ? wstring(lpApplicationName) : L"";
    wstring strCommandLine     = (lpCommandLine != NULL)     ? wstring(lpCommandLine)     : L"";

    bool bSkip = CheckSkipW(strApplicationName) ||
                 CheckSkipW(strCommandLine);

    if (s_dwInsideWrapper == 0 && !bSkip)
    {
        RefTracker rf(&s_dwInsideWrapper);
        LogW(traceMESSAGE, L"Calling DetourCreateProcessWithDll(Unicode) %s %s", lpApplicationName, lpCommandLine);
        ret = AMDT::CreateProcessAndInjectDllW(lpApplicationName,
                                               lpCommandLine,
                                               lpProcessAttributes,
                                               lpThreadAttributes,
                                               bInheritHandles,
                                               dwCreationFlags,
                                               lpEnvironment,
                                               lpCurrentDirectory,
                                               lpStartupInfo,
                                               lpProcessInformation,
                                               g_strMicroDllPath.c_str());

        if (!ret)
        {
            // If we failed to detour create process, call real one so that app can continue
            LogW(logERROR, L"Failed to detour create process[ app name:%s, arg = %s ]", lpApplicationName, lpCommandLine);
            return Real_CreateProcessW(lpApplicationName,
                                       lpCommandLine,
                                       lpProcessAttributes,
                                       lpThreadAttributes,
                                       bInheritHandles,
                                       dwCreationFlags,
                                       lpEnvironment,
                                       lpCurrentDirectory,
                                       lpStartupInfo,
                                       lpProcessInformation);
        }
    }
    else
    {
        ret = Real_CreateProcessW(lpApplicationName,
                                  lpCommandLine,
                                  lpProcessAttributes,
                                  lpThreadAttributes,
                                  bInheritHandles,
                                  dwCreationFlags,
                                  lpEnvironment,
                                  lpCurrentDirectory,
                                  lpStartupInfo,
                                  lpProcessInformation);
    }

    return ret;
}

bool DetoursAttachCreateProcess()
{
    if (s_bCreateProcessAttached)
    {
        return true;
    }

    // CL
    s_vecSkipList.push_back(string("as.exe"));
    s_vecSkipList.push_back(string("clc.exe"));
    s_vecSkipList.push_back(string("as"));
    s_vecSkipList.push_back(string("clc"));
    s_vecSkipList.push_back(string("ld"));
    s_vecSkipList.push_back(string("ld.exe"));
    s_vecSkipList.push_back(string("amdocl_ld.exe"));
    s_vecSkipList.push_back(string("amdocl_ld"));
    s_vecSkipList.push_back(string("amdocl_ld64.exe"));
    s_vecSkipList.push_back(string("amdocl_ld64"));
    s_vecSkipList.push_back(string("amdocl_ld32.exe"));
    s_vecSkipList.push_back(string("amdocl_ld32"));
    s_vecSkipList.push_back(string("amdocl_as.exe"));
    s_vecSkipList.push_back(string("amdocl_as"));
    s_vecSkipList.push_back(string("amdocl_as64.exe"));
    s_vecSkipList.push_back(string("amdocl_as64"));
    s_vecSkipList.push_back(string("amdocl_as32.exe"));
    s_vecSkipList.push_back(string("amdocl_as32"));

    // .NET
    s_vecSkipList.push_back(string("csc.exe"));
    s_vecSkipList.push_back(string("cvtres.exe"));

    // CL
    s_vecSkipListW.push_back(wstring(L"as.exe"));
    s_vecSkipListW.push_back(wstring(L"clc.exe"));
    s_vecSkipListW.push_back(wstring(L"as"));
    s_vecSkipListW.push_back(wstring(L"clc"));
    s_vecSkipListW.push_back(wstring(L"ld"));
    s_vecSkipListW.push_back(wstring(L"ld.exe"));
    s_vecSkipListW.push_back(wstring(L"amdocl_ld.exe"));
    s_vecSkipListW.push_back(wstring(L"amdocl_ld"));
    s_vecSkipListW.push_back(wstring(L"amdocl_ld64.exe"));
    s_vecSkipListW.push_back(wstring(L"amdocl_ld64"));
    s_vecSkipListW.push_back(wstring(L"amdocl_ld32.exe"));
    s_vecSkipListW.push_back(wstring(L"amdocl_ld32"));
    s_vecSkipListW.push_back(wstring(L"amdocl_as.exe"));
    s_vecSkipListW.push_back(wstring(L"amdocl_as"));
    s_vecSkipListW.push_back(wstring(L"amdocl_as64.exe"));
    s_vecSkipListW.push_back(wstring(L"amdocl_as64"));
    s_vecSkipListW.push_back(wstring(L"amdocl_as32.exe"));
    s_vecSkipListW.push_back(wstring(L"amdocl_as32"));

    // .NET
    s_vecSkipListW.push_back(wstring(L"csc.exe"));
    s_vecSkipListW.push_back(wstring(L"cvtres.exe"));

    LONG error = AMDT::BeginHook();

    if (NO_ERROR == error)
    {
        error |= AMDT::HookAPICall(&(PVOID&)Real_CreateProcessW, Mine_CreateProcessW);
        error |= AMDT::HookAPICall(&(PVOID&)Real_CreateProcessA, Mine_CreateProcessA);
        error |= AMDT::EndHook();
    }

    s_bCreateProcessAttached = true;

    if (NO_ERROR != error)
    {
        return false;
    }

    return true;
}

bool DetoursDetachCreateProcess()
{
    if (!s_bCreateProcessAttached)
    {
        return true;
    }

    LONG error = AMDT::BeginHook();

    if (NO_ERROR == error)
    {
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_CreateProcessW, Mine_CreateProcessW);
        error |= AMDT::UnhookAPICall(&(PVOID&)Real_CreateProcessA, Mine_CreateProcessA);
        error |= AMDT::EndHook();
    }

    s_bCreateProcessAttached = false;

    if (NO_ERROR != error)
    {
        return false;
    }

    return true;
}
