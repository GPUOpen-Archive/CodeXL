//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Stack tracer
//==============================================================================

#include <iostream>
#include "StackTracer.h"
#include "StringUtils.h"
#include "Logger.h"

#include <AMDTBaseTools/Include/gtString.h>

using namespace std;
using namespace GPULogger;

#define MAX_TRACE_SIZE 20

StackTracer::StackTracer(void)
{
#ifdef _WIN32
    m_pSymbolSize = (SP_MAX_PATH + sizeof(PIMAGEHLP_SYMBOL));
    m_pSymbol = (PIMAGEHLP_SYMBOL64)malloc(m_pSymbolSize);
    m_hProcess = GetCurrentProcess();
    m_pMtx = new(nothrow) AMDTMutex("stackTracer");
    SpAssert(m_pMtx != NULL);
#endif
    m_bInit = false;
}

bool StackTracer::InitSymPath(wchar_t* pszSymPath)
{
    SP_UNREFERENCED_PARAMETER(pszSymPath);
#ifdef _WIN32

    if (!LoadDbgHelp())
    {
        return false;
    }

    SP_TODO("revisit the setting of the symbol path");
    /*
    TCHAR lpszPath[SP_MAX_PATH];
    gtString lpszSymbolPath(L".;..\\;..\\..\\");

    // Creating the default path where the dgbhelp.dll is located
    // ".;%_NT_SYMBOL_PATH%;%_NT_ALTERNATE_SYMBOL_PATH%;%SYSTEMROOT%;%SYSTEMROOT%\System32;"

    // environment variable _NT_SYMBOL_PATH
    if (GetEnvironmentVariable(_T("_NT_SYMBOL_PATH"), lpszPath, SP_MAX_PATH))
    {
        lpszSymbolPath.appendFormattedString(L";%ls", lpszPath);
    }

    // environment variable _NT_ALTERNATE_SYMBOL_PATH
    if (GetEnvironmentVariable(_T("_NT_ALTERNATE_SYMBOL_PATH"), lpszPath, SP_MAX_PATH))
    {
        lpszSymbolPath.appendFormattedString(L";%ls", lpszPath);
    }

    // environment variable SYSTEMROOT
    if (GetEnvironmentVariable(_T("SYSTEMROOT"), lpszPath, SP_MAX_PATH))
    {
        lpszSymbolPath.appendFormattedString(L";%ls;%ls\\System32", lpszPath,lpszPath);
    }

    if (pszSymPath != NULL)
    {
        lpszSymbolPath.appendFormattedString(L";%ls", pszSymPath);
    }
    */
    // Get the Debug Symbols engine options:
    DWORD symOptions = m_pSymGetOptions();

    // Add the following flags to the the Debug Symbols engine options:
    // - SYMOPT_DEFERRED_LOADS - Symbols are not loaded until a reference is made requiring
    //                            the symbols to be loaded.
    // - SYMOPT_LOAD_LINES - Loads line number information.
    // - SYMOPT_UNDNAME - All symbols are presented in undecorated form.
    m_pSymSetOptions(symOptions | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);

    m_bInit = m_pSymInitialize(m_hProcess, NULL, TRUE) == TRUE;

    return m_bInit;
#else
    m_bInit = true;
    return true;
#endif
}

StackTracer::~StackTracer(void)
{
    m_bInit = false;

#ifdef _WIN32
    SAFE_FREE(m_pSymbol);
    SAFE_DELETE(m_pMtx);
    UnloadDbgHelp();
#endif
}

#ifdef _WIN32

#if defined (_M_X64)
    #include <intrin.h>
#endif


#if defined(_M_IX86)
    #define SIZEOFPTR 4
    #define X86X64ARCHITECTURE IMAGE_FILE_MACHINE_I386
    #define AXREG eax
    #define BPREG ebp
    #define SPREG esp
#elif defined(_M_X64)
    #define SIZEOFPTR 8
    #define X86X64ARCHITECTURE IMAGE_FILE_MACHINE_AMD64
    #define AXREG rax
    #define BPREG rbp
    #define SPREG rsp
#endif // _M_IX86

