//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsActiveUniformsMonitor.cpp
///
//==================================================================================

//------------------------------ gsActiveUniformsMonitor.cpp ------------------------------

// Standard C:
#include <string.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsActiveUniformsMonitor.h>

// The maximal logged size of a uniform name
#define GS_MAX_UNIFORM_NAME 256


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::gsActiveUniformsMonitor
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        28/5/2005
// ---------------------------------------------------------------------------
gsActiveUniformsMonitor::gsActiveUniformsMonitor()
    :
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    _glGetObjectParameterivARB(NULL), _glGetActiveUniformARB(NULL), _glGetUniformfvARB(NULL),
    _glGetUniformivARB(NULL), _glGetUniformLocationARB(NULL), _glGetProgramiv(NULL),
    _glGetActiveUniform(NULL), _glGetUniformfv(NULL), _glGetUniformiv(NULL), _glGetUniformLocation(NULL),
#endif
    _glUniformMatrix2x3fv(NULL), _glUniformMatrix3x2fv(NULL), _glUniformMatrix2x4fv(NULL),
    _glUniformMatrix4x2fv(NULL), _glUniformMatrix3x4fv(NULL), _glUniformMatrix4x3fv(NULL),
    _glGetActiveUniformBlockName(NULL), _glGetUniformBlockIndex(NULL),
    _isARBShaderObjectsExtSupported(false), _isOpenGL20Supported(false), _isOpenGL21Supported(false), _isUniformBufferObjectsExtSupported(false)
{
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::onFirstTimeContextMadeCurrent
// Description: Is called on the first time in which my program's context
//              is made the current context.
// Author:      Yaki Tebeka
// Date:        28/5/2005
// ---------------------------------------------------------------------------
void gsActiveUniformsMonitor::onFirstTimeContextMadeCurrent()
{
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // Initialize GL_ARB_shader_objects function pointers:
    _glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)gsGetSystemsOGLModuleProcAddress("glGetObjectParameterivARB");
    _glGetActiveUniformARB = (PFNGLGETACTIVEUNIFORMARBPROC)gsGetSystemsOGLModuleProcAddress("glGetActiveUniformARB");
    _glGetUniformfvARB = (PFNGLGETUNIFORMFVARBPROC)gsGetSystemsOGLModuleProcAddress("glGetUniformfvARB");
    _glGetUniformivARB = (PFNGLGETUNIFORMIVARBPROC)gsGetSystemsOGLModuleProcAddress("glGetUniformivARB");
    _glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)gsGetSystemsOGLModuleProcAddress("glGetUniformLocationARB");

    _glUniform1ivARB = (PFNGLUNIFORM1IVARBPROC)gsGetSystemsOGLModuleProcAddress("glUniform1ivARB");
    _glUniform2ivARB = (PFNGLUNIFORM2IVARBPROC)gsGetSystemsOGLModuleProcAddress("glUniform2ivARB");
    _glUniform3ivARB = (PFNGLUNIFORM3IVARBPROC)gsGetSystemsOGLModuleProcAddress("glUniform3ivARB");
    _glUniform4ivARB = (PFNGLUNIFORM4IVARBPROC)gsGetSystemsOGLModuleProcAddress("glUniform4ivARB");

    _glUniform1fvARB = (PFNGLUNIFORM1FVARBPROC)gsGetSystemsOGLModuleProcAddress("glUniform1fvARB");
    _glUniform2fvARB = (PFNGLUNIFORM2FVARBPROC)gsGetSystemsOGLModuleProcAddress("glUniform2fvARB");
    _glUniform3fvARB = (PFNGLUNIFORM3FVARBPROC)gsGetSystemsOGLModuleProcAddress("glUniform3fvARB");
    _glUniform4fvARB = (PFNGLUNIFORM4FVARBPROC)gsGetSystemsOGLModuleProcAddress("glUniform4fvARB");

    _glUniformMatrix2fvARB = (PFNGLUNIFORMMATRIX2FVARBPROC)gsGetSystemsOGLModuleProcAddress("glUniformMatrix2fvARB");
    _glUniformMatrix3fvARB = (PFNGLUNIFORMMATRIX3FVARBPROC)gsGetSystemsOGLModuleProcAddress("glUniformMatrix3fvARB");
    _glUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)gsGetSystemsOGLModuleProcAddress("glUniformMatrix4fvARB");

    // Verify that we managed to get all function pointers:
    _isARBShaderObjectsExtSupported = ((_glGetObjectParameterivARB != NULL) && (_glGetActiveUniformARB != NULL) &&
                                       (_glGetUniformfvARB != NULL) && (_glGetUniformivARB != NULL) &&
                                       (_glGetUniformLocationARB != NULL) &&
                                       (_glUniform1ivARB != NULL) && (_glUniform2ivARB != NULL) &&
                                       (_glUniform3ivARB != NULL) && (_glUniform4ivARB != NULL) &&
                                       (_glUniform1fvARB != NULL) && (_glUniform2fvARB != NULL) &&
                                       (_glUniform3fvARB != NULL) && (_glUniform4fvARB != NULL) &&
                                       (_glUniformMatrix2fvARB != NULL) && (_glUniformMatrix3fvARB != NULL) &&
                                       (_glUniformMatrix4fvARB != NULL));

    // Initialize OpenGL 2.0 function pointers:

    _glGetProgramiv = (PFNGLGETPROGRAMIVPROC)gsGetSystemsOGLModuleProcAddress("glGetProgramiv");
    _glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)gsGetSystemsOGLModuleProcAddress("glGetActiveUniform");
    _glGetUniformfv = (PFNGLGETUNIFORMFVPROC)gsGetSystemsOGLModuleProcAddress("glGetUniformfv");
    _glGetUniformiv = (PFNGLGETUNIFORMIVPROC)gsGetSystemsOGLModuleProcAddress("glGetUniformiv");
    _glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)gsGetSystemsOGLModuleProcAddress("glGetUniformLocation");

    _glUniform1iv = (PFNGLUNIFORM1IVPROC)gsGetSystemsOGLModuleProcAddress("glUniform1iv");
    _glUniform2iv = (PFNGLUNIFORM2IVPROC)gsGetSystemsOGLModuleProcAddress("glUniform2iv");
    _glUniform3iv = (PFNGLUNIFORM3IVPROC)gsGetSystemsOGLModuleProcAddress("glUniform3iv");
    _glUniform4iv = (PFNGLUNIFORM4IVPROC)gsGetSystemsOGLModuleProcAddress("glUniform4iv");

    _glUniform1fv = (PFNGLUNIFORM1FVPROC)gsGetSystemsOGLModuleProcAddress("glUniform1fv");
    _glUniform2fv = (PFNGLUNIFORM2FVPROC)gsGetSystemsOGLModuleProcAddress("glUniform2fv");
    _glUniform3fv = (PFNGLUNIFORM3FVPROC)gsGetSystemsOGLModuleProcAddress("glUniform3fv");
    _glUniform4fv = (PFNGLUNIFORM4FVPROC)gsGetSystemsOGLModuleProcAddress("glUniform4fv");

    _glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)gsGetSystemsOGLModuleProcAddress("glUniformMatrix2fv");
    _glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)gsGetSystemsOGLModuleProcAddress("glUniformMatrix3fv");
    _glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)gsGetSystemsOGLModuleProcAddress("glUniformMatrix4fv");

    // Verify that we managed to get all function pointers:
    _isOpenGL20Supported = ((_glGetProgramiv != NULL) && (_glGetActiveUniform != NULL) &&
                            (_glGetUniformfv != NULL) && (_glGetUniformiv != NULL) &&
                            (_glGetUniformLocation != NULL) &&
                            (_glUniform1iv != NULL) && (_glUniform2iv != NULL) &&
                            (_glUniform3iv != NULL) && (_glUniform4iv != NULL) &&
                            (_glUniform1fv != NULL) && (_glUniform2fv != NULL) &&
                            (_glUniform3fv != NULL) && (_glUniform4fv != NULL) &&
                            (_glUniformMatrix2fv != NULL) && (_glUniformMatrix3fv != NULL) &&
                            (_glUniformMatrix4fv != NULL));

#else
#ifdef _GR_IPHONE_BUILD
    // ARB_shader_objects is not supported on the iPhone. OpenGL 2.0-type shaders are
    // supported only on OpenGL ES 2.0+, which is included in the iPhone SDK 3.0 and higher.
    // So, we check if all functions are present:
    _isARBShaderObjectsExtSupported = false;
    _isOpenGL20Supported = ((gs_stat_realFunctionPointers.glGetProgramiv != NULL) && (gs_stat_realFunctionPointers.glGetActiveUniform != NULL) &&
                            (gs_stat_realFunctionPointers.glGetUniformfv != NULL) && (gs_stat_realFunctionPointers.glGetUniformiv != NULL) &&
                            (gs_stat_realFunctionPointers.glGetUniformLocation != NULL) && (gs_stat_realFunctionPointers.glUniform1iv != NULL) &&
                            (gs_stat_realFunctionPointers.glUniform2iv != NULL) && (gs_stat_realFunctionPointers.glUniform3iv != NULL) &&
                            (gs_stat_realFunctionPointers.glUniform4iv != NULL) && (gs_stat_realFunctionPointers.glUniform1fv != NULL) &&
                            (gs_stat_realFunctionPointers.glUniform2fv != NULL) && (gs_stat_realFunctionPointers.glUniform3fv != NULL) &&
                            (gs_stat_realFunctionPointers.glUniform4fv != NULL) && (gs_stat_realFunctionPointers.glUniformMatrix2fv != NULL) &&
                            (gs_stat_realFunctionPointers.glUniformMatrix3fv != NULL) && (gs_stat_realFunctionPointers.glUniformMatrix4fv != NULL));
#else
    // MAC OS X - OpenGL is the base level:
    _isARBShaderObjectsExtSupported = true;
    _isOpenGL20Supported = true;
