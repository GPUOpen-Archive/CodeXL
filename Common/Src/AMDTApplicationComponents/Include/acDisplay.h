//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDisplay.h
///
//==================================================================================

//------------------------------ acDisplay.h ------------------------------

#ifndef __ACDISPLAY
#define __ACDISPLAY

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

#define AC_BASE_DPI 96

#define AC_DEFAULT_LINE_HEIGHT 19

AC_API unsigned int acGetApplicationDPI();
AC_API unsigned int acScalePixelSizeToDisplayDPI(unsigned int baseSize);
AC_API int acScaleSignedPixelSizeToDisplayDPI(int baseSize);
AC_API unsigned int acGetDisplayScalePercent();


#endif  // __ACDISPLAY