#if defined(_M_IX86) || defined(_M_X64)
#pragma auto_inline(off)
DWORD_PTR GetProgramCounterx86x64()
{
    DWORD_PTR programcounter;

#ifdef _M_IX86
    __asm mov AXREG, [BPREG + SIZEOFPTR];    // Get the return address out of the current stack frame
    __asm mov [programcounter], AXREG;       // Put the return address into the variable we'll return
#elif defined(_M_X64)
    programcounter = (DWORD_PTR)_ReturnAddress() + SIZEOFPTR;
#else
#error Unknown address space size!
#endif

    return programcounter;
}
#pragma auto_inline(on)
#else
#error Unknown windows configuration
#endif // defined(_M_IX86) || defined(_M_X64)

#define STATIC_LINK

bool StackTracer::LoadDbgHelp()
{
#ifdef STATIC_LINK
    m_pStackWalk64 = StackWalk64;
    m_pSymInitialize = SymInitialize;
    m_pSymCleanup = SymCleanup;
    m_pSymGetOptions = SymGetOptions;
    m_pSymSetOptions = SymSetOptions;
    m_pSymGetSymFromAddr64 = SymGetSymFromAddr64;
    m_pSymGetLineFromAddr64 = SymGetLineFromAddr64;
    m_pSymFunctionTableAccess64 = SymFunctionTableAccess64;
    m_pSymGetModuleBase64 = SymGetModuleBase64;
    m_pSymGetModuleInfo64 = SymGetModuleInfo64;
    m_pSymLoadModule64 = SymLoadModule64;
    m_pSymUnloadModule64 = SymUnloadModule64;
#else

    if (m_hDbgHelp != NULL)
    {
        return false;
    }

    m_hDbgHelp = LoadLibrary(_T("DbgHelp.dll"));

    if (m_hDbgHelp == NULL)
    {
        Log(logERROR, "StackTracer: Error - could not load DbgHelp.dll\n");
        return false;
    }

    m_pStackWalk64 = (StackWalk64Proc) GetProcAddress(m_hDbgHelp, "StackWalk64");

    if (m_pStackWalk64 == NULL)
    {
        Log(logERROR, "StackTracer: Error - could not get StackWalk64 address\n");
        UnloadDbgHelp();
        return false;
    }

#ifdef _UNICODE
    m_pSymInitialize = (SymInitializeProc) GetProcAddress(m_hDbgHelp, "SymInitializeW");
#else // ANSI
    m_pSymInitialize = (SymInitializeProc) GetProcAddress(m_hDbgHelp, "SymInitialize");
#endif // ANSI

    if (m_pSymInitialize == NULL)
    {
        Log(logERROR, "StackTracer: Error - could not get SymInitialize address\n");
        UnloadDbgHelp();
        return false;
    }

    m_pSymGetOptions = (SymGetOptionsProc) GetProcAddress(m_hDbgHelp, "SymGetOptions");

    if (m_pSymGetOptions == NULL)
    {
        Log(logERROR, "StackTracer: Error - could not get SymGetOptions address\n");
        UnloadDbgHelp();
        return false;
    }

    m_pSymSetOptions = (SymSetOptionsProc) GetProcAddress(m_hDbgHelp, "SymSetOptions");

    if (m_pSymSetOptions == NULL)
    {
        Log(logERROR, "StackTracer: Error - could not get SymSetOptions address\n");
        UnloadDbgHelp();
        return false;
    }

    m_pSymCleanup = (SymCleanupProc) GetProcAddress(m_hDbgHelp, "SymCleanup");

    if (m_pSymCleanup == NULL)
    {
        Log(logERROR, "StackTracer: Error - could not get SymCleanup address\n");
        UnloadDbgHelp();
        return false;
    }

    m_pSymGetSymFromAddr64 = (SymGetSymFromAddr64Proc) GetProcAddress(m_hDbgHelp, "SymGetSymFromAddr64");

    if (m_pSymGetSymFromAddr64 == NULL)
    {
        Log(logERROR, "StackTracer: Error - could not get SymGetSymFromAddr64 address\n");
        UnloadDbgHelp();
        return false;
    }

    m_pSymGetLineFromAddr64 = (SymGetLineFromAddr64Proc) GetProcAddress(m_hDbgHelp, "SymGetLineFromAddr64");

    if (m_pSymGetLineFromAddr64 == NULL)
    {
        Log(logERROR, "StackTracer: Error - could not get SymGetLineFromAddr64 address\n");
        UnloadDbgHelp();
        return false;
    }

    m_pSymFunctionTableAccess64 = (PFUNCTION_TABLE_ACCESS_ROUTINE64) GetProcAddress(m_hDbgHelp, "SymFunctionTableAccess64");

    if (m_pSymFunctionTableAccess64 == NULL)
    {
        Log(logERROR, "StackTracer: Error - could not get SymFunctionTableAccess64 address\n");
        UnloadDbgHelp();
        return false;
    }

    m_pSymGetModuleBase64 = (PGET_MODULE_BASE_ROUTINE64) GetProcAddress(m_hDbgHelp, "SymGetModuleBase64");

    if (m_pSymGetModuleBase64 == NULL)
    {
        Log(logERROR, "StackTracer: Error - could not get SymFunctionTableAccess64 address\n");
        UnloadDbgHelp();
        return false;
    }

    m_pSymGetModuleInfo64 = (SymGetModuleInfo64Proc) GetProcAddress(m_hDbgHelp, "SymGetModuleInfo64");

    if (m_pSymGetModuleInfo64 == NULL)
    {
        Log(logERROR, "StackTracer: Error - could not get SymGetModuleInfo64 address\n");
        UnloadDbgHelp();
        return false;
    }

    m_pSymLoadModule64 = (SymLoadModule64Proc) GetProcAddress(m_hDbgHelp, "SymLoadModule64");

    if (m_pSymLoadModule64 == NULL)
    {
        Log(logERROR, "StackTracer: Error - could not get SymLoadModule64 address\n");
        UnloadDbgHelp();
        return false;
    }

    m_pSymUnloadModule64 = (SymUnloadModule64Proc) GetProcAddress(m_hDbgHelp, "SymUnloadModule64");

    if (m_pSymUnloadModule64 == NULL)
    {
        Log(logERROR, "StackTracer: Error - could not get SymUnloadModule64 address\n");
        UnloadDbgHelp();
        return false;
    }

#endif
    return true;
}

