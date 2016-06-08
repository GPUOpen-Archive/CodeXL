//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLShaderObject.h
///
//==================================================================================

//------------------------------ apGLShaderObject.h ------------------------------

#ifndef __APGLSHADEROBJECT
#define __APGLSHADEROBJECT

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTAPIClasses/Include/apAllocatedObject.h>
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apGLShadingObjectType.h>
#include <AMDTAPIClasses/Include/apAPIVersion.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLShaderObject : public osTransferableObject
// General Description:
//   Base class for classes that represent OpenGL shader object.
//
// Author:  AMD Developer Tools Team
// Creation Date:        04/04/2005
// ----------------------------------------------------------------------------------
class AP_API apGLShaderObject : public apAllocatedObject
{
public:
    // Self functions:
    apGLShaderObject(GLuint shaderName = 0, apGLShadingObjectType shaderType = AP_GL_2_0_SHADING_OBJECT);
    apGLShaderObject(const apGLShaderObject& other);
    virtual ~apGLShaderObject();
    apGLShaderObject& operator=(const apGLShaderObject& other);

    apGLShadingObjectType shaderObjectType() const { return _shaderObjectType; };
    GLuint shaderName() const { return _shaderName; };
    apGLSLVersion shaderVersion() {return _shaderVersion;};
    void setShaderVersion(apGLSLVersion shaderVersion) {_shaderVersion = shaderVersion;};
    bool isMarkedForDeletion() const { return _isMarkedForDeletion; };
    bool isShaderCompiled() const { return _isShaderCompiled; };
    bool isShaderBinaryDataUpdated() const { return _isShaderBinaryDataUpdated; };
    const gtString& compilationLog() const { return _shaderCompilationLog; };
    const osFilePath& sourceCodeFilePath() const { return _sourceCodeFilePath; };
    unsigned long sourceCodeLength() const { return m_sourceCodeLength; };

    void markSourceCodeAsForced(bool isSourceCodeForced) { _isSourceCodeForced = isSourceCodeForced; };
    bool isSourceCodeForced() const { return _isSourceCodeForced; };
    void setAttachedToHolderProgram(bool isAttached) { _isAttachedToHolderProgram = isAttached; };
    bool isAttachedToHolderProgram() const { return _isAttachedToHolderProgram; };

    // On event functions:
    void onShaderCompilation(bool wasCompilationSuccessful, const gtString& compilationLog);
    void onShaderSourceCodeSet(const osFilePath& sourceCodeFilePath);
    void onShaderMarkedForDeletion() { _isMarkedForDeletion = true; };
    void onShaderSourceCodeMarkedAsForced() { _isShaderBinaryDataUpdated = false; };

    // Overrides osTransferableObject:
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
    // The OpenGL vertex shader name:
    GLuint _shaderName;

    // The shader object type:
    apGLShadingObjectType _shaderObjectType;

    // Contains true iff this shader was marked for deletion:
    bool _isMarkedForDeletion;

    // Contains true iff the shader was compiled successfully:
    bool _isShaderCompiled;

    // Contains true iff the shader binary data is updated with the current
    // shader source code:
    bool _isShaderBinaryDataUpdated;

    // Contains the compilation log:
    gtString _shaderCompilationLog;

    // A path of a file that contains the shader object source code:
    osFilePath _sourceCodeFilePath;
    unsigned long m_sourceCodeLength;

    // Contains true iff the source code is "forced".
    // I.E: It is forced by the debugger and the debugged program cannot change it.
    bool _isSourceCodeForced;

    // Contains true iff this shader is currently attached to the holder program:
    bool _isAttachedToHolderProgram;

    // Contain the shader GLSL version:
    apGLSLVersion _shaderVersion;
};


#endif  // __APGLSHADEROBJECT