#endif
#endif

    // Initialize OpenGL 2.1 function pointers:
    _glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC)gsGetSystemsOGLModuleProcAddress("glUniformMatrix2x3fv");
    _glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC)gsGetSystemsOGLModuleProcAddress("glUniformMatrix3x2fv");
    _glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC)gsGetSystemsOGLModuleProcAddress("glUniformMatrix2x4fv");
    _glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC)gsGetSystemsOGLModuleProcAddress("glUniformMatrix4x2fv");
    _glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC)gsGetSystemsOGLModuleProcAddress("glUniformMatrix3x4fv");
    _glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC)gsGetSystemsOGLModuleProcAddress("glUniformMatrix4x3fv");

    // Verify that we managed to get all function pointers:
    _isOpenGL21Supported = ((_glUniformMatrix2x3fv != NULL) && (_glUniformMatrix3x2fv != NULL) &&
                            (_glUniformMatrix2x4fv != NULL) && (_glUniformMatrix4x2fv != NULL) &&
                            (_glUniformMatrix3x4fv != NULL) && (_glUniformMatrix4x3fv != NULL));

    // Initialize GL_ARB_uniform_buffer_object extension functions:
    _glGetActiveUniformBlockName = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)gsGetSystemsOGLModuleProcAddress("glGetActiveUniformBlockName");
    _glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)gsGetSystemsOGLModuleProcAddress("glGetUniformBlockIndex");
    _isUniformBufferObjectsExtSupported = (_glGetActiveUniformBlockName != NULL) && (_glGetUniformBlockIndex != NULL);
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::onProgramCreation
// Description: Is called when a program is created.
// Arguments:   programName - The program object name.
//              programType - The program type.
// Author:      Yaki Tebeka
// Date:        29/5/2005
// ---------------------------------------------------------------------------
void gsActiveUniformsMonitor::onProgramCreation(GLuint programName, apGLShadingObjectType programType)
{
    // Create an object that will contain the program uniforms data:
    int programIndex = (int)_programUniformsData.size();
    gsProgramUniformsData* pProgramUniformData = new gsProgramUniformsData(programName, programType);

    // Add it to the _programUniformsData vector:
    _programUniformsData.push_back(pProgramUniformData);

    // Add it to the "name to vector index" map:
    _programNameToVecIndex[programName] = programIndex;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::onProgramLinked
// Description: Is called when a program is linked.
// Arguments:   programName - The OpenGL name of the linked program.
//              wasLinkSuccessful - Will get true iff the link was successfully.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        28/5/2005
// Implementation notes:
//   Logs the program's active uniforms.
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::onProgramLinked(GLuint programName, bool wasLinkSuccessful)
{
    bool retVal = false;

    // Get the index of the program in this class vectors:
    int vecIndex = programsVecIndex(programName);

    if (vecIndex != -1)
    {
        // Get the programs uniforms data:
        gsProgramUniformsData* pProgramUniformsData = _programUniformsData[vecIndex];

        if (pProgramUniformsData)
        {
            // Update the program link status:
            pProgramUniformsData->setProgramLinkStatus(wasLinkSuccessful);

            // Clear the program uniforms data:
            pProgramUniformsData->clearUniformsData();

            // If the program was linked successfully:
            if (wasLinkSuccessful)
            {
                // Update the programs available uniforms:
                bool rc1 = updateProgramAvailableUniforms(programName);

                // Update the programs uniform locations:
                bool rc2 = updateProgramsUniformLocations(programName);

                retVal = rc1 && rc2;
            }
            else
            {
                // The program link failed, therefore it does not extern uniforms.
                retVal = true;
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::onProgramPhysicalDeletion
// Description: Is called when a program is "physically" deleted by OpenGL.
// Arguments:   programName - The deleted program name.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        8/6/2005
// Implementation notes:
//  a. Delete the program's uniforms container.
//  b. Remove the program from this class vectors.
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::onProgramPhysicalDeletion(GLuint programName)
{
    bool retVal = false;

    // Get the program vectors index:
    int vecIndex = programsVecIndex(programName);

    if (vecIndex != -1)
    {
        // Delete the program's uniforms data:
        gsProgramUniformsData* pProgramUniformsData = _programUniformsData[vecIndex];
        _programUniformsData[vecIndex] = NULL;
        delete pProgramUniformsData;

        // Remove the program from this class vectors and maps:
        int amontOfPrograms = (int)_programUniformsData.size();
        int lastIndexToHandle = amontOfPrograms - 2;

        for (int i = vecIndex; i <= lastIndexToHandle; i++)
        {
            // _programUniformsData vector:
            _programUniformsData[i] = _programUniformsData[i + 1];

            // _programNameToVecIndex map:
            GLuint curProgName = _programUniformsData[i]->programName();
            _programNameToVecIndex[curProgName] = i;
        }

        _programUniformsData.pop_back();

        // Set the program name index as -1:
        _programNameToVecIndex[programName] = -1;

        retVal = true;
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::clearContextDataSnapshot
// Description: Clears context data held by this class.
// Author:      Yaki Tebeka
// Date:        15/6/2005
// Implementation Notes:
//   Clears all linked programs active uniform values.
// ---------------------------------------------------------------------------
void gsActiveUniformsMonitor::clearContextDataSnapshot()
{
    // Iterate the programs:
    gtPtrVector<gsProgramUniformsData*>::const_iterator iter = _programUniformsData.begin();
    gtPtrVector<gsProgramUniformsData*>::const_iterator endIter = _programUniformsData.end();

    while (iter != endIter)
    {
        // Get the current program name:
        GLuint curProgramName = (*iter)->programName();

        // Clear its uniform values:
        clearProgramActiveUniformsValues(curProgramName);

        iter++;
    }
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::updateContextDataSnapshot
// Description: Updates context data held by this class.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/5/2005
// Implementation notes:
//  Updates programs active uniform values.
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::updateContextDataSnapshot()
{
    bool retVal = true;

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateProgramsUniformsDataStarted, OS_DEBUG_LOG_DEBUG);

    // Clear OpenGL errors (if there were any):
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    (void) gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    // Iterate the linked programs:
    gtPtrVector<gsProgramUniformsData*>::const_iterator iter = _programUniformsData.begin();
    gtPtrVector<gsProgramUniformsData*>::const_iterator endIter = _programUniformsData.end();

    while (iter != endIter)
    {
        // Get the current program name:
        GLuint curProgramName = (*iter)->programName();

        // If the program was linked successfully:
        if ((*iter)->isProgramLinkedSuccessfully())
        {
            // Update its uniform values:
            bool rc = updateProgramActiveUniformsValues(curProgramName);
            retVal = retVal && rc;
        }

        iter++;
    }

    // Verify that we didn't produce OpenGL errors:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GLenum errorEnum = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GT_ASSERT(errorEnum == GL_NO_ERROR);

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateProgramsUniformsDataEnded, OS_DEBUG_LOG_DEBUG);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::restoreProgramUniformValues
// Description:
//   Restores a given program uniforms values to a given stored values container.
// Arguments:   programName - The input program name.
//              storedUniformValues - The stored values container.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/6/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::restoreProgramUniformValues(GLuint programName, const apGLItemsCollection& storedUniformValues)
{
    bool retVal = false;

    // Get the index of the program in this class vectors:
    int vecIndex = programsVecIndex(programName);

    if (vecIndex != -1)
    {
        // Get the programs uniforms data:
        gsProgramUniformsData* pProgramUniformsData = _programUniformsData[vecIndex];

        if (pProgramUniformsData)
        {
            // Verify that the program is linked successfully:
            bool wasProgramLinkedSuccessfully = pProgramUniformsData->isProgramLinkedSuccessfully();

            if (wasProgramLinkedSuccessfully)
            {
                retVal = true;

                // Get the program type:
                apGLShadingObjectType programType = pProgramUniformsData->programType();

                // Get the program uniforms container:
                apGLItemsCollection& programUniformsContainer = pProgramUniformsData->programActiveUniforms();

                // Iterate the program active uniforms:
                int activeUniformsAmount = programUniformsContainer.amountOfItems();

                for (int i = 0; i < activeUniformsAmount; i++)
                {
                    // Will get the uniform location in its program:
                    const gtString& uniformName = programUniformsContainer.itemName(i);
                    int uniformLocation = pProgramUniformsData->getUniformLocation(uniformName);

                    // If the current uniform value is accessible:
                    if (uniformLocation != -1)
                    {
                        // Find the stored uniform index in the stored uniform values collection:
                        int storedUniformIndex = storedUniformValues.itemIndex(uniformName);

                        if (storedUniformIndex != -1)
                        {
                            // Get the current uniform type:
                            GLenum curUniformType = programUniformsContainer.itemType(i);

                            // Verify that it matches the stored uniform type:
                            GT_IF_WITH_ASSERT(curUniformType == storedUniformValues.itemType(storedUniformIndex))
                            {
                                // Get the uniform stored value:
                                const apParameter* pUniformStoredValue = storedUniformValues.itemValue(storedUniformIndex);
                                GT_IF_WITH_ASSERT(pUniformStoredValue != NULL)
                                {
                                    // If we have a stored value:
                                    if (pUniformStoredValue->type() != OS_TOBJ_ID_NOT_AVAILABLE_PARAMETER)
                                    {
                                        // Restore the uniform value:
                                        retVal = setActiveUniformValue(programName, programType, curUniformType,
                                                                       uniformLocation, *pUniformStoredValue);
                                    }
                                }
                            }
                        }
                        else
                        {
                            // TO_DO: There is a bug here - if the program is created in
                            //        stub FS mode, we don't have the uniform values.
                            //        To solve the problem, in stub FS mode, we need to log
                            //        the uniform values in pProgramUniformsData.
                            // retVal = false;
                        }
                    }
                }
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::updateStubbedUniformValue
// Description: Emulates the effect of a call to a glUniformXXX call when "stub
//              fragment / geometry shaders" modes are on
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        16/12/2009
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::updateStubbedUniformValue(GLuint programName, GLint uniformLocation, GLenum uniformType, void* values, bool transpose)
{
    bool retVal = false;

    // TO_DO: handle glUniformMatrixXXX with count > 1

    // Get the Program data:
    int programIndex = programsVecIndex(programName);

    if (programIndex > -1)
    {
        // Get the uniforms data:
        gsProgramUniformsData* pUniformsData = _programUniformsData[programIndex];

        if (pUniformsData != NULL)
        {
            // Find this uniform in the program:
            int uniformIndex = pUniformsData->uniformIndexFromLocation(uniformLocation);

            if (uniformIndex > -1)
            {
                // Make sure this uniform is of the right type:
                apGLItemsCollection& programUniforms = pUniformsData->programActiveUniforms();

                if (uniformType == programUniforms.itemType(uniformIndex))
                {
                    gtAutoPtr<apParameter> aptrParamValue;

                    switch (uniformType)
                    {
                        case GL_FLOAT:
                            aptrParamValue = new apGLfloatParameter(*(GLfloat*)values);
                            break;

                        case GL_FLOAT_VEC2:
                            multiParameterFromVoidPointer(2, 1, false, values, aptrParamValue);
                            break;

                        case GL_FLOAT_VEC3:
                            multiParameterFromVoidPointer(3, 1, false, values, aptrParamValue);
                            break;

                        case GL_FLOAT_VEC4:
                            multiParameterFromVoidPointer(4, 1, false, values, aptrParamValue);
                            break;

                        case GL_FLOAT_MAT2:
                            multiParameterFromVoidPointer(2, 2, false, values, aptrParamValue, transpose);
                            break;

                        case GL_FLOAT_MAT3:
                            multiParameterFromVoidPointer(3, 3, false, values, aptrParamValue, transpose);
                            break;

                        case GL_FLOAT_MAT4:
                            multiParameterFromVoidPointer(4, 4, false, values, aptrParamValue, transpose);
                            break;

                        case GL_FLOAT_MAT2x3:
                            multiParameterFromVoidPointer(2, 3, false, values, aptrParamValue, transpose);
                            break;

                        case GL_FLOAT_MAT2x4:
                            multiParameterFromVoidPointer(2, 4, false, values, aptrParamValue, transpose);
                            break;

                        case GL_FLOAT_MAT3x2:
                            multiParameterFromVoidPointer(3, 2, false, values, aptrParamValue, transpose);
                            break;

                        case GL_FLOAT_MAT3x4:
                            multiParameterFromVoidPointer(3, 4, false, values, aptrParamValue, transpose);
                            break;

                        case GL_FLOAT_MAT4x2:
                            multiParameterFromVoidPointer(4, 2, false, values, aptrParamValue, transpose);
                            break;

                        case GL_FLOAT_MAT4x3:
                            multiParameterFromVoidPointer(4, 3, false, values, aptrParamValue, transpose);
                            break;
                            break;

                        case GL_INT:
                        case GL_BOOL:
                            aptrParamValue = new apGLintParameter(*(GLint*)values);
                            break;

                        case GL_INT_VEC2:
                        case GL_BOOL_VEC2:
                            multiParameterFromVoidPointer(2, 1, true, values, aptrParamValue);
                            break;

                        case GL_INT_VEC3:
                        case GL_BOOL_VEC3:
                            multiParameterFromVoidPointer(3, 1, true, values, aptrParamValue);
                            break;

                        case GL_INT_VEC4:
                        case GL_BOOL_VEC4:
                            multiParameterFromVoidPointer(4, 1, true, values, aptrParamValue);
                            break;

                        default:
                            // Will be handled by the nullptr case below
                            break;
                    }

                    if (nullptr == aptrParamValue.pointedObject())
                    {
                        aptrParamValue = new apNotAvailableParameter();
                    }

                    // Note the new value in our system:
                    programUniforms.setItemValue(uniformIndex, aptrParamValue);

                    // See if this uniform still exists (e.g. a vertex shader uniform) and if it does, set it:
                    apGLShadingObjectType progType = pUniformsData->programType();
                    GLint uniformNewLocation = getUniformLocation(programName, progType, programUniforms.itemName(uniformIndex).asCharArray());

                    if (uniformNewLocation > -1)
                    {
                        // Get the item from its new location:
                        const apParameter* pParam = programUniforms.itemValue(uniformIndex);
                        GT_IF_WITH_ASSERT(pParam != NULL)
                        {
                            bool rcSet = setActiveUniformValue(programName, progType, uniformType, uniformNewLocation, *pParam);
                            GT_ASSERT(rcSet);
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::getProgramActiveUniforms
// Description: Retrieves a given program uniforms.
// Arguments:   programName - The queried program name.
//              programUniforms - Will get the program uniform names, types and
//                                values.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/5/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::getProgramActiveUniforms(GLuint programName, const apGLItemsCollection*& programUniforms) const
{
    bool retVal = false;
    programUniforms = nullptr;

    // Get the index of the program in this class vectors:
    int vecIndex = programsVecIndex(programName);

    if (vecIndex != -1)
    {
        // Get the programs uniforms data:
        gsProgramUniformsData* pProgramUniformsData = _programUniformsData[vecIndex];

        if (pProgramUniformsData)
        {
            // If the program was linked successfully:
            if (pProgramUniformsData->isProgramLinkedSuccessfully())
            {
                // Get the program uniforms container:
                const apGLItemsCollection& programUniformsContainer = pProgramUniformsData->programActiveUniforms();
                programUniforms = &programUniformsContainer;
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::copyProgramActiveUniforms
// Description: Copies a given program uniforms.
// Arguments:   programName - The queried program name.
//              programUniforms - Will get the program uniform names, types and
//                                values.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        60/6/2015
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::copyProgramActiveUniforms(GLuint programName, apGLItemsCollection& programUniforms) const
{
    bool retVal = false;

    const apGLItemsCollection* pUniforms = nullptr;
    bool rcUni = getProgramActiveUniforms(programName, pUniforms);
    GT_IF_WITH_ASSERT(rcUni && (nullptr != pUniforms))
    {
        programUniforms = *pUniforms;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::getProgramActiveUniform
// Description:
//  Queries the name and type of a program's given active uniform.
//  This function signature is similar to the standard glGetActiveUniform function.
//
// Arguments:   programName - The OpenGL name of the program that contains the uniform.
//              index - The uniform index in that program.
//              bufSize - The size of a buffer that will get the uniform name.
//              length - Returns the number of characters actually written into the
//                       uniform name buffer.
//              size -  Returns the size of the uniform variable data.
//              type - Returns the uniform variable type.
//              name - A buffer that will receive the uniform name.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/6/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::getProgramActiveUniform(GLuint programName, GLuint index,
                                                      GLsizei bufSize, GLsizei* pLength,
                                                      GLint* pSize, GLenum* pType, GLchar* pName) const
{
    bool retVal = false;

    GLsizei length;
    GLint size;

    // Get the index of the program in this class vectors:
    int vecIndex = programsVecIndex(programName);

    if (vecIndex != -1)
    {
        // Get the programs uniforms data:
        gsProgramUniformsData* pProgramUniformsData = _programUniformsData[vecIndex];

        if (pProgramUniformsData)
        {
            // Get the program uniforms container:
            const apGLItemsCollection& programUniformsContainer = pProgramUniformsData->programActiveUniforms();

            // Verify that the queried uniform exists in that program:
            unsigned int uniformsAmount = (unsigned int)(programUniformsContainer.amountOfItems());

            if (index < uniformsAmount)
            {
                // Output the uniforms name length:
                const gtString& uniformName = programUniformsContainer.itemName(index);
                length = uniformName.length();
                GLsizei strWriteSpace = bufSize - 1;

                // If the uniform name is larger that the output name buffer,
                // adjust it to the buffer size:
                if (strWriteSpace < length)
                {
                    length = strWriteSpace;
                }

                // Output the uniform name:
                if (pName != NULL)
                {
                    // TO_DO: Unicode: is this safe????
                    pName = (GLchar*)uniformName.asASCIICharArray(length);
                }

                // Output the uniform type:
                if (pType != NULL)
                {
                    *pType = programUniformsContainer.itemType(index);
                }

                // Output the length, if the user requested it
                if (pLength != NULL)
                {
                    *pLength = length;
                }

                // Output the uniform size: According to OpenGL documentation, size should
                // be 1, except for vectors and matrices where size is the amount of elements:
                size = 1;

                const apParameter* pUniformValue = programUniformsContainer.itemValue(index);

                if (pUniformValue)
                {
                    osTransferableObjectType valueType = pUniformValue->type();

                    if (valueType == OS_TOBJ_ID_VECTOR_PARAMETER)
                    {
                        size = ((const apVectorParameter*)pUniformValue)->vectorSize();
                    }
                    else if (valueType == OS_TOBJ_ID_MATRIX_PARAMETER)
                    {
                        unsigned int matSizeN = 0;
                        unsigned int matSizeM = 0;
                        ((const apMatrixParameter*)pUniformValue)->matrixSize(matSizeN, matSizeM);
                        size = matSizeN * matSizeM;
                    }
                }

                if (pSize != NULL)
                {
                    *pSize = size;
                }
            }

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::getUniformLocation
// Description: Returns a program's active uniform location.
// Arguments:   programName - The queried program name.
//              uniformName - The queried uniform name.
// Return Val:  GLint - Will get the uniform location or -1 if it is NA.
// Author:      Yaki Tebeka
// Date:        14/6/2005
// ---------------------------------------------------------------------------
GLint gsActiveUniformsMonitor::getUniformLocation(GLuint programName, const GLchar* uniformName) const
{
    GLint retVal = -1;

    // Get the index of the program in this class vectors:
    int vecIndex = programsVecIndex(programName);

    if (vecIndex != -1)
    {
        // Get the programs uniforms data:
        gsProgramUniformsData* pProgramUniformsData = _programUniformsData[vecIndex];

        if (pProgramUniformsData)
        {
            // TO_DO: Unicode
            gtString uniformNameStr;
            uniformNameStr.fromASCIIString(uniformName);
            retVal = pProgramUniformsData->getUniformLocation(uniformNameStr);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::updateProgramAvailableUniforms
// Description: Updates a given program available active uniforms and
//              stores them in the program's uniforms data.
// Arguments: programName - The given program OpenGL name.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/11/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::updateProgramAvailableUniforms(GLuint programName)
{
    bool retVal = false;

    // Get the index of the program in this class vectors:
    int vecIndex = programsVecIndex(programName);

    if (vecIndex != -1)
    {
        // Get the programs uniforms data:
        gsProgramUniformsData* pProgramUniformsData = _programUniformsData[vecIndex];

        if (pProgramUniformsData)
        {
            // Get the program uniforms container (which we will fill):
            apGLItemsCollection& programUniformsContainer = pProgramUniformsData->programActiveUniforms();

            // Get the program type:
            apGLShadingObjectType programType = pProgramUniformsData->programType();

            if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
            {
                // Update an ARB program available uniforms:
                retVal = updateARBProgramAvailableUniforms(programUniformsContainer, programName);
            }
            else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
            {
                retVal = update20ProgramAvailableUniforms(programUniformsContainer, programName);
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::updateProgramsUniformLocations
// Description:
//   Updates a given program uniforms locations.
//   We assume that the programs uniforms names are already known.
// Arguments:   programName - The input program name.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/6/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::updateProgramsUniformLocations(GLuint programName)
{
    bool retVal = false;

    // Get the index of the program in this class vectors:
    int vecIndex = programsVecIndex(programName);

    if (vecIndex != -1)
    {
        // Get the programs uniforms data:
        gsProgramUniformsData* pProgramUniformsData = _programUniformsData[vecIndex];

        if (pProgramUniformsData)
        {
            retVal = true;

            // Get the program type:
            apGLShadingObjectType programType = pProgramUniformsData->programType();

            // Get the program uniforms collections:
            apGLItemsCollection& uniformsCollection = pProgramUniformsData->programActiveUniforms();

            // Iterate the program uniforms:
            int uniformsAmount = uniformsCollection.amountOfItems();

            for (int i = 0; i < uniformsAmount; i++)
            {
                // Get the current uniform name:
                const gtString& curUniformName = uniformsCollection.itemName(i);

                // Will get the uniform location:
                GLint uniformLocation = -1;

                // If the uniform is not an OpenGL generic uniform:
                // (The reserved prefix "gl_" tell us that this uniform is an OpenGL
                // generic uniform that cannot be queried by the glGetUniformXX functions)
                static gtString reservedUniformsPrefix(L"gl_");

                if (!(curUniformName.startsWith(reservedUniformsPrefix)))
                {
                    // Get the uniform location:
                    uniformLocation = getUniformLocation(programName, programType, curUniformName);
                }

                // Update the uniform location:
                pProgramUniformsData->setUniformLocation(curUniformName, uniformLocation, i);
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::clearProgramActiveUniformsValues
// Description: Clears a given program cached uniform values.
// Author:      Yaki Tebeka
// Date:        15/6/2005
// ---------------------------------------------------------------------------
void gsActiveUniformsMonitor::clearProgramActiveUniformsValues(GLuint programName)
{
    bool rc = false;

    // Get the index of the program in this class vectors:
    int vecIndex = programsVecIndex(programName);

    if (vecIndex != -1)
    {
        // Get the programs uniforms data:
        gsProgramUniformsData* pProgramUniformsData = _programUniformsData[vecIndex];

        if (pProgramUniformsData)
        {
            rc = true;

            // Get the program uniforms container:
            apGLItemsCollection& programUniformsContainer = pProgramUniformsData->programActiveUniforms();

            // Iterate the program active uniforms:
            int activeUniformsAmount = programUniformsContainer.amountOfItems();

            for (int i = 0; i < activeUniformsAmount; i++)
            {
                // Clear the current uniform value:
                gtAutoPtr<apParameter> aptrUniformValue = new apNotAvailableParameter;
                programUniformsContainer.setItemValue(i, aptrUniformValue);
            }
        }
    }

    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::updateProgramActiveUniformsValues
// Description: Updates a given program active uniforms values.
// Arguments: programName - The program who's uniform values will be updated.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/5/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::updateProgramActiveUniformsValues(GLuint programName)
{
    bool retVal = false;

    // Get the index of the program in this class vectors:
    int vecIndex = programsVecIndex(programName);

    if (vecIndex != -1)
    {
        // Get the programs uniforms data:
        gsProgramUniformsData* pProgramUniformsData = _programUniformsData[vecIndex];

        if (pProgramUniformsData)
        {
            retVal = true;

            // Get the program link status:
            bool wasProgramLinkedSuccessfully = pProgramUniformsData->isProgramLinkedSuccessfully();

            // Get the program type:
            apGLShadingObjectType programType = pProgramUniformsData->programType();

            // Get the program uniforms container:
            apGLItemsCollection& programUniformsContainer = pProgramUniformsData->programActiveUniforms();

            // Iterate the program active uniforms:
            int activeUniformsAmount = programUniformsContainer.amountOfItems();

            for (int i = 0; i < activeUniformsAmount; i++)
            {
                // Get the current uniform type and size:
                GLenum curUniformType = programUniformsContainer.itemType(i);
                GLint curUniformSize = programUniformsContainer.itemSize(i);

                // UBO values are set when uniform block binding is set:
                if ((curUniformType == GL_UNIFORM_BUFFER) || (curUniformType == GL_UNIFORM_BUFFER_EXT))
                {
                    continue;
                }

                // Will get the uniform location in its program:
                int uniformLocation = -1;

                // Uniform values are accessible only after a successful link operation:
                if (wasProgramLinkedSuccessfully)
                {
                    // Get the uniform location:
                    const gtString& uniformName = programUniformsContainer.itemName(i);
                    uniformLocation = pProgramUniformsData->getUniformLocation(uniformName);
                }

                // Will get the uniform value:
                gtAutoPtr<apParameter> aptrUniformValue;

                // If the current uniform value is accessible:
                if (uniformLocation != -1)
                {
                    // Get the uniform value:
                    bool rc = getActiveUniformValue(programName, programType, uniformLocation, curUniformType, curUniformSize, aptrUniformValue);
                    retVal = retVal && rc;
                }

                if (nullptr == aptrUniformValue.pointedObject())
                {
                    // The current uniform value is not accessible:
                    aptrUniformValue = new apNotAvailableParameter;
                }

                // Update the uniform value:
                programUniformsContainer.setItemValue(i, aptrUniformValue);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::getUniformLocation
// Description: Retrieves the location of a given uniform in its program.
// Arguments:   programName - The program name.
//              programType - The program type.
//              uniformName - The uniform name.
// Return Val:  int - Returns the uniform location in the above program, or -1
//                    if the program does not contain the input uniform.
// Author:      Yaki Tebeka
// Date:        6/6/2005
// ---------------------------------------------------------------------------
GLint gsActiveUniformsMonitor::getUniformLocation(GLuint programName, apGLShadingObjectType programType, const gtString& uniformName) const
{
    int retVal = -1;

    if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
    {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformLocationARB);
        retVal = gs_stat_realFunctionPointers.glGetUniformLocationARB(programName, uniformName.asCharArray());
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformLocationARB);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformLocationARB);
        retVal = _glGetUniformLocationARB(programName, uniformName.asASCIICharArray());
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformLocationARB);
#endif
    }
    else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
    {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformLocation);
        retVal = gs_stat_realFunctionPointers.glGetUniformLocation(programName, uniformName.asCharArray());
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformLocation);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformLocation);
        retVal = _glGetUniformLocation(programName, uniformName.asASCIICharArray());
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformLocation);
#endif
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::getActiveUniformValue
// Description: Retrieves an active uniform value.
// Arguments:   programName - The active uniform program name.
//              programType - The program type.
//              uniformLocation - The uniform location in its program.
//              uniformType - The uniform type.
//              uniformSize - The uniform size (>1 if this is an array).
//              aptrUniformValue - Will get the uniform value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/5/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::getActiveUniformValue(GLuint programName, apGLShadingObjectType programType,
                                                    GLint uniformLocation, GLenum uniformType, GLint uniformSize,
                                                    gtAutoPtr<apParameter>& aptrUniformValue)
{
    bool retVal = false;

    switch (uniformType)
    {
        case GL_FLOAT:
        {
            // If this is a single item uniform:
            if (uniformSize == 1)
            {
                retVal = getFloatUniformValue(programName, programType, uniformLocation, aptrUniformValue);
            }
            else
            {
                // This is a vector uniform:
                retVal = getFloatVecUniformValue(programName, programType, uniformLocation, uniformSize, aptrUniformValue);
            }
        }
        break;

        case GL_FLOAT_VEC2:
            retVal = getFloatVecUniformValue(programName, programType, uniformLocation, 2 * uniformSize, aptrUniformValue);
            break;

        case GL_FLOAT_VEC3:
            retVal = getFloatVecUniformValue(programName, programType, uniformLocation, 3 * uniformSize, aptrUniformValue);
            break;

        case GL_FLOAT_VEC4:
            retVal = getFloatVecUniformValue(programName, programType, uniformLocation, 4 * uniformSize, aptrUniformValue);
            break;

        // Square matrices:
        case GL_FLOAT_MAT2:
            retVal = getFloatMatUniformValue(programName, programType, uniformLocation, 2 * uniformSize, 2, aptrUniformValue);
            break;

        case GL_FLOAT_MAT3:
            retVal = getFloatMatUniformValue(programName, programType, uniformLocation, 3 * uniformSize, 3, aptrUniformValue);
            break;

        case GL_FLOAT_MAT4:
            retVal = getFloatMatUniformValue(programName, programType, uniformLocation, 4 * uniformSize, 4, aptrUniformValue);
            break;

        // Non square matrices:
        case GL_FLOAT_MAT2x3:
            retVal = getFloatMatUniformValue(programName, programType, uniformLocation, 2 * uniformSize, 3, aptrUniformValue);
            break;

        case GL_FLOAT_MAT2x4:
            retVal = getFloatMatUniformValue(programName, programType, uniformLocation, 2 * uniformSize, 4, aptrUniformValue);
            break;

        case GL_FLOAT_MAT3x2:
            retVal = getFloatMatUniformValue(programName, programType, uniformLocation, 3 * uniformSize, 2, aptrUniformValue);
            break;

        case GL_FLOAT_MAT3x4:
            retVal = getFloatMatUniformValue(programName, programType, uniformLocation, 3 * uniformSize, 4, aptrUniformValue);
            break;

        case GL_FLOAT_MAT4x2:
            retVal = getFloatMatUniformValue(programName, programType, uniformLocation, 4 * uniformSize, 2, aptrUniformValue);
            break;

        case GL_FLOAT_MAT4x3:
            retVal = getFloatMatUniformValue(programName, programType, uniformLocation, 4 * uniformSize, 3, aptrUniformValue);
            break;

        case GL_INT:
        case GL_BOOL:
        {
            // If this is a single item uniform:
            if (uniformSize == 1)
            {
                retVal = getIntUniformValue(programName, programType, uniformLocation, aptrUniformValue);
            }
            else
            {
                // This is a vector uniform:
                retVal = getIntVecUniformValue(programName, programType, uniformLocation, uniformSize, aptrUniformValue);
            }
        }
        break;

        case GL_INT_VEC2:
        case GL_BOOL_VEC2:
            retVal = getIntVecUniformValue(programName, programType, uniformLocation, 2 * uniformSize, aptrUniformValue);
            break;

        case GL_INT_VEC3:
        case GL_BOOL_VEC3:
            retVal = getIntVecUniformValue(programName, programType, uniformLocation, 3 * uniformSize, aptrUniformValue);
            break;

        case GL_INT_VEC4:
        case GL_BOOL_VEC4:
            retVal = getIntVecUniformValue(programName, programType, uniformLocation, 4 * uniformSize, aptrUniformValue);
            break;

        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_CUBE_MAP_ARRAY:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
            // Sampler uniforms have an int value which signifies which texture unit they use:
            retVal = getIntUniformValue(programName, programType, uniformLocation, aptrUniformValue);
            break;

        default:
        {
            // Unknown uniform type:
            aptrUniformValue = new apNotAvailableParameter;
            retVal = true;

            // Getting this assert means that there is a new uniform type in
            // OpenGL that we need to support.
            // Notice: Support should be added also to setActiveUniformValue)

            gtString errString;
            apGLenumValueToString(uniformType, errString);
            errString.prepend(L"Unknown uniform type used: ");
            GT_ASSERT_EX(false, errString.asCharArray());
        }
        break;
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::getFloatUniformValue
// Description: Queried a float uniform value.
// Arguments:   programName - The queried uniforms program.
//              programType - The program type.
//              uniformLocation - The uniforms location in its program.
//              aptrUniformValue - will get the uniform value.
// Author:      Yaki Tebeka
// Date:        28/5/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::getFloatUniformValue(GLuint programName, apGLShadingObjectType programType,
                                                   GLint uniformLocation, gtAutoPtr<apParameter>& aptrUniformValue)
{
    bool retVal = true;

    // Get the uniform value:
    GLfloat value = 0;

    if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
    {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformfvARB);
        gs_stat_realFunctionPointers.glGetUniformfvARB(programName, uniformLocation, &value);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformfvARB);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformfvARB);
        _glGetUniformfvARB(programName, uniformLocation, &value);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformfvARB);
#endif
    }
    else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
    {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformfv);
        gs_stat_realFunctionPointers.glGetUniformfv(programName, uniformLocation, &value);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformfv);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformfv);
        _glGetUniformfv(programName, uniformLocation, &value);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformfv);
#endif
    }
    else
    {
        retVal = false;
    }

    if (retVal)
    {
        // Output the uniform value:
        apGLfloatParameter* pValueAsParam = new apGLfloatParameter(value);

        aptrUniformValue = pValueAsParam;

        retVal = (pValueAsParam != NULL);
    }
    else
    {
        aptrUniformValue = new apNotAvailableParameter;
    }

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::getFloatVecUniformValue
// Description: Queried a float vector uniform value.
// Arguments:   programName - The queried uniforms program.
//              programType - The program type.
//              uniformLocation - The uniforms location in its program.
//              vecSize - the amount of vector elements.
//              aptrUniformValue - will get the uniform value.
// Author:      Yaki Tebeka
// Date:        28/5/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::getFloatVecUniformValue(GLuint programName, apGLShadingObjectType programType,
                                                      GLint uniformLocation, int vecSize, gtAutoPtr<apParameter>& aptrUniformValue)
{
    bool retVal = false;

    // Sanity check:
    if (2 <= vecSize)
    {
        // A C vector that will contain the uniform value:
        GLfloat* valuesVec = new GLfloat[vecSize];

        // Get the uniform value:
        bool gotUniformValue = true;

        if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformfvARB);
            gs_stat_realFunctionPointers.glGetUniformfvARB(programName, uniformLocation, valuesVec);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformfvARB);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformfvARB);
            _glGetUniformfvARB(programName, uniformLocation, valuesVec);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformfvARB);
#endif
        }
        else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformfv);
            gs_stat_realFunctionPointers.glGetUniformfv(programName, uniformLocation, valuesVec);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformfv);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformfv);
            _glGetUniformfv(programName, uniformLocation, valuesVec);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformfv);
#endif
        }
        else
        {
            gotUniformValue = false;
        }

        if (gotUniformValue)
        {
            // Create the vector parameter:
            apVectorParameter* pValueAsVecParam = new apVectorParameter(vecSize);

            if (pValueAsVecParam)
            {
                // Fill the vector parameter:
                for (int i = 0; i < vecSize; i++)
                {
                    apGLfloatParameter* pCurrentVecItemAsParam = new apGLfloatParameter(valuesVec[i]);

                    if (pCurrentVecItemAsParam)
                    {
                        gtAutoPtr<apParameter> aptrCurrentVecItem(pCurrentVecItemAsParam);
                        pValueAsVecParam->setItem(i, aptrCurrentVecItem);
                    }
                }

                // Output the vector:
                aptrUniformValue = pValueAsVecParam;
                retVal = true;
            }
        }

        // Clean up:
        delete[] valuesVec;
    }

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::getFloatMatUniformValue
// Description: Queried a float matrix uniform value.
// Arguments:   programName - The queried uniforms program.
//              programType - The program type.
//              uniformLocation - The uniforms location in its program.
//              matSizeN, matSizeM - the matrix dimensions.
//              aptrUniformValue - will get the uniform value.
// Author:      Yaki Tebeka
// Date:        28/5/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::getFloatMatUniformValue(GLuint programName, apGLShadingObjectType programType,
                                                      GLint uniformLocation, int matSizeN, int matSizeM,
                                                      gtAutoPtr<apParameter>& aptrUniformValue)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((2 <= matSizeN) && (matSizeN <= 4) && (2 <= matSizeM) && (matSizeM <= 4))
    {
        // A C vector that will contain the uniform value:
        GLfloat valueVec[16];

        // Get the uniform value:
        bool gotUniformValue = true;

        if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformfvARB);
            gs_stat_realFunctionPointers.glGetUniformfvARB(programName, uniformLocation, valueVec);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformfvARB);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformfvARB);
            _glGetUniformfvARB(programName, uniformLocation, valueVec);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformfvARB);
#endif
        }
        else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformfv);
            gs_stat_realFunctionPointers.glGetUniformfv(programName, uniformLocation, valueVec);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformfv);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformfv);
            _glGetUniformfv(programName, uniformLocation, valueVec);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformfv);
#endif
        }
        else
        {
            gotUniformValue = false;
        }

        if (gotUniformValue)
        {
            // Create the matrix parameter:
            apMatrixParameter* pValueAsMatParam = new apMatrixParameter(matSizeN, matSizeM);

            // Fill the matrix parameter:
            for (int i = 0; i < matSizeN; i++)
            {
                for (int j = 0; j < matSizeM; j++)
                {
                    int itemLocation = (i * matSizeN) + j;
                    apGLfloatParameter* pCurrentVecItemAsParam = new apGLfloatParameter(valueVec[itemLocation]);

                    if (pCurrentVecItemAsParam)
                    {
                        gtAutoPtr<apParameter> aptrCurrentVecItem(pCurrentVecItemAsParam);
                        pValueAsMatParam->setItem(i, j, aptrCurrentVecItem);
                    }
                }
            }

            // Output the vector:
            aptrUniformValue = pValueAsMatParam;
            retVal = true;
        }
    }

    GT_RETURN_WITH_ASSERT(retVal);
}

// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::getIntUniformValue
// Description: Queried a int uniform value.
// Arguments:   programName - The queried uniforms program.
//              programType - The program type.
//              uniformLocation - The uniforms location in its program.
//              aptrUniformValue - will get the uniform value.
// Author:      Yaki Tebeka
// Date:        28/5/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::getIntUniformValue(GLuint programName, apGLShadingObjectType programType,
                                                 GLint uniformLocation, gtAutoPtr<apParameter>& aptrUniformValue)
{
    bool retVal = true;

    // Get the uniform value:
    GLint value = 0;

    if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
    {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformivARB);
        gs_stat_realFunctionPointers.glGetUniformivARB(programName, uniformLocation, &value);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformivARB);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformivARB);
        _glGetUniformivARB(programName, uniformLocation, &value);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformivARB);
#endif
    }
    else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
    {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformiv);
        gs_stat_realFunctionPointers.glGetUniformiv(programName, uniformLocation, &value);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformiv);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformiv);
        _glGetUniformiv(programName, uniformLocation, &value);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformiv);
