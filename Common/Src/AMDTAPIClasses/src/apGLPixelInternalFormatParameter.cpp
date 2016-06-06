//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLPixelInternalFormatParameter.cpp
///
//==================================================================================

//------------------------------ apGLPixelInternalFormatParameter.cpp ------------------------------

// Standard C:
#include <stdarg.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apGLPixelInternalFormatParameter.h>


// ---------------------------------------------------------------------------
// Name:        apGLPixelInternalFormatParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLPixelInternalFormatParameter::type() const
{
    return OS_TOBJ_ID_GL_PIXEL_INTERNAL_FORMAT_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLPixelInternalFormatParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLPixelInternalFormatParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLPixelInternalFormatParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLPixelInternalFormatParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 valueAsInt32 = 0;
    ipcChannel >> valueAsInt32;
    _value = (GLenum)valueAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLPixelInternalFormatParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLPixelInternalFormatParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (GLenum)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLPixelInternalFormatParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLPixelInternalFormatParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLenum*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLPixelInternalFormatParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLPixelInternalFormatParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLenum);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLPixelInternalFormatParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apGLPixelInternalFormatParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}


// ---------------------------------------------------------------------------
// Name:        apGLPixelInternalFormatParameter::valueAsString
// Description: Returns the Internal Pixel Format parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        16/9/2007
// ---------------------------------------------------------------------------
void apGLPixelInternalFormatParameter::valueAsString(gtString& valueString) const
{
    // Check if the format is a number (see OpenGL 2.1 spec, section 3.8.1, page 152):
    // "internalformat may (for backwards compatibility with the 1.0 version of the GL)
    //  also take on the integer values 1, 2, 3, and 4, which are equivalent to symbolic
    //  constants LUMINANCE, LUMINANCE ALPHA, RGB, and RGBA respectively."
    switch (_value)
    {
        case GL_NONE:
        {
            // Internal format is unknown
            valueString = L"Unknown";
        }
        break;

        case 1:
            valueString = L"1 (Luminance)";
            break;

        case 2:
            valueString = L"2 (Luminance Alpha)";
            break;

        case 3:
            valueString = L"3 (RGB)";
            break;

        case 4:
            valueString = L"4 (RGBA)";
            break;

        default:
        {
            // Return the GLEnum as string
            apGLenumParameter internalFormat(_value);
            internalFormat.valueAsString(valueString);
        }
        break;
    }
}


// ---------------------------------------------------------------------------
// Name:        apGLPixelInternalFormatParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLPixelInternalFormatParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLPixelInternalFormatParameter* pParam  = (apGLPixelInternalFormatParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLPixelInternalFormatParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLPixelInternalFormatParameter::setValueFromInt(GLint value)
{
    _value = (GLenum)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLPixelInternalFormatParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLPixelInternalFormatParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLenum)value;
}
