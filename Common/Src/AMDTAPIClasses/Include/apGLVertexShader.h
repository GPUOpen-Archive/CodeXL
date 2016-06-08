//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLVertexShader.h
///
//==================================================================================

//------------------------------ apGLVertexShader.h ------------------------------

#ifndef __APGLVERTEXSHADER
#define __APGLVERTEXSHADER

// Local:
#include <AMDTAPIClasses/Include/apGLShaderObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLVertexShader : public apGLShaderObject
// General Description:
//   Represents an OpenGL vertex shader.
//
// Author:  AMD Developer Tools Team
// Creation Date:        04/04/2005
// ----------------------------------------------------------------------------------
class AP_API apGLVertexShader : public apGLShaderObject
{
public:
    // Self functions:
    apGLVertexShader(GLuint shaderName = 0,  apGLShadingObjectType shaderType = AP_GL_2_0_SHADING_OBJECT);
    apGLVertexShader(const apGLVertexShader& other);
    virtual ~apGLVertexShader();
    apGLVertexShader& operator=(const apGLVertexShader& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:

};


#endif  // __APGLVERTEXSHADER
