//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  DLL Injection class definitions
//=============================================================================
#ifndef __DLLINJECTOR_H__
#define __DLLINJECTOR_H__

// ---------------------------------------------------------------------------
/// Implementation-specific base class
// ---------------------------------------------------------------------------
class DLLInjectorImpl
{
public:
    /// constructor
    DLLInjectorImpl()
    {
    }

    /// destructor
    virtual ~DLLInjectorImpl()
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
    virtual bool CalledFromRundll() = 0;

    // ---------------------------------------------------------------------------
    /// Inject a dll into a suspended process. This is an external function called
    /// from rundll32.exe when process-hopping between 64 and 32 bit processes.
    /// As such, the arguments to this function will be provided by rundll32
    /// \param lpszCmdLine The command line arguments passed to this function. This
    ///        consists of a string containing the process ID of the application
    ///        to inject into and the full path of the dll to inject.
    // ---------------------------------------------------------------------------
    virtual void Inject(LPSTR lpszCmdLine) = 0;

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
                                                   _In_ LPCSTR lpDllName) = 0;

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
                                                   _In_ LPCSTR lpDllName) = 0;
};

// ---------------------------------------------------------------------------
/// DLL Injector container class
// ---------------------------------------------------------------------------
class DLLInjector
{
public:
    /// constructor
    DLLInjector();

    /// destructor
    ~DLLInjector();

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
    bool CalledFromRundll();

    // ---------------------------------------------------------------------------
    /// Inject a dll into a suspended process. This is an external function called
    /// from rundll32.exe when process-hopping between 64 and 32 bit processes.
    /// As such, the arguments to this function will be provided by rundll32
    /// \param lpszCmdLine The command line arguments passed to this function. This
    ///        consists of a string containing the process ID of the application
    ///        to inject into and the full path of the dll to inject.
    // ---------------------------------------------------------------------------
    void Inject(LPSTR lpszCmdLine);

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
                                           _In_ LPCSTR lpDllName);

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
                                           _In_ LPCSTR lpDllName);

private:
    DLLInjectorImpl* m_pImpl;  ///< pointer to the implementation object
};

#endif // __DLLINJECTOR_H__
