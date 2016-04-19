//==================================================================================
// Copyright (c) 2001-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProcessEnum.cpp
/// \brief file that contains the implemenation of process, module enumeration --
///                to gather the module info.
///
//==================================================================================



#ifndef UNICODE
    #define UNICODE
#endif

#include <windows.h>
#include <winbase.h>

#include <stdio.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <shellapi.h>
#include <stdlib.h>
#include <tchar.h>

///////////////////////////////////////////////////////////////////////////////
// EnumerateProcMod(char *pstring, unsigned int pid)
//  Enumerate process and module. And gather the module info which includes
//  module name, load address, module image size etc. And write module info
//  into snapshot file.
//
//  Snapshot file format is :
//      user module count:      DWORD
//      kernel module count :   DWORD
//      Max app address:        UINT64
//      Min app address:        UINT64
//      (n) user module info
//      (k) kernel module info
//
//  Note: The snapshot file contains 2 sections: user mode module section and kernel
//        mode module section.
//        In each section, it has number of module info records and module info records.
//
//  Param: char* | pstring | snapshot file name
//


static HRESULT SetPrivilege(
    HANDLE hToken,  // token handle
    LPCWSTR Privilege,  // Privilege to enable/disable
    BOOL bEnablePrivilege  // TRUE to enable. FALSE to disable
)
{
    TOKEN_PRIVILEGES tp = { 0 };
    // Initialize everything to zero
    LUID luid;
    DWORD cb = sizeof(TOKEN_PRIVILEGES);

    if (!LookupPrivilegeValueW(NULL, Privilege, &luid))
    {
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;

    if (bEnablePrivilege)
    {
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    }
    else
    {
        tp.Privileges[0].Attributes = 0;
    }

    AdjustTokenPrivileges(hToken, FALSE, &tp, cb, NULL, NULL);

    if (GetLastError() != ERROR_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
}


HRESULT EnumerateProcMod(wchar_t* pstring, unsigned int pid)
{
    HRESULT hr = S_OK;

    // open the snapshot file
    FILE* p_tifile = NULL;
    _wfopen_s(&p_tifile, pstring, L"w+b");

    if (p_tifile == NULL)
    {
        hr = E_FAIL;
        return hr;
    }

    UINT64  t_u64;
    DWORD   t_d32;
    DWORD   modulecount = 0;

    SYSTEM_INFO SystemInfo;
    BOOL isSys64;
#ifdef AMD64
    isSys64 = true;
#else
    IsWow64Process(GetCurrentProcess(), &isSys64);
#endif

    if (isSys64)
    {
        GetNativeSystemInfo(&SystemInfo);
    }
    else
    {
        GetSystemInfo(&SystemInfo);
    }

    // this is at the begin of snapshot file.
    // write temp user mode module counts here, we will update it later.
    fwrite(&modulecount, sizeof(DWORD), 1, p_tifile);

    // write temp kernel module counts here, we will update it later.
    fwrite(&modulecount, sizeof(DWORD), 1, p_tifile);

    // get the max address that user mode application can reach.
    t_u64 = (UINT64) SystemInfo.lpMaximumApplicationAddress;
    fwrite(&t_u64, sizeof(UINT64), 1, p_tifile);
    t_u64 = (UINT64) SystemInfo.lpMinimumApplicationAddress;
    fwrite(&t_u64, sizeof(UINT64), 1, p_tifile);

    HANDLE  hToken = NULL;
    HRESULT tpSetFlag = 0;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        tpSetFlag = SetPrivilege(hToken, SE_DEBUG_NAME, TRUE);
    }
    else
    {
        CloseHandle(hToken);
        hToken = NULL;
    }

    // Take a snapshot of all processes in the system.
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        hr = E_FAIL;
    }

    PROCESSENTRY32W pe32 = {0};
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    // Retrieve information about the first process
    if (S_OK == hr && !Process32FirstW(hProcessSnap, &pe32))
    {
        hr = E_FAIL;
    }

    if (S_OK == hr)
    {
        do
        {
            if (pid != 0 && pid != pe32.th32ProcessID)
            {
                continue;
            }

            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);

            BOOL wow64 = false;

            if (NULL != hProcess)
            {
                IsWow64Process(hProcess, &wow64);
                CloseHandle(hProcess);
            }

            // Take a snapshot of all modules in the specified process.
            HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID);

            if (hModuleSnap == INVALID_HANDLE_VALUE)
            {
                continue;
            }

            MODULEENTRY32W me32;
            me32.dwSize = sizeof(MODULEENTRY32W);

            // Retrieve information about the first module
            if (!Module32FirstW(hModuleSnap, &me32))
            {
                CloseHandle(hModuleSnap);
                continue;
            }

            do
            {
                modulecount++;

                // Process ID
                t_u64 = pe32.th32ProcessID;
                fwrite(&t_u64, sizeof(UINT64), 1, p_tifile);

                // WOW64 (32-bit app)
                fwrite(&wow64, sizeof(BOOL), 1, p_tifile);

                // module load address
                t_u64 = (UINT64) me32.modBaseAddr;
                fwrite(&t_u64, sizeof(UINT64), 1, p_tifile);

                // module load time
                t_d32 = 0;
                fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

                // image base address
                t_u64 = 0;
                fwrite(&t_u64, sizeof(UINT64), 1, p_tifile);

                // module size
                t_u64 = me32.modBaseSize;
                fwrite(&t_u64, sizeof(UINT64), 1, p_tifile);

                // module unload time
                t_d32 = 0;
                fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

                // length of the module name string
                t_d32 = (DWORD) wcslen(me32.szExePath);
                fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

                // module name string
                fwrite((void*) me32.szExePath, sizeof(wchar_t), t_d32, p_tifile);
            }
            while (Module32NextW(hModuleSnap, &me32));

            CloseHandle(hModuleSnap);
        }
        while (Process32NextW(hProcessSnap, &pe32));
    }

    if (hProcessSnap != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hProcessSnap);
    }

    // clean up mess
    if (tpSetFlag)
    {
        SetPrivilege(hToken, SE_DEBUG_NAME, FALSE);
    }

    if (hToken)
    {
        CloseHandle(hToken);
        hToken = NULL;
    }


    // Enumerate driver info
    DWORD t_DriverArraySize = 0;
    LPVOID* pDriverAddrArray = NULL;

