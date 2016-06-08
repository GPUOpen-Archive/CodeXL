//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTessellationEvaluationShader.h
///
//==================================================================================

//------------------------------ apGLTessellationEvaluationShader.h ------------------------------

#ifndef __APGLTESSELLATIONEVALUATIONSHADER
#define __APGLTESSELLATIONEVALUATIONSHADER

// Local:
#include <AMDTAPIClasses/Include/apGLShaderObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLTessellationEvaluationShader : public apGLShaderObject
// General Description:
//   Represents an OpenGL tessellation evaluation shader.
//
// Author:  AMD Developer Tools Team
// Creation Date:        12/09/2013
// ----------------------------------------------------------------------------------
class AP_API apGLTessellationEvaluationShader : public apGLShaderObject
{
public:
    // Self functions:
    apGLTessellationEvaluationShader(GLuint shaderName = 0, apGLShadingObjectType shaderType = AP_GL_2_0_SHADING_OBJECT);
    apGLTessellationEvaluationShader(const apGLTessellationEvaluationShader& other);
    virtual ~apGLTessellationEvaluationShader();
    apGLTessellationEvaluationShader& operator=(const apGLTessellationEvaluationShader& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
};


#endif  // __APGLTESSELLATIONEVALUATIONSHADER
