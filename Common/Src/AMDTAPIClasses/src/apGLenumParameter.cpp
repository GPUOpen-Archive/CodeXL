//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLenumParameter.cpp
///
//==================================================================================

//------------------------------ apGLenumParameter.cpp ------------------------------

// Standard C:
#include <stdarg.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apGLenumParameter.h>

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define _TEXT(x) L ## x
#endif

// The function that translate the gl enumeration to string contain many simple cases
// This macro spare the need to write those 3 line again and again:
#define AP_GL_ENUM_TOSTRING_CASE(enumExpression) \
    case enumExpression: \
    {   \
        valueString = _TEXT(# enumExpression) ;\
        break;  \
    }


// ---------------------------------------------------------------------------
// Name:        apGLenumParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLenumParameter::type() const
{
    return OS_TOBJ_ID_GL_ENUM_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apGLenumParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLenumParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLenumParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/4/2004
// ---------------------------------------------------------------------------
bool apGLenumParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 valueAsInt32 = 0;
    ipcChannel >> valueAsInt32;
    _value = (GLenum)valueAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLenumParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        5/5/2004
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apGLenumParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    unsigned long argumentValue = va_arg(pArgumentList , unsigned long);
    _value = (GLenum)argumentValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLenumParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
void apGLenumParameter::readValueFromPointer(void* pValue)
{
    _value = *((GLenum*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apGLenumParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        28/7/2004
// ---------------------------------------------------------------------------
gtSizeType apGLenumParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(GLenum);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apGLenumParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        12/4/2004
// Implementation notes:
//
//  Duplicated enumerations problem:
//  -------------------------------
//  OpenGL enumerators are not as clean as we expect them to be:
//  a. The values 0 is defined for: GL_NO_ERROR, GL_NONE, GL_ZERO, GL_FALSE, GL_POINTS.
//  b. The value 1 is defined for: GL_TRUE, GL_ONE, GL_LINES.
//  c. Texture coordinates values: GL_S, GL_T, GL_R, GL_Q are also used by extensions
//     enumerators (WGL_NUMBER_PIXEL_FORMATS_ARB, WGL_DRAW_TO_WINDOW_ARB,
//     WGL_DRAW_TO_BITMAP_ARB, WGL_ACCELERATION_ARB).
//  d. WGL_I3D_image_buffer enumerators values are 1 and 2 !!!
//
//  Solving the duplicated enumerations problem:
//  --------------------------------------------
//  a. and b. are solved by:
//     - Displaying 0 and 1 as the string value.
//     - Using an apGLPrimitiveTypeParameter for OpenGL primitive types.
//  c. Texture coordinates are handled using gsTextureCoordinateString().
//  d. No solution yet (since we currently don't support the WGL_I3D_image_buffer extension).
// ---------------------------------------------------------------------------
void apGLenumParameter::valueAsString(gtString& valueString) const
{
    // Call the function for enum - string translation:
    bool rc = apGLenumValueToString(_value, valueString);
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        apGLenumParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool apGLenumParameter::compareToOther(const apParameter& other) const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apGLenumParameter* pParam  = (apGLenumParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLenumParameter::setValueFromInt
// Description: Set my value from a GLint.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLenumParameter::setValueFromInt(GLint value)
{
    _value = (GLenum)value;
}


// ---------------------------------------------------------------------------
// Name:        apGLenumParameter::setValueFromFloat
// Description: Set my value from a GLfloat.
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLenumParameter::setValueFromFloat(GLfloat value)
{
    _value = (GLenum)value;
}

// ---------------------------------------------------------------------------
// Name:        apGLenumParameter::apGLenumValueToString
// Description: Translate a GLenum value to a string
// Arguments:   GLenum enumValue - the requested enum value
//              gtString& valueString - the enum as string
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        5/12/2010
// ---------------------------------------------------------------------------
bool apGLenumValueToString(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;

    // Check if this is a 1.0* enumeration:
    retVal = checkOpenGL1Enum(enumValue, valueString);

    if (!retVal)
    {
        // Check if this is a 2.0* enumeration:
        retVal = checkOpenGL2Enum(enumValue, valueString);
    }

    if (!retVal)
    {
        // Check if this is a 3.0* enumeration:
        retVal = checkOpenGL3Enum(enumValue, valueString);
    }

    if (!retVal)
    {
        // Check if this is a 4.0* enumeration:
        retVal = checkOpenGL4Enum(enumValue, valueString);
    }

    if (!retVal)
    {
        // Check if this is an OpenGL ES enumeration:
        retVal = checkOpenGLESEnum(enumValue, valueString);
    }

    if (!retVal)
    {
        // Check if this is an WGL enumeration:
        retVal = checkWGLExtensionsEnum(enumValue, valueString);
    }

    if (!retVal)
    {
        // Check if this is an OpenGL ARB extensions enumeration:
        retVal = checkOpenGLARBExtensionsEnum(enumValue, valueString);
    }

    if (!retVal)
    {
        // Check if this is an OpenGL EXT extensions enumeration:
        retVal = checkOpenGLEXTExtensionsEnum(enumValue, valueString);
    }

    if (!retVal)
    {
        // Check if this is an OpenGL AMD extensions enumeration:
        retVal = checkOpenGLAMDExtensionsEnum(enumValue, valueString);
    }

    if (!retVal)
    {
        // Check if this is an OpenGL Apple extensions enumeration:
        retVal = checkOpenGLAppleExtensionsEnum(enumValue, valueString);
    }

    if (!retVal)
    {
        // Check if this is an OpenGL EXT extensions enumeration:
        retVal = checkOpenGLSGISExtensionsEnum(enumValue, valueString);
    }

    if (!retVal)
    {
        // Check if this is an OpenGL HP extensions enumeration:
        retVal = checkOpenGLHPExtensionsEnum(enumValue, valueString);
    }

    if (!retVal)
    {
        // Check if this is an OpenGL NV extensions enumeration:
        retVal = checkOpenGLNVExtensionsEnum(enumValue, valueString);
    }

    if (!retVal)
    {
        // Check if this is an OpenGL extensions enumeration:
        retVal = checkOpenGLOtherExtensionsEnum(enumValue, valueString);
    }

    if (!retVal)
    {
        // Unknown enum
        // (Usually because of the usage of an OpenGL extension that we don't support)
        gtString unknownEnumString;
        unknownEnumString.appendFormattedString(L"Unknown enum: 0x%X", enumValue);
        GT_ASSERT_EX(0, unknownEnumString.asCharArray());

        valueString.makeEmpty().appendFormattedString(L"Unknown (0x%X)", enumValue);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        checkOpenGL1Enum
// Description: Check if this is an OpenGL 1.0* enum:
// Arguments:   GLenum enumValue
//              gtString& valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkOpenGL1Enum(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;

    switch (enumValue)
    {
        case 0:
            // 0 is the value of the following enumerations:
            // GL_NO_ERROR, GL_NONE, GL_ZERO, GL_FALSE and GL_POINTS.
            // (GL_POINTS - Is treated using apGLPrimitiveTypeParameter).
            valueString = L"0";
            break;

        case 1:
            // 1 is the value of the following enumerations:
            // GL_TRUE, GL_ONE, GL_LINES.
            // (GL_LINES - Is treated using apGLPrimitiveTypeParameter).
            valueString = L"1";
            break;

            // OpenGL 1.0 and 1.1 enumerators:
            AP_GL_ENUM_TOSTRING_CASE(GL_2D);
            AP_GL_ENUM_TOSTRING_CASE(GL_2_BYTES);
            AP_GL_ENUM_TOSTRING_CASE(GL_3D);
            AP_GL_ENUM_TOSTRING_CASE(GL_3D_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_3D_COLOR_TEXTURE);
            AP_GL_ENUM_TOSTRING_CASE(GL_3_BYTES);
            AP_GL_ENUM_TOSTRING_CASE(GL_4D_COLOR_TEXTURE);
            AP_GL_ENUM_TOSTRING_CASE(GL_4_BYTES);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACCUM);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACCUM_BLUE_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACCUM_CLEAR_VALUE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACCUM_GREEN_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACCUM_RED_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_ADD);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALL_ATTRIB_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA12);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA16);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA4);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA8);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA_TEST);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA_TEST_FUNC);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA_TEST_REF);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALWAYS);
            AP_GL_ENUM_TOSTRING_CASE(GL_AMBIENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_AMBIENT_AND_DIFFUSE);
            AP_GL_ENUM_TOSTRING_CASE(GL_AND);
            AP_GL_ENUM_TOSTRING_CASE(GL_AND_INVERTED);
            AP_GL_ENUM_TOSTRING_CASE(GL_AND_REVERSE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATTRIB_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_AUTO_NORMAL);
            AP_GL_ENUM_TOSTRING_CASE(GL_AUX0);
            AP_GL_ENUM_TOSTRING_CASE(GL_AUX1);
            AP_GL_ENUM_TOSTRING_CASE(GL_AUX2);
            AP_GL_ENUM_TOSTRING_CASE(GL_AUX3);
            AP_GL_ENUM_TOSTRING_CASE(GL_AUX_BUFFERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_BACK);
            AP_GL_ENUM_TOSTRING_CASE(GL_BACK_LEFT);
            AP_GL_ENUM_TOSTRING_CASE(GL_BACK_RIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_BITMAP);
            AP_GL_ENUM_TOSTRING_CASE(GL_BITMAP_TOKEN);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLEND);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLEND_DST);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLEND_SRC);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLUE);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLUE_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLUE_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLUE_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_BYTE);
            AP_GL_ENUM_TOSTRING_CASE(GL_C3F_V3F);
            AP_GL_ENUM_TOSTRING_CASE(GL_C4F_N3F_V3F);
            AP_GL_ENUM_TOSTRING_CASE(GL_C4UB_V2F);
            AP_GL_ENUM_TOSTRING_CASE(GL_C4UB_V3F);
            AP_GL_ENUM_TOSTRING_CASE(GL_CCW);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLAMP);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLEAR);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIENT_ATTRIB_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_PLANE0);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_PLANE1);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_PLANE2);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_PLANE3);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_PLANE4);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_PLANE5);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_DISTANCE6);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_DISTANCE7);
            AP_GL_ENUM_TOSTRING_CASE(GL_COEFF);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ARRAY_COUNT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ARRAY_POINTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ARRAY_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ARRAY_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ARRAY_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_CLEAR_VALUE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_INDEXES);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_LOGIC_OP);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_MATERIAL);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_MATERIAL_FACE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_MATERIAL_PARAMETER);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_WRITEMASK);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPILE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPILE_AND_EXECUTE);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONSTANT_ATTENUATION);
            AP_GL_ENUM_TOSTRING_CASE(GL_COPY);
            AP_GL_ENUM_TOSTRING_CASE(GL_COPY_INVERTED);
            AP_GL_ENUM_TOSTRING_CASE(GL_COPY_PIXEL_TOKEN);
            AP_GL_ENUM_TOSTRING_CASE(GL_CULL_FACE);
            AP_GL_ENUM_TOSTRING_CASE(GL_CULL_FACE_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_NORMAL);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_RASTER_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_RASTER_DISTANCE);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_RASTER_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_RASTER_POSITION);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_RASTER_POSITION_VALID);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_RASTER_TEXTURE_COORDS);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_TEXTURE_COORDS);
            AP_GL_ENUM_TOSTRING_CASE(GL_CW);
            AP_GL_ENUM_TOSTRING_CASE(GL_DECAL);
            AP_GL_ENUM_TOSTRING_CASE(GL_DECR);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_CLEAR_VALUE);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_COMPONENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_FUNC);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_RANGE);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_TEST);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_WRITEMASK);
            AP_GL_ENUM_TOSTRING_CASE(GL_DIFFUSE);
            AP_GL_ENUM_TOSTRING_CASE(GL_DITHER);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOMAIN);
            AP_GL_ENUM_TOSTRING_CASE(GL_DONT_CARE);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLEBUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_PIXEL_TOKEN);
            AP_GL_ENUM_TOSTRING_CASE(GL_DST_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_DST_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_EDGE_FLAG);
            AP_GL_ENUM_TOSTRING_CASE(GL_EDGE_FLAG_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_EDGE_FLAG_ARRAY_COUNT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_EDGE_FLAG_ARRAY_POINTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_EDGE_FLAG_ARRAY_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_EMISSION);
            AP_GL_ENUM_TOSTRING_CASE(GL_EQUAL);
            AP_GL_ENUM_TOSTRING_CASE(GL_EQUIV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EXP);
            AP_GL_ENUM_TOSTRING_CASE(GL_EXP2);
            AP_GL_ENUM_TOSTRING_CASE(GL_EXTENSIONS);
            AP_GL_ENUM_TOSTRING_CASE(GL_EYE_LINEAR);
            AP_GL_ENUM_TOSTRING_CASE(GL_EYE_PLANE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FASTEST);
            AP_GL_ENUM_TOSTRING_CASE(GL_FEEDBACK);
            AP_GL_ENUM_TOSTRING_CASE(GL_FEEDBACK_BUFFER_POINTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_FEEDBACK_BUFFER_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FEEDBACK_BUFFER_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FILL);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_DENSITY);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_END);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_HINT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_START);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRONT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRONT_AND_BACK);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRONT_FACE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRONT_LEFT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRONT_RIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEQUAL);
            AP_GL_ENUM_TOSTRING_CASE(GL_GREATER);
            AP_GL_ENUM_TOSTRING_CASE(GL_GREEN);
            AP_GL_ENUM_TOSTRING_CASE(GL_GREEN_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_GREEN_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_GREEN_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INCR);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_ARRAY_COUNT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_ARRAY_POINTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_ARRAY_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_ARRAY_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_CLEAR_VALUE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_LOGIC_OP);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_SHIFT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_WRITEMASK);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY12);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY16);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY4);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY8);
            AP_GL_ENUM_TOSTRING_CASE(GL_INVALID_ENUM);
            AP_GL_ENUM_TOSTRING_CASE(GL_INVALID_OPERATION);
            AP_GL_ENUM_TOSTRING_CASE(GL_INVALID_VALUE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INVERT);
            AP_GL_ENUM_TOSTRING_CASE(GL_KEEP);
            AP_GL_ENUM_TOSTRING_CASE(GL_LEFT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LEQUAL);
            AP_GL_ENUM_TOSTRING_CASE(GL_LESS);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT0);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT1);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT2);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT3);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT4);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT5);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT6);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT7);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHTING);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT_MODEL_AMBIENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT_MODEL_LOCAL_VIEWER);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT_MODEL_TWO_SIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINEAR);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINEAR_ATTENUATION);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINEAR_MIPMAP_LINEAR);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINEAR_MIPMAP_NEAREST);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE_LOOP);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE_RESET_TOKEN);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE_SMOOTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE_SMOOTH_HINT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE_STIPPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE_STIPPLE_PATTERN);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE_STIPPLE_REPEAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE_STRIP);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE_TOKEN);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE_WIDTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE_WIDTH_GRANULARITY);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE_WIDTH_RANGE);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIST_BASE);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIST_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIST_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_LOAD);
            AP_GL_ENUM_TOSTRING_CASE(GL_LOGIC_OP_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE12);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE12_ALPHA12);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE12_ALPHA4);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE16);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE16_ALPHA16);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE4);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE4_ALPHA4);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE6_ALPHA2);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE8);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE8_ALPHA8);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_COLOR_4);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_GRID_DOMAIN);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_GRID_SEGMENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_NORMAL);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_TEXTURE_COORD_1);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_TEXTURE_COORD_2);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_TEXTURE_COORD_3);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_TEXTURE_COORD_4);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_3);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_4);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_COLOR_4);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_GRID_DOMAIN);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_GRID_SEGMENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_NORMAL);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_TEXTURE_COORD_1);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_TEXTURE_COORD_2);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_TEXTURE_COORD_3);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_TEXTURE_COORD_4);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_3);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_4);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP_STENCIL);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_ATTRIB_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_CLIP_PLANES);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_EVAL_ORDER);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_LIGHTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_LIST_NESTING);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_MODELVIEW_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_NAME_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PIXEL_MAP_TABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROJECTION_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TEXTURE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TEXTURE_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VIEWPORT_DIMS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW_MATRIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODULATE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MULT);
            AP_GL_ENUM_TOSTRING_CASE(GL_N3F_V3F);
            AP_GL_ENUM_TOSTRING_CASE(GL_NAME_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_NAND);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEAREST);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEAREST_MIPMAP_LINEAR);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEAREST_MIPMAP_NEAREST);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEVER);
            AP_GL_ENUM_TOSTRING_CASE(GL_NICEST);
            AP_GL_ENUM_TOSTRING_CASE(GL_NOOP);
            AP_GL_ENUM_TOSTRING_CASE(GL_NOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMALIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMAL_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMAL_ARRAY_COUNT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMAL_ARRAY_POINTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMAL_ARRAY_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMAL_ARRAY_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_NOTEQUAL);
            AP_GL_ENUM_TOSTRING_CASE(GL_OBJECT_LINEAR);
            AP_GL_ENUM_TOSTRING_CASE(GL_OBJECT_PLANE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ONE_MINUS_DST_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_ONE_MINUS_DST_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_ONE_MINUS_SRC_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_ONE_MINUS_SRC_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_OR);
            AP_GL_ENUM_TOSTRING_CASE(GL_ORDER);
            AP_GL_ENUM_TOSTRING_CASE(GL_OR_INVERTED);
            AP_GL_ENUM_TOSTRING_CASE(GL_OR_REVERSE);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUT_OF_MEMORY);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_ALIGNMENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_LSB_FIRST);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_ROW_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_SKIP_PIXELS);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_SKIP_ROWS);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_SWAP_BYTES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PASS_THROUGH_TOKEN);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERSPECTIVE_CORRECTION_HINT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_A_TO_A);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_A_TO_A_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_B_TO_B);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_B_TO_B_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_G_TO_G);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_G_TO_G_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_I_TO_A);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_I_TO_A_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_I_TO_B);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_I_TO_B_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_I_TO_G);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_I_TO_G_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_I_TO_I);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_I_TO_I_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_I_TO_R);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_I_TO_R_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_R_TO_R);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_R_TO_R_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_S_TO_S);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAP_S_TO_S_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SIZE_GRANULARITY);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SIZE_RANGE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SMOOTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SMOOTH_HINT);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_TOKEN);
            AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON);
            AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON_OFFSET_FACTOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON_OFFSET_FILL);
            AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON_OFFSET_LINE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON_OFFSET_POINT);
            AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON_OFFSET_UNITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON_SMOOTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON_SMOOTH_HINT);
            AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON_STIPPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON_TOKEN);
            AP_GL_ENUM_TOSTRING_CASE(GL_POSITION);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROJECTION);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROJECTION_MATRIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROJECTION_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_1D);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_2D);
            // GL_Q - Is handled using gsTextureCoordinateString().
            AP_GL_ENUM_TOSTRING_CASE(GL_QUADRATIC_ATTENUATION);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUADS);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUAD_STRIP);
            // GL_R - Is handled using gsTextureCoordinateString().
            AP_GL_ENUM_TOSTRING_CASE(GL_R3_G3_B2);
            AP_GL_ENUM_TOSTRING_CASE(GL_READ_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_RED);
            AP_GL_ENUM_TOSTRING_CASE(GL_RED_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_RED_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_RED_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDER);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERER);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDER_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_REPEAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REPLACE);
            AP_GL_ENUM_TOSTRING_CASE(GL_RETURN);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB10);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB10_A2);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB12);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB16);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB4);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB5);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB5_A1);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB8);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA12);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA16);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA2);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA4);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA8);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_RIGHT);
            // GL_S - Is handled using gsTextureCoordinateString().
            AP_GL_ENUM_TOSTRING_CASE(GL_SCISSOR_BOX);
            AP_GL_ENUM_TOSTRING_CASE(GL_SCISSOR_TEST);
            AP_GL_ENUM_TOSTRING_CASE(GL_SELECT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SELECTION_BUFFER_POINTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_SELECTION_BUFFER_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SET);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADE_MODEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHININESS);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHORT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SMOOTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPECULAR);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPHERE_MAP);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPOT_CUTOFF);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPOT_DIRECTION);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPOT_EXPONENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SRC_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_SRC_ALPHA_SATURATE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SRC_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_STACK_OVERFLOW);
            AP_GL_ENUM_TOSTRING_CASE(GL_STACK_UNDERFLOW);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_CLEAR_VALUE);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_FAIL);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_FUNC);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_PASS_DEPTH_FAIL);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_PASS_DEPTH_PASS);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_REF);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_TEST);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_VALUE_MASK);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_WRITEMASK);
            AP_GL_ENUM_TOSTRING_CASE(GL_STEREO);
            AP_GL_ENUM_TOSTRING_CASE(GL_SUBPIXEL_BITS);
            // GL_T - Is handled using gsTextureCoordinateString().
            AP_GL_ENUM_TOSTRING_CASE(GL_T2F_C3F_V3F);
            AP_GL_ENUM_TOSTRING_CASE(GL_T2F_C4F_N3F_V3F);
            AP_GL_ENUM_TOSTRING_CASE(GL_T2F_C4UB_V3F);
            AP_GL_ENUM_TOSTRING_CASE(GL_T2F_N3F_V3F);
            AP_GL_ENUM_TOSTRING_CASE(GL_T2F_V3F);
            AP_GL_ENUM_TOSTRING_CASE(GL_T4F_C4F_N3F_V4F);
            AP_GL_ENUM_TOSTRING_CASE(GL_T4F_V4F);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_1D);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_2D);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_ALPHA_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_1D);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_2D);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BLUE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BORDER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BORDER_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COORD_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COORD_ARRAY_COUNT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COORD_ARRAY_POINTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COORD_ARRAY_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COORD_ARRAY_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COORD_ARRAY_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_ENV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_ENV_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_ENV_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_GEN_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_GEN_Q);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_GEN_R);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_GEN_S);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_GEN_T);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_GREEN_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_HEIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_INTENSITY_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_INTERNAL_FORMAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_LUMINANCE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MAG_FILTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MATRIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MIN_FILTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_PRIORITY);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_RED_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_RESIDENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_WIDTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_WRAP_S);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_WRAP_T);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRIANGLES);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRIANGLE_FAN);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRIANGLE_STRIP);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_ALIGNMENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_LSB_FIRST);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_ROW_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_SKIP_PIXELS);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_SKIP_ROWS);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_SWAP_BYTES);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_BYTE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_SHORT);
            AP_GL_ENUM_TOSTRING_CASE(GL_V2F);
            AP_GL_ENUM_TOSTRING_CASE(GL_V3F);
            AP_GL_ENUM_TOSTRING_CASE(GL_VENDOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERSION);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_COUNT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_POINTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEWPORT);
            AP_GL_ENUM_TOSTRING_CASE(GL_XOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_ZOOM_X);
            AP_GL_ENUM_TOSTRING_CASE(GL_ZOOM_Y);

            // OpenGL 1.2 enumerators:
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_BYTE_3_3_2);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_SHORT_4_4_4_4);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_SHORT_5_5_5_1);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_8_8_8_8);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_10_10_10_2);
            AP_GL_ENUM_TOSTRING_CASE(GL_RESCALE_NORMAL);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_3D);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_SKIP_IMAGES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_IMAGE_HEIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_SKIP_IMAGES);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_IMAGE_HEIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_3D);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_3D);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_WRAP_R);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_3D_TEXTURE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_BYTE_2_3_3_REV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_SHORT_5_6_5);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_SHORT_5_6_5_REV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_SHORT_4_4_4_4_REV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_SHORT_1_5_5_5_REV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_8_8_8_8_REV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_2_10_10_10_REV);
            AP_GL_ENUM_TOSTRING_CASE(GL_BGR);
            AP_GL_ENUM_TOSTRING_CASE(GL_BGRA);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_ELEMENTS_VERTICES);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_ELEMENTS_INDICES);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLAMP_TO_EDGE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MIN_LOD);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MAX_LOD);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BASE_LEVEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MAX_LEVEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT_MODEL_COLOR_CONTROL);
            AP_GL_ENUM_TOSTRING_CASE(GL_SINGLE_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_SEPARATE_SPECULAR_COLOR);
            // GL_SMOOTH_POINT_SIZE_RANGE - Is the same as GL_POINT_SIZE_RANGE
            // GL_SMOOTH_POINT_SIZE_GRANULARITY - Is the same as GL_POINT_SIZE_GRANULARITY.
            // GL_SMOOTH_LINE_WIDTH_RANGE - Is the same as GL_LINE_WIDTH_RANGE.
            // GL_SMOOTH_LINE_WIDTH_GRANULARITY - Is the same as GL_LINE_WIDTH_GRANULARITY.
            AP_GL_ENUM_TOSTRING_CASE(GL_ALIASED_POINT_SIZE_RANGE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALIASED_LINE_WIDTH_RANGE);
            // Imaging subset:
            AP_GL_ENUM_TOSTRING_CASE(GL_CONSTANT_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_ONE_MINUS_CONSTANT_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONSTANT_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_ONE_MINUS_CONSTANT_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLEND_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_FUNC_ADD);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIN);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLEND_EQUATION);
            AP_GL_ENUM_TOSTRING_CASE(GL_FUNC_SUBTRACT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FUNC_REVERSE_SUBTRACT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONVOLUTION_1D);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONVOLUTION_2D);
            AP_GL_ENUM_TOSTRING_CASE(GL_SEPARABLE_2D);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONVOLUTION_BORDER_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONVOLUTION_FILTER_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONVOLUTION_FILTER_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_REDUCE);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONVOLUTION_FORMAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONVOLUTION_WIDTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONVOLUTION_HEIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_CONVOLUTION_WIDTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_CONVOLUTION_HEIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_CONVOLUTION_RED_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_CONVOLUTION_GREEN_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_CONVOLUTION_BLUE_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_CONVOLUTION_ALPHA_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_CONVOLUTION_RED_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_CONVOLUTION_GREEN_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_CONVOLUTION_BLUE_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_CONVOLUTION_ALPHA_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_HISTOGRAM);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_HISTOGRAM);
            AP_GL_ENUM_TOSTRING_CASE(GL_HISTOGRAM_WIDTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_HISTOGRAM_FORMAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_HISTOGRAM_RED_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_HISTOGRAM_GREEN_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_HISTOGRAM_BLUE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_HISTOGRAM_ALPHA_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_HISTOGRAM_LUMINANCE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_HISTOGRAM_SINK);
            AP_GL_ENUM_TOSTRING_CASE(GL_MINMAX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MINMAX_FORMAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MINMAX_SINK);
            AP_GL_ENUM_TOSTRING_CASE(GL_TABLE_TOO_LARGE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_MATRIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_MATRIX_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COLOR_MATRIX_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_COLOR_MATRIX_RED_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_COLOR_MATRIX_GREEN_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_COLOR_MATRIX_BLUE_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_COLOR_MATRIX_ALPHA_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_COLOR_MATRIX_RED_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_COLOR_MATRIX_GREEN_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_COLOR_MATRIX_BLUE_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_COLOR_MATRIX_ALPHA_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_TABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_CONVOLUTION_COLOR_TABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_COLOR_MATRIX_COLOR_TABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_COLOR_TABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_POST_CONVOLUTION_COLOR_TABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_TABLE_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_TABLE_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_TABLE_FORMAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_TABLE_WIDTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_TABLE_RED_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_TABLE_GREEN_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_TABLE_BLUE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_TABLE_ALPHA_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_TABLE_LUMINANCE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_TABLE_INTENSITY_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONSTANT_BORDER);
            AP_GL_ENUM_TOSTRING_CASE(GL_REPLICATE_BORDER);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONVOLUTION_BORDER_COLOR);

            // -------------------------------------------------------------------
            //                OpenGL 1.3 enumerators
            // -------------------------------------------------------------------
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE0);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE1);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE2);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE3);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE4);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE5);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE6);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE7);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE8);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE9);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE10);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE11);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE12);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE13);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE14);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE15);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE16);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE17);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE18);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE19);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE20);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE21);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE22);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE23);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE24);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE25);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE26);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE27);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE28);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE29);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE30);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE31);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_TEXTURE);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIENT_ACTIVE_TEXTURE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TEXTURE_UNITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSPOSE_MODELVIEW_MATRIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSPOSE_PROJECTION_MATRIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSPOSE_TEXTURE_MATRIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSPOSE_COLOR_MATRIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MULTISAMPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_ALPHA_TO_COVERAGE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_ALPHA_TO_ONE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_COVERAGE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_BUFFERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLES);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_COVERAGE_VALUE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_COVERAGE_INVERT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MULTISAMPLE_BIT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMAL_MAP);
            AP_GL_ENUM_TOSTRING_CASE(GL_REFLECTION_MAP);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CUBE_MAP);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_CUBE_MAP);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CUBE_MAP_POSITIVE_X);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_CUBE_MAP);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_CUBE_MAP_TEXTURE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_LUMINANCE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_LUMINANCE_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_INTENSITY);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGBA);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COMPRESSION_HINT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COMPRESSED_IMAGE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COMPRESSED);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_COMPRESSED_TEXTURE_FORMATS);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_TEXTURE_FORMATS);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLAMP_TO_BORDER);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINE_RGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINE_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE0_RGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE1_RGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE2_RGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE0_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE1_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE2_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND0_RGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND1_RGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND2_RGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND0_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND1_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND2_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB_SCALE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ADD_SIGNED);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERPOLATE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SUBTRACT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONSTANT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PRIMARY_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_PREVIOUS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT3_RGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT3_RGBA);

            // OpenGL 1.4 enumerators:
            AP_GL_ENUM_TOSTRING_CASE(GL_BLEND_DST_RGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLEND_SRC_RGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLEND_DST_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLEND_SRC_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SIZE_MIN);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SIZE_MAX);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_FADE_THRESHOLD_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_DISTANCE_ATTENUATION);
            AP_GL_ENUM_TOSTRING_CASE(GL_GENERATE_MIPMAP);
            AP_GL_ENUM_TOSTRING_CASE(GL_GENERATE_MIPMAP_HINT);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_COMPONENT16);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_COMPONENT24);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_COMPONENT32);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIRRORED_REPEAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_COORDINATE_SOURCE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_COORDINATE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_FOG_COORDINATE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_COORDINATE_ARRAY_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_COORDINATE_ARRAY_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_COORDINATE_ARRAY_POINTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_COORDINATE_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_SUM);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_SECONDARY_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_SECONDARY_COLOR_ARRAY_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SECONDARY_COLOR_ARRAY_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SECONDARY_COLOR_ARRAY_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SECONDARY_COLOR_ARRAY_POINTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_SECONDARY_COLOR_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TEXTURE_LOD_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_FILTER_CONTROL);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_LOD_BIAS);
            AP_GL_ENUM_TOSTRING_CASE(GL_INCR_WRAP);
            AP_GL_ENUM_TOSTRING_CASE(GL_DECR_WRAP);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_DEPTH_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_TEXTURE_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COMPARE_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COMPARE_FUNC);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPARE_R_TO_TEXTURE);

            // OpenGL 1.5 enumerators:
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_USAGE);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_COUNTER_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_QUERY);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_RESULT);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_RESULT_AVAILABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ARRAY_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ARRAY_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_ARRAY_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ARRAY_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_READ_ONLY);
            AP_GL_ENUM_TOSTRING_CASE(GL_WRITE_ONLY);
            AP_GL_ENUM_TOSTRING_CASE(GL_READ_WRITE);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_ACCESS);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_MAPPED);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_MAP_POINTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_STREAM_DRAW);
            AP_GL_ENUM_TOSTRING_CASE(GL_STREAM_READ);
            AP_GL_ENUM_TOSTRING_CASE(GL_STREAM_COPY);
            AP_GL_ENUM_TOSTRING_CASE(GL_STATIC_DRAW);
            AP_GL_ENUM_TOSTRING_CASE(GL_STATIC_READ);
            AP_GL_ENUM_TOSTRING_CASE(GL_STATIC_COPY);
            AP_GL_ENUM_TOSTRING_CASE(GL_DYNAMIC_DRAW);
            AP_GL_ENUM_TOSTRING_CASE(GL_DYNAMIC_READ);
            AP_GL_ENUM_TOSTRING_CASE(GL_DYNAMIC_COPY);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLES_PASSED);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMAL_ARRAY_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ARRAY_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_ARRAY_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_EDGE_FLAG_ARRAY_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_WEIGHT_ARRAY_BUFFER_BINDING);

        default:
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        checkOpenGL2Enum
// Description: Check if this is a 2.0* enumeration:
// Arguments:   enumValue
//              valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkOpenGL2Enum(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;

    switch (enumValue)
    {
            // OpenGL 2.0 enumerators:
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_ENABLED);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_VERTEX_ATTRIB);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_PROGRAM_POINT_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_PROGRAM_TWO_SIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_POINTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_BACK_FUNC);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_BACK_FAIL);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_BACK_PASS_DEPTH_FAIL);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_BACK_PASS_DEPTH_PASS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_DRAW_BUFFERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER0);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER1);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER2);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER3);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER4);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER5);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER6);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER7);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER8);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER9);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER10);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER11);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER12);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER13);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER14);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_BUFFER15);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLEND_EQUATION_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SPRITE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COORD_REPLACE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_ATTRIBS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_NORMALIZED);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TEXTURE_COORDS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TEXTURE_IMAGE_UNITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_UNIFORM_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VARYING_FLOATS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_VEC2);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_VEC3);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_VEC4);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_VEC2);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_VEC3);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_VEC4);
            AP_GL_ENUM_TOSTRING_CASE(GL_BOOL);
            AP_GL_ENUM_TOSTRING_CASE(GL_BOOL_VEC2);
            AP_GL_ENUM_TOSTRING_CASE(GL_BOOL_VEC3);
            AP_GL_ENUM_TOSTRING_CASE(GL_BOOL_VEC4);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_MAT2);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_MAT3);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_MAT4);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_1D);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_2D);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_3D);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_CUBE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_1D_SHADOW);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_2D_SHADOW);
            AP_GL_ENUM_TOSTRING_CASE(GL_DELETE_STATUS);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPILE_STATUS);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINK_STATUS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VALIDATE_STATUS);
            AP_GL_ENUM_TOSTRING_CASE(GL_INFO_LOG_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATTACHED_SHADERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_UNIFORMS);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_UNIFORM_MAX_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_SOURCE_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_ATTRIBUTES);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_SHADER_DERIVATIVE_HINT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADING_LANGUAGE_VERSION);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_PROGRAM);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SPRITE_COORD_ORIGIN);
            AP_GL_ENUM_TOSTRING_CASE(GL_LOWER_LEFT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UPPER_LEFT);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_BACK_REF);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_BACK_VALUE_MASK);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_BACK_WRITEMASK);

            // OpenGL 2.1 enumerators
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_RASTER_SECONDARY_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_PACK_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_UNPACK_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_PACK_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_UNPACK_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_MAT2x3);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_MAT2x4);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_MAT3x2);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_MAT3x4);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_MAT4x2);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_MAT4x3);
            AP_GL_ENUM_TOSTRING_CASE(GL_SRGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SRGB8);
            AP_GL_ENUM_TOSTRING_CASE(GL_SRGB_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_SRGB8_ALPHA8);
            AP_GL_ENUM_TOSTRING_CASE(GL_SLUMINANCE_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_SLUMINANCE8_ALPHA8);
            AP_GL_ENUM_TOSTRING_CASE(GL_SLUMINANCE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SLUMINANCE8);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SRGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SRGB_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SLUMINANCE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SLUMINANCE_ALPHA);

        default:
            retVal = false;
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        checkOpenGL3Enum
// Description: Check if this is a 3.* enumeration
// Arguments:   enumValue
//              valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkOpenGL3Enum(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;

    switch (enumValue)
    {
            // OpenGL 3.0 enumerators
            // GL_COMPARE_REF_TO_TEXTURE is equal to GL_COMPARE_R_TO_TEXTURE_ARB
            // GL_CLIP_DISTANCE0 is equal to GL_CLIP_PLANE0
            // GL_CLIP_DISTANCE1 is equal to GL_CLIP_PLANE1
            // GL_CLIP_DISTANCE2 is equal to GL_CLIP_PLANE2
            // GL_CLIP_DISTANCE3 is equal to GL_CLIP_PLANE3
            // GL_CLIP_DISTANCE4 is equal to GL_CLIP_PLANE4
            // GL_CLIP_DISTANCE5 is equal to GL_CLIP_PLANE5
            // GL_MAX_CLIP_DISTANCES is equal to GL_MAX_CLIP_PLANES
            AP_GL_ENUM_TOSTRING_CASE(GL_MAJOR_VERSION);
            AP_GL_ENUM_TOSTRING_CASE(GL_MINOR_VERSION);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_EXTENSIONS);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONTEXT_FLAGS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RED);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RG);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA32F);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB32F);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA16F);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB16F);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_INTEGER);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_ARRAY_TEXTURE_LAYERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIN_PROGRAM_TEXEL_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_TEXEL_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLAMP_VERTEX_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLAMP_FRAGMENT_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLAMP_READ_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_FIXED_ONLY);
            // GL_MAX_VARYING_COMPONENTS is equal to GL_MAX_VARYING_FLOATS
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_RED_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_GREEN_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BLUE_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_ALPHA_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_LUMINANCE_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_INTENSITY_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_DEPTH_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_NORMALIZED);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_1D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_1D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_2D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_2D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_1D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_2D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_R11F_G11F_B10F);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_10F_11F_11F_REV);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB9_E5);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_5_9_9_9_REV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SHARED_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BUFFER_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_VARYINGS);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BUFFER_START);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BUFFER_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PRIMITIVES_GENERATED);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
            AP_GL_ENUM_TOSTRING_CASE(GL_RASTERIZER_DISCARD);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERLEAVED_ATTRIBS);
            AP_GL_ENUM_TOSTRING_CASE(GL_SEPARATE_ATTRIBS);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA32UI);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB32UI);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA16UI);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB16UI);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA8UI);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB8UI);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA32I);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB32I);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA16I);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB16I);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA8I);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB8I);
            AP_GL_ENUM_TOSTRING_CASE(GL_RED_INTEGER);
            AP_GL_ENUM_TOSTRING_CASE(GL_GREEN_INTEGER);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLUE_INTEGER);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA_INTEGER);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB_INTEGER);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA_INTEGER);
            AP_GL_ENUM_TOSTRING_CASE(GL_BGR_INTEGER);
            AP_GL_ENUM_TOSTRING_CASE(GL_BGRA_INTEGER);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_1D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_2D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_1D_ARRAY_SHADOW);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_2D_ARRAY_SHADOW);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_CUBE_SHADOW);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_VEC2);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_VEC3);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_VEC4);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_1D);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_2D);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_3D);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_CUBE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_1D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_2D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_1D);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_2D);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_3D);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_CUBE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_WAIT);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_NO_WAIT);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_BY_REGION_WAIT);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_BY_REGION_NO_WAIT);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_ACCESS_FLAGS);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_MAP_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_MAP_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_BINDING);

            // OpenGL 3.1 enumerators:
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_2D_RECT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_2D_RECT_SHADOW);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_2D_RECT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_2D_RECT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TEXTURE_BUFFER_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BUFFER_DATA_STORE_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BUFFER_FORMAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_RECTANGLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_RECTANGLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_RECTANGLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_RECTANGLE_TEXTURE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_RED_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_R8_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG8_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB8_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA8_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_R16_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG16_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB16_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA16_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_NORMALIZED);
            AP_GL_ENUM_TOSTRING_CASE(GL_PRIMITIVE_RESTART);
            AP_GL_ENUM_TOSTRING_CASE(GL_PRIMITIVE_RESTART_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_COPY_READ_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_COPY_WRITE_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER_START);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_UNIFORM_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_UNIFORM_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_UNIFORM_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_UNIFORM_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_UNIFORM_BUFFER_BINDINGS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_UNIFORM_BLOCK_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_UNIFORM_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_NAME_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_ARRAY_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_MATRIX_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_IS_ROW_MAJOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_DATA_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_NAME_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER);

            // OpenGL 3.2 enumerators:
            AP_GL_ENUM_TOSTRING_CASE(GL_LINES_ADJACENCY);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINE_STRIP_ADJACENCY);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRIANGLES_ADJACENCY);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRIANGLE_STRIP_ADJACENCY);
            // GL_PROGRAM_POINT_SIZE_EXT same as: GL_PROGRAM_POINT_SIZE (OpenGL3.2 rename)
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_LAYERED);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_VERTICES_OUT);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_INPUT_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_OUTPUT_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_OUTPUT_VERTICES);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_OUTPUT_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_INPUT_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_INPUT_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONTEXT_PROFILE_MASK);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION);
            AP_GL_ENUM_TOSTRING_CASE(GL_FIRST_VERTEX_CONVENTION);
            AP_GL_ENUM_TOSTRING_CASE(GL_LAST_VERTEX_CONVENTION);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROVOKING_VERTEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CUBE_MAP_SEAMLESS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SERVER_WAIT_TIMEOUT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OBJECT_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SYNC_CONDITION);
            AP_GL_ENUM_TOSTRING_CASE(GL_SYNC_STATUS);
            AP_GL_ENUM_TOSTRING_CASE(GL_SYNC_FLAGS);
            AP_GL_ENUM_TOSTRING_CASE(GL_SYNC_FENCE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SYNC_GPU_COMMANDS_COMPLETE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNALED);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNALED);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALREADY_SIGNALED);
            AP_GL_ENUM_TOSTRING_CASE(GL_TIMEOUT_EXPIRED);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONDITION_SATISFIED);
            AP_GL_ENUM_TOSTRING_CASE(GL_WAIT_FAILED);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_POSITION);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_MASK);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_MASK_VALUE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SAMPLE_MASK_WORDS);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_2D_MULTISAMPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_2D_MULTISAMPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_2D_MULTISAMPLE_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_2D_MULTISAMPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SAMPLES);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_FIXED_SAMPLE_LOCATIONS);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_2D_MULTISAMPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_2D_MULTISAMPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_2D_MULTISAMPLE_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COLOR_TEXTURE_SAMPLES);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_DEPTH_TEXTURE_SAMPLES);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_INTEGER_SAMPLES);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_CLAMP);

            // OpenGL 3.3 enumerators:
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_DIVISOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_SRC1_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_ONE_MINUS_SRC1_COLOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_ONE_MINUS_SRC1_ALPHA);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_ANY_SAMPLES_PASSED);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB10_A2UI);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_R);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_G);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_B);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_A);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_RGBA);
            AP_GL_ENUM_TOSTRING_CASE(GL_TIME_ELAPSED);
            AP_GL_ENUM_TOSTRING_CASE(GL_TIMESTAMP);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_2_10_10_10_REV);

        default:
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        checkOpenGL4Enum
// Description: Check if this is an OpenGL 4.0 enumeration
// Arguments:   GLenum enumValue
//              gtString& valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkOpenGL4Enum(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;

    switch (enumValue)
    {
            // OpenGL 4.0 enumerators::
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_SHADING);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIN_SAMPLE_SHADING_VALUE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CUBE_MAP_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_CUBE_MAP_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_CUBE_MAP_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_CUBE_MAP_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_CUBE_MAP_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_INDIRECT_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_INDIRECT_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_SHADER_INVOCATIONS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_SHADER_INVOCATIONS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIN_FRAGMENT_INTERPOLATION_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_INTERPOLATION_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_INTERPOLATION_OFFSET_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_STREAMS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_VEC2);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_VEC3);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_VEC4);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT2);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT3);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT4);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT2x3);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT2x4);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT3x2);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT3x4);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT4x2);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT4x3);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_SUBROUTINES);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_SUBROUTINE_UNIFORMS);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_SUBROUTINE_MAX_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SUBROUTINES);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_COMPATIBLE_SUBROUTINES);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPATIBLE_SUBROUTINES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATCHES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATCH_VERTICES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATCH_DEFAULT_INNER_LEVEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATCH_DEFAULT_OUTER_LEVEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_CONTROL_OUTPUT_VERTICES);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_GEN_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_GEN_SPACING);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_GEN_VERTEX_ORDER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_GEN_POINT_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ISOLINES);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRACTIONAL_ODD);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRACTIONAL_EVEN);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PATCH_VERTICES);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_GEN_LEVEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_PATCH_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_INPUT_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_EVALUATION_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_CONTROL_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK);
            //          AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED); // Renamed GL_TRANSFORM_FEEDBACK_PAUSED (OpenGL 4.2)
            //          AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE); // Renamed GL_TRANSFORM_FEEDBACK_ACTIVE (OpenGL 4.2)
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TRANSFORM_FEEDBACK_BUFFERS);

            // OpenGL 4.1 enumerators:
            AP_GL_ENUM_TOSTRING_CASE(GL_FIXED);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMPLEMENTATION_COLOR_READ_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMPLEMENTATION_COLOR_READ_FORMAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LOW_FLOAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MEDIUM_FLOAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_HIGH_FLOAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LOW_INT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MEDIUM_INT);
            AP_GL_ENUM_TOSTRING_CASE(GL_HIGH_INT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_COMPILER);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_BINARY_FORMATS);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_SHADER_BINARY_FORMATS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_UNIFORM_VECTORS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VARYING_VECTORS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_UNIFORM_VECTORS);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB565);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_BINARY_RETRIEVABLE_HINT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_BINARY_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_PROGRAM_BINARY_FORMATS);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_BINARY_FORMATS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SHADER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_SHADER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_SHADER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TESS_CONTROL_SHADER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TESS_EVALUATION_SHADER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ALL_SHADER_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_SEPARABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_PROGRAM);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_PIPELINE_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VIEWPORTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEWPORT_SUBPIXEL_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEWPORT_BOUNDS_RANGE);
            AP_GL_ENUM_TOSTRING_CASE(GL_LAYER_PROVOKING_VERTEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEWPORT_INDEX_PROVOKING_VERTEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNDEFINED_VERTEX);

            // OpenGL 4.2 enumerators:
            // AP_GL_ENUM_TOSTRING_CASE(GL_COPY_READ_BUFFER_BINDING);   // Same as GL_COPY_READ_BUFFER
            // AP_GL_ENUM_TOSTRING_CASE(GL_COPY_WRITE_BUFFER_BINDING);  // Same as GL_COPY_WRITE_BUFFER
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_ACTIVE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_PAUSED);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_COMPRESSED_BLOCK_WIDTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_COMPRESSED_BLOCK_HEIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_COMPRESSED_BLOCK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_COMPRESSED_BLOCK_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_COMPRESSED_BLOCK_WIDTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_COMPRESSED_BLOCK_HEIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_COMPRESSED_BLOCK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_COMPRESSED_BLOCK_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_SAMPLE_COUNTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIN_MAP_BUFFER_ALIGNMENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER_START);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_ATOMIC_COUNTERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_ATOMIC_COUNTERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_ATOMIC_COUNTERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_ATOMIC_COUNTERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_ATOMIC_COUNTER_BUFFERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_ATOMIC_COUNTER);
            // AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ARRAY_BARRIER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BARRIER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_FETCH_BARRIER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_COMMAND_BARRIER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_BUFFER_BARRIER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_UPDATE_BARRIER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_UPDATE_BARRIER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_BARRIER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BARRIER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BARRIER_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ALL_BARRIER_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_IMAGE_UNITS);
            // GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS - Replaced by GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BINDING_NAME);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BINDING_LEVEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BINDING_LAYERED);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BINDING_LAYER);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BINDING_ACCESS);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_1D);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_2D);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_3D);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_2D_RECT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CUBE);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_1D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_2D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CUBE_MAP_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_2D_MULTISAMPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_2D_MULTISAMPLE_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_1D);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_2D);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_3D);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_2D_RECT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_CUBE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_1D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_2D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_CUBE_MAP_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_2D_MULTISAMPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_1D);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_2D);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_3D);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_2D_RECT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_CUBE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_1D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_2D_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_IMAGE_SAMPLES);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BINDING_FORMAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_FORMAT_COMPATIBILITY_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_IMAGE_UNIFORMS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_IMAGE_UNIFORMS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_IMAGE_UNIFORMS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_IMAGE_UNIFORMS);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGBA_BPTC_UNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_IMMUTABLE_FORMAT);

            // OpenGL 4.3 enumerators:
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_SHADING_LANGUAGE_VERSIONS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_LONG);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGB8_ETC2);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SRGB8_ETC2);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGBA8_ETC2_EAC);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_R11_EAC);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SIGNED_R11_EAC);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RG11_EAC);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SIGNED_RG11_EAC);
            AP_GL_ENUM_TOSTRING_CASE(GL_PRIMITIVE_RESTART_FIXED_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_ELEMENT_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPUTE_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMPUTE_UNIFORM_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMPUTE_IMAGE_UNIFORMS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMPUTE_UNIFORM_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMPUTE_ATOMIC_COUNTERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS); // Renamed from GL_MAX_COMPUTE_LOCAL_INVOCATIONS
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMPUTE_WORK_GROUP_COUNT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMPUTE_WORK_GROUP_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPUTE_WORK_GROUP_SIZE); // Renamed from GL_COMPUTE_LOCAL_WORK_SIZE
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_DISPATCH_INDIRECT_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_DISPATCH_INDIRECT_BUFFER_BINDING);
            // AP_GL_ENUM_TOSTRING_CASE(GL_COMPUTE_SHADER_BIT);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_CALLBACK_FUNCTION);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_CALLBACK_USER_PARAM);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SOURCE_API);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SOURCE_WINDOW_SYSTEM);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SOURCE_SHADER_COMPILER);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SOURCE_THIRD_PARTY);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SOURCE_APPLICATION);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SOURCE_OTHER);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_ERROR);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_PORTABILITY);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_PERFORMANCE);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_OTHER);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_DEBUG_MESSAGE_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_DEBUG_LOGGED_MESSAGES);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_LOGGED_MESSAGES);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SEVERITY_HIGH);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SEVERITY_MEDIUM);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SEVERITY_LOW);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_MARKER);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_PUSH_GROUP);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_POP_GROUP);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SEVERITY_NOTIFICATION);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_DEBUG_GROUP_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_GROUP_STACK_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_PIPELINE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_LABEL_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_OUTPUT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_CONTEXT_FLAG_DEBUG_BIT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_UNIFORM_LOCATIONS);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_DEFAULT_WIDTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_DEFAULT_HEIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_DEFAULT_LAYERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_DEFAULT_SAMPLES);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAMEBUFFER_WIDTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAMEBUFFER_HEIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAMEBUFFER_LAYERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAMEBUFFER_SAMPLES);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_SUPPORTED);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_PREFERRED);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_RED_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_GREEN_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_BLUE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_ALPHA_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_DEPTH_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_STENCIL_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_SHARED_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_RED_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_GREEN_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_BLUE_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_ALPHA_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_DEPTH_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERNALFORMAT_STENCIL_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_WIDTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_HEIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_DEPTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_LAYERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_DIMENSIONS);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_COMPONENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_RENDERABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_RENDERABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_RENDERABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_RENDERABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_RENDERABLE_LAYERED);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_BLEND);
            AP_GL_ENUM_TOSTRING_CASE(GL_READ_PIXELS);
            AP_GL_ENUM_TOSTRING_CASE(GL_READ_PIXELS_FORMAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_READ_PIXELS_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_IMAGE_FORMAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_IMAGE_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_GET_TEXTURE_IMAGE_FORMAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_GET_TEXTURE_IMAGE_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIPMAP);
            AP_GL_ENUM_TOSTRING_CASE(GL_MANUAL_GENERATE_MIPMAP);
            AP_GL_ENUM_TOSTRING_CASE(GL_AUTO_GENERATE_MIPMAP);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ENCODING);
            AP_GL_ENUM_TOSTRING_CASE(GL_SRGB_READ);
            AP_GL_ENUM_TOSTRING_CASE(GL_SRGB_WRITE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FILTER);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_TEXTURE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_CONTROL_TEXTURE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_EVALUATION_TEXTURE);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_TEXTURE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_TEXTURE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPUTE_TEXTURE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SHADOW);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_GATHER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_GATHER_SHADOW);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_IMAGE_LOAD);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_IMAGE_STORE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_IMAGE_ATOMIC);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_TEXEL_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_COMPATIBILITY_CLASS);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_PIXEL_FORMAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_PIXEL_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COMPRESSED_BLOCK_WIDTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COMPRESSED_BLOCK_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLEAR_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_VIEW);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_COMPATIBILITY_CLASS);
            AP_GL_ENUM_TOSTRING_CASE(GL_FULL_SUPPORT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CAVEAT_SUPPORT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CLASS_4_X_32);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CLASS_2_X_32);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CLASS_1_X_32);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CLASS_4_X_16);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CLASS_2_X_16);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CLASS_1_X_16);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CLASS_4_X_8);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CLASS_2_X_8);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CLASS_1_X_8);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CLASS_11_11_10);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CLASS_10_10_10_2);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_128_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_96_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_64_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_48_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_32_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_24_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_16_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_8_BITS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_S3TC_DXT1_RGB);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_S3TC_DXT1_RGBA);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_S3TC_DXT3_RGBA);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_S3TC_DXT5_RGBA);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_RGTC1_RED);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_RGTC2_RG);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_BPTC_UNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIEW_CLASS_BPTC_FLOAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_INPUT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_OUTPUT);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_VARIABLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_STORAGE_BLOCK);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SUBROUTINE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_CONTROL_SUBROUTINE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_EVALUATION_SUBROUTINE);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_SUBROUTINE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_SUBROUTINE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPUTE_SUBROUTINE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SUBROUTINE_UNIFORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_CONTROL_SUBROUTINE_UNIFORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_EVALUATION_SUBROUTINE_UNIFORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_SUBROUTINE_UNIFORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_SUBROUTINE_UNIFORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPUTE_SUBROUTINE_UNIFORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_VARYING);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_RESOURCES);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_NAME_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_NUM_ACTIVE_VARIABLES);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_NUM_COMPATIBLE_SUBROUTINES);
            AP_GL_ENUM_TOSTRING_CASE(GL_NAME_LENGTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ARRAY_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLOCK_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_ARRAY_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_IS_ROW_MAJOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BUFFER_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_DATA_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_ACTIVE_VARIABLES);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_VARIABLES);
            AP_GL_ENUM_TOSTRING_CASE(GL_REFERENCED_BY_VERTEX_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_REFERENCED_BY_TESS_CONTROL_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_REFERENCED_BY_TESS_EVALUATION_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_REFERENCED_BY_GEOMETRY_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_REFERENCED_BY_FRAGMENT_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_REFERENCED_BY_COMPUTE_SHADER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TOP_LEVEL_ARRAY_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TOP_LEVEL_ARRAY_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_LOCATION);
            AP_GL_ENUM_TOSTRING_CASE(GL_LOCATION_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_IS_PER_PATCH);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_STORAGE_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_STORAGE_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_STORAGE_BUFFER_START);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_STORAGE_BUFFER_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SHADER_STORAGE_BLOCK_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_STORAGE_BARRIER_BIT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_STENCIL_TEXTURE_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BUFFER_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BUFFER_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_VIEW_MIN_LEVEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_VIEW_NUM_LEVELS);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_VIEW_MIN_LAYER);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_VIEW_NUM_LAYERS);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_IMMUTABLE_LEVELS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_RELATIVE_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_BINDING_DIVISOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_BINDING_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_BINDING_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_ATTRIB_BINDINGS);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_BINDING_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_DISPLAY_LIST);

            // OpenGL 4.4 enumerators:
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_ATTRIB_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BUFFER_BINDING);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAP_PERSISTENT_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAP_COHERENT_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DYNAMIC_STORAGE_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_CLIENT_STORAGE_BIT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_IMMUTABLE_STORAGE);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_STORAGE_FLAGS);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLEAR_TEXTURE);
            AP_GL_ENUM_TOSTRING_CASE(GL_LOCATION_COMPONENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BUFFER_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_BUFFER);
            // AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_BUFFER_BARRIER_BIT);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_BUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_RESULT_NO_WAIT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIRROR_CLAMP_TO_EDGE);

            // OpenGL 4.5 enumerators:
            AP_GL_ENUM_TOSTRING_CASE(GL_CONTEXT_LOST);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEGATIVE_ONE_TO_ONE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ZERO_TO_ONE);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_ORIGIN);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_DEPTH_MODE);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_WAIT_INVERTED);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_NO_WAIT_INVERTED);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_BY_REGION_WAIT_INVERTED);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_BY_REGION_NO_WAIT_INVERTED);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_CULL_DISTANCES);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_TARGET);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_TARGET);
            AP_GL_ENUM_TOSTRING_CASE(GL_GUILTY_CONTEXT_RESET);
            AP_GL_ENUM_TOSTRING_CASE(GL_INNOCENT_CONTEXT_RESET);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNKNOWN_CONTEXT_RESET);
            AP_GL_ENUM_TOSTRING_CASE(GL_RESET_NOTIFICATION_STRATEGY);
            AP_GL_ENUM_TOSTRING_CASE(GL_LOSE_CONTEXT_ON_RESET);
            AP_GL_ENUM_TOSTRING_CASE(GL_NO_RESET_NOTIFICATION);
            // AP_GL_ENUM_TOSTRING_CASE(GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONTEXT_RELEASE_BEHAVIOR);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH);

        default:
            retVal = false;
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        checkOpenGLESEnum
// Description: Checks if this is an OpenGL ES enumeration:
// Arguments:   GLenum enumValue
//              gtString& valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkOpenGLESEnum(GLenum enumValue, gtString& valueString)
{
    bool retVal = false;

    // Resolve the compiler warning for the Linux variant
    (void)(enumValue);
    (void)(valueString);
    // OpenGL ES is currently supported only on windows and Mac:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

    retVal = true;

    // OpenGL ES enumerators:
    switch (enumValue)
    {
            // OpenGL ES 2.0
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS);
            // GL_IMPLEMENTATION_COLOR_READ_TYPE_OES is the same as GL_IMPLEMENTATION_COLOR_READ_TYPE
            // GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES is the same as GL_IMPLEMENTATION_COLOR_READ_FORMAT
            // GL_SRC0_RGB is the same as GL_SOURCE0_RGB
            // GL_SRC1_RGB is the same as GL_SOURCE1_RGB
            // GL_SRC2_RGB is the same as GL_SOURCE2_RGB
            // GL_SRC0_ALPHA is the same as GL_SOURCE0_ALPHA
            // GL_SRC1_ALPHA is the same as GL_SOURCE1_ALPHA
            // GL_SRC2_ALPHA is the same as GL_SOURCE2_ALPHA

            // OpenGL ES extensions enumerators:

            // OpenGL ES is currently supported only on Windows and Mac:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

            // OES_draw_texture:
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CROP_RECT_OES);

            // OES_matrix_get:
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW_MATRIX_FLOAT_AS_INT_BITS_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROJECTION_MATRIX_FLOAT_AS_INT_BITS_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MATRIX_FLOAT_AS_INT_BITS_OES);

            // OES_matrix_palette:
            // GL_MAX_VERTEX_UNITS_OES - Is the same as GL_MAX_VERTEX_UNITS_ARB
            // GL_MAX_PALETTE_MATRICES_OES - Is the same as GL_MAX_PALETTE_MATRICES_ARB.
            // GL_MATRIX_PALETTE_OES - Is the same as GL_MATRIX_PALETTE_ARB.
            // GL_MATRIX_INDEX_ARRAY_OES - Is the same as GL_MATRIX_INDEX_ARRAY_ARB.
            // GL_WEIGHT_ARRAY_OES - Is the same as GL_WEIGHT_ARRAY_ARB.
            // GL_CURRENT_PALETTE_MATRIX_OES is the same as GL_CURRENT_PALETTE_MATRIX_ARB
            // GL_MATRIX_INDEX_ARRAY_SIZE_OES - Is the same as GL_MATRIX_INDEX_ARRAY_SIZE_ARB.
            // GL_MATRIX_INDEX_ARRAY_TYPE_OES - Is the same as GL_MATRIX_INDEX_ARRAY_TYPE_ARB.
            // GL_MATRIX_INDEX_ARRAY_STRIDE_OES - Is the same as GL_MATRIX_INDEX_ARRAY_STRIDE_ARB.
            // GL_MATRIX_INDEX_ARRAY_POINTER_OES - Is the same as GL_MATRIX_INDEX_ARRAY_POINTER_ARB.
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES);
            // GL_WEIGHT_ARRAY_SIZE_OES - Is the same as GL_WEIGHT_ARRAY_SIZE_ARB.
            // GL_WEIGHT_ARRAY_TYPE_OES - Is the same as GL_WEIGHT_ARRAY_TYPE_ARB.
            // GL_WEIGHT_ARRAY_STRIDE_OES - Is the same as GL_WEIGHT_ARRAY_STRIDE_ARB.
            // GL_WEIGHT_ARRAY_POINTER_OES - Is the same as GL_WEIGHT_ARRAY_POINTER_ARB.
            // GL_WEIGHT_ARRAY_BUFFER_BINDING_OES - Is the same as GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB.

            // OES_point_size_array:
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SIZE_ARRAY_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SIZE_ARRAY_TYPE_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SIZE_ARRAY_STRIDE_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SIZE_ARRAY_POINTER_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES);

            // OES_point_sprite:
            // GL_POINT_SPRITE_OES - Is the same as GL_POINT_SPRITE_ARB.
            // GL_COORD_REPLACE_OES - Is the same as GL_COORD_REPLACE_ARB.

            // GL_OES_compressed_paletted_texture:
            AP_GL_ENUM_TOSTRING_CASE(GL_PALETTE4_RGB8_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PALETTE4_RGBA8_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PALETTE4_R5_G6_B5_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PALETTE4_RGBA4_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PALETTE4_RGB5_A1_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PALETTE8_RGB8_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PALETTE8_RGBA8_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PALETTE8_R5_G6_B5_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PALETTE8_RGBA4_OES);
            AP_GL_ENUM_TOSTRING_CASE(GL_PALETTE8_RGB5_A1_OES);

            // GL_OES_blend_equation_separate
            // BLEND_EQUATION_RGB_OES same as BLEND_EQUATION_OES
            // GL_BLEND_EQUATION_RGB_OES is the same as GL_BLEND_EQUATION_RGB which is the same as GL_BLEND_EQUATION
            //GL_BLEND_EQUATION_ALPHA_OES is the same as GL_BLEND_EQUATION_ALPHA

            // GL_OES_blend_func_separate
            // GL_BLEND_DST_RGB_OES is the same as GL_BLEND_DST_RGB
            // GL_BLEND_SRC_RGB_OES is the same as GL_BLEND_SRC_RGB
            // GL_BLEND_DST_ALPHA_OES is the same as GL_BLEND_DST_ALPHA
            // GL_BLEND_SRC_ALPHA_OES is the same as GL_BLEND_SRC_ALPHA

            // GL_OES_blend_subtract
            // GL_BLEND_EQUATION_OES is the same as GL_BLEND_EQUATION
            // GL_FUNC_ADD_OES is the same as GL_FUNC_ADD
            // GL_FUNC_SUBTRACT_OES is the same as GL_FUNC_SUBTRACT
            // GL_FUNC_REVERSE_SUBTRACT_OES is the same as GL_FUNC_REVERSE_SUBTRACT

            // GL_OES_depth24
            // GL_DEPTH_COMPONENT24_OES is the same as GL_DEPTH_COMPONENT24

            // GL_OES_stencil8
            // GL_STENCIL_INDEX8_OES is the same as GL_STENCIL_INDEX8

            // GL_OES_packed_depth_stencil
            // GL_DEPTH_STENCIL_OES is the same as GL_DEPTH_STENCIL
            // GL_UNSIGNED_INT_24_8_OES is the same as GL_UNSIGNED_INT_24_8
            // GL_DEPTH24_STENCIL8_OES is the same as GL_DEPTH24_STENCIL8

            // GL_OES_EGL_image
            // No enums

            // GL_OES_fixed_point
            // GL_FIXED_OES is the same as GL_FIXED

            // GL_OES_framebuffer_object
            // GL_FRAMEBUFFER_OES is the same as GL_FRAMEBUFFER
            // GL_RENDERBUFFER_OES is the same as GL_RENDERBUFFER
            // GL_RGBA4_OES is the same as GL_RGBA4
            // GL_RGB5_A1_OES is the same as GL_RGB5_A1
            // GL_RGB565_OES is the same as GL_RGB565
            // GL_DEPTH_COMPONENT16_OES is the same as GL_DEPTH_COMPONENT16
            // GL_RENDERBUFFER_WIDTH_OES is the same as GL_RENDERBUFFER_WIDTH
            // GL_RENDERBUFFER_HEIGHT_OES is the same as GL_RENDERBUFFER_HEIGHT
            // GL_RENDERBUFFER_INTERNAL_FORMAT_OES is the same as GL_RENDERBUFFER_INTERNAL_FORMAT
            // GL_RENDERBUFFER_RED_SIZE_OES is the same as GL_RENDERBUFFER_RED_SIZE
            // GL_RENDERBUFFER_GREEN_SIZE_OES is the same as GL_RENDERBUFFER_GREEN_SIZE
            // GL_RENDERBUFFER_BLUE_SIZE_OES is the same as GL_RENDERBUFFER_BLUE_SIZE
            // GL_RENDERBUFFER_ALPHA_SIZE_OES is the same as GL_RENDERBUFFER_ALPHA_SIZE
            // GL_RENDERBUFFER_DEPTH_SIZE_OES is the same as GL_RENDERBUFFER_DEPTH_SIZE
            // GL_RENDERBUFFER_STENCIL_SIZE_OES is the same as GL_RENDERBUFFER_STENCIL_SIZE
            // GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_OES is the same as GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE
            // GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_OES is the same as GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME
            // GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_OES is the same as GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL
            // GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_OES is the same as GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE
            // GL_COLOR_ATTACHMENT0_OES is the same as GL_COLOR_ATTACHMENT0
            // GL_DEPTH_ATTACHMENT_OES is the same as GL_DEPTH_ATTACHMENT
            // GL_STENCIL_ATTACHMENT_OES is the same as GL_STENCIL_ATTACHMENT
            // GL_FRAMEBUFFER_COMPLETE_OES is the same as GL_FRAMEBUFFER_COMPLETE
            // GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_OES is the same as GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
            // GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_OES is the same as GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
            // GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES is the same as GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
            // GL_FRAMEBUFFER_INCOMPLETE_FORMATS_OES is the same as GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT
            // GL_FRAMEBUFFER_UNSUPPORTED_OES is the same as GL_FRAMEBUFFER_UNSUPPORTED
            // GL_FRAMEBUFFER_BINDING_OES is the same as GL_FRAMEBUFFER_BINDING
            // GL_RENDERBUFFER_BINDING_OES is the same as GL_RENDERBUFFER_BINDING
            // GL_MAX_RENDERBUFFER_SIZE_OES is the same as GL_MAX_RENDERBUFFER_SIZE
            // GL_INVALID_FRAMEBUFFER_OPERATION_OES is the same as GL_INVALID_FRAMEBUFFER_OPERATION
