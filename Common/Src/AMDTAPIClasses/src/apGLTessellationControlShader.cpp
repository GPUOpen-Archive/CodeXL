//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTessellationControlShader.cpp
///
//==================================================================================

//------------------------------ apGLTessellationControlShader.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apGLTessellationControlShader.h>


// ---------------------------------------------------------------------------
// Name:        apGLTessellationControlShader::apGLTessellationControlShader
// Description: Constructor
// Arguments:   shaderName - The tessellation control shader OpenGL name.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
apGLTessellationControlShader::apGLTessellationControlShader(GLuint shaderName, apGLShadingObjectType shaderType)
    : apGLShaderObject(shaderName, shaderType)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLTessellationControlShader::apGLTessellationControlShader
// Description: Copy constructor
// Arguments:   other - The other tessellation control shader from which I am initialized.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
apGLTessellationControlShader::apGLTessellationControlShader(const apGLTessellationControlShader& other)
    : apGLShaderObject(other)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLTessellationControlShader::~apGLTessellationControlShader
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
apGLTessellationControlShader::~apGLTessellationControlShader()
{
}


// ---------------------------------------------------------------------------
// Name:        apGLTessellationControlShader::operator=
// Description: Copy other content into self.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
apGLTessellationControlShader& apGLTessellationControlShader::operator=(const apGLTessellationControlShader& other)
{
    // Copy the shader object data:
    apGLShaderObject::operator=(other);

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apGLTessellationControlShader::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apGLTessellationControlShader::type() const
{
    return OS_TOBJ_ID_GL_TESSELLATION_CONTROL_SHADER;
}


// ---------------------------------------------------------------------------
// Name:        apGLTessellationControlShader::writeSelfIntoChannel
// Description: Writes my content into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
bool apGLTessellationControlShader::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    // Write the shader object data:
    retVal = apGLShaderObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTessellationControlShader::readSelfFromChannel
// Description: Reads my content from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
bool apGLTessellationControlShader::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the shader object data:
    retVal = apGLShaderObject::readSelfFromChannel(ipcChannel);

    return retVal;
}
