//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osProcess.cpp
///
//=====================================================================

//------------------------------ osProcess.cpp ------------------------------

// C++:
#include <iterator>
#include <string>
#include <fstream>

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <shellapi.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <aclapi.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>

// Local:
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osProcessSharedFile.h>
#include <AMDTOSWrappers/Include/osUser.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osSystemError.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTime.h>


extern "C" {

    typedef LONG NTSTATUS, *PNTSTATUS;

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth

    typedef enum _PROCESSINFOCLASS
    {
        ProcessBasicInformation = 0,
        ProcessDebugPort = 7,
        ProcessWow64Information = 26,
        ProcessImageFileName = 27,
        ProcessBreakOnTermination = 29
    } PROCESSINFOCLASS;

    typedef NTSTATUS NTAPI PfnNtQueryInformationProcess(
        __in HANDLE ProcessHandle,
        __in PROCESSINFOCLASS ProcessInformationClass,
        __out_bcount(ProcessInformationLength) PVOID ProcessInformation,
        __in ULONG ProcessInformationLength,
        __out_opt PULONG ReturnLength
    );

    typedef NTSTATUS NTAPI PfnNtWow64QueryInformationProcess64(
        __in HANDLE ProcessHandle,
        __in PROCESSINFOCLASS ProcessInformationClass,
        __out_bcount(ProcessInformationLength) PVOID ProcessInformation64,
        __in ULONG ProcessInformationLength,
        __out_opt PULONG ReturnLength
    );

    /* PVOID64 and ULONGLONG take 2 params each */
    typedef NTSTATUS NTAPI PfnNtWow64ReadVirtualMemory64(
        __in HANDLE ProcessHandle,
        __in_opt PVOID64 BaseAddress,
        __out_bcount(BufferSize) PVOID Buffer,
        __in ULONGLONG BufferSize,
        __out_opt PULONGLONG NumberOfBytesRead
    );
} // extern "C"

template <typename TPtr>
struct UNICODE_STRING_T
{
    USHORT Length;
    USHORT MaximumLength;
    TPtr  Buffer;
};

template <typename TPtr>
struct LIST_ENTRY_T
{
    TPtr Flink;
    TPtr Blink;
};

template <typename TPtr>
struct LDR_DATA_TABLE_ENTRY_T
{
    LIST_ENTRY_T<TPtr> InLoadOrderLinks;
    LIST_ENTRY_T<TPtr> InMemoryOrderLinks;
    LIST_ENTRY_T<TPtr> InInitializationOrderLinks;
    TPtr DllBase;
    TPtr EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING_T<TPtr> FullDllName;
    UNICODE_STRING_T<TPtr> BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
};

template <typename TPtr>
struct PEB_LDR_DATA_T
{
    ULONG Length;
    BOOLEAN Initialized;
    TPtr SsHandle;
    LIST_ENTRY_T<TPtr> InLoadOrderModuleList;
    LIST_ENTRY_T<TPtr> InMemoryOrderModuleList;
    LIST_ENTRY_T<TPtr> InInitializationOrderModuleList;
    TPtr EntryInProgress;
};

template <typename TPtr>
struct RTL_USER_PROCESS_PARAMETERS_T
{
    ULONG MaximumLength;
    ULONG Length;

    ULONG Flags;
    ULONG DebugFlags;

    TPtr ConsoleHandle;
    ULONG  ConsoleFlags;
    TPtr StandardInput;
    TPtr StandardOutput;
    TPtr StandardError;

    UNICODE_STRING_T<TPtr> CurrentDirectory;// ProcessParameters
    TPtr CurrentDirectoryHandle;
    UNICODE_STRING_T<TPtr> DllPath;         // ProcessParameters
    UNICODE_STRING_T<TPtr> ImagePathName;   // ProcessParameters
    UNICODE_STRING_T<TPtr> CommandLine;     // ProcessParameters
};

#pragma warning(push)
#pragma warning (disable : 4201)
template <typename TPtr>
struct PEB_T
{
    BOOLEAN InheritedAddressSpace;      // These four fields cannot change unless the
    BOOLEAN ReadImageFileExecOptions;   //
    BOOLEAN BeingDebugged;              //
    union
    {
        BOOLEAN BitField;                  //
        struct
        {
            BOOLEAN ImageUsesLargePages : 1;
            BOOLEAN SpareBits : 7;
        };
    };
    TPtr Mutant;      // INITIAL_PEB structure is also updated.

    TPtr ImageBaseAddress;
    TPtr Ldr;
    TPtr ProcessParameters;
};
#pragma warning(pop)

template <typename TPtr>
struct PROCESS_BASIC_INFORMATION_T
{
    NTSTATUS ExitStatus;
    TPtr PebBaseAddress;
    TPtr AffinityMask;
    LONG BasePriority;
    TPtr UniqueProcessId;
    TPtr ParentProcessId;
};

static void TrimExecutablePath(const gtString& executablePath, gtString& arguments);

static bool ReadProcessInformation(HANDLE hProcess,
                                   osRuntimePlatform& platform,
                                   gtString& executablePath, gtString& arguments, gtString& workDirectory);
static bool ReadProcessPlatform(HANDLE hProcess, osRuntimePlatform& platform);

#if AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE
static bool ReadProcessInformationWow64(HANDLE hProcess,
                                        osRuntimePlatform& platform,
                                        gtString& executablePath, gtString& arguments, gtString& workDirectory);
static bool ReadProcessPlatformWow64(HANDLE hProcess, osRuntimePlatform& platform);
#endif

// redirection files:
osProcessSharedFile g_outputRedirectFile;
osProcessSharedFile g_inputRedirectFile;

#if !defined ( SID_REVISION )
#define SID_REVISION ( 1 ) ///< SID definition
#endif

