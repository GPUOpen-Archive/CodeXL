//------------------------------ apBufferDataFormat.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <inc/apStringConstants.h>
#include <AMDTAPIClasses/Include/apBufferDataFormat.h>


// ---------------------------------------------------------------------------
// Name:        apAmountOfDataComponentsPerPixel
// Description: Calculates the amount of data components that holds a single
//              pixels data.
// Return Val: int - Will get the calculated data components amount, or -1
//                   in case of failure.
// Author:      Yaki Tebeka
// Date:        9/7/2006
// ---------------------------------------------------------------------------
int apAmountOfDataComponentsPerPixel(apBufferDataFormat bufferDataFormat)
{
    int retVal = -1;

    switch (bufferDataFormat)
    {
        case AP_DEPTH_COMPONENT:
            retVal = 1;
            break;

        case AP_STENCIL_INDEX:
            retVal = 1;
            break;

        case AP_RGBA:
            retVal = 4;
            break;

        case AP_RGB:
            retVal = 3;
            break;

        default:
        {
            // Unknown buffer data format:
            gtString errString = AP_STR_unknownBufferDataFormat;
            errString.appendFormattedString(": %d", bufferDataFormat);
            GT_ASSERT_EX(false, errString.asCharArray());
            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apBufferDataFormatToGLEnum
// Description: Translates apBufferDataFormat to its equivalent GLenum.
// Arguments: bufferDataFormat - The input buffer data format.
// Return Val: GLenum  - Will get the output GLenum or GL_NONE on failure.
// Author:      Yaki Tebeka
// Date:        9/7/2006
// ---------------------------------------------------------------------------
GLenum apBufferDataFormatToGLEnum(apBufferDataFormat bufferDataFormat)
{
    GLenum retVal = GL_NONE;

    switch (bufferDataFormat)
    {
        case AP_DEPTH_COMPONENT:
            retVal = GL_DEPTH_COMPONENT;
            break;

        case AP_STENCIL_INDEX:
            retVal = GL_STENCIL_INDEX;
            break;

        case AP_RGBA:
            retVal = GL_RGBA;
            break;

        case AP_RGB:
            retVal = GL_RGB;
            break;

        default:
        {
            // Unknown buffer data format:
            gtString errString = AP_STR_unknownBufferDataFormat;
            errString.appendFormattedString(": %d", bufferDataFormat);
            GT_ASSERT_EX(false, errString.asCharArray());
        }
        break;
    }

    return retVal;
}