#endif
    }
    else
    {
        retVal = false;
    }

    if (retVal)
    {
        // Output the uniform value:
        apGLintParameter* pValueAsParam = new apGLintParameter(value);
        aptrUniformValue = pValueAsParam;

        retVal = (pValueAsParam != NULL);
    }
    else
    {
        aptrUniformValue = new apNotAvailableParameter;
    }

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::getIntVecUniformValue
// Description: Queried a int vector uniform value.
// Arguments:   programName - The queried uniforms program.
//              programType - The program type.
//              uniformLocation - The uniforms location in its program.
//              vecSize - the amount of vector elements.
//              aptrUniformValue - will get the uniform value.
// Author:      Yaki Tebeka
// Date:        28/5/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::getIntVecUniformValue(GLuint programName, apGLShadingObjectType programType,
                                                    GLint uniformLocation, int vecSize, gtAutoPtr<apParameter>& aptrUniformValue)
{
    bool retVal = false;

    // Sanity check:
    if (2 <= vecSize)
    {
        // A C vector that will contain the uniform value:
        GLint* valuesVec = new GLint[vecSize];

        if (valuesVec)
        {
            // Get the uniform value:
            bool gotUniformValue = true;

            if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
            {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformivARB);
                gs_stat_realFunctionPointers.glGetUniformivARB(programName, uniformLocation, valuesVec);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformivARB);
#else
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformivARB);
                _glGetUniformivARB(programName, uniformLocation, valuesVec);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformivARB);
