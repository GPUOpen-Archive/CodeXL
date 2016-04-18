//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Stack tracer
//==============================================================================

#ifndef _STACK_TRACER_H_
#define _STACK_TRACER_H_

#include <string>
#include <vector>
#include <map>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "OSUtils.h"
#include "AMDTMutex.h"

#ifdef _WIN32
    #pragma warning( push )
    #pragma warning( disable : 4091 )
    #include <dbghelp.h>
    #pragma warning( pop )
    #include <tchar.h>
#else
    #include <execinfo.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
#endif

typedef std::map<std::string, bool> DbgInfoMap;
typedef std::pair<std::string, bool> DbgInfoMapPair;

#ifdef _WIN32
typedef BOOL (WINAPI FAR* StackWalk64Proc)(DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME64 StackFrame,
                                           PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
                                           PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
                                           PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
                                           PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);

typedef BOOL (WINAPI FAR* SymInitializeProc)(HANDLE hProcess, PCSTR UserSearchPath, BOOL fInvadeProcess);
typedef BOOL (WINAPI FAR* SymCleanupProc)(HANDLE hProcess);
typedef DWORD (WINAPI FAR* SymGetOptionsProc)();
typedef DWORD (WINAPI FAR* SymSetOptionsProc)(DWORD SymOptions);
typedef BOOL (WINAPI FAR* SymGetSymFromAddr64Proc)(HANDLE hProcess, DWORD64 dwAddress, PDWORD64 pdwDisplacement, PIMAGEHLP_SYMBOL64 Symbol);
typedef BOOL (WINAPI FAR* SymGetLineFromAddr64Proc)(HANDLE hProcess, DWORD64 dwAddress, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line);
typedef BOOL (WINAPI FAR* SymGetModuleInfo64Proc)(HANDLE hProcess, DWORD64 dwAddress, PIMAGEHLP_MODULE64 ModuleInfo);
typedef DWORD64(WINAPI FAR* SymLoadModule64Proc)(HANDLE hProcess, HANDLE hFile, PCSTR ImageName, PCSTR ModuleName, DWORD64 BaseOfDll, DWORD SizeOfDll);
typedef BOOL (WINAPI FAR* SymUnloadModule64Proc)(HANDLE hProcess, DWORD64 BaseOfDll);
#endif

#ifdef WIN32
    #define LineNum DWORD
    #define Address DWORD64
#else
    #define LineNum size_t
    #define Address void*
#endif

//------------------------------------------------------------------------------------
/// Stack Entry
/// On Linux, we can extact individual attributes but since we don't have a GUI frontend,
/// string message is enough.
//------------------------------------------------------------------------------------
struct StackEntry
{
    Address m_dwAddress;                ///< Address
    LineNum m_dwDisplacement;           ///< Displacement
    LineNum m_dwLineNum;                ///< Line number
    std::string m_strSymAddr;           ///< String version of Address
    std::string m_strFile;              ///< File
    std::string m_strModName;           ///< Module name
    std::string m_strSymName;           ///< Symbol name

    /// Constructor
    StackEntry()
    {
        m_strSymAddr.clear();
        m_strFile.clear();
        m_strModName.clear();
        m_dwLineNum = 0;
        m_dwAddress = 0;
        m_strSymName.clear();
        m_dwDisplacement = 0;
    }

    /// Copy constructor
    /// \param en Stack entry to be copied
    StackEntry(const StackEntry& en)
    {
        m_strSymAddr = en.m_strSymAddr;
        m_strFile = en.m_strFile;
        m_strModName = en.m_strModName;
        m_dwLineNum = en.m_dwLineNum;
        m_dwAddress = en.m_dwAddress;
        m_strSymName = en.m_strSymName;
        m_dwDisplacement = en.m_dwDisplacement;
    }
};

//------------------------------------------------------------------------------------
/// Singleton class that used to collect stack trace
//------------------------------------------------------------------------------------
class StackTracer : public TSingleton<StackTracer>
{
    friend class TSingleton<StackTracer>;
public:
    /// Constructor
    StackTracer(void);

    /// Destructor
    ~StackTracer(void);

    /// Init symbol file path
    /// \param pszSymPath optional search path
    bool InitSymPath(wchar_t* pszSymPath = NULL);

    /// Is initialized
    /// \return whether or not Stack tracer is initialized.
    bool IsInitialized()
    {
        return m_bInit;
    }

    /// Stack walker
    /// \param[out] stackTrace Stack entries
    /// \param[in]  bGetSymbol flag indicating whether or not to get symbol info immediately,
    ///             for normal mode, in order to minimize trace overhead, we retrieve symbol info during program shutdown. Ignored for Linux.
    /// \param[in]  tid ThreadID for which stack trace is performed, Optional, default is current thread. Ingored for Linux
    bool GetStackTrace(std::vector<StackEntry>& stackTrace, bool bGetSymbol = true, osThreadId tid = 0);

    /// Get Symbol Name
    /// \param[in]    dwAddress Address
    /// \param[out]   Output stack entry
    bool GetSymbolName(Address dwAddress, StackEntry& en);

#ifdef _WIN32
private:
    /// Dynamically load dbghelp.dll
    /// \return true if succeed
    bool LoadDbgHelp();

    /// Unload dbghelp.dll
    void UnloadDbgHelp();
#endif
private:
#ifdef _WIN32
    PIMAGEHLP_SYMBOL64 m_pSymbol;                                  ///< pre-allocated symbol object
    unsigned int m_pSymbolSize;                                    ///< Size of symbol object
    HANDLE m_hProcess;                                             ///< Process handle
    AMDTMutex* m_pMtx;                                             ///< Mutex to protect non-thread-safe win32 APIs
    HMODULE m_hDbgHelp;                                            ///< A handle to the DbgHelp.dll library.
    StackWalk64Proc m_pStackWalk64;                                ///< Function pointer to StackWalk64.
    SymInitializeProc m_pSymInitialize;                            ///< Function pointer to SymInitialize.
    SymCleanupProc m_pSymCleanup;                                  ///< Function pointer to SymCleanup.
    SymGetOptionsProc m_pSymGetOptions;                            ///< Function pointer to SymGetOptions.
    SymSetOptionsProc m_pSymSetOptions;                            ///< Function pointer to SymSetOptions.
    SymGetSymFromAddr64Proc m_pSymGetSymFromAddr64;                ///< Function pointer to SymGetSymFromAddr64.
    SymGetLineFromAddr64Proc m_pSymGetLineFromAddr64;              ///< Function pointer to SymGetLineFromAddr64.
    PFUNCTION_TABLE_ACCESS_ROUTINE64 m_pSymFunctionTableAccess64;  ///< Function pointer to SymFunctionTableAccess64.
    PGET_MODULE_BASE_ROUTINE64 m_pSymGetModuleBase64;              ///< Function pointer to SymGetModuleBase64.
    SymGetModuleInfo64Proc m_pSymGetModuleInfo64;                  ///< Function pointer to SymGetModuleInfo64.
    SymLoadModule64Proc m_pSymLoadModule64;                        ///< Function pointer to SymLoadModule64.
    SymUnloadModule64Proc m_pSymUnloadModule64;                    ///< Function pointer to SymUnloadModule64.
#else // Linux specific mem vars
    /// Check debug info availability
    /// \param strMod Module name
    /// \return true if debug info is available
    bool CheckDbgInfo(const std::string& strMod);

    DbgInfoMap m_dbgInfoMods;                                      ///< Debug info map
#endif
    bool m_bInit;                                                  ///< Init flag
};

#endif //_STACK_TRACER_H_
