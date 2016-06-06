//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaDataType.h
///
//=====================================================================

//------------------------------ oaDataType.h ------------------------------

#ifndef __OADATATYPE_H
#define __OADATATYPE_H

// Forward declarations:
class gtString;

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIWrappersDLLBuild.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#ifndef _GR_IPHONE_BUILD
    #include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#endif // _GR_IPHONE_BUILD

// Describes data type:
enum oaDataType
{
    // Single Component data types:
    OA_UNSIGNED_BYTE,
    OA_BYTE,
    OA_UNSIGNED_CHAR,
    OA_CHAR,
    OA_UNSIGNED_SHORT,
    OA_SHORT,
    OA_UNSIGNED_INT,
    OA_INT,
    OA_UNSIGNED_LONG,
    OA_LONG,
    OA_FLOAT,
    OA_DOUBLE,

    // Multi component data types:
    OA_UNSIGNED_BYTE_3_3_2,
    OA_UNSIGNED_BYTE_2_3_3_REV,
    OA_UNSIGNED_SHORT_5_6_5,
    OA_UNSIGNED_SHORT_5_6_5_REV,
    OA_UNSIGNED_SHORT_4_4_4_4,
    OA_UNSIGNED_SHORT_4_4_4_4_REV,
    OA_UNSIGNED_SHORT_5_5_5_1,
    OA_UNSIGNED_SHORT_1_5_5_5_REV,
    OA_UNSIGNED_INT_8_8_8_8,
    OA_UNSIGNED_INT_8_8_8_8_REV,
    OA_UNSIGNED_INT_10_10_10_2,
    OA_UNSIGNED_INT_2_10_10_10_REV,

    OA_UNKNOWN_DATA_TYPE
};


// Translates GLenum to its equivalent oaDataType
bool OA_API oaGLEnumToDataType(GLenum glEnumDataType, oaDataType& dataType);

// Translates oaDataType to its equivalent GLenum
GLenum OA_API oaDataTypeToGLEnum(oaDataType dataType);

// Translates cl_uint constant to its equivalent oaDataType:
#ifndef _GR_IPHONE_BUILD
    bool OA_API oaCLImageDataTypeToOSDataType(const cl_uint& clImageDataType, oaDataType& dataType);
#endif

// Translates oaDataType to a printable string:
bool OA_API oaDataTypeAsString(oaDataType dataType, gtString& dataTypeAsString);

// Inputs a data type and returns its size, measured in bytes
int OA_API oaSizeOfDataType(oaDataType dataType);

// Inputs a data type and returns its components amount
int OA_API oaAmountComponentsInDataType(oaDataType dataType);

// Calculates the amount of bits allocated for a specific channel
int OA_API oaAmountOfComponentBits(oaDataType dataType, int channelIndex);

bool OA_API oaGetPixelSizeInBitsByOSDataType(oaDataType dataType, GLuint& pixelSize);

// Return a data size according to the GLenum representing the data type:
int OA_API oaGLEnumToDataSize(GLenum glEnumDataType);

// Return true iff the data type can be displayed as hexadecimal display:
bool OA_API oaCanBeDisplayedAsHexadecimal(GLenum dataType);

#endif //__OADATATYPE_H