bool osEnableTokenPrivilege(HANDLE htok, LPCTSTR szPrivilege, TOKEN_PRIVILEGES* tpOld)
{
    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (LookupPrivilegeValue(0, szPrivilege, &tp.Privileges[0].Luid))
    {
        DWORD cbOld = sizeof(*tpOld);

        if (AdjustTokenPrivileges(htok, FALSE, &tp, cbOld, tpOld, &cbOld))
        {
            return (ERROR_NOT_ALL_ASSIGNED != osGetLastSystemError());
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(L"osAdjustDACL() failed", OS_DEBUG_LOG_ERROR);

            return false;
        }
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"osAdjustDACL() failed", OS_DEBUG_LOG_ERROR);

        return (FALSE);
    }
}
// ---------------------------------------------------------------------------
// Name:        osAdjustDacl
// Description: Adjust the  discretionary access control list (DACL) of a windows object
// Arguments:   h - handle of the Windows object
//              dwAccessRights - the desired access rights
// Return Val:  true if DACL was adjusted to provide the desired access was successfully
// Author:      AMD Developer Tools Team
// Date:        
// ---------------------------------------------------------------------------
static bool osAdjustDACL(HANDLE h, DWORD dwDesiredAccess)
{
    bool retVal = false;
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

        retVal = (err == ERROR_SUCCESS);
    }
    else
    {
        OS_OUTPUT_DEBUG_LOG(L"osAdjustDACL() failed", OS_DEBUG_LOG_ERROR);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osAdvancedOpenProcess
// Description: Makes every effort possible to open a process handle
// Arguments:   pid - The process id of the process to open
//              dwAccessRights - the desired access rights
// Return Val:  The handle to the target process
// Author:      AMD Developer Tools Team
// Date:        
// ---------------------------------------------------------------------------
static osProcessHandle osAdvancedOpenProcess(osProcessId pid, DWORD dwAccessRights)
{
    osProcessHandle hProcess = OpenProcess(dwAccessRights, FALSE, pid);

    if (hProcess == NULL)
    {
        osProcessHandle hpWriteDAC = OpenProcess(WRITE_DAC, FALSE, pid);

        if (hpWriteDAC == NULL)
        {
            HANDLE htok;
            TOKEN_PRIVILEGES tpOld;

            if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &htok) == false)
            {
                return (FALSE);
            }

            if (osEnableTokenPrivilege(htok, SE_TAKE_OWNERSHIP_NAME, &tpOld))
            {
                osProcessHandle hpWriteOwner = OpenProcess(WRITE_OWNER, FALSE, pid);

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
            osAdjustDACL(hpWriteDAC, dwAccessRights);

            if (!DuplicateHandle(GetCurrentProcess(), hpWriteDAC, GetCurrentProcess(), &hProcess, dwAccessRights, FALSE, 0))
            {
                hProcess = NULL;
            }

            CloseHandle(hpWriteDAC);
        }
    }

    return hProcess;
}

