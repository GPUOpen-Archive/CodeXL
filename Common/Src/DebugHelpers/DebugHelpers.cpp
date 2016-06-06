//=====================================================================
//
// Author: Seth Sowerby
//         Graphics Products Group
//         Developer Tools
//         AMD, Inc.
//
// DebugHelpers.cpp
//
//=====================================================================
// $Id: //devtools/main/Common/Src/DebugHelpers/DebugHelpers.cpp#4 $
//
// Last checkin:  $DateTime: 2015/11/25 07:43:31 $
// Last edited by: $Author: rbober $
//=====================================================================
//   ( C ) AMD, Inc. 2008 All rights reserved.
//=====================================================================

#include <Windows.h>
#include <tchar.h>
#include <Tlhelp32.h.>

#include "DebugHelpers.h"

using namespace std;
using namespace DebugHelpers;

CDebugHelpers::CDebugHelpers(void)
{
    m_hDbgHelp = NULL;
    ClearFunctionPointers();
    Initialize();
}

CDebugHelpers::~CDebugHelpers(void)
{
    if (m_pSymCleanup != NULL)
    {
        m_pSymCleanup(GetCurrentProcess());
    }

    UnloadDbgHelp();
}

bool CDebugHelpers::Initialize()
{
    if (LoadDbgHelp())
    {
        /// \todo Replace this with code to build the symbol search path & add interface for passing it in as well
        TCHAR szSearchPath[] = _T("SRV*C:/Work/Symbols*http://msdl.microsoft.com/download/symbols;C:/Perforce/lmperforce/3darg/Tools/PerfTools/PerfStudio2/Server/bin/debug/plugins;C:/Program Files/Microsoft DirectX SDK (November 2008)/Extras/Symbols/retail/x86/exe");

        if (m_pSymInitialize(GetCurrentProcess(), szSearchPath, TRUE) == FALSE)
        {
            OutputDebugString(_T("DebugHelpers: Error - SymInitialize failed"));
        }

        DWORD dwOptions = m_pSymGetOptions() | SYMOPT_UNDNAME;
        m_pSymSetOptions(dwOptions);
    }

    return false;
}

bool CDebugHelpers::CheckValid()
{
    if (m_hDbgHelp == NULL)
    {
        return false;
    }

    if (m_pStackWalk64 == NULL)
    {
        return false;
    }

    if (m_pSymInitialize == NULL)
    {
        return false;
    }

    if (m_pSymCleanup == NULL)
    {
        return false;
    }

    if (m_pSymGetOptions == NULL)
    {
        return false;
    }

    if (m_pSymSetOptions == NULL)
    {
        return false;
    }

    if (m_pSymGetSymFromAddr64 == NULL)
    {
        return false;
    }

    if (m_pSymGetLineFromAddr64 == NULL)
    {
        return false;
    }

    if (m_pSymFunctionTableAccess64 == NULL)
    {
        return false;
    }

    if (m_pSymGetModuleBase64 == NULL)
    {
        return false;
    }

    if (m_pSymGetModuleInfo64 == NULL)
    {
        return false;
    }

    if (m_pSymLoadModule64 == NULL)
    {
        return false;
    }

    if (m_pSymUnloadModule64 == NULL)
    {
        return false;
    }

    return true;
}