#endif // (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

            // Windows onlt OpenGL ES extensions:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

            // GL_OES_compressed_ETC1_RGB8_texture
            AP_GL_ENUM_TOSTRING_CASE(GL_ETC1_RGB8_OES);

            // GL_OES_depth32
            // GL_DEPTH_COMPONENT32_OES is the same as GL_DEPTH_COMPONENT32
#endif

            // Mac Only OpenGL ES extensions:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

            // GL_IMG_read_format
            // GL_UNSIGNED_SHORT_4_4_4_4_REV already defined in OpenGL 1.2

            // GL_IMG_read_format || GL_IMG_texture_format_BGRA8888
            // GL_BGRA already defined in OpenGL 1.2

            // GL_IMG_texture_compression_pvrtc
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG);

            // GL_OES_mapbuffer
            // GL_WRITE_ONLY_OES is the same as GL_WRITE_ONLY
            // GL_BUFFER_ACCESS_OES is the same as GL_BUFFER_ACCESS
            // GL_BUFFER_MAPPED_OES is the same as GL_BUFFER_MAPPED
            // GL_BUFFER_MAP_POINTER_OES is the same as GL_BUFFER_MAP_POINTER

            // GL_OES_rgb8_rgba8
            // GL_RGB8_OES is the same as GL_RGB8
            // GL_RGBA8_OES is the same as GL_RGBA8

            // GL_OES_standard_derivatives
            // GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES is the same as GL_FRAGMENT_SHADER_DERIVATIVE_HINT

            // GL_OES_texture_mirrored_repeat
            // GL_MIRRORED_REPEAT_OES is the same as GL_MIRRORED_REPEAT
#endif // ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        default:
            retVal = false;
            break;
    }

