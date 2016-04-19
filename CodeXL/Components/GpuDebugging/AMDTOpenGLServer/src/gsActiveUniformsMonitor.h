//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsActiveUniformsMonitor.h
///
//==================================================================================

//------------------------------ gsActiveUniformsMonitor.h ------------------------------

#ifndef __GSACTIVEUNIFORMSMONITOR
#define __GSACTIVEUNIFORMSMONITOR

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTAPIClasses/Include/apGLItemsCollection.h>

// Local:
#include <src/gsProgramUniformsData.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsActiveUniformsMonitor
// General Description: Monitors a program active uniforms.
// Author:               Yaki Tebeka
// Creation Date:        28/5/2005
// ----------------------------------------------------------------------------------
class gsActiveUniformsMonitor
{
public:
    gsActiveUniformsMonitor();

    // On event functions:
    void onFirstTimeContextMadeCurrent();
    void onProgramCreation(GLuint programName, apGLShadingObjectType programType);
    bool onProgramLinked(GLuint programName, bool wasLinkSuccessful);
    bool onProgramPhysicalDeletion(GLuint programName);

    // Attach 'buffer' to index point in the active program:
    bool onUBOBindBuffer(GLuint program, GLenum target, GLuint index, GLuint buffer);

    // Set uniform block binding:
    bool onUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);

    // Query data functions:
    bool getProgramActiveUniforms(GLuint programName, const apGLItemsCollection*& programUniforms) const;
    bool copyProgramActiveUniforms(GLuint programName, apGLItemsCollection& programUniforms) const;
    bool getProgramActiveUniform(GLuint programName, GLuint index, GLsizei bufSize,
                                 GLsizei* pLength, GLint* pSize, GLenum* pType, GLchar* pName) const;
    GLint getUniformLocation(GLuint programName, const GLchar* uniformName) const;

    // Other functions:
    void clearContextDataSnapshot();
    bool updateContextDataSnapshot();
    bool restoreProgramUniformValues(GLuint programName, const apGLItemsCollection& storedUniformValues);
    bool updateStubbedUniformValue(GLuint programName, GLint uniformLocation, GLenum uniformType, void* values, bool transpose = false);