bool CDebugHelpers::LoadDbgHelp()
{
    if (m_hDbgHelp != NULL)
    {
        return false;
    }

    m_hDbgHelp = LoadLibrary(_T("DbgHelp.dll"));

    if (m_hDbgHelp == NULL)
    {
        OutputDebugString(_T("DebugHelpers: Error - could not load DbgHelp.dll"));
        return false;
    }

    m_pStackWalk64 = (StackWalk64PROC) GetProcAddress(m_hDbgHelp, "StackWalk64");

    if (m_pStackWalk64 == NULL)
    {
        OutputDebugString(_T("DebugHelpers: Error - could not get StackWalk64 address"));
        UnloadDbgHelp();
        return false;
    }

#ifdef _UNICODE
    m_pSymInitialize = (SymInitializePROC) GetProcAddress(m_hDbgHelp, "SymInitializeW");
#else // ANSI
    m_pSymInitialize = (SymInitializePROC) GetProcAddress(m_hDbgHelp, "SymInitialize");
#endif // ANSI

    if (m_pSymInitialize == NULL)
    {
        OutputDebugString(_T("DebugHelpers: Error - could not get SymInitialize address"));
        UnloadDbgHelp();
        return false;
    }

    m_pSymGetOptions = (SymGetOptionsPROC) GetProcAddress(m_hDbgHelp, "SymGetOptions");

    if (m_pSymGetOptions == NULL)
    {
        OutputDebugString(_T("DebugHelpers: Error - could not get SymGetOptions address"));
        UnloadDbgHelp();
        return false;
    }

    m_pSymSetOptions = (SymSetOptionsPROC) GetProcAddress(m_hDbgHelp, "SymSetOptions");

    if (m_pSymSetOptions == NULL)
    {
        OutputDebugString(_T("DebugHelpers: Error - could not get SymSetOptions address"));
        UnloadDbgHelp();
        return false;
    }

    m_pSymCleanup = (SymCleanupPROC) GetProcAddress(m_hDbgHelp, "SymCleanup");

    if (m_pSymCleanup == NULL)
    {
        OutputDebugString(_T("DebugHelpers: Error - could not get SymCleanup address"));
        UnloadDbgHelp();
        return false;
    }

    m_pSymGetSymFromAddr64 = (SymGetSymFromAddr64PROC) GetProcAddress(m_hDbgHelp, "SymGetSymFromAddr64");

    if (m_pSymGetSymFromAddr64 == NULL)
    {
        OutputDebugString(_T("DebugHelpers: Error - could not get SymGetSymFromAddr64 address"));
        UnloadDbgHelp();
        return false;
    }

    m_pSymGetLineFromAddr64 = (SymGetLineFromAddr64PROC) GetProcAddress(m_hDbgHelp, "SymGetLineFromAddr64");

    if (m_pSymGetLineFromAddr64 == NULL)
    {
        OutputDebugString(_T("DebugHelpers: Error - could not get SymGetLineFromAddr64 address"));
        UnloadDbgHelp();
        return false;
    }

    m_pSymFunctionTableAccess64 = (PFUNCTION_TABLE_ACCESS_ROUTINE64) GetProcAddress(m_hDbgHelp, "SymFunctionTableAccess64");

    if (m_pSymFunctionTableAccess64 == NULL)
    {
        OutputDebugString(_T("DebugHelpers: Error - could not get SymFunctionTableAccess64 address"));
        UnloadDbgHelp();
        return false;
    }

    m_pSymGetModuleBase64 = (PGET_MODULE_BASE_ROUTINE64) GetProcAddress(m_hDbgHelp, "SymGetModuleBase64");

    if (m_pSymGetModuleBase64 == NULL)
    {
        OutputDebugString(_T("DebugHelpers: Error - could not get SymFunctionTableAccess64 address"));
        UnloadDbgHelp();
        return false;
    }

    m_pSymGetModuleInfo64 = (SymGetModuleInfo64PROC) GetProcAddress(m_hDbgHelp, "SymGetModuleInfo64");

    if (m_pSymGetModuleInfo64 == NULL)
    {
        OutputDebugString(_T("DebugHelpers: Error - could not get SymGetModuleInfo64 address"));
        UnloadDbgHelp();
        return false;
    }

    m_pSymLoadModule64 = (SymLoadModule64PROC) GetProcAddress(m_hDbgHelp, "SymLoadModule64");

    if (m_pSymLoadModule64 == NULL)
    {
        OutputDebugString(_T("DebugHelpers: Error - could not get SymLoadModule64 address"));
        UnloadDbgHelp();
        return false;
    }

    m_pSymUnloadModule64 = (SymUnloadModule64PROC) GetProcAddress(m_hDbgHelp, "SymUnloadModule64");

    if (m_pSymUnloadModule64 == NULL)
    {
        OutputDebugString(_T("DebugHelpers: Error - could not get SymUnloadModule64 address"));
        UnloadDbgHelp();
        return false;
    }

    return true;
}

