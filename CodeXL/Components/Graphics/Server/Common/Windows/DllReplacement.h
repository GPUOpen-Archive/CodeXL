//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Miscellaneous helper functions for DLL Replacement functionality
//==============================================================================

namespace DllReplacement
{
//--------------------------------------------------------------
/// LoadRealLibrary: Load in the real library from the Windows
/// system folder. Called by the replacement or proxy dll to get
/// the real function calls
///
/// \param libName The name of the library to load
/// \return handle to the opened library or NULL if error
//--------------------------------------------------------------
HINSTANCE LoadRealLibrary(const char* libName);

//--------------------------------------------------------------
/// Set the DLL directory to where the replaced system dll's reside.
/// The dll names are the same for 32 and 64 bit, so they are placed in
/// the Plugins folder in x86 and x64 subfolders. The Windows loader
/// will get the replaced Dll's rather than the system Dll's
///
/// \param is64Bit Whether the 64-bit directory is needed. Set to
/// true if 64-bit, false for 32-bit
//--------------------------------------------------------------
void SetDllDirectory(bool is64Bit);
}