#ifdef AMD64
    DWORD cbNeeded;

    if (S_OK == hr && pid == 0)
    {

        EnumDeviceDrivers(NULL, 0, &cbNeeded);

        // calculate driver numbers;
        t_DriverArraySize = cbNeeded / sizeof(LPVOID);

        // allocate space for the driver load addresses.
        pDriverAddrArray = (LPVOID*) malloc(sizeof(LPVOID) * t_DriverArraySize);

        if (NULL != pDriverAddrArray)
        {
            // enumerate driver again to fill in the driver array,
            if (EnumDeviceDrivers(pDriverAddrArray, sizeof(LPVOID) *t_DriverArraySize, &cbNeeded))
            {
                wchar_t szDriverName[1024];

                // for each driver
                for (DWORD i = 0; i < t_DriverArraySize; i++)
                {
                    // write kernel module load address
                    t_u64 = (UINT64) pDriverAddrArray[i];
                    fwrite(&t_u64, sizeof(UINT64), 1, p_tifile);

                    // write kernel module load time
                    t_d32 = 0;
                    fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

                    //                  // write kernel module end address
                    //                  t_u64 = 0;
                    //                  fwrite(&t_u64, sizeof(UINT64), 1, p_tifile);
                    //
                    //                  // write kernel module image base
                    //                  t_u64 = 0;
                    //                  fwrite(&t_u64, sizeof(UINT64), 1, p_tifile);
                    //
                    //                  // write kernel module unload time
                    //                  t_d32 = 0;
                    //                  fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

                    // get driver file name
                    if (!GetDeviceDriverFileNameW(pDriverAddrArray[i], szDriverName, 1024))
                    {
                        wcscpy_s(szDriverName, 1024, L"Unknown Device Driver");
                    }

                    // write length of kernel module name
                    t_d32 = (DWORD) wcslen(szDriverName);
                    fwrite(&t_d32, sizeof(DWORD), 1, p_tifile);

                    // write kernel module name
                    fwrite((void*) szDriverName, sizeof(wchar_t), t_d32, p_tifile);

                }
            }   // if (EnumDeviceDrivers(pDriverAddrArray, t_DriverArraySize, &cbNeeded))
            else
            {
                hr = E_FAIL;
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

#endif

    // update user module count and driver module count
    if (S_OK == hr)
    {
        // before we close the file, we update the module count which is stored at the begin of the file.
        rewind(p_tifile);
        // write user module count here.
        fwrite(&modulecount, sizeof(DWORD), 1, p_tifile);
        // write kernel module count here
        fwrite(&t_DriverArraySize, sizeof(DWORD), 1, p_tifile);
    }

    // close snap shot file.
    if (p_tifile != NULL)
    {
        fclose(p_tifile);
        p_tifile = NULL;
    }

    // clean up mess
    if (pDriverAddrArray != NULL)
    {
        free(pDriverAddrArray);
        pDriverAddrArray = NULL;
    }

    return hr;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
    (void)(hInstance);
    (void)(hPrevInstance);
    (void)(lpCmdLine);
    (void)(nShowCmd);
    int returncode = 0;

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (NULL == argv)
    {
        return 1;
    }

    if (2 == argc)
    {
        if (S_OK != EnumerateProcMod(argv[1], 0))
        {
            returncode = 2;
        }
    }
    else if (3 == argc)
    {
        int n = 0;
        unsigned int pid = 0;
        n = swscanf_s(argv[2], L"%i", &pid);

        if (n != 1)
        {
            returncode = 2;
        }
        else
        {
            if (S_OK != EnumerateProcMod(argv[1], pid))
            {
                returncode = 2;
            }
        }
    }

    // Free memory allocated for CommandLineToArgvW arguments.
    LocalFree(argv);

    return returncode;
}
