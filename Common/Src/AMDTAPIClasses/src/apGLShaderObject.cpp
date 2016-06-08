//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLShaderObject.cpp
///
//==================================================================================

//------------------------------ apGLShaderObject.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osFile.h>

// Local:
#include <AMDTAPIClasses/Include/apGLShaderObject.h>


// ---------------------------------------------------------------------------
// Name:        apGLShaderObject::apGLShaderObject
// Description: Constructor
// Arguments:   shaderName - The OpenGL shader object name.
//              shaderType - The shader object type.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLShaderObject::apGLShaderObject(GLuint shaderName, apGLShadingObjectType shaderType)
    : apAllocatedObject(), _shaderName(shaderName), _shaderObjectType(shaderType), _isMarkedForDeletion(false),
      _isShaderCompiled(false), _isShaderBinaryDataUpdated(false), m_sourceCodeLength(0), _isSourceCodeForced(false), _isAttachedToHolderProgram(false), _shaderVersion(AP_GLSL_VERSION_NONE)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLShaderObject::apGLShaderObject
// Description: Copy constructor
// Arguments:   other - The other shader object from which I am initialized.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLShaderObject::apGLShaderObject(const apGLShaderObject& other)
    : apAllocatedObject(), _shaderName(other._shaderName), _shaderObjectType(other._shaderObjectType),
      _isMarkedForDeletion(other._isMarkedForDeletion), _isShaderCompiled(other._isShaderCompiled),
      _isShaderBinaryDataUpdated(false), _shaderCompilationLog(other._shaderCompilationLog),
      _sourceCodeFilePath(other._sourceCodeFilePath), m_sourceCodeLength(other.m_sourceCodeLength), _isSourceCodeForced(false), _isAttachedToHolderProgram(false), _shaderVersion(other._shaderVersion)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLShaderObject::~apGLShaderObject
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLShaderObject::~apGLShaderObject()
{
}