#endif
            }
            else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
            {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformiv);
                gs_stat_realFunctionPointers.glGetUniformiv(programName, uniformLocation, valuesVec);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformiv);
#else
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformiv);
                _glGetUniformiv(programName, uniformLocation, valuesVec);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformiv);
#endif
            }
            else
            {
                gotUniformValue = false;
            }

            if (gotUniformValue)
            {
                // Create the vector parameter:
                apVectorParameter* pValueAsVecParam = new apVectorParameter(vecSize);

                if (pValueAsVecParam)
                {
                    // Fill the vector parameter:
                    for (int i = 0; i < vecSize; i++)
                    {
                        apGLintParameter* pCurrentVecItemAsParam = new apGLintParameter(valuesVec[i]);

                        if (pCurrentVecItemAsParam)
                        {
                            gtAutoPtr<apParameter> aptrCurrentVecItem(pCurrentVecItemAsParam);
                            pValueAsVecParam->setItem(i, aptrCurrentVecItem);
                        }
                    }

                    // Output the vector:
                    aptrUniformValue = pValueAsVecParam;
                    retVal = true;
                }
            }

            // Clean up:
            delete[] valuesVec;
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::getIntMatUniformValue
// Description: Queried a int matrix uniform value.
// Arguments:   programName - The queried uniforms program.
//              programType - The program type.
//              uniformLocation - The uniforms location in its program.
//              matSize - the matrix size (matSize X matSize).
//              aptrUniformValue - will get the uniform value.
// Author:      Yaki Tebeka
// Date:        28/5/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::getIntMatUniformValue(GLuint programName, apGLShadingObjectType programType,
                                                    GLint uniformLocation, int matSize, gtAutoPtr<apParameter>& aptrUniformValue)
{
    bool retVal = false;

    // Sanity check:
    if ((2 <= matSize) && (matSize <= 4))
    {
        // A C vector that will contain the uniform value:
        GLint valueVec[16];

        // Get the uniform value:
        bool gotUniformValue = true;

        if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformivARB);
            gs_stat_realFunctionPointers.glGetUniformivARB(programName, uniformLocation, valueVec);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformivARB);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformivARB);
            _glGetUniformivARB(programName, uniformLocation, valueVec);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformivARB);
