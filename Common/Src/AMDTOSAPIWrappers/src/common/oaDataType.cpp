//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaDataType.cpp
///
//=====================================================================

//------------------------------ oaDataType.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaDataType.h>

// ---------------------------------------------------------------------------
// Name:        oaDataTypeToGLEnum
// Description: Translates oaDataType to its equivalent GLenum.
// Arguments:   dataType - The input data type.
// Return Val:  GLenum - Will get the output GLenum, or GL_NONE in case of failure.
// Author:      AMD Developer Tools Team
// Date:        8/12/2007
// ---------------------------------------------------------------------------
GLenum oaDataTypeToGLEnum(oaDataType dataType)
{
    GLenum retVal = GL_NONE;

    switch (dataType)
    {
        case OA_UNSIGNED_BYTE:
            retVal = GL_UNSIGNED_BYTE;
            break;

        case OA_BYTE:
            retVal = GL_BYTE;
            break;

        case OA_UNSIGNED_SHORT:
            retVal = GL_UNSIGNED_SHORT;
            break;

        case OA_SHORT:
            retVal = GL_SHORT;
            break;

        case OA_UNSIGNED_INT:
            retVal = GL_UNSIGNED_INT;
            break;

        case OA_INT:
            retVal = GL_INT;
            break;

        case OA_FLOAT:
            retVal = GL_FLOAT;
            break;

        case OA_DOUBLE:
            retVal = GL_DOUBLE;
            break;

        case OA_UNSIGNED_BYTE_3_3_2:
            retVal = GL_UNSIGNED_BYTE_3_3_2;
            break;

        case OA_UNSIGNED_BYTE_2_3_3_REV:
            retVal = GL_UNSIGNED_BYTE_2_3_3_REV;
            break;

        case OA_UNSIGNED_SHORT_5_6_5:
            retVal = GL_UNSIGNED_SHORT_5_6_5;
            break;

        case OA_UNSIGNED_SHORT_5_6_5_REV:
            retVal = GL_UNSIGNED_SHORT_5_6_5_REV;
            break;

        case OA_UNSIGNED_SHORT_4_4_4_4:
            retVal = GL_UNSIGNED_SHORT_4_4_4_4;
            break;

        case OA_UNSIGNED_SHORT_4_4_4_4_REV:
            retVal = GL_UNSIGNED_SHORT_4_4_4_4_REV;
            break;

        case OA_UNSIGNED_SHORT_5_5_5_1:
            retVal = GL_UNSIGNED_SHORT_5_5_5_1;
            break;

        case OA_UNSIGNED_SHORT_1_5_5_5_REV:
            retVal = GL_UNSIGNED_SHORT_1_5_5_5_REV;
            break;

        case OA_UNSIGNED_INT_8_8_8_8:
            retVal = GL_UNSIGNED_INT_8_8_8_8;
            break;

        case OA_UNSIGNED_INT_8_8_8_8_REV:
            retVal = GL_UNSIGNED_INT_8_8_8_8_REV;
            break;

        case OA_UNSIGNED_INT_10_10_10_2:
            retVal = GL_UNSIGNED_INT_10_10_10_2;
            break;

        case OA_UNSIGNED_INT_2_10_10_10_REV:
            retVal = GL_UNSIGNED_INT_2_10_10_10_REV;
            break;

        default:
        {
            // Unknown buffer data format:
            gtString errString = L"Unsupported data type";
            errString.appendFormattedString(L": %d", dataType);

            GT_ASSERT_EX(false, errString.asCharArray());
            retVal = GL_NONE;
            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaDataTypeAsString
// Description: Translates oaDataType to a printable string.
// Arguments:   dataType - The input data type.
//              dataTypeAsString - The output string.
// Return Val:  bool - success / failure
// Author:      AMD Developer Tools Team
// Date:        3/12/2007
// ---------------------------------------------------------------------------
bool oaDataTypeAsString(oaDataType dataType, gtString& dataTypeAsString)
{
    bool retVal = true;

    // Clear the output string:
    dataTypeAsString.makeEmpty();

    switch (dataType)
    {
        case OA_UNSIGNED_BYTE:
            dataTypeAsString = L"OA_UNSIGNED_BYTE";
            break;

        case OA_BYTE:
            dataTypeAsString = L"OA_BYTE";
            break;

        case OA_UNSIGNED_CHAR:
            dataTypeAsString = L"OA_UNSIGNED_CHAR";
            break;

        case OA_CHAR:
            dataTypeAsString = L"OA_CHAR";
            break;

        case OA_UNSIGNED_SHORT:
            dataTypeAsString = L"OA_UNSIGNED_SHORT";
            break;

        case OA_SHORT:
            dataTypeAsString = L"OA_SHORT";
            break;

        case OA_UNSIGNED_INT:
            dataTypeAsString = L"OA_UNSIGNED_INT";
            break;

        case OA_INT:
            dataTypeAsString = L"OA_INT";
            break;

        case OA_UNSIGNED_LONG:
            dataTypeAsString = L"OA_UNSIGNED_LONG";
            break;

        case OA_LONG:
            dataTypeAsString = L"OA_LONG";
            break;

        case OA_FLOAT:
            dataTypeAsString = L"OA_FLOAT";
            break;

        case OA_DOUBLE:
            dataTypeAsString = L"OA_DOUBLE";
            break;

        case OA_UNSIGNED_BYTE_3_3_2:
            dataTypeAsString = L"OA_UNSIGNED_BYTE_3_3_2";
            break;

        case OA_UNSIGNED_BYTE_2_3_3_REV:
            dataTypeAsString = L"OA_UNSIGNED_BYTE_2_3_3_REV";
            break;

        case OA_UNSIGNED_SHORT_5_6_5:
            dataTypeAsString = L"OA_UNSIGNED_SHORT_5_6_5";
            break;

        case OA_UNSIGNED_SHORT_5_6_5_REV:
            dataTypeAsString = L"OA_UNSIGNED_SHORT_5_6_5_REV";
            break;

        case OA_UNSIGNED_SHORT_4_4_4_4:
            dataTypeAsString = L"OA_UNSIGNED_SHORT_4_4_4_4";
            break;

        case OA_UNSIGNED_SHORT_4_4_4_4_REV:
            dataTypeAsString = L"OA_UNSIGNED_SHORT_4_4_4_4_REV";
            break;

        case OA_UNSIGNED_SHORT_5_5_5_1:
            dataTypeAsString = L"OA_UNSIGNED_SHORT_5_5_5_1";
            break;

        case OA_UNSIGNED_SHORT_1_5_5_5_REV:
            dataTypeAsString = L"OA_UNSIGNED_SHORT_1_5_5_5_REV";
            break;

        case OA_UNSIGNED_INT_8_8_8_8:
            dataTypeAsString = L"OA_UNSIGNED_INT_8_8_8_8";
            break;

        case OA_UNSIGNED_INT_8_8_8_8_REV:
            dataTypeAsString = L"OA_UNSIGNED_INT_8_8_8_8_REV";
            break;

        case OA_UNSIGNED_INT_10_10_10_2:
            dataTypeAsString = L"OA_UNSIGNED_INT_10_10_10_2";
            break;

        case OA_UNSIGNED_INT_2_10_10_10_REV:
            dataTypeAsString = L"OA_UNSIGNED_INT_2_10_10_10_REV";
            break;

        default:
        {
            retVal = false;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaGLEnumToDataType
// Description: Translates GLenum to its equivalent oaDataType.
// Arguments:   glEnumDataType - The input GLenum data type.
//              dataType - Output oaDataType equivalent.
// Return Val:  bool - Convert success / failure
// Author:      AMD Developer Tools Team
// Date:        3/12/2007
// ---------------------------------------------------------------------------
bool OA_API oaGLEnumToDataType(GLenum glEnumDataType, oaDataType& dataType)
{
    bool retVal = true;

    switch (glEnumDataType)
    {
        case GL_UNSIGNED_BYTE:
            dataType = OA_UNSIGNED_BYTE;
            break;

        case GL_BYTE:
            dataType = OA_BYTE;
            break;

        case GL_UNSIGNED_SHORT:
            dataType = OA_UNSIGNED_SHORT;
            break;

        case GL_SHORT:
            dataType = OA_SHORT;
            break;

        case GL_UNSIGNED_INT:
            dataType = OA_UNSIGNED_INT;
            break;

        case GL_INT:
            dataType = OA_INT;
            break;

        case GL_FLOAT:
            dataType = OA_FLOAT;
            break;

        case GL_DOUBLE:
            dataType = OA_DOUBLE;
            break;

        case GL_UNSIGNED_BYTE_3_3_2:
            dataType = OA_UNSIGNED_BYTE_3_3_2;
            break;

        case GL_UNSIGNED_BYTE_2_3_3_REV:
            dataType = OA_UNSIGNED_BYTE_2_3_3_REV;
            break;

        case GL_UNSIGNED_SHORT_5_6_5:
            dataType = OA_UNSIGNED_SHORT_5_6_5;
            break;

        case GL_UNSIGNED_SHORT_5_6_5_REV:
            dataType = OA_UNSIGNED_SHORT_5_6_5_REV;
            break;

        case GL_UNSIGNED_SHORT_4_4_4_4:
            dataType = OA_UNSIGNED_SHORT_4_4_4_4;
            break;

        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
            dataType = OA_UNSIGNED_SHORT_4_4_4_4_REV;
            break;

        case GL_UNSIGNED_SHORT_5_5_5_1:
            dataType = OA_UNSIGNED_SHORT_5_5_5_1;
            break;

        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            dataType = OA_UNSIGNED_SHORT_1_5_5_5_REV;
            break;

        case GL_UNSIGNED_INT_8_8_8_8:
            dataType = OA_UNSIGNED_INT_8_8_8_8;
            break;

        case GL_UNSIGNED_INT_8_8_8_8_REV:
            dataType = OA_UNSIGNED_INT_8_8_8_8_REV;
            break;

        case GL_UNSIGNED_INT_10_10_10_2:
            dataType = OA_UNSIGNED_INT_10_10_10_2;
            break;

        case GL_UNSIGNED_INT_2_10_10_10_REV:
            dataType = OA_UNSIGNED_INT_2_10_10_10_REV;
            break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaAmountOfComponentBits
// Description: Calculates the amount of bits allocated for a specific channel
//              in dataTypes that contain multiple channel bit array
// Arguments:   dataType - The input data type.
//              channelIndex - The index of the channel to return the amount of bits
// Return Val:  Amount of bits allocated for the specific channel, -1 if failure
// Author:      AMD Developer Tools Team
// Date:        10/12/2007
// ---------------------------------------------------------------------------
int oaAmountOfComponentBits(oaDataType dataType, int channelIndex)
{
    int retVal = -1;

    switch (dataType)
    {
        case OA_UNSIGNED_BYTE_3_3_2:
            switch (channelIndex)
            {
                case 0: retVal = 3; break;

                case 1: retVal = 3; break;

                case 2: retVal = 2; break;

                default: retVal = -1; break;
            }

            break;

        case OA_UNSIGNED_BYTE_2_3_3_REV:
            switch (channelIndex)
            {
                case 0: retVal = 2; break;

                case 1: retVal = 3; break;

                case 2: retVal = 3; break;

                default: retVal = -1; break;
            }

            break;

        case OA_UNSIGNED_SHORT_5_6_5:
        case OA_UNSIGNED_SHORT_5_6_5_REV:
            switch (channelIndex)
            {
                case 0: retVal = 5; break;

                case 1: retVal = 6; break;

                case 2: retVal = 5; break;

                default: retVal = -1; break;
            }

            break;

        case OA_UNSIGNED_SHORT_4_4_4_4:
        case OA_UNSIGNED_SHORT_4_4_4_4_REV:
            switch (channelIndex)
            {
                case 0: retVal = 4; break;

                case 1: retVal = 4; break;

                case 2: retVal = 4; break;

                case 3: retVal = 4; break;

                default: retVal = -1; break;
            }

            break;

        case OA_UNSIGNED_SHORT_5_5_5_1:
            switch (channelIndex)
            {
                case 0: retVal = 5; break;

                case 1: retVal = 5; break;

                case 2: retVal = 5; break;

                case 3: retVal = 1; break;

                default: retVal = -1; break;
            }

            break;

        case OA_UNSIGNED_SHORT_1_5_5_5_REV:
            switch (channelIndex)
            {
                case 0: retVal = 1; break;

                case 1: retVal = 5; break;

                case 2: retVal = 5; break;

                case 3: retVal = 5; break;

                default: retVal = -1; break;
            }

            break;

        case OA_UNSIGNED_INT_8_8_8_8:
        case OA_UNSIGNED_INT_8_8_8_8_REV:
            switch (channelIndex)
            {
                case 0: retVal = 8; break;

                case 1: retVal = 8; break;

                case 2: retVal = 8; break;

                case 3: retVal = 8; break;

                default: retVal = -1; break;
            }

            break;

        case OA_UNSIGNED_INT_10_10_10_2:
            switch (channelIndex)
            {
                case 0: retVal = 10; break;

                case 1: retVal = 10; break;

                case 2: retVal = 10; break;

                case 3: retVal = 2; break;

                default: retVal = -1; break;
            }

            break;

        case OA_UNSIGNED_INT_2_10_10_10_REV:
            switch (channelIndex)
            {
                case 0: retVal = 2; break;

                case 1: retVal = 10; break;

                case 2: retVal = 10; break;

                case 3: retVal = 10; break;

                default: retVal = -1; break;
            }

            break;

        default:
        {
            GT_ASSERT(false);
            retVal = -1;
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaSizeOfDataType
// Description: Inputs a data type and returns its size, measured in bytes.
//              -1 is returned on failure.
// Arguments:   dataType - The dataType to calculate it's size
// Author:      AMD Developer Tools Team
// Date:        8/12/2007
// ---------------------------------------------------------------------------
int oaSizeOfDataType(oaDataType dataType)
{
    int retVal = -1;

    switch (dataType)
    {
        case OA_UNSIGNED_BYTE:
            retVal = sizeof(GLubyte);
            break;

        case OA_BYTE:
            retVal = sizeof(GLbyte);
            break;

        case OA_UNSIGNED_CHAR:
            retVal = sizeof(cl_uchar);
            break;

        case OA_CHAR:
            retVal = sizeof(cl_char);
            break;

        case OA_UNSIGNED_SHORT:
            retVal = sizeof(GLushort);
            break;

        case OA_SHORT:
            retVal = sizeof(GLushort);
            break;

        case OA_UNSIGNED_INT:
            retVal = sizeof(GLuint);
            break;

        case OA_INT:
            retVal = sizeof(GLint);
            break;

        case OA_UNSIGNED_LONG:
            retVal = sizeof(cl_ulong);
            break;

        case OA_LONG:
            retVal = sizeof(cl_long);
            break;

        case OA_FLOAT:
            retVal = sizeof(GLfloat);
            break;

        case OA_DOUBLE:
            retVal = sizeof(GLdouble);
            break;

        case OA_UNSIGNED_BYTE_3_3_2:
        case OA_UNSIGNED_BYTE_2_3_3_REV:
        case OA_UNSIGNED_SHORT_5_6_5:
        case OA_UNSIGNED_SHORT_5_6_5_REV:
        case OA_UNSIGNED_SHORT_4_4_4_4:
        case OA_UNSIGNED_SHORT_4_4_4_4_REV:
        case OA_UNSIGNED_SHORT_5_5_5_1:
        case OA_UNSIGNED_SHORT_1_5_5_5_REV:
        case OA_UNSIGNED_INT_8_8_8_8:
        case OA_UNSIGNED_INT_8_8_8_8_REV:
        case OA_UNSIGNED_INT_10_10_10_2:
        case OA_UNSIGNED_INT_2_10_10_10_REV:
        {
            // Get number of components in data type
            int amountOfComponents = oaAmountComponentsInDataType(dataType);
            GT_IF_WITH_ASSERT(amountOfComponents != -1)
            {
                // Loop and sum up all the bits in the data type
                int amountOfDataTypeBits = 0;

                for (int i = 0; i < amountOfComponents; i++)
                {
                    int amountOfComponentBits = oaAmountOfComponentBits(dataType, i);
                    GT_IF_WITH_ASSERT(amountOfComponentBits != -1)
                    {
                        amountOfDataTypeBits += amountOfComponentBits;
                    }
                }

                // Let's check that we have a reasonable result
                GT_IF_WITH_ASSERT(amountOfDataTypeBits % GT_BITS_PER_BYTE == 0)
                {
                    // Return the size of bytes for the data type
                    retVal = amountOfDataTypeBits / GT_BITS_PER_BYTE;
                }
            }
        }
        break;

        default:
        {
            // Unknown buffer data format:
            gtString errString = L"Unsupported data type";
            errString.appendFormattedString(L": %d", dataType);

            GT_ASSERT_EX(false, errString.asCharArray());
            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaAmountComponentsInDataType
// Description: Inputs a data type and returns its components amount
//              -1 is returned on failure.
// Arguments:   dataType - The dataType to calculate it's components amount
// Return Val:  Amount of data components, -1 If failure.
// Author:      AMD Developer Tools Team
// Date:        8/12/2007
// ---------------------------------------------------------------------------
int oaAmountComponentsInDataType(oaDataType dataType)
{
    int retVal = -1;

    switch (dataType)
    {
        case OA_UNSIGNED_BYTE:
        case OA_BYTE:
        case OA_UNSIGNED_SHORT:
        case OA_SHORT:
        case OA_UNSIGNED_INT:
        case OA_INT:
        case OA_UNSIGNED_LONG:
        case OA_LONG:
        case OA_FLOAT:
        case OA_DOUBLE:
            retVal = 1;
            break;

        case OA_UNSIGNED_BYTE_3_3_2:
        case OA_UNSIGNED_BYTE_2_3_3_REV:
        case OA_UNSIGNED_SHORT_5_6_5:
        case OA_UNSIGNED_SHORT_5_6_5_REV:
            retVal = 3;
            break;

        case OA_UNSIGNED_SHORT_4_4_4_4:
        case OA_UNSIGNED_SHORT_4_4_4_4_REV:
        case OA_UNSIGNED_SHORT_5_5_5_1:
        case OA_UNSIGNED_SHORT_1_5_5_5_REV:
        case OA_UNSIGNED_INT_8_8_8_8:
        case OA_UNSIGNED_INT_8_8_8_8_REV:
        case OA_UNSIGNED_INT_10_10_10_2:
        case OA_UNSIGNED_INT_2_10_10_10_REV:
            retVal = 4;
            break;

        default:
        {
            // Unknown buffer data format:
            gtString errString = L"Unsupported data type";
            errString.appendFormattedString(L": %d", dataType);

            GT_ASSERT_EX(false, errString.asCharArray());
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaGetPixelSizeInBitsByOSDataType
// Description: Returns a pixel size in bits according to data type
// Arguments: oaDataType dataType
//            GLuint& pixelSizeInBits
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        12/10/2008
// ---------------------------------------------------------------------------
bool oaGetPixelSizeInBitsByOSDataType(oaDataType dataType, GLuint& pixelSizeInBits)
{
    bool retVal = true;

    switch (dataType)
    {
        case OA_UNSIGNED_BYTE:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * sizeof(gtUByte);
            break;
        }

        case OA_BYTE:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * sizeof(gtByte);
            break;
        }

        case OA_UNSIGNED_SHORT:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * sizeof(unsigned short);
            break;
        }

        case OA_SHORT:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * sizeof(short);
            break;
        }

        case OA_UNSIGNED_INT:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * sizeof(unsigned int);
            break;
        }

        case OA_INT:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * sizeof(int);
            break;
        }

        case OA_UNSIGNED_LONG:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * sizeof(cl_ulong);
            break;
        }

        case OA_LONG:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * sizeof(cl_long);
            break;
        }

        case OA_FLOAT:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * sizeof(float);
            break;
        }

        case OA_DOUBLE:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * sizeof(double);
            break;
        }

        case OA_UNSIGNED_BYTE_3_3_2:
        case OA_UNSIGNED_BYTE_2_3_3_REV:
        {
            pixelSizeInBits = 8;
            break;
        }

        case OA_UNSIGNED_SHORT_5_6_5:
        case OA_UNSIGNED_SHORT_5_6_5_REV:
        case OA_UNSIGNED_SHORT_4_4_4_4:
        case OA_UNSIGNED_SHORT_4_4_4_4_REV:
        case OA_UNSIGNED_SHORT_5_5_5_1:
        {
            pixelSizeInBits = 16;
            break;
        }

        case OA_UNSIGNED_INT_8_8_8_8:
        case OA_UNSIGNED_INT_8_8_8_8_REV:
        case OA_UNSIGNED_INT_10_10_10_2:
        case OA_UNSIGNED_INT_2_10_10_10_REV:
        {
            pixelSizeInBits = 32;
            break;
        }

        default:
        {
            // Pixel size is not set yet:
            pixelSizeInBits = 0;
            retVal = false;
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaGLEnumToDataSize
// Description: Return a data size according to the GLenum representing the data type:
// Arguments: GLenum glEnumDataType
// Return Val: int - the data size
// Author:      AMD Developer Tools Team
// Date:        8/6/2009
// ---------------------------------------------------------------------------
int oaGLEnumToDataSize(GLenum glEnumDataType)
{
    int retVal = -1;

    switch (glEnumDataType)
    {
        case GL_UNSIGNED_BYTE:
            retVal = sizeof(GLubyte);
            break;

        case GL_BYTE:
            retVal = sizeof(GLbyte);
            break;

        case GL_UNSIGNED_SHORT:
            retVal = sizeof(GLushort);
            break;

        case GL_SHORT:
            retVal = sizeof(GLushort);
            break;

        case GL_UNSIGNED_INT:
            retVal = sizeof(GLuint);
            break;

        case GL_INT:
            retVal = sizeof(GLint);
            break;

        case GL_FLOAT:
            retVal = sizeof(GLfloat);
            break;

        case GL_DOUBLE:
            retVal = sizeof(GLdouble);
            break;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        // OpenGL ES is only supported on Mac and Windows:
        case GL_FIXED:
            retVal = sizeof(GLfixed);
            break;
#endif

        default:
        {
            // Unknown buffer data format:
            gtString errString = L"Unsupported data type";
            errString.appendFormattedString(L": %d", glEnumDataType);

            GT_ASSERT_EX(false, errString.asCharArray());
            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaCanBeDisplayedAsHexadecimal
// Description: Return true iff the data type can be displayed as hexadecimal
//              display
// Arguments:   GLenum dataType
// Author:      AMD Developer Tools Team
// Date:        17/4/2011
// ---------------------------------------------------------------------------
bool oaCanBeDisplayedAsHexadecimal(GLenum dataType)
{
    bool retVal = false;

    switch (dataType)
    {
        case OA_UNSIGNED_BYTE:
        case OA_BYTE:
        case OA_UNSIGNED_CHAR:
        case OA_CHAR:
        case OA_UNSIGNED_SHORT:
        case OA_SHORT:
        case OA_UNSIGNED_INT:
        case OA_INT:
        case OA_UNSIGNED_LONG:
        case OA_LONG:
        case OA_UNSIGNED_BYTE_3_3_2:
        case OA_UNSIGNED_BYTE_2_3_3_REV:
        case OA_UNSIGNED_SHORT_5_6_5:
        case OA_UNSIGNED_SHORT_5_6_5_REV:
        case OA_UNSIGNED_SHORT_4_4_4_4:
        case OA_UNSIGNED_SHORT_4_4_4_4_REV:
        case OA_UNSIGNED_SHORT_5_5_5_1:
        case OA_UNSIGNED_SHORT_1_5_5_5_REV:
        case OA_UNSIGNED_INT_8_8_8_8:
        case OA_UNSIGNED_INT_8_8_8_8_REV:
        case OA_UNSIGNED_INT_10_10_10_2:
        case OA_UNSIGNED_INT_2_10_10_10_REV:
            retVal = true;
            break;


        default:
        {
            retVal = false;
            break;
        }
    }

    return retVal;
}

#ifndef _GR_IPHONE_BUILD

// ---------------------------------------------------------------------------
// Name:        oaCLImageDataTypeToOSDataType
// Description: Translate OpenCL data type to our data type
// Arguments:   cl_uint clImageDataType - the CL data type constant
//              oaTexelDataFormat& dataFormat - the data format in our format
// Return Val:  bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/7/2010
// ---------------------------------------------------------------------------
bool oaCLImageDataTypeToOSDataType(const cl_uint& clImageDataType, oaDataType& dataType)
{
    bool retVal = true;

    switch (clImageDataType)
    {
        // 8-bit normalized signed integer value:
        case CL_SNORM_INT8:
        {
            dataType = OA_BYTE;
        }
        break;

        // 16-bit normalized signed integer value:
        case CL_SNORM_INT16:
        {
            dataType = OA_SHORT;
        }
        break;

        // 8-bit normalized unsigned integer value:
        case CL_UNORM_INT8:
        {
            dataType = OA_UNSIGNED_BYTE;
        }
        break;

        // 16-bit normalized unsigned integer value:
        case CL_UNORM_INT16:
        {
            dataType = OA_UNSIGNED_SHORT;
        }
        break;

        // A normalized unsigned 16-bit integer, divided to three channels:
        case CL_UNORM_SHORT_565:
        {
            dataType = OA_UNSIGNED_SHORT_5_6_5;
        }
        break;

        // A normalized unsigned 16-bit integer, divided to three channels and a 1-bit data channel:
        case CL_UNORM_SHORT_555:
        {
            dataType = OA_UNSIGNED_SHORT_5_5_5_1;
        }
        break;

        // A normalized unsigned 32-bit integer, divided to three channels and a 2-bit data channel:
        case CL_UNORM_INT_101010:
        {
            dataType = OA_UNSIGNED_INT_10_10_10_2;
        }
        break;

        // 8-bit unnormalized signed integer value:
        case CL_SIGNED_INT8:
        {
            dataType = OA_BYTE;
        }
        break;

        // 16-bit unnormalized signed integer value:
        case CL_SIGNED_INT16:
        {
            dataType = OA_SHORT;
        }
        break;

        // 32-bit unnormalized signed integer value:
        case CL_SIGNED_INT32:
        {
            dataType = OA_INT;
        }
        break;

        // 8-bit unnormalized unsigned integer value
        case CL_UNSIGNED_INT8:
        {
            dataType = OA_UNSIGNED_BYTE;
        }
        break;

        // 16-bit unnormalized unsigned integer value:
        case CL_UNSIGNED_INT16:
        {
            dataType = OA_UNSIGNED_SHORT;
        }
        break;

        // 32-bit unnormalized unsigned integer value:
        case CL_UNSIGNED_INT32:
        {
            dataType = OA_UNSIGNED_INT;
        }
        break;

        // Floating point value:
        case CL_FLOAT:
        {
            dataType = OA_FLOAT;
        }
        break;

        // TO_DO: OpenCL buffers - support this data formats?
        case CL_HALF_FLOAT:
        {
            GT_ASSERT_EX(false, L"Unsupported data format");
            retVal = false;
        }
        break;
    }

    return retVal;
}


#endif // __GR_IPHONE_BUILD
