//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspTimer.cpp
///
//==================================================================================

//------------------------------ vspTimer.cpp ------------------------------

#include "stdafx.h"

// C++:
#include <cassert>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <src/vspCoreAPI.h>
#include <src/vspDTEConnector.h>
#include <src/vspTimer.h>
#include <src/vspUtils.h>

// Static member initialization:

// ---------------------------------------------------------------------------
// Name:        vspTimer::vspTimer
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        24/5/2012
// ---------------------------------------------------------------------------
vspTimer::vspTimer() : m_pTimer(nullptr)
{
    m_pTimer = VSCORE(vscTimer_CreateInstance)();
    // Register as the vscTimer owner.
    VSCORE(vscTimer_SetOwner)(m_pTimer, this);
}

// ---------------------------------------------------------------------------
// Name:        vspTimer::vspTimer
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        24/5/2012
// ---------------------------------------------------------------------------
vspTimer::~vspTimer()
{
    VSCORE(vscTimer_DestroyInstance)(m_pTimer);
}

bool vspTimer::vscTimerOwner_GetStartupProjectDebugInfo(wchar_t*& pExecutableFilePathBuffer,
                                                        wchar_t*& pWorkingDirectoryPathBuffer,
                                                        wchar_t*& pCommandLineArgumentsBuffer,
                                                        wchar_t*& pEnvironmentBuffer,
                                                        bool& isProjectOpenBuffer,
                                                        bool& isProjectTypeValidBuffer,
                                                        bool& isNonNativeProjectBuffer)
{
    bool retVal = false;

    // Init.
    pExecutableFilePathBuffer       = NULL;
    pWorkingDirectoryPathBuffer     = NULL;
    pCommandLineArgumentsBuffer     = NULL;
    pEnvironmentBuffer              = NULL;

    // Get the DTEConnector instance.
    vspDTEConnector& theDTEConnector = vspDTEConnector::instance();

    if (theDTEConnector.hasStartupProjectExecutableChanged(m_executableFilePath))
    {
        std::wstring newExecPath;
        std::wstring newWorkDir;
        std::wstring newCmdArgs;
        std::wstring newExecEnv;

        // Check the current project settings from VS:
        theDTEConnector.getStartupProjectDebugInformation(newExecPath, newWorkDir, newCmdArgs, newExecEnv, isProjectOpenBuffer, isProjectTypeValidBuffer, isNonNativeProjectBuffer);

        // Allocate the output strings.
        pExecutableFilePathBuffer    = vspAllocateAndCopy(newExecPath);
        pWorkingDirectoryPathBuffer  = vspAllocateAndCopy(newWorkDir);
        pCommandLineArgumentsBuffer  = vspAllocateAndCopy(newCmdArgs);
        pEnvironmentBuffer           = vspAllocateAndCopy(newExecEnv);

        retVal = true;
    }

    return retVal;
}

void vspTimer::vscTimerOwner_UpdateCommandShellUi()  const
{
    throw std::exception("The method or operation is not implemented.");
}

void vspTimer::vscTimerOwner_GetParentWindowHandle(HWND& winHandle)  const
{
    GT_UNREFERENCED_PARAMETER(winHandle);

    throw std::exception("The method or operation is not implemented.");
}

void vspTimer::vscTimerOwner_DeleteWCharStr(wchar_t*& pStr)  const
{
    vspDeleteWcharString(pStr);
}

void vspTimer::vscTimerOwner_UpdateProjectSettingsFromStartupInfo() const
{
    // Update the project settings from the vcproj file:
    vspUpdateProjectSettingsFromStartupProject();
}

void vspTimer::vscTimerOwner_GetActiveDocumentFileFullPath(wchar_t*& pStrBuffer, bool& hasActiveDocBuffer) const
{
    std::wstring filePathStr;

    // Get the DTEConnector instance.
    vspDTEConnector& theDTEConnector = vspDTEConnector::instance();
    hasActiveDocBuffer = theDTEConnector.getActiveDocumentFileFullPath(filePathStr);
    pStrBuffer = vspAllocateAndCopy(filePathStr);
}

bool vspTimer::vscTimerOwner_GetActiveWindowHandle(HWND& winHandle) const
{
    // Get the DTEConnector instance.
    vspDTEConnector& theDTEConnector = vspDTEConnector::instance();
    return theDTEConnector.GetActiveWindowHandle(winHandle);
}

void vspTimer::onClockTick()
{
    VSCORE(vscTimer_OnClockTick)(m_pTimer);
}
