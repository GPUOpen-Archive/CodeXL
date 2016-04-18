//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class provides wrappers for OS-specific functionality
//==============================================================================

#include <map>
#include <sstream>
#include <fstream>
#include <iostream>

#include "AMDTOSWrappers/Include/osGeneralFunctions.h"

#include "OSUtils.h"
#include "Logger.h"


using namespace std;
using namespace GPULogger;

static map<TIMERID, TimerCallbackFunc> sTimerCallbackTable;

#ifdef _WIN32

#include <process.h>

OSUtils::OSUtils() :
    m_pGetUserTime(NULL),
    m_pUserTimerInit(NULL),
    m_pUserTimerDestroy(NULL),
    m_bUserTimer(false),
    m_userTimerLibraryHandle(NULL)
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    m_dInvFrequency = 1.0 / freq.QuadPart * 1e9;
}

ULONGLONG OSUtils::GetTimeNanos(void)
{
    if (m_bUserTimer == true)
    {
        return m_pGetUserTime();
    }
    else
    {
        LARGE_INTEGER current;
        QueryPerformanceCounter(&current);
        return (unsigned long long)((double) current.QuadPart * m_dInvFrequency);
    }
}

LIB_HANDLE OSUtils::GenericLoadLibrary(const std::string& strFullLibraryName)
{
    LIB_HANDLE hUserLibrary = NULL;
    SP_TODO("revisit use of LoadLibraryA for Unicode support")
    hUserLibrary = ::LoadLibraryA((LPCSTR)strFullLibraryName.c_str());
    return hUserLibrary;
}


void OSUtils::GenericUnloadLibrary(LIB_HANDLE pLibrary)
{
    FreeLibrary(pLibrary);
}

LIB_HANDLE OSUtils::GetLibraryHandle(const char* szLibName)
{
    SP_TODO("revisit use of GetModuleHandleA for Unicode support")
    return GetModuleHandleA(szLibName);
}

void* OSUtils::GetSymbolAddr(LIB_HANDLE pModule, std::string strFunctionName)
{
    if (pModule == NULL)
    {
        return NULL;
    }

    if (strFunctionName.empty() == true)
    {
        return NULL;
    }

    return (void*)GetProcAddress(pModule, (LPCSTR)strFunctionName.c_str());
}

VOID CALLBACK TimerCallbackWrapper(
    HWND hwnd,
    UINT uMsg,
    UINT_PTR idEvent,
    DWORD dwTime)
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(dwTime);

    map<TIMERID, TimerCallbackFunc>::iterator it = sTimerCallbackTable.find(idEvent);

    if (it != sTimerCallbackTable.end())
    {
        TimerCallbackFunc callbackPtr = it->second;
        callbackPtr(idEvent);
    }
}

static unsigned __stdcall ThreadFuncWrapper(void* pArguments)
{
    ThreadFuncWrapperParam* pWrapperParam = (ThreadFuncWrapperParam*)pArguments;
    pWrapperParam->m_pFunc(pWrapperParam->m_pParam);
    delete pWrapperParam;
    return 0;
}

THREADHANDLE OSUtils::CreateThread(ThreadFunc pFunc, void* pParam)
{
    unsigned int tid;
    ThreadFuncWrapperParam* pWrapperParam = new(nothrow) ThreadFuncWrapperParam;
    SpAssertRet(pWrapperParam != NULL) 0;

    pWrapperParam->m_pFunc = pFunc;
    pWrapperParam->m_pParam = pParam;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &ThreadFuncWrapper, pWrapperParam, 0, &tid);

    if (hThread == 0)
    {
        return 0;
    }
    else
    {
        return hThread;
    }
}

void OSUtils::SleepMillisecond(unsigned int milisecond)
{
    Sleep(milisecond);
}

int OSUtils::Join(THREADHANDLE tid)
{
    return (int)WaitForSingleObject(tid, INFINITE);
}

bool OSUtils::WaitForProcess(PROCESSID pid)
{
    return WaitForSingleObject(pid.hProcess, INFINITE) == WAIT_OBJECT_0;
}

