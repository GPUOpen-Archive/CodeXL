//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLFragmentShader.cpp
///
//==================================================================================

//------------------------------ apGLFragmentShader.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apGLFragmentShader.h>


// ---------------------------------------------------------------------------
// Name:        apGLFragmentShader::apGLFragmentShader
// Description: Constructor
// Arguments:   shaderName - The fragment shader OpenGL name.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLFragmentShader::apGLFragmentShader(GLuint shaderName, apGLShadingObjectType shaderType)
    : apGLShaderObject(shaderName, shaderType)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLFragmentShader::apGLFragmentShader
// Description: Copy constructor
// Arguments:   other - The other fragment shader from which I am initialized.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLFragmentShader::apGLFragmentShader(const apGLFragmentShader& other)
    : apGLShaderObject(other)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLFragmentShader::~apGLFragmentShader
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLFragmentShader::~apGLFragmentShader()
{
}


// ---------------------------------------------------------------------------
// Name:        apGLFragmentShader::operator=
// Description: Copy other content into self.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLFragmentShader& apGLFragmentShader::operator=(const apGLFragmentShader& other)
{
    // Copy the shader object data:
    apGLShaderObject::operator=(other);

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apGLFragmentShader::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
osTransferableObjectType apGLFragmentShader::type() const
{
    return OS_TOBJ_ID_GL_FRAGMENT_SHADER;
}


// ---------------------------------------------------------------------------
// Name:        apGLFragmentShader::writeSelfIntoChannel
// Description: Writes my content into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
bool apGLFragmentShader::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    // Write the shader object data:
    retVal = apGLShaderObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLFragmentShader::readSelfFromChannel
// Description: Reads my content from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
bool apGLFragmentShader::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the shader object data:
    retVal = apGLShaderObject::readSelfFromChannel(ipcChannel);

    return retVal;
}
