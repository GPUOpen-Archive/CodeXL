//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtHashMap.h
///
//=====================================================================

//------------------------------ gtHashMap.h ------------------------------

#ifndef __GTHASHMAP
#define __GTHASHMAP

// Local:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#if AMDT_HAS_CPP0X
    #include <unordered_map>
    #define gtHashMap std::unordered_map
#else
    #if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
        #include <unordered_map>
    #else
        #include <tr1/unordered_map>
    #endif
    #define gtHashMap std::tr1::unordered_map
#endif

#endif  // __GTHASHMAP
