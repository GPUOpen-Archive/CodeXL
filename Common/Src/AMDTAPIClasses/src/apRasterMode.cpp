//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apRasterMode.cpp
///
//==================================================================================

//------------------------------ apRasterMode.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apRasterMode.h>


// ---------------------------------------------------------------------------
// Name:        apRasterModeToGLenum
// Description: Translates apRaster mode to the equivalent OpenGL enum raster mode.
// Author:  AMD Developer Tools Team
// Date:        14/11/2004
// ---------------------------------------------------------------------------
GLenum apRasterModeToGLenum(apRasterMode rasterMode)
{
    GLenum retVal = GL_FILL;

    if (rasterMode == AP_RASTER_LINES)
    {
        retVal = GL_LINE;
    }
    else if (rasterMode == AP_RASTER_POINTS)
    {
        retVal = GL_POINT;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLEnumToRasterMode
// Description: Translated GLenum that describes a raster mode to apRasterMode.
// Author:  AMD Developer Tools Team
// Date:        14/11/2004
// ---------------------------------------------------------------------------
apRasterMode apGLEnumToRasterMode(GLenum rasterMode)
{
    apRasterMode retVal = AP_RASTER_FILL;

    if (rasterMode == GL_LINE)
    {
        retVal = AP_RASTER_LINES;
    }
    else if (rasterMode == GL_POINT)
    {
        retVal = AP_RASTER_POINTS;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apRasterModeAsString
// Description: Translates an apRaster mode variable into a string.
// Arguments:   rasterMode - The input raster mode.
//              string - The output string.
// Author:  AMD Developer Tools Team
// Date:        19/6/2005
// ---------------------------------------------------------------------------
void apRasterModeAsString(apRasterMode rasterMode, gtString& string)
{
    string = L"Unknown raster mode";

    switch (rasterMode)
    {
        case AP_RASTER_POINTS:
            string = L"AP_RASTER_POINTS";
            break;

        case AP_RASTER_LINES:
            string = L"AP_RASTER_LINES";
            break;

        case AP_RASTER_FILL:
            string = L"AP_RASTER_FILL";
            break;
    }
}
