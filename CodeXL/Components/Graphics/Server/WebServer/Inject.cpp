//==============================================================================
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Contains functions relating to process injection
//==============================================================================

#if defined (_WIN32)
    #include <Windows.h>
    #include <tlhelp32.h>
    #include <aclapi.h>
#elif defined (_LINUX)
    #include <signal.h>
    #include <proc.h>
#endif

#include "../Common/Logger.h"

#include "Inject.h"

#if !defined ( SID_REVISION )
    #define SID_REVISION ( 1 ) ///< SID definition
#endif

//--------------------------------------------------------------
// Function declarations for functions only needed in this file
//--------------------------------------------------------------
#ifdef _WIN32
    static bool adjust_dacl(HANDLE h, DWORD dwDesiredAccess);
    static HANDLE adv_open_process(DWORD pid, DWORD dwAccessRights);
#endif
//--------------------------------------------------------------
/// Function definitions
/// \param szLibrary
/// \param out
/// \return False if fail, true if success.
//--------------------------------------------------------------
bool get_process_list(const char* szLibrary, ProcessInfoList& out)
{
#ifdef _WIN32
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    // get a snapshot of the current processes
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        Log(logWARNING, "CreateToolhelp32Snapshot ( of processes )\n");
        return false;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    // get the first process
    if (Process32First(hProcessSnap, &pe32) == false)
    {
        Log(logWARNING, "Process32First\n");
        CloseHandle(hProcessSnap);
        return false;
    }

    // check to see if the library is in each process
    do
    {
        ProcessInfo pi;

        if (IsLibraryLoadedInProcess(pe32.th32ProcessID, szLibrary, pi.szPath))
        {
            pi.th32ProcessID = pe32.th32ProcessID;

            // copy data from pe32 into ProcessInfo
            // there is an assumption here that PS_MAX_PATH is >= MAX_PATH
            assert(PS_MAX_PATH >= MAX_PATH);
            strncpy_s(pi.szExeFile, MAX_PATH, pe32.szExeFile, MAX_PATH);

            out.push_back(pi);
        }
    }
    while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

#else

    Proc proc;
    proc.Open();

    while (proc.Read() == true)
    {
        // Ignore the current PerfStudio executable.
        const char* processName = proc.GetProcName();

        if (0 != strcmp(program_invocation_short_name, processName))
        {
            int processId = proc.GetProcessId();
            ProcessInfo pi;

            if (IsLibraryLoadedInProcess(processId, szLibrary, pi.szPath))
            {
                pi.th32ProcessID = processId;

                // copy data from proc_info into ProcessInfo
                strncpy_s(pi.szExeFile, PS_MAX_PATH, processName, PS_MAX_PATH);

                out.push_back(pi);
            }
        }
    }

    proc.Close();

#endif
    return true;
}

/// Determines if the specified library has been loaded into the specified process.
bool IsLibraryLoadedInProcess(unsigned long dwPID, LPCTSTR szLibrary, char* outExePath)
{
#ifdef _WIN32
    HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32;

    enable_privilege(SE_DEBUG_NAME);
    hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwPID);

    if (hModuleSnap == INVALID_HANDLE_VALUE)
    {
        // According to the windows documentation here: https://msdn.microsoft.com/en-us/library/windows/desktop/ms682489%28v=vs.85%29.aspx
        // If the specified process is a 64-bit process and the caller is a 32-bit process, this function fails and the last error code is ERROR_PARTIAL_COPY (299).

        // Log ( logERROR, "CreateToolhelp32Snapshot() failed!: %s\n", szProcessExeFile);
        // Commented out this message for now - it is correct behaviour for this to fail
        // for some processes
        return false;
    }

    me32.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hModuleSnap, &me32) == false)
    {
        Log(logWARNING, "Module32First returned false.\n");
        CloseHandle(hModuleSnap);

        return false;
    }

    // the first module will be the exe file itself, so make sure to copy the exePath here.
    if (outExePath != NULL)
    {
        strncpy_s(outExePath, MAX_PATH, me32.szExePath, MAX_PATH);
    }

    do
    {
        if (lstrcmpi(szLibrary, me32.szModule) == 0)
        {
            // check whether there could be more libs with the same name?
            //            Log(logMESSAGE, "%s is loaded in %s.\n", szLibrary, outExePath);
            return true;
        }
    }
    while (Module32Next(hModuleSnap, &me32));

    CloseHandle(hModuleSnap);
