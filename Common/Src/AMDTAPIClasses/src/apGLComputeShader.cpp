//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLComputeShader.cpp
///
//==================================================================================

//------------------------------ apGLComputeShader.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apGLComputeShader.h>


// ---------------------------------------------------------------------------
// Name:        apGLComputeShader::apGLComputeShader
// Description: Constructor
// Arguments:   shaderName - The compute shader OpenGL name.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
apGLComputeShader::apGLComputeShader(GLuint shaderName, apGLShadingObjectType shaderType)
    : apGLShaderObject(shaderName, shaderType)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLComputeShader::apGLComputeShader
// Description: Copy constructor
// Arguments:   other - The other compute shader from which I am initialized.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
apGLComputeShader::apGLComputeShader(const apGLComputeShader& other)
    : apGLShaderObject(other)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLComputeShader::~apGLComputeShader
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
apGLComputeShader::~apGLComputeShader()
{
}


// ---------------------------------------------------------------------------
// Name:        apGLComputeShader::operator=
// Description: Copy other content into self.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
apGLComputeShader& apGLComputeShader::operator=(const apGLComputeShader& other)
{
    // Copy the shader object data:
    apGLShaderObject::operator=(other);

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apGLComputeShader::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apGLComputeShader::type() const
{
    return OS_TOBJ_ID_GL_COMPUTE_SHADER;
}


// ---------------------------------------------------------------------------
// Name:        apGLComputeShader::writeSelfIntoChannel
// Description: Writes my content into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
bool apGLComputeShader::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    // Write the shader object data:
    retVal = apGLShaderObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLComputeShader::readSelfFromChannel
// Description: Reads my content from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
bool apGLComputeShader::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the shader object data:
    retVal = apGLShaderObject::readSelfFromChannel(ipcChannel);

    return retVal;
}