#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        checkOpenGLARBExtensionsEnum
// Description: Check if this is an OpenGL ARB extensions enumeration:
// Arguments:   GLenum enumValue
//              gtString& valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkOpenGLARBExtensionsEnum(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;

    switch (enumValue)
    {
            // GL_ARB_multitexture
            // Included in OpenGL 1.3
            // GL_TEXTURE0_ARB - Is the same as GL_TEXTURE0.
            // GL_TEXTURE1_ARB - Is the same as GL_TEXTURE1.
            // GL_TEXTURE2_ARB - Is the same as GL_TEXTURE2.
            // GL_TEXTURE3_ARB - Is the same as GL_TEXTURE3.
            // GL_TEXTURE4_ARB - Is the same as GL_TEXTURE4.
            // GL_TEXTURE5_ARB - Is the same as GL_TEXTURE5.
            // GL_TEXTURE6_ARB - Is the same as GL_TEXTURE6.
            // GL_TEXTURE7_ARB - Is the same as GL_TEXTURE7.
            // GL_TEXTURE8_ARB - Is the same as GL_TEXTURE8.
            // GL_TEXTURE9_ARB - Is the same as GL_TEXTURE9.
            // GL_TEXTURE10_ARB - Is the same as GL_TEXTURE10.
            // GL_TEXTURE11_ARB - Is the same as GL_TEXTURE11.
            // GL_TEXTURE12_ARB - Is the same as GL_TEXTURE12.
            // GL_TEXTURE13_ARB - Is the same as GL_TEXTURE13.
            // GL_TEXTURE14_ARB - Is the same as GL_TEXTURE14.
            // GL_TEXTURE15_ARB - Is the same as GL_TEXTURE15.
            // GL_TEXTURE16_ARB - Is the same as GL_TEXTURE16.
            // GL_TEXTURE17_ARB - Is the same as GL_TEXTURE17.
            // GL_TEXTURE18_ARB - Is the same as GL_TEXTURE18.
            // GL_TEXTURE19_ARB - Is the same as GL_TEXTURE19.
            // GL_TEXTURE20_ARB - Is the same as GL_TEXTURE20.
            // GL_TEXTURE21_ARB - Is the same as GL_TEXTURE21.
            // GL_TEXTURE22_ARB - Is the same as GL_TEXTURE22.
            // GL_TEXTURE23_ARB - Is the same as GL_TEXTURE23.
            // GL_TEXTURE24_ARB - Is the same as GL_TEXTURE24.
            // GL_TEXTURE25_ARB - Is the same as GL_TEXTURE25.
            // GL_TEXTURE26_ARB - Is the same as GL_TEXTURE26.
            // GL_TEXTURE27_ARB - Is the same as GL_TEXTURE27.
            // GL_TEXTURE28_ARB - Is the same as GL_TEXTURE28.
            // GL_TEXTURE29_ARB - Is the same as GL_TEXTURE29.
            // GL_TEXTURE30_ARB - Is the same as GL_TEXTURE30.
            // GL_TEXTURE31_ARB - Is the same as GL_TEXTURE31.
            // GL_ACTIVE_TEXTURE_ARB - Is the same as GL_ACTIVE_TEXTURE.
            // GL_CLIENT_ACTIVE_TEXTURE_ARB - Is the same as GL_CLIENT_ACTIVE_TEXTURE.
            // GL_MAX_TEXTURE_UNITS_ARB - Is the same as GL_MAX_TEXTURE_UNITS.

            // GL_ARB_transpose_matrix
            // Included in OpenGL 1.3
            // GL_TRANSPOSE_MODELVIEW_MATRIX_ARB - Is the same as GL_TRANSPOSE_MODELVIEW_MATRIX.
            // GL_TRANSPOSE_PROJECTION_MATRIX_ARB - Is the same as GL_TRANSPOSE_PROJECTION_MATRIX.
            // GL_TRANSPOSE_TEXTURE_MATRIX_ARB - Is the same as GL_TRANSPOSE_TEXTURE_MATRIX.
            // GL_TRANSPOSE_COLOR_MATRIX_ARB - Is the same as GL_TRANSPOSE_COLOR_MATRIX.

            // GL_ARB_multisample
            // Included in OpenGL 1.3
            // GL_MULTISAMPLE_ARB - Is the same as GL_MULTISAMPLE.
            // GL_SAMPLE_ALPHA_TO_COVERAGE_ARB - Is the same as GL_SAMPLE_ALPHA_TO_COVERAGE.
            // GL_SAMPLE_ALPHA_TO_ONE_ARB - Is the same as GL_SAMPLE_ALPHA_TO_ONE.
            // GL_SAMPLE_COVERAGE_ARB - Is the same as GL_SAMPLE_COVERAGE.
            // GL_SAMPLE_BUFFERS_ARB - Is the same as GL_SAMPLE_BUFFERS.
            // GL_SAMPLES_ARB - Is the same as GL_SAMPLES.
            // GL_SAMPLE_COVERAGE_VALUE_ARB - Is the same as GL_SAMPLE_COVERAGE_VALUE.
            // GL_SAMPLE_COVERAGE_INVERT_ARB - Is the same as GL_SAMPLE_COVERAGE_INVERT.
            // GL_MULTISAMPLE_BIT_ARB - Is the same as GL_MULTISAMPLE_BIT.

            // GL_ARB_texture_env_add
            // Included in OpenGL 1.3

            // GL_ARB_texture_cube_map
            // Included in OpenGL 1.3
            // GL_NORMAL_MAP_ARB - Is the same as GL_NORMAL_MAP.
            // GL_REFLECTION_MAP_ARB - Is the same as GL_REFLECTION_MAP.
            // GL_TEXTURE_CUBE_MAP_ARB - Is the same as GL_TEXTURE_CUBE_MAP.
            // GL_TEXTURE_BINDING_CUBE_MAP_ARB - Is the same as GL_TEXTURE_BINDING_CUBE_MAP.
            // GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB - Is the same as GL_TEXTURE_CUBE_MAP_POSITIVE_X.
            // GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB - Is the same as GL_TEXTURE_CUBE_MAP_NEGATIVE_X.
            // GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB - Is the same as GL_TEXTURE_CUBE_MAP_POSITIVE_Y.
            // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB - Is the same as GL_TEXTURE_CUBE_MAP_NEGATIVE_Y.
            // GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB - Is the same as GL_TEXTURE_CUBE_MAP_POSITIVE_Z.
            // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB - Is the same as GL_TEXTURE_CUBE_MAP_NEGATIVE_Z.
            // GL_PROXY_TEXTURE_CUBE_MAP_ARB - Is the same as GL_PROXY_TEXTURE_CUBE_MAP.
            // GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB - Is the same as GL_MAX_CUBE_MAP_TEXTURE_SIZE.

            // GL_ARB_texture_compression:
            // Included in OpenGL 1.3
            // GL_COMPRESSED_ALPHA_ARB - Is the same as GL_COMPRESSED_ALPHA
            // GL_COMPRESSED_LUMINANCE_ARB - Is the same as GL_COMPRESSED_LUMINANCE
            // GL_COMPRESSED_LUMINANCE_ALPHA_ARB - Is the same as GL_COMPRESSED_LUMINANCE_ALPHA
            // GL_COMPRESSED_INTENSITY_ARB - Is the same as GL_COMPRESSED_INTENSITY
            // GL_COMPRESSED_RGB_ARB - Is the same as GL_COMPRESSED_RGB
            // GL_COMPRESSED_RGBA_ARB - Is the same as GL_COMPRESSED_RGBA
            // GL_TEXTURE_COMPRESSION_HINT_ARB - Is the same as GL_TEXTURE_COMPRESSION_HINT
            // GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB - Is the same as GL_TEXTURE_COMPRESSED_IMAGE_SIZE
            // GL_TEXTURE_COMPRESSED_ARB - Is the same as GL_TEXTURE_COMPRESSED
            // GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB - Is the same as GL_NUM_COMPRESSED_TEXTURE_FORMATS
            // GL_COMPRESSED_TEXTURE_FORMATS_ARB - Is the same as GL_COMPRESSED_TEXTURE_FORMATS

            // GL_ARB_texture_border_clamp
            // Included in OpenGL 1.3
            // GL_CLAMP_TO_BORDER_ARB - Is the same as GL_CLAMP_TO_BORDER

            // GL_ARB_point_parameters
            // Included in OpenGL 1.4
            // GL_POINT_SIZE_MIN_ARB - Is the same as GL_POINT_SIZE_MIN
            // GL_POINT_SIZE_MAX_ARB - Is the same as GL_POINT_SIZE_MAX
            // GL_POINT_FADE_THRESHOLD_SIZE_ARB - Is the same as GL_POINT_FADE_THRESHOLD_SIZE
            // GL_POINT_DISTANCE_ATTENUATION_ARB - Is the same as GL_POINT_DISTANCE_ATTENUATION

            // GL_ARB_vertex_blend
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_UNITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_VERTEX_UNITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_WEIGHT_SUM_UNITY_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_BLEND_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_WEIGHT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_WEIGHT_ARRAY_TYPE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_WEIGHT_ARRAY_STRIDE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_WEIGHT_ARRAY_SIZE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_WEIGHT_ARRAY_POINTER_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_WEIGHT_ARRAY_ARB);
            // GL_MODELVIEW0_ARB - Is the same as GL_MODELVIEW.
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW1_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW2_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW3_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW4_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW5_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW6_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW7_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW8_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW9_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW10_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW11_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW12_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW13_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW14_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW15_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW16_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW17_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW18_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW19_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW20_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW21_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW22_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW23_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW24_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW25_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW26_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW27_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW28_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW29_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW30_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW31_ARB);

            // GL_ARB_matrix_palette
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX_PALETTE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PALETTE_MATRICES_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_PALETTE_MATRIX_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX_INDEX_ARRAY_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_MATRIX_INDEX_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX_INDEX_ARRAY_SIZE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX_INDEX_ARRAY_TYPE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX_INDEX_ARRAY_STRIDE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX_INDEX_ARRAY_POINTER_ARB);

            // GL_ARB_texture_env_combine
            // Included in OpenGL 1.3
            // GL_COMBINE_ARB - Is the same as GL_COMBINE.
            // GL_COMBINE_RGB_ARB - Is the same as GL_COMBINE_RGB.
            // GL_COMBINE_ALPHA_ARB - Is the same as GL_COMBINE_ALPHA.
            // GL_SOURCE0_RGB_ARB - Is the same as GL_SOURCE0_RGB.
            // GL_SOURCE1_RGB_ARB - Is the same as GL_SOURCE1_RGB.
            // GL_SOURCE2_RGB_ARB - Is the same as GL_SOURCE2_RGB.
            // GL_SOURCE0_ALPHA_ARB - Is the same as GL_SOURCE0_ALPHA.
            // GL_SOURCE1_ALPHA_ARB - Is the same as GL_SOURCE1_ALPHA.
            // GL_SOURCE2_ALPHA_ARB - Is the same as GL_SOURCE2_ALPHA.
            // GL_OPERAND0_RGB_ARB - Is the same as GL_OPERAND0_RGB.
            // GL_OPERAND1_RGB_ARB - Is the same as GL_OPERAND1_RGB.
            // GL_OPERAND2_RGB_ARB - Is the same as GL_OPERAND2_RGB.
            // GL_OPERAND0_ALPHA_ARB - Is the same as GL_OPERAND0_ALPHA.
            // GL_OPERAND1_ALPHA_ARB - Is the same as GL_OPERAND1_ALPHA.
            // GL_OPERAND2_ALPHA_ARB - Is the same as GL_OPERAND2_ALPHA.
            // GL_RGB_SCALE_ARB - Is the same as GL_RGB_SCALE.
            // GL_ADD_SIGNED_ARB - Is the same as GL_ADD_SIGNED.
            // GL_INTERPOLATE_ARB - Is the same as GL_INTERPOLATE
            // GL_SUBTRACT_ARB - Is the same as GL_SUBTRACT
            // GL_CONSTANT_ARB - Is the same as GL_CONSTANT
            // GL_PRIMARY_COLOR_ARB - Is the same as GL_PRIMARY_COLOR
            // GL_PREVIOUS_ARB - Is the same as GL_PREVIOUS

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

            // GL_ARB_texture_env_combine (apple-only addition)
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE3_RGB_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE4_RGB_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE5_RGB_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE6_RGB_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE7_RGB_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE3_ALPHA_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE4_ALPHA_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE5_ALPHA_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE6_ALPHA_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE7_ALPHA_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND3_RGB_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND4_RGB_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND5_RGB_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND6_RGB_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND7_RGB_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND3_ALPHA_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND4_ALPHA_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND5_ALPHA_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND6_ALPHA_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND7_ALPHA_ARB);
#endif

            // GL_ARB_texture_env_crossbar
            // Included in OpenGL 1.4

            // GL_ARB_texture_env_dot3
            // Included in OpenGL 1.3
            // GL_DOT3_RGB_ARB - Is the same as GL_DOT3_RGB
            // GL_DOT3_RGBA_ARB - Is the same as GL_DOT3_RGBA

            // GL_ARB_texture_mirrored_repeat
            // Included in OpenGL 1.4
            // GL_MIRRORED_REPEAT_ARB - Is the same as GL_MIRRORED_REPEAT.

            // GL_ARB_depth_texture
            // Included in OpenGL 1.4
            // GL_DEPTH_COMPONENT16_ARB - Is the same as GL_DEPTH_COMPONENT16.
            // GL_DEPTH_COMPONENT24_ARB - Is the same as GL_DEPTH_COMPONENT24.
            // GL_DEPTH_COMPONENT32_ARB - Is the same as GL_DEPTH_COMPONENT32.
            // GL_TEXTURE_DEPTH_SIZE_ARB - Is the same as GL_TEXTURE_DEPTH_SIZE.
            // GL_DEPTH_TEXTURE_MODE_ARB - Is the same as GL_DEPTH_TEXTURE_MODE.

            // GL_ARB_shadow
            // Included in OpenGL 1.4
            // GL_TEXTURE_COMPARE_MODE_ARB - Is the same as GL_TEXTURE_COMPARE_MODE.
            // GL_TEXTURE_COMPARE_FUNC_ARB - Is the same as GL_TEXTURE_COMPARE_FUNC.
            // GL_COMPARE_R_TO_TEXTURE_ARB - Is the same as GL_COMPARE_R_TO_TEXTURE.

            // GL_ARB_shadow_ambient
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COMPARE_FAIL_VALUE_ARB);

            // GL_ARB_window_pos
            // Included in OpenGL 1.4

            // GL_ARB_vertex_program
            // GL_COLOR_SUM_ARB - Is the same as GL_COLOR_SUM.
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_PROGRAM_ARB);
            // GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB - Is the same as GL_VERTEX_ATTRIB_ARRAY_ENABLED.
            // GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB - Is the same as GL_VERTEX_ATTRIB_ARRAY_SIZE.
            // GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB  - Is the same as GL_VERTEX_ATTRIB_ARRAY_STRIDE.
            // GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB  - Is the same as GL_VERTEX_ATTRIB_ARRAY_TYPE.
            // GL_CURRENT_VERTEX_ATTRIB_ARB  - Is the same as GL_CURRENT_VERTEX_ATTRIB.
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_LENGTH_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_STRING_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_MATRICES_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_MATRIX_STACK_DEPTH_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_MATRIX_ARB);
            // GL_VERTEX_PROGRAM_POINT_SIZE_ARB  - Is the same as GL_VERTEX_PROGRAM_POINT_SIZE
            // GL_VERTEX_PROGRAM_TWO_SIDE_ARB  - Is the same as GL_VERTEX_PROGRAM_TWO_SIDE
            // GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB  - Is the same as GL_VERTEX_ATTRIB_ARRAY_POINTER
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_ERROR_POSITION_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_BINDING_ARB);
            // GL_PROGRAM_NAME_ARB is the same as GL_PROGRAM_BINDING_ARB (Apple only)
            // GL_MAX_VERTEX_ATTRIBS_ARB  - Is the same as GL_MAX_VERTEX_ATTRIBS.
            // GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB  - Is the same as GL_VERTEX_ATTRIB_ARRAY_NORMALIZED.
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_ERROR_STRING_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_FORMAT_ASCII_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_FORMAT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_INSTRUCTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_INSTRUCTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_TEMPORARIES_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_TEMPORARIES_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_NATIVE_TEMPORARIES_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_PARAMETERS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_PARAMETERS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_NATIVE_PARAMETERS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_ATTRIBS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_ATTRIBS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_NATIVE_ATTRIBS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_ADDRESS_REGISTERS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_ENV_PARAMETERS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSPOSE_CURRENT_MATRIX_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX0_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX1_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX2_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX3_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX4_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX5_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX6_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX7_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX8_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX9_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX10_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX11_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX12_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX13_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX14_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX15_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX16_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX17_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX18_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX19_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX20_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX21_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX22_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX23_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX24_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX25_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX26_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX27_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX28_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX29_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX30_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX31_ARB);

            // GL_ARB_fragment_program
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_PROGRAM_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_ALU_INSTRUCTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_TEX_INSTRUCTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_TEX_INDIRECTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB);
            // GL_MAX_TEXTURE_COORDS_ARB  - Is the same as GL_MAX_TEXTURE_COORDS.
            // GL_MAX_TEXTURE_IMAGE_UNITS_ARB - Is the same as GL_MAX_TEXTURE_IMAGE_UNITS.

            // GL_ARB_vertex_buffer_object
            // Included in OpenGL 1.5
            // GL_BUFFER_SIZE_ARB - Is the same as GL_BUFFER_SIZE.
            // GL_BUFFER_USAGE_ARB - Is the same as GL_BUFFER_USAGE.
            // GL_ARRAY_BUFFER_ARB - Is the same as GL_ARRAY_BUFFER.
            // GL_ELEMENT_ARRAY_BUFFER_ARB - Is the same as GL_ELEMENT_ARRAY_BUFFER.
            // GL_ARRAY_BUFFER_BINDING_ARB - Is the same as GL_ARRAY_BUFFER_BINDING.
            // GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB - Is the same as GL_ELEMENT_ARRAY_BUFFER_BINDING.
            // GL_VERTEX_ARRAY_BUFFER_BINDING_ARB - Is the same as GL_VERTEX_ARRAY_BUFFER_BINDING.
            // GL_NORMAL_ARRAY_BUFFER_BINDING_ARB - Is the same as GL_NORMAL_ARRAY_BUFFER_BINDING.
            // GL_COLOR_ARRAY_BUFFER_BINDING_ARB - Is the same as GL_COLOR_ARRAY_BUFFER_BINDING.
            // GL_INDEX_ARRAY_BUFFER_BINDING_ARB - Is the same as GL_INDEX_ARRAY_BUFFER_BINDING.
            // GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB - Is the same as GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING.
            // GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB - Is the same as GL_EDGE_FLAG_ARRAY_BUFFER_BINDING.
            // GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB - Is the same as GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING.
            // GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB - Is the same as GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING.
            // GL_FOG_COORD_ARRAY_BUFFER_BINDING_ARB is the same as GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING (Apple only).
            // GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB - Is the same as GL_WEIGHT_ARRAY_BUFFER_BINDING.
            // GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB - Is the same as GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING.
            // GL_READ_ONLY_ARB - Is the same as GL_READ_ONLY.
            // GL_WRITE_ONLY_ARB - Is the same as GL_WRITE_ONLY.
            // GL_READ_WRITE_ARB - Is the same as GL_READ_WRITE.
            // GL_BUFFER_ACCESS_ARB - Is the same as GL_BUFFER_ACCESS.
            // GL_BUFFER_MAPPED_ARB - Is the same as GL_BUFFER_MAPPED.
            // GL_BUFFER_MAP_POINTER_ARB - Is the same as GL_BUFFER_MAP_POINTER.
            // GL_STREAM_DRAW_ARB - Is the same as GL_STREAM_DRAW.
            // GL_STREAM_READ_ARB - Is the same as GL_STREAM_READ.
            // GL_STREAM_COPY_ARB - Is the same as GL_STREAM_COPY.
            // GL_STATIC_DRAW_ARB - Is the same as GL_STATIC_DRAW.
            // GL_STATIC_READ_ARB - Is the same as GL_STATIC_READ.
            // GL_STATIC_COPY_ARB - Is the same as GL_STATIC_COPY.
            // GL_DYNAMIC_DRAW_ARB - Is the same as GL_DYNAMIC_DRAW.
            // GL_DYNAMIC_READ_ARB - Is the same as GL_DYNAMIC_READ.
            // GL_DYNAMIC_COPY_ARB - Is the same as GL_DYNAMIC_COPY.

            // GL_ARB_occlusion_query
            // Included in OpenGL 1.5
            // GL_QUERY_COUNTER_BITS_ARB - Is the same as GL_QUERY_COUNTER_BITS.
            // GL_CURRENT_QUERY_ARB - Is the same as GL_CURRENT_QUERY.
            // GL_QUERY_RESULT_ARB - Is the same as GL_QUERY_RESULT.
            // GL_QUERY_RESULT_AVAILABLE_ARB - Is the same as GL_QUERY_RESULT_AVAILABLE.
            // GL_SAMPLES_PASSED_ARB - Is the same as GL_SAMPLES_PASSED

            // GL_ARB_shader_objects
            // Included in OpenGL 2.0 with some changes
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_OBJECT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_OBJECT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_OBJECT_TYPE_ARB);
            // GL_OBJECT_SUBTYPE_ARB - Is the same as  GL_SHADER_TYPE
            // GL_FLOAT_VEC2_ARB - Is the same as GL_FLOAT_VEC2.
            // GL_FLOAT_VEC3_ARB  - Is the same as GL_FLOAT_VEC3
            // GL_FLOAT_VEC4_ARB  - Is the same as GL_FLOAT_VEC4
            // GL_INT_VEC2_ARB  - Is the same as GL_INT_VEC2
            // GL_INT_VEC3_ARB  - Is the same as GL_INT_VEC3
            // GL_INT_VEC4_ARB  - Is the same as GL_INT_VEC4
            // GL_BOOL_ARB  - Is the same as GL_BOOL
            // GL_BOOL_VEC2_ARB  - Is the same as GL_BOOL_VEC2.
            // GL_BOOL_VEC3_ARB  - Is the same as GL_BOOL_VEC3
            // GL_BOOL_VEC4_ARB - Is the same as GL_BOOL_VEC4
            // GL_FLOAT_MAT2_ARB - Is the same as GL_FLOAT_MAT2
            // GL_FLOAT_MAT3_ARB - Is the same as GL_FLOAT_MAT3
            // GL_FLOAT_MAT4_ARB  - Is the same as GL_FLOAT_MAT4
            // GL_SAMPLER_1D_ARB - Is the same as GL_SAMPLER_1D
            // GL_SAMPLER_2D_ARB  - Is the same as GL_SAMPLER_2D
            // GL_SAMPLER_3D_ARB- Is the same as GL_SAMPLER_3D
            // GL_SAMPLER_CUBE_ARB  - Is the same as GL_SAMPLER_CUBE
            // GL_SAMPLER_1D_SHADOW_ARB  - Is the same as GL_SAMPLER_1D_SHADOW
            // GL_SAMPLER_2D_SHADOW_ARB  - Is the same as GL_SAMPLER_2D_SHADOW
            // GL_SAMPLER_2D_RECT_ARB - Is the same as GL_SAMPLER_2D_RECT
            // GL_SAMPLER_2D_RECT_SHADOW_ARB - Is the same as GL_SAMPLER_2D_RECT_SHADOW
            // GL_OBJECT_DELETE_STATUS_ARB  - Is the same as GL_DELETE_STATUS
            // GL_OBJECT_COMPILE_STATUS_ARB  - Is the same as GL_COMPILE_STATUS
            // GL_OBJECT_LINK_STATUS_ARB  - Is the same as GL_LINK_STATUS
            // GL_OBJECT_VALIDATE_STATUS_ARB  - Is the same as GL_VALIDATE_STATUS
            // GL_OBJECT_INFO_LOG_LENGTH_ARB  - Is the same as GL_INFO_LOG_LENGTH
            // GL_OBJECT_ATTACHED_OBJECTS_ARB  - Is the same as GL_ATTACHED_SHADERS
            // GL_OBJECT_ACTIVE_UNIFORMS_ARB  - Is the same as GL_ACTIVE_UNIFORMS
            // GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB  - Is the same as GL_ACTIVE_UNIFORM_MAX_LENGTH
            // GL_OBJECT_SHADER_SOURCE_LENGTH_ARB  - Is the same as GL_SHADER_SOURCE_LENGTH

            // GL_ARB_vertex_shader
            // Included in OpenGL 2.0 with some changes
            // GL_VERTEX_SHADER_ARB  - Is the same as GL_VERTEX_SHADER
            // GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB  - Is the same as GL_MAX_VERTEX_UNIFORM_COMPONENTS
            // GL_MAX_VARYING_FLOATS_ARB  - Is the same as GL_MAX_VARYING_FLOATS
            // GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB  - Is the same as GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS
            // GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB  - Is the same as GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
            // GL_OBJECT_ACTIVE_ATTRIBUTES_ARB  - Is the same as GL_ACTIVE_ATTRIBUTES
            // GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB  - Is the same as GL_ACTIVE_ATTRIBUTE_MAX_LENGTH

            // GL_ARB_fragment_shader
            // Included in OpenGL 2.0 with some changes
            // GL_FRAGMENT_SHADER_ARB  - Is the same as GL_FRAGMENT_SHADER
            // GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB  - Is the same as GL_MAX_FRAGMENT_UNIFORM_COMPONENTS
            // GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB  - Is the same as GL_FRAGMENT_SHADER_DERIVATIVE_HINT

            // GL_ARB_shading_language_100
            // Included in OpenGL 2.0 with some changes
            // GL_SHADING_LANGUAGE_VERSION_ARB  - Is the same as GL_SHADING_LANGUAGE_VERSION

            // GL_ARB_texture_non_power_of_two
            // Included in OpenGL 2.0

            // GL_ARB_point_sprite
            // Included in OpenGL 2.0
            // GL_POINT_SPRITE_ARB  - Is the same as GL_POINT_SPRITE
            // GL_COORD_REPLACE_ARB  - Is the same as GL_COORD_REPLACE

            // GL_ARB_fragment_program_shadow

            // GL_ARB_draw_buffers
            // Included in OpenGL 2.0
            // GL_MAX_DRAW_BUFFERS_ARB  - Is the same as GL_MAX_DRAW_BUFFERS
            // GL_DRAW_BUFFER0_ARB  - Is the same as GL_DRAW_BUFFER0
            // GL_DRAW_BUFFER1_ARB  - Is the same as GL_DRAW_BUFFER1
            // GL_DRAW_BUFFER2_ARB  - Is the same as GL_DRAW_BUFFER2
            // GL_DRAW_BUFFER3_ARB  - Is the same as GL_DRAW_BUFFER3
            // GL_DRAW_BUFFER4_ARB  - Is the same as GL_DRAW_BUFFER4
            // GL_DRAW_BUFFER5_ARB  - Is the same as GL_DRAW_BUFFER5
            // GL_DRAW_BUFFER6_ARB  - Is the same as GL_DRAW_BUFFER6
            // GL_DRAW_BUFFER7_ARB  - Is the same as GL_DRAW_BUFFER7
            // GL_DRAW_BUFFER8_ARB  - Is the same as GL_DRAW_BUFFER8
            // GL_DRAW_BUFFER9_ARB  - Is the same as GL_DRAW_BUFFER9
            // GL_DRAW_BUFFER10_ARB  - Is the same as GL_DRAW_BUFFER10
            // GL_DRAW_BUFFER11_ARB  - Is the same as GL_DRAW_BUFFER11
            // GL_DRAW_BUFFER12_ARB  - Is the same as GL_DRAW_BUFFER12
            // GL_DRAW_BUFFER13_ARB  - Is the same as GL_DRAW_BUFFER13
            // GL_DRAW_BUFFER14_ARB  - Is the same as GL_DRAW_BUFFER14
            // GL_DRAW_BUFFER15_ARB  - Is the same as GL_DRAW_BUFFER15

            // GL_ARB_texture_rectangle
            // Included in OpenGL 3.1
            // GL_TEXTURE_RECTANGLE_ARB - Is the same as GL_TEXTURE_RECTANGLE
            // GL_TEXTURE_BINDING_RECTANGLE_ARB - Is the same as GL_TEXTURE_BINDING_RECTANGLE
            // GL_PROXY_TEXTURE_RECTANGLE_ARB - Is the same as GL_PROXY_TEXTURE_RECTANGLE
            // GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB - Is the same as GL_MAX_RECTANGLE_TEXTURE_SIZE

            // GL_ARB_color_buffer_float
            // Included in OpenGL 3.0
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA_FLOAT_MODE_ARB);
            // GL_CLAMP_VERTEX_COLOR_ARB is equal to GL_CLAMP_VERTEX_COLOR
            // GL_CLAMP_FRAGMENT_COLOR_ARB is equal to  GL_CLAMP_FRAGMENT_COLOR
            // GL_CLAMP_READ_COLOR_ARB is equal to GL_CLAMP_READ_COLOR
            // GL_FIXED_ONLY_ARB is equal to GL_FIXED_ONLY

            // GL_ARB_half_float_pixel
            // GL_HALF_FLOAT_ARB is equal to GL_HALF_FLOAT

            // GL_ARB_instanced_arrays:
            // Included in OpenGL 3.3
            // AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB); // Same asGL_VERTEX_ATTRIB_ARRAY_DIVISOR

            // GL_ARB_texture_float
            // Included in OpenGL 3.0
            // GL_TEXTURE_RED_TYPE_ARB is equal to GL_TEXTURE_RED_TYPE
            // GL_TEXTURE_GREEN_TYPE_ARB is equal to GL_TEXTURE_GREEN_TYPE
            // GL_TEXTURE_BLUE_TYPE_ARB is equal to GL_TEXTURE_BLUE_TYPE
            // GL_TEXTURE_ALPHA_TYPE_ARB is equal to GL_TEXTURE_ALPHA_TYPE
            // GL_TEXTURE_LUMINANCE_TYPE_ARB is equal to GL_TEXTURE_LUMINANCE_TYPE
            // GL_TEXTURE_INTENSITY_TYPE_ARB is equal to GL_TEXTURE_INTENSITY_TYPE
            // GL_TEXTURE_DEPTH_TYPE_ARB is equal to GL_TEXTURE_DEPTH_TYPE
            // GL_UNSIGNED_NORMALIZED_ARB is equal to GL_UNSIGNED_NORMALIZED
            // GL_RGBA32F_ARB is equal to GL_RGBA32F
            // GL_RGB32F_ARB is equal to GL_RGB32F
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA32F_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY32F_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE32F_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE_ALPHA32F_ARB);
            // GL_RGBA16F_ARB is equal to GL_RGBA16F
            // GL_RGB16F_ARB is equal to GL_RGB16F
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA16F_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY16F_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE16F_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE_ALPHA16F_ARB);

            // GL_ARB_pixel_buffer_object
            // Included in OpenGL 2.1
            // GL_PIXEL_PACK_BUFFER_ARB is equal to GL_PIXEL_PACK_BUFFER
            // GL_PIXEL_UNPACK_BUFFER_ARB is equal to GL_PIXEL_UNPACK_BUFFER
            // GL_PIXEL_PACK_BUFFER_BINDING_ARB is equal to GL_PIXEL_PACK_BUFFER_BINDING
            // GL_PIXEL_UNPACK_BUFFER_BINDING_ARB is equal to GL_PIXEL_UNPACK_BUFFER_BINDING

            // GL_ARB_depth_buffer_float
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_COMPONENT32F);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH32F_STENCIL8);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_32_UNSIGNED_INT_24_8_REV);

            // GL_ARB_draw_instanced
            // Included in OpenGL 3.1

            // GL_ARB_framebuffer_object
            // Included in OpenGL 3.0
            AP_GL_ENUM_TOSTRING_CASE(GL_INVALID_FRAMEBUFFER_OPERATION);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_DEFAULT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_UNDEFINED);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_STENCIL_ATTACHMENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_RENDERBUFFER_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_STENCIL);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_24_8);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH24_STENCIL8);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_STENCIL_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_BINDING);
            // GL_DRAW_FRAMEBUFFER_BINDING is equal to GL_FRAMEBUFFER_BINDING
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_READ_FRAMEBUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_FRAMEBUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_READ_FRAMEBUFFER_BINDING);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER_SAMPLES);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_COMPLETE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_UNSUPPORTED);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COLOR_ATTACHMENTS);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT0);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT1);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT2);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT3);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT4);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT5);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT6);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT7);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT8);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT9);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT10);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT11);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT12);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT13);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT14);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT15);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT16);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT17);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT18);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT19);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT20);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT21);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT22);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT23);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT24);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT25);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT26);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT27);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT28);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT29);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT30);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ATTACHMENT31);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_ATTACHMENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_ATTACHMENT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER_WIDTH);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER_HEIGHT);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER_INTERNAL_FORMAT);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_INDEX1);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_INDEX4);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_INDEX8);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_INDEX16);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER_RED_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER_GREEN_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER_BLUE_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER_ALPHA_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER_DEPTH_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER_STENCIL_SIZE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SAMPLES);

            // GL_ARB_framebuffer_sRGB
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_SRGB);

            // GL_ARB_geometry_shader4
            // Included in OpenGL 3.2
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_VERTICES_OUT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_INPUT_TYPE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_OUTPUT_TYPE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_VARYING_COMPONENTS_ARB);
            // GL_LINES_ADJACENCY_ARB is equal to GL_LINES_ADJACENCY
            // GL_LINE_STRIP_ADJACENCY_ARB is equal to GL_LINE_STRIP_ADJACENCY
            // GL_TRIANGLES_ADJACENCY_ARB is equal to GL_TRIANGLES_ADJACENCY
            // GL_TRIANGLE_STRIP_ADJACENCY_ARB is equal to GL_TRIANGLE_STRIP_ADJACENCY
            // GL_PROGRAM_POINT_SIZE_ARB is equal to GL_PROGRAM_POINT_SIZE
            // GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB is equal to GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS
            // GL_FRAMEBUFFER_ATTACHMENT_LAYERED_ARB is equal to GL_FRAMEBUFFER_ATTACHMENT_LAYERED
            // GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB is equal to GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
            // GL_GEOMETRY_SHADER_ARB is equal to GL_GEOMETRY_SHADER
            // GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB is equal to GL_MAX_GEOMETRY_UNIFORM_COMPONENTS
            // GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB is equal to GL_MAX_GEOMETRY_OUTPUT_VERTICES
            // GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB is equal to GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS

            // GL_ARB_half_float_vertex
            AP_GL_ENUM_TOSTRING_CASE(GL_HALF_FLOAT);

            // GL_ARB_instanced_arrays

            // GL_ARB_map_buffer_range

            // GL_ARB_texture_buffer_object
            // Included in OpenGL 3.1
            // GL_TEXTURE_BUFFER_ARB - Is the same as GL_TEXTURE_BUFFER
            // GL_MAX_TEXTURE_BUFFER_SIZE_ARB - Is the same as GL_MAX_TEXTURE_BUFFER_SIZE
            // GL_TEXTURE_BUFFER_DATA_STORE_BINDING_ARB - Is the same as GL_TEXTURE_BUFFER_DATA_STORE_BINDING
            // GL_TEXTURE_BUFFER_FORMAT_ARB - Is the same as GL_TEXTURE_BUFFER_FORMAT

            // GL_ARB_texture_compression_rgtc
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RED_RGTC1);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SIGNED_RED_RGTC1);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RG_RGTC2);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SIGNED_RG_RGTC2);

            // GL_ARB_texture_rg
            AP_GL_ENUM_TOSTRING_CASE(GL_RG);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG_INTEGER);
            AP_GL_ENUM_TOSTRING_CASE(GL_R8);
            AP_GL_ENUM_TOSTRING_CASE(GL_R16);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG8);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG16);
            AP_GL_ENUM_TOSTRING_CASE(GL_R16F);
            AP_GL_ENUM_TOSTRING_CASE(GL_R32F);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG16F);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG32F);
            AP_GL_ENUM_TOSTRING_CASE(GL_R8I);
            AP_GL_ENUM_TOSTRING_CASE(GL_R8UI);
            AP_GL_ENUM_TOSTRING_CASE(GL_R16I);
            AP_GL_ENUM_TOSTRING_CASE(GL_R16UI);
            AP_GL_ENUM_TOSTRING_CASE(GL_R32I);
            AP_GL_ENUM_TOSTRING_CASE(GL_R32UI);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG8I);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG8UI);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG16I);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG16UI);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG32I);
            AP_GL_ENUM_TOSTRING_CASE(GL_RG32UI);

            // GL_ARB_uniform_buffer_object
            // Included in OpenGL 3.1
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER_BINDING);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER_START);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER_SIZE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_UNIFORM_BLOCKS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_UNIFORM_BLOCKS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_UNIFORM_BLOCKS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_UNIFORM_BLOCKS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_UNIFORM_BUFFER_BINDINGS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_UNIFORM_BLOCK_SIZE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_UNIFORM_BLOCKS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_TYPE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_SIZE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_NAME_LENGTH);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_INDEX);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_OFFSET);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_ARRAY_STRIDE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_MATRIX_STRIDE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_IS_ROW_MAJOR);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_BINDING);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_DATA_SIZE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_NAME_LENGTH);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER);

            // GL_ARB_vertex_array_object
            // Included in OpenGL 3.0
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_BINDING);

            // GL_ARB_provoking_vertex
            // Included in OpenGL 3.2
            // AP_GL_ENUM_TOSTRING_CASE(GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION);
            // AP_GL_ENUM_TOSTRING_CASE(GL_FIRST_VERTEX_CONVENTION);
            // AP_GL_ENUM_TOSTRING_CASE(GL_LAST_VERTEX_CONVENTION);
            // AP_GL_ENUM_TOSTRING_CASE(GL_PROVOKING_VERTEX);

            // GL_ARB_seamless_cube_map
            // Included in OpenGL 3.2
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CUBE_MAP_SEAMLESS);

            // GL_ARB_sync
            // Included in OpenGL 3.2
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SERVER_WAIT_TIMEOUT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_OBJECT_TYPE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SYNC_CONDITION);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SYNC_STATUS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SYNC_FLAGS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SYNC_FENCE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SYNC_GPU_COMMANDS_COMPLETE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNALED);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SIGNALED);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ALREADY_SIGNALED);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TIMEOUT_EXPIRED);
            // AP_GL_ENUM_TOSTRING_CASE(GL_CONDITION_SATISFIED);
            // AP_GL_ENUM_TOSTRING_CASE(GL_WAIT_FAILED);

            // GL_ARB_texture_multisample
            // Included in OpenGL 3.2
            // AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_POSITION);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_MASK);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_MASK_VALUE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SAMPLE_MASK_WORDS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_2D_MULTISAMPLE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_2D_MULTISAMPLE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_2D_MULTISAMPLE_ARRAY);
            // AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_2D_MULTISAMPLE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SAMPLES);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_FIXED_SAMPLE_LOCATIONS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_2D_MULTISAMPLE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_2D_MULTISAMPLE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_2D_MULTISAMPLE_ARRAY);
            // AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COLOR_TEXTURE_SAMPLES);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_DEPTH_TEXTURE_SAMPLES);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_INTEGER_SAMPLES);

            //GL_ARB_viewport_array:
            // Included in OpenGL 4.1
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VIEWPORTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_VIEWPORT_SUBPIXEL_BITS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_VIEWPORT_BOUNDS_RANGE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_LAYER_PROVOKING_VERTEX);
            // AP_GL_ENUM_TOSTRING_CASE(GL_VIEWPORT_INDEX_PROVOKING_VERTEX);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNDEFINED_VERTEX);

            // GL_ARB_cl_event:
            AP_GL_ENUM_TOSTRING_CASE(GL_SYNC_CL_EVENT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SYNC_CL_EVENT_COMPLETE_ARB);

            // GL_ARB_debug_output
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_CALLBACK_FUNCTION_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_CALLBACK_USER_PARAM_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SOURCE_API_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SOURCE_SHADER_COMPILER_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SOURCE_THIRD_PARTY_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SOURCE_APPLICATION_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SOURCE_OTHER_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_ERROR_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_PORTABILITY_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_PERFORMANCE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_TYPE_OTHER_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_DEBUG_MESSAGE_LENGTH_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_DEBUG_LOGGED_MESSAGES_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_LOGGED_MESSAGES_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SEVERITY_HIGH_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SEVERITY_MEDIUM_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SEVERITY_LOW_ARB);

            // GL_ARB_robustness:
            AP_GL_ENUM_TOSTRING_CASE(GL_LOSE_CONTEXT_ON_RESET_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_GUILTY_CONTEXT_RESET_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_INNOCENT_CONTEXT_RESET_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNKNOWN_CONTEXT_RESET_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_RESET_NOTIFICATION_STRATEGY_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_NO_RESET_NOTIFICATION_ARB);

            // GL_ARB_tessellation_shader:
            // Included in OpenGL 4.0
            // AP_GL_ENUM_TOSTRING_CASE(GL_PATCHES);
            // AP_GL_ENUM_TOSTRING_CASE(GL_PATCH_VERTICES);
            // AP_GL_ENUM_TOSTRING_CASE(GL_PATCH_DEFAULT_INNER_LEVEL);
            // AP_GL_ENUM_TOSTRING_CASE(GL_PATCH_DEFAULT_OUTER_LEVEL);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TESS_CONTROL_OUTPUT_VERTICES);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TESS_GEN_MODE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TESS_GEN_SPACING);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TESS_GEN_VERTEX_ORDER);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TESS_GEN_POINT_MODE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ISOLINES);
            // AP_GL_ENUM_TOSTRING_CASE(GL_FRACTIONAL_ODD);
            // AP_GL_ENUM_TOSTRING_CASE(GL_FRACTIONAL_EVEN);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PATCH_VERTICES);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_GEN_LEVEL);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_PATCH_COMPONENTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_CONTROL_INPUT_COMPONENTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TESS_EVALUATION_SHADER);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TESS_CONTROL_SHADER);

            // GL_ARB_texture_buffer_object_rgb32
            // Included in OpenGL 4.0

            // GL_ARB_texture_gather:
            // Included in OpenGL 4.0
            // AP_GL_ENUM_TOSTRING_CASE(GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_ARB); // Same as GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_ARB); // Same as GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET

            // GL_ARB_texture_query_lod:
            // Included in OpenGL 4.0

            // GL_ARB_shading_language_include:
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_INCLUDE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_NAMED_STRING_LENGTH_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_NAMED_STRING_TYPE_ARB);

            // GL_ARB_texture_compression_bptc:
            // Included in OpenGL 4.2
            // AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGBA_BPTC_UNORM_ARB); // same as GL_COMPRESSED_RGBA_BPTC_UNORM
            // AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB); // same as GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM
            // AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB); // same as GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT
            // AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB); // same as GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT

            // GL_ARB_blend_func_extended:
            // Included in OpenGL 3.3
            // AP_GL_ENUM_TOSTRING_CASE(GL_SRC1_COLOR);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ONE_MINUS_SRC1_COLOR);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ONE_MINUS_SRC1_ALPHA);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS);

            // GL_ARB_explicit_attrib_location:
            // Included in OpenGL 3.3

            // GL_ARB_occlusion_query2:
            // Included in OpenGL 3.3
            // AP_GL_ENUM_TOSTRING_CASE(GL_ANY_SAMPLES_PASSED);

            // GL_ARB_sampler_objects:
            // Included in OpenGL 3.3
            // AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_BINDING);

            // GL_ARB_shader_bit_encoding:
            // Included in OpenGL 3.3

            // GL_ARB_texture_rgb10_a2ui:
            // Included in OpenGL 3.3
            // AP_GL_ENUM_TOSTRING_CASE(GL_RGB10_A2UI);

            // GL_ARB_texture_swizzle:
            // Included in OpenGL 3.3
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_R);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_G);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_B);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_A);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_RGBA);

            // GL_ARB_timer_query:
            // Included in OpenGL 3.3
            // AP_GL_ENUM_TOSTRING_CASE(GL_TIME_ELAPSED);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TIMESTAMP);

            // GL_ARB_vertex_type_2_10_10_10_rev:
            // Included in OpenGL 3.3
            // AP_GL_ENUM_TOSTRING_CASE(GL_INT_2_10_10_10_REV);

            // GL_ARB_draw_indirect:
            // Included in OpenGL 4.0
            // AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_INDIRECT_BUFFER);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_INDIRECT_BUFFER_BINDING);

            // GL_ARB_sample_shading
            // Included in OpenGL 4.0
            // AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_SHADING_ARB); // same as GL_SAMPLE_SHADING
            // AP_GL_ENUM_TOSTRING_CASE(GL_MIN_SAMPLE_SHADING_VALUE_ARB); // same as GL_MIN_SAMPLE_SHADING_VALUE

            // GL_ARB_texture_cube_map_array
            // Included in OpenGL 4.0
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CUBE_MAP_ARRAY_ARB); // same as GL_TEXTURE_CUBE_MAP_ARRAY
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_ARB); // same as GL_TEXTURE_BINDING_CUBE_MAP_ARRAY
            // AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_CUBE_MAP_ARRAY_ARB); // same as GL_PROXY_TEXTURE_CUBE_MAP_ARRAY
            // AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_CUBE_MAP_ARRAY_ARB); // same as GL_SAMPLER_CUBE_MAP_ARRAY
            // AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_ARB); // same as GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW
            // AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_CUBE_MAP_ARRAY_ARB); // same as GL_INT_SAMPLER_CUBE_MAP_ARRAY
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_ARB); // same as GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY

            // GL_ARB_gpu_shader5:
            // Included in OpenGL 4.0
            // AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_SHADER_INVOCATIONS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_SHADER_INVOCATIONS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MIN_FRAGMENT_INTERPOLATION_OFFSET);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_INTERPOLATION_OFFSET);
            // AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_INTERPOLATION_OFFSET_BITS);

            // GL_ARB_gpu_shader_fp64:
            // Included in OpenGL 4.0
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_VEC2);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_VEC3);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_VEC4);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT2);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT3);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT4);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT2x3);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT2x4);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT3x2);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT3x4);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT4x2);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT4x3);

            // GL_ARB_shader_subroutine:
            // Included in OpenGL 4.0
            // AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_SUBROUTINES);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_SUBROUTINE_UNIFORMS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_SUBROUTINE_MAX_LENGTH);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SUBROUTINES);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_NUM_COMPATIBLE_SUBROUTINES);
            // AP_GL_ENUM_TOSTRING_CASE(GL_COMPATIBLE_SUBROUTINES);

            // GL_ARB_texture_buffer_object_rgb32:

            // GL_ARB_transform_feedback2:
            // Included in OpenGL 4.0
            // Included in OpenGL 4.0
            // AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BINDING);

            // GL_ARB_transform_feedback3:
            // Included in OpenGL 4.0
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TRANSFORM_FEEDBACK_BUFFERS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_STREAMS);

            // GL_ARB_get_program_binary:
            // Included in OpenGL 4.1
            // AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_BINARY_RETRIEVABLE_HINT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_BINARY_LENGTH);
            // AP_GL_ENUM_TOSTRING_CASE(GL_NUM_PROGRAM_BINARY_FORMATS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_BINARY_FORMATS);

            // GL_ARB_separate_shader_objects:
            // Included in OpenGL 4.1
            // AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_SEPARABLE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_PROGRAM);
            // AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_PIPELINE_BINDING);

            // GL_ARB_shader_precision:
            // Included in OpenGL 4.1

            // GL_ARB_ES2_compatibility
            // Included in OpenGL 4.1
            // AP_GL_ENUM_TOSTRING_CASE(GL_FIXED);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_UNIFORM_VECTORS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VARYING_VECTORS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_UNIFORM_VECTORS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_IMPLEMENTATION_COLOR_READ_TYPE);
            // AP_GL_ENUM_TOSTRING_CASE(GL_IMPLEMENTATION_COLOR_READ_FORMAT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_COMPILER);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_BINARY_FORMATS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_NUM_SHADER_BINARY_FORMATS);
            // AP_GL_ENUM_TOSTRING_CASE(GL_LOW_FLOAT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MEDIUM_FLOAT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_HIGH_FLOAT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_LOW_INT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_MEDIUM_INT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_HIGH_INT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_RGB565);

            // GL_ARB_shader_stencil_export

            // GL_ARB_copy_buffer:
            // Included in OpenGL 3.1
            // AP_GL_ENUM_TOSTRING_CASE(GL_COPY_READ_BUFFER);
            // AP_GL_ENUM_TOSTRING_CASE(GL_COPY_WRITE_BUFFER);

            // GL_ARB_pipeline_statistics_query:
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTICES_SUBMITTED_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_PRIMITIVES_SUBMITTED_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SHADER_INVOCATIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_CONTROL_SHADER_PATCHES_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_SHADER_INVOCATIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPUTE_SHADER_INVOCATIONS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIPPING_INPUT_PRIMITIVES_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIPPING_OUTPUT_PRIMITIVES_ARB);

            // GL_ARB_sparse_buffer:
            // AP_GL_ENUM_TOSTRING_CASE(GL_SPARSE_STORAGE_BIT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPARSE_BUFFER_PAGE_SIZE_ARB);

            // GL_ARB_sparse_texture:
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SPARSE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIRTUAL_PAGE_SIZE_INDEX_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_SPARSE_LEVELS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_VIRTUAL_PAGE_SIZES_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIRTUAL_PAGE_SIZE_X_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIRTUAL_PAGE_SIZE_Y_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIRTUAL_PAGE_SIZE_Z_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SPARSE_TEXTURE_SIZE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SPARSE_3D_TEXTURE_SIZE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIN_SPARSE_LEVEL_ARB);

            // GL_ARB_transform_feedback_overflow_query:
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_OVERFLOW_ARB);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW_ARB);

            // GL_KHR_blend_equation_advanced:
            AP_GL_ENUM_TOSTRING_CASE(GL_MULTIPLY_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_SCREEN_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_OVERLAY_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_DARKEN_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHTEN_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLORDODGE_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLORBURN_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_HARDLIGHT_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOFTLIGHT_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_DIFFERENCE_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_EXCLUSION_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_HSL_HUE_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_HSL_SATURATION_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_HSL_COLOR_KHR);
            AP_GL_ENUM_TOSTRING_CASE(GL_HSL_LUMINOSITY_KHR);

            // GL_KHR_blend_equation_advanced_coherent:
            AP_GL_ENUM_TOSTRING_CASE(GL_BLEND_ADVANCED_COHERENT_KHR);

            // GL_KHR_no_error:
            // AP_GL_ENUM_TOSTRING_CASE(GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR);

            // GL_KHR_robustness:
            AP_GL_ENUM_TOSTRING_CASE(GL_CONTEXT_ROBUST_ACCESS);

        default:
            retVal = false;
    }

    return retVal;
} // checkOpenGLARBExtensionsEnum