#endif
        }
        else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformiv);
            gs_stat_realFunctionPointers.glGetUniformiv(programName, uniformLocation, valueVec);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformiv);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformiv);
            _glGetUniformiv(programName, uniformLocation, valueVec);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformiv);
#endif
        }
        else
        {
            gotUniformValue = false;
        }

        if (gotUniformValue)
        {
            // Create the matrix parameter:
            apMatrixParameter* pValueAsMatParam = new apMatrixParameter(matSize, matSize);

            if (pValueAsMatParam)
            {
                // Fill the matrix parameter:
                for (int i = 0; i < matSize; i++)
                {
                    for (int j = 0; j < matSize; i++)
                    {
                        int itemLocation = i * matSize + j;
                        apGLintParameter* pCurrentVecItemAsParam = new apGLintParameter(valueVec[itemLocation]);

                        if (pCurrentVecItemAsParam)
                        {
                            gtAutoPtr<apParameter> aptrCurrentVecItem(pCurrentVecItemAsParam);
                            pValueAsMatParam->setItem(i, j, aptrCurrentVecItem);
                        }
                    }
                }

                // Output the vector:
                aptrUniformValue = pValueAsMatParam;
                retVal = true;
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::setActiveUniformValue
// Description:
//  Inputs a program name, a uniform and its value, and sets the uniform
//  value to the input value.
//
// Author:      Yaki Tebeka
// Date:        29/05/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::setActiveUniformValue(GLuint programName, apGLShadingObjectType programType,
                                                    GLenum uniformType, int uniformLocation, const apParameter& uniformValue)
{
    bool retVal = false;

    switch (uniformType)
    {
        case GL_FLOAT:
        {
            // If this is a vector of floats:
            if (uniformValue.type() == OS_TOBJ_ID_VECTOR_PARAMETER)
            {
                retVal = setFloatVecUniformValue(programName, programType, uniformLocation, uniformValue);
            }
            else
            {
                // This is a single float:
                retVal = setFloatUniformValue(programName, programType, uniformLocation, uniformValue);
            }
        }
        break;

        case GL_FLOAT_VEC2:
            retVal = setFloatVecUniformValue(programName, programType, uniformLocation, uniformValue);
            break;

        case GL_FLOAT_VEC3:
            retVal = setFloatVecUniformValue(programName, programType, uniformLocation, uniformValue);
            break;

        case GL_FLOAT_VEC4:
            retVal = setFloatVecUniformValue(programName, programType, uniformLocation, uniformValue);
            break;

        case GL_FLOAT_MAT2:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:
        case GL_FLOAT_MAT2x3:
        case GL_FLOAT_MAT2x4:
        case GL_FLOAT_MAT3x2:
        case GL_FLOAT_MAT3x4:
        case GL_FLOAT_MAT4x2:
        case GL_FLOAT_MAT4x3:
            retVal = setFloatMatUniformValue(programName, programType, uniformLocation, uniformValue);
            break;

        case GL_INT:
        case GL_BOOL:
        {
            // If this is a vector of int / bool items:
            if (uniformValue.type() == OS_TOBJ_ID_VECTOR_PARAMETER)
            {
                retVal = setIntVecUniformValue(programName, programType, uniformLocation, uniformValue);
            }
            else
            {
                // This is a single int / bool:
                retVal = setIntUniformValue(programName, programType, uniformLocation, uniformValue);
            }
        }
        break;

        case GL_INT_VEC2:
        case GL_BOOL_VEC2:
            retVal = setIntVecUniformValue(programName, programType, uniformLocation, uniformValue);
            break;

        case GL_INT_VEC3:
        case GL_BOOL_VEC3:
            retVal = setIntVecUniformValue(programName, programType, uniformLocation, uniformValue);
            break;

        case GL_INT_VEC4:
        case GL_BOOL_VEC4:
            retVal = setIntVecUniformValue(programName, programType, uniformLocation, uniformValue);
            break;

        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_CUBE_MAP_ARRAY:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
        {
            // Sampler uniforms have an int value which signifies which texture unit they use:
            retVal = setIntUniformValue(programName, programType, uniformLocation, uniformValue);
        }
        break;

        default:
        {
            // Getting this assert means that there is a new uniform type in
            // OpenGL that we need to support.
            // Notice: Support should be added also to getActiveUniformValue)
            GT_ASSERT(0);
            retVal = true;
        }
        break;
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::setFloatUniformValue
// Description:
//  Inputs a program name, a float uniform and its value, and sets the uniform
//  value to the input value.
//
// Author:      Yaki Tebeka
// Date:        29/05/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::setFloatUniformValue(GLuint programName, apGLShadingObjectType programType,
                                                   GLint uniformLocation, const apParameter& uniformValue)
{
    (void)(programName); // unused

    bool retVal = false;

    // Sanity check:
    if (uniformValue.type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
    {
        // Get the float input value:
        GLfloat uniformValAsFloat = ((const apGLfloatParameter&)uniformValue).value();

        // Set the uniform value:
        if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1fvARB);
            gs_stat_realFunctionPointers.glUniform1fvARB(uniformLocation, 1, &uniformValAsFloat);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1fvARB);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1fvARB);
            _glUniform1fvARB(uniformLocation, 1, &uniformValAsFloat);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1fvARB);
#endif
        }
        else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1fv);
            gs_stat_realFunctionPointers.glUniform1fv(uniformLocation, 1, &uniformValAsFloat);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1fv);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1fv);
            _glUniform1fv(uniformLocation, 1, &uniformValAsFloat);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1fv);
