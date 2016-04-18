//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CommonUtils.h
///
//==================================================================================

#ifndef __CPUPROFILE_CLI_COMMON_UTILS_H
#define __CPUPROFILE_CLI_COMMON_UTILS_H

// String format
#define STR_FORMAT L"%ls"
// String format embraced with double quotes
#define DQ_STR_FORMAT L"\"%ls\""

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define PATH_SEPARATOR L"\\"
#else
    #define PATH_SEPARATOR L"/"
#endif


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define wcsncpy_truncate(dst, size, src) wcsncpy_s(dst, size, src, _TRUNCATE)
    #define wcsncat_truncate(dst, size, src) wcsncat_s(dst, size, src, _TRUNCATE)
    #define mbstowcs_truncate(dst, size, src) { size_t retVal; mbstowcs_s(&retVal, dst, size, src, _TRUNCATE); }
    #define wcsdup   _wcsdup
#else
    #define wcsncpy_truncate(dst, size, src) wcsncpy(dst, src, (size) - 1)
    #define wcsncat_truncate(dst, size, src) wcsncat(dst, src, (size) - 1 - wcslen(dst))
    #define mbstowcs_truncate(dst, size, src) mbstowcs(dst, src, (size) - 1)
#endif


#endif //__CPUPROFILE_CLI_COMMON_UTILS_H