// ---------------------------------------------------------------------------
// Name:        checkOpenGLEXTExtensionsEnum
// Description: Check if this is an OpenGL EXT extension
// Arguments:   GLenum enumValue
//              gtString& valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkOpenGLEXTExtensionsEnum(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;

    switch (enumValue)
    {
            // GL_EXT_abgr
            AP_GL_ENUM_TOSTRING_CASE(GL_ABGR_EXT);

            // GL_EXT_blend_color
            // Included in OpenGL 1.2
            // GL_CONSTANT_COLOR_EXT - Is the same as GL_CONSTANT_COLOR
            // GL_ONE_MINUS_CONSTANT_COLOR_EXT - Is the same as GL_ONE_MINUS_CONSTANT_COLOR
            // GL_CONSTANT_ALPHA_EXT - Is the same as GL_CONSTANT_ALPHA
            // GL_ONE_MINUS_CONSTANT_ALPHA_EXT - Is the same as GL_ONE_MINUS_CONSTANT_ALPHA
            // GL_BLEND_COLOR_EXT - Is the same as GL_BLEND_COLOR

            // GL_EXT_polygon_offset
            // Included in OpenGL 1.1
            // GL_POLYGON_OFFSET_EXT - Is the same as GL_POLYGON_OFFSET_FILL
            // GL_POLYGON_OFFSET_FACTOR_EXT - Is the same as GL_POLYGON_OFFSET_FACTOR
            AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON_OFFSET_BIAS_EXT);

            // GL_EXT_texture
            // Included in OpenGL 1.1
            // GL_ALPHA4_EXT - Is the same as GL_ALPHA4
            // GL_ALPHA8_EXT - Is the same as GL_ALPHA8
            // GL_ALPHA12_EXT - Is the same as GL_ALPHA12
            // GL_ALPHA16_EXT - Is the same as GL_ALPHA16
            // GL_LUMINANCE4_EXT - Is the same as GL_LUMINANCE4
            // GL_LUMINANCE8_EXT - Is the same as GL_LUMINANCE8
            // GL_LUMINANCE12_EXT - Is the same as GL_LUMINANCE12
            // GL_LUMINANCE16_EXT - Is the same as GL_LUMINANCE16
            // GL_LUMINANCE4_ALPHA4_EXT - Is the same as GL_LUMINANCE4_ALPHA4
            // GL_LUMINANCE6_ALPHA2_EXT - Is the same as GL_LUMINANCE6_ALPHA2
            // GL_LUMINANCE8_ALPHA8_EXT - Is the same as GL_LUMINANCE8_ALPHA8
            // GL_LUMINANCE12_ALPHA4_EXT - Is the same as GL_LUMINANCE12_ALPHA4
            // GL_LUMINANCE12_ALPHA12_EXT - Is the same as GL_LUMINANCE12_ALPHA12
            // GL_LUMINANCE16_ALPHA16_EXT - Is the same as GL_LUMINANCE16_ALPHA16
            // GL_INTENSITY_EXT - Is the same as GL_INTENSITY
            // GL_INTENSITY4_EXT - Is the same as GL_INTENSITY4
            // GL_INTENSITY8_EXT - Is the same as GL_INTENSITY8
            // GL_INTENSITY12_EXT - Is the same as GL_INTENSITY12
            // GL_INTENSITY16_EXT - Is the same as GL_INTENSITY16
            // GL_RGB2_EXT - Is the same as GL_RGB2
            // GL_RGB4_EXT - Is the same as GL_RGB4
            // GL_RGB5_EXT - Is the same as GL_RGB5
            // GL_RGB8_EXT - Is the same as GL_RGB8
            // GL_RGB10_EXT - Is the same as GL_RGB10
            // GL_RGB12_EXT - Is the same as GL_RGB12
            // GL_RGB16_EXT - Is the same as GL_RGB16
            // GL_RGBA2_EXT - Is the same as GL_RGBA2
            // GL_RGBA4_EXT - Is the same as GL_RGBA4
            // GL_RGB5_A1_EXT - Is the same as GL_RGB5_A1
            // GL_RGBA8_EXT - Is the same as GL_RGBA8
            // GL_RGB10_A2_EXT - Is the same as GL_RGB10_A2
            // GL_RGBA12_EXT - Is the same as GL_RGBA12
            // GL_RGBA16_EXT - Is the same as GL_RGBA16
            // GL_TEXTURE_RED_SIZE_EXT - Is the same as GL_TEXTURE_RED_SIZE
            // GL_TEXTURE_GREEN_SIZE_EXT - Is the same as GL_TEXTURE_GREEN_SIZE
            // GL_TEXTURE_BLUE_SIZE_EXT - Is the same as GL_TEXTURE_BLUE_SIZE
            // GL_TEXTURE_ALPHA_SIZE_EXT - Is the same as GL_TEXTURE_ALPHA_SIZE
            // GL_TEXTURE_LUMINANCE_SIZE_EXT - Is the same as GL_TEXTURE_LUMINANCE_SIZE
            // GL_TEXTURE_INTENSITY_SIZE_EXT - Is the same as GL_TEXTURE_INTENSITY_SIZE
            AP_GL_ENUM_TOSTRING_CASE(GL_REPLACE_EXT);
            // GL_PROXY_TEXTURE_1D_EXT - Is the same as GL_PROXY_TEXTURE_1D
            // GL_PROXY_TEXTURE_2D_EXT - Is the same as GL_PROXY_TEXTURE_2D
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_TOO_LARGE_EXT);

            // GL_EXT_texture3D:
            // Included in OpenGL 1.2
            // GL_PACK_SKIP_IMAGES_EXT - Is the same as GL_PACK_SKIP_IMAGES.
            // GL_PACK_IMAGE_HEIGHT_EXT - Is the same as GL_PACK_IMAGE_HEIGHT.
            // GL_UNPACK_SKIP_IMAGES_EXT - Is the same as GL_UNPACK_SKIP_IMAGES.
            // GL_UNPACK_IMAGE_HEIGHT_EXT - Is the same as GL_UNPACK_IMAGE_HEIGHT.
            // GL_TEXTURE_3D_EXT - Is the same as GL_TEXTURE_3D.
            // GL_PROXY_TEXTURE_3D_EXT - Is the same as GL_PROXY_TEXTURE_3D.
            // GL_TEXTURE_DEPTH_EXT - Is the same as GL_TEXTURE_DEPTH.
            // GL_TEXTURE_WRAP_R_EXT - Is the same as GL_TEXTURE_WRAP_R.
            // GL_MAX_3D_TEXTURE_SIZE_EXT - Is the same as GL_MAX_3D_TEXTURE_SIZE.

            // GL_EXT_subtexture
            // Included in OpenGL 1.1

            // GL_EXT_copy_texture
            // Included in OpenGL 1.1

            // GL_EXT_histogram
            // Included in OpenGL 1.2
            // GL_HISTOGRAM_EXT - Is the same as GL_HISTOGRAM
            // GL_PROXY_HISTOGRAM_EXT - Is the same as GL_PROXY_HISTOGRAM
            // GL_HISTOGRAM_WIDTH_EXT - Is the same as GL_HISTOGRAM_WIDTH
            // GL_HISTOGRAM_FORMAT_EXT - Is the same as GL_HISTOGRAM_FORMAT
            // GL_HISTOGRAM_RED_SIZE_EXT - Is the same as GL_HISTOGRAM_RED_SIZE
            // GL_HISTOGRAM_GREEN_SIZE_EXT - Is the same as GL_HISTOGRAM_GREEN_SIZE
            // GL_HISTOGRAM_BLUE_SIZE_EXT - Is the same as GL_HISTOGRAM_BLUE_SIZE
            // GL_HISTOGRAM_ALPHA_SIZE_EXT - Is the same as GL_HISTOGRAM_ALPHA_SIZE
            // GL_HISTOGRAM_LUMINANCE_SIZE_EXT - Is the same as GL_HISTOGRAM_LUMINANCE_SIZE
            // GL_HISTOGRAM_SINK_EXT - Is the same as GL_HISTOGRAM_SINK
            // GL_MINMAX_EXT - Is the same as GL_MINMAX
            // GL_MINMAX_FORMAT_EXT - Is the same as GL_MINMAX_FORMAT
            // GL_MINMAX_SINK_EXT - Is the same as GL_MINMAX_SINK
            // GL_TABLE_TOO_LARGE_EXT - Is the same as GL_TABLE_TOO_LARGE

            // GL_EXT_convolution
            // Included in OpenGL 1.2
            // GL_CONVOLUTION_1D_EXT - Is the same as GL_CONVOLUTION_1D
            // GL_CONVOLUTION_2D_EXT - Is the same as GL_CONVOLUTION_2D
            // GL_SEPARABLE_2D_EXT - Is the same as GL_SEPARABLE_2D
            // GL_CONVOLUTION_BORDER_MODE_EXT - Is the same as GL_CONVOLUTION_BORDER_MODE
            // GL_CONVOLUTION_FILTER_SCALE_EXT - Is the same as GL_CONVOLUTION_FILTER_SCALE
            // GL_CONVOLUTION_FILTER_BIAS_EXT - Is the same as GL_CONVOLUTION_FILTER_BIAS
            // GL_REDUCE_EXT - Is the same as GL_REDUCE
            // GL_CONVOLUTION_FORMAT_EXT - Is the same as GL_CONVOLUTION_FORMAT
            // GL_CONVOLUTION_WIDTH_EXT - Is the same as GL_CONVOLUTION_WIDTH
            // GL_CONVOLUTION_HEIGHT_EXT - Is the same as GL_CONVOLUTION_HEIGHT
            // GL_MAX_CONVOLUTION_WIDTH_EXT - Is the same as GL_MAX_CONVOLUTION_WIDTH
            // GL_MAX_CONVOLUTION_HEIGHT_EXT - Is the same as GL_MAX_CONVOLUTION_HEIGHT
            // GL_POST_CONVOLUTION_RED_SCALE_EXT - Is the same as GL_POST_CONVOLUTION_RED_SCALE
            // GL_POST_CONVOLUTION_GREEN_SCALE_EXT - Is the same as GL_POST_CONVOLUTION_GREEN_SCALE
            // GL_POST_CONVOLUTION_BLUE_SCALE_EXT - Is the same as GL_POST_CONVOLUTION_BLUE_SCALE
            // GL_POST_CONVOLUTION_ALPHA_SCALE_EXT - Is the same as GL_POST_CONVOLUTION_ALPHA_SCALE
            // GL_POST_CONVOLUTION_RED_BIAS_EXT - Is the same as GL_POST_CONVOLUTION_RED_BIAS
            // GL_POST_CONVOLUTION_GREEN_BIAS_EXT - Is the same as GL_POST_CONVOLUTION_GREEN_BIAS
            // GL_POST_CONVOLUTION_BLUE_BIAS_EXT - Is the same as GL_POST_CONVOLUTION_BLUE_BIAS
            // GL_POST_CONVOLUTION_ALPHA_BIAS_EXT - Is the same as GL_POST_CONVOLUTION_ALPHA_BIAS

            // GL_EXT_cmyka
            AP_GL_ENUM_TOSTRING_CASE(GL_CMYK_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CMYKA_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_CMYK_HINT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_CMYK_HINT_EXT);

            // GL_EXT_texture_object
            // Included in OpenGL 1.1
            // GL_TEXTURE_PRIORITY_EXT - Is the same as GL_TEXTURE_PRIORITY
            // GL_TEXTURE_RESIDENT_EXT - Is the same as GL_TEXTURE_RESIDENT
            // GL_TEXTURE_1D_BINDING_EXT - Is the same as GL_TEXTURE_1D_BINDING
            // GL_TEXTURE_2D_BINDING_EXT - Is the same as GL_TEXTURE_2D_BINDING
            // GL_TEXTURE_3D_BINDING_EXT - Is the same as GL_TEXTURE_3D_BINDING.

            // GL_EXT_packed_pixels:
            // Included in OpenGL 1.2
            // GL_UNSIGNED_BYTE_3_3_2_EXT - Is the same as GL_UNSIGNED_BYTE_3_3_2.
            // GL_UNSIGNED_SHORT_4_4_4_4_EXT - Is the same as GL_UNSIGNED_SHORT_4_4_4_4.
            // GL_UNSIGNED_SHORT_5_5_5_1_EXT - Is the same as GL_UNSIGNED_SHORT_5_5_5_1.
            // GL_UNSIGNED_INT_8_8_8_8_EXT - Is the same as GL_UNSIGNED_INT_8_8_8_8.
            // GL_UNSIGNED_INT_10_10_10_2_EXT - Is the same as GL_UNSIGNED_INT_10_10_10_2.

            // GL_EXT_rescale_normal
            // Included in OpenGL 1.2
            // GL_RESCALE_NORMAL_EXT - Is the same as GL_RESCALE_NORMAL.

            // GL_EXT_vertex_array
            // Included in OpenGL 1.1
            // GL_VERTEX_ARRAY_EXT - Is the same as GL_VERTEX_ARRAY
            // GL_NORMAL_ARRAY_EXT - Is the same as GL_NORMAL_ARRAY
            // GL_COLOR_ARRAY_EXT - Is the same as GL_COLOR_ARRAY
            // GL_INDEX_ARRAY_EXT - Is the same as GL_INDEX_ARRAY
            // GL_TEXTURE_COORD_ARRAY_EXT - Is the same as GL_TEXTURE_COORD_ARRAY
            // GL_EDGE_FLAG_ARRAY_EXT - Is the same as GL_EDGE_FLAG_ARRAY
            // GL_VERTEX_ARRAY_SIZE_EXT - Is the same as GL_VERTEX_ARRAY_SIZE
            // GL_VERTEX_ARRAY_TYPE_EXT - Is the same as GL_VERTEX_ARRAY_TYPE
            // GL_VERTEX_ARRAY_STRIDE_EXT - Is the same as GL_VERTEX_ARRAY_STRIDE
            // GL_VERTEX_ARRAY_COUNT_EXT - Is the same as GL_VERTEX_ARRAY_COUNT
            // GL_NORMAL_ARRAY_TYPE_EXT - Is the same as GL_NORMAL_ARRAY_TYPE
            // GL_NORMAL_ARRAY_STRIDE_EXT - Is the same as GL_NORMAL_ARRAY_STRIDE
            // GL_NORMAL_ARRAY_COUNT_EXT - Is the same as GL_NORMAL_ARRAY_COUNT
            // GL_COLOR_ARRAY_SIZE_EXT - Is the same as GL_COLOR_ARRAY_SIZE
            // GL_COLOR_ARRAY_TYPE_EXT - Is the same as GL_COLOR_ARRAY_TYPE
            // GL_COLOR_ARRAY_STRIDE_EXT - Is the same as GL_COLOR_ARRAY_STRIDE
            // GL_COLOR_ARRAY_COUNT_EXT - Is the same as GL_COLOR_ARRAY_COUNT
            // GL_INDEX_ARRAY_TYPE_EXT - Is the same as GL_INDEX_ARRAY_TYPE
            // GL_INDEX_ARRAY_STRIDE_EXT - Is the same as GL_INDEX_ARRAY_STRIDE
            // GL_INDEX_ARRAY_COUNT_EXT - Is the same as GL_INDEX_ARRAY_COUNT
            // GL_TEXTURE_COORD_ARRAY_SIZE_EXT - Is the same as GL_TEXTURE_COORD_ARRAY_SIZE
            // GL_TEXTURE_COORD_ARRAY_TYPE_EXT - Is the same as GL_TEXTURE_COORD_ARRAY_TYPE
            // GL_TEXTURE_COORD_ARRAY_STRIDE_EXT - Is the same as GL_TEXTURE_COORD_ARRAY_STRIDE
            // GL_TEXTURE_COORD_ARRAY_COUNT_EXT - Is the same as GL_TEXTURE_COORD_ARRAY_COUNT
            // GL_EDGE_FLAG_ARRAY_STRIDE_EXT - Is the same as GL_EDGE_FLAG_ARRAY_STRIDE
            // GL_EDGE_FLAG_ARRAY_COUNT_EXT - Is the same as GL_EDGE_FLAG_ARRAY_COUNT
            // GL_VERTEX_ARRAY_POINTER_EXT - Is the same as GL_VERTEX_ARRAY_POINTER
            // GL_NORMAL_ARRAY_POINTER_EXT - Is the same as GL_NORMAL_ARRAY_POINTER
            // GL_COLOR_ARRAY_POINTER_EXT - Is the same as GL_COLOR_ARRAY_POINTER
            // GL_INDEX_ARRAY_POINTER_EXT - Is the same as GL_INDEX_ARRAY_POINTER
            // GL_TEXTURE_COORD_ARRAY_POINTER_EXT - Is the same as GL_TEXTURE_COORD_ARRAY_POINTER
            // GL_EDGE_FLAG_ARRAY_POINTER_EXT - Is the same as GL_EDGE_FLAG_ARRAY_POINTER

            // GL_EXT_misc_attribute

            // GL_EXT_blend_minmax
            // Included in OpenGL 1.2
            // GL_FUNC_ADD_EXT - Is the same as GL_FUNC_ADD
            // GL_MIN_EXT - Is the same as GL_MIN
            // GL_MAX_EXT - Is the same as GL_MAX
            // GL_BLEND_EQUATION_EXT - Is the same as GL_BLEND_EQUATION

            // GL_EXT_blend_subtract
            // Included in OpenGL 1.2
            // GL_FUNC_SUBTRACT_EXT - Is the same as GL_FUNC_SUBTRACT
            // GL_FUNC_REVERSE_SUBTRACT_EXT - Is the same as GL_FUNC_REVERSE_SUBTRACT

            // GL_EXT_blend_logic_op
            // Included in OpenGL 1.1

            // GL_EXT_point_parameters
            // GL_POINT_SIZE_MIN_EXT - Is the same as GL_POINT_SIZE_MIN
            // GL_POINT_SIZE_MAX_EXT - Is the same as GL_POINT_SIZE_MAX
            // GL_POINT_FADE_THRESHOLD_SIZE_EXT - Is the same as GL_POINT_FADE_THRESHOLD_SIZE
            // GL_DISTANCE_ATTENUATION_EXT - Is the same as GL_DISTANCE_ATTENUATION

            // GL_EXT_color_subtable
            // Included in OpenGL 1.2

            // GL_EXT_paletted_texture
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_INDEX1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_INDEX2_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_INDEX4_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_INDEX8_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_INDEX12_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_INDEX16_EXT);

        case 0x80ED: // GL_TEXTURE_INDEX_SIZE_EXT:
            valueString = L"GL_TEXTURE_INDEX_SIZE_EXT";
            break;

            // GL_EXT_clip_volume_hint
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_VOLUME_CLIPPING_HINT_EXT);

            // GL_EXT_index_texture

            // GL_EXT_index_material
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_MATERIAL_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_MATERIAL_PARAMETER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_MATERIAL_FACE_EXT);

            // GL_EXT_index_func
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_TEST_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_TEST_FUNC_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_TEST_REF_EXT);

            // GL_EXT_index_array_formats
            AP_GL_ENUM_TOSTRING_CASE(GL_IUI_V2F_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IUI_V3F_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IUI_N3F_V2F_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IUI_N3F_V3F_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_T2F_IUI_V2F_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_T2F_IUI_V3F_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_T2F_IUI_N3F_V2F_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_T2F_IUI_N3F_V3F_EXT);

            // GL_EXT_compiled_vertex_array
            AP_GL_ENUM_TOSTRING_CASE(GL_ARRAY_ELEMENT_LOCK_FIRST_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_ARRAY_ELEMENT_LOCK_COUNT_EXT);

            // GL_EXT_cull_vertex
            AP_GL_ENUM_TOSTRING_CASE(GL_CULL_VERTEX_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CULL_VERTEX_EYE_POSITION_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CULL_VERTEX_OBJECT_POSITION_EXT);

            // GL_EXT_draw_range_elements:
            // Included in OpenGL 1.2
            // GL_MAX_ELEMENTS_VERTICES_EXT - Is the same as GL_MAX_ELEMENTS_VERTICES.
            // GL_MAX_ELEMENTS_INDICES_EXT - Is the same as GL_MAX_ELEMENTS_INDICES.

            // GL_EXT_light_texture
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_MATERIAL_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_NORMAL_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_COLOR_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATTENUATION_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADOW_ATTENUATION_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_APPLICATION_MODE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_LIGHT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MATERIAL_FACE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MATERIAL_PARAMETER_EXT);

            // GL_EXT_bgra:
            // Included in OpenGL 1.2
            // GL_BGR_EXT - Is the same as GL_BGR.
            // GL_BGRA_EXT - Is the same as GL_BGRA.

            // GL_EXT_pixel_transform
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TRANSFORM_2D_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MAG_FILTER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_MIN_FILTER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_CUBIC_WEIGHT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CUBIC_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_AVERAGE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TRANSFORM_2D_MATRIX_EXT);

            // GL_EXT_pixel_transform_color_table

            // GL_EXT_shared_texture_palette
            AP_GL_ENUM_TOSTRING_CASE(GL_SHARED_TEXTURE_PALETTE_EXT);

            // GL_EXT_separate_specular_color
            // Included in OpenGL 1.2
            // GL_LIGHT_MODEL_COLOR_CONTROL_EXT - Is the same as GL_LIGHT_MODEL_COLOR_CONTROL
            // GL_SINGLE_COLOR_EXT - Is the same as GL_SINGLE_COLOR
            // GL_SEPARATE_SPECULAR_COLOR_EXT - Is the same as GL_SEPARATE_SPECULAR_COLOR

            // GL_EXT_secondary_color
            // Included in OpenGL 1.4
            // GL_COLOR_SUM_EXT - Is the same as GL_COLOR_SUM
            // GL_CURRENT_SECONDARY_COLOR_EXT - Is the same as GL_CURRENT_SECONDARY_COLOR
            // GL_SECONDARY_COLOR_ARRAY_SIZE_EXT - Is the same as GL_SECONDARY_COLOR_ARRAY_SIZE
            // GL_SECONDARY_COLOR_ARRAY_TYPE_EXT - Is the same as GL_SECONDARY_COLOR_ARRAY_TYPE
            // GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT - Is the same as GL_SECONDARY_COLOR_ARRAY_STRIDE
            // GL_SECONDARY_COLOR_ARRAY_POINTER_EXT - Is the same as GL_SECONDARY_COLOR_ARRAY_POINTER
            // GL_SECONDARY_COLOR_ARRAY_EXT - Is the same as GL_SECONDARY_COLOR_ARRAY

            // GL_EXT_texture_perturb_normal
            AP_GL_ENUM_TOSTRING_CASE(GL_PERTURB_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_NORMAL_EXT);

            // GL_EXT_multi_draw_arrays
            // Included in OpenGL 1.4

            // GL_EXT_fog_coord
            // Included in OpenGL 1.4
            // GL_FOG_COORDINATE_SOURCE_EXT - Is the same as GL_FOG_COORDINATE_SOURCE
            // GL_FOG_COORDINATE_EXT - Is the same as GL_FOG_COORDINATE
            // GL_FRAGMENT_DEPTH_EXT - Is the same as GL_FRAGMENT_DEPTH
            // GL_CURRENT_FOG_COORDINATE_EXT - Is the same as GL_CURRENT_FOG_COORDINATE
            // GL_FOG_COORDINATE_ARRAY_TYPE_EXT - Is the same as GL_FOG_COORDINATE_ARRAY_TYPE
            // GL_FOG_COORDINATE_ARRAY_STRIDE_EXT - Is the same as GL_FOG_COORDINATE_ARRAY_STRIDE
            // GL_FOG_COORDINATE_ARRAY_POINTER_EXT - Is the same as GL_FOG_COORDINATE_ARRAY_POINTER
            // GL_FOG_COORDINATE_ARRAY_EXT - Is the same as GL_FOG_COORDINATE_ARRAY

            // GL_REND_screen_coordinates
            AP_GL_ENUM_TOSTRING_CASE(GL_SCREEN_COORDINATES_REND);
            AP_GL_ENUM_TOSTRING_CASE(GL_INVERTED_SCREEN_W_REND);

            // GL_EXT_coordinate_frame
            AP_GL_ENUM_TOSTRING_CASE(GL_TANGENT_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_BINORMAL_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_TANGENT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_BINORMAL_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TANGENT_ARRAY_TYPE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TANGENT_ARRAY_STRIDE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_BINORMAL_ARRAY_TYPE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_BINORMAL_ARRAY_STRIDE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TANGENT_ARRAY_POINTER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_BINORMAL_ARRAY_POINTER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_TANGENT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_TANGENT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_BINORMAL_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_BINORMAL_EXT);

            // GL_EXT_texture_env_combine
            // GL_COMBINE_EXT - Is the same as GL_COMBINE
            // GL_COMBINE_RGB_EXT - Is the same as GL_COMBINE_RGB
            // GL_COMBINE_ALPHA_EXT - Is the same as GL_COMBINE_ALPHA
            // GL_RGB_SCALE_EXT - Is the same as GL_RGB_SCALE
            // GL_ADD_SIGNED_EXT - Is the same as GL_ADD_SIGNED
            // GL_INTERPOLATE_EXT - Is the same as GL_INTERPOLATE
            // GL_CONSTANT_EXT - Is the same as GL_CONSTANT
            // GL_PRIMARY_COLOR_EXT - Is the same as GL_PRIMARY_COLOR
            // GL_PREVIOUS_EXT - Is the same as GL_PREVIOUS
            // GL_SOURCE0_RGB_EXT - Is the same as GL_SOURCE0_RGB
            // GL_SOURCE1_RGB_EXT - Is the same as GL_SOURCE1_RGB
            // GL_SOURCE2_RGB_EXT - Is the same as GL_SOURCE2_RGB
            // GL_SOURCE0_ALPHA_EXT - Is the same as GL_SOURCE0_ALPHA
            // GL_SOURCE1_ALPHA_EXT - Is the same as GL_SOURCE1_ALPHA
            // GL_SOURCE2_ALPHA_EXT - Is the same as GL_SOURCE2_ALPHA
            // GL_OPERAND0_RGB_EXT - Is the same as GL_OPERAND0_RGB
            // GL_OPERAND1_RGB_EXT - Is the same as GL_OPERAND1_RGB
            // GL_OPERAND2_RGB_EXT - Is the same as GL_OPERAND2_RGB
            // GL_OPERAND0_ALPHA_EXT - Is the same as GL_OPERAND0_ALPHA
            // GL_OPERAND1_ALPHA_EXT - Is the same as GL_OPERAND1_ALPHA
            // GL_OPERAND2_ALPHA_EXT - Is the same as GL_OPERAND2_ALPHA

            // GL_SOURCE3_RGB_EXT is the same as GL_SOURCE3_RGB_ARB (Apple only)
            // GL_SOURCE4_RGB_EXT is the same as GL_SOURCE4_RGB_ARB (Apple only)
            // GL_SOURCE5_RGB_EXT is the same as GL_SOURCE5_RGB_ARB (Apple only)
            // GL_SOURCE6_RGB_EXT is the same as GL_SOURCE6_RGB_ARB (Apple only)
            // GL_SOURCE7_RGB_EXT is the same as GL_SOURCE7_RGB_ARB (Apple only)
            // GL_SOURCE3_ALPHA_EXT is the same as GL_SOURCE3_ALPHA_ARB (Apple only)
            // GL_SOURCE4_ALPHA_EXT is the same as GL_SOURCE4_ALPHA_ARB (Apple only)
            // GL_SOURCE5_ALPHA_EXT is the same as GL_SOURCE5_ALPHA_ARB (Apple only)
            // GL_SOURCE6_ALPHA_EXT is the same as GL_SOURCE6_ALPHA_ARB (Apple only)
            // GL_SOURCE7_ALPHA_EXT is the same as GL_SOURCE7_ALPHA_ARB (Apple only)
            // GL_OPERAND3_RGB_EXT is the same as GL_OPERAND3_RGB_ARB (Apple only)
            // GL_OPERAND4_RGB_EXT is the same as GL_OPERAND4_RGB_ARB (Apple only)
            // GL_OPERAND5_RGB_EXT is the same as GL_OPERAND5_RGB_ARB (Apple only)
            // GL_OPERAND6_RGB_EXT is the same as GL_OPERAND6_RGB_ARB (Apple only)
            // GL_OPERAND7_RGB_EXT is the same as GL_OPERAND7_RGB_ARB (Apple only)
            // GL_OPERAND3_ALPHA_EXT is the same as GL_OPERAND3_ALPHA_ARB (Apple only)
            // GL_OPERAND4_ALPHA_EXT is the same as GL_OPERAND4_ALPHA_ARB (Apple only)
            // GL_OPERAND5_ALPHA_EXT is the same as GL_OPERAND5_ALPHA_ARB (Apple only)
            // GL_OPERAND6_ALPHA_EXT is the same as GL_OPERAND6_ALPHA_ARB (Apple only)
            // GL_OPERAND7_ALPHA_EXT is the same as GL_OPERAND7_ALPHA_ARB (Apple only)

            // GL_EXT_texture_env_add

            // GL_EXT_texture_lod_bias
            // Included in OpenGL 1.4
            // GL_MAX_TEXTURE_LOD_BIAS_EXT - Is the same as GL_MAX_TEXTURE_LOD_BIAS
            // GL_TEXTURE_FILTER_CONTROL_EXT - Is the same as GL_TEXTURE_FILTER_CONTROL
            // GL_TEXTURE_LOD_BIAS_EXT - Is the same as GL_TEXTURE_LOD_BIAS

            // GL_EXT_blend_func_separate
            // Included in OpenGL 1.4
            // GL_BLEND_DST_RGB_EXT - Is the same as GL_BLEND_DST_RGB
            // GL_BLEND_SRC_RGB_EXT - Is the same as GL_BLEND_SRC_RGB
            // GL_BLEND_DST_ALPHA_EXT - Is the same as GL_BLEND_DST_ALPHA
            // GL_BLEND_SRC_ALPHA_EXT - Is the same as GL_BLEND_SRC_ALPHA

            // GL_EXT_stencil_wrap
            // Included in OpenGL 1.4
            // GL_INCR_WRAP_EXT - Is the same as GL_INCR_WRAP
            // GL_DECR_WRAP_EXT - Is the same as GL_DECR_WRAP

            // GL_EXT_422_pixels
            AP_GL_ENUM_TOSTRING_CASE(GL_422_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_422_REV_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_422_AVERAGE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_422_REV_AVERAGE_EXT);

            // GL_EXT_texture_cube_map:
            // GL_NORMAL_MAP_EXT - Is the same as GL_NORMAL_MAP_EX
            // GL_REFLECTION_MAP_EXT - Is the same as GL_REFLECTION_MAP.
            // GL_TEXTURE_CUBE_MAP_EXT - Is the same as GL_TEXTURE_CUBE_MAP.
            // GL_TEXTURE_BINDING_CUBE_MAP_EXT - Is the same as GL_TEXTURE_BINDING_CUBE_MAP.
            // GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT - Is the same as GL_TEXTURE_CUBE_MAP_POSITIVE_X.
            // GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT - Is the same as GL_TEXTURE_CUBE_MAP_NEGATIVE_X.
            // GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT - Is the same as GL_TEXTURE_CUBE_MAP_POSITIVE_Y.
            // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT - Is the same as GL_TEXTURE_CUBE_MAP_NEGATIVE_Y.
            // GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT - Is the same as GL_TEXTURE_CUBE_MAP_POSITIVE_Z.
            // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT - Is the same as GL_TEXTURE_CUBE_MAP_NEGATIVE_Z.
            // GL_PROXY_TEXTURE_CUBE_MAP_EXT - Is the same as GL_PROXY_TEXTURE_CUBE_MAP.
            // GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT - Is the same as GL_MAX_CUBE_MAP_TEXTURE_SIZE.

            // GL_EXT_texture_filter_anisotropic
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MAX_ANISOTROPY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT);
            // GL_MODELVIEW1_EXT - Is the same as GL_MODELVIEW1_ARB.

            // GL_EXT_vertex_weighting:
            // GL_MODELVIEW0_STACK_DEPTH_EXT is the same as GL_MODELVIEW_STACK_DEPTH
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW1_STACK_DEPTH_EXT);
            // GL_MODELVIEW0_MATRIX_EXT is the same as GL_MODELVIEW_MATRIX
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW1_MATRIX_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_WEIGHTING_EXT);
            // GL_MODELVIEW0_EXT is the same as GL_MODELVIEW
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_VERTEX_WEIGHT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_WEIGHT_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT);

            // GL_EXT_texture_compression_s3tc
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);

            // GL_EXT_texture_rectangle
            // GL_TEXTURE_RECTANGLE_EXT is the same as GL_TEXTURE_RECTANGLE_ARB
            // GL_TEXTURE_BINDING_RECTANGLE_EXT is the same as GL_TEXTURE_BINDING_RECTANGLE_ARB
            // GL_PROXY_TEXTURE_RECTANGLE_EXT is the same as GL_PROXY_TEXTURE_RECTANGLE_ARB
            // GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT is the same as GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB

            // GL_EXT_multisample
            // GL_MULTISAMPLE_EXT - Is the same as GL_MULTISAMPLE
            // GL_SAMPLE_ALPHA_TO_MASK_EXT - Is the same as GL_SAMPLE_ALPHA_TO_COVERAGE
            // GL_SAMPLE_ALPHA_TO_ONE_EXT - Is the same as GL_SAMPLE_ALPHA_TO_ONE.
            // GL_SAMPLE_MASK_EXT - Is the same as GL_SAMPLE_COVERAGE.
            AP_GL_ENUM_TOSTRING_CASE(GL_1PASS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_2PASS_0_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_2PASS_1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_4PASS_0_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_4PASS_1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_4PASS_2_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_4PASS_3_EXT);
            // GL_SAMPLE_BUFFERS_EXT - Is the same as GL_SAMPLE_BUFFERS
            // GL_SAMPLES_EXT - Is the same as GL_SAMPLES
            // GL_SAMPLE_MASK_VALUE_EXT - Is the same as GL_SAMPLE_COVERAGE_VALUE.
            // GL_SAMPLE_MASK_INVERT_EXT - Is the same as GL_SAMPLE_COVERAGE_INVERT.
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_PATTERN_EXT);

            // GL_EXT_texture_env_dot3
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT3_RGB_EXT);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOT3_RGBA_EXT);

            // GL_ATI_texture_mirror_once
            // GL_MIRROR_CLAMP_ATI - Is the same as GL_MIRROR_CLAMP_EXT
            // GL_MIRROR_CLAMP_TO_EDGE_ATI - Is the same as GL_MIRROR_CLAMP_TO_EDGE_EXT

            // GL_EXT_vertex_shader
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SHADER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SHADER_BINDING_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_INDEX_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_NEGATE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_DOT3_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_DOT4_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_MUL_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_ADD_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_MADD_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_FRAC_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_MAX_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_MIN_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_SET_GE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_SET_LT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_CLAMP_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_FLOOR_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_ROUND_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_EXP_BASE_2_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_LOG_BASE_2_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_POWER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_RECIP_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_RECIP_SQRT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_SUB_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_CROSS_PRODUCT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_MULTIPLY_MATRIX_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OP_MOV_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_VERTEX_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_COLOR0_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_COLOR1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD0_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD2_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD3_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD4_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD5_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD6_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD7_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD8_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD9_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD10_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD11_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD12_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD13_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD14_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD15_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD16_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD17_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD18_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD19_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD20_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD21_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD22_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD23_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD24_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD25_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD26_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD27_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD28_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD29_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD30_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_TEXTURE_COORD31_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_OUTPUT_FOG_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SCALAR_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VECTOR_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIANT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INVARIANT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LOCAL_CONSTANT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LOCAL_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_SHADER_VARIANTS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_SHADER_INVARIANTS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_SHADER_LOCALS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SHADER_INSTRUCTIONS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SHADER_VARIANTS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SHADER_INVARIANTS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SHADER_LOCALS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SHADER_OPTIMIZED_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_X_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_Y_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_Z_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_W_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEGATIVE_X_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEGATIVE_Y_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEGATIVE_Z_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEGATIVE_W_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_ZERO_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_ONE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEGATIVE_ONE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMALIZED_RANGE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FULL_RANGE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_VERTEX_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MVP_MATRIX_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIANT_VALUE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIANT_DATATYPE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIANT_ARRAY_STRIDE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIANT_ARRAY_TYPE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIANT_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIANT_ARRAY_POINTER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INVARIANT_VALUE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INVARIANT_DATATYPE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LOCAL_CONSTANT_VALUE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LOCAL_CONSTANT_DATATYPE_EXT);
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

            // GL_EXT_fragment_shader
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_SHADER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_0_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_2_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_3_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_4_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_5_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_6_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_7_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_8_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_9_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_10_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_11_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_12_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_13_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_14_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_15_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_16_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_17_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_18_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_19_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_20_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_21_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_22_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_23_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_24_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_25_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_26_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_27_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_28_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_29_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_30_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_31_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_0_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_2_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_3_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_4_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_5_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_6_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_7_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_8_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_9_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_10_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_11_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_12_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_13_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_14_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_15_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_16_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_17_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_18_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_19_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_20_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_21_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_22_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_23_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_24_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_25_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_26_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_27_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_28_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_29_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_30_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_31_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MOV_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_ADD_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MUL_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SUB_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT3_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT4_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAD_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LERP_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CND_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_CND0_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT2_ADD_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SECONDARY_INTERPOLATOR_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_FRAGMENT_REGISTERS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_FRAGMENT_CONSTANTS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_PASSES_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_INSTRUCTIONS_PER_PASS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_INSTRUCTIONS_TOTAL_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_LOOPBACK_COMPONENTS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ALPHA_PAIRING_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SWIZZLE_STR_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SWIZZLE_STQ_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SWIZZLE_STR_DR_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SWIZZLE_STQ_DQ_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SWIZZLE_STRQ_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SWIZZLE_STRQ_DQ_EXT);
#endif

            // GL_EXT_shadow_funcs
            // Included in OpenGL 1.5

            // GL_EXT_stencil_two_side
            // Included in OpenGL 2.0
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_TEST_TWO_SIDE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_STENCIL_FACE_EXT);

            // GL_EXT_separate_shader_objects
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_PROGRAM_EXT);

            // GL_EXT_depth_bounds_test
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_BOUNDS_TEST_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_BOUNDS_EXT);

            // GL_EXT_texture_mirror_clamp
            AP_GL_ENUM_TOSTRING_CASE(GL_MIRROR_CLAMP_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIRROR_CLAMP_TO_EDGE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIRROR_CLAMP_TO_BORDER_EXT);

            // GL_EXT_stencil_clear_tag:
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_TAG_BITS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_CLEAR_TAG_VALUE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT);
            // GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT is equal to GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
            // GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT is equal to GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
            // GL_FRAMEBUFFER_UNSUPPORTED_EXT is equal to GL_FRAMEBUFFER_UNSUPPORTED
            // GL_FRAMEBUFFER_STATUS_ERROR_EXT was removed from GL_GLEXT_VERSION version 29
            AP_GL_ENUM_TOSTRING_CASE(0x8CDE);
            // GL_MAX_COLOR_ATTACHMENTS_EXT is equal to GL_MAX_COLOR_ATTACHMENTS
            // GL_COLOR_ATTACHMENT0_EXT is equal to GL_COLOR_ATTACHMENT0
            // GL_COLOR_ATTACHMENT1_EXT is equal to GL_COLOR_ATTACHMENT1
            // GL_COLOR_ATTACHMENT2_EXT is equal to GL_COLOR_ATTACHMENT2
            // GL_COLOR_ATTACHMENT3_EXT is equal to GL_COLOR_ATTACHMENT3
            // GL_COLOR_ATTACHMENT4_EXT is equal to GL_COLOR_ATTACHMENT4
            // GL_COLOR_ATTACHMENT5_EXT is equal to GL_COLOR_ATTACHMENT5
            // GL_COLOR_ATTACHMENT6_EXT is equal to GL_COLOR_ATTACHMENT6
            // GL_COLOR_ATTACHMENT7_EXT is equal to GL_COLOR_ATTACHMENT7
            // GL_COLOR_ATTACHMENT8_EXT is equal to GL_COLOR_ATTACHMENT8
            // GL_COLOR_ATTACHMENT9_EXT is equal to GL_COLOR_ATTACHMENT9
            // GL_COLOR_ATTACHMENT10_EXT is equal to GL_COLOR_ATTACHMENT10
            // GL_COLOR_ATTACHMENT11_EXT is equal to GL_COLOR_ATTACHMENT11
            // GL_COLOR_ATTACHMENT12_EXT is equal to GL_COLOR_ATTACHMENT12
            // GL_COLOR_ATTACHMENT13_EXT is equal to GL_COLOR_ATTACHMENT13
            // GL_COLOR_ATTACHMENT14_EXT is equal to GL_COLOR_ATTACHMENT14
            // GL_COLOR_ATTACHMENT15_EXT is equal to GL_COLOR_ATTACHMENT15
            // GL_DEPTH_ATTACHMENT_EXT is equal to GL_DEPTH_ATTACHMENT
            // GL_STENCIL_ATTACHMENT_EXT is equal to GL_STENCIL_ATTACHMENT
            // GL_FRAMEBUFFER_EXT is equal to GL_FRAMEBUFFER
            // GL_RENDERBUFFER_EXT is equal to GL_RENDERBUFFER
            // GL_RENDERBUFFER_WIDTH_EXT is equal to GL_RENDERBUFFER_WIDTH
            // GL_RENDERBUFFER_HEIGHT_EXT is equal to GL_RENDERBUFFER_HEIGHT
            // GL_RENDERBUFFER_INTERNAL_FORMAT_EXT is equal to GL_RENDERBUFFER_INTERNAL_FORMAT
            // GL_STENCIL_INDEX_EXT was removed from GL_GLEXT_VERSION version 29
            AP_GL_ENUM_TOSTRING_CASE(0x8D45);
            // GL_STENCIL_INDEX1_EXT is equal to GL_STENCIL_INDEX1
            // GL_STENCIL_INDEX4_EXT is equal to GL_STENCIL_INDEX4
            // GL_STENCIL_INDEX8_EXT is equal to GL_STENCIL_INDEX8
            // GL_STENCIL_INDEX16_EXT is equal to GL_STENCIL_INDEX16
            // GL_RENDERBUFFER_RED_SIZE_EXT is equal to GL_RENDERBUFFER_RED_SIZE
            // GL_RENDERBUFFER_GREEN_SIZE_EXT is equal to GL_RENDERBUFFER_GREEN_SIZE
            // GL_RENDERBUFFER_BLUE_SIZE_EXT is equal to GL_RENDERBUFFER_BLUE_SIZE
            // GL_RENDERBUFFER_ALPHA_SIZE_EXT is equal to GL_RENDERBUFFER_ALPHA_SIZE
            // GL_RENDERBUFFER_DEPTH_SIZE_EXT is equal to GL_RENDERBUFFER_DEPTH_SIZE
            // GL_RENDERBUFFER_STENCIL_SIZE_EXT is equal to GL_RENDERBUFFER_STENCIL_SIZE

            // GL_EXT_packed_depth_stencil:
            // GL_DEPTH_STENCIL_EXT is equal to GL_DEPTH_STENCIL
            // GL_UNSIGNED_INT_24_8_EXT is equal to GL_UNSIGNED_INT_24_8
            // GL_DEPTH24_STENCIL8_EXT is equal to GL_DEPTH24_STENCIL8
            // GL_TEXTURE_STENCIL_SIZE_EXT is equal to GL_TEXTURE_STENCIL_SIZE

            // GL_EXT_framebuffer_multisample:
            // GL_RENDERBUFFER_SAMPLES_EXT is equal to GL_RENDERBUFFER_SAMPLES

            // GL_EXT_texture_sRGB:
            // Included in OpenGL 2.1
            // GL_SRGB_EXT - Is the same as GL_SRGB
            // GL_SRGB8_EXT - Is the same as GL_SRGB8
            // GL_SRGB_ALPHA_EXT - Is the same as GL_SRGB_ALPHA
            // GL_SRGB8_ALPHA8_EXT - Is the same as GL_SRGB8_ALPHA8
            // GL_SLUMINANCE_ALPHA_EXT - Is the same as GL_SLUMINANCE_ALPHA
            // GL_SLUMINANCE8_ALPHA8_EXT - Is the same as GL_SLUMINANCE8_ALPHA8
            // GL_SLUMINANCE_EXT - Is the same as GL_SLUMINANCE
            // GL_SLUMINANCE8_EXT - Is the same as GL_SLUMINANCE8
            // GL_COMPRESSED_SRGB_EXT - Is the same as GL_COMPRESSED_SRGB
            // GL_COMPRESSED_SRGB_ALPHA_EXT - Is the same as GL_COMPRESSED_SRGB_ALPHA
            // GL_COMPRESSED_SLUMINANCE_EXT - Is the same as GL_COMPRESSED_SLUMINANCE
            // GL_COMPRESSED_SLUMINANCE_ALPHA_EXT - Is the same as GL_COMPRESSED_SLUMINANCE_ALPHA
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SRGB_S3TC_DXT1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT);

            // GL_EXT_framebuffer_blit:
            // GL_READ_FRAMEBUFFER_EXT is equal to GL_READ_FRAMEBUFFER
            // GL_DRAW_FRAMEBUFFER_EXT is equal to GL_DRAW_FRAMEBUFFER
            // GL_READ_FRAMEBUFFER_BINDING_EXT same as: GL_FRAMEBUFFER_BINDING_EXT
            // GL_DRAW_FRAMEBUFFER_BINDING_EXT same as: GL_FRAMEBUFFER_BINDING_EXT

            // GL_EXT_timer_query:
            AP_GL_ENUM_TOSTRING_CASE(GL_TIME_ELAPSED_EXT);

            // GL_EXT_blend_equation_separate
            // GL_BLEND_EQUATION_RGB_EXT - Is the same as GL_BLEND_EQUATION
            // GL_BLEND_EQUATION_ALPHA_EXT  - Is the same as GL_BLEND_EQUATION_ALPHA

            // GL_EXT_framebuffer_object:
            // GL_INVALID_FRAMEBUFFER_OPERATION_EXT is equal to GL_INVALID_FRAMEBUFFER_OPERATION
            // GL_MAX_RENDERBUFFER_SIZE_EXT is equal to GL_MAX_RENDERBUFFER_SIZE
            // GL_FRAMEBUFFER_BINDING_EXT is equal to GL_FRAMEBUFFER_BINDING
            // GL_RENDERBUFFER_BINDING_EXT is equal to GL_RENDERBUFFER_BINDING
            // GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT is equal to GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE
            // GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT is equal to GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME
            // GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT is equal to GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL
            // GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT is equal to GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE
            // GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT is equal to GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET
            // GL_FRAMEBUFFER_COMPLETE_EXT is equal to GL_FRAMEBUFFER_COMPLETE
            // GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT is equal to GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
            // GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT is equal to GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT

            /* Yaki - 18/10/2007 - This was removed from glext.h version 39 */
            /*
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT);*/

            // GL_EXT_packed_depth_stencil:
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_STENCIL_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_24_8_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH24_STENCIL8_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_STENCIL_SIZE_EXT);