void StackTracer::UnloadDbgHelp()
{
    if (m_hDbgHelp != NULL)
    {
        FreeLibrary(m_hDbgHelp);
        m_hDbgHelp = NULL;
    }

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

    m_bInit = false;
}

bool StackTracer::GetSymbolName(Address dwAddress, StackEntry& en)
{
    if (dwAddress == NULL || !m_bInit)
    {
        return false;
    }

    AMDTScopeLock lock(m_pMtx);

    en.m_strSymAddr = StringUtils::ToHexString(dwAddress);
    en.m_dwAddress = dwAddress;

    IMAGEHLP_LINE64 line;
    ZeroMemory(&line, sizeof(line));
    line.SizeOfStruct = sizeof(line);

    ZeroMemory(m_pSymbol, m_pSymbolSize);
    m_pSymbol->SizeOfStruct  = m_pSymbolSize;
    m_pSymbol->MaxNameLength = m_pSymbolSize - sizeof(IMAGEHLP_SYMBOL);

    IMAGEHLP_MODULE64 moduleInfo;
    ZeroMemory(&moduleInfo, sizeof(IMAGEHLP_MODULE64));
    moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

    // get module name
    if (m_pSymGetModuleInfo64(m_hProcess, dwAddress, &moduleInfo))
    {
        en.m_strModName = moduleInfo.ModuleName;
    }
    else
    {
        en.m_strModName.clear();

        wchar_t szError[1024];
        DWORD dwError = GetLastError();

        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                      NULL,
                      dwError,
                      0,
                      szError,
                      1024,
                      NULL);


        Log(logERROR, "SymGetModuleInfo64 Failed - %s.\n", szError);
    }

    DWORD64 dw64Displacement = 0;
    DWORD dwDisplacement;

    // Get file name
    if (m_pSymGetLineFromAddr64(m_hProcess, dwAddress, &dwDisplacement, &line))
    {
        en.m_dwLineNum = line.LineNumber;
        en.m_strFile = line.FileName;
    }

    // Get line number
    if (m_pSymGetSymFromAddr64(m_hProcess, dwAddress, &dw64Displacement, m_pSymbol))
    {
        en.m_strSymName = m_pSymbol->Name;
    }
    else
    {
        en.m_dwDisplacement = dwDisplacement;
        en.m_strSymName.clear();
    }

    return true;
}