#endif
        }

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::setFloatVecUniformValue
// Description:
//  Inputs a program name, a float vector uniform and its value, and sets the uniform
//  value to the input value.
//
// Author:      Yaki Tebeka
// Date:        29/05/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::setFloatVecUniformValue(GLuint programName, apGLShadingObjectType programType,
                                                      GLint uniformLocation, const apParameter& uniformValue)
{
    (void)(programName); // unused
    bool retVal = false;

    // Sanity check:
    if (uniformValue.type() == OS_TOBJ_ID_VECTOR_PARAMETER)
    {
        // Cast it down to a vector parameter:
        const apVectorParameter& vectorParam = (const apVectorParameter&)uniformValue;

        // Get the amount of vector items:
        int itemsAmount = vectorParam.vectorSize();

        // Sanity check:
        if (itemsAmount <= 4)
        {
            retVal = true;

            // Translate the vector into a C vector:
            GLfloat oglVec[4];

            for (int i = 0; i < itemsAmount; i++)
            {
                const apParameter* pCurrentItem = vectorParam[i];

                if (pCurrentItem && (pCurrentItem->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER))
                {
                    oglVec[i] = ((const apGLfloatParameter*)pCurrentItem)->value();
                }
                else
                {
                    retVal = false;
                }
            }

            if (retVal)
            {
                // Set the uniform value:
                if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
                {
                    switch (itemsAmount)
                    {
                        case 1:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1fvARB);
                            gs_stat_realFunctionPointers.glUniform1fvARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1fvARB);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1fvARB);
                            _glUniform1fvARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1fvARB);
#endif
                        }
                        break;

                        case 2:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform2fvARB);
                            gs_stat_realFunctionPointers.glUniform2fvARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform2fvARB);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform2fvARB);
                            _glUniform2fvARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform2fvARB);
#endif
                        }
                        break;

                        case 3:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform3fvARB);
                            gs_stat_realFunctionPointers.glUniform3fvARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform3fvARB);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform3fvARB);
                            _glUniform3fvARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform3fvARB);
#endif
                        }
                        break;

                        case 4:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform4fvARB);
                            gs_stat_realFunctionPointers.glUniform4fvARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform4fvARB);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform4fvARB);
                            _glUniform4fvARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform4fvARB);
#endif
                        }
                        break;

                        default:
                        {
                            GT_ASSERT(0);
                            retVal = false;
                        }
                        break;
                    }
                }
                else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
                {
                    switch (itemsAmount)
                    {
                        case 1:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1fv);
                            gs_stat_realFunctionPointers.glUniform1fv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1fv);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1fv);
                            _glUniform1fv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1fv);
#endif
                        }
                        break;

                        case 2:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform2fv);
                            gs_stat_realFunctionPointers.glUniform2fv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform2fv);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform2fv);
                            _glUniform2fv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform2fv);
#endif
                        }
                        break;

                        case 3:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform3fv);
                            gs_stat_realFunctionPointers.glUniform3fv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform3fv);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform3fv);
                            _glUniform3fv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform3fv);
#endif
                        }
                        break;

                        case 4:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform4fv);
                            gs_stat_realFunctionPointers.glUniform4fv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform4fv);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform4fv);
                            _glUniform4fv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform4fv);