#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
            // OpenGL ES defines GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS, which has the same value as GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT.
#else
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT);
#endif

            // GL_EXT_gpu_shader4
            // GL_SAMPLER_1D_ARRAY_EXT is equal to GL_SAMPLER_1D_ARRAY
            // GL_SAMPLER_2D_ARRAY_EXT is equal to GL_SAMPLER_2D_ARRAY
            // GL_SAMPLER_BUFFER_EXT - Is the same as GL_SAMPLER_BUFFER
            // GL_SAMPLER_1D_ARRAY_SHADOW_EXT is equal to GL_SAMPLER_1D_ARRAY_SHADOW
            // GL_SAMPLER_2D_ARRAY_SHADOW_EXT is equal to GL_SAMPLER_2D_ARRAY_SHADOW
            // GL_SAMPLER_CUBE_SHADOW_EXT is equal to GL_SAMPLER_CUBE_SHADOW
            // GL_UNSIGNED_INT_VEC2_EXT is equal to GL_UNSIGNED_INT_VEC2
            // GL_UNSIGNED_INT_VEC3_EXT is equal to GL_UNSIGNED_INT_VEC3
            // GL_UNSIGNED_INT_VEC4_EXT is equal to GL_UNSIGNED_INT_VEC4
            // GL_INT_SAMPLER_1D_EXT is equal to GL_INT_SAMPLER_1D
            // GL_INT_SAMPLER_2D_EXT is equal to GL_INT_SAMPLER_2D
            // GL_INT_SAMPLER_3D_EXT is equal to GL_INT_SAMPLER_3D
            // GL_INT_SAMPLER_CUBE_EXT is equal to GL_INT_SAMPLER_CUBE
            // GL_INT_SAMPLER_2D_RECT_EXT is equal to GL_INT_SAMPLER_2D_RECT
            // GL_INT_SAMPLER_1D_ARRAY_EXT is equal to GL_INT_SAMPLER_1D_ARRAY
            // GL_INT_SAMPLER_2D_ARRAY_EXT is equal to GL_INT_SAMPLER_2D_ARRAY
            // GL_INT_SAMPLER_BUFFER_EXT is equal to GL_INT_SAMPLER_BUFFER
            // GL_UNSIGNED_INT_SAMPLER_1D_EXT is equal to GL_UNSIGNED_INT_SAMPLER_1D
            // GL_UNSIGNED_INT_SAMPLER_2D_EXT is equal to GL_UNSIGNED_INT_SAMPLER_2D
            // GL_UNSIGNED_INT_SAMPLER_3D_EXT is equal to GL_UNSIGNED_INT_SAMPLER_3D
            // GL_UNSIGNED_INT_SAMPLER_CUBE_EXT is equal to GL_UNSIGNED_INT_SAMPLER_CUBE
            // GL_UNSIGNED_INT_SAMPLER_2D_RECT_EXT is equal to GL_UNSIGNED_INT_SAMPLER_2D_RECT
            // GL_UNSIGNED_INT_SAMPLER_1D_ARRAY_EXT is equal to GL_UNSIGNED_INT_SAMPLER_1D_ARRAY
            // GL_UNSIGNED_INT_SAMPLER_2D_ARRAY_EXT is equal to GL_UNSIGNED_INT_SAMPLER_2D_ARRAY
            // GL_UNSIGNED_INT_SAMPLER_BUFFER_EXT is equal to GL_UNSIGNED_INT_SAMPLER_BUFFER
            // GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT is the same as GL_VERTEX_ATTRIB_ARRAY_INTEGER (Apple only)
            // GL_MIN_PROGRAM_TEXEL_OFFSET_EXT is the same as GL_MIN_PROGRAM_TEXEL_OFFSET (Apple only)
            // GL_MAX_PROGRAM_TEXEL_OFFSET_EXT is the same as GL_MAX_PROGRAM_TEXEL_OFFSET (Apple only)

            // GL_EXT_draw_instanced

            // GL_EXT_packed_float
            // Included in OpenGL 3.0
            // GL_R11F_G11F_B10F_EXT is equal to GL_R11F_G11F_B10F
            // GL_UNSIGNED_INT_10F_11F_11F_REV_EXT is equal to GL_UNSIGNED_INT_10F_11F_11F_REV
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA_SIGNED_COMPONENTS_EXT);

            // GL_EXT_texture_array
            // Included in OpenGL 3.0
            // GL_TEXTURE_1D_ARRAY_EXT is equal to GL_TEXTURE_1D_ARRAY
            // GL_PROXY_TEXTURE_1D_ARRAY_EXT is equal to GL_PROXY_TEXTURE_1D_ARRAY
            // GL_TEXTURE_2D_ARRAY_EXT is equal to GL_TEXTURE_2D_ARRAY
            // GL_PROXY_TEXTURE_2D_ARRAY_EXT is equal to GL_PROXY_TEXTURE_2D_ARRAY
            // GL_TEXTURE_BINDING_1D_ARRAY_EXT is equal to GL_TEXTURE_BINDING_1D_ARRAY
            // GL_TEXTURE_BINDING_2D_ARRAY_EXT is equal to GL_TEXTURE_BINDING_2D_ARRAY
            // GL_MAX_ARRAY_TEXTURE_LAYERS_EXT is equal to GL_MAX_ARRAY_TEXTURE_LAYERS
            // GL_COMPARE_REF_DEPTH_TO_TEXTURE_EXT same as: GL_COMPARE_R_TO_TEXTURE

            // GL_EXT_texture_buffer_object
            // GL_TEXTURE_BUFFER_EXT is equal to GL_TEXTURE_BUFFER_ARB
            // GL_MAX_TEXTURE_BUFFER_SIZE_EXT is equal to GL_MAX_TEXTURE_BUFFER_SIZE_ARB
            // GL_TEXTURE_BINDING_BUFFER_EXT is equal to GL_TEXTURE_BINDING_BUFFER_ARB
            // GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT is equal to GL_TEXTURE_BUFFER_DATA_STORE_BINDING_ARB
            // GL_TEXTURE_BUFFER_FORMAT_EXT is equal to GL_TEXTURE_BUFFER_FORMAT_ARB

            // GL_EXT_texture_compression_latc
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_LUMINANCE_LATC1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT);

            // GL_EXT_texture_compression_rgtc
            // Included in OpenGL 3.0
            // GL_COMPRESSED_RED_RGTC1_EXT is equal to GL_COMPRESSED_RED_RGTC1
            // GL_COMPRESSED_SIGNED_RED_RGTC1_EXT is equal to GL_COMPRESSED_SIGNED_RED_RGTC1
            // GL_COMPRESSED_RED_GREEN_RGTC2_EXT is equal to GL_COMPRESSED_RG_RGTC2
            // GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT is equal to GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2

            // GL_EXT_texture_shared_exponent
            // Included in OpenGL 3.0
            // AP_GL_ENUM_TOSTRING_CASE(GL_RGB9_E5_EXT); // same as GL_RGB9_E5
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_5_9_9_9_REV_EXT); // same as GL_UNSIGNED_INT_5_9_9_9_REV
            // AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SHARED_SIZE_EXT); // same as GL_TEXTURE_SHARED_SIZE

            // GL_EXT_bindable_uniform
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_BINDABLE_UNIFORM_SIZE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER_BINDING_EXT);

            // GL_EXT_texture_integer
            // Included in OpenGL 3.0
            // GL_RGBA32UI_EXT is equal to GL_RGBA32UI
            // GL_RGB32UI_EXT is equal to GL_RGB32UI
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA32UI_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY32UI_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE32UI_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE_ALPHA32UI_EXT);
            // GL_RGBA16UI_EXT is equal to GL_RGBA16UI
            // GL_RGB16UI_EXT is equal to GL_RGB16UI
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA16UI_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY16UI_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE16UI_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE_ALPHA16UI_EXT);
            // GL_RGBA8UI_EXT is equal to GL_RGBA8UI
            // GL_RGB8UI_EXT is equal to GL_RGB8UI
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA8UI_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY8UI_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE8UI_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE_ALPHA8UI_EXT);
            // GL_RGBA32I_EXT is equal to GL_RGBA32I
            // GL_RGB32I_EXT is equal to GL_RGB32I
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA32I_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY32I_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE32I_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE_ALPHA32I_EXT);
            // GL_RGBA16I_EXT is equal to GL_RGBA16I
            // GL_RGB16I_EXT is equal to GL_RGB16I
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA16I_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY16I_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE16I_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE_ALPHA16I_EXT);
            // GL_RGBA8I_EXT is equal to GL_RGBA8I
            // GL_RGB8I_EXT is equal to GL_RGB8I
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA8I_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY8I_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE8I_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE_ALPHA8I_EXT);
            // GL_RED_INTEGER_EXT is equal to GL_RED_INTEGER
            // GL_GREEN_INTEGER_EXT is equal to GL_GREEN_INTEGER
            // GL_BLUE_INTEGER_EXT is equal to GL_BLUE_INTEGER
            // GL_ALPHA_INTEGER_EXT is equal to GL_ALPHA_INTEGER
            // GL_RGB_INTEGER_EXT is equal to GL_RGB_INTEGER
            // GL_RGBA_INTEGER_EXT is equal to GL_RGBA_INTEGER
            // GL_BGR_INTEGER_EXT is equal to GL_BGR_INTEGER
            // GL_BGRA_INTEGER_EXT is equal to GL_BGRA_INTEGER
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE_INTEGER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE_ALPHA_INTEGER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA_INTEGER_MODE_EXT);

            // GL_GREMEDY_frame_terminator

            // GL_EXT_transform_feedback
            // GL_TRANSFORM_FEEDBACK_BUFFER_EXT is equal to GL_TRANSFORM_FEEDBACK_BUFFER
            // GL_TRANSFORM_FEEDBACK_BUFFER_START_EXT is equal to GL_TRANSFORM_FEEDBACK_BUFFER_START
            // GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_EXT is equal to GL_TRANSFORM_FEEDBACK_BUFFER_SIZE
            // GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_EXT is equal to GL_TRANSFORM_FEEDBACK_BUFFER_BINDING
            // GL_INTERLEAVED_ATTRIBS_EXT is equal to GL_INTERLEAVED_ATTRIBS
            // GL_SEPARATE_ATTRIBS_EXT is equal to GL_SEPARATE_ATTRIBS
            // GL_PRIMITIVES_GENERATED_EXT is equal to GL_PRIMITIVES_GENERATED
            // GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_EXT is equal to GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN
            // GL_RASTERIZER_DISCARD_EXT is equal to GL_RASTERIZER_DISCARD
            // GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT is equal to GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS
            // GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_EXT is equal to GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS
            // GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_EXT is equal to GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS
            // GL_TRANSFORM_FEEDBACK_VARYINGS_EXT is equal to GL_TRANSFORM_FEEDBACK_VARYINGS
            // GL_TRANSFORM_FEEDBACK_BUFFER_MODE_EXT is equal to GL_TRANSFORM_FEEDBACK_BUFFER_MODE
            // GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH_EXT is equal to GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH

            // GL_EXT_direct_state_access
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_MATRIX_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSPOSE_PROGRAM_MATRIX_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_MATRIX_STACK_DEPTH_EXT);

            // GL_EXT_vertex_array_bgra
            // Included in OpenGL 3.2
            /* reuse GL_BGRA */

            // GL_EXT_texture_swizzle
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_R_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_G_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_B_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_A_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SWIZZLE_RGBA_EXT);

            // GL_EXT_provoking_vertex:
            AP_GL_ENUM_TOSTRING_CASE(GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FIRST_VERTEX_CONVENTION_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_LAST_VERTEX_CONVENTION_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROVOKING_VERTEX_EXT);

            // GL_EXT_texture_snorm:
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE_ALPHA_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA8_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE8_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE8_ALPHA8_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY8_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA16_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE16_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_LUMINANCE16_ALPHA16_SNORM);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTENSITY16_SNORM);

            // GL_EXT_shader_image_load_store:
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_IMAGE_UNITS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BINDING_NAME_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BINDING_LEVEL_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BINDING_LAYERED_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BINDING_LAYER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BINDING_ACCESS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_1D_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_2D_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_3D_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_2D_RECT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CUBE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BUFFER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_1D_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_2D_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CUBE_MAP_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_2D_MULTISAMPLE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_2D_MULTISAMPLE_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_1D_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_2D_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_3D_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_2D_RECT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_CUBE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_BUFFER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_1D_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_2D_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_CUBE_MAP_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_2D_MULTISAMPLE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_1D_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_2D_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_3D_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_2D_RECT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_CUBE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_BUFFER_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_1D_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_2D_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_IMAGE_SAMPLES_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_BINDING_FORMAT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ARRAY_BARRIER_BIT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BARRIER_BIT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_FETCH_BARRIER_BIT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMMAND_BARRIER_BIT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_BUFFER_BARRIER_BIT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_UPDATE_BARRIER_BIT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_UPDATE_BARRIER_BIT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_BARRIER_BIT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BARRIER_BIT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_ATOMIC_COUNTER_BARRIER_BIT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALL_BARRIER_BITS_EXT);

            // GL_EXT_vertex_attrib_64bit:
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_VEC2_EXT); // same as GL_DOUBLE_VEC2
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_VEC3_EXT); // same as GL_DOUBLE_VEC3
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_VEC4_EXT); // same as GL_DOUBLE_VEC4
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT2_EXT); // same as GL_DOUBLE_MAT2
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT3_EXT); // same as GL_DOUBLE_MAT3
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT4_EXT); // same as GL_DOUBLE_MAT4
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT2x3_EXT); // same as GL_DOUBLE_MAT2x3
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT2x4_EXT); // same as GL_DOUBLE_MAT2x4
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT3x2_EXT); // same as GL_DOUBLE_MAT3x2
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT3x4_EXT); // same as GL_DOUBLE_MAT3x4
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT4x2_EXT); // same as GL_DOUBLE_MAT4x2
            // AP_GL_ENUM_TOSTRING_CASE(GL_DOUBLE_MAT4x3_EXT); // same as GL_DOUBLE_MAT4x3

            // GL_EXT_debug_label:
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_PIPELINE_OBJECT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_OBJECT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_OBJECT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_OBJECT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_OBJECT_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_OBJECT_EXT);

            // GL_EXT_polygon_offset_clamp:
            AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON_OFFSET_CLAMP_EXT);

            // GL_EXT_raster_multisample:
            AP_GL_ENUM_TOSTRING_CASE(GL_RASTER_MULTISAMPLE_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_RASTER_SAMPLES_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_RASTER_SAMPLES_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_RASTER_FIXED_SAMPLE_LOCATIONS_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_MULTISAMPLE_RASTERIZATION_ALLOWED_EXT);
            AP_GL_ENUM_TOSTRING_CASE(GL_EFFECTIVE_RASTER_SAMPLES_EXT);

        default:
            retVal = false;
            break;
    }

    return retVal;
} // checkOpenGLEXTExtensionsEnum


