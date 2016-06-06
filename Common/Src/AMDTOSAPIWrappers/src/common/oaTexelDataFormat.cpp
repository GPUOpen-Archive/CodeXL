//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaTexelDataFormat.cpp
///
//=====================================================================

//------------------------------ oaTexelDataFormat.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>


// ---------------------------------------------------------------------------
// Name:        oaAmountOfTexelFormatComponents
// Description: Calculates the amount of data components that holds a single
//              texel data format.
// Arguments:   texelDataFormat - The texel data format
// Return Val:  int - Will get the calculated data components amount, or -1
//              in case of failure.
// Author:      AMD Developer Tools Team
// Date:        3/9/2007
// ---------------------------------------------------------------------------
int oaAmountOfTexelFormatComponents(oaTexelDataFormat texelDataFormat)
{
    int retVal = -1;

    switch (texelDataFormat)
    {
        case OA_TEXEL_FORMAT_STENCIL:
        case OA_TEXEL_FORMAT_STENCIL_INDEX1_EXT:
        case OA_TEXEL_FORMAT_STENCIL_INDEX4_EXT:
        case OA_TEXEL_FORMAT_STENCIL_INDEX8_EXT:
        case OA_TEXEL_FORMAT_STENCIL_INDEX16_EXT:

        case OA_TEXEL_FORMAT_DEPTH:
        case OA_TEXEL_FORMAT_DEPTH_COMPONENT16:
        case OA_TEXEL_FORMAT_DEPTH_COMPONENT24:
        case OA_TEXEL_FORMAT_DEPTH_COMPONENT32:

        case OA_TEXEL_FORMAT_DEPTH_EXT:
        case OA_TEXEL_FORMAT_LUMINANCE:
        case OA_TEXEL_FORMAT_LUMINANCE_COMPRESSED:
        case OA_TEXEL_FORMAT_COLORINDEX:
        case OA_TEXEL_FORMAT_RED:
        case OA_TEXEL_FORMAT_RED_COMPRESSED:
        case OA_TEXEL_FORMAT_GREEN:
        case OA_TEXEL_FORMAT_BLUE:
        case OA_TEXEL_FORMAT_ALPHA:
        case OA_TEXEL_FORMAT_ALPHA_COMPRESSED:
        case OA_TEXEL_FORMAT_INTENSITY:
        case OA_TEXEL_FORMAT_INTENSITY_COMPRESSED:
        case OA_TEXEL_FORMAT_VARIABLE_VALUE:

        case OA_TEXEL_FORMAT_V1F:
        case OA_TEXEL_FORMAT_V1D:
        case OA_TEXEL_FORMAT_V1S:
        case OA_TEXEL_FORMAT_V1B:
        case OA_TEXEL_FORMAT_V1UB:
        case OA_TEXEL_FORMAT_V1US:
        case OA_TEXEL_FORMAT_V1I:
        case OA_TEXEL_FORMAT_V1UI:
        case OA_TEXEL_FORMAT_T1S:
        case OA_TEXEL_FORMAT_T1D:
        case OA_TEXEL_FORMAT_T1F:
        case OA_TEXEL_FORMAT_T1I:

        case OA_TEXEL_FORMAT_I1UI:
        case OA_TEXEL_FORMAT_I1UB:
        case OA_TEXEL_FORMAT_I1US:
        case OA_TEXEL_FORMAT_I1S:
        case OA_TEXEL_FORMAT_I1I:
        case OA_TEXEL_FORMAT_I1F:
        case OA_TEXEL_FORMAT_I1D:

        case OA_TEXEL_FORMAT_NORMAL:
        case OA_TEXEL_FORMAT_VECTOR:
        case OA_TEXEL_FORMAT_TEXTURE:
        case OA_TEXEL_FORMAT_INDEX:
        case OA_TEXEL_FORMAT_COLOR:

        case OA_TEXEL_FORMAT_UCHAR:
        case OA_TEXEL_FORMAT_CHAR:
        case OA_TEXEL_FORMAT_SHORT:
        case OA_TEXEL_FORMAT_USHORT:
        case OA_TEXEL_FORMAT_INT:
        case OA_TEXEL_FORMAT_UINT:
        case OA_TEXEL_FORMAT_LONG:
        case OA_TEXEL_FORMAT_ULONG:
        case OA_TEXEL_FORMAT_FLOAT:
        case OA_TEXEL_FORMAT_DOUBLE:
            retVal = 1;
            break;


        case OA_TEXEL_FORMAT_LUMINANCEALPHA:
        case OA_TEXEL_FORMAT_LUMINANCEALPHA_COMPRESSED:
        case OA_TEXEL_FORMAT_RG:
        case OA_TEXEL_FORMAT_RG_COMPRESSED:
        case OA_TEXEL_FORMAT_RA:

        case OA_TEXEL_FORMAT_T2F:
        case OA_TEXEL_FORMAT_T2D:
        case OA_TEXEL_FORMAT_T2S:
        case OA_TEXEL_FORMAT_T2I:
        case OA_TEXEL_FORMAT_V2F:
        case OA_TEXEL_FORMAT_V2D:
        case OA_TEXEL_FORMAT_V2S:
        case OA_TEXEL_FORMAT_V2B:
        case OA_TEXEL_FORMAT_V2UB:
        case OA_TEXEL_FORMAT_V2US:
        case OA_TEXEL_FORMAT_V2I:
        case OA_TEXEL_FORMAT_V2UI:
        case OA_TEXEL_FORMAT_CHAR2:
        case OA_TEXEL_FORMAT_UCHAR2:
        case OA_TEXEL_FORMAT_SHORT2:
        case OA_TEXEL_FORMAT_USHORT2:

        case OA_TEXEL_FORMAT_INT2:
        case OA_TEXEL_FORMAT_UINT2:
        case OA_TEXEL_FORMAT_LONG2:
        case OA_TEXEL_FORMAT_ULONG2:
        case OA_TEXEL_FORMAT_FLOAT2:
        case OA_TEXEL_FORMAT_DOUBLE2:
            retVal = 2;
            break;

        case OA_TEXEL_FORMAT_RGB:
        case OA_TEXEL_FORMAT_BGR:
        case OA_TEXEL_FORMAT_RGB_REVERSED:
        case OA_TEXEL_FORMAT_BGR_REVERSED:
        case OA_TEXEL_FORMAT_RGB_COMPRESSED:

        case OA_TEXEL_FORMAT_RGB_INTEGER:
        case OA_TEXEL_FORMAT_BGR_INTEGER:

        case OA_TEXEL_FORMAT_V3F:
        case OA_TEXEL_FORMAT_V3D:
        case OA_TEXEL_FORMAT_V3S:
        case OA_TEXEL_FORMAT_V3US:
        case OA_TEXEL_FORMAT_V3UB:
        case OA_TEXEL_FORMAT_V3B:
        case OA_TEXEL_FORMAT_V3I:
        case OA_TEXEL_FORMAT_V3UI:
        case OA_TEXEL_FORMAT_C3F:
        case OA_TEXEL_FORMAT_C3D:
        case OA_TEXEL_FORMAT_C3S:
        case OA_TEXEL_FORMAT_C3US:
        case OA_TEXEL_FORMAT_C3B:
        case OA_TEXEL_FORMAT_C3I:
        case OA_TEXEL_FORMAT_C3UB:
        case OA_TEXEL_FORMAT_C3UI:
        case OA_TEXEL_FORMAT_N3F:
        case OA_TEXEL_FORMAT_N3D:
        case OA_TEXEL_FORMAT_N3I:
        case OA_TEXEL_FORMAT_N3S:
        case OA_TEXEL_FORMAT_N3B:
        case OA_TEXEL_FORMAT_T3F:
        case OA_TEXEL_FORMAT_T3D:
        case OA_TEXEL_FORMAT_T3S:
        case OA_TEXEL_FORMAT_T3I:

        case OA_TEXEL_FORMAT_CHAR3:
        case OA_TEXEL_FORMAT_UCHAR3:
        case OA_TEXEL_FORMAT_SHORT3:
        case OA_TEXEL_FORMAT_USHORT3:
        case OA_TEXEL_FORMAT_INT3:
        case OA_TEXEL_FORMAT_UINT3:
        case OA_TEXEL_FORMAT_LONG3:
        case OA_TEXEL_FORMAT_ULONG3:
        case OA_TEXEL_FORMAT_FLOAT3:
        case OA_TEXEL_FORMAT_DOUBLE3:
            retVal = 3;
            break;


        case OA_TEXEL_FORMAT_RGBA:
        case OA_TEXEL_FORMAT_BGRA:
        case OA_TEXEL_FORMAT_RGBA_REVERSED:
        case OA_TEXEL_FORMAT_BGRA_REVERSED:
        case OA_TEXEL_FORMAT_RGBA_COMPRESSED:

        case OA_TEXEL_FORMAT_RGBA_INTEGER:
        case OA_TEXEL_FORMAT_BGRA_INTEGER:

        case OA_TEXEL_FORMAT_V4F:
        case OA_TEXEL_FORMAT_V4D:
        case OA_TEXEL_FORMAT_V4S:
        case OA_TEXEL_FORMAT_V4US:
        case OA_TEXEL_FORMAT_V4B:
        case OA_TEXEL_FORMAT_V4UB:
        case OA_TEXEL_FORMAT_V4I:
        case OA_TEXEL_FORMAT_V4UI:
        case OA_TEXEL_FORMAT_C4F:
        case OA_TEXEL_FORMAT_C4D:
        case OA_TEXEL_FORMAT_C4S:
        case OA_TEXEL_FORMAT_C4US:
        case OA_TEXEL_FORMAT_C4B:
        case OA_TEXEL_FORMAT_C4UB:
        case OA_TEXEL_FORMAT_C4I:
        case OA_TEXEL_FORMAT_C4UI:
        case OA_TEXEL_FORMAT_T4F:
        case OA_TEXEL_FORMAT_T4D:
        case OA_TEXEL_FORMAT_T4S:
        case OA_TEXEL_FORMAT_T4I:

        case OA_TEXEL_FORMAT_CHAR4:
        case OA_TEXEL_FORMAT_UCHAR4:
        case OA_TEXEL_FORMAT_SHORT4:
        case OA_TEXEL_FORMAT_USHORT4:
        case OA_TEXEL_FORMAT_INT4:
        case OA_TEXEL_FORMAT_UINT4:
        case OA_TEXEL_FORMAT_LONG4:
        case OA_TEXEL_FORMAT_ULONG4:
        case OA_TEXEL_FORMAT_FLOAT4:
        case OA_TEXEL_FORMAT_DOUBLE4:
            retVal = 4;
            break;

        case OA_TEXEL_FORMAT_T2F_V3F:
            retVal = 5;
            break;

        case OA_TEXEL_FORMAT_C4UB_V2F:
        case OA_TEXEL_FORMAT_C3F_V3F:
        case OA_TEXEL_FORMAT_N3F_V3F:
        case OA_TEXEL_FORMAT_T2F_C4UB_V3F:
            retVal = 6;
            break;

        case OA_TEXEL_FORMAT_C4UB_V3F:
            retVal = 7;
            break;

        case OA_TEXEL_FORMAT_T4F_V4F:
        case OA_TEXEL_FORMAT_T2F_C3F_V3F:
        case OA_TEXEL_FORMAT_T2F_N3F_V3F:

        case OA_TEXEL_FORMAT_CHAR8:
        case OA_TEXEL_FORMAT_UCHAR8:
        case OA_TEXEL_FORMAT_SHORT8:
        case OA_TEXEL_FORMAT_USHORT8:
        case OA_TEXEL_FORMAT_INT8:
        case OA_TEXEL_FORMAT_UINT8:
        case OA_TEXEL_FORMAT_LONG8:
        case OA_TEXEL_FORMAT_ULONG8:
        case OA_TEXEL_FORMAT_FLOAT8:
        case OA_TEXEL_FORMAT_DOUBLE8:
            retVal = 8;
            break;

        case OA_TEXEL_FORMAT_C4F_N3F_V3F:
            retVal = 10;
            break;

        case OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F:
            retVal = 12;
            break;

        case OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F:
            retVal = 15;
            break;

        case OA_TEXEL_FORMAT_CHAR16:
        case OA_TEXEL_FORMAT_UCHAR16:
        case OA_TEXEL_FORMAT_SHORT16:
        case OA_TEXEL_FORMAT_USHORT16:
        case OA_TEXEL_FORMAT_INT16:
        case OA_TEXEL_FORMAT_UINT16:
        case OA_TEXEL_FORMAT_LONG16:
        case OA_TEXEL_FORMAT_ULONG16:
        case OA_TEXEL_FORMAT_FLOAT16:
        case OA_TEXEL_FORMAT_DOUBLE16:
            retVal = 16;
            break;

        default:
        {
            // Unknown texel data format:
            gtString errString = L"Unknown texel data type";
            errString.appendFormattedString(L": %d", texelDataFormat);
            GT_ASSERT_EX(false, errString.asCharArray());
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaTexelDataFormatToGLEnum
// Description: Translates oaTexelDataFormat to its equivalent GLenum.
// Arguments:   texelDataFormat - The texel data format
// Return Val:  GLenum  - Will get the output GLenum or GL_NONE on failure.
// Author:      AMD Developer Tools Team
// Date:        3/9/2007
// ---------------------------------------------------------------------------
GLenum oaTexelDataFormatToGLEnum(oaTexelDataFormat texelDataFormat)
{
    GLenum retVal = GL_NONE;

    switch (texelDataFormat)
    {
        case OA_TEXEL_FORMAT_STENCIL:
            retVal = GL_STENCIL_INDEX;
            break;

        case OA_TEXEL_FORMAT_STENCIL_INDEX1_EXT:
            retVal = GL_STENCIL_INDEX1_EXT;
            break;

        case OA_TEXEL_FORMAT_STENCIL_INDEX4_EXT:
            retVal = GL_STENCIL_INDEX4_EXT;
            break;

        case OA_TEXEL_FORMAT_STENCIL_INDEX8_EXT:
            retVal = GL_STENCIL_INDEX8_EXT;
            break;

        case OA_TEXEL_FORMAT_STENCIL_INDEX16_EXT:
            retVal = GL_STENCIL_INDEX16_EXT;
            break;

        case OA_TEXEL_FORMAT_DEPTH:
            retVal = GL_DEPTH_COMPONENT;
            break;

        case OA_TEXEL_FORMAT_DEPTH_COMPONENT16:
            retVal = GL_DEPTH_COMPONENT16;
            break;

        case OA_TEXEL_FORMAT_DEPTH_COMPONENT24:
            retVal = GL_DEPTH_COMPONENT24;
            break;

        case OA_TEXEL_FORMAT_DEPTH_COMPONENT32:
            retVal = GL_DEPTH_COMPONENT32;
            break;

        case OA_TEXEL_FORMAT_DEPTH_EXT:
            retVal = GL_DEPTH_COMPONENT;
            break;

        case OA_TEXEL_FORMAT_LUMINANCE:
        case OA_TEXEL_FORMAT_LUMINANCE_COMPRESSED:
            retVal = GL_LUMINANCE;
            break;

        case OA_TEXEL_FORMAT_INTENSITY:
        case OA_TEXEL_FORMAT_INTENSITY_COMPRESSED:
            retVal = GL_INTENSITY;
            break;

        case OA_TEXEL_FORMAT_COLORINDEX:
            retVal = GL_COLOR_INDEX;
            break;

        case OA_TEXEL_FORMAT_RED:
        case OA_TEXEL_FORMAT_RED_COMPRESSED:
            retVal = GL_RED;
            break;

        case OA_TEXEL_FORMAT_GREEN:
            retVal = GL_GREEN;
            break;

        case OA_TEXEL_FORMAT_BLUE:
            retVal = GL_BLUE;
            break;

        case OA_TEXEL_FORMAT_ALPHA:
        case OA_TEXEL_FORMAT_ALPHA_COMPRESSED:
            retVal = GL_ALPHA;
            break;

        case OA_TEXEL_FORMAT_VARIABLE_VALUE:
            // This is not a real texel format, so it should not be read from OpenGL:
            GT_ASSERT(false);
            break;

        case OA_TEXEL_FORMAT_LUMINANCEALPHA:
        case OA_TEXEL_FORMAT_LUMINANCEALPHA_COMPRESSED:
            retVal = GL_LUMINANCE_ALPHA;
            break;

        case OA_TEXEL_FORMAT_RG:
        case OA_TEXEL_FORMAT_RG_COMPRESSED:
            retVal = GL_RG;
            break;

        case OA_TEXEL_FORMAT_RA:
            // Red-alpha format is not supported by OpenGL:
            GT_ASSERT(false);
            break;

        case OA_TEXEL_FORMAT_RGB:
        case OA_TEXEL_FORMAT_RGB_REVERSED:
        case OA_TEXEL_FORMAT_RGB_COMPRESSED:
            retVal = GL_RGB;
            break;

        case OA_TEXEL_FORMAT_BGR:
        case OA_TEXEL_FORMAT_BGR_REVERSED:
            retVal = GL_BGR;
            break;

        case OA_TEXEL_FORMAT_RGBA:
        case OA_TEXEL_FORMAT_RGBA_REVERSED:
        case OA_TEXEL_FORMAT_RGBA_COMPRESSED:
            retVal = GL_RGBA;
            break;

        case OA_TEXEL_FORMAT_BGRA:
        case OA_TEXEL_FORMAT_BGRA_REVERSED:
            retVal = GL_BGRA;
            break;

        case OA_TEXEL_FORMAT_RGB_INTEGER:
            retVal = GL_RGB_INTEGER;
            break;

        case OA_TEXEL_FORMAT_BGR_INTEGER:
            retVal = GL_BGR_INTEGER;
            break;

        case OA_TEXEL_FORMAT_RGBA_INTEGER:
            retVal = GL_RGBA_INTEGER;
            break;

        case OA_TEXEL_FORMAT_BGRA_INTEGER:
            retVal = GL_BGRA_INTEGER;
            break;

        // VBO formats:
        case OA_TEXEL_FORMAT_V2F:
            retVal = GL_V2F;
            break;

        case OA_TEXEL_FORMAT_V3F:
            retVal = GL_V3F;
            break;

        case OA_TEXEL_FORMAT_C4UB_V2F:
            retVal = GL_C4UB_V2F;
            break;

        case OA_TEXEL_FORMAT_C4UB_V3F:
            retVal = GL_C4UB_V3F;
            break;

        case OA_TEXEL_FORMAT_C3F_V3F:
            retVal = GL_C3F_V3F;
            break;

        case OA_TEXEL_FORMAT_N3F_V3F:
            retVal = GL_N3F_V3F;
            break;

        case OA_TEXEL_FORMAT_C4F_N3F_V3F:
            retVal = GL_C4F_N3F_V3F;
            break;

        case OA_TEXEL_FORMAT_T2F_V3F:
            retVal = GL_T2F_V3F;
            break;

        case OA_TEXEL_FORMAT_T4F_V4F:
            retVal = GL_T4F_V4F;
            break;

        case OA_TEXEL_FORMAT_T2F_C4UB_V3F:
            retVal = GL_T2F_C4UB_V3F;
            break;

        case OA_TEXEL_FORMAT_T2F_C3F_V3F:
            retVal = GL_T2F_C3F_V3F;
            break;

        case OA_TEXEL_FORMAT_T2F_N3F_V3F:
            retVal = GL_T2F_N3F_V3F;
            break;

        case OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F:
            retVal = GL_T2F_C4F_N3F_V3F;
            break;

        case OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F:
            retVal = GL_T4F_C4F_N3F_V4F;
            break;

        default:
        {
            // Unknown texel data format:
            gtString errString = L"Unknown texel data type";
            errString.appendFormattedString(L": %d", texelDataFormat);
            GT_ASSERT_EX(false, errString.asCharArray());
        }
        break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaTexelDataFormatAsString
// Description: Translates oaTexelDataFormat to a printable string.
// Arguments:   texelDataFormat - Input texel data format.
//              texelDataFormatAsString - Output string.
// Return Val:  bool - Convert was successful / failure
// Author:      AMD Developer Tools Team
// Date:        3/4/2008
// ---------------------------------------------------------------------------
bool oaTexelDataFormatAsString(oaTexelDataFormat texelDataFormat, gtString& texelDataFormatAsString)
{
    bool retVal = true;

    // Clear the output string:
    texelDataFormatAsString.makeEmpty();

    switch (texelDataFormat)
    {
        case OA_TEXEL_FORMAT_UNKNOWN:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_UNKNOWN";
            break;

        case OA_TEXEL_FORMAT_STENCIL:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_STENCIL";
            break;

        case OA_TEXEL_FORMAT_STENCIL_INDEX1_EXT:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_STENCIL_INDEX1_EXT";
            break;

        case OA_TEXEL_FORMAT_STENCIL_INDEX4_EXT:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_STENCIL_INDEX4_EXT";
            break;

        case OA_TEXEL_FORMAT_STENCIL_INDEX8_EXT:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_STENCIL_INDEX8_EXT";
            break;

        case OA_TEXEL_FORMAT_STENCIL_INDEX16_EXT:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_STENCIL_INDEX16EXT";
            break;

        case OA_TEXEL_FORMAT_DEPTH:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_DEPTH";
            break;

        case OA_TEXEL_FORMAT_DEPTH_COMPONENT16:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_DEPTH_COMPONENT16";
            break;

        case OA_TEXEL_FORMAT_DEPTH_COMPONENT24:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_DEPTH_COMPONENT24";
            break;

        case OA_TEXEL_FORMAT_DEPTH_COMPONENT32:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_DEPTH_COMPONENT32";
            break;

        case OA_TEXEL_FORMAT_DEPTH_EXT:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_DEPTH_EXT";
            break;

        case OA_TEXEL_FORMAT_LUMINANCE:
        case OA_TEXEL_FORMAT_LUMINANCE_COMPRESSED:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_LUMINANCE";
            break;

        case OA_TEXEL_FORMAT_INTENSITY:
        case OA_TEXEL_FORMAT_INTENSITY_COMPRESSED:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_INTENSITY";
            break;

        case OA_TEXEL_FORMAT_COLORINDEX:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_COLORINDEX";
            break;

        case OA_TEXEL_FORMAT_RED:
        case OA_TEXEL_FORMAT_RED_COMPRESSED:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_RED";
            break;

        case OA_TEXEL_FORMAT_GREEN:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_GREEN";
            break;

        case OA_TEXEL_FORMAT_BLUE:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_BLUE";
            break;

        case OA_TEXEL_FORMAT_ALPHA:
        case OA_TEXEL_FORMAT_ALPHA_COMPRESSED:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_ALPHA";
            break;

        case OA_TEXEL_FORMAT_VARIABLE_VALUE:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_VARIABLE_VALUE";
            break;

        case OA_TEXEL_FORMAT_LUMINANCEALPHA:
        case OA_TEXEL_FORMAT_LUMINANCEALPHA_COMPRESSED:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_LUMINANCEALPHA";
            break;

        case OA_TEXEL_FORMAT_RG:
        case OA_TEXEL_FORMAT_RG_COMPRESSED:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_RG";
            break;

        case OA_TEXEL_FORMAT_RA:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_RA";
            break;

        case OA_TEXEL_FORMAT_RGB:
        case OA_TEXEL_FORMAT_RGB_REVERSED:
        case OA_TEXEL_FORMAT_RGB_COMPRESSED:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_RGB";
            break;

        case OA_TEXEL_FORMAT_BGR:
        case OA_TEXEL_FORMAT_BGR_REVERSED:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_BGR";
            break;

        case OA_TEXEL_FORMAT_RGBA:
        case OA_TEXEL_FORMAT_RGBA_REVERSED:
        case OA_TEXEL_FORMAT_RGBA_COMPRESSED:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_RGBA";
            break;

        case OA_TEXEL_FORMAT_BGRA:
        case OA_TEXEL_FORMAT_BGRA_REVERSED:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_BGRA";
            break;

        case OA_TEXEL_FORMAT_RGB_INTEGER:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_RGB_INTEGER";
            break;

        case OA_TEXEL_FORMAT_BGR_INTEGER:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_BGR_INTEGER";
            break;

        case OA_TEXEL_FORMAT_RGBA_INTEGER:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_RGBA_INTEGER";
            break;

        case OA_TEXEL_FORMAT_BGRA_INTEGER:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_BGRA_INTEGER";
            break;

        case OA_TEXEL_FORMAT_V1F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V1F";
            break;

        case OA_TEXEL_FORMAT_V2F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V2F";
            break;

        case OA_TEXEL_FORMAT_V3F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V3F";
            break;

        case OA_TEXEL_FORMAT_C4UB_V2F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C4UB_V2F";
            break;

        case OA_TEXEL_FORMAT_C4UB_V3F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C4UB_V3F";
            break;

        case OA_TEXEL_FORMAT_C3F_V3F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C3F_V3F";
            break;

        case OA_TEXEL_FORMAT_N3F_V3F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_N3F_V3F";
            break;

        case OA_TEXEL_FORMAT_C4F_N3F_V3F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C4F_N3F_V3F";
            break;

        case OA_TEXEL_FORMAT_T2F_V3F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T2F_V3F";
            break;

        case OA_TEXEL_FORMAT_T4F_V4F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T4F_V4F";
            break;

        case OA_TEXEL_FORMAT_T2F_C4UB_V3F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T2F_C4UB_V3F";
            break;

        case OA_TEXEL_FORMAT_T2F_C3F_V3F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T2F_C3F_V3F";
            break;

        case OA_TEXEL_FORMAT_T2F_N3F_V3F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T2F_N3F_V3F";
            break;

        case OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F";
            break;

        case OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F";
            break;

        case OA_TEXEL_FORMAT_V4F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V4F";
            break;

        case OA_TEXEL_FORMAT_V1D:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V1D";
            break;

        case OA_TEXEL_FORMAT_V2D:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V2D";
            break;

        case OA_TEXEL_FORMAT_V3D:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V3D";
            break;

        case OA_TEXEL_FORMAT_V4D:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V4D";
            break;

        case OA_TEXEL_FORMAT_V1S:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V1S";
            break;

        case OA_TEXEL_FORMAT_V2S:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V2S";
            break;

        case OA_TEXEL_FORMAT_V3S:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V3S";
            break;

        case OA_TEXEL_FORMAT_V4S:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V4S";
            break;

        case OA_TEXEL_FORMAT_V1B:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V1B";
            break;

        case OA_TEXEL_FORMAT_V2B:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V2B";
            break;

        case OA_TEXEL_FORMAT_V3B:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V3B";
            break;

        case OA_TEXEL_FORMAT_V4B:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V4B";
            break;

        case OA_TEXEL_FORMAT_V1US:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V1US";
            break;

        case OA_TEXEL_FORMAT_V2US:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V2US";
            break;

        case OA_TEXEL_FORMAT_V3US:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V3US";
            break;

        case OA_TEXEL_FORMAT_V4US:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V4US";
            break;

        case OA_TEXEL_FORMAT_V1UB:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V1UB";
            break;

        case OA_TEXEL_FORMAT_V2UB:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V2UB";
            break;

        case OA_TEXEL_FORMAT_V3UB:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V3UB";
            break;

        case OA_TEXEL_FORMAT_V4UB:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V4UB";
            break;

        case OA_TEXEL_FORMAT_V1I:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V1I";
            break;

        case OA_TEXEL_FORMAT_V2I:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V2I";
            break;

        case OA_TEXEL_FORMAT_V3I:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V3I";
            break;

        case OA_TEXEL_FORMAT_V4I:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V4I";
            break;

        case OA_TEXEL_FORMAT_V1UI:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V1UI";
            break;

        case OA_TEXEL_FORMAT_V2UI:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V2UI";
            break;

        case OA_TEXEL_FORMAT_V3UI:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V3UI";
            break;

        case OA_TEXEL_FORMAT_V4UI:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_V4UI";
            break;

        case OA_TEXEL_FORMAT_C3F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C3F";
            break;

        case OA_TEXEL_FORMAT_C4F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C4F";
            break;

        case OA_TEXEL_FORMAT_C3D:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C3D";
            break;

        case OA_TEXEL_FORMAT_C4D:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C4D";
            break;

        case OA_TEXEL_FORMAT_C3S:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C3S";
            break;

        case OA_TEXEL_FORMAT_C4S:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C4S";
            break;

        case OA_TEXEL_FORMAT_C3US:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C3US";
            break;

        case OA_TEXEL_FORMAT_C4US:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C4US";
            break;

        case OA_TEXEL_FORMAT_C3B:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C3B";
            break;

        case OA_TEXEL_FORMAT_C4B:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C4B";
            break;

        case OA_TEXEL_FORMAT_C3UB:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C3UB";
            break;

        case OA_TEXEL_FORMAT_C4UB:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C4UB";
            break;

        case OA_TEXEL_FORMAT_C3I:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C3I";
            break;

        case OA_TEXEL_FORMAT_C4I:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C4I";
            break;

        case OA_TEXEL_FORMAT_C3UI:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C3UI";
            break;

        case OA_TEXEL_FORMAT_C4UI:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_C4UI";
            break;


        case OA_TEXEL_FORMAT_N3F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_N3F";
            break;

        case OA_TEXEL_FORMAT_N3D:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_N3D";
            break;

        case OA_TEXEL_FORMAT_N3I:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_N3I";
            break;

        case OA_TEXEL_FORMAT_N3S:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_N3S";
            break;

        case OA_TEXEL_FORMAT_N3B:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_N3B";
            break;

        case OA_TEXEL_FORMAT_T1F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T1F";
            break;

        case OA_TEXEL_FORMAT_T2F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T2F";
            break;

        case OA_TEXEL_FORMAT_T3F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T3F";
            break;

        case OA_TEXEL_FORMAT_T4F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T4F";
            break;

        case OA_TEXEL_FORMAT_T1D:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T1D";
            break;

        case OA_TEXEL_FORMAT_T2D:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T2D";
            break;

        case OA_TEXEL_FORMAT_T3D:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T3D";
            break;

        case OA_TEXEL_FORMAT_T4D:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T4D";
            break;

        case OA_TEXEL_FORMAT_T1S:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T1S";
            break;

        case OA_TEXEL_FORMAT_T2S:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T2S";
            break;

        case OA_TEXEL_FORMAT_T3S:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T3S";
            break;

        case OA_TEXEL_FORMAT_T4S:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T4S";
            break;

        case OA_TEXEL_FORMAT_T1I:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T1I";
            break;

        case OA_TEXEL_FORMAT_T2I:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T2I";
            break;

        case OA_TEXEL_FORMAT_T3I:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T3I";
            break;

        case OA_TEXEL_FORMAT_T4I:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_T4I";
            break;

        case OA_TEXEL_FORMAT_I1UI:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_I1UI";
            break;

        case OA_TEXEL_FORMAT_I1UB:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_I1UB";
            break;

        case OA_TEXEL_FORMAT_I1US:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_I1US";
            break;

        case OA_TEXEL_FORMAT_I1S:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_I1S";
            break;

        case OA_TEXEL_FORMAT_I1I:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_I1I";
            break;

        case OA_TEXEL_FORMAT_I1F:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_I1F";
            break;

        case OA_TEXEL_FORMAT_I1D:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_I1D";
            break;

        case OA_TEXEL_FORMAT_CHAR2:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_CHAR2";
            break;

        case OA_TEXEL_FORMAT_CHAR3:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_CHAR3";
            break;

        case OA_TEXEL_FORMAT_CHAR4:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_CHAR4";
            break;

        case OA_TEXEL_FORMAT_CHAR8:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_CHAR8";
            break;

        case OA_TEXEL_FORMAT_CHAR16:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_CHAR16";
            break;

        case OA_TEXEL_FORMAT_UCHAR2:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_UCHAR2";
            break;

        case OA_TEXEL_FORMAT_UCHAR3:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_UCHAR3";
            break;

        case OA_TEXEL_FORMAT_UCHAR4:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_UCHAR4";
            break;

        case OA_TEXEL_FORMAT_UCHAR8:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_UCHAR8";
            break;

        case OA_TEXEL_FORMAT_UCHAR16:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_UCHAR16";
            break;

        case OA_TEXEL_FORMAT_SHORT2:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_SHORT2";
            break;

        case OA_TEXEL_FORMAT_SHORT3:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_SHORT3";
            break;

        case OA_TEXEL_FORMAT_SHORT4:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_SHORT4";
            break;

        case OA_TEXEL_FORMAT_SHORT8:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_SHORT8";
            break;

        case OA_TEXEL_FORMAT_SHORT16:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_SHORT16";
            break;

        case OA_TEXEL_FORMAT_USHORT2:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_USHORT2";
            break;

        case OA_TEXEL_FORMAT_USHORT3:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_USHORT3";
            break;

        case OA_TEXEL_FORMAT_USHORT4:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_USHORT4";
            break;

        case OA_TEXEL_FORMAT_USHORT8:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_USHORT8";
            break;

        case OA_TEXEL_FORMAT_USHORT16:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_USHORT16";
            break;

        case OA_TEXEL_FORMAT_INT2:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_INT2";
            break;

        case OA_TEXEL_FORMAT_INT3:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_INT3";
            break;

        case OA_TEXEL_FORMAT_INT4:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_INT4";
            break;

        case OA_TEXEL_FORMAT_INT8:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_INT8";
            break;

        case OA_TEXEL_FORMAT_INT16:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_INT16";
            break;

        case OA_TEXEL_FORMAT_UINT2:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_UINT2";
            break;

        case OA_TEXEL_FORMAT_UINT3:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_UINT3";
            break;

        case OA_TEXEL_FORMAT_UINT4:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_UINT4";
            break;

        case OA_TEXEL_FORMAT_UINT8:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_UINT8";
            break;

        case OA_TEXEL_FORMAT_UINT16:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_UINT16";
            break;

        case OA_TEXEL_FORMAT_LONG2:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_LONG2";
            break;

        case OA_TEXEL_FORMAT_LONG3:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_LONG2";
            break;

        case OA_TEXEL_FORMAT_LONG4:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_LONG4";
            break;

        case OA_TEXEL_FORMAT_LONG8:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_LONG8";
            break;

        case OA_TEXEL_FORMAT_LONG16:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_LONG16";
            break;

        case OA_TEXEL_FORMAT_ULONG2:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_ULONG2";
            break;

        case OA_TEXEL_FORMAT_ULONG3:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_ULONG2";
            break;

        case OA_TEXEL_FORMAT_ULONG4:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_ULONG4";
            break;

        case OA_TEXEL_FORMAT_ULONG8:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_ULONG8";
            break;

        case OA_TEXEL_FORMAT_ULONG16:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_ULONG16";
            break;

        case OA_TEXEL_FORMAT_FLOAT2:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_FLOAT2";
            break;

        case OA_TEXEL_FORMAT_FLOAT3:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_FLOAT3";
            break;

        case OA_TEXEL_FORMAT_FLOAT4:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_FLOAT4";
            break;

        case OA_TEXEL_FORMAT_FLOAT8:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_FLOAT8";
            break;

        case OA_TEXEL_FORMAT_FLOAT16:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_FLOAT16";
            break;

        case OA_TEXEL_FORMAT_DOUBLE2:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_DOUBLE2";
            break;

        case OA_TEXEL_FORMAT_DOUBLE3:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_DOUBLE3";
            break;

        case OA_TEXEL_FORMAT_DOUBLE4:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_DOUBLE4";
            break;

        case OA_TEXEL_FORMAT_DOUBLE8:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_DOUBLE8";
            break;

        case OA_TEXEL_FORMAT_DOUBLE16:
            texelDataFormatAsString = L"OA_TEXEL_FORMAT_DOUBLE16";
            break;

        default:
        {
            retVal = false;
            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaGLEnumToTexelDataFormat
// Description: Translates GLenum to its equivalent oaTexelDataFormat
// Arguments:   dataFormat - Input GLenum data format
//              texelDataFormat - Output texel data format
// Return Val:  bool - Convert was successful / failure
// Author:      AMD Developer Tools Team
// Date:        3/12/2007
// ---------------------------------------------------------------------------
bool OA_API oaGLEnumToTexelDataFormat(GLenum dataFormat, oaTexelDataFormat& texelDataFormat)
{
    bool retVal = true;

    switch (dataFormat)
    {
        case GL_STENCIL_INDEX:
            texelDataFormat = OA_TEXEL_FORMAT_STENCIL;
            break;

        case GL_STENCIL_INDEX1_EXT:
            texelDataFormat = OA_TEXEL_FORMAT_STENCIL_INDEX1_EXT;
            break;

        case GL_STENCIL_INDEX4_EXT:
            texelDataFormat = OA_TEXEL_FORMAT_STENCIL_INDEX4_EXT;
            break;

        case GL_STENCIL_INDEX8_EXT:
            texelDataFormat = OA_TEXEL_FORMAT_STENCIL_INDEX8_EXT;
            break;

        case GL_STENCIL_INDEX16_EXT:
            texelDataFormat = OA_TEXEL_FORMAT_STENCIL_INDEX16_EXT;
            break;

        case GL_DEPTH_COMPONENT:
            texelDataFormat = OA_TEXEL_FORMAT_DEPTH;
            break;

        case GL_DEPTH_COMPONENT16:
            texelDataFormat = OA_TEXEL_FORMAT_DEPTH_COMPONENT16;
            break;

        case GL_DEPTH_COMPONENT24:
            texelDataFormat = OA_TEXEL_FORMAT_DEPTH_COMPONENT24;
            break;

        case GL_DEPTH_COMPONENT32:
            texelDataFormat = OA_TEXEL_FORMAT_DEPTH_COMPONENT32;
            break;

        case GL_DEPTH_ATTACHMENT_EXT:
            texelDataFormat = OA_TEXEL_FORMAT_DEPTH_EXT;
            break;

        case 1:
        case GL_LUMINANCE:
        case GL_LUMINANCE4:
        case GL_LUMINANCE8:
        case GL_LUMINANCE12:
        case GL_LUMINANCE16:
        case GL_LUMINANCE8I_EXT:
        case GL_LUMINANCE8UI_EXT:
        case GL_LUMINANCE16F_ARB:
        case GL_LUMINANCE16I_EXT:
        case GL_LUMINANCE16UI_EXT:
        case GL_LUMINANCE32F_ARB:
        case GL_LUMINANCE32I_EXT:
        case GL_LUMINANCE32UI_EXT:
        case GL_SLUMINANCE:
        case GL_SLUMINANCE8:
        case GL_LUMINANCE_SNORM:
        case GL_LUMINANCE8_SNORM:
        case GL_LUMINANCE16_SNORM:
            texelDataFormat = OA_TEXEL_FORMAT_LUMINANCE;
            // TODO: Should add more texel format types????
            break;

        case GL_COMPRESSED_LUMINANCE:
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SLUMINANCE_EXT:
            texelDataFormat = OA_TEXEL_FORMAT_LUMINANCE_COMPRESSED;
            break;

        case GL_INTENSITY:
        case GL_INTENSITY4:
        case GL_INTENSITY8:
        case GL_INTENSITY12:
        case GL_INTENSITY16:
        case GL_INTENSITY8I_EXT:
        case GL_INTENSITY8UI_EXT:
        case GL_INTENSITY16F_ARB:
        case GL_INTENSITY16I_EXT:
        case GL_INTENSITY16UI_EXT:
        case GL_INTENSITY32F_ARB:
        case GL_INTENSITY32I_EXT:
        case GL_INTENSITY32UI_EXT:
        case GL_INTENSITY_SNORM:
        case GL_INTENSITY8_SNORM:
        case GL_INTENSITY16_SNORM:
            // TODO: Should add more texel format types????
            texelDataFormat = OA_TEXEL_FORMAT_INTENSITY;
            break;

        case GL_COMPRESSED_INTENSITY:
            texelDataFormat = OA_TEXEL_FORMAT_INTENSITY_COMPRESSED;
            break;

        case GL_COLOR_INDEX:
        case GL_COLOR_INDEX1_EXT:
        case GL_COLOR_INDEX2_EXT:
        case GL_COLOR_INDEX4_EXT:
        case GL_COLOR_INDEX8_EXT:
        case GL_COLOR_INDEX12_EXT:
        case GL_COLOR_INDEX16_EXT:
            texelDataFormat = OA_TEXEL_FORMAT_COLORINDEX;
            break;

        case GL_RED:
        case GL_R8:
        case GL_R16:
        case GL_R16F:
        case GL_R32F:
        case GL_R8I:
        case GL_R8UI:
        case GL_R16I:
        case GL_R16UI:
        case GL_R32I:
        case GL_R32UI:
        case GL_R8_SNORM:
        case GL_R16_SNORM:
        case GL_RED_SNORM:
            texelDataFormat = OA_TEXEL_FORMAT_RED;
            break;

        case GL_COMPRESSED_RED:
        case GL_COMPRESSED_RED_RGTC1:
        case GL_COMPRESSED_SIGNED_RED_RGTC1:
        case GL_COMPRESSED_R11_EAC:
        case GL_COMPRESSED_SIGNED_R11_EAC:
            texelDataFormat = OA_TEXEL_FORMAT_RED_COMPRESSED;
            break;

        case GL_GREEN:
            texelDataFormat = OA_TEXEL_FORMAT_GREEN;
            break;

        case GL_BLUE:
            texelDataFormat = OA_TEXEL_FORMAT_BLUE;
            break;

        case GL_ALPHA:
        case GL_ALPHA4:
        case GL_ALPHA8:
        case GL_ALPHA12:
        case GL_ALPHA16:
        case GL_ALPHA8I_EXT:
        case GL_ALPHA8UI_EXT:
        case GL_ALPHA16F_ARB:
        case GL_ALPHA16I_EXT:
        case GL_ALPHA16UI_EXT:
        case GL_ALPHA32F_ARB:
        case GL_ALPHA32I_EXT:
        case GL_ALPHA32UI_EXT:
        case GL_ALPHA_SNORM:
        case GL_ALPHA8_SNORM:
        case GL_ALPHA16_SNORM:
            // TODO: Should add more texel format types????
            texelDataFormat = OA_TEXEL_FORMAT_ALPHA;
            break;

        case GL_COMPRESSED_ALPHA:
            texelDataFormat = OA_TEXEL_FORMAT_ALPHA_COMPRESSED;
            break;

        case 2:
        case GL_LUMINANCE_ALPHA:
        case GL_SLUMINANCE_ALPHA:
        case GL_SLUMINANCE8_ALPHA8:
        case GL_LUMINANCE4_ALPHA4:
        case GL_LUMINANCE6_ALPHA2:
        case GL_LUMINANCE8_ALPHA8:
        case GL_LUMINANCE12_ALPHA4:
        case GL_LUMINANCE12_ALPHA12:
        case GL_LUMINANCE16_ALPHA16:
        case GL_LUMINANCE_ALPHA8I_EXT:
        case GL_LUMINANCE_ALPHA8UI_EXT:
        case GL_LUMINANCE_ALPHA16F_ARB:
        case GL_LUMINANCE_ALPHA16I_EXT:
        case GL_LUMINANCE_ALPHA16UI_EXT:
        case GL_LUMINANCE_ALPHA32F_ARB:
        case GL_LUMINANCE_ALPHA32I_EXT:
        case GL_LUMINANCE_ALPHA32UI_EXT:
        case GL_LUMINANCE_ALPHA_SNORM:
        case GL_LUMINANCE8_ALPHA8_SNORM:
        case GL_LUMINANCE16_ALPHA16_SNORM:
            // TODO: Should add more texel format types????
            texelDataFormat = OA_TEXEL_FORMAT_LUMINANCEALPHA;
            break;

        case GL_COMPRESSED_LUMINANCE_ALPHA:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SLUMINANCE_ALPHA_EXT:
            texelDataFormat = OA_TEXEL_FORMAT_LUMINANCEALPHA_COMPRESSED;
            break;

        case GL_RG:
        case GL_RG_INTEGER:
        case GL_RG8:
        case GL_RG16:
        case GL_RG16F:
        case GL_RG32F:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG32I:
        case GL_RG32UI:
        case GL_RG_SNORM:
        case GL_RG8_SNORM:
        case GL_RG16_SNORM:
            texelDataFormat = OA_TEXEL_FORMAT_RG;
            break;

        case GL_COMPRESSED_RG:
        case GL_COMPRESSED_RG_RGTC2:
        case GL_COMPRESSED_SIGNED_RG_RGTC2:
        case GL_COMPRESSED_RG11_EAC:
        case GL_COMPRESSED_SIGNED_RG11_EAC:
            texelDataFormat = OA_TEXEL_FORMAT_RG_COMPRESSED;
            break;

        case 3:
        case GL_RGB:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
        case GL_SRGB:
        case GL_SRGB8:
        case GL_RGB8I_EXT:
        case GL_RGB8UI_EXT:
        case GL_RGB9_E5_EXT:
        case GL_RGB_FLOAT16_ATI:
        case GL_RGB16I_EXT:
        case GL_RGB16UI_EXT:
        case GL_RGB32F:
        case GL_RGB32I_EXT:
        case GL_RGB32UI_EXT:
        case GL_RGB_SNORM:
        case GL_RGB8_SNORM:
        case GL_RGB16_SNORM:
            // TODO: Should add more texel format types????
            texelDataFormat = OA_TEXEL_FORMAT_RGB;
            break;

        case GL_COMPRESSED_RGB:
        case GL_COMPRESSED_RGB_FXT1_3DFX:
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_EXT:
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGB8_ETC2:
        case GL_COMPRESSED_SRGB8_ETC2:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        case GL_PALETTE4_R5_G6_B5_OES:
        case GL_PALETTE4_RGB8_OES:
        case GL_PALETTE8_R5_G6_B5_OES:
        case GL_PALETTE8_RGB8_OES:
#endif
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
        case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
#endif
            texelDataFormat = OA_TEXEL_FORMAT_RGB_COMPRESSED;
            break;

        case GL_BGR:
            texelDataFormat = OA_TEXEL_FORMAT_BGR;
            break;

        case GL_RGB_INTEGER:
            texelDataFormat = OA_TEXEL_FORMAT_RGB_INTEGER;
            break;

        case GL_BGR_INTEGER:
            texelDataFormat = OA_TEXEL_FORMAT_BGR_INTEGER;
            break;

        case 4:
        case GL_RGBA:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGBA8:
        case GL_RGB10_A2:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_RGBA8I_EXT:
        case GL_RGBA8UI_EXT:
        case GL_RGBA16F:
        case GL_RGBA16I_EXT:
        case GL_RGBA16UI_EXT:
        case GL_RGBA32F_ARB:
        case GL_RGBA32I_EXT:
        case GL_RGBA32UI_EXT:
        case GL_SRGB_ALPHA:
        case GL_SRGB8_ALPHA8:
        case GL_RGBA_SNORM:
        case GL_RGBA8_SNORM:
        case GL_RGBA16_SNORM:
            // TODO: Should add more texel format types????
            texelDataFormat = OA_TEXEL_FORMAT_RGBA;
            break;

        case GL_BGRA:
            texelDataFormat = OA_TEXEL_FORMAT_BGRA;
            break;

        case GL_COMPRESSED_RGBA:
        case GL_COMPRESSED_RGBA_FXT1_3DFX:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_RGBA8_ETC2_EAC:
        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        case GL_PALETTE4_RGBA4_OES:
        case GL_PALETTE4_RGB5_A1_OES:
        case GL_PALETTE4_RGBA8_OES:
        case GL_PALETTE8_RGBA4_OES:
        case GL_PALETTE8_RGB5_A1_OES:
        case GL_PALETTE8_RGBA8_OES:
#endif
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
        case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
#endif
            texelDataFormat = OA_TEXEL_FORMAT_RGBA_COMPRESSED;
            break;

        case GL_RGBA_INTEGER:
            texelDataFormat = OA_TEXEL_FORMAT_RGBA_INTEGER;
            break;

        case GL_BGRA_INTEGER:
            texelDataFormat = OA_TEXEL_FORMAT_BGRA_INTEGER;
            break;

        case GL_V2F:
            texelDataFormat = OA_TEXEL_FORMAT_V2F;
            break;

        case GL_V3F:
            texelDataFormat = OA_TEXEL_FORMAT_V3F;
            break;

        case GL_C4UB_V2F:
            texelDataFormat = OA_TEXEL_FORMAT_C4UB_V2F;
            break;

        case GL_C4UB_V3F:
            texelDataFormat = OA_TEXEL_FORMAT_C4UB_V3F;
            break;

        case GL_C3F_V3F:
            texelDataFormat = OA_TEXEL_FORMAT_C3F_V3F;
            break;

        case GL_N3F_V3F:
            texelDataFormat = OA_TEXEL_FORMAT_N3F_V3F;
            break;

        case GL_C4F_N3F_V3F:
            texelDataFormat = OA_TEXEL_FORMAT_C4F_N3F_V3F;
            break;

        case GL_T2F_V3F:
            texelDataFormat = OA_TEXEL_FORMAT_T2F_V3F;
            break;

        case GL_T4F_V4F:
            texelDataFormat = OA_TEXEL_FORMAT_T4F_V4F;
            break;

        case GL_T2F_C4UB_V3F:
            texelDataFormat = OA_TEXEL_FORMAT_T2F_C4UB_V3F;
            break;

        case GL_T2F_C3F_V3F:
            texelDataFormat = OA_TEXEL_FORMAT_T2F_C3F_V3F;
            break;

        case GL_T2F_N3F_V3F:
            texelDataFormat = OA_TEXEL_FORMAT_T2F_N3F_V3F;
            break;

        case GL_T2F_C4F_N3F_V3F:
            texelDataFormat = OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F;
            break;

        case GL_T4F_C4F_N3F_V4F:
            texelDataFormat = OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F;
            break;

        default:
        {
            // Unknown texel data format:
            retVal = false;
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaGetTexelDataFormatName
// Description: Returns the channel name, given a channel type
// Arguments:   texelDataFormat - The channel type
//              channelName - Output channel name
//              shortVersion - Get the name in full version or short version
// Return Val:  bool - Success / Failure
// Author:      AMD Developer Tools Team
// Date:        3/9/2007
// ---------------------------------------------------------------------------
bool oaGetTexelDataFormatName(oaTexelDataFormat texelDataFormat, gtString& channelName, bool shortVersion)
{
    bool retVal = false;

    // Empty output string
    channelName.makeEmpty();

    // We only return the channel name for individual channels
    int amountOfComponents = oaAmountOfTexelFormatComponents(texelDataFormat);

    if (amountOfComponents == 1)
    {
        retVal = true;

        switch (texelDataFormat)
        {
            case OA_TEXEL_FORMAT_RED:
            case OA_TEXEL_FORMAT_RED_COMPRESSED:
            {
                if (shortVersion)
                {
                    channelName = L"R";
                }
                else
                {
                    channelName = L"Red";
                }
            }
            break;


            case OA_TEXEL_FORMAT_GREEN:
            {
                if (shortVersion)
                {
                    channelName = L"G";
                }
                else
                {
                    channelName = L"Green";
                }
            }
            break;

            case OA_TEXEL_FORMAT_BLUE:
            {
                if (shortVersion)
                {
                    channelName = L"B";
                }
                else
                {
                    channelName = L"Blue";
                }
            }
            break;

            case OA_TEXEL_FORMAT_ALPHA:
            case OA_TEXEL_FORMAT_ALPHA_COMPRESSED:
            {
                if (shortVersion)
                {
                    channelName = L"A";
                }
                else
                {
                    channelName = L"Alpha";
                }
            }
            break;

            case OA_TEXEL_FORMAT_LUMINANCEALPHA:
            case OA_TEXEL_FORMAT_LUMINANCEALPHA_COMPRESSED:
            {
                if (shortVersion)
                {
                    channelName = L"LA";
                }
                else
                {
                    channelName = L"LuminanceAlpha";
                }

            }
            break;

            case OA_TEXEL_FORMAT_DEPTH:
            case OA_TEXEL_FORMAT_DEPTH_COMPONENT16:
            case OA_TEXEL_FORMAT_DEPTH_COMPONENT24:
            case OA_TEXEL_FORMAT_DEPTH_COMPONENT32:
            case OA_TEXEL_FORMAT_DEPTH_EXT:
            {
                if (shortVersion)
                {
                    channelName = L"DEP";
                }
                else
                {
                    channelName = L"Depth";
                }
            }
            break;

            case OA_TEXEL_FORMAT_STENCIL:
            case OA_TEXEL_FORMAT_STENCIL_INDEX1_EXT:
            case OA_TEXEL_FORMAT_STENCIL_INDEX4_EXT:
            case OA_TEXEL_FORMAT_STENCIL_INDEX8_EXT:
            case OA_TEXEL_FORMAT_STENCIL_INDEX16_EXT:
            {
                if (shortVersion)
                {
                    channelName = L"STN";
                }
                else
                {
                    channelName = L"Stencil";
                }
            }
            break;

            case OA_TEXEL_FORMAT_INTENSITY:
            case OA_TEXEL_FORMAT_INTENSITY_COMPRESSED:
            {
                if (shortVersion)
                {
                    channelName = L"Intens";
                }
                else
                {
                    channelName = L"Intensity";
                }
            }
            break;

            case OA_TEXEL_FORMAT_LUMINANCE:
            case OA_TEXEL_FORMAT_LUMINANCE_COMPRESSED:
            {
                if (shortVersion)
                {
                    channelName = L"LUM";
                }
                else
                {
                    channelName = L"Luminance";
                }
            }
            break;

            case OA_TEXEL_FORMAT_VARIABLE_VALUE:
            {
                // We don't want a header for these:
                channelName = L"Y";
            }
            break;

            case OA_TEXEL_FORMAT_COLORINDEX:
            {
                if (shortVersion)
                {
                    channelName = L"CI";
                }
                else
                {
                    channelName = L"Color Index";
                }
            }
            break;

            case OA_TEXEL_FORMAT_V2F:
            case OA_TEXEL_FORMAT_V3F:
            case OA_TEXEL_FORMAT_C4UB_V2F:
            case OA_TEXEL_FORMAT_C4UB_V3F:
            case OA_TEXEL_FORMAT_C3F_V3F:
            case OA_TEXEL_FORMAT_N3F_V3F:
            case OA_TEXEL_FORMAT_C4F_N3F_V3F:
            case OA_TEXEL_FORMAT_T2F_V3F:
            case OA_TEXEL_FORMAT_T4F_V4F:
            case OA_TEXEL_FORMAT_T2F_C4UB_V3F:
            case OA_TEXEL_FORMAT_T2F_C3F_V3F:
            case OA_TEXEL_FORMAT_T2F_N3F_V3F:
            case OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F:
            case OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F:
            {
                // VBO texel data format:
                // We should not get here with VBOs texel format:
                gtString errString = L"Should not get to this function with a VBO display format";
                errString.appendFormattedString(L": %d", texelDataFormat);
                GT_ASSERT_EX(false, errString.asCharArray());
            }
            break;

            default:
            {
                // Unknown channel name
                channelName = L"N/A";

                // Unknown texel data format:
                retVal = false;
            }
            break;
        }
    }
    else
    {
        retVal = true;

        switch (texelDataFormat)
        {
            case OA_TEXEL_FORMAT_RG:
            case OA_TEXEL_FORMAT_RG_COMPRESSED:
                channelName = L"RG";
                break;

            case OA_TEXEL_FORMAT_RA:
                channelName = L"RA";
                break;

            // TO_DO: Uri, 6/11/08 - we might want to reverse the order of components in the _REVERSED formats.
            case OA_TEXEL_FORMAT_RGB:
            case OA_TEXEL_FORMAT_RGB_REVERSED:
            case OA_TEXEL_FORMAT_RGB_COMPRESSED:
            case OA_TEXEL_FORMAT_RGB_INTEGER:
                channelName = L"RGB";
                break;

            case OA_TEXEL_FORMAT_BGR:
            case OA_TEXEL_FORMAT_BGR_REVERSED:
            case OA_TEXEL_FORMAT_BGR_INTEGER:
                channelName = L"BGR";
                break;

            case OA_TEXEL_FORMAT_BGRA:
            case OA_TEXEL_FORMAT_BGRA_REVERSED:
            case OA_TEXEL_FORMAT_BGRA_INTEGER:
                channelName = L"BGRA";
                break;

            case OA_TEXEL_FORMAT_RGBA:
            case OA_TEXEL_FORMAT_RGBA_REVERSED:
            case OA_TEXEL_FORMAT_RGBA_COMPRESSED:
            case OA_TEXEL_FORMAT_RGBA_INTEGER:
                channelName = L"RGBA";
                break;

            case OA_TEXEL_FORMAT_V2F:
            case OA_TEXEL_FORMAT_V3F:
            case OA_TEXEL_FORMAT_C4UB_V2F:
            case OA_TEXEL_FORMAT_C4UB_V3F:
            case OA_TEXEL_FORMAT_C3F_V3F:
            case OA_TEXEL_FORMAT_N3F_V3F:
            case OA_TEXEL_FORMAT_C4F_N3F_V3F:
            case OA_TEXEL_FORMAT_T2F_V3F:
            case OA_TEXEL_FORMAT_T4F_V4F:
            case OA_TEXEL_FORMAT_T2F_C4UB_V3F:
            case OA_TEXEL_FORMAT_T2F_C3F_V3F:
            case OA_TEXEL_FORMAT_T2F_N3F_V3F:
            case OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F:
            case OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F:
            {
                // VBO texel data format:
                // We should not get here with VBOs texel format:
                gtString errString = L"Should not get to this function with a VBO display format";
                errString.appendFormattedString(L": %d", texelDataFormat);
                GT_ASSERT_EX(false, errString.asCharArray());
            }
            break;

            case OA_TEXEL_FORMAT_NORMAL:
                channelName = L"NOR";
                break;

            case OA_TEXEL_FORMAT_COLOR:
                channelName = L"COL";
                break;

            case OA_TEXEL_FORMAT_VECTOR:
                channelName = L"VEC";
                break;

            case OA_TEXEL_FORMAT_TEXTURE:
                channelName = L"TEX";
                break;

            case OA_TEXEL_FORMAT_INDEX:
                channelName = L"IND";
                break;

            case OA_TEXEL_FORMAT_CHAR:
                channelName = L"char";
                break;

            case OA_TEXEL_FORMAT_UCHAR:
                channelName = L"uchar";
                break;

            case OA_TEXEL_FORMAT_SHORT:
                channelName = L"short";
                break;

            case OA_TEXEL_FORMAT_USHORT:
                channelName = L"ushort";
                break;

            case OA_TEXEL_FORMAT_LONG:
                channelName = L"long";
                break;

            case OA_TEXEL_FORMAT_ULONG:
                channelName = L"ulong";
                break;

            case OA_TEXEL_FORMAT_FLOAT:
                channelName = L"float";
                break;

            case OA_TEXEL_FORMAT_DOUBLE:
                channelName = L"double";
                break;


            default:
            {
                // Unknown channel name
                channelName = L"N/A";

                // Unknown texel data format:
                gtString errString = L"Unknown texel data type";
                errString.appendFormattedString(L": %d", texelDataFormat);
                GT_ASSERT_EX(false, errString.asCharArray());

                retVal = false;
            }
            break;
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        oaGetTexelDataBufferFormatName
// Description: Returns the channel name, given a channel type and a component index
// Arguments:   texelDataFormat - The channel type
//              channelName - Output channel name
//              componentIndex - The index of the requested component
// Return Val:  bool - Success / Failure
// Author:      AMD Developer Tools Team
// Date:        19/4/2009
// ---------------------------------------------------------------------------
bool oaGetTexelDataBufferFormatName(oaTexelDataFormat texelDataFormat, gtString& channelName, int componentIndex)
{
    bool retVal = true;

    // Empty output string
    channelName.makeEmpty();

    switch (texelDataFormat)
    {
        case OA_TEXEL_FORMAT_NORMAL:
        {
            switch (componentIndex)
            {
                case 0:
                    channelName = L"Nx";
                    break;

                case 1:
                    channelName = L"Ny";
                    break;

                case 2:
                    channelName = L"Nz";
                    break;

                default:
                    retVal = false;
                    break;
            }
        }
        break;

        case OA_TEXEL_FORMAT_COLOR:
        {
            switch (componentIndex)
            {
                case 0:
                    channelName = L"R";
                    break;

                case 1:
                    channelName = L"G";
                    break;

                case 2:
                    channelName = L"B";
                    break;

                case 3:
                    channelName = L"A";
                    break;

                default:
                    retVal = false;
                    break;
            }
        }
        break;

        case OA_TEXEL_FORMAT_VECTOR:
        {
            switch (componentIndex)
            {
                case 0:
                    channelName = L"X";
                    break;

                case 1:
                    channelName = L"Y";
                    break;

                case 2:
                    channelName = L"Z";
                    break;

                case 3:
                    channelName = L"W";
                    break;

                default:
                    retVal = false;
                    break;
            }
        }
        break;

        case OA_TEXEL_FORMAT_TEXTURE:
        {
            switch (componentIndex)
            {
                case 0:
                    channelName = L"S";
                    break;

                case 1:
                    channelName = L"T";
                    break;

                case 2:
                    channelName = L"R";
                    break;

                case 3:
                    channelName = L"Q";
                    break;

                default:
                    retVal = false;
                    break;
            }
        }
        break;

        case OA_TEXEL_FORMAT_INDEX:
        {
            switch (componentIndex)
            {
                case 0:
                    channelName = L"I";
                    break;
            }

            break;
        }

        case OA_TEXEL_FORMAT_CHAR:
        {
            channelName = L"c";
        }
        break;

        case OA_TEXEL_FORMAT_UCHAR:
        {
            channelName = L"uc";
        }
        break;

        case OA_TEXEL_FORMAT_SHORT:
        {
            channelName = L"s";
        }
        break;

        case OA_TEXEL_FORMAT_USHORT:
        {
            channelName = L"us";
        }
        break;


        case OA_TEXEL_FORMAT_LONG:
        {
            channelName = L"l";
        }
        break;

        case OA_TEXEL_FORMAT_ULONG:
        {
            channelName = L"ul";
        }
        break;


        case OA_TEXEL_FORMAT_INT:
        {
            channelName = L"i";
        }
        break;

        case OA_TEXEL_FORMAT_UINT:
        {
            channelName = L"ui";
        }
        break;

        case OA_TEXEL_FORMAT_FLOAT:
        {
            channelName = L"f";
        }
        break;

        case OA_TEXEL_FORMAT_DOUBLE:
        {
            channelName = L"d";
        }
        break;

        default:
        {
            // Unknown channel name
            channelName = L"N/A";

            // Unknown texel data format:
            gtString errString = L"Unknown texel data type";
            errString.appendFormattedString(L": %d", texelDataFormat);
            GT_ASSERT_EX(false, errString.asCharArray());

            retVal = false;
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaGetTexelFormatComponentType
// Description: Get the component type from the texel format in a specific index.
//
//              For example:
//              OA_TEXEL_FORMAT_BLUE will be the return val of calling the
//              oaGetTexelFormatComponentType(OA_TEXEL_FORMAT_RGBA, 2)
//              function, because blue is the third component of RGBA.
//
// Arguments:   texelDataFormat - The channel type
//              componentIndex - The index of the component to retrieve
// Return Val:  The channel type in the specific requested index.
//              OA_TEXEL_FORMAT_UNKNOWN if an error occurs.
// Author:      AMD Developer Tools Team
// Date:        3/9/2007
// ---------------------------------------------------------------------------
oaTexelDataFormat OA_API oaGetTexelFormatComponentType(oaTexelDataFormat texelDataFormat, int componentIndex)
{
    oaTexelDataFormat componentType = OA_TEXEL_FORMAT_UNKNOWN;

    switch (texelDataFormat)
    {
        case OA_TEXEL_FORMAT_RGB:
        case OA_TEXEL_FORMAT_BGR_REVERSED:
        case OA_TEXEL_FORMAT_RGB_COMPRESSED:
        case OA_TEXEL_FORMAT_RGB_INTEGER:
            switch (componentIndex)
            {
                case 0:
                    componentType = OA_TEXEL_FORMAT_RED;
                    break;

                case 1:
                    componentType = OA_TEXEL_FORMAT_GREEN;
                    break;

                case 2:
                    componentType = OA_TEXEL_FORMAT_BLUE;
                    break;

                default:
                    // Do nothing...
                    break;
            }

            break;

        case OA_TEXEL_FORMAT_RGBA:
        case OA_TEXEL_FORMAT_RGBA_COMPRESSED:
        case OA_TEXEL_FORMAT_RGBA_INTEGER:
        {
            switch (componentIndex)
            {
                case 0:
                    componentType = OA_TEXEL_FORMAT_RED;
                    break;

                case 1:
                    componentType = OA_TEXEL_FORMAT_GREEN;
                    break;

                case 2:
                    componentType = OA_TEXEL_FORMAT_BLUE;
                    break;

                case 3:
                    componentType = OA_TEXEL_FORMAT_ALPHA;
                    break;

                default:
                    // Do nothing...
                    break;
            }
        }
        break;

        case OA_TEXEL_FORMAT_BGR:
        case OA_TEXEL_FORMAT_BGR_INTEGER:
        case OA_TEXEL_FORMAT_RGB_REVERSED:
            switch (componentIndex)
            {
                case 0:
                    componentType = OA_TEXEL_FORMAT_BLUE;
                    break;

                case 1:
                    componentType = OA_TEXEL_FORMAT_GREEN;
                    break;

                case 2:
                    componentType = OA_TEXEL_FORMAT_RED;
                    break;

                default:
                    // Do nothing...
                    break;
            }

            break;


        case OA_TEXEL_FORMAT_BGRA:
        case OA_TEXEL_FORMAT_BGRA_INTEGER:
        {
            switch (componentIndex)
            {
                case 0:
                    componentType = OA_TEXEL_FORMAT_BLUE;
                    break;

                case 1:
                    componentType = OA_TEXEL_FORMAT_GREEN;
                    break;

                case 2:
                    componentType = OA_TEXEL_FORMAT_RED;
                    break;

                case 3:
                    componentType = OA_TEXEL_FORMAT_ALPHA;
                    break;

                default:
                    // Do nothing...
                    break;
            }
        }
        break;

        case OA_TEXEL_FORMAT_RGBA_REVERSED:
        {
            switch (componentIndex)
            {
                case 0:
                    componentType = OA_TEXEL_FORMAT_ALPHA;
                    break;

                case 1:
                    componentType = OA_TEXEL_FORMAT_BLUE;
                    break;

                case 2:
                    componentType = OA_TEXEL_FORMAT_GREEN;
                    break;

                case 3:
                    componentType = OA_TEXEL_FORMAT_RED;
                    break;

                default:
                    // Do nothing...
                    break;
            }
        }
        break;

        case OA_TEXEL_FORMAT_BGRA_REVERSED:
        {
            switch (componentIndex)
            {
                case 0:
                    componentType = OA_TEXEL_FORMAT_ALPHA;
                    break;

                case 1:
                    componentType = OA_TEXEL_FORMAT_RED;
                    break;

                case 2:
                    componentType = OA_TEXEL_FORMAT_GREEN;
                    break;

                case 3:
                    componentType = OA_TEXEL_FORMAT_BLUE;
                    break;

                default:
                    // Do nothing...
                    break;
            }
        }
        break;

        case OA_TEXEL_FORMAT_LUMINANCEALPHA:
        case OA_TEXEL_FORMAT_LUMINANCEALPHA_COMPRESSED:
        {
            switch (componentIndex)
            {
                case 0:
                    componentType = OA_TEXEL_FORMAT_LUMINANCE;
                    break;

                case 1:
                    componentType = OA_TEXEL_FORMAT_ALPHA;
                    break;

                default:
                    // Do nothing...
                    break;
            }
        }
        break;

        case OA_TEXEL_FORMAT_RG:
        case OA_TEXEL_FORMAT_RG_COMPRESSED:
        {
            switch (componentIndex)
            {
                case 0:
                    componentType = OA_TEXEL_FORMAT_RED;
                    break;

                case 1:
                    componentType = OA_TEXEL_FORMAT_GREEN;
                    break;

                default:
                    // Do nothing...
                    break;
            }
        }
        break;

        case OA_TEXEL_FORMAT_RA:
        {
            switch (componentIndex)
            {
                case 0:
                    componentType = OA_TEXEL_FORMAT_RED;
                    break;

                case 1:
                    componentType = OA_TEXEL_FORMAT_ALPHA;
                    break;

                default:
                    // Do nothing...
                    break;
            }
        }
        break;

        case OA_TEXEL_FORMAT_RED:
        case OA_TEXEL_FORMAT_RED_COMPRESSED:
        {
            // Make sure we asked for the first component
            if (componentIndex == 0)
            {
                // Return the basic data format
                componentType = OA_TEXEL_FORMAT_RED;
            }
        }
        break;

        case OA_TEXEL_FORMAT_ALPHA:
        case OA_TEXEL_FORMAT_ALPHA_COMPRESSED:
        {
            // Make sure we asked for the first component
            if (componentIndex == 0)
            {
                // Return the basic data format
                componentType = OA_TEXEL_FORMAT_ALPHA;
            }
        }
        break;

        case OA_TEXEL_FORMAT_GREEN:
        case OA_TEXEL_FORMAT_BLUE:
        case OA_TEXEL_FORMAT_COLORINDEX:
        case OA_TEXEL_FORMAT_VARIABLE_VALUE:
        {
            // Make sure we asked for the first component
            if (componentIndex == 0)
            {
                // Just return this data format
                componentType = texelDataFormat;
            }
        }
        break;

        case OA_TEXEL_FORMAT_LUMINANCE:
        case OA_TEXEL_FORMAT_LUMINANCE_COMPRESSED:
        {
            // Make sure we asked for the first component
            if (componentIndex == 0)
            {
                // Return the basic data format
                componentType = OA_TEXEL_FORMAT_LUMINANCE;
            }
        }
        break;

        case OA_TEXEL_FORMAT_STENCIL:
        case OA_TEXEL_FORMAT_STENCIL_INDEX1_EXT:
        case OA_TEXEL_FORMAT_STENCIL_INDEX4_EXT:
        case OA_TEXEL_FORMAT_STENCIL_INDEX8_EXT:
        case OA_TEXEL_FORMAT_STENCIL_INDEX16_EXT:
        {
            // Make sure we asked for the first component
            if (componentIndex == 0)
            {
                // Return the basic data format
                componentType = OA_TEXEL_FORMAT_STENCIL;
            }
        }
        break;

        case OA_TEXEL_FORMAT_DEPTH:
        case OA_TEXEL_FORMAT_DEPTH_EXT:
        case OA_TEXEL_FORMAT_DEPTH_COMPONENT16:
        case OA_TEXEL_FORMAT_DEPTH_COMPONENT24:
        case OA_TEXEL_FORMAT_DEPTH_COMPONENT32:
        {
            // Make sure we asked for the first component
            if (componentIndex == 0)
            {
                // Return the basic data format
                componentType = OA_TEXEL_FORMAT_DEPTH;
            }
        }
        break;

        case OA_TEXEL_FORMAT_V1F:
        case OA_TEXEL_FORMAT_V2F:
        case OA_TEXEL_FORMAT_V3F:
        case OA_TEXEL_FORMAT_V4F:
        case OA_TEXEL_FORMAT_V1D:
        case OA_TEXEL_FORMAT_V2D:
        case OA_TEXEL_FORMAT_V3D:
        case OA_TEXEL_FORMAT_V4D:
        case OA_TEXEL_FORMAT_V1S:
        case OA_TEXEL_FORMAT_V2S:
        case OA_TEXEL_FORMAT_V3S:
        case OA_TEXEL_FORMAT_V4S:
        case OA_TEXEL_FORMAT_V1B:
        case OA_TEXEL_FORMAT_V2B:
        case OA_TEXEL_FORMAT_V3B:
        case OA_TEXEL_FORMAT_V4B:
        case OA_TEXEL_FORMAT_V1US:
        case OA_TEXEL_FORMAT_V2US:
        case OA_TEXEL_FORMAT_V3US:
        case OA_TEXEL_FORMAT_V4US:
        case OA_TEXEL_FORMAT_V1UB:
        case OA_TEXEL_FORMAT_V2UB:
        case OA_TEXEL_FORMAT_V3UB:
        case OA_TEXEL_FORMAT_V4UB:
        case OA_TEXEL_FORMAT_V1I:
        case OA_TEXEL_FORMAT_V2I:
        case OA_TEXEL_FORMAT_V3I:
        case OA_TEXEL_FORMAT_V4I:
        case OA_TEXEL_FORMAT_V1UI:
        case OA_TEXEL_FORMAT_V2UI:
        case OA_TEXEL_FORMAT_V3UI:
        case OA_TEXEL_FORMAT_V4UI:
        {
            componentType = OA_TEXEL_FORMAT_VECTOR;
        }
        break;

        case OA_TEXEL_FORMAT_C3F:
        case OA_TEXEL_FORMAT_C4F:
        case OA_TEXEL_FORMAT_C3D:
        case OA_TEXEL_FORMAT_C4D:
        case OA_TEXEL_FORMAT_C3S:
        case OA_TEXEL_FORMAT_C4S:
        case OA_TEXEL_FORMAT_C3US:
        case OA_TEXEL_FORMAT_C4US:
        case OA_TEXEL_FORMAT_C3B:
        case OA_TEXEL_FORMAT_C4B:
        case OA_TEXEL_FORMAT_C3UB:
        case OA_TEXEL_FORMAT_C4UB:
        case OA_TEXEL_FORMAT_C3I:
        case OA_TEXEL_FORMAT_C4I:
        case OA_TEXEL_FORMAT_C3UI:
        case OA_TEXEL_FORMAT_C4UI:
        {
            componentType = OA_TEXEL_FORMAT_COLOR;
        }
        break;

        case OA_TEXEL_FORMAT_N3F:
        case OA_TEXEL_FORMAT_N3D:
        case OA_TEXEL_FORMAT_N3I:
        case OA_TEXEL_FORMAT_N3S:
        case OA_TEXEL_FORMAT_N3B:
        {
            componentType = OA_TEXEL_FORMAT_NORMAL;
        }
        break;

        case OA_TEXEL_FORMAT_T1F:
        case OA_TEXEL_FORMAT_T2F:
        case OA_TEXEL_FORMAT_T3F:
        case OA_TEXEL_FORMAT_T4F:
        case OA_TEXEL_FORMAT_T1D:
        case OA_TEXEL_FORMAT_T2D:
        case OA_TEXEL_FORMAT_T3D:
        case OA_TEXEL_FORMAT_T4D:
        case OA_TEXEL_FORMAT_T1S:
        case OA_TEXEL_FORMAT_T2S:
        case OA_TEXEL_FORMAT_T3S:
        case OA_TEXEL_FORMAT_T4S:
        case OA_TEXEL_FORMAT_T1I:
        case OA_TEXEL_FORMAT_T2I:
        case OA_TEXEL_FORMAT_T3I:
        case OA_TEXEL_FORMAT_T4I:
        {
            componentType = OA_TEXEL_FORMAT_TEXTURE;
        }
        break;

        case OA_TEXEL_FORMAT_I1UI:
        case OA_TEXEL_FORMAT_I1UB:
        case OA_TEXEL_FORMAT_I1US:
        case OA_TEXEL_FORMAT_I1S:
        case OA_TEXEL_FORMAT_I1I:
        case OA_TEXEL_FORMAT_I1F:
        case OA_TEXEL_FORMAT_I1D:
        {
            componentType = OA_TEXEL_FORMAT_INDEX;
        }
        break;

        case OA_TEXEL_FORMAT_C4UB_V2F:
        case OA_TEXEL_FORMAT_C4UB_V3F:
        {
            if (componentIndex >= 3)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
            }
            else
            {
                componentType = OA_TEXEL_FORMAT_COLOR;
            }
        }
        break;

        case OA_TEXEL_FORMAT_C3F_V3F:
        case OA_TEXEL_FORMAT_N3F_V3F:
        {
            if (componentIndex >= 3)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
            }
            else
            {
                if (texelDataFormat == OA_TEXEL_FORMAT_N3F_V3F)
                {
                    componentType = OA_TEXEL_FORMAT_NORMAL;
                }
                else
                {
                    componentType = OA_TEXEL_FORMAT_COLOR;
                }
            }
        }
        break;

        case OA_TEXEL_FORMAT_T2F_V3F:
        {
            if (componentIndex <= 1)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
            }
        }
        break;

        case OA_TEXEL_FORMAT_T2F_C4UB_V3F:
        {
            if (componentIndex <= 1)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else if (componentIndex <= 5)
            {
                componentType = OA_TEXEL_FORMAT_COLOR;
            }
            else if (componentIndex <= 8)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
            }
        }
        break;

        case OA_TEXEL_FORMAT_T2F_C3F_V3F:
        {
            if (componentIndex <= 1)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else if (componentIndex <= 4)
            {
                componentType = OA_TEXEL_FORMAT_COLOR;
            }
            else if (componentIndex <= 8)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
            }
        }
        break;

        case OA_TEXEL_FORMAT_T2F_N3F_V3F:
        {
            if (componentIndex <= 1)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else if (componentIndex <= 4)
            {
                componentType = OA_TEXEL_FORMAT_NORMAL;
            }
            else if (componentIndex <= 7)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
            }
        }
        break;

        case OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F:
        {
            if (componentIndex <= 1)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else if (componentIndex <= 5)
            {
                componentType = OA_TEXEL_FORMAT_COLOR;
            }
            else if (componentIndex <= 8)
            {
                componentType = OA_TEXEL_FORMAT_NORMAL;
            }
            else if (componentIndex <= 11)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
            }
        }
        break;

        case OA_TEXEL_FORMAT_C4F_N3F_V3F:
        {
            if (componentIndex <= 3)
            {
                componentType = OA_TEXEL_FORMAT_COLOR;
            }
            else if (componentIndex <= 6)
            {
                componentType = OA_TEXEL_FORMAT_NORMAL;
            }
            else if (componentIndex <= 9)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
            }
        }
        break;

        case OA_TEXEL_FORMAT_T4F_V4F:
        {
            if (componentIndex <= 3)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else if (componentIndex <= 7)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
            }
        }
        break;

        case OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F:
        {
            if (componentIndex <= 3)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else if (componentIndex <= 7)
            {
                componentType = OA_TEXEL_FORMAT_COLOR;
            }
            else if (componentIndex <= 10)
            {
                componentType = OA_TEXEL_FORMAT_NORMAL;
            }
            else if (componentIndex <= 14)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
            }
        }
        break;

        case OA_TEXEL_FORMAT_CHAR2:
        case OA_TEXEL_FORMAT_CHAR3:
        case OA_TEXEL_FORMAT_CHAR4:
        case OA_TEXEL_FORMAT_CHAR8:
        case OA_TEXEL_FORMAT_CHAR16:
        {
            componentType = OA_TEXEL_FORMAT_CHAR;
        }
        break;

        case OA_TEXEL_FORMAT_UCHAR2:
        case OA_TEXEL_FORMAT_UCHAR3:
        case OA_TEXEL_FORMAT_UCHAR4:
        case OA_TEXEL_FORMAT_UCHAR8:
        case OA_TEXEL_FORMAT_UCHAR16:
        {
            componentType = OA_TEXEL_FORMAT_UCHAR;
        }
        break;

        case OA_TEXEL_FORMAT_SHORT2:
        case OA_TEXEL_FORMAT_SHORT3:
        case OA_TEXEL_FORMAT_SHORT4:
        case OA_TEXEL_FORMAT_SHORT8:
        case OA_TEXEL_FORMAT_SHORT16:
        {
            componentType = OA_TEXEL_FORMAT_SHORT;
        }
        break;

        case OA_TEXEL_FORMAT_USHORT2:
        case OA_TEXEL_FORMAT_USHORT3:
        case OA_TEXEL_FORMAT_USHORT4:
        case OA_TEXEL_FORMAT_USHORT8:
        case OA_TEXEL_FORMAT_USHORT16:
        {
            componentType = OA_TEXEL_FORMAT_USHORT;
        }
        break;

        case OA_TEXEL_FORMAT_INT2:
        case OA_TEXEL_FORMAT_INT3:
        case OA_TEXEL_FORMAT_INT4:
        case OA_TEXEL_FORMAT_INT8:
        case OA_TEXEL_FORMAT_INT16:
        {
            componentType = OA_TEXEL_FORMAT_INT;
        }
        break;

        case OA_TEXEL_FORMAT_UINT2:
        case OA_TEXEL_FORMAT_UINT3:
        case OA_TEXEL_FORMAT_UINT4:
        case OA_TEXEL_FORMAT_UINT8:
        case OA_TEXEL_FORMAT_UINT16:
        {
            componentType = OA_TEXEL_FORMAT_UINT;
        }
        break;

        case OA_TEXEL_FORMAT_LONG2:
        case OA_TEXEL_FORMAT_LONG3:
        case OA_TEXEL_FORMAT_LONG4:
        case OA_TEXEL_FORMAT_LONG8:
        case OA_TEXEL_FORMAT_LONG16:
        {
            componentType = OA_TEXEL_FORMAT_LONG;
        }
        break;

        case OA_TEXEL_FORMAT_ULONG2:
        case OA_TEXEL_FORMAT_ULONG3:
        case OA_TEXEL_FORMAT_ULONG4:
        case OA_TEXEL_FORMAT_ULONG8:
        case OA_TEXEL_FORMAT_ULONG16:
        {
            componentType = OA_TEXEL_FORMAT_ULONG;
        }
        break;

        case OA_TEXEL_FORMAT_FLOAT2:
        case OA_TEXEL_FORMAT_FLOAT3:
        case OA_TEXEL_FORMAT_FLOAT4:
        case OA_TEXEL_FORMAT_FLOAT8:
        case OA_TEXEL_FORMAT_FLOAT16:
        {
            componentType = OA_TEXEL_FORMAT_FLOAT;
        }
        break;

        case OA_TEXEL_FORMAT_DOUBLE2:
        case OA_TEXEL_FORMAT_DOUBLE3:
        case OA_TEXEL_FORMAT_DOUBLE4:
        case OA_TEXEL_FORMAT_DOUBLE8:
        case OA_TEXEL_FORMAT_DOUBLE16:
        {
            componentType = OA_TEXEL_FORMAT_DOUBLE;
        }
        break;

        default:
        {
            // This is wrong, we want to get component type of a specific index and
            // we didn't handle it yet - return an error
            GT_ASSERT_EX(false, L"Unknown Texel format type");
        }
        break;
    }

    return componentType;
}


// ---------------------------------------------------------------------------
// Name:        oaGetTexelFormatComponentType
// Description: Get the data type from the texel format in a specific index.
// Arguments:   texelDataFormat - The channel type
//              componentIndex - The index of the component to retrieve
// Return Val:  The data type for the specific requested index.
//              OA_UNKNOWN_DATA_TYPE if an error occurs.
// Author:      AMD Developer Tools Team
// Date:        16/4/2009
// ---------------------------------------------------------------------------
oaDataType OA_API oaGetTexelFormatDataType(oaTexelDataFormat  texelDataFormat, int componentIndex)
{
    oaDataType dataType = OA_UNKNOWN_DATA_TYPE;

    switch (texelDataFormat)
    {

        case OA_TEXEL_FORMAT_CHAR:
        case OA_TEXEL_FORMAT_CHAR2:
        case OA_TEXEL_FORMAT_CHAR3:
        case OA_TEXEL_FORMAT_CHAR4:
        case OA_TEXEL_FORMAT_CHAR8:
        case OA_TEXEL_FORMAT_CHAR16:
        {
            dataType = OA_CHAR;
        }
        break;

        case OA_TEXEL_FORMAT_UCHAR:
        case OA_TEXEL_FORMAT_UCHAR2:
        case OA_TEXEL_FORMAT_UCHAR3:
        case OA_TEXEL_FORMAT_UCHAR4:
        case OA_TEXEL_FORMAT_UCHAR8:
        case OA_TEXEL_FORMAT_UCHAR16:
        {
            dataType = OA_UNSIGNED_CHAR;
        }
        break;

        case OA_TEXEL_FORMAT_I1F:
        case OA_TEXEL_FORMAT_C3F_V3F:
        case OA_TEXEL_FORMAT_N3F_V3F:
        case OA_TEXEL_FORMAT_T2F_V3F:
        case OA_TEXEL_FORMAT_T2F_C3F_V3F:
        case OA_TEXEL_FORMAT_T2F_N3F_V3F:
        case OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F:
        case OA_TEXEL_FORMAT_C4F_N3F_V3F:
        case OA_TEXEL_FORMAT_T4F_V4F:
        case OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F:
        case OA_TEXEL_FORMAT_V1F:
        case OA_TEXEL_FORMAT_V2F:
        case OA_TEXEL_FORMAT_V3F:
        case OA_TEXEL_FORMAT_V4F:
        case OA_TEXEL_FORMAT_C3F:
        case OA_TEXEL_FORMAT_C4F:
        case OA_TEXEL_FORMAT_N3F:
        case OA_TEXEL_FORMAT_T1F:
        case OA_TEXEL_FORMAT_T2F:
        case OA_TEXEL_FORMAT_T3F:
        case OA_TEXEL_FORMAT_T4F:
        case OA_TEXEL_FORMAT_FLOAT:
        case OA_TEXEL_FORMAT_FLOAT2:
        case OA_TEXEL_FORMAT_FLOAT3:
        case OA_TEXEL_FORMAT_FLOAT4:
        case OA_TEXEL_FORMAT_FLOAT8:
        case OA_TEXEL_FORMAT_FLOAT16:
        {
            dataType = OA_FLOAT;
        }
        break;

        case OA_TEXEL_FORMAT_I1D:
        case OA_TEXEL_FORMAT_V1D:
        case OA_TEXEL_FORMAT_V2D:
        case OA_TEXEL_FORMAT_V3D:
        case OA_TEXEL_FORMAT_V4D:
        case OA_TEXEL_FORMAT_C3D:
        case OA_TEXEL_FORMAT_C4D:
        case OA_TEXEL_FORMAT_N3D:
        case OA_TEXEL_FORMAT_T1D:
        case OA_TEXEL_FORMAT_T2D:
        case OA_TEXEL_FORMAT_T3D:
        case OA_TEXEL_FORMAT_T4D:
        case OA_TEXEL_FORMAT_DOUBLE:
        case OA_TEXEL_FORMAT_DOUBLE2:
        case OA_TEXEL_FORMAT_DOUBLE3:
        case OA_TEXEL_FORMAT_DOUBLE4:
        case OA_TEXEL_FORMAT_DOUBLE8:
        case OA_TEXEL_FORMAT_DOUBLE16:
        {
            dataType = OA_DOUBLE;
        }
        break;

        case OA_TEXEL_FORMAT_I1S:
        case OA_TEXEL_FORMAT_V1S:
        case OA_TEXEL_FORMAT_V2S:
        case OA_TEXEL_FORMAT_V3S:
        case OA_TEXEL_FORMAT_V4S:
        case OA_TEXEL_FORMAT_C3S:
        case OA_TEXEL_FORMAT_C4S:
        case OA_TEXEL_FORMAT_N3S:
        case OA_TEXEL_FORMAT_T1S:
        case OA_TEXEL_FORMAT_T2S:
        case OA_TEXEL_FORMAT_T3S:
        case OA_TEXEL_FORMAT_T4S:
        case OA_TEXEL_FORMAT_SHORT:
        case OA_TEXEL_FORMAT_SHORT2:
        case OA_TEXEL_FORMAT_SHORT3:
        case OA_TEXEL_FORMAT_SHORT4:
        case OA_TEXEL_FORMAT_SHORT8:
        case OA_TEXEL_FORMAT_SHORT16:
        {
            dataType = OA_SHORT;
        }
        break;

        case OA_TEXEL_FORMAT_V1I:
        case OA_TEXEL_FORMAT_V2I:
        case OA_TEXEL_FORMAT_V3I:
        case OA_TEXEL_FORMAT_V4I:
        case OA_TEXEL_FORMAT_C3I:
        case OA_TEXEL_FORMAT_C4I:
        case OA_TEXEL_FORMAT_N3I:
        case OA_TEXEL_FORMAT_T1I:
        case OA_TEXEL_FORMAT_T2I:
        case OA_TEXEL_FORMAT_T3I:
        case OA_TEXEL_FORMAT_T4I:
        case OA_TEXEL_FORMAT_I1I:
        case OA_TEXEL_FORMAT_INT:
        case OA_TEXEL_FORMAT_INT2:
        case OA_TEXEL_FORMAT_INT3:
        case OA_TEXEL_FORMAT_INT4:
        case OA_TEXEL_FORMAT_INT8:
        case OA_TEXEL_FORMAT_INT16:
        {
            dataType = OA_INT;
        }
        break;

        case OA_TEXEL_FORMAT_I1UI:
        case OA_TEXEL_FORMAT_V1UI:
        case OA_TEXEL_FORMAT_V2UI:
        case OA_TEXEL_FORMAT_V3UI:
        case OA_TEXEL_FORMAT_V4UI:
        case OA_TEXEL_FORMAT_C3UI:
        case OA_TEXEL_FORMAT_C4UI:
        case OA_TEXEL_FORMAT_UINT:
        case OA_TEXEL_FORMAT_UINT2:
        case OA_TEXEL_FORMAT_UINT3:
        case OA_TEXEL_FORMAT_UINT4:
        case OA_TEXEL_FORMAT_UINT8:
        case OA_TEXEL_FORMAT_UINT16:
        {
            dataType = OA_UNSIGNED_INT;
        }
        break;

        case OA_TEXEL_FORMAT_I1US:
        case OA_TEXEL_FORMAT_V1US:
        case OA_TEXEL_FORMAT_V2US:
        case OA_TEXEL_FORMAT_V3US:
        case OA_TEXEL_FORMAT_V4US:
        case OA_TEXEL_FORMAT_C3US:
        case OA_TEXEL_FORMAT_C4US:
        case OA_TEXEL_FORMAT_USHORT:
        case OA_TEXEL_FORMAT_USHORT2:
        case OA_TEXEL_FORMAT_USHORT3:
        case OA_TEXEL_FORMAT_USHORT4:
        case OA_TEXEL_FORMAT_USHORT8:
        case OA_TEXEL_FORMAT_USHORT16:
        {
            dataType = OA_UNSIGNED_SHORT;
        }
        break;

        case OA_TEXEL_FORMAT_LONG:
        case OA_TEXEL_FORMAT_LONG2:
        case OA_TEXEL_FORMAT_LONG3:
        case OA_TEXEL_FORMAT_LONG4:
        case OA_TEXEL_FORMAT_LONG8:
        case OA_TEXEL_FORMAT_LONG16:
        {
            dataType = OA_LONG;
        }
        break;

        case OA_TEXEL_FORMAT_ULONG:
        case OA_TEXEL_FORMAT_ULONG2:
        case OA_TEXEL_FORMAT_ULONG3:
        case OA_TEXEL_FORMAT_ULONG4:
        case OA_TEXEL_FORMAT_ULONG8:
        case OA_TEXEL_FORMAT_ULONG16:
        {
            dataType = OA_UNSIGNED_LONG;
        }
        break;

        case OA_TEXEL_FORMAT_C3B:
        case OA_TEXEL_FORMAT_C4B:
        case OA_TEXEL_FORMAT_N3B:
        case OA_TEXEL_FORMAT_V1B:
        case OA_TEXEL_FORMAT_V2B:
        case OA_TEXEL_FORMAT_V3B:
        case OA_TEXEL_FORMAT_V4B:
        {
            dataType = OA_BYTE;
        }
        break;

        case OA_TEXEL_FORMAT_I1UB:
        case OA_TEXEL_FORMAT_C3UB:
        case OA_TEXEL_FORMAT_C4UB:
        case OA_TEXEL_FORMAT_V1UB:
        case OA_TEXEL_FORMAT_V2UB:
        case OA_TEXEL_FORMAT_V3UB:
        case OA_TEXEL_FORMAT_V4UB:
        {
            dataType = OA_UNSIGNED_BYTE;
        }
        break;

        case OA_TEXEL_FORMAT_C4UB_V2F:
        case OA_TEXEL_FORMAT_C4UB_V3F:
        {
            if (componentIndex >= 3)
            {
                dataType = OA_FLOAT;
            }
            else
            {
                dataType = OA_BYTE;
            }
        }
        break;

        case OA_TEXEL_FORMAT_T2F_C4UB_V3F:
        {
            if ((componentIndex <= 1) || (componentIndex <= 8))
            {
                dataType = OA_FLOAT;
            }
            else if (componentIndex <= 5)
            {
                dataType = OA_UNSIGNED_BYTE;
            }
        }
        break;

        default:
        {
            // This is wrong, we want to get component type of a specific index and
            // we didn't handle it yet - return an error
            GT_ASSERT_EX(false, L"Unknown Texel format data  type");
        }
        break;
    }

    return dataType;
}


// ---------------------------------------------------------------------------
// Name:        oaCalculatePixelUnitByteSize
// Description: Calculate the size of a pixel unit, given a specific data
//              format and data type
// Return Val:  Size of raw data pixel unit size if successful,
//              -1 if failed.
// Author:      AMD Developer Tools Team
// Date:        13/12/2007
// ---------------------------------------------------------------------------
int oaCalculatePixelUnitByteSize(oaTexelDataFormat dataFormat, oaDataType dataType)
{
    int retVal = -1;

    // We have two cases which we need to calculate pixel size for:
    // ------------------------------------------------------------
    // 1. When data type is either byte, int or float then the pixel size
    //    is simply data type size * number of components.
    // 2. When data type is an array of bits containing either RGB or RGBA components.
    //    Then the size is the size of the bits array.

    // Get amount of components inside the data type
    int amountOfDataTypeComponents = oaAmountComponentsInDataType(dataType);
    GT_IF_WITH_ASSERT(amountOfDataTypeComponents != -1)
    {
        // One component data type, for example: "UByte", "Integer", "Float", etc...
        if (amountOfDataTypeComponents == 1)
        {
            // Get a single data component size:
            int itemDataSize = oaSizeOfDataType(dataType);
            GT_IF_WITH_ASSERT(itemDataSize != -1)
            {
                // Get amount of components per pixel
                int amountOfComponents = oaAmountOfTexelFormatComponents(dataFormat);
                GT_IF_WITH_ASSERT(amountOfComponents != -1)
                {
                    // Calculate source pixel size, which is size is data type size * number of components.
                    retVal = amountOfComponents * itemDataSize;
                }
            }
        }
        // Multiple component data type, for example: "UInt 10_10_10_2", "UByte 2_2_4", etc...
        else
        {
            // Get a data type component size:
            int itemDataSize = oaSizeOfDataType(dataType);
            GT_IF_WITH_ASSERT(itemDataSize != -1)
            {
                // Return source pixel size
                retVal = itemDataSize;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaCalculateChunkByteSize
// Description: Calculate the size of single chunk for a VBO texel format:
//              NOTICE: this function is only handling VBO formats
// Arguments: oaTexelDataFormat texelDataFormat
// Return Val: int OA_API
// Author:      AMD Developer Tools Team
// Date:        19/4/2009
// ---------------------------------------------------------------------------
int OA_API oaCalculateChunkByteSize(oaTexelDataFormat texelDataFormat)
{
    int chunkSize = 0;

    switch (texelDataFormat)
    {

        case OA_TEXEL_FORMAT_I1D:
        case OA_TEXEL_FORMAT_V1D:
        case OA_TEXEL_FORMAT_T1D:
        case OA_TEXEL_FORMAT_DOUBLE:
        {
            chunkSize = sizeof(GLdouble);
        }
        break;

        case OA_TEXEL_FORMAT_V2D:
        case OA_TEXEL_FORMAT_T2D:
        case OA_TEXEL_FORMAT_DOUBLE2:
        {
            chunkSize = 2 * sizeof(GLdouble);
        }
        break;

        case OA_TEXEL_FORMAT_V3D:
        case OA_TEXEL_FORMAT_C3D:
        case OA_TEXEL_FORMAT_N3D:
        case OA_TEXEL_FORMAT_T3D:
        case OA_TEXEL_FORMAT_DOUBLE3:
        {
            chunkSize = 3 * sizeof(GLdouble);
        }
        break;

        case OA_TEXEL_FORMAT_V4D:
        case OA_TEXEL_FORMAT_C4D:
        case OA_TEXEL_FORMAT_T4D:
        case OA_TEXEL_FORMAT_DOUBLE4:
        {
            chunkSize = 4 * sizeof(GLdouble);
        }
        break;

        case OA_TEXEL_FORMAT_I1S:
        case OA_TEXEL_FORMAT_V1S:
        case OA_TEXEL_FORMAT_T1S:
        case OA_TEXEL_FORMAT_SHORT:
        {
            chunkSize = sizeof(GLshort);
        }
        break;

        case OA_TEXEL_FORMAT_V2S:
        case OA_TEXEL_FORMAT_T2S:
        case OA_TEXEL_FORMAT_SHORT2:
        {
            chunkSize = 2 * sizeof(GLshort);
        }
        break;

        case OA_TEXEL_FORMAT_V3S:
        case OA_TEXEL_FORMAT_N3S:
        case OA_TEXEL_FORMAT_T3S:
        case OA_TEXEL_FORMAT_C3S:
        case OA_TEXEL_FORMAT_SHORT3:
        {
            chunkSize = 3 * sizeof(GLshort);
        }
        break;

        case OA_TEXEL_FORMAT_V4S:
        case OA_TEXEL_FORMAT_C4S:
        case OA_TEXEL_FORMAT_T4S:
        case OA_TEXEL_FORMAT_SHORT4:
        {
            chunkSize = 4 * sizeof(GLshort);
        }
        break;

        case OA_TEXEL_FORMAT_SHORT8:
        {
            chunkSize = 8 * sizeof(GLshort);
        }
        break;

        case OA_TEXEL_FORMAT_SHORT16:
        {
            chunkSize = 16 * sizeof(GLshort);
        }
        break;

        case OA_TEXEL_FORMAT_I1I:
        case OA_TEXEL_FORMAT_V1I:
        case OA_TEXEL_FORMAT_T1I:
        case OA_TEXEL_FORMAT_INT:
        {
            chunkSize = sizeof(GLint);
        }
        break;

        case OA_TEXEL_FORMAT_V2I:
        case OA_TEXEL_FORMAT_T2I:
        case OA_TEXEL_FORMAT_INT2:
        {
            chunkSize = 2 * sizeof(GLint);
        }
        break;

        case OA_TEXEL_FORMAT_V3I:
        case OA_TEXEL_FORMAT_N3I:
        case OA_TEXEL_FORMAT_T3I:
        case OA_TEXEL_FORMAT_C3I:
        case OA_TEXEL_FORMAT_INT3:
        {
            chunkSize = 3 * sizeof(GLint);
        }
        break;

        case OA_TEXEL_FORMAT_V4I:
        case OA_TEXEL_FORMAT_C4I:
        case OA_TEXEL_FORMAT_T4I:
        case OA_TEXEL_FORMAT_INT4:
        {
            chunkSize = 4 * sizeof(GLint);
        }
        break;

        case OA_TEXEL_FORMAT_INT8:
        {
            chunkSize = 8 * sizeof(GLint);
        }
        break;

        case OA_TEXEL_FORMAT_INT16:
        {
            chunkSize = 16 * sizeof(GLint);
        }
        break;


        case OA_TEXEL_FORMAT_I1US:
        case OA_TEXEL_FORMAT_V1US:
        case OA_TEXEL_FORMAT_USHORT:
        {
            chunkSize = sizeof(GLushort);
        }
        break;

        case OA_TEXEL_FORMAT_V2US:
        case OA_TEXEL_FORMAT_USHORT2:
        {
            chunkSize = 2 * sizeof(GLushort);
        }
        break;

        case OA_TEXEL_FORMAT_V3US:
        case OA_TEXEL_FORMAT_C3US:
        case OA_TEXEL_FORMAT_USHORT3:
        {
            chunkSize = 3 * sizeof(GLushort);
        }
        break;

        case OA_TEXEL_FORMAT_V4US:
        case OA_TEXEL_FORMAT_C4US:
        case OA_TEXEL_FORMAT_USHORT4:
        {
            chunkSize = 4 * sizeof(GLushort);
        }
        break;

        case OA_TEXEL_FORMAT_USHORT8:
        {
            chunkSize = 8 * sizeof(GLushort);
        }
        break;

        case OA_TEXEL_FORMAT_USHORT16:
        {
            chunkSize = 16 * sizeof(GLushort);
        }
        break;


        case OA_TEXEL_FORMAT_C3B:
        case OA_TEXEL_FORMAT_V3B:
        case OA_TEXEL_FORMAT_N3B:
        {
            chunkSize = 3 * sizeof(GLbyte);
        }
        break;

        case OA_TEXEL_FORMAT_V4B:
        case OA_TEXEL_FORMAT_C4B:
        {
            chunkSize = 4 * sizeof(GLbyte);
        }
        break;

        case OA_TEXEL_FORMAT_V3UB:
        case OA_TEXEL_FORMAT_C3UB:
        {
            chunkSize = 3 * sizeof(GLubyte);
        }
        break;

        case OA_TEXEL_FORMAT_V4UB:
        case OA_TEXEL_FORMAT_C4UB:
        {
            chunkSize = 4 * sizeof(GLubyte);
        }
        break;

        case OA_TEXEL_FORMAT_V2UB:
        {
            chunkSize = 2 * sizeof(GLubyte);
        }
        break;

        case OA_TEXEL_FORMAT_V1B:
        {
            chunkSize = sizeof(GLbyte);
        }
        break;

        case OA_TEXEL_FORMAT_V2B:
        {
            chunkSize = 2 * sizeof(GLbyte);
        }
        break;

        case OA_TEXEL_FORMAT_I1UB:
        case OA_TEXEL_FORMAT_V1UB:
        {
            chunkSize = sizeof(GLubyte);
        }
        break;

        case OA_TEXEL_FORMAT_V1UI:
        case OA_TEXEL_FORMAT_I1UI:
        case OA_TEXEL_FORMAT_UINT:
        {
            chunkSize = sizeof(GLuint);
        }
        break;

        case OA_TEXEL_FORMAT_UINT2:
        case OA_TEXEL_FORMAT_V2UI:
        {
            chunkSize = 2 * sizeof(GLuint);
        }
        break;

        case OA_TEXEL_FORMAT_V3UI:
        case OA_TEXEL_FORMAT_C3UI:
        case OA_TEXEL_FORMAT_UINT3:
        {
            chunkSize = 3 * sizeof(GLuint);
        }
        break;

        case OA_TEXEL_FORMAT_V4UI:
        case OA_TEXEL_FORMAT_C4UI:
        case OA_TEXEL_FORMAT_UINT4:
        {
            chunkSize = 4 * sizeof(GLuint);
        }
        break;

        case OA_TEXEL_FORMAT_UINT8:
        {
            chunkSize = 8 * sizeof(GLuint);
        }
        break;

        case OA_TEXEL_FORMAT_UINT16:
        {
            chunkSize = 16 * sizeof(GLuint);
        }
        break;

        case OA_TEXEL_FORMAT_I1F:
        case OA_TEXEL_FORMAT_V1F:
        case OA_TEXEL_FORMAT_T1F:
        case OA_TEXEL_FORMAT_FLOAT:
        {
            chunkSize = sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_V2F:
        case OA_TEXEL_FORMAT_T2F:
        case OA_TEXEL_FORMAT_FLOAT2:
        {
            chunkSize = 2 * sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_T3F:
        case OA_TEXEL_FORMAT_N3F:
        case OA_TEXEL_FORMAT_V3F:
        case OA_TEXEL_FORMAT_C3F:
        case OA_TEXEL_FORMAT_FLOAT3:
        {
            chunkSize = 3 * sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_T4F:
        case OA_TEXEL_FORMAT_V4F:
        case OA_TEXEL_FORMAT_C4F:
        case OA_TEXEL_FORMAT_FLOAT4:
        {
            chunkSize = 4 * sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_C3F_V3F:
        case OA_TEXEL_FORMAT_N3F_V3F:
        {
            chunkSize = 6 * sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_T2F_V3F:
        {
            chunkSize = 5 * sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_T2F_C3F_V3F:
        case OA_TEXEL_FORMAT_T2F_N3F_V3F:
        case OA_TEXEL_FORMAT_T4F_V4F:
        case OA_TEXEL_FORMAT_FLOAT8:
        {
            chunkSize = 8 * sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_FLOAT16:
        {
            chunkSize = 16 * sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F:
        {
            chunkSize = 12 * sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_C4F_N3F_V3F:
        {
            chunkSize = 10 * sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F:
        {
            chunkSize = 15 * sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_DOUBLE8:
        {
            chunkSize = 8 * sizeof(GLdouble);
        }
        break;

        case OA_TEXEL_FORMAT_DOUBLE16:
        {
            chunkSize = 16 * sizeof(GLdouble);
        }
        break;

        case OA_TEXEL_FORMAT_C4UB_V2F:
        {
            chunkSize = 4 * sizeof(GLubyte) + 2 * sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_C4UB_V3F:
        {
            chunkSize = 4 * sizeof(GLubyte) + 3 * sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_T2F_C4UB_V3F:
        {
            chunkSize = 4 * sizeof(GLubyte) + 5 * sizeof(GLfloat);
        }
        break;

        case OA_TEXEL_FORMAT_CHAR:
        {
            chunkSize = sizeof(GLchar);
        }
        break;

        case OA_TEXEL_FORMAT_CHAR2:
        {
            chunkSize = 2 * sizeof(GLchar);
        }
        break;

        case OA_TEXEL_FORMAT_CHAR3:
        {
            chunkSize = 3 * sizeof(GLchar);
        }
        break;

        case OA_TEXEL_FORMAT_CHAR4:
        {
            chunkSize = 4 * sizeof(GLchar);
        }
        break;

        case OA_TEXEL_FORMAT_CHAR8:
        {
            chunkSize = 8 * sizeof(GLchar);
        }
        break;

        case OA_TEXEL_FORMAT_CHAR16:
        {
            chunkSize = 16 * sizeof(GLchar);
        }
        break;

        case OA_TEXEL_FORMAT_UCHAR:
        {
            chunkSize = sizeof(cl_uchar);
        }
        break;

        case OA_TEXEL_FORMAT_UCHAR2:
        {
            chunkSize = 2 * sizeof(cl_uchar);
        }
        break;

        case OA_TEXEL_FORMAT_UCHAR3:
        {
            chunkSize = 3 * sizeof(cl_uchar);
        }
        break;

        case OA_TEXEL_FORMAT_UCHAR4:
        {
            chunkSize = 4 * sizeof(cl_uchar);
        }
        break;

        case OA_TEXEL_FORMAT_UCHAR8:
        {
            chunkSize = 8 * sizeof(cl_uchar);
        }
        break;

        case OA_TEXEL_FORMAT_UCHAR16:
        {
            chunkSize = 16 * sizeof(cl_uchar);
        }
        break;

        case OA_TEXEL_FORMAT_LONG:
        {
            chunkSize = sizeof(cl_long);
        }
        break;

        case OA_TEXEL_FORMAT_LONG2:
        {
            chunkSize = 2 * sizeof(cl_long);
        }
        break;

        case OA_TEXEL_FORMAT_LONG3:
        {
            chunkSize = 3 * sizeof(cl_long);
        }
        break;

        case OA_TEXEL_FORMAT_LONG4:
        {
            chunkSize = 4 * sizeof(cl_long);
        }
        break;

        case OA_TEXEL_FORMAT_LONG8:
        {
            chunkSize = 8 * sizeof(cl_long);
        }
        break;

        case OA_TEXEL_FORMAT_LONG16:
        {
            chunkSize = 16 * sizeof(cl_long);
        }
        break;

        case OA_TEXEL_FORMAT_ULONG:
        {
            chunkSize = sizeof(cl_ulong);
        }
        break;

        case OA_TEXEL_FORMAT_ULONG2:
        {
            chunkSize = 2 * sizeof(cl_ulong);
        }
        break;

        case OA_TEXEL_FORMAT_ULONG3:
        {
            chunkSize = 3 * sizeof(cl_ulong);
        }
        break;

        case OA_TEXEL_FORMAT_ULONG4:
        {
            chunkSize = 4 * sizeof(cl_ulong);
        }
        break;

        case OA_TEXEL_FORMAT_ULONG8:
        {
            chunkSize = 8 * sizeof(cl_ulong);
        }
        break;

        case OA_TEXEL_FORMAT_ULONG16:
        {
            chunkSize = 16 * sizeof(cl_ulong);
        }
        break;

        default:
        {
            // This is wrong, we want to get component type of a specific index and
            // we didn't handle it yet - return an error
            GT_ASSERT_EX(false, L"Unknown Texel format data  type");
        }
        break;
    }

    return chunkSize;
}

// ---------------------------------------------------------------------------
// Name:        oaGetTexelFormatComponentType
// Description: Get the component type from the texel format in a specific index. Also get the location of the requested component
//              within the specific elements vector.
//              Example: OA_TEXEL_FORMAT_C3F_V3F, with 4 element requested will return OA_TEXEL_FORMAT_VECTOR, and 1
//             (the 4th element is the first vector element)
// Arguments: oaTexelDataFormat texelDataFormat
//            int componentIndex
//            oaTexelDataFormat& componentType
//            int& componentLocalIndex
// Return Val: bool OA_API  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/4/2009
// ---------------------------------------------------------------------------
bool OA_API oaGetTexelFormatBufferComponentType(oaTexelDataFormat texelDataFormat, int componentIndex, oaTexelDataFormat& componentType, int& componentLocalIndex)
{
    bool retVal = true;

    componentLocalIndex = componentIndex;

    switch (texelDataFormat)
    {
        case OA_TEXEL_FORMAT_V1F:
        case OA_TEXEL_FORMAT_V2F:
        case OA_TEXEL_FORMAT_V3F:
        case OA_TEXEL_FORMAT_V4F:
        case OA_TEXEL_FORMAT_V1D:
        case OA_TEXEL_FORMAT_V2D:
        case OA_TEXEL_FORMAT_V3D:
        case OA_TEXEL_FORMAT_V4D:
        case OA_TEXEL_FORMAT_V1S:
        case OA_TEXEL_FORMAT_V2S:
        case OA_TEXEL_FORMAT_V3S:
        case OA_TEXEL_FORMAT_V4S:
        case OA_TEXEL_FORMAT_V1US:
        case OA_TEXEL_FORMAT_V2US:
        case OA_TEXEL_FORMAT_V3US:
        case OA_TEXEL_FORMAT_V4US:
        case OA_TEXEL_FORMAT_V1B:
        case OA_TEXEL_FORMAT_V2B:
        case OA_TEXEL_FORMAT_V3B:
        case OA_TEXEL_FORMAT_V4B:
        case OA_TEXEL_FORMAT_V1UB:
        case OA_TEXEL_FORMAT_V2UB:
        case OA_TEXEL_FORMAT_V3UB:
        case OA_TEXEL_FORMAT_V4UB:
        case OA_TEXEL_FORMAT_V1I:
        case OA_TEXEL_FORMAT_V2I:
        case OA_TEXEL_FORMAT_V3I:
        case OA_TEXEL_FORMAT_V4I:
        case OA_TEXEL_FORMAT_V1UI:
        case OA_TEXEL_FORMAT_V2UI:
        case OA_TEXEL_FORMAT_V3UI:
        case OA_TEXEL_FORMAT_V4UI:
        {
            componentType = OA_TEXEL_FORMAT_VECTOR;
        }
        break;

        case OA_TEXEL_FORMAT_C3F:
        case OA_TEXEL_FORMAT_C4F:
        case OA_TEXEL_FORMAT_C3D:
        case OA_TEXEL_FORMAT_C4D:
        case OA_TEXEL_FORMAT_C3S:
        case OA_TEXEL_FORMAT_C4S:
        case OA_TEXEL_FORMAT_C3US:
        case OA_TEXEL_FORMAT_C4US:
        case OA_TEXEL_FORMAT_C3B:
        case OA_TEXEL_FORMAT_C4B:
        case OA_TEXEL_FORMAT_C3UB:
        case OA_TEXEL_FORMAT_C4UB:
        case OA_TEXEL_FORMAT_C3I:
        case OA_TEXEL_FORMAT_C4I:
        case OA_TEXEL_FORMAT_C3UI:
        case OA_TEXEL_FORMAT_C4UI:
        {
            componentType = OA_TEXEL_FORMAT_COLOR;
        }
        break;

        case OA_TEXEL_FORMAT_N3F:
        case OA_TEXEL_FORMAT_N3D:
        case OA_TEXEL_FORMAT_N3I:
        case OA_TEXEL_FORMAT_N3S:
        case OA_TEXEL_FORMAT_N3B:
        {
            componentType = OA_TEXEL_FORMAT_NORMAL;
        }
        break;

        case OA_TEXEL_FORMAT_T1F:
        case OA_TEXEL_FORMAT_T2F:
        case OA_TEXEL_FORMAT_T3F:
        case OA_TEXEL_FORMAT_T4F:
        case OA_TEXEL_FORMAT_T1D:
        case OA_TEXEL_FORMAT_T2D:
        case OA_TEXEL_FORMAT_T3D:
        case OA_TEXEL_FORMAT_T4D:
        case OA_TEXEL_FORMAT_T1S:
        case OA_TEXEL_FORMAT_T2S:
        case OA_TEXEL_FORMAT_T3S:
        case OA_TEXEL_FORMAT_T4S:
        case OA_TEXEL_FORMAT_T1I:
        case OA_TEXEL_FORMAT_T2I:
        case OA_TEXEL_FORMAT_T3I:
        case OA_TEXEL_FORMAT_T4I:
        {
            componentType = OA_TEXEL_FORMAT_TEXTURE;
        }
        break;

        case OA_TEXEL_FORMAT_I1UI:
        case OA_TEXEL_FORMAT_I1UB:
        case OA_TEXEL_FORMAT_I1US:
        case OA_TEXEL_FORMAT_I1S:
        case OA_TEXEL_FORMAT_I1I:
        case OA_TEXEL_FORMAT_I1F:
        case OA_TEXEL_FORMAT_I1D:
        {
            componentType = OA_TEXEL_FORMAT_INDEX;
        }
        break;


        case OA_TEXEL_FORMAT_C4UB_V2F:
        case OA_TEXEL_FORMAT_C4UB_V3F:
        {
            if (componentIndex >= 4)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
                componentLocalIndex = componentIndex - 4;
            }
            else
            {
                componentType = OA_TEXEL_FORMAT_COLOR;
            }
        }
        break;

        case OA_TEXEL_FORMAT_C3F_V3F:
        case OA_TEXEL_FORMAT_N3F_V3F:
        {
            if (componentIndex >= 3)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
                componentLocalIndex = componentIndex - 3;
            }
            else
            {
                if (texelDataFormat == OA_TEXEL_FORMAT_N3F_V3F)
                {
                    componentType = OA_TEXEL_FORMAT_NORMAL;
                }
                else
                {
                    componentType = OA_TEXEL_FORMAT_COLOR;
                }
            }
        }
        break;

        case OA_TEXEL_FORMAT_T2F_V3F:
        {
            if (componentIndex <= 1)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
                componentLocalIndex = componentIndex - 2;
            }
        }
        break;

        case OA_TEXEL_FORMAT_T2F_C4UB_V3F:
        {
            if (componentIndex <= 1)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else if (componentIndex <= 5)
            {
                componentType = OA_TEXEL_FORMAT_COLOR;
                componentLocalIndex = componentIndex - 2;
            }
            else if (componentIndex <= 8)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
                componentLocalIndex = componentIndex - 6;
            }
        }
        break;

        case OA_TEXEL_FORMAT_T2F_C3F_V3F:
        {
            if (componentIndex <= 1)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else if (componentIndex <= 4)
            {
                componentType = OA_TEXEL_FORMAT_COLOR;
                componentLocalIndex = componentIndex - 2;
            }
            else if (componentIndex <= 8)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
                componentLocalIndex = componentIndex - 5;
            }
        }
        break;

        case OA_TEXEL_FORMAT_T2F_N3F_V3F:
        {
            if (componentIndex <= 1)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else if (componentIndex <= 4)
            {
                componentType = OA_TEXEL_FORMAT_NORMAL;
                componentLocalIndex = componentIndex - 2;
            }
            else if (componentIndex <= 7)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
                componentLocalIndex = componentIndex - 5;
            }
        }
        break;

        case OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F:
        {
            if (componentIndex <= 1)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else if (componentIndex <= 5)
            {
                componentType = OA_TEXEL_FORMAT_COLOR;
                componentLocalIndex = componentIndex - 2;
            }
            else if (componentIndex <= 8)
            {
                componentType = OA_TEXEL_FORMAT_NORMAL;
                componentLocalIndex = componentIndex - 6;
            }
            else if (componentIndex <= 11)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
                componentLocalIndex = componentIndex - 9;
            }
        }
        break;

        case OA_TEXEL_FORMAT_C4F_N3F_V3F:
        {
            if (componentIndex <= 3)
            {
                componentType = OA_TEXEL_FORMAT_COLOR;
            }
            else if (componentIndex <= 6)
            {
                componentType = OA_TEXEL_FORMAT_NORMAL;
                componentLocalIndex = componentIndex - 4;
            }
            else if (componentIndex <= 9)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
                componentLocalIndex = componentIndex - 7;
            }
        }
        break;

        case OA_TEXEL_FORMAT_T4F_V4F:
        {
            if (componentIndex <= 3)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else if (componentIndex <= 7)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
                componentLocalIndex = componentIndex - 4;
            }
        }
        break;

        case OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F:
        {
            if (componentIndex <= 3)
            {
                componentType = OA_TEXEL_FORMAT_TEXTURE;
            }
            else if (componentIndex <= 7)
            {
                componentType = OA_TEXEL_FORMAT_COLOR;
                componentLocalIndex = componentIndex - 4;
            }
            else if (componentIndex <= 10)
            {
                componentType = OA_TEXEL_FORMAT_NORMAL;
                componentLocalIndex = componentIndex - 8;
            }
            else if (componentIndex <= 14)
            {
                componentType = OA_TEXEL_FORMAT_VECTOR;
                componentLocalIndex = componentIndex - 11;
            }
        }
        break;

        case OA_TEXEL_FORMAT_CHAR:
        case OA_TEXEL_FORMAT_CHAR2:
        case OA_TEXEL_FORMAT_CHAR3:
        case OA_TEXEL_FORMAT_CHAR4:
        case OA_TEXEL_FORMAT_CHAR8:
        case OA_TEXEL_FORMAT_CHAR16:
        {
            componentType = OA_TEXEL_FORMAT_CHAR;
        }
        break;

        case OA_TEXEL_FORMAT_UCHAR:
        case OA_TEXEL_FORMAT_UCHAR2:
        case OA_TEXEL_FORMAT_UCHAR3:
        case OA_TEXEL_FORMAT_UCHAR4:
        case OA_TEXEL_FORMAT_UCHAR8:
        case OA_TEXEL_FORMAT_UCHAR16:
        {
            componentType = OA_TEXEL_FORMAT_UCHAR;
        }
        break;

        case OA_TEXEL_FORMAT_SHORT:
        case OA_TEXEL_FORMAT_SHORT2:
        case OA_TEXEL_FORMAT_SHORT3:
        case OA_TEXEL_FORMAT_SHORT4:
        case OA_TEXEL_FORMAT_SHORT8:
        case OA_TEXEL_FORMAT_SHORT16:
        {
            componentType = OA_TEXEL_FORMAT_SHORT;
        }
        break;

        case OA_TEXEL_FORMAT_USHORT:
        case OA_TEXEL_FORMAT_USHORT2:
        case OA_TEXEL_FORMAT_USHORT3:
        case OA_TEXEL_FORMAT_USHORT4:
        case OA_TEXEL_FORMAT_USHORT8:
        case OA_TEXEL_FORMAT_USHORT16:
        {
            componentType = OA_TEXEL_FORMAT_USHORT;
        }
        break;

        case OA_TEXEL_FORMAT_INT:
        case OA_TEXEL_FORMAT_INT2:
        case OA_TEXEL_FORMAT_INT3:
        case OA_TEXEL_FORMAT_INT4:
        case OA_TEXEL_FORMAT_INT8:
        case OA_TEXEL_FORMAT_INT16:
        {
            componentType = OA_TEXEL_FORMAT_INT;
        }
        break;

        case OA_TEXEL_FORMAT_UINT:
        case OA_TEXEL_FORMAT_UINT2:
        case OA_TEXEL_FORMAT_UINT3:
        case OA_TEXEL_FORMAT_UINT4:
        case OA_TEXEL_FORMAT_UINT8:
        case OA_TEXEL_FORMAT_UINT16:
        {
            componentType = OA_TEXEL_FORMAT_UINT;
        }
        break;

        case OA_TEXEL_FORMAT_LONG:
        case OA_TEXEL_FORMAT_LONG2:
        case OA_TEXEL_FORMAT_LONG3:
        case OA_TEXEL_FORMAT_LONG4:
        case OA_TEXEL_FORMAT_LONG8:
        case OA_TEXEL_FORMAT_LONG16:
        {
            componentType = OA_TEXEL_FORMAT_LONG;
        }
        break;

        case OA_TEXEL_FORMAT_ULONG:
        case OA_TEXEL_FORMAT_ULONG2:
        case OA_TEXEL_FORMAT_ULONG3:
        case OA_TEXEL_FORMAT_ULONG4:
        case OA_TEXEL_FORMAT_ULONG8:
        case OA_TEXEL_FORMAT_ULONG16:
        {
            componentType = OA_TEXEL_FORMAT_ULONG;
        }
        break;

        case OA_TEXEL_FORMAT_FLOAT:
        case OA_TEXEL_FORMAT_FLOAT2:
        case OA_TEXEL_FORMAT_FLOAT3:
        case OA_TEXEL_FORMAT_FLOAT4:
        case OA_TEXEL_FORMAT_FLOAT8:
        case OA_TEXEL_FORMAT_FLOAT16:
        {
            componentType = OA_TEXEL_FORMAT_FLOAT;
        }
        break;

        case OA_TEXEL_FORMAT_DOUBLE:
        case OA_TEXEL_FORMAT_DOUBLE2:
        case OA_TEXEL_FORMAT_DOUBLE3:
        case OA_TEXEL_FORMAT_DOUBLE4:
        case OA_TEXEL_FORMAT_DOUBLE8:
        case OA_TEXEL_FORMAT_DOUBLE16:
        {
            componentType = OA_TEXEL_FORMAT_DOUBLE;
        }
        break;

        default:
        {
            // This is wrong, we want to get component type of a specific index and
            // we didn't handle it yet - return an error
            GT_ASSERT_EX(false, L"Unknown Texel format type");
            retVal = false;
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acRawFileHandler::isVBOTexelFormat
// Checks if a texel data format holds a VBO data:
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        16/4/2009
// ---------------------------------------------------------------------------
bool OA_API oaIsBufferTexelFormat(oaTexelDataFormat texelDataFormat)
{
    bool retVal = false;

    switch (texelDataFormat)
    {
        case OA_TEXEL_FORMAT_V1F:
        case OA_TEXEL_FORMAT_V2F:
        case OA_TEXEL_FORMAT_V3F:
        case OA_TEXEL_FORMAT_V4F:
        case OA_TEXEL_FORMAT_V1D:
        case OA_TEXEL_FORMAT_V2D:
        case OA_TEXEL_FORMAT_V3D:
        case OA_TEXEL_FORMAT_V4D:
        case OA_TEXEL_FORMAT_V1S:
        case OA_TEXEL_FORMAT_V2S:
        case OA_TEXEL_FORMAT_V3S:
        case OA_TEXEL_FORMAT_V4S:
        case OA_TEXEL_FORMAT_V1US:
        case OA_TEXEL_FORMAT_V2US:
        case OA_TEXEL_FORMAT_V3US:
        case OA_TEXEL_FORMAT_V4US:
        case OA_TEXEL_FORMAT_V1B:
        case OA_TEXEL_FORMAT_V2B:
        case OA_TEXEL_FORMAT_V3B:
        case OA_TEXEL_FORMAT_V4B:
        case OA_TEXEL_FORMAT_V1UB:
        case OA_TEXEL_FORMAT_V2UB:
        case OA_TEXEL_FORMAT_V3UB:
        case OA_TEXEL_FORMAT_V4UB:
        case OA_TEXEL_FORMAT_V1I:
        case OA_TEXEL_FORMAT_V2I:
        case OA_TEXEL_FORMAT_V3I:
        case OA_TEXEL_FORMAT_V4I:
        case OA_TEXEL_FORMAT_V1UI:
        case OA_TEXEL_FORMAT_V2UI:
        case OA_TEXEL_FORMAT_V3UI:
        case OA_TEXEL_FORMAT_V4UI:

        case OA_TEXEL_FORMAT_C3F:
        case OA_TEXEL_FORMAT_C4F:
        case OA_TEXEL_FORMAT_C3D:
        case OA_TEXEL_FORMAT_C4D:
        case OA_TEXEL_FORMAT_C3S:
        case OA_TEXEL_FORMAT_C4S:
        case OA_TEXEL_FORMAT_C3US:
        case OA_TEXEL_FORMAT_C4US:
        case OA_TEXEL_FORMAT_C3B:
        case OA_TEXEL_FORMAT_C4B:
        case OA_TEXEL_FORMAT_C3UB:
        case OA_TEXEL_FORMAT_C4UB:
        case OA_TEXEL_FORMAT_C3I:
        case OA_TEXEL_FORMAT_C4I:
        case OA_TEXEL_FORMAT_C3UI:
        case OA_TEXEL_FORMAT_C4UI:

        case OA_TEXEL_FORMAT_N3F:
        case OA_TEXEL_FORMAT_N3D:
        case OA_TEXEL_FORMAT_N3I:
        case OA_TEXEL_FORMAT_N3S:
        case OA_TEXEL_FORMAT_N3B:

        case OA_TEXEL_FORMAT_T1F:
        case OA_TEXEL_FORMAT_T2F:
        case OA_TEXEL_FORMAT_T3F:
        case OA_TEXEL_FORMAT_T4F:
        case OA_TEXEL_FORMAT_T1D:
        case OA_TEXEL_FORMAT_T2D:
        case OA_TEXEL_FORMAT_T3D:
        case OA_TEXEL_FORMAT_T4D:
        case OA_TEXEL_FORMAT_T1S:
        case OA_TEXEL_FORMAT_T2S:
        case OA_TEXEL_FORMAT_T3S:
        case OA_TEXEL_FORMAT_T4S:
        case OA_TEXEL_FORMAT_T1I:
        case OA_TEXEL_FORMAT_T2I:
        case OA_TEXEL_FORMAT_T3I:
        case OA_TEXEL_FORMAT_T4I:

        case OA_TEXEL_FORMAT_I1UI:
        case OA_TEXEL_FORMAT_I1UB:
        case OA_TEXEL_FORMAT_I1US:
        case OA_TEXEL_FORMAT_I1S:
        case OA_TEXEL_FORMAT_I1I:
        case OA_TEXEL_FORMAT_I1F:
        case OA_TEXEL_FORMAT_I1D:

        case OA_TEXEL_FORMAT_C4UB_V2F:
        case OA_TEXEL_FORMAT_C4UB_V3F:
        case OA_TEXEL_FORMAT_C3F_V3F:
        case OA_TEXEL_FORMAT_N3F_V3F:
        case OA_TEXEL_FORMAT_C4F_N3F_V3F:
        case OA_TEXEL_FORMAT_T2F_V3F:
        case OA_TEXEL_FORMAT_T4F_V4F:
        case OA_TEXEL_FORMAT_T2F_C4UB_V3F:
        case OA_TEXEL_FORMAT_T2F_C3F_V3F:
        case OA_TEXEL_FORMAT_T2F_N3F_V3F:
        case OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F:
        case OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F:

        case OA_TEXEL_FORMAT_CHAR:
        case OA_TEXEL_FORMAT_CHAR2:
        case OA_TEXEL_FORMAT_CHAR3:
        case OA_TEXEL_FORMAT_CHAR4:
        case OA_TEXEL_FORMAT_CHAR8:
        case OA_TEXEL_FORMAT_CHAR16:

        case OA_TEXEL_FORMAT_UCHAR:
        case OA_TEXEL_FORMAT_UCHAR2:
        case OA_TEXEL_FORMAT_UCHAR3:
        case OA_TEXEL_FORMAT_UCHAR4:
        case OA_TEXEL_FORMAT_UCHAR8:
        case OA_TEXEL_FORMAT_UCHAR16:

        case OA_TEXEL_FORMAT_SHORT:
        case OA_TEXEL_FORMAT_SHORT2:
        case OA_TEXEL_FORMAT_SHORT3:
        case OA_TEXEL_FORMAT_SHORT4:
        case OA_TEXEL_FORMAT_SHORT8:
        case OA_TEXEL_FORMAT_SHORT16:

        case OA_TEXEL_FORMAT_USHORT:
        case OA_TEXEL_FORMAT_USHORT2:
        case OA_TEXEL_FORMAT_USHORT3:
        case OA_TEXEL_FORMAT_USHORT4:
        case OA_TEXEL_FORMAT_USHORT8:
        case OA_TEXEL_FORMAT_USHORT16:

        case OA_TEXEL_FORMAT_INT:
        case OA_TEXEL_FORMAT_INT2:
        case OA_TEXEL_FORMAT_INT3:
        case OA_TEXEL_FORMAT_INT4:
        case OA_TEXEL_FORMAT_INT8:
        case OA_TEXEL_FORMAT_INT16:

        case OA_TEXEL_FORMAT_UINT:
        case OA_TEXEL_FORMAT_UINT2:
        case OA_TEXEL_FORMAT_UINT3:
        case OA_TEXEL_FORMAT_UINT4:
        case OA_TEXEL_FORMAT_UINT8:
        case OA_TEXEL_FORMAT_UINT16:

        case OA_TEXEL_FORMAT_LONG:
        case OA_TEXEL_FORMAT_LONG2:
        case OA_TEXEL_FORMAT_LONG3:
        case OA_TEXEL_FORMAT_LONG4:
        case OA_TEXEL_FORMAT_LONG8:
        case OA_TEXEL_FORMAT_LONG16:

        case OA_TEXEL_FORMAT_ULONG:
        case OA_TEXEL_FORMAT_ULONG2:
        case OA_TEXEL_FORMAT_ULONG3:
        case OA_TEXEL_FORMAT_ULONG4:
        case OA_TEXEL_FORMAT_ULONG8:
        case OA_TEXEL_FORMAT_ULONG16:

        case OA_TEXEL_FORMAT_FLOAT:
        case OA_TEXEL_FORMAT_FLOAT2:
        case OA_TEXEL_FORMAT_FLOAT3:
        case OA_TEXEL_FORMAT_FLOAT4:
        case OA_TEXEL_FORMAT_FLOAT8:
        case OA_TEXEL_FORMAT_FLOAT16:

        case OA_TEXEL_FORMAT_DOUBLE:
        case OA_TEXEL_FORMAT_DOUBLE2:
        case OA_TEXEL_FORMAT_DOUBLE3:
        case OA_TEXEL_FORMAT_DOUBLE4:
        case OA_TEXEL_FORMAT_DOUBLE8:
        case OA_TEXEL_FORMAT_DOUBLE16:

        {
            retVal = true;
        }
        break;

        default:
        {
            retVal = false;
        }
        break;
    }

    return retVal;
}


// Get the data types contained within the texel format:

// ---------------------------------------------------------------------------
// Name:        oaGetTexelFormatContainedDataTypes
// Description: Return a vector of data types contained in the texel format
// Arguments: gtVector<oaDataType>& dataTypes
// Return Val: bool OA_API  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        23/4/2009
// ---------------------------------------------------------------------------
bool OA_API oaGetTexelFormatContainedDataTypes(oaTexelDataFormat texelDataFormat, gtVector<oaDataType>& dataTypes)
{
    bool retVal = true;

    // Clear the data types:
    dataTypes.clear();

    switch (texelDataFormat)
    {
        case OA_TEXEL_FORMAT_C3F_V3F:
        case OA_TEXEL_FORMAT_N3F_V3F:
        case OA_TEXEL_FORMAT_T2F_V3F:
        case OA_TEXEL_FORMAT_T2F_C3F_V3F:
        case OA_TEXEL_FORMAT_T2F_N3F_V3F:
        case OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F:
        case OA_TEXEL_FORMAT_C4F_N3F_V3F:
        case OA_TEXEL_FORMAT_T4F_V4F:
        case OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F:
        case OA_TEXEL_FORMAT_V1F:
        case OA_TEXEL_FORMAT_I1F:
        case OA_TEXEL_FORMAT_V2F:
        case OA_TEXEL_FORMAT_V3F:
        case OA_TEXEL_FORMAT_V4F:
        case OA_TEXEL_FORMAT_C3F:
        case OA_TEXEL_FORMAT_C4F:
        case OA_TEXEL_FORMAT_N3F:
        case OA_TEXEL_FORMAT_T1F:
        case OA_TEXEL_FORMAT_T2F:
        case OA_TEXEL_FORMAT_T3F:
        case OA_TEXEL_FORMAT_T4F:
        case OA_TEXEL_FORMAT_FLOAT:
        case OA_TEXEL_FORMAT_FLOAT2:
        case OA_TEXEL_FORMAT_FLOAT3:
        case OA_TEXEL_FORMAT_FLOAT4:
        case OA_TEXEL_FORMAT_FLOAT8:
        case OA_TEXEL_FORMAT_FLOAT16:
        {
            dataTypes.push_back(OA_FLOAT);
        }
        break;

        case OA_TEXEL_FORMAT_I1D:
        case OA_TEXEL_FORMAT_V1D:
        case OA_TEXEL_FORMAT_V2D:
        case OA_TEXEL_FORMAT_V3D:
        case OA_TEXEL_FORMAT_V4D:
        case OA_TEXEL_FORMAT_C3D:
        case OA_TEXEL_FORMAT_C4D:
        case OA_TEXEL_FORMAT_N3D:
        case OA_TEXEL_FORMAT_T1D:
        case OA_TEXEL_FORMAT_T2D:
        case OA_TEXEL_FORMAT_T3D:
        case OA_TEXEL_FORMAT_T4D:
        case OA_TEXEL_FORMAT_DOUBLE:
        case OA_TEXEL_FORMAT_DOUBLE2:
        case OA_TEXEL_FORMAT_DOUBLE3:
        case OA_TEXEL_FORMAT_DOUBLE4:
        case OA_TEXEL_FORMAT_DOUBLE8:
        case OA_TEXEL_FORMAT_DOUBLE16:
        {
            dataTypes.push_back(OA_DOUBLE);
        }
        break;

        case OA_TEXEL_FORMAT_I1S:
        case OA_TEXEL_FORMAT_V1S:
        case OA_TEXEL_FORMAT_V2S:
        case OA_TEXEL_FORMAT_V3S:
        case OA_TEXEL_FORMAT_V4S:
        case OA_TEXEL_FORMAT_C3S:
        case OA_TEXEL_FORMAT_C4S:
        case OA_TEXEL_FORMAT_N3S:
        case OA_TEXEL_FORMAT_T1S:
        case OA_TEXEL_FORMAT_T2S:
        case OA_TEXEL_FORMAT_T3S:
        case OA_TEXEL_FORMAT_T4S:
        case OA_TEXEL_FORMAT_SHORT:
        case OA_TEXEL_FORMAT_SHORT2:
        case OA_TEXEL_FORMAT_SHORT3:
        case OA_TEXEL_FORMAT_SHORT4:
        case OA_TEXEL_FORMAT_SHORT8:
        case OA_TEXEL_FORMAT_SHORT16:
        {
            dataTypes.push_back(OA_SHORT);
        }
        break;

        case OA_TEXEL_FORMAT_I1I:
        case OA_TEXEL_FORMAT_V1I:
        case OA_TEXEL_FORMAT_V2I:
        case OA_TEXEL_FORMAT_V3I:
        case OA_TEXEL_FORMAT_V4I:
        case OA_TEXEL_FORMAT_C3I:
        case OA_TEXEL_FORMAT_C4I:
        case OA_TEXEL_FORMAT_N3I:
        case OA_TEXEL_FORMAT_T1I:
        case OA_TEXEL_FORMAT_T2I:
        case OA_TEXEL_FORMAT_T3I:
        case OA_TEXEL_FORMAT_T4I:
        case OA_TEXEL_FORMAT_INT:
        case OA_TEXEL_FORMAT_INT2:
        case OA_TEXEL_FORMAT_INT3:
        case OA_TEXEL_FORMAT_INT4:
        case OA_TEXEL_FORMAT_INT8:
        case OA_TEXEL_FORMAT_INT16:
        {
            dataTypes.push_back(OA_INT);
        }
        break;

        case OA_TEXEL_FORMAT_I1UI:
        case OA_TEXEL_FORMAT_V2UI:
        case OA_TEXEL_FORMAT_V3UI:
        case OA_TEXEL_FORMAT_V4UI:
        case OA_TEXEL_FORMAT_C3UI:
        case OA_TEXEL_FORMAT_C4UI:
        case OA_TEXEL_FORMAT_UINT:
        case OA_TEXEL_FORMAT_UINT2:
        case OA_TEXEL_FORMAT_UINT3:
        case OA_TEXEL_FORMAT_UINT4:
        case OA_TEXEL_FORMAT_UINT8:
        case OA_TEXEL_FORMAT_UINT16:
        {
            dataTypes.push_back(OA_UNSIGNED_INT);
        }
        break;

        case OA_TEXEL_FORMAT_V1US:
        case OA_TEXEL_FORMAT_V2US:
        case OA_TEXEL_FORMAT_V3US:
        case OA_TEXEL_FORMAT_V4US:
        case OA_TEXEL_FORMAT_C3US:
        case OA_TEXEL_FORMAT_C4US:
        case OA_TEXEL_FORMAT_USHORT:
        case OA_TEXEL_FORMAT_USHORT2:
        case OA_TEXEL_FORMAT_USHORT3:
        case OA_TEXEL_FORMAT_USHORT4:
        case OA_TEXEL_FORMAT_USHORT8:
        case OA_TEXEL_FORMAT_USHORT16:
        {
            dataTypes.push_back(OA_UNSIGNED_SHORT);
        }
        break;

        case OA_TEXEL_FORMAT_CHAR:
        case OA_TEXEL_FORMAT_CHAR2:
        case OA_TEXEL_FORMAT_CHAR3:
        case OA_TEXEL_FORMAT_CHAR4:
        case OA_TEXEL_FORMAT_CHAR8:
        case OA_TEXEL_FORMAT_CHAR16:
        {
            dataTypes.push_back(OA_CHAR);
        }
        break;

        case OA_TEXEL_FORMAT_UCHAR:
        case OA_TEXEL_FORMAT_UCHAR2:
        case OA_TEXEL_FORMAT_UCHAR3:
        case OA_TEXEL_FORMAT_UCHAR4:
        case OA_TEXEL_FORMAT_UCHAR8:
        case OA_TEXEL_FORMAT_UCHAR16:
        {
            dataTypes.push_back(OA_UNSIGNED_CHAR);
        }
        break;

        case OA_TEXEL_FORMAT_LONG:
        case OA_TEXEL_FORMAT_LONG2:
        case OA_TEXEL_FORMAT_LONG3:
        case OA_TEXEL_FORMAT_LONG4:
        case OA_TEXEL_FORMAT_LONG8:
        case OA_TEXEL_FORMAT_LONG16:
        {
            dataTypes.push_back(OA_LONG);
        }
        break;

        case OA_TEXEL_FORMAT_ULONG:
        case OA_TEXEL_FORMAT_ULONG2:
        case OA_TEXEL_FORMAT_ULONG3:
        case OA_TEXEL_FORMAT_ULONG4:
        case OA_TEXEL_FORMAT_ULONG8:
        case OA_TEXEL_FORMAT_ULONG16:
        {
            dataTypes.push_back(OA_UNSIGNED_LONG);
        }
        break;

        case OA_TEXEL_FORMAT_V1B:
        case OA_TEXEL_FORMAT_V2B:
        case OA_TEXEL_FORMAT_V3B:
        case OA_TEXEL_FORMAT_V4B:
        case OA_TEXEL_FORMAT_C3B:
        case OA_TEXEL_FORMAT_C4B:
        case OA_TEXEL_FORMAT_N3B:
        {
            dataTypes.push_back(OA_BYTE);
        }
        break;

        case OA_TEXEL_FORMAT_I1US:
        case OA_TEXEL_FORMAT_I1UB:
        case OA_TEXEL_FORMAT_V1UB:
        case OA_TEXEL_FORMAT_V2UB:
        case OA_TEXEL_FORMAT_V3UB:
        case OA_TEXEL_FORMAT_V4UB:
        case OA_TEXEL_FORMAT_C3UB:
        case OA_TEXEL_FORMAT_C4UB:
        {
            dataTypes.push_back(OA_UNSIGNED_BYTE);
        }
        break;

        case OA_TEXEL_FORMAT_C4UB_V2F:
        case OA_TEXEL_FORMAT_C4UB_V3F:
        case OA_TEXEL_FORMAT_T2F_C4UB_V3F:
        {
            dataTypes.push_back(OA_FLOAT);
            dataTypes.push_back(OA_UNSIGNED_BYTE);
        }
        break;

        default:
        {
            retVal = false;
        }
        break;
    }

    return retVal;
}

#ifndef _GR_IPHONE_BUILD
// ---------------------------------------------------------------------------
// Name:        oaCLImageFormatToTexelFormat
// Description: Translate OpenCL data format to our format
// Arguments:   cl_uint clImageFormat - the CL data format constant
//              oaTexelDataFormat& dataFormat - the data format in our format
// Return Val:  bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/7/2010
// ---------------------------------------------------------------------------
bool oaCLImageFormatToTexelFormat(const cl_uint& clImageFormat, oaTexelDataFormat& dataFormat)
{
    bool retVal = true;

    // Note: the CL_*x formats are exactly identical to their non-x counterparts, except for the
    // behavior when a clamping sampler goes out of range. As a result, we must treat them exactly
    // the same.
    switch (clImageFormat)
    {
        case CL_R:
        case CL_Rx:
            dataFormat = OA_TEXEL_FORMAT_RED;
            break;

        case CL_A:
            dataFormat = OA_TEXEL_FORMAT_ALPHA;
            break;

        case CL_INTENSITY:
            dataFormat = OA_TEXEL_FORMAT_INTENSITY;
            break;

        case CL_LUMINANCE:
            dataFormat = OA_TEXEL_FORMAT_LUMINANCE;
            break;

        case CL_RGB:
        case CL_RGBx:
        case CL_sRGB:
        case CL_sRGBx:
            dataFormat = OA_TEXEL_FORMAT_RGB;
            break;

        case CL_RGBA:
        case CL_sRGBA:
            dataFormat = OA_TEXEL_FORMAT_RGBA;
            break;

        case CL_BGRA:
        case CL_sBGRA:
            dataFormat = OA_TEXEL_FORMAT_BGRA;
            break;

        case CL_ARGB:
            dataFormat = OA_TEXEL_FORMAT_BGRA_REVERSED;
            break;

        case CL_ABGR:
            dataFormat = OA_TEXEL_FORMAT_RGBA_REVERSED;
            break;

        case CL_RG:
        case CL_RGx:
            dataFormat = OA_TEXEL_FORMAT_RG;
            break;

        case CL_RA:
            dataFormat = OA_TEXEL_FORMAT_RA;
            break;

        default:
        {
            GT_ASSERT_EX(false, L"Unknown data format");
            retVal = false;
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaIsTypeDisplayedUpsideDown
// Description: Are we displaying the data upside down (for data display - kernel
//              variables and data buffers- no, for all the other objects - yes)
// Return Val:  bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        22/4/2009
// ---------------------------------------------------------------------------
bool oaIsTypeDisplayedUpsideDown(oaTexelDataFormat texelDataFormat)
{
    bool retVal = true;

    if (oaIsBufferTexelFormat(texelDataFormat) || (texelDataFormat == OA_TEXEL_FORMAT_VARIABLE_VALUE))
    {
        retVal = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaCanTypeBeDisplayedAsHex
// Description: Can a texel format be displayed as hexadecimal value?
// Arguments:   oaTexelDataFormat dataFormat
// Return Val:  bool OA_API - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        16/9/2011
// ---------------------------------------------------------------------------
bool OA_API oaCanTypeBeDisplayedAsHex(oaTexelDataFormat dataFormat)
{
    bool retVal = true;

    switch (dataFormat)
    {
        case OA_TEXEL_FORMAT_FLOAT:
        case OA_TEXEL_FORMAT_FLOAT2:
        case OA_TEXEL_FORMAT_FLOAT3:
        case OA_TEXEL_FORMAT_FLOAT4:
        case OA_TEXEL_FORMAT_FLOAT8:
        case OA_TEXEL_FORMAT_FLOAT16:
        case OA_TEXEL_FORMAT_DOUBLE:
        case OA_TEXEL_FORMAT_DOUBLE2:
        case OA_TEXEL_FORMAT_DOUBLE3:
        case OA_TEXEL_FORMAT_DOUBLE4:
        case OA_TEXEL_FORMAT_DOUBLE8:
        case OA_TEXEL_FORMAT_DOUBLE16:
        case OA_TEXEL_FORMAT_V1F:
        case OA_TEXEL_FORMAT_V2F:
        case OA_TEXEL_FORMAT_V3F:
        case OA_TEXEL_FORMAT_C3F:
        case OA_TEXEL_FORMAT_C4F:
        case OA_TEXEL_FORMAT_N3F:
        case OA_TEXEL_FORMAT_T1F:
        case OA_TEXEL_FORMAT_T2F:
        case OA_TEXEL_FORMAT_T3F:
        case OA_TEXEL_FORMAT_T4F:
        case OA_TEXEL_FORMAT_I1F:
        case OA_TEXEL_FORMAT_DEPTH:
        {
            retVal = false;
            break;
        }

        default:
        {
            retVal = true;
            break;
        }
    }

    return retVal;
}



#endif // __GR_IPHONE_BUILD


