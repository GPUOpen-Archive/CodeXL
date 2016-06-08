//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLGeometryShader.cpp
///
//==================================================================================

//------------------------------ apGLGeometryShader.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apGLGeometryShader.h>


// ---------------------------------------------------------------------------
// Name:        apGLGeometryShader::apGLGeometryShader
// Description: Constructor
// Arguments:   shaderName - The geometry shader OpenGL name.
// Author:  AMD Developer Tools Team
// Creation Date:        3/4/2008
// ---------------------------------------------------------------------------
apGLGeometryShader::apGLGeometryShader(GLuint shaderName, apGLShadingObjectType shaderType)
    : apGLShaderObject(shaderName, shaderType)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLGeometryShader::apGLGeometryShader
// Description: Copy constructor
// Arguments:   other - The other geometry shader from which I am initialized.
// Author:  AMD Developer Tools Team
// Creation Date:        3/4/2008
// ---------------------------------------------------------------------------
apGLGeometryShader::apGLGeometryShader(const apGLGeometryShader& other)
    : apGLShaderObject(other)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLGeometryShader::~apGLGeometryShader
// Description: Destructor
// Author:  AMD Developer Tools Team
// Creation Date:        3/4/2008
// ---------------------------------------------------------------------------
apGLGeometryShader::~apGLGeometryShader()
{
}


// ---------------------------------------------------------------------------
// Name:        apGLGeometryShader::operator=
// Description: Copy other content into self.
// Author:  AMD Developer Tools Team
// Creation Date:        3/4/2008
// ---------------------------------------------------------------------------
apGLGeometryShader& apGLGeometryShader::operator=(const apGLGeometryShader& other)
{
    // Copy the shader object data:
    apGLShaderObject::operator=(other);

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apGLGeometryShader::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Creation Date:        3/4/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apGLGeometryShader::type() const
{
    return OS_TOBJ_ID_GL_GEOMETRY_SHADER;
}


// ---------------------------------------------------------------------------
// Name:        apGLGeometryShader::writeSelfIntoChannel
// Description: Writes my content into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Creation Date:        3/4/2008
// ---------------------------------------------------------------------------
bool apGLGeometryShader::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    // Write the shader object data:
    retVal = apGLShaderObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLGeometryShader::readSelfFromChannel
// Description: Reads my content from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Creation Date:        3/4/2008
// ---------------------------------------------------------------------------
bool apGLGeometryShader::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the shader object data:
    retVal = apGLShaderObject::readSelfFromChannel(ipcChannel);

    return retVal;
}