// ---------------------------------------------------------------------------
// Name:        checkOpenGLAMDExtensionsEnum
// Description: Checks if this is an OpenGL AMD extension enumeration
// Arguments:   GLenum enumValue
//              gtString& valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkOpenGLAMDExtensionsEnum(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;

    switch (enumValue)
    {
            // GL_ATI_envmap_bumpmap
            AP_GL_ENUM_TOSTRING_CASE(GL_BUMP_ROT_MATRIX_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUMP_ROT_MATRIX_SIZE_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUMP_NUM_TEX_UNITS_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUMP_TEX_UNITS_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUDV_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_DU8DV8_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUMP_ENVMAP_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUMP_TARGET_ATI);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

            // GL_ATI_fragment_shader
            // GL_FRAGMENT_SHADER_ATI is the same as GL_FRAGMENT_SHADER_EXT
            // GL_REG_0_ATI is the same as GL_REG_0_EXT
            // GL_REG_1_ATI is the same as GL_REG_1_EXT
            // GL_REG_2_ATI is the same as GL_REG_2_EXT
            // GL_REG_3_ATI is the same as GL_REG_3_EXT
            // GL_REG_4_ATI is the same as GL_REG_4_EXT
            // GL_REG_5_ATI is the same as GL_REG_5_EXT
            // GL_REG_6_ATI is the same as GL_REG_6_EXT
            // GL_REG_7_ATI is the same as GL_REG_7_EXT
            // GL_REG_8_ATI is the same as GL_REG_8_EXT
            // GL_REG_9_ATI is the same as GL_REG_9_EXT
            // GL_REG_10_ATI is the same as GL_REG_10_EXT
            // GL_REG_11_ATI is the same as GL_REG_11_EXT
            // GL_REG_12_ATI is the same as GL_REG_12_EXT
            // GL_REG_13_ATI is the same as GL_REG_13_EXT
            // GL_REG_14_ATI is the same as GL_REG_14_EXT
            // GL_REG_15_ATI is the same as GL_REG_15_EXT
            // GL_REG_16_ATI is the same as GL_REG_16_EXT
            // GL_REG_17_ATI is the same as GL_REG_17_EXT
            // GL_REG_18_ATI is the same as GL_REG_18_EXT
            // GL_REG_19_ATI is the same as GL_REG_19_EXT
            // GL_REG_20_ATI is the same as GL_REG_20_EXT
            // GL_REG_21_ATI is the same as GL_REG_21_EXT
            // GL_REG_22_ATI is the same as GL_REG_22_EXT
            // GL_REG_23_ATI is the same as GL_REG_23_EXT
            // GL_REG_24_ATI is the same as GL_REG_24_EXT
            // GL_REG_25_ATI is the same as GL_REG_25_EXT
            // GL_REG_26_ATI is the same as GL_REG_26_EXT
            // GL_REG_27_ATI is the same as GL_REG_27_EXT
            // GL_REG_28_ATI is the same as GL_REG_28_EXT
            // GL_REG_29_ATI is the same as GL_REG_29_EXT
            // GL_REG_30_ATI is the same as GL_REG_30_EXT
            // GL_REG_31_ATI is the same as GL_REG_31_EXT
            // GL_CON_0_ATI is the same as GL_CON_0_EXT
            // GL_CON_1_ATI is the same as GL_CON_1_EXT
            // GL_CON_2_ATI is the same as GL_CON_2_EXT
            // GL_CON_3_ATI is the same as GL_CON_3_EXT
            // GL_CON_4_ATI is the same as GL_CON_4_EXT
            // GL_CON_5_ATI is the same as GL_CON_5_EXT
            // GL_CON_6_ATI is the same as GL_CON_6_EXT
            // GL_CON_7_ATI is the same as GL_CON_7_EXT
            // GL_CON_8_ATI is the same as GL_CON_8_EXT
            // GL_CON_9_ATI is the same as GL_CON_9_EXT
            // GL_CON_10_ATI is the same as GL_CON_10_EXT
            // GL_CON_11_ATI is the same as GL_CON_11_EXT
            // GL_CON_12_ATI is the same as GL_CON_12_EXT
            // GL_CON_13_ATI is the same as GL_CON_13_EXT
            // GL_CON_14_ATI is the same as GL_CON_14_EXT
            // GL_CON_15_ATI is the same as GL_CON_15_EXT
            // GL_CON_16_ATI is the same as GL_CON_16_EXT
            // GL_CON_17_ATI is the same as GL_CON_17_EXT
            // GL_CON_18_ATI is the same as GL_CON_18_EXT
            // GL_CON_19_ATI is the same as GL_CON_19_EXT
            // GL_CON_20_ATI is the same as GL_CON_20_EXT
            // GL_CON_21_ATI is the same as GL_CON_21_EXT
            // GL_CON_22_ATI is the same as GL_CON_22_EXT
            // GL_CON_23_ATI is the same as GL_CON_23_EXT
            // GL_CON_24_ATI is the same as GL_CON_24_EXT
            // GL_CON_25_ATI is the same as GL_CON_25_EXT
            // GL_CON_26_ATI is the same as GL_CON_26_EXT
            // GL_CON_27_ATI is the same as GL_CON_27_EXT
            // GL_CON_28_ATI is the same as GL_CON_28_EXT
            // GL_CON_29_ATI is the same as GL_CON_29_EXT
            // GL_CON_30_ATI is the same as GL_CON_30_EXT
            // GL_CON_31_ATI is the same as GL_CON_31_EXT
            // GL_MOV_ATI is the same as GL_MOV_EXT
            // GL_ADD_ATI is the same as GL_ADD_EXT
            // GL_MUL_ATI is the same as GL_MUL_EXT
            // GL_SUB_ATI is the same as GL_SUB_EXT
            // GL_DOT3_ATI is the same as GL_DOT3_EXT
            // GL_DOT4_ATI is the same as GL_DOT4_EXT
            // GL_MAD_ATI is the same as GL_MAD_EXT
            // GL_LERP_ATI is the same as GL_LERP_EXT
            // GL_CND_ATI is the same as GL_CND_EXT
            // GL_CND0_ATI is the same as GL_CND0_EXT
            // GL_DOT2_ADD_ATI is the same as GL_DOT2_ADD_EXT
            // GL_SECONDARY_INTERPOLATOR_ATI is the same as GL_SECONDARY_INTERPOLATOR_EXT
            // GL_NUM_FRAGMENT_REGISTERS_ATI is the same as GL_NUM_FRAGMENT_REGISTERS_EXT
            // GL_NUM_FRAGMENT_CONSTANTS_ATI is the same as GL_NUM_FRAGMENT_CONSTANTS_EXT
            // GL_NUM_PASSES_ATI is the same as GL_NUM_PASSES_EXT
            // GL_NUM_INSTRUCTIONS_PER_PASS_ATI is the same as GL_NUM_INSTRUCTIONS_PER_PASS_EXT
            // GL_NUM_INSTRUCTIONS_TOTAL_ATI is the same as GL_NUM_INSTRUCTIONS_TOTAL_EXT
            // GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI is the same as GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_EXT
            // GL_NUM_LOOPBACK_COMPONENTS_ATI is the same as GL_NUM_LOOPBACK_COMPONENTS_EXT
            // GL_COLOR_ALPHA_PAIRING_ATI is the same as GL_COLOR_ALPHA_PAIRING_EXT
            // GL_SWIZZLE_STR_ATI is the same as GL_SWIZZLE_STR_EXT
            // GL_SWIZZLE_STQ_ATI is the same as GL_SWIZZLE_STQ_EXT
            // GL_SWIZZLE_STR_DR_ATI is the same as GL_SWIZZLE_STR_DR_EXT
            // GL_SWIZZLE_STQ_DQ_ATI is the same as GL_SWIZZLE_STQ_DQ_EXT
            // GL_SWIZZLE_STRQ_ATI is the same as GL_SWIZZLE_STRQ_EXT
            // GL_SWIZZLE_STRQ_DQ_ATI is the same as GL_SWIZZLE_STRQ_DQ_EXT
#else

            // GL_ATI_fragment_shader
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_SHADER_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_0_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_1_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_2_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_3_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_4_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_5_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_6_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_7_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_8_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_9_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_10_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_11_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_12_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_13_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_14_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_15_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_16_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_17_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_18_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_19_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_20_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_21_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_22_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_23_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_24_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_25_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_26_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_27_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_28_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_29_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_30_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_REG_31_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_0_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_1_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_2_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_3_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_4_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_5_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_6_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_7_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_8_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_9_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_10_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_11_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_12_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_13_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_14_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_15_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_16_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_17_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_18_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_19_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_20_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_21_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_22_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_23_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_24_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_25_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_26_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_27_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_28_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_29_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_30_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CON_31_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_MOV_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_ADD_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_MUL_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_SUB_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT3_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT4_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAD_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_LERP_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CND_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CND0_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT2_ADD_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_SECONDARY_INTERPOLATOR_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_FRAGMENT_REGISTERS_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_FRAGMENT_CONSTANTS_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_PASSES_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_INSTRUCTIONS_PER_PASS_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_INSTRUCTIONS_TOTAL_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_LOOPBACK_COMPONENTS_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ALPHA_PAIRING_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_SWIZZLE_STR_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_SWIZZLE_STQ_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_SWIZZLE_STR_DR_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_SWIZZLE_STQ_DQ_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_SWIZZLE_STRQ_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_SWIZZLE_STRQ_DQ_ATI);
#endif

            // GL_ATI_pn_triangles
            // This extension's Enums have different values in the glext.h file supplied with Macs (glext_apple.h)
            // than the ones in the official glext.h file (from opengl.org):
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case 0x6090:
#else
        case GL_PN_TRIANGLES_ATI:
#endif
            valueString = L"GL_PN_TRIANGLES_ATI";
            break;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case 0x6091:
#else
        case GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI:
#endif
            valueString = L"GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI";
            break;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case 0x6092:
#else
        case GL_PN_TRIANGLES_POINT_MODE_ATI:
#endif
            valueString = L"GL_PN_TRIANGLES_POINT_MODE_ATI";
            break;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case 0x6093:
#else
        case GL_PN_TRIANGLES_NORMAL_MODE_ATI:
#endif
            valueString = L"GL_PN_TRIANGLES_NORMAL_MODE_ATI";
            break;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case 0x6094:
#else
        case GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI:
#endif
            valueString = L"GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI";
            break;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case 0x6095:
#else
        case GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI:
#endif
            valueString = L"GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI";
            break;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case 0x6096:
#else
        case GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI:
#endif
            valueString = L"GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI";
            break;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case 0x6097:
#else
        case GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI:
#endif
            valueString = L"GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI";
            break;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case 0x6098:
#else
        case GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI:
#endif
            valueString = L"GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI";
            break;

            // GL_ATI_vertex_array_object
            AP_GL_ENUM_TOSTRING_CASE(GL_STATIC_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_DYNAMIC_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_PRESERVE_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_DISCARD_ATI);
            // GL_OBJECT_BUFFER_SIZE_ATI - Is the same as GL_OBJECT_BUFFER_SIZE
            // GL_OBJECT_BUFFER_USAGE_ATI - Is the same as GL_OBJECT_BUFFER_USAGE
            AP_GL_ENUM_TOSTRING_CASE(GL_ARRAY_OBJECT_BUFFER_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_ARRAY_OBJECT_OFFSET_ATI);

            // GL_ATI_vertex_streams
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_STREAMS_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_STREAM0_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_STREAM1_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_STREAM2_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_STREAM3_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_STREAM4_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_STREAM5_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_STREAM6_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_STREAM7_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_SOURCE_ATI);

            // GL_ATI_element_array
            AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ARRAY_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ARRAY_TYPE_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ARRAY_POINTER_ATI);

            // GL_ATI_text_fragment_shader
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXT_FRAGMENT_SHADER_ATI);
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

            // GL_ATI_blend_equation_separate
            // GL_ALPHA_BLEND_EQUATION_ATI is the same as GL_BLEND_EQUATION_ALPHA

            // GL_ATI_point_cull_mode
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_CULL_MODE_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_CULL_CENTER_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_CULL_CLIP_ATI);

            // GL_ATIX_pn_triangles
            // GL_PN_TRIANGLES_ATIX is the same as GL_PN_TRIANGLES_ATI
            // GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATIX is the same as GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI
            // GL_PN_TRIANGLES_POINT_MODE_ATIX is the same as GL_PN_TRIANGLES_POINT_MODE_ATI
            // GL_PN_TRIANGLES_NORMAL_MODE_ATIX is the same as GL_PN_TRIANGLES_NORMAL_MODE_ATI
            // GL_PN_TRIANGLES_TESSELATION_LEVEL_ATIX is the same as GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI
            // GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATIX is the same as GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI
            // GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATIX is the same as GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI
            // GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATIX is the same as GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI
            // GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATIX is the same as GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI

            // GL_ATI_texture_compression_3dc
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI);

#endif

            // GL_ATI_draw_buffers
            // GL_MAX_DRAW_BUFFERS_ATI - Is the same as GL_MAX_DRAW_BUFFERS_ARB
            // GL_DRAW_BUFFER0_ATI - Is the same as GL_DRAW_BUFFER0_ARB
            // GL_DRAW_BUFFER1_ATI - Is the same as GL_DRAW_BUFFER1_ARB
            // GL_DRAW_BUFFER2_ATI - Is the same as GL_DRAW_BUFFER2_ARB
            // GL_DRAW_BUFFER3_ATI - Is the same as GL_DRAW_BUFFER3_ARB
            // GL_DRAW_BUFFER4_ATI - Is the same as GL_DRAW_BUFFER4_ARB
            // GL_DRAW_BUFFER5_ATI - Is the same as GL_DRAW_BUFFER5_ARB
            // GL_DRAW_BUFFER6_ATI - Is the same as GL_DRAW_BUFFER6_ARB
            // GL_DRAW_BUFFER7_ATI - Is the same as GL_DRAW_BUFFER7_ARB
            // GL_DRAW_BUFFER8_ATI - Is the same as GL_DRAW_BUFFER8_ARB
            // GL_DRAW_BUFFER9_ATI - Is the same as GL_DRAW_BUFFER9_ARB
            // GL_DRAW_BUFFER10_ATI - Is the same as GL_DRAW_BUFFER10_ARB
            // GL_DRAW_BUFFER11_ATI - Is the same as GL_DRAW_BUFFER11_ARB
            // GL_DRAW_BUFFER12_ATI - Is the same as GL_DRAW_BUFFER12_ARB
            // GL_DRAW_BUFFER13_ATI - Is the same as GL_DRAW_BUFFER13_ARB
            // GL_DRAW_BUFFER14_ATI - Is the same as GL_DRAW_BUFFER14_ARB
            // GL_DRAW_BUFFER15_ATI - Is the same as GL_DRAW_BUFFER15_ARB

            // GL_ATI_pixel_format_float
            // GL_TYPE_RGBA_FLOAT_ATI is equal to GL_RGBA_FLOAT_MODE_ARB
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_CLEAR_UNCLAMPED_VALUE_ATI);

            // GL_ATI_texture_env_combine3
            AP_GL_ENUM_TOSTRING_CASE(GL_MODULATE_ADD_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODULATE_SIGNED_ADD_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_MODULATE_SUBTRACT_ATI);

            // GL_ATI_texture_float
            // GL_RGBA_FLOAT32_ATI is equal to GL_RGBA32F
            // GL_RGB_FLOAT32_ATI is equal to GL_RGB32F
            // GL_ALPHA_FLOAT32_ATI is equal to GL_ALPHA32F_ARB
            // GL_INTENSITY_FLOAT32_ATI is equal to GL_INTENSITY_32F_ARB
            // GL_LUMINANCE_FLOAT32_ATI is equal to GL_LUMINANCE_32F_ARB
            // GL_LUMINANCE_ALPHA_FLOAT32_ATI is equal to GL_LUMINANCE_ALPHA32F_ARB
            // GL_RGBA_FLOAT16_ATI is equal to GL_RGBA16F
            // GL_RGB_FLOAT16_ATI is equal to GL_RGB16F
            // GL_ALPHA_FLOAT16_ATI is equal to GL_ALPHA16F_ARB
            // GL_INTENSITY_FLOAT16_ATI is equal to GL_INTENSITY_16F_ARB
            // GL_LUMINANCE_FLOAT16_ATI is equal to GL_LUMINANCE_16F_ARB
            // GL_LUMINANCE_ALPHA_FLOAT16_ATI is equal to GL_LUMINANCE_ALPHA16F_ARB

            // GL_ATI_map_object_buffer

            // GL_ATI_separate_stencil
            // Included in OpenGL 2.0
            // GL_STENCIL_BACK_FUNC_ATI  - Is the same as GL_STENCIL_BACK_FUNC
            // GL_STENCIL_BACK_FAIL_ATI  - Is the same as GL_STENCIL_BACK_FAIL
            // GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI  - Is the same as GL_STENCIL_BACK_PASS_DEPTH_FAIL
            // GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI  - Is the same as GL_STENCIL_BACK_PASS_DEPTH_PASS

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

            // GL_ATI_array_rev_comps_in_4_bytes
            AP_GL_ENUM_TOSTRING_CASE(GL_ARRAY_REV_COMPS_IN_4_BYTES_ATI);
#endif

            // GL_ATI_vertex_attrib_array_object

            // GL_ATI_meminfo:
            AP_GL_ENUM_TOSTRING_CASE(GL_VBO_FREE_MEMORY_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_FREE_MEMORY_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER_FREE_MEMORY_ATI);

            // GL_AMD_performance_monitor:
            AP_GL_ENUM_TOSTRING_CASE(GL_COUNTER_TYPE_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_COUNTER_RANGE_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT64_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERCENTAGE_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFMON_RESULT_AVAILABLE_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFMON_RESULT_SIZE_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFMON_RESULT_AMD);

            // GL_AMD_texture_texture4:

            // GL_AMD_vertex_shader_tesselator:
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_BUFFER_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_BUFFER_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_BUFFER_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESSELLATION_MODE_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESSELLATION_FACTOR_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_DISCRETE_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONTINUOUS_AMD);

            // GL_AMD_name_gen_delete:
            AP_GL_ENUM_TOSTRING_CASE(GL_DATA_BUFFER_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFORMANCE_MONITOR_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_OBJECT_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_OBJECT_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_OBJECT_AMD);

            // GL_AMD_debug_output:
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_DEBUG_LOGGED_MESSAGES_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_LOGGED_MESSAGES_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SEVERITY_HIGH_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SEVERITY_MEDIUM_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_SEVERITY_LOW_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_CATEGORY_API_ERROR_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_CATEGORY_DEPRECATION_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_CATEGORY_PERFORMANCE_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_CATEGORY_APPLICATION_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEBUG_CATEGORY_OTHER_AMD);

            // GL_AMD_gpu_shader_int64:
            AP_GL_ENUM_TOSTRING_CASE(GL_INT64_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT64_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT8_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT8_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT8_VEC4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT16_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT16_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT16_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT16_VEC4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT64_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT64_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT64_VEC4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT8_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT8_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT8_VEC4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT16_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT16_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT16_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT16_VEC4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT64_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT64_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT64_VEC4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT16_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT16_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT16_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT16_VEC4_NV);

            // GL_AMD_occlusion_query_event:
            AP_GL_ENUM_TOSTRING_CASE(GL_OCCLUSION_QUERY_EVENT_MASK_AMD);
            // AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_DEPTH_PASS_EVENT_BIT_AMD);
            // AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_DEPTH_FAIL_EVENT_BIT_AMD);
            // AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_STENCIL_FAIL_EVENT_BIT_AMD);
            // AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_DEPTH_BOUNDS_FAIL_EVENT_BIT_AMD);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUERY_ALL_EVENT_BITS_AMD);

            // GL_AMD_transform_feedback4:
            AP_GL_ENUM_TOSTRING_CASE(GL_STREAM_RASTERIZATION_AMD);

        default:
            retVal = false;
            break;
    }

    return retVal;
} // checkOpenGLAMDExtensionsEnum

// ---------------------------------------------------------------------------
// Name:        checkOpenGLAppleExtensionsEnum
// Description: Checks if this is an OpenGL Apple extension enumeration
// Arguments:   GLenum enumValue
//              gtString& valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkOpenGLAppleExtensionsEnum(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;

    switch (enumValue)
    {
            // GL_APPLE_specular_vector
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE);

            // GL_APPLE_transform_hint
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_HINT_APPLE);

            // GL_APPLE_client_storage
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_CLIENT_STORAGE_APPLE);

            // GL_APPLE_element_array
            AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ARRAY_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ARRAY_TYPE_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ARRAY_POINTER_APPLE);

            // GL_APPLE_element_array
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case 0x8A0C:
            valueString = L"GL_ELEMENT_ARRAY_APPLE"; // in the real glext (from opengl.org) this enum's value is equal to the ATI one(0x8768)
            break;

        case 0x8A0D:
            valueString = L"GL_ELEMENT_ARRAY_TYPE_APPLE"; // in the real glext (from opengl.org) this enum's value is equal to the ATI one(0x8769)
            break;

        case 0x8A0E:
            valueString = L"GL_ELEMENT_ARRAY_POINTER_APPLE"; // in the real glext (from opengl.org) this enum's value is equal to the ATI one(0x876A)
            break;
            /*
            // Uri, 1/11/09 - This value (0x8A11) was reused by Khronos for GL_UNIFORM_BUFFER in OpenGL 3.1. This causes
            // a conflict when compiling on Mac. Since this was replaced by GL_ELEMENT_ARRAY_BUFFER_BINDING (0x8895) when
            // the extension GL_ARB_vertex_buffer_object was accepted into the OpenGL spec, even modern Apple users should not use this
            // enum / value - so it's better to simply ignore it here.
            case GL_ELEMENT_BUFFER_BINDING_APPLE:
                valueString = L"GL_ELEMENT_BUFFER_BINDING_APPLE";
                break;
                        */
#else
            // GL_ELEMENT_ARRAY_APPLE - Is the same as GL_ELEMENT_ARRAY_ATI
            // GL_ELEMENT_ARRAY_TYPE_APPLE - Is the same as GL_ELEMENT_ARRAY_TYPE_ATI
            // GL_ELEMENT_ARRAY_POINTER_APPLE - Is the same as GL_ELEMENT_ARRAY_POINTER_ATI
#endif

            // GL_APPLE_fence
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_PIXELS_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_FENCE_APPLE);

            // GL_APPLE_aux_depth_stencil
            // This extension is supported only on MAC OS
            AP_GL_ENUM_TOSTRING_CASE(GL_AUX_DEPTH_STENCIL_APPLE);

            // GL_APPLE_object_purgeable
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_OBJECT_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_RELEASED_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VOLATILE_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_RETAINED_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNDEFINED_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PURGEABLE_APPLE);

            // GL_APPLE_row_bytes
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_ROW_BYTES_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_ROW_BYTES_APPLE);

            // GL_APPLE_rgb_422:
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB_422_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB_RAW_422_APPLE);

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

            // GL_APPLE_row_bytes
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_IMAGE_BYTES_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_IMAGE_BYTES_APPLE);

            // GL_APPLE_float_pixels
            // GL_HALF_APPLE is equal to GL_HALF_FLOAT

            // GL_APPLE_pixel_buffer
            AP_GL_ENUM_TOSTRING_CASE(GL_MIN_PBUFFER_VIEWPORT_DIMS_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_FLOAT_APPLE);
            // GL_RGBA_FLOAT32_APPLE is equal to GL_RGBA32F
            // GL_RGB_FLOAT32_APPLE is equal to GL_RGB32F
            // GL_ALPHA_FLOAT32_APPLE is equal to GL_ALPHA32F_ARB
            // GL_INTENSITY_FLOAT32_APPLE is equal to GL_INTENSITY_32F_ARB
            // GL_LUMINANCE_FLOAT32_APPLE is equal to GL_LUMINANCE_32F_ARB
            // GL_LUMINANCE_ALPHA_FLOAT32_APPLEis equal to GL_LUMINANCE_ALPHA32F_ARB
            // GL_RGBA_FLOAT16_APPLEis equal to GL_RGBA16F
            // GL_RGB_FLOAT16_APPLEis equal to GL_RGB16F
            // GL_ALPHA_FLOAT16_APPLEis equal to GL_ALPHA16F_ARB
            // GL_INTENSITY_FLOAT16_APPLEis equal to GL_INTENSITY_16F_ARB
            // GL_LUMINANCE_FLOAT16_APPLEis equal to GL_LUMINANCE_16F_ARB
            // GL_LUMINANCE_ALPHA_FLOAT16_APPLEis equal to GL_LUMINANCE_ALPHA16F_ARB

            // GL_APPLE_vertex_array_object
            // GL_VERTEX_ARRAY_BINDING_APPLE is equal to GL_VERTEX_ARRAY_BINDING

            // GL_APPLE_vertex_array_range
            // GL_VERTEX_ARRAY_RANGE_APPLE - Is the same as GL_VERTEX_ARRAY_RANGE_NV
            // GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE - Is the same as GL_VERTEX_ARRAY_RANGE_LENGTH_NV
            // GL_VERTEX_ARRAY_STORAGE_HINT_APPLE - Is the same as GL_VERTEX_ARRAY_RANGE_VALID_NV
            // GL_VERTEX_ARRAY_RANGE_POINTER_APPLE - Is the same as GL_VERTEX_ARRAY_RANGE_POINTER_NV
            // GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_APPLE is the same as GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV
            AP_GL_ENUM_TOSTRING_CASE(GL_STORAGE_CLIENT_APPLE);

            // GL_APPLE_texture_range (apple-only enums)
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_RANGE_LENGTH_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_RANGE_POINTER_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_STORAGE_HINT_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MINIMIZE_STORAGE_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_STORAGE_PRIVATE_APPLE);
#endif

            // GL_APPLE_texture_range (general glext.h enums)
            AP_GL_ENUM_TOSTRING_CASE(GL_STORAGE_CACHED_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_STORAGE_SHARED_APPLE);

            // GL_APPLE_ycbcr_422
            AP_GL_ENUM_TOSTRING_CASE(GL_YCBCR_422_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_SHORT_8_8_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_SHORT_8_8_REV_APPLE);

            // GL_APPLE_flush_buffer_range
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_SERIALIZED_MODIFY_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_FLUSHING_UNMAP_APPLE);

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

            // GL_APPLE_object_purgeable
            AP_GL_ENUM_TOSTRING_CASE(GL_RELEASED_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VOLATILE_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_RETAINED_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNDEFINED_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_PURGEABLE_APPLE);

            // GL_ATI_blend_weighted_minmax
            AP_GL_ENUM_TOSTRING_CASE(GL_MIN_WEIGHTED_ATI);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_WEIGHTED_ATI);

            // GL_APPLE_vertex_program_evaluators
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_MAP1_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_MAP2_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_MAP1_SIZE_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_MAP1_COEFF_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_MAP1_ORDER_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_MAP1_DOMAIN_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_MAP2_SIZE_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_MAP2_COEFF_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_MAP2_ORDER_APPLE);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_MAP2_DOMAIN_APPLE);

