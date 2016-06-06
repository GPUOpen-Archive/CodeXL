//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenGLParameters.cpp
///
//==================================================================================

//------------------------------ apParameters.cpp ------------------------------

// Standard C:
#include <stdarg.h>

// OS Definitions:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osTransferableObjectCreatorsManager.h>

// Local:
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>

// OpenGL ES is currently supported only on Windows and Mac:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

    #include <AMDTAPIClasses/Include/apGLFixedWrapper.h>

    // GLclampx type conversion constants:
    const float AP_INT_TO_CLAMPX_FACTOR = 1.0f / 255.0f;
    const float AP_CLAMPX_TO_INT_FACTOR = 255.0f;

#endif

// Calculates the amount of bits we need to shirt right in order to convert one OpenGL Data type or another:
static unsigned int statIntToByteBitShift   = (sizeof(GLuint) - sizeof(GLubyte)) * GT_BITS_PER_BYTE;
static unsigned int statShortToByteBitShift = (sizeof(GLushort) - sizeof(GLubyte)) * GT_BITS_PER_BYTE;
static unsigned int statInt64ToByteBitShift   = (sizeof(GLuint64) - sizeof(GLubyte)) * GT_BITS_PER_BYTE;

// ---------------------------- The use of stdarg ------------------------
//
//    a. We decided to use stdarg for logging the called function argument
//       values because of efficiency reasons.
//    b. In Microsoft implementation of stdarg, the C calling convention
//       force us to use:
//       - int for char, and short types
//       - double for float types
//       - pointer for array types
//       (See an appropriate comment at Microsoft stdarg.h file).
//
// ------------------------------------------------------------------------


// -----------------------------   apOpenGLParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apOpenGLParameter::isOpenGLParameter
// Description: Returns true - yes I am an OpenGL parameter.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
bool apOpenGLParameter::isOpenGLParameter() const
{
    return true;
}


// -----------------------------   apGLPrimitiveTypeParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLPrimitiveTypeParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLPrimitiveTypeParameter::type() const
{
    return OS_TOBJ_ID_GL_PRIMITIVE_TYPE_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apGLPrimitiveTypeParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLPrimitiveTypeParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLPrimitiveTypeParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLPrimitiveTypeParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 valueAsInt32 = 0;
    ipcChannel >> valueAsInt32;
    _value = (GLenum)valueAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLPrimitiveTypeParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLPrimitiveTypeParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (GLenum)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLPrimitiveTypeParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLPrimitiveTypeParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLenum*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLPrimitiveTypeParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLPrimitiveTypeParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLenum);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLPrimitiveTypeParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLPrimitiveTypeParameter::valueAsString(gtString& valueString) const
{
    switch (_value)
    {
        case GL_POINTS:
            valueString = L"GL_POINTS";
            break;

        case GL_LINES:
            valueString = L"GL_LINES";
            break;

        case GL_LINE_STRIP:
            valueString = L"GL_LINE_STRIP";
            break;

        case GL_LINE_LOOP:
            valueString = L"GL_LINE_LOOP";
            break;

        case GL_TRIANGLES:
            valueString = L"GL_TRIANGLES";
            break;

        case GL_TRIANGLE_STRIP:
            valueString = L"GL_TRIANGLE_STRIP";
            break;

        case GL_TRIANGLE_FAN:
            valueString = L"GL_TRIANGLE_FAN";
            break;

        case GL_QUADS:
            valueString = L"GL_QUADS";
            break;

        case GL_QUAD_STRIP:
            valueString = L"GL_QUAD_STRIP";
            break;

        case GL_POLYGON:
            valueString = L"GL_POLYGON";
            break;

        case GL_LINES_ADJACENCY:
            valueString = L"GL_LINES_ADJACENCY";
            break;

        case GL_LINE_STRIP_ADJACENCY:
            valueString = L"GL_LINE_STRIP_ADJACENCY";
            break;

        case GL_TRIANGLES_ADJACENCY:
            valueString = L"GL_TRIANGLES_ADJACENCY";
            break;

        case GL_TRIANGLE_STRIP_ADJACENCY:
            valueString = L"GL_TRIANGLE_STRIP_ADJACENCY";
            break;

        case GL_PATCHES:
            valueString = L"GL_PATCHES";
            break;

        default:
            // Unknown enum:
            valueString.appendFormattedString(L"Unknown (%#x)", _value);
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLPrimitiveTypeParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLPrimitiveTypeParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLPrimitiveTypeParameter* pParam  = (apGLPrimitiveTypeParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLPrimitiveTypeParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLPrimitiveTypeParameter::setValueFromInt(GLint value)
{
    _value = value;
}


// ---------------------------------------------------------------------------
// Name:        apGLPrimitiveTypeParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLPrimitiveTypeParameter::setValueFromFloat(GLfloat value)
{
    _value = GLenum(value);
}


// -----------------------------   apGLbooleanParameter ------------------------------



// ---------------------------------------------------------------------------
// Name:        apGLbooleanParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLbooleanParameter::type() const
{
    return OS_TOBJ_ID_GL_BOOL_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLbooleanParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLbooleanParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLbooleanParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLbooleanParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLbooleanParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLbooleanParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);

    if (argumentValue == 0)
    {
        _value = 0;
    }
    else
    {
        _value = 1;
    }
}


// ---------------------------------------------------------------------------
// Name:        apGLbooleanParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLbooleanParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLboolean*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLbooleanParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLbooleanParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLboolean);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLbooleanParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLbooleanParameter::valueAsString(gtString& valueString) const
{
    if (_value == 0)
    {
        valueString = AP_STR_FALSE;
    }
    else
    {
        valueString = AP_STR_TRUE;
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLbooleanParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLbooleanParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLbooleanParameter* pParam  = (apGLbooleanParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLbooleanParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLbooleanParameter::setValueFromInt(GLint value)
{
    if (value == 0)
    {
        _value = GL_FALSE;
    }
    else
    {
        _value = GL_TRUE;
    }
}


// ---------------------------------------------------------------------------
// Name:        apGLbooleanParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLbooleanParameter::setValueFromFloat(GLfloat value)
{
    if (value == 0)
    {
        _value = GL_FALSE;
    }
    else
    {
        _value = GL_TRUE;
    }
}


// -----------------------------   apGLbitfieldParameter ------------------------------



// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLbitfieldParameter::type() const
{
    return OS_TOBJ_ID_GL_BITFIELD_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLbitfieldParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLbitfieldParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (GLbitfield)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLbitfieldParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (GLbitfield)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLbitfieldParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLbitfield*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLbitfieldParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLbitfield);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLbitfieldParameter::valueAsString(gtString& valueString) const
{
    // We will display the hexadecimal value of the parameter.
    // Example: 0xFFFFDBEE
    valueString = L"";
    valueString.appendFormattedString(L"0x%X", _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLbitfieldParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLbitfieldParameter* pParam  = (apGLbitfieldParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLbitfieldParameter::setValueFromInt(GLint value)
{
    _value = (GLbitfield)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLbitfieldParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLbitfieldParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLbitfield)value;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// -----------------------------   apGLclearBitfieldParameter ------------------------------



// ---------------------------------------------------------------------------
// Name:        apGLclearBitfieldParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        16/9/2007
// ---------------------------------------------------------------------------
osTransferableObjectType apGLclearBitfieldParameter::type() const
{
    return OS_TOBJ_ID_GL_CLEAR_BITFIELD_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLclearBitfieldParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/9/2007
// ---------------------------------------------------------------------------
bool apGLclearBitfieldParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLclearBitfieldParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/9/2007
// ---------------------------------------------------------------------------
bool apGLclearBitfieldParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (GLbitfield)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLclearBitfieldParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        16/9/2007
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLclearBitfieldParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (GLbitfield)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLclearBitfieldParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        16/9/2007
// ---------------------------------------------------------------------------
void apGLclearBitfieldParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLbitfield*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLclearBitfieldParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        16/9/2007
// ---------------------------------------------------------------------------
gtSizeType apGLclearBitfieldParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLbitfield);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLclearBitfieldParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        16/9/2007
// ---------------------------------------------------------------------------
void apGLclearBitfieldParameter::valueAsString(gtString& valueString) const
{
    bool firstParam = true;

    valueString = L"";

    if (_value & GL_COLOR_BUFFER_BIT)
    {
        valueString += L"GL_COLOR_BUFFER_BIT";
        firstParam = false;
    }

    if (_value & GL_DEPTH_BUFFER_BIT)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString += L"GL_DEPTH_BUFFER_BIT";
        firstParam = false;
    }

    if (_value & GL_ACCUM_BUFFER_BIT)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString += L"GL_ACCUM_BUFFER_BIT";
        firstParam = false;
    }

    if (_value & GL_STENCIL_BUFFER_BIT)
    {
        if (!firstParam)
        {
            valueString += L" | ";
        }

        valueString += L"GL_STENCIL_BUFFER_BIT";
        firstParam = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLclearBitfieldParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLclearBitfieldParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLclearBitfieldParameter* pParam  = (apGLclearBitfieldParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLclearBitfieldParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        16/9/2007
// ---------------------------------------------------------------------------
void apGLclearBitfieldParameter::setValueFromInt(GLint value)
{
    _value = (GLbitfield)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLclearBitfieldParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        16/9/2007
// ---------------------------------------------------------------------------
void apGLclearBitfieldParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLbitfield)value;
}




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// -----------------------------   apGLbyteParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLbyteParameter::type() const
{
    return OS_TOBJ_ID_GL_BYTE_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        12/12/2007
// ---------------------------------------------------------------------------
char apGLbyteParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return _value;
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        13/12/2007
// ---------------------------------------------------------------------------
void apGLbyteParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLbyteParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLbyteParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLbyteParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long argumentValue = va_arg(pArgumentList , long);
    _value = (GLbyte)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLbyteParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLbyte*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLbyteParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLbyte);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apGLbyteParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"0x%02x", (gtUByte)_value);
}

// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLbyteParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(L"%d", _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLbyteParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLbyteParameter* pParam  = (apGLbyteParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLbyteParameter::setValueFromInt(GLint value)
{
    _value = (GLbyte)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLbyteParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLbyte)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
void apGLbyteParameter::setValueFromDouble(double value)
{
    _value = (GLbyte)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLbyteParameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
double apGLbyteParameter::valueAsDouble()
{
    return (double)_value;
}

// -----------------------------   apGLshortParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLshortParameter::type() const
{
    return OS_TOBJ_ID_GL_SHORT_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        12/12/2007
// ---------------------------------------------------------------------------
char apGLshortParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return char(_value >> statShortToByteBitShift);
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        13/12/2007
// ---------------------------------------------------------------------------
void apGLshortParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLshortParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt16)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLshortParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt16 valueAsInt16 = 0;
    ipcChannel >> valueAsInt16;
    _value = (GLshort)valueAsInt16;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLshortParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long argumentValue = va_arg(pArgumentList , long);
    _value = (GLshort)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLshortParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLshort*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLshortParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLshort);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apGLshortParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"0x%04x", (gtUInt16)_value);
}

// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLshortParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(L"%d", _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLshortParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLshortParameter* pParam  = (apGLshortParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLshortParameter::setValueFromInt(GLint value)
{
    _value = (GLshort)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLshortParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLshort)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
void apGLshortParameter::setValueFromDouble(double value)
{
    _value = (GLshort)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLshortParameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
double apGLshortParameter::valueAsDouble()
{
    return (double)_value;
}


// -----------------------------   apGLintParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLintParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLintParameter::type() const
{
    return OS_TOBJ_ID_GL_INT_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apGLintParameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        12/12/2007
// ---------------------------------------------------------------------------
char apGLintParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return char(_value >> statIntToByteBitShift);
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLintParameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        13/12/2007
// ---------------------------------------------------------------------------
void apGLintParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apGLintParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLintParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLintParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLintParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 valueAsInt32 = 0;
    ipcChannel >> valueAsInt32;
    _value = (GLint)valueAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLintParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLintParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long argumentValue = va_arg(pArgumentList , long);
    _value = (GLint)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLintParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLintParameter::readValueFromPointer(void* pValue)
{
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE))
    // Uri, 17/9/09: Mac 64-bit has a problem where glGetIntergerv takes a GLint (= long = 8 bytes) pointer as a parameter, but the output is
    // as int (= 4 bytes). This causes "x" to contain x + (y << 32), "y" to contain width + (height << 32), and "width" and "height" to contain
    // garbage data. So - we cast the pointer to an int* to overcome this:
    _value = *((int*)(pValue));
#else
    _value = *((GLint*)(pValue));
#endif
}


// ---------------------------------------------------------------------------
// Name:        apGLintParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLintParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLint);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLintParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apGLintParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apGLintParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLintParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(L"%d", _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLintParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLintParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLintParameter* pParam  = (apGLintParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLintParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLintParameter::setValueFromInt(GLint value)
{
    _value = value;
}


// ---------------------------------------------------------------------------
// Name:        apGLintParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLintParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLint)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLintParameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
void apGLintParameter::setValueFromDouble(double value)
{
    _value = (GLint)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLintParameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
double apGLintParameter::valueAsDouble()
{
    return (double)_value;
}


// -----------------------------   apGLint64Parameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apGLint64Parameter::type() const
{
    return OS_TOBJ_ID_GL_INT_64_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// ---------------------------------------------------------------------------
char apGLint64Parameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return char(_value >> statInt64ToByteBitShift);
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// ---------------------------------------------------------------------------
void apGLint64Parameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// ---------------------------------------------------------------------------
bool apGLint64Parameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt64)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// ---------------------------------------------------------------------------
bool apGLint64Parameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt64 valueAsInt64 = 0;
    ipcChannel >> valueAsInt64;
    _value = (GLint64)valueAsInt64;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLint64Parameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long long argumentValue = va_arg(pArgumentList , long long);
    _value = (GLint64)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// ---------------------------------------------------------------------------
void apGLint64Parameter::readValueFromPointer(void* pValue)
{
    _value = *((GLint64*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// ---------------------------------------------------------------------------
gtSizeType apGLint64Parameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLint64);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apGLint64Parameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_16_CHAR_FORMAT, (gtUInt64)_value);
}

// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// ---------------------------------------------------------------------------
void apGLint64Parameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(L"%lld", _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// ---------------------------------------------------------------------------
bool apGLint64Parameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLint64Parameter* pParam  = (apGLint64Parameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// ---------------------------------------------------------------------------
void apGLint64Parameter::setValueFromInt(GLint value)
{
    _value = value;
}


// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// ---------------------------------------------------------------------------
void apGLint64Parameter::setValueFromFloat(GLfloat value)
{
    _value = (GLint64)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// ---------------------------------------------------------------------------
void apGLint64Parameter::setValueFromDouble(double value)
{
    _value = (GLint64)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        15/10/2009
// ---------------------------------------------------------------------------
double apGLint64Parameter::valueAsDouble()
{
    return (double)_value;
}


// -----------------------------   apGLuint64Parameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apGLuint64Parameter::type() const
{
    return OS_TOBJ_ID_GL_UINT_64_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apGLint64Parameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
char apGLuint64Parameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return char(_value >> statInt64ToByteBitShift);
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
void apGLuint64Parameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
bool apGLuint64Parameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
bool apGLuint64Parameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 valueAsInt64 = 0;
    ipcChannel >> valueAsInt64;
    _value = (GLuint64)valueAsInt64;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLuint64Parameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long long argumentValue = va_arg(pArgumentList , long long);
    _value = (GLuint64)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
void apGLuint64Parameter::readValueFromPointer(void* pValue)
{
    _value = *((GLuint64*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
gtSizeType apGLuint64Parameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLuint64);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apGLuint64Parameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_16_CHAR_FORMAT, (gtUInt64)_value);
}

// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
void apGLuint64Parameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(L"%lld", _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
bool apGLuint64Parameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apGLuint64Parameter:
        apGLuint64Parameter* pParam  = (apGLuint64Parameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
void apGLuint64Parameter::setValueFromInt(GLint value)
{
    _value = value;
}


// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
void apGLuint64Parameter::setValueFromFloat(GLfloat value)
{
    _value = (GLuint64)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
void apGLuint64Parameter::setValueFromDouble(double value)
{
    _value = (GLuint64)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLuint64Parameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
double apGLuint64Parameter::valueAsDouble()
{
    return (double)_value;
}

// -----------------------------   apGLuint64AddressParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLuint64AddressParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        29/11/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apGLuint64AddressParameter::type() const
{
    return OS_TOBJ_ID_GL_UINT_64_ADDRESS_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apGLuint64AddressParameter::valueAsString
// Description: Returns the parameter value as a string (print the UInt64 value
//              as Hex address
// Author:  AMD Developer Tools Team
// Date:        29/11/2010
// ---------------------------------------------------------------------------
void apGLuint64AddressParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(GT_64_BIT_POINTER_FORMAT_UPPERCASE, _value);
}

// -----------------------------   apGLubyteParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLubyteParameter::type() const
{
    return OS_TOBJ_ID_GL_UBYTE_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        12/12/2007
// ---------------------------------------------------------------------------
char apGLubyteParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return _value;
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        13/12/2007
// ---------------------------------------------------------------------------
void apGLubyteParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLubyteParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLubyteParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLubyteParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (GLubyte)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLubyteParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLubyte*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLubyteParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLubyte);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apGLubyteParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"0x%02x", (gtUByte)_value);
}


// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLubyteParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(L"%u", _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLubyteParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLubyteParameter* pParam  = (apGLubyteParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLubyteParameter::setValueFromInt(GLint value)
{
    _value = (GLubyte)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLubyteParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLubyte)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
void apGLubyteParameter::setValueFromDouble(double value)
{
    _value = (GLubyte)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLubyteParameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
double apGLubyteParameter::valueAsDouble()
{
    return (double)_value;
}

// -----------------------------   apGLushortParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLushortParameter::type() const
{
    return OS_TOBJ_ID_GL_USHORT_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        12/12/2007
// ---------------------------------------------------------------------------
char apGLushortParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return char(_value >> statShortToByteBitShift);
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        13/12/2007
// ---------------------------------------------------------------------------
void apGLushortParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLushortParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt16)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLushortParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt16 valueAsUInt16 = 0;
    ipcChannel >> valueAsUInt16;
    _value = (GLushort)valueAsUInt16;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLushortParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (GLushort)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLushortParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLushort*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLushortParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLushort);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apGLushortParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"0x%04x", (gtUInt16)_value);
}

// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLushortParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(L"%u", _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLushortParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLushortParameter* pParam  = (apGLushortParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLushortParameter::setValueFromInt(GLint value)
{
    _value = (GLushort)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLushortParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLushort)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
void apGLushortParameter::setValueFromDouble(double value)
{
    _value = (GLushort)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLushortParameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
double apGLushortParameter::valueAsDouble()
{
    return (double)_value;
}


// -----------------------------   apGLuintParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLuintParameter::type() const
{
    return OS_TOBJ_ID_GL_UINT_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        12/12/2007
// ---------------------------------------------------------------------------
char apGLuintParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return char(_value >> statIntToByteBitShift);
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        13/12/2007
// ---------------------------------------------------------------------------
void apGLuintParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLuintParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLuintParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (GLuint)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLuintParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (GLuint)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLuintParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLuint*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLuintParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLuint);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apGLuintParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLuintParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(L"%u", _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLuintParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLuintParameter* pParam  = (apGLuintParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLuintParameter::setValueFromInt(GLint value)
{
    _value = (GLuint)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLuintParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLuint)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
void apGLuintParameter::setValueFromDouble(double value)
{
    _value = (GLuint)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLuintParameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
double apGLuintParameter::valueAsDouble()
{
    return (double)_value;
}


// -----------------------------   apGLsizeiParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLsizeiParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLsizeiParameter::type() const
{
    return OS_TOBJ_ID_GL_SIZEI_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLsizeiParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLsizeiParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLsizeiParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLsizeiParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 valueAsUInt64 = 0;
    ipcChannel >> valueAsUInt64;
    _value = (GLsizei)valueAsUInt64;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLsizeiParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLsizeiParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long argumentValue = va_arg(pArgumentList , long);
    _value = (GLsizei)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLsizeiParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLsizeiParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLsizei*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLsizeiParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLsizeiParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLsizei);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLsizeiParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apGLsizeiParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apGLsizeiParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLsizeiParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(L"%d", _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLsizeiParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLsizeiParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLsizeiParameter* pParam  = (apGLsizeiParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLsizeiParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLsizeiParameter::setValueFromInt(GLint value)
{
    _value = (GLsizei)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLsizeiParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLsizeiParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLsizei)value;
}



// -----------------------------   apGLfloatParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLfloatParameter::type() const
{
    return OS_TOBJ_ID_GL_FLOAT_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLfloatParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::asPixelValue
// Description: Converts the float value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        12/12/2007
// ---------------------------------------------------------------------------
char apGLfloatParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return char(_value * 255.0f);
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        13/12/2007
// ---------------------------------------------------------------------------
void apGLfloatParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    (void)(bitsMask); // unused
    (void)(amountOfBitsToShiftRight); // unused
    // Not implemented under float!
    GT_ASSERT_EX(false, L"Tried to shift and mask bits under float!");
}

// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLfloatParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLfloatParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    double argumentValue = va_arg(pArgumentList , double);
    _value = (GLfloat)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLfloatParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLfloat*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLfloatParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLfloat);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLfloatParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    const wchar_t* pFloatParamFormatString = apGetFloatParamsFormatString();
    valueString.appendFormattedString(pFloatParamFormatString, _value);
}


// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLfloatParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLfloatParameter* pParam  = (apGLfloatParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLfloatParameter::setValueFromInt(GLint value)
{
    _value = (GLfloat)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLfloatParameter::setValueFromFloat(GLfloat value)
{
    _value = value;
}

// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
void apGLfloatParameter::setValueFromDouble(double value)
{
    _value = (GLfloat)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLfloatParameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        28/12/2007
// ---------------------------------------------------------------------------
double apGLfloatParameter::valueAsDouble()
{
    return (double)_value;
}



// -----------------------------   apGLclampfParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLclampfParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLclampfParameter::type() const
{
    return OS_TOBJ_ID_GL_CLAMPF_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampfParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLclampfParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampfParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLclampfParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampfParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLclampfParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    double argumentValue = va_arg(pArgumentList , double);
    _value = (GLclampf)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampfParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLclampfParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLclampf*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLclampfParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLclampfParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLclampf);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampfParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLclampfParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    const wchar_t* pFloatParamFormatString = apGetFloatParamsFormatString();
    valueString.appendFormattedString(pFloatParamFormatString, _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLclampfParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLclampfParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLclampfParameter* pParam  = (apGLclampfParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampfParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLclampfParameter::setValueFromInt(GLint value)
{
    // GLint to GLclampf usually means color value.
    // We will move it from [0,255] to [0,1]

    // Sanity test:
    if ((0 <= value) && (value <= 255))
    {
        _value = GLclampf((GLfloat)value / 255.0);
    }
    else
    {
        // This is not a color value:
        _value = 0.0f;
        GT_ASSERT(0);
    }
}


// ---------------------------------------------------------------------------
// Name:        apGLclampfParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLclampfParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLclampf)value;
}



// -----------------------------   apGLsyncParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apGLsyncParameter::apGLsyncParameter
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        1/6/2010
// ---------------------------------------------------------------------------
apGLsyncParameter::apGLsyncParameter(oaGLSyncHandle value)
    : _value(value),
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
      _is64BitPointer(false)
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
      _is64BitPointer(true)
#else
#error Unknown address space size!
#endif
{

}

// ---------------------------------------------------------------------------
// Name:        apGLsyncParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apGLsyncParameter::type() const
{
    return OS_TOBJ_ID_GL_SYNC_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLsyncParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
bool apGLsyncParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_value;
    ipcChannel << _is64BitPointer;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLsyncParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
bool apGLsyncParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 valueAsUint64 = 0;
    ipcChannel >> valueAsUint64;
    _value = (oaGLSyncHandle)valueAsUint64;
    ipcChannel >> _is64BitPointer;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLsyncParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLsyncParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    void* argumentValue = va_arg(pArgumentList , void*);
    _value = (oaGLSyncHandle)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLsyncParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
void apGLsyncParameter::readValueFromPointer(void* pValue)
{
    _value = (oaGLSyncHandle)(*((GLsync*)(pValue)));
}


// ---------------------------------------------------------------------------
// Name:        apGLsyncParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
gtSizeType apGLsyncParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLsync);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLsyncParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
void apGLsyncParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();

    if (_is64BitPointer)
    {
        valueString.appendFormattedString(GT_64_BIT_POINTER_FORMAT_UPPERCASE, _value);
    }
    else
    {
        gtUInt32 valueAsUInt32 = (gtUInt32)_value;
        valueString.appendFormattedString(GT_32_BIT_POINTER_FORMAT_UPPERCASE, valueAsUInt32);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLsyncParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
bool apGLsyncParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apGLsyncParameter:
        apGLsyncParameter* pParam  = (apGLsyncParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLsyncParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
void apGLsyncParameter::setValueFromInt(GLint value)
{
    // This is not a color value:
    _value = (oaGLSyncHandle)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLsyncParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        27/10/2009
// ---------------------------------------------------------------------------
void apGLsyncParameter::setValueFromFloat(GLfloat value)
{
    GT_ASSERT(0);
    _value = (oaGLSyncHandle)((int)value);
}

// -----------------------------   apGLdoubleParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLdoubleParameter::type() const
{
    return OS_TOBJ_ID_GL_DOUBLE_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLdoubleParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLdoubleParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLdoubleParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    double argumentValue = va_arg(pArgumentList , double);
    _value = (GLdouble)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLdoubleParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLdouble*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLdoubleParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLdouble);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLdoubleParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    const wchar_t* pFloatParamFormatString = apGetFloatParamsFormatString();
    valueString.appendFormattedString(pFloatParamFormatString, _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLdoubleParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLdoubleParameter* pParam  = (apGLdoubleParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLdoubleParameter::setValueFromInt(GLint value)
{
    _value = (GLdouble)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLdoubleParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLdouble)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::setValueFromDouble
// Description: Set my value from a GLdouble.
// Arguments: double value
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        23/4/2009
// ---------------------------------------------------------------------------
void apGLdoubleParameter::setValueFromDouble(double value)
{
    _value = (GLdouble)value;
}



// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        23/4/2009
// ---------------------------------------------------------------------------
char apGLdoubleParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return (char)_value;
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::shiftAndMaskBits
// Description:
// Arguments: unsigned int bitsMask
//            int amountOfBitsToShiftRight
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        23/4/2009
// ---------------------------------------------------------------------------
void apGLdoubleParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    (void)(bitsMask); // unused
    (void)(amountOfBitsToShiftRight); // unused
    // Not implemented under float!
    GT_ASSERT_EX(false, L"Tried to shift and mask bits under double!");
}

// ---------------------------------------------------------------------------
// Name:        apGLdoubleParameter::valueAsDouble
// Description: Return my value as a double
// Return Val: double
// Author:  AMD Developer Tools Team
// Date:        23/4/2009
// ---------------------------------------------------------------------------
double apGLdoubleParameter::valueAsDouble()
{
    return (double)_value;
}

// -----------------------------   apGLclampdParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLclampdParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLclampdParameter::type() const
{
    return OS_TOBJ_ID_GL_CLAMPD_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampdParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLclampdParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampdParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLclampdParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampdParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLclampdParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    double argumentValue = va_arg(pArgumentList , double);
    _value = (GLclampd)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampdParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLclampdParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLclampd*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLclampdParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLclampdParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLclampd);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampdParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLclampdParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    const wchar_t* pFloatParamFormatString = apGetFloatParamsFormatString();
    valueString.appendFormattedString(pFloatParamFormatString, _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLbooleanParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLclampdParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLclampdParameter* pParam  = (apGLclampdParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampdParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLclampdParameter::setValueFromInt(GLint value)
{
    // GLint to GLclampd usually means color value.
    // We will move it from [0,255] to [0,1]

    // Sanity test:
    if ((0 <= value) && (value <= 255))
    {
        _value = (GLdouble)value / 255.0;
    }
    else
    {
        // This is not a color value:
        _value = 0.0f;
        GT_ASSERT(0);
    }
}


// ---------------------------------------------------------------------------
// Name:        apGLclampdParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLclampdParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLclampd)value;
}




// -----------------------------   apGLintptrParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLintptrParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLintptrParameter::type() const
{
    return OS_TOBJ_ID_GL_INTPTR_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLintptrParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLintptrParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLintptrParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLintptrParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 valueAsUInt64 = 0;
    ipcChannel >> valueAsUInt64;
    _value = (GLintptr)valueAsUInt64;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLintptrParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLintptrParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    int argumentValue = va_arg(pArgumentList , int);
    _value = (GLintptr)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLintptrParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLintptrParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLintptr*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLintptrParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLintptrParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLintptr);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLintptrParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLintptrParameter::valueHexAsString(gtString& valueString) const
{
    valueString.appendFormattedString(GT_32_BIT_POINTER_FORMAT_UPPERCASE, (gtInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apGLintptrParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLintptrParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(L"%d", _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLintptrParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLintptrParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLintptrParameter* pParam  = (apGLintptrParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLintptrParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLintptrParameter::setValueFromInt(GLint value)
{
    _value = (GLintptr)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLintptrParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLintptrParameter::setValueFromFloat(GLfloat value)
{
    // Should not be called:
    GT_ASSERT(0);
    _value = (GLintptr)value;
}


// -----------------------------   apGLsizeiptrParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLsizeiptrParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLsizeiptrParameter::type() const
{
    return OS_TOBJ_ID_GL_SIZEIPTR_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLsizeiptrParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLsizeiptrParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLsizeiptrParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLsizeiptrParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 valueAsUInt64 = 0;
    ipcChannel >> valueAsUInt64;
    _value = (GLsizeiptr)valueAsUInt64;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLsizeiptrParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLsizeiptrParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    int argumentValue = va_arg(pArgumentList , int);
    _value = (GLsizeiptr)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLsizeiptrParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLsizeiptrParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLsizeiptr*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLsizeiptrParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLsizeiptrParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLsizeiptr);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apGLsizeiptrParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apGLsizeiptrParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apGLsizeiptrParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLsizeiptrParameter::valueAsString(gtString& valueString) const
{
    valueString = L"";
    valueString.appendFormattedString(L"%d", _value);
}

// ---------------------------------------------------------------------------
// Name:        apGLsizeiptrParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLsizeiptrParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLsizeiptrParameter* pParam  = (apGLsizeiptrParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLsizeiptrParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLsizeiptrParameter::setValueFromInt(GLint value)
{
    _value = (GLsizeiptr)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLsizeiptrParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLsizeiptrParameter::setValueFromFloat(GLfloat value)
{
    // Should not be called:
    GT_ASSERT(0);
    _value = (GLsizeiptr)value;
}



// -----------------------------   apGLStringParameter ------------------------------



// ---------------------------------------------------------------------------
// Name:        apGLStringParameter::apGLStringParameter
// Description: Constructor
// Arguments:   len - The string length.
//              pString - The string content (Not null terminated).
// Author:  AMD Developer Tools Team
// Date:        2/2/2005
// ---------------------------------------------------------------------------
apGLStringParameter::apGLStringParameter(GLsizei len, const GLvoid* pString)
{
    setFromOpenGLString(len, pString);
}


// ---------------------------------------------------------------------------
// Name:        apGLStringParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLStringParameter::type() const
{
    return OS_TOBJ_ID_GL_STRING_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLStringParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLStringParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLStringParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLStringParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLStringParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLStringParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    // Get the string length and content:
    GLsizei len = va_arg(pArgumentList , GLsizei);
    const GLvoid* pString = va_arg(pArgumentList , GLvoid*);

    setFromOpenGLString(len, pString);
}


// ---------------------------------------------------------------------------
// Name:        apGLStringParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLStringParameter::readValueFromPointer(void* pValue)
{
    (void)(pValue); // unused
    // Should not be called !
    GT_ASSERT(0);
    _value.makeEmpty();
}


// ---------------------------------------------------------------------------
// Name:        apGLStringParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLStringParameter::sizeofData()
{
    // This function should not be called !!!
    GT_ASSERT(0);

    static gtSizeType sizeOfChar = sizeof(char);
    return sizeOfChar;
}


// ---------------------------------------------------------------------------
// Name:        apGLStringParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLStringParameter::valueAsString(gtString& valueString) const
{
    valueString.fromASCIIString(_value.asCharArray());
}

// ---------------------------------------------------------------------------
// Name:        apGLStringParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLStringParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLStringParameter* pParam  = (apGLStringParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLStringParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLStringParameter::setValueFromInt(GLint value)
{
    // Should not be called:
    GT_ASSERT(0);
    _value.makeEmpty();
    _value.appendFormattedString("%d", value);
}


// ---------------------------------------------------------------------------
// Name:        apGLStringParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLStringParameter::setValueFromFloat(GLfloat value)
{
    // Should not be called:
    GT_ASSERT(0);
    _value.makeEmpty();
    _value.appendFormattedString("%f", value);
}


// ---------------------------------------------------------------------------
// Name:        apGLStringParameter::setFromOpenGLString
// Description: Sets my content from an OpenGL string
// Arguments:   len - The string length.
//              pString - The string content (Not null terminated).
// Author:  AMD Developer Tools Team
// Date:        2/2/2005
// ---------------------------------------------------------------------------
void apGLStringParameter::setFromOpenGLString(GLsizei len, const GLvoid* pString)
{
    _value.makeEmpty();

    if (len > 0)
    {
        _value.append((const char*)pString, len);
    }
    else // len <= 0
    {
        _value.append((const char*)pString);
    }
}

// -----------------------------   apGLMultiStringParameter ------------------------------
// ---------------------------------------------------------------------------
// Name:        apGLMultiStringParameter::apGLMultiStringParameter
// Description: Constructor
// Arguments:   nstrings - The amount of input strings.
//              strings - The array of strings.
//              lengths - An array containing the string lengths.
// Author:  AMD Developer Tools Team
// Date:        2/2/2005
// ---------------------------------------------------------------------------
apGLMultiStringParameter::apGLMultiStringParameter(GLsizei nstrings, const GLcharARB* const* strings, const GLint* lengths)
{
    setGLStrings(nstrings, strings, lengths);
}


// ---------------------------------------------------------------------------
// Name:        apGLMultiStringParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLMultiStringParameter::type() const
{
    return OS_TOBJ_ID_GL_MULTI_STRING_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLMultiStringParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLMultiStringParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the amount of strings:
    gtInt64 amountOfStrings = (gtInt64)_value.size();
    ipcChannel << amountOfStrings;

    // Write the strings:
    for (int i = 0; i < amountOfStrings; i++)
    {
        ipcChannel << _value[i];
    }

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLMultiStringParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLMultiStringParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    // Clear the strings vector:
    _value.clear();

    // Read the amount of strings:
    gtInt64 amountOfStrings = 0;
    ipcChannel >> amountOfStrings;

    // Read the strings:
    for (int i = 0; i < amountOfStrings; i++)
    {
        gtASCIIString currentString;
        ipcChannel >> currentString;
        _value.push_back(currentString);
    }

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLMultiStringParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLMultiStringParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    // Get the amount of strings:
    GLsizei nstrings = va_arg(pArgumentList , int);

    // Get the strings array:
    const GLcharARB** strings = (const GLcharARB**)(va_arg(pArgumentList , void**));

    // Get the string lengths:
    const GLint* lengths = va_arg(pArgumentList , GLint*);

    // Copy the strings into this class:
    setGLStrings(nstrings, strings, lengths);
}


// ---------------------------------------------------------------------------
// Name:        apGLMultiStringParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLMultiStringParameter::readValueFromPointer(void* pValue)
{
    (void)(pValue); // unused
    // Should not be called !
    GT_ASSERT(0);
    _value.clear();
}


// ---------------------------------------------------------------------------
// Name:        apGLMultiStringParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLMultiStringParameter::sizeofData()
{
    // This function should not be called !!!
    GT_ASSERT(0);

    static gtSizeType sizeOfChar = sizeof(char);
    return sizeOfChar;
}


// ---------------------------------------------------------------------------
// Name:        apGLMultiStringParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// Implementation notes:
//  a. We concatenate all the strings into one big string.
//  b. After every original string, except the last, we add a new line
//  c. We replace every carriage return '\r' with a new line '\n'
// ---------------------------------------------------------------------------
void apGLMultiStringParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();

    int amountOfStrings = (int)_value.size();
    int lastStringIndex = amountOfStrings - 1;

    for (int i = 0; i < amountOfStrings; i++)
    {
        gtString currentString;
        currentString.fromASCIIString(_value[i].asCharArray());
        valueString += currentString;

        int currentLength = valueString.length();
        int lastCharPos = currentLength - 1;

        // If this is not the last string:
        if (i != lastStringIndex)
        {
            // If the string does not end with a new line / carriage return - add a new line:
            wchar_t lastChar = '\0';

            if (lastCharPos >= 0)
            {
                lastChar = valueString[lastCharPos];
            }

            if (!((lastChar == '\r') || (lastChar == '\n')))
            {
                valueString += '\n';
            }
        }
    }

    // TO_DO: Unicode
    // Iterate the resulting string:
    int valueStringLength = valueString.length();
    int lastCharPos = valueStringLength - 1;

    for (int i = 0; i < valueStringLength; i++)
    {
        // If the current char is a carriage return:
        if (valueString[i] == '\r')
        {
            // If we have a sequence of '\r\n' - we will replace the '\r' with a space,
            // otherwise - we will replace it with a new line:
            if ((i != lastCharPos) && (valueString[i + 1] == '\n'))
            {
                valueString[i] = ' ';
            }
            else
            {
                valueString[i] = '\n';
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        apGLMultiStringParameter::valueAsString
// Description: Returns the parameter value as an ASCII string. The function is used when
//              no conversion to unicode is needed.
// Author:  AMD Developer Tools Team
// Date:        16/9/2010
// ---------------------------------------------------------------------------
void apGLMultiStringParameter::valueAsASCIIString(gtASCIIString& valueString) const
{
    valueString.makeEmpty();

    int amountOfStrings = (int)_value.size();
    int lastStringIndex = amountOfStrings - 1;

    for (int i = 0; i < amountOfStrings; i++)
    {
        // Append the current string:
        valueString += _value[i];

        int currentLength = valueString.length();
        int lastCharPos = currentLength - 1;

        // If this is not the last string:
        if (i != lastStringIndex)
        {
            // If the string does not end with a new line / carriage return - add a new line:
            char lastChar = '\0';

            if (lastCharPos >= 0)
            {
                lastChar = valueString[lastCharPos];
            }

            if (!((lastChar == '\r') || (lastChar == '\n')))
            {
                valueString += '\n';
            }
        }
    }

    // Iterate the resulting string:
    int valueStringLength = valueString.length();
    int lastCharPos = valueStringLength - 1;

    for (int i = 0; i < valueStringLength; i++)
    {
        // If the current char is a carriage return:
        if (valueString[i] == '\r')
        {
            // If we have a sequence of '\r\n' - we will replace the '\r' with a space,
            // otherwise - we will replace it with a new line:
            if ((i != lastCharPos) && (valueString[i + 1] == '\n'))
            {
                valueString[i] = ' ';
            }
            else
            {
                valueString[i] = '\n';
            }
        }
    }

    // Make sure the string does not end if the NULL. This cause the VS to open a notepad with very small files
    if (valueString[lastCharPos] == 0)
    {
        valueString[lastCharPos] = '\n';
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLbooleanParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLMultiStringParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLMultiStringParameter* pParam  = (apGLMultiStringParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLMultiStringParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLMultiStringParameter::setValueFromInt(GLint value)
{
    // Should not be called:
    GT_ASSERT(0);

    _value.clear();
    gtASCIIString intAsString;
    intAsString.appendFormattedString("%d", value);
    _value.push_back(intAsString);
}


// ---------------------------------------------------------------------------
// Name:        apGLMultiStringParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLMultiStringParameter::setValueFromFloat(GLfloat value)
{
    // Should not be called:
    GT_ASSERT(0);

    _value.clear();
    gtASCIIString floatAsString;
    floatAsString.appendFormattedString("%f", value);
    _value.push_back(floatAsString);
}


// ---------------------------------------------------------------------------
// Name:        apGLMultiStringParameter::setGLStrings
// Description: Sets my contained strings.
// Arguments:   nstrings - The amount of input strings.
//              strings - The array of strings.
//              lengths - An array containing the string lengths.
// Author:  AMD Developer Tools Team
// Date:        29/3/2005
// ---------------------------------------------------------------------------
void apGLMultiStringParameter::setGLStrings(GLsizei nstrings, const GLcharARB* const* strings, const GLint* lengths)
{
    // Clear the strings vector:
    _value.clear();

    // Add the input strings into the vector:
    for (int i = 0; i < nstrings; i++)
    {
        gtASCIIString currentString;

        // NULL lengths array or current string length that is less than 0 means that the
        // current string is a null terminated string:
        int stringLength;

        if ((lengths == NULL) || (lengths[i] < 0))
        {
            stringLength = (int)strlen(strings[i]) + 1;
        }
        else
        {
            stringLength = lengths[i] + 1;
        }

        // Append the string:
        currentString.append(strings[i], stringLength);

        _value.push_back(currentString);
    }
}



// -----------------------------   apAssociatedTextureNamesPseudoParameter ------------------------------



// ---------------------------------------------------------------------------
// Name:        apAssociatedTextureNamesPseudoParameter::apAssociatedTextureNamesPseudoParameter
// Description: Constructor
// Arguments:   associatedTextureName - A texture name that is associated with the function call.
// Author:  AMD Developer Tools Team
// Date:        12/1/2005
// ---------------------------------------------------------------------------
apAssociatedTextureNamesPseudoParameter::apAssociatedTextureNamesPseudoParameter(GLuint associatedTextureName)
{
    // If we have an associated texture name:
    if (associatedTextureName != 0)
    {
        // Add it to the associated texture names vector:
        _associatedTextureNames.push_back(associatedTextureName);
    }
};


// ---------------------------------------------------------------------------
// Name:        apAssociatedTextureNamesPseudoParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apAssociatedTextureNamesPseudoParameter::type() const
{
    return OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedTextureNamesPseudoParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apAssociatedTextureNamesPseudoParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    gtInt64 amountOfAssociatedTextureNames = (gtInt64)_associatedTextureNames.size();
    ipcChannel << amountOfAssociatedTextureNames;

    for (int i = 0; i < amountOfAssociatedTextureNames; i++)
    {
        ipcChannel << (gtUInt32)_associatedTextureNames[i];
    }

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedTextureNamesPseudoParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apAssociatedTextureNamesPseudoParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    // Initialize all class members:
    initialize();

    // Read the amount of associated texture names:
    gtInt64 amountOfAssociatedTextureNames = 0;
    ipcChannel >> amountOfAssociatedTextureNames;

    for (int i = 0; i < amountOfAssociatedTextureNames; i++)
    {
        gtUInt32 currentTextureName = 0;
        ipcChannel >> currentTextureName;
        _associatedTextureNames.push_back((GLuint)currentTextureName);
    }

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedTextureNamesPseudoParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apAssociatedTextureNamesPseudoParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    // Initialize all class members:
    initialize();

    // Get the amount of texture names:
    int amountOfTextureNames = va_arg(pArgumentList , int);

    // Get a pointer to the "C" vector that contains the texture names:
    GLuint* pCVector = va_arg(pArgumentList , GLuint*);

    // Copy the input vector texture names into _associatedTextureNames:
    if (pCVector != NULL)
    {
        for (int i = 0; i < amountOfTextureNames; i++)
        {
            _associatedTextureNames.push_back(pCVector[i]);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedTextureNamesPseudoParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apAssociatedTextureNamesPseudoParameter::readValueFromPointer(void* pValue)
{
    (void)(pValue); // unused
    // Should not be called !!
    GT_ASSERT(0);
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedTextureNamesPseudoParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apAssociatedTextureNamesPseudoParameter::sizeofData()
{
    // Should not be called:
    return 4;
}

// ---------------------------------------------------------------------------
// Name:        apAssociatedTextureNamesPseudoParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apAssociatedTextureNamesPseudoParameter::valueAsString(gtString& valueString) const
{
    valueString = '{';

    int amountOfAssociatedTextures = (int)_associatedTextureNames.size();

    // Iterate the vector elements:
    for (int i = 0; i < amountOfAssociatedTextures; i++)
    {
        // Append the current texture name as a string:
        valueString.appendFormattedString(L"%u", _associatedTextureNames[i]);

        if (i < (amountOfAssociatedTextures - 1))
        {
            // Add separator:
            valueString += L", ";
        }
    }

    valueString += '}';
}

// ---------------------------------------------------------------------------
// Name:        apAssociatedTextureNamesPseudoParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apAssociatedTextureNamesPseudoParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apAssociatedTextureNamesPseudoParameter* pParam  = (apAssociatedTextureNamesPseudoParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            if (_associatedTextureNames.size() == pParam->_associatedTextureNames.size())
            {
                retVal = true;

                // Compare the vector items:
                for (size_t i = 0; i < _associatedTextureNames.size(); i++)
                {
                    if (_associatedTextureNames[i] != pParam->_associatedTextureNames[i])
                    {
                        retVal = false;
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apAssociatedTextureNamesPseudoParameter::initialize
// Description: Initialize this class members.
// Author:  AMD Developer Tools Team
// Date:        12/1/2005
// ---------------------------------------------------------------------------
void apAssociatedTextureNamesPseudoParameter::initialize()
{
    // Make the associated texture names empty:
    _associatedTextureNames.clear();
}


// -----------------------------   apAssociatedProgramNamePseudoParameter ------------------------------



// ---------------------------------------------------------------------------
// Name:        apAssociatedProgramNamePseudoParameter::apAssociatedProgramNamePseudoParameter
// Description: Constructor
// Arguments:   associatedProgramName - A program name that is associated with the
//                                      function call.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
apAssociatedProgramNamePseudoParameter::apAssociatedProgramNamePseudoParameter(GLuint associatedProgramName)
    : _associatedProgramName(associatedProgramName)
{
};


// ---------------------------------------------------------------------------
// Name:        apAssociatedProgramNamePseudoParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
osTransferableObjectType apAssociatedProgramNamePseudoParameter::type() const
{
    return OS_TOBJ_ID_ASSOCIATED_PROGRAM_NAME_PSEUDO_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedProgramNamePseudoParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
bool apAssociatedProgramNamePseudoParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_associatedProgramName;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedProgramNamePseudoParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
bool apAssociatedProgramNamePseudoParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 associatedProgramNameAsUInt32 = 0;
    ipcChannel >> associatedProgramNameAsUInt32;
    _associatedProgramName = (GLuint)associatedProgramNameAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedProgramNamePseudoParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apAssociatedProgramNamePseudoParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    // Read the associated program name:
    _associatedProgramName = va_arg(pArgumentList , unsigned int);
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedProgramNamePseudoParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
void apAssociatedProgramNamePseudoParameter::readValueFromPointer(void* pValue)
{
    _associatedProgramName = *((GLuint*)pValue);
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedProgramNamePseudoParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
gtSizeType apAssociatedProgramNamePseudoParameter::sizeofData()
{
    return sizeof(GLuint);
}

// ---------------------------------------------------------------------------
// Name:        apAssociatedProgramNamePseudoParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
void apAssociatedProgramNamePseudoParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%u", _associatedProgramName);
}

// ---------------------------------------------------------------------------
// Name:        apAssociatedProgramNamePseudoParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apAssociatedProgramNamePseudoParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apAssociatedProgramNamePseudoParameter* pParam  = (apAssociatedProgramNamePseudoParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_associatedProgramName == pParam->_associatedProgramName);
        }
    }

    return retVal;
}

// -----------------------------   apAssociatedShaderNamePseudoParameter ------------------------------



// ---------------------------------------------------------------------------
// Name:        apAssociatedShaderNamePseudoParameter::apAssociatedShaderNamePseudoParameter
// Description: Constructor
// Arguments:   associatedShaderName - A shader name that is associated with the
//                                      function call.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
apAssociatedShaderNamePseudoParameter::apAssociatedShaderNamePseudoParameter(GLuint associatedShaderName)
    : _associatedShaderName(associatedShaderName)
{
};


// ---------------------------------------------------------------------------
// Name:        apAssociatedShaderNamePseudoParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
osTransferableObjectType apAssociatedShaderNamePseudoParameter::type() const
{
    return OS_TOBJ_ID_ASSOCIATED_SHADER_NAME_PSEUDO_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedShaderNamePseudoParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
bool apAssociatedShaderNamePseudoParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_associatedShaderName;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedShaderNamePseudoParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
bool apAssociatedShaderNamePseudoParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 associatedShaderNameAsUInt32 = 0;
    ipcChannel >> associatedShaderNameAsUInt32;
    _associatedShaderName = (int)associatedShaderNameAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedShaderNamePseudoParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apAssociatedShaderNamePseudoParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    // Read the associated Shader name:
    _associatedShaderName = va_arg(pArgumentList , unsigned int);
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedShaderNamePseudoParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
void apAssociatedShaderNamePseudoParameter::readValueFromPointer(void* pValue)
{
    _associatedShaderName = *((GLuint*)pValue);
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedShaderNamePseudoParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
gtSizeType apAssociatedShaderNamePseudoParameter::sizeofData()
{
    return sizeof(GLuint);
}

// ---------------------------------------------------------------------------
// Name:        apAssociatedShaderNamePseudoParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        18/5/2005
// ---------------------------------------------------------------------------
void apAssociatedShaderNamePseudoParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%u", _associatedShaderName);
}


// ---------------------------------------------------------------------------
// Name:        apAssociatedShaderNamePseudoParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apAssociatedShaderNamePseudoParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apAssociatedShaderNamePseudoParameter* pParam  = (apAssociatedShaderNamePseudoParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_associatedShaderName == pParam->_associatedShaderName);
        }
    }

    return retVal;
}



// OpenGL ES is currently supported only on Windows and Mac:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

// ------------------------------- apGLfixedParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::intToFixed
// Description: Translates GLint to GLfixed.
// Author:  AMD Developer Tools Team
// Date:        1/11/2005
// ---------------------------------------------------------------------------
GLfixed apGLfixedParameter::intToFixed(GLint intNum)
{
    GLfixed retVal = intNum << AP_FIXED_POINT_SHIFT;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::fixedToInt
// Description: Translates GLfixed to GLint.
// Author:  AMD Developer Tools Team
// Date:        1/11/2005
// ---------------------------------------------------------------------------
GLint apGLfixedParameter::fixedToInt(GLfixed fixedNum)
{
    GLint retVal = fixedNum >> AP_FIXED_POINT_SHIFT;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::floatToFixed
// Description: Translates GLfixed to GLfloat.
// Author:  AMD Developer Tools Team
// Date:        1/11/2005
// ---------------------------------------------------------------------------
GLfixed apGLfixedParameter::floatToFixed(GLfloat floatNum)
{
    GLfixed retVal = (GLfixed)(floatNum * AP_FLOAT_TO_FIXED_FACTOR);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::fixedToFloat
// Description: Translates GLfloat to GLfixed.
// Author:  AMD Developer Tools Team
// Date:        1/11/2005
// ---------------------------------------------------------------------------
GLfloat apGLfixedParameter::fixedToFloat(GLfixed fixedNum)
{
    GLfloat retVal = ((float)fixedNum *  AP_FIXED_TO_FLOAT_FACTOR);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLfixedParameter::type() const
{
    return OS_TOBJ_ID_GL_FIXED_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLfixedParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLfixedParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLfixedParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long argumentValue = va_arg(pArgumentList , long);
    _value = (GLfixed)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLfixedParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLfixed*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLfixedParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLfixed);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLfixedParameter::valueAsString(gtString& valueString) const
{
    // Translate my value into a float number:
    GLfloat valAsFloat = fixedToFloat(_value);

    valueString.makeEmpty();
    valueString.appendFormattedString(L"%f", valAsFloat);
}

// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLfixedParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLfixedParameter* pParam  = (apGLfixedParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLfixedParameter::setValueFromInt(GLint value)
{
    _value = intToFixed(value);
}


// ---------------------------------------------------------------------------
// Name:        apGLfixedParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLfixedParameter::setValueFromFloat(GLfloat value)
{
    _value = floatToFixed(value);
}



// -----------------------------   apGLclampxParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apGLclampxParameter::intToClampx
// Description: Translates GLint to GLclampx.
//              We assume that the int is in the [0,255] range.
// Author:  AMD Developer Tools Team
// Date:        5/12/2005
// ---------------------------------------------------------------------------
GLclampx apGLclampxParameter::intToClampx(GLint intNum)
{
    // Translate from [0,255] to [0,1]:
    GLfloat clampedNum = float(intNum) * AP_INT_TO_CLAMPX_FACTOR;

    // Translate from float to fixed:
    GLfixed fixedNum = apGLfixedParameter::floatToFixed(clampedNum);

    // Return the result as GLclampx:
    return GLclampx(fixedNum);
}


// ---------------------------------------------------------------------------
// Name:        apGLclampxParameter::clampxToInt
// Description: Translates GLclampx to GLint.
//              We transform from [0,1] to [0,255].
// Author:  AMD Developer Tools Team
// Date:        5/12/2005
// ---------------------------------------------------------------------------
GLint apGLclampxParameter::clampxToInt(GLclampx clampxNum)
{
    // Translate from clamp to fixed:
    GLfixed fixedNum = clampxNum;

    // Translate to float number:
    GLfloat floatNum = apGLfixedParameter::fixedToFloat(fixedNum);

    // Transform from [0,1] to [0,255]:
    GLfloat transformedNum = floatNum * AP_CLAMPX_TO_INT_FACTOR;

    // Translate to int:
    GLint retVal = GLint(transformedNum);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampxParameter::floatToClampx
// Description: Translates GLclampx to GLfloat.
// Author:  AMD Developer Tools Team
// Date:        5/12/2005
// ---------------------------------------------------------------------------
GLclampx apGLclampxParameter::floatToClampx(GLfloat floatNum)
{
    // Translate to fixed number:
    GLfixed fixedNum = apGLfixedParameter::floatToFixed(floatNum);

    // Return as GLclampx (which is actually the same, but should be truncated to [0,1]:
    GLclampx retVal = GLclampx(fixedNum);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampxParameter::clampxToFloat
// Description: Translates GLclampx to GLfloat.
// Author:  AMD Developer Tools Team
// Date:        5/12/2005
// ---------------------------------------------------------------------------
GLfloat apGLclampxParameter::clampxToFloat(GLclampx clampxNum)
{
    // Translate to fixed number:
    GLfixed fixedNum = GLfixed(clampxNum);

    // Translate to float number:
    GLfloat retVal = apGLfixedParameter::fixedToFloat(fixedNum);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampxParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLclampxParameter::type() const
{
    return OS_TOBJ_ID_GL_CLAMPX_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampxParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLclampxParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampxParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLclampxParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampxParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLclampxParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    long argumentValue = va_arg(pArgumentList , long);
    _value = (GLclampx)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampxParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLclampxParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLclampx*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLclampxParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLclampxParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLclampx);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampxParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
void apGLclampxParameter::valueAsString(gtString& valueString) const
{
    // Translate my value into a float number:
    GLfloat valAsFloat = clampxToFloat(_value);

    valueString.makeEmpty();
    valueString.appendFormattedString(L"%f", valAsFloat);
}

// ---------------------------------------------------------------------------
// Name:        apGLbooleanParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLclampxParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLclampxParameter* pParam  = (apGLclampxParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLclampxParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLclampxParameter::setValueFromInt(GLint value)
{
    _value = intToClampx(value);
}


// ---------------------------------------------------------------------------
// Name:        apGLclampxParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLclampxParameter::setValueFromFloat(GLfloat value)
{
    _value = floatToClampx(value);
}


#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS



// ---------------------------------------------------------------------------
// Name:        apCreateOpenGLParameterObject
// Description: Inputs an parameter type and creates the appropriate
//              apOpenGLParameter class instance.
//              Notice that the caller is responsible for deleting the created
//              object.
// Arguments:   parameterType - The input parameter type.
// Return Val:  apOpenGLParameter* - Will get a pointer to the created apOpenGLParameter
//                                   object (or NULL in case of failure).
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
apOpenGLParameter* apCreateOpenGLParameterObject(osTransferableObjectType parameterType)
{
    apOpenGLParameter* retVal = NULL;

    // Create an apParameter object of the input type:
    apParameter* pParameter = apCreateParameterObject(parameterType);

    if (pParameter)
    {
        // Verify that it is an apOpenGLParameter:
        if (pParameter->isOpenGLParameter())
        {
            // Down cast it to apParameter (and get its memory ownership):
            retVal = (apOpenGLParameter*)(pParameter);
        }
        else
        {
            // Clean up:
            delete pParameter;
        }
    }

    return retVal;
}
