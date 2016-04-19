//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class provides wrappers for OS-specific functionality
//==============================================================================

#ifndef _OS_UTILS_H_
#define _OS_UTILS_H_

/// \addtogroup Common
// @{

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
#else
    #include <sys/types.h>
    #include <unistd.h>
#endif
#include "Defs.h"
#include "LocalTSingleton.h"

#ifdef _WIN32
    #define THREADHANDLE HANDLE
    #define TIMERID UINT_PTR
    #define PROCESSID PROCESS_INFORMATION
    #define LIB_HANDLE     HMODULE
    #define ENVSYSBLOCK const TCHAR*
#elif defined (_LINUX) || defined (LINUX)
    #define THREADHANDLE pthread_t
    #define TIMERID int
    #define PROCESSID pid_t
    #define LIB_HANDLE     void *
    #define ENVSYSBLOCK const char*
#endif

typedef void (*TimerCallbackFunc)(TIMERID timerID);
typedef void (*ThreadFunc)(void* param);

#ifdef WIN32
    #define SP_strcpy(des,size,src) strcpy_s(des, size, src)
    #define SP_wstrcpy(des,size,src) wcscpy_s(des, size, src)
    #define SP_strcat(des,size,src) strcat_s(des, size, src)
    #define SP_wstrcat(des,size,src) wcscat_s(des, size, src)
    #define SP_snprintf(buf, nLen, format, ...) _snprintf_s(buf, nLen, _TRUNCATE, format, ##__VA_ARGS__)
    #define SP_snwprintf(buf, nLen, format, ...) _snwprintf_s(buf, nLen, _TRUNCATE, format, ##__VA_ARGS__)
    #define SP_vsnwprintf(buf, nLen, format, ...) _vsnwprintf_s(buf, nLen, _TRUNCATE, format, ##__VA_ARGS__)
    #define SP_vsnprintf(buf, nLen, format, ...) vsnprintf_s(buf, nLen, _TRUNCATE, format, ##__VA_ARGS__)
    #define SP_sprintf(buf, nLen, format, ...) sprintf_s(buf, nLen, format, ##__VA_ARGS__)
    //#define SP_swprintf() swprintf_s
    #define SP_max(x,y) max(x,y)
    #define SP_getcwd(buf, len) _getcwd(buf, len)
#else
    #define SP_strcpy(des,size,src) strcpy(des, src)
    #define SP_wstrcpy(des,size,src) wcscpy(des, src)
    #define SP_strcat(des,size,src) strcat(des, src)
    #define SP_wstrcat(des,size,src) wcscat(des, src)
    #define SP_snprintf(buf, nLen, format, ...) snprintf(buf, nLen, format, ##__VA_ARGS__)
    #define SP_vsnprintf(buf, nLen, format, ...) vsnprintf(buf, nLen, format, ##__VA_ARGS__)
    #define SP_sprintf(buf, nLen, format, ...) sprintf(buf, format, ##__VA_ARGS__)
    #define SP_max(x,y) std::max(x,y)
    #define SP_getcwd(buf, len) getcwd(buf, len)
#endif

typedef ULONGLONG(*GetTimeProc)(void);
typedef bool (*UserTimerInitProc)(void);
typedef void (*UserTimerDestroyProc)(void);


/// structure used when creating threads
struct ThreadFuncWrapperParam
{
    ThreadFunc m_pFunc;   ///< thread function
    void*      m_pParam;  ///< param passed to thread function
};

#ifdef CL_UNITTEST_MOCK
    TSingletonMockGen(OSUtils, OSUtilsMock)
#endif

class OSUtils : public TSingleton<OSUtils>
{
    friend class TSingleton<OSUtils>;
public:

    /// Sets up the user timer based on the specified parameters
    /// \param params the parameters to check
    void SetupUserTimer(const Parameters& params);

    /// Shuts down the user timer
    void ShutdownUserTimer();

    /// Indicates whether or not a user timer is being used
    /// \return true if a user timer is being used, false otherwise
    bool IsUserTimerEnabled() const { return m_bUserTimer; }

    /// Get Current time in nano second
    /// Default time function, but if user selected
    /// time function is loaded, that function is called
    /// \return time
    ULONGLONG GetTimeNanos(void);

