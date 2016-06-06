//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTessellationControlShader.h
///
//==================================================================================

//------------------------------ apGLTessellationControlShader.h ------------------------------

#ifndef __APGLTESSELLATIONCONTROLSHADER
#define __APGLTESSELLATIONCONTROLSHADER

// Local:
#include <AMDTAPIClasses/Include/apGLShaderObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLTessellationControlShader : public apGLShaderObject
// General Description:
//   Represents an OpenGL tessellation evaluation shader.
//
// Author:  AMD Developer Tools Team
// Creation Date:        12/09/2013
// ----------------------------------------------------------------------------------
class AP_API apGLTessellationControlShader : public apGLShaderObject
{
public:
    // Self functions:
    apGLTessellationControlShader(GLuint shaderName = 0, apGLShadingObjectType shaderType = AP_GL_2_0_SHADING_OBJECT);
    apGLTessellationControlShader(const apGLTessellationControlShader& other);
    virtual ~apGLTessellationControlShader();
    apGLTessellationControlShader& operator=(const apGLTessellationControlShader& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
};


#endif  // __APGLTESSELLATIONCONTROLSHADER
