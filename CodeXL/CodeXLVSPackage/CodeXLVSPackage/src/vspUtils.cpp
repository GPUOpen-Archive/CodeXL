//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspUtils.cpp
///
//==================================================================================

//------------------------------ vspUtils.cpp ------------------------------

#include "stdafx.h"

// C++:
#include <cassert>
#include <string>
#include <algorithm>

// Local:
#include "..\CodeXLVSPackageUI\CommandIds.h"
#include <src/vspCoreAPI.h>
#include <src/vspDTEConnector.h>
#include <CodeXLVSPackage/Include/vspStringConstants.h>
#include <src/vspUtils.h>


// ---------------------------------------------------------------------------
// Name:        vspGetStartDebuggingCommandName
// Description: Create the current debugging command string
// Arguments:   gtString& debuggingCommandStr
// Author:      Sigal Algranaty
// Date:        22/5/2011
// ---------------------------------------------------------------------------
void vspGetStartActionCommandName(std::wstring& verbName, std::wstring& actionCommandStr, bool addKeyboardShortcut /*= false*/, bool fullString /* = true */)
{
    wchar_t* pVerbNameStr = NULL;
    wchar_t* pActionCommandStr = NULL;

    // Invoke core logic.
    VSCORE(vscUtilsGetStartActionCommandName)(pVerbNameStr, pActionCommandStr, addKeyboardShortcut, fullString);

    // Assign the output buffers.
    assert(pVerbNameStr != NULL);
    assert(pActionCommandStr != NULL);
    verbName = pVerbNameStr;
    actionCommandStr = pActionCommandStr;

    // Release the strings which were allocated by the core.
    VSCORE(vscDeleteWcharString)(pVerbNameStr);
    VSCORE(vscDeleteWcharString)(pActionCommandStr);
}

// ---------------------------------------------------------------------------
// Name:        vspUpdateProjectSettingsFromStartupProject
// Description: Updates the project settings from the VS startup project.
//              Must be called before any call to afProjectManager::currentProjectSettings()
// Author:      Uri Shomroni
// Date:        2/2/2011
// ---------------------------------------------------------------------------
void vspUpdateProjectSettingsFromStartupProject()
{
    std::wstring execPath;
    std::wstring workDir;
    std::wstring cmdArgs;
    std::wstring execEnv;
    bool isProjectTypeSupported = false;
    bool isProjectOpened = false;
    bool isNonNativeProject = false;

    // Get the project settings from the VS project:
    vspDTEConnector::instance().getStartupProjectDebugInformation(execPath, workDir, cmdArgs, execEnv, isProjectOpened, isProjectTypeSupported, isNonNativeProject);

    // Invoke the core logic.
    VSCORE(vscUtilsUpdateProjectSettingsFromStartupProject)(execPath.c_str(), workDir.c_str(), cmdArgs.c_str(), execEnv.c_str(), isProjectOpened, isProjectTypeSupported, isNonNativeProject);
}

// ---------------------------------------------------------------------------
// Name:        vspIsAMDSpyDll
// Description: Checks is the specific dll is AMD dll by checking company name
//              in version info.
// Return Val:  false/true
// Author:      Gilad Yarnitzky
// Date:        23/3/2011
// ---------------------------------------------------------------------------
bool vspIsAMDSpyDll(const std::wstring& dllPath)
{
    return VSCORE(vscIsCodeXLServerDll)(dllPath.c_str());
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        vspGetExecutionCommandName
/// \brief Description: Get execution command name according to current execution mode
/// \param[in]          commandId - the requested command id
/// \param[in]          commandName - the command name
/// \return True :
/// \return False:
/// -----------------------------------------------------------------------------------------------
bool vspGetExecutionCommandName(DWORD commandId, std::wstring& commandName)
{
    bool ret = false;
    commandName.clear();
    wchar_t* pCommandNameStr = NULL;
    VSCORE(vscGetExecutionCommandName)(commandId, pCommandNameStr);
    assert(pCommandNameStr != NULL);

    if (pCommandNameStr != NULL)
    {
        commandName = pCommandNameStr;
        ret = true;
    }

    return ret;
}

wchar_t* vspAllocateAndCopy(const std::wstring& strToCopy)
{
    wchar_t* ret = NULL;

    if (!strToCopy.empty())
    {
        size_t sz = strToCopy.size() + 1;
        ret = new wchar_t[sz];
        std::copy(strToCopy.begin(), strToCopy.end(), ret);
        ret[sz - 1] = '\0';
    }

    return ret;
}

void vspDeleteWcharString(wchar_t*& pStrToDelete)
{
    delete[] pStrToDelete;
    pStrToDelete = NULL;
}
