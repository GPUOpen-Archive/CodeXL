//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscCoreUtils.h
///
//==================================================================================

#ifndef vscCoreUtils_h__
#define vscCoreUtils_h__

// For some reason the compiler could not find this symbol.
#ifndef NULL
    #define NULL (0)
#endif

// Local (core):
#include <Include/Public/CoreInterfaces/IVscApplicationCommandsOwner.h>
#include "CodeXLVSPackageCoreDefs.h"
#include <ctime>

// -----------------------------------------------------------------------------------
// Name:        vscDeleteWcharString
// Description: Deletes memory (which was allocated using the core's (vsc's) runtime).
// Author:      Amit Ben-Moshe
// Date:        5/11/2014
// -----------------------------------------------------------------------------------
void vscDeleteWcharString(wchar_t*& pStr);

void vscDeleteWcharStringArray(wchar_t**& pStr);

// -----------------------------------------------------------------------------------
// Name:        vscDeleteWcharString
// Description: Deletes memory (which was allocated using the core's (vsc's) runtime).
// Author:      Amit Ben-Moshe
// Date:        5/11/2014
// -----------------------------------------------------------------------------------
void vscDeleteCharString(char*& pStr);

// -----------------------------------------------------------------------------------
// Name:        vscDeleteWcharString
// Description: Deletes memory (which was allocated using the core's (vsc's) runtime).
// Author:      Amit Ben-Moshe
// Date:        5/11/2014
// -----------------------------------------------------------------------------------
void vscDeleteUintBuffer(unsigned int*& pBuffer);

// -----------------------------------------------------------------------------------
// Name:        vscDeleteWcharString
// Description: Returns true if and only if the strings are identical in their contents
// Arguments:   const wchar_t* pathStrA, const wchar_t* pathStrB - the strings to check
// Return Val:  bool - true means that the strings are equal, false means they are not
// Author:      Amit Ben-Moshe
// Date:        5/11/2014
// -----------------------------------------------------------------------------------
bool vscIsPathStringsEqual(const wchar_t* pathStrA, const wchar_t* pathStrB);

// -----------------------------------------------------------------------------------
// Name:        vscExtractFileExtension
// Description: Returns true if and only if the strings are identical in their contents
// Arguments:   filePathStr - the string containing the file's path in the file system
//              pExtensionStrBuffer - a pointer which will be set by the function to
//                                    point at the allocated string which will contain
//                                    the file's extension.
// Note:        Important: the output string should be allocated using the vscDeleteWcharString
//                         to avoid mixing runtimes.
// Author:      Amit Ben-Moshe
// Date:        5/11/2014
// ----------------------------------------------------------------
void vscExtractFileExtension(const wchar_t* filePathStr, wchar_t*& pExtensionStrBuffer);

// -----------------------------------------------------------------------------------
// Name:        vscIsPathExists
// Description: Returns true if and only pathStr represents a full path for a file that
//              actually exists in the filesystem.
// Arguments:   pathStr - full path string to be checked
// Author:      Amit Ben-Moshe
// Date:        5/11/2014
// -----------------------------------------------------------------------------------
bool vscIsPathExists(const wchar_t* pathStr);

// ---------------------------------------------------------------------------
// Name:        vscGetFileDirectoryAsString
// Description:
//   Returns the file containing directory path.
//   Example: "c:\temp\foo.txt" - the containing directory path is "c:\temp"
// Arguments:   fileDirectory - Will get the file containing directory path.
// Return Val:  bool - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        5/11/2014
// ---------------------------------------------------------------------------
void vscGetFileDirectoryAsString(const wchar_t* pathStr, wchar_t*& pDirStrBuffer);

void vscGetFileName(const wchar_t* pathStr, wchar_t*& pFileNameStrBuffer);

wchar_t vscGetOsPathSeparator();

wchar_t vscGetOsExtensionSeparator();

bool vscStartsWith(const wchar_t* str, const wchar_t* substring);

void vscPrintDebugMsgToDebugLog(const wchar_t* msg);

void vscPrintErrorMsgToDebugLog(const wchar_t* msg);

bool vscGetLastModifiedDate(const wchar_t* pFileNameStr, time_t& result);

bool vscIsCodeXLServerDll(const wchar_t* dllFullPath);

//////////////////////////////////////////////////////////////////////////
// DLL register utilities:                                              //
//////////////////////////////////////////////////////////////////////////
int vscDllRegisterServer();

int vscDllUnregisterServer();

void vscApplicationCommands_SetOwner(IVscApplicationCommandsOwner* pOwner);

#endif // vscCoreUtils_h__