// ---------------------------------------------------------------------------
// Name:        apGLShaderObject::operator=
// Description: Copy other content into self.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLShaderObject& apGLShaderObject::operator=(const apGLShaderObject& other)
{
    // Copy the other shader object type:
    _shaderObjectType = other.shaderObjectType();

    // Copy the shader name:
    _shaderName = other.shaderName();

    // Copy the "is marked for deletion status:
    _isMarkedForDeletion = other.isMarkedForDeletion();

    // Copy the compile status:
    _isShaderCompiled = other.isShaderCompiled();

    // Copy the compilation log:
    _shaderCompilationLog = other.compilationLog();

    // Copy the source code file path:
    _sourceCodeFilePath = other.sourceCodeFilePath();

    // Copy the source code length:
    m_sourceCodeLength = other.sourceCodeLength();

    // Copy the "source code forced" flag:
    _isSourceCodeForced = other.isSourceCodeForced();

    // Copy the "attached to holder program" flag:
    _isAttachedToHolderProgram = other.isAttachedToHolderProgram();

    // Copy the "Is binary data updated" flag:
    _isShaderBinaryDataUpdated = other.isShaderBinaryDataUpdated();

    // Copy the shader version:
    _shaderVersion = other._shaderVersion;

    setAllocatedObjectId(other.getAllocatedObjectId(), true);

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apGLShaderObject::onShaderCompilation
// Description: Is called when a shader is compiled.
// Arguments: wasCompilationSuccessful - True iff the shader was compiled successfully.
//            compilationLog - The shader compilation log.
// Author:  AMD Developer Tools Team
// Date:        17/11/2005
// ---------------------------------------------------------------------------
void apGLShaderObject::onShaderCompilation(bool wasCompilationSuccessful, const gtString& compilationLog)
{
    // Log the shader compilation status and log:
    _isShaderCompiled = wasCompilationSuccessful;
    _shaderCompilationLog = compilationLog;

    // If the compilation was successful, mark that the binary is in syn with the source code:
    if (wasCompilationSuccessful)
    {
        _isShaderBinaryDataUpdated = true;
    }
}


// ---------------------------------------------------------------------------
// Name:        apGLShaderObject::onShaderSourceCodeSet
// Description: Is called when the shader source code is set.
// Arguments: sourceCodeFilePath - A file that contains the new shader source
//                                 code.
// Author:  AMD Developer Tools Team
// Date:        17/11/2005
// ---------------------------------------------------------------------------
void apGLShaderObject::onShaderSourceCodeSet(const osFilePath& sourceCodeFilePath)
{
    // Log the new shader source code:
    _sourceCodeFilePath = sourceCodeFilePath;

    osFile sourceCodeFile(_sourceCodeFilePath);
    sourceCodeFile.getSize(m_sourceCodeLength);

    // Mark that the shader binary data is not update with its source code:
    _isShaderBinaryDataUpdated = false;
};



// ---------------------------------------------------------------------------
// Name:        apGLShaderObject::writeSelfIntoChannel
// Description: Writes my content into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
bool apGLShaderObject::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the shader name:
    ipcChannel << (gtUInt32)_shaderName;

    // Write the shader type:
    ipcChannel << (gtInt32)_shaderObjectType;

    // Write the "is marked for deletion" status:
    ipcChannel << _isMarkedForDeletion;

    // Write the compilation status:
    ipcChannel << _isShaderCompiled;

    // Write the compilation log:
    ipcChannel << _shaderCompilationLog;

    // Write the "is binary data updated" flag:
    ipcChannel << _isShaderBinaryDataUpdated;

    // Write the source code file path:
    ipcChannel << _sourceCodeFilePath.asString();

    // Write the source code length:
    ipcChannel << (gtUInt32)m_sourceCodeLength;

    // Write the "is source code forced" flag:
    ipcChannel << _isSourceCodeForced;

    // Write the "is attached to holder program" flag:
    ipcChannel << _isAttachedToHolderProgram;

    // Write the shader version:
    ipcChannel << (gtInt32)_shaderVersion;

    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLShaderObject::readSelfFromChannel
// Description: Reads my content from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
bool apGLShaderObject::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the shader name:
    gtUInt32 shaderNameAsUInt32 = 0;
    ipcChannel >> shaderNameAsUInt32;
    _shaderName = (int)shaderNameAsUInt32;

    // Read the shader type:
    gtInt32 shaderObjectTypeAsInt = AP_GL_2_0_SHADING_OBJECT;
    ipcChannel >> shaderObjectTypeAsInt;
    _shaderObjectType = (apGLShadingObjectType)shaderObjectTypeAsInt;

    // Read the "is marked for deletion" status:
    ipcChannel >> _isMarkedForDeletion;

    // Read the compilation status:
    ipcChannel >> _isShaderCompiled;

    // Read the compilation log:
    ipcChannel >> _shaderCompilationLog;

    // Read the "is binary data updated" flag:
    ipcChannel >> _isShaderBinaryDataUpdated;

    // Read the source code file path:
    gtString sourceCodeFilePathAsStr;
    ipcChannel >> sourceCodeFilePathAsStr;
    _sourceCodeFilePath = sourceCodeFilePathAsStr;

    // Read the source code length:
    gtUInt32 sourceCodeLengthAsUInt32 = 0;
    ipcChannel >> sourceCodeLengthAsUInt32;
    m_sourceCodeLength = (unsigned long)sourceCodeLengthAsUInt32;

    // Read the "is source code forced" flag:
    ipcChannel >> _isSourceCodeForced;

    // Read the "is attached to holder program" flag:
    ipcChannel >> _isAttachedToHolderProgram;

    // Read the shader version:
    gtInt32 shaderVersionAsInt = 0;
    ipcChannel >> shaderVersionAsInt;
    _shaderVersion = (apGLSLVersion)shaderVersionAsInt;

    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return true;
}

