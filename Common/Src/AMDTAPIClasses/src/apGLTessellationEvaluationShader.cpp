//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTessellationEvaluationShader.cpp
///
//==================================================================================

//------------------------------ apGLTessellationEvaluationShader.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apGLTessellationEvaluationShader.h>


// ---------------------------------------------------------------------------
// Name:        apGLTessellationEvaluationShader::apGLTessellationEvaluationShader
// Description: Constructor
// Arguments:   shaderName - The tessellation evaluation shader OpenGL name.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
apGLTessellationEvaluationShader::apGLTessellationEvaluationShader(GLuint shaderName, apGLShadingObjectType shaderType)
    : apGLShaderObject(shaderName, shaderType)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLTessellationEvaluationShader::apGLTessellationEvaluationShader
// Description: Copy constructor
// Arguments:   other - The other tessellation evaluation shader from which I am initialized.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
apGLTessellationEvaluationShader::apGLTessellationEvaluationShader(const apGLTessellationEvaluationShader& other)
    : apGLShaderObject(other)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLTessellationEvaluationShader::~apGLTessellationEvaluationShader
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
apGLTessellationEvaluationShader::~apGLTessellationEvaluationShader()
{
}


// ---------------------------------------------------------------------------
// Name:        apGLTessellationEvaluationShader::operator=
// Description: Copy other content into self.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
apGLTessellationEvaluationShader& apGLTessellationEvaluationShader::operator=(const apGLTessellationEvaluationShader& other)
{
    // Copy the shader object data:
    apGLShaderObject::operator=(other);

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apGLTessellationEvaluationShader::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apGLTessellationEvaluationShader::type() const
{
    return OS_TOBJ_ID_GL_TESSELLATION_EVALUATION_SHADER;
}


// ---------------------------------------------------------------------------
// Name:        apGLTessellationEvaluationShader::writeSelfIntoChannel
// Description: Writes my content into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
bool apGLTessellationEvaluationShader::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    // Write the shader object data:
    retVal = apGLShaderObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTessellationEvaluationShader::readSelfFromChannel
// Description: Reads my content from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/9/2013
// ---------------------------------------------------------------------------
bool apGLTessellationEvaluationShader::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the shader object data:
    retVal = apGLShaderObject::readSelfFromChannel(ipcChannel);

    return retVal;
}
