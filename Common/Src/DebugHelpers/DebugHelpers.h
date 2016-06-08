//=====================================================================
//
// Author: Seth Sowerby
//         Graphics Products Group
//         Developer Tools
//         AMD, Inc.
//
// DebugHelpers.h
//
//=====================================================================
// $Id: //devtools/main/Common/Src/DebugHelpers/DebugHelpers.h#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author: salgrana $
//=====================================================================
//   ( C ) AMD, Inc. 2008 All rights reserved.
//=====================================================================

#pragma once

#ifndef DEBUGHELPERS_H
#define DEBUGHELPERS_H

#include <Windows.h>
#pragma warning( push )
#pragma warning( disable : 4091)
#include <dbghelp.h>
#pragma warning( pop )
#include <string>
#include <vector>

#ifdef UNICODE
    typedef std::wstring String;
#else // !UNICODE
    typedef std::string String;
#endif // !UNICODE

typedef BOOL (WINAPI FAR* StackWalk64PROC)(DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME64 StackFrame,
                                           PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
                                           PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
                                           PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
                                           PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);

typedef BOOL (WINAPI FAR* SymInitializePROC)(HANDLE hProcess, PTCH UserSearchPath, BOOL fInvadeProcess);

typedef BOOL (WINAPI FAR* SymCleanupPROC)(HANDLE hProcess);

typedef DWORD (WINAPI FAR* SymGetOptionsPROC)();

typedef DWORD (WINAPI FAR* SymSetOptionsPROC)(DWORD SymOptions);

typedef BOOL (WINAPI FAR* SymGetSymFromAddr64PROC)(HANDLE hProcess, DWORD64 dwAddress, PDWORD64 pdwDisplacement, PIMAGEHLP_SYMBOL64 Symbol);

typedef BOOL (WINAPI FAR* SymGetLineFromAddr64PROC)(HANDLE hProcess, DWORD64 dwAddress, PDWORD64 pdwDisplacement, PIMAGEHLP_LINE64 Line);

typedef BOOL (WINAPI FAR* SymGetModuleInfo64PROC)(HANDLE hProcess, DWORD64 dwAddress, PIMAGEHLP_MODULE64 ModuleInfo);

typedef DWORD64(WINAPI FAR* SymLoadModule64PROC)(HANDLE hProcess, HANDLE hFile, PSTR ImageName, PSTR ModuleName, DWORD64 BaseOfDll, DWORD SizeOfDll);

typedef DWORD64(WINAPI FAR* SymUnloadModule64PROC)(HANDLE hProcess, DWORD64 BaseOfDll);

/// DebugHelpers is the namespace for the Debug Helpers.

namespace DebugHelpers
{

/// CDebugHelpers provides a simple interface around the DbgHelp.dll functions for walking stack & decoding
/// addresses to symbol names.

class CDebugHelpers
{
public:
    /// CDebugHelpers constructor.
    /// This calls Initialize to load the DbgHelp.dll library so it so not a lightweight constructor.
    /// You should seek to avoid repeated instantiation of CDebugHelpers objects for performance reasons.
    CDebugHelpers(void);

    /// CDebugHelpers destructor
    virtual ~CDebugHelpers(void);

    /// Get the names of functions in the call stack.
    /// \param[in] strCallStackList A vector of function name strings.
    /// \return True if successful, otherwise false.
    bool GetCallStack(std::vector<String>& strCallStackList);

    /// Get the names of functions in the call stack.
    /// This is a wrapper around the GetCallStack(std::vector<String>& strCallStackList) function.
    /// \param[in] strCallStack The function name seperated by new-lines.
    /// \return True if successful, otherwise false.
    bool GetCallStack(String& strCallStack);

    /// Get the name of the symbol matching the specified address.
    /// \param[in] dwAddress The address for which to get the symbol name.
    /// \param[out] strSymbol A string representing the name of the symbol.
    /// \return True if successful, otherwise false.
    bool GetSymbolName(DWORD64 dwAddress, String& strSymbol);

protected:
    /// Initialize the object.
    /// \return True if successful, otherwise false.
    bool Initialize();

    /// Load the DbgHelp.dll library & get the addresses of the library functions.
    /// \return True if successful, otherwise false.
    bool LoadDbgHelp();

    /// Unload the DbgHelp.dll library.
    void UnloadDbgHelp();

    /// Clear the function pointers for the DbgHelp.dll library functions.
    void ClearFunctionPointers();

    /// Check that the library is successfully initialized & ready for business.
    bool CheckValid();

    typedef struct
    {
        std::string strModulePath;
        DWORD64 dwBaseAddress;
    } LoadedModule;

    /// Load the modules used in the process
    bool LoadModules(HANDLE hProcess, std::vector<LoadedModule>& loadedModules);

    /// Unload the modules listed in the module list
    bool UnloadModules(HANDLE hProcess, std::vector<LoadedModule>& loadedModules);

    /// A handle to the DbgHelp.dll library.
    HMODULE m_hDbgHelp;

    /// Function pointer to StackWalk64.
    StackWalk64PROC m_pStackWalk64;

    /// Function pointer to SymInitialize.
    SymInitializePROC m_pSymInitialize;

    /// Function pointer to SymCleanup.
    SymCleanupPROC m_pSymCleanup;

    /// Function pointer to SymGetOptions.
    SymGetOptionsPROC m_pSymGetOptions;

    /// Function pointer to SymSetOptions.
    SymSetOptionsPROC m_pSymSetOptions;

    /// Function pointer to SymGetSymFromAddr64.
    SymGetSymFromAddr64PROC m_pSymGetSymFromAddr64;

    /// Function pointer to SymGetLineFromAddr64.
    SymGetLineFromAddr64PROC m_pSymGetLineFromAddr64;

    /// Function pointer to SymFunctionTableAccess64.
    PFUNCTION_TABLE_ACCESS_ROUTINE64 m_pSymFunctionTableAccess64;

    /// Function pointer to SymGetModuleBase64.
    PGET_MODULE_BASE_ROUTINE64 m_pSymGetModuleBase64;

    /// Function pointer to SymGetModuleInfo64.
    SymGetModuleInfo64PROC m_pSymGetModuleInfo64;

    /// Function pointer to SymLoadModule64.
    SymLoadModule64PROC m_pSymLoadModule64;

    /// Function pointer to SymUnloadModule64.
    SymUnloadModule64PROC m_pSymUnloadModule64;

    /// Vector of symbol search path strings.
    std::vector<String> m_strSymbolPathList;

    /// Maximum symbol name size supported.
    static const DWORD s_dwMaxNameLength = 1024;

    /// Maximum symbol size supported.
    static const DWORD s_dwSymbolSize = sizeof(IMAGEHLP_SYMBOL64) + s_dwMaxNameLength;
};

} // namespace DebugHelpers

#endif // DEBUGHELPERS_H
