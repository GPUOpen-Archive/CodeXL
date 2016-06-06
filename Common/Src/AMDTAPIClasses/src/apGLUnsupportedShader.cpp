//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLUnsupportedShader.cpp
///
//==================================================================================

//------------------------------ apGLUnsupportedShader.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apGLUnsupportedShader.h>


// ---------------------------------------------------------------------------
// Name:        apGLUnsupportedShader::apGLUnsupportedShader
// Description: Constructor
// Arguments:   shaderName - The shader OpenGL name.
// Author:  AMD Developer Tools Team
// Date:        3/6/2013
// ---------------------------------------------------------------------------
apGLUnsupportedShader::apGLUnsupportedShader(GLuint shaderName, apGLShadingObjectType shaderType)
    : apGLShaderObject(shaderName, shaderType)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLUnsupportedShader::apGLUnsupportedShader
// Description: Copy constructor
// Arguments:   other - The other shader from which I am initialized.
// Author:  AMD Developer Tools Team
// Date:        3/6/2013
// ---------------------------------------------------------------------------
apGLUnsupportedShader::apGLUnsupportedShader(const apGLUnsupportedShader& other)
    : apGLShaderObject(other)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLUnsupportedShader::~apGLUnsupportedShader
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        3/6/2013
// ---------------------------------------------------------------------------
apGLUnsupportedShader::~apGLUnsupportedShader()
{
}


// ---------------------------------------------------------------------------
// Name:        apGLUnsupportedShader::operator=
// Description: Copy other content into self.
// Author:  AMD Developer Tools Team
// Date:        3/6/2013
// ---------------------------------------------------------------------------
apGLUnsupportedShader& apGLUnsupportedShader::operator=(const apGLUnsupportedShader& other)
{
    // Copy the shader object data:
    apGLShaderObject::operator=(other);

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apGLUnsupportedShader::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        3/6/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apGLUnsupportedShader::type() const
{
    return OS_TOBJ_ID_GL_UNSUPPORTED_SHADER;
}


// ---------------------------------------------------------------------------
// Name:        apGLUnsupportedShader::writeSelfIntoChannel
// Description: Writes my content into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        3/6/2013
// ---------------------------------------------------------------------------
bool apGLUnsupportedShader::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    // Write the shader object data:
    retVal = apGLShaderObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLUnsupportedShader::readSelfFromChannel
// Description: Reads my content from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        3/6/2013
// ---------------------------------------------------------------------------
bool apGLUnsupportedShader::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the shader object data:
    retVal = apGLShaderObject::readSelfFromChannel(ipcChannel);

    return retVal;
}