PROCESSID OSUtils::ExecProcess(const char* szExe,
                               const char* szArgs,
                               const char* szWorkingDir,
                               const char* szEnvBlock,
                               bool bCreateSuspended)
{
    SP_TODO("revisit use of STARTUPINFOA for Unicode support")
    STARTUPINFOA si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(pi));

    DWORD dwCreationFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE;

    if (bCreateSuspended)
    {
        dwCreationFlags |= CREATE_SUSPENDED;
    }

    SP_TODO("revisit use of CreateProcessA for Unicode support")
    CreateProcessA(szExe,
                   (LPSTR)szArgs,
                   NULL, NULL, TRUE,
                   dwCreationFlags,
                   (LPVOID)szEnvBlock,
                   szWorkingDir,
                   &si,
                   &pi);
    return pi;
}

bool OSUtils::SetEnvVar(const char* szName, const char* szVal)
{
    SP_TODO("revisit use of SetEnvironmentVariableA for Unicode support")
    SetEnvironmentVariableA(szName, szVal);
    return true;
}

bool OSUtils::UnsetEnvVar(const char* szName)
{
    SP_TODO("revisit use of SetEnvironmentVariableA for Unicode support")
    SetEnvironmentVariableA(szName, NULL);
    return true;
}

std::string OSUtils::GetEnvVar(const char* szName)
{
    char szTmpPath[ SP_MAX_PATH ];
    SP_TODO("revisit use of GetEnvironmentVariableA for Unicode support")
    DWORD ret = GetEnvironmentVariableA(szName, szTmpPath, SP_MAX_PATH);

    if (ret == 0)
    {
        return "";
    }
    else
    {
        return szTmpPath;
    }
}

ENVSYSBLOCK OSUtils::GetSysEnvBlock()
{
#ifdef UNICODE
    return GetEnvironmentStrings();
#else
    SP_TODO("revisit use of GetEnvironmentStrings for non-Unicode support")
    assert(!"GetSysEnvBlock not supported in non-Unicode mode");
    return NULL;
#endif
}

void OSUtils::ReleaseSysEnvBlock(ENVSYSBLOCK pEnvBlock)
{
    FreeEnvironmentStrings(const_cast<TCHAR*>(pEnvBlock));
}

bool OSUtils::osCopyFile(const char* szFrom, const char* szTo)
{
    SP_TODO("revisit use of CopyFileA for Unicode support")
    return CopyFileA(szFrom, szTo, FALSE) == TRUE;
}

bool OSUtils::osMoveFile(const char* szFrom, const char* szTo)
{
    SP_TODO("revisit use of MoveFileA for Unicode support")
    return MoveFileA(szFrom, szTo) != 0;
}

#elif defined(_LINUX) || defined(LINUX)

#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <dlfcn.h>
#include <sys/wait.h>


OSUtils::OSUtils() :
    m_pGetUserTime(NULL),
    m_pUserTimerInit(NULL),
    m_pUserTimerDestroy(NULL),
    m_bUserTimer(false),
    m_userTimerLibraryHandle(NULL)
{
}

ULONGLONG OSUtils::GetTimeNanos()
{
    struct timespec tp;

    if (m_bUserTimer == true)
    {
        return m_pGetUserTime();
    }
    else
    {
        clock_gettime(CLOCK_MONOTONIC, &tp);
        return (ULONGLONG) tp.tv_sec * (1000ULL * 1000ULL * 1000ULL) +
               (ULONGLONG) tp.tv_nsec;
    }
}

void OSUtils::SleepMillisecond(unsigned int milisecond)
{
    // usleep takes microseconds
    usleep(milisecond * 1000);
}

static void* ThreadFuncWrapper(void* pArguments)
{
    if (pArguments != NULL)
    {
        ThreadFuncWrapperParam* pWrapperParam = (ThreadFuncWrapperParam*)pArguments;
        pWrapperParam->m_pFunc(pWrapperParam->m_pParam);
        delete pWrapperParam;
    }

    return 0;
}

THREADHANDLE OSUtils::CreateThread(ThreadFunc pFunc, void* pParam)
{
    osThreadId tid;
    ThreadFuncWrapperParam* pWrapperParam = new(nothrow) ThreadFuncWrapperParam;
    SpAssertRet(pWrapperParam != NULL) 0;

    pWrapperParam->m_pFunc = pFunc;
    pWrapperParam->m_pParam = pParam;

    pthread_t thread;
    tid = pthread_create(&thread, NULL, ThreadFuncWrapper, pWrapperParam);

    if (tid != 0)
    {
        return 0;
    }
    else
    {
        return thread;
    }
}

int OSUtils::Join(THREADHANDLE tid)
{
    return pthread_join(tid, NULL);
}

