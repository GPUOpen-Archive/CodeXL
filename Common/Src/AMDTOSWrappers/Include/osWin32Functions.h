//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osWin32Functions.h
///
//=====================================================================

//------------------------------ osWin32Functions.h ------------------------------

#ifndef __OSWIN32FUNCTIONS
#define __OSWIN32FUNCTIONS

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>


//  -------------------------------------------------------------------------------
//  This file provides access to Win32 functions that are not accessible from our
//  DevStudio version.
//
//  To use a function - ask for its pointer using one of the below functions.
//  (Make sure that you check if the pointer you got is not NULL)
//  -------------------------------------------------------------------------------


// Function types:
typedef BOOL (WINAPI* osPROCSetDLLDirectoryW)(LPCTSTR);
typedef DWORD (WINAPI* osPROCGetModuleFileNameExA)(HANDLE, HMODULE, LPTSTR, DWORD);


// Function pointers:
OS_API osPROCSetDLLDirectoryW osGetWin32SetDLLDirectoryW();


#endif  // __OSWIN32FUNCTIONS
