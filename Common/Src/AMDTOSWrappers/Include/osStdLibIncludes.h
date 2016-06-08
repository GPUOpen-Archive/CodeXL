//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osStdLibIncludes.h
///
//=====================================================================

//------------------------------ osStdLibIncludes.h ------------------------------

#ifndef __OSSTDLIBINCLUDES_H
#define __OSSTDLIBINCLUDES_H

// ---------------------------------------------------------------------------
// File:        osStdLibIncludes.h
//
// Description:
//   This file is a wrapper for stdlib.h that fixes few incompetability
//   problems.
//
// Author:      AMD Developer Tools Team
// Date:        5/7/2003
// ---------------------------------------------------------------------------

// C:
#include <stdlib.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// If we are under a Windows build:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // va_copy is not defined on windows. We define it here:
    #define va_copy(a,b) a = b

#endif


#endif //__OSSTDLIBINCLUDES_H