#else
    char fname[PATH_MAX];
    FILE* fp;

    sprintf(fname, "/proc/%ld/maps", dwPID);
    fp = fopen(fname, "r");

    if (!fp)
    {
        return false;
    }

    bool gotFirstFile = false;

    while (!feof(fp))
    {
        char buf[PATH_MAX + 100], perm[5], dev[6], mapName[PATH_MAX];
        unsigned long begin, end, inode, foo;

        if (fgets(buf, sizeof(buf), fp) == 0)
        {
            break;
        }

        mapName[0] = '\0';
        sscanf(buf, "%lx-%lx %4s %lx %5s %ld %s", &begin, &end, perm,
               &foo, dev, &inode, mapName);

        // First file in the list will be the exe file itself, so copy the exePath
        if (outExePath != NULL && gotFirstFile == false)
        {
            strncpy_s(outExePath, PS_MAX_PATH, mapName, PS_MAX_PATH);
            gotFirstFile = true;
        }

        if (mapName[0] == '/')
        {
            if (strstr(mapName, szLibrary) != NULL)
            {
                // found the library so exit
                Log(logMESSAGE, "%s is loaded in %s.\n", szLibrary, outExePath);
                fclose(fp);
                return true;
            }
        }
    }

    fclose(fp);
#endif
    return false;
}

#ifdef _WIN32
static bool adjust_dacl(HANDLE h, DWORD dwDesiredAccess)
{
    SID world = { SID_REVISION, 1, SECURITY_WORLD_SID_AUTHORITY, 0 };

    EXPLICIT_ACCESS ea =
    {
        0,
        SET_ACCESS,
        NO_INHERITANCE,
        {
            0,
            NO_MULTIPLE_TRUSTEE,
            TRUSTEE_IS_SID,
            TRUSTEE_IS_USER,
            0
        }
    };

    ACL* pdacl = 0;
    DWORD err = SetEntriesInAcl(1, &ea, 0, &pdacl);

    ea.grfAccessPermissions = dwDesiredAccess;
    ea.Trustee.ptstrName = (LPTSTR)(&world);

    if (err == ERROR_SUCCESS)
    {
        err = SetSecurityInfo(h, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, pdacl, 0);
        LocalFree(pdacl);

        return (err == ERROR_SUCCESS);
    }
    else
    {
        Log(logWARNING, "adjust_dacl\n");

        return false;
    }
}

static HANDLE adv_open_process(DWORD pid, DWORD dwAccessRights)
{
    HANDLE hProcess = OpenProcess(dwAccessRights, FALSE, pid);

    if (hProcess == NULL)
    {
        HANDLE hpWriteDAC = OpenProcess(WRITE_DAC, FALSE, pid);

        if (hpWriteDAC == NULL)
        {
            HANDLE htok;
            TOKEN_PRIVILEGES tpOld;

            if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &htok) == false)
            {
                return (FALSE);
            }

            if (enable_token_privilege(htok, SE_TAKE_OWNERSHIP_NAME, &tpOld))
            {
                HANDLE hpWriteOwner = OpenProcess(WRITE_OWNER, FALSE, pid);

                if (hpWriteOwner != NULL)
                {
                    BYTE buf[512];
                    DWORD cb = sizeof buf;

                    if (GetTokenInformation(htok, TokenUser, buf, cb, &cb))
                    {
                        DWORD err = SetSecurityInfo(hpWriteOwner, SE_KERNEL_OBJECT, OWNER_SECURITY_INFORMATION, ((TOKEN_USER*)(buf))->User.Sid, 0, 0, 0);

                        if (err == ERROR_SUCCESS)
                        {
                            if (!DuplicateHandle(GetCurrentProcess(), hpWriteOwner, GetCurrentProcess(), &hpWriteDAC, WRITE_DAC, FALSE, 0))
                            {
                                hpWriteDAC = NULL;
                            }
                        }
                    }

                    CloseHandle(hpWriteOwner);
                }

                AdjustTokenPrivileges(htok, FALSE, &tpOld, 0, 0, 0);
            }

            CloseHandle(htok);
        }

        if (hpWriteDAC)
        {
            adjust_dacl(hpWriteDAC, dwAccessRights);

            if (!DuplicateHandle(GetCurrentProcess(), hpWriteDAC, GetCurrentProcess(), &hProcess, dwAccessRights, FALSE, 0))
            {
                hProcess = NULL;
            }

            CloseHandle(hpWriteDAC);
        }
    }

    return hProcess;
}
#endif // def _WIN32

//----------------------------------------------------------------
// Kills a given DLL from a given process
//----------------------------------------------------------------
bool KillProcess(unsigned long pid)
{
#ifdef _WIN32
    HANDLE hp = adv_open_process(pid, PROCESS_TERMINATE);

    if (hp != NULL)
    {
        bool bRet = (TerminateProcess(hp, 1) == TRUE);
        CloseHandle(hp);

        return bRet;
    }

#else
    kill(pid, SIGTERM);
#endif
    return false;
}
