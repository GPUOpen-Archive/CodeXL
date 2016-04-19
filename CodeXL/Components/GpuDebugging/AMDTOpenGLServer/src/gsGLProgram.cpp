//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsGLProgram.cpp
///
//==================================================================================

//------------------------------ gsGLProgram.cpp ------------------------------


// Infra:
#include <AMDTAPIClasses/Include/apGLShaderObject.h>

// Local:
#include <src/gsGLProgram.h>


// ---------------------------------------------------------------------------
// Name:        gsGLProgram::gsGLProgram
// Description: Constructor
// Arguments:   programName - The program name.
//              programType - The program type.
// Author:      Yaki Tebeka
// Date:        12/4/2005
// ---------------------------------------------------------------------------
gsGLProgram::gsGLProgram(GLuint programName, apGLShadingObjectType programType)
    : apGLProgram(programName, programType), _amountOfAttachedGeometryShaders(0),
      _isStubGeometryShaderAttached(false), _amountOfAttachedFragmentShaders(0),
      _isStubFragmentShaderAttached(false), _wasProgramRestoredFromStubFS(false),
      _wasUsedInCurrentFrame(false)
{
}


// ---------------------------------------------------------------------------
// Name:        gsGLProgram::attachShaderObject
// Description: Is called when a shader object is attached to this program.
// Arguments:   shaderObj - The attached shader object.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/4/2005
// ---------------------------------------------------------------------------
bool gsGLProgram::onShaderObjectAttached(const apGLShaderObject& shaderObj)
{
    bool retVal = false;

    // Call my parent class:
    retVal = apGLProgram::onShaderObjectAttached(shaderObj);

    if (retVal)
    {
        // Get the shader object type:
        osTransferableObjectType objType = shaderObj.type();

        // If this is a geometry shader:
        if (objType == OS_TOBJ_ID_GL_GEOMETRY_SHADER)
        {
            // Increment the amount of attached geometry shaders:
            _amountOfAttachedGeometryShaders++;
        }

        // If this is a fragment shader:
        if (objType == OS_TOBJ_ID_GL_FRAGMENT_SHADER)
        {
            // Increment the amount of attached fragment shaders:
            _amountOfAttachedFragmentShaders++;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsGLProgram::onShaderObjectDetached
// Description: Is called when a shader object is detached from this program.
// Arguments:   shaderObj - The detached shader object.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/4/2005
// ---------------------------------------------------------------------------
bool gsGLProgram::onShaderObjectDetached(const apGLShaderObject& shaderObj)
{
    bool retVal = false;

    // Call my parent class:
    retVal = apGLProgram::onShaderObjectDetached(shaderObj);

    if (retVal)
    {
        // Get the shader object type:
        osTransferableObjectType objType = shaderObj.type();

        // If this is a fragment shader:
        if (objType == OS_TOBJ_ID_GL_GEOMETRY_SHADER)
        {
            // Decrement the amount of attached geometry shaders:
            _amountOfAttachedGeometryShaders--;
        }

        // If this is a fragment shader:
        if (objType == OS_TOBJ_ID_GL_FRAGMENT_SHADER)
        {
            // Decrement the amount of attached fragment shaders:
            _amountOfAttachedFragmentShaders--;
        }
    }

    return retVal;
}