PROCESSID OSUtils::ExecProcess(const char* szExe,
                               const char* szArgs,
                               const char* szWorkingDir,
                               const char* szEnvBlock,
                               bool bCreateSuspended)
{
    SP_UNREFERENCED_PARAMETER(bCreateSuspended);
    PROCESSID processId = fork();

    if (processId == 0)
    {
        // Child code

        // change working directory
        if (szWorkingDir != NULL)
        {
            if (chdir(szWorkingDir) == -1)
            {
                cout << "Failed to switch to working directory - " << szWorkingDir << endl;
            }
        }

        char* pArg;
        char* pPtr;
        char* argv[SP_MAX_ARG + 1];
        int argc;

        pArg = const_cast<char*>(szExe);

        argv[0] = pArg;
        argc = 1;

        if (szArgs[0] != '\0')
        {
            pArg = strtok_r(const_cast<char*>(szArgs) , " ", &pPtr);

            while (pArg != NULL)
            {
                argv[argc] = pArg;
                argc++;

                if (argc >= SP_MAX_ARG)
                {
                    break;
                }

                pArg = strtok_r(NULL, " ", &pPtr);
            }
        }

        argv[argc] = NULL;

        if (szEnvBlock != NULL)
        {
            char* env[SP_MAX_ENVVARS + 1];
            char* pEnv = const_cast<char*>(szEnvBlock);
            int numEnvVars = 0;

            while (pEnv[0] != '\0')
            {
                env[numEnvVars] = pEnv;
                pEnv += strlen(env[numEnvVars++]) + 1;

                if (numEnvVars >= SP_MAX_ENVVARS)
                {
                    break;
                }
            }

            env[numEnvVars] = NULL;
            execve(szExe, argv, env);
        }
        else
        {
            execv(szExe, argv);
        }

        exit(0);
    }

    return processId;
}

bool OSUtils::WaitForProcess(PROCESSID pid)
{
    if (pid <= 0)
    {
        // error
        return false;
    }
    else
    {
        waitpid(pid, NULL, 0);
        return true;
    }
}

