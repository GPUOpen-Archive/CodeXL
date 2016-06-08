//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLVertexShader.cpp
///
//==================================================================================

//------------------------------ apGLVertexShader.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apGLVertexShader.h>


// ---------------------------------------------------------------------------
// Name:        apGLVertexShader::apGLVertexShader
// Description: Constructor
// Arguments:   shaderName - The vertex shader OpenGL name.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLVertexShader::apGLVertexShader(GLuint shaderName, apGLShadingObjectType shaderType)
    : apGLShaderObject(shaderName, shaderType)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLVertexShader::apGLVertexShader
// Description: Copy constructor
// Arguments:   other - The other vertex shader from which I am initialized.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLVertexShader::apGLVertexShader(const apGLVertexShader& other)
    : apGLShaderObject(other)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLVertexShader::~apGLVertexShader
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLVertexShader::~apGLVertexShader()
{
}


// ---------------------------------------------------------------------------
// Name:        apGLVertexShader::operator=
// Description: Copy other content into self.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLVertexShader& apGLVertexShader::operator=(const apGLVertexShader& other)
{
    // Copy the shader object data:
    apGLShaderObject::operator=(other);

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apGLVertexShader::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
osTransferableObjectType apGLVertexShader::type() const
{
    return OS_TOBJ_ID_GL_VERTEX_SHADER;
}


// ---------------------------------------------------------------------------
// Name:        apGLVertexShader::writeSelfIntoChannel
// Description: Writes my content into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
bool apGLVertexShader::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    // Write the shader object data:
    retVal = apGLShaderObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLVertexShader::readSelfFromChannel
// Description: Reads my content from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
bool apGLVertexShader::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the shader object data:
    retVal = apGLShaderObject::readSelfFromChannel(ipcChannel);

    return retVal;
}
