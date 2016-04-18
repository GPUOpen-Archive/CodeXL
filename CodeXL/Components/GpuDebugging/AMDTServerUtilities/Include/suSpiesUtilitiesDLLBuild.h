//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSpiesUtilitiesDLLBuild.h
///
//==================================================================================

//------------------------------ suSpiesUtilitiesDLLBuild.h ------------------------------

#ifndef __SUSPIESUTILITIESDLLBUILD_H
#define __SUSPIESUTILITIESDLLBUILD_H

// Under Win32 builds - define: SU_API to be:
// - When building GRSpiesUtilities.dll:    __declspec(dllexport).
// - When building other projects:          __declspec(dllimport).

#if defined(_WIN32)
    #if defined(_GR_SPIES_UTILITIES_EXPORTS)
        #define SU_API __declspec(dllexport)
    #else
        #define SU_API __declspec(dllimport)
    #endif
#else
    #define SU_API
#endif


#endif //__SUSPIESUTILITIESDLLBUILD_H

