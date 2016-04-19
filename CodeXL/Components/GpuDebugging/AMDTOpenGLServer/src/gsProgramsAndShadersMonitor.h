//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsProgramsAndShadersMonitor.h
///
//==================================================================================

//------------------------------ gsProgramsAndShadersMonitor.h ------------------------------

#ifndef __GSPROGRAMSANDSHADERSMONITOR
#define __GSPROGRAMSANDSHADERSMONITOR

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTAPIClasses/Include/apGLShaderObject.h>

// Local:
#include <src/gsActiveUniformsMonitor.h>
#include <src/gsGLProgram.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsProgramsAndShadersMonitor
// General Description:
//   Monitors OpenGL Programs, Vertex Shaders and Fragment Shaders.
//
// Author:               Yaki Tebeka
// Creation Date:        04/04/2004
// ----------------------------------------------------------------------------------
class gsProgramsAndShadersMonitor
{
public:
    gsProgramsAndShadersMonitor(int spyContextId = 0);
    ~gsProgramsAndShadersMonitor();

    // On event functions:
    void onFirstTimeContextMadeCurrent();
    void onFrameTerminatorCall();
    void onProgramCreation(GLuint programName, apGLShadingObjectType programType);
    bool onProgramDeletion(GLuint programName);
    bool onProgramLinked(GLuint programName, bool& wasLinkSuccessful);

    void onVertexShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType);
    void onTessellationControlShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType);
    void onTessellationEvaluationShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType);
    void onGeometryShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType);
    void onFragmentShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType);
    void onComputeShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType);
    void onUnsupportedShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType);
    bool onShaderObjectDeletion(GLuint shaderName);
    bool onShaderSourceCodeSet(GLuint shaderName, GLsizei nstrings, const GLcharARB* const* strings, const GLint* lengths);
    bool onShaderCompilation(GLuint shaderName, bool& wasCompilationSuccessful);

    bool onShaderAttachedToProgram(GLuint programName, GLuint shaderName);
    bool onShaderDetachedFromProgram(GLuint programName, GLuint shaderName);
    bool onProgramParameteri(GLuint program, GLenum pname, GLint value);

    // Query data functions:
    bool isManagedObject(GLuint objectName) const;
    bool isProgramObject(GLuint objectName) const;
    bool isShaderObject(GLuint objectName) const;
    osTransferableObjectType shaderObjectType(GLuint objectName) const;
    int amountOfProgramObjects() const { return (int)_programObjects.size(); };
    int amountOfLivingProgramObjects() const;
    int amountOfShaderObjects() const { return (int)_shaderObjects.size(); };
    int amountOfLivingShaderObjects() const;
    GLuint programObjectName(int programId) const;
    GLuint shaderObjectName(int shaderId) const;
    const apGLProgram* programObjectDetails(GLuint programName) const;
    const apGLShaderObject* shaderObjectDetails(GLuint shaderName) const;
    gsGLProgram* getProgramObject(GLuint programName);

    // Forced modes:
    bool applyForcedStubGeometryShader();
    bool applyForcedStubGeometryShaderToProgram(gsGLProgram& program);
    bool cancelForcedStubGeometryShader();
    bool applyForcedStubFragmentShader();
    bool applyForcedStubFragmentShaderToProgram(gsGLProgram& program);
    bool cancelForcedStubFragmentShader();
    bool stubFragmentShaderSources(bool ignoreMultipleCalls = true);
    bool restoreStubbedFragmentShaderSources();
    bool markShaderObjectSourceCodeAsForced(GLuint shaderName, bool isSourceCodeForced);
    bool setShaderObjectSourceCode(GLuint shaderName, const osFilePath& inputSourceCodeFilePath);
    bool compileShaderObject(GLuint shaderName, bool& wasCompilationSuccessful, gtString& compilationLog);
    bool linkProgramObject(GLuint programName, bool& wasLinkSuccessful, gtString& linkLog, GLuint activeProgramName);
    bool validateProgramObject(GLuint programName, bool& wasValidationSuccessful, gtString& validationLog);
    bool areStubFragmentShadersForced() { return _areStubFragmentShadersForced; };
    bool areStubGeometryShadersForced() { return _areStubGeometryShadersForced; };
    bool areFragmentShaderSourcesStubbed() { return _areFragmentShaderSourcesStubbed; };
    bool activateProgramForUpdate(GLuint programName);
    bool restoreActiveProgram(GLuint activeProgramName);

    // Other:
    const gsActiveUniformsMonitor& programsActiveUniformsMgr() const { return _programsActiveUniforms; };
    gsActiveUniformsMonitor& programsActiveUniformsMgr() { return _programsActiveUniforms; };
    void clearContextDataSnapshot();
    bool updateContextDataSnapshot();

    // Memory:
    bool calculateShadersMemorySize(gtUInt64& buffersMemorySize) const ;

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    gsProgramsAndShadersMonitor& operator=(const gsProgramsAndShadersMonitor& otherMonitor);
    gsProgramsAndShadersMonitor(const gsProgramsAndShadersMonitor& otherMonitor);

    int programsVecIndex(GLuint programName) const;
    int shadersVecIndex(GLuint shaderName) const;

    GLuint createStubFragmentShader(apGLShadingObjectType shaderType);
    GLuint createFragmentShaderObject(apGLShadingObjectType shaderType);
    GLuint getShaderHolderProgram(apGLShadingObjectType programType, bool createIfNeeded = true);
    GLuint createProgramObject(apGLShadingObjectType programType);
    bool deleteFragmentShaderObject(GLuint shaderName, apGLShadingObjectType shaderObjType);
    bool deleteProgramObject(GLuint programName, apGLShadingObjectType programObjType);
    bool detachProgramGeometryShaders(const gsGLProgram& program);
    bool attachBackProgramGeometryShaders(const gsGLProgram& program);
    bool detachProgramFragmentShaders(const gsGLProgram& program);
    bool attachBackProgramFragmentShaders(const gsGLProgram& program);
    bool detachProgramFragmentShadersFromHolderProgram(const gsGLProgram& program);
    bool attachProgramFragmentShadersToHolderProgram(const gsGLProgram& program);
    bool detachProgramGeometryShadersFromHolderProgram(const gsGLProgram& program);
    bool attachProgramGeometryShadersToHolderProgram(const gsGLProgram& program);
    bool detachStubFragmentShaderFromProgram(gsGLProgram& program);
    bool attachStubFragmentShaderToProgram(gsGLProgram& program);
    bool detachShaderFromProgram(GLuint programName, GLuint shaderName, apGLShadingObjectType shaderObjType);
    bool attachShaderToProgram(GLuint programName, GLuint shaderName, apGLShadingObjectType shaderObjType);
    bool linkProgram(GLuint programName, apGLShadingObjectType programObjType, bool& wasLinkSuccessful);
    bool validateProgram(GLuint programName, apGLShadingObjectType programObjType, bool& wasValidationSuccessful);
    void generateShaderSourceCodeFilePath(apGLShaderObject& shaderObj, osFilePath& filePath) const;
    bool isShaderAttachedToAnyProgram(GLuint shaderName) const;

    bool setShaderSourceCode(GLuint shaderName, const gtASCIIString& shaderSourceCode);
    bool logShaderSourceCodeUpdate(apGLShaderObject& shaderObj, const gtASCIIString& newShaderSourceCode);
    bool getShaderSourceCodeFromFile(const apGLShaderObject& shaderObj, gtString& shaderSource);
    bool wasShaderCompiledSuccessfully(GLuint shaderName, apGLShadingObjectType shaderType) const;
    void getShaderCompilationLog(GLuint shaderName, apGLShadingObjectType shaderType, gtString& compilationLog) const;
    bool wasProgramSuccessfullyLinked(GLuint programName, apGLShadingObjectType programType) const;
    bool getProgramInfoLog(GLuint programName, apGLShadingObjectType programType, gtString& linkLog) const;
    bool wasShaderPhysicallyDeleted(GLuint shaderName, apGLShadingObjectType shaderType) const;
    bool wasProgramPhysicallyDeleted(GLuint programName, apGLShadingObjectType programType) const;
    void onProgramPhysicalDeletion(GLuint programName, const gsGLProgram* pProgramObj);
    bool removeManagedObjectFromVectors(GLuint objectName);
    bool removeShaderFromVectors(GLuint shaderName);
    bool removeProgramFromVectors(GLuint programName);

    // Shader version:
    void getShaderSourceCodeVersion(const gtASCIIString& sourceCodeAsString, apGLSLVersion& glslSourceVersion);

private:
    // The Spy id of my monitored render context:
    int _spyContextId;

    // Holds OpenGL program objects data:
    gtPtrVector<gsGLProgram*> _programObjects;

    // Holds shader objects data:
    gtPtrVector<apGLShaderObject*> _shaderObjects;

    // Maps OpenGL program object name to _programObjects indices:
    gtMap<GLuint, int> _openGLProgramNameToProgramObjectsVecIndex;

    // Maps OpenGL shader object name to _shaderObjects indices:
    gtMap<GLuint, int> _openGLShaderNameToShaderObjectsVecIndex;

    // Monitors the programs active uniforms:
    gsActiveUniformsMonitor _programsActiveUniforms;

    // Contains true iff stub fragment shaders are forced:
    bool _areStubGeometryShadersForced;

    // Contains true iff stub fragment shaders are forced:
    bool _areStubFragmentShadersForced;

    // Contains true iff we are using the iPhone's version of fragment shader stubbing:
    bool _areFragmentShaderSourcesStubbed;

    // Contains the stub fragment shader names:
    // (per fragment shader extension: GL_ARB_shader_object, OpenGL 2.0):
    GLuint _shaderObjectExtStubFragmentShaderName;
    GLuint _openGL20StubFragmentShaderName;

    // Contains the program we use so that shaders marked for deletion
    // will not be destroyed when we force stub shaders
    // (per fragment shader extension: GL_ARB_shader_object, OpenGL 2.0):
    GLuint _shaderObjectExtShaderHolderProgramName;
    GLuint _openGL20ShaderHolderProgramName;

    // Contains true iff GL_ARB_shader_objects is supported:
    bool _isARBShaderObjectsExtSupported;

    // Contains true iff OpenGL 2.0 is supported:
    bool _isOpenGL20Supported;

#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // GL_ARB_shader_objects function pointers:
    PFNGLCREATESHADEROBJECTARBPROC _glCreateShaderObjectARB;
    PFNGLSHADERSOURCEARBPROC _glShaderSourceARB;
    PFNGLCOMPILESHADERARBPROC _glCompileShaderARB;
    PFNGLGETOBJECTPARAMETERIVARBPROC _glGetObjectParameterivARB;
    PFNGLDETACHOBJECTARBPROC _glAttachObjectARB;
    PFNGLDETACHOBJECTARBPROC _glDetachObjectARB;
    PFNGLLINKPROGRAMARBPROC _glLinkProgramARB;
    PFNGLVALIDATEPROGRAMARBPROC _glValidateProgramARB;
    PFNGLDELETEOBJECTARBPROC _glDeleteObjectARB;
    PFNGLGETINFOLOGARBPROC _glGetInfoLogARB;
    PFNGLUSEPROGRAMOBJECTARBPROC _glUseProgramObjectARB;
    PFNGLCREATEPROGRAMOBJECTARBPROC _glCreateProgramObjectARB;

    // OpenGL 2.0 function pointers:
    PFNGLCREATESHADERPROC _glCreateShader;
    PFNGLSHADERSOURCEPROC _glShaderSource;
    PFNGLCOMPILESHADERPROC _glCompileShader;
    PFNGLGETSHADERIVPROC _glGetShaderiv;
    PFNGLATTACHSHADERPROC _glAttachShader;
    PFNGLDETACHSHADERPROC _glDetachShader;
    PFNGLLINKPROGRAMPROC _glLinkProgram;
    PFNGLVALIDATEPROGRAMPROC _glValidateProgram;
    PFNGLGETPROGRAMIVPROC _glGetProgramiv;
    PFNGLDELETESHADERPROC _glDeleteShader;
    PFNGLGETPROGRAMINFOLOGPROC _glGetProgramInfoLog;
    PFNGLGETSHADERINFOLOGPROC _glGetShaderInfoLog;
    PFNGLUSEPROGRAMPROC _glUseProgram;
    PFNGLCREATEPROGRAMPROC _glCreateProgram;
    PFNGLDELETEPROGRAMPROC _glDeleteProgram;
#endif
};


#endif  // __GSPROGRAMSANDSHADERSMONITOR
