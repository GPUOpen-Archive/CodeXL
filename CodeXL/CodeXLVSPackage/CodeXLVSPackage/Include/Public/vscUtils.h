//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscUtils.h
///
//==================================================================================

#ifndef vscUtils_h__
#define vscUtils_h__
#include "CodeXLVSPackageCoreDefs.h"
#include <WinDef.h>

// **********************************************************************************************************
// This is a proxy for vspUtils.
// Do not confuse with vscCoreUtils which is an actual utility class to be used by both core and VS code.
// Do not confuse with vscCoreInternalUtils which is an actual utility class to be used by the core code only.
// ***********************************************************************************************************
void vscUtilsGetStartActionCommandName(wchar_t*& verbNameBuffer, wchar_t*& actionCommandStrBuffer, bool addKeyboardShortcut = false, bool fullString = true);

void vscUtilsUpdateProjectSettingsFromStartupProject(const wchar_t* execPath, const wchar_t* workDir, const wchar_t* cmdArgs, const wchar_t* execEnv, bool isProjectOpened, bool isProjectTypeSupported, bool isNonNativeProject);

bool vscGetExecutionCommandName(DWORD commandId, wchar_t*& commandNameBuffer);

#endif // vscUtils_h__
