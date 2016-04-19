//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsProgramUniformsData.cpp
///
//==================================================================================

//------------------------------ gsProgramUniformsData.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>

// Local:
#include <src/gsProgramUniformsData.h>


// ---------------------------------------------------------------------------
// Name:        gsProgramUniformsData::gsProgramUniformsData
// Description: Constructor
// Arguments:   programName - The program name.
//              programType - The program type.
// Author:      Yaki Tebeka
// Date:        14/6/2005
// ---------------------------------------------------------------------------
gsProgramUniformsData::gsProgramUniformsData(GLuint programName, apGLShadingObjectType programType)
    : _programName(programName), _programType(programType), _isProgramLinkedSuccessfully(false)
{
}


// ---------------------------------------------------------------------------
// Name:        gsProgramUniformsData::~gsProgramUniformsData
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        14/6/2005
// ---------------------------------------------------------------------------
gsProgramUniformsData::~gsProgramUniformsData()
{
}


// ---------------------------------------------------------------------------
// Name:        gsProgramUniformsData::getUniformLocation
// Description: Inputs a uniform name and outputs the uniform location in its program.
// Arguments:   uniformName - The queried uniform name.
// Return Val:  GLint - The uniform location, or -1 if the location is not available.
// Author:      Yaki Tebeka
// Date:        15/6/2005
// ---------------------------------------------------------------------------
GLint gsProgramUniformsData::getUniformLocation(const gtString& uniformName) const
{
    GLint retVal = -1;

    // Look for the uniform name in the _programActiveUniformLocations map:
    gtMap<gtString, GLint>::const_iterator iter = _programActiveUniformLocations.find(uniformName);

    // If we found the uniform name:
    if (iter != _programActiveUniformLocations.end())
    {
        // Return the its location:
        retVal = (*iter).second;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramUniformsData::uniformIndexFromLocation
// Description: Returns a uniform's index in _programActiveUniforms from its
//              OpenGL location
// Author:      Uri Shomroni
// Date:        16/12/2009
// ---------------------------------------------------------------------------
int gsProgramUniformsData::uniformIndexFromLocation(GLint uniformLocation)
{
    int retVal = -1;

    // Look for the uniform name in the _programActiveUniformLocations map:
    gtMap<GLint, int>::const_iterator iter = _programActiveUniformLocationToIndex.find(uniformLocation);

    // If we found the uniform name:
    if (iter != _programActiveUniformLocationToIndex.end())
    {
        // Return the its location:
        retVal = (*iter).second;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramUniformsData::clearUniformsData
// Description: Clears the uniform data.
// Author:      Yaki Tebeka
// Date:        15/6/2005
// ---------------------------------------------------------------------------
void gsProgramUniformsData::clearUniformsData()
{
    _programActiveUniformLocations.clear();
    _programActiveUniforms.clear();
}


// ---------------------------------------------------------------------------
// Name:        gsProgramUniformsData::setUniformLocation
// Description: Sets a uniform location.
// Arguments:   uniformName - The uniform name
//              uniformLocation - The uniform location in its program.
// Author:      Yaki Tebeka
// Date:        15/6/2005
// ---------------------------------------------------------------------------
void gsProgramUniformsData::setUniformLocation(const gtString& uniformName, GLint uniformLocation, int vectorIndex)
{
    _programActiveUniformLocations[uniformName] = uniformLocation;
    _programActiveUniformLocationToIndex[uniformLocation] = vectorIndex;
    _programActiveUniforms.setItemLocation(vectorIndex, uniformLocation);
}



// ---------------------------------------------------------------------------
// Name:        gsProgramUniformsData::addUBOAttachmentPoint
// Description: Attach an index to a named buffer for UBOs of this program
// Arguments: GLuint index
//            GLuint buffer
// Return Val: void
// Author:      Sigal Algranaty
// Date:        1/11/2009
// ---------------------------------------------------------------------------
void gsProgramUniformsData::addUBOAttachmentPoint(GLuint index, GLuint buffer)
{
    // Attach buffer to attachment point 'index':
    _programActiveUniformBufferObjectAttachments[index] = buffer;
}

// ---------------------------------------------------------------------------
// Name:        gsProgramUniformsData::getUBOBuffer
// Description: Return a uniform buffer object buffer by index
// Arguments: GLuint index
//            GLuint& buffer
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/11/2009
// ---------------------------------------------------------------------------
bool gsProgramUniformsData::getUBOBuffer(GLuint index, GLuint& buffer)
{
    bool retVal = false;
    buffer = 0;
    gtMap<GLuint, GLuint>::const_iterator iterFind = _programActiveUniformBufferObjectAttachments.find(index);

    if (iterFind != _programActiveUniformBufferObjectAttachments.end())
    {
        buffer = _programActiveUniformBufferObjectAttachments[index];
        retVal = true;
    }

    return retVal;
}

