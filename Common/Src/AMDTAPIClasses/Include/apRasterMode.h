//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apRasterMode.h
///
//==================================================================================

//------------------------------ apRasterMode.h ------------------------------

#ifndef __APRASTERMODE
#define __APRASTERMODE

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>


// Describes a raster mode:
typedef enum
{
    AP_RASTER_POINTS,
    AP_RASTER_LINES,
    AP_RASTER_FILL
} apRasterMode;


AP_API GLenum apRasterModeToGLenum(apRasterMode rasterMode);
AP_API apRasterMode apGLEnumToRasterMode(GLenum rasterMode);
AP_API void apRasterModeAsString(apRasterMode rasterMode, gtString& string);

#endif  // __APRASTERMODE