void CDebugHelpers::UnloadDbgHelp()
{
    if (m_hDbgHelp != NULL)
    {
        FreeLibrary(m_hDbgHelp);
        m_hDbgHelp = NULL;
    }

    ClearFunctionPointers();
}

void CDebugHelpers::ClearFunctionPointers()
{
    m_pStackWalk64 = NULL;
    m_pSymInitialize = NULL;
    m_pSymCleanup = NULL;
    m_pSymGetOptions = NULL;
    m_pSymSetOptions = NULL;
    m_pSymGetSymFromAddr64 = NULL;
    m_pSymGetLineFromAddr64 = NULL;
    m_pSymFunctionTableAccess64 = NULL;
    m_pSymGetModuleBase64 = NULL;
    m_pSymGetModuleInfo64 = NULL;
    m_pSymLoadModule64 = NULL;
    m_pSymUnloadModule64 = NULL;
}

bool CDebugHelpers::LoadModules(HANDLE hProcess, std::vector<LoadedModule>& loadedModules)
{
    loadedModules.clear();

    DWORD dwProcessID = GetProcessId(hProcess);

    if (dwProcessID == 0)
    {
        return false;
    }

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessID);

    if (hSnapshot == (HANDLE) - 1)
    {
        return false;
    }

    MODULEENTRY32 module;
    BOOL bFoundModule = Module32First(hSnapshot, &module);

    while (bFoundModule)
    {
        LoadedModule loadedModule;
        loadedModule.dwBaseAddress = m_pSymLoadModule64(hProcess, NULL, module.szExePath, module.szModule, (DWORD64) module.modBaseAddr, module.modBaseSize);

        if (loadedModule.dwBaseAddress != NULL)
        {
            //          static const int nBufSize = 256;
            //          TCHAR szMessage[nBufSize];
            //          sprintf_s(szMessage, nBufSize, _T("Module %s loaded\n"), module.szModule);
            //          OutputDebugString(szMessage);
            //
            loadedModule.strModulePath = module.szExePath;
            loadedModules.push_back(loadedModule);
        }
        else
        {
            //          static const int nBufSize = 256;
            //          TCHAR szMessage[nBufSize];
            //          sprintf_s(szMessage, nBufSize, _T("Module %s load failed\n"), module.szModule);
            //          OutputDebugString(szMessage);
        }

        bFoundModule = Module32Next(hSnapshot, &module);
    }

    CloseHandle(hSnapshot);

    return true;
}

bool CDebugHelpers::UnloadModules(HANDLE hProcess, std::vector<LoadedModule>& loadedModules)
{
    vector<LoadedModule>::iterator it = loadedModules.begin();

    // Then iterate through the remaining stack members adding them to the string.
    while (it != loadedModules.end())
    {
        LoadedModule& module = *it++;
        m_pSymUnloadModule64(hProcess, module.dwBaseAddress);
    }

    return true;
}

