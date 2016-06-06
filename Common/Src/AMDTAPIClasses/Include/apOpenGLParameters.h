//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenGLParameters.h
///
//==================================================================================

//------------------------------ apOpenGLParameters.h ------------------------------

#ifndef __APOPENGLPARAMETERS
#define __APOPENGLPARAMETERS

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apBasicParameters.h>


// -------------------  OpenGL parameters ---------------------

// Uri, 5/4/15 - This class is incorrectly named and should be called "API numeric parameter" and allowed
// something that accepts OpenCL. TBH next version.
// ----------------------------------------------------------------------------------
// Class Name:           AP_API apOpenGLParameter : public apParameter
// General Description: Base class for OpenGL parameter types.
// Author:  AMD Developer Tools Team
// Creation Date:        23/1/2005
// ----------------------------------------------------------------------------------
class AP_API apOpenGLParameter : public apParameter
{
public:
    // Overrides apParameter:
    virtual bool isOpenGLParameter() const;

    // Must be implemented by sub-classes:
    virtual void setValueFromInt(GLint value) = 0;
    virtual void setValueFromFloat(GLfloat value) = 0;
};

// ----------------------------------------------------------------------------------
// Class Name:           apPixelValueParameter : public apParameter
// General Description:  Base class for all parameters that represent pixel value.
//                       Is mainly used for converting textures and buffers raw data into an image.
// Author:  AMD Developer Tools Team
// Creation Date:        27/3/2011
// NOTICE:               This parameter used to be called apOpenGLValueParameter, but its name
//                       and location was changed since we've started using cl parameters as pixel
//                       values
// ----------------------------------------------------------------------------------
class AP_API apPixelValueParameter : public apOpenGLParameter
{
public:
    // Constructor:
    apPixelValueParameter() : _isDefaultMulitiplier(true) {};

public:
    // Convert the raw data value into a pixel value (unsigned byte)
    virtual char asPixelValue() const = 0;

    // Set the value multiplier manually
    virtual void setValueMultiplier(double valueMultiplier) { _valueMultiplier = valueMultiplier; _isDefaultMulitiplier = false; };

    // Shifts and masks the bits of the Parameter value
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight) = 0;

    // Set value from double
    virtual void setValueFromDouble(double value) = 0;

    // Return parameter value as double
    virtual double valueAsDouble() = 0;

protected:
    // The value multiplier
    double _valueMultiplier;

    // Should we use the default multiplier or the one assigned manually?
    bool _isDefaultMulitiplier;
};

// -------------------  OpenGL Value parameters ---------------------

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLPrimitiveTypeParameter : public apOpenGLParameter
// General Description:
//  Represents OpenGL primitive type: GL_POINTS, GL_LINES, etc.
//
//  Primitive types in OpenGL are represented using GLenum parameter, but
//  unfortunately, the primitive type enumeration values are also used by extensions
//  enumerations. This force us to have a separate class for primitive types.
//  (See comment at apGLenumParameter::valueAsString - Implementation notes)
//
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLPrimitiveTypeParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLPrimitiveTypeParameter(GLenum value = 0) : _value(value) {};
    GLenum value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    // The parameter value:
    GLenum _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLbooleanParameter : public apOpenGLParameter
// General Description: Represents a GLboolean parameter (OpenGL bool).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLbooleanParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLbooleanParameter(GLboolean value = GL_FALSE) : _value(value) {};
    GLboolean value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    // The parameter value:
    GLboolean _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLbitfieldParameter : public apOpenGLParameter
// General Description: Represents a GLbitfield parameter (OpenGL bit field).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLbitfieldParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLbitfieldParameter(GLbitfield value = 0) : _value(value) {};
    GLbitfield value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    // The parameter value:
    GLbitfield _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLclearBitfieldParameter : public apOpenGLParameter
// General Description: Represents a GLbitfield parameter for glClear.
// Author:  AMD Developer Tools Team
// Creation Date:        16/9/2007
// ----------------------------------------------------------------------------------
class AP_API apGLclearBitfieldParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLclearBitfieldParameter(GLbitfield value = 0) : _value(value) {};
    GLbitfield value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    // The parameter value:
    GLbitfield _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLbyteParameter : public apOpenGLParameter
// General Description: Represents a GLbyte parameter (OpenGL 1-byte signed).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLbyteParameter : public apPixelValueParameter
{
public:
    // Self functions:
    apGLbyteParameter(GLbyte value = 0) : _value(value) { };
    GLbyte value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    GLbyte _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLshortParameter : public apOpenGLParameter
// General Description: Represents a GLshort parameter (OpenGL 2-bytes signed).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLshortParameter : public apPixelValueParameter
{
public:
    // Self functions:
    apGLshortParameter(GLshort value = 0) : _value(value) { };
    GLshort value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    GLshort _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLintParameter : public apOpenGLParameter
// General Description: Represents a GLint parameter (OpenGL 4-bytes signed).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLintParameter : public apPixelValueParameter
{
public:
    // Self functions:
    apGLintParameter(GLint value = 0) : _value(value) { };
    GLint value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    GLint _value;
};




// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLubyteParameter : public apOpenGLParameter
// General Description: Represents a GLubyte parameter (OpenGL 1-byte unsigned).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLubyteParameter : public apPixelValueParameter
{
public:
    // Self functions:
    apGLubyteParameter(GLubyte value = 0) : _value(value) { };
    GLubyte value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    GLubyte _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLushortParameter : public apOpenGLParameter
// General Description: Represents a GLushort parameter (OpenGL 2-bytes unsigned).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLushortParameter : public apPixelValueParameter
{
public:
    // Self functions:
    apGLushortParameter(GLushort value = 0) : _value(value) { };
    GLushort value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    GLushort _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLuintParameter : public apOpenGLParameter
// General Description: Represents a GLuint parameter (OpenGL 4-bytes unsigned).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLuintParameter : public apPixelValueParameter
{
public:
    // Self functions:
    apGLuintParameter(GLuint value = 0) : _value(value) { };
    GLuint value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    GLuint _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLint64Parameter : public apOpenGLParameter
// General Description: Represents a GLint64 parameter (OpenGL 8-bytes signed).
// Author:  AMD Developer Tools Team
// Creation Date:        15/10/2009
// ----------------------------------------------------------------------------------
class AP_API apGLint64Parameter : public apPixelValueParameter
{
public:
    // Self functions:
    apGLint64Parameter(GLint64 value = 0) : _value(value) { };
    GLint64 value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    GLint64 _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLuint64Parameter : public apOpenGLParameter
// General Description: Represents a GLuint64 parameter (OpenGL 8-bytes unsigned).
// Author:  AMD Developer Tools Team
// Creation Date:        27/10/2009
// ----------------------------------------------------------------------------------
class AP_API apGLuint64Parameter : public apPixelValueParameter
{
public:
    // Self functions:
    apGLuint64Parameter(GLuint64 value = 0) : _value(value) { };
    GLuint64 value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

protected:

    // The parameter value:
    GLuint64 _value;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apGLuint64AddressParameter : public apOpenGLParameter
// General Description: Represents a GLuint64 parameter when
// Author:  AMD Developer Tools Team
// Creation Date:       29/11/2010
// ----------------------------------------------------------------------------------
class AP_API apGLuint64AddressParameter : public apGLuint64Parameter
{
public:
    // Self functions:
    apGLuint64AddressParameter(GLuint64 value = 0) : apGLuint64Parameter(value) { };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual void valueAsString(gtString& valueString) const;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLsizeiParameter : public apOpenGLParameter
// General Description: Represents a GLsizei parameter (OpenGL 4-bytes signed).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLsizeiParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLsizeiParameter(GLsizei value = 0) : _value(value) {};
    GLsizei value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    // The parameter value:
    GLsizei _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLfloatParameter : public apOpenGLParameter
// General Description: Represents a GLfloat parameter (OpenGL single precision float).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLfloatParameter : public apPixelValueParameter
{
public:
    // Self functions:
    apGLfloatParameter(GLfloat value = 0.0f) : _value(value) { };
    GLfloat value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    GLfloat _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLclampfParameter : public apOpenGLParameter
// General Description: Represents a GLclampf parameter (OpenGL single precision float in [0,1]).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLclampfParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLclampfParameter(GLclampf value = 0.0f) : _value(value) {};
    GLclampf value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    // The parameter value:
    GLclampf _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLsyncParameter : public apOpenGLParameter
// General Description: Represents a GLSync parameter.
// Author:  AMD Developer Tools Team
// Creation Date:        27/10/2009
// ----------------------------------------------------------------------------------
class AP_API apGLsyncParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLsyncParameter(oaGLSyncHandle value = OA_GL_NULL_HANDLE);
    oaGLSyncHandle value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    // The parameter value:
    oaGLSyncHandle _value;

    // Is the debugged app 64-bit?
    bool _is64BitPointer;
};



// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLdoubleParameter : public apPixelValueParameter
// General Description: Represents a GLdouble parameter (OpenGL double precision float).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLdoubleParameter : public apPixelValueParameter
{
public:
    // Self functions:
    apGLdoubleParameter(GLdouble value = 0.0f) : _value(value) {};
    GLdouble value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

    // Overrides apPixelValueParameter:
    virtual char asPixelValue() const;
    virtual void shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight);
    virtual void setValueFromDouble(double value);
    virtual double valueAsDouble();

private:
    // The parameter value:
    GLdouble _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLclampdParameter : public apOpenGLParameter
// General Description: Represents a GLclampd parameter (OpenGL double precision float in [0,1]).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLclampdParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLclampdParameter(GLclampd value = 0.0f) : _value(value) {};
    GLclampd value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    // The parameter value:
    GLclampd _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLintptrParameter : public apOpenGLParameter
// General Description:
//   Represents a GLintptr parameter (OpenGL type for representing large memory offsets).
// Author:  AMD Developer Tools Team
// Creation Date:        17/9/2004
// ----------------------------------------------------------------------------------
class AP_API apGLintptrParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLintptrParameter(GLintptr value = 0) : _value(value) {};
    GLintptr value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueHexAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    // The parameter value:
    GLintptr _value;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLsizeiptrParameter : public apOpenGLParameter
// General Description:
//   Represents a GLsizeiptr parameter (OpenGL type for specifying large sizes).
// Author:  AMD Developer Tools Team
// Creation Date:        17/9/2004
// ----------------------------------------------------------------------------------
class AP_API apGLsizeiptrParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLsizeiptrParameter(GLsizeiptr value = 0) : _value(value) {};
    GLsizeiptr value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    // The parameter value:
    GLsizeiptr _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLStringParameter : public apOpenGLParameter
// General Description:
//   Represents an OpenGL string parameter that does not use \0-terminated strings.
//   Instead, it uses a list of GLubytes with an explicit length.
//
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLStringParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLStringParameter() {};
    apGLStringParameter(GLsizei len, const GLvoid* pString);
    const gtASCIIString& value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    void setFromOpenGLString(GLsizei len, const GLvoid* pString);

private:
    // The parameter value:
    gtASCIIString _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLMultiStringParameter : public apOpenGLParameter
// General Description:
//   Represents an OpenGL multi string parameter: an array of strings.
//   This parameter type is used by OpenGL to load Shaders source codes, etc.
//
// Author:  AMD Developer Tools Team
// Creation Date:        29/3/2005
// ----------------------------------------------------------------------------------
class AP_API apGLMultiStringParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLMultiStringParameter() {};
    apGLMultiStringParameter(GLsizei nstrings, const GLcharARB* const* strings, const GLint* lengths);
    const gtVector<gtASCIIString>& value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual void valueAsASCIIString(gtASCIIString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    void setGLStrings(GLsizei nstrings, const GLcharARB* const* strings, const GLint* lengths);

private:
    // The parameter value:
    gtVector<gtASCIIString> _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apAssociatedTextureNamesPseudoParameter : public apPseudoParameter
// General Description:
//   Holds the OpenGL names of a textures that is associated with this parameter function call.
// Author:  AMD Developer Tools Team
// Creation Date:        6/1/2005
// ----------------------------------------------------------------------------------
class AP_API apAssociatedTextureNamesPseudoParameter : public apPseudoParameter
{
public:
    // Self functions:
    apAssociatedTextureNamesPseudoParameter(GLuint associatedTextureName = 0);
    const gtVector<GLuint>& associatedTextureNames() const { return _associatedTextureNames; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    void initialize();

private:
    // The associated texture names:
    gtVector<GLuint> _associatedTextureNames;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apAssociatedProgramNamePseudoParameter : public apPseudoParameter
// General Description:
//   Holds the OpenGL name of a program object that is associated with this parameter function call.
// Author:  AMD Developer Tools Team
// Creation Date:        19/05/2005
// ----------------------------------------------------------------------------------
class AP_API apAssociatedProgramNamePseudoParameter : public apPseudoParameter
{
public:
    // Self functions:
    apAssociatedProgramNamePseudoParameter(GLuint associatedProgramName = 0);
    GLuint associatedProgramName() const { return _associatedProgramName; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The associated program name:
    GLuint _associatedProgramName;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apAssociatedShaderNamePseudoParameter : public apPseudoParameter
// General Description:
//   Holds the OpenGL names of a shader object that is associated with this parameter function call.
// Author:  AMD Developer Tools Team
// Creation Date:        19/05/2005
// ----------------------------------------------------------------------------------
class AP_API apAssociatedShaderNamePseudoParameter : public apPseudoParameter
{
public:
    // Self functions:
    apAssociatedShaderNamePseudoParameter(GLuint associatedShaderName = 0);
    GLuint associatedShaderName() const { return _associatedShaderName; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

private:
    // The associated shader name:
    GLuint _associatedShaderName;
};




// -------------------  OpenGL ES parameters ---------------------

// OpenGL ES is currently supported only on Windows and Mac:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLfixedParameter : public apOpenGLParameter
// General Description:
//   A parameter that represents the OpenGL ES GLfixed data type.
//   This data type represents float numbers as integer numbers. This type is useful
//   when working on embedded devices that does not have a floating point unit.
//
//   OpenGL ES GLfixed uses two's complement s15.16 representation:
//   - 1 bit - sign.
//   - 15 bits - integer part.
//   - 16 bits - fraction part.
//
//   For more details see:
//   - "Coping with Fixed Point / Mik BRY CEO mbry@apoje.com".
//      www.khronos.org/devu/library/coping_with_fixed_point-Bry.pdf
//
//   - GL_OES_fixed_point extension:
//     http://oss.sgi.com/projects/ogl-sample/registry/OES/fixed_point.txt
//
// Author:  AMD Developer Tools Team
// Creation Date:        1/11/2005
// ----------------------------------------------------------------------------------
class AP_API apGLfixedParameter : public apOpenGLParameter
{
public:
    // Data types translation functions:
    static GLfixed intToFixed(GLint intNum);
    static GLint fixedToInt(GLfixed fixedNum);
    static GLfixed floatToFixed(GLfloat floatNum);
    static GLfloat fixedToFloat(GLfixed fixedNum);

public:
    // Self functions:
    apGLfixedParameter(GLfixed value = 0) : _value(value) {};
    GLfixed value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    // The parameter value:
    GLfixed _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLclampxParameter : public apOpenGLParameter
// General Description:
//   A parameter that represents the OpenGL ES GLclampx data type.
//   GLclampx is similar to GLfixed, except that it is clamped to the [0,1] range.
//   See apGLfixedParameter documentation for more details.
//
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLclampxParameter : public apOpenGLParameter
{
public:
    // Data types translation functions:
    static GLclampx intToClampx(GLint intNum);
    static GLint clampxToInt(GLclampx clampxNum);
    static GLclampx floatToClampx(GLfloat floatNum);
    static GLfloat clampxToFloat(GLclampx clampxNum);
public:
    // Self functions:
    apGLclampxParameter(GLclampx value = 0.0f) : _value(value) {};
    GLclampx value() const { return _value; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apParameter:
    virtual void readValueFromArgumentsList(va_list& pArgumentList);
    virtual void readValueFromPointer(void* pValue);
    virtual gtSizeType sizeofData();
    virtual void valueAsString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;

    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    // The parameter value:
    GLclampx _value;
};

#endif // ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

// Parameters that have their own .h and .cpp files:
#include <AMDTAPIClasses/Include/apGLenumParameter.h>

AP_API apOpenGLParameter* apCreateOpenGLParameterObject(osTransferableObjectType parameterType);

#endif  // __APOPENGLPARAMETERS
