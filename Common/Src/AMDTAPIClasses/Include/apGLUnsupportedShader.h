//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLUnsupportedShader.h
///
//==================================================================================

//------------------------------ apGLUnsupportedShader.h ------------------------------

#ifndef __APGLUNSUPPORTEDSHADER
#define __APGLUNSUPPORTEDSHADER

// Local:
#include <AMDTAPIClasses/Include/apGLShaderObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLUnsupportedShader : public apGLShaderObject
// General Description:
//   Represnets an OpenGL shader of an unknown or unsupported type.
//
// Author:  AMD Developer Tools Team
// Creation Date:        03/06/2013
// ----------------------------------------------------------------------------------
class AP_API apGLUnsupportedShader : public apGLShaderObject
{
public:
    // Self functions:
    apGLUnsupportedShader(GLuint shaderName = 0,  apGLShadingObjectType shaderType = AP_GL_2_0_SHADING_OBJECT);
    apGLUnsupportedShader(const apGLUnsupportedShader& other);
    virtual ~apGLUnsupportedShader();
    apGLUnsupportedShader& operator=(const apGLUnsupportedShader& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
};


#endif  // __APGLUNSUPPORTEDSHADER
