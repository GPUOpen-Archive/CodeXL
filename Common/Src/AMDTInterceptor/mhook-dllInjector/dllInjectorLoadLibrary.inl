//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  DLL Injection using LoadLibrary class implementation
//=============================================================================

#include <stdlib.h>
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <strsafe.h>

#include "dllInjector.h"

#pragma comment(lib,"Advapi32.lib")

#ifdef DEBUG_PRINT
#define DEBUG_OUTPUT(x) printf x
#else
#define DEBUG_OUTPUT(x)
#endif

typedef BOOL(WINAPI* CreateProcessA_type)(
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

typedef BOOL(WINAPI* CreateProcessW_type)(
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

// size of buffer required to store command line
static const int COMMAND_LINE_SIZE = 1024;

// ---------------------------------------------------------------------------
/// A class to do dll injection via LoadLibrary. The name of the dll to be
/// loaded is written into the target application memory space and a remote
/// thread is set up in the target application to call LoadLibrary with the
/// dll to be injected. This method works in most cases but it's an easy
/// injection method to detect and some copy-protection schemes are aware of
/// it.
// ---------------------------------------------------------------------------
class DLLInjectorLoadLibrary : public DLLInjectorImpl
{
public:
    /// constructor
    DLLInjectorLoadLibrary()
    {
    }

    /// destructor
    virtual ~DLLInjectorLoadLibrary()
    {
    }

    // ---------------------------------------------------------------------------
    /// Determine if this function was called from Rundll32.exe. If it was,
    /// chances are it is being used to inject our DLL into the target application.
    /// This function is called at the very start of DllMain in the Graphics Server
    /// Interceptor Dll and ensures that DllMain isn't called for rundll.exe
    /// processes; the rundll32.exe process needs to terminate as quickly as
    /// possible and returning true here will inform the OS that the dll containing
    /// this function (MicroDLL.dll) needs to be unloaded.
    /// \returns true if started from rundll32.dll, false if not
    // ---------------------------------------------------------------------------
    virtual bool CalledFromRundll()
    {
        char modulename[MAX_PATH];
        GetModuleFileNameA(NULL, modulename, MAX_PATH);

        if (strstr(modulename, "rundll") != NULL)
        {
            return true;
        }

        return false;
    }

    // ---------------------------------------------------------------------------
    /// Inject a dll into a suspended process. This is an external function called
    /// from rundll32.exe when process-hopping between 64 and 32 bit processes.
    /// As such, the arguments to this function will be provided by rundll32
    /// \param lpszCmdLine The command line arguments passed to this function. This
    ///        consists of a string containing the process ID of the application
    ///        to inject into and the full path of the dll to inject.
    // ---------------------------------------------------------------------------
    virtual void Inject(LPSTR lpszCmdLine)
    {
        char modulename[MAX_PATH];
        GetModuleFileNameA(NULL, modulename, MAX_PATH);

        // find end of process ID and put a '\0' marker in
        char* lpDllName = lpszCmdLine;
        for (int i = 0; i < 6; i++)
        {
            if (*lpDllName == ' ')
            {
                *lpDllName++ = '\0';
                break;
            }
            else
            {
                lpDllName++;
            }
        }

        int processID = atoi(lpszCmdLine);

        // inject the DLL into the target process
        if (false == InjectDLLIntoProcess(lpDllName, processID))
        {
        }

        ExitProcess(0);
    }

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
    virtual BOOL WINAPI CreateProcessAndInjectDllA(_In_opt_ LPCSTR lpApplicationName,
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
        // Get bitness of current module
        char currentFileName[MAX_PATH];
        GetModuleFileNameA(NULL, currentFileName, MAX_PATH);

        DEBUG_OUTPUT(("CreateProcessAndInjectDllA running from process name >>%s<<\n", currentFileName));

        DWORD currentType;
        GetBinaryTypeA(currentFileName, &currentType);

        // Get bitness of application name (if it isn't NULL)
        DWORD appType = currentType;

        if (lpApplicationName != NULL && strlen(lpApplicationName) > 0)
        {
            DEBUG_OUTPUT(("CreateProcessAndInjectDllA: application name >>%s<<\n", lpApplicationName));
            GetBinaryTypeA(lpApplicationName, &appType);
        }
        else
        {
            // if there's no application name, then get it from the command line.
            // Take into account whether the command line is in quotes and ignore spaces if so
            char buffer[MAX_PATH];
            buffer[0] = '\0';

            // if the command line string contains a '.exe' string, assume this is the application name and use that
            const char* pExe = strstr(lpCommandLine, ".exe");

            if (pExe != NULL)
            {
                // find length of string up to start of '.exe'
                int length = (int)(pExe - lpCommandLine);

                // add length of '.exe'
                length += 4;

                // copy the application name from the command line string, ignoring '"' characters
                int index = 0;

                for (int loop = 0; loop < length; loop++)
                {
                    char currentChar = lpCommandLine[loop];

                    if (currentChar != '\"')
                    {
                        buffer[index++] = currentChar;
                    }

                    buffer[index] = '\0';
                }
            }
            else
            {
                int insideQuotes = 0;
                int index = 0;

                for (int loop = 0; loop < MAX_PATH; loop++)
                {
                    char currentChar = lpCommandLine[loop];

                    if (currentChar == '\"')
                    {
                        // it's a '"', so toggle the quote flag. Don't copy character to the buffer
                        insideQuotes = 1 - insideQuotes;   // toggle value 0 -> 1, 1 -> 0
                    }
                    else if (currentChar == ' ')
                    {
                        // it's a space character, so decide what to do. If inside a quoted string, treat it as a normal
                        // character. Otherwise, use it as a string terminator
                        if (insideQuotes == 0)
                        {
                            buffer[index] = '\0';
                            break;
                        }
                        else
                        {
                            buffer[index++] = currentChar;
                        }
                    }
                    else
                    {
                        // copy the character
                        buffer[index++] = currentChar;
                    }
                }
            }

            if (buffer != NULL)
            {
                DEBUG_OUTPUT(("CreateProcessAndInjectDllA: command line exe >>%s<<\n", buffer));
                GetBinaryTypeA(buffer, &appType);
            }
        }

        BOOL result = FALSE;

        // Create the process suspended
        DWORD dwFlags = dwCreationFlags | CREATE_SUSPENDED;
        CreateProcessA_type pfCreateProcess = CreateProcessA;
        result = pfCreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwFlags,
            lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

        if (result == TRUE)
        {
            if (appType == currentType)
            {
                // This module and the application to be created are the same bitness, so just inject the dll into the new process
                // and resume it when done
                InjectDLLIntoProcess(lpDllName, lpProcessInformation->hProcess);
                ResumeThread(lpProcessInformation->hThread);
            }
            else
            {
                // This module and the application to be created are different bitness, so use rundll to call a function in MicroDLL to
                // do the dll injection. Resume the application when done.
                StartRundllA(lpDllName, lpProcessInformation->dwProcessId);
                ResumeThread(lpProcessInformation->hThread);
            }
        }

        return result;
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
    virtual BOOL WINAPI CreateProcessAndInjectDllW(_In_opt_ LPCWSTR lpApplicationName,
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
        DEBUG_OUTPUT(("CreateProcessAndInjectDllW\n"));

        // Get bitness of current module
        WCHAR currentFileName[MAX_PATH];
        GetModuleFileNameW(NULL, currentFileName, MAX_PATH);
        DWORD currentType;
        GetBinaryTypeW(currentFileName, &currentType);

        // Get bitness of application name
        DWORD appType = currentType;

        if (lpApplicationName != NULL && lstrlenW(lpApplicationName) > 0)
        {
            DEBUG_OUTPUT(("CreateProcessAndInjectDllW: application name >>%S<<\n", lpApplicationName));
            GetBinaryTypeW(lpApplicationName, &appType);
        }
        else
        {
            // if there's no application name, then get it from the command line.
            // Take into account whether the command line is in quotes and ignore spaces if so
            WCHAR buffer[MAX_PATH];
            buffer[0] = '\0';

            // if the command line string contains a '.exe' string, assume this is the application name and use that
            const WCHAR* pExe = wcsstr(lpCommandLine, L".exe");

            if (pExe != NULL)
            {
                // find length of string up to start of '.exe'
                int length = (int)(pExe - lpCommandLine);

                // add length of '.exe'
                length += 4;

                // copy the application name from the command line string, ignoring '"' characters
                int index = 0;

                for (int loop = 0; loop < length; loop++)
                {
                    WCHAR currentChar = lpCommandLine[loop];

                    if (currentChar != '\"')
                    {
                        buffer[index++] = currentChar;
                    }

                    buffer[index] = '\0';
                }
            }
            else
            {
                int insideQuotes = 0;
                int index = 0;

                for (int loop = 0; loop < MAX_PATH; loop++)
                {
                    WCHAR currentChar = lpCommandLine[loop];

                    if (currentChar == '\"')
                    {
                        // it's a '"', so toggle the quote flag. Don't copy character to the buffer
                        insideQuotes = 1 - insideQuotes;   // toggle value 0 -> 1, 1 -> 0
                    }
                    else if (currentChar == ' ')
                    {
                        // it's a space character, so decide what to do. If inside a quoted string, treat it as a normal
                        // character. Otherwise, use it as a string terminator
                        if (insideQuotes == 0)
                        {
                            buffer[index] = '\0';
                            break;
                        }
                        else
                        {
                            buffer[index++] = currentChar;
                        }
                    }
                    else
                    {
                        // copy the character
                        buffer[index++] = currentChar;
                    }
                }
            }

            if (buffer != NULL)
            {
                DEBUG_OUTPUT(("CreateProcessAndInjectDllW: command line exe >>%S<<\n", buffer));
                GetBinaryTypeW(buffer, &appType);
            }
        }

        BOOL result = FALSE;
        // Create the process suspended
        DWORD dwFlags = dwCreationFlags | CREATE_SUSPENDED;
        CreateProcessW_type pfCreateProcess = CreateProcessW;
        result = pfCreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwFlags,
            lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

        if (result == TRUE)
        {
            if (appType == currentType)
            {
                // This module and the application to be created are the same bitness, so just inject the dll into the new process
                // and resume it when done
                InjectDLLIntoProcess(lpDllName, lpProcessInformation->hProcess);
                ResumeThread(lpProcessInformation->hThread);
            }
            else
            {
                // This module and the application to be created are different bitness, so use rundll to call a function in MicroDLL to
                // do the dll injection. Resume the application when done.
                StartRundllW(lpDllName, lpProcessInformation->dwProcessId);
                ResumeThread(lpProcessInformation->hThread);
            }
        }

        return result;
    }

private:
    // ---------------------------------------------------------------------------
    /// Loads the DLL into the target process via a process handle
    /// \param [in] dllPath Full path and name of dll to be injected
    /// \param [in] hProcess Handle to target application process
    ///        into.
    /// \return true if successful, false otherwise.
    // ---------------------------------------------------------------------------
    bool InjectDLLIntoProcess(LPCSTR dllPath, HANDLE hProcess)
    {
        bool retVal = InjectDLLPathIntoTargetProcess(hProcess, dllPath);
        if (retVal)
        {
            retVal = LoadDLLIntoTargetProcess(hProcess);
        }
        return retVal;
    }

    // ---------------------------------------------------------------------------
    /// Does all the work in this class - loads the DLL into the target process.
    /// \param [in] dllPath Full path and name of dll to be injected
    /// \param [in] processID The process ID of the target application to inject
    ///        into.
    /// \return true if successful, false otherwise.
    // ---------------------------------------------------------------------------
    bool InjectDLLIntoProcess(LPCSTR dllPath, DWORD processID)
    {
        HANDLE targetProcHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, processID);
        bool retVal = InjectDLLIntoProcess(dllPath, targetProcHandle);
        CloseHandle(targetProcHandle);
        return retVal;
    }

    // ---------------------------------------------------------------------------
    /// Injects the DLL path into the target process.
    /// \param [in] targetProcHandle Handle to target application process
    /// \param [in] dllPath Full path and name of dll to be injected
    /// \return true if successful, false otherwise.
    // ---------------------------------------------------------------------------
    bool InjectDLLPathIntoTargetProcess(HANDLE targetProcHandle, LPCSTR dllPath)
    {
        bool retVal = false;
        size_t sizeOfMemoryToInject = strlen(dllPath);
        if (sizeOfMemoryToInject == 0)
        {
            DEBUG_OUTPUT(("ERROR: dllPath length is 0\n"));
            return retVal;
        }

        retVal = AllocateSpaceForInjectedMemory(targetProcHandle, sizeOfMemoryToInject);

        if (retVal)
        {
            retVal = InjectMemory(targetProcHandle, dllPath, sizeOfMemoryToInject);
        }

        return retVal;
    }

    // ---------------------------------------------------------------------------
    /// Allocated memory that will contain the injected memory
    /// This memory will be accessible by the target process.
    /// \param [in] targetProcHandle Handle to target application process
    /// \param [in] sizeOfMemoryToInject Size of memory to inject, in bytes
    /// \return true if successful, false otherwise.
    // ---------------------------------------------------------------------------
    bool AllocateSpaceForInjectedMemory(HANDLE targetProcHandle, size_t sizeOfMemoryToInject)
    {
        bool retVal = false;

        DEBUG_OUTPUT(("Allocating space for the path of the DLL\n"));

        m_pTargetMemoryAddress = VirtualAllocEx(targetProcHandle, NULL, sizeOfMemoryToInject, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (m_pTargetMemoryAddress == NULL)
        {
            DEBUG_OUTPUT(("ERROR: VirtualAllocEx Failed [%u]\n", GetLastError()));
        }
        else
        {
            retVal = true;
        }
        return retVal;
    }

    // ---------------------------------------------------------------------------
    /// Injects the memory into the target process.
    /// This function should be called after allocateSpaceForInjectedMemory()
    /// was called.
    /// \param [in] targetProcHandle Handle to target application process
    /// \param [in] dllPath Full path and name of dll to be injected
    /// \param [in] sizeOfMemoryToInject Size of memory to inject, in bytes
    /// \return true if successful, false otherwise.
    // ---------------------------------------------------------------------------
    bool InjectMemory(HANDLE targetProcHandle, LPCSTR dllPath, size_t sizeOfMemoryToInject)
    {
        bool retVal = false;

        // Verify that we have allocated space for the memory to inject:
        if (m_pTargetMemoryAddress)
        {
            // Inject the memory into the target process:
            SIZE_T numberOfBytesWritten = 0;

            int rc = WriteProcessMemory(targetProcHandle, m_pTargetMemoryAddress, dllPath, sizeOfMemoryToInject, &numberOfBytesWritten);
            retVal = ((rc != 0) && (numberOfBytesWritten == sizeOfMemoryToInject));

            if (retVal == false)
            {
                DEBUG_OUTPUT(("ERROR: WriteProcessMemory Failed [%u]\n", GetLastError()));
            }
        }
        return retVal;
    }

    // ---------------------------------------------------------------------------
    /// Loads the DLL into the target process.
    /// \param [in] targetProcHandle Handle to target application process
    /// \return true if successful, false otherwise.
    // ---------------------------------------------------------------------------
    bool LoadDLLIntoTargetProcess(HANDLE targetProcHandle)
    {
        static LPCSTR static_LoadLibraryFunctionName =
#ifdef UNICODE
            "LoadLibraryW";
#else
            "LoadLibraryA";
#endif
        bool retVal = false;

        DEBUG_OUTPUT(("Looking for LoadLibrary in kernel32\n"));
        HMODULE handle = GetModuleHandle(TEXT("kernel32.dll"));
        FARPROC fpLoadLibrary = NULL;
        if (handle != NULL)
        {
            fpLoadLibrary = GetProcAddress(handle, static_LoadLibraryFunctionName);
            if (fpLoadLibrary == NULL)
            {
                DEBUG_OUTPUT(("ERROR: Failed to find LoadLibrary in Kernel32. Exiting...\n"));
                return retVal;
            }
        }
        else
        {
            DEBUG_OUTPUT(("ERROR: Failed to get module handle for Kernel32. Exiting...\n"));
            return retVal;
        }

        DEBUG_OUTPUT(("\t\tFound at %p\n", fpLoadLibrary));

        HANDLE remoteThreadHandle = NULL;
        DWORD remoteThreadId = 0;

        // Create a remote thread that loads the DLL into the target process:
        remoteThreadHandle = CreateRemoteThread(targetProcHandle, NULL, 0, (LPTHREAD_START_ROUTINE)fpLoadLibrary, m_pTargetMemoryAddress, 0, &remoteThreadId);

        if (remoteThreadHandle == NULL)
        {
            DEBUG_OUTPUT(("ERROR: CreateRemoteThread Failed [%u]. Exiting....\n", GetLastError()));
            return retVal;
        }
        else
        {
            // Wait for the thread to end its task:
            WaitForSingleObject(remoteThreadHandle, INFINITE);

            // Get the thread exit code:
            // (This is actually the LoadLibrary return code)
            DWORD threadExitCode = 0;
            BOOL rc = GetExitCodeThread(remoteThreadHandle, (LPDWORD)&threadExitCode);

            if (rc)
            {
                // If the remote LoadLibrary succeeded:
                if (threadExitCode != NULL)
                {
                    retVal = true;
                }
            }

            // Clean up:
            CloseHandle(remoteThreadHandle);
        }

        return retVal;
    }

    void* m_pTargetMemoryAddress;    ///< The address of the injected memory (In target process address space)

    // ---------------------------------------------------------------------------
    /// StartRundll Functions:
    /// It is only possible to inject a 32-bit dll into a 32-bit process from
    /// another 32-bit process. The same is true for 64-bit; all dll's and
    /// processes must be the same bitness. It isn't possible to inject a 64-bit
    /// process with a 64-bit dll from a 32-bit process.
    /// The issue here is how can a 64-bit process be injected with a 64-bit
    /// dll from a 32-bit process (in the case of steam starting 64-bit games).
    /// The way of getting around this is to have the 32-bit process create a second
    /// 64-bit process and have this secondary process inject the 64-bit dll into
    /// the 64-bit target application.
    /// The method employed here uses rundll32.exe as the secondary process.
    /// This uses LoadLibrary to load the dll to be injected. It then calls a
    /// method, InjectDLL(), which takes the dll name and a process ID as
    /// parameters and does the dll injection
    // ---------------------------------------------------------------------------

    // ---------------------------------------------------------------------------
    /// Create a process for rundll32.dll and use this to call a function inside
    /// the dll specified. This is needed when injecting a 64 bit process from a
    /// 32 bit process. This is the non-unicode version.
    /// \param [in] lpDllName The full path and name of the dll to be injected.
    /// \param [in] appProcessID The process ID of the application to be injected into.
    // ---------------------------------------------------------------------------
    void StartRundllA(LPCSTR lpDllName, DWORD appProcessID)
    {
        char exeName[COMMAND_LINE_SIZE];
        char commandLine[COMMAND_LINE_SIZE];
        char winDir[MAX_PATH];
        PROCESS_INFORMATION pi;
        STARTUPINFOA si;
        DWORD nLen = GetEnvironmentVariableA("WINDIR", winDir, ARRAYSIZE(winDir));

        if (nLen == 0)
        {
            DEBUG_OUTPUT(("Can't find windows folder\n"));
        }

#ifdef X64
        const char *sysDir = "syswow64";
#else
        const char *sysDir = "system32";
#endif

        sprintf_s(exeName, COMMAND_LINE_SIZE, "%s\\%s\\rundll32.exe", winDir, sysDir);
        sprintf_s(commandLine, COMMAND_LINE_SIZE, "rundll32.exe \"%s\",InjectDLL %d %s", lpDllName, appProcessID, lpDllName);

        ZeroMemory(&pi, sizeof(pi));
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        CreateProcessA_type pfCreateProcess = CreateProcessA;
        pfCreateProcess(exeName, commandLine, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
        ResumeThread(pi.hThread);

        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    // ---------------------------------------------------------------------------
    /// Create a process for rundll32.dll and use this to call a function inside
    /// the dll specified. This is needed when injecting a 64 bit process from a
    /// 32 bit process. This is the unicode (wide char) version.
    /// \param [in] lpDllName The full path and name of the dll to be injected.
    /// \param [in] appProcessID The process ID of the application to be injected into.
    // ---------------------------------------------------------------------------
    void StartRundllW(LPCSTR lpDllName, DWORD appProcessID)
    {
        WCHAR exeName[COMMAND_LINE_SIZE];
        WCHAR commandLine[COMMAND_LINE_SIZE];
        WCHAR winDir[MAX_PATH];
        PROCESS_INFORMATION pi;
        STARTUPINFOW si;
        DWORD nLen = GetEnvironmentVariableW(L"WINDIR", winDir, ARRAYSIZE(winDir));

        if (nLen == 0)
        {
            DEBUG_OUTPUT(("Can't find windows folder\n"));
        }

#ifdef X64
        const WCHAR *sysDir = L"syswow64";
#else
        const WCHAR *sysDir = L"system32";
#endif

        swprintf_s(exeName, L"%s\\%s\\rundll32.exe", winDir, sysDir);
        swprintf_s(commandLine, L"rundll32.exe \"%hs\",InjectDLL %d %hs", lpDllName, appProcessID, lpDllName);

        ZeroMemory(&pi, sizeof(pi));
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        CreateProcessW_type pfCreateProcess = CreateProcessW;
        pfCreateProcess(exeName, commandLine, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
        ResumeThread(pi.hThread);

        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

};