// ---------------------------------------------------------------------------
// Name:        osSetCurrentProcessEnvVariable
// Description: Adds / sets the value of an environment variable in the current
//              process environment block.
// Arguments:   envVariable - The environment variable to be added / set.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        6/9/2005
// ---------------------------------------------------------------------------
bool osSetCurrentProcessEnvVariable(const osEnvironmentVariable& envVariable)
{
    bool retVal = false;

    // Expand the requested environment value (replace %ENV_NAME% with the current value):
    gtString requestedEnvValue = envVariable._value;
    gtString expandedEnvValue;
    bool rc1 = osExpandEnvironmentStrings(requestedEnvValue, expandedEnvValue);

    if (rc1)
    {
        // Add / Set the environment variable value:
        BOOL rc2 = ::SetEnvironmentVariable(envVariable._name.asCharArray(), expandedEnvValue.asCharArray());

        if (rc2 != 0)
        {
            retVal = true;
        }
    }

    // In case of failure:
    if (!retVal)
    {
        gtString errMessage = OS_STR_FailedToSetEnvVariable;
        errMessage += envVariable._name;

        GT_ASSERT_EX(false, errMessage.asCharArray());
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osExpandEnvironmentStrings
// Description: Expand the environment value with the existing environment values
//              (replace %VAR_NAME% with the current value of VAR_NAME)
// Arguments:   const gtString& envVariableValue
//              gtString& expandedEnvVariableValue
// Return Val:  bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        6/9/2009
// ---------------------------------------------------------------------------
bool osExpandEnvironmentStrings(const gtString& envVariableValue, gtString& expandedEnvVariableValue)
{
    bool retVal = false;

    // Get the expanded environment value size:
    DWORD requiredBuffSize = ::ExpandEnvironmentStrings(envVariableValue.asCharArray(), NULL, 0);

    if (requiredBuffSize > 0)
    {
        // Allocate a buffer that will contain the variable value:
        wchar_t* pBuff = new wchar_t[requiredBuffSize];


        // Get the environment variable value:
        DWORD rc = ::ExpandEnvironmentStrings(envVariableValue.asCharArray(), pBuff , requiredBuffSize);

        if (rc == requiredBuffSize)
        {
            // Output the environment variable value:
            expandedEnvVariableValue = pBuff;
            retVal = true;
        }
        else
        {
            GT_ASSERT(false);
        }

        // Clean up:
        delete[] pBuff;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        osRemoveCurrentProcessEnvVariable
// Description: Remove an environment variable from the current process
//              environment block.
// Arguments:   envVariableName - The name of the environment variable to be removed.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        6/9/2005
// ---------------------------------------------------------------------------
bool osRemoveCurrentProcessEnvVariable(const gtString& envVariableName)
{
    bool retVal = false;

    // Add / Set the environment variable value:
    BOOL rc = ::SetEnvironmentVariable(envVariableName.asCharArray(), NULL);

    if (rc != 0)
    {
        retVal = true;
    }

    // In case of failure:
    if (!retVal)
    {
        gtString errMessage = OS_STR_FailedToRemoveEnvVariable;
        errMessage += envVariableName;

        GT_ASSERT_EX(false, errMessage.asCharArray());
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetCurrentProcessEnvVariableValue
// Description: Retrieves the value of an environment variable in the current
//              process environment block.
// Arguments:   envVariableName - The queried environment variable name
//              envVariableValue - Will get the environment variable value.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        6/9/2005
// ---------------------------------------------------------------------------
bool osGetCurrentProcessEnvVariableValue(const gtString& envVariableName, gtString& envVariableValue)
{
    bool retVal = false;

    // Get the size of the buffer required to hold the variable name + a NULL terminating char:
    DWORD requiredBuffSize = ::GetEnvironmentVariable(envVariableName.asCharArray(), NULL, 0);

    // If the state variable exists:
    if (0 < requiredBuffSize)
    {
        // Allocate a buffer that will contain the variable value:
        wchar_t* pBuff = new wchar_t[requiredBuffSize];


        // Calculate the variable value size:
        DWORD variableValueSize = requiredBuffSize - 1;

        // Get the environment variable value:
        DWORD rc = ::GetEnvironmentVariable(envVariableName.asCharArray(), pBuff , requiredBuffSize);

        if (rc == variableValueSize)
        {
            // Output the environment variable value:
            envVariableValue = pBuff;
            retVal = true;
        }
        else
        {
            GT_ASSERT(false);
        }

        // Clean up:
        delete[] pBuff;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetCurrentProcessId
// Description: Returns the OS Id of the current process.
// Author:      AMD Developer Tools Team
// Date:        16/8/2006
// ---------------------------------------------------------------------------
osProcessId osGetCurrentProcessId()
{
    osProcessId retVal = ::GetCurrentProcessId();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osExitCurrentProcess
// Description:
//   Exits the current process and all its threads in a "clean way".
//   (This includes notifying all attached DLLs about the process exit, etc)
// Author:      AMD Developer Tools Team
// Date:        6/10/2004
// ---------------------------------------------------------------------------
void osExitCurrentProcess(int exitCode)
{
    ExitProcess(exitCode);
}

// ---------------------------------------------------------------------------
// Name:        osWaitForProcessToTerminate
// Description: Waits for a process to terminate, with a limit of timeoutMsec.
//              if pExitCode is not NULL (and retVal isn't false), returns the
//              process's exit code into it.
// Return Val: true - the process exited.
//             false - the timeout expired before the process exited.
// Author:      AMD Developer Tools Team
// Date:        17/8/2009
// ---------------------------------------------------------------------------
bool osWaitForProcessToTerminate(osProcessId processId, unsigned long timeoutMsec, long* pExitCode, bool child)
{
    GT_UNREFERENCED_PARAMETER(child);

    bool retVal = false;
    // Get a handle to the process:
    osProcessHandle processHandle = NULL;

    // If we need to return the exit code:
    if (NULL != pExitCode)
    {
        *pExitCode = 0L;

        // We need the PROCESS_QUERY_INFORMATION privilege to get the exit code:
        processHandle = ::OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, processId);
    }
    else
    {
        processHandle = ::OpenProcess(SYNCHRONIZE, FALSE, processId);
    }

    if (NULL != processHandle)
    {
        // Wait for the object
        DWORD retCode = ::WaitForSingleObject(processHandle, (DWORD)timeoutMsec);

        if (retCode == WAIT_FAILED)
        {
            // The function failed:
            GT_ASSERT(false);
        }
        else if (retCode == WAIT_OBJECT_0)
        {
            // The process exited properly:
            retVal = true;

            // Get its exit code if requested:
            if (pExitCode != NULL)
            {
                DWORD exitCode = 0;
                BOOL rcExitCode = ::GetExitCodeProcess(processHandle, &exitCode);
                GT_IF_WITH_ASSERT(rcExitCode == TRUE)
                {
                    *pExitCode = (long)exitCode;
                }
                else
                {
                    // We requested the exit code but couldn't get it
                    retVal = false;
                }
            }
        }
        else if (retCode == WAIT_TIMEOUT)
        {
            // We timed out
            retVal = false;
        }
        else
        {
            // Unexpected result
            GT_ASSERT(false);
        }

        // Close the handle
        BOOL rcClose = ::CloseHandle(processHandle);
        GT_ASSERT(rcClose == TRUE);
    }
    else
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osTerminateProcess
// Description: Terminates a process, running on the local machine.
// Arguments: processId - The id of the process to be terminated.
//            exitCode - The terminated process exit code.
//            isTerminateChildren - should processes spawned by the target process be terminated too
//            isGracefulShutdownRequired - attempts to close the process gracefully before forcefully terminating it. 
//                                          This option is currently not implemented on Windows.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        17/8/2009
// ---------------------------------------------------------------------------
bool osTerminateProcess(osProcessId processId, long exitCode, bool isTerminateChildren, bool isGracefulShutdownRequired)
{
    bool retVal = false;

    if (isTerminateChildren)
    {
        osTerminateChildren(processId, isGracefulShutdownRequired);
    }

    // Get a handle to the process:
    osProcessHandle processHandle = osAdvancedOpenProcess(processId, PROCESS_TERMINATE);

    GT_IF_WITH_ASSERT(processHandle != NULL)
    {
        // Terminates the process:
        int rc = ::TerminateProcess(processHandle, (UINT)exitCode);
        CloseHandle(processHandle);

        retVal = (rc != 0);

    }

    return retVal;
}


// ----------------------------------------------------------------------------------
// Name:        osRemoveRedirectionFromCmdLine
// Description: Remove SDTIN/STDOUT from command line argument
// Author:      AMD Developer Tools Team
// Date:        10/04/2012
// ----------------------------------------------------------------------------------
void osRemoveRedirectionFromCmdLine(const gtString& originalString, gtString& truncatedString)
{
    // Neglecting stdin/stdout redirection as it is not supported in Beta4 release for Windows.
#pragma message ("TODO: Need to support stdin/stdout redirection")
    truncatedString = originalString;

    // process STDOUT command
    int position = originalString.find('>');

    if (position != -1)
    {
        truncatedString = truncatedString.truncate(0, position - 1);
    }

    // process STDIN command
    position = truncatedString.find('<');

    if (position != -1)
    {
        int endOfStdInCmd = truncatedString.find(' ', position + 2);
        int length = truncatedString.length();
        gtString subStr1, subStr2;
        truncatedString.getSubString(0, position - 1, subStr1);
        truncatedString.getSubString(endOfStdInCmd + 1, length, subStr2);
        truncatedString = subStr1;
        truncatedString += subStr2;
    }
}

// ----------------------------------------------------------------------------------
// Name:        osLaunchSuspendedProcess
// Description: Launches a suspended process, running on the local machine
//              Note: the processHandle and processThreadHandle need to be closed
//              via a call to CloseHandle when they are no longer needed.
//              osResumeSuspendedProcess can optionally do that for you, otherwise
//              you will need to do that yourself
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/11/2012
// ----------------------------------------------------------------------------------
bool osLaunchSuspendedProcess(const osFilePath& executablePath, const gtString& arguments,
                              const osFilePath& workDirectory, osProcessId& processId, osProcessHandle& processHandle,
                              osThreadHandle& processThreadHandle, bool createWindow, bool redirectFiles, bool removeCodeXLPaths)
{
    GT_UNREFERENCED_PARAMETER(removeCodeXLPaths);

    gtString commandLine;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    commandLine.appendFormattedString(L"\"%ls\" %ls", executablePath.asString().asCharArray(),
                                      arguments.asCharArray());


    BOOL handleInheritance = FALSE;

    if (redirectFiles)
    {
        gtString outputFileName;
        gtString inputFileName;
        bool appendMode;

        if (osCheckForOutputRedirection(commandLine, outputFileName, appendMode))
        {
            g_outputRedirectFile.openFile(outputFileName, true, appendMode);
        }

        if (osCheckForInputRedirection(commandLine, inputFileName))
        {
            g_inputRedirectFile.openFile(inputFileName, false, false);
        }

        if (g_outputRedirectFile.handle() != NULL || g_inputRedirectFile.handle() != NULL)
        {
            si.dwFlags = STARTF_USESTDHANDLES;
            handleInheritance = TRUE;
        }

        si.hStdOutput = g_outputRedirectFile.handle();
        si.hStdInput = g_inputRedirectFile.handle();
    }

    DWORD createFlag = CREATE_NEW_PROCESS_GROUP;

    // Create target with suspended.
    createFlag |= CREATE_SUSPENDED;

    if (!createWindow)
    {
        createFlag |= CREATE_NO_WINDOW;
    }

    // Start the child process.
    if (!CreateProcess(NULL,            // No module name (use command line).
                       (wchar_t*)commandLine.asCharArray(),   // Command line.
                       NULL,           // Process handle not inheritable.
                       NULL,           // Thread handle not inheritable.
                       handleInheritance, // Set handle inheritance to FALSE.
                       createFlag,     // creation flags.
                       NULL,           // Use parent's environment block.
                       (wchar_t*)workDirectory.asString().asCharArray(),   // Working Dir
                       &si,            // Pointer to STARTUPINFO structure.
                       &pi))           // Pointer to PROCESS_INFORMATION structure.
    {
        DWORD lastError = ::GetLastError();

        if (ERROR_ELEVATION_REQUIRED == lastError)
        {
            SHELLEXECUTEINFO shExInfo = { 0 };
            shExInfo.cbSize = sizeof(shExInfo);
            shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
            shExInfo.hwnd = 0;
            shExInfo.lpVerb = _T("runas");                // Operation to perform
            shExInfo.lpFile = executablePath.asString().asCharArray();       // Application to start
            shExInfo.lpParameters = arguments.asCharArray();                  // Additional parameters
            shExInfo.lpDirectory = 0;
            shExInfo.nShow = SW_SHOW;
            shExInfo.hInstApp = 0;

            if (ShellExecuteEx(&shExInfo))
            {
                processHandle = shExInfo.hProcess;
                processId = ::GetProcessId(shExInfo.hProcess);
                // ew can't get the thread handle in this case (lots of work getting it)
                processThreadHandle = nullptr;
                return true;
            }

            return false;
        }
        else
        {
            return false;
        }
    }

    processHandle = pi.hProcess;
    processId = pi.dwProcessId;
    processThreadHandle = pi.hThread;

    return true;
}

// ----------------------------------------------------------------------------------
// Name:        osResumeSuspendedProcess
// Description: Resumes a suspended process, running on the local machine, optionally
//              closing the handles returned from the CreateProcess call
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/11/2012
// ----------------------------------------------------------------------------------
bool osResumeSuspendedProcess(const osProcessId& processId, const osProcessHandle& processHandle,
                              const osThreadHandle& processThreadHandle, bool closeHandles)
{
    GT_UNREFERENCED_PARAMETER(processId);

    bool retVal = 0xFFFFFFFF != ResumeThread(processThreadHandle);

    if (closeHandles)
    {
        CloseHandle(processHandle);
        CloseHandle(processThreadHandle);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osCloseProcessRedirectionFiles
// Description: close redirection files for the process
// Author:      AMD Developer Tools Team
// Date:        23/6/2013
// ---------------------------------------------------------------------------
void osCloseProcessRedirectionFiles()
{
    g_inputRedirectFile.closeFile();
    g_outputRedirectFile.closeFile();
}

bool osSetProcessAffinityMask(osProcessId processId, const osProcessHandle processHandle, gtUInt64 affinityMask)
{
    GT_UNREFERENCED_PARAMETER(processId);

#pragma message ("TODO: Handle more than 32 cores")
    BOOL res = SetProcessAffinityMask(processHandle, static_cast<DWORD_PTR>(affinityMask));
    return TRUE == res;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        osGetRegistryKeyValue
/// \brief Description: Returns a registry key value
/// \param[in]          keyPath
/// \param[in]          keyName
/// \param[in]          keyValue
/// \return OS_API bool
/// -----------------------------------------------------------------------------------------------
OS_API bool osGetRegistryKeyValue(HKEY hKey, const gtString& keyPath, const gtString& keyName, gtString& keyValue)
{
    bool retVal = false;
    HKEY key;
    WCHAR keyValueAsTCHAR[1024];
    DWORD bufferLength = 1024 * sizeof(TCHAR);
    long ret = ERROR_SUCCESS;

    // Open the key:
    ret = RegOpenKeyExW(hKey, keyPath.asCharArray(), 0, KEY_QUERY_VALUE, &key);

    if (ret == ERROR_SUCCESS)
    {
        // Get the key value:
        ret = RegQueryValueExW(key, keyName.asCharArray(), 0, 0, (LPBYTE) keyValueAsTCHAR, &bufferLength);

        // Close the key:
        RegCloseKey(key);

        if ((ret == ERROR_SUCCESS) && (bufferLength < 1024 * sizeof(TCHAR)))
        {
            for (int i = 0; i < (int) bufferLength; i++)
            {
                keyValue.append(keyValueAsTCHAR[i]);
            }

            keyValue.append(L'\0');

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osIsProcessAlive
// Description: Returns true iff the function succeeded. Buffer is set to true
//              iff the process is alive.
// Author:      AMD Developer Tools Team
// Date:        08/08/2013
// ---------------------------------------------------------------------------
OS_API bool osIsProcessAlive(osProcessId processId, bool& buffer)
{
    bool retVal = true;
    buffer = false;

    // Acquire a handle to the process.
    osProcessHandle processHandle = ::OpenProcess(SYNCHRONIZE, FALSE, processId);

    // Set a low timeout, for us to simply check if the process is alive.
    const DWORD timeoutMsec = 1;

    if (processHandle != NULL)
    {
        // Wait for the object.
        DWORD retCode = ::WaitForSingleObject(processHandle, (DWORD)timeoutMsec);

        if (retCode == WAIT_TIMEOUT)
        {
            // The process is alive.
            buffer = true;
        }

        //else if (retCode == WAIT_FAILED)
        //{
        //    // The function failed.
        //    retVal = false;
        //}

        // Close the handle
        BOOL rcClose = ::CloseHandle(processHandle);

        GT_ASSERT(rcClose == TRUE);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTerminateChildren
// Description: Terminates all of the processes which were spawned by the process
//              with pid processId
// Author:      AMD Developer Tools Team
// Date:        08/10/2013
// ---------------------------------------------------------------------------
OS_API bool osTerminateChildren(osProcessId processId, bool isGracefulShutdownRequired)
{
    // Take a snapshot of all processes that are currently running in the system.
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    bool ret = (INVALID_HANDLE_VALUE != h);

    if (ret)
    {
        PROCESSENTRY32 pe = { 0 };
        pe.dwSize = sizeof(PROCESSENTRY32);

        // Run through all running processes.
        if (Process32First(h, &pe))
        {
            do
            {
                // If this is a child of our process.
                if (pe.th32ParentProcessID == processId)
                {
                    // Terminate this child.
                    bool isOk = osTerminateProcess(pe.th32ProcessID, 0, true, isGracefulShutdownRequired);
                    GT_ASSERT_EX(isOk, L"Failed to terminate a child process.");
                    ret = ret && isOk;
                }
            }
            while (Process32Next(h, &pe));
        }

        // Clean.
        CloseHandle(h);
    }

    return ret;
}

OS_API bool osGetProcessType(osProcessId processId, osModuleArchitecture& arch, osRuntimePlatform& platform, bool setPrivilege)
{
    void* tokenHandle = NULL;

    if (setPrivilege)
    {
        osSetPrivilege(tokenHandle, SE_DEBUG_NAME, true, false);
    }

    HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);

    bool ret = (NULL != handle);

    if (ret)
    {
        BOOL isWow64 = FALSE;
        IsWow64Process(handle, &isWow64);

#if AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE
        BOOL is32on64Sys = FALSE;
        IsWow64Process(GetCurrentProcess(), &is32on64Sys);

        platform = OS_UNKNOWN_PLATFORM;

        if (is32on64Sys && !isWow64)
        {
            arch = OS_X86_64_ARCHITECTURE;

            ret = ReadProcessPlatformWow64(handle, platform);
        }
        else
        {
            arch = OS_I386_ARCHITECTURE;
#else
        {
            arch = isWow64 ? OS_I386_ARCHITECTURE : OS_X86_64_ARCHITECTURE;
#endif
            ret = ReadProcessPlatform(handle, platform);
        }

        CloseHandle(handle);
    }

    if (NULL != tokenHandle)
    {
        osSetPrivilege(tokenHandle, SE_DEBUG_NAME, false, true);
    }

    return ret;
}

OS_API bool osGetProcessLaunchInfo(osProcessId processId,
                                   osModuleArchitecture& arch, osRuntimePlatform& platform,
                                   gtString& executablePath, gtString& arguments, gtString& workDirectory,
                                   bool setPrivilege)
{
    void* tokenHandle = NULL;

    if (setPrivilege)
    {
        osSetPrivilege(tokenHandle, SE_DEBUG_NAME, true, false);
    }

    HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);

    bool ret = (NULL != handle);

    if (ret)
    {
        BOOL isWow64 = FALSE;
        IsWow64Process(handle, &isWow64);

#if AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE
        BOOL is32on64Sys = FALSE;
        IsWow64Process(GetCurrentProcess(), &is32on64Sys);

        platform = OS_UNKNOWN_PLATFORM;

        if (is32on64Sys && !isWow64)
        {
            arch = OS_X86_64_ARCHITECTURE;

            ret = ReadProcessInformationWow64(handle, platform, executablePath, arguments, workDirectory);
        }
        else
        {
            arch = OS_I386_ARCHITECTURE;
#else
        {
            arch = isWow64 ? OS_I386_ARCHITECTURE : OS_X86_64_ARCHITECTURE;
#endif
            ret = ReadProcessInformation(handle, platform, executablePath, arguments, workDirectory);
        }

        CloseHandle(handle);

        if (ret)
        {
            TrimExecutablePath(executablePath, arguments);
        }
    }

    if (NULL != tokenHandle)
    {
        osSetPrivilege(tokenHandle, SE_DEBUG_NAME, false, true);
    }

    return ret;
}

bool osIsProcessAttachable(osProcessId processId)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE, FALSE, processId);

    bool ret = (NULL != hProcess);

    if (ret)
    {
        CloseHandle(hProcess);
    }

    return ret;
}


osProcessesEnumerator::osProcessesEnumerator() : m_pEnumHandler(INVALID_HANDLE_VALUE)
{
    m_pe32.dwSize = sizeof(m_pe32);
}

osProcessesEnumerator::~osProcessesEnumerator()
{
    deinitialize();
}

bool osProcessesEnumerator::initialize()
{
    m_pEnumHandler = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    bool ret = false;

    if (INVALID_HANDLE_VALUE != m_pEnumHandler)
    {
        if (Process32First(m_pEnumHandler, &m_pe32) != FALSE)
        {
            ret = true;
        }
        else
        {
            deinitialize();
        }
    }

    return ret;
}

void osProcessesEnumerator::deinitialize()
{
    if (INVALID_HANDLE_VALUE != m_pEnumHandler)
    {
        CloseHandle(m_pEnumHandler);
        m_pEnumHandler = INVALID_HANDLE_VALUE;
    }
}

bool osProcessesEnumerator::next(osProcessId& processId, gtString* pName)
{
    const bool ret = (((osProcessId) - 1) != m_pe32.th32ProcessID);

    if (ret)
    {
        processId = m_pe32.th32ProcessID;

        if (NULL != pName)
        {
            *pName = m_pe32.szExeFile;
        }
    }

    if (Process32Next(m_pEnumHandler, &m_pe32) == FALSE)
    {
        m_pe32.th32ProcessID = ((osProcessId) - 1);
    }

    return ret;
}


static void TrimExecutablePath(const gtString& executablePath, gtString& arguments)
{
    gtString executableName;

    int pos = executablePath.reverseFind(L'\\');

    if (-1 != pos)
    {
        executablePath.getSubString(pos + 1, executablePath.length() - 1, executableName);
    }
    else
    {
        executableName = executablePath;
    }

    int szExeName = executableName.length();

    if (!executableName.isEmpty() && szExeName <= arguments.length())
    {
        int prefix;

        if (L'\"' == arguments[0])
        {
            prefix = 1;
            pos = arguments.find(L'\"', 1);
        }
        else
        {
            prefix = 0;
            pos = arguments.findFirstOf(L" \t");

            if (-1 == pos)
            {
                pos = arguments.length();
            }
        }

        if (-1 != pos)
        {
            if (szExeName <= (pos - prefix) && 0 == arguments.compare(pos - szExeName, szExeName, executableName))
            {
                pos = arguments.findFirstNotOf(L" \t", pos + 1);

                if (-1 != pos)
                {
                    arguments.getSubString(pos, arguments.length() - 1, arguments);
                }
                else
                {
                    arguments.makeEmpty();
                }
            }
        }
    }
}


#if AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE
    static PfnNtWow64QueryInformationProcess64* NtWow64QueryInformationProcess64 = NULL;
    static PfnNtWow64ReadVirtualMemory64* NtWow64ReadVirtualMemory64 = NULL;
#endif
static PfnNtQueryInformationProcess* NtQueryInformationProcess = NULL;

#if AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE
bool ReadVirtualMemory(HANDLE hProcess, ULONG64 baseAddress, PVOID pBuffer, ULONG64 bufferSize)
{
    return STATUS_SUCCESS == NtWow64ReadVirtualMemory64(hProcess, (PVOID64)baseAddress, pBuffer, bufferSize, NULL);
}
#endif

bool ReadVirtualMemory(HANDLE hProcess, SIZE_T baseAddress, PVOID pBuffer, SIZE_T bufferSize)
{
    return TRUE == ReadProcessMemory(hProcess, (PVOID)baseAddress, pBuffer, bufferSize, NULL);
}

bool GetProcessEnvironmentBlockAddress(HANDLE hProcess, SIZE_T& peb)
{
    bool ret = false;

    if (NULL == NtQueryInformationProcess)
    {
        NtQueryInformationProcess =
            reinterpret_cast<PfnNtQueryInformationProcess*>(GetProcAddress(GetModuleHandleA("ntdll.dll"),
                                                                           "NtQueryInformationProcess"));
    }

    if (NULL != NtQueryInformationProcess)
    {
        PROCESS_BASIC_INFORMATION_T<SIZE_T> pbi;
        NTSTATUS status = NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), NULL);

        if (STATUS_SUCCESS == status)
        {
            peb = pbi.PebBaseAddress;
            ret = true;
        }
    }

    return ret;
}

#if AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE
bool GetProcessEnvironmentBlockAddress(HANDLE hProcess, ULONG64& peb)
{
    bool ret = false;

    if (NULL == NtWow64QueryInformationProcess64)
    {
        NtWow64QueryInformationProcess64 =
            reinterpret_cast<PfnNtWow64QueryInformationProcess64*>(GetProcAddress(GetModuleHandleA("ntdll.dll"),
                                                                   "NtWow64QueryInformationProcess64"));

        if (NULL != NtWow64QueryInformationProcess64)
        {
            NtWow64ReadVirtualMemory64 =
                reinterpret_cast<PfnNtWow64ReadVirtualMemory64*>(GetProcAddress(GetModuleHandleA("ntdll.dll"),
                                                                                "NtWow64ReadVirtualMemory64"));
        }
    }

    if (NULL != NtWow64ReadVirtualMemory64)
    {
        PROCESS_BASIC_INFORMATION_T<ULONG64> pbi;
        NTSTATUS status = NtWow64QueryInformationProcess64(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), NULL);

        if (STATUS_SUCCESS == status)
        {
            peb = pbi.PebBaseAddress;
            ret = true;
        }
    }

    return ret;
}
#endif

template <typename TPtr>
bool GetModuleListHeadAddress(HANDLE hProcess, TPtr peb, TPtr& moduleListHead)
{
    TPtr ldrDataAddr = ((TPtr)peb) + offsetof(PEB_T<TPtr>, Ldr);

    bool ret = ReadVirtualMemory(hProcess, ldrDataAddr, &ldrDataAddr, sizeof(ldrDataAddr));

    if (ret)
    {
        moduleListHead = ldrDataAddr + offsetof(PEB_LDR_DATA_T<TPtr>, InLoadOrderModuleList);
    }

    return ret;
}

template <typename TPtr>
bool ForeachLoadedModuleT(HANDLE hProcess, TPtr moduleListHead, bool (*pfnCallback)(gtString&, void*), void* pCbData)
{
    LDR_DATA_TABLE_ENTRY_T<TPtr> ldrData;
    bool ret = ReadVirtualMemory(hProcess, moduleListHead, &ldrData, sizeof(LIST_ENTRY_T<TPtr>));

    if (ret)
    {
        gtString fullDllName;

        while (ldrData.InLoadOrderLinks.Flink != moduleListHead)
        {
            if (!ReadVirtualMemory(hProcess, ldrData.InLoadOrderLinks.Flink, &ldrData, sizeof(LDR_DATA_TABLE_ENTRY_T<TPtr>)))
            {
                ret = false;
                break;
            }

            fullDllName.resize(ldrData.FullDllName.Length / sizeof(WCHAR));

            if (!ReadVirtualMemory(hProcess,
                                   ldrData.FullDllName.Buffer,
                                   const_cast<wchar_t*>(fullDllName.asCharArray()),
                                   ldrData.FullDllName.Length))
            {
                ret = false;
                break;
            }

            if (!pfnCallback(fullDllName, pCbData))
            {
                break;
            }
        }
    }

    return ret;
}

template <typename TPtr>
bool ReadRtlUserProcessParameters(HANDLE hProcess, TPtr peb, RTL_USER_PROCESS_PARAMETERS_T<TPtr>& rtlUserProcParams)
{
    bool ret;

    TPtr processParamsAddr = ((TPtr)peb) + offsetof(PEB_T<TPtr>, ProcessParameters);

    TPtr rtlUserProcParamsAddr;

    if (ReadVirtualMemory(hProcess, processParamsAddr, &rtlUserProcParamsAddr, sizeof(rtlUserProcParamsAddr)))
    {
        TPtr offsetofCurrentDirectory = offsetof(RTL_USER_PROCESS_PARAMETERS_T<TPtr>, CurrentDirectory);

        ret = ReadVirtualMemory(hProcess,
                                rtlUserProcParamsAddr + offsetofCurrentDirectory,
                                &rtlUserProcParams.CurrentDirectory,
                                sizeof(rtlUserProcParams) - offsetofCurrentDirectory);
    }
    else
    {
        ret = false;
    }

    return ret;
}

template <typename TPtr>
bool ReadImagePathName(HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS_T<TPtr>& rtlUserProcParams, gtString& imagePathName)
{
    imagePathName.resize(rtlUserProcParams.ImagePathName.Length / sizeof(WCHAR));

    return ReadVirtualMemory(hProcess,
                             rtlUserProcParams.ImagePathName.Buffer,
                             const_cast<wchar_t*>(imagePathName.asCharArray()),
                             rtlUserProcParams.ImagePathName.Length);
}

template <typename TPtr>
bool ReadCurrentDirectory(HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS_T<TPtr>& rtlUserProcParams, gtString& currentDirectory)
{
    currentDirectory.resize(rtlUserProcParams.CurrentDirectory.Length / sizeof(WCHAR));

    return ReadVirtualMemory(hProcess,
                             rtlUserProcParams.CurrentDirectory.Buffer,
                             const_cast<wchar_t*>(currentDirectory.asCharArray()),
                             rtlUserProcParams.CurrentDirectory.Length);
}

template <typename TPtr>
bool ReadCommandLine(HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS_T<TPtr>& rtlUserProcParams, gtString& commandLine)
{
    commandLine.resize(rtlUserProcParams.CommandLine.Length / sizeof(WCHAR));

    return ReadVirtualMemory(hProcess,
                             rtlUserProcParams.CommandLine.Buffer,
                             const_cast<wchar_t*>(commandLine.asCharArray()),
                             rtlUserProcParams.CommandLine.Length);
}


static bool IdentifyProcessPlatform(gtString& fullDllName, void* pData)
{
    bool continueEnum;

    osRuntimePlatform* pPlatform = static_cast<osRuntimePlatform*>(pData);

    fullDllName.toLowerCase();

    if (fullDllName.endsWith(L"\\clr.dll") ||
        fullDllName.endsWith(L"\\mscorwks.dll") ||
        fullDllName.endsWith(L"\\mscorsvr.dll"))
    {
        *pPlatform = OS_DOT_NET_PLATFORM;

        continueEnum = false;
    }
    else if (fullDllName.endsWith(L"\\jvm.dll") ||
             fullDllName.endsWith(L"\\java.dll"))
    {
        *pPlatform = OS_JAVA_PLATFORM;

        continueEnum = false;
    }
    else
    {
        continueEnum = true;
    }

    return continueEnum;
}

template <typename TPtr>
bool ReadProcessInformationT(HANDLE hProcess,
                             osRuntimePlatform& platform,
                             gtString& executablePath, gtString& arguments, gtString& workDirectory)
{
    bool ret = false;

    TPtr peb;

    if (GetProcessEnvironmentBlockAddress(hProcess, peb))
    {
        RTL_USER_PROCESS_PARAMETERS_T<TPtr> rtlUserProcParams;

        if (ReadRtlUserProcessParameters(hProcess, peb, rtlUserProcParams))
        {
            if (ReadImagePathName(hProcess, rtlUserProcParams, executablePath) &&
                ReadCurrentDirectory(hProcess, rtlUserProcParams, workDirectory) &&
                ReadCommandLine(hProcess, rtlUserProcParams, arguments))
            {
                ret = true;

                platform = OS_NATIVE_PLATFORM;

                TPtr moduleListHead;

                if (GetModuleListHeadAddress(hProcess, peb, moduleListHead))
                {
                    ForeachLoadedModuleT(hProcess, moduleListHead, IdentifyProcessPlatform, &platform);
                }
            }
        }
    }

    return ret;
}

template <typename TPtr>
bool ReadProcessPlatformT(HANDLE hProcess, osRuntimePlatform& platform)
{
    bool ret = false;

    TPtr peb;

    if (GetProcessEnvironmentBlockAddress(hProcess, peb))
    {
        platform = OS_NATIVE_PLATFORM;

        TPtr moduleListHead;

        if (GetModuleListHeadAddress(hProcess, peb, moduleListHead))
        {
            ret = ForeachLoadedModuleT(hProcess, moduleListHead, IdentifyProcessPlatform, &platform);
        }
    }

    return ret;
}

static bool ReadProcessInformation(HANDLE hProcess,
                                   osRuntimePlatform& platform,
                                   gtString& executablePath, gtString& arguments, gtString& workDirectory)
{
    return ReadProcessInformationT<SIZE_T>(hProcess, platform, executablePath, arguments, workDirectory);
}

static bool ReadProcessPlatform(HANDLE hProcess, osRuntimePlatform& platform)
{
    return ReadProcessPlatformT<SIZE_T>(hProcess, platform);
}

#if AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE
static bool ReadProcessInformationWow64(HANDLE hProcess,
                                        osRuntimePlatform& platform,
                                        gtString& executablePath, gtString& arguments, gtString& workDirectory)
{
    return ReadProcessInformationT<ULONG64>(hProcess, platform, executablePath, arguments, workDirectory);
}

static bool ReadProcessPlatformWow64(HANDLE hProcess, osRuntimePlatform& platform)
{
    return ReadProcessPlatformT<ULONG64>(hProcess, platform);
}

#endif // AMDT_ADDRESS_SPACE_TYPE != AMDT_64_BIT_ADDRESS_SPACE

// ---------------------------------------------------------------------------
// Name:        osExecAndGrabOutput
// Description: Executes the in a different process and captures its output.
//              This routine blocks, but, using the cancelSignal flag, it allows
//              the caller to terminate the command's execution.
// Arguments:   cmd - The command to be executed.
//              cancelSignal - A reference to the cancel flag. Upon calling this
//              routine, the cancelSignal flag should be set to false. In case that
//              the caller wants to terminate the commands' execution, this flag
//              should be set to true.
//              cmdOutput - an output parameter to hold the command's output.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/08/2015
// ---------------------------------------------------------------------------
OS_API bool osExecAndGrabOutput(const char* cmd, const bool& cancelSignal, gtString& cmdOutput)
{
    bool ret = false;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    // Clear the output string.
    cmdOutput.makeEmpty();

    // Generate a unique file name for the temp file.
    gtString cxlTempFileName = L"cxlTempFile_";
    cxlTempFileName.appendFormattedString(L"%llu", __rdtsc());
    const gtString TMP_OUTPUT_FILE_EXT = L"txt";
    osFilePath tmpFilePath(osFilePath::OS_TEMP_DIRECTORY);

    if (cmd != NULL)
    {
        // Prepare the file name.
        tmpFilePath.setFileName(cxlTempFileName);
        tmpFilePath.setFileExtension(TMP_OUTPUT_FILE_EXT);

        HANDLE h = CreateFile(tmpFilePath.asString().asCharArray(),
                              FILE_APPEND_DATA,
                              FILE_SHARE_WRITE | FILE_SHARE_READ,
                              &sa,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);

        PROCESS_INFORMATION pi;
        STARTUPINFO si;
        BOOL rc = FALSE;
        DWORD flags = CREATE_NO_WINDOW;

        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdInput = NULL;
        si.hStdError = h;
        si.hStdOutput = h;

        // Fix the command line to handle white spaces in the CLI's path by wrapping it with quote characters at the beginning and the end.
        gtString tmpCmdLine;
        tmpCmdLine << cmd;

        // Log the launch command to the CodeXL log file
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Launching command: %ls", tmpCmdLine.asCharArray());

        const wchar_t* pCmd = tmpCmdLine.asCharArray();
        wchar_t* _pCmd = const_cast<wchar_t*>(pCmd);
        rc = CreateProcess(NULL, _pCmd, NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi);

        if (rc)
        {
            while (WAIT_TIMEOUT == WaitForSingleObject(pi.hProcess, 100))
            {
                if (cancelSignal)
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                    CloseHandle(h);
                }

            }

            if (!cancelSignal)
            {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                CloseHandle(h);
            }

            // Read the temp file
            if (tmpFilePath.exists())
            {
                // Read the command's output.
                osFile tmpFile(tmpFilePath);
                bool rc1 = tmpFile.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);
                if (rc1)
                {
                    gtASCIIString cmdOutputASCII;
                    rc1 = tmpFile.readIntoString(cmdOutputASCII);
                    tmpFile.close();

                    if (rc1)
                    {
                        cmdOutput.fromASCIIString(cmdOutputASCII.asCharArray());
                    }
                }

                // Delete the temporary file.
                tmpFile.deleteFile();
            }

            ret = true;
        }
        else
        {
            unsigned int errorNum = GetLastError();
            gtString errMsg = L"Failed to launch the command. Error = ";
            errMsg << errorNum;
            OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
        }
    }

    return ret;
}

OS_API bool osIsParent(osProcessId parentProcessId, osProcessId processId)
{
    bool retVal = false;

    gtMap<osProcessId, osProcessId> childParentMap;

    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    bool ret = (INVALID_HANDLE_VALUE != h);

    // Create the map of processes
    if (ret)
    {
        PROCESSENTRY32 pe = { 0 };
        pe.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(h, &pe))
        {
            do
            {
                childParentMap[pe.th32ProcessID] = pe.th32ParentProcessID;
            }
            while (Process32Next(h, &pe));
        }
    }

    // walk using the map from process id -> parent -> parent
    osProcessId currentParent = processId;

    // look for the parent until we are at the top of the tree (parent 0, or junk in case of problems)
    while (childParentMap.count(processId) != 0 && currentParent != 0)
    {
        currentParent = childParentMap[processId];

        if (currentParent == parentProcessId)
        {
            retVal = true;
            break;
        }

        // now check for this parent as if it is the current process looking for the next parent in the tree
        processId = currentParent;
    }

    return retVal;
}

OS_API bool osGetMemoryUsage(const unsigned int processID,
                             unsigned int& PageFaultCount,
                             size_t& WorkingSetSize,
                             size_t& PeakWorkingSetSize,
                             size_t& QuotaPeakPagedPoolUsage,
                             size_t& QuotaPagedPoolUsage,
                             size_t& QuotaPeakNonPagedPoolUsage,
                             size_t& QuotaNonPagedPoolUsage,
                             size_t& PagefileUsage,
                             size_t& PeakPagefileUsage,
                             size_t& PrivateUsage)
{

    bool result = false;
    // Print information about the memory usage of the process.

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                                  PROCESS_VM_READ,
                                  FALSE, processID);
    PROCESS_MEMORY_COUNTERS_EX pmc = {};
    pmc.cb = sizeof(pmc);

    if (nullptr != hProcess)
    {
        if (TRUE == GetProcessMemoryInfo(hProcess, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), pmc.cb))
        {
            PageFaultCount = pmc.PageFaultCount;
            PeakWorkingSetSize = pmc.PeakWorkingSetSize;
            WorkingSetSize = pmc.WorkingSetSize;
            QuotaPeakPagedPoolUsage = pmc.QuotaPeakPagedPoolUsage;
            QuotaPagedPoolUsage = pmc.QuotaPagedPoolUsage;
            QuotaPeakNonPagedPoolUsage = pmc.QuotaPeakNonPagedPoolUsage;
            QuotaNonPagedPoolUsage = pmc.QuotaNonPagedPoolUsage;
            PagefileUsage = pmc.PagefileUsage;
            PeakPagefileUsage = pmc.PeakPagefileUsage;
            PrivateUsage = pmc.PrivateUsage;
            result = true;
        }

        CloseHandle(hProcess);
    }

    return result;
}

OS_API bool osGetMainThreadId(osProcessId processId, osThreadId& mainThreadId)
{
    bool gotMainThread = false;
    mainThreadId = {};

    // Fill a vector with all of the active ThreadIds within the given process.
    std::vector<gtUInt32> threadIds;
    osProcessThreadsEnumerator threadEnumerator;

    if (threadEnumerator.initialize(processId))
    {
        gtUInt32 threadId;

        while (threadEnumerator.next(threadId))
        {
            threadIds.push_back(threadId);
        }
    }

    size_t numThreads = threadIds.size();

    if (numThreads > 0)
    {
        // Start by assuming the first thread is the main thread.
        osTime earliestStartTime = {};
        osGetThreadStartTime(threadIds[0], earliestStartTime);
        mainThreadId = threadIds[0];

        // If there are two or more threads in the process, find the oldest. That's the main thread.
        if (numThreads > 1)
        {
            osTime startTime = {};

            for (size_t threadIndex = 1; threadIndex < numThreads; ++threadIndex)
            {
                osThreadId threadId = threadIds[threadIndex];

                if (osGetThreadStartTime(threadId, startTime))
                {
                    if (startTime < earliestStartTime)
                    {
                        // Update the candidate for "the oldest thread in the process".
                        earliestStartTime = startTime;
                        mainThreadId = static_cast<osThreadId>(threadIndex);
                    }
                }
            }
        }

        gotMainThread = true;
    }

    return gotMainThread;
}