bool OSUtils::SetEnvVar(const char* szName, const char* szVal)
{
    int res = setenv(szName, szVal, 1);

    if (res != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool OSUtils::UnsetEnvVar(const char* szName)
{
    unsetenv(szName);
    return true;
}

std::string OSUtils::GetEnvVar(const char* szName)
{
    char* var = getenv(szName);

    if (var == NULL)
    {
        return "";
    }
    else
    {
        return std::string(var);
    }
}

ENVSYSBLOCK OSUtils::GetSysEnvBlock()
{
    return const_cast<const char*>(*environ);
}

void OSUtils::ReleaseSysEnvBlock(ENVSYSBLOCK pEnvBlock)
{
    SP_UNREFERENCED_PARAMETER(pEnvBlock);
}

LIB_HANDLE OSUtils::GenericLoadLibrary(const std::string& strFullLibraryName)
{
    LIB_HANDLE hUserLibrary = NULL;
    hUserLibrary = dlopen(strFullLibraryName.c_str(), RTLD_LAZY);
    return hUserLibrary;
}


void OSUtils::GenericUnloadLibrary(LIB_HANDLE pLibrary)
{
    dlclose(pLibrary);
}

LIB_HANDLE OSUtils::GetLibraryHandle(const char* szLibName)
{
    return dlopen(szLibName, RTLD_NOLOAD);
}

void* OSUtils::GetSymbolAddr(LIB_HANDLE pLibrary, std::string strFunctionName)
{
    if (pLibrary == NULL)
    {
        return NULL;
    }

    if (strFunctionName.empty() == true)
    {
        return NULL;
    }

    return (void*)dlsym(pLibrary, strFunctionName.c_str());

}

// This implementation is from stackoverflow:  http://stackoverflow.com/questions/3680730/c-fileio-copy-vs-systemcp-file1-x-file2-x
bool OSUtils::osCopyMoveFileHelper(const char* szFrom, const char* szTo, bool bMove)
{
    bool retVal = false;

    int read_fd = open(szFrom, O_RDONLY);

    if (read_fd != -1)
    {
        // get the size
        struct stat stat_buf;
        fstat(read_fd, &stat_buf);

        // open the output file for writing, with the same permissions as the source file.
        int write_fd = open(szTo, O_WRONLY | O_CREAT, stat_buf.st_mode);

        if (write_fd != -1)
        {
            // move the contents from one file to the other with sendfile
            off_t offset = 0;
            retVal = sendfile(write_fd, read_fd, &offset, stat_buf.st_size) != -1;
            close(write_fd);
        }

        close(read_fd);

        // delete the file if moving
        if (bMove && retVal)
        {
            retVal = remove(szFrom) == 0;
        }
    }

    return retVal;
}


bool OSUtils::osCopyFile(const char* szFrom, const char* szTo)
{
    return osCopyMoveFileHelper(szFrom, szTo, false);
}

bool OSUtils::osMoveFile(const char* szFrom, const char* szTo)
{
    return osCopyMoveFileHelper(szFrom, szTo, true);
}

#endif

std::string OSUtils::GetOSInfo(void)
{
    std::string retVal;

    int majorVersion = 0;
    int minorVersion = 0;
    int buildNumber = 0;
    gtString osVersionName;

    bool success = osGetOperatingSystemVersionString(osVersionName);

    if (success)
    {
        std::stringstream osInfo(std::stringstream::in | std::stringstream::out);
        osInfo.clear();

        osInfo << osVersionName.asUTF8CharArray();

        success = osGetOperatingSystemVersionNumber(majorVersion, minorVersion, buildNumber);

        if (success)
        {
            osInfo << " " << "Build " << majorVersion << "." << minorVersion << "." << buildNumber;
        }

        retVal = osInfo.str();
    }
    else
    {
        retVal.clear();
    }

    return retVal;
}

void OSUtils::SetupUserTimer(const Parameters& params)
{
    //Check whether the user timer is to be used and load the appropriate timer
    if (params.m_bUserTimer)
    {
        //If the path is found, try to load the user-specified timer library
        //However, if unable to retrieve the function pointer, supply default timer function
        if (!params.m_strTimerDLLFile.empty())
        {
            std::string TimerDLL = params.m_strTimerDLLFile;
            m_userTimerLibraryHandle = (LIB_HANDLE)OSUtils::Instance()->GenericLoadLibrary(TimerDLL);

            if (m_userTimerLibraryHandle == NULL)
            {
                std::cout << "Unable to load user-timer library.  Reverting to default timer" << std::endl;
            }
            else
            {
                if (params.m_strUserTimerFn.empty() || params.m_strUserTimerInitFn.empty() || params.m_strUserTimerDestroyFn.empty())
                {
                    std::cout << "Unable to initialize function pointers in user-timer library.  Reverting to default timer" << std::endl;
                }
                else
                {
                    //check that we can actually get the time and timer init/destruction functions before setting
                    //the user timer
                    m_pUserTimerInit = (UserTimerInitProc)GetSymbolAddr(m_userTimerLibraryHandle, params.m_strUserTimerInitFn.c_str());
                    m_pUserTimerDestroy = (UserTimerDestroyProc)GetSymbolAddr(m_userTimerLibraryHandle, params.m_strUserTimerDestroyFn.c_str());
                    m_pGetUserTime = (GetTimeProc)GetSymbolAddr(m_userTimerLibraryHandle, params.m_strUserTimerFn.c_str());

                    if ((m_pUserTimerInit == NULL) || (m_pUserTimerDestroy == NULL) || (m_pGetUserTime == NULL))
                    {
                        std::cout << "Unable to initialize function pointers in user-timer library.  Reverting to default timer" << std::endl;
                    }
                    else
                    {
                        //Initialize the timer, but if it fails, signal that the system time is to be used
                        if (false == m_pUserTimerInit())
                        {
                            std::cout << "User-timer initialization failed.  Reverting to default timer" << std::endl;
                        }
                        else
                        {
                            m_bUserTimer = true;
                            std::cout << "User timer loaded." << std::endl;
                        }
                    }
                }
            }
        }
        else
        {
            std::cout << "User-timer library not found.  Reverting to default timer" << std::endl;
        }
    }
}

/// Shuts down the user timer
/// \param params the parameters to check
void OSUtils::ShutdownUserTimer()
{
    if (NULL != m_userTimerLibraryHandle && NULL != m_pUserTimerDestroy)
    {
        m_pUserTimerDestroy();
        m_pUserTimerDestroy = NULL;
        m_pUserTimerInit = NULL;
        m_pGetUserTime = NULL;
        GenericUnloadLibrary(m_userTimerLibraryHandle);
        m_userTimerLibraryHandle = NULL;
    }
}
