//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains the definitions of constants used in FileUtils
//==============================================================================

#ifndef _FILE_UTILS_DEFS_H_
#define _FILE_UTILS_DEFS_H_

namespace FileUtils
{
const int FTYPE_UNKNOWN   = 0;
const int FTYPE_EXE       = 1;
const int FTYPE_DLL       = 2;
const int FTYPE_LIB       = 3;

const int FILE_X86          = 32;
const int FILE_X64          = 64;
const int FILE_BITS_UNKNOWN = 0;

// *INDENT-OFF*
#ifdef _WIN32
const int FTYPE_NOT_WIN_BINARY = 10;
#endif
// *INDENT-ON*

const int FILE_FOUND         =  0;
const int FILE_NOT_FOUND     = -1;
const int FILE_NOT_OPENED    = -2;
const int FILE_HDR_NOT_KNOWN = -3;
}
#endif // _FILE_UTILS_DEFS_H_