#endif // (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

        default:
            retVal = false;
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        checkOpenGLSGISExtensionsEnum
// Description: Checks if this is an OpenGL SGIS extension enumeration
// Arguments:   GLenum enumValue
//              gtString& valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkOpenGLSGISExtensionsEnum(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;

    switch (enumValue)
    {
            // GL_SGIS_texture_filter4
            AP_GL_ENUM_TOSTRING_CASE(GL_FILTER4_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_FILTER4_SIZE_SGIS);

            // GL_SGI_color_matrix
            // GL_COLOR_MATRIX_SGI - Is the same as GL_COLOR_MATRIX
            // GL_COLOR_MATRIX_STACK_DEPTH_SGI - Is the same as GL_COLOR_MATRIX_STACK_DEPTH
            // GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI - Is the same as GL_MAX_COLOR_MATRIX_STACK_DEPTH
            // GL_POST_COLOR_MATRIX_RED_SCALE_SGI - Is the same as GL_POST_COLOR_MATRIX_RED_SCALE
            // GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI - Is the same as GL_POST_COLOR_MATRIX_GREEN_SCALE
            // GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI - Is the same as GL_POST_COLOR_MATRIX_BLUE_SCALE
            // GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI - Is the same as GL_POST_COLOR_MATRIX_ALPHA_SCALE
            // GL_POST_COLOR_MATRIX_RED_BIAS_SGI - Is the same as GL_POST_COLOR_MATRIX_RED_BIAS
            // GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI - Is the same as GL_POST_COLOR_MATRIX_GREEN_BIAS
            // GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI - Is the same as GL_POST_COLOR_MATRIX_BLUE_BIAS
            // GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI - Is the same as GL_POST_COLOR_MATRIX_ALPHA_BIAS

            // GL_SGI_color_table
            // GL_COLOR_TABLE_SGI - Is the same as GL_COLOR_TABLE
            // GL_POST_CONVOLUTION_COLOR_TABLE_SGI - Is the same as GL_POST_CONVOLUTION_COLOR_TABLE
            // GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI - Is the same as GL_POST_COLOR_MATRIX_COLOR_TABLE
            // GL_PROXY_COLOR_TABLE_SGI - Is the same as GL_PROXY_COLOR_TABLE
            // GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI - Is the same as GL_PROXY_POST_CONVOLUTION_COLOR_TABLE
            // GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI - Is the same as GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE
            // GL_COLOR_TABLE_SCALE_SGI - Is the same as GL_COLOR_TABLE_SCALE
            // GL_COLOR_TABLE_BIAS_SGI - Is the same as GL_COLOR_TABLE_BIAS
            // GL_COLOR_TABLE_FORMAT_SGI - Is the same as GL_COLOR_TABLE_FORMAT
            // GL_COLOR_TABLE_WIDTH_SGI - Is the same as GL_COLOR_TABLE_WIDTH
            // GL_COLOR_TABLE_RED_SIZE_SGI - Is the same as GL_COLOR_TABLE_RED_SIZE
            // GL_COLOR_TABLE_GREEN_SIZE_SGI - Is the same as GL_COLOR_TABLE_GREEN_SIZE
            // GL_COLOR_TABLE_BLUE_SIZE_SGI - Is the same as GL_COLOR_TABLE_BLUE_SIZE
            // GL_COLOR_TABLE_ALPHA_SIZE_SGI - Is the same as GL_COLOR_TABLE_ALPHA_SIZE
            // GL_COLOR_TABLE_LUMINANCE_SIZE_SGI - Is the same as GL_COLOR_TABLE_LUMINANCE_SIZE
            // GL_COLOR_TABLE_INTENSITY_SIZE_SGI - Is the same as GL_COLOR_TABLE_INTENSITY_SIZE

            // GL_SGIS_pixel_texture
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TEXTURE_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_GROUP_COLOR_SGIS);

            // GL_SGIX_pixel_texture
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TEX_GEN_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TEX_GEN_MODE_SGIX);

            // GL_SGIS_texture4D
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_SKIP_VOLUMES_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_IMAGE_DEPTH_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_SKIP_VOLUMES_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_IMAGE_DEPTH_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_4D_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_4D_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_4DSIZE_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_WRAP_Q_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_4D_TEXTURE_SIZE_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_4D_BINDING_SGIS);

            // GL_SGI_texture_color_table
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COLOR_TABLE_SGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_COLOR_TABLE_SGI);

            // GL_SGIS_detail_texture
            AP_GL_ENUM_TOSTRING_CASE(GL_DETAIL_TEXTURE_2D_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DETAIL_TEXTURE_2D_BINDING_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINEAR_DETAIL_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINEAR_DETAIL_ALPHA_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINEAR_DETAIL_COLOR_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DETAIL_TEXTURE_LEVEL_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DETAIL_TEXTURE_MODE_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DETAIL_TEXTURE_FUNC_POINTS_SGIS);

            // GL_SGIS_sharpen_texture
            AP_GL_ENUM_TOSTRING_CASE(GL_LINEAR_SHARPEN_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINEAR_SHARPEN_ALPHA_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINEAR_SHARPEN_COLOR_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHARPEN_TEXTURE_FUNC_POINTS_SGIS);

            // GL_SGIS_texture_lod
            // Included in OpenGL 1.2
            // GL_TEXTURE_MIN_LOD_SGIS - Is the same as GL_TEXTURE_MIN_LOD
            // GL_TEXTURE_MAX_LOD_SGIS - Is the same as GL_TEXTURE_MAX_LOD
            // GL_TEXTURE_BASE_LEVEL_SGIS - Is the same as GL_TEXTURE_BASE_LEVEL
            // GL_TEXTURE_MAX_LEVEL_SGIS - Is the same as GL_TEXTURE_MAX_LEVEL

            // GL_SGIS_multisample
            // GL_MULTISAMPLE_SGIS - Is the same as GL_MULTISAMPLE_
            // GL_SAMPLE_ALPHA_TO_MASK_SGIS - Is the same as GL_SAMPLE_ALPHA_TO_COVERAGE.
            // GL_SAMPLE_ALPHA_TO_ONE_SGIS - Is the same as GL_SAMPLE_ALPHA_TO_ONE_
            // GL_SAMPLE_MASK_SGIS - Is the same as GL_SAMPLE_COVERAGE

            // GL_1PASS_SGIS - Is the same as GL_1PASS_
            // GL_2PASS_0_SGIS - Is the same as GL_2PASS_0_EXT
            // GL_2PASS_1_SGIS - Is the same as GL_2PASS_1_EXT
            // GL_4PASS_0_SGIS - Is the same as GL_4PASS_0_EXT
            // GL_4PASS_1_SGIS - Is the same as GL_4PASS_1_EXT
            // GL_4PASS_2_SGIS - Is the same as GL_4PASS_2_EXT
            // GL_4PASS_3_SGIS - Is the same as GL_4PASS_3_EXT
            // GL_SAMPLE_BUFFERS_SGIS - Is the same as GL_SAMPLE_BUFFERS
            // GL_SAMPLES_SGIS - Is the same as GL_SAMPLES
            // GL_SAMPLE_MASK_VALUE_SGIS - Is the same as GL_SAMPLE_COVERAGE_VALUE.
            // GL_SAMPLE_MASK_INVERT_SGIS - Is the same as GL_SAMPLE_COVERAGE_INVERT
            // GL_SAMPLE_PATTERN_SGIS - Is the same as GL_SAMPLE_PATTERN_EXT.

            // GL_SGIS_generate_mipmap
            // Included in OpenGL 1.4
            // GL_GENERATE_MIPMAP_SGIS - Is the same as GL_GENERATE_MIPMAP
            // GL_GENERATE_MIPMAP_HINT_SGIS - Is the same as GL_GENERATE_MIPMAP_HINT

            // GL_SGIX_clipmap
            AP_GL_ENUM_TOSTRING_CASE(GL_LINEAR_CLIPMAP_LINEAR_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CLIPMAP_CENTER_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CLIPMAP_FRAME_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CLIPMAP_OFFSET_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CLIPMAP_DEPTH_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_CLIPMAP_DEPTH_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEAREST_CLIPMAP_NEAREST_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEAREST_CLIPMAP_LINEAR_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_LINEAR_CLIPMAP_NEAREST_SGIX);

            // GL_SGIX_shadow
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COMPARE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COMPARE_OPERATOR_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_LEQUAL_R_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_GEQUAL_R_SGIX);

            // GL_SGIS_texture_edge_clamp
            // Included in OpenGL 1.2
            // GL_CLAMP_TO_EDGE_SGIS - Is the same as GL_CLAMP_TO_EDGE

            // GL_SGIS_texture_border_clamp
            // GL_CLAMP_TO_BORDER_SGIS - Is the same as GL_CLAMP_TO_BORDER

            // GL_SGIX_interlace
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERLACE_SGIX);

            // GL_SGIX_pixel_tiles
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TILE_CACHE_INCREMENT_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TILE_WIDTH_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TILE_HEIGHT_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TILE_GRID_WIDTH_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TILE_GRID_HEIGHT_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TILE_GRID_DEPTH_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TILE_CACHE_SIZE_SGIX);

            // GL_SGIS_texture_select
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_ALPHA4_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_ALPHA8_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_ALPHA12_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_ALPHA16_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_LUMINANCE4_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_LUMINANCE8_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_LUMINANCE12_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_LUMINANCE16_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_INTENSITY4_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_INTENSITY8_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_INTENSITY12_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_INTENSITY16_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_LUMINANCE_ALPHA4_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_LUMINANCE_ALPHA8_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUAD_ALPHA4_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUAD_ALPHA8_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUAD_LUMINANCE4_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUAD_LUMINANCE8_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUAD_INTENSITY4_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUAD_INTENSITY8_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_DUAL_TEXTURE_SELECT_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_QUAD_TEXTURE_SELECT_SGIS);

            // GL_SGIX_sprite
            AP_GL_ENUM_TOSTRING_CASE(GL_SPRITE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPRITE_MODE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPRITE_AXIS_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPRITE_TRANSLATION_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPRITE_AXIAL_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPRITE_OBJECT_ALIGNED_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPRITE_EYE_ALIGNED_SGIX);

            // GL_SGIX_texture_multi_buffer
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MULTI_BUFFER_HINT_SGIX);

            // GL_SGIS_point_parameters
            // GL_POINT_SIZE_MIN_SGIS - Is the same as GL_POINT_SIZE_MIN
            // GL_POINT_SIZE_MAX_SGIS - Is the same as GL_POINT_SIZE_MAX
            // GL_POINT_FADE_THRESHOLD_SIZE_SGIS - Is the same as GL_POINT_FADE_THRESHOLD_SIZE
            // GL_DISTANCE_ATTENUATION_SGIS - Is the same as GL_DISTANCE_ATTENUATION

            // GL_SGIX_instruments
            AP_GL_ENUM_TOSTRING_CASE(GL_INSTRUMENT_BUFFER_POINTER_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_INSTRUMENT_MEASUREMENTS_SGIX);

            // GL_SGIX_texture_scale_bias
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_TEXTURE_FILTER_BIAS_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_TEXTURE_FILTER_SCALE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX);

            // GL_SGIX_framezoom
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEZOOM_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEZOOM_FACTOR_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAMEZOOM_FACTOR_SGIX);

            // GL_SGIX_tag_sample_buffer

            // GL_FfdMaskSGIX

            // GL_SGIX_polynomial_ffd
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_DEFORMATION_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_DEFORMATION_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEFORMATIONS_MASK_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_DEFORMATION_ORDER_SGIX);

            // GL_SGIX_reference_plane
            AP_GL_ENUM_TOSTRING_CASE(GL_REFERENCE_PLANE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_REFERENCE_PLANE_EQUATION_SGIX);

            // GL_SGIX_flush_raster

            // GL_SGIX_depth_texture
            // GL_DEPTH_COMPONENT16_SGIX - Is the same as GL_DEPTH_COMPONENT16
            // GL_DEPTH_COMPONENT24_SGIX - Is the same as GL_DEPTH_COMPONENT24
            // GL_DEPTH_COMPONENT32_SGIX - Is the same as GL_DEPTH_COMPONENT32

            // GL_SGIS_fog_function
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_FUNC_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_FUNC_POINTS_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FOG_FUNC_POINTS_SGIS);

            // GL_SGIX_fog_offset
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_OFFSET_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_OFFSET_VALUE_SGIX);

            // GL_SGIX_texture_add_env
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_ENV_BIAS_SGIX);

            // GL_SGIX_list_priority
            AP_GL_ENUM_TOSTRING_CASE(GL_LIST_PRIORITY_SGIX);

            // GL_SGIX_ir_instrument1
            AP_GL_ENUM_TOSTRING_CASE(GL_IR_INSTRUMENT1_SGIX);

            // GL_SGIX_calligraphic_fragment
            AP_GL_ENUM_TOSTRING_CASE(GL_CALLIGRAPHIC_FRAGMENT_SGIX);

            // GL_SGIX_texture_lod_bias
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_LOD_BIAS_S_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_LOD_BIAS_T_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_LOD_BIAS_R_SGIX);

            // GL_SGIX_shadow_ambient
            // GL_SHADOW_AMBIENT_SGIX - Is the same as GL_TEXTURE_COMPARE_FAIL_VALUE_ARB

            // GL_SGIX_ycrcb
            AP_GL_ENUM_TOSTRING_CASE(GL_YCRCB_422_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_YCRCB_444_SGIX);

            // GL_SGIX_fragment_lighting
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_LIGHTING_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_COLOR_MATERIAL_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_COLOR_MATERIAL_FACE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_LIGHTS_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_ACTIVE_LIGHTS_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_RASTER_NORMAL_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_LIGHT_ENV_MODE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_LIGHT_MODEL_AMBIENT_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_LIGHT0_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_LIGHT1_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_LIGHT2_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_LIGHT3_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_LIGHT4_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_LIGHT5_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_LIGHT6_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_LIGHT7_SGIX);

            // GL_SGIX_blend_alpha_minmax
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA_MIN_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA_MAX_SGIX);

            // GL_SGIX_impact_pixel_texture
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TEX_GEN_Q_CEILING_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TEX_GEN_Q_ROUND_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TEX_GEN_Q_FLOOR_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TEX_GEN_ALPHA_REPLACE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TEX_GEN_ALPHA_NO_REPLACE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TEX_GEN_ALPHA_LS_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_TEX_GEN_ALPHA_MS_SGIX);

            // GL_SGIX_async
            AP_GL_ENUM_TOSTRING_CASE(GL_ASYNC_MARKER_SGIX);

            // GL_SGIX_async_pixel
            AP_GL_ENUM_TOSTRING_CASE(GL_ASYNC_TEX_IMAGE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_ASYNC_DRAW_PIXELS_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_ASYNC_READ_PIXELS_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_ASYNC_TEX_IMAGE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_ASYNC_DRAW_PIXELS_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_ASYNC_READ_PIXELS_SGIX);

            // GL_SGIX_async_histogram
            AP_GL_ENUM_TOSTRING_CASE(GL_ASYNC_HISTOGRAM_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_ASYNC_HISTOGRAM_SGIX);

            // GL_SGIX_fog_scale
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_SCALE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_SCALE_VALUE_SGIX);

            // GL_SGIX_subsample
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_SUBSAMPLE_RATE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_SUBSAMPLE_RATE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_SUBSAMPLE_4444_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_SUBSAMPLE_2424_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PIXEL_SUBSAMPLE_4242_SGIX);

            // GL_SGIX_ycrcb_subsample

            // GL_SGIX_ycrcba
            AP_GL_ENUM_TOSTRING_CASE(GL_YCRCB_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_YCRCBA_SGIX);

            // GL_SGI_depth_pass_instrument
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_PASS_INSTRUMENT_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_PASS_INSTRUMENT_COUNTERS_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_PASS_INSTRUMENT_MAX_SGIX);

            // GL_SGIX_vertex_preclip
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_PRECLIP_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_PRECLIP_HINT_SGIX);

            // GL_SGIX_convolution_accuracy
            AP_GL_ENUM_TOSTRING_CASE(GL_CONVOLUTION_HINT_SGIX);

            // GL_SGIX_resample
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_RESAMPLE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_RESAMPLE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_RESAMPLE_REPLICATE_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_RESAMPLE_ZERO_FILL_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_RESAMPLE_DECIMATE_SGIX);

            // GL_SGIS_point_line_texgen
            AP_GL_ENUM_TOSTRING_CASE(GL_EYE_DISTANCE_TO_POINT_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_OBJECT_DISTANCE_TO_POINT_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_EYE_DISTANCE_TO_LINE_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_OBJECT_DISTANCE_TO_LINE_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_EYE_POINT_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_OBJECT_POINT_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_EYE_LINE_SGIS);
            AP_GL_ENUM_TOSTRING_CASE(GL_OBJECT_LINE_SGIS);

            // GL_SGIS_texture_color_mask
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COLOR_WRITEMASK_SGIS);

            // GL_SGIX_texture_coordinate_clamp
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MAX_CLAMP_S_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MAX_CLAMP_T_SGIX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MAX_CLAMP_R_SGIX);

            // GL_SGIX_scalebias_hint
            AP_GL_ENUM_TOSTRING_CASE(GL_SCALEBIAS_HINT_SGIX);

            // GL_SGIX_resample:
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_RESAMPLE_SGIX_);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_RESAMPLE_SGIX_);

        // AP_GL_ENUM_TOSTRING_CASE(GL_RESAMPLE_REPLICATE_SGIX_); // This is the same as the renamed GL_PACK_RESAMPLE_SGIX
        // AP_GL_ENUM_TOSTRING_CASE(GL_RESAMPLE_ZERO_FILL_SGIX_); // This is the same as the renamed GL_UNPACK_RESAMPLE_SGIX

        default:
            retVal = false;
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        checkOpenGLHPExtensionsEnum
// Description: Checks if this is an OpenGL HP extension enumeration
// Arguments:   GLenum enumValue
//              gtString& valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkOpenGLHPExtensionsEnum(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;

    switch (enumValue)
    {
            // GL_HP_image_transform
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_SCALE_X_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_SCALE_Y_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_TRANSLATE_X_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_TRANSLATE_Y_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_ROTATE_ANGLE_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_ROTATE_ORIGIN_X_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_ROTATE_ORIGIN_Y_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_MAG_FILTER_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_MIN_FILTER_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_CUBIC_WEIGHT_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_CUBIC_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_AVERAGE_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_IMAGE_TRANSFORM_2D_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP);

            // GL_HP_convolution_border_modes
            AP_GL_ENUM_TOSTRING_CASE(GL_IGNORE_BORDER_HP);
            // GL_CONSTANT_BORDER_HP - Is the same as GL_CONSTANT_BORDER
            // GL_REPLICATE_BORDER_HP - Is the same as GL_REPLICATE_BORDER
            // GL_CONVOLUTION_BORDER_COLOR_HP - Is the same as GL_CONVOLUTION_BORDER_COLOR

            // GL_HP_texture_lighting
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_LIGHTING_MODE_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_POST_SPECULAR_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_PRE_SPECULAR_HP);

            // GL_HP_occlusion_test
            AP_GL_ENUM_TOSTRING_CASE(GL_OCCLUSION_TEST_HP);
            AP_GL_ENUM_TOSTRING_CASE(GL_OCCLUSION_TEST_RESULT_HP);

        default:
            retVal = false;
            break;
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        checkOpenGLNVExtensionsEnum
// Description: Checks if this is an OpenGL NVIDIA extension enumeration
// Arguments:   GLenum enumValue
//              gtString& valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkOpenGLNVExtensionsEnum(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;

    switch (enumValue)
    {
            // GL_NV_texgen_reflection
            // GL_NORMAL_MAP_NV - Is the same as GL_NORMAL_MAP.
            // GL_REFLECTION_MAP_NV - Is the same as GL_REFLECTION_MAP.

            // GL_NV_light_max_exponent
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SHININESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SPOT_EXPONENT_NV);

            // GL_NV_vertex_array_range
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_RANGE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_RANGE_LENGTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_RANGE_VALID_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_RANGE_POINTER_NV);

            // GL_NV_register_combiners
            AP_GL_ENUM_TOSTRING_CASE(GL_REGISTER_COMBINERS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIABLE_A_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIABLE_B_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIABLE_C_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIABLE_D_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIABLE_E_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIABLE_F_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VARIABLE_G_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONSTANT_COLOR0_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONSTANT_COLOR1_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PRIMARY_COLOR_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SECONDARY_COLOR_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPARE0_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPARE1_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DISCARD_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_E_TIMES_F_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SPARE0_PLUS_SECONDARY_COLOR_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_IDENTITY_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INVERT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EXPAND_NORMAL_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EXPAND_NEGATE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_HALF_BIAS_NORMAL_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_HALF_BIAS_NEGATE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_IDENTITY_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_NEGATE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SCALE_BY_TWO_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SCALE_BY_FOUR_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SCALE_BY_ONE_HALF_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_BIAS_BY_NEGATIVE_ONE_HALF_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER_INPUT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER_MAPPING_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER_COMPONENT_USAGE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER_AB_DOT_PRODUCT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER_CD_DOT_PRODUCT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER_MUX_SUM_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER_SCALE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER_BIAS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER_AB_OUTPUT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER_CD_OUTPUT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER_SUM_OUTPUT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GENERAL_COMBINERS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_GENERAL_COMBINERS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_SUM_CLAMP_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER0_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER1_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER5_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER6_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINER7_NV);

            // GL_NV_fog_distance
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_DISTANCE_MODE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EYE_RADIAL_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EYE_PLANE_ABSOLUTE_NV);

            // GL_NV_texgen_emboss
            AP_GL_ENUM_TOSTRING_CASE(GL_EMBOSS_LIGHT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EMBOSS_CONSTANT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EMBOSS_MAP_NV);

            // GL_NV_blend_square
            // Included in OpenGL 1.4

            // GL_NV_texture_env_combine4
            AP_GL_ENUM_TOSTRING_CASE(GL_COMBINE4_NV);
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            // GL_SOURCE3_RGB_NV is the same as GL_SOURCE3_RGB_ARB
            // GL_SOURCE3_ALPHA_NV is the same as GL_SOURCE3_ALPHA_ARB
            // GL_OPERAND3_RGB_NV is the same as GL_OPERAND3_RGB_ARB
            // GL_OPERAND3_ALPHA_NV is the same as GL_OPERAND3_ALPHA_ARB
#else
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE3_RGB_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SOURCE3_ALPHA_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND3_RGB_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OPERAND3_ALPHA_NV);
#endif

            // GL_NV_shader_buffer_load
            AP_GL_ENUM_TOSTRING_CASE(GL_BUFFER_GPU_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_GPU_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SHADER_BUFFER_ADDRESS_NV);

            // GL_NV_fence
            AP_GL_ENUM_TOSTRING_CASE(GL_ALL_COMPLETED_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FENCE_STATUS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FENCE_CONDITION_NV);

            // GL_IBM_texture_mirrored_repeat
            // GL_MIRRORED_REPEAT_IBM - Is the same as GL_MIRRORED_REPEAT

            // GL_NV_evaluators
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_2D_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_TRIANGULAR_2D_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP_TESSELLATION_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP_ATTRIB_U_ORDER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP_ATTRIB_V_ORDER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_FRACTIONAL_TESSELLATION_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB0_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB1_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB5_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB6_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB7_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB9_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB10_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB11_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB12_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB13_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB14_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EVAL_VERTEX_ATTRIB15_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_MAP_TESSELLATION_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_RATIONAL_EVAL_ORDER_NV);

            // GL_NV_packed_depth_stencil
            // GL_DEPTH_STENCIL_NV - Is the same as GL_DEPTH_STENCIL_EXT
            // GL_UNSIGNED_INT_24_8_NV - Is the same as GL_UNSIGNED_INT_24_8_EXT

            // GL_NV_register_combiners2
            AP_GL_ENUM_TOSTRING_CASE(GL_PER_STAGE_CONSTANTS_NV);

            // GL_NV_texture_compression_vtc

            // GL_NV_texture_rectangle
            // GL_TEXTURE_RECTANGLE_NV - Is the same as GL_TEXTURE_RECTANGLE_ARB
            // GL_TEXTURE_BINDING_RECTANGLE_NV - Is the same as GL_TEXTURE_BINDING_RECTANGLE_ARB
            // GL_PROXY_TEXTURE_RECTANGLE_NV - Is the same as GL_PROXY_TEXTURE_RECTANGLE_ARB.
            // GL_MAX_RECTANGLE_TEXTURE_SIZE_NV - Is the same as GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB.

            // GL_NV_texture_shader
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_TEXTURE_RECTANGLE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_S8_S8_8_8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_8_8_S8_S8_REV_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DSDT_MAG_INTENSITY_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_CONSISTENT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_SHADER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_OPERATION_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_CULL_MODES_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_TEXTURE_MATRIX_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_TEXTURE_SCALE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_TEXTURE_BIAS_NV);
            // GL_OFFSET_TEXTURE_2D_MATRIX_NV - Is the same as GL_OFFSET_TEXTURE_MATRIX_NV
            // GL_OFFSET_TEXTURE_2D_SCALE_NV - Is the same as GL_OFFSET_TEXTURE_SCALE_NV
            // GL_OFFSET_TEXTURE_2D_BIAS_NV - Is the same as GL_OFFSET_TEXTURE_BIAS_NV
            AP_GL_ENUM_TOSTRING_CASE(GL_PREVIOUS_TEXTURE_INPUT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONST_EYE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PASS_THROUGH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_CULL_FRAGMENT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_TEXTURE_2D_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPENDENT_AR_TEXTURE_2D_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPENDENT_GB_TEXTURE_2D_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT_PRODUCT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT_PRODUCT_DEPTH_REPLACE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT_PRODUCT_TEXTURE_2D_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_HILO_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DSDT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DSDT_MAG_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DSDT_MAG_VIB_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_HILO16_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_HILO_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_HILO16_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_RGBA_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_RGBA8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_RGB_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_RGB8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_LUMINANCE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_LUMINANCE8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_LUMINANCE_ALPHA_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_LUMINANCE8_ALPHA8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_ALPHA_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_ALPHA8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_INTENSITY_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_INTENSITY8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DSDT8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DSDT8_MAG8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DSDT8_MAG8_INTENSITY8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_RGB_UNSIGNED_ALPHA_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_HI_SCALE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_LO_SCALE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DS_SCALE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DT_SCALE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAGNITUDE_SCALE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIBRANCE_SCALE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_HI_BIAS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_LO_BIAS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DS_BIAS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DT_BIAS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAGNITUDE_BIAS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIBRANCE_BIAS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BORDER_VALUES_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_HI_SIZE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_LO_SIZE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_DS_SIZE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_DT_SIZE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_MAG_SIZE_NV);

            // GL_NV_texture_shader2
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT_PRODUCT_TEXTURE_3D_NV);

            // GL_NV_vertex_array_range2
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV);

            // GL_NV_vertex_program
            // GL_VERTEX_PROGRAM_NV - Is the same as GL_VERTEX_PROGRAM_ARB
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_STATE_PROGRAM_NV);
            // GL_ATTRIB_ARRAY_SIZE_NV - Is the same as GL_ATTRIB_ARRAY_SIZE_ARB
            // GL_ATTRIB_ARRAY_STRIDE_NV - Is the same as GL_ATTRIB_ARRAY_STRIDE_ARB
            // GL_ATTRIB_ARRAY_TYPE_NV - Is the same as GL_ATTRIB_ARRAY_TYPE_ARB
            // GL_CURRENT_ATTRIB_NV - Is the same as GL_CURRENT_ATTRIB_ARB
            // GL_PROGRAM_LENGTH_NV - Is the same as GL_PROGRAM_LENGTH_ARB
            // GL_PROGRAM_STRING_NV - Is the same as GL_PROGRAM_STRING_ARB
            AP_GL_ENUM_TOSTRING_CASE(GL_MODELVIEW_PROJECTION_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_IDENTITY_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INVERSE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSPOSE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INVERSE_TRANSPOSE_NV);
            // GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV - Is the same as GL_MAX_TRACK_MATRIX_STACK_DEPTH_ARB
            // GL_MAX_TRACK_MATRICES_NV - Is the same as GL_MAX_TRACK_MATRICES_ARB
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX0_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX1_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX5_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX6_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATRIX7_NV);
            // GL_CURRENT_MATRIX_STACK_DEPTH_NV - Is the same as GL_CURRENT_MATRIX_STACK_DEPT_ARB
            // GL_CURRENT_MATRIX_NV - Is the same as GL_CURRENT_MATRIX_ARB
            // GL_VERTEX_PROGRAM_POINT_SIZE_NV - Is the same as GL_VERTEX_PROGRAM_POINT_SIZE_ARB
            // GL_VERTEX_PROGRAM_TWO_SIDE_NV - Is the same as GL_VERTEX_PROGRAM_TWO_SIDE_ARB
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_PARAMETER_NV);
            // GL_ATTRIB_ARRAY_POINTER_NV - Is the same as GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB.
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_TARGET_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_RESIDENT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRACK_MATRIX_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRACK_MATRIX_TRANSFORM_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_PROGRAM_BINDING_NV);
            // GL_PROGRAM_ERROR_POSITION_NV - Is the same as GL_PROGRAM_ERROR_POSITION_ARB
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY0_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY1_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY5_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY6_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY7_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY9_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY10_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY11_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY12_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY13_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY14_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY15_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB0_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB1_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB2_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB3_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB4_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB5_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB6_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB7_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB8_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB9_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB10_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB11_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB12_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB13_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB14_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP1_VERTEX_ATTRIB15_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB0_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB1_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB2_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB3_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB4_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB5_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB6_4_NV);
            //GL_MAP2_VERTEX_ATTRIB7_4_NV - Is the same as GL_PROGRAM_BINDING_ARB

            // GL_NV_copy_depth_to_color
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_STENCIL_TO_RGBA_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_STENCIL_TO_BGRA_NV);

            // GL_NV_multisample_filter_hint
            AP_GL_ENUM_TOSTRING_CASE(GL_MULTISAMPLE_FILTER_HINT_NV);

            // GL_NV_depth_clamp
            // Included in OpenGL 3.2
            // AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_CLAMP_NV); // Same as GL_DEPTH_CLAMP

            // GL_NV_occlusion_query
            // GL_PIXEL_COUNTER_BITS_NV - Is the same as GL_PIXEL_COUNTER_BIT
            // GL_CURRENT_OCCLUSION_QUERY_ID_NV - Is the same as GL_CURRENT_QUERY
            // GL_PIXEL_COUNT_NV - Is the same as GL_QUERY_RESULT
            // GL_PIXEL_COUNT_AVAILABLE_NV - Is the same as GL_QUERY_RESULT_AVAILABLE

            // GL_NV_point_sprite
            // GL_POINT_SPRITE_NV - Is the same as GL_POINT_SPRITE_ARB
            // GL_COORD_REPLACE_NV - Is the same as GL_COORD_REPLACE_ARB
            AP_GL_ENUM_TOSTRING_CASE(GL_POINT_SPRITE_R_MODE_NV);

            // GL_NV_texture_shader3
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_HILO_TEXTURE_2D_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_HILO_TEXTURE_RECTANGLE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPENDENT_HILO_TEXTURE_2D_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPENDENT_RGB_TEXTURE_3D_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT_PRODUCT_PASS_THROUGH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT_PRODUCT_TEXTURE_1D_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_HILO8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SIGNED_HILO8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FORCE_BLUE_TO_ONE_NV);

            // GL_NV_vertex_program1_1

            // GL_NV_video_capture
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_BUFFER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_BUFFER_BINDING_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FIELD_UPPER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FIELD_LOWER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_VIDEO_CAPTURE_STREAMS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEXT_VIDEO_CAPTURE_BUFFER_STATUS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_CAPTURE_TO_422_SUPPORTED_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_LAST_VIDEO_CAPTURE_STATUS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_BUFFER_PITCH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_COLOR_CONVERSION_MATRIX_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_COLOR_CONVERSION_MAX_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_COLOR_CONVERSION_MIN_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_COLOR_CONVERSION_OFFSET_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_BUFFER_INTERNAL_FORMAT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PARTIAL_SUCCESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SUCCESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FAILURE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_YCBYCR8_422_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_YCBAYCR8A_4224_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_Z6Y10Z6CB10Z6Y10Z6CR10_422_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_Z6Y10Z6CB10Z6A10Z6Y10Z6CR10Z6A10_4224_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_Z4Y12Z4CB12Z4Y12Z4CR12_422_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_Z4Y12Z4CB12Z4A12Z4Y12Z4CR12Z4A12_4224_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_Z4Y12Z4CB12Z4CR12_444_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_CAPTURE_FRAME_WIDTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_CAPTURE_FRAME_HEIGHT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_CAPTURE_FIELD_UPPER_HEIGHT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_CAPTURE_FIELD_LOWER_HEIGHT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VIDEO_CAPTURE_SURFACE_ORIGIN_NV);

            // GL_NV_parameter_buffer_object

            // GL_NV_vertex_buffer_unified_memory
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ARRAY_UNIFIED_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMAL_ARRAY_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ARRAY_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_ARRAY_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COORD_ARRAY_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EDGE_FLAG_ARRAY_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SECONDARY_COLOR_ARRAY_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_COORD_ARRAY_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ARRAY_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ATTRIB_ARRAY_LENGTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_LENGTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMAL_ARRAY_LENGTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ARRAY_LENGTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_ARRAY_LENGTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COORD_ARRAY_LENGTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EDGE_FLAG_ARRAY_LENGTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SECONDARY_COLOR_ARRAY_LENGTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_COORD_ARRAY_LENGTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ARRAY_LENGTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_INDIRECT_UNIFIED_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_INDIRECT_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_INDIRECT_LENGTH_NV);

            // GL_NV_float_buffer
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_R_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_RG_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_RGB_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_RGBA_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_R16_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_R32_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_RG16_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_RG32_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_RGB16_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_RGB32_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_RGBA16_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_RGBA32_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_FLOAT_COMPONENTS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_CLEAR_COLOR_VALUE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT_RGBA_MODE_NV);

            // GL_NV_fragment_program
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_PROGRAM_NV);
            // GL_MAX_TEXTURE_COORDS_NV - Is the same as GL_MAX_TEXTURE_COORD_ARB
            // GL_MAX_TEXTURE_IMAGE_UNITS_NV - Is the same as GL_MAX_TEXTURE_IMAGE_UNIT_ARB
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_PROGRAM_BINDING_NV);
            // GL_PROGRAM_ERROR_STRING_NV - Is the same as GL_PROGRAM_ERROR_STRING_ARB

            // GL_NV_half_float
            // Included in OpenGL 3.0
            // GL_HALF_FLOAT_NV is equal to GL_HALF_FLOAT

            // GL_NV_pixel_data_range
            AP_GL_ENUM_TOSTRING_CASE(GL_WRITE_PIXEL_DATA_RANGE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_READ_PIXEL_DATA_RANGE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_WRITE_PIXEL_DATA_RANGE_LENGTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_READ_PIXEL_DATA_RANGE_LENGTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_WRITE_PIXEL_DATA_RANGE_POINTER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_READ_PIXEL_DATA_RANGE_POINTER_NV);

            // GL_NV_primitive_restart
            // Included in OpenGL 3.1 but enum values were changed
            AP_GL_ENUM_TOSTRING_CASE(GL_PRIMITIVE_RESTART_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PRIMITIVE_RESTART_INDEX_NV);

            // GL_NV_texture_expand_normal
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_UNSIGNED_REMAP_MODE_NV);

            // GL_NV_vertex_program2
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB8_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB9_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB10_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB11_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB12_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB13_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB14_4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAP2_VERTEX_ATTRIB15_4_NV);

            // GL_NV_fragment_program_option

            // GL_NV_fragment_program2
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_CALL_DEPTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_IF_DEPTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_LOOP_DEPTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_LOOP_COUNT_NV);

            // GL_NV_vertex_program2_option

            // GL_NV_vertex_program3

            // GL_NV_gpu_program4
            // GL_MIN_PROGRAM_TEXEL_OFFSET_NV is equal to GL_MIN_PROGRAM_TEXEL_OFFSET
            // GL_MAX_PROGRAM_TEXEL_OFFSET_NV is equal to GL_MAX_PROGRAM_TEXEL_OFFSET
            // GL_VERTEX_ATTRIB_ARRAY_INTEGER_EXT is the same as GL_VERTEX_ATTRIB_ARRAY_INTEGER (apple-only)
            // GL_MIN_PROGRAM_TEXEL_OFFSET_EXT is the same as GL_MIN_PROGRAM_TEXEL_OFFSET (apple-only)
            // GL_MAX_PROGRAM_TEXEL_OFFSET_EXT is the same as GL_MAX_PROGRAM_TEXEL_OFFSET (apple-only)
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_ATTRIB_COMPONENTS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAM_RESULT_COMPONENTS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_ATTRIB_COMPONENTS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_RESULT_COMPONENTS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_GENERIC_ATTRIBS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_GENERIC_RESULTS_NV);

            // GL_NV_geometry_program4
            // GL_LINES_ADJACENCY_EXT is equal to GL_LINES_ADJACENCY
            // GL_LINE_STRIP_ADJACENCY_EXT is equal to GL_LINE_STRIP_ADJACENCY
            // GL_TRIANGLES_ADJACENCY_EXT is equal to GL_TRIANGLES_ADJACENCY
            // GL_TRIANGLE_STRIP_ADJACENCY_EXT is equal to GL_TRIANGLE_STRIP_ADJACENCY
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_PROGRAM_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_OUTPUT_VERTICES_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_TOTAL_OUTPUT_COMPONENTS_NV);
            // GL_GEOMETRY_VERTICES_OUT_EXT is equal to GL_GEOMETRY_VERTICES_OUT_ARB
            // GL_GEOMETRY_INPUT_TYPE_EXT is equal to GL_GEOMETRY_INPUT_TYPE_ARB
            // GL_GEOMETRY_OUTPUT_TYPE_EXT is equal to GL_GEOMETRY_OUTPUT_TYPE_ARB
            // GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT is equal to GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB
            // GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT is equal to GL_FRAMEBUFFER_ATTACHMENT_LAYERED_ARB
            // GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT is equal to GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB
            // GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_EXT is equal to GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB
            // GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT same as: GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT
            // GL_PROGRAM_POINT_SIZE_EXT same as: GL_VERTEX_PROGRAM_POINT_SIZE

            // GL_EXT_geometry_shader4
            // GL_GEOMETRY_SHADER_EXT is equal to GL_GEOMETRY_SHADER_ARB
            // GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT is equal to GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB
            // GL_MAX_VERTEX_VARYING_COMPONENTS_EXT is equal to GL_MAX_VERTEX_VARYING_COMPONENTS_ARB
            // GL_MAX_VARYING_COMPONENTS_EXT same as: GL_MAX_VARYING_FLOATS
            // GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT is equal to GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB
            // GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT is equal to GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB
            // GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT is equal to GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB

            // GL_NV_vertex_program4
            // GL_VERTEX_ATTRIB_ARRAY_INTEGER_NV is equal to GL_VERTEX_ATTRIB_ARRAY_INTEGER

            // GL_NV_depth_buffer_float
            // Included in OpenGL 3.0
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_COMPONENT32F_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH32F_STENCIL8_NV);
            // GL_FLOAT_32_UNSIGNED_INT_24_8_REV_NV is equal to GL_FLOAT_32_UNSIGNED_INT_24_8_REV
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_BUFFER_FLOAT_MODE_NV);

            // GL_NV_fragment_program4

            // GL_NV_framebuffer_multisample_coverage
            // GL_RENDERBUFFER_COVERAGE_SAMPLES_NV same as: GL_RENDERBUFFER_SAMPLES_EXT
            AP_GL_ENUM_TOSTRING_CASE(GL_RENDERBUFFER_COLOR_SAMPLES_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MULTISAMPLE_COVERAGE_MODES_NV);

            // GL_EXT_framebuffer_sRGB
            // GL_FRAMEBUFFER_SRGB_EXT is equal to GL_FRAMEBUFFER_SRGB_ARB
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_SRGB_CAPABLE_EXT);

            // GL_NV_geometry_shader4

            // GL_NV_parameter_buffer_object
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_PARAMETER_BUFFER_BINDINGS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_PARAMETER_BUFFER_SIZE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_PROGRAM_PARAMETER_BUFFER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_GEOMETRY_PROGRAM_PARAMETER_BUFFER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_PROGRAM_PARAMETER_BUFFER_NV);

            // GL_EXT_draw_buffers2
            // Included in OpenGL 3.0

            // GL_NV_transform_feedback
            AP_GL_ENUM_TOSTRING_CASE(GL_BACK_PRIMARY_COLOR_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_BACK_SECONDARY_COLOR_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COORD_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_DISTANCE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ID_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PRIMITIVE_ID_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_GENERIC_ATTRIB_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_ATTRIBS_NV);
            // GL_TRANSFORM_FEEDBACK_BUFFER_MODE_NV is equal to GL_TRANSFORM_FEEDBACK_BUFFER_MODE
            // GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_NV is equal to GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_VARYINGS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_ACTIVE_VARYING_MAX_LENGTH_NV);
            // GL_TRANSFORM_FEEDBACK_VARYINGS_NV is equal to GL_TRANSFORM_FEEDBACK_VARYINGS
            // GL_TRANSFORM_FEEDBACK_BUFFER_START_NV is equal to GL_TRANSFORM_FEEDBACK_BUFFER_START
            // GL_TRANSFORM_FEEDBACK_BUFFER_SIZE_NV is equal to GL_TRANSFORM_FEEDBACK_BUFFER_SIZE
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_RECORD_NV);
            // GL_PRIMITIVES_GENERATED_NV is equal to GL_PRIMITIVES_GENERATED
            // GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN_NV is equal to GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN
            // GL_RASTERIZER_DISCARD_NV is equal to GL_RASTERIZER_DISCARD
            // GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_ATTRIBS_NV is equal to GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_ATTRIBS
            // GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_NV is equal to GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS
            // GL_INTERLEAVED_ATTRIBS_NV is equal to GL_INTERLEAVED_ATTRIBS
            // GL_SEPARATE_ATTRIBS_NV is equal to GL_SEPARATE_ATTRIBS
            // GL_TRANSFORM_FEEDBACK_BUFFER_NV is equal to GL_TRANSFORM_FEEDBACK_BUFFER
            // GL_TRANSFORM_FEEDBACK_BUFFER_BINDING_NV is equal to GL_TRANSFORM_FEEDBACK_BUFFER_BINDING
            AP_GL_ENUM_TOSTRING_CASE(GL_LAYER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_NEXT_BUFFER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SKIP_COMPONENTS4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SKIP_COMPONENTS3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SKIP_COMPONENTS2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SKIP_COMPONENTS1_NV);

            // GL_NV_conditional_render
            // Included in OpenGL 3.0
            // GL_QUERY_WAIT_NV is equal to GL_QUERY_WAIT
            // GL_QUERY_NO_WAIT_NV is equal to GL_QUERY_NO_WAIT
            // GL_QUERY_BY_REGION_WAIT_NV is equal to GL_QUERY_BY_REGION_WAIT
            // GL_QUERY_BY_REGION_NO_WAIT_NV is equal to GL_QUERY_BY_REGION_NO_WAIT

            // GL_NV_present_video
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAME_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FIELDS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_CURRENT_TIME_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_NUM_FILL_STREAMS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PRESENT_TIME_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PRESENT_DURATION_NV);

            // GL_NV_explicit_multisample
            // GL_SAMPLE_POSITION_NV is equal to GL_SAMPLE_POSITION
            // GL_SAMPLE_MASK_NV is equal to GL_SAMPLE_MASK
            // GL_SAMPLE_MASK_VALUE_NV is equal to GL_SAMPLE_MASK_VALUE
            // GL_MAX_SAMPLE_MASK_WORDS_NV is equal to GL_MAX_SAMPLE_MASK_WORDS
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_BINDING_RENDERBUFFER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_RENDERBUFFER_DATA_STORE_BINDING_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_RENDERBUFFER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLER_RENDERBUFFER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT_SAMPLER_RENDERBUFFER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT_SAMPLER_RENDERBUFFER_NV);

            // GL_NV_transform_feedback2
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRANSFORM_FEEDBACK_BINDING_NV);

            // GL_NV_gpu_program5:
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_GEOMETRY_PROGRAM_INVOCATIONS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_PROGRAM_INTERPOLATION_OFFSET_BITS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_SUBROUTINE_PARAMETERS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_SUBROUTINE_NUM_NV);

            // GL_NV_gpu_shader5:
            AP_GL_ENUM_TOSTRING_CASE(GL_INT64_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT64_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT8_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT8_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT8_VEC4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT16_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT16_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT16_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT16_VEC4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT64_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT64_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_INT64_VEC4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT8_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT8_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT8_VEC4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT16_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT16_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT16_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT16_VEC4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT64_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT64_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNSIGNED_INT64_VEC4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT16_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT16_VEC2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT16_VEC3_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FLOAT16_VEC4_NV);

            // GL_NV_shader_buffer_store:
            AP_GL_ENUM_TOSTRING_CASE(GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV);

            // GL_NV_tessellation_program5:
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_PROGRAM_PATCH_ATTRIBS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_CONTROL_PROGRAM_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_EVALUATION_PROGRAM_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_CONTROL_PROGRAM_PARAMETER_BUFFER_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_TESS_EVALUATION_PROGRAM_PARAMETER_BUFFER_NV);

            // GL_NV_vertex_attrib_integer_64bit

            // GL_NV_multisample_coverage:
            // GL_COVERAGE_SAMPLES_NV - Replaced by GL_SAMPLES[_ARB]
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_SAMPLES_NV);

            // GL_NV_vdpau_interop:
            AP_GL_ENUM_TOSTRING_CASE(GL_SURFACE_STATE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SURFACE_REGISTERED_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SURFACE_MAPPED_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_WRITE_DISCARD_NV);

            // GL_NVX_gpu_memory_info:
            AP_GL_ENUM_TOSTRING_CASE(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX);
            AP_GL_ENUM_TOSTRING_CASE(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX);
            AP_GL_ENUM_TOSTRING_CASE(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX);
            AP_GL_ENUM_TOSTRING_CASE(GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX);
            AP_GL_ENUM_TOSTRING_CASE(GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX);

            // GL_NV_blend_equation_advanced:
            AP_GL_ENUM_TOSTRING_CASE(GL_BLUE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_GREEN_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_RED_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_XOR_NV);

            // GL_NV_blend_equation_advanced_coherent:
            AP_GL_ENUM_TOSTRING_CASE(GL_BLEND_ADVANCED_COHERENT_NV);

            // AP_GL_ENUM_TOSTRING_CASE(GL_TERMINATE_SEQUENCE_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_NOP_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_ELEMENTS_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_ARRAYS_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_ELEMENTS_STRIP_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_ARRAYS_STRIP_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_ELEMENTS_INSTANCED_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_DRAW_ARRAYS_INSTANCED_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ELEMENT_ADDRESS_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ATTRIBUTE_ADDRESS_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_ADDRESS_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_BLEND_COLOR_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_REF_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_LINE_WIDTH_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_POLYGON_OFFSET_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA_REF_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_VIEWPORT_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_SCISSOR_COMMAND_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_FRONT_FACE_COMMAND_NV);

            // GL_NV_conservative_raster:
            AP_GL_ENUM_TOSTRING_CASE(GL_CONSERVATIVE_RASTERIZATION_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SUBPIXEL_PRECISION_BIAS_X_BITS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SUBPIXEL_PRECISION_BIAS_Y_BITS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_SUBPIXEL_PRECISION_BIAS_BITS_NV);

            // GL_NV_fill_rectangle:
            AP_GL_ENUM_TOSTRING_CASE(GL_FILL_RECTANGLE_NV);

            // GL_NV_fragment_coverage_to_color:
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_COVERAGE_TO_COLOR_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_COVERAGE_COLOR_NV);

            // GL_NV_framebuffer_mixed_samples:
            AP_GL_ENUM_TOSTRING_CASE(GL_COVERAGE_MODULATION_TABLE_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_SAMPLES_NV); // Already defined in GL_NV_multisample_coverage:
            AP_GL_ENUM_TOSTRING_CASE(GL_DEPTH_SAMPLES_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_STENCIL_SAMPLES_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIXED_DEPTH_SAMPLES_SUPPORTED_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_MIXED_STENCIL_SAMPLES_SUPPORTED_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COVERAGE_MODULATION_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_COVERAGE_MODULATION_TABLE_SIZE_NV);

            // GL_NV_internalformat_sample_query:
            AP_GL_ENUM_TOSTRING_CASE(GL_MULTISAMPLES_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SUPERSAMPLE_SCALE_X_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SUPERSAMPLE_SCALE_Y_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONFORMANT_NV);

            // GL_NV_path_rendering:
            AP_GL_ENUM_TOSTRING_CASE(GL_ROUNDED_RECT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_RELATIVE_ROUNDED_RECT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_ROUNDED_RECT2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_RELATIVE_ROUNDED_RECT2_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_ROUNDED_RECT4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_RELATIVE_ROUNDED_RECT4_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_ROUNDED_RECT8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_RELATIVE_ROUNDED_RECT8_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_RELATIVE_RECT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FONT_GLYPHS_AVAILABLE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FONT_TARGET_UNAVAILABLE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FONT_UNAVAILABLE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FONT_UNINTELLIGIBLE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONIC_CURVE_TO_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_RELATIVE_CONIC_CURVE_TO_NV);
            // AP_GL_ENUM_TOSTRING_CASE(GL_FONT_NUM_GLYPH_INDICES_BIT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_STANDARD_FONT_FORMAT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_2_BYTES_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_3_BYTES_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_4_BYTES_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_EYE_LINEAR_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_OBJECT_LINEAR_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONSTANT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATH_FOG_GEN_MODE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATH_GEN_COLOR_FORMAT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATH_PROJECTION_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATH_MODELVIEW_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATH_MODELVIEW_STACK_DEPTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATH_MODELVIEW_MATRIX_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATH_MAX_MODELVIEW_STACK_DEPTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATH_TRANSPOSE_MODELVIEW_MATRIX_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATH_PROJECTION_STACK_DEPTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATH_PROJECTION_MATRIX_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATH_MAX_PROJECTION_STACK_DEPTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PATH_TRANSPOSE_PROJECTION_MATRIX_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAGMENT_INPUT_NV);

            // GL_NV_path_rendering_shared_edge:
            AP_GL_ENUM_TOSTRING_CASE(GL_SHARED_EDGE_NV);

            // GL_NV_sample_locations:
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_LOCATION_SUBPIXEL_BITS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_LOCATION_PIXEL_GRID_WIDTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_LOCATION_PIXEL_GRID_HEIGHT_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAMMABLE_SAMPLE_LOCATION_TABLE_SIZE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_LOCATION_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROGRAMMABLE_SAMPLE_LOCATION_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_PROGRAMMABLE_SAMPLE_LOCATIONS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_SAMPLE_LOCATION_PIXEL_GRID_NV);

            // GL_NV_shader_thread_group:
            AP_GL_ENUM_TOSTRING_CASE(GL_WARP_SIZE_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_WARPS_PER_SM_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_SM_COUNT_NV);

            // GL_NV_uniform_buffer_unified_memory:
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER_UNIFIED_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER_ADDRESS_NV);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNIFORM_BUFFER_LENGTH_NV);

        default:
            retVal = false;
            break;
    }

    return retVal;
} // checkOpenGLNVExtensionsEnum


