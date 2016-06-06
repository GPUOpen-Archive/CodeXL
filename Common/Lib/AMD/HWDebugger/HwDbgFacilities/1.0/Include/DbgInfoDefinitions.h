//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools
/// \file
/// \brief  Decl definition for Debug Information
//==============================================================================
#ifndef DBGNINFODEFINITIONS_H_
#define DBGNINFODEFINITIONS_H_

/// DLL export macro
#if defined(_WIN32)
    #if defined(DBGINF_EXPORTS)
        #define DBGINF_API __declspec(dllexport)
    #else
        #define DBGINF_API __declspec(dllimport)
    #endif
#else
    #define DBGINF_API
#endif

// The following support move semantics:
// * C++ version 201103 and up
// * MSC (Windows) compiler version 1800 and up
// * Users who explicitly set a flag HWDBGINFO_MOVE_SEMANTICS_ENABLE
// Enable it in relevant class iff one of the above is true, in addition to the flag
// HWDBGINFO_MOVE_SEMANTICS_DISABLE not being set.
#undef HWDBGINFO_MOVE_SEMANTICS
#if (defined(HWDBGINFO_MOVE_SEMANTICS_DISABLE) && (0 == HWDBGINFO_MOVE_SEMANTICS_DISABLE))
    #undef HWDBGINFO_MOVE_SEMANTICS_DISABLE
#endif
#if (((__cplusplus >= 201103L) || (_MSC_VER >= 1800) || defined(HWDBGINFO_MOVE_SEMANTICS_ENABLE)) && (!defined(HWDBGINFO_MOVE_SEMANTICS_DISABLE)))
    #define HWDBGINFO_MOVE_SEMANTICS 1
#endif

#endif // DBGNINFODEFINITIONS_H_
