//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtHashSet.h
///
//=====================================================================

//------------------------------ gtHashSet.h ------------------------------

#ifndef __GTHASHSET
#define __GTHASHSET

// Local:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#if AMDT_HAS_CPP0X
    #include <unordered_set>
    #define gtHashSet std::unordered_set
#else
    #if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
        #include <unordered_set>
    #else
        #include <tr1/unordered_set>
    #endif
    #define gtHashSet std::tr1::unordered_set
#endif

#endif  // __GTHASHSET
