//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDebuggingFunctions.cpp
///
//=====================================================================

//------------------------------ osDebuggingFunctions.cpp ------------------------------

// POSIX:
#include <sys/types.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <unistd.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>


// ---------------------------------------------------------------------------
// Name:        osOutputDebugString
// Description:
//   Writes the debug string into the debug log file.
//
// Arguments:   debugString - The string to be sent to the debugger.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/10/2004
// Implementation notes:
// As far as we know, there is no Linux equivalent to OutputDebugString.
// therefore, we only output to the debug log.
// ---------------------------------------------------------------------------
void osOutputDebugString(const gtString& debugString)
{
    // Output log message
    gtString logMessage = OS_STR_DebugStringOutputPrefix;
    logMessage += debugString;

    // If I am running under a debugger:
    bool isUnderDebugger = osIsRunningUnderDebugger();

    if (isUnderDebugger)
    {
        // Write the debug string to the console (we get the debugged app's output through the tty pipe):
        printf("%s", logMessage.asASCIICharArray());
        printf("\n");
        ::fflush(stdout);
    }

    osDebugLog& theDebugLog = osDebugLog::instance();
    theDebugLog.addPrintout(__FUNCTION__, __FILE__ , __LINE__ , logMessage.asCharArray() , OS_DEBUG_LOG_INFO);
}


// ---------------------------------------------------------------------------
// Name:        osWPerror
// Description:
//   Writes an error message to the console.
//
// Arguments:   pErrorMessage - The message to be printed
// Author:      AMD Developer Tools Team
// Date:        19/6/2011
// ---------------------------------------------------------------------------
void osWPerror(const wchar_t* pErrorMessage)
{
    if (pErrorMessage != NULL)
    {
        // Translate the error message into an ASCII string:
        gtString errorMessageAsString(pErrorMessage);
        gtASCIIString errorMessageAsASCIIString = errorMessageAsString.asASCIICharArray();
        perror(errorMessageAsASCIIString.asCharArray());
    }
}


// ---------------------------------------------------------------------------
// Name:        osThrowBreakpointException
// Description: Throws a breakpoint exception.
// Author:      AMD Developer Tools Team
// Date:        5/10/2004
// ---------------------------------------------------------------------------
void osThrowBreakpointException()
{
    pid_t thisProcessId = ::getpid();
    int rc = ::kill(thisProcessId, SIGTRAP);

    GT_ASSERT(rc == 0);
}


// ---------------------------------------------------------------------------
// Name:        osIsRunningUnderDebugger
// Description: Returns true iff the calling process runs under a debugger.
// Author:      AMD Developer Tools Team
// Date:        5/10/2004
// Implementation notes:
//   Almost all debuggers use ptrace to trace the debugged process signals.
//   This function checks if this process is currently traced by ptrace.
//   See ptrace man page for more information.
// ---------------------------------------------------------------------------
bool osIsRunningUnderDebugger()
{
    bool retVal = true;

    long int err = ::ptrace(PT_TRACE_ME, 0, NULL, NULL);

    if (err == 0)
    {
        // I am not traced by ptrace:
        retVal = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osOpenFileInSourceCodeEditor
// Description: Opens the input source code location in an editor.
// Arguments: filePath - The path of the file to be opened.
//            lineNumber - The line, in the above file that the editor should display.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/2/2009
// ---------------------------------------------------------------------------
bool osOpenFileInSourceCodeEditor(const osFilePath& filePath, int lineNumber)
{
    (void)(filePath); // unused
    (void)(lineNumber); // unused
    bool retVal = false;

    // TO_DO: LNX: Not implemented yet under Linux (we need to try to launch Scite, etc)
    GT_ASSERT_EX(false, L"osOpenFileInSourceCodeEditor is not yet implemented under Linux");

    return retVal;
}