#endif
                        }
                        break;

                        default:
                        {
                            GT_ASSERT(0);
                            retVal = false;
                        }
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::setFloatMatUniformValue
// Description:
//  Inputs a program name, a float uniform matrix and its value, and sets the uniform
//  value to the input value.
//
// Author:      Yaki Tebeka
// Date:        29/05/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::setFloatMatUniformValue(GLuint programName, apGLShadingObjectType programType,
                                                      GLint uniformLocation, const apParameter& uniformValue)
{
    (void)(programName); // unused
    bool retVal = false;

    // Sanity check:
    if (uniformValue.type() == OS_TOBJ_ID_MATRIX_PARAMETER)
    {
        // Cast it down to a matrix parameter:
        const apMatrixParameter& matrixParam = (const apMatrixParameter&)uniformValue;

        // Get the amount of matrix items:
        unsigned int sizeN = 0;
        unsigned int sizeM = 0;
        matrixParam.matrixSize(sizeN, sizeM);

        // Sanity check:
        if ((sizeN <= 4) && (sizeM <= 4))
        {
            retVal = true;

            // Translate the matrix into a C vector:
            GLfloat oglMat[16];

            for (unsigned int i = 0; i < sizeN; i++)
            {
                for (unsigned int j = 0; j < sizeM; j++)
                {
                    const apParameter* pCurrentItem = matrixParam.element(i, j);

                    if (pCurrentItem && (pCurrentItem->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER))
                    {
                        oglMat[(i * sizeN) + j] = ((const apGLfloatParameter*)pCurrentItem)->value();
                    }
                    else
                    {
                        retVal = false;
                    }
                }
            }

            if (retVal)
            {
                // If this is a square matrix:
                if (sizeN == sizeM)
                {
                    // Set the uniform value:
                    int matSize = sizeN;

                    if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
                    {
                        switch (matSize)
                        {
                            case 2:
                            {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix2fvARB);
                                gs_stat_realFunctionPointers.glUniformMatrix2fvARB(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix2fvARB);
#else
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix2fvARB);
                                _glUniformMatrix2fvARB(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix2fvARB);
#endif
                            }
                            break;

                            case 3:
                            {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix3fvARB);
                                gs_stat_realFunctionPointers.glUniformMatrix3fvARB(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix3fvARB);
#else
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix3fvARB);
                                _glUniformMatrix3fvARB(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix3fvARB);
#endif
                            }
                            break;

                            case 4:
                            {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix4fvARB);
                                gs_stat_realFunctionPointers.glUniformMatrix4fvARB(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix4fvARB);
#else
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix4fvARB);
                                _glUniformMatrix4fvARB(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix4fvARB);
#endif
                            }
                            break;

                            default:
                            {
                                GT_ASSERT(false);
                                retVal = false;
                            }
                            break;
                        }
                    }
                    else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
                    {
                        switch (matSize)
                        {
                            case 2:
                            {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix2fv);
                                gs_stat_realFunctionPointers.glUniformMatrix2fv(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix2fv);
#else
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix2fv);
                                _glUniformMatrix2fv(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix2fv);
#endif
                            }
                            break;

                            case 3:
                            {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix3fv);
                                gs_stat_realFunctionPointers.glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix3fv);
#else
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix3fv);
                                _glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix3fv);
#endif
                            }
                            break;

                            case 4:
                            {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix4fv);
                                gs_stat_realFunctionPointers.glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix4fv);
#else
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix4fv);
                                _glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix4fv);
#endif
                            }
                            break;

                            default:
                            {
                                GT_ASSERT(false);
                                retVal = false;
                            }
                            break;
                        }
                    }
                }
                else
                {
                    // This is a non-square matrix:

                    // Non-square matrices were introduced in OpenGL 2.1:
                    GT_IF_WITH_ASSERT((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL21Supported)
                    {
                        if (sizeN == 2)
                        {
                            if (sizeM == 3)
                            {
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix2x3fv);
                                _glUniformMatrix2x3fv(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix2x3fv);
                            }
                            else if (sizeM == 4)
                            {
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix2x4fv);
                                _glUniformMatrix2x4fv(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix2x4fv);
                            }
                            else
                            {
                                GT_ASSERT(false);
                                retVal = false;
                            }
                        }
                        else if (sizeN == 3)
                        {
                            if (sizeM == 2)
                            {
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix3x2fv);
                                _glUniformMatrix3x2fv(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix3x2fv);
                            }
                            else if (sizeM == 4)
                            {
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix3x4fv);
                                _glUniformMatrix3x4fv(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix3x4fv);
                            }
                            else
                            {
                                GT_ASSERT(false);
                                retVal = false;
                            }
                        }
                        else if (sizeN == 4)
                        {
                            if (sizeM == 2)
                            {
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix4x2fv);
                                _glUniformMatrix4x2fv(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix4x2fv);
                            }
                            else if (sizeM == 3)
                            {
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix4x3fv);
                                _glUniformMatrix4x3fv(uniformLocation, 1, GL_FALSE, oglMat);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniformMatrix4x3fv);
                            }
                            else
                            {
                                GT_ASSERT(false);
                                retVal = false;
                            }
                        }
                        else
                        {
                            GT_ASSERT(false);
                            retVal = false;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::setIntUniformValue
// Description:
//  Inputs a program name, an int vector uniform and its value, and sets the uniform
//  value to the input value.
//
// Author:      Yaki Tebeka
// Date:        29/05/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::setIntUniformValue(GLuint programName, apGLShadingObjectType programType,
                                                 GLint uniformLocation, const apParameter& uniformValue)
{
    (void)(programName); // unused
    bool retVal = false;

    // Sanity check:
    if (uniformValue.type() == OS_TOBJ_ID_GL_INT_PARAMETER)
    {
        // Get the int input value:
        GLint uniformValAsInt = ((const apGLintParameter&)uniformValue).value();

        // Set the uniform value:
        if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1ivARB);
            gs_stat_realFunctionPointers.glUniform1ivARB(uniformLocation, 1, &uniformValAsInt);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1ivARB);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1ivARB);
            _glUniform1ivARB(uniformLocation, 1, &uniformValAsInt);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1ivARB);
#endif
        }
        else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1iv);
            gs_stat_realFunctionPointers.glUniform1iv(uniformLocation, 1, &uniformValAsInt);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1iv);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1iv);
            _glUniform1iv(uniformLocation, 1, &uniformValAsInt);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1iv);
#endif
        }

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::setIntVecUniformValue
// Description:
//  Inputs a program name, an int uniform vector and its value, and sets the uniform
//  value to the input value.
//
// Author:      Yaki Tebeka
// Date:        29/05/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::setIntVecUniformValue(GLuint programName, apGLShadingObjectType programType,
                                                    GLint uniformLocation, const apParameter& uniformValue)
{
    (void)(programName); // unused
    bool retVal = false;

    // Sanity check:
    if (uniformValue.type() == OS_TOBJ_ID_VECTOR_PARAMETER)
    {
        // Cast it down to a vector parameter:
        const apVectorParameter& vectorParam = (const apVectorParameter&)uniformValue;

        // Get the amount of vector items:
        int itemsAmount = vectorParam.vectorSize();

        // Sanity check:
        if (itemsAmount <= 16)
        {
            retVal = true;

            // Translate the vector into a C vector:
            GLint oglVec[16];

            for (int i = 0; i < itemsAmount; i++)
            {
                const apParameter* pCurrentItem = vectorParam[i];

                if (pCurrentItem && (pCurrentItem->type() == OS_TOBJ_ID_GL_INT_PARAMETER))
                {
                    oglVec[i] = ((const apGLintParameter*)pCurrentItem)->value();
                }
                else
                {
                    retVal = false;
                }
            }

            if (retVal)
            {
                // Set the uniform value:
                if ((programType == AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT) && _isARBShaderObjectsExtSupported)
                {
                    switch (itemsAmount)
                    {
                        case 1:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1ivARB);
                            gs_stat_realFunctionPointers.glUniform1ivARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1ivARB);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1ivARB);
                            _glUniform1ivARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1ivARB);
#endif
                        }
                        break;

                        case 2:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform2ivARB);
                            gs_stat_realFunctionPointers.glUniform2ivARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform2ivARB);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform2ivARB);
                            _glUniform2ivARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform2ivARB);
#endif
                        }
                        break;

                        case 3:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform3ivARB);
                            gs_stat_realFunctionPointers.glUniform3ivARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform3ivARB);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform3ivARB);
                            _glUniform3ivARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform3ivARB);
#endif
                        }
                        break;

                        case 4:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform4ivARB);
                            gs_stat_realFunctionPointers.glUniform4ivARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform4ivARB);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform4ivARB);
                            _glUniform4ivARB(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform4ivARB);
#endif
                        }
                        break;

                        default:
                        {
                            GT_ASSERT(0);
                            retVal = false;
                        }
                        break;
                    }
                }
                else if ((programType == AP_GL_2_0_SHADING_OBJECT) && _isOpenGL20Supported)
                {
                    switch (itemsAmount)
                    {
                        case 1:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1iv);
                            gs_stat_realFunctionPointers.glUniform1iv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1iv);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform1iv);
                            _glUniform1iv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform1iv);
#endif
                        }
                        break;

                        case 2:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform2iv);
                            gs_stat_realFunctionPointers.glUniform2iv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform2iv);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform2iv);
                            _glUniform2iv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform2iv);
#endif
                        }
                        break;

                        case 3:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform3iv);
                            gs_stat_realFunctionPointers.glUniform3iv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform3iv);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform3iv);
                            _glUniform3iv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform3iv);
#endif
                        }
                        break;

                        case 4:
                        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform4iv);
                            gs_stat_realFunctionPointers.glUniform4iv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform4iv);
#else
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glUniform4iv);
                            _glUniform4iv(uniformLocation, 1, oglVec);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glUniform4iv);