    /// Function to load a library
    /// \param strFullLibraryName name of library containing function to be called (including path)
    /// \return handle to library module
    LIB_HANDLE GenericLoadLibrary(const std::string& strFullLibraryName);

    /// Function to unload library
    /// \param pLibrary handle to library to be released
    void GenericUnloadLibrary(LIB_HANDLE pLibrary);

    /// Get loaded library handle
    /// \param szLibName library name
    /// \param library handle if libraray is loaded
    LIB_HANDLE GetLibraryHandle(const char* szLibName);

    /// Function to get a pointer to the timer functions in the user library
    /// \param pModule handle to library/module where function is found
    /// \param strFunctionName string containing the name of the function to be called
    /// \return pointer to function; NULL if function not located
    void* GetSymbolAddr(LIB_HANDLE pModule, std::string strFunctionName);

    /// Create a new thread
    /// \param pFunc function pointer
    /// \param param parameters to the new thread
    THREADHANDLE CreateThread(ThreadFunc pFunc, void* param);

    /// Sleep in millisecond
    /// \param milisecond Time in millisecond
    void SleepMillisecond(unsigned int milisecond);

    /// Join thread
    /// \param tid thread id
    int Join(THREADHANDLE tid);

    /// Create process
    /// \param szExe executable path
    /// \param szArgs command line argments, it can' be NULL, the first argment has to be exe path
    /// \param szWorkingDir working directory, not applicable to linux
    /// \param szEnvBlock a zero-separated, double-zero-terminated string containing the environment block.  NULL indicates that it should use the calling process' environment block
    /// \param bCreateSuspended created in suspended state
    /// \return process id(linux) or PROCESS_INFORMATION(windows)
    PROCESSID ExecProcess(const char* szExe, const char* szArgs, const char* szWorkingDir, const char* szEnvBlock, bool bCreateSuspended = false);

    /// Wait for a process to finish
    /// \param pid Process ID
    /// \return TRUE if succeeded
    bool WaitForProcess(PROCESSID pid);

    /// Set environment variable
    /// \param szName Variable name
    /// \param szVal Value
    /// \return true if succeed
    bool SetEnvVar(const char* szName, const char* szVal);

    /// Unset environment variable
    /// \param szName Variable name
    /// \return true if succeed
    bool UnsetEnvVar(const char* szName);

    /// Get environment variable
    /// \param szName Variable name
    /// \return value
    std::string GetEnvVar(const char* szName);

    /// Get the process environment block
    /// \return a pointer to the environment block
    ENVSYSBLOCK GetSysEnvBlock();

    /// Release an env block previously returned by GetSysteEnvBlock
    /// \param pEnvBlock pointer to the block previously returned by GetSysEnvBlock
    void ReleaseSysEnvBlock(ENVSYSBLOCK pEnvBlock);

    /// retrieve the OS information
    /// \return OS version
    std::string GetOSInfo(void);

    /// Copy file
    /// \param szFrom Source path
    /// \param szTo Destination path
    /// \return true if succeed.
    bool osCopyFile(const char* szFrom, const char* szTo);

    /// Move file
    /// \param szFrom Source path
    /// \param szTo Destination path
    /// \return true if succeed.
    bool osMoveFile(const char* szFrom, const char* szTo);

protected:
    OSUtils();

private:
#ifdef _WIN32
    double               m_dInvFrequency;          ///< Inverse of timer frequency
#endif

    GetTimeProc          m_pGetUserTime;           ///< Pointer to user timer function: GetTime()
    UserTimerInitProc    m_pUserTimerInit;         ///< Pointer to user timer initialization function: InitTimer()
    UserTimerDestroyProc m_pUserTimerDestroy;      ///< Pointer to user timer destruction function: DestroyTimer()
    bool                 m_bUserTimer;             ///< flag to signal the use of the standard timer (value is false) or a user timer (value is true)
    LIB_HANDLE           m_userTimerLibraryHandle; ///< handle to the user timer library

#if defined(_LINUX) || defined(LINUX)
    /// helper function used by Linux versions of osCopyFile and osMoveFile
    /// \param szFrom Source path
    /// \param szTo Destination path
    /// \param bMove flag indicating whether or not this is a copy or a move
    /// \return true if succeed.
    bool osCopyMoveFileHelper(const char* szFrom, const char* szTo, bool bMove);
#endif
};

// @}


#endif //_OS_UTILS_H_
