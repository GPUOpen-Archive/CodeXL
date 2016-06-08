//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLenumParameter.h
///
//==================================================================================

//------------------------------ apGLenumParameter.h ------------------------------

#ifndef __APGLENUMPARAMETER
#define __APGLENUMPARAMETER

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>

// Gremedy special GLEnum values:
#define GL_GREMEDY_COPIED_FROM_BUFFER 0x2DC6C0

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLenumParameter : public apOpenGLParameter
// General Description:
//  Represents a GLenum parameter (OpenGL enumeration).
// Author:  AMD Developer Tools Team
// Creation Date:        28/4/2004
// ----------------------------------------------------------------------------------
class AP_API apGLenumParameter : public apOpenGLParameter
{
public:
    // Self functions:
    apGLenumParameter(GLenum value = 0) : _value(value) {};
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


bool checkOpenGL1Enum(GLenum enumValue, gtString& valueString);
bool checkOpenGL2Enum(GLenum enumValue, gtString& valueString);
bool checkOpenGL3Enum(GLenum enumValue, gtString& valueString);
bool checkOpenGL4Enum(GLenum enumValue, gtString& valueString);
bool checkOpenGLESEnum(GLenum enumValue, gtString& valueString);
bool checkOpenGLARBExtensionsEnum(GLenum enumValue, gtString& valueString);
bool checkOpenGLEXTExtensionsEnum(GLenum enumValue, gtString& valueString);
bool checkOpenGLAMDExtensionsEnum(GLenum enumValue, gtString& valueString);
bool checkOpenGLNVExtensionsEnum(GLenum enumValue, gtString& valueString);
bool checkOpenGLIntelExtensionsEnum(GLenum enumValue, gtString& valueString);
bool checkOpenGLAppleExtensionsEnum(GLenum enumValue, gtString& valueString);
bool checkOpenGLHPExtensionsEnum(GLenum enumValue, gtString& valueString);
bool checkOpenGLSGISExtensionsEnum(GLenum enumValue, gtString& valueString);
bool checkOpenGLOtherExtensionsEnum(GLenum enumValue, gtString& valueString);
bool checkWGLExtensionsEnum(GLenum enumValue, gtString& valueString);

bool AP_API apGLenumValueToString(GLenum enumValue, gtString& valueString);


#endif  // __APGLENUMPARAMETER
