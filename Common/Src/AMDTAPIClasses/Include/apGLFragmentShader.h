//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLFragmentShader.h
///
//==================================================================================

//------------------------------ apGLFragmentShader.h ------------------------------

#ifndef __APGLFRAGMENTSHADER
#define __APGLFRAGMENTSHADER

// Local:
#include <AMDTAPIClasses/Include/apGLShaderObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLFragmentShader : public apGLShaderObject
// General Description:
//   Represnets an OpenGL fragment shader.
//
// Author:  AMD Developer Tools Team
// Creation Date:        04/04/2005
// ----------------------------------------------------------------------------------
class AP_API apGLFragmentShader : public apGLShaderObject
{
public:
    // Self functions:
    apGLFragmentShader(GLuint shaderName = 0,  apGLShadingObjectType shaderType = AP_GL_2_0_SHADING_OBJECT);
    apGLFragmentShader(const apGLFragmentShader& other);
    virtual ~apGLFragmentShader();
    apGLFragmentShader& operator=(const apGLFragmentShader& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:

};


#endif  // __APGLFRAGMENTSHADER