// ---------------------------------------------------------------------------
// Name:        checkWGLExtensionsEnum
// Description: Check if this is a WGL enumeration
// Arguments:   GLenum enumValue
//              gtString& valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkWGLExtensionsEnum(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;
    GT_UNREFERENCED_PARAMETER(valueString); // Resolve the compiler warning for the Linux variant

    switch (enumValue)
    {
            // WGL Extensions:
            // WGL is windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

            // WGL_ARB_multisample:
            AP_GL_ENUM_TOSTRING_CASE(WGL_SAMPLE_BUFFERS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_SAMPLES_ARB);

            // WGL_ARB_pixel_format:
            AP_GL_ENUM_TOSTRING_CASE(WGL_NUMBER_PIXEL_FORMATS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_DRAW_TO_WINDOW_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_DRAW_TO_BITMAP_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_ACCELERATION_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_NEED_PALETTE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_NEED_SYSTEM_PALETTE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_SWAP_LAYER_BUFFERS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_SWAP_METHOD_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_NUMBER_OVERLAYS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_NUMBER_UNDERLAYS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TRANSPARENT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TRANSPARENT_RED_VALUE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TRANSPARENT_GREEN_VALUE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TRANSPARENT_BLUE_VALUE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TRANSPARENT_ALPHA_VALUE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TRANSPARENT_INDEX_VALUE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_SHARE_DEPTH_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_SHARE_STENCIL_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_SHARE_ACCUM_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_SUPPORT_GDI_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_SUPPORT_OPENGL_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_DOUBLE_BUFFER_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_STEREO_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_PIXEL_TYPE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_COLOR_BITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_RED_BITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_RED_SHIFT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GREEN_BITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GREEN_SHIFT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_BLUE_BITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_BLUE_SHIFT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_ALPHA_BITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_ALPHA_SHIFT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_ACCUM_BITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_ACCUM_RED_BITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_ACCUM_GREEN_BITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_ACCUM_BLUE_BITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_ACCUM_ALPHA_BITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_DEPTH_BITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_STENCIL_BITS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_AUX_BUFFERS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_NO_ACCELERATION_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GENERIC_ACCELERATION_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_FULL_ACCELERATION_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_SWAP_EXCHANGE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_SWAP_COPY_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_SWAP_UNDEFINED_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TYPE_RGBA_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TYPE_COLORINDEX_ARB);

            // WGL_ARB_make_current_read:
            AP_GL_ENUM_TOSTRING_CASE(ERROR_INVALID_PIXEL_TYPE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB);

            // WGL_ARB_pbuffer:
            AP_GL_ENUM_TOSTRING_CASE(WGL_DRAW_TO_PBUFFER_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_MAX_PBUFFER_PIXELS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_MAX_PBUFFER_WIDTH_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_MAX_PBUFFER_HEIGHT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_PBUFFER_LARGEST_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_PBUFFER_WIDTH_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_PBUFFER_HEIGHT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_PBUFFER_LOST_ARB);

            // WGL_ARB_render_texture:
            AP_GL_ENUM_TOSTRING_CASE(WGL_BIND_TO_TEXTURE_RGB_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_BIND_TO_TEXTURE_RGBA_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_FORMAT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_TARGET_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_MIPMAP_TEXTURE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_RGB_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_RGBA_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_NO_TEXTURE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_CUBE_MAP_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_1D_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_2D_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_MIPMAP_LEVEL_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_CUBE_MAP_FACE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_FRONT_LEFT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_FRONT_RIGHT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_BACK_LEFT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_BACK_RIGHT_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_AUX0_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_AUX1_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_AUX2_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_AUX3_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_AUX4_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_AUX5_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_AUX6_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_AUX7_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_AUX8_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_AUX9_ARB);

            // WGL_ARB_pixel_format_float
            AP_GL_ENUM_TOSTRING_CASE(WGL_TYPE_RGBA_FLOAT_ARB);

            // GL_ARB_create_context
            AP_GL_ENUM_TOSTRING_CASE(WGL_CONTEXT_MAJOR_VERSION_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_CONTEXT_MINOR_VERSION_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_CONTEXT_LAYER_PLANE_ARB);
            AP_GL_ENUM_TOSTRING_CASE(WGL_CONTEXT_FLAGS_ARB);
            AP_GL_ENUM_TOSTRING_CASE(ERROR_INVALID_VERSION_ARB);

            // WGL_EXT_make_current_read:
            // ERROR_INVALID_PIXEL_TYPE_EXT - Is the same as ERROR_INVALID_PIXEL_TYPE_ARB
            // WGL_NEED_PALETTE_EXT - Is the same as WGL_NEED_PALETTE_ARB.
            // WGL_NEED_SYSTEM_PALETTE_EXT - Is the same as WGL_NEED_SYSTEM_PALETTE_ARB.
            // WGL_SWAP_LAYER_BUFFERS_EXT - Is the same as WGL_SWAP_LAYER_BUFFERS_ARB.
            // WGL_SWAP_METHOD_EXT - Is the same as WGL_SWAP_METHOD_ARB.
            // WGL_NUMBER_OVERLAYS_EXT - Is the same as WGL_NUMBER_OVERLAYS_ARB.
            // WGL_NUMBER_UNDERLAYS_EXT - Is the same as WGL_NUMBER_UNDERLAYS_ARB.
            // WGL_TRANSPARENT_EXT - Is the same as WGL_TRANSPARENT_ARB.
            AP_GL_ENUM_TOSTRING_CASE(WGL_TRANSPARENT_VALUE_EXT);
            // WGL_SHARE_DEPTH_EXT - Is the same as WGL_SHARE_DEPTH_ARB.
            // WGL_SHARE_STENCIL_EXT - Is the same as WGL_SHARE_STENCIL_ARB.
            // WGL_SHARE_ACCUM_EXT - Is the same as WGL_SHARE_ACCUM_EXT_ARB.
            // WGL_SUPPORT_GDI_EXT - Is the same as WGL_SUPPORT_GDI_ARB.
            // WGL_SUPPORT_OPENGL_EXT - Is the same as WGL_SUPPORT_OPENGL_ARB.
            // WGL_DOUBLE_BUFFER_EXT - Is the same as WGL_DOUBLE_BUFFER_ARB.
            // WGL_STEREO_EXT - Is the same as WGL_STEREO_ARB.
            // WGL_PIXEL_TYPE_EXT - Is the same as WGL_PIXEL_TYPE_ARB.
            // WGL_COLOR_BITS_EXT - Is the same as WGL_COLOR_BITS_ARB.
            // WGL_RED_BITS_EXT - Is the same as WGL_RED_BITS_ARB.
            // WGL_RED_SHIFT_EXT - Is the same as WGL_RED_SHIFT_ARB.
            // WGL_GREEN_BITS_EXT - Is the same as WGL_GREEN_BITS_ARB.
            // WGL_GREEN_SHIFT_EXT - Is the same as WGL_GREEN_SHIFT_ARB.
            // WGL_BLUE_BITS_EXT - Is the same as WGL_BLUE_BITS_ARB.
            // WGL_BLUE_SHIFT_EXT - Is the same as WGL_BLUE_SHIFT_ARB.
            // WGL_ALPHA_BITS_EXT - Is the same as WGL_ALPHA_BITS_ARB.
            // WGL_ALPHA_SHIFT_EXT - Is the same as WGL_ALPHA_SHIFT_ARB.
            // WGL_ACCUM_BITS_EXT - Is the same as WGL_ACCUM_BITS_ARB.
            // WGL_ACCUM_RED_BITS_EXT - Is the same as WGL_ACCUM_RED_BITS_ARB.
            // WGL_ACCUM_GREEN_BITS_EXT - Is the same as WGL_ACCUM_GREEN_BITS_ARB.
            // WGL_ACCUM_BLUE_BITS_EXT - Is the same as WGL_ACCUM_BLUE_BITS_ARB.
            // WGL_ACCUM_ALPHA_BITS_EXT - Is the same as WGL_ACCUM_ALPHA_BITS_ARB.
            // WGL_DEPTH_BITS_EXT - Is the same as WGL_DEPTH_BITS_ARB.
            // WGL_STENCIL_BITS_EXT - Is the same as WGL_STENCIL_BITS_ARB.
            // WGL_AUX_BUFFERS_EXT - Is the same as WGL_AUX_BUFFERS_ARB.
            // WGL_NO_ACCELERATION_EXT - Is the same as WGL_NO_ACCELERATION_ARB.
            // WGL_GENERIC_ACCELERATION_EXT - Is the same as WGL_GENERIC_ACCELERATION_ARB.
            // WGL_FULL_ACCELERATION_EXT - Is the same as WGL_FULL_ACCELERATION_ARB.
            // WGL_SWAP_EXCHANGE_EXT - Is the same as WGL_SWAP_EXCHANGE_ARB.
            // WGL_SWAP_COPY_EXT - Is the same as WGL_SWAP_COPY_ARB.
            // WGL_SWAP_UNDEFINED_EXT - Is the same as WGL_SWAP_UNDEFINED_ARB.
            // WGL_TYPE_RGBA_EXT - Is the same as WGL_TYPE_RGBA_ARB.
            // WGL_TYPE_COLORINDEX_EXT - Is the same as WGL_TYPE_COLORINDEX_ARB.

            // WGL_EXT_pbuffer:
            // WGL_DRAW_TO_PBUFFER_EXT - Is the same as WGL_DRAW_TO_PBUFFER_ARB.
            // WGL_MAX_PBUFFER_PIXELS_EXT - Is the same as WGL_MAX_PBUFFER_PIXELS_ARB.
            // WGL_MAX_PBUFFER_WIDTH_EXT - Is the same as WGL_MAX_PBUFFER_WIDTH_ARB.
            // WGL_MAX_PBUFFER_HEIGHT_EXT - Is the same as WGL_MAX_PBUFFER_HEIGHT_ARB.
            // WGL_PBUFFER_LARGEST_EXT - Is the same as WGL_PBUFFER_LARGEST_ARB.
            // WGL_PBUFFER_WIDTH_EXT - Is the same as WGL_PBUFFER_WIDTH_ARB.
            // WGL_PBUFFER_HEIGHT_EXT - Is the same as WGL_PBUFFER_HEIGHT_ARB.
            AP_GL_ENUM_TOSTRING_CASE(WGL_OPTIMAL_PBUFFER_WIDTH_EXT);
            AP_GL_ENUM_TOSTRING_CASE(WGL_OPTIMAL_PBUFFER_HEIGHT_EXT);

            // WGL_EXT_depth_float:
            AP_GL_ENUM_TOSTRING_CASE(WGL_DEPTH_FLOAT_EXT);

            // WGL_3DFX_multisample:
            AP_GL_ENUM_TOSTRING_CASE(WGL_SAMPLE_BUFFERS_3DFX);
            AP_GL_ENUM_TOSTRING_CASE(WGL_SAMPLES_3DFX);

            // WGL_EXT_multisample:
            // WGL_SAMPLE_BUFFERS_EXT - Is the same as WGL_SAMPLE_BUFFERS_ARB.
            // WGL_SAMPLES_EXT - Is the same as WGL_SAMPLES_EXT.

            // WGL_I3D_digital_video_control:
            AP_GL_ENUM_TOSTRING_CASE(WGL_DIGITAL_VIDEO_CURSOR_ALPHA_FRAMEBUFFER_I3D);
            AP_GL_ENUM_TOSTRING_CASE(WGL_DIGITAL_VIDEO_CURSOR_ALPHA_VALUE_I3D);
            AP_GL_ENUM_TOSTRING_CASE(WGL_DIGITAL_VIDEO_CURSOR_INCLUDED_I3D);
            AP_GL_ENUM_TOSTRING_CASE(WGL_DIGITAL_VIDEO_GAMMA_CORRECTED_I3D);

            // WGL_I3D_gamma:
            AP_GL_ENUM_TOSTRING_CASE(WGL_GAMMA_TABLE_SIZE_I3D);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GAMMA_EXCLUDE_DESKTOP_I3D);

            // WGL_I3D_genlock:
            AP_GL_ENUM_TOSTRING_CASE(WGL_GENLOCK_SOURCE_MULTIVIEW_I3D);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GENLOCK_SOURCE_EXTENAL_SYNC_I3D);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GENLOCK_SOURCE_EXTENAL_FIELD_I3D);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GENLOCK_SOURCE_EXTENAL_TTL_I3D);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GENLOCK_SOURCE_DIGITAL_SYNC_I3D);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GENLOCK_SOURCE_DIGITAL_FIELD_I3D);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GENLOCK_SOURCE_EDGE_FALLING_I3D);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GENLOCK_SOURCE_EDGE_RISING_I3D);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GENLOCK_SOURCE_EDGE_BOTH_I3D);

            // WGL_I3D_image_buffer:
            /*
                Notice: The below enums values are 1 and 2 !
                        If we will support the WGL_I3D_image_buffer extension, we will
                        have to translate them to strings using a dedicated function.
            AP_GL_ENUM_TOSTRING_CASE(WGL_IMAGE_BUFFER_MIN_ACCESS_I3D);
            AP_GL_ENUM_TOSTRING_CASE(WGL_IMAGE_BUFFER_LOCK_I3D);
            */

            // WGL_NV_render_depth_texture:
            AP_GL_ENUM_TOSTRING_CASE(WGL_BIND_TO_TEXTURE_DEPTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_BIND_TO_TEXTURE_RECTANGLE_DEPTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_DEPTH_TEXTURE_FORMAT_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_DEPTH_COMPONENT_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_DEPTH_COMPONENT_NV);

            // WGL_NV_render_texture_rectangle:
            AP_GL_ENUM_TOSTRING_CASE(WGL_BIND_TO_TEXTURE_RECTANGLE_RGB_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_BIND_TO_TEXTURE_RECTANGLE_RGBA_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_RECTANGLE_NV);

            // WGL_ATI_pixel_format_float:
            // WGL_TYPE_RGBA_FLOAT_ATI is equal to WGL_TYPE_RGBA_FLOAT_ARB

            // WGL_NV_float_buffer
            AP_GL_ENUM_TOSTRING_CASE(WGL_FLOAT_COMPONENTS_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_R_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RG_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGB_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGBA_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_FLOAT_R_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_FLOAT_RG_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_FLOAT_RGB_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_TEXTURE_FLOAT_RGBA_NV);

            // WGL_3DL_stereo_control
            AP_GL_ENUM_TOSTRING_CASE(WGL_STEREO_EMITTER_ENABLE_3DL);
            AP_GL_ENUM_TOSTRING_CASE(WGL_STEREO_EMITTER_DISABLE_3DL);
            AP_GL_ENUM_TOSTRING_CASE(WGL_STEREO_POLARITY_NORMAL_3DL);
            AP_GL_ENUM_TOSTRING_CASE(WGL_STEREO_POLARITY_INVERT_3DL);

            // WGL_EXT_pixel_format_packed_float
            AP_GL_ENUM_TOSTRING_CASE(WGL_TYPE_RGBA_UNSIGNED_FLOAT_EXT);

            // WGL_EXT_framebuffer_sRGB
            AP_GL_ENUM_TOSTRING_CASE(WGL_FRAMEBUFFER_SRGB_CAPABLE_EXT);

            // WGL_NV_present_video
            AP_GL_ENUM_TOSTRING_CASE(WGL_NUM_VIDEO_SLOTS_NV);

            // WGL_NV_video_out
            AP_GL_ENUM_TOSTRING_CASE(WGL_BIND_TO_VIDEO_RGB_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_BIND_TO_VIDEO_RGBA_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_BIND_TO_VIDEO_RGB_AND_DEPTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_VIDEO_OUT_COLOR_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_VIDEO_OUT_ALPHA_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_VIDEO_OUT_DEPTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_VIDEO_OUT_COLOR_AND_ALPHA_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_VIDEO_OUT_COLOR_AND_DEPTH_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_VIDEO_OUT_FRAME);
            AP_GL_ENUM_TOSTRING_CASE(WGL_VIDEO_OUT_FIELD_1);
            AP_GL_ENUM_TOSTRING_CASE(WGL_VIDEO_OUT_FIELD_2);
            AP_GL_ENUM_TOSTRING_CASE(WGL_VIDEO_OUT_STACKED_FIELDS_1_2);
            AP_GL_ENUM_TOSTRING_CASE(WGL_VIDEO_OUT_STACKED_FIELDS_2_1);

            // WGL_NV_swap_group

            // WGL_NV_gpu_affinity
            AP_GL_ENUM_TOSTRING_CASE(WGL_ERROR_INCOMPATIBLE_AFFINITY_MASKS_NV);
            AP_GL_ENUM_TOSTRING_CASE(WGL_ERROR_MISSING_AFFINITY_MASK_NV);

            // WGL_AMD_gpu_association
            // WGL_GPU_VENDOR_AMD is the same as GL_VENDOR
            // WGL_GPU_RENDERER_STRING_AMD is the same as GL_RENDERER
            // WGL_GPU_OPENGL_VERSION_STRING_AMD is the same as GL_VERSION
            AP_GL_ENUM_TOSTRING_CASE(WGL_GPU_FASTEST_TARGET_GPUS_AMD);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GPU_RAM_AMD);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GPU_CLOCK_AMD);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GPU_NUM_PIPES_AMD);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GPU_NUM_SIMD_AMD);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GPU_NUM_RB_AMD);
            AP_GL_ENUM_TOSTRING_CASE(WGL_GPU_NUM_SPI_AMD);

#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        default:
            retVal = false;
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        checkOpenGLOtherExtensionsEnum
// Description: Check if this is one of the other companies enumerations
// Arguments:   GLenum enumValue
//              gtString& valueString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/7/2011
// ---------------------------------------------------------------------------
bool checkOpenGLOtherExtensionsEnum(GLenum enumValue, gtString& valueString)
{
    bool retVal = true;

    switch (enumValue)
    {
            // GL_INGR_palette_buffer

            // GL_IBM_rasterpos_clip:
            AP_GL_ENUM_TOSTRING_CASE(GL_RASTER_POSITION_UNCLIPPED_IBM);

            // GL_WIN_phong_shading:
            AP_GL_ENUM_TOSTRING_CASE(GL_PHONG_WIN);
            AP_GL_ENUM_TOSTRING_CASE(GL_PHONG_HINT_WIN);

            // GL_WIN_specular_fog:
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_SPECULAR_TEXTURE_WIN);

            // GL_PGI_vertex_hints:
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_DATA_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_CONSISTENT_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_MATERIAL_SIDE_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VERTEX_HINT_PGI);

            // GL_PGI_misc_hints
            AP_GL_ENUM_TOSTRING_CASE(GL_PREFER_DOUBLEBUFFER_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CONSERVE_MEMORY_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_RECLAIM_MEMORY_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_NATIVE_GRAPHICS_HANDLE_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_NATIVE_GRAPHICS_BEGIN_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_NATIVE_GRAPHICS_END_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALWAYS_FAST_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALWAYS_SOFT_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALLOW_DRAW_OBJ_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALLOW_DRAW_WIN_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALLOW_DRAW_FRG_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALLOW_DRAW_MEM_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_STRICT_DEPTHFUNC_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_STRICT_LIGHTING_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_STRICT_SCISSOR_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_FULL_STIPPLE_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_NEAR_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_CLIP_FAR_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_WIDE_LINE_HINT_PGI);
            AP_GL_ENUM_TOSTRING_CASE(GL_BACK_NORMALS_HINT_PGI);

            // GL_INTEL_texture_scissor

            // GL_INTEL_parallel_arrays
            AP_GL_ENUM_TOSTRING_CASE(GL_PARALLEL_ARRAYS_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL);

            // GL_SUNX_constant_data
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_CONSTANT_DATA_SUNX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_CONSTANT_DATA_SUNX);

            // GL_SUN_global_alpha
            AP_GL_ENUM_TOSTRING_CASE(GL_GLOBAL_ALPHA_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_GLOBAL_ALPHA_FACTOR_SUN);

            // GL_SUN_triangle_list
            // GL_RESTART_SUN - Is defined as 1 !
            // GL_REPLACE_MIDDLE_SUN - Is defined as 2 !:
            // GL_REPLACE_OLDEST_SUN - Is defined as 3 !
            AP_GL_ENUM_TOSTRING_CASE(GL_TRIANGLE_LIST_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_REPLACEMENT_CODE_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_REPLACEMENT_CODE_ARRAY_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_R1UI_V3F_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_R1UI_C4UB_V3F_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_R1UI_C3F_V3F_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_R1UI_N3F_V3F_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_R1UI_C4F_N3F_V3F_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_R1UI_T2F_V3F_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_R1UI_T2F_N3F_V3F_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_R1UI_T2F_C4F_N3F_V3F_SUN);

            // GL_SUN_vertex

            // GL_INGR_color_clamp
            AP_GL_ENUM_TOSTRING_CASE(GL_RED_MIN_CLAMP_INGR);
            AP_GL_ENUM_TOSTRING_CASE(GL_GREEN_MIN_CLAMP_INGR);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLUE_MIN_CLAMP_INGR);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA_MIN_CLAMP_INGR);
            AP_GL_ENUM_TOSTRING_CASE(GL_RED_MAX_CLAMP_INGR);
            AP_GL_ENUM_TOSTRING_CASE(GL_GREEN_MAX_CLAMP_INGR);
            AP_GL_ENUM_TOSTRING_CASE(GL_BLUE_MAX_CLAMP_INGR);
            AP_GL_ENUM_TOSTRING_CASE(GL_ALPHA_MAX_CLAMP_INGR);

            // GL_INGR_interlace_read
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERLACE_READ_INGR);

            // GL_SUN_convolution_border_modes
            AP_GL_ENUM_TOSTRING_CASE(GL_WRAP_BORDER_SUN);

            // GL_MESA_resize_buffers

            // GL_MESA_window_pos

            // GL_IBM_cull_vertex
            AP_GL_ENUM_TOSTRING_CASE(GL_CULL_VERTEX_IBM);

            // GL_IBM_multimode_draw_arrays

            // GL_IBM_vertex_array_lists
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_LIST_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMAL_ARRAY_LIST_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ARRAY_LIST_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_ARRAY_LIST_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COORD_ARRAY_LIST_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_EDGE_FLAG_ARRAY_LIST_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_COORDINATE_ARRAY_LIST_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_SECONDARY_COLOR_ARRAY_LIST_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_VERTEX_ARRAY_LIST_STRIDE_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_NORMAL_ARRAY_LIST_STRIDE_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_COLOR_ARRAY_LIST_STRIDE_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_INDEX_ARRAY_LIST_STRIDE_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_EDGE_FLAG_ARRAY_LIST_STRIDE_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM);
            AP_GL_ENUM_TOSTRING_CASE(GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM);

            // GL_3DFX_texture_compression_FXT1
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGB_FXT1_3DFX);
            AP_GL_ENUM_TOSTRING_CASE(GL_COMPRESSED_RGBA_FXT1_3DFX);

            // GL_3DFX_multisample
            AP_GL_ENUM_TOSTRING_CASE(GL_MULTISAMPLE_3DFX);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLE_BUFFERS_3DFX);
            AP_GL_ENUM_TOSTRING_CASE(GL_SAMPLES_3DFX);

            // GL_3DFX_tbuffer

            // GL_OML_interlace
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERLACE_OML);
            AP_GL_ENUM_TOSTRING_CASE(GL_INTERLACE_READ_OML);

            // GL_OML_subsample
            AP_GL_ENUM_TOSTRING_CASE(GL_FORMAT_SUBSAMPLE_24_24_OML);
            AP_GL_ENUM_TOSTRING_CASE(GL_FORMAT_SUBSAMPLE_244_244_OML);

            // GL_OML_resample
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_RESAMPLE_OML);
            AP_GL_ENUM_TOSTRING_CASE(GL_UNPACK_RESAMPLE_OML);
            AP_GL_ENUM_TOSTRING_CASE(GL_RESAMPLE_REPLICATE_OML);
            AP_GL_ENUM_TOSTRING_CASE(GL_RESAMPLE_ZERO_FILL_OML);
            AP_GL_ENUM_TOSTRING_CASE(GL_RESAMPLE_AVERAGE_OML);
            AP_GL_ENUM_TOSTRING_CASE(GL_RESAMPLE_DECIMATE_OML);

            // GL_SUN_mesh_array
            AP_GL_ENUM_TOSTRING_CASE(GL_QUAD_MESH_SUN);
            AP_GL_ENUM_TOSTRING_CASE(GL_TRIANGLE_MESH_SUN);

            // GL_SUN_slice_accum
            AP_GL_ENUM_TOSTRING_CASE(GL_SLICE_ACCUM_SUN);

            // GL_S3_s3tc
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB_S3TC);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGB4_S3TC);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA_S3TC);
            AP_GL_ENUM_TOSTRING_CASE(GL_RGBA4_S3TC);

            // GL_MESA_pack_invert
            AP_GL_ENUM_TOSTRING_CASE(GL_PACK_INVERT_MESA);

            // GL_MESA_ycbcr_texture
            // GL_UNSIGNED_SHORT_8_8_MESA - Is the same as GL_UNSIGNED_SHORT_8_8_APPLE
            // GL_UNSIGNED_SHORT_8_8_REV_MESA - Is the same as GL_UNSIGNED_SHORT_8_8_REV_APPLE
            AP_GL_ENUM_TOSTRING_CASE(GL_YCBCR_MESA);

            // GL_EXT_pixel_buffer_object
            // GL_PIXEL_PACK_BUFFER_EXT - Is the same as GL_PIXEL_PACK_BUFFER
            // GL_PIXEL_UNPACK_BUFFER_EXT - Is the same as GL_PIXEL_UNPACK_BUFFER
            // GL_PIXEL_PACK_BUFFER_BINDING_EXT - Is the same as GL_PIXEL_PACK_BUFFER_BINDING
            // GL_PIXEL_UNPACK_BUFFER_BINDING_EXT - Is the same as GL_PIXEL_UNPACK_BUFFER_BINDING

            // GL_MESAX_texture_stack:
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_1D_STACK_MESAX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_2D_STACK_MESAX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_1D_STACK_MESAX);
            AP_GL_ENUM_TOSTRING_CASE(GL_PROXY_TEXTURE_2D_STACK_MESAX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_1D_STACK_BINDING_MESAX);
            AP_GL_ENUM_TOSTRING_CASE(GL_TEXTURE_2D_STACK_BINDING_MESAX);

            // GL_INTEL_performance_query:
            // AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_SINGLE_CONTEXT_INTEL);
            // AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_GLOBAL_CONTEXT_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_WAIT_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_FLUSH_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_DONOT_FLUSH_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_COUNTER_EVENT_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_COUNTER_DURATION_NORM_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_COUNTER_DURATION_RAW_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_COUNTER_THROUGHPUT_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_COUNTER_RAW_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_COUNTER_TIMESTAMP_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_COUNTER_DATA_UINT32_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_COUNTER_DATA_UINT64_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_COUNTER_DATA_FLOAT_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_COUNTER_DATA_DOUBLE_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_COUNTER_DATA_BOOL32_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_QUERY_NAME_LENGTH_MAX_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_COUNTER_NAME_LENGTH_MAX_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_COUNTER_DESC_LENGTH_MAX_INTEL);
            AP_GL_ENUM_TOSTRING_CASE(GL_PERFQUERY_GPA_EXTENDED_COUNTERS_INTEL);

            // GL_OVR_multiview:
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_OVR);
            AP_GL_ENUM_TOSTRING_CASE(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_OVR);
            AP_GL_ENUM_TOSTRING_CASE(GL_MAX_VIEWS_OVR);

        // Gremedy special enums:
        case GL_GREMEDY_COPIED_FROM_BUFFER:
        {
            // Represents the texture's format of a texture that was copied from a buffer
            // (example: using glCopyTexImage2D). We use this enum since we cannot always
            // express buffer formats using GLenum values.
            valueString = L"Copied from buffer";
        }

        break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}