private:
    bool updateProgramAvailableUniforms(GLuint programName);
    bool updateARBProgramAvailableUniforms(apGLItemsCollection& programUniformsContainer, GLuint programName);
    bool update20ProgramAvailableUniforms(apGLItemsCollection& programUniformsContainer, GLuint programName);
    bool updateProgramsUniformLocations(GLuint programName);
    void clearProgramActiveUniformsValues(GLuint programName);
    bool updateProgramActiveUniformsValues(GLuint programName);
    GLint getUniformLocation(GLuint programName, apGLShadingObjectType programType, const gtString& uniformName) const;

    bool getActiveUniformValue(GLuint programName, apGLShadingObjectType programType,
                               GLint uniformLocation, GLenum uniformType, GLint uniformSize,
                               gtAutoPtr<apParameter>& aptrUniformValue);
    bool getFloatUniformValue(GLuint programName, apGLShadingObjectType programType,
                              GLint uniformLocation, gtAutoPtr<apParameter>& aptrUniformValue);
    bool getFloatVecUniformValue(GLuint programName, apGLShadingObjectType programType,
                                 GLint uniformLocation, int vecSize, gtAutoPtr<apParameter>& aptrUniformValue);
    bool getFloatMatUniformValue(GLuint programName, apGLShadingObjectType programType,
                                 GLint uniformLocation, int matSizeN, int matSizeM,
                                 gtAutoPtr<apParameter>& aptrUniformValue);
    bool getIntUniformValue(GLuint programName, apGLShadingObjectType programType,
                            GLint uniformLocation, gtAutoPtr<apParameter>& aptrUniformValue);
    bool getIntVecUniformValue(GLuint programName, apGLShadingObjectType programType,
                               GLint uniformLocation, int vecSize, gtAutoPtr<apParameter>& aptrUniformValue);
    bool getIntMatUniformValue(GLuint programName, apGLShadingObjectType programType,
                               GLint uniformLocation, int matSize, gtAutoPtr<apParameter>& aptrUniformValue);

    bool setActiveUniformValue(GLuint programName, apGLShadingObjectType programType, GLenum uniformType,
                               int uniformLocation, const apParameter& uniformValue);
    bool setFloatUniformValue(GLuint programName, apGLShadingObjectType programType,
                              GLint uniformLocation, const apParameter& uniformValue);
    bool setFloatVecUniformValue(GLuint programName, apGLShadingObjectType programType,
                                 GLint uniformLocation, const apParameter& uniformValue);
    bool setFloatMatUniformValue(GLuint programName, apGLShadingObjectType programType,
                                 GLint uniformLocation, const apParameter& uniformValue);
    bool setIntUniformValue(GLuint programName, apGLShadingObjectType programType,
                            GLint uniformLocation, const apParameter& uniformValue);
    bool setIntVecUniformValue(GLuint programName, apGLShadingObjectType programType,
                               GLint uniformLocation, const apParameter& uniformValue);

    int programsVecIndex(GLuint programName) const;

    void multiParameterFromVoidPointer(int width, int height, bool isInteger, void* values, gtAutoPtr<apParameter>& aptrParameter, bool transpose = false);

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    gsActiveUniformsMonitor& operator=(const gsActiveUniformsMonitor& otherMonitor);
    gsActiveUniformsMonitor(const gsActiveUniformsMonitor& otherMonitor);

    // The names, types, link status and active uniforms of the programs who's active uniforms
    // are monitored by this class:
    gtPtrVector<gsProgramUniformsData*> _programUniformsData;

    // Maps OpenGL program name to the _programUniformsData vectors indices:
    gtMap<GLuint, int> _programNameToVecIndex;

    // For MAC OS OpenGL 2.0 is the base version and ARB_shader_objects is natively supported:
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // GL_ARB_shader_objects extension function pointers:
    PFNGLGETOBJECTPARAMETERIVARBPROC _glGetObjectParameterivARB;
    PFNGLGETACTIVEUNIFORMARBPROC _glGetActiveUniformARB;
    PFNGLGETUNIFORMFVARBPROC _glGetUniformfvARB;
    PFNGLGETUNIFORMIVARBPROC _glGetUniformivARB;
    PFNGLGETUNIFORMLOCATIONARBPROC _glGetUniformLocationARB;

    PFNGLUNIFORM1IVARBPROC _glUniform1ivARB;
    PFNGLUNIFORM2IVARBPROC _glUniform2ivARB;
    PFNGLUNIFORM3IVARBPROC _glUniform3ivARB;
    PFNGLUNIFORM4IVARBPROC _glUniform4ivARB;

    PFNGLUNIFORM1FVARBPROC _glUniform1fvARB;
    PFNGLUNIFORM2FVARBPROC _glUniform2fvARB;
    PFNGLUNIFORM3FVARBPROC _glUniform3fvARB;
    PFNGLUNIFORM4FVARBPROC _glUniform4fvARB;

    PFNGLUNIFORMMATRIX2FVARBPROC _glUniformMatrix2fvARB;
    PFNGLUNIFORMMATRIX3FVARBPROC _glUniformMatrix3fvARB;
    PFNGLUNIFORMMATRIX4FVARBPROC _glUniformMatrix4fvARB;

    // OpenGL 2.0 extension function pointers:
    PFNGLGETPROGRAMIVPROC _glGetProgramiv;
    PFNGLGETACTIVEUNIFORMPROC _glGetActiveUniform;
    PFNGLGETUNIFORMFVPROC _glGetUniformfv;
    PFNGLGETUNIFORMIVPROC _glGetUniformiv;
    PFNGLGETUNIFORMLOCATIONPROC _glGetUniformLocation;
    PFNGLUNIFORM1IVPROC _glUniform1iv;
    PFNGLUNIFORM2IVPROC _glUniform2iv;
    PFNGLUNIFORM3IVPROC _glUniform3iv;
    PFNGLUNIFORM4IVPROC _glUniform4iv;
    PFNGLUNIFORM1FVPROC _glUniform1fv;
    PFNGLUNIFORM2FVPROC _glUniform2fv;
    PFNGLUNIFORM3FVPROC _glUniform3fv;
    PFNGLUNIFORM4FVPROC _glUniform4fv;
    PFNGLUNIFORMMATRIX2FVPROC _glUniformMatrix2fv;
    PFNGLUNIFORMMATRIX3FVPROC _glUniformMatrix3fv;
    PFNGLUNIFORMMATRIX4FVPROC _glUniformMatrix4fv;
#endif

    // OpenGL 2.1 extension function pointers:
    PFNGLUNIFORMMATRIX2X3FVPROC _glUniformMatrix2x3fv;
    PFNGLUNIFORMMATRIX3X2FVPROC _glUniformMatrix3x2fv;
    PFNGLUNIFORMMATRIX2X4FVPROC _glUniformMatrix2x4fv;
    PFNGLUNIFORMMATRIX4X2FVPROC _glUniformMatrix4x2fv;
    PFNGLUNIFORMMATRIX3X4FVPROC _glUniformMatrix3x4fv;
    PFNGLUNIFORMMATRIX4X3FVPROC _glUniformMatrix4x3fv;

    // GL_ARB_uniform_buffer_object extension function pointers:
    PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC _glGetActiveUniformBlockName;
    PFNGLGETUNIFORMBLOCKINDEXPROC _glGetUniformBlockIndex;

    // Contains true iff the appropriate extension functions are supported:
    bool _isARBShaderObjectsExtSupported;
    bool _isOpenGL20Supported;
    bool _isOpenGL21Supported;
    bool _isUniformBufferObjectsExtSupported;
};


#endif  // __GSACTIVEUNIFORMSMONITOR
