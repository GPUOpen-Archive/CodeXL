//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsProgramsAndShadersMonitor.cpp
///
//==================================================================================

//------------------------------ gsProgramsAndShadersMonitor.cpp ------------------------------

#include <math.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apGLVertexShader.h>
#include <AMDTAPIClasses/Include/apGLTessellationControlShader.h>
#include <AMDTAPIClasses/Include/apGLTessellationEvaluationShader.h>
#include <AMDTAPIClasses/Include/apGLGeometryShader.h>
#include <AMDTAPIClasses/Include/apGLFragmentShader.h>
#include <AMDTAPIClasses/Include/apGLComputeShader.h>
#include <AMDTAPIClasses/Include/apGLUnsupportedShader.h>
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>

// Spy utilities:
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsGLProgram.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsProgramsAndShadersMonitor.h>


// ------------- Static data -----------------

// The stub vertex shader source code:
static const char* stat_stubVertexShaderSourceCode = "\
void main(void) \
{ \
   gl_FragColor = vec4 (1.0, 0.3176, 0.0, 1.0); \
} ";


// ------------- Class methods -----------------


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::gsProgramsAndShadersMonitor
// Description: Constructor
// Arguments: spyContextId - The Spy id of my monitored render context.
// Author:      Yaki Tebeka
// Date:        4/4/2005
// ---------------------------------------------------------------------------
gsProgramsAndShadersMonitor::gsProgramsAndShadersMonitor(int spyContextId)
    : _spyContextId(spyContextId), _areStubGeometryShadersForced(false),
      _areStubFragmentShadersForced(false), _areFragmentShaderSourcesStubbed(false), _shaderObjectExtStubFragmentShaderName(0),
      _openGL20StubFragmentShaderName(0), _shaderObjectExtShaderHolderProgramName(0), _openGL20ShaderHolderProgramName(0),
      _isARBShaderObjectsExtSupported(false), _isOpenGL20Supported(false)
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    , _glCreateShaderObjectARB(NULL), _glShaderSourceARB(NULL), _glCompileShaderARB(NULL), _glGetObjectParameterivARB(NULL),
      _glAttachObjectARB(NULL), _glDetachObjectARB(NULL), _glLinkProgramARB(NULL), _glValidateProgramARB(NULL),
      _glDeleteObjectARB(NULL), _glGetInfoLogARB(NULL), _glUseProgramObjectARB(NULL), _glCreateProgramObjectARB(NULL),
      _glCreateShader(NULL), _glShaderSource(NULL), _glCompileShader(NULL), _glGetShaderiv(NULL),
      _glAttachShader(NULL), _glDetachShader(NULL), _glLinkProgram(NULL), _glValidateProgram(NULL),
      _glGetProgramiv(NULL), _glDeleteShader(NULL), _glGetProgramInfoLog(NULL), _glGetShaderInfoLog(NULL),
      _glUseProgram(NULL), _glCreateProgram(NULL), _glDeleteProgram(NULL)
#endif
{
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::~gsProgramsAndShadersMonitor
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        4/4/2005
// ---------------------------------------------------------------------------
gsProgramsAndShadersMonitor::~gsProgramsAndShadersMonitor()
{
    // The member vectors and maps content is deleted automatically at their destruction.

    // Delete any objects created by this monitor:
    if (_shaderObjectExtStubFragmentShaderName != 0)
    {
        bool rcStubEXT = deleteFragmentShaderObject(_shaderObjectExtStubFragmentShaderName, AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT);
        GT_ASSERT(rcStubEXT);
        _shaderObjectExtStubFragmentShaderName = 0;
    }

    if (_openGL20StubFragmentShaderName != 0)
    {
        bool rcStub20 = deleteFragmentShaderObject(_openGL20StubFragmentShaderName, AP_GL_2_0_SHADING_OBJECT);
        GT_ASSERT(rcStub20);
        _openGL20StubFragmentShaderName = 0;
    }

    if (_shaderObjectExtShaderHolderProgramName != 0)
    {
        bool rcHoldEXT = deleteProgramObject(_shaderObjectExtShaderHolderProgramName, AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT);
        GT_ASSERT(rcHoldEXT);
        _shaderObjectExtShaderHolderProgramName = 0;
    }

    if (_openGL20ShaderHolderProgramName != 0)
    {
        bool rcHold20 = deleteProgramObject(_openGL20ShaderHolderProgramName, AP_GL_2_0_SHADING_OBJECT);
        GT_ASSERT(rcHold20);
        _openGL20ShaderHolderProgramName = 0;
    }
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onFirstTimeContextMadeCurrent
// Description: Is called on the first time in which the context, in which
//              this class logs shaders and programs, is made the current
//              context.
// Author:      Yaki Tebeka
// Date:        10/4/2005
// Implementation Notes:
//   Initialize supported extensions and extensions function pointers.
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::onFirstTimeContextMadeCurrent()
{
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // Initialize GL_ARB_shader_objects function pointers:
    _glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)gsGetSystemsOGLModuleProcAddress("glCreateShaderObjectARB");
    _glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)gsGetSystemsOGLModuleProcAddress("glShaderSourceARB");
    _glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)gsGetSystemsOGLModuleProcAddress("glCompileShaderARB");
    _glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)gsGetSystemsOGLModuleProcAddress("glGetObjectParameterivARB");
    _glAttachObjectARB = (PFNGLDETACHOBJECTARBPROC)gsGetSystemsOGLModuleProcAddress("glAttachObjectARB");
    _glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)gsGetSystemsOGLModuleProcAddress("glDetachObjectARB");
    _glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)gsGetSystemsOGLModuleProcAddress("glLinkProgramARB");
    _glValidateProgramARB = (PFNGLVALIDATEPROGRAMARBPROC)gsGetSystemsOGLModuleProcAddress("glValidateProgramARB");
    _glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)gsGetSystemsOGLModuleProcAddress("glDeleteObjectARB");
    _glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)gsGetSystemsOGLModuleProcAddress("glGetInfoLogARB");
    _glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)gsGetSystemsOGLModuleProcAddress("glUseProgramObjectARB");
    _glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)gsGetSystemsOGLModuleProcAddress("glCreateProgramObjectARB");

    // Verify that we managed to get all function pointers:
    _isARBShaderObjectsExtSupported = ((_glCreateShaderObjectARB != NULL) && (_glShaderSourceARB != NULL) &&
                                       (_glCompileShaderARB != NULL) && (_glGetObjectParameterivARB != NULL) &&
                                       (_glAttachObjectARB != NULL) && (_glDetachObjectARB != NULL) &&
                                       (_glLinkProgramARB != NULL) && (_glValidateProgramARB != NULL) &&
                                       (_glDeleteObjectARB != NULL) && (_glGetInfoLogARB != NULL) &&
                                       (_glUseProgramObjectARB != NULL) && (_glCreateProgramObjectARB != NULL));

    // Initialize OpenGL 2.0 function pointers:
    _glCreateShader = (PFNGLCREATESHADERPROC)gsGetSystemsOGLModuleProcAddress("glCreateShader");
    _glShaderSource = (PFNGLSHADERSOURCEPROC)gsGetSystemsOGLModuleProcAddress("glShaderSource");
    _glCompileShader = (PFNGLCOMPILESHADERPROC)gsGetSystemsOGLModuleProcAddress("glCompileShader");
    _glGetShaderiv = (PFNGLGETSHADERIVPROC)gsGetSystemsOGLModuleProcAddress("glGetShaderiv");
    _glAttachShader = (PFNGLATTACHSHADERPROC)gsGetSystemsOGLModuleProcAddress("glAttachShader");
    _glDetachShader = (PFNGLDETACHSHADERPROC)gsGetSystemsOGLModuleProcAddress("glDetachShader");
    _glLinkProgram = (PFNGLLINKPROGRAMPROC)gsGetSystemsOGLModuleProcAddress("glLinkProgram");
    _glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)gsGetSystemsOGLModuleProcAddress("glValidateProgram");
    _glGetProgramiv = (PFNGLGETPROGRAMIVPROC)gsGetSystemsOGLModuleProcAddress("glGetProgramiv");
    _glDeleteShader = (PFNGLDELETESHADERPROC)gsGetSystemsOGLModuleProcAddress("glDeleteShader");
    _glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)gsGetSystemsOGLModuleProcAddress("glGetProgramInfoLog");
    _glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)gsGetSystemsOGLModuleProcAddress("glGetShaderInfoLog");
    _glUseProgram = (PFNGLUSEPROGRAMPROC)gsGetSystemsOGLModuleProcAddress("glUseProgram");
    _glCreateProgram = (PFNGLCREATEPROGRAMPROC)gsGetSystemsOGLModuleProcAddress("glCreateProgram");
    _glDeleteProgram = (PFNGLDELETEPROGRAMPROC)gsGetSystemsOGLModuleProcAddress("glDeleteProgram");

    // Verify that we managed to get all function pointers:
    _isOpenGL20Supported = ((_glCreateShader != NULL) && (_glShaderSource != NULL) &&
                            (_glCompileShader != NULL) && (_glGetShaderiv != NULL) &&
                            (_glAttachShader != NULL) && (_glDetachShader != NULL) &&
                            (_glLinkProgram != NULL) && (_glValidateProgram != NULL) &&
                            (_glGetProgramiv != NULL) && (_glDeleteShader != NULL) &&
                            (_glGetProgramInfoLog != NULL) && (_glGetShaderInfoLog != NULL) &&
                            (_glUseProgram != NULL) && (_glCreateProgram != NULL) &&
                            (_glDeleteProgram != NULL));
#else
#ifdef _GR_IPHONE_BUILD
    // ARB_shader_objects is not supported on the iPhone. OpenGL 2.0-type shaders are
    // supported only on OpenGL ES 2.0+, which is included in the iPhone SDK 3.0 and higher.
    // So, we check if all functions are present:
    _isARBShaderObjectsExtSupported = false;
    _isOpenGL20Supported = ((gs_stat_realFunctionPointers.glCreateShader != NULL) && (gs_stat_realFunctionPointers.glShaderSource != NULL) &&
                            (gs_stat_realFunctionPointers.glCompileShader != NULL) && (gs_stat_realFunctionPointers.glGetShaderiv != NULL) &&
                            (gs_stat_realFunctionPointers.glAttachShader != NULL) && (gs_stat_realFunctionPointers.glDetachShader != NULL) &&
                            (gs_stat_realFunctionPointers.glLinkProgram != NULL) && (gs_stat_realFunctionPointers.glValidateProgram != NULL) &&
                            (gs_stat_realFunctionPointers.glGetProgramiv != NULL) && (gs_stat_realFunctionPointers.glDeleteShader != NULL) &&
                            (gs_stat_realFunctionPointers.glGetProgramInfoLog != NULL) && (gs_stat_realFunctionPointers.glGetShaderInfoLog != NULL) &&
                            (gs_stat_realFunctionPointers.glUseProgram != NULL));
#else
    // OpenGL 2.0 On MAC platform is the base version, and ARB_shader_objects is supported natively:
    _isARBShaderObjectsExtSupported = true;
    _isOpenGL20Supported = true;
