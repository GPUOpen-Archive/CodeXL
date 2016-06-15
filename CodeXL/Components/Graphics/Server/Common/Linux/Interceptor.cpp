//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Linux specific implementation of DLL Injection (just process creation
/// since the 'injection' is handled by LD_PRELOAD)
//==============================================================================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>

#include "WinDefs.h"
#include "Interceptor.h"
#include "misc.h"

#include <boost/tokenizer.hpp>
using boost::tokenizer;
using boost::escaped_list_separator;

typedef tokenizer<escaped_list_separator<char> > so_tokenizer;

extern char** environ;

BOOL CreateProcess(const char* lpApplicationName,
                   const char* lpCommandLine,
                   const char* lpCurrentDirectory,
                   PROCESS_INFORMATION* lpProcessInformation
                  )
{
    PS_UNREFERENCED_PARAMETER(lpCurrentDirectory);
    // split the command line by spaces, taking into account quoted strings
    vector<string> argList;
    string s = lpCommandLine;
    so_tokenizer tok(s, escaped_list_separator<char>('\\', ' ', '\"'));

    for (so_tokenizer::iterator tokIt = tok.begin(); tokIt != tok.end(); ++tokIt)
    {
        const char* str = (*tokIt).c_str();

        if (strlen(str) > 0)
        {
            argList.push_back(str);
        }
    }

    unsigned int numArgs = argList.size();

    // build the command line to pass to the application
    char** argv = new char* [numArgs + 1];        // add 1 for NULL terminator

    for (unsigned int loop = 0; loop < numArgs; loop++)
    {
        argv[loop] = (char*)argList[loop].c_str();
    }

    argv[numArgs] = NULL;

    pid_t pid;
    int status;
    fflush(NULL);
    status = posix_spawn(&pid, lpApplicationName, NULL, NULL, argv, environ);

    delete [] argv;

    if (status != 0)
    {
        Log(logERROR, "Can't create process for %s: %s\n", lpApplicationName, strerror(status));
        return FALSE;
    }

    lpProcessInformation->dwProcessId = pid;

    return TRUE;
}

// Stub function definitions for Linux builds. These functions are windows
// specific at the moment
INT32 LOGGER_D3DPerfMarkers_Hook()
{
    //   LogConsole(logMESSAGE, "calling LOGGER_D3DPerfMarkers_Hook\n");
    return 0;
}

INT32 LOGGER_D3DPerfMarkers_Unhook()
{
    //   LogConsole(logMESSAGE, "calling LOGGER_D3DPerfMarkers_Unhook\n");
    return 0;
}

class TraceAnalyzer;
long Hook_OutputDebugString(TraceAnalyzer* pTraceAnalyzer)
{
    PS_UNREFERENCED_PARAMETER(pTraceAnalyzer);
    //   LogConsole(logMESSAGE, "calling Hook_OutputDebugString\n");
    return 0;
}

long Unhook_OutputDebugString()
{
    //   LogConsole(logMESSAGE, "calling Unhook_OutputDebugString\n");
    return 0;
}

