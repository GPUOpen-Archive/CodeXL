//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CodeXLVSPackageCoreDefs.h
///
//==================================================================================

#ifndef __CodeXLVSPackageCoreDefs_h
#define __CodeXLVSPackageCoreDefs_h
#include <wchar.h>
#if defined(CXL_VSPACKAGE_CORE_EXPORTS)
    #define CXLVSCORE_API __declspec(dllexport)
#else
    #define CXLVSCORE_API __declspec(dllimport)
#endif
#endif // __CodeXLVSPackageCoreDefs_h
