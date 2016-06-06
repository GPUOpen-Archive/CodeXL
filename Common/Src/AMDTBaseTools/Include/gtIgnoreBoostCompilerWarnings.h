//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtIgnoreBoostCompilerWarnings.h
///
//=====================================================================

//------------------------------ gtIgnoreBoostCompilerWarnings.h ------------------------------

#ifndef __GTIGNOREBOOSTCOMPILERWARNINGS_H
#define __GTIGNOREBOOSTCOMPILERWARNINGS_H

// Local:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>


// ----------------------------------------------------------------------------------
// File Name:            gtIgnoreBoostCompilerWarnings
// General Description:
//  This file contains commands that instruct the compiler to ignore Boost framework warnings
//  that we thing should be GLOBALLY ignored.
//
//  If you would like to have a single warning ignored for a given code, please
//  use #pragma warning( push ) and #pragma warning( pop ) instead.
//
// Author:      AMD Developer Tools Team
// Creation Date:        11/10/2006
// ----------------------------------------------------------------------------------

// If this is a Microsoft compiler build:
#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    //boost warnings disable
    #pragma warning(disable : 4100)
    #pragma warning(disable : 4458)
    #pragma warning(disable : 4459)
    #pragma warning(disable : 4245)
    #pragma warning(disable : 4706)
    #pragma warning(disable : 4503)
    #pragma warning(disable : 4477)
    #pragma warning(disable : 4702)

#endif // AMDT_CPP_COMPILER




#endif //__GTIGNOREBOOSTCOMPILERWARNINGS_H
