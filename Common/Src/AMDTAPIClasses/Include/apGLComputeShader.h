//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLComputeShader.h
///
//==================================================================================

//------------------------------ apGLComputeShader.h ------------------------------

#ifndef __APGLCOMPUTESHADER
#define __APGLCOMPUTESHADER

// Local:
#include <AMDTAPIClasses/Include/apGLShaderObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLComputeShader : public apGLShaderObject
// General Description:
//   Represents an OpenGL compute shader.
//
// Author:  AMD Developer Tools Team
// Creation Date:        12/09/2013
// ----------------------------------------------------------------------------------
class AP_API apGLComputeShader : public apGLShaderObject
{
public:
    // Self functions:
    apGLComputeShader(GLuint shaderName = 0,  apGLShadingObjectType shaderType = AP_GL_2_0_SHADING_OBJECT);
    apGLComputeShader(const apGLComputeShader& other);
    virtual ~apGLComputeShader();
    apGLComputeShader& operator=(const apGLComputeShader& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
};


#endif  // __APGLCOMPUTESHADER