#endif
                        }
                        break;

                        default:
                        {
                            GT_ASSERT(0);
                            retVal = false;
                        }
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::programsVecIndex
// Description:
//  Inputs a program name and outputs its index in this class vectors,
//  (or -1 if it is not registered in this class vectors)
//
// Author:      Yaki Tebeka
// Date:        29/05/2005
// ---------------------------------------------------------------------------
int gsActiveUniformsMonitor::programsVecIndex(GLuint programName) const
{
    int retVal = -1;

    // Look for the program name in the _programNameToVecIndex map:
    gtMap<GLuint, int>::const_iterator iter = _programNameToVecIndex.find(programName);

    // If we found the program id:
    if (iter != _programNameToVecIndex.end())
    {
        // Return the program id:
        retVal = (*iter).second;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::multiParameterFromVoidPointer
// Description: Creates a apVectorParameter or apMatrixParameter from values
// Arguments: width - the vector length or matrix width
//            height - the matrix height or 1 for vectors
//            isInteger - false = float, true = int / boolean
// Author:      Uri Shomroni
// Date:        16/12/2009
// ---------------------------------------------------------------------------
void gsActiveUniformsMonitor::multiParameterFromVoidPointer(int width, int height, bool isInteger, void* values, gtAutoPtr<apParameter>& aptrParameter, bool transpose)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((width > 1) && (width <= 4) && (height > 0) && (height <= 4) && (values != NULL))
    {
        if (height == 1) // This is a vector
        {
            apVectorParameter* pVec = new apVectorParameter(width);

            for (int i = 0; i < width; i++)
            {
                if (isInteger)
                {
                    gtAutoPtr<apParameter> aptrCurrentItem = new apGLintParameter(((GLint*)values)[i]);
                    pVec->setItem(i, aptrCurrentItem);
                }
                else //! isInteger
                {
                    gtAutoPtr<apParameter> aptrCurrentItem = new apGLfloatParameter(((GLfloat*)values)[i]);
                    pVec->setItem(i, aptrCurrentItem);
                }
            }

            aptrParameter = pVec;
        }
        else // height > 1 - this is a matrix
        {
            int totalItems = width * height;
            apMatrixParameter* pMat = new apMatrixParameter(width, height);

            for (int i = 0; i < totalItems; i++)
            {
                if (isInteger)
                {
                    gtAutoPtr<apParameter> aptrCurrentItem = new apGLintParameter(((GLint*)values)[i]);

                    if (transpose)
                    {
                        pMat->setItem((i % width), (i / width), aptrCurrentItem);
                    }
                    else
                    {
                        pMat->setItem((i / width), (i % width), aptrCurrentItem);
                    }
                }
                else //! isInteger
                {
                    gtAutoPtr<apParameter> aptrCurrentItem = new apGLfloatParameter(((GLfloat*)values)[i]);

                    if (transpose)
                    {
                        pMat->setItem((i % width), (i / width), aptrCurrentItem);
                    }
                    else
                    {
                        pMat->setItem((i / width), (i % width), aptrCurrentItem);
                    }
                }
            }

            aptrParameter = pMat;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::updateARBProgramAvailableUniforms
// Description: Updates available uniform for an ARB program
// Arguments: apGLItemsCollection& programUniformsContainer
//            GLuint programName
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/11/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::updateARBProgramAvailableUniforms(apGLItemsCollection& programUniformsContainer, GLuint programName)
{
    bool retVal = false;

    // Get the amount of program active uniforms:
    GLint amountOfProgramUniforms = 0;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
    gs_stat_realFunctionPointers.glGetObjectParameterivARB(programName, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &amountOfProgramUniforms);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#else
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
    _glGetObjectParameterivARB(programName, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &amountOfProgramUniforms);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetObjectParameterivARB);
#endif

    retVal = true;

    if (amountOfProgramUniforms > 0)
    {
        // Add the program uniforms to the uniforms container:
        char uniformName[GS_MAX_UNIFORM_NAME + 1];

        for (int i = 0; i < amountOfProgramUniforms; i++)
        {
            // Get the current uniform details:
            uniformName[0] = 0;
            GLsizei nameLength = 0;
            GLint uniformSize = 0;
            GLenum uniformType = 0;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetActiveUniformARB);
            gs_stat_realFunctionPointers.glGetActiveUniformARB(programName, i, GS_MAX_UNIFORM_NAME, &nameLength, &uniformSize,
                                                               &uniformType, uniformName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetActiveUniformARB);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetActiveUniformARB);
            _glGetActiveUniformARB(programName, i, GS_MAX_UNIFORM_NAME, &nameLength, &uniformSize, &uniformType, uniformName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetActiveUniformARB);
#endif

            // TO_DO: Unicode
            gtString uniformNameStr;
            uniformNameStr.fromASCIIString(uniformName);

            // Add them to the uniforms container:
            programUniformsContainer.addItem(uniformNameStr, uniformType, uniformSize, -2, -1);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::update20ProgramAvailableUniforms
// Description: Updates available uniform for an OpenGL 2.0 program
// Arguments: apGLItemsCollection& programUniformsContainer
//            GLuint programName
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/11/2005
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::update20ProgramAvailableUniforms(apGLItemsCollection& programUniformsContainer, GLuint programName)
{
    bool retVal = false;

    // Get the amount of program active uniforms:
    GLint amountOfProgramUniforms = 0;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
    gs_stat_realFunctionPointers.glGetProgramiv(programName, GL_ACTIVE_UNIFORMS, &amountOfProgramUniforms);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
#else
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
    _glGetProgramiv(programName, GL_ACTIVE_UNIFORMS, &amountOfProgramUniforms);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
#endif

    retVal = true;

    if (amountOfProgramUniforms > 0)
    {
        // Add the program uniforms to the uniforms container:
        char uniformName[GS_MAX_UNIFORM_NAME + 1];

        for (int i = 0; i < amountOfProgramUniforms; i++)
        {
            // Get the current uniform details:
            uniformName[0] = 0;
            GLsizei nameLength = 0;
            GLint uniformSize = 0;
            GLenum uniformType = 0;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetActiveUniform);
            gs_stat_realFunctionPointers.glGetActiveUniform(programName, i, GS_MAX_UNIFORM_NAME, &nameLength, &uniformSize,
                                                            &uniformType, uniformName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetActiveUniform);
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetActiveUniform);
            _glGetActiveUniform(programName, i, GS_MAX_UNIFORM_NAME, &nameLength, &uniformSize, &uniformType, uniformName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetActiveUniform);
#endif

            // TO_DO: Unicode
            gtString uniformNameStr;
            uniformNameStr.fromASCIIString(uniformName);

            // Add them to the uniforms container:
            programUniformsContainer.addItem(uniformNameStr, uniformType, uniformSize, -2, -1);
        }
    }

    if (_isUniformBufferObjectsExtSupported)
    {
        // Get the amount of program active uniforms blocks:
        GLint amountOfProgramUniformsBlocks = 0;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        gs_stat_realFunctionPointers.glGetProgramiv(programName, GL_ACTIVE_UNIFORM_BLOCKS, &amountOfProgramUniformsBlocks);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
        _glGetProgramiv(programName, GL_ACTIVE_UNIFORM_BLOCKS, &amountOfProgramUniformsBlocks);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramiv);
#endif

        if (amountOfProgramUniformsBlocks > 0)
        {
            // Add the program uniforms to the uniforms container:
            char uniformName[GS_MAX_UNIFORM_NAME + 1];

            for (int i = 0; i < amountOfProgramUniforms; i++)
            {
                // Get the current uniform details:
                uniformName[0] = 0;
                GLsizei nameLength = 0;
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetActiveUniformBlockName);
                _glGetActiveUniformBlockName(programName, i, GS_MAX_UNIFORM_NAME, &nameLength, uniformName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetActiveUniformBlockName);

                // Get the UBO index:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetUniformBlockIndex);
                GLuint uniformBlockIndex = _glGetUniformBlockIndex(programName, uniformName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetUniformBlockIndex);

                // TO_DO: Unicode
                gtString uniformNameStr;
                uniformNameStr.fromASCIIString(uniformName);

                // Add them to the uniforms container:
                programUniformsContainer.addItem(uniformNameStr, GL_UNIFORM_BUFFER, 0, -2, uniformBlockIndex);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::onUBOBindBuffer
// Description: Attach a buffer to a UBO binding point for a program
// Arguments: GLuint program
//            GLenum target
//            GLuint index
//            GLuint buffer
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/11/2009
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::onUBOBindBuffer(GLuint programName, GLenum target, GLuint index, GLuint buffer)
{
    bool retVal = false;

    if ((target == GL_UNIFORM_BUFFER) || (target == GL_UNIFORM_BUFFER_EXT))
    {
        // Get the index of the program in this class vectors:
        int vecIndex = programsVecIndex(programName);

        if (vecIndex != -1)
        {
            // Get the programs uniforms data:
            gsProgramUniformsData* pProgramUniformsData = _programUniformsData[vecIndex];

            if (pProgramUniformsData != NULL)
            {
                // Add the UBO attachment to the program:
                pProgramUniformsData->addUBOAttachmentPoint(index, buffer);
            }
        }
    }
    else
    {
        // TO_DO: OpenGL3.2 add detected error
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsActiveUniformsMonitor::onUniformBlockBinding
// Description: Attach a uniform block index to a uniform block binding
// Arguments: GLuint programName
//            GLuint uniformBlockIndex
//            GLuint uniformBlockBinding
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/11/2009
// ---------------------------------------------------------------------------
bool gsActiveUniformsMonitor::onUniformBlockBinding(GLuint programName, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    (void)(uniformBlockBinding); // unused
    bool retVal = false;

    // Get the index of the program in this class vectors:
    int vecIndex = programsVecIndex(programName);

    if (vecIndex != -1)
    {
        // Get the programs uniforms data:
        gsProgramUniformsData* pProgramUniformsData = _programUniformsData[vecIndex];

        if (pProgramUniformsData != NULL)
        {
            // Add the UBO attachment to the program:
            GLuint buffer = 0;
            bool rc = pProgramUniformsData->getUBOBuffer(uniformBlockIndex, buffer);

            if (!rc)
            {
                buffer = 0;
            }

            // Get the program uniforms container:
            apGLItemsCollection& programUniformsContainer = pProgramUniformsData->programActiveUniforms();

            // Get the item with 'uniformBlockIndex':
            int itemIndex = programUniformsContainer.itemIndex((GLint)uniformBlockIndex);

            // If the current uniform value is accessible:
            if (itemIndex != -1)
            {
                // Will get the uniform value:
                gtAutoPtr<apParameter> aptrUniformValue;

                if (buffer != 0)
                {
                    // Create a GLUint parameter with the buffer name:
                    apGLuintParameter* pParam = new apGLuintParameter;

                    // Set the buffer as the int parameter value:
                    pParam->setValueFromInt(buffer);
                    aptrUniformValue = pParam;
                }

                if (nullptr == aptrUniformValue.pointedObject())
                {
                    aptrUniformValue = new apNotAvailableParameter;
                }

                // Update the uniform value:
                programUniformsContainer.setItemValue(itemIndex, aptrUniformValue);
            }
        }
    }

    return retVal;
}

