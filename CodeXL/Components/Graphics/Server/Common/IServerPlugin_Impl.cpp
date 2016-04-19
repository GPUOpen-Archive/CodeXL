//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The function defined here allows an injected plugin to report to the main
// server that it is actively being used by the application. See the function
// header for the definition of being "actively used".
//==============================================================================

#include <AMDTOSWrappers/Include/osProcess.h>
#include "IServerPlugin.h"
#include "SharedMemoryManager.h"
#include <string>
#include "Logger.h"
#include "misc.h"

//-----------------------------------------------------------------------------
/// RegisterActivePlugin
///
/// Allows a plugin to register with the server to indicate that it is actively
/// used by the application. A prerequisite to being "actively used" is that
/// the plugin must be in a state that it can receive and respond to commands.
///
/// \param strShortDescription Must be the same value
///        as is returned by GetShortDescription( )
/// \return true if the plugin could be registered; false otherwise
//-----------------------------------------------------------------------------
bool RegisterActivePlugin(const char* strShortDescription)
{
    // combine processID and plugin name so that we can support talking to the same plugin in two different processes
    char name[ PS_MAX_PATH ];
    DWORD pid = osGetCurrentProcessId();
    strcpy_s(name, PS_MAX_PATH, FormatText("%lu/%s", pid, strShortDescription).asCharArray());

    // make sure we only register once
    static bool bAlreadyRegistered = false;

    if (bAlreadyRegistered == true)
    {
        return true;
    }

    // haven't registered yet, so try to
    if (smOpen("ActivePlugins") == false)
    {
        Log(logERROR, "Failed to open ActivePlugins shared memory for %s in process %lu. Commands will not be sent to the plugin\n", strShortDescription, pid);
        return false;
    }

    if (smLockPut("ActivePlugins", PS_MAX_PATH, 1) == false)
    {
        Log(logERROR, "Not enough space in shared memory to register plugin %s in process %lu. Commands will not be sent to the plugin\n", strShortDescription, pid);
        return false;
    }

    bool bResult = smPut("ActivePlugins", (void*) name, PS_MAX_PATH);

    if (bResult == false)
    {
        Log(logERROR, "Failed to register active plugin named '%s' in process %lu. Commands will not be sent to the plugin\n", strShortDescription, pid);
    }
    else
    {
        bAlreadyRegistered = true;
#pragma TODO("Should wait to make sure that the expected shared memory has been created")
    }

    smUnlockPut("ActivePlugins");

    return bResult;
}