//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLPixelInternalFormatParameter.h
///
//==================================================================================

//------------------------------ apGLPixelInternalFormatParameter.h ------------------------------

#ifndef __APGLPIXELINTERNALFORMATPARAMETER
#define __APGLPIXELINTERNALFORMATPARAMETER

// Local:
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLPixelInternalFormatParameter : public apOpenGLParameter
// General Description:
//  Represents an OpenGL Pixel Internal Format (GL_ALPHA, GL_ALPHA4, GL_ALPHA8, etc.).
// Author:  AMD Developer Tools Team
// Creation Date:        16/9/2007
// ----------------------------------------------------------------------------------
class AP_API apGLPixelInternalFormatParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLPixelInternalFormatParameter(GLenum value = 0) : _value(value) {};
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
    virtual void valueAsHexString(gtString& valueString) const;
    virtual bool compareToOther(const apParameter& other)const;


    // Overrides apOpenGLParameter:
    virtual void setValueFromInt(GLint value);
    virtual void setValueFromFloat(GLfloat value);

private:
    // The parameter value:
    GLenum _value;
};


#endif  // __APGLPIXELINTERNALFORMATPARAMETER