bool StackTracer::GetStackTrace(std::vector<StackEntry>& stackTrace, bool bGetSymbol, osThreadId tid)
{
    HANDLE hThread = tid == 0 ? GetCurrentThread() : OpenThread(THREAD_ALL_ACCESS, FALSE, tid);

    if (!hThread || !m_bInit)
    {
        Log(logERROR, "Stack tracer not initialized.\n");
        return false;
    }

    AMDTScopeLock lock(m_pMtx);

    bool bSuspend = false;

    // Stack walk code taken from gDEBugger (\\depot\devtools\main\gDebugger\Workspace\GROSWrappers\src\win32\osWin32CallStackReader.cpp)
    CONTEXT threadContext;
    CONTEXT* pThreadExecutionContext;
    ZeroMemory(&threadContext , sizeof(CONTEXT));

    // A Windows structure that will contain the current stack frames:
    STACKFRAME64 currentWinStackFrame;

    // Initialize the structure:
    ZeroMemory(&currentWinStackFrame , sizeof(STACKFRAME64));

    if (tid == 0)
    {
        tid = GetCurrentThreadId();
        // Get the program counter:
        currentWinStackFrame.AddrPC.Offset  = GetProgramCounterx86x64();
        currentWinStackFrame.AddrPC.Mode    = AddrModeFlat;
        //*
        // gDEBugger way of retrieving frame addr and stack address
        // Get the frame address:
        DWORD_PTR frameAddr = 0;
#ifdef _M_IX86
        __asm mov [frameAddr], BPREG // Get the frame pointer (aka base pointer)
#elif defined(_M_X64)
        CONTEXT currentContext;
        RtlCaptureContext(&currentContext);
        frameAddr = currentContext.Rbp;
        // frameAddr = _ReturnAddress();
#endif
        currentWinStackFrame.AddrFrame.Offset  = frameAddr;
        currentWinStackFrame.AddrFrame.Mode    = AddrModeFlat;

        // Get the stack address:
        DWORD_PTR stackAddr = 0;
#ifdef _M_IX86
        __asm mov [stackAddr], SPREG
#elif defined(_M_X64)
        stackAddr = currentContext.Rsp;
#endif
        currentWinStackFrame.AddrStack.Offset  = stackAddr;
        currentWinStackFrame.AddrStack.Mode    = AddrModeFlat;
        /*/
              CONTEXT currentContext;
              RtlCaptureContext (&currentContext);
        #ifdef _M_IX86
              currentWinStackFrame.AddrFrame.Offset = currentContext.Ebp;
              currentWinStackFrame.AddrFrame.Mode   = AddrModeFlat;
              currentWinStackFrame.AddrStack.Offset = currentContext.Esp;
              currentWinStackFrame.AddrStack.Mode   = AddrModeFlat;
        #elif _M_X64
              currentWinStackFrame.AddrFrame.Offset = currentContext.Rbp;
              currentWinStackFrame.AddrFrame.Mode   = AddrModeFlat;
              currentWinStackFrame.AddrStack.Offset = currentContext.Rsp;
              currentWinStackFrame.AddrStack.Mode   = AddrModeFlat;
        #endif

        //*/
        // Fill the context:
#ifdef _M_IX86
        threadContext.Eip = (DWORD)currentWinStackFrame.AddrPC.Offset;
        threadContext.Esp = (DWORD)currentWinStackFrame.AddrStack.Offset;
        threadContext.Ebp = (DWORD)currentWinStackFrame.AddrFrame.Offset;
#elif defined(_M_X64)
        threadContext.Rip = (DWORD64)currentWinStackFrame.AddrPC.Offset;
        threadContext.Rsp = (DWORD64)currentWinStackFrame.AddrStack.Offset;
        threadContext.Rbp = (DWORD64)currentWinStackFrame.AddrFrame.Offset;
#endif
        pThreadExecutionContext = &threadContext;
    }
    else
    {
        SuspendThread(hThread);
        bSuspend = true;
        threadContext.ContextFlags = CONTEXT_FULL;
        pThreadExecutionContext = &threadContext;

        if (GetThreadContext(hThread, pThreadExecutionContext) == FALSE)
        {
            ResumeThread(hThread);
            return false;
        }

#ifdef _M_IX86
        currentWinStackFrame.AddrPC.Offset     = pThreadExecutionContext->Eip;
        currentWinStackFrame.AddrPC.Mode       = AddrModeFlat;
        currentWinStackFrame.AddrStack.Offset  = pThreadExecutionContext->Esp;
        currentWinStackFrame.AddrStack.Mode    = AddrModeFlat;
        currentWinStackFrame.AddrFrame.Offset  = pThreadExecutionContext->Ebp;
        currentWinStackFrame.AddrFrame.Mode    = AddrModeFlat;
#elif defined(_M_X64)
        currentWinStackFrame.AddrPC.Offset     = pThreadExecutionContext->Rip;
        currentWinStackFrame.AddrPC.Mode       = AddrModeFlat;
        currentWinStackFrame.AddrStack.Offset  = pThreadExecutionContext->Rsp;
        currentWinStackFrame.AddrStack.Mode    = AddrModeFlat;
        currentWinStackFrame.AddrFrame.Offset  = pThreadExecutionContext->Rbp;
        currentWinStackFrame.AddrFrame.Mode    = AddrModeFlat;
#else
#error Unknown address space size!
#endif
    }

    DWORD machineType = X86X64ARCHITECTURE;
    STACKFRAME64 preFrame;
    ZeroMemory(&preFrame , sizeof(STACKFRAME64));

    while (m_pStackWalk64(machineType,
                          m_hProcess,
                          hThread,
                          &currentWinStackFrame,
                          pThreadExecutionContext,
                          NULL,
                          m_pSymFunctionTableAccess64,
                          m_pSymGetModuleBase64,
                          NULL) == TRUE)
    {
        if (currentWinStackFrame.AddrFrame.Offset != 0 && currentWinStackFrame.AddrReturn.Offset != 0)
        {
            StackEntry en;
            currentWinStackFrame.AddrPC.Offset = currentWinStackFrame.AddrPC.Offset - 1;

            if (bGetSymbol)
            {
                GetSymbolName(currentWinStackFrame.AddrPC.Offset, en);
            }
            else
            {
                en.m_dwAddress = currentWinStackFrame.AddrPC.Offset;
            }

            stackTrace.push_back(en);
            memcpy(&preFrame, &currentWinStackFrame, sizeof(STACKFRAME64));
        }
    }

    if (bSuspend)
    {
        ResumeThread(hThread);
    }

    return !stackTrace.empty();
}

