//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaTexelDataFormat.h
///
//=====================================================================

//------------------------------ oaTexelDataFormat.h ------------------------------

#ifndef __OATEXELDATAFORMAT
#define __OATEXELDATAFORMAT

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#ifndef _GR_IPHONE_BUILD
    #include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#endif // _GR_IPHONE_BUILD

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaOSAPIWrappersDLLBuild.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>

// Describes texel data format:
enum oaTexelDataFormat
{
    // Unknown / uninitialized texel format
    OA_TEXEL_FORMAT_UNKNOWN,

    // Individual channels:
    OA_TEXEL_FORMAT_STENCIL,
    OA_TEXEL_FORMAT_STENCIL_INDEX1_EXT,
    OA_TEXEL_FORMAT_STENCIL_INDEX4_EXT,
    OA_TEXEL_FORMAT_STENCIL_INDEX8_EXT,
    OA_TEXEL_FORMAT_STENCIL_INDEX16_EXT,
    OA_TEXEL_FORMAT_DEPTH,
    OA_TEXEL_FORMAT_DEPTH_COMPONENT16,
    OA_TEXEL_FORMAT_DEPTH_COMPONENT24,
    OA_TEXEL_FORMAT_DEPTH_COMPONENT32,
    OA_TEXEL_FORMAT_DEPTH_EXT,
    OA_TEXEL_FORMAT_LUMINANCE,
    OA_TEXEL_FORMAT_LUMINANCE_COMPRESSED,
    OA_TEXEL_FORMAT_COLORINDEX,
    OA_TEXEL_FORMAT_RED,
    OA_TEXEL_FORMAT_RED_COMPRESSED,
    OA_TEXEL_FORMAT_GREEN,
    OA_TEXEL_FORMAT_BLUE,
    OA_TEXEL_FORMAT_ALPHA,
    OA_TEXEL_FORMAT_ALPHA_COMPRESSED,
    OA_TEXEL_FORMAT_INTENSITY,
    OA_TEXEL_FORMAT_INTENSITY_COMPRESSED,
    OA_TEXEL_FORMAT_VARIABLE_VALUE,         // Pseudo-texels used for visualizing variable values

    // Multiple channels format:
    OA_TEXEL_FORMAT_LUMINANCEALPHA,
    OA_TEXEL_FORMAT_LUMINANCEALPHA_COMPRESSED,
    OA_TEXEL_FORMAT_RG,
    OA_TEXEL_FORMAT_RG_COMPRESSED,
    OA_TEXEL_FORMAT_RA,
    OA_TEXEL_FORMAT_RGB,
    OA_TEXEL_FORMAT_BGR,
    OA_TEXEL_FORMAT_RGB_REVERSED,
    OA_TEXEL_FORMAT_BGR_REVERSED,
    OA_TEXEL_FORMAT_RGB_COMPRESSED,
    OA_TEXEL_FORMAT_RGBA,
    OA_TEXEL_FORMAT_BGRA,
    OA_TEXEL_FORMAT_RGBA_REVERSED,
    OA_TEXEL_FORMAT_BGRA_REVERSED,
    OA_TEXEL_FORMAT_RGBA_COMPRESSED,

    // Texture integer formats:
    OA_TEXEL_FORMAT_RGB_INTEGER,
    OA_TEXEL_FORMAT_BGR_INTEGER,
    OA_TEXEL_FORMAT_RGBA_INTEGER,
    OA_TEXEL_FORMAT_BGRA_INTEGER,

    // Buffer formats:
    OA_TEXEL_FORMAT_VECTOR,
    OA_TEXEL_FORMAT_COLOR,
    OA_TEXEL_FORMAT_TEXTURE,
    OA_TEXEL_FORMAT_INDEX,
    OA_TEXEL_FORMAT_NORMAL,