bool CDebugHelpers::GetCallStack(vector<String>& strCallStackList)
{
    if (CheckValid() == false)
    {
        return false;
    }

    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
    std::vector<LoadedModule> loadedModules;

    if (LoadModules(hProcess, loadedModules) == false)
    {
        UnloadModules(hProcess, loadedModules);
        return false;
    }

    CONTEXT context;
    ZeroMemory(&context, sizeof(context));
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

    STACKFRAME64 stackFrame;
    ZeroMemory(&stackFrame, sizeof(stackFrame));
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Mode = AddrModeFlat;
#ifdef _M_IX86
    DWORD dwMachineType = IMAGE_FILE_MACHINE_I386;
    stackFrame.AddrPC.Offset = context.Eip;
    stackFrame.AddrFrame.Offset = context.Ebp;
    stackFrame.AddrStack.Offset = context.Esp;
#elif _M_X64
    DWORD dwMachineType = IMAGE_FILE_MACHINE_AMD64;
    stackFrame.AddrPC.Offset = context.Rip;
    stackFrame.AddrFrame.Offset = context.Rsp;
    stackFrame.AddrStack.Offset = context.Rsp;
#endif

    do
    {
        m_pStackWalk64(dwMachineType, hProcess, hThread, &stackFrame, &context, NULL, NULL/*m_pSymFunctionTableAccess64*/, NULL/*m_pSymGetModuleBase64*/, NULL);

        String strSymbol;
        GetSymbolName(stackFrame.AddrPC.Offset, strSymbol);
        strCallStackList.push_back(strSymbol);
    }
    while (stackFrame.AddrReturn.Offset != 0 && stackFrame.AddrPC.Offset != stackFrame.AddrReturn.Offset);

    UnloadModules(hProcess, loadedModules);

    return !strCallStackList.empty();
}

bool CDebugHelpers::GetCallStack(String& strCallStack)
{
    strCallStack.clear();

    vector<String> strCallStackList;

    if (GetCallStack(strCallStackList))
    {
        vector<String>::iterator it = strCallStackList.begin();

        // Skip the first entry on the call stack as that is this routine.
        if (it != strCallStackList.end())
        {
            it++;
        }

        // Then iterate through the remaining stack members adding them to the string.
        while (it != strCallStackList.end())
        {
            if (!strCallStack.empty())
            {
                strCallStack += '\n';
            }

            strCallStack += *it++;
        }

        return !strCallStack.empty();
    }
    else
    {
        strCallStack.clear();
        return false;
    }
}

bool CDebugHelpers::GetSymbolName(DWORD64 dwAddress, String& strSymbol)
{
    TCHAR szName[s_dwMaxNameLength];
    _stprintf_s(szName, s_dwMaxNameLength, _T("0x%llu - Unknown Function"), dwAddress);

    strSymbol = szName;

    if (dwAddress == NULL)
    {
        return false;
    }

    HANDLE hProcess = GetCurrentProcess();

    char symbol[s_dwSymbolSize];
    IMAGEHLP_SYMBOL64* pSymbol = (IMAGEHLP_SYMBOL64*) symbol;
    ZeroMemory(pSymbol, s_dwSymbolSize);
    pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
    pSymbol->MaxNameLength = s_dwMaxNameLength;

    DWORD dwLineNumber = 0xffffffff;

    IMAGEHLP_LINE64 line;
    ZeroMemory(&line, sizeof(line));
    line.SizeOfStruct = sizeof(line);

    IMAGEHLP_MODULE64 moduleInfo;
    ZeroMemory(&moduleInfo, sizeof(IMAGEHLP_MODULE64));
    moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

    DWORD64 dwDisplacement = 0;

    if (m_pSymGetLineFromAddr64(hProcess, dwAddress, &dwDisplacement, &line))
    {
        dwLineNumber = line.LineNumber;
    }

    if (m_pSymGetSymFromAddr64(hProcess, dwAddress, &dwDisplacement, pSymbol))
    {
        if (dwLineNumber != 0xffffffff)
        {
            _stprintf_s(szName, s_dwMaxNameLength, _T("0x%p - %s+0x%x - Line %llu"), (DWORD*) dwAddress, pSymbol->Name, dwLineNumber, dwDisplacement);
        }
        else
        {
            _stprintf_s(szName, s_dwMaxNameLength, _T("0x%p - %s+0x%llu"), (DWORD*) dwAddress, pSymbol->Name, dwDisplacement);
        }

        strSymbol = szName;

        return true;
    }

    return false;
}