#endif
#endif

    // Notify the active uniforms manager:
    _programsActiveUniforms.onFirstTimeContextMadeCurrent();
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onFrameTerminatorCall
// Description: Is called when a frame terminator call (wglSwapBugger, etc) is
//              called.
// Author:      Yaki Tebeka
// Date:        13/12/2005
// Implementation notes:
//  - Copy programs _wasUsedInCurrentFrame flag to _wasUsedInLastFrame flag.
//  - Re-initialize the _wasUsedInCurrentFrame flag to false.
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::onFrameTerminatorCall()
{
    // Iterate the existing program objects:
    int amountOfProgramObjects = (int)_programObjects.size();

    for (int i = 0; i < amountOfProgramObjects; i++)
    {
        // Get the current program:
        gsGLProgram* pCurrentProgram = _programObjects[i];

        if (NULL != pCurrentProgram)
        {
            // Copy programs _wasUsedInCurrentFrame flag to _wasUsedInLastFrame flag:
            bool wasUsedInCurrentFrame = pCurrentProgram->wasUsedInCurrentFrame();
            pCurrentProgram->setWasUsedInLastFrame(wasUsedInCurrentFrame);

            // Re-initialize the _wasUsedInCurrentFrame flag to false:
            pCurrentProgram->setWasUsedInCurrentFrame(false);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onProgramCreation
// Description: Is called when a program object is created.
// Arguments:   programName - The OpenGL program name.
//              programType - The program type.
// Author:      Yaki Tebeka
// Date:        4/4/2005
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::onProgramCreation(GLuint programName, apGLShadingObjectType programType)
{
    bool rc = false;

    // If the name already exist in out vectors:
    if (isManagedObject(programName))
    {
        // It means that the object to which this name belonged to was
        // physically deleted and now the name is allocated to the new shader.
        // We need to remove the old object from our vectors:
        removeManagedObjectFromVectors(programName);
    }

    // Create an object that will represent the program:
    gsGLProgram* pNewProgram = new gsGLProgram(programName, programType);

    if (pNewProgram != NULL)
    {
        // Add the program to this class vectors and maps:
        int vectorId = (int)_programObjects.size();
        _programObjects.push_back(pNewProgram);

        _openGLProgramNameToProgramObjectsVecIndex[programName] = vectorId;

        // Register this object in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pNewProgram);

        // Notify the active uniforms manager:
        _programsActiveUniforms.onProgramCreation(programName, programType);

        rc = true;
    }

    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onProgramDeletion
// Description: Is called when a program is deleted.
// Arguments: programName - The OpenGL name of the deleted program.
// Return Val:  bool - Success / failure. Failure occurs when trying to delete
//                     a program that does not exist.
// Author:      Yaki Tebeka
// Date:        5/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::onProgramDeletion(GLuint programName)
{
    bool retVal = false;

    // Get the program object vector index:
    int progIndex = programsVecIndex(programName);

    if (progIndex != -1)
    {
        // Get the program object representation:
        gsGLProgram* pProgramObj = _programObjects[progIndex];

        if (NULL != pProgramObj)
        {
            retVal = true;

            // Mark it for deletion:
            pProgramObj->onProgramMarkedForDeletion();

            // Get the program type:
            apGLShadingObjectType programType = pProgramObj->programType();

            // If the program was "physically" deleted:
            bool wasProgramDeleted = wasProgramPhysicallyDeleted(programName, programType);

            if (wasProgramDeleted)
            {
                onProgramPhysicalDeletion(programName, pProgramObj);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onProgramLinked
// Description: Is called when a program is linked (successfully).
// Arguments:   programName - The OpenGL name of the linked program.
//              wasLinkSuccessful - Will get true iff the link was successful.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        6/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::onProgramLinked(GLuint programName, bool& wasLinkSuccessful)
{
    bool retVal = false;

    // Get the object that represents the program:
    int programsVectorIndex = programsVecIndex(programName);

    if (programsVectorIndex != -1)
    {
        // Get the program type:
        gsGLProgram* pProgramObj = _programObjects[programsVectorIndex];
        apGLShadingObjectType programType = pProgramObj->programType();

        // Check if the link was successful:
        wasLinkSuccessful = wasProgramSuccessfullyLinked(programName, programType);

        // Get the program link log:
        gtString linkLog;
        getProgramInfoLog(programName, programType, linkLog);

        // Update the object that represents the program:
        _programObjects[programsVectorIndex]->onProgramLink(wasLinkSuccessful, linkLog);

        // Log the program active uniforms:
        _programsActiveUniforms.onProgramLinked(programName, wasLinkSuccessful);

        // If the link was successful and stub fragment shaders are forced:
        if (wasLinkSuccessful && _areStubFragmentShadersForced)
        {
            // Apply the forced stub fragment shader on the program:
            applyForcedStubFragmentShaderToProgram(*pProgramObj);
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onVertexShaderCreation
// Description: Is called when a vertex shader is created
// Arguments:   shaderName - The vertex shader OpenGL name.
//              shaderType - The vertex shader object type.
// Author:      Yaki Tebeka
// Date:        4/4/2005
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::onVertexShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType)
{
    bool rc = false;

    // If the name already exist in out vectors:
    if (isManagedObject(shaderName))
    {
        // It means that the object to which this name belonged to was
        // physically deleted and now the name is allocated to the new shader.
        // We need to remove the old object from our vectors:
        removeManagedObjectFromVectors(shaderName);
    }

    // Create an object that will represent the vertex shader:
    apGLShaderObject* pNewShaderObject = new apGLVertexShader(shaderName, shaderType);

    if (NULL != pNewShaderObject)
    {
        // Add the shader object  to this class vectors and maps:
        int vectorId = (int)_shaderObjects.size();
        _shaderObjects.push_back(pNewShaderObject);

        _openGLShaderNameToShaderObjectsVecIndex[shaderName] = vectorId;

        // Register this object in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pNewShaderObject);

        rc = true;
    }

    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onTessellationControlShaderCreation
// Description: Is called when a tessellation control shader is created
// Arguments:   shaderName - The vertex shader OpenGL name.
//              shaderType - The vertex shader object type.
// Author:      Uri Shomroni
// Date:        12/9/2013
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::onTessellationControlShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType)
{
    bool rc = false;

    // If the name already exist in out vectors:
    if (isManagedObject(shaderName))
    {
        // It means that the object to which this name belonged to was
        // physically deleted and now the name is allocated to the new shader.
        // We need to remove the old object from our vectors:
        removeManagedObjectFromVectors(shaderName);
    }

    // Create an object that will represent the vertex shader:
    apGLShaderObject* pNewShaderObject = new apGLTessellationControlShader(shaderName, shaderType);

    if (NULL != pNewShaderObject)
    {
        // Add the shader object  to this class vectors and maps:
        int vectorId = (int)_shaderObjects.size();
        _shaderObjects.push_back(pNewShaderObject);

        _openGLShaderNameToShaderObjectsVecIndex[shaderName] = vectorId;

        // Register this object in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pNewShaderObject);

        rc = true;
    }

    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onTesselationEvaluationShaderCreation
// Description: Is called when a tessellation control shader is created
// Arguments:   shaderName - The vertex shader OpenGL name.
//              shaderType - The vertex shader object type.
// Author:      Uri Shomroni
// Date:        12/9/2013
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::onTessellationEvaluationShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType)
{
    bool rc = false;

    // If the name already exist in out vectors:
    if (isManagedObject(shaderName))
    {
        // It means that the object to which this name belonged to was
        // physically deleted and now the name is allocated to the new shader.
        // We need to remove the old object from our vectors:
        removeManagedObjectFromVectors(shaderName);
    }

    // Create an object that will represent the vertex shader:
    apGLShaderObject* pNewShaderObject = new apGLTessellationEvaluationShader(shaderName, shaderType);

    if (NULL != pNewShaderObject)
    {
        // Add the shader object  to this class vectors and maps:
        int vectorId = (int)_shaderObjects.size();
        _shaderObjects.push_back(pNewShaderObject);

        _openGLShaderNameToShaderObjectsVecIndex[shaderName] = vectorId;

        // Register this object in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pNewShaderObject);

        rc = true;
    }

    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onGeometryShaderCreation
// Description: Is called when a geometric shader is created
// Arguments:   shaderName - The geometric shader OpenGL name.
//              shaderType - The geometric shader object type.
// Return Val: void
// Author:      Uri Shomroni
// Date:        3/4/2008
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::onGeometryShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType)
{
    bool rc = false;

    // If the name already exist in out vectors:
    if (isManagedObject(shaderName))
    {
        // It means that the object to which this name belonged to was
        // physically deleted and now the name is allocated to the new shader.
        // We need to remove the old object from our vectors:
        removeManagedObjectFromVectors(shaderName);
    }

    // Create an object that will represent the vertex shader:
    apGLShaderObject* pNewShaderObject = new apGLGeometryShader(shaderName, shaderType);

    if (NULL != pNewShaderObject)
    {
        // Add the shader object  to this class vectors and maps:
        int vectorId = (int)_shaderObjects.size();
        _shaderObjects.push_back(pNewShaderObject);

        _openGLShaderNameToShaderObjectsVecIndex[shaderName] = vectorId;

        // Register this object in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pNewShaderObject);

        rc = true;
    }

    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onFragmentShaderCreation
// Description: Is called when a fragment shader is created.
// Arguments:   shaderName - The fragment shader OpenGL name.
//              shaderType - The fragment shader OpenGL type.
// Author:      Yaki Tebeka
// Date:        5/4/2005
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::onFragmentShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType)
{
    bool rc = false;

    // If the name already exist in out vectors:
    if (isManagedObject(shaderName))
    {
        // It means that the object to which this name belonged to was
        // physically deleted and now the name is allocated to the new shader.
        // We need to remove the old object from our vectors:
        removeManagedObjectFromVectors(shaderName);
    }

    // Create an object that will represent the fragment shader:
    apGLShaderObject* pNewShaderObject = new apGLFragmentShader(shaderName, shaderType);

    if (NULL != pNewShaderObject)
    {
        // Add the shader object  to this class vectors and maps:
        int vectorId = (int)_shaderObjects.size();
        _shaderObjects.push_back(pNewShaderObject);

        _openGLShaderNameToShaderObjectsVecIndex[shaderName] = vectorId;

        // Register this object in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pNewShaderObject);

        rc = true;
    }

    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onComputeShaderCreation
// Description: Is called when a tessellation control shader is created
// Arguments:   shaderName - The vertex shader OpenGL name.
//              shaderType - The vertex shader object type.
// Author:      Uri Shomroni
// Date:        12/9/2013
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::onComputeShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType)
{
    bool rc = false;

    // If the name already exist in out vectors:
    if (isManagedObject(shaderName))
    {
        // It means that the object to which this name belonged to was
        // physically deleted and now the name is allocated to the new shader.
        // We need to remove the old object from our vectors:
        removeManagedObjectFromVectors(shaderName);
    }

    // Create an object that will represent the vertex shader:
    apGLShaderObject* pNewShaderObject = new apGLComputeShader(shaderName, shaderType);

    if (NULL != pNewShaderObject)
    {
        // Add the shader object  to this class vectors and maps:
        int vectorId = (int)_shaderObjects.size();
        _shaderObjects.push_back(pNewShaderObject);

        _openGLShaderNameToShaderObjectsVecIndex[shaderName] = vectorId;

        // Register this object in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pNewShaderObject);

        rc = true;
    }

    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onUnsupportedShaderCreation
// Description: Is called when a shader of an unknown or unsupported type is created.
// Arguments:   shaderName - The shader OpenGL name.
//              shaderType - The shader OpenGL type.
// Author:      Uri Shomroni
// Date:        3/6/2013
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::onUnsupportedShaderCreation(GLuint shaderName, apGLShadingObjectType shaderType)
{
    bool rc = false;

    // If the name already exist in out vectors:
    if (isManagedObject(shaderName))
    {
        // It means that the object to which this name belonged to was
        // physically deleted and now the name is allocated to the new shader.
        // We need to remove the old object from our vectors:
        removeManagedObjectFromVectors(shaderName);
    }

    // Create an object that will represent the fragment shader:
    apGLShaderObject* pNewShaderObject = new apGLUnsupportedShader(shaderName, shaderType);

    if (NULL != pNewShaderObject)
    {
        // Add the shader object  to this class vectors and maps:
        int vectorId = (int)_shaderObjects.size();
        _shaderObjects.push_back(pNewShaderObject);

        _openGLShaderNameToShaderObjectsVecIndex[shaderName] = vectorId;

        // Register this object in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pNewShaderObject);

        rc = true;
    }

    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onShaderObjectDeletion
// Description: Is called when a shader object is deleted.
// Arguments:   shaderName - The shader object name.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::onShaderObjectDeletion(GLuint shaderName)
{
    bool retVal = false;

    // Get the shader object vector index:
    int shaderIndex = shadersVecIndex(shaderName);

    if (shaderIndex != -1)
    {
        // Get the shader object representation:
        apGLShaderObject* pShaderObj = _shaderObjects[shaderIndex];

        if (NULL != pShaderObj)
        {
            retVal = true;

            // Mark it for deletion:
            pShaderObj->onShaderMarkedForDeletion();

            // Get the shader type:
            apGLShadingObjectType shaderType = pShaderObj->shaderObjectType();

            // If the shader was "physically" deleted:
            bool wasShaderDeleted = wasShaderPhysicallyDeleted(shaderName, shaderType);

            if (wasShaderDeleted)
            {
                // Remove it from this class vectors:
                removeShaderFromVectors(shaderName);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onShaderSourceCodeSet
// Description: Is called when a shader source code is set.
// Arguments:   shaderName - The shader OpenGL name.
//              nstrings - The amount of input strings.
//              strings - An array containing the input strings.
//              lengths - An array containing the input strings lengths.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/5/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::onShaderSourceCodeSet(GLuint shaderName, GLsizei nstrings, const GLcharARB* const* strings, const GLint* lengths)
{
    bool retVal = false;

    // Get the shader object vector index:
    int shdaerIndex = shadersVecIndex(shaderName);

    if (shdaerIndex != -1)
    {
        // Translate the shader strings array into one string:
        apGLMultiStringParameter sourceCodeAsMultiString(nstrings, strings, lengths);
        gtASCIIString sourceCodeAsString;
        sourceCodeAsMultiString.valueAsASCIIString(sourceCodeAsString);

        // Get shader source version:
        apGLSLVersion glslSourceVersion = AP_GLSL_VERSION_NONE;
        getShaderSourceCodeVersion(sourceCodeAsString, glslSourceVersion);

        // Get the shader object representation:
        apGLShaderObject* pShaderObj = _shaderObjects[shdaerIndex];

        if (NULL != pShaderObj)
        {
            // Set shader source version:
            pShaderObj->setShaderVersion(glslSourceVersion);

            // Log the shader source code update:
            retVal = logShaderSourceCodeUpdate(*pShaderObj, sourceCodeAsString);
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onShaderCompilation
// Description: Is called when a shader object is compiled.
// Arguments:   shaderName - The name of the shader object.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        6/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::onShaderCompilation(GLuint shaderName, bool& wasCompilationSuccessful)
{
    bool retVal = false;

    // Get the shader object vector index:
    int shdaerIndex = shadersVecIndex(shaderName);

    if (shdaerIndex != -1)
    {
        // Get the shader type:
        apGLShadingObjectType shaderType = _shaderObjects[shdaerIndex]->shaderObjectType();

        // Get the shader compilation status:
        wasCompilationSuccessful = wasShaderCompiledSuccessfully(shaderName, shaderType);

        // Get the compile log:
        gtString compilationLog;
        getShaderCompilationLog(shaderName, shaderType, compilationLog);

        // Mark that the shader was compiled:
        _shaderObjects[shdaerIndex]->onShaderCompilation(wasCompilationSuccessful, compilationLog);
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onShaderAttachedToProgram
// Description: Is called when a shader is attached to a program.
// Arguments:   programName - The OpenGL name of the program to which the shader is attached.
//              shaderName - The OpenGL shader name.
// Author:      Yaki Tebeka
// Date:        5/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::onShaderAttachedToProgram(GLuint programName, GLuint shaderName)
{
    bool retVal = false;

    // Get the vector index of the object that represents the program:
    int programsVectorIndex = programsVecIndex(programName);

    if (programsVectorIndex != -1)
    {
        // Get the vector index of the object that represents the shader:
        int shadersVectorIndex = shadersVecIndex(shaderName);

        if (shadersVectorIndex != -1)
        {
            // Mark that the shader is attached to the program:
            const apGLShaderObject* pShaderObj = _shaderObjects[shadersVectorIndex];
            gsGLProgram* pProgramObj = _programObjects[programsVectorIndex];
            retVal = pProgramObj->onShaderObjectAttached(*pShaderObj);

            // See if we need to attach this shader to the holder program:
            if (!pShaderObj->isAttachedToHolderProgram())
            {
                // If the shader is being forced out of the program:
                bool attachToHolderProgram = false;
                osTransferableObjectType shaderTObjType = pShaderObj->type();

                if (shaderTObjType == OS_TOBJ_ID_GL_GEOMETRY_SHADER)
                {
                    // Geometry shaders are forced out in "Stub Geometry" mode:
                    attachToHolderProgram = _areStubGeometryShadersForced;
                }
                else if (shaderTObjType == OS_TOBJ_ID_GL_FRAGMENT_SHADER)
                {
                    // Fragment shaders are forced out in "Stub Geometry" and "Stub Fragment" mode:
                    attachToHolderProgram = _areStubGeometryShadersForced || _areStubFragmentShadersForced;
                }

                if (attachToHolderProgram)
                {
                    // Attach the shader to the holder program, so it won't be deleted if the user
                    // marks it for deletion:
                    apGLShadingObjectType shaderType = pShaderObj->shaderObjectType();
                    GLuint holderProgramName = getShaderHolderProgram(shaderType);
                    bool rc = attachShaderToProgram(holderProgramName, shaderName, shaderType);
                    ((apGLShaderObject*)pShaderObj)->setAttachedToHolderProgram(rc);
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onShaderDetachedFromProgram
// Description: Is called when a shader object is detached from a program object.
// Arguments:   programName - The OpenGL name of the program.
//              shaderName - The OpenGL shader name.
// Author:      Yaki Tebeka
// Date:        5/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::onShaderDetachedFromProgram(GLuint programName, GLuint shaderName)
{
    bool retVal = false;

    // Get the programs vector index of the object that represents the program:
    int programsVectorIndex = programsVecIndex(programName);

    if (programsVectorIndex != -1)
    {
        // Get the shaders vector index of the object that represents the shader:
        int shdaersVectorIndex = shadersVecIndex(shaderName);

        if (shdaersVectorIndex != -1)
        {
            // Get the object that represents the shader:
            const apGLShaderObject* pShaderObj = _shaderObjects[shdaersVectorIndex];

            if (pShaderObj != NULL)
            {
                // Mark that the shader is detached from the program:
                gsGLProgram* pProgramObj = _programObjects[programsVectorIndex];
                retVal = pProgramObj->onShaderObjectDetached(*pShaderObj);

                // Get any info we need before deleting the shader object:
                bool isShaderAttachedToHolderProg = pShaderObj->isAttachedToHolderProgram();

                // If the shader is marked for deletion:
                if (pShaderObj->isMarkedForDeletion())
                {
                    // Get the shader type:
                    apGLShadingObjectType shaderType = pShaderObj->shaderObjectType();

                    // If the shader was "physically" deleted:
                    bool wasShaderDeleted = wasShaderPhysicallyDeleted(shaderName, shaderType);

                    if (wasShaderDeleted)
                    {
                        // Remove it from this class vectors:
                        removeShaderFromVectors(shaderName);
                    }
                }

                // If stub fragment shaders are forced:
                if (retVal && _areStubFragmentShadersForced)
                {
                    // Get the amount of program fragment shaders:
                    int amountOfProgramFS = pProgramObj->amountOfAttachedFragmentShaders();

                    // If there are no more fragment shaders in this program and the stub
                    // fragment shader is still attached to it:
                    if ((amountOfProgramFS < 1) && (pProgramObj->isStubFragmentShaderAttached()))
                    {
                        // Detach the stub fragment shader:
                        bool rc1 = detachStubFragmentShaderFromProgram(*pProgramObj);
                        GT_ASSERT(rc1);

                        // Relink the program:
                        apGLShadingObjectType programType = pProgramObj->programType();
                        bool wasLinkSuccessfull = false;
                        bool rc2 = linkProgram(programName, programType, wasLinkSuccessfull);
                        GT_ASSERT(rc2);
                        GT_ASSERT(wasLinkSuccessfull);
                        retVal = retVal && rc2 && wasLinkSuccessfull;
                    }
                }

                if (retVal)
                {
                    // Mark that the geometry shader was successfully detached.
                    pProgramObj->onStubGeometryShaderDetached();
                }

                // See if we need to detach this shader from the holder program:
                if (isShaderAttachedToHolderProg)
                {
                    // If the shader was being forced out of the program:
                    bool detachFromHolderProgram = false;
                    osTransferableObjectType shaderTObjType = pShaderObj->type();

                    if (shaderTObjType == OS_TOBJ_ID_GL_GEOMETRY_SHADER)
                    {
                        // Geometry shaders are forced out in "Stub Geometry" mode:
                        detachFromHolderProgram = _areStubGeometryShadersForced;
                    }
                    else if (shaderTObjType == OS_TOBJ_ID_GL_FRAGMENT_SHADER)
                    {
                        // Fragment shaders are forced out in "Stub Geometry" and "Stub Fragment" mode:
                        detachFromHolderProgram = _areStubGeometryShadersForced || _areStubFragmentShadersForced;
                    }

                    if (detachFromHolderProgram)
                    {
                        // We need to make sure that this shader is no longer attached to any programs (externally),
                        // Otherwise a workflow that looks like this:
                        // glAttachShader(p1, s); glAttachShader(p2, s); glDeleteShader(s); glDetachShader(p2, s);
                        // would cause the shader to be deleted before it is really supposed to be:
                        // Iterate all the programs:
                        bool foundShader = isShaderAttachedToAnyProgram(shaderName);

                        // If this shader is no longer attached (externally) to any program:
                        if (!foundShader)
                        {
                            // Detach the shader from the holder program, so it will be deleted if the user
                            // marked it for deletion:
                            apGLShadingObjectType shaderType = pShaderObj->shaderObjectType();
                            GLuint holderProgramName = getShaderHolderProgram(shaderType, false);
                            bool rc = detachShaderFromProgram(holderProgramName, shaderName, shaderType);
                            ((apGLShaderObject*)pShaderObj)->setAttachedToHolderProgram(!rc);
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onProgramParameteri
// Description: Called when a program parameter is changed using gl_ProgramParameteriEXT
// Arguments: Same as gl_ProgramParameteriEXT
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        13/5/2008
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::onProgramParameteri(GLuint program, GLenum pname, GLint value)
{
    bool retVal = false;
    int programsVectorIndex = programsVecIndex(program);

    if (programsVectorIndex != -1)
    {
        gsGLProgram* pProgramObj = _programObjects[programsVectorIndex];
        retVal = pProgramObj->onProgramParameteri(pname, value);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::isManagedObject
// Description: Checks if a given name is the name of an OpenGL managed object.
// Arguments:   objectName - The input name.
// Return Val:  bool - true iff the input name belongs to an OpenGL managed object.
// Author:      Yaki Tebeka
// Date:        8/6/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::isManagedObject(GLuint objectName) const
{
    bool retVal = false;

    if (isProgramObject(objectName))
    {
        retVal = true;
    }
    else if (isShaderObject(objectName))
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::isProgramObject
// Description: Inputs an OpenGL managed object name and returns true iff the
//              object is a program object.
// Author:      Yaki Tebeka
// Date:        5/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::isProgramObject(GLuint objectName) const
{
    bool retVal = false;

    // Check it the given object name is a program object name:
    int programIndex = programsVecIndex(objectName);

    if (programIndex != -1)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::isShaderObject
// Description: Inputs an OpenGL managed object name and returns true iff the
//              object is a shader object.
// Author:      Yaki Tebeka
// Date:        5/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::isShaderObject(GLuint objectName) const
{
    bool retVal = false;

    // Check it the given object name is a shader object name:
    int shdaerIndex = shadersVecIndex(objectName);

    if (shdaerIndex != -1)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::isFragmentShader
// Description: Inputs a managed OpenGL object name and returns true iff
//              it is a fragment shader.
// Author:      Yaki Tebeka
// Date:        12/4/2005
// ---------------------------------------------------------------------------
osTransferableObjectType gsProgramsAndShadersMonitor::shaderObjectType(GLuint objectName) const
{
    osTransferableObjectType retVal = OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES;

    // Check it the given object name is a shader object name:
    int vecIndex = shadersVecIndex(objectName);

    if (vecIndex != -1)
    {
        // Get an object that represents the shader object:
        apGLShaderObject* pShaderObj = _shaderObjects[vecIndex];

        if (NULL != pShaderObj)
        {
            // If it is a fragment shader:
            retVal = pShaderObj->type();

            switch (retVal)
            {
                case OS_TOBJ_ID_GL_VERTEX_SHADER:
                case OS_TOBJ_ID_GL_TESSELLATION_CONTROL_SHADER:
                case OS_TOBJ_ID_GL_TESSELLATION_EVALUATION_SHADER:
                case OS_TOBJ_ID_GL_GEOMETRY_SHADER:
                case OS_TOBJ_ID_GL_FRAGMENT_SHADER:
                case OS_TOBJ_ID_GL_COMPUTE_SHADER:
                case OS_TOBJ_ID_GL_UNSUPPORTED_SHADER:
                    // Accepted types, continue as normal:
                    break;

                default:
                    // Unexpected value!
                    GT_ASSERT(false);
                    retVal = OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES;
                    break;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::amountOfLivingProgramObjects
// Description: Returns the number of Shading Programs that were not yet marked
//              for deletion.
// Author:      Uri Shomroni
// Date:        8/8/2010
// ---------------------------------------------------------------------------
int gsProgramsAndShadersMonitor::amountOfLivingProgramObjects() const
{
    int retVal = 0;

    // Iterate all programs:
    int amountOfPrograms = (int)_programObjects.size();

    for (int i = 0; i < amountOfPrograms; i++)
    {
        // Sanity check:
        gsGLProgram* pProgram = _programObjects[i];
        GT_IF_WITH_ASSERT(pProgram != NULL)
        {
            // If the program was not marked for deletion:
            if (!pProgram->isMarkedForDeletion())
            {
                // Add it to the count:
                retVal++;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::amountOfLivingShaderObjects
// Description: Returns the number of shaders that were not yet marked for deletion.
// Author:      Uri Shomroni
// Date:        8/8/2010
// ---------------------------------------------------------------------------
int gsProgramsAndShadersMonitor::amountOfLivingShaderObjects() const
{
    int retVal = 0;

    // Iterate all shaders:
    int amountOfShaders = (int)_shaderObjects.size();

    for (int i = 0; i < amountOfShaders; i++)
    {
        // Sanity check:
        apGLShaderObject* pShader = _shaderObjects[i];
        GT_IF_WITH_ASSERT(pShader != NULL)
        {
            // If the shader was not marked for deletion:
            if (!pShader->isMarkedForDeletion())
            {
                // Add it to the count:
                retVal++;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::programObjectName
// Description: Inputs a program object id (in this class vectors) and returns
//              its name, or 0 if the id is out of index.
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
GLuint gsProgramsAndShadersMonitor::programObjectName(int programId) const
{
    GLuint retVal = 0;

    // Index range check:
    int amountOfPrograms = (int)_programObjects.size();

    if ((0 <= programId) && (programId < amountOfPrograms))
    {
        retVal = _programObjects[programId]->programName();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::shaderObjectName
// Description: Inputs a shader object id (in this class vectors) and returns
//              its name, or 0 if the id is out of index.
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
GLuint gsProgramsAndShadersMonitor::shaderObjectName(int shaderId) const
{
    GLuint retVal = 0;

    // Index range check:
    int amountOfShaders = (int)_shaderObjects.size();

    if ((0 <= shaderId) && (shaderId < amountOfShaders))
    {
        retVal = _shaderObjects[shaderId]->shaderName();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::programObjectDetails
// Description: Inputs a program object name and returns its details.
// Arguments:   programName - The program object name.
// Return Val:  const apGLProgram* - Will get the program object details, or NULL
//                                   if the index is illegal.
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
const apGLProgram* gsProgramsAndShadersMonitor::programObjectDetails(GLuint programName) const
{
    const gsGLProgram* retVal = NULL;

    // Get the program index in the _programObjects vector:
    int programIndex = programsVecIndex(programName);

    if (programIndex != -1)
    {
        retVal = _programObjects[programIndex];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::shaderObjectDetails
// Description: Inputs a shader object name and returns its details.
// Arguments:   shaderName - The shader object name.
// Return Val:  const apGLShaderObject* - Will get the shader object details or
//                                        NULL if the index is invalid.
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
const apGLShaderObject* gsProgramsAndShadersMonitor::shaderObjectDetails(GLuint shaderName) const
{
    const apGLShaderObject* retVal = NULL;

    // Get the shader index in the _shaderObjects vector:
    int shaderIndex =  shadersVecIndex(shaderName);

    if (shaderIndex != -1)
    {
        retVal = _shaderObjects[shaderIndex];
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::getProgramObject
// Description: Returns the (modifiable) gsGLProgram representing the program
//              named programName. If it is not a valid program name (eg it is
//              a shader), returns null.
// Author:      Uri Shomroni
// Date:        17/6/2008
// ---------------------------------------------------------------------------
gsGLProgram* gsProgramsAndShadersMonitor::getProgramObject(GLuint programName)
{
    gsGLProgram* retVal = NULL;

    int programIndex = programsVecIndex(programName);

    if (programIndex != -1)
    {
        retVal = _programObjects[programIndex];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::applyForcedStubGeometryShader
// Description: Force the programs that are monitored by this class to use a
//              simple stub geometry shader.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/4/2008
// Implementation notes:
//   To force the stub fragment shader:
//   a. Iterate the programs.
//   b. For each program, if it has geometry shaders, we will detach them
//      from the program and attach our simple fragment shader instead of their
//      own fragment shaders (to avoid missing varying variables).
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::applyForcedStubGeometryShader()
{
    bool retVal = true;

    // Verify that stub geometry shaders are not already forced:
    if (!_areStubGeometryShadersForced)
    {
        // Clear OpenGL errors (if there were any):
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        (void) gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        // Update the uniforms values of all the linked programs:
        // (This will enable us to answer uniform queries while in force stub GS mode):
        // but only do it if we were not in a force mode to begin with:
        if (!_areStubFragmentShadersForced && !_areStubGeometryShadersForced)
        {
            _programsActiveUniforms.updateContextDataSnapshot();
            // Iterate the existing programs:
            int amountOfPrograms = (int)_programObjects.size();

            for (int i = 0; i < amountOfPrograms; i++)
            {
                // Get the current program:
                gsGLProgram* pCurrentProgram = _programObjects[i];

                if (pCurrentProgram != NULL)
                {
                    // If the program was linked successfully:
                    if (pCurrentProgram->isProgramLinkedSuccesfully())
                    {
                        // Detach the geometry shaders and apply the stub fragment shader on it:
                        bool rc1 = applyForcedStubGeometryShaderToProgram(*pCurrentProgram);
                        GT_IF_WITH_ASSERT(rc1)
                        {
                            // Get the program uniform values:
                            GLuint programName = pCurrentProgram->programName();
                            const apGLItemsCollection* programUniforms = nullptr;
                            bool rc2 = _programsActiveUniforms.getProgramActiveUniforms(programName, programUniforms);
                            GT_IF_WITH_ASSERT(rc2 && (nullptr != programUniforms))
                            {
                                // This change will be undone by our caller after the whole loop runs:
                                bool rcAct = activateProgramForUpdate(programName);
                                GT_IF_WITH_ASSERT(rcAct)
                                {
                                    // Restore the program uniform values:
                                    bool rc3 = _programsActiveUniforms.restoreProgramUniformValues(programName, *programUniforms);
                                    retVal = retVal && rc3;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Mark that the stub geometry shader are forced:
        _areStubGeometryShadersForced = true;

        // Verify that we didn't produce OpenGL errors:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        GLenum errorEnum = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);
        GT_ASSERT(errorEnum == GL_NO_ERROR);
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::applyForcedStubGeometryShaderToProgram
// Description: Applies the forces stub geometry shader on a program that was
//              successfully linked.
// Arguments:   program - The input program object.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/4/2008
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::applyForcedStubGeometryShaderToProgram(gsGLProgram& program)
{
    bool retVal = false;

    // Get the amount of geometry shaders attached to the current program:
    int geometryShadersAmount = program.amountOfAttachedGeometryShaders();

    // If there are attached geometry shaders:
    if (geometryShadersAmount > 0)
    {
        // If the stub fragment shader is not already attached to the program:
        bool rc1 = true;

        if (!(program.isStubFragmentShaderAttached()))
        {
            // Attach our stub fragment shader to it:
            rc1 = attachStubFragmentShaderToProgram(program);
            GT_ASSERT(rc1);
        }

        // Attach this program's fragment and geometry shaders to the holder program:
        bool rc2 = attachProgramGeometryShadersToHolderProgram(program);
        GT_ASSERT(rc2);
        bool rc3 = attachProgramFragmentShadersToHolderProgram(program);
        GT_ASSERT(rc3);

        // Detach geometry shaders that are attached to the current program:
        bool rc4 = detachProgramGeometryShaders(program);
        GT_ASSERT(rc4);
        bool rc5 = detachProgramFragmentShaders(program);
        GT_ASSERT(rc5);

        // Relink the program:
        GLuint programName = program.programName();
        apGLShadingObjectType programType = program.programType();
        bool wasLinkSuccessful = false;
        bool rc6 = linkProgram(programName, programType, wasLinkSuccessful);
        GT_ASSERT(rc6);
        GT_ASSERT(wasLinkSuccessful);

        retVal = rc1 && rc2 && rc3 && rc4 && rc5 && rc6 && wasLinkSuccessful;
    }
    else
    {
        // If there are no Geometry shaders, still apply the stub fragment shader
        retVal = applyForcedStubFragmentShaderToProgram(program);
    }

    GT_IF_WITH_ASSERT(retVal)
    {
        // Mark the success of detaching the geometry shader and make sure the
        // program object doesn't "think" it's only in stub fragment shader mode.
        program.onStubFragmentShaderDetached();
        program.onStubGeometryShaderAttached();
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::cancelForcedStubGeometryShader
// Description: Cancel the effect of applyForcedStubGeometryShader() - let
//              the programs use their real fragment shaders.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/4/2008
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::cancelForcedStubGeometryShader()
{
    bool retVal = true;

    // Verify that stub fragment shaders are forced:
    if (_areStubGeometryShadersForced)
    {
        // Clear OpenGL errors (if there were any):
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        (void) gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        // Iterate the existing programs:
        int amountOfPrograms = (int)_programObjects.size();

        for (int i = 0; i < amountOfPrograms; i++)
        {
            // Get the current program:
            gsGLProgram* pCurrentProgram = _programObjects[i];

            if (pCurrentProgram != NULL)
            {
                // Make sure the program doesn't know it has the stub fragment shader attached
                // to it, if it does:
                pCurrentProgram->onStubFragmentShaderDetached();

                // If the stub fragment shader is attached to the program:
                if (pCurrentProgram->isStubGeometryShaderAttached())
                {
                    // Attach back the program "original" fragment and geometry shaders:
                    bool rc1 = attachBackProgramGeometryShaders(*pCurrentProgram);
                    bool rc2 = attachBackProgramFragmentShaders(*pCurrentProgram);

                    // Detach the program's shaders from the holder program:
                    bool rc3 = detachProgramGeometryShadersFromHolderProgram(*pCurrentProgram);
                    bool rc4 = detachProgramFragmentShadersFromHolderProgram(*pCurrentProgram);

                    // Detach the stub fragment shader:
                    bool rc5 = detachStubFragmentShaderFromProgram(*pCurrentProgram);

                    // Relink the program:
                    GLuint programName = pCurrentProgram->programName();
                    apGLShadingObjectType programType = pCurrentProgram->programType();
                    bool wasLinkSuccessfull = false;
                    bool rc6 = linkProgram(programName, programType, wasLinkSuccessfull);
                    GT_ASSERT(rc6);
                    GT_ASSERT(wasLinkSuccessfull);

                    // Mark the program as restored from stub FS:
                    pCurrentProgram->setProgramRestoredFromStubFS(true);

                    retVal = retVal && rc1 && rc2 && rc3 && rc4 && rc5 && rc6 && wasLinkSuccessfull;
                }
            }
        }

        if (retVal)
        {
            // Verify that we didn't produce OpenGL errors:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
            GLenum errorEnum = gs_stat_realFunctionPointers.glGetError();
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

            if (errorEnum == GL_NO_ERROR)
            {
                // Mark the we canceled the stub Geometry Shader forcing:
                _areStubGeometryShadersForced = false;
            }
            else
            {
                retVal = false;
                GT_ASSERT(0);
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::applyForcedStubFragmentShader
// Description: Force the programs that are monitored by this class to use a
//              simple stub fragment shader.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/4/2005
// Implementation notes:
//   To force the stub fragment shader:
//   a. Iterate the programs.
//   b. For each program, if it has fragment shaders, we will detach them
//      from the program and attach our simple fragment shader instead.
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::applyForcedStubFragmentShader()
{
    bool retVal = true;

    // Verify that stub fragment shaders are not already forced:
    if (!_areStubFragmentShadersForced)
    {
        // Clear OpenGL errors (if there were any):
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        (void) gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        // Update the uniforms values of all the linked programs:
        // (This will enable us to answer uniform queries while in force stub FS mode),
        // but only do it if we were not in a force mode to begin with:
        if (!_areStubFragmentShadersForced && !_areStubGeometryShadersForced)
        {
            _programsActiveUniforms.updateContextDataSnapshot();

            // Iterate the existing programs:
            int amountOfPrograms = (int)_programObjects.size();

            for (int i = 0; i < amountOfPrograms; i++)
            {
                // Get the current program:
                gsGLProgram* pCurrentProgram = _programObjects[i];

                if (NULL != pCurrentProgram)
                {
                    // If the program was linked successfully:
                    if (pCurrentProgram->isProgramLinkedSuccesfully())
                    {
                        // Apply the forces stub fragment shaders on it:
                        bool rc1 = applyForcedStubFragmentShaderToProgram(*pCurrentProgram);
                        GT_IF_WITH_ASSERT(rc1)
                        {
                            // Get the program uniform values:
                            GLuint programName = pCurrentProgram->programName();
                            const apGLItemsCollection* programUniforms = nullptr;
                            bool rc2 = _programsActiveUniforms.getProgramActiveUniforms(programName, programUniforms);
                            GT_IF_WITH_ASSERT(rc2 && (nullptr != programUniforms))
                            {
                                // This change will be undone by our caller after the whole loop runs:
                                bool rcAct = activateProgramForUpdate(programName);
                                GT_IF_WITH_ASSERT(rcAct)
                                {
                                    // Restore the program uniform values:
                                    bool rc3 = _programsActiveUniforms.restoreProgramUniformValues(programName, *programUniforms);
                                    retVal = retVal && rc3;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Mark that the stub fragment shader are forced:
        _areStubFragmentShadersForced = true;

        // Verify that we didn't produce OpenGL errors:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        GLenum errorEnum = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);
        GT_ASSERT(errorEnum == GL_NO_ERROR);
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::applyForcedStubFragmentShaderToProgram
// Description: Applies the forces stub fragment shader on a program that was
//              successfully linked.
// Arguments:   program - The input program object.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/6/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::applyForcedStubFragmentShaderToProgram(gsGLProgram& program)
{
    bool retVal = false;

    // Get the amount of fragment shaders attached to the current program:
    int fragmentShadersAmount = program.amountOfAttachedFragmentShaders();

    // If there are attached fragment shaders:
    if (fragmentShadersAmount > 0)
    {
        // If the stub fragment shader is not already attached to the program:
        bool rc1 = true;

        if (!(program.isStubFragmentShaderAttached()))
        {
            // Attach our stub fragment shader to it:
            rc1 = attachStubFragmentShaderToProgram(program);
            GT_ASSERT(rc1);
        }

        // Attach the program's fragment shaders to the holder program:
        bool rc2 = attachProgramFragmentShadersToHolderProgram(program);

        // Detach fragment shaders that are attached to the current program:
        bool rc3 = detachProgramFragmentShaders(program);
        GT_ASSERT(rc3);

        // Relink the program:
        GLuint programName = program.programName();
        apGLShadingObjectType programType = program.programType();
        bool wasLinkSuccessful = false;
        bool rc4 = linkProgram(programName, programType, wasLinkSuccessful);
        GT_ASSERT(rc4);
        GT_ASSERT(wasLinkSuccessful);

        retVal = rc1 && rc2 && rc3 && rc4 && wasLinkSuccessful;
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::cancelForcedStubFragmentShader
// Description: Cancel the effect of applyForcedStubFragmentShader() - let
//              the programs use their real fragment shaders.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::cancelForcedStubFragmentShader()
{
    bool retVal = true;

    // Verify that stub fragment shaders are forced:
    if (_areStubFragmentShadersForced)
    {
        // Clear OpenGL errors (if there were any):
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        (void) gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        // Iterate the existing programs:
        int amountOfPrograms = (int)_programObjects.size();

        for (int i = 0; i < amountOfPrograms; i++)
        {
            // Get the current program:
            gsGLProgram* pCurrentProgram = _programObjects[i];

            if (NULL != pCurrentProgram)
            {
                // If the stub fragment shader is attached to the program:
                if (pCurrentProgram->isStubFragmentShaderAttached())
                {
                    // Attach back the program "original" fragment shaders:
                    bool rc1 = attachBackProgramFragmentShaders(*pCurrentProgram);

                    // Attach the fragment shader(s) to the holder program:
                    bool rc2 = attachProgramFragmentShadersToHolderProgram(*pCurrentProgram);

                    // Detach the stub fragment shader:
                    bool rc3 = detachStubFragmentShaderFromProgram(*pCurrentProgram);

                    // Relink the program:
                    GLuint programName = pCurrentProgram->programName();
                    apGLShadingObjectType programType = pCurrentProgram->programType();
                    bool wasLinkSuccessfull = false;
                    bool rc4 = linkProgram(programName, programType, wasLinkSuccessfull);
                    GT_ASSERT(rc4);
                    GT_ASSERT(wasLinkSuccessfull);
                    retVal = retVal && rc1 && rc2 && rc3 && rc4 && wasLinkSuccessfull;

                    if (retVal)
                    {
                        // Mark the program as restored from stub FS:
                        pCurrentProgram->setProgramRestoredFromStubFS(true);
                    }
                }
            }
        }

        if (retVal)
        {
            // Verify that we didn't produce OpenGL errors:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
            GLenum errorEnum = gs_stat_realFunctionPointers.glGetError();
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

            if (errorEnum == GL_NO_ERROR)
            {
                // Mark the we canceled the stub Fragment Shader forcing:
                _areStubFragmentShadersForced = false;
            }
            else
            {
                retVal = false;
                GT_ASSERT(0);
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::stubFragmentShaderSources
// Description: Replaces all the fragment shaders' sources with the stub one.
//              This is needed in the iPhone, since there are a few problems
//              that are caused by attaching / detaching shaders.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        10/9/2009
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::stubFragmentShaderSources(bool ignoreMultipleCalls)
{
    bool retVal = true;

    // If this mode is already on, do nothing and return true:
    if (!(_areFragmentShaderSourcesStubbed && ignoreMultipleCalls))
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glShaderSource);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCompileShader);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);

        // Iterate all the shaders:
        int numberofShaders = (int)_shaderObjects.size();

        for (int i = 0; i < numberofShaders; i++)
        {
            apGLShaderObject* pCurrentShader = _shaderObjects[i];

            if (pCurrentShader != NULL)
            {
                // If this shader is a fragment shader:
                if (pCurrentShader->type() == OS_TOBJ_ID_GL_FRAGMENT_SHADER)
                {
                    // Set its source code to the stub code and compile it:
                    GLuint shaderName = pCurrentShader->shaderName();
                    gs_stat_realFunctionPointers.glShaderSource(shaderName, 1, &stat_stubVertexShaderSourceCode, NULL);
                    gs_stat_realFunctionPointers.glCompileShader(shaderName);

                    GLint wasCompiled = GL_FALSE;
                    gs_stat_realFunctionPointers.glGetShaderiv(shaderName, GL_COMPILE_STATUS, &wasCompiled);

                    GT_ASSERT(wasCompiled == GL_TRUE);
                    retVal = retVal && (wasCompiled == GL_TRUE);
                }
            }
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCompileShader);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glShaderSource);

        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glLinkProgram);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        // Relink all programs that need linking:
        int numberOfPrograms = (int)_programObjects.size();

        for (int i = 0; i < numberOfPrograms; i++)
        {
            gsGLProgram* pCurrentProg = _programObjects[i];

            if (pCurrentProg != NULL)
            {
                // If a program was not linked properly, we do not need to link it here:
                if (pCurrentProg->isProgramLinkedSuccesfully())
                {
                    GLuint progName = pCurrentProg->programName();
                    gs_stat_realFunctionPointers.glLinkProgram(progName);

                    GLint wasLinked = GL_FALSE;
                    gs_stat_realFunctionPointers.glGetProgramiv(progName, GL_LINK_STATUS, &wasLinked);
                    GT_ASSERT(wasLinked == GL_TRUE);
                    retVal = retVal && (wasLinked == GL_TRUE);
                }
            }
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glLinkProgram);

        // Mark the change:
        _areFragmentShaderSourcesStubbed = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::restoreStubbedFragmentShaderSources
// Description: Restores the sources from being stubbed
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        10/9/2009
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::restoreStubbedFragmentShaderSources()
{
    bool retVal = true;

    // If this mode is already off, do nothing and return true:
    if (_areFragmentShaderSourcesStubbed)
    {
        int numberofShaders = (int)_shaderObjects.size();

        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glShaderSource);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCompileShader);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);

        // Iterate all the shaders:
        for (int i = 0; i < numberofShaders; i++)
        {
            apGLShaderObject* pCurrentShader = _shaderObjects[i];

            if (pCurrentShader != NULL)
            {
                // If this shader is a fragment shader:
                if (pCurrentShader->type() == OS_TOBJ_ID_GL_FRAGMENT_SHADER)
                {
                    // Get the original (or user-modified) source from the file:
                    gtString shaderRealSource;
                    bool rcSource = getShaderSourceCodeFromFile(*pCurrentShader, shaderRealSource);

                    if (rcSource)
                    {
                        // Set its source code to the stub code and compile it:
                        GLuint shaderName = pCurrentShader->shaderName();
                        const char* pSourceCode = shaderRealSource.asASCIICharArray();
                        gs_stat_realFunctionPointers.glShaderSource(shaderName, 1, &pSourceCode, NULL);
                        gs_stat_realFunctionPointers.glCompileShader(shaderName);

                        GLint wasCompiled = GL_FALSE;
                        gs_stat_realFunctionPointers.glGetShaderiv(shaderName, GL_COMPILE_STATUS, &wasCompiled);

                        GT_ASSERT(wasCompiled == GL_TRUE);
                        retVal = retVal && (wasCompiled == GL_TRUE);
                    }
                    else
                    {
                        GT_ASSERT(rcSource);
                        retVal = false;
                    }
                }
            }
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCompileShader);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glShaderSource);

        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glLinkProgram);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        // Relink all programs that need linking:
        int numberOfPrograms = (int)_programObjects.size();

        for (int i = 0; i < numberOfPrograms; i++)
        {
            gsGLProgram* pCurrentProg = _programObjects[i];

            if (pCurrentProg != NULL)
            {
                // If a program was not linked properly, we do not need to link it here:
                if (pCurrentProg->isProgramLinkedSuccesfully())
                {
                    GLuint progName = pCurrentProg->programName();
                    gs_stat_realFunctionPointers.glLinkProgram(progName);

                    GLint wasLinked = GL_FALSE;
                    gs_stat_realFunctionPointers.glGetProgramiv(progName, GL_LINK_STATUS, &wasLinked);
                    GT_ASSERT(wasLinked == GL_TRUE);
                    retVal = retVal && (wasLinked == GL_TRUE);
                }
            }
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glLinkProgram);

        // Mark the change:
        _areFragmentShaderSourcesStubbed = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::markShaderObjectSourceCodeAsForced
// Description:
//   Marks a given shader source code as "forced".
//   I.E: From this moment on, the debugged program will not be able to change
//        the shader's source code.
// Arguments: shaderName - The given shader OpenGL name.
//            isSourceCodeForced - true iff the shader source code is forced.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/11/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::markShaderObjectSourceCodeAsForced(GLuint shaderName, bool isSourceCodeForced)
{
    bool retVal = false;

    // Get the shader object vector index:
    int shaderIndex = shadersVecIndex(shaderName);

    if (shaderIndex != -1)
    {
        // Get the shader object representation:
        apGLShaderObject* pShaderObj = _shaderObjects[shaderIndex];

        if (NULL != pShaderObj)
        {
            // Mark its source code as "forced":
            pShaderObj->markSourceCodeAsForced(isSourceCodeForced);

            // Mark that the shader binary is not update:
            pShaderObj->onShaderSourceCodeMarkedAsForced();

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::setShaderObjectSourceCode
// Description: Sets / replaces the source code of a given shader.
// Arguments: shaderName - The given shader name.
//            inputSourceCodeFilePath - The path of a file that contains the new shaders
//                                      source code.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/11/2005
// Implementation notes:
//   We assume that the current thread current context is the context that this
//   class monitors.
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::setShaderObjectSourceCode(GLuint shaderName, const osFilePath& inputSourceCodeFilePath)
{
    bool retVal = false;

    // Open the input source code file:
    osFile inputSourceCodeFile;
    bool rc = inputSourceCodeFile.open(inputSourceCodeFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);
    GT_IF_WITH_ASSERT(rc)
    {
        // Load the input source code into a string:
        gtASCIIString inputSourceCode;
        rc = inputSourceCodeFile.readIntoString(inputSourceCode);

        // Close the file:
        inputSourceCodeFile.close();

        // If we managed to read the source code:
        GT_IF_WITH_ASSERT(rc)
        {
            // Set the shader source code:
            retVal = setShaderSourceCode(shaderName, inputSourceCode);
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::compileShaderObject
// Description: Compiles a given shader object.
// Arguments: shaderName - The OpenGL name of the given shader object.
//            wasCompilationSuccessful - will get true iff the compilation was successfully.
//            compilationLog - Will get the compilation log.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/11/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::compileShaderObject(GLuint shaderName, bool& wasCompilationSuccessful, gtString& compilationLog)
{
    bool retVal = false;

    // Get the shader object vector index:
    int shdaerIndex = shadersVecIndex(shaderName);
    GT_IF_WITH_ASSERT(shdaerIndex != -1)
    {
        // Get the shader object representation:
        apGLShaderObject* pShaderObj = _shaderObjects[shdaerIndex];
        GT_IF_WITH_ASSERT(pShaderObj != NULL)
        {
            // Will get 1 if the shader was compiled successfully:
            GLint isShaderCompiledSuccessfully = 0;

            // Get the shader object type:
            apGLShadingObjectType shaderType = pShaderObj->shaderObjectType();

            // If this is an GL_ARB_shader_objects:
            if ((shaderType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
            {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                // Compile the shader:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCompileShaderARB);
                gs_stat_realFunctionPointers.glCompileShaderARB(shaderName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCompileShaderARB);

                // Check the shader compilation status:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
                gs_stat_realFunctionPointers.glGetObjectParameterivARB(shaderName, GL_OBJECT_COMPILE_STATUS_ARB, &isShaderCompiledSuccessfully);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#else
                // Compile the shader:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCompileShaderARB);
                _glCompileShaderARB(shaderName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCompileShaderARB);

                // Check the shader compilation status:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
                _glGetObjectParameterivARB(shaderName, GL_OBJECT_COMPILE_STATUS_ARB, &isShaderCompiledSuccessfully);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#endif

                retVal = true;
            }
            else if ((shaderType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
            {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                // Compile the shader object:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCompileShader);
                gs_stat_realFunctionPointers.glCompileShader(shaderName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCompileShader);

                // Check the shader compilation status:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
                gs_stat_realFunctionPointers.glGetShaderiv(shaderName, GL_COMPILE_STATUS, &isShaderCompiledSuccessfully);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
#else
                // Compile the shader object:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCompileShader);
                _glCompileShader(shaderName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCompileShader);

                // Check the shader compilation status:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
                _glGetShaderiv(shaderName, GL_COMPILE_STATUS, &isShaderCompiledSuccessfully);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
#endif

                retVal = true;
            }

            // If the shader was compiled:
            GT_IF_WITH_ASSERT(retVal)
            {
                // Cast the compilation status to a bool:
                wasCompilationSuccessful = (isShaderCompiledSuccessfully != 0);

                // Get the compilation log:
                getShaderCompilationLog(shaderName, shaderType, compilationLog);

                // Log the shader compilation status and log:
                pShaderObj->onShaderCompilation(wasCompilationSuccessful, compilationLog);
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::linkProgramObject
// Description: Links a given program object.
// Arguments: programName - The given program object name.
//            wasLinkSuccessful - Will get true iff the link was successful.
//            linkLog - Will get the link log.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/11/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::linkProgramObject(GLuint programName, bool& wasLinkSuccessful, gtString& linkLog, GLuint activeProgramName)
{
    bool retVal = false;
    wasLinkSuccessful = false;

    // Get the program object vector index:
    int programsVectorIndex = programsVecIndex(programName);

    if (programsVectorIndex != -1)
    {
        // Get the object that represents the program:
        gsGLProgram* pProgramObj = _programObjects[programsVectorIndex];
        GT_IF_WITH_ASSERT(pProgramObj != NULL)
        {
            // Get the program type:
            apGLShadingObjectType programType = pProgramObj->programType();

            // Get the program uniforms:
            const apGLItemsCollection* programUniforms = nullptr;
            bool gotUniforms = _programsActiveUniforms.getProgramActiveUniforms(programName, programUniforms);

            // Link the program:
            bool rc = linkProgram(programName, programType, wasLinkSuccessful);

            if (rc)
            {
                // Mark that the program was linked:
                bool rc1 = onProgramLinked(programName, wasLinkSuccessful);

                // If we managed to get the uniform values before this change, restore them
                // (note that we cannot use gsGLProgram::setProgramRestoredFromStubFS since
                // onProgramLinked copies the "garbage" values that are put in the uniforms
                // after relinking it):
                if (gotUniforms && (nullptr != programUniforms))
                {
                    // To set program uniforms, we need to activate it:
                    bool isActiveProgram = (programName == activeProgramName);

                    if (!isActiveProgram)
                    {
                        isActiveProgram = activateProgramForUpdate(programName);
                    }

                    GT_IF_WITH_ASSERT(isActiveProgram)
                    {
                        // Restore the values:
                        _programsActiveUniforms.restoreProgramUniformValues(programName, *programUniforms);

                        // Update the values back into our vectors, as the "garbage" values might be read otherwise.
                        _programsActiveUniforms.updateContextDataSnapshot();

                        if (programName != activeProgramName)
                        {
                            bool rcRest = restoreActiveProgram(activeProgramName);
                            GT_ASSERT(rcRest);
                        }
                    }
                }

                // Get the program link log:
                linkLog = pProgramObj->programLinkLog();

                retVal = rc1;
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::validateProgramObject
// Description: Validates a given program object.
// Arguments: programName - The given program object name.
//            wasValidationSuccessful - Will get true iff the validation was successful.
//            validationLog - Will get the validation log.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/11/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::validateProgramObject(GLuint programName, bool& wasValidationSuccessful, gtString& validationLog)
{
    bool retVal = false;

    // Get the program object vector index:
    int programsVectorIndex = programsVecIndex(programName);

    if (programsVectorIndex != -1)
    {
        // Get the object that represents the program:
        gsGLProgram* pProgramObj = _programObjects[programsVectorIndex];
        GT_IF_WITH_ASSERT(pProgramObj != NULL)
        {
            // Get the program type:
            apGLShadingObjectType programType = pProgramObj->programType();

            // Validate the program:
            bool rc = validateProgram(programName, programType, wasValidationSuccessful);

            if (rc)
            {
                // Get the program validation log:
                validationLog.makeEmpty();
                getProgramInfoLog(programName, programType, validationLog);

                retVal = true;
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::activateProgramForUpdate
// Description: Called to make a program active to apply certain changes to it.
//              (like gsTexturesMonitor::bindTextureForUpdate)
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/12/2009
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::activateProgramForUpdate(GLuint programName)
{
    bool retVal = false;

    // Get the program type:
    apGLShadingObjectType progType = AP_GL_2_0_SHADING_OBJECT;

    if (programName != 0)
    {
        const apGLProgram* pProgram = programObjectDetails(programName);
        GT_IF_WITH_ASSERT(pProgram != NULL)
        {
            progType = pProgram->programType();
        }
    }
    else // programName == 0
    {
        // Get the best version that works:
        if (_isOpenGL20Supported)
        {
            progType = AP_GL_2_0_SHADING_OBJECT;
        }
        else if (_isARBShaderObjectsExtSupported)
        {
            progType = AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT;
        }
    }

    // Activate the program using the appropriate function:
    if (progType == AP_GL_2_0_SHADING_OBJECT)
    {
        GT_IF_WITH_ASSERT(_isOpenGL20Supported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUseProgram);
            gs_stat_realFunctionPointers.glUseProgram(programName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUseProgram);
#else // !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUseProgram);
            _glUseProgram(programName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUseProgram);
#endif // ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            retVal = true;
        }
    }
    else if (progType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT)
    {
        GT_IF_WITH_ASSERT(_isARBShaderObjectsExtSupported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUseProgramObjectARB);
            gs_stat_realFunctionPointers.glUseProgramObjectARB(programName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUseProgramObjectARB);
#else // !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUseProgramObjectARB);
            _glUseProgramObjectARB(programName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUseProgramObjectARB);
#endif // ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::restoreActiveProgram
// Description: Called to restore the active program after using activateProgramForUpdate
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/12/2009
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::restoreActiveProgram(GLuint activeProgramName)
{
    // The operation is the same as activating a program for the update, so we do it like that:
    bool retVal = activateProgramForUpdate(activeProgramName);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::clearContextDataSnapshot
// Description: Clears context data snapshot that is held by this class.
// Author:      Yaki Tebeka
// Date:        15/6/2005
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::clearContextDataSnapshot()
{
    // Clear active uniforms snapshot data:
    _programsActiveUniforms.clearContextDataSnapshot();
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::updateContextDataSnapshot
// Description: Update context data snapshot that is held by this class.
// Author:      Yaki Tebeka
// Date:        8/6/2005
// Implementation notes:
//   a. Look for programs and shaders that were physically deleted.
//   b. Update also the uniforms snapshot data.
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::updateContextDataSnapshot()
{
    bool retVal = true;

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateProgramsAnsShadersDataStarted, OS_DEBUG_LOG_DEBUG);

    // Iterate the program objects:
    int amountOfPrograms = (int)_programObjects.size();
    gtVector <gsGLProgram*> physicallyDeletedPrograms;

    for (int i = 0; i < amountOfPrograms; i++)
    {
        // Get the object that represents the current program:
        gsGLProgram* pCurProgramObj = _programObjects[i];

        if (pCurProgramObj)
        {
            // If it was marked for deletion:
            if (pCurProgramObj->isMarkedForDeletion())
            {
                // Get the program name and type:
                GLuint curProgamName = pCurProgramObj->programName();
                apGLShadingObjectType curProgType = pCurProgramObj->programType();

                // If the program was physically deleted:
                bool wasProgDeleted = wasProgramPhysicallyDeleted(curProgamName, curProgType);

                if (wasProgDeleted)
                {
                    physicallyDeletedPrograms.push_back(pCurProgramObj);
                }
            }
        }
    }

    // If any programs were deleted, remove them:
    int numberOfDeletedPrograms = (int)physicallyDeletedPrograms.size();

    for (int i = 0; i < numberOfDeletedPrograms; i++)
    {
        gsGLProgram* pCurrentProg = physicallyDeletedPrograms[i];
        GT_IF_WITH_ASSERT(NULL != pCurrentProg)
        {
            onProgramPhysicalDeletion(pCurrentProg->programName(), pCurrentProg);
            physicallyDeletedPrograms[i] = NULL;
        }
    }

    // If stub fragment shaders / geometry shaders are not forced:
    if (!_areStubFragmentShadersForced && !_areStubGeometryShadersForced)
    {
        // Update also the uniforms snapshot data:
        _programsActiveUniforms.updateContextDataSnapshot();
    }

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateProgramsAnsShadersDataEnded, OS_DEBUG_LOG_DEBUG);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::programsVecIndex
// Description:
//  Inputs a program OpenGL name and outputs its _programObjects vector index,
//  or -1 if it is not registered in _programObjects.
//
// Author:      Yaki Tebeka
// Date:        5/4/2005
// ---------------------------------------------------------------------------
int gsProgramsAndShadersMonitor::programsVecIndex(GLuint programName) const
{
    int retVal = -1;

    // Look for the program name in the _openGLProgramNameToProgramObjectsVecIndex map:
    gtMap<GLuint, int>::const_iterator iter = _openGLProgramNameToProgramObjectsVecIndex.find(programName);

    // If we found the program id:
    if (iter != _openGLProgramNameToProgramObjectsVecIndex.end())
    {
        // Return the program id:
        retVal = (*iter).second;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::shadersVecIndex
// Description:
//  Inputs a shader OpenGL name and outputs its _shaderObjects vector index,
//  or -1 if it is not registered in _shaderObjects.
//
// Author:      Yaki Tebeka
// Date:        5/4/2005
// ---------------------------------------------------------------------------
int gsProgramsAndShadersMonitor::shadersVecIndex(GLuint shaderName) const
{
    int retVal = -1;

    // Look for the shader name in the _openGLShaderNameToShaderObjectsVecIndex map:
    gtMap<GLuint, int>::const_iterator iter = _openGLShaderNameToShaderObjectsVecIndex.find(shaderName);

    // If we found the program id:
    if (iter != _openGLShaderNameToShaderObjectsVecIndex.end())
    {
        // Return the program id:
        retVal = (*iter).second;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::createStubFragmentShader
// Description: Create and compiles a stub fragment shader - a very simple fragment shader.
// Arguments:   shaderType - The type of the shader to be created.
// Return Val:  GLuint - The created shader id or 0 if the creation failed.
// Author:      Yaki Tebeka
// Date:        7/4/2005
// ---------------------------------------------------------------------------
GLuint gsProgramsAndShadersMonitor::createStubFragmentShader(apGLShadingObjectType shaderType)
{
    GLuint retVal = 0;

    // Create the fragment shader object:
    GLuint shaderObjName = createFragmentShaderObject(shaderType);

    if (shaderObjName != 0)
    {
        if (shaderType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT)
        {
            // Verify that GL_ARB_shader_objects is supported:
            if (_isARBShaderObjectsExtSupported)
            {
                // Set the shader source code:
                const char* pShaderStrings[1] = { stat_stubVertexShaderSourceCode };
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glShaderSourceARB);
                gs_stat_realFunctionPointers.glShaderSourceARB(shaderObjName, 1, pShaderStrings, NULL);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glShaderSourceARB);

                // Compile the shader:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCompileShaderARB);
                gs_stat_realFunctionPointers.glCompileShaderARB(shaderObjName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCompileShaderARB);

                // Verify that the shader was compiled successfully:
                GLint isShaderCompiledSuccessfully = 0;
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
                gs_stat_realFunctionPointers.glGetObjectParameterivARB(shaderObjName, GL_OBJECT_COMPILE_STATUS_ARB, &isShaderCompiledSuccessfully);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#else
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glShaderSourceARB);
                _glShaderSourceARB(shaderObjName, 1, pShaderStrings, NULL);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glShaderSourceARB);

                // Compile the shader:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCompileShaderARB);
                _glCompileShaderARB(shaderObjName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCompileShaderARB);

                // Verify that the shader was compiled successfully:
                GLint isShaderCompiledSuccessfully = 0;
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
                _glGetObjectParameterivARB(shaderObjName, GL_OBJECT_COMPILE_STATUS_ARB, &isShaderCompiledSuccessfully);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#endif

                if (isShaderCompiledSuccessfully)
                {
                    retVal = shaderObjName;
                }
            }
        }
        else if (shaderType == AP_GL_2_0_SHADING_OBJECT)
        {
            if (_isOpenGL20Supported)
            {
                // Set the shader source code:
                const char* pShaderStrings[1] = { stat_stubVertexShaderSourceCode };
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glShaderSource);
                gs_stat_realFunctionPointers.glShaderSource(shaderObjName, 1, pShaderStrings, NULL);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glShaderSource);

                // Compile the shader:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCompileShader);
                gs_stat_realFunctionPointers.glCompileShader(shaderObjName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCompileShader);

                // Verify that the shader was compiled successfully:
                GLint isShaderCompiledSuccessfully = 0;
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
                gs_stat_realFunctionPointers.glGetShaderiv(shaderObjName, GL_COMPILE_STATUS, &isShaderCompiledSuccessfully);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
#else
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glShaderSource);
                _glShaderSource(shaderObjName, 1, pShaderStrings, NULL);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glShaderSource);

                // Compile the shader:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCompileShader);
                _glCompileShader(shaderObjName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCompileShader);

                // Verify that the shader was compiled successfully:
                GLint isShaderCompiledSuccessfully = 0;
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
                _glGetShaderiv(shaderObjName, GL_COMPILE_STATUS, &isShaderCompiledSuccessfully);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
#endif

                if (isShaderCompiledSuccessfully)
                {
                    retVal = shaderObjName;
                }
            }
        }

        // In case of failure - we need to delete the created shader object:
        if (!retVal)
        {
            deleteFragmentShaderObject(shaderObjName, shaderType);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::getShaderHolderProgram
// Description: Returns the shader holder program of the given type. If this program
//              was not created and createIfNeeded is true, create it.
// Author:      Uri Shomroni
// Date:        4/2/2010
// ---------------------------------------------------------------------------
GLuint gsProgramsAndShadersMonitor::getShaderHolderProgram(apGLShadingObjectType programType, bool createIfNeeded)
{
    GLuint retVal = 0;

    if (programType == AP_GL_2_0_SHADING_OBJECT)
    {
        // If we want to create the project, and it doesn't exist yet:
        if (createIfNeeded && (_openGL20ShaderHolderProgramName == 0))
        {
            // Create it:
            _openGL20ShaderHolderProgramName = createProgramObject(programType);
        }

        // Return the program:
        retVal = _openGL20ShaderHolderProgramName;
    }
    else if (programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT)
    {
        // If we want to create the project, and it doesn't exist yet:
        if (createIfNeeded && (_shaderObjectExtShaderHolderProgramName == 0))
        {
            // Create it:
            _shaderObjectExtShaderHolderProgramName = createProgramObject(programType);
        }

        // Return the program:
        retVal = _shaderObjectExtShaderHolderProgramName;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::createFragmentShaderObject
// Description: Creates an OpenGL shader object.
// Arguments: shaderObjType - The requested shader object type.
// Return Val:  GLuint - The created shader id or 0 if the creation failed.
// Author:      Yaki Tebeka
// Date:        7/4/2005
// ---------------------------------------------------------------------------
GLuint gsProgramsAndShadersMonitor::createFragmentShaderObject(apGLShadingObjectType shaderObjType)
{
    GLuint retVal = 0;

    if (shaderObjType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT)
    {
        if (_isARBShaderObjectsExtSupported)
        {
            // Create the shader object:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCreateShaderObjectARB);
            retVal = gs_stat_realFunctionPointers.glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCreateShaderObjectARB);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCreateShaderObjectARB);
            retVal = _glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCreateShaderObjectARB);
#endif
        }
    }
    else if (shaderObjType == AP_GL_2_0_SHADING_OBJECT)
    {
        if (_isOpenGL20Supported)
        {
            // Create the shader object:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCreateShader);
            retVal = gs_stat_realFunctionPointers.glCreateShader(GL_FRAGMENT_SHADER);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCreateShader);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCreateShader);
            retVal = _glCreateShader(GL_FRAGMENT_SHADER);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCreateShader);
#endif
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::createProgramObject
// Description: Creates an OpenGL program object.
// Arguments: programType - The requested program object type.
// Return Val:  GLuint - The created program id or 0 if the creation failed.
// Author:      Uri Shomroni
// Date:        4/2/2010
// ---------------------------------------------------------------------------
GLuint gsProgramsAndShadersMonitor::createProgramObject(apGLShadingObjectType programType)
{
    GLuint retVal = 0;

    if (programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT)
    {
        if (_isARBShaderObjectsExtSupported)
        {
            // Create the shader object:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCreateProgramObjectARB);
            retVal = gs_stat_realFunctionPointers.glCreateProgramObjectARB();
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCreateProgramObjectARB);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCreateProgramObjectARB);
            retVal = _glCreateProgramObjectARB();
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCreateProgramObjectARB);
#endif
        }
    }
    else if (programType == AP_GL_2_0_SHADING_OBJECT)
    {
        if (_isOpenGL20Supported)
        {
            // Create the shader object:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCreateProgram);
            retVal = gs_stat_realFunctionPointers.glCreateProgram();
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCreateProgram);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCreateProgram);
            retVal = _glCreateProgram();
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCreateProgram);
#endif
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::deleteFragmentShaderObject
// Description: Deletes a given fragment shader object.
// Arguments:   shaderName - The shader object name.
//              shaderObjType - The shader object type.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::deleteFragmentShaderObject(GLuint shaderName, apGLShadingObjectType shaderObjType)
{
    bool retVal = true;

    if (shaderObjType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT)
    {
        if (_isARBShaderObjectsExtSupported)
        {
            // Mark the object to be deleted:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteObjectARB);
            gs_stat_realFunctionPointers.glDeleteObjectARB(shaderName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDeleteObjectARB);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteObjectARB);
            _glDeleteObjectARB(shaderName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDeleteObjectARB);
#endif
        }
    }
    else if (shaderObjType == AP_GL_2_0_SHADING_OBJECT)
    {
        if (_isOpenGL20Supported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteShader);
            gs_stat_realFunctionPointers.glDeleteShader(shaderName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDeleteShader);
#else
            // Mark the object to be deleted:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteShader);
            _glDeleteShader(shaderName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDeleteShader);
#endif
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::deleteProgramObject
// Description: Deletes a program object
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        4/2/2010
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::deleteProgramObject(GLuint programName, apGLShadingObjectType programObjType)
{
    (void)(programName); // unused
    bool retVal = true;

    if (programObjType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT)
    {
        if (_isARBShaderObjectsExtSupported)
        {
            // Mark the object to be deleted:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteObjectARB);
            gs_stat_realFunctionPointers.glDeleteObjectARB(programObjType);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDeleteObjectARB);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteObjectARB);
            _glDeleteObjectARB(programObjType);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDeleteObjectARB);
#endif
        }
    }
    else if (programObjType == AP_GL_2_0_SHADING_OBJECT)
    {
        if (_isOpenGL20Supported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteProgram);
            gs_stat_realFunctionPointers.glDeleteProgram(programObjType);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDeleteProgram);
#else
            // Mark the object to be deleted:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteProgram);
            _glDeleteProgram(programObjType);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDeleteProgram);
#endif
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::detachProgramGeometryShaders
// Description:
//  Detach program geometry shaders (if any).
//
// Arguments:   program - The input program
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/4/2008
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::detachProgramGeometryShaders(const gsGLProgram& program)
{
    bool retVal = true;

    // Get the program name:
    GLuint programName = program.programName();

    // Iterate the program shader objects:
    const gtList<GLuint>& shaderObjects = program.shaderObjects();
    gtList<GLuint>::const_iterator iter = shaderObjects.begin();
    gtList<GLuint>::const_iterator endIter = shaderObjects.end();

    while (iter != endIter)
    {
        // Get the current shader:
        GLuint shaderName = *iter;
        int vecIndex = shadersVecIndex(shaderName);

        if (vecIndex != -1)
        {
            apGLShaderObject* pCurrentShader = _shaderObjects[vecIndex];

            if (pCurrentShader)
            {
                // If it is a geometry shader:
                if (pCurrentShader->type() == OS_TOBJ_ID_GL_GEOMETRY_SHADER)
                {
                    // Detach the geometry shader from the program:
                    apGLShadingObjectType shaderType = pCurrentShader->shaderObjectType();
                    bool rc = detachShaderFromProgram(programName, shaderName, shaderType);
                    retVal = retVal && rc;
                }
            }
        }

        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::attachBackProgramGeometryShaders
// Description: Attach back a given program geometry shaders, that were detached
//              by detachProgramGeometryShaders().
// Arguments:   program - An object that represents the input program.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/4/2008
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::attachBackProgramGeometryShaders(const gsGLProgram& program)
{
    bool retVal = true;

    // Get the program name:
    GLuint programName = program.programName();

    // Iterate the program shader objects:
    const gtList<GLuint>& shaderObjects = program.shaderObjects();
    gtList<GLuint>::const_iterator iter = shaderObjects.begin();
    gtList<GLuint>::const_iterator endIter = shaderObjects.end();

    while (iter != endIter)
    {
        // Get the current shader:
        GLuint shaderName = *iter;
        int vecIndex = shadersVecIndex(shaderName);

        if (vecIndex != -1)
        {
            apGLShaderObject* pCurrentShader = _shaderObjects[vecIndex];

            if (pCurrentShader)
            {
                // If it is a fragment shader:
                if (pCurrentShader->type() == OS_TOBJ_ID_GL_GEOMETRY_SHADER)
                {
                    // Attach the fragment shader to the program:
                    apGLShadingObjectType shaderType = pCurrentShader->shaderObjectType();
                    bool rc = attachShaderToProgram(programName, shaderName, shaderType);
                    retVal = retVal && rc;
                }
            }
        }

        iter++;
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::detachProgramFragmentShaders
// Description:
//  Detach program fragment shaders (if any).
//
// Arguments:   program - The input program
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::detachProgramFragmentShaders(const gsGLProgram& program)
{
    bool retVal = true;

    // Get the program name:
    GLuint programName = program.programName();

    // Iterate the program shader objects:
    const gtList<GLuint>& shaderObjects = program.shaderObjects();
    gtList<GLuint>::const_iterator iter = shaderObjects.begin();
    gtList<GLuint>::const_iterator endIter = shaderObjects.end();

    while (iter != endIter)
    {
        // Get the current shader:
        GLuint shaderName = *iter;
        int vecIndex = shadersVecIndex(shaderName);

        if (vecIndex != -1)
        {
            apGLShaderObject* pCurrentShader = _shaderObjects[vecIndex];

            if (pCurrentShader)
            {
                // If it is a fragment shader:
                if (pCurrentShader->type() == OS_TOBJ_ID_GL_FRAGMENT_SHADER)
                {
                    // Detach the fragment shader from the program:
                    apGLShadingObjectType shaderType = pCurrentShader->shaderObjectType();
                    bool rc = detachShaderFromProgram(programName, shaderName, shaderType);
                    retVal = retVal && rc;
                }
            }
        }

        iter++;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::attachBackProgramFragmentShaders
// Description: Attach back a given program fragment shaders, that were detached
//              by detachProgramFragmentShaders().
// Arguments:   program - An object that represents the input program.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::attachBackProgramFragmentShaders(const gsGLProgram& program)
{
    bool retVal = true;

    // Get the program name:
    GLuint programName = program.programName();

    // Iterate the program shader objects:
    const gtList<GLuint>& shaderObjects = program.shaderObjects();
    gtList<GLuint>::const_iterator iter = shaderObjects.begin();
    gtList<GLuint>::const_iterator endIter = shaderObjects.end();

    while (iter != endIter)
    {
        // Get the current shader:
        GLuint shaderName = *iter;
        int vecIndex = shadersVecIndex(shaderName);

        if (vecIndex != -1)
        {
            apGLShaderObject* pCurrentShader = _shaderObjects[vecIndex];

            if (pCurrentShader)
            {
                // If it is a fragment shader:
                if (pCurrentShader->type() == OS_TOBJ_ID_GL_FRAGMENT_SHADER)
                {
                    // Attach the fragment shader to the program:
                    apGLShadingObjectType shaderType = pCurrentShader->shaderObjectType();
                    bool rc = attachShaderToProgram(programName, shaderName, shaderType);
                    retVal = retVal && rc;
                }
            }
        }

        iter++;
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::detachProgramFragmentShadersFromHolderProgram
// Description: Detach the program's fragment shaders from the holder program
// Arguments:   program - The input program
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        4/2/2010
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::detachProgramFragmentShadersFromHolderProgram(const gsGLProgram& program)
{
    bool retVal = false;

    // Get the program name:
    apGLShadingObjectType programType = program.programType();
    GLuint holderProgramName = getShaderHolderProgram(programType, false);

    // Make sure everything's okay:
    GT_IF_WITH_ASSERT(holderProgramName > 0)
    {
        retVal = true;

        // Iterate the program shader objects:
        const gtList<GLuint>& shaderObjects = program.shaderObjects();
        gtList<GLuint>::const_iterator iter = shaderObjects.begin();
        gtList<GLuint>::const_iterator endIter = shaderObjects.end();

        while (iter != endIter)
        {
            // Get the current shader:
            GLuint shaderName = *iter;
            int vecIndex = shadersVecIndex(shaderName);

            if (vecIndex != -1)
            {
                apGLShaderObject* pCurrentShader = _shaderObjects[vecIndex];

                if (pCurrentShader != NULL)
                {
                    // If it is a fragment shader:
                    if ((pCurrentShader->type() == OS_TOBJ_ID_GL_FRAGMENT_SHADER) && pCurrentShader->isAttachedToHolderProgram())
                    {
                        // Detach the fragment shader from the program:
                        apGLShadingObjectType shaderType = pCurrentShader->shaderObjectType();
                        bool rc = detachShaderFromProgram(holderProgramName, shaderName, shaderType);
                        pCurrentShader->setAttachedToHolderProgram(!rc);
                        retVal = retVal && rc;
                    }
                }
            }

            iter++;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::attachProgramFragmentShadersToHolderProgram
// Description: Attach all a program's fragment shaders to the holder program, so
//              that if they were marked for deletion, they will not be deleted until
//              detached.
// Arguments:   program - An object that represents the input program.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        4/2/2010
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::attachProgramFragmentShadersToHolderProgram(const gsGLProgram& program)
{
    bool retVal = true;

    // Get the program name:
    apGLShadingObjectType programType = program.programType();
    GLuint holderProgramName = getShaderHolderProgram(programType);

    // Iterate the program shader objects:
    const gtList<GLuint>& shaderObjects = program.shaderObjects();
    gtList<GLuint>::const_iterator iter = shaderObjects.begin();
    gtList<GLuint>::const_iterator endIter = shaderObjects.end();

    while (iter != endIter)
    {
        // Get the current shader:
        GLuint shaderName = *iter;
        int vecIndex = shadersVecIndex(shaderName);

        if (vecIndex != -1)
        {
            apGLShaderObject* pCurrentShader = _shaderObjects[vecIndex];

            if (pCurrentShader != NULL)
            {
                // If it is a fragment shader:
                if ((pCurrentShader->type() == OS_TOBJ_ID_GL_FRAGMENT_SHADER) && (!pCurrentShader->isAttachedToHolderProgram()))
                {
                    // Attach the fragment shader to the program:
                    apGLShadingObjectType shaderType = pCurrentShader->shaderObjectType();
                    bool rc = attachShaderToProgram(holderProgramName, shaderName, shaderType);
                    pCurrentShader->setAttachedToHolderProgram(rc);
                    retVal = retVal && rc;
                }
            }
        }

        iter++;
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::detachProgramGeometryShadersFromHolderProgram
// Description: Detach the program's geometry shaders from the holder program
// Arguments:   program - The input program
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        4/2/2010
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::detachProgramGeometryShadersFromHolderProgram(const gsGLProgram& program)
{
    bool retVal = false;

    // Get the program name:
    apGLShadingObjectType programType = program.programType();
    GLuint holderProgramName = getShaderHolderProgram(programType, false);

    // Make sure everything's okay:
    GT_IF_WITH_ASSERT(holderProgramName > 0)
    {
        retVal = true;

        // Iterate the program shader objects:
        const gtList<GLuint>& shaderObjects = program.shaderObjects();
        gtList<GLuint>::const_iterator iter = shaderObjects.begin();
        gtList<GLuint>::const_iterator endIter = shaderObjects.end();

        while (iter != endIter)
        {
            // Get the current shader:
            GLuint shaderName = *iter;
            int vecIndex = shadersVecIndex(shaderName);

            if (vecIndex != -1)
            {
                apGLShaderObject* pCurrentShader = _shaderObjects[vecIndex];

                if (pCurrentShader != NULL)
                {
                    // If it is a geometry shader:
                    if ((pCurrentShader->type() == OS_TOBJ_ID_GL_GEOMETRY_SHADER) && pCurrentShader->isAttachedToHolderProgram())
                    {
                        // Detach the geometry shader from the program:
                        apGLShadingObjectType shaderType = pCurrentShader->shaderObjectType();
                        bool rc = detachShaderFromProgram(holderProgramName, shaderName, shaderType);
                        pCurrentShader->setAttachedToHolderProgram(!rc);
                        retVal = retVal && rc;
                    }
                }
            }

            iter++;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::attachProgramGeometryShadersToHolderProgram
// Description: Attach all a program's geometry shaders to the holder program, so
//              that if they were marked for deletion, they will not be deleted until
//              detached.
// Arguments:   program - An object that represents the input program.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        4/2/2010
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::attachProgramGeometryShadersToHolderProgram(const gsGLProgram& program)
{
    bool retVal = true;

    // Get the program name:
    apGLShadingObjectType programType = program.programType();
    GLuint holderProgramName = getShaderHolderProgram(programType);

    // Iterate the program shader objects:
    const gtList<GLuint>& shaderObjects = program.shaderObjects();
    gtList<GLuint>::const_iterator iter = shaderObjects.begin();
    gtList<GLuint>::const_iterator endIter = shaderObjects.end();

    while (iter != endIter)
    {
        // Get the current shader:
        GLuint shaderName = *iter;
        int vecIndex = shadersVecIndex(shaderName);

        if (vecIndex != -1)
        {
            apGLShaderObject* pCurrentShader = _shaderObjects[vecIndex];

            if (pCurrentShader != NULL)
            {
                // If it is a geometry shader:
                if ((pCurrentShader->type() == OS_TOBJ_ID_GL_GEOMETRY_SHADER) && (!pCurrentShader->isAttachedToHolderProgram()))
                {
                    // Attach the geometry shader to the program:
                    apGLShadingObjectType shaderType = pCurrentShader->shaderObjectType();
                    bool rc = attachShaderToProgram(holderProgramName, shaderName, shaderType);
                    retVal = retVal && rc;
                    pCurrentShader->setAttachedToHolderProgram(rc);
                }
            }
        }

        iter++;
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::detachStubFragmentShaderFromProgram
// Description: Detach the stub fragment shader (installed by attachStubFragmentShaderToProgram)
//              from a given program.
// Arguments:   program - An object that represent the program.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::detachStubFragmentShaderFromProgram(gsGLProgram& program)
{
    bool retVal = false;

    // Get the program name and type:
    GLuint programName = program.programName();
    apGLShadingObjectType programObjType = program.programType();

    if (programObjType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT)
    {
        retVal = detachShaderFromProgram(programName, _shaderObjectExtStubFragmentShaderName, AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT);
    }
    else if (programObjType == AP_GL_2_0_SHADING_OBJECT)
    {
        retVal = detachShaderFromProgram(programName, _openGL20StubFragmentShaderName, AP_GL_2_0_SHADING_OBJECT);
    }

    if (retVal)
    {
        // Mark that the stub fragment shader was detached from the program:
        program.onStubFragmentShaderDetached();
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::attachStubFragmentShaderToProgram
// Description: Creates (if needed) and attach a simple stub fragment shader to
//              the input program a program.
// Arguments:   program - An object that represent the program.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::attachStubFragmentShaderToProgram(gsGLProgram& program)
{
    bool retVal = false;

    // Get the program name and type:
    GLuint programName = program.programName();
    apGLShadingObjectType programObjType = program.programType();

    if ((programObjType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
    {
        // Create the stub fragment shader (if needed):
        if (_shaderObjectExtStubFragmentShaderName == 0)
        {
            _shaderObjectExtStubFragmentShaderName = createStubFragmentShader(AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT);
        }

        // If the stub shader object exists:
        if (_shaderObjectExtStubFragmentShaderName != 0)
        {
            if (_isARBShaderObjectsExtSupported)
            {
                // Attach the stub fragment shader object:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glAttachObjectARB);
                gs_stat_realFunctionPointers.glAttachObjectARB(programName, _shaderObjectExtStubFragmentShaderName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glAttachObjectARB);
#else
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glAttachObjectARB);
                _glAttachObjectARB(programName, _shaderObjectExtStubFragmentShaderName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glAttachObjectARB);
#endif
                retVal = true;
            }
        }
    }
    else if ((programObjType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
    {
        // Create the stub fragment shader (if needed):
        if (_openGL20StubFragmentShaderName == 0)
        {
            _openGL20StubFragmentShaderName = createStubFragmentShader(AP_GL_2_0_SHADING_OBJECT);
        }

        // If the stub shader object exists:
        if (_openGL20StubFragmentShaderName != 0)
        {
            if (_isOpenGL20Supported)
            {
                // Attach the stub fragment shader object:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glAttachShader);
                gs_stat_realFunctionPointers.glAttachShader(programName, _openGL20StubFragmentShaderName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glAttachShader);
#else
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glAttachShader);
                _glAttachShader(programName, _openGL20StubFragmentShaderName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glAttachShader);
#endif
                retVal = true;
            }
        }
    }

    if (retVal)
    {
        // Mark that the stub fragment shader was attached to the program:
        program.onStubFragmentShaderAttached();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::detachShaderFromProgram
// Description:
//  Detach an OpenGL shader from its program.
//  NOTICE: This function does not update this class data !!!
//
// Arguments:   programName - The OpenGL name of the program to which the shader is attached.
//              shaderName - The shader name.
//              shaderObjType - The type of the shader object.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::detachShaderFromProgram(GLuint programName, GLuint shaderName, apGLShadingObjectType shaderObjType)
{
    bool retVal = false;

    if (shaderObjType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT)
    {
        if (_isARBShaderObjectsExtSupported)
        {
            // Detach the shader object:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDetachObjectARB);
            gs_stat_realFunctionPointers.glDetachObjectARB(programName, shaderName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDetachObjectARB);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDetachObjectARB);
            _glDetachObjectARB(programName, shaderName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDetachObjectARB);
#endif
            retVal = true;
        }
    }
    else if (shaderObjType == AP_GL_2_0_SHADING_OBJECT)
    {
        if (_isOpenGL20Supported)
        {
            // Detach the shader object:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDetachShader);
            gs_stat_realFunctionPointers.glDetachShader(programName, shaderName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDetachShader);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDetachShader);
            _glDetachShader(programName, shaderName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDetachShader);
#endif
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::attachShaderToProgram
// Description:
//  Attach an OpenGL shader to a program.
//  NOTICE: This function does not update this class data !!!
//
// Arguments:   programName - The OpenGL name of the program to which the shader will be attached.
//              shaderName - The shader name.
//              shaderObjType - The type of the shader object.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::attachShaderToProgram(GLuint programName, GLuint shaderName, apGLShadingObjectType shaderObjType)
{
    bool retVal = false;

    if (shaderObjType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT)
    {
        if (_isARBShaderObjectsExtSupported)
        {
            // Attach the shader object:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glAttachObjectARB);
            gs_stat_realFunctionPointers.glAttachObjectARB(programName, shaderName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glAttachObjectARB);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glAttachObjectARB);
            _glAttachObjectARB(programName, shaderName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glAttachObjectARB);
#endif
            retVal = true;
        }
    }
    else if (shaderObjType == AP_GL_2_0_SHADING_OBJECT)
    {
        if (_isOpenGL20Supported)
        {
            // Attach the shader object:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glAttachShader);
            gs_stat_realFunctionPointers.glAttachShader(programName, shaderName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glAttachShader);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glAttachShader);
            _glAttachShader(programName, shaderName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glAttachShader);
#endif
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::linkProgram
// Description: Links a program
// Arguments:   programName - The input program name.
//              programObjType - The program object type.
//              wasLinkSuccessful - Will get true iff the link operation succeeded.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::linkProgram(GLuint programName, apGLShadingObjectType programObjType, bool& wasLinkSuccessful)
{
    bool retVal = false;

    if ((programObjType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
    {
        // Link the program:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glLinkProgramARB);
        gs_stat_realFunctionPointers.glLinkProgramARB(programName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glLinkProgramARB);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glLinkProgramARB);
        _glLinkProgramARB(programName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glLinkProgramARB);
#endif

        retVal = true;
    }
    else if ((programObjType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
    {
        // Link the program:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glLinkProgram);
        gs_stat_realFunctionPointers.glLinkProgram(programName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glLinkProgram);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glLinkProgram);
        _glLinkProgram(programName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glLinkProgram);
#endif

        retVal = true;
    }

    if (retVal)
    {
        // Check if the link was successful:
        wasLinkSuccessful = wasProgramSuccessfullyLinked(programName, programObjType);
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::validateProgram
// Description: Validates a given program object
// Arguments:   programName - The program object name.
//              programObjType - The program object type.
//              wasValidationSuccessful - Will get true iff the validation operation succeeded.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/4/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::validateProgram(GLuint programName, apGLShadingObjectType programObjType, bool& wasValidationSuccessful)
{
    bool retVal = false;
    GLint validationSucceeded = GL_FALSE;

    if ((programObjType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
    {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        // Validate the program:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glValidateProgramARB);
        gs_stat_realFunctionPointers.glValidateProgramARB(programName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glValidateProgramARB);

        // Check if the validation was successful:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        gs_stat_realFunctionPointers.glGetObjectParameterivARB(programName, GL_OBJECT_VALIDATE_STATUS_ARB, &validationSucceeded);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#else
        // Validate the program:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glValidateProgramARB);
        _glValidateProgramARB(programName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glValidateProgramARB);

        // Check if the validation was successful:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        _glGetObjectParameterivARB(programName, GL_OBJECT_VALIDATE_STATUS_ARB, &validationSucceeded);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#endif

        retVal = true;
    }
    else if ((programObjType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
    {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        // Validate the program:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glValidateProgram);
        gs_stat_realFunctionPointers.glValidateProgram(programName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glValidateProgram);

        // Check if the validation was successful:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        gs_stat_realFunctionPointers.glGetProgramiv(programName, GL_VALIDATE_STATUS, &validationSucceeded);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
#else
        // Validate the program:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glValidateProgram);
        _glValidateProgram(programName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glValidateProgram);

        // Check if the validation was successful:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        _glGetProgramiv(programName, GL_VALIDATE_STATUS, &validationSucceeded);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
#endif

        retVal = true;
    }

    // Output the validation success flag status:
    wasValidationSuccessful = (validationSucceeded == GL_TRUE);

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::generateShaderSourceCodeFilePath
// Description: Generates a shader source code file path.
// Arguments:   shaderObj - The shader object used to generate the Path.
//              filePath - The output file path.
// Author:      Yaki Tebeka
// Date:        27/12/2004
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::generateShaderSourceCodeFilePath(apGLShaderObject& shaderObj, osFilePath& filePath) const
{
    // Get the shader object name:
    GLuint shaderName = shaderObj.shaderName();

    // Generate a file name for this shader source code:
    gtString typeOfShaderAsText;
    osTransferableObjectType typeOfShader = shaderObj.type();

    switch (typeOfShader)
    {
        case OS_TOBJ_ID_GL_VERTEX_SHADER:
        {
            typeOfShaderAsText = L"Vertex";
        }
        break;

        case OS_TOBJ_ID_GL_TESSELLATION_CONTROL_SHADER:
        {
            typeOfShaderAsText = L"TessCtrl";
        }
        break;

        case OS_TOBJ_ID_GL_TESSELLATION_EVALUATION_SHADER:
        {
            typeOfShaderAsText = L"TessEval";
        }
        break;

        case OS_TOBJ_ID_GL_GEOMETRY_SHADER:
        {
            typeOfShaderAsText = L"Geometry";
        }
        break;

        case OS_TOBJ_ID_GL_FRAGMENT_SHADER:
        {
            typeOfShaderAsText = L"Fragment";
        }
        break;

        case OS_TOBJ_ID_GL_COMPUTE_SHADER:
        {
            typeOfShaderAsText = L"Compute";
        }
        break;

        default:
        {
            typeOfShaderAsText = L"Unknown";
        }
        break;
    }

    // Build the log file name:
    gtString logFileName;

    logFileName.appendFormattedString(GS_STR_shaderFilePath, _spyContextId, shaderName, typeOfShaderAsText.asCharArray());
    // Set the log file path:
    filePath = suCurrentSessionLogFilesDirectory();
    filePath.setFileName(logFileName);

    // Set the log file extension:
    gtString extensionString = L"glsl";

    // TO_DO: Handle here different shader types file extensions (Cg, etc)
    filePath.setFileExtension(extensionString);
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::isShaderAttachedToAnyProgram
// Description: Returns true iff this shader is attached to any program
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        4/2/2010
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::isShaderAttachedToAnyProgram(GLuint shaderName) const
{
    bool retVal = false;

    int numberOfPrograms = (int)_programObjects.size();

    for (int i = 0; i < numberOfPrograms; i++)
    {
        // Get the program data:
        const gsGLProgram* pCurrentProgram = _programObjects[i];

        if (pCurrentProgram != NULL)
        {
            // Search for the shader in the current program:
            const gtList<GLuint>& currentProgramAttachedShaders = pCurrentProgram->shaderObjects();
            gtList<GLuint>::const_iterator attachedShadersIter = currentProgramAttachedShaders.begin();
            gtList<GLuint>::const_iterator attachedShadersEndIter = currentProgramAttachedShaders.end();

            while (attachedShadersIter != attachedShadersEndIter)
            {
                // If this program has this shader attached:
                if ((*attachedShadersIter) == shaderName)
                {
                    retVal = true;
                    break;
                }

                attachedShadersIter++;
            }
        }

        // No need to continue if we already found the shader:
        if (retVal)
        {
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::setShaderSourceCode
// Description: Sets
// Arguments: GLuint shaderName
//            const gtString& shaderSourceCode
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/11/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::setShaderSourceCode(GLuint shaderName, const gtASCIIString& shaderSourceCode)
{
    bool retVal = false;

    // Get the shader object vector index:
    int shdaerIndex = shadersVecIndex(shaderName);
    GT_IF_WITH_ASSERT(shdaerIndex != -1)
    {
        // Get the shader object representation:
        apGLShaderObject* pShaderObj = _shaderObjects[shdaerIndex];
        GT_IF_WITH_ASSERT(pShaderObj != NULL)
        {
            // If we have changed the source code in order to stub the shader, ignore this (the new code will be implemented when canceling the forced mode)
            if (_areFragmentShaderSourcesStubbed)
            {
                retVal = true;
            }
            else
            {
                // Cast the input source code into a C string:
                const char* pShaderStrings[1] = {NULL};
                pShaderStrings[0] = shaderSourceCode.asCharArray();
                GT_IF_WITH_ASSERT(pShaderStrings[0] != NULL)
                {
                    // Get the shader object type:
                    apGLShadingObjectType shaderType = pShaderObj->shaderObjectType();

                    // If this is an GL_ARB_shader_objects:
                    if ((shaderType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
                    {
                        // Set the shader source code:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glShaderSourceARB);
                        gs_stat_realFunctionPointers.glShaderSourceARB(shaderName, 1, pShaderStrings, NULL);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glShaderSourceARB);
#else
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glShaderSourceARB);
                        _glShaderSourceARB(shaderName, 1, pShaderStrings, NULL);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glShaderSourceARB);
#endif
                        retVal = true;
                    }
                    else if ((shaderType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
                    {
                        // This is an OpenGL 2.0 shader. Set its source code:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glShaderSource);
                        gs_stat_realFunctionPointers.glShaderSource(shaderName, 1, pShaderStrings, NULL);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glShaderSource);
#else
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glShaderSource);
                        _glShaderSource(shaderName, 1, pShaderStrings, NULL);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glShaderSource);
#endif
                        retVal = true;
                    }
                }
            }

            // If the shader source code was successfully set:
            if (retVal)
            {
                // Log the shader source code update:
                retVal = logShaderSourceCodeUpdate(*pShaderObj, shaderSourceCode);
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::logShaderSourceCodeUpdate
// Description:
//  Is called after a shader source code is changed.
//  Logs the new source code into a file and updates the object that represents
//  the shader.
// Arguments: shaderObj - The object that represents the shader.
//            newShaderSourceCode - The new shader source code.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/11/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::logShaderSourceCodeUpdate(apGLShaderObject& shaderObj, const gtASCIIString& newShaderSourceCode)
{
    bool retVal = false;

    // Generate a file path for the input shader's source code:
    osFilePath sourceCodeFilePath;
    generateShaderSourceCodeFilePath(shaderObj, sourceCodeFilePath);

    // Save the shader source code into this file:
    osFile sourceCodeFile;
    bool rc = sourceCodeFile.open(sourceCodeFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    if (rc)
    {
        // Write the shader source code into the file:
        sourceCodeFile << newShaderSourceCode;

        // Close the file:
        sourceCodeFile.close();

        // Log the file path in the shader wrapper object:
        shaderObj.onShaderSourceCodeSet(sourceCodeFilePath);

        retVal = true;
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::getShaderSourceCodeFromFile
// Description: Gets the shader source code from the file used by us (either
//              saved from glShaderSource calls or from editing in the shaders
//              source code editor)
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        10/9/2009
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::getShaderSourceCodeFromFile(const apGLShaderObject& shaderObj, gtString& shaderSource)
{
    bool retVal = false;

    shaderSource.makeEmpty();

    // Get the path where the shader's source is saved:
    const osFilePath& sourceCodeFilePath = shaderObj.sourceCodeFilePath();

    // Save the shader source code into this file:
    osFile sourceCodeFile;
    bool rc = sourceCodeFile.open(sourceCodeFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);

    if (rc)
    {
        // Write the shader source code into the file:
        retVal = sourceCodeFile.readIntoString(shaderSource);

        // Close the file:
        sourceCodeFile.close();
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::wasShaderCompiledSuccessfully
// Description: Queries OpenGL for a given shader compilation status.
// Arguments:   shaderName - The shader name.
//              shaderType - The shader type.
// Return Val:  bool - true iff the shader was compiled successfully.
// Author:      Yaki Tebeka
// Date:        31/5/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::wasShaderCompiledSuccessfully(GLuint shaderName, apGLShadingObjectType shaderType) const
{
    bool retVal = false;

    GLint wasCompilationSuccessful = 0;

    if ((shaderType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
    {
        // Get the shader object compilation status:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        gs_stat_realFunctionPointers.glGetObjectParameterivARB(shaderName, GL_OBJECT_COMPILE_STATUS_ARB, &wasCompilationSuccessful);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        _glGetObjectParameterivARB(shaderName, GL_OBJECT_COMPILE_STATUS_ARB, &wasCompilationSuccessful);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#endif
    }
    else if ((shaderType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
    {
        // Get the shader object compilation status:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
        gs_stat_realFunctionPointers.glGetShaderiv(shaderName, GL_COMPILE_STATUS, &wasCompilationSuccessful);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
        _glGetShaderiv(shaderName, GL_COMPILE_STATUS, &wasCompilationSuccessful);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
#endif
    }

    // If the compilation was successful:
    if (wasCompilationSuccessful == 1)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::getShaderCompilationLog
// Description: Retrieves a shader compilation log.
// Arguments:   shaderName - The shader name.
//              shaderType - The shader type.
//              compilationLog - Will get the shader compilation log.
//              Notice: _glGetObjectParameterivARB return no log string on compilation
//              success on MAC OS-X. Do not fail the calling functions for not getting the log string
// Author:      Yaki Tebeka
// Date:        31/5/2005
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::getShaderCompilationLog(GLuint shaderName, apGLShadingObjectType shaderType,
                                                          gtString& compilationLog) const
{
    if ((shaderType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
    {
        // Get the length of the compilation log:
        GLint compilationLogLength = 0;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        gs_stat_realFunctionPointers.glGetObjectParameterivARB(shaderName, GL_OBJECT_INFO_LOG_LENGTH_ARB, &compilationLogLength);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        _glGetObjectParameterivARB(shaderName, GL_OBJECT_INFO_LOG_LENGTH_ARB, &compilationLogLength);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#endif

        if (compilationLogLength > 0)
        {
            // Allocate space for the compilation log:
            GLcharARB* pCompilationLog = new GLcharARB[compilationLogLength + 1];

            if (pCompilationLog)
            {
                // Get the compilation log:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetInfoLogARB);
                gs_stat_realFunctionPointers.glGetInfoLogARB(shaderName, compilationLogLength, NULL, pCompilationLog);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetInfoLogARB);
#else
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetInfoLogARB);
                _glGetInfoLogARB(shaderName, compilationLogLength, NULL, pCompilationLog);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetInfoLogARB);
#endif

                // Output it:
                compilationLog.fromASCIIString(pCompilationLog);

                // Clean up:
                delete[] pCompilationLog;
            }
        }
    }
    else if ((shaderType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
    {
        // Get the length of the compilation log:
        GLint compilationLogLength = 0;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
        gs_stat_realFunctionPointers.glGetShaderiv(shaderName, GL_INFO_LOG_LENGTH, &compilationLogLength);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
        _glGetShaderiv(shaderName, GL_INFO_LOG_LENGTH, &compilationLogLength);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
#endif

        if (compilationLogLength > 0)
        {
            // Allocate space for the compilation log:
            GLchar* pCompilationLog = new GLchar[compilationLogLength + 1];

            if (pCompilationLog)
            {
                // Get the compilation log:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderInfoLog);
                gs_stat_realFunctionPointers.glGetShaderInfoLog(shaderName, compilationLogLength, NULL, pCompilationLog);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderInfoLog);
#else
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderInfoLog);
                _glGetShaderInfoLog(shaderName, compilationLogLength, NULL, pCompilationLog);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderInfoLog);
#endif

                // Output it:
                compilationLog.fromASCIIString(pCompilationLog);

                // Clean up:
                delete[] pCompilationLog;
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::wasProgramSuccessfullyLinked
// Description: Queries OpenGL for a given program link status
// Arguments:   programName - The input program name.
// Return Val:  bool - true - iff the program was linked successfully.
// Author:      Yaki Tebeka
// Date:        31/5/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::wasProgramSuccessfullyLinked(GLuint programName, apGLShadingObjectType programType) const
{
    bool retVal = false;

    // Get the program link status:
    GLint wasLinkSuccesful = 0;

    if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
    {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        gs_stat_realFunctionPointers.glGetObjectParameterivARB(programName, GL_OBJECT_LINK_STATUS_ARB, &wasLinkSuccesful);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        _glGetObjectParameterivARB(programName, GL_OBJECT_LINK_STATUS_ARB, &wasLinkSuccesful);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#endif
    }
    else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
    {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        gs_stat_realFunctionPointers.glGetProgramiv(programName, GL_LINK_STATUS, &wasLinkSuccesful);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        _glGetProgramiv(programName, GL_LINK_STATUS, &wasLinkSuccesful);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
#endif
    }

    // If the link was successful:
    if (wasLinkSuccesful == GL_TRUE)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::getProgramInfoLog
// Description: Retrieves a given program link log.
// Arguments:   programName - The program name.
//              programType - The program type.
//              linkLog - The program link log.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        31/5/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::getProgramInfoLog(GLuint programName, apGLShadingObjectType programType,
                                                    gtString& linkLog) const
{
    bool retVal = true;

    if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
    {
        // Get the length of the link log:
        GLint linkLogLength = 0;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        gs_stat_realFunctionPointers.glGetObjectParameterivARB(programName, GL_OBJECT_INFO_LOG_LENGTH_ARB, &linkLogLength);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        _glGetObjectParameterivARB(programName, GL_OBJECT_INFO_LOG_LENGTH_ARB, &linkLogLength);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#endif

        if (linkLogLength > 0)
        {
            // Allocate space for the link log:
            GLcharARB* pLinkLog = new GLcharARB[linkLogLength + 1];

            if (pLinkLog)
            {
                // Get the link log:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetInfoLogARB);
                gs_stat_realFunctionPointers.glGetInfoLogARB(programName, linkLogLength, NULL, pLinkLog);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetInfoLogARB);
#else
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetInfoLogARB);
                _glGetInfoLogARB(programName, linkLogLength, NULL, pLinkLog);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetInfoLogARB);
#endif

                // Output it:
                linkLog.fromASCIIString(pLinkLog);

                // Clean up:
                delete[] pLinkLog;

                retVal = true;
            }
        }
    }
    else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
    {
        // Get the length of the link log:
        GLint linkLogLength = 0;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        gs_stat_realFunctionPointers.glGetProgramiv(programName, GL_INFO_LOG_LENGTH, &linkLogLength);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        _glGetProgramiv(programName, GL_INFO_LOG_LENGTH, &linkLogLength);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
#endif

        if (linkLogLength > 0)
        {
            // Allocate space for the link log:
            GLchar* pLinkLog = new GLchar[linkLogLength + 1];

            if (pLinkLog)
            {
                // Get the link log:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramInfoLog);
                gs_stat_realFunctionPointers.glGetProgramInfoLog(programName, linkLogLength, NULL, pLinkLog);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramInfoLog);
#else
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramInfoLog);
                _glGetProgramInfoLog(programName, linkLogLength, NULL, pLinkLog);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramInfoLog);
#endif

                // Output it:
                linkLog.fromASCIIString(pLinkLog);

                // Clean up:
                delete[] pLinkLog;

                retVal = true;
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::wasShaderPhysicallyDeleted
// Description:
//  Inputs a shader name and returns true iff it was physically deleted from
//  OpenGL.
//
// Arguments:   shaderName - The input shader name.
//              shaderType - The input shader type.
//
// Return Val:  bool - true iff the shader was physically deleted from OpenGL.
// Author:      Yaki Tebeka
// Date:        8/6/2005
// Implementation notes:
//   We try to get the shader type. If we get a GL_INVALID_VALUE OpenGL error, then
//   the shader was deleted.
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::wasShaderPhysicallyDeleted(GLuint shaderName, apGLShadingObjectType shaderType) const
{
    bool retVal = false;

    // Clear OpenGL errors (if there were any):
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    (void) gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    // Will get true iff we executed a query on the shader object:
    bool executedObjQuery = false;

    if ((shaderType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
    {
        // Try to get the shader object type:
        GLint notUsed = 0;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        gs_stat_realFunctionPointers.glGetObjectParameterivARB(shaderName, GL_OBJECT_TYPE_ARB, &notUsed);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        _glGetObjectParameterivARB(shaderName, GL_OBJECT_TYPE_ARB, &notUsed);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#endif

        executedObjQuery = true;
    }
    else if ((shaderType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
    {
        // Try to get the shader object type:
        GLint notUsed = 0;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
        gs_stat_realFunctionPointers.glGetShaderiv(shaderName, GL_SHADER_TYPE, &notUsed);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
        _glGetShaderiv(shaderName, GL_SHADER_TYPE, &notUsed);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetShaderiv);
#endif

        executedObjQuery = true;
    }

    // If we executed a query on the shader object:
    if (executedObjQuery)
    {
        // If we got an GL_INVALID_VALUE error - it means that the object does not exist:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        GLenum currentError = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        if (currentError == GL_INVALID_VALUE)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::wasProgramPhysicallyDeleted
// Description:
//  Inputs a program name and returns true iff it was physically deleted from
//  OpenGL.
//
// Arguments:   programName - The input program name.
//              programType - The input program type.
//
// Return Val:  bool - true iff the program was physically deleted from OpenGL.
// Author:      Yaki Tebeka
// Date:        8/6/2005
// Implementation notes:
//   We try to get the program type. If we get a GL_INVALID_VALUE OpenGL error, then
//   the program was deleted.
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::wasProgramPhysicallyDeleted(GLuint programName, apGLShadingObjectType programType) const
{
    bool retVal = false;

    // Clear OpenGL errors (if there were any):
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    (void) gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    // Will get true iff we executed a query on the program object:
    bool executedObjQuery = false;

    if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
    {
        // Try to get the program object type:
        GLint notUsed = 0;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        gs_stat_realFunctionPointers.glGetObjectParameterivARB(programName, GL_OBJECT_TYPE_ARB, &notUsed);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
        _glGetObjectParameterivARB(programName, GL_OBJECT_TYPE_ARB, &notUsed);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#endif

        executedObjQuery = true;
    }
    else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
    {
        // Try to get the program object type:
        GLint notUsed = 0;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        gs_stat_realFunctionPointers.glGetProgramiv(programName, GL_DELETE_STATUS, &notUsed);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        _glGetProgramiv(programName, GL_DELETE_STATUS, &notUsed);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
#endif

        executedObjQuery = true;
    }

    // If we executed a query on the program object:
    if (executedObjQuery)
    {
        // If we got an GL_INVALID_VALUE error - it means that the object does not exist:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        GLenum currentError = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        if (currentError == GL_INVALID_VALUE)
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::onProgramPhysicalDeletion
// Description: Called when we know a program was physically deleted, i.e. when
//              it was marked for deletion and is no longer used as the active
//              program at any context.
// Author:      Uri Shomroni
// Date:        8/8/2010
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::onProgramPhysicalDeletion(GLuint programName, const gsGLProgram* pProgramObj)
{
    // Detach all shaders from this program (according to the GL_ARB_shader_objects spec, when a container
    // object is deleted, all its attached objects are detached):

    // First, get the names of the shaders to be detached:
    const gtList<GLuint>& shaderObjects = pProgramObj->shaderObjects();
    gtList<GLuint>::const_iterator iter = shaderObjects.begin();
    gtList<GLuint>::const_iterator endIter = shaderObjects.end();
    gtVector<GLuint> vShadersToBeDetached;

    while (iter != endIter)
    {
        GLuint currentShader = *iter;
        vShadersToBeDetached.push_back(currentShader);
        iter++;
    }

    // Now detach the shaders (this has to be done separately as deleting them while running on the list
    // will invalidate the iterators):
    int n = (int)vShadersToBeDetached.size();

    for (int i = 0; i < n; i++)
    {
        bool rcShader = onShaderDetachedFromProgram(programName, vShadersToBeDetached[i]);
        GT_ASSERT(rcShader);
    }

    // Remove it from this class vectors:
    removeProgramFromVectors(programName);
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::removeManagedObjectFromVectors
// Description: Removes a managed object from this class vectors.
// Arguments:   objectName - The managed object name.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        8/6/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::removeManagedObjectFromVectors(GLuint objectName)
{
    bool retVal = false;

    if (isShaderObject(objectName))
    {
        retVal = removeShaderFromVectors(objectName);
    }
    else if (isProgramObject(objectName))
    {
        retVal = removeProgramFromVectors(objectName);
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::removeShaderFromVectors
// Description: Removes a shader from this class vectors.
// Arguments:   shaderName - The name of the shader to be removed.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        8/6/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::removeShaderFromVectors(GLuint shaderName)
{
    bool retVal = false;

    // Get the shader object index in this class vectors:
    int vecIndex = shadersVecIndex(shaderName);

    if (vecIndex != -1)
    {
        // Remove the shader from the _openGLShaderNameToShaderObjectsVecIndex map:
        _openGLShaderNameToShaderObjectsVecIndex[shaderName] = -1;

        // Delete object that represents the shader:
        apGLShaderObject* pShaderObj = _shaderObjects[vecIndex];
        _shaderObjects[vecIndex] = NULL;
        delete pShaderObj;

        // Remove it from _shaderObjects vector:
        int amountOfShaders = (int)_shaderObjects.size();
        int lastIndexToHandle = amountOfShaders - 2;

        for (int i = vecIndex; i <= lastIndexToHandle; i++)
        {
            _shaderObjects[i] = _shaderObjects[i + 1];
            apGLShaderObject* pCurShaderObj = _shaderObjects[i];

            if (pCurShaderObj)
            {
                GLuint curShaderObjName = pCurShaderObj->shaderName();
                _openGLShaderNameToShaderObjectsVecIndex[curShaderObjName] = i;
            }
        }

        _shaderObjects.pop_back();

        retVal = true;
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::removeProgramFromVectors
// Description: Removes a program from this class vectors.
// Arguments:   programName - The name of the program to be removed.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        8/6/2005
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::removeProgramFromVectors(GLuint programName)
{
    bool retVal = false;

    // Get the program object index in this class vectors:
    int vecIndex = programsVecIndex(programName);

    if (vecIndex != -1)
    {
        // Remove the program from the _openGLProgramNameToProgramObjectsVecIndex map:
        _openGLProgramNameToProgramObjectsVecIndex[programName] = -1;

        // Delete object that represents the program:
        gsGLProgram* pProgramObj = _programObjects[vecIndex];
        _programObjects[vecIndex] = NULL;
        delete pProgramObj;

        // Remove it from _programObjects vector:
        int amountOfPrograms = (int)_programObjects.size();
        int lastIndexToHandle = amountOfPrograms - 2;

        for (int i = vecIndex; i <= lastIndexToHandle; i++)
        {
            _programObjects[i] = _programObjects[i + 1];
            gsGLProgram* pCurProgram = _programObjects[i];

            if (pCurProgram)
            {
                GLuint curProgName = pCurProgram->programName();
                _openGLProgramNameToProgramObjectsVecIndex[curProgName] = i;
            }
        }

        _programObjects.pop_back();

        // Remove the program active uniforms manager:
        retVal = _programsActiveUniforms.onProgramPhysicalDeletion(programName);
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::getShaderSourceCodeVersion
// Description: Extract the GLSL version from the shader source code
// Arguments: const gtString& sourceCodeAsString
//            int& glslSourceVersion
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/3/2009
// ---------------------------------------------------------------------------
void gsProgramsAndShadersMonitor::getShaderSourceCodeVersion(const gtASCIIString& sourceCodeAsString, apGLSLVersion& glslSourceVersion)
{
    // Find the version string within the source code:
    int versionStringPos = sourceCodeAsString.find("#version");

    if (versionStringPos >= 0)
    {
        int versionStringStart = versionStringPos;
        bool firstDigitFound = false;

        while (!firstDigitFound)
        {
            versionStringStart ++;
            char currentChar = sourceCodeAsString[versionStringStart];
            firstDigitFound = gtIsDigit(currentChar);
        }

        int versionStringEnd = versionStringStart;
        bool lastDigitFound = false;

        while (!lastDigitFound)
        {
            versionStringEnd ++;
            char currentChar = sourceCodeAsString[versionStringEnd];
            lastDigitFound = !gtIsDigit(currentChar);
        }

        // Get the number string from the version string:
        gtASCIIString versionNumberAsString;
        sourceCodeAsString.getSubString(versionStringStart, versionStringEnd - 1,  versionNumberAsString);

        // Translate the version from string to a GLSL version enum:
        gtString versionNumberUnicode;
        versionNumberUnicode.fromASCIIString(versionNumberAsString.asCharArray());
        apStringToGLSLVersion(versionNumberUnicode, glslSourceVersion);
    }
}


// ---------------------------------------------------------------------------
// Name:        gsProgramsAndShadersMonitor::calculateShadersMemorySize
// Description: Calculate the shaders memory size
// Arguments:   gtUInt64& shadersMemorySize
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool gsProgramsAndShadersMonitor::calculateShadersMemorySize(gtUInt64& shadersMemorySize) const
{
    bool retVal = true;
    shadersMemorySize = 0;

    // Iterate the shaders:
    for (int i = 0 ; i < (int)_shaderObjects.size(); i++)
    {
        // Get the current shader:
        apGLShaderObject* pCurrentShader = _shaderObjects[i];

        if (pCurrentShader != NULL)
        {
            // Get the current shader source code:
            unsigned long sourceCodeLength = pCurrentShader->sourceCodeLength();

            // Set the object size in KB
            // The size is given in bytes, so divide it by 1024 and ceil it:
            shadersMemorySize += (gtSize_t)ceil((double)sourceCodeLength / 1024.0);
        }
    }

    return retVal;
}