    // OpenCL C primitive types:
    OA_TEXEL_FORMAT_CHAR,
    OA_TEXEL_FORMAT_UCHAR,
    OA_TEXEL_FORMAT_SHORT,
    OA_TEXEL_FORMAT_USHORT,
    OA_TEXEL_FORMAT_INT,
    OA_TEXEL_FORMAT_UINT,
    OA_TEXEL_FORMAT_LONG,
    OA_TEXEL_FORMAT_ULONG,
    OA_TEXEL_FORMAT_FLOAT,
    OA_TEXEL_FORMAT_DOUBLE,

    // OpenCL C:
    OA_TEXEL_FORMAT_CHAR2, // = OA_TEXEL_FORMAT_FIRST_CL_BUFFER_FORMAT, see below
    OA_TEXEL_FORMAT_CHAR3,
    OA_TEXEL_FORMAT_CHAR4,
    OA_TEXEL_FORMAT_CHAR8,
    OA_TEXEL_FORMAT_CHAR16,

    OA_TEXEL_FORMAT_UCHAR2,
    OA_TEXEL_FORMAT_UCHAR3,
    OA_TEXEL_FORMAT_UCHAR4,
    OA_TEXEL_FORMAT_UCHAR8,
    OA_TEXEL_FORMAT_UCHAR16,

    OA_TEXEL_FORMAT_SHORT2,
    OA_TEXEL_FORMAT_SHORT3,
    OA_TEXEL_FORMAT_SHORT4,
    OA_TEXEL_FORMAT_SHORT8,
    OA_TEXEL_FORMAT_SHORT16,

    OA_TEXEL_FORMAT_USHORT2,
    OA_TEXEL_FORMAT_USHORT3,
    OA_TEXEL_FORMAT_USHORT4,
    OA_TEXEL_FORMAT_USHORT8,
    OA_TEXEL_FORMAT_USHORT16,

    OA_TEXEL_FORMAT_INT2,
    OA_TEXEL_FORMAT_INT3,
    OA_TEXEL_FORMAT_INT4,
    OA_TEXEL_FORMAT_INT8,
    OA_TEXEL_FORMAT_INT16,

    OA_TEXEL_FORMAT_UINT2,
    OA_TEXEL_FORMAT_UINT3,
    OA_TEXEL_FORMAT_UINT4,
    OA_TEXEL_FORMAT_UINT8,
    OA_TEXEL_FORMAT_UINT16,

    OA_TEXEL_FORMAT_LONG2,
    OA_TEXEL_FORMAT_LONG3,
    OA_TEXEL_FORMAT_LONG4,
    OA_TEXEL_FORMAT_LONG8,
    OA_TEXEL_FORMAT_LONG16,

    OA_TEXEL_FORMAT_ULONG2,
    OA_TEXEL_FORMAT_ULONG3,
    OA_TEXEL_FORMAT_ULONG4,
    OA_TEXEL_FORMAT_ULONG8,
    OA_TEXEL_FORMAT_ULONG16,

    OA_TEXEL_FORMAT_FLOAT2,
    OA_TEXEL_FORMAT_FLOAT3,
    OA_TEXEL_FORMAT_FLOAT4,
    OA_TEXEL_FORMAT_FLOAT8,
    OA_TEXEL_FORMAT_FLOAT16,

    OA_TEXEL_FORMAT_DOUBLE2,
    OA_TEXEL_FORMAT_DOUBLE3,
    OA_TEXEL_FORMAT_DOUBLE4,
    OA_TEXEL_FORMAT_DOUBLE8,
    OA_TEXEL_FORMAT_DOUBLE16, // = OA_TEXEL_FORMAT_LAST_CL_BUFFER_FORMAT, see below