#else

#include <signal.h>
#include <sys/wait.h>

bool StackTracer::GetStackTrace(std::vector<StackEntry>& stackTrace, bool bGetSymbol, osThreadId tid)
{
    int nEntries = 0;
    void* pBuffer[MAX_TRACE_SIZE];
    char** ppStrs;

    nEntries = backtrace(pBuffer, MAX_TRACE_SIZE);

    if (nEntries == 0)
    {
        return false;
    }

    stackTrace.reserve(nEntries);

    ppStrs = backtrace_symbols(pBuffer, nEntries);

    if (ppStrs == NULL)
    {
        return false;
    }
    else
    {
        for (int i = 0; i < nEntries; ++i)
        {
            if (ppStrs[i])
            {
                StackEntry en;
                en.m_dwAddress = pBuffer[i];
                bool bModSet = false;
                string str = string(ppStrs[i]);
                size_t pos0 = str.find_first_of("(");

                if (pos0 != string::npos)
                {
                    // symbol name found
                    size_t pos1 = str.find_last_of(")");

                    // extract symbol + displacement
                    if (pos1 != string::npos && pos1 > pos0 + 1)
                    {
                        string sym = str.substr(pos0 + 1, pos1 - pos0 - 1);
                        size_t pos2 = sym.find_first_of("+");

                        if (pos2 != string::npos && pos2 + 1 < sym.length() - 1)
                        {
                            sym = sym.substr(0, pos2);
                            en.m_strSymName = sym;

                            en.m_strModName = str.substr(0, pos0);
                            bModSet = true;
                        }
                        else
                        {
                            //shouldn't happen, ignore
                            continue;
                        }
                    }
                }

                // no symbol found
                // search for address
                pos0 = str.find_first_of("[");

                if (pos0 != string::npos)
                {
                    // symbol name found
                    size_t pos1 = str.find_last_of("]");

                    // extract address
                    if (pos1 != string::npos && pos1 > pos0 + 1)
                    {
                        if (!bModSet)
                        {
                            // no symbol available, extract module name here
                            en.m_strModName = str.substr(0, pos0 - 3);
                            bModSet = true;
                        }
                    }
                }
                else
                {
                    //shouldn't happen
                    continue;
                }

                stackTrace.push_back(en);
            }
        }

        return true;
    }
}

