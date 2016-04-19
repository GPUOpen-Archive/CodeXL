//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspUtils.h
///
//==================================================================================

//------------------------------ vspUtils.h ------------------------------

#ifndef __VSPUTILS_H
#define __VSPUTILS_H

// Get current debugging command string name:
void vspGetStartActionCommandName(std::wstring& verbName, std::wstring& actionCommandStr, bool addKeyboardShortcut = false);

// Update settings from Visual Studio project:
void vspUpdateProjectSettingsFromStartupProject();

// Get execution command name according to current execution mode:
bool vspGetExecutionCommandName(DWORD commandId, std::wstring& commandName);

// Dll version:
bool vspIsAMDSpyDll(const std::wstring& dllPath);

// Allocate a wchar_t* from an std::wstring.
wchar_t* vspAllocateAndCopy(const std::wstring& strToCopy);

// Delete a wchar_t string.
void vspDeleteWcharString(wchar_t*& pStrToDelete);

#endif //__VSPUTILS_H