    // Vertex texel types:
    OA_TEXEL_FORMAT_V1F, // = OA_TEXEL_FORMAT_FIRST_BUFFER_FORMAT, OA_TEXEL_FIRST_GL_VERTEX_BUFFER_FORMAT, see below
    OA_TEXEL_FORMAT_V2F,
    OA_TEXEL_FORMAT_V3F,
    OA_TEXEL_FORMAT_V4F,
    OA_TEXEL_FORMAT_V1D,
    OA_TEXEL_FORMAT_V2D,
    OA_TEXEL_FORMAT_V3D,
    OA_TEXEL_FORMAT_V4D,
    OA_TEXEL_FORMAT_V1S,
    OA_TEXEL_FORMAT_V2S,
    OA_TEXEL_FORMAT_V3S,
    OA_TEXEL_FORMAT_V4S,
    OA_TEXEL_FORMAT_V1US,
    OA_TEXEL_FORMAT_V2US,
    OA_TEXEL_FORMAT_V3US,
    OA_TEXEL_FORMAT_V4US,
    OA_TEXEL_FORMAT_V1B,
    OA_TEXEL_FORMAT_V2B,
    OA_TEXEL_FORMAT_V3B,
    OA_TEXEL_FORMAT_V4B,
    OA_TEXEL_FORMAT_V1UB,
    OA_TEXEL_FORMAT_V2UB,
    OA_TEXEL_FORMAT_V3UB,
    OA_TEXEL_FORMAT_V4UB,
    OA_TEXEL_FORMAT_V1I,
    OA_TEXEL_FORMAT_V2I,
    OA_TEXEL_FORMAT_V3I,
    OA_TEXEL_FORMAT_V4I,
    OA_TEXEL_FORMAT_V1UI,
    OA_TEXEL_FORMAT_V2UI,
    OA_TEXEL_FORMAT_V3UI,
    OA_TEXEL_FORMAT_V4UI, // = OA_TEXEL_LAST_GL_VERTEX_BUFFER_FORMAT, see below

    // Color texel types:
    OA_TEXEL_FORMAT_C3F, // = OA_TEXEL_FIRST_GL_COLOR_BUFFER_FORMAT, see below
    OA_TEXEL_FORMAT_C4F,
    OA_TEXEL_FORMAT_C3D,
    OA_TEXEL_FORMAT_C4D,
    OA_TEXEL_FORMAT_C3S,
    OA_TEXEL_FORMAT_C4S,
    OA_TEXEL_FORMAT_C3US,
    OA_TEXEL_FORMAT_C4US,
    OA_TEXEL_FORMAT_C3B,
    OA_TEXEL_FORMAT_C4B,
    OA_TEXEL_FORMAT_C3UB,
    OA_TEXEL_FORMAT_C4UB,
    OA_TEXEL_FORMAT_C3I,
    OA_TEXEL_FORMAT_C4I,
    OA_TEXEL_FORMAT_C3UI,
    OA_TEXEL_FORMAT_C4UI, // = OA_TEXEL_LAST_GL_COLOR_BUFFER_FORMAT, see below

    // Normal texel types:
    OA_TEXEL_FORMAT_N3F, // = OA_TEXEL_FIRST_GL_NORMAL_BUFFER_FORMAT, see below
    OA_TEXEL_FORMAT_N3D,
    OA_TEXEL_FORMAT_N3I,
    OA_TEXEL_FORMAT_N3S,
    OA_TEXEL_FORMAT_N3B, // = OA_TEXEL_LAST_GL_NORMAL_BUFFER_FORMAT, see below

    // Tex coord texel types:
    OA_TEXEL_FORMAT_T1F, // = OA_TEXEL_FIRST_GL_TEXCOORD_BUFFER_FORMAT, see below
    OA_TEXEL_FORMAT_T2F,
    OA_TEXEL_FORMAT_T3F,
    OA_TEXEL_FORMAT_T4F,
    OA_TEXEL_FORMAT_T1D,
    OA_TEXEL_FORMAT_T2D,
    OA_TEXEL_FORMAT_T3D,
    OA_TEXEL_FORMAT_T4D,
    OA_TEXEL_FORMAT_T1S,
    OA_TEXEL_FORMAT_T2S,
    OA_TEXEL_FORMAT_T3S,
    OA_TEXEL_FORMAT_T4S,
    OA_TEXEL_FORMAT_T1I,
    OA_TEXEL_FORMAT_T2I,
    OA_TEXEL_FORMAT_T3I,
    OA_TEXEL_FORMAT_T4I, // = OA_TEXEL_LAST_GL_TEXCOORD_BUFFER_FORMAT, see below

