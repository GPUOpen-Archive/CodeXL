//------------------------------ osDebuggingFunctions.cpp ------------------------------

// POSIX:
#include <sys/types.h>
#include <sys/sysctl.h>
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
//   a. If the calling process is running under a debugger, sends the input debug string
//      to the debugger. Debuggers will usually display this string.
//   b. Writes the debug string into the debug log file.
//
// Arguments:   debugString - The string to be sent to the debugger.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/10/2004
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
        // Write the debug string to the console (gdb, etc displays stdout strings under Mac):
        printf("%s", logMessage.asCharArray());
        printf("\n");
        ::fflush(stdout);
    }

    // Write the message also to the debug log:
    osDebugLog& theDebugLog = osDebugLog::instance();
    theDebugLog.addPrintout(__FUNCTION__, __FILE__ , __LINE__ , logMessage.asCharArray() , OS_DEBUG_LOG_INFO);
}


// ---------------------------------------------------------------------------
// Name:        osThrowBreakpointException
// Description: Throws a breakpoint exception.
// Author:      Yaki Tebeka
// Date:        5/10/2004
// ---------------------------------------------------------------------------
void osThrowBreakpointException()
{
    pthread_t thisThreadId = ::pthread_self();
    int rc = ::pthread_kill(thisThreadId, SIGTRAP);

    GT_ASSERT(rc == 0);
}


// ---------------------------------------------------------------------------
// Name:        osIsRunningUnderDebugger
// Description: Returns true iff the calling process runs under a debugger.
// Author:      Yaki Tebeka
// Date:        11/3/2009
// ---------------------------------------------------------------------------
bool osIsRunningUnderDebugger()
{
    // Create a struct that will get the queried information:
    struct kinfo_proc info;
    info.kp_proc.p_flag = 0;
    size_t size = sizeof(info);

    // Create an array that contains the current process id:
    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    // Query information about the current process:
    sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);

    // Check if the current process is being traced by a debugger:
    bool retVal = ((info.kp_proc.p_flag & P_TRACED) != 0);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osOpenFileInSourceCodeEditor
// Description: Opens the input source code location in an editor.
// Arguments: filePath - The path of the file to be opened.
//            lineNumber - The line, in the above file that the editor should display.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        4/2/2009
// ---------------------------------------------------------------------------
bool osOpenFileInSourceCodeEditor(const osFilePath& filePath, int lineNumber)
{
    bool retVal = false;

    // TO_DO: LNX: Not implemented yet under Linux (we need to try to launch Scite, etc)
    GT_ASSERT_EX(false, L"osOpenFileInSourceCodeEditor is not yet implemented under Linux");

    return retVal;
}

