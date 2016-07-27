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
#include <AMDTOSWrappers/Include/osProcess.h>

#include "Inject.h"

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


//----------------------------------------------------------------
// Kills a given DLL from a given process
//----------------------------------------------------------------
bool KillProcess(unsigned long pid)
{
    bool retVal = osTerminateProcess(pid, 1, true, true);

    return retVal;
}