    // Index texel types:
    OA_TEXEL_FORMAT_I1UI, // = OA_TEXEL_FIRST_GL_COLORINDEX_BUFFER_FORMAT, see below
    OA_TEXEL_FORMAT_I1UB,
    OA_TEXEL_FORMAT_I1US,
    OA_TEXEL_FORMAT_I1S,
    OA_TEXEL_FORMAT_I1I,
    OA_TEXEL_FORMAT_I1F,
    OA_TEXEL_FORMAT_I1D, // = OA_TEXEL_LAST_GL_COLORINDEX_BUFFER_FORMAT, see below

    // Interleaved texel types:
    OA_TEXEL_FORMAT_C4UB_V2F, // = OA_TEXEL_FIRST_GL_INTERLEAVED_BUFFER_FORMAT, see below
    OA_TEXEL_FORMAT_C4UB_V3F,
    OA_TEXEL_FORMAT_C3F_V3F,
    OA_TEXEL_FORMAT_N3F_V3F,
    OA_TEXEL_FORMAT_C4F_N3F_V3F,
    OA_TEXEL_FORMAT_T2F_V3F,
    OA_TEXEL_FORMAT_T4F_V4F,
    OA_TEXEL_FORMAT_T2F_C4UB_V3F,
    OA_TEXEL_FORMAT_T2F_C3F_V3F,
    OA_TEXEL_FORMAT_T2F_N3F_V3F,
    OA_TEXEL_FORMAT_T2F_C4F_N3F_V3F,
    OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F, // = OA_TEXEL_FORMAT_LAST_GL_BUFFER_FORMAT, OA_TEXEL_LAST_GL_INTERLEAVED_BUFFER_FORMAT, see below

    // Gathered these here to make the list cleaner:
    OA_TEXEL_FORMAT_FIRST_CL_BUFFER_FORMAT = OA_TEXEL_FORMAT_CHAR2,
    OA_TEXEL_FORMAT_LAST_CL_BUFFER_FORMAT = OA_TEXEL_FORMAT_DOUBLE16,
    OA_TEXEL_FORMAT_FIRST_GL_BUFFER_FORMAT = OA_TEXEL_FORMAT_V1F,
    OA_TEXEL_FORMAT_LAST_GL_BUFFER_FORMAT = OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F,
    OA_TEXEL_FIRST_GL_VERTEX_BUFFER_FORMAT = OA_TEXEL_FORMAT_V1F,
    OA_TEXEL_LAST_GL_VERTEX_BUFFER_FORMAT = OA_TEXEL_FORMAT_V4UI,
    OA_TEXEL_FIRST_GL_COLOR_BUFFER_FORMAT = OA_TEXEL_FORMAT_C3F,
    OA_TEXEL_LAST_GL_COLOR_BUFFER_FORMAT = OA_TEXEL_FORMAT_C4UI,
    OA_TEXEL_FIRST_GL_NORMAL_BUFFER_FORMAT = OA_TEXEL_FORMAT_N3F,
    OA_TEXEL_LAST_GL_NORMAL_BUFFER_FORMAT = OA_TEXEL_FORMAT_N3B,
    OA_TEXEL_FIRST_GL_TEXCOORD_BUFFER_FORMAT = OA_TEXEL_FORMAT_T1F,
    OA_TEXEL_LAST_GL_TEXCOORD_BUFFER_FORMAT = OA_TEXEL_FORMAT_T4I,
    OA_TEXEL_FIRST_GL_COLORINDEX_BUFFER_FORMAT = OA_TEXEL_FORMAT_I1UI,
    OA_TEXEL_LAST_GL_COLORINDEX_BUFFER_FORMAT = OA_TEXEL_FORMAT_I1D,
    OA_TEXEL_FIRST_GL_INTERLEAVED_BUFFER_FORMAT = OA_TEXEL_FORMAT_C4UB_V2F,
    OA_TEXEL_LAST_GL_INTERLEAVED_BUFFER_FORMAT = OA_TEXEL_FORMAT_T4F_C4F_N3F_V4F
};

