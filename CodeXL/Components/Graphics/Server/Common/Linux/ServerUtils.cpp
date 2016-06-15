//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of shared functionality used by the server plugins
//==============================================================================

#include <cstring>

#include <AMDTOSWrappers/Include/osProcess.h>
#include "../SharedGlobal.h"
#include "ServerUtils.h"

//-----------------------------------------------------------------------------
/// CanBind
/// Is this shared library allowed to bind with the executable it's currently
/// preloaded into
/// \return true if it is allowed to bind to this exe, false if not
//-----------------------------------------------------------------------------
bool ServerUtils::CanBind(const char* programName)
{
    // list of known commands that this shared library should not attach to
    const char* ignoreList[] =
    {
        "sh",
        "bash",
        "xmessage",
        "steam",
    };

    int count = sizeof(ignoreList) / sizeof(ignoreList[0]);

    for (int i = 0; i < count; i++)
    {
        if (0 == strcmp(ignoreList[i], programName))
        {
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Put up a dialog box to give the user time to attach a debugger to the
/// application rather than the perf studio app.
/// \param serverName the name of the server plugin loaded into the target
/// application
/// \param initialized has the server plugin been initialized
/// \return 0 if successful, non-zero on error
//-----------------------------------------------------------------------------
int ServerUtils::CheckForDebuggerAttach(const char* serverName, bool initialized)
{
    char commandString[1024];
    int retVal = 0;

    static bool alreadyChecked = false;  // only pause application once.
    static const char fmtString[] = "The application has been paused to allow GDB to be attached to the process.\nApplication name: %s\nOpen a terminal and cd to where the %s.so is running from\n(so that gdb can load debug symbols). Use:\n\nsudo gdb attach %d\n\nPress OK to continue";
    char message[sizeof(fmtString) + 10 + PS_MAX_PATH];    // Add extra bytes for process ID

    if (SG_GET_BOOL(OptionBreak) && !alreadyChecked && initialized)         // did we run PerfServer with --break option
    {
        alreadyChecked = true;
        sprintf_s(message, sizeof(message), fmtString, program_invocation_short_name, serverName, osGetCurrentProcessId());
        sprintf(commandString, "xmessage \"%s\" -center -buttons OK", message);
        retVal = system(commandString);
    }

    return retVal;
}