bool StackTracer::CheckDbgInfo(const std::string& strMod)
{
    bool bRet = false;
    // find debug info info from map
    DbgInfoMap::iterator it = m_dbgInfoMods.find(strMod);

    if (it == m_dbgInfoMods.end())
    {
        // check for dbg info availability
        char filecmd[SP_MAX_PATH];
        sprintf(filecmd, "file %s", strMod.c_str());

        FILE* pFile = popen(filecmd, "r");

        if (pFile != NULL)
        {
            char fileOut[SP_MAX_PATH];

            if (fgets(fileOut, SP_MAX_PATH, pFile) != NULL)
            {
                string strOut = fileOut;

                if (strOut.find("not stripped") == string::npos)
                {
                    // debug info missing
                    m_dbgInfoMods.insert(DbgInfoMapPair(strMod, false));
                }
                else
                {
                    m_dbgInfoMods.insert(DbgInfoMapPair(strMod, true));
                    bRet = true;
                }
            }

            pclose(pFile);
        }
        else
        {
            // check failed, assume it has debug info
            m_dbgInfoMods.insert(DbgInfoMapPair(strMod, true));
            bRet = true;
        }
    }
    else
    {
        bRet = it->second;
    }

    return bRet;
}

//
pid_t popen2(const char* pCommand, int* pInfp, int* pOutfp)
{
    const int READ = 0;
    const int WRITE = 1;
    int p_stdin[2], p_stdout[2];
    pid_t pid;

    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
    {
        return -1;
    }

    pid = fork();

    if (pid < 0)
    {
        return pid;
    }
    else if (pid == 0)
    {
        close(p_stdin[WRITE]);
        dup2(p_stdin[READ], READ);
        close(p_stdout[READ]);
        dup2(p_stdout[WRITE], WRITE);

        execl("/bin/sh", "sh", "-c", pCommand, NULL);
        perror("execl");
        exit(1);
    }

    // Parent process
    // Close unused pipe ends
    close(p_stdin[READ]);
    close(p_stdout[WRITE]);

    if (pInfp == NULL)
    {
        close(p_stdin[WRITE]);
    }
    else
    {
        *pInfp = p_stdin[WRITE];
    }

    if (pOutfp == NULL)
    {
        close(p_stdout[READ]);
    }
    else
    {
        *pOutfp = p_stdout[READ];
    }

    return pid;
}

bool StackTracer::GetSymbolName(Address dwAddress, StackEntry& en)
{
    if (dwAddress == NULL || en.m_strModName.empty())
    {
        return false;
    }

    bool bDbgInfo = CheckDbgInfo(en.m_strModName);

    if (!bDbgInfo)
    {
        return false;
    }

    char syscom[SP_MAX_PATH];
    sprintf(syscom, "addr2line -e %s %p", en.m_strModName.c_str(), en.m_dwAddress); //last parameter is the name of this app
    int pstdout = 0;
    pid_t pid = popen2(syscom, NULL, &pstdout);

    if (pid > 0)
    {
        char addr2lineOut[SP_MAX_PATH];
        memset(addr2lineOut, 0, SP_MAX_PATH);

        if (read(pstdout, addr2lineOut, SP_MAX_PATH) != -1)
        {
            string strOut = addr2lineOut;

            if (strOut.find("??") == string::npos)
            {
                size_t pos = strOut.find_last_of(":");
                en.m_strFile = strOut.substr(0, pos);
                string line = strOut.substr(pos + 1);
                StringUtils::Parse(line, en.m_dwLineNum);
            }

            close(pstdout);
        }
        else
        {
            //failed to read output, kill process
            kill(pid, SIGKILL);
            return false;
        }

        waitpid(pid, NULL, 0);
        return true;
    }

    return false;
}

#endif