// Calculates the amount of components of a texel data format:
int OA_API oaAmountOfTexelFormatComponents(oaTexelDataFormat texelDataFormat);

// Translates oaTexelDataFormat to its equivalent GLenum:
GLenum OA_API oaTexelDataFormatToGLEnum(oaTexelDataFormat texelDataFormat);

#ifndef _GR_IPHONE_BUILD
    // Translates cl image format to its equivalent oaTexelDataFormat:
    bool OA_API oaCLImageFormatToTexelFormat(const cl_uint& clImageFormat, oaTexelDataFormat& dataFormat);

    // Translates oaTexelDataFormat to its equivalent cl_uint constant:
    bool OA_API oaTexelDataTypeToCLEnum(oaDataType texelDataFormat, cl_uint dataForamtAsCLEnum);
#endif

// Translated texel data format to a string:
bool OA_API oaTexelDataFormatAsString(oaTexelDataFormat texelDataFormat, gtString& texelDataFormatAsString);

// Translates GLenum to its equivalent oaTexelDataFormat
bool OA_API oaGLEnumToTexelDataFormat(GLenum dataFormat, oaTexelDataFormat& texelDataFormat);

// Get channel name (Full Version or Short version) by suppling channel type (only individual channels):
bool OA_API oaGetTexelDataFormatName(oaTexelDataFormat texelDataFormat, gtString& channelName, bool shortVersion = false);

// Get channel name (Full Version or Short version) by suppling channel type (only individual channels):
bool OA_API oaGetTexelDataBufferFormatName(oaTexelDataFormat texelDataFormat, gtString& channelName, int componentIndex);

// Get the component type from the texel format in a specific index. For example:
// OA_TEXEL_FORMAT_BLUE = oaGetTexelFormatComponentType(OA_TEXEL_FORMAT_RGBA, 2)
oaTexelDataFormat OA_API oaGetTexelFormatComponentType(oaTexelDataFormat texelDataFormat, int componentIndex);

// Get the component type from the texel format in a specific index. For example:
// OA_FLOAT = oaGetTexelFormatDataType(OA_TEXEL_FORMAT_V2F, 1)
oaDataType OA_API oaGetTexelFormatDataType(oaTexelDataFormat texelDataFormat, int componentIndex);

// Calculate the size of pixel unit, given a specific data format and data type:
int OA_API oaCalculatePixelUnitByteSize(oaTexelDataFormat dataFormat, oaDataType dataType);

// Calculate the size of single chunk for a buffer texel format:
int OA_API oaCalculateChunkByteSize(oaTexelDataFormat dataFormat);

// Get the component type from the texel format in a specific index. Also get the location of the requested component
// within the specific elements vector:
bool OA_API oaGetTexelFormatBufferComponentType(oaTexelDataFormat texelDataFormat, int componentIndex, oaTexelDataFormat& componentType, int& componentLocalIndex);

// Checks if a texel data format holds a buffer data:
bool OA_API oaIsBufferTexelFormat(oaTexelDataFormat texelDataFormat);

// Get the data types contained within the texel format:
bool OA_API oaGetTexelFormatContainedDataTypes(oaTexelDataFormat texelDataFormat, gtVector<oaDataType>& dataTypes);

// Are we displaying the data type upside down?
bool OA_API oaIsTypeDisplayedUpsideDown(oaTexelDataFormat dataFormat);

// Can a texel data type be displayed as hexadecimal value:
bool OA_API oaCanTypeBeDisplayedAsHex(oaTexelDataFormat dataFormat);

#endif //__OATEXELDATAFORMAT

