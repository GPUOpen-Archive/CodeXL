//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLGeometryShader.h
///
//==================================================================================

//------------------------------ apGLGeometryShader.h ------------------------------

#ifndef __APGLGEOMETRYSHADER_H
#define __APGLGEOMETRYSHADER_H




// Local:
#include <AMDTAPIClasses/Include/apGLShaderObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLGeometryShader : public apGLShaderObject
// General Description:
//   Represents an OpenGL geometry shader.
//
// Author:  AMD Developer Tools Team
// Creation Date:        3/4/2008
// ----------------------------------------------------------------------------------
class AP_API apGLGeometryShader : public apGLShaderObject
{
public:
    // Self functions:
    apGLGeometryShader(GLuint shaderName = 0,  apGLShadingObjectType shaderType = AP_GL_2_0_SHADING_OBJECT);
    apGLGeometryShader(const apGLGeometryShader& other);
    virtual ~apGLGeometryShader();
    apGLGeometryShader& operator=(const apGLGeometryShader& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:

};




#endif //__APGLGEOMETRYSHADER